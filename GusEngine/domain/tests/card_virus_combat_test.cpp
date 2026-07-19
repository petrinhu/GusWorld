// card_virus_combat_test.cpp
//
// Spec executavel (Catch2 v3) da fatia 1 da onda CARDS-HW-2 (VIRUS EM COMBATE):
// docs/design/mecanicas/cartas-spec-logica.md secao 1/4/5.2. Cobre a PONTE
// instancia->combate (CardIntegrityRef/integrity_ledger, CombatAction::card_instance_id)
// + os 3 payloads desta fatia:
//   - LogicBomb (pre-cast, apos recursos debitados): pool[instance_id%3] determinístico
//     ({HpBelow30pct, IsBossOrMiniBossEncounter, RoundIndexAtLeast(5)}); condicao satisfeita
//     INTERCEPTA o efeito nominal e vira contra o proprio caster (mesmo canal de dano/status
//     de qualquer outra carta); ZERO consumo de RNG (funcao pura).
//   - Worm (pos-cast): SEMPRE Slow idempotente no dono + rola 13% (rng_ injetado) de
//     propagacao em 1 de 3 direcoes (EnemyDeck/OwnDeck/WorldEcosystem); guard de classe
//     protegida (recusa ESPECIAL/SUPER e alvo fora do card_registry, fail-safe).
//   - ZipBomb (pos-cast): MemoryJammed bloqueia UseCard (nao Attack) pelo resto do turno;
//     limpa no refresh do proximo turno proprio (mesmo padrao de overclock_used_).
//
// Ledger nulo OU card_instance_id ausente => comportamento IDENTICO ao motor sem vírus
// (regressao: a suite ANTIGA nao muda 1 linha).
//
// Cartas de teste montadas LOCALMENTE (nunca do registry de producao), mesma convencao de
// techmagic_faraday_test.cpp/techmagic_delay_test.cpp.
//
// Cross-ref: gus/domain/combat/card_integrity_ledger.hpp; gus/domain/infection/
//            integrity_state.hpp; gus/domain/combat/combat_state_machine.cpp
//            (resolve_use_card, dispatch_virus_payload_pre_cast/post_cast).

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "fixed_random.hpp"
#include "gus/domain/combat/card_integrity_ledger.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/domain/combat/random_source.hpp"
#include "gus/domain/infection/integrity_state.hpp"

using namespace gus::domain::combat;
using gus::domain::infection::IntegrityState;
using gus::domain::infection::VirusKind;
using gus::domain::tests::FixedRandom;

namespace {

CombatActor make_actor(const std::string& id, bool player_side, int hp = 100, int atk = 0,
                       int def = 0, int spd = 20, CardFamily family = CardFamily::Eletrico) {
    return CombatActor(id, id, hp, atk, def, spd, family, player_side);
}

const StatusEffect* find_status(const CombatActor& a, StatusId id) {
    for (const auto& s : a.status_effects())
        if (s.id == id) return &s;
    return nullptr;
}

// Carta COMUM de teste: dano puro (sem status), foco no payload de virus (nao no
// resolvedor de efeito em si). mana_cost/ap_cost baixos pra nao brigar com o ramp da FSM.
Card damage_card(const std::string& id, int power = 20, CardTier tier = CardTier::Comum) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Cinetico;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = TargetShape::Single;
    c.tier = tier;
    return c;
}

std::unordered_map<std::string, Card> registry(std::initializer_list<Card> cards) {
    std::unordered_map<std::string, Card> d;
    for (const auto& c : cards) d.emplace(c.id, c);
    return d;
}

// Toca as acoes de `actions`, em ordem, so quando e a vez do lado da party; dali em diante
// passa (0 AP). Mesmo padrao de techmagic_faraday_test.cpp::play_sequence.
CombatActionProvider play_sequence(std::vector<CombatAction> actions) {
    auto acts = std::make_shared<std::vector<CombatAction>>(std::move(actions));
    auto idx = std::make_shared<std::size_t>(0);
    return [acts, idx](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side() || *idx >= acts->size()) return CombatAction::pass();
        return (*acts)[(*idx)++];
    };
}

// Duplo de IRandomSource que CONTA as chamadas (mesmo padrao de techmagic_faraday_test.cpp).
class CountingRandom final : public IRandomSource {
public:
    double next_double() override {
        ++next_double_calls;
        return 0.5;
    }
    int next(int max_value) override {
        ++next_calls;
        return max_value <= 0 ? 0 : std::min(99, max_value - 1);
    }
    int next_calls = 0;
    int next_double_calls = 0;
};

bool log_has(const CombatStateMachine& sm, const std::string& needle) {
    for (const auto& entry : sm.log())
        if (entry.message.find(needle) != std::string::npos) return true;
    return false;
}

}  // namespace

// ===== LogicBomb =====================================================================

TEST_CASE("virus logic-bomb: condicao HpBelow30pct satisfeita INTERCEPTA o efeito nominal "
         "(apos recursos ja debitados) e vira contra o proprio caster, ZERO RNG",
         "[domain][combat][virus][logicbomb]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor target = make_actor("e", false, /*hp=*/300, /*atk=*/0, /*def=*/0, /*spd=*/10);
    caster.take_damage(75);  // hp 25/100 = 25% < 30% -> pool[0]=HpBelow30pct satisfeita.
    REQUIRE(caster.hp() == 25);

    Card bomb = damage_card("virus.logicbomb.hpcond", /*power=*/20);
    auto reg = registry({bomb});

    IntegrityState integrity;
    integrity.is_infected = true;
    integrity.virus_kind = VirusKind::LogicBomb;
    std::vector<CardIntegrityRef> ledger = {
        CardIntegrityRef{/*instance_id=*/0, bomb.id, /*owner_actor_id=*/1, &integrity}};

    CombatAction cast = CombatAction::use_card(bomb.id, target.id());
    cast.card_instance_id = 0;  // 0 % 3 == 0 -> HpBelow30pct.

    CountingRandom rng;
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng, &ledger);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(target.hp() == target.max_hp());  // alvo NOMINAL intocado - efeito desviado.
    REQUIRE(caster.hp() == 25 - bomb.power);   // backfire no proprio caster.
    REQUIRE(rng.next_calls == 0);              // ZERO sorteio de canal (funcao pura).
    REQUIRE(rng.next_double_calls == 0);
    REQUIRE(log_has(sm, "logic-bomb"));
    REQUIRE_NOTHROW(integrity.validate());
}

TEST_CASE("virus logic-bomb: condicao NAO satisfeita -> passthrough, efeito nominal roda "
         "normalmente contra o alvo declarado",
         "[domain][combat][virus][logicbomb]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor target = make_actor("e", false, /*hp=*/300, /*atk=*/0, /*def=*/0, /*spd=*/10);
    // caster com HP CHEIO (pool[0]=HpBelow30pct NAO satisfeita), sem boss no encontro, rodada 0.

    Card bomb = damage_card("virus.logicbomb.passthrough", /*power=*/20);
    auto reg = registry({bomb});

    IntegrityState integrity;
    integrity.is_infected = true;
    integrity.virus_kind = VirusKind::LogicBomb;
    std::vector<CardIntegrityRef> ledger = {
        CardIntegrityRef{/*instance_id=*/0, bomb.id, /*owner_actor_id=*/1, &integrity}};

    CombatAction cast = CombatAction::use_card(bomb.id, target.id());
    cast.card_instance_id = 0;

    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/99);  // canal COMUM, sem crit/fumble.
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng, &ledger);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(target.hp() < target.max_hp());     // efeito nominal aplicou no alvo declarado.
    REQUIRE(caster.hp() == caster.max_hp());    // caster intocado (sem backfire).
    REQUIRE_FALSE(log_has(sm, "logic-bomb"));
}

TEST_CASE("virus logic-bomb: recursos (mana) sao debitados ANTES da interceptacao - a carta "
         "'cobra' normalmente mesmo quando o efeito nominal e substituido pelo backfire "
         "(secao 1 da spec: ordem pre-cast e SEMPRE debito->intercepcao, nunca o inverso)",
         "[domain][combat][virus][logicbomb][regression]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor target = make_actor("e", false, /*hp=*/300, /*atk=*/0, /*def=*/0, /*spd=*/10);
    caster.take_damage(75);  // hp 25/100 = 25% < 30% -> pool[0]=HpBelow30pct satisfeita.
    caster.refresh_resources_for_turn(/*round_index=*/0);  // mana_max=2, mana=2 (kBaseMana).
    REQUIRE(caster.mana() == 2);

    Card bomb = damage_card("virus.logicbomb.manadebit", /*power=*/10);
    bomb.mana_cost = 2;  // == mana disponivel: se NAO debitar, sobra 2; se debitar, sobra 0.
    auto reg = registry({bomb});

    IntegrityState integrity;
    integrity.is_infected = true;
    integrity.virus_kind = VirusKind::LogicBomb;
    std::vector<CardIntegrityRef> ledger = {
        CardIntegrityRef{/*instance_id=*/0, bomb.id, /*owner_actor_id=*/1, &integrity}};

    CombatAction cast = CombatAction::use_card(bomb.id, target.id());
    cast.card_instance_id = 0;  // 0 % 3 == 0 -> HpBelow30pct (ja satisfeita acima).

    CountingRandom rng;
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng, &ledger);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(target.hp() == target.max_hp());  // backfire desviou o efeito nominal, como sempre.
    REQUIRE(caster.mana() == 0);              // mana FOI debitada apesar do backfire.
    REQUIRE(log_has(sm, "logic-bomb"));
}

// ===== Worm: sempre-Slow idempotente ==================================================

TEST_CASE("virus worm: SEMPRE aplica Slow idempotente no dono a cada cast (sem stack), "
         "loga em cada disparo",
         "[domain][combat][virus][worm]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor filler = make_actor("e", false, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    Card worm_card = damage_card("virus.worm.slow", /*power=*/0);
    auto reg = registry({worm_card});

    IntegrityState integrity;
    integrity.is_infected = true;
    integrity.virus_kind = VirusKind::Worm;
    std::vector<CardIntegrityRef> ledger = {
        CardIntegrityRef{/*instance_id=*/50, worm_card.id, /*owner_actor_id=*/1, &integrity}};

    CombatAction cast = CombatAction::use_card(worm_card.id, filler.id());
    cast.card_instance_id = 50;

    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/50);  // chance 50 >= 13% -> sem propagacao.
    auto provider = play_sequence({cast, cast});  // 2 casts (1 por turno proprio do caster).
    CombatStateMachine sm({&caster, &filler}, provider, &reg, nullptr, &rng, &ledger);

    // Rodada 0: caster (cast #1), filler (pass).
    sm.begin_turn();
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();
    sm.begin_turn();
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();

    const StatusEffect* slow_after_1 = find_status(caster, StatusId::Slow);
    REQUIRE(slow_after_1 != nullptr);
    const int magnitude_1 = slow_after_1->magnitude;
    const int duration_1 = slow_after_1->duration;

    // Rodada 1: caster (cast #2), filler (pass) - idempotente (Refresh, sem stack).
    sm.begin_turn();
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();
    sm.begin_turn();
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();

    const StatusEffect* slow_after_2 = find_status(caster, StatusId::Slow);
    REQUIRE(slow_after_2 != nullptr);
    REQUIRE(slow_after_2->magnitude == magnitude_1);  // idempotente: mesma magnitude.
    REQUIRE(caster.status_effects().size() ==
           caster.status_effects().size());  // sanity (sem duplicata: 1 unico Slow).
    int slow_count = 0;
    for (const auto& s : caster.status_effects())
        if (s.id == StatusId::Slow) ++slow_count;
    REQUIRE(slow_count == 1);
    (void)duration_1;

    // ACHADO QA (mutation testing, CARDS-HW-2 fatia 1 review): log_has(sm, "worm") SOZINHO
    // e um assert FRACO aqui - o card.id de teste ("virus.worm.slow") contem a substring
    // "worm", e o log GENERICO incondicional "<ator> compila <card.id> em <alvo> por 0."
    // (resolve_use_card, canal de dano comum) TAMBEM bate nesse grep mesmo se a mensagem
    // ESPECIFICA do Slow ("arrasta a execucao - desempenho degradado (worm ativo)") for
    // removida - mutante sobreviveu com a mensagem especifica deletada. Assert PRECISO na
    // frase que so essa mensagem contem, fechando o furo de cobertura de log.
    REQUIRE(log_has(sm, "desempenho degradado"));
    REQUIRE_NOTHROW(integrity.validate());
}

// ===== Worm: propagacao 13% em 3 direcoes ==============================================

TEST_CASE("virus worm: propagacao (roll<13%) direcao EnemyDeck infecta a instancia de "
         "owner_actor_id DIFERENTE, guard de classe passa (COMUM no registry)",
         "[domain][combat][virus][worm][propagation]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor filler = make_actor("e", false, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    Card worm_card = damage_card("virus.worm.enemydeck", /*power=*/0);
    Card enemy_card = damage_card("virus.worm.enemydeck.candidate", /*power=*/0);
    auto reg = registry({worm_card, enemy_card});

    IntegrityState source_state;
    source_state.is_infected = true;
    source_state.virus_kind = VirusKind::Worm;
    IntegrityState candidate_state;  // ainda limpa.

    std::vector<CardIntegrityRef> ledger = {
        CardIntegrityRef{/*instance_id=*/1, worm_card.id, /*owner_actor_id=*/1, &source_state},
        CardIntegrityRef{/*instance_id=*/2, enemy_card.id, /*owner_actor_id=*/2,
                         &candidate_state}};

    CombatAction cast = CombatAction::use_card(worm_card.id, filler.id());
    cast.card_instance_id = 1;

    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/0);  // chance 0<13%; direcao 0=EnemyDeck.
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &filler}, provider, &reg, nullptr, &rng, &ledger);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(candidate_state.is_infected);
    REQUIRE(candidate_state.virus_kind == VirusKind::Worm);
    REQUIRE_NOTHROW(candidate_state.validate());
    REQUIRE(sm.world_events().empty());
    REQUIRE(log_has(sm, "inimigo"));
}

TEST_CASE("virus worm: propagacao (roll<13%) direcao OwnDeck infecta a instancia de "
         "owner_actor_id IGUAL, guard de classe passa",
         "[domain][combat][virus][worm][propagation]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor filler = make_actor("e", false, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    Card worm_card = damage_card("virus.worm.owndeck", /*power=*/0);
    Card own_card = damage_card("virus.worm.owndeck.candidate", /*power=*/0);
    auto reg = registry({worm_card, own_card});

    IntegrityState source_state;
    source_state.is_infected = true;
    source_state.virus_kind = VirusKind::Worm;
    IntegrityState candidate_state;

    std::vector<CardIntegrityRef> ledger = {
        CardIntegrityRef{/*instance_id=*/1, worm_card.id, /*owner_actor_id=*/1, &source_state},
        CardIntegrityRef{/*instance_id=*/2, own_card.id, /*owner_actor_id=*/1,
                         &candidate_state}};

    CombatAction cast = CombatAction::use_card(worm_card.id, filler.id());
    cast.card_instance_id = 1;

    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/1);  // chance 1<13%; direcao 1=OwnDeck.
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &filler}, provider, &reg, nullptr, &rng, &ledger);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(candidate_state.is_infected);
    REQUIRE(candidate_state.virus_kind == VirusKind::Worm);
    REQUIRE_NOTHROW(candidate_state.validate());
    REQUIRE(sm.world_events().empty());
    REQUIRE(log_has(sm, "proprio deck"));
}

TEST_CASE("virus worm: propagacao (roll<13%) direcao WorldEcosystem so registra outbox + "
         "loga (fora do escopo de combate)",
         "[domain][combat][virus][worm][propagation]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor filler = make_actor("e", false, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    Card worm_card = damage_card("virus.worm.world", /*power=*/0);
    auto reg = registry({worm_card});

    IntegrityState source_state;
    source_state.is_infected = true;
    source_state.virus_kind = VirusKind::Worm;
    std::vector<CardIntegrityRef> ledger = {
        CardIntegrityRef{/*instance_id=*/7, worm_card.id, /*owner_actor_id=*/1, &source_state}};

    CombatAction cast = CombatAction::use_card(worm_card.id, filler.id());
    cast.card_instance_id = 7;

    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/2);  // chance 2<13%; direcao 2=WorldEcosystem.
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &filler}, provider, &reg, nullptr, &rng, &ledger);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(sm.world_events().size() == 1);
    REQUIRE(sm.world_events().front().source_instance_id == 7);
    REQUIRE(log_has(sm, "rede"));
}

TEST_CASE("virus worm: guard de classe protegida RECUSA alvo ESPECIAL/SUPER (com log), "
         "NAO infecta",
         "[domain][combat][virus][worm][guard]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor filler = make_actor("e", false, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    Card worm_card = damage_card("virus.worm.guard.special", /*power=*/0);
    Card special_card =
        damage_card("virus.worm.guard.special.candidate", /*power=*/0, CardTier::Especial);
    auto reg = registry({worm_card, special_card});

    IntegrityState source_state;
    source_state.is_infected = true;
    source_state.virus_kind = VirusKind::Worm;
    IntegrityState candidate_state;

    std::vector<CardIntegrityRef> ledger = {
        CardIntegrityRef{/*instance_id=*/1, worm_card.id, /*owner_actor_id=*/1, &source_state},
        CardIntegrityRef{/*instance_id=*/2, special_card.id, /*owner_actor_id=*/2,
                         &candidate_state}};

    CombatAction cast = CombatAction::use_card(worm_card.id, filler.id());
    cast.card_instance_id = 1;

    FixedRandom rng(0.5, 0);  // forca EnemyDeck.
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &filler}, provider, &reg, nullptr, &rng, &ledger);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE_FALSE(candidate_state.is_infected);
    REQUIRE(log_has(sm, "ESPECIAL/SUPER"));
}

TEST_CASE("virus worm: fail-safe RECUSA alvo fora do card_registry do encontro (com log), "
         "NAO infecta, NAO lanca",
         "[domain][combat][virus][worm][guard]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor filler = make_actor("e", false, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    Card worm_card = damage_card("virus.worm.guard.ghost", /*power=*/0);
    auto reg = registry({worm_card});  // NAO inclui a carta-fantasma abaixo.

    IntegrityState source_state;
    source_state.is_infected = true;
    source_state.virus_kind = VirusKind::Worm;
    IntegrityState candidate_state;

    std::vector<CardIntegrityRef> ledger = {
        CardIntegrityRef{/*instance_id=*/1, worm_card.id, /*owner_actor_id=*/1, &source_state},
        CardIntegrityRef{/*instance_id=*/2, "carta-fantasma-fora-do-registry",
                         /*owner_actor_id=*/2, &candidate_state}};

    CombatAction cast = CombatAction::use_card(worm_card.id, filler.id());
    cast.card_instance_id = 1;

    FixedRandom rng(0.5, 0);  // forca EnemyDeck.
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &filler}, provider, &reg, nullptr, &rng, &ledger);

    REQUIRE_NOTHROW(sm.begin_turn());
    REQUIRE_NOTHROW(sm.run_active_turn_to_end());

    REQUIRE_FALSE(candidate_state.is_infected);
    REQUIRE(log_has(sm, "fail-safe"));
}

// ===== ZipBomb: MemoryJammed ===========================================================

TEST_CASE("virus zip-bomb: entope a memoria (MemoryJammed) - bloqueia UseCard subsequente "
         "no MESMO turno, mas NAO bloqueia ataque basico; limpa no proximo turno proprio",
         "[domain][combat][virus][zipbomb]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/50, /*def=*/0, /*spd=*/50);
    CombatActor target = make_actor("e", false, /*hp=*/300, /*atk=*/0, /*def=*/0, /*spd=*/10);

    Card zip_card = damage_card("virus.zipbomb.jam", /*power=*/0);
    Card other_card = damage_card("virus.zipbomb.other", /*power=*/10);
    auto reg = registry({zip_card, other_card});

    IntegrityState integrity;
    integrity.is_infected = true;
    integrity.virus_kind = VirusKind::ZipBomb;
    std::vector<CardIntegrityRef> ledger = {
        CardIntegrityRef{/*instance_id=*/9, zip_card.id, /*owner_actor_id=*/1, &integrity}};

    CombatAction cast_zip = CombatAction::use_card(zip_card.id, target.id());
    cast_zip.card_instance_id = 9;
    CombatAction attack = CombatAction::attack(target.id());
    CombatAction cast_other = CombatAction::use_card(other_card.id, target.id());

    FixedRandom rng(0.5, 99);
    auto provider = play_sequence({cast_zip, attack, cast_other});
    CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng, &ledger);

    sm.begin_turn();

    bool threw = false;
    try {
        sm.run_active_turn_to_end();
    } catch (const std::logic_error& ex) {
        threw = true;
        REQUIRE(std::string(ex.what()).find("ERRO DE COMPILACAO") != std::string::npos);
    }

    REQUIRE(threw);                        // 3a acao (UseCard) foi bloqueada.
    REQUIRE(caster.memory_jammed());
    REQUIRE(target.hp() < target.max_hp()); // 2a acao (Attack) rodou normalmente antes.
    REQUIRE(log_has(sm, "memoria"));

    // Limpa no proximo turno PROPRIO do caster (mesmo padrao de overclock_used_).
    sm.advance_to_next_actor();  // fecha o turno interrompido, vai pro alvo.
    sm.begin_turn();
    sm.run_active_turn_to_end();  // alvo passa (provider so tem acoes pro player side).
    sm.advance_to_next_actor();
    sm.begin_turn();              // proximo turno proprio do caster: refresh limpa a trava.
    REQUIRE_FALSE(caster.memory_jammed());
}

// ===== Regressao: ledger nulo/instance ausente = comportamento IDENTICO ================

TEST_CASE("virus: ledger nulo (default, todo call site pre-existente) preserva o "
         "comportamento IDENTICO ao motor sem vírus - nenhum log de vírus aparece",
         "[domain][combat][virus][regression]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor target = make_actor("e", false, /*hp=*/300, /*atk=*/0, /*def=*/0, /*spd=*/10);

    Card plain = damage_card("virus.regression.plain", /*power=*/20);
    auto reg = registry({plain});

    CombatAction cast = CombatAction::use_card(plain.id, target.id());
    cast.card_instance_id = 123;  // presente na ACAO, mas SEM ledger (default nullptr).

    FixedRandom rng(0.5, 99);
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng);  // sem ledger.

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(target.hp() < target.max_hp());
    REQUIRE_FALSE(log_has(sm, "logic-bomb"));
    REQUIRE_FALSE(log_has(sm, "worm"));
    REQUIRE_FALSE(log_has(sm, "memoria sobrecarregada"));
}
