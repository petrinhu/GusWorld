// cards_hw_lifecycle_integration_test.cpp
//
// Spec executavel de INTEGRACAO (Catch2 v3) da onda cartas-hardware (CARDS-HW-QA1,
// TODO.md): fecha os elos ENTRE os servicos ja cobertos isoladamente por unit tests
// (contamination_service_test.cpp, turing_service_test.cpp, deck_transactions_test.cpp,
// save_v7_test.cpp, backdoor_signal_test.cpp) - aqui o objetivo NAO e duplicar a logica
// fina de cada um (fronteiras exatas, distribuicao estatistica, guards isolados), e sim
// provar que o CICLO DE VIDA completo de uma carta atravessa os servicos reais sem
// perder estado nas costuras: aquisicao (deck_transactions) -> infeccao oculta
// (contamination_service) -> persistencia (save_serializer V7) -> diagnostico/cura
// (turing_service) -> persistencia de novo -> saida do deck (sell/upload) -> reflexo em
// combate (card_integrity_ledger).
//
// Cobre (1 TEST_CASE por cadeia, asserts de estado COMPLETO a cada elo):
//   1. acquire() infectado oculto -> roundtrip V7 no meio (infeccao oculta sobrevive
//      byte a byte) -> diagnose -> attempt_cure (queima) -> roundtrip V7 de novo
//      (is_burned_out + is_infected + virus_kind simultaneos) -> sell() da sucata
//      (comportamento ATUAL permitido, preco //PLAYTEST) -> orfao removido da mao.
//   2. craft() infectado -> diagnose -> attempt_cure (cura) -> roundtrip V7 do estado
//      limpo -> reinfeccao via NOVA acquire() (instance_id nunca reusa entre geracoes).
//   3. upload() de carta infectada NAO diagnosticada - pin do comportamento ATUAL do
//      codigo (achado reportado, NAO consertado aqui).
//   4. cura fora de combate reflete AO VIVO no card_integrity_ledger (leaked_intel
//      recomputado por turno, nao um snapshot congelado).
//
// ACHADO QA (nao consertado aqui, so documentado): CardCollection::active()/dead() sao
// views READ-ONLY (card_collection.hpp) - nao existe hoje um jeito de MUTAR o
// CardPhysicalState de uma instancia JA presente no deck ativo (turing_service::
// diagnose()/attempt_cure() operam sobre um CardPhysicalState& que o CHAMADOR precisa
// manter por fora; add_to_active(..., initial_physical) so serve pra CRIAR uma
// instancia NOVA, o guard XOR de instance_id recusa reusar o mesmo id pra "atualizar").
// Os testes abaixo refletem esse gap explicitamente: a copia de CardInstance devolvida
// pelas transacoes (`current`/`crafted`) e o "dono" do estado fisico ao longo da cadeia,
// NAO o vetor interno de CardCollection (que fica esperado como referencia congelada no
// momento da aquisicao). Onda futura (gameplay_engineer) precisa de uma API de
// write-back antes de orquestrar cura de verdade dentro do agregado real.
//
// Cross-ref: docs/design/mecanicas/cartas-spec-dados.md secao 5/6/7/10 (CardPhysicalState,
//            invariantes, save V7); docs/design/mecanicas/cartas-spec-logica.md secao 5.1/6
//            (contaminacao na aquisicao, AttemptCure); docs/design/mecanicas/
//            deck-mao-sistema.md secao 6.1/6.2/7 (acquire/craft/sell/upload, orfaos da
//            mao); gus/domain/deck/{contamination_service,turing_service,
//            deck_transactions,card_collection,hand_loadout,card_hardware}.hpp;
//            gus/domain/save/save_serializer.hpp; gus/domain/combat/
//            card_integrity_ledger.hpp; TODO.md CARDS-HW-QA1.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "fixed_random.hpp"
#include "gus/domain/cards/card_enums.hpp"
#include "gus/domain/combat/card_integrity_ledger.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/domain/combat/enemy_brain.hpp"
#include "gus/domain/combat/random_source.hpp"
#include "gus/domain/deck/card_collection.hpp"
#include "gus/domain/deck/card_hardware.hpp"
#include "gus/domain/deck/card_hardware_constants.hpp"
#include "gus/domain/deck/contamination_service.hpp"
#include "gus/domain/deck/deck_constants.hpp"
#include "gus/domain/deck/deck_records.hpp"
#include "gus/domain/deck/deck_transactions.hpp"
#include "gus/domain/deck/hand_loadout.hpp"
#include "gus/domain/deck/turing_service.hpp"
#include "gus/domain/domain_info.hpp"
#include "gus/domain/infection/integrity_state.hpp"
#include "gus/domain/save/save_data.hpp"
#include "gus/domain/save/save_serializer.hpp"

using gus::domain::cards::CardTier;
using gus::domain::deck::acquire;
using gus::domain::deck::AcquireResult;
using gus::domain::deck::attempt_cure;
using gus::domain::deck::CardCollection;
using gus::domain::deck::CardInstance;
using gus::domain::deck::CardOrigin;
using gus::domain::deck::CardPhysicalState;
using gus::domain::deck::ContaminationRollOutcome;
using gus::domain::deck::craft;
using gus::domain::deck::CraftResult;
using gus::domain::deck::CureOutcome;
using gus::domain::deck::diagnose;
using gus::domain::deck::DiagnoseOutcome;
using gus::domain::deck::HandLoadout;
using gus::domain::deck::kDeckCapacityTier1;
using gus::domain::deck::kHandSizeBase;
using gus::domain::deck::kNpcSellPriceMin;
using gus::domain::deck::kShopBuyPriceMax;
using gus::domain::deck::kTuringCureSuccessPercent;
using gus::domain::deck::kUploadCreditMax;
using gus::domain::deck::sell;
using gus::domain::deck::SellResult;
using gus::domain::deck::TransactionError;
using gus::domain::deck::upload;
using gus::domain::deck::UploadResult;
using gus::domain::deck::VirusKind;
using gus::domain::save::CardCollectionState;
using gus::domain::save::CharacterSaveState;
using gus::domain::save::deserialize_save;
using gus::domain::save::SaveData;
using gus::domain::save::serialize_save;
using gus::domain::tests::FixedRandom;

namespace {

// Mesma convencao de deck_transactions_test.cpp/hand_loadout_test.cpp: sufixo
// "_especial"/"_super" marca a classe protegida (secao 7 inv.9), sem depender do
// registry real de combate.
CardTier fake_tier_of(const std::string& card_id) {
    if (card_id.find("_especial") != std::string::npos) return CardTier::Especial;
    if (card_id.find("_super") != std::string::npos) return CardTier::Super;
    return CardTier::Comum;
}

// Monta um SaveData V7 minimo carregando UMA CardInstance no ativo do "gus" - reusado
// pelos 2 roundtrips da cadeia 1 e pelo roundtrip da cadeia 2 (mesmo padrao de
// rich_v7()/legacy_v6_fixture() em save_v7_test.cpp, so que parametrizado pela
// instancia que vem de um servico REAL, nao de uma fixture escrita a mao).
SaveData make_save_with_card(const CardInstance& inst, std::int64_t timestamp_ms) {
    SaveData s;
    s.schema_version = gus::domain::kSaveSchemaVersion;
    s.timestamp_ms = timestamp_ms;
    s.current_scene_path = "res://world/gusworld_city.tscn";
    s.party_roster = {"gus"};
    s.party_active = {"gus"};

    CharacterSaveState gus_state;
    gus_state.current_hp = 34;
    gus_state.xp = 55;
    gus_state.card_collection.active = {inst};
    gus_state.card_collection.next_instance_id = inst.instance_id + 1;

    s.character_states = {{"gus", gus_state}};
    s.credits = 21;
    return s;
}

// ---- helpers de combate (mesmo padrao de backdoor_signal_test.cpp) -----------------

gus::domain::combat::CombatActor make_actor(const std::string& id, bool player_side,
                                             int hp = 100, int atk = 0, int def = 0,
                                             int spd = 20,
                                             gus::domain::combat::CardFamily family =
                                                 gus::domain::combat::CardFamily::Eletrico) {
    return gus::domain::combat::CombatActor(id, id, hp, atk, def, spd, family, player_side);
}

gus::domain::combat::Card damage_card(const std::string& id, int power = 0,
                                       CardTier tier = CardTier::Comum) {
    gus::domain::combat::Card c;
    c.id = id;
    c.display_name = id;
    c.family = gus::domain::combat::CardFamily::Cinetico;
    c.base_type = gus::domain::combat::CardBaseType::Pulso;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = gus::domain::combat::TargetShape::Single;
    c.tier = tier;
    return c;
}

}  // namespace

// ===== Cadeia 1: acquire() infectado -> roundtrip -> diagnose -> queima -> roundtrip ->
//       sell() da sucata -> orfao removido da mao =====================================

TEST_CASE("cards_hw_lifecycle: acquire() infectado oculto sobrevive a 2 roundtrips V7, "
          "diagnose+queima e sell() remove a sucata (orfao some da mao)",
          "[domain][cards_hw][integration]") {
    CardCollection deck(kDeckCapacityTier1);
    std::int64_t credits = kShopBuyPriceMax;
    FixedRandom infect_rng(0.5, 0);  // draw=0 - infecta qualquer classe nao-protegida

    // ---- elo 1: acquire() com RNG forcando infeccao (draw=0, ComumOriginal risco 1%) --
    const AcquireResult acq =
        acquire(deck, credits, "pulso_infectado", kShopBuyPriceMax, fake_tier_of, infect_rng);

    REQUIRE(acq.ok());
    REQUIRE(acq.contamination == ContaminationRollOutcome::Infected);
    REQUIRE(acq.instance.physical.is_infected);
    REQUIRE_FALSE(acq.instance.physical.is_diagnosed);
    REQUIRE(acq.instance.physical.virus_kind != VirusKind::None);
    REQUIRE_NOTHROW(acq.instance.physical.validate());
    REQUIRE(deck.active_count() == 1);
    // Persistiu no container tambem, nao so na copia devolvida (elo com card_collection).
    REQUIRE(deck.active().front().physical.is_infected);

    // Guard defensivo: Especial NUNCA infecta, mesmo com o MESMO RNG que acabou de
    // infectar a carta Comum acima (verifica em TODO ponto de aquisicao desta cadeia).
    CardCollection special_deck(kDeckCapacityTier1);
    std::int64_t special_credits = 1000;
    const AcquireResult special_acq = acquire(special_deck, special_credits, "carta_especial",
                                               0, fake_tier_of, infect_rng);
    REQUIRE(special_acq.ok());
    REQUIRE_FALSE(special_acq.instance.physical.is_infected);
    REQUIRE(special_acq.contamination == ContaminationRollOutcome::SkippedProtectedTier);

    // `current` e o "dono" do estado fisico threadado pela cadeia inteira (a copia
    // devolvida por acquire() - ver ACHADO QA no topo do arquivo sobre a ausencia de
    // write-back no CardCollection real).
    CardInstance current = acq.instance;

    // ---- elo 2: roundtrip V7 NO MEIO da cadeia - infeccao oculta sobrevive byte a byte
    const SaveData original = make_save_with_card(current, /*timestamp_ms=*/1721150000000LL);
    const SaveData restored = deserialize_save(serialize_save(original));
    const CardInstance& restored_hidden =
        restored.character_states.at("gus").card_collection.active.at(0);

    REQUIRE(restored_hidden == current);
    REQUIRE(restored_hidden.physical.is_infected);
    REQUIRE_FALSE(restored_hidden.physical.is_diagnosed);
    REQUIRE(restored_hidden.physical.virus_kind == current.physical.virus_kind);
    REQUIRE_NOTHROW(restored_hidden.physical.validate());

    // ---- elo 3: diagnose() confirma a infeccao (Turing revela o que estava oculto) ---
    const DiagnoseOutcome diag_outcome = diagnose(current.physical);
    REQUIRE(diag_outcome == DiagnoseOutcome::Diagnosed);
    REQUIRE(current.physical.is_diagnosed);
    REQUIRE_NOTHROW(current.physical.validate());

    // ---- elo 4: attempt_cure() com roll de QUEIMA (borda exata, >=62 - AMB-T1) -------
    FixedRandom burn_rng(0.5, kTuringCureSuccessPercent);  // ==62, borda: queima
    const CureOutcome cure_outcome = attempt_cure(current.physical, CardTier::Comum, burn_rng);

    REQUIRE(cure_outcome == CureOutcome::Burned);
    REQUIRE(current.physical.is_burned_out);
    // AMB-T1 RESOLVIDA: queima = sucata, NAO destruicao - is_infected/virus_kind ficam
    // como estavam (nao e revertido pela queima).
    REQUIRE(current.physical.is_infected);
    REQUIRE(current.physical.virus_kind == restored_hidden.physical.virus_kind);
    REQUIRE(current.physical.is_diagnosed);
    REQUIRE_NOTHROW(current.physical.validate());

    // ---- elo 5: roundtrip V7 DEPOIS da queima - is_burned_out + is_infected +
    //      virus_kind simultaneos sobrevivem juntos ---------------------------------
    const SaveData original_burned = make_save_with_card(current, /*timestamp_ms=*/1721150100000LL);
    const SaveData restored_burned = deserialize_save(serialize_save(original_burned));
    const CardInstance& restored_sucata =
        restored_burned.character_states.at("gus").card_collection.active.at(0);

    REQUIRE(restored_sucata == current);
    REQUIRE(restored_sucata.physical.is_burned_out);
    REQUIRE(restored_sucata.physical.is_infected);
    REQUIRE(restored_sucata.physical.virus_kind == current.physical.virus_kind);
    REQUIRE(restored_sucata.physical.is_diagnosed);
    REQUIRE_NOTHROW(restored_sucata.physical.validate());

    // ---- elo 6: sell() da sucata - pin do comportamento ATUAL (permitido) -----------
    //
    // A spec (cartas-spec-logica.md secao 6, AMB-T1) diz que a carta queimada "vende no
    // ferro-velho" - venda permitida e o comportamento CANONICO; preco DIFERENCIADO por
    // sucata (vs carta saudavel) e design futuro, ainda nao existe (deck_transactions.cpp
    // nao le is_burned_out/is_infected em NENHUM ponto - so o guard de tier Especial/
    // Super de CardCollection::remove_for_sale). //PLAYTEST: kNpcSellPriceMin aqui e o
    // MESMO preco de uma carta saudavel, sem penalidade/bonus por estado fisico.
    //
    // ACHADO QA (ver topo do arquivo): `deck` (o CardCollection real) ainda guarda a
    // copia ORIGINAL da instancia (infectada-oculta, pre-diagnose/pre-queima) - o
    // dominio nao tem write-back. Pra vender a copia REALMENTE queimada (`current`),
    // usamos um CardCollection dedicado onde injetamos o estado fisico final via
    // add_to_active(..., instance_id_override, initial_physical) - a UNICA via hoje
    // pra um CardCollection guardar um physical especifico, mesmo que so na CRIACAO.
    CardCollection sell_deck(kDeckCapacityTier1);
    sell_deck.add_to_active(current.card_id, current.instance_id, current.physical);

    // A instancia tambem estava na mao (bancada) antes da venda.
    HandLoadout hand(kHandSizeBase);
    hand.add_card(current.instance_id, sell_deck, fake_tier_of);
    REQUIRE(hand.size() == 1);

    std::int64_t sell_credits = 0;
    const SellResult sell_result =
        sell(sell_deck, sell_credits, current.instance_id, kNpcSellPriceMin, fake_tier_of);

    REQUIRE(sell_result.ok());
    REQUIRE(sell_result.instance.instance_id == current.instance_id);
    // A sucata sai do ativo do jeito que estava - sell() nao apaga/reseta o estado
    // fisico da instancia devolvida, so remove do container.
    REQUIRE(sell_result.instance.physical.is_burned_out);
    REQUIRE(sell_result.instance.physical.is_infected);
    REQUIRE(sell_result.credited == kNpcSellPriceMin);
    REQUIRE(sell_credits == kNpcSellPriceMin);
    REQUIRE(sell_deck.active_count() == 0);

    // ---- elo 7: orfao na mao (find_orphan_instance_ids + revalidate a removem) ------
    const auto orphans = hand.find_orphan_instance_ids(sell_deck);
    REQUIRE(orphans.size() == 1);
    REQUIRE(orphans.front() == current.instance_id);

    const auto removed = hand.revalidate(sell_deck);
    REQUIRE(removed.size() == 1);
    REQUIRE(removed.front() == current.instance_id);
    REQUIRE(hand.size() == 0);
}

// ===== Cadeia 2: craft() infectado -> diagnose -> cura -> roundtrip limpo -> reinfeccao
//       via NOVA acquire() (instance_id NUNCA reusa entre geracoes) ====================

TEST_CASE("cards_hw_lifecycle: craft() infectado -> diagnose+cura -> roundtrip V7 limpo -> "
          "reinfeccao via novo acquire() com instance_id NUNCA reusado",
          "[domain][cards_hw][integration]") {
    CardCollection deck(kDeckCapacityTier1);
    std::int64_t credits = kShopBuyPriceMax;

    auto consumer = []() { return true; };
    FixedRandom infect_rng(0.5, 0);  // draw=0 - infecta HomebrewEprom (risco 55%)

    // ---- elo 1: craft() com RNG forcando infeccao -----------------------------------
    const CraftResult craft_result =
        craft(deck, "carta_craftada_infectada", consumer, fake_tier_of, infect_rng);

    REQUIRE(craft_result.ok());
    REQUIRE(craft_result.contamination == ContaminationRollOutcome::Infected);
    REQUIRE(craft_result.instance.physical.origin == CardOrigin::HomebrewEprom);
    REQUIRE(craft_result.instance.physical.is_infected);
    REQUIRE_FALSE(craft_result.instance.physical.is_diagnosed);
    REQUIRE_NOTHROW(craft_result.instance.physical.validate());

    // Guard defensivo: craft() NUNCA crafta Especial/Super por design (secao 9 inv.9),
    // mas o guard defensivo da rolagem de contaminacao se aplica igual - reforca "nunca
    // aparece infectada" tambem no canal craft.
    auto special_consumer = []() { return true; };
    const CraftResult special_craft =
        craft(deck, "carta_craftada_especial", special_consumer, fake_tier_of, infect_rng);
    REQUIRE(special_craft.ok());
    REQUIRE_FALSE(special_craft.instance.physical.is_infected);
    REQUIRE(special_craft.contamination == ContaminationRollOutcome::SkippedProtectedTier);

    CardInstance crafted = craft_result.instance;

    // ---- elo 2: diagnose() + attempt_cure() com roll de CURA (<62) -------------------
    REQUIRE(diagnose(crafted.physical) == DiagnoseOutcome::Diagnosed);
    REQUIRE(crafted.physical.is_diagnosed);

    FixedRandom cure_rng(0.5, 0);  // <62 => Cured
    const CureOutcome cure_outcome = attempt_cure(crafted.physical, CardTier::Comum, cure_rng);

    REQUIRE(cure_outcome == CureOutcome::Cured);
    REQUIRE_FALSE(crafted.physical.is_infected);
    REQUIRE(crafted.physical.virus_kind == VirusKind::None);
    REQUIRE_FALSE(crafted.physical.is_diagnosed);
    REQUIRE_FALSE(crafted.physical.is_burned_out);
    REQUIRE_NOTHROW(crafted.physical.validate());

    // ---- elo 3: roundtrip V7 do estado LIMPO ------------------------------------------
    const SaveData original = make_save_with_card(crafted, /*timestamp_ms=*/1721150200000LL);
    const SaveData restored = deserialize_save(serialize_save(original));
    const CardInstance& restored_clean =
        restored.character_states.at("gus").card_collection.active.at(0);

    REQUIRE(restored_clean == crafted);
    REQUIRE_FALSE(restored_clean.physical.is_infected);
    REQUIRE(restored_clean.physical.virus_kind == VirusKind::None);
    REQUIRE_FALSE(restored_clean.physical.is_diagnosed);
    REQUIRE_NOTHROW(restored_clean.physical.validate());

    // ---- elo 4: reinfeccao via NOVA acquire() - instance_id NUNCA reusa --------------
    // `deck` ja tem 2 instancias (craftada infectada + craftada especial), next_instance_id
    // avancou por conta delas - a nova instancia adquirida abaixo NUNCA pode colidir com
    // `crafted.instance_id`, mesmo que ele ja tenha virado limpo/reentrado no ciclo.
    FixedRandom reinfect_rng(0.5, 0);  // draw=0 - infecta de novo (ComumOriginal risco 1%)
    const AcquireResult reacquired =
        acquire(deck, credits, "pulso_reinfectado", kShopBuyPriceMax, fake_tier_of, reinfect_rng);

    REQUIRE(reacquired.ok());
    REQUIRE(reacquired.contamination == ContaminationRollOutcome::Infected);
    REQUIRE(reacquired.instance.physical.is_infected);
    REQUIRE(reacquired.instance.instance_id != crafted.instance_id);
    // Determinismo do contador sequencial (card_collection.hpp inv.1): craftada(1) +
    // craftada_especial(2) + esta nova(3) - NUNCA reusa, mesmo apos o ciclo de cura.
    REQUIRE(reacquired.instance.instance_id == 3);
    REQUIRE_NOTHROW(reacquired.instance.physical.validate());
}

// ===== Cadeia 3: upload() de carta infectada NAO diagnosticada - pin do comportamento
//       ATUAL (achado reportado, NAO consertado aqui) ==================================

TEST_CASE("cards_hw_lifecycle: upload() de carta infectada NAO diagnosticada e PERMITIDO "
          "pelo codigo atual (pin - deck_transactions.cpp nao gate por is_infected)",
          "[domain][cards_hw][integration]") {
    CardCollection deck(kDeckCapacityTier1);
    std::int64_t credits = 0;
    FixedRandom infect_rng(0.5, 0);  // draw=0 - infecta

    const AcquireResult acq =
        acquire(deck, credits, "pulso_sujo_upload", /*price=*/0, fake_tier_of, infect_rng);

    REQUIRE(acq.ok());
    REQUIRE(acq.instance.physical.is_infected);
    REQUIRE_FALSE(acq.instance.physical.is_diagnosed);

    // ACHADO QA (lido em deck_transactions.cpp: sell()/upload() chamam a MESMA funcao
    // interna remove_and_credit(), que so valida presenca-no-ativo + guard de tier
    // Especial/Super - NENHUM dos dois le physical.is_infected/is_diagnosed em momento
    // algum). Resultado: hoje e POSSIVEL reciclar/doar ao repositorio-commons
    // (deck-mao-sistema.md secao 6.2, "sera reusado por devs precisando de ideia") uma
    // carta infectada NUNCA diagnosticada, sem aviso nenhum - a spec de deck-mao-sistema
    // NAO menciona esse cruzamento com o sistema de virus (cartas-spec-logica.md). Isto
    // e reportado aqui como ACHADO de design/spec (nao um bug de codigo - o codigo faz
    // exatamente o que os 2 docs, lidos isoladamente, descrevem) - NAO conserto aqui,
    // fica pra decisao do lider/game designer se upload() deveria exigir diagnose antes.
    const UploadResult upload_result =
        upload(deck, credits, acq.instance.instance_id, kUploadCreditMax, fake_tier_of);

    REQUIRE(upload_result.ok());
    REQUIRE(upload_result.instance.instance_id == acq.instance.instance_id);
    // Pin: a instancia sai do ativo AINDA infectada e AINDA nao-diagnosticada - upload()
    // nao muda nem gate no estado fisico.
    REQUIRE(upload_result.instance.physical.is_infected);
    REQUIRE_FALSE(upload_result.instance.physical.is_diagnosed);
    REQUIRE(upload_result.credited == kUploadCreditMax);
    REQUIRE(credits == kUploadCreditMax);
    REQUIRE(deck.active_count() == 0);
    REQUIRE_NOTHROW(upload_result.instance.physical.validate());

    // Guard defensivo: Especial/Super nunca aparecem infectadas nem chegam a ser
    // candidatas a upload (remove_and_credit rejeita ANTES por ProtectedTier).
    CardCollection special_deck(kDeckCapacityTier1);
    std::int64_t special_credits = 0;
    const AcquireResult special_acq = acquire(special_deck, special_credits, "carta_especial",
                                               0, fake_tier_of, infect_rng);
    REQUIRE(special_acq.ok());
    REQUIRE_FALSE(special_acq.instance.physical.is_infected);

    const UploadResult special_upload = upload(special_deck, special_credits,
                                                special_acq.instance.instance_id,
                                                kUploadCreditMax, fake_tier_of);
    REQUIRE_FALSE(special_upload.ok());
    REQUIRE(special_upload.error == TransactionError::ProtectedTier);
    REQUIRE(special_deck.active_count() == 1);
}

// ===== Cadeia 4: cura fora de combate reflete AO VIVO no card_integrity_ledger =========
//
// leaked_intel() NAO e um snapshot congelado no momento do registro - a
// CombatStateMachine recomputa collect_leaked_intel() a CADA CombatState construido
// (begin_turn/advance_to_next_actor, combat_state_machine.cpp), lendo o IntegrityState*
// do ledger AO VIVO. Como CardPhysicalState HERDA publicamente de IntegrityState
// (card_hardware.hpp), attempt_cure() mutando o MESMO objeto apontado pelo ledger se
// reflete no proximo turno, sem precisar reconstruir/reregistrar nada.

TEST_CASE("cards_hw_lifecycle: attempt_cure() fora de combate faz o dono sumir do "
          "leaked_intel no PROXIMO turno (ledger live, nao snapshot congelado)",
          "[domain][cards_hw][integration]") {
    using gus::domain::combat::CardIntegrityRef;
    using gus::domain::combat::CombatAction;
    using gus::domain::combat::CombatActionProvider;
    using gus::domain::combat::CombatActor;
    using gus::domain::combat::CombatState;
    using gus::domain::combat::CombatStateMachine;

    CombatActor caster = make_actor("h", /*player_side=*/true, /*hp=*/100, /*atk=*/0,
                                     /*def=*/0, /*spd=*/50);
    CombatActor filler = make_actor("e", /*player_side=*/false, /*hp=*/100, /*atk=*/0,
                                     /*def=*/0, /*spd=*/10);

    const gus::domain::combat::Card plain = damage_card("cards_hw_lifecycle.ledger.card");
    std::unordered_map<std::string, gus::domain::combat::Card> reg;
    reg.emplace(plain.id, plain);

    // Carta infectada+diagnosticada com VirusKind::Backdoor (o UNICO virus que alimenta
    // leaked_intel, secao 4.2 - backdoor_signal_test.cpp) - is_diagnosed=true de entrada
    // pra attempt_cure() nao barrar pelo guard RejectedNotDiagnosed.
    CardPhysicalState physical;
    physical.is_infected = true;
    physical.virus_kind = VirusKind::Backdoor;
    physical.is_diagnosed = true;
    REQUIRE_NOTHROW(physical.validate());

    // `&physical` (CardPhysicalState*) converte implicitamente pra IntegrityState*
    // (heranca publica) - o mesmo padrao documentado no header de card_integrity_ledger.
    std::vector<CardIntegrityRef> ledger = {
        CardIntegrityRef{/*instance_id=*/1, plain.id, /*owner_actor_id=*/7, &physical}};

    std::vector<int> leaked_before;
    std::vector<int> leaked_after;
    bool captured_before = false;
    bool captured_after = false;
    CombatActionProvider provider = [&](CombatActor&,
                                         const CombatState& state) -> CombatAction {
        if (!captured_before) {
            leaked_before = state.leaked_intel();
            captured_before = true;
        } else if (!captured_after) {
            leaked_after = state.leaked_intel();
            captured_after = true;
        }
        return CombatAction::pass();
    };

    CombatStateMachine sm({&caster, &filler}, provider, &reg, nullptr, nullptr, &ledger);

    // ---- turno 1 (caster): leaked_intel reporta o dono (owner_actor_id=7) -----------
    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(captured_before);
    REQUIRE(leaked_before == std::vector<int>{7});

    // ---- cura FORA de combate, no MESMO objeto apontado pelo ledger -----------------
    FixedRandom cure_rng(0.5, 0);  // <62 => Cured
    const CureOutcome cure_outcome = attempt_cure(physical, CardTier::Comum, cure_rng);

    REQUIRE(cure_outcome == CureOutcome::Cured);
    REQUIRE_FALSE(physical.is_infected);
    REQUIRE(physical.virus_kind == VirusKind::None);
    REQUIRE_NOTHROW(physical.validate());

    // ---- turno 2 (filler): leaked_intel ja NAO lista mais o dono --------------------
    sm.advance_to_next_actor();
    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(captured_after);
    REQUIRE(leaked_after.empty());
}
