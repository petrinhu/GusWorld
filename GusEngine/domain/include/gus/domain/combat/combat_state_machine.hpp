// gus/domain/combat/combat_state_machine.hpp
//
// FSM por-ator do combate turn-based (secao 3). POCO 100% testavel sem runtime Godot.
// Portado de engine/foundation/turn_combat/CombatStateMachine.cs. POCO puro, ZERO Qt,
// ZERO I/O (invariante de domain/, engine-design.md secao 2). Paridade 1:1 com o C#.
//
// Fluxo (secao 3):
//   SetupPhase -> TurnStart -> (ActionSelect <-> ActionResolve)* -> TurnEnd -> CheckEnd
//     |- nao-fim -> proximo ator -> TurnStart ...
//     +- fim     -> CombatEnd
//
// A selecao de acao e injetada via std::function (CombatActionProvider) pra manter a FSM
// deterministica e testavel. Em jogo (incremento 4) o CombatManager (Node AutoLoad)
// fornece o provider: input do jogador pra party, IEnemyBrain pra inimigos.
//
// OWNERSHIP (porte fiel do C#): o C# guarda referencias a CombatActor (class). No C++ a
// FSM/fila guardam ponteiros NAO-DONOS (CombatActor*): os atores vivem no escopo do dono
// (CombatManager / teste). Mesmo padrao ja usado em InitiativeQueue/CombatState.
//
// MAPEAMENTO de tipos C# -> C++:
//   delegate CombatActionProvider              -> std::function<CombatAction(CombatActor&,
//                                                 const CombatState&)>
//   IReadOnlyDictionary<string,Card>           -> const std::unordered_map<string,Card>*
//                                                 (nullptr => registry vazio)
//   IReadOnlyDictionary<string,IEnemyBrain>    -> const std::unordered_map<string,
//                                                 IEnemyBrain*>* (nullptr => vazio)
//   Random rng                                 -> IRandomSource* (porta injetavel, secao 11)
//   IntentPreview? LastPrediction              -> std::optional<IntentPreview>
//
// PORTA DE RNG (secao 11): a FSM recebe IRandomSource* por injecao (NUNCA RNG global no
// dominio). Default C# = Random.Shared (global, nao-deterministico); como o dominio e
// puro/deterministico, quando NENHUM rng e injetado usamos um default DETERMINISTICO
// (next_double 0.5 = variancia zero, next 0 = sem crit). Os testes que aferem dano exato
// SEMPRE injetam um FixedRandom; os que so afere mana/efeito omitem rng e dependem desse
// default neutro. A semente real (data+hora+ms, ADR-006 item 5) e injetada na fronteira
// app/ DEPOIS, NAO aqui.
//
// Cross-ref: engine/foundation/turn_combat/CombatStateMachine.cs;
//            docs/design/mecanicas/combat.md secao 3/4/5/7/8/9/10/11/12/13/14/16/18; ADR-006.

#ifndef GUS_DOMAIN_COMBAT_COMBAT_STATE_MACHINE_HPP
#define GUS_DOMAIN_COMBAT_COMBAT_STATE_MACHINE_HPP

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/environment_clock.hpp"
#include "gus/domain/combat/environment_enums.hpp"
#include "gus/domain/combat/environment_modifier.hpp"
#include "gus/domain/combat/initiative_queue.hpp"
#include "gus/domain/combat/random_source.hpp"

namespace gus::domain::combat {

class CombatActor;
class CombatState;
class IEnemyBrain;

// Delegate de selecao de acao (secao 3). Recebe o ator ativo + estado read-only, devolve
// a acao. O jogador (UI) ou a AI (IEnemyBrain) implementam isto no incremento 4.
using CombatActionProvider =
    std::function<CombatAction(CombatActor& actor, const CombatState& state)>;

// Lado do combate (Janela de Comando da Party, §4.1). Nao e serializado (nao entra em
// combat_enums.hpp, que e o contrato binario do save); e so um tipo de retorno de query
// para o host/UI decidir quem abre a rodada. Party = bloco da party; Enemy = bloco inimigo.
enum class CombatSide : std::uint32_t {
    Party = 0,
    Enemy = 1,
};

// Maquina de estados do combate (secao 3).
class CombatStateMachine {
public:
    // Monta o combate (SetupPhase): ordena a fila por SPD, fica em TurnStart pronto.
    //   actors          : party + inimigos (1..4 por lado, secao 2). Ponteiros NAO-DONOS.
    //   action_provider : fornecedor de acoes por ator (UI/AI). Obrigatorio.
    //   card_registry   : banco de cartas (id->carta). nullptr => vazio (sem deck).
    //   brain_registry  : AI inimiga por id (id->IEnemyBrain*). nullptr => vazio.
    //   rng             : porta de aleatoriedade (secao 11). nullptr => default neutro
    //                     deterministico (variancia zero, sem crit).
    CombatStateMachine(
        std::vector<CombatActor*> actors,
        CombatActionProvider action_provider,
        const std::unordered_map<std::string, Card>* card_registry = nullptr,
        const std::unordered_map<std::string, IEnemyBrain*>* brain_registry = nullptr,
        IRandomSource* rng = nullptr);

    // ---- Estado observavel ----

    // Fila de iniciativa visivel (secao 4).
    [[nodiscard]] const InitiativeQueue& queue() const noexcept { return queue_; }
    [[nodiscard]] InitiativeQueue& queue() noexcept { return queue_; }

    // Fase atual da FSM.
    [[nodiscard]] CombatPhase phase() const noexcept { return phase_; }

    // Resultado corrente (Ongoing ate CheckEnd detectar fim).
    [[nodiscard]] CombatOutcome outcome() const noexcept { return outcome_; }

    // Ator do turno corrente (atalho pra queue().current()).
    [[nodiscard]] CombatActor* active_actor() const noexcept { return queue_.current(); }

    // Log de combate acumulado (auditoria + UI feed). secao 16.
    [[nodiscard]] const std::vector<CombatLogEntry>& log() const noexcept { return log_; }

    // Mudancas de status acumuladas (Applied/Expired/Absorbed), secao 16.
    [[nodiscard]] const std::vector<StatusEffectChange>& status_changes() const noexcept {
        return status_changes_;
    }

    // Ultimo IntentPreview lido por Gambito-Prever (secao 12). nullopt antes do 1o uso.
    [[nodiscard]] const std::optional<IntentPreview>& last_prediction() const noexcept {
        return last_prediction_;
    }

    // ---- Janela de Comando da Party (comando livre sobre o CTB, modelo 1B, §4.1) ----
    //
    // Extensao ADITIVA: a FORMA da FSM nao muda. Muda a SELECAO de ator quando e a vez do
    // bloco da party (o jogador comanda QUAL membro age e em que ordem). A SPD continua
    // decidindo (a) qual lado abre a rodada e (b) o pre-selecionado. Nada disto consome RNG
    // (§11): selecao de ator e input de jogador/host, nao sorteio. Se NENHUM select_party_
    // actor for chamado, begin_turn opera no queue_.current() de sempre => motor pre-1B
    // (forcar o topo = caso do conjunto elegivel ter 1 elemento; testes antigos intactos).

    // Lado que ABRE a rodada, por comparacao de SPD entre os lados (maior SPD alive de
    // cada lado). Empate favorece a Party (regra documentada; §4.1 nao fixa o desempate).
    // Query pura sobre os atores vivos; reflete a SPD corrente (recomputa entre rodadas).
    [[nodiscard]] CombatSide round_opening_side() const;

    // Membros da party ELEGIVEIS nesta rodada (vivos, player-side, ainda-nao-agiram),
    // ordenados por SPD desc (front = pre-selecionado). Query pura, derivada da fila
    // (slots >= cursor). Vazio quando o bloco da party ja se esvaziou nesta rodada.
    [[nodiscard]] std::vector<CombatActor*> pending_party_actors() const;

    // Pre-selecionado ao abrir a Janela = o de maior SPD entre os elegiveis (front de
    // pending_party_actors), ou nullptr se nao ha party pendente. Sugestao, nao trava.
    [[nodiscard]] CombatActor* preselected_party_actor() const;

    // Escolhe QUAL membro pendente da party entra no proximo begin_turn (comando livre do
    // jogador/host, chamado ANTES de begin_turn). O begin_turn consome a escolha trazendo
    // o ator para o slot do cursor. Lanca std::invalid_argument se `actor` nao e um membro
    // elegivel da party nesta rodada (consulte pending_party_actors()). Sem chamada, o
    // default (queue_.current()) vale.
    void select_party_actor(CombatActor* actor);

    // ---- Ambiente de combate (secao 18) ----

    // Camadas de ambiente ATIVAS (terreno + clima + periodo), secao 18.
    [[nodiscard]] const std::vector<EnvironmentModifier>& active_environments() const noexcept {
        return active_environments_;
    }

    // Relogio deterministico do periodo (secao 18.3). nullptr se o encontro nao roda periodo.
    [[nodiscard]] const EnvironmentClock* period_clock() const noexcept {
        return period_clock_ ? &*period_clock_ : nullptr;
    }

    // true apos um Scan-ambiente (1 AP) neste encontro (secao 18.5).
    [[nodiscard]] bool environment_scanned() const noexcept { return environment_scanned_; }

    // Trocas de ambiente acumuladas (secao 16/18.10).
    [[nodiscard]] const std::vector<EnvironmentId>& environment_changes() const noexcept {
        return environment_changes_;
    }

    // Marca o conjunto de ambiente do encontro (secao 18.10). Substitui as camadas ativas.
    // Ids None sao ignorados. Se houver fase de periodo, instancia o EnvironmentClock.
    void set_environment(const std::vector<EnvironmentId>& ids);

    // mult_ambiente da familia dada (secao 11): produto das camadas ativas que a afetam,
    // capeado em [0.44, 2.25]. Inclui a fase de periodo corrente do relogio. Sem ambiente
    // => 1.0 (combate inalterado).
    [[nodiscard]] float mult_ambiente_for(CardFamily family) const;

    // ---- Preview read-only pra UI (nao muta NADA) ----

    // Perda de HP PREVISTA de um ataque basico de attacker contra target, exibida no
    // modo-mira (battle-screen.md §3.5) ANTES de confirmar. Espelha, sem efeito colateral,
    // resolve_basic_attack (dano bruto = max(kMinDamage, atk - def)) seguido da absorcao de
    // Shield de CombatActor::absorb_with_shield (secao 9): devolve max(0, dano_bruto -
    // magnitude_do_Shield_ativo), com piso 0. PURO e const: NAO aplica dano, NAO consome AP,
    // NAO registra log/status - so LE atk/def/status_effects. Se a formula real (resolve_
    // basic_attack) ou a absorcao (absorb_with_shield) mudarem, ESPELHE aqui (as duas ficam
    // adjacentes no .cpp de proposito).
    [[nodiscard]] int preview_basic_attack_damage(const CombatActor& attacker,
                                                  const CombatActor& target) const noexcept;

    // ---- Conducao da FSM ----

    // TurnStart do ator corrente (secao 3/5): recarrega mana/AP, aplica tick de status.
    // Transiciona pra ActionSelect. Retorna true se o ator esta Stun e PERDE o turno.
    bool begin_turn();

    // Loop interno ActionSelect <-> ActionResolve (secao 3): consome AP ate 0 ou Pass.
    // Termina em TurnEnd. Lanca std::logic_error se nao estiver em ActionSelect.
    void run_active_turn_to_end();

    // CheckEnd (secao 3): avalia as 3 condicoes de termino e seta outcome. Retorna true
    // se o combate terminou.
    bool check_end();

    // TurnEnd -> proximo ator (secao 3). Remove mortos da fila e avanca o ponteiro.
    void advance_to_next_actor();

    // Roda o combate completo ate CombatEnd e devolve o CombatResult (secao 3/16).
    CombatResult run_until_end();

private:
    // ---- ambiente interno ----
    [[nodiscard]] std::vector<EnvironmentModifier> effective_environments() const;
    void advance_period_clock();

    // ---- turno/tick ----
    void drain_status_changes();
    bool apply_status_tick(CombatActor& actor);
    void expire_on_stunned_turn_end(CombatActor& actor);
    void prune_dead();

    // ---- resolucao de acoes ----
    void resolve_action(CombatActor& actor, const CombatAction& action,
                        const CombatState& state);
    void resolve_basic_attack(CombatActor& attacker, const CombatAction& action);
    void resolve_defend(CombatActor& actor);
    void resolve_scan(CombatActor& actor, const CombatAction& action);
    void resolve_scan_environment(CombatActor& actor);
    void resolve_gambit_predict(CombatActor& actor, const CombatAction& action);
    void resolve_gambit_reorder(CombatActor& actor, const CombatAction& action);
    void resolve_use_card(CombatActor& actor, const CombatAction& action,
                          const CombatState& state);
    void resolve_flee(CombatActor& actor);

    [[nodiscard]] CombatActor* resolve_primary_target(CombatActor& actor,
                                                      const CombatAction& action,
                                                      const Card& card);
    [[nodiscard]] std::vector<CombatActor*> resolve_targets(CombatActor& actor,
                                                            const CombatAction& action,
                                                            const Card& card);

    [[nodiscard]] static bool dispel_buffs(CombatActor& actor);

    // ---- Janela de Comando da Party (§4.1) ----
    // true se `actor` e um membro elegivel da party nesta rodada (esta em
    // pending_party_actors): vivo, player-side, ainda-nao-agiu. nullptr => false.
    [[nodiscard]] bool is_pending_party_actor(const CombatActor* actor) const;

    // Registries (nao-donos) + RNG injetado (nao-dono). nullptr tratado no construtor.
    CombatActionProvider action_provider_;
    const std::unordered_map<std::string, Card>* card_registry_;
    const std::unordered_map<std::string, IEnemyBrain*>* brain_registry_;
    IRandomSource* rng_;

    InitiativeQueue queue_;
    CombatPhase phase_ = CombatPhase::SetupPhase;
    CombatOutcome outcome_ = CombatOutcome::Ongoing;

    // Janela de Comando da Party (§4.1): membro escolhido pelo host para o PROXIMO
    // begin_turn (comando livre). nullptr = sem escolha => default (queue_.current()).
    // One-shot: begin_turn consome e zera. Ponteiro NAO-DONO (ator vive no escopo do dono).
    CombatActor* selected_next_party_ = nullptr;

    std::vector<CombatLogEntry> log_;
    std::vector<StatusEffectChange> status_changes_;
    std::optional<IntentPreview> last_prediction_;

    std::vector<EnvironmentModifier> active_environments_;
    std::optional<EnvironmentClock> period_clock_;
    bool environment_scanned_ = false;
    std::vector<EnvironmentId> environment_changes_;
};

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_COMBAT_STATE_MACHINE_HPP
