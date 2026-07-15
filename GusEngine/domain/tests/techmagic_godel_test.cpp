// techmagic_godel_test.cpp
//
// Spec executavel (Catch2 v3) do Null-Proof (Godel), PR3 do "Balde B" (ADR-016, decisoes
// do lider 2026-07-15, G-1..G-3):
//   G-1: Godel vira castavel mana-0 (Ativa) - OnCast -> ApplyStatus NullProof no PORTADOR
//        (side_filter AllyOnly, inclui self). 1x/batalha via specials_cast_ (mesmo gate de
//        Volta/Newton/Faraday).
//   G-2: o fura-defesa SO queima quando ha algo a furar - no calculo de dano
//        (resolve_use_card), se o ATACANTE tem NullProof E o mult_fraqueza calculado e <
//        1.0, o resolvedor forca mult 1.0 e CONSOME (remove) o status. Contra Neutro/Fraco
//        (mult >= 1.0) o status fica intacto - NAO e um consume-sempre.
//   G-3: fura os DOIS tiers < 1.0 (Resistente 0.66 E Imune 0.0) - o codigo checa soh
//        `mult_fraqueza < 1.0f`, sem distinguir o tier.
// Item 11 GENERICO (wiring real do trunfo original de Godel): qualquer carta com
// ignores_weakness_wheel=true sempre resolve mult 1.0, via SEMPRE, independente de NullProof
// de quem quer que seja (via INDEPENDENTE do pierce por-status).
//
// LIMITE HONESTO DE COBERTURA (mesmo padrao ja registrado em combat_formula_test.cpp e em
// techmagic_faraday_test.cpp/teste 6b - "NOTA DE COBERTURA"): o tier WeaknessTier::Imune
// (mult 0.0) NAO E ALCANCAVEL hoje via nenhuma combinacao real de CardFamily (WeaknessWheel::
// tier_for so devolve Fraco/Resistente/Neutro para os 5 pares nao-Universal - confirmado por
// inspecao de weakness_wheel.hpp e pela nota de combat_formula_test.cpp: "o tier Imune NAO e
// exposto pela API publica hoje... e flag de inimigo/lore, incremento futuro"). Os testes que
// o brief pede "vs IMUNE" sao portanto substituidos aqui por "vs RESISTENTE" (0.66, a UNICA
// combinacao real com mult < 1.0 hoje) - o codigo do pierce (`mult_fraqueza < 1.0f`, SEM
// distinguir o tier) trata os dois IDENTICAMENTE, entao a cobertura via Resistente exercita a
// MESMA branch que cobriria Imune no dia em que a flag de imunidade for plugada (nao ha
// logica dedicada a 0.0 que ficasse sem teste). SEM ISSO nao ha soft-skip silencioso: este
// comentario e a marca honesta do gap, mesma convencao ja usada no codebase.
//
// Cartas de teste montadas LOCALMENTE (id "techmagic.godel.*"/"techmagic.dmg.*"), NUNCA do
// registry de producao (MasterCards) - o teste do CATALOGO (Godel em MasterCards::
// build_registry()) vive em master_cards_test.cpp.
//
// Cross-ref: gus/domain/combat/combat_enums.hpp (StatusId::NullProof); combat_state_machine.cpp
//            (resolve_use_card - wiring do pierce; estimate_card_damage - gemeo PURO);
//            master_cards.cpp (godel); docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md
//            (AMB-05); docs/design/mecanicas/cartas-technomagik.md secao 6/9; ADR-016.

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <unordered_map>
#include <vector>

#include "fixed_random.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/domain/combat/random_source.hpp"

using namespace gus::domain::combat;
using gus::domain::tests::FixedRandom;

namespace {

CombatActor make_actor(const std::string& id, bool player_side, int hp = 100, int atk = 8,
                       int def = 0, int spd = 20,
                       CardFamily family = CardFamily::Eletrico) {
    return CombatActor(id, id, hp, atk, def, spd, family, player_side);
}

const StatusEffect* find_status(const CombatActor& a, StatusId id) {
    for (const auto& s : a.status_effects())
        if (s.id == id) return &s;
    return nullptr;
}

// Carta de dano PLANA (tier Comum default - isenta do gate 1x/batalha), family
// configuravel, alvo Single. Reusada nos cenarios de pierce (Resistente/Neutro/Fraco).
Card damage_card(const std::string& id, CardFamily family, int power) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = family;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = TargetShape::Single;
    return c;
}

// Duplo LOCAL da Godel de producao: mesmo shape (Ativa/Universal/mana 0, OnCast ->
// ApplyStatus NullProof AllyOnly, ignores_weakness_wheel=true), mesma convencao "cartas de
// teste nunca do registry de producao" (ver techmagic_faraday_test.cpp::faraday_card).
Card godel_card(const std::string& id) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Universal;
    c.base_type = CardBaseType::Glifo;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = 0;
    c.target_shape = TargetShape::Single;
    c.tier = CardTier::Especial;
    c.category = CardCategory::Ativa;
    c.effects = {EffectSpec{.trigger = TriggerHook::OnCast,
                            .kind = EffectKind::ApplyStatus,
                            .duration = 99,
                            .status = StatusId::NullProof,
                            .stack_rule = StackRule::Refresh,
                            .side_filter = SideFilter::AllyOnly}};
    c.ignores_weakness_wheel = true;
    return c;
}

// Carta Tesla-like LOCAL (OnCast -> ChainDamage), pro teste do domino (#8): dano-base do
// primario (power) escala do card.power + atk, mesma formula da secao 11; magnitude=1 salto,
// percent=50% de retencao.
Card chain_card(const std::string& id, CardFamily family, int power) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = family;
    c.base_type = CardBaseType::Glifo;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = TargetShape::Single;
    c.tier = CardTier::Especial;
    c.category = CardCategory::Ativa;
    c.effects = {EffectSpec{.trigger = TriggerHook::OnCast,
                            .kind = EffectKind::ChainDamage,
                            .magnitude = 1,
                            .percent = 50}};
    return c;
}

std::unordered_map<std::string, Card> registry(std::initializer_list<Card> cards) {
    std::unordered_map<std::string, Card> d;
    for (const auto& c : cards) d.emplace(c.id, c);
    return d;
}

// Toca as acoes de `actions`, MAPEADAS por id de ator (uma so vez cada); fora disso passa
// (0 AP). Permite orquestrar cenarios com >1 ator player-side agindo em ordens diferentes
// (teste #9), diferente de play_sequence (que so serve 1 ator generico "a vez da party").
CombatActionProvider play_by_actor(std::unordered_map<std::string, CombatAction> actions) {
    auto acts = std::make_shared<std::unordered_map<std::string, CombatAction>>(std::move(actions));
    return [acts](CombatActor& a, const CombatState&) -> CombatAction {
        auto it = acts->find(a.id());
        if (it == acts->end()) return CombatAction::pass();
        CombatAction action = it->second;
        acts->erase(it);  // 1x por ator (senao repassaria a mesma acao pra sempre).
        return action;
    };
}

// Duplo de IRandomSource que CONTA as chamadas (mesmo padrao de techmagic_faraday_test.cpp/
// techmagic_delay_test.cpp), pro teste de determinismo/RNG (#4).
class CountingRandom final : public IRandomSource {
public:
    explicit CountingRandom(double next_double = 0.5, int next_int = 99)
        : next_double_(next_double), next_int_(next_int) {}
    double next_double() override {
        ++double_calls;
        return next_double_;
    }
    int next(int max_value) override {
        ++int_calls;
        return max_value <= 0 ? 0 : std::min(next_int_, max_value - 1);
    }
    int int_calls = 0;
    int double_calls = 0;

private:
    double next_double_;
    int next_int_;
};

}  // namespace

// ===== 1/2. Atacante com NullProof vs RESISTENTE (0.66, unica combinacao real < 1.0 hoje - =====
// =====       ver LIMITE HONESTO DE COBERTURA no topo): dano>0, mult forcado 1.0 (delta ======
// =====       exato vs controle SEM NullProof), status CONSUMIDO ============================

TEST_CASE("techmagic godel: atacante com NullProof vs alvo RESISTENTE fura a defesa (mult "
         "-> 1.0, delta exato vs controle sem NullProof) e CONSOME o status",
         "[domain][combat][techmagic][godel]") {
    // card.family=Eletrico, target.family=Bioquimico => Resistente (strong_against(Bioquimico)
    // == Eletrico, weakness_wheel.hpp), mult=kMultResistente=0.66.
    Card card = damage_card("techmagic.godel.dmg_resistente", CardFamily::Eletrico, /*power=*/10);
    auto reg = registry({card});
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/99);  // canal COMUM, variancia zero.

    // Controle: SEM NullProof - mult fica 0.66. base=(10+8)*0.66=11.88 -> round=12.
    {
        CombatActor h = make_actor("h", true, /*hp=*/100, /*atk=*/8, /*def=*/0, /*spd=*/50);
        CombatActor e = make_actor("e", false, /*hp=*/300, /*atk=*/0, /*def=*/0, /*spd=*/10,
                                   CardFamily::Bioquimico);
        auto provider = play_by_actor({{"h", CombatAction::use_card(card.id, e.id())}});
        CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        REQUIRE(e.max_hp() - e.hp() == 12);
    }

    // Com NullProof: mult pierced -> 1.0. base=(10+8)*1.0=18 -> round=18. Status CONSUMIDO.
    {
        CombatActor h = make_actor("h", true, /*hp=*/100, /*atk=*/8, /*def=*/0, /*spd=*/50);
        CombatActor e = make_actor("e", false, /*hp=*/300, /*atk=*/0, /*def=*/0, /*spd=*/10,
                                   CardFamily::Bioquimico);
        h.add_status(
            StatusEffect{StatusId::NullProof, 0, 99, StackRule::Refresh, CardFamily::Universal});
        auto provider = play_by_actor({{"h", CombatAction::use_card(card.id, e.id())}});
        CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();

        REQUIRE(e.max_hp() - e.hp() == 18);           // dano pierced (mult 1.0, nao 0.66).
        REQUIRE(find_status(h, StatusId::NullProof) == nullptr);  // consumido.
        bool pierce_logged = false;
        for (const auto& entry : sm.log())
            if (entry.message.find("Null-Proof") != std::string::npos) pierce_logged = true;
        REQUIRE(pierce_logged);
    }
}

// ===== 3. vs NEUTRO/FRACO (mult >= 1.0): dano inalterado, status NAO consumido (mata =====
// =====    mutante "consume-sempre") ==========================================================

TEST_CASE("techmagic godel: atacante com NullProof vs alvo NEUTRO ou FRACO nao fura (mult ja "
         ">= 1.0) e NAO consome o status (nao e consume-sempre)",
         "[domain][combat][techmagic][godel]") {
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/99);

    // Neutro: card.family=Eletrico, target.family=Sonico. mult=1.0. base=(10+8)*1.0=18.
    {
        Card card = damage_card("techmagic.godel.dmg_neutro", CardFamily::Eletrico, /*power=*/10);
        auto reg = registry({card});
        CombatActor h = make_actor("h", true, /*hp=*/100, /*atk=*/8, /*def=*/0, /*spd=*/50);
        CombatActor e = make_actor("e", false, /*hp=*/300, /*atk=*/0, /*def=*/0, /*spd=*/10,
                                   CardFamily::Sonico);
        h.add_status(
            StatusEffect{StatusId::NullProof, 0, 99, StackRule::Refresh, CardFamily::Universal});
        auto provider = play_by_actor({{"h", CombatAction::use_card(card.id, e.id())}});
        CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();

        REQUIRE(e.max_hp() - e.hp() == 18);
        REQUIRE(find_status(h, StatusId::NullProof) != nullptr);  // intacto.
    }

    // Fraco: card.family=Eletrico, target.family=Cinetico. mult=1.5. base=(10+8)*1.5=27.
    {
        Card card = damage_card("techmagic.godel.dmg_fraco", CardFamily::Eletrico, /*power=*/10);
        auto reg = registry({card});
        CombatActor h = make_actor("h", true, /*hp=*/100, /*atk=*/8, /*def=*/0, /*spd=*/50);
        CombatActor e = make_actor("e", false, /*hp=*/300, /*atk=*/0, /*def=*/0, /*spd=*/10,
                                   CardFamily::Cinetico);
        h.add_status(
            StatusEffect{StatusId::NullProof, 0, 99, StackRule::Refresh, CardFamily::Universal});
        auto provider = play_by_actor({{"h", CombatAction::use_card(card.id, e.id())}});
        CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();

        REQUIRE(e.max_hp() - e.hp() == 27);
        REQUIRE(find_status(h, StatusId::NullProof) != nullptr);  // intacto.
    }
}

// ===== 4. Determinismo/RNG: o pierce e RNG-NEUTRO - MESMA contagem/ordem de consumo com e =====
// =====    sem NullProof contra o MESMO alvo Resistente (protege a ordem canonica da secao ====
// =====    11; ver LIMITE HONESTO DE COBERTURA no topo pro caso Imune=0-consumos) ============

TEST_CASE("techmagic godel: o pierce nao consome RNG - mesma contagem/ordem de rng.next(100)/"
         "next_double() com e sem NullProof contra o mesmo alvo Resistente",
         "[domain][combat][techmagic][godel]") {
    auto run = [](bool with_null_proof) {
        Card card = damage_card("techmagic.godel.dmg_rng", CardFamily::Eletrico, /*power=*/10);
        auto reg = registry({card});
        CombatActor h = make_actor("h", true, /*hp=*/100, /*atk=*/8, /*def=*/0, /*spd=*/50);
        CombatActor e = make_actor("e", false, /*hp=*/300, /*atk=*/0, /*def=*/0, /*spd=*/10,
                                   CardFamily::Bioquimico);  // Resistente ao card Eletrico.
        if (with_null_proof)
            h.add_status(StatusEffect{StatusId::NullProof, 0, 99, StackRule::Refresh,
                                      CardFamily::Universal});
        CountingRandom rng(/*next_double=*/0.5, /*next_int=*/99);  // canal COMUM.
        auto provider = play_by_actor({{"h", CombatAction::use_card(card.id, e.id())}});
        CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        return std::pair<int, int>{rng.int_calls, rng.double_calls};
    };

    const auto without_pierce = run(/*with_null_proof=*/false);
    const auto with_pierce = run(/*with_null_proof=*/true);

    // Canal COMUM organico (secao 11): 1 consumo next(100) + 1 consumo next_double(), IDENTICO
    // com ou sem pierce - o pierce em si (mult -> 1.0 + remove_status) NAO toca rng_.
    REQUIRE(without_pierce.first == 1);
    REQUIRE(without_pierce.second == 1);
    REQUIRE(with_pierce == without_pierce);
}

// ===== 5. Ataque BASICO com NullProof: nao consulta a roda, status intacto ==================

TEST_CASE("techmagic godel: ataque BASICO nunca consulta a roda de fraqueza - NullProof do "
         "atacante fica intacto",
         "[domain][combat][techmagic][godel]") {
    CombatActor h = make_actor("h", true, /*hp=*/100, /*atk=*/8, /*def=*/0, /*spd=*/50);
    CombatActor e = make_actor("e", false, /*hp=*/300, /*atk=*/0, /*def=*/0, /*spd=*/10,
                               CardFamily::Bioquimico);
    h.add_status(
        StatusEffect{StatusId::NullProof, 0, 99, StackRule::Refresh, CardFamily::Universal});

    auto provider = play_by_actor({{"h", CombatAction::attack(e.id())}});
    CombatStateMachine sm({&h, &e}, provider, nullptr, nullptr, nullptr);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(e.max_hp() - e.hp() == 8);  // subtrativa clamp(atk-def,1) = 8-0.
    REQUIRE(find_status(h, StatusId::NullProof) != nullptr);
}

// ===== 6. Carta com ignores_weakness_wheel=true jogada direto: mult 1.0 SEMPRE (via ========
// =====    INDEPENDENTE do pierce por-status), SEM consumir NullProof de ninguem ============

TEST_CASE("techmagic godel: item 11 GENERICO - carta com ignores_weakness_wheel=true resolve "
         "mult 1.0 via propria flag, sem tocar no NullProof do atacante (vias independentes)",
         "[domain][combat][techmagic][godel]") {
    Card ignore_wheel_card = damage_card("techmagic.godel.ignore_wheel", CardFamily::Eletrico,
                                         /*power=*/10);
    ignore_wheel_card.ignores_weakness_wheel = true;
    auto reg = registry({ignore_wheel_card});
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/99);

    CombatActor h = make_actor("h", true, /*hp=*/100, /*atk=*/8, /*def=*/0, /*spd=*/50);
    // Alvo Resistente (Bioquimico vs Eletrico, mult base 0.66) - se a flag NAO furasse, o
    // dano seria 12, nao 18.
    CombatActor e = make_actor("e", false, /*hp=*/300, /*atk=*/0, /*def=*/0, /*spd=*/10,
                               CardFamily::Bioquimico);
    h.add_status(
        StatusEffect{StatusId::NullProof, 0, 99, StackRule::Refresh, CardFamily::Universal});

    auto provider = play_by_actor({{"h", CombatAction::use_card(ignore_wheel_card.id, e.id())}});
    CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(e.max_hp() - e.hp() == 18);  // mult 1.0 via flag (nao 12, que seria 0.66).
    REQUIRE(find_status(h, StatusId::NullProof) != nullptr);  // NAO consumido (via independente).
}

// ===== 6b. Mesma carta ignores_weakness_wheel=true, mas o atacante NUNCA teve NullProof =====
// =====     (isolamento real da via item-11 - mata o mutante "flag passa a exigir ===========
// =====     NullProof"; o teste #6 acima nao pega esse mutante porque da NullProof ao =======
// =====     mesmo atacante que joga a carta com a flag, entao as duas vias colapsam) ========

TEST_CASE("techmagic godel: item 11 GENERICO isolado - ignores_weakness_wheel=true fura mesmo "
         "quando o atacante JAMAIS teve NullProof (via nao depende do status)",
         "[domain][combat][techmagic][godel]") {
    Card ignore_wheel_card = damage_card("techmagic.godel.ignore_wheel_isolado",
                                         CardFamily::Eletrico, /*power=*/10);
    ignore_wheel_card.ignores_weakness_wheel = true;
    auto reg = registry({ignore_wheel_card});
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/99);

    // h NUNCA recebe NullProof - se a via da flag dependesse do status (mutante), o mult
    // ficaria 0.66 (dano 12) em vez de 1.0 (dano 18) pierced pela flag sozinha.
    CombatActor h = make_actor("h", true, /*hp=*/100, /*atk=*/8, /*def=*/0, /*spd=*/50);
    CombatActor e = make_actor("e", false, /*hp=*/300, /*atk=*/0, /*def=*/0, /*spd=*/10,
                               CardFamily::Bioquimico);  // Resistente ao Eletrico, mult base 0.66.

    auto provider = play_by_actor({{"h", CombatAction::use_card(ignore_wheel_card.id, e.id())}});
    CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(e.max_hp() - e.hp() == 18);  // mult 1.0 via flag sozinha, sem NullProof algum.
    REQUIRE(find_status(h, StatusId::NullProof) == nullptr);  // nunca existiu, nada a consumir.
}

// ===== 7. Preview espelha: estimate_card_damage com NullProof NAO retorna immune=true, bate =====
// =====    com a faixa real, e NAO muta (2x = mesmo resultado, status presente) ==============

TEST_CASE("techmagic godel: estimate_card_damage espelha o pierce (mult -> 1.0, immune=false, "
         "faixa bate com o dano real) SEM consumir o NullProof - preview e PURO",
         "[domain][combat][techmagic][godel]") {
    Card card = damage_card("techmagic.godel.preview", CardFamily::Eletrico, /*power=*/10);
    auto reg = registry({card});
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/99);

    CombatActor h = make_actor("h", true, /*hp=*/100, /*atk=*/8, /*def=*/0, /*spd=*/50);
    CombatActor e = make_actor("e", false, /*hp=*/300, /*atk=*/0, /*def=*/0, /*spd=*/10,
                               CardFamily::Bioquimico);  // Resistente (0.66 sem pierce).
    h.add_status(
        StatusEffect{StatusId::NullProof, 0, 99, StackRule::Refresh, CardFamily::Universal});

    CombatStateMachine sm({&h, &e}, [](CombatActor&, const CombatState&) {
        return CombatAction::pass();
    }, &reg, nullptr, &rng);

    const CardDamageEstimate est1 = sm.estimate_card_damage(h, e, card);
    REQUIRE(est1.mult_fraqueza == 1.0f);  // pierced (nao 0.66).
    REQUIRE_FALSE(est1.immune);
    // base_damage pierced = (10+8)*1.0 = 18; v (kills=0) = 0.3. piso r=0: 18*0.7=12.6 ->
    // round 13; teto r=1: 18*1.3=23.4 -> round 23 (mesmos helpers comum_channel_damage de
    // resolve_use_card, secao 4 do metodo).
    REQUIRE(est1.min_damage == 13);
    REQUIRE(est1.max_damage == 23);

    // Contraste SEM NullProof (mult continuaria 0.66): piso 11.88*0.7=8.316 -> round 8, bem
    // ABAIXO da faixa pierced - prova que o preview realmente reflete o furo, nao so por
    // coincidencia numerica.
    CombatActor h_no_pierce = make_actor("h2", true, /*hp=*/100, /*atk=*/8, /*def=*/0,
                                         /*spd=*/50);
    const CardDamageEstimate est_control = sm.estimate_card_damage(h_no_pierce, e, card);
    REQUIRE(est_control.mult_fraqueza == combat_constants::kMultResistente);
    REQUIRE(est_control.min_damage == 8);
    REQUIRE(est_control.min_damage < est1.min_damage);

    // NAO muta: 2a chamada devolve o MESMO resultado, e o status continua presente (preview
    // PURO - NUNCA chama remove_status).
    const CardDamageEstimate est2 = sm.estimate_card_damage(h, e, card);
    REQUIRE(est1 == est2);
    REQUIRE(find_status(h, StatusId::NullProof) != nullptr);

    // Paridade com o dano REAL: joga a mesma carta (canal COMUM, r=0.5 => variancia zero,
    // MESMA formula de comum_channel_damage) e confere que o dano real cai dentro da faixa
    // [min_damage, max_damage] prevista pelo preview (ja pierced).
    auto provider = play_by_actor({{"h", CombatAction::use_card(card.id, e.id())}});
    CombatStateMachine sm2({&h, &e}, provider, &reg, nullptr, &rng);
    sm2.begin_turn();
    sm2.run_active_turn_to_end();
    const int real_damage = 300 - e.hp();
    REQUIRE(real_damage >= est1.min_damage);
    REQUIRE(real_damage <= est1.max_damage);
}

// ===== 8. Domino ChainDamage/Tesla: o dano PIERCED (mult 1.0) alimenta ctx.damage da cadeia - =====
// =====    com NullProof o salto e MAIOR (escala do dano pierced, nao do dano Resistente) =====

TEST_CASE("techmagic godel (domino Tesla): NullProof no atacante pierce o dano do primario, "
         "que alimenta ctx.damage da ChainDamage - o salto escala do dano PIERCED",
         "[domain][combat][techmagic][godel]") {
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/99);

    // Controle: SEM NullProof. Primario Resistente (mult 0.66): base=(10+8)*0.66=11.88 ->
    // round=12. Salto (magnitude 1, percent 50%): lround(12*0.5)=6.
    {
        Card card = chain_card("techmagic.godel.chain_control", CardFamily::Eletrico,
                               /*power=*/10);
        auto reg = registry({card});
        CombatActor h = make_actor("h", true, /*hp=*/100, /*atk=*/8, /*def=*/0, /*spd=*/50);
        CombatActor primary = make_actor("e0", false, /*hp=*/300, /*atk=*/0, /*def=*/0,
                                         /*spd=*/20, CardFamily::Bioquimico);
        CombatActor jump = make_actor("e1", false, /*hp=*/300, /*atk=*/0, /*def=*/0,
                                      /*spd=*/10, CardFamily::Eletrico);
        auto provider = play_by_actor({{"h", CombatAction::use_card(card.id, primary.id())}});
        CombatStateMachine sm({&h, &primary, &jump}, provider, &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();

        REQUIRE(primary.max_hp() - primary.hp() == 12);
        REQUIRE(jump.max_hp() - jump.hp() == 6);
    }

    // Com NullProof: pierce -> mult 1.0. base=(10+8)*1.0=18. Salto: lround(18*0.5)=9.
    {
        Card card = chain_card("techmagic.godel.chain_pierced", CardFamily::Eletrico,
                               /*power=*/10);
        auto reg = registry({card});
        CombatActor h = make_actor("h", true, /*hp=*/100, /*atk=*/8, /*def=*/0, /*spd=*/50);
        CombatActor primary = make_actor("e0", false, /*hp=*/300, /*atk=*/0, /*def=*/0,
                                         /*spd=*/20, CardFamily::Bioquimico);
        CombatActor jump = make_actor("e1", false, /*hp=*/300, /*atk=*/0, /*def=*/0,
                                      /*spd=*/10, CardFamily::Eletrico);
        h.add_status(
            StatusEffect{StatusId::NullProof, 0, 99, StackRule::Refresh, CardFamily::Universal});
        auto provider = play_by_actor({{"h", CombatAction::use_card(card.id, primary.id())}});
        CombatStateMachine sm({&h, &primary, &jump}, provider, &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();

        REQUIRE(primary.max_hp() - primary.hp() == 18);
        REQUIRE(jump.max_hp() - jump.hp() == 9);  // maior que o controle (6): domino provado.
        REQUIRE(find_status(h, StatusId::NullProof) == nullptr);  // consumido no primario.
    }
}

// ===== 9. Cast godel em ALIADO: aliado ganha NullProof; consumo no ataque DO ALIADO, nao do =====
// =====    caster ==============================================================================

TEST_CASE("techmagic godel (via FSM): cast em ALIADO concede NullProof a ELE (nao ao caster); "
         "o consumo acontece no ataque do ALIADO",
         "[domain][combat][techmagic][godel]") {
    Card godel = godel_card("techmagic.godel.ally_cast");
    Card dmg = damage_card("techmagic.godel.ally_dmg", CardFamily::Eletrico, /*power=*/10);
    auto reg = registry({godel, dmg});
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/99);

    CombatActor caster = make_actor("caster", true, /*hp=*/100, /*atk=*/8, /*def=*/0, /*spd=*/50);
    CombatActor ally = make_actor("ally", true, /*hp=*/100, /*atk=*/8, /*def=*/0, /*spd=*/40);
    CombatActor foe = make_actor("foe", false, /*hp=*/300, /*atk=*/0, /*def=*/0, /*spd=*/10,
                                 CardFamily::Bioquimico);  // Resistente ao card Eletrico.

    auto provider = play_by_actor({
        {"caster", CombatAction::use_card(godel.id, ally.id())},
        {"ally", CombatAction::use_card(dmg.id, foe.id())},
    });
    CombatStateMachine sm({&caster, &ally, &foe}, provider, &reg, nullptr, &rng);

    // Turno 1: caster casta Godel no ally.
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(find_status(caster, StatusId::NullProof) == nullptr);  // caster NAO ganhou.
    REQUIRE(find_status(ally, StatusId::NullProof) != nullptr);    // ally ganhou.

    // Turno 2: ally ataca o foe Resistente - o PROPRIO NullProof do ally fura e e consumido.
    sm.advance_to_next_actor();
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(foe.max_hp() - foe.hp() == 18);  // pierced (nao 12).
    REQUIRE(find_status(ally, StatusId::NullProof) == nullptr);  // consumido.
}

// ===== 10. Cast godel em INIMIGO: dissipa (side_filter AllyOnly, guard do PR1) ==============

TEST_CASE("techmagic godel (via FSM): cast em alvo INIMIGO dissipa (side_filter AllyOnly), "
         "nunca lanca, inimigo nao ganha o status",
         "[domain][combat][techmagic][godel]") {
    Card godel = godel_card("techmagic.godel.enemy_dissipate");
    auto reg = registry({godel});
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/99);

    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/8, /*def=*/0, /*spd=*/50);
    CombatActor foe = make_actor("e", false, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    auto provider = play_by_actor({{"h", CombatAction::use_card(godel.id, foe.id())}});
    CombatStateMachine sm({&caster, &foe}, provider, &reg, nullptr, &rng);

    REQUIRE_NOTHROW(sm.begin_turn());
    REQUIRE_NOTHROW(sm.run_active_turn_to_end());

    REQUIRE(find_status(foe, StatusId::NullProof) == nullptr);
    REQUIRE(find_status(caster, StatusId::NullProof) == nullptr);
    bool dissipate_logged = false;
    for (const auto& entry : sm.log())
        if (entry.message.find("hostis") != std::string::npos) dissipate_logged = true;
    REQUIRE(dissipate_logged);
}

// ===== 11. 1x/batalha via specials_cast_: 2o cast lanca ERRO DE COMPILACAO (mesmo gate das =====
// =====     outras Ativa/Hibrida) =============================================================

TEST_CASE("techmagic godel (via FSM): 1x/batalha - 2o cast de Godel lanca ERRO DE COMPILACAO",
         "[domain][combat][techmagic][godel]") {
    CombatActor h = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e = make_actor("e", false, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    Card godel = godel_card("techmagic.godel.gate");
    auto reg = registry({godel});
    FixedRandom rng(0.5, 99);

    int calls = 0;
    CombatActionProvider provider = [&](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side()) return CombatAction::pass();
        ++calls;
        return CombatAction::use_card(godel.id, h.id());  // self-cast (AllyOnly aceita).
    };

    CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);
    sm.begin_turn();

    bool threw = false;
    try {
        sm.run_active_turn_to_end();
    } catch (const std::logic_error& ex) {
        threw = true;
        REQUIRE(std::string(ex.what()).find("ERRO DE COMPILACAO") != std::string::npos);
    }
    REQUIRE(threw);
    REQUIRE(calls == 2);
    REQUIRE(find_status(h, StatusId::NullProof) != nullptr);  // 1o cast aplicou normalmente.
}
