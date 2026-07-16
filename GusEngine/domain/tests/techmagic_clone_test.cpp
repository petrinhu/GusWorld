// techmagic_clone_test.cpp
//
// Spec executavel (Catch2 v3) do EffectKind::CloneAlly (Eco/Molde-Fiel, von Neumann/Fork +
// Giordano Bruno/Echo-Self) + EffectKind::TokenRefund (Construtor Universal), CARD-ENGINE-
// MANIFESTO item 8 - o ULTIMO item do manifesto (decisoes do lider no brief 2026-07-16,
// AMB-11 em docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md):
//
//   Clone = STATUS-DRIVEN (StatusId::Eco, ordinal 19), NAO entidade nova na fila (decisao
//   AUTONOMA a confirmar retroativamente - respeita o canon do lider 2026-07-14: sem 4o
//   ator na fila, Party=3 intacto). handle_clone_ally (OnCast) aplica o Eco no ALVO da
//   carta (side_filter AllyOnly, inimigo dissipa) via add_status LEGADO (buff, fora do
//   portao de imunidade). Ao FIM de CADA turno PROPRIO do portador (os DOIS sitios de
//   fim-de-turno, ANTES de expire_elapsed_statuses - CombatStateMachine::
//   process_eco_turn_end_hook), se a ULTIMA ACAO DE DANO desta rodada foi do PROPRIO
//   portador (last_action_.actor == portador), techMagic::replicate_eco reaplica os hits
//   dele a magnitude% via take_damage PURO (anti-recursao/anti-inflacao, MESMA disciplina
//   de RepeatLastAction/HypotenuseCombo/Reflect).
//
//   TokenRefund = refund do gate 1x/batalha das especiais Ativa/Hibrida (marcador fora do
//   dispatcher, mesmo padrao DamageQuantize/DiversityBonus/ApEfficiency): a 1a especial
//   jogada na batalha (inclusive a PROPRIA von Neumann) "se reconstroi" - nao entra em
//   specials_cast_ - SE algum VIVO do lado do conjurador porta TokenRefund equipado E o
//   refund ainda nao foi gasto (1x/batalha).
//
// TESTE-REI (secao 6): composicao Ada (RepeatLastAction/OnAllyTurnEnd) + Eco no mesmo
// ator/rodada NAO recursa nem dobra indevidamente - os dois handlers usam take_damage
// PURO e NENHUM dos dois toca last_action_/round_hits_, entao o segundo a rodar sempre le
// o dado ORIGINAL, nunca o eco do outro.
//
// Cartas de teste montadas LOCALMENTE (id "techmagic.clone.*"), NUNCA do registry de
// producao (MasterCards) - mesmo padrao de techmagic_delay_test.cpp/techmagic_hayek_test.cpp.
// O teste do CATALOGO (vonneumann/bruno em MasterCards::build_registry()) vive em
// master_cards_test.cpp.
//
// Cross-ref: gus/domain/combat/combat_enums.hpp (StatusId::Eco/EffectKind::CloneAlly/
//            TokenRefund); gus/domain/combat/techmagic.hpp (techMagic::replicate_eco);
//            gus/domain/combat/techmagic.cpp (handle_clone_ally/handle_token_refund);
//            combat_state_machine.cpp (process_eco_turn_end_hook/
//            token_refund_equipped_on_side/resolve_use_card); master_cards.cpp
//            (vonneumann/bruno); docs/design/mecanicas/cartas-technomagik.md secao 9;
//            docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md (AMB-11);
//            docs/tech/adr/ADR-016-techmagic-effect-engine-data-driven.md.

#include <catch2/catch_test_macros.hpp>

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "counting_random.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/domain/combat/techmagic.hpp"

using namespace gus::domain::combat;
using gus::domain::tests::CountingRandom;

namespace {

CombatActor make_actor(const std::string& id, bool player_side, int hp = 100, int atk = 0,
                       int def = 0, int spd = 20) {
    return CombatActor(id, id, hp, atk, def, spd, CardFamily::Eletrico, player_side);
}

const StatusEffect* find_status(const CombatActor& a, StatusId id) {
    for (const auto& s : a.status_effects())
        if (s.id == id) return &s;
    return nullptr;
}

// Duplo LOCAL de von Neumann/Fork: Hibrida/Universal, OnCast -> CloneAlly (Eco
// magnitude/duration configuraveis, AllyOnly) + OnCast -> TokenRefund. target_shape
// configuravel (Single por padrao, mesmo shape de producao).
Card vonneumann_card(const std::string& id, int magnitude = 50, int duration = 3,
                     bool with_refund = true,
                     TargetShape target_shape = TargetShape::Single) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Universal;
    c.base_type = CardBaseType::Glifo;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = 0;
    c.target_shape = target_shape;
    c.tier = CardTier::Especial;
    c.category = CardCategory::Hibrida;
    c.effects = {EffectSpec{.trigger = TriggerHook::OnCast,
                            .kind = EffectKind::CloneAlly,
                            .magnitude = magnitude,
                            .duration = duration,
                            .status = StatusId::Eco,
                            .stack_rule = StackRule::Refresh,
                            .side_filter = SideFilter::AllyOnly}};
    if (with_refund)
        c.effects.push_back(
            EffectSpec{.trigger = TriggerHook::OnCast, .kind = EffectKind::TokenRefund});
    return c;
}

// Duplo LOCAL de Giordano Bruno/Echo-Self: Ativa/Universal/Self, OnCast -> CloneAlly (sem
// TokenRefund - so o von Neumann porta a passiva-dupla).
Card bruno_card(const std::string& id, int magnitude = 62, int duration = 2) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Universal;
    c.base_type = CardBaseType::Glifo;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = 0;
    c.target_shape = TargetShape::Self;
    c.tier = CardTier::Especial;
    c.category = CardCategory::Ativa;
    c.effects = {EffectSpec{.trigger = TriggerHook::OnCast,
                            .kind = EffectKind::CloneAlly,
                            .magnitude = magnitude,
                            .duration = duration,
                            .status = StatusId::Eco,
                            .stack_rule = StackRule::Refresh,
                            .side_filter = SideFilter::AllyOnly}};
    return c;
}

// Duplo LOCAL de Ada/Re-Run (mesmo shape de techmagic_repeat_test.cpp): Passiva mana 0,
// OnAllyTurnEnd -> RepeatLastAction. magnitude=0 (chance) = SEMPRE dispara, 0 consumo de
// RNG - usado como surrogate deterministico no TESTE-REI (secao 6), sem depender de
// FixedRandom.
Card ada_surrogate_card(const std::string& id, int chance, int percent = 100) {
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
    c.category = CardCategory::Passiva;
    c.effects = {EffectSpec{.trigger = TriggerHook::OnAllyTurnEnd,
                            .kind = EffectKind::RepeatLastAction,
                            .magnitude = chance,
                            .percent = percent}};
    return c;
}

std::unordered_map<std::string, Card> registry(std::initializer_list<Card> cards) {
    std::unordered_map<std::string, Card> d;
    for (const auto& c : cards) d.emplace(c.id, c);
    return d;
}

// Plano de acoes POR RODADA (mesmo padrao de techmagic_repeat_test.cpp/
// techmagic_roundend_test.cpp).
using ActionPlan = std::unordered_map<std::string, std::unordered_map<int, std::vector<CombatAction>>>;

CombatActionProvider provider_for(ActionPlan plan) {
    auto plan_ptr = std::make_shared<ActionPlan>(std::move(plan));
    auto idx_ptr = std::make_shared<std::map<std::pair<std::string, int>, std::size_t>>();
    return [plan_ptr, idx_ptr](CombatActor& a, const CombatState& state) -> CombatAction {
        const auto ait = plan_ptr->find(a.id());
        if (ait == plan_ptr->end()) return CombatAction::pass();
        const auto rit = ait->second.find(state.round_index());
        if (rit == ait->second.end()) return CombatAction::pass();
        std::size_t& i = (*idx_ptr)[{a.id(), state.round_index()}];
        if (i < rit->second.size()) return rit->second[i++];
        return CombatAction::pass();
    };
}

// Conduz UM turno completo (begin_turn -> run_active_turn_to_end -> advance_to_next_actor).
void act(CombatStateMachine& sm) {
    sm.begin_turn();
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();
}

}  // namespace

// ===== 1. handle_clone_ally (execute() direto): aplica Eco, dissipa em inimigo, self, =====
// =====    guarda de argumentos =========================================================

TEST_CASE("techmagic clone: handle_clone_ally aplica StatusId::Eco no ALIADO alvo "
         "(magnitude/duration/stack_rule/familia Universal preservados)",
         "[domain][combat][techmagic][clone]") {
    const Card c = vonneumann_card("techmagic.clone.apply", /*magnitude=*/50,
                                   /*duration=*/3, /*with_refund=*/false);

    CombatActor caster = make_actor("h", true);
    CombatActor ally = make_actor("h2", true);

    std::vector<CombatLogEntry> log;
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &ally;
    ctx.log = &log;

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, c, ctx));

    const StatusEffect* eco = find_status(ally, StatusId::Eco);
    REQUIRE(eco != nullptr);
    REQUIRE(eco->magnitude == 50);
    REQUIRE(eco->duration == 3);
    REQUIRE(eco->stack_rule == StackRule::Refresh);
    // Regra todo-efeito-loga: o log de INVOCACAO nao pode ser silenciavel (mata o mutante
    // 8a do qa adversarial). Frase exata emitida por handle_clone_ally.
    REQUIRE_FALSE(log.empty());
    REQUIRE(log.back().message.find("molde de eco ativo") != std::string::npos);
    REQUIRE(eco->family_origin == CardFamily::Universal);
}

TEST_CASE("techmagic clone: handle_clone_ally SELF-target (Bruno) aplica o Eco no PROPRIO "
         "conjurador",
         "[domain][combat][techmagic][clone]") {
    const Card c = bruno_card("techmagic.clone.self", /*magnitude=*/62, /*duration=*/2);

    CombatActor caster = make_actor("h", true);

    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &caster;  // TargetShape::Self.

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, c, ctx));

    const StatusEffect* eco = find_status(caster, StatusId::Eco);
    REQUIRE(eco != nullptr);
    REQUIRE(eco->magnitude == 62);
    REQUIRE(eco->duration == 2);
}

TEST_CASE("techmagic clone: handle_clone_ally DISSIPA em alvo INIMIGO (side_filter "
         "AllyOnly), no-op logado, NAO lanca",
         "[domain][combat][techmagic][clone]") {
    const Card c = vonneumann_card("techmagic.clone.dissipate", /*magnitude=*/50,
                                   /*duration=*/3, /*with_refund=*/false);

    CombatActor caster = make_actor("h", true);
    CombatActor foe = make_actor("e", false);

    std::vector<CombatLogEntry> log;
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &foe;
    ctx.log = &log;

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, c, ctx));
    REQUIRE(find_status(foe, StatusId::Eco) == nullptr);
    REQUIRE_FALSE(log.empty());
    REQUIRE(log.back().message.find("dissipa") != std::string::npos);
}

TEST_CASE("techmagic clone: handle_clone_ally lanca logic_error se ctx.caster ou "
         "ctx.counterpart forem nulos",
         "[domain][combat][techmagic][clone]") {
    const Card c = vonneumann_card("techmagic.clone.guards", 50, 3, /*with_refund=*/false);
    CombatActor caster = make_actor("h", true);
    CombatActor ally = make_actor("h2", true);

    techMagic::TechMagicContext no_caster;
    no_caster.counterpart = &ally;
    REQUIRE_THROWS_AS(techMagic::execute(TriggerHook::OnCast, c, no_caster),
                     std::logic_error);

    techMagic::TechMagicContext no_target;
    no_target.caster = &caster;
    REQUIRE_THROWS_AS(techMagic::execute(TriggerHook::OnCast, c, no_target),
                     std::logic_error);
}

// ===== 2. TokenRefund: handler no-op (dispatcher direto, execute() nao muta nada) =====

TEST_CASE("techmagic clone: handle_token_refund executa via techMagic::execute sem "
         "lancar - marcador no-op deliberado (o refund vive fora do dispatcher)",
         "[domain][combat][techmagic][clone]") {
    Card c;
    c.id = "techmagic.clone.tokenrefund.noop";
    c.display_name = c.id;
    c.family = CardFamily::Universal;
    c.tier = CardTier::Especial;
    c.category = CardCategory::Passiva;
    c.effects = {EffectSpec{.trigger = TriggerHook::OnCast, .kind = EffectKind::TokenRefund}};

    CombatActor caster = make_actor("h", true);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, c, ctx));
}

// ===== 3. techMagic::replicate_eco (chamada isolada): echoable, ocioso, multi-alvo, =====
// =====    alvo morto, arredondamento a 0, guardas de argumento =========================

TEST_CASE("techmagic clone: replicate_eco reaplica os hits do PROPRIO portador a "
         "percent% via take_damage PURO",
         "[domain][combat][techmagic][clone]") {
    CombatActor portador = make_actor("h", true);
    CombatActor t1 = make_actor("e1", false, /*hp=*/300);
    CombatActor t2 = make_actor("e2", false, /*hp=*/300);

    const techMagic::LastActionRecord last{
        &portador, CombatActionType::Attack, /*card_id=*/"", {{&t1, 7}, {&t2, 9}}};

    std::vector<CombatLogEntry> log;
    techMagic::TechMagicContext ctx;
    ctx.caster = &portador;
    ctx.last_action = &last;
    ctx.log = &log;

    techMagic::replicate_eco(/*percent=*/50, ctx);

    // lround(7*0.5)=4 (arredonda pra cima); lround(9*0.5)=5 (banker/lround padrao arredonda
    // 4.5 pra 5, mesma convencao ja usada em RepeatLastAction).
    REQUIRE(t1.hp() == 300 - 4);
    REQUIRE(t2.hp() == 300 - 5);
    // Regra todo-efeito-loga: cada hit replicado emite uma linha "o eco ... replica ..."
    // (mata o mutante 8c do qa adversarial - log do eco replicado silenciavel). 1 linha por
    // alvo; a ultima e a do t2.
    REQUIRE_FALSE(log.empty());
    REQUIRE(log.back().message.find("replica") != std::string::npos);
}

TEST_CASE("techmagic clone: replicate_eco - eco OCIOSO quando a ultima acao NAO foi do "
         "PROPRIO portador (mesmo do lado, mesmo aliado - exige actor == ctx.caster "
         "EXATAMENTE)",
         "[domain][combat][techmagic][clone]") {
    CombatActor portador = make_actor("h", true);
    CombatActor outro_aliado = make_actor("h2", true);
    CombatActor t = make_actor("e", false, /*hp=*/300);

    const techMagic::LastActionRecord last{
        &outro_aliado, CombatActionType::Attack, /*card_id=*/"", {{&t, 10}}};

    std::vector<CombatLogEntry> log;
    techMagic::TechMagicContext ctx;
    ctx.caster = &portador;
    ctx.last_action = &last;
    ctx.log = &log;

    techMagic::replicate_eco(/*percent=*/50, ctx);

    REQUIRE(t.hp() == 300);  // sem eco: a acao foi de OUTRO aliado, nao do portador.
    REQUIRE_FALSE(log.empty());
    REQUIRE(log.back().message.find("ocioso") != std::string::npos);
}

TEST_CASE("techmagic clone: replicate_eco - eco OCIOSO quando hits esta vazio (portador "
         "nao causou dano nesta rodada)",
         "[domain][combat][techmagic][clone]") {
    CombatActor portador = make_actor("h", true);
    const techMagic::LastActionRecord last{&portador, CombatActionType::Defend,
                                           /*card_id=*/"", /*hits=*/{}};

    std::vector<CombatLogEntry> log;
    techMagic::TechMagicContext ctx;
    ctx.caster = &portador;
    ctx.last_action = &last;
    ctx.log = &log;

    REQUIRE_NOTHROW(techMagic::replicate_eco(50, ctx));
    // Regra todo-efeito-loga: hits vazio (last_action_.actor==portador, mas 0 dano proprio)
    // ainda loga o "eco ocioso" - o guard `!last.hits.empty()` nao pode virar no-op MUDO
    // (mata o mutante 5 do qa adversarial - echoable sem o sub-guard de hits-vazio). Mesma
    // asercao do teste irmao acima (ultima acao de OUTRO aliado).
    REQUIRE_FALSE(log.empty());
    REQUIRE(log.back().message.find("ocioso") != std::string::npos);
}

TEST_CASE("techmagic clone: replicate_eco pula alvo MORTO nos hits (sem crash, os demais "
         "alvos ainda ecoam)",
         "[domain][combat][techmagic][clone]") {
    CombatActor portador = make_actor("h", true);
    CombatActor morto = make_actor("e1", false, /*hp=*/5);
    morto.take_damage(5);  // hp=0, morto.
    REQUIRE_FALSE(morto.is_alive());
    CombatActor vivo = make_actor("e2", false, /*hp=*/300);

    const techMagic::LastActionRecord last{
        &portador, CombatActionType::Attack, /*card_id=*/"", {{&morto, 10}, {&vivo, 10}}};

    techMagic::TechMagicContext ctx;
    ctx.caster = &portador;
    ctx.last_action = &last;

    REQUIRE_NOTHROW(techMagic::replicate_eco(50, ctx));
    REQUIRE(morto.hp() == 0);       // take_damage no cadaver seria clamp em 0 de qualquer
                                    // forma, mas o guard is_alive() pula ANTES de tentar.
    REQUIRE(vivo.hp() == 300 - 5);  // lround(10*0.5)=5.
}

TEST_CASE("techmagic clone: replicate_eco - echo que arredonda pra <=0 e pulado (sem "
         "take_damage(0) nem log de hit)",
         "[domain][combat][techmagic][clone]") {
    CombatActor portador = make_actor("h", true);
    CombatActor t = make_actor("e", false, /*hp=*/300);

    const techMagic::LastActionRecord last{&portador, CombatActionType::Attack,
                                           /*card_id=*/"", {{&t, 1}}};

    techMagic::TechMagicContext ctx;
    ctx.caster = &portador;
    ctx.last_action = &last;

    // lround(1 * 10/100) = lround(0.1) = 0 -> pulado.
    REQUIRE_NOTHROW(techMagic::replicate_eco(/*percent=*/10, ctx));
    REQUIRE(t.hp() == 300);
}

TEST_CASE("techmagic clone: replicate_eco lanca logic_error se ctx.caster ou "
         "ctx.last_action forem nulos",
         "[domain][combat][techmagic][clone]") {
    CombatActor portador = make_actor("h", true);
    const techMagic::LastActionRecord last;

    techMagic::TechMagicContext no_caster;
    no_caster.last_action = &last;
    REQUIRE_THROWS_AS(techMagic::replicate_eco(50, no_caster), std::logic_error);

    techMagic::TechMagicContext no_last_action;
    no_last_action.caster = &portador;
    REQUIRE_THROWS_AS(techMagic::replicate_eco(50, no_last_action), std::logic_error);
}

// ===== 4. Integracao FSM: cast real -> Eco aplicado -> eco replica no fim do turno do =====
// =====    PROPRIO portador (von Neumann em aliado, Bruno em self, dissipacao em =====
// =====    inimigo real, turno sem dano = ocioso, morte-mid-Eco) =========================

TEST_CASE("techmagic clone: von Neumann conjurado em ALIADO aplica Eco; no fim do turno "
         "PROPRIO do aliado (nao do conjurador), o ataque dele ecoa 50%",
         "[domain][combat][techmagic][clone]") {
    CombatActor h1 = make_actor("h1", true, 100, 0, 0, /*spd=*/50);  // conjura von Neumann.
    CombatActor h2 = make_actor("h2", true, 100, 10, 0, /*spd=*/40); // clonado; ataca e.
    CombatActor e = make_actor("e", false, 300, 0, 0, /*spd=*/10);

    auto reg = registry({vonneumann_card("vn_test", /*magnitude=*/50, /*duration=*/3,
                                         /*with_refund=*/false)});

    auto provider = provider_for(
        {{"h1", {{0, {CombatAction::use_card("vn_test", h2.id())}}}},
         {"h2", {{0, {CombatAction::attack(e.id())}}}}});
    CombatStateMachine sm({&h1, &h2, &e}, provider, &reg);

    act(sm);  // h1 conjura von Neumann em h2: fogo amigo desligado (0 dano proprio) + Eco
              // aplicado em h2 (side_filter AllyOnly, alvo aliado - nao dissipa).
    REQUIRE(find_status(h2, StatusId::Eco) != nullptr);
    REQUIRE(e.hp() == 300);  // nenhum dano ainda.

    act(sm);  // h2 ataca e por 10 (atk 10, def 0). Turno de h2 fecha: eco (magnitude 50%)
              // reaplica 50% de 10 = 5, ANTES de qualquer outra coisa.
    REQUIRE(e.hp() == 300 - 10 - 5);
}

TEST_CASE("techmagic clone: Giordano Bruno (self) conjurado por h1 aplica Eco no PROPRIO "
         "h1; o proprio ataque de h1 (turno seguinte) ecoa 62%",
         "[domain][combat][techmagic][clone]") {
    CombatActor h1 = make_actor("h1", true, 100, 10, 0, /*spd=*/50);
    CombatActor e = make_actor("e", false, 300, 0, 0, /*spd=*/10);

    auto reg = registry({bruno_card("bruno_test", /*magnitude=*/62, /*duration=*/2)});

    // Rodada 0: h1 conjura Bruno em si mesmo. Rodada 1: h1 ataca e.
    auto provider = provider_for(
        {{"h1", {{0, {CombatAction::use_card("bruno_test", h1.id())}},
                {1, {CombatAction::attack(e.id())}}}}});
    CombatStateMachine sm({&h1, &e}, provider, &reg);

    act(sm);  // rodada 0: h1 conjura Bruno em si (TargetShape::Self) -> Eco aplicado.
    REQUIRE(find_status(h1, StatusId::Eco) != nullptr);
    act(sm);  // rodada 0: e passa -> fecha a rodada (wrap), last_action_ zera.

    act(sm);  // rodada 1: h1 ataca e por 10. Turno de h1 fecha: eco (62%) reaplica
              // lround(10*0.62)=6.
    REQUIRE(e.hp() == 300 - 10 - 6);
}

TEST_CASE("techmagic clone: von Neumann mirado num INIMIGO real (via UseCard) dissipa - "
         "sem Eco, log de dissipacao, o motor NAO lanca",
         "[domain][combat][techmagic][clone]") {
    CombatActor h1 = make_actor("h1", true, 100, 0, 0, /*spd=*/50);
    CombatActor e = make_actor("e", false, 100, 0, 0, /*spd=*/10);

    auto reg = registry({vonneumann_card("vn_dissipate", 50, 3, /*with_refund=*/false)});

    auto provider =
        provider_for({{"h1", {{0, {CombatAction::use_card("vn_dissipate", e.id())}}}}});
    CombatStateMachine sm({&h1, &e}, provider, &reg);

    act(sm);  // h1 mira von Neumann em 'e' (inimigo): dano-base normal roda (nao e o ramo
             // "fogo amigo desligado" - alvo e do lado OPOSTO), mas o OnCast->CloneAlly
             // dissipa (side_filter AllyOnly, alvo hostil).
    REQUIRE(find_status(e, StatusId::Eco) == nullptr);
    bool has_dissipa_log = false;
    for (const auto& entry : sm.log())
        if (entry.message.find("dissipa") != std::string::npos) has_dissipa_log = true;
    REQUIRE(has_dissipa_log);
}

TEST_CASE("techmagic clone: turno do portador SEM dano (Defend) = eco OCIOSO, nenhum "
         "dano extra",
         "[domain][combat][techmagic][clone]") {
    CombatActor h2 = make_actor("h2", true, 100, 10, 0, /*spd=*/40);
    CombatActor e = make_actor("e", false, 300, 0, 0, /*spd=*/10);
    h2.add_status(StatusEffect{StatusId::Eco, /*magnitude=*/50, /*duration=*/3,
                               StackRule::Refresh, CardFamily::Universal});

    auto provider = provider_for({{"h2", {{0, {CombatAction::defend()}}}}});
    CombatStateMachine sm({&h2, &e}, provider, nullptr);

    act(sm);  // h2 defende (0 dano); last_action_ continua vazio -> eco ocioso.
    REQUIRE(e.hp() == 300);
}

TEST_CASE("techmagic clone: portador MORTO antes do proprio TurnEnd (Poison letal no "
         "TurnStart) - process_eco_turn_end_hook (via expire_on_stunned_turn_end) nao "
         "crasha, so no-op silencioso",
         "[domain][combat][techmagic][clone]") {
    CombatActor h2 = make_actor("h2", true, /*hp=*/5, 10, 0, /*spd=*/40);
    CombatActor e = make_actor("e", false, 300, 0, 0, /*spd=*/10);
    h2.add_status(StatusEffect{StatusId::Eco, 50, 3, StackRule::Refresh,
                               CardFamily::Universal});
    h2.add_status(StatusEffect{StatusId::Poison, /*magnitude=*/10, /*duration=*/1,
                               StackRule::Replace, CardFamily::Eletrico});

    CombatActionProvider always_pass = [](CombatActor&, const CombatState&) {
        return CombatAction::pass();
    };
    CombatStateMachine sm({&h2, &e}, always_pass, nullptr);

    // run_until_end() cobre o caminho REAL ponta-a-ponta: begin_turn() aplica o tick de
    // Poison (10 dano em hp=5 -> h2 morre ANTES de agir), o runner detecta
    // `!queue_.current()->is_alive()` e chama expire_on_stunned_turn_end(h2), que despacha
    // process_eco_turn_end_hook(h2) - o guard `is_alive()` no topo do hook precisa
    // sobreviver a um portador JA morto sem lancar nem tentar ecoar.
    REQUIRE_NOTHROW(sm.run_until_end());
    REQUIRE_FALSE(h2.is_alive());
    REQUIRE(e.hp() == 300);  // sem eco: h2 nunca causou dano (morreu antes de agir).
}

// ===== 5. Duracao = N turnos PROPRIOS do portador, EXATOS (cobre o ULTIMO turno em que =====
// =====    a duracao ja tickou pra 0 mas ainda nao foi removida pelo expire) =============

TEST_CASE("techmagic clone: Eco com duration=3 cobre EXATAMENTE 3 turnos PROPRIOS do "
         "portador (inclusive o ultimo, quando a duracao ja tickou pra 0) - o 4o turno "
         "nao ecoa mais",
         "[domain][combat][techmagic][clone]") {
    CombatActor h2 = make_actor("h2", true, 100, 10, 0, /*spd=*/50);  // sempre age primeiro.
    CombatActor e = make_actor("e", false, 1000, 0, 0, /*spd=*/10);
    h2.add_status(StatusEffect{StatusId::Eco, /*magnitude=*/50, /*duration=*/3,
                               StackRule::Refresh, CardFamily::Universal});

    auto provider = provider_for(
        {{"h2", {{0, {CombatAction::attack(e.id())}},
                {1, {CombatAction::attack(e.id())}},
                {2, {CombatAction::attack(e.id())}},
                {3, {CombatAction::attack(e.id())}}}}});
    CombatStateMachine sm({&h2, &e}, provider, nullptr);

    for (int round = 0; round < 4; ++round) {
        act(sm);  // h2 ataca por 10.
        act(sm);  // e passa -> fecha a rodada.
    }

    // Rodadas 0,1,2: Eco presente (duration tickando 3->2->1->0, removido SO no expire
    // DEPOIS do hook rodar na rodada 2) -> 3 ecos de 5 (lround(10*0.5)). Rodada 3: Eco ja
    // foi removido no expire da rodada 2 -> sem eco.
    // Base: 4 * 10 = 40. Eco: 3 * 5 = 15. Total = 55.
    REQUIRE(e.hp() == 1000 - 40 - 15);
    REQUIRE(find_status(h2, StatusId::Eco) == nullptr);  // expirou apos a rodada 2.
}

// ===== 6. TESTE-REI: composicao Ada (OnAllyTurnEnd/RepeatLastAction) + Eco no MESMO =====
// =====    ator/rodada - anti-recursao/anti-dobra (nenhum dos dois toca last_action_/ =====
// =====    round_hits_, entao o Ada le SEMPRE o dado ORIGINAL, nunca o eco do Eco) ========

TEST_CASE("techmagic clone: TESTE-REI - Eco (no proprio turno de A) + Ada (no fim do "
         "MESMO turno, reagindo a A) NAO recursam nem dobram - Ada ecoa o hit ORIGINAL, "
         "nao o ja-ecoado pelo Eco",
         "[domain][combat][techmagic][clone]") {
    CombatActor a = make_actor("a", true, 100, /*atk=*/10, 0, /*spd=*/50);
    CombatActor b = make_actor("b", true, 100, 0, 0, /*spd=*/40);  // dono do Ada; nao age.
    CombatActor t = make_actor("t", false, /*hp=*/300, 0, 0, /*spd=*/10);

    a.add_status(StatusEffect{StatusId::Eco, /*magnitude=*/50, /*duration=*/3,
                              StackRule::Refresh, CardFamily::Universal});
    b.set_equipped_special_ids({"ada_surrogate"});
    // magnitude=0 (chance) = SEMPRE dispara, 0 RNG - determinismo total no TESTE-REI.
    auto reg = registry({ada_surrogate_card("ada_surrogate", /*chance=*/0, /*percent=*/100)});

    auto provider = provider_for({{"a", {{0, {CombatAction::attack(t.id())}}}}});
    CombatStateMachine sm({&a, &b, &t}, provider, &reg);

    act(sm);  // a ataca t por 10: last_action_={a,{t:10}}. Turno de a fecha:
              // (1) process_eco_turn_end_hook(a) - a porta Eco -> replicate_eco 50% de 10
              //     = 5, via take_damage PURO (NAO toca last_action_/round_hits_).
              // (2) process_ally_turn_end_hooks(a) -> b (outro aliado, Ada equipada) ->
              //     echoable (last_action_.actor==a, MESMO lado) -> ecoa 100% do hit
              //     ORIGINAL {t:10} (INTACTO, o Eco nao o alterou) = +10.
    // Se o anti-recursao estivesse quebrado (Eco atualizando last_action_ pra si mesmo, ou
    // Ada ecoando o valor JA reduzido), o total seria diferente de 25.
    REQUIRE(t.hp() == 300 - 10 - 5 - 10);  // = 275.

    // Prova adicional: nem o Eco nem o Ada tocaram o ledger/registro - so o hit ORIGINAL
    // de 'a' esta no round_hits_, e last_action_ continua {a,{t:10}} intacto.
    REQUIRE(sm.round_hits().size() == 1);
    REQUIRE(sm.round_hits().front().damage == 10);
    REQUIRE(sm.last_action().actor == &a);
    REQUIRE(sm.last_action().hits.size() == 1);
    REQUIRE(sm.last_action().hits.front().second == 10);
}

// ===== 7. TokenRefund: refund 1x/batalha da 1a especial Ativa/Hibrida (inclusive a =====
// =====    PROPRIA von Neumann), gate normal SEM a passiva, 2o refund nao acontece =====

TEST_CASE("techmagic clone: SEM TokenRefund equipado, o gate 1x/batalha normal vale "
         "(2a compilacao da MESMA especial lanca ERRO DE COMPILACAO)",
         "[domain][combat][techmagic][clone]") {
    CombatActor h1 = make_actor("h1", true, 100, 0, 0, 50);
    CombatActor e = make_actor("e", false, 100, 0, 0, 10);
    auto reg = registry({vonneumann_card("vn_gate", 50, 3, /*with_refund=*/false)});

    auto provider = provider_for(
        {{"h1", {{0, {CombatAction::use_card("vn_gate", h1.id()),
                     CombatAction::use_card("vn_gate", h1.id())}}}}});
    CombatStateMachine sm({&h1, &e}, provider, &reg);

    REQUIRE_THROWS_AS(act(sm), std::logic_error);
}

TEST_CASE("techmagic clone: COM TokenRefund equipado num aliado VIVO do mesmo lado, a 1a "
         "especial Ativa/Hibrida jogada na batalha 'se reconstroi' - pode ser jogada de "
         "novo sem lancar",
         "[domain][combat][techmagic][clone]") {
    CombatActor h1 = make_actor("h1", true, 100, 0, 0, 50);
    CombatActor h2 = make_actor("h2", true, 100, 0, 0, 45);  // porta o refund.
    CombatActor e = make_actor("e", false, 100, 0, 0, 10);
    h2.set_equipped_special_ids({"tokenrefund_only"});

    Card refund_only;
    refund_only.id = "tokenrefund_only";
    refund_only.display_name = refund_only.id;
    refund_only.family = CardFamily::Universal;
    refund_only.tier = CardTier::Especial;
    refund_only.category = CardCategory::Passiva;
    refund_only.effects = {
        EffectSpec{.trigger = TriggerHook::OnCast, .kind = EffectKind::TokenRefund}};

    auto reg = registry({vonneumann_card("vn_refund", 50, 3, /*with_refund=*/false),
                         refund_only});

    auto provider = provider_for(
        {{"h1", {{0, {CombatAction::use_card("vn_refund", h1.id()),
                     CombatAction::use_card("vn_refund", h1.id())}}}}});
    CombatStateMachine sm({&h1, &h2, &e}, provider, &reg);

    // 1o cast: refund consumido, "vn_refund" NAO entra em specials_cast_. 2o cast (mesmo
    // turno, se AP permitir) ou turno seguinte: passa direto, sem ERRO DE COMPILACAO.
    REQUIRE_NOTHROW(act(sm));

    bool has_refund_log = false;
    for (const auto& entry : sm.log())
        if (entry.message.find("reconstroi") != std::string::npos) has_refund_log = true;
    REQUIRE(has_refund_log);
}

TEST_CASE("techmagic clone: o refund do TokenRefund cobre a PROPRIA carta que o porta "
         "(von Neumann com a passiva equipada nele mesmo consegue recastar 'Molde Fiel')",
         "[domain][combat][techmagic][clone]") {
    CombatActor h1 = make_actor("h1", true, 100, 0, 0, 50);  // porta e joga von Neumann.
    CombatActor h2 = make_actor("h2", true, 100, 0, 0, 45);  // alvo do clone.
    CombatActor e = make_actor("e", false, 100, 0, 0, 10);
    h1.set_equipped_special_ids({"vn_self_refund"});  // a PROPRIA carta equipada.

    auto reg = registry(
        {vonneumann_card("vn_self_refund", 50, 3, /*with_refund=*/true)});

    auto provider = provider_for(
        {{"h1", {{0, {CombatAction::use_card("vn_self_refund", h2.id()),
                     CombatAction::use_card("vn_self_refund", h2.id())}}}}});
    CombatStateMachine sm({&h1, &h2, &e}, provider, &reg);

    // 1o cast de "vn_self_refund": ele mesmo esta equipado em h1 -> refund cobre a PROPRIA
    // ativa (nao entra em specials_cast_) -> 2o cast no MESMO turno nao lanca.
    REQUIRE_NOTHROW(act(sm));
    REQUIRE(find_status(h2, StatusId::Eco) != nullptr);
}

TEST_CASE("techmagic clone: TokenRefund e 1x/BATALHA (nao 1x/carta) - apos o 1o refund "
         "ser gasto, a PROXIMA especial diferente ja gate normal",
         "[domain][combat][techmagic][clone]") {
    CombatActor h1 = make_actor("h1", true, 100, 0, 0, 50);
    CombatActor h2 = make_actor("h2", true, 100, 0, 0, 45);  // porta o refund.
    CombatActor e = make_actor("e", false, 100, 0, 0, 10);
    h2.set_equipped_special_ids({"tokenrefund_only2"});

    Card refund_only;
    refund_only.id = "tokenrefund_only2";
    refund_only.display_name = refund_only.id;
    refund_only.family = CardFamily::Universal;
    refund_only.tier = CardTier::Especial;
    refund_only.category = CardCategory::Passiva;
    refund_only.effects = {
        EffectSpec{.trigger = TriggerHook::OnCast, .kind = EffectKind::TokenRefund}};

    auto reg = registry({vonneumann_card("vn_a", 50, 3, /*with_refund=*/false),
                         bruno_card("bruno_b", 62, 2), refund_only});

    // h1 joga vn_a (1o cast da batalha -> refund consumido, "vn_a" nao entra no gate),
    // depois bruno_b (2a especial jogada -> refund JA foi gasto, entra no gate normal),
    // depois bruno_b DE NOVO (deveria lancar - 2a compilacao da mesma especial, gate
    // normal, sem refund disponivel).
    auto provider = provider_for(
        {{"h1", {{0, {CombatAction::use_card("vn_a", h2.id()),
                     CombatAction::use_card("bruno_b", h1.id()),
                     CombatAction::use_card("bruno_b", h1.id())}}}}});
    CombatStateMachine sm({&h1, &h2, &e}, provider, &reg);

    REQUIRE_THROWS_AS(act(sm), std::logic_error);
}

// ===== 8. Determinismo (invariante do manifesto: 0 RNG no eco E no refund) - CountingRandom =====
// =====    prova que NENHUM dos dois caminhos consome sorteio, e que 2 execucoes identicas =====
// =====    produzem HP byte-identico =======================================================

TEST_CASE("techmagic clone: o Eco NAO consome RNG (CountingRandom: ataque basico + eco de "
         "von Neumann num aliado = ZERO next()/next_double())",
         "[domain][combat][techmagic][clone]") {
    CombatActor h1 = make_actor("h1", true, 100, 0, 0, /*spd=*/50);  // conjura von Neumann.
    CombatActor h2 = make_actor("h2", true, 100, 10, 0, /*spd=*/40); // clonado; ataca e.
    CombatActor e = make_actor("e", false, 300, 0, 0, /*spd=*/10);

    auto reg = registry({vonneumann_card("vn_rng", /*magnitude=*/50, /*duration=*/3,
                                        /*with_refund=*/false)});

    auto provider = provider_for(
        {{"h1", {{0, {CombatAction::use_card("vn_rng", h2.id())}}}},
         {"h2", {{0, {CombatAction::attack(e.id())}}}}});
    CountingRandom counting;
    CombatStateMachine sm({&h1, &h2, &e}, provider, &reg, nullptr, &counting);

    act(sm);  // h1 conjura von Neumann em h2 (fogo amigo desligado: dano 0, sem sorteio de
              // canal - o guard damage<=0 nunca sorteia) + Eco aplicado.
    act(sm);  // h2 ataca e por 10 (ATAQUE BASICO = 0 RNG por construcao). Turno de h2 fecha:
              // process_eco_turn_end_hook -> replicate_eco (take_damage PURO, 0 RNG).
    REQUIRE(e.hp() == 300 - 10 - 5);  // eco de fato rodou (prova que 0-RNG nao e no-op).
    REQUIRE(counting.next_calls == 0);
    REQUIRE(counting.next_double_calls == 0);
}

TEST_CASE("techmagic clone: o refund do TokenRefund NAO consome RNG (CountingRandom: cast "
         "que dispara o refund = ZERO next()/next_double()) e e deterministico entre 2 runs",
         "[domain][combat][techmagic][clone]") {
    auto build = [](CountingRandom& rng, int* out_h2_eco_present) {
        auto h1 = std::make_shared<CombatActor>(make_actor("h1", true, 100, 0, 0, 50));
        auto h2 = std::make_shared<CombatActor>(make_actor("h2", true, 100, 0, 0, 45));
        auto e = std::make_shared<CombatActor>(make_actor("e", false, 100, 0, 0, 10));
        h1->set_equipped_special_ids({"vn_self_refund_rng"});  // a PROPRIA carta equipada.

        auto reg = std::make_shared<std::unordered_map<std::string, Card>>(
            registry({vonneumann_card("vn_self_refund_rng", 50, 3, /*with_refund=*/true)}));

        // h1 joga a von Neumann em h2 (aliado): fogo amigo desligado (dano 0, sem sorteio) +
        // refund dispara (0 RNG por construcao) + Eco aplicado em h2.
        auto provider = provider_for(
            {{"h1", {{0, {CombatAction::use_card("vn_self_refund_rng", h2->id())}}}}});
        CombatStateMachine sm({h1.get(), h2.get(), e.get()}, provider, reg.get(), nullptr,
                              &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();

        *out_h2_eco_present = find_status(*h2, StatusId::Eco) != nullptr ? 1 : 0;
    };

    CountingRandom rng1;
    int eco1 = 0;
    build(rng1, &eco1);
    REQUIRE(eco1 == 1);                    // Eco aplicado (o cast rodou de fato).
    REQUIRE(rng1.next_calls == 0);         // refund + fogo-amigo + Eco = 0 sorteio de canal.
    REQUIRE(rng1.next_double_calls == 0);  // idem, 0 variancia.

    // 2a execucao identica: mesma contagem de RNG (determinismo).
    CountingRandom rng2;
    int eco2 = 0;
    build(rng2, &eco2);
    REQUIRE(rng2.next_calls == rng1.next_calls);
    REQUIRE(rng2.next_double_calls == rng1.next_double_calls);
    REQUIRE(eco2 == eco1);
}
