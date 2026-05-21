// SceneManager.cs
//
// AutoLoad global pra carregamento de cenas com fade + progress.
//
// API public:
//   - Task<Error> LoadSceneAsync(string path, FadeConfig? config = null)
//   - Task<Error> LoadSceneInstantAsync(string path)  // sem fade
//   - float GetLoadProgress()                          // 0.0 a 1.0
//   - void ReloadCurrentScene()
//   - bool IsLoading { get; private set; }
//
// Signals:
//   - SceneLoadStarted(string path)
//   - SceneLoadProgress(float progress)
//   - SceneLoadCompleted(string path)
//   - SceneLoadFailed(string path, string error)
//   + relay GameStateBus.SceneChanged(path) em sucesso
//
// Pattern: ResourceLoader.LoadThreadedRequest + poll LoadThreadedGetStatus em _Process.
// Fade overlay: ColorRect fullscreen em CanvasLayer dinâmico Layer=128 (acima UI normal).
//
// Cross-ref: docs/tech/engine-modules.md §2.5 + ADR-002 batch 6 F2-E.4.

using System;
using System.Threading.Tasks;
using Godot;
using GusDragon.Engine.Foundation.SceneManager;
using GusWorld.Game.Foundation.Buses;

namespace GusWorld.Game.Foundation.SceneManager;

/// <summary>
/// Manager AutoLoad de transição de cena. Async API com fade + progress polling.
/// </summary>
public partial class SceneManager : Node
{
    public static SceneManager? Instance { get; private set; }

    /// <summary>Layer canon do CanvasLayer do fade overlay (acima UI normal).</summary>
    private const int FadeCanvasLayer = 128;

    /// <summary>Polling rate do LoadThreadedGetStatus em segundos.</summary>
    private const float ProgressPollSeconds = 0.05f;

    // Signals canon F2-E.4
    [Signal] public delegate void SceneLoadStartedEventHandler(string path);
    [Signal] public delegate void SceneLoadProgressEventHandler(float progress);
    [Signal] public delegate void SceneLoadCompletedEventHandler(string path);
    [Signal] public delegate void SceneLoadFailedEventHandler(string path, string error);

    private CanvasLayer? _fadeLayer;
    private ColorRect? _fadeRect;
    private float _currentProgress;
    private string? _pendingScenePath;

    /// <summary>True enquanto request + load + swap + fade-in não completaram.</summary>
    public bool IsLoading { get; private set; }

    public override void _Ready()
    {
        Instance = this;
        BuildFadeOverlay();
    }

    public override void _ExitTree()
    {
        if (Instance == this) Instance = null;
    }

    /// <summary>
    /// Constrói CanvasLayer + ColorRect fullscreen pra fade overlay.
    /// ColorRect inicia transparente (alpha 0).
    /// </summary>
    private void BuildFadeOverlay()
    {
        _fadeLayer = new CanvasLayer
        {
            Name = "FadeLayer",
            Layer = FadeCanvasLayer,
        };
        AddChild(_fadeLayer);

        _fadeRect = new ColorRect
        {
            Name = "FadeRect",
            Color = new Color(0f, 0f, 0f, 0f),
            MouseFilter = Control.MouseFilterEnum.Ignore,
            AnchorLeft = 0f,
            AnchorTop = 0f,
            AnchorRight = 1f,
            AnchorBottom = 1f,
            OffsetLeft = 0f,
            OffsetTop = 0f,
            OffsetRight = 0f,
            OffsetBottom = 0f,
            Visible = false,
        };
        _fadeLayer.AddChild(_fadeRect);
    }

    /// <summary>Retorna progresso atual da load em andamento (0.0 a 1.0). 0 quando idle.</summary>
    public float GetLoadProgress() => _currentProgress;

    /// <summary>
    /// Carrega cena assíncrono com fade (out → swap → in).
    /// </summary>
    /// <param name="path">Path res:// da cena (.tscn).</param>
    /// <param name="config">Config do fade ou null pra FadeConfig.Default.</param>
    /// <returns>Godot.Error.Ok em sucesso, código erro caso contrário.</returns>
    public async Task<Error> LoadSceneAsync(string path, FadeConfig? config = null)
    {
        if (IsLoading)
        {
            GD.PushWarning($"SceneManager: ignorando LoadSceneAsync({path}) — load já em andamento");
            return Error.Busy;
        }

        var fadeConfig = config ?? FadeConfig.Default;
        return await LoadInternalAsync(path, fadeConfig);
    }

    /// <summary>
    /// Carrega cena assíncrono SEM fade (durações zero). Útil pra debug e cenas loading-screen.
    /// </summary>
    public async Task<Error> LoadSceneInstantAsync(string path)
    {
        if (IsLoading)
        {
            GD.PushWarning($"SceneManager: ignorando LoadSceneInstantAsync({path}) — load já em andamento");
            return Error.Busy;
        }

        return await LoadInternalAsync(path, FadeConfig.Instant);
    }

    /// <summary>Recarrega cena atual (mesmo path) com fade default.</summary>
    public void ReloadCurrentScene()
    {
        var currentPath = GetTree().CurrentScene?.SceneFilePath;
        if (string.IsNullOrEmpty(currentPath))
        {
            GD.PushWarning("SceneManager: ReloadCurrentScene chamado sem CurrentScene válida");
            return;
        }
        _ = LoadSceneAsync(currentPath);
    }

    private async Task<Error> LoadInternalAsync(string path, FadeConfig fadeConfig)
    {
        IsLoading = true;
        _pendingScenePath = path;
        _currentProgress = 0f;
        EmitSignal(SignalName.SceneLoadStarted, path);

        try
        {
            // Fase 1: fade-out
            if (fadeConfig.DurationOut > 0f)
                await FadeAsync(fadeConfig.FadeColor, 0f, 1f, fadeConfig.DurationOut, fadeConfig.EaseType);
            else
                SetFadeOpaque(fadeConfig.FadeColor);

            // Fase 2: request threaded load
            var requestErr = ResourceLoader.LoadThreadedRequest(path);
            if (requestErr != Error.Ok)
            {
                var msg = $"LoadThreadedRequest retornou {requestErr}";
                FailAndReset(path, msg);
                return requestErr;
            }

            // Fase 3: poll progress até status final
            var pollErr = await PollLoadProgressAsync(path);
            if (pollErr != Error.Ok)
                return pollErr;

            // Fase 4: pega resource + swap
            var resource = ResourceLoader.LoadThreadedGet(path);
            if (resource is not PackedScene packed)
            {
                var msg = $"resource carregado não é PackedScene (path={path})";
                FailAndReset(path, msg);
                return Error.InvalidData;
            }

            var swapErr = GetTree().ChangeSceneToPacked(packed);
            if (swapErr != Error.Ok)
            {
                var msg = $"ChangeSceneToPacked retornou {swapErr}";
                FailAndReset(path, msg);
                return swapErr;
            }

            // ChangeSceneToPacked é deferred (free old + add new no fim do frame).
            // Aguarda 1 process frame pra SceneTree.CurrentScene ser populado antes de declarar Completed.
            // Sem esse await, callers que checam CurrentScene logo após LoadSceneAsync veem o ponteiro stale.
            await ToSignal(GetTree(), SceneTree.SignalName.ProcessFrame);

            // Fase 5: fade-in
            if (fadeConfig.DurationIn > 0f)
                await FadeAsync(fadeConfig.FadeColor, 1f, 0f, fadeConfig.DurationIn, fadeConfig.EaseType);
            else
                SetFadeTransparent();

            // Sucesso
            EmitSignal(SignalName.SceneLoadCompleted, path);
            GameStateBus.Instance?.EmitSignal(GameStateBus.SignalName.SceneChanged, path);
            return Error.Ok;
        }
        catch (Exception ex)
        {
            FailAndReset(path, ex.Message);
            return Error.Failed;
        }
        finally
        {
            IsLoading = false;
            _pendingScenePath = null;
            _currentProgress = 0f;
        }
    }

    /// <summary>
    /// Poll LoadThreadedGetStatus até status terminal. Atualiza progress + emit signal.
    /// </summary>
    private async Task<Error> PollLoadProgressAsync(string path)
    {
        var progressArray = new Godot.Collections.Array();
        while (true)
        {
            var status = ResourceLoader.LoadThreadedGetStatus(path, progressArray);
            if (progressArray.Count > 0)
            {
                _currentProgress = (float)progressArray[0].AsDouble();
                EmitSignal(SignalName.SceneLoadProgress, _currentProgress);
            }

            switch (status)
            {
                case ResourceLoader.ThreadLoadStatus.InProgress:
                    await Task.Delay(TimeSpan.FromSeconds(ProgressPollSeconds));
                    continue;

                case ResourceLoader.ThreadLoadStatus.Loaded:
                    _currentProgress = 1f;
                    EmitSignal(SignalName.SceneLoadProgress, _currentProgress);
                    return Error.Ok;

                case ResourceLoader.ThreadLoadStatus.InvalidResource:
                    FailAndReset(path, "ThreadLoadStatus.InvalidResource (path inexistente ou inválido)");
                    return Error.FileNotFound;

                case ResourceLoader.ThreadLoadStatus.Failed:
                    FailAndReset(path, "ThreadLoadStatus.Failed");
                    return Error.Failed;

                default:
                    FailAndReset(path, $"ThreadLoadStatus inesperado: {status}");
                    return Error.Failed;
            }
        }
    }

    /// <summary>
    /// Anima alpha do _fadeRect via Tween. Aguarda completar via SignalAwaiter.
    /// </summary>
    private async Task FadeAsync(Color baseColor, float fromAlpha, float toAlpha, float duration, Tween.EaseType ease)
    {
        if (_fadeRect == null) return;

        _fadeRect.Color = new Color(baseColor.R, baseColor.G, baseColor.B, fromAlpha);
        _fadeRect.Visible = true;

        var tween = CreateTween();
        tween.SetEase(ease);
        tween.SetTrans(Tween.TransitionType.Cubic);
        tween.TweenProperty(
            _fadeRect,
            "color",
            new Color(baseColor.R, baseColor.G, baseColor.B, toAlpha),
            duration);

        await ToSignal(tween, Tween.SignalName.Finished);

        // Mantém visível só enquanto opaco; transparente esconde pra economizar fillrate.
        if (Mathf.IsZeroApprox(toAlpha))
            _fadeRect.Visible = false;
    }

    private void SetFadeOpaque(Color baseColor)
    {
        if (_fadeRect == null) return;
        _fadeRect.Color = new Color(baseColor.R, baseColor.G, baseColor.B, 1f);
        _fadeRect.Visible = true;
    }

    private void SetFadeTransparent()
    {
        if (_fadeRect == null) return;
        _fadeRect.Color = new Color(_fadeRect.Color.R, _fadeRect.Color.G, _fadeRect.Color.B, 0f);
        _fadeRect.Visible = false;
    }

    private void FailAndReset(string path, string errorMessage)
    {
        GD.PushError($"SceneManager: load falhou em '{path}': {errorMessage}");
        EmitSignal(SignalName.SceneLoadFailed, path, errorMessage);
        SetFadeTransparent();
    }
}
