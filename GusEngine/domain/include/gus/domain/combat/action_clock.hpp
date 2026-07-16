// gus/domain/combat/action_clock.hpp
//
// PROVA DE CONCEITO ISOLADA do ADR-017 (Action-Clock unificado, estilo Pokemon Legends:
// Arceus). ZERO acoplamento a CombatActor/InitiativeQueue/CombatStateMachine de producao
// - modulo autonomo, novo, POCO puro (ZERO Qt, ZERO SDL, ZERO RNG: o clock e 100%
// deterministico). O cutover real (religar isto na FSM de combate) e onda dedicada
// pos-M7 ("ARCEUS-CLOCK", ver ADR-017 "Plano"); ate la este arquivo nao e chamado por
// nenhum codigo de producao.
//
// Escopo desta prova: escalonador por ticks inteiros (min por next_action_at, desempate
// D3), formula de reset pos-acao (BASE_CLOCK/SPD x cardSpeedMult x styleMult), os 3
// estilos (Normal/Agil/Forte) com decaimento suave do Agil consecutivo, o
// starvation-guard (D1 hibrido, protege a party) e a PendingResolution (agendamento +
// cancelamento da carta interpretada). Fonte da verdade: ADR-017.
//
// ActorId: handle leve (nao um ponteiro pra CombatActor) - o objetivo aqui e provar a
// MECANICA isolada; a fiacao real com CombatActor/SPD de verdade acontece no cutover.
//
// Cross-ref: docs/tech/adr/ADR-017-action-clock-combat-unificado.md;
//            docs/design/mecanicas/pedido-arceus-battle-engine.md (Casos 1-6);
//            docs/design/mecanicas/combat-flavor.md secao 1-2 (cardSpeedMult).

#ifndef GUS_DOMAIN_COMBAT_ACTION_CLOCK_HPP
#define GUS_DOMAIN_COMBAT_ACTION_CLOCK_HPP

#include <cstdint>
#include <cstddef>
#include <vector>

namespace gus::domain::combat {

// Handle leve de ator (party slot / enemy slot / o que o caller quiser). Este modulo
// isolado nao conhece CombatActor; o cutover real mapeia ActorId -> CombatActor*.
using ActorId = std::uint32_t;

// Os 3 estilos de acao por-cast (ADR-017, pedido do Gus). Aplicam a QUALQUER acao
// (ataque basico ou carta); a alavanca de dano fica fora deste modulo (o executor de
// dano/carta consome styleMult via a cadeia divisiva, secao 11 combat.md) - aqui so
// provamos o efeito no TIMING.
enum class CastStyle : std::uint32_t {
    Normal = 0,
    Agil = 1,
    Forte = 2,
};

// Classe de velocidade da carta (compilada/interpretada/basico), ja canonica em
// combat-flavor.md secao 1-2. O ADR-017 unifica isto como o `cardSpeedMult` da formula
// de reset.
enum class CardSpeedClass : std::uint32_t {
    Compilada = 0,     // rapida (C-Arcane/Oxido/Asmodico)
    Interpretada = 1,  // lenta (Pythia/DRE-GRE)
    Basico = 2,        // ataque basico, sem carta
};

// Constantes //PLAYTEST (ADR-017 "Numeros baseline" - parecer do lead-game-designer,
// N=3 afina). Nomeadas, nao magic numbers. Ver combat_constants.hpp para o padrao k+
// PascalCase do domain/.
namespace action_clock_constants {

// Constante de calibracao da formula de reset (ADR-017 arquitetura).
inline constexpr int kBaseClock = 1000;

// cardSpeedMult (ADR-017 numeros baseline; combat-flavor.md secao 1-2).
inline constexpr float kCardSpeedCompilada = 0.85f;
inline constexpr float kCardSpeedInterpretada = 1.35f;
inline constexpr float kCardSpeedBasico = 1.00f;

// styleMult - fator de TEMPO (ADR-017 numeros baseline). O fator de DANO (Agil -50%
// fixo, Forte +50%) e responsabilidade do executor de dano (fora de escopo aqui; ver
// kStyleDamageAgil/kStyleDamageForte so como referencia documental).
inline constexpr float kStyleTimeNormal = 1.00f;
inline constexpr float kStyleTimeForte = 1.60f;
inline constexpr float kStyleDamageNormal = 1.00f;
inline constexpr float kStyleDamageForte = 1.50f;
inline constexpr float kStyleDamageAgil = 0.50f;  // fixo, nao decai (so o TEMPO decai)

// Decaimento suave do Agil CONSECUTIVO (ADR-017 D3): 1a x0.55, 2a x0.75, 3a+ x0.90.
// O contador (agile_streak) reseta ao usar Normal/Forte (ver ActionClockEntry).
inline constexpr float kStyleTimeAgil1st = 0.55f;
inline constexpr float kStyleTimeAgil2nd = 0.75f;
inline constexpr float kStyleTimeAgil3rdPlus = 0.90f;

// Starvation-guard (D1 hibrido, protege a party - ADR-017 D1). O ADR NAO fixa um
// numero baseline pra isto (fica como pendencia explicita "vigiar no N=3"); o valor
// aqui e um placeholder de TESTE/prova de conceito, ajustavel livremente sem tocar a
// formula central. NAO e uma decisao de balance - e so o suficiente pra exercitar o
// mecanismo do guard nos testes.
inline constexpr int kMaxStarveTicksDefault = 250;

}  // namespace action_clock_constants

// Uma resolucao agendada (carta interpretada, ADR-017 "Cast interpretado"): entra AGORA
// (o caller ja pagou o custo/AP), mas resolve num tick futuro `resolve_at`. Cancelavel
// ate la (Stun/Disrupt/Silence/dano no alvo - RUNTIME ERROR, ja canon
// combat-flavor.md). Este modulo isolado NAO executa efeito real: so prova que a
// entrada e agendada no tick certo, dispara quando devido, e e cancelavel antes.
struct PendingResolution {
    int id = -1;
    ActorId target = 0;
    int resolve_at = 0;
    bool cancelled = false;
    bool fired = false;
};

// Entrada do escalonador por-ator. Estende o esboco minimo do pedido (actor_id/
// next_action_at/last_acted_at) com os campos que o desempate D3 e o Agil consecutivo
// EXIGEM (SPD e preferencia pro desempate; agile_streak pro decaimento; is_protected
// pro starvation-guard D1). Nao acopla a CombatActor: spd e um snapshot que o CALLER
// fornece/atualiza (o cutover real leria de CombatActor::spd() com Haste/Slow
// aplicados; fora de escopo aqui).
struct ActionClockEntry {
    ActorId actor_id = 0;
    int next_action_at = 0;
    int last_acted_at = -1;

    // Extensoes alem do esboco minimo do pedido, necessarias pro desempate D3 e pro
    // decaimento/starvation-guard (documentado, nao uma mudanca de design silenciosa):
    int spd = 1;
    int preference_order = 0;   // ordem de insercao = "preferencia do jogador" (D3)
    bool is_protected = false;  // starvation-guard (D1 hibrido) - marca membros da party
    int agile_streak = 0;       // decaimento suave do Agil consecutivo (ADR-017 D3)
};

// Escalonador por-ator (ADR-017). Ticks INTEIROS (round/lround, sem float persistente -
// so a formula de reset usa float NA HORA, o estado guardado e sempre int). Determinista
// por construcao: ZERO RNG neste modulo.
class ActionClock {
public:
    explicit ActionClock(int max_starve_ticks = action_clock_constants::kMaxStarveTicksDefault)
        : max_starve_ticks_(max_starve_ticks) {}

    // Registra um ator no clock. `initial_next_action_at` e o tick em que a PRIMEIRA
    // acao dele fica disponivel (tipicamente round(BASE_CLOCK/SPD), mesma formula da
    // primeira janela - o caller decide). `preference_order` desempata em igualdade
    // total (D3): menor valor vence. `is_protected` liga o starvation-guard (D1).
    // `initial_last_acted_at` existe pra testes de desempate isolarem a dimensao
    // last_acted_at sem precisar simular acoes reais primeiro (default -1 = "nunca
    // agiu").
    void add_actor(ActorId id, int initial_next_action_at, int spd, int preference_order,
                   bool is_protected = false, int initial_last_acted_at = -1);

    // Numero de atores registrados.
    [[nodiscard]] std::size_t actor_count() const noexcept { return entries_.size(); }

    // Snapshot read-only de uma entrada (por indice de insercao, 0-based). Uso em teste/
    // inspecao; nao e a ordem de acao (essa e decidida a cada advance()).
    [[nodiscard]] const ActionClockEntry& entry_at(std::size_t index) const {
        return entries_.at(index);
    }

    // Snapshot read-only por actor_id. Lanca std::invalid_argument se ausente.
    [[nodiscard]] const ActionClockEntry& entry_of(ActorId id) const;

    // Tick "agora" do clock: o next_action_at do ULTIMO vencedor de advance() (0 antes da
    // primeira chamada).
    [[nodiscard]] int global_tick() const noexcept { return global_tick_; }

    // Aplica o starvation-guard (D1 hibrido) e devolve o ActorId do proximo a agir,
    // avancando global_tick() pro next_action_at dele. Desempate (D3): next_action_at
    // asc -> last_acted_at asc (quem nao agiu ha mais tempo) -> SPD desc -> preferencia
    // (preference_order asc). Lanca std::logic_error se nao ha atores.
    [[nodiscard]] ActorId advance();

    // Reseta o next_action_at do ator que ACABOU de agir, seguindo a formula do
    // ADR-017:
    //   next_action_at = global_tick() + round(BASE_CLOCK/spd x cardSpeedMult x
    //                                          styleMult(style, streak_pos_uso))
    // Atualiza last_acted_at = global_tick() e o agile_streak (incrementa em Agil,
    // reseta em Normal/Forte - "o contador por-ator reseta ao usar Normal/Forte").
    // `spd` e passado explicitamente (nao lido de volta da entry) pra permitir
    // Haste/Slow no cutover real sem mudar a assinatura. Lanca std::invalid_argument se
    // `id` nao esta registrado.
    void reset_after_action(ActorId id, int spd, CardSpeedClass card_speed,
                             CastStyle style);

    // Overload de conveniencia com cardSpeedMult ja resolvido (teste/uso direto sem
    // depender do enum, ex.: ataque basico custom).
    void reset_after_action(ActorId id, int spd, float card_speed_mult, CastStyle style);

    // ---- PendingResolution (cast interpretado) ----------------------------------

    // Agenda uma resolucao one-shot em `resolve_at` (tick futuro do MESMO clock).
    // Retorna o id da pendencia (estavel, usado por cancel_pending/is_pending_*).
    int schedule_pending(ActorId target, int resolve_at);

    // Cancela TODAS as pendencias nao disparadas/nao canceladas de `target` (a
    // interrupcao de Stun/Disrupt/Silence/dano do combat-flavor.md secao 1). No-op se
    // nao ha pendencia ativa pra esse alvo.
    void cancel_pending(ActorId target);

    // Devolve os ids das pendencias que se tornaram devidas (resolve_at <=
    // global_tick(), nao canceladas, ainda nao disparadas) e as marca fired=true. Uso
    // tipico: chamar apos cada advance() pra ver o que "estoura" neste tick.
    std::vector<int> collect_due_pending();

    [[nodiscard]] bool is_pending_cancelled(int pending_id) const;
    [[nodiscard]] bool is_pending_fired(int pending_id) const;
    [[nodiscard]] std::size_t pending_count() const noexcept { return pending_.size(); }

    // Resolve cardSpeedMult a partir da classe (ADR-017 numeros baseline). Publico pra
    // uso em teste/preview sem duplicar a tabela.
    [[nodiscard]] static float card_speed_mult(CardSpeedClass card_speed) noexcept;

private:
    // Aplica o catch-up bump (D1 hibrido): nenhuma entry protegida pode ter
    // next_action_at > last_acted_at + max_starve_ticks_. Roda ANTES de cada advance()
    // decidir o vencedor - garante a invariante "protegido nunca fica mais que
    // MAX_STARVE_TICKS sem agir" por construcao (nao depende do caller lembrar de
    // chamar nada).
    void apply_starvation_guard();

    // true se `a` deveria agir ANTES de `b` (desempate D3 completo).
    static bool acts_before(const ActionClockEntry& a, const ActionClockEntry& b) noexcept;

    ActionClockEntry& mutable_entry_of(ActorId id);

    std::vector<ActionClockEntry> entries_;
    std::vector<PendingResolution> pending_;
    int global_tick_ = 0;
    int max_starve_ticks_;
    int next_pending_id_ = 0;
};

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_ACTION_CLOCK_HPP
