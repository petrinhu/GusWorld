// PlayerBus.cs
//
// AutoLoad global de signals do player (locomoção, interação, dano, morte).
//
// Cross-ref: docs/tech/engine-modules.md §2.1.2.

using Godot;

namespace GusWorld.Game.Foundation.Buses;

/// <summary>
/// Bus global de signals do player.
/// AutoLoad registrado como "PlayerBus" em project.godot.
/// </summary>
public partial class PlayerBus : Node
{
    public static PlayerBus? Instance { get; private set; }

    /// <summary>Emitido a cada movimento de locomoção concluído.</summary>
    [Signal] public delegate void PlayerMovedEventHandler(Vector3 fromPosition, Vector3 toPosition);

    /// <summary>Emitido quando player interage com NPC, objeto ou trigger.</summary>
    [Signal] public delegate void PlayerInteractedEventHandler(string targetName);

    /// <summary>Emitido quando HP do player muda.</summary>
    [Signal] public delegate void PlayerHpChangedEventHandler(int current, int maximum);

    /// <summary>Emitido quando player morre. Game over flow começa aqui.</summary>
    [Signal] public delegate void PlayerDiedEventHandler();

    /// <summary>
    /// Emitido ao fim de um combate. CombatManager → PlayerBus pra concessão de XP/mestria/loot.
    /// Stub no slice: payload simplificado (outcome string + roundsElapsed int).
    /// </summary>
    [Signal] public delegate void CombatResultReceivedEventHandler(string outcome, int roundsElapsed);

    public override void _Ready()
    {
        Instance = this;
    }

    public override void _ExitTree()
    {
        if (Instance == this) Instance = null;
    }
}
