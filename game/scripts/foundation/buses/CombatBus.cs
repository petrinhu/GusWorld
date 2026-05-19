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

    /// <summary>Emitido quando turn de um combatente começa.</summary>
    [Signal] public delegate void TurnStartedEventHandler(string combatantName);

    /// <summary>Emitido quando turn termina.</summary>
    [Signal] public delegate void TurnEndedEventHandler(string combatantName);

    /// <summary>Emitido após ação ser resolvida (card jogado, dano aplicado, status mudado).</summary>
    [Signal] public delegate void ActionResolvedEventHandler(string actor, string target, string actionType, int value);

    public override void _Ready()
    {
        Instance = this;
    }

    public override void _ExitTree()
    {
        if (Instance == this) Instance = null;
    }
}
