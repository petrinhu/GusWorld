// cards_hw_ops_property_test.cpp
//
// CARDS-HW-QA1 Fatia B (property test STATEFUL/model-based, qa-engineer): sequencias
// aleatorias de operacoes do sistema deck+hardware (acquire/craft/sell/upload/
// discard_to_dead/diagnose/attempt_cure/hand.add_card/hand.remove_card/revalidate),
// checando 8 invariantes globais apos CADA passo, com um MODELO simplificado
// (mapa instance_id -> estado esperado) mantido EM PARALELO e comparado ao sistema
// real. Mesmo estilo "a mao" de deck_invariants_property_test.cpp (LCG semeado,
// property_gen.hpp) - NAO ha rapidcheck/FsCheck linkado (so Catch2).
//
// Invariantes cobertos (numeracao do brief CARDS-HW-QA1):
//   1. toda instancia VIVA (nao removida do sistema por sell/upload) passa
//      CardPhysicalState::validate() sem lancar.
//   2. XOR de containers (cada instance_id em EXATAMENTE ativo OU morto, nunca os
//      dois nem nenhum enquanto viva) + mao subconjunto do ativo APOS revalidate().
//   3. credits nunca negativo + delta da carteira EXATO (contabilidade sombra
//      expected_credits - so sell()/upload() creditam, so acquire() debita,
//      craft() nunca mexe).
//   4. carta CardTier::Especial/Super jamais infectada, jamais sai do ativo por
//      sell/upload/discard_to_dead (outcome de rejeicao - ProtectedTier/
//      std::invalid_argument - conferido no PONTO de cada chamada via o modelo).
//   5. CardPhysicalState::is_burned_out e MONOTONICO (uma vez sucata, nunca
//      reverte - nem apos CureOutcome::Cured, AMB-T1).
//   6. instance_id nunca reusado na vida inteira de UMA sequencia (o modelo nunca
//      esquece um id, mesmo apos sell/upload - so troca pra "removido").
//   7. capacidade do deck ATIVO nunca excedida (patamar 34/55/89,
//      kDeckCapacityTier1/2/3 - deck_constants.hpp - sorteado 1x por seed/setup).
//   8. is_diagnosed==true implica is_infected==true em TODO instante observavel
//      (inv.1 de IntegrityState::validate(), reconferido aqui por cima de
//      sequencias arbitrarias, nao so os casos fixos de turing_service_test.cpp).
//
// ACHADO DE INTEGRACAO (pinado, NAO consertado aqui - QA reporta, nao conserta
// producao): CardCollection SO expoe active()/dead() como views READ-ONLY
// (const std::vector<CardInstance>&) - nao ha NENHUM accessor mutavel pra uma
// CardInstance ja armazenada no deck. turing_service::diagnose()/attempt_cure()
// operam sobre uma CardPhysicalState& (ou IntegrityState&) que o CHAMADOR precisa
// fornecer - hoje NAO EXISTE caminho de producao pra obter essa referencia mutavel
// apontando pra dentro do deck de um personagem (nem CardCollection nem
// HandLoadout expoem isso). Ou seja: diagnose()/attempt_cure() estao PRONTOS e
// testados isoladamente (turing_service_test.cpp) mas AINDA NAO ESTAO FIAVEIS a
// nenhuma carta viva no jogo - onda futura (gameplay_engineer/backend-engineer)
// precisa de algo como `CardInstance* CardCollection::find_mutable_active(id)` ou
// `apply_to_physical(id, fn)` antes disso rodar em producao de verdade. Por isso
// este teste simula diagnose()/attempt_cure() sobre uma COPIA sombra (mc.physical
// no modelo, semeada do CardPhysicalState devolvido por acquire()/craft() na
// criacao) - NAO sobre a instancia dentro de collection.active()/dead() (que e
// congelada por construcao apos a criacao, via a API publica de hoje). O restante
// dos invariantes (XOR/capacidade/carteira/instance_id) SEGUE sendo checado contra
// o CardCollection REAL.
//
// Determinismo TOTAL (LCG + PropertyRandom, property_gen.hpp): 5 seeds x 300
// operacoes, runtime-alvo <1s (mesmo patamar dos property tests vizinhos). Falha
// imprime seed + indice do passo via INFO() do Catch2 (reproducao determinista).
//
// NAO altera codigo de producao (POCO puro e o oraculo). Cross-ref:
// gus/domain/deck/{card_collection,hand_loadout,deck_transactions,turing_service,
// contamination_service,card_hardware,card_hardware_constants,deck_records,
// deck_constants}.hpp; gus/domain/infection/integrity_state.hpp;
// docs/design/mecanicas/{deck-mao-sistema,cartas-spec-dados,cartas-spec-logica,
// cartas-numeros-proposta}.md; property_gen.hpp; deck_invariants_property_test.cpp
// (harness irmao); turing_service_test.cpp (oraculo unitario do split 62/38%).

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "gus/domain/cards/card_enums.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/random_source.hpp"
#include "gus/domain/deck/card_collection.hpp"
#include "gus/domain/deck/card_hardware.hpp"
#include "gus/domain/deck/deck_constants.hpp"
#include "gus/domain/deck/deck_records.hpp"
#include "gus/domain/deck/deck_transactions.hpp"
#include "gus/domain/deck/hand_loadout.hpp"
#include "gus/domain/deck/turing_service.hpp"
#include "gus/domain/infection/integrity_state.hpp"
#include "property_gen.hpp"

using namespace gus::domain::deck;
using gus::domain::combat::CardTier;
using gus::domain::combat::IRandomSource;
using gus::domain::infection::IntegrityState;
using gus::domain::infection::VirusKind;
using gus::domain::tests::Lcg;
using gus::domain::tests::PropertyRandom;

namespace {

constexpr int kSeedsCount = 5;
constexpr int kStepsPerSeed = 300;

// Patamares reais de capacidade do deck ativo (deck_constants.hpp) - invariante 7
// pede "conforme patamar usado no setup", nao um numero pequeno arbitrario.
constexpr std::array<int, 3> kCapacityLevels = {kDeckCapacityTier1, kDeckCapacityTier2,
                                                 kDeckCapacityTier3};

// Estado esperado (SOMBRA) de UMA instancia por toda a vida da sequencia. Nunca
// removido do mapa (nem apos sell/upload) - so muda pra removed=true - e assim que
// o invariante 6 (instance_id nunca reusado) consegue detectar colisao mesmo
// contra um id que ja saiu do sistema ha varios passos.
struct ModelCard {
    std::string card_id;
    CardTier tier = CardTier::Comum;
    bool in_active = true;   // valido SO quando !removed (true=ativo, false=morto)
    bool removed = false;    // sell()/upload() com sucesso - some do sistema pra sempre
    // Copia SOMBRA do estado fisico (ACHADO acima: CardCollection nao expoe
    // accessor mutavel pra instancia viva - diagnose()/attempt_cure() rodam aqui,
    // nao dentro do CardInstance real armazenado no deck).
    CardPhysicalState physical;
    bool burned_ever = false;  // rastreia monotonicidade de is_burned_out (inv.5)
};

// Catalogo-fake por PREFIXO (mesma convencao de deck_invariants_property_test.cpp):
// "esp_*" = Especial, "sup_*" = Super, resto = Comum.
CardTier tier_of_pool(const std::string& card_id) {
    if (card_id.rfind("esp_", 0) == 0) return CardTier::Especial;
    if (card_id.rfind("sup_", 0) == 0) return CardTier::Super;
    return CardTier::Comum;
}

// card_id aleatorio: ~8% Especial, ~6% Super, resto Comum - propositalmente inclui
// as classes protegidas tambem em acquire()/craft() (nenhuma das duas funcoes
// guarda tier NA ENTRADA - so na saida via sell/upload/discard_to_dead - entao
// exercitar Especial/Super aqui ataca o guard defensivo de
// roll_contamination_on_acquisition, secao 5.1).
std::string random_card_id(Lcg& g) {
    const int roll = g.in_range(0, 99);
    if (roll < 8) return "esp_" + std::to_string(g.in_range(0, 2));
    if (roll < 14) return "sup_" + std::to_string(g.in_range(0, 2));
    return "card_" + std::to_string(g.in_range(0, 9));
}

// Candidato de instance_id pra sell/upload/discard_to_dead: ~55% id LEGITIMO do
// ativo, ~30% id do deck MORTO (ataca idempotencia/one-way inv.4/inv.5), ~15% id
// BOGUS nunca emitido (faixa alta, disjunta de qualquer id real - contador comeca
// em 1 e cresce devagar dentro de 300 passos).
std::uint64_t random_candidate_id(Lcg& g, const CardCollection& collection) {
    const int roll = g.in_range(0, 99);
    if (roll < 55 && !collection.active().empty()) {
        return collection
            .active()[static_cast<std::size_t>(g.in_range(0, static_cast<int>(collection.active().size()) - 1))]
            .instance_id;
    }
    if (roll < 85 && !collection.dead().empty()) {
        return collection
            .dead()[static_cast<std::size_t>(g.in_range(0, static_cast<int>(collection.dead().size()) - 1))]
            .instance_id;
    }
    return static_cast<std::uint64_t>(g.in_range(7000000, 8000000));  // nunca emitido
}

bool is_in_active(const CardCollection& collection, std::uint64_t id) {
    for (const auto& c : collection.active())
        if (c.instance_id == id) return true;
    return false;
}

bool is_in_dead(const CardCollection& collection, std::uint64_t id) {
    for (const auto& c : collection.dead())
        if (c.instance_id == id) return true;
    return false;
}

// inv.2b: apos revalidate()/add_card()/etc que garantem a selecao INTEIRA
// revalidada, nenhum id da mao pode estar fora do deck ativo.
void check_hand_only_active(const HandLoadout& hand, const CardCollection& collection) {
    for (const std::uint64_t id : hand.selection()) {
        REQUIRE(is_in_active(collection, id));
    }
}

// Roda os 8 invariantes globais contra o estado REAL (collection/credits) e a
// copia SOMBRA (model, diagnose/attempt_cure). Chamada apos CADA passo.
void check_all_invariants(std::unordered_map<std::uint64_t, ModelCard>& model,
                           const CardCollection& collection, std::int64_t credits,
                           std::int64_t expected_credits) {
    // inv.3: carteira nunca negativa + delta exato (contabilidade sombra).
    REQUIRE(credits >= 0);
    REQUIRE(credits == expected_credits);

    // inv.2 (XOR) + inv.7 (capacidade): sem duplicata entre ativo/morto, cada id
    // aparece no MAXIMO 1x no total.
    std::set<std::uint64_t> seen;
    for (const auto& c : collection.active()) {
        REQUIRE(seen.insert(c.instance_id).second);
    }
    for (const auto& c : collection.dead()) {
        REQUIRE(seen.insert(c.instance_id).second);
    }
    REQUIRE(collection.active_count() <= static_cast<std::size_t>(collection.active_capacity()));

    // Compara o MODELO (mapa id -> estado esperado) contra o sistema real, id a
    // id - o cerne do property test STATEFUL/model-based.
    for (auto& [id, mc] : model) {
        if (mc.removed) {
            // Removido (sell/upload com sucesso) - nunca mais aparece em NENHUM
            // container (nao vira "morto", so some do sistema, deck_transactions.hpp).
            REQUIRE_FALSE(is_in_active(collection, id));
            REQUIRE_FALSE(is_in_dead(collection, id));
            continue;
        }
        if (mc.in_active) {
            REQUIRE(is_in_active(collection, id));
            REQUIRE_FALSE(is_in_dead(collection, id));
        } else {
            REQUIRE(is_in_dead(collection, id));
            REQUIRE_FALSE(is_in_active(collection, id));
        }

        // inv.1: toda instancia viva passa validate() sem lancar (copia sombra).
        REQUIRE_NOTHROW(mc.physical.validate());

        // inv.4: classe protegida jamais infectada (contaminacao pula Especial/
        // Super por guard defensivo, secao 5.1).
        if (mc.tier == CardTier::Especial || mc.tier == CardTier::Super) {
            REQUIRE_FALSE(mc.physical.is_infected);
        }

        // inv.5: is_burned_out monotonico - uma vez sucata, nunca reverte (nem
        // apos CureOutcome::Cured, AMB-T1 - turing_service so limpa
        // infected/virus_kind/diagnosed, nunca is_burned_out).
        if (mc.burned_ever) {
            REQUIRE(mc.physical.is_burned_out);
        }
        mc.burned_ever = mc.burned_ever || mc.physical.is_burned_out;

        // inv.8: is_diagnosed implica is_infected em TODO instante observavel.
        REQUIRE((!mc.physical.is_diagnosed || mc.physical.is_infected));
    }
}

// Aplica 1 operacao aleatoria (op em [0,9]) sobre o estado do caso. Devolve TRUE
// quando (e SO quando) a operacao GARANTE inv.2b - a selecao inteira da mao acabou
// de ser revalidada com sucesso contra o deck ativo (mesmo racional de
// deck_invariants_property_test.cpp::apply_random_step - remove_card so encolhe,
// NAO revalida o resto; ops de container puro podem legitimamente orfanizar a mao
// ate o proximo revalidate).
bool apply_random_step(Lcg& g, int op, CardCollection& collection, HandLoadout& hand,
                        std::int64_t& credits, std::int64_t& expected_credits,
                        std::unordered_map<std::uint64_t, ModelCard>& model,
                        IRandomSource& rng) {
    switch (op) {
        case 0: {  // acquire - inv.6 checado ANTES de inserir no modelo.
            const std::string card_id = random_card_id(g);
            const CardTier tier = tier_of_pool(card_id);
            const int price = g.in_range(0, 30);
            const AcquireResult res = acquire(collection, credits, card_id, price, tier_of_pool, rng);
            if (res.ok()) {
                expected_credits -= price;
                const std::uint64_t id = res.instance.instance_id;
                REQUIRE(model.find(id) == model.end());  // inv.6: id nunca visto antes
                ModelCard mc;
                mc.card_id = card_id;
                mc.tier = tier;
                mc.in_active = true;
                mc.removed = false;
                mc.physical = res.instance.physical;  // semeia a sombra do estado real de criacao
                mc.burned_ever = mc.physical.is_burned_out;
                model.emplace(id, std::move(mc));
            } else {
                REQUIRE((res.error == TransactionError::InsufficientCredits ||
                         res.error == TransactionError::ActiveCapacityFull));
            }
            return false;
        }
        case 1: {  // craft - consumer true/false sorteado; credits NUNCA muda.
            const std::string card_id = random_card_id(g);
            const CardTier tier = tier_of_pool(card_id);
            const bool has_material = g.coin();
            const CraftResult res = craft(
                collection, card_id, [has_material] { return has_material; }, tier_of_pool, rng);
            if (res.ok()) {
                const std::uint64_t id = res.instance.instance_id;
                REQUIRE(model.find(id) == model.end());  // inv.6
                ModelCard mc;
                mc.card_id = card_id;
                mc.tier = tier;
                mc.in_active = true;
                mc.removed = false;
                mc.physical = res.instance.physical;
                mc.burned_ever = mc.physical.is_burned_out;
                model.emplace(id, std::move(mc));
            } else {
                REQUIRE((res.error == TransactionError::ActiveCapacityFull ||
                         res.error == TransactionError::MaterialsUnavailable));
            }
            return false;
        }
        case 2: {  // sell - modelo prediz o desfecho ANTES da chamada real.
            const std::uint64_t id = random_candidate_id(g, collection);
            const auto it = model.find(id);
            const bool model_active = it != model.end() && !it->second.removed && it->second.in_active;
            const bool protected_tier =
                model_active && (it->second.tier == CardTier::Especial || it->second.tier == CardTier::Super);
            const int price = g.in_range(0, 25);
            const SellResult res = sell(collection, credits, id, price, tier_of_pool);
            if (res.ok()) {
                // Divergencia real x modelo seria um ACHADO grave: sucesso so pode
                // acontecer se o modelo tambem esperava id ativo e nao-protegido.
                REQUIRE(model_active);
                REQUIRE_FALSE(protected_tier);
                expected_credits += price;
                it->second.removed = true;
            } else {
                REQUIRE((res.error == TransactionError::InstanceNotInActive ||
                         res.error == TransactionError::ProtectedTier));
                if (res.error == TransactionError::ProtectedTier) {
                    REQUIRE(protected_tier);
                } else {
                    // rejeitado por ausencia - nao podia ser um caso "deveria ter
                    // vendido" (ativo e nao-protegido), senao e regressao real.
                    REQUIRE_FALSE((model_active && !protected_tier));
                }
            }
            return false;
        }
        case 3: {  // upload - mesmo racional de sell, preco de upload (menor faixa).
            const std::uint64_t id = random_candidate_id(g, collection);
            const auto it = model.find(id);
            const bool model_active = it != model.end() && !it->second.removed && it->second.in_active;
            const bool protected_tier =
                model_active && (it->second.tier == CardTier::Especial || it->second.tier == CardTier::Super);
            const int price = g.in_range(0, 8);
            const UploadResult res = upload(collection, credits, id, price, tier_of_pool);
            if (res.ok()) {
                REQUIRE(model_active);
                REQUIRE_FALSE(protected_tier);
                expected_credits += price;
                it->second.removed = true;
            } else {
                REQUIRE((res.error == TransactionError::InstanceNotInActive ||
                         res.error == TransactionError::ProtectedTier));
                if (res.error == TransactionError::ProtectedTier) {
                    REQUIRE(protected_tier);
                } else {
                    REQUIRE_FALSE((model_active && !protected_tier));
                }
            }
            return false;
        }
        case 4: {  // discard_to_dead - lanca (nao devolve Result); try/catch.
            const std::uint64_t id = random_candidate_id(g, collection);
            const auto it = model.find(id);
            const bool model_active = it != model.end() && !it->second.removed && it->second.in_active;
            const bool protected_tier =
                model_active && (it->second.tier == CardTier::Especial || it->second.tier == CardTier::Super);
            try {
                collection.discard_to_dead(id, tier_of_pool);
                REQUIRE(model_active);
                REQUIRE_FALSE(protected_tier);
                it->second.in_active = false;  // agora mora no deck morto
            } catch (const std::invalid_argument&) {
                REQUIRE_FALSE((model_active && !protected_tier));
            }
            return false;
        }
        case 5: {  // diagnose - instancia aleatoria dentre as VIVAS (ativo+morto).
                   // Roda sobre a copia SOMBRA (ver ACHADO no cabecalho do arquivo -
                   // CardCollection nao expoe accessor mutavel pra instancia real).
            std::vector<std::uint64_t> alive_ids;
            alive_ids.reserve(model.size());
            for (const auto& [id, mc] : model)
                if (!mc.removed) alive_ids.push_back(id);
            if (!alive_ids.empty()) {
                const std::uint64_t id =
                    alive_ids[static_cast<std::size_t>(g.in_range(0, static_cast<int>(alive_ids.size()) - 1))];
                ModelCard& mc = model.at(id);
                const DiagnoseOutcome outcome = diagnose(mc.physical);
                if (outcome == DiagnoseOutcome::Diagnosed) {
                    REQUIRE(mc.physical.is_infected);
                    REQUIRE(mc.physical.is_diagnosed);
                } else {
                    REQUIRE_FALSE(mc.physical.is_infected);
                }
            }
            return false;
        }
        case 6: {  // attempt_cure - instancia aleatoria dentre as VIVAS, mesma
                   // ressalva da copia sombra do case 5.
            std::vector<std::uint64_t> alive_ids;
            alive_ids.reserve(model.size());
            for (const auto& [id, mc] : model)
                if (!mc.removed) alive_ids.push_back(id);
            if (!alive_ids.empty()) {
                const std::uint64_t id =
                    alive_ids[static_cast<std::size_t>(g.in_range(0, static_cast<int>(alive_ids.size()) - 1))];
                ModelCard& mc = model.at(id);
                const bool was_burned = mc.physical.is_burned_out;
                const CureOutcome outcome = attempt_cure(mc.physical, mc.tier, rng);
                switch (outcome) {
                    case CureOutcome::Cured:
                        REQUIRE_FALSE(mc.physical.is_infected);
                        REQUIRE(mc.physical.virus_kind == VirusKind::None);
                        REQUIRE_FALSE(mc.physical.is_diagnosed);
                        // AMB-T1: Cured NUNCA reverte is_burned_out (monotonico, inv.5).
                        REQUIRE(mc.physical.is_burned_out == was_burned);
                        break;
                    case CureOutcome::Burned:
                        REQUIRE(mc.physical.is_burned_out);
                        break;
                    case CureOutcome::RejectedNotDiagnosed:
                        REQUIRE_FALSE(mc.physical.is_diagnosed);
                        break;
                    case CureOutcome::RejectedProtectedTier:
                        REQUIRE((mc.tier == CardTier::Especial || mc.tier == CardTier::Super));
                        break;
                }
            }
            return false;
        }
        case 7: {  // hand.add_card - id LEGITIMO (ativo) OU envenenado (morto/bogus).
            const std::uint64_t id = random_candidate_id(g, collection);
            try {
                hand.add_card(id, collection, tier_of_pool);
                return true;  // validate_candidate revalidou a selecao INTEIRA
            } catch (const std::invalid_argument&) {
                return false;
            }
        }
        case 8: {  // hand.remove_card - so ENCOLHE, id sempre valido (vem da propria selecao).
            if (!hand.selection().empty()) {
                const auto id = hand.selection()[static_cast<std::size_t>(
                    g.in_range(0, static_cast<int>(hand.selection().size()) - 1))];
                REQUIRE_NOTHROW(hand.remove_card(id));
            }
            return false;
        }
        default: {  // 9: hand.revalidate - SEMPRE remove todo orfao, por definicao.
            hand.revalidate(collection);
            return true;
        }
    }
}

}  // namespace

TEST_CASE("property stateful: sequencia arbitraria de acquire/craft/sell/upload/"
          "discard/diagnose/attempt_cure/hand mantem os 8 invariantes globais do "
          "sistema deck+hardware, com modelo-sombra comparado a cada passo",
          "[domain][cards_hw][property]") {
    for (int c = 0; c < kSeedsCount; ++c) {
        const unsigned seed = 0xCA2D0000u + static_cast<unsigned>(c);
        Lcg g(seed);
        PropertyRandom rng(seed);  // canal de RNG real do dominio (contaminacao/cura)

        const int capacity = kCapacityLevels[static_cast<std::size_t>(g.in_range(0, 2))];
        CardCollection collection(capacity);
        const bool is_gus = g.coin();
        const int comum_capacity = g.in_range(0, 6);
        HandLoadout hand(comum_capacity, is_gus);
        std::int64_t credits = g.in_range(0, 60);
        std::int64_t expected_credits = credits;
        std::unordered_map<std::uint64_t, ModelCard> model;

        check_all_invariants(model, collection, credits, expected_credits);

        for (int step = 0; step < kStepsPerSeed; ++step) {
            INFO("seed=0x" << std::hex << seed << std::dec << " (case " << c << ") step=" << step);

            const int op = g.in_range(0, 9);
            const bool hand_fully_revalidated =
                apply_random_step(g, op, collection, hand, credits, expected_credits, model, rng);

            check_all_invariants(model, collection, credits, expected_credits);

            // inv.2b: SO afirmavel quando a operacao acabou de garantir a selecao
            // inteira revalidada (add_card com sucesso ou revalidate) - checar fora
            // dessas garantias seria falso-positivo (orfao transitorio legitimo).
            if (hand_fully_revalidated) {
                check_hand_only_active(hand, collection);
            }
        }
    }
}
