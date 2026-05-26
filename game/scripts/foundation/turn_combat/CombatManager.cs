// CombatManager.cs
//
// Ponte entre o POCO CombatStateMachine (engine, GusDragon.Engine.Foundation.TurnCombat)
// e o grafo de Nodes Godot. AutoLoad "CombatManager".
//
// Responsabilidade: avançar a FSM turno-a-turno e traduzir cada transição em signals
// de CombatBus (§16). Inimigos resolvem automaticamente via IEnemyBrain; turnos da
// party aguardam SubmitPlayerAction antes de avançar.
//
// Fora de escopo no slice: render/animação, fila de input de UI, concessão real de
// XP/mestria/loot (apenas sinalizada via PlayerBus.CombatResultReceived).
//
// Cross-ref: docs/design/mecanicas/combat.md §16 (event bus), §17 (escopo do slice).

using Godot;
using GusDragon.Engine.Foundation.TurnCombat;
using GusWorld.Game.Foundation.Buses;
using System.Collections.Generic;
using System.Linq;

namespace GusWorld.Game.Foundation.TurnCombat;

/// <summary>
/// AutoLoad que orquestra um combate ativo sobre a FSM POCO e emite os signals do
/// CombatBus/PlayerBus (§16). Um combate por vez no slice.
/// </summary>
public partial class CombatManager : Node
{
    public static CombatManager? Instance { get; private set; }

    // Estado do combate ativo.
    private CombatStateMachine? _fsm;
    private bool _combatActive;
    private bool _waitingForPlayerAction;
    private CombatAction? _pendingPlayerAction;

    // Atores registrados em StartCombat (para mapear HP 0 -> defeated/incapacitated).
    private readonly List<CombatActor> _actors = new();

    // Atores já reportados como derrotados/incapacitados (evita re-emitir o signal).
    private readonly HashSet<string> _reportedDown = new();

    // Ponteiro no log da FSM: a partir de onde ainda não emitimos ActionResolved.
    private int _lastLogIndex;

    // Ponteiro no buffer de mudanças de status da FSM: a partir de onde ainda não
    // emitimos StatusApplied/StatusExpired (§16). Mesmo padrão do _lastLogIndex.
    private int _lastStatusChangeIndex;

    private IReadOnlyDictionary<string, Card> _cardRegistry = new Dictionary<string, Card>();
    private IReadOnlyDictionary<string, IEnemyBrain> _brainRegistry = new Dictionary<string, IEnemyBrain>();

    /// <summary>Fase corrente do combate (espelha o ponto da orquestração). UI consome.</summary>
    public CombatPhase Phase { get; private set; } = CombatPhase.SetupPhase;

    /// <summary>Id do ator do turno corrente, ou null se não há combate ativo.</summary>
    public string? ActiveActorId => _fsm?.ActiveActor?.Id;

    public override void _Ready() => Instance = this;

    public override void _ExitTree()
    {
        if (Instance == this) Instance = null;
    }

    /// <summary>
    /// Inicia um combate. Monta a FSM, emite CombatBus.CombatStarted e avança até o
    /// primeiro ponto de espera (turno de inimigo resolve sozinho; turno de party para
    /// aguardando SubmitPlayerAction).
    /// </summary>
    public void StartCombat(
        IEnumerable<CombatActor> actors,
        IReadOnlyDictionary<string, Card>? cardRegistry = null,
        IReadOnlyDictionary<string, IEnemyBrain>? brainRegistry = null)
    {
        _actors.Clear();
        _actors.AddRange(actors);
        _reportedDown.Clear();
        _lastLogIndex = 0;
        _lastStatusChangeIndex = 0;

        _cardRegistry = cardRegistry ?? new Dictionary<string, Card>();
        _brainRegistry = brainRegistry ?? new Dictionary<string, IEnemyBrain>();

        _fsm = new CombatStateMachine(_actors, ActionProvider, _cardRegistry, _brainRegistry);

        _combatActive = true;
        _waitingForPlayerAction = false;
        _pendingPlayerAction = null;
        Phase = CombatPhase.TurnStart;

        var names = new Godot.Collections.Array(_actors.Select(a => Variant.From(a.Id)).ToArray());
        CombatBus.Instance?.EmitSignal(CombatBus.SignalName.CombatStarted, names);

        // Resolve turnos de inimigo até parar no primeiro turno de party (que aguarda
        // SubmitPlayerAction). Se o primeiro ator for da party, paramos imediatamente:
        // NÃO chamar AdvanceFsm direto, senão o turno do jogador seria consumido como Pass.
        TryAdvance();
    }

    /// <summary>
    /// Enfileira a ação do jogador para o turno corrente. No-op se não há combate ativo
    /// ou se o turno corrente não está aguardando ação da party.
    /// </summary>
    public void SubmitPlayerAction(CombatAction action)
    {
        if (!_combatActive || !_waitingForPlayerAction) return;
        _pendingPlayerAction = action;
        _waitingForPlayerAction = false;
    }

    /// <summary>
    /// Provider injetado na FSM. Inimigo: decide via IEnemyBrain. Party: devolve a ação
    /// pendente (o fluxo de _Process garante que só chamamos a FSM quando há ação).
    /// </summary>
    private CombatAction ActionProvider(CombatActor actor, CombatState state)
    {
        if (!actor.IsPlayerSide)
        {
            return _brainRegistry.TryGetValue(actor.Id, out var brain)
                ? brain.DecideAction(state, actor)
                : CombatAction.Pass();
        }

        var action = _pendingPlayerAction ?? CombatAction.Pass();
        _pendingPlayerAction = null;
        return action;
    }

    public override void _Process(double delta) => TryAdvance();

    /// <summary>
    /// Gate de avanço: só chama AdvanceFsm quando o turno corrente pode resolver agora.
    /// Turno de party sem ação pendente fica em espera (não consome o turno como Pass).
    /// Compartilhado por StartCombat e _Process pra garantir a mesma regra de gating.
    /// </summary>
    private void TryAdvance()
    {
        if (!_combatActive || _fsm == null) return;
        if (_fsm.Outcome != CombatOutcome.Ongoing) return;

        var actor = _fsm.ActiveActor;
        if (actor.IsPlayerSide && _pendingPlayerAction == null)
        {
            _waitingForPlayerAction = true;
            return; // aguarda SubmitPlayerAction
        }

        _waitingForPlayerAction = false;
        AdvanceFsm();
    }

    /// <summary>
    /// Avança 1 turno completo da FSM e emite os signals correspondentes:
    /// TurnStarted -> (resolução) -> ActionResolved* + Defeated/Incapacitated -> TurnEnded
    /// -> CheckEnd (CombatEnded + CombatResultReceived no fim).
    /// </summary>
    private void AdvanceFsm()
    {
        if (_fsm == null || _fsm.Outcome != CombatOutcome.Ongoing) return;

        var actor = _fsm.ActiveActor;

        Phase = CombatPhase.TurnStart;
        var stunned = _fsm.BeginTurn();
        CombatBus.Instance?.EmitSignal(
            CombatBus.SignalName.TurnStarted, actor.Id, _fsm.Queue.RoundIndex);

        if (!stunned)
        {
            Phase = CombatPhase.ActionResolve;
            _fsm.RunActiveTurnToEnd();
        }

        EmitRecentLogSignals();
        EmitRecentStatusChanges();
        EmitDownedSignals();

        Phase = CombatPhase.TurnEnd;
        CombatBus.Instance?.EmitSignal(CombatBus.SignalName.TurnEnded, actor.Id);

        Phase = CombatPhase.CheckEnd;
        if (_fsm.CheckEnd())
        {
            // CheckEnd remove derrotados da fila; capturar quaisquer quedas finais.
            EmitDownedSignals();

            _combatActive = false;
            Phase = CombatPhase.CombatEnd;

            var outcome = _fsm.Outcome.ToString();
            CombatBus.Instance?.EmitSignal(CombatBus.SignalName.CombatEnded, outcome);
            PlayerBus.Instance?.EmitSignal(
                PlayerBus.SignalName.CombatResultReceived, outcome, _fsm.Queue.RoundIndex);
            return;
        }

        _fsm.AdvanceToNextActor();
    }

    /// <summary>
    /// Traduz cada nova entrada do log da FSM em CombatBus.ActionResolved. No slice toda
    /// entrada vira um ActionResolved genérico (sem parsear o tipo específico).
    /// </summary>
    private void EmitRecentLogSignals()
    {
        if (_fsm == null) return;

        var log = _fsm.Log;
        for (var i = _lastLogIndex; i < log.Count; i++)
        {
            var entry = log[i];
            CombatBus.Instance?.EmitSignal(
                CombatBus.SignalName.ActionResolved,
                entry.ActorId,
                entry.TargetId ?? string.Empty,
                entry.Action.ToString(),
                entry.Value);
        }
        _lastLogIndex = log.Count;
    }

    /// <summary>
    /// Traduz cada nova mudança de status do buffer da FSM (§16) em CombatBus.StatusApplied /
    /// StatusExpired. Mapeia StatusId enum -> string via ToString() (mesma convenção dos
    /// demais signals). Absorbed não tem signal próprio no contrato atual: emitido como
    /// StatusApplied com o pool restante (Magnitude) pra a UI atualizar a barra de Shield
    /// sem alargar o contrato do bus.
    /// </summary>
    private void EmitRecentStatusChanges()
    {
        if (_fsm == null) return;

        var changes = _fsm.StatusChanges;
        for (var i = _lastStatusChangeIndex; i < changes.Count; i++)
        {
            var change = changes[i];
            var statusId = change.Id.ToString();
            switch (change.Kind)
            {
                case StatusChangeKind.Applied:
                case StatusChangeKind.Absorbed:
                    CombatBus.Instance?.EmitSignal(
                        CombatBus.SignalName.StatusApplied,
                        change.ActorId, statusId, change.Magnitude, change.Duration);
                    break;
                case StatusChangeKind.Expired:
                    CombatBus.Instance?.EmitSignal(
                        CombatBus.SignalName.StatusExpired, change.ActorId, statusId);
                    break;
            }
        }
        _lastStatusChangeIndex = changes.Count;
    }

    /// <summary>
    /// Emite ActorDefeated (inimigo) ou ActorIncapacitated (party, Pillar 4) para cada
    /// ator que chegou a HP 0 e ainda não foi reportado. Idempotente via _reportedDown.
    /// </summary>
    private void EmitDownedSignals()
    {
        foreach (var actor in _actors)
        {
            if (actor.IsAlive || _reportedDown.Contains(actor.Id)) continue;
            _reportedDown.Add(actor.Id);

            var signal = actor.IsPlayerSide
                ? CombatBus.SignalName.ActorIncapacitated
                : CombatBus.SignalName.ActorDefeated;
            CombatBus.Instance?.EmitSignal(signal, actor.Id);
        }
    }
}
