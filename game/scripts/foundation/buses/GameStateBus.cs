// GameStateBus.cs
//
// AutoLoad global de signals do lifecycle do jogo.
//
// Registrar em Project Settings > AutoLoad como "GameStateBus".
//
// Uso:
//   GameStateBus.Instance.EmitSignal(GameStateBus.SignalName.GameStarted);
//   GameStateBus.Instance.GameStarted += OnGameStarted;
//
// Cross-ref: docs/tech/engine-modules.md §2.1.1.

using Godot;

namespace GusWorld.Game.Foundation.Buses;

/// <summary>
/// Bus global de signals de lifecycle do jogo (start, pause, resume, save, load).
/// AutoLoad registrado como "GameStateBus" em project.godot.
/// </summary>
public partial class GameStateBus : Node
{
    public static GameStateBus? Instance { get; private set; }

    /// <summary>Emitido quando jogo inicia (após splash + main menu, antes da primeira cena).</summary>
    [Signal] public delegate void GameStartedEventHandler();

    /// <summary>Emitido quando jogo é pausado (player abriu menu in-game).</summary>
    [Signal] public delegate void GamePausedEventHandler();

    /// <summary>Emitido quando jogo retoma de pause.</summary>
    [Signal] public delegate void GameResumedEventHandler();

    /// <summary>Emitido após save bem-sucedido em slot.</summary>
    [Signal] public delegate void GameSavedEventHandler(int slot);

    /// <summary>Emitido após load bem-sucedido.</summary>
    [Signal] public delegate void GameLoadedEventHandler(int slot);

    public override void _Ready()
    {
        Instance = this;
    }

    public override void _ExitTree()
    {
        if (Instance == this) Instance = null;
    }
}
