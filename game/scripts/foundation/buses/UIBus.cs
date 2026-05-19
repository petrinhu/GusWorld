// UIBus.cs
//
// AutoLoad global de signals da UI (dialogue, menus).
//
// Cross-ref: docs/tech/engine-modules.md §2.1.4.

using Godot;

namespace GusWorld.Game.Foundation.Buses;

/// <summary>
/// Bus global de signals da UI.
/// AutoLoad registrado como "UIBus" em project.godot.
/// </summary>
public partial class UIBus : Node
{
    public static UIBus? Instance { get; private set; }

    /// <summary>Emitido quando dialogue UI abre.</summary>
    [Signal] public delegate void DialogueShownEventHandler(string dialogueId);

    /// <summary>Emitido quando jogador escolhe opção em branching dialogue.</summary>
    [Signal] public delegate void DialogueChoiceMadeEventHandler(string dialogueId, int choiceIndex);

    /// <summary>Emitido quando menu (settings, save, pause, inventory) abre.</summary>
    [Signal] public delegate void MenuOpenedEventHandler(string menuName);

    /// <summary>Emitido quando menu fecha.</summary>
    [Signal] public delegate void MenuClosedEventHandler(string menuName);

    public override void _Ready()
    {
        Instance = this;
    }

    public override void _ExitTree()
    {
        if (Instance == this) Instance = null;
    }
}
