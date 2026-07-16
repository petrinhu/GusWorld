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
#include <unordered_set>
#include <vector>

#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/environment_clock.hpp"
#include "gus/domain/combat/environment_enums.hpp"
#include "gus/domain/combat/environment_modifier.hpp"
#include "gus/domain/combat/initiative_queue.hpp"
#include "gus/domain/combat/random_source.hpp"
#include "gus/domain/combat/techmagic.hpp"

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

// Estimativa PURA de UseCard (COMBATE-TEORIA-JOGOS item [2]; secao 11), devolvida por
// CombatStateMachine::estimate_card_damage. Nao e serializada (query de UI, nao contrato de
// save). Perda de HP PREVISTA por canal, ja com a absorcao de Shield do alvo (mesma tecnica
// de preview_basic_attack_damage).
struct CardDamageEstimate {
    int min_damage = 0;    // piso do canal COMUM (perda de HP, pos-Shield)
    int max_damage = 0;    // teto do canal COMUM (perda de HP, pos-Shield)
    int crit_damage = 0;   // dano do canal CRIT (perda de HP, pos-Shield)
    int fumble_chance_pct = 0;
    int crit_chance_pct = 0;
    float mult_fraqueza = 1.0f;
    // true quando multFraqueza == 0 (curto-circuito de imunidade, secao 11): os 3 danos
    // acima ficam 0 e as chances perdem sentido (o motor tambem nao sorteia canal nesse
    // caso, ver resolve_use_card).
    bool immune = false;

    // ---- Quantum-Lock (Planck, ADR-016 manifesto item 5, A2: perfect information) ----
    // Campos ADITIVOS: default (false/0) preserva TODOS os call sites/testes anteriores
    // intactos. quantized=true SO quando o ATACANTE porta a passiva Planck equipada (mesma
    // regra de resolve_use_card, ver combat_state_machine.cpp::quantize_spec_of) E o alvo
    // nao e imune (o curto-circuito de imunidade acima retorna ANTES destes campos serem
    // tocados). min_damage/max_damage ACIMA continuam sendo piso/teto do canal COMUM de
    // SEMPRE (NAO mudam de semantica - com Planck eles viram tambem os degraus baixo/alto,
    // ja que os 3 degraus SAO a faixa completa da variancia). mid_damage = degrau CENTRAL
    // (r=0.5), pos-Shield, MESMO helper comum_channel_damage dos outros dois.
    bool quantized = false;
    int mid_damage = 0;    // degrau central (r=0.5), perda de HP pos-Shield.
    int step_low_pct = 0;  // chance% do degrau piso (= EffectSpec.percent do Planck).
    int step_mid_pct = 0;  // chance% do degrau centro (= EffectSpec.magnitude do Planck).
    int step_high_pct = 0; // chance% do degrau teto (= EffectSpec.percent, simetrico).

    [[nodiscard]] bool operator==(const CardDamageEstimate&) const = default;
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

    // Ledger de hits DA RODADA CORRENTE (ADR-016 secao 20 item 4, ledger cross-ator):
    // observabilidade de teste do HypotenuseCombo/OnRoundEnd. Populado em
    // apply_damage_with_hooks (cada hit com damage > 0), limpo no fim de
    // process_round_end_hooks (fronteira de rodada, ver advance_to_next_actor). Vazio fora
    // do meio de uma rodada em curso ja processada (a limpeza acontece logo apos o
    // despacho do hook, entao o ledger nunca "vaza" pra rodada seguinte).
    [[nodiscard]] const std::vector<techMagic::RoundHitEntry>& round_hits() const noexcept {
        return round_hits_;
    }

    // Registro da ULTIMA ACAO DE DANO de QUALQUER aliado NESTA RODADA (ADR-016 secao 20
    // item 5, RepeatLastAction/Mandelbrot+Ada): observabilidade de teste. Populado ao FIM
    // de resolve_basic_attack/resolve_use_card quando ha pelo menos 1 hit>0, limpo em
    // process_round_end_hooks junto do round_hits_ (Q4 - a janela nao atravessa rodada).
    // actor == nullptr antes do 1o hit de dano da batalha/rodada.
    [[nodiscard]] const techMagic::LastActionRecord& last_action() const noexcept {
        return last_action_;
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

    // Estimativa PURA de UseCard de `card` (attacker -> target) ANTES de confirmar
    // (COMBATE-TEORIA-JOGOS item [2]; Pillar 4: dano esperado x custo visivel antes de
    // comprometer a acao). Espelha, SEM efeito colateral, a cadeia divisiva + as chances de
    // canal de resolve_use_card (secao 11): NAO chama rng_->next()/next_double() (o sorteio
    // de canal e a variancia SO acontecem na resolucao real - contador de RNG intocado),
    // NAO aplica dano/status/mana/AP, NAO consome Disrupt do atacante (so LE) - so LE atk/
    // def/family/status_effects/knowledge_kills. `mult_combo` e do CALLER (a UI, se ja sabe
    // que o pipeline fecharia um combo); default 1.0 (a estimativa NAO resolve pipeline
    // sozinha, isso e ComboTable::match, fora do escopo do preview de uma carta). Reusa os
    // MESMOS helpers de canal (comum_channel_damage/crit_channel_damage) e a MESMA absorcao
    // de Shield (shield_absorbed_loss) que resolve_use_card/preview_basic_attack_damage -
    // nao ha formula paralela pra divergir.
    [[nodiscard]] CardDamageEstimate estimate_card_damage(const CombatActor& attacker,
                                                          const CombatActor& target,
                                                          const Card& card,
                                                          float mult_combo = 1.0f) const noexcept;

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

    // Aplica um status OFENSIVO (card.status_applied OU combo->result_status) via
    // CombatActor::try_add_status e loga o resultado REAL (ADR-016 Balde B, Faraday/
    // EM-Shield): choke point unico dos 4 sitios de status ofensivo de resolve_use_card
    // (curto-circuito de imunidade + resolucao normal, x status_applied + result_status) -
    // sem isso, o portao de imunidade bloquearia so a carta especial (Faraday/handle_
    // apply_status), mas um Stun de carta COMUM eletrica ou um combo passaria batido
    // direto por CombatActor::add_status. NAO usado pelos sitios de BUFF/defesa (Shield do
    // Defend etc - esses continuam com add_status legado, nunca sao bloqueados).
    void apply_offensive_status(CombatActor& actor, CombatActor& target,
                                const StatusEffect& status);

    // Aplica `damage` em `target` (take_damage) e despacha os hooks OnDamageDealt/
    // OnDamageReceived do executor techMagic (ADR-016 secao 20 item 3): fontes de
    // OnDamageDealt = `source_card` (se houver, a carta jogada por `attacker`) + as
    // especiais EQUIPADAS de `attacker`; fontes de OnDamageReceived = as especiais
    // EQUIPADAS de `target`. HELPER COMPARTILHADO entre resolve_use_card e
    // resolve_basic_attack (anti-gemeo obrigatorio: os DOIS sitios de dano fiam aqui, NAO
    // ha copy-paste da logica de hook). No-op de hooks se damage <= 0 (canal FALHA/
    // imunidade nao dispara). source_card == nullptr no ataque basico (nao ha carta).
    void apply_damage_with_hooks(CombatActor& attacker, CombatActor& target, int damage,
                                 const Card* source_card);

    // Despacha OnRoundEnd (ADR-016 secao 20 item 4, HypotenuseCombo) nas especiais
    // EQUIPADAS de cada ator VIVO da fila, com ctx.round_hits apontando pro ledger da
    // rodada que acabou de fechar (round_hits_) e um set de dedup (bonused_targets)
    // compartilhado por TODA a chamada (1x/alvo/rodada mesmo com >1 dono da passiva).
    // Chamado SO na fronteira da rodada (wrap de advance_to_next_actor), ANTES de
    // regroup_round_by_side - a ordem de iteracao e queue_.order() (a ordem QUE FECHOU a
    // rodada, antes do regroup da PROXIMA). Limpa round_hits_ ao final SEMPRE (hits nao
    // atravessam fronteira de rodada, mesmo se nenhuma especial disparar).
    void process_round_end_hooks();

    // Despacha OnAllyTurnEnd (ADR-016 secao 20 item 5, RepeatLastAction/Ada) nas
    // especiais EQUIPADAS dos aliados VIVOS do MESMO lado de `ended`, EXCLUINDO o proprio
    // `ended` (a passiva reage a um ALIADO fechando o turno, nunca ao proprio turno de
    // quem a possui - decisao do lider 2026-07-14). Gemeo de process_round_end_hooks, mas
    // por-TURNO (nao por-rodada) e sem ledger/dedup (RepeatLastAction nao usa round_hits
    // nem bonused_targets). Chamado nos DOIS caminhos de fim-de-turno
    // (run_active_turn_to_end e expire_on_stunned_turn_end).
    void process_ally_turn_end_hooks(CombatActor& ended);

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

    // Reagrupa a fila por LADO no INICIO da rodada (§4.1): o lado que ABRE (round_opening_
    // side, por SPD corrente) vai inteiro pra frente, o outro atras, preservando a ordem
    // relativa DENTRO de cada lado (delega a InitiativeQueue::regroup_stable => stable_
    // partition, NAO sort => Gambito-safe). Chamado SO na fronteira da rodada: construcao
    // (rodada 0) e wrap de advance_to_next_actor (rodadas 1+). NAO consome RNG.
    void regroup_round_by_side();

    // Registries (nao-donos) + RNG injetado (nao-dono). nullptr tratado no construtor.
    CombatActionProvider action_provider_;
    const std::unordered_map<std::string, Card>* card_registry_;
    const std::unordered_map<std::string, IEnemyBrain*>* brain_registry_;
    IRandomSource* rng_;

    InitiativeQueue queue_;
    CombatPhase phase_ = CombatPhase::SetupPhase;
    CombatOutcome outcome_ = CombatOutcome::Ongoing;

    // GUS-CENTRIC (canon, decisao do lider, M7-COSTURA BUG-4): ponteiro NAO-DONO pro ator
    // com is_player_side() && is_universal_compiler()==true (o Gus), cacheado no
    // construtor (SetupPhase, antes de qualquer prune). check_end() consulta ISTO (nao a
    // fila, ja podada) pra saber se o Gus caiu, mesmo depois de prune_dead() te-lo
    // removido de queue_ (prune so tira da FILA de iniciativa - o objeto CombatActor
    // continua vivo no escopo do dono, o ponteiro segue valido). nullptr se nenhum ator
    // tem a flag (FSMs headless/testes que nunca a setam preservam o wipe-total legado).
    CombatActor* gus_actor_ = nullptr;

    // Janela de Comando da Party (§4.1): membro escolhido pelo host para o PROXIMO
    // begin_turn (comando livre). nullptr = sem escolha => default (queue_.current()).
    // One-shot: begin_turn consome e zera. Ponteiro NAO-DONO (ator vive no escopo do dono).
    CombatActor* selected_next_party_ = nullptr;

    // Regra 1x/batalha das especiais ATIVA/HIBRIDA (executor techMagic, ADR-016 secao
    // 20): Card.Id ja jogados nesta batalha (escopo = vida desta CombatStateMachine).
    // Reusa a SEMANTICA "nao recarrega na batalha" da Analise Preditiva (secao 2.1) - NAO
    // e o mesmo flag (a Analise Preditiva ainda nao tem campo no dado da carta). Comuns e
    // Passiva/ForaDeCombate NUNCA entram aqui (isentas, ver resolve_use_card).
    std::unordered_set<std::string> specials_cast_;

    // Ledger de hits DA RODADA CORRENTE (ADR-016 secao 20 item 4, ledger cross-ator):
    // acumulado em apply_damage_with_hooks (DEPOIS do guard damage<=0), consultado e
    // limpo em process_round_end_hooks na fronteira da rodada. Ver round_hits() acima.
    std::vector<techMagic::RoundHitEntry> round_hits_;

    // Registro da ULTIMA ACAO DE DANO de QUALQUER aliado NESTA RODADA (ADR-016 secao 20
    // item 5, RepeatLastAction/Mandelbrot+Ada): atualizado ao FIM de resolve_basic_attack/
    // resolve_use_card (so quando ha hit>0, ver last_action() acima), limpo em
    // process_round_end_hooks junto do round_hits_.
    techMagic::LastActionRecord last_action_;

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
