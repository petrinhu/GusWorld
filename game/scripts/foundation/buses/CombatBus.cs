// CombatBus.cs
//
// AutoLoad global de signals do combate turn-based.
//
// Cross-ref: docs/tech/engine-modules.md §2.1.3.

using Godot;

namespace GusWorld.Game.Foundation.Buses;

/// <summary>
/// Bus global de signals do combate turn-based.
/// AutoLoad registrado como "CombatBus" em project.godot.
/// </summary>
public partial class CombatBus : Node
{
    public static CombatBus? Instance { get; private set; }

    /// <summary>Emitido no início de uma cena de combate.</summary>
    [Signal] public delegate void CombatStartedEventHandler(Godot.Collections.Array combatants);

    /// <summary>Emitido no fim de cena de combate.</summary>
    [Signal] public delegate void CombatEndedEventHandler(string winner);

    /// <summary>Emitido quando turn de um combatente começa. roundIndex = rodada de fila (ramp de mana).</summary>
    [Signal] public delegate void TurnStartedEventHandler(string combatantName, int roundIndex);

    /// <summary>Emitido quando turn termina.</summary>
    [Signal] public delegate void TurnEndedEventHandler(string combatantName);

    /// <summary>Emitido após ação ser resolvida (card jogado, dano aplicado, status mudado).</summary>
    [Signal] public delegate void ActionResolvedEventHandler(string actor, string target, string actionType, int value);

    /// <summary>Status aplicado a um ator.</summary>
    [Signal] public delegate void StatusAppliedEventHandler(string actorId, string statusId, int magnitude, int duration);

    /// <summary>Status expirado em um ator.</summary>
    [Signal] public delegate void StatusExpiredEventHandler(string actorId, string statusId);

    /// <summary>Ator (inimigo) derrotado (HP 0, removido do combate).</summary>
    [Signal] public delegate void ActorDefeatedEventHandler(string actorId);

    /// <summary>Companion incapacitado (HP 0, fica fora do combate mas recuperável). Pillar 4.</summary>
    [Signal] public delegate void ActorIncapacitatedEventHandler(string actorId);

    /// <summary>
    /// Ambiente de combate marcado/trocado (§18.10). envId = EnvironmentId (string). Cobre
    /// tanto a marcação inicial da arena quanto o avanço automático da roda de período (§18.3).
    /// </summary>
    [Signal] public delegate void EnvironmentSetEventHandler(string envId);

    public override void _Ready()
    {
        Instance = this;
    }

    public override void _ExitTree()
    {
        if (Instance == this) Instance = null;
    }
}
