// SceneManagerTest.cs
//
// Script standalone TDD pra F2-E.4 SceneManager.
//
// Uso:
//   cd game/
//   godot --headless -s tools/SceneManagerTest.cs
//
// Test cases:
//   1. LoadSceneInstantAsync(test_scene_a) → Error.Ok, signals emitidos
//   2. LoadSceneAsync(test_scene_b) com fade → Error.Ok, signals emitidos + GameStateBus.SceneChanged
//   3. ReloadCurrentScene → recarrega cena atual
//   4. Error path: LoadSceneAsync(path inválido) → Error.FileNotFound, SceneLoadFailed emitido
//   5. Race condition: LoadSceneAsync chamado durante outra load → Error.Busy
//
// Exit code 0 = sucesso, 1 = falha.

using Godot;
using GusDragon.Engine.Foundation.SceneManager;
using GusWorld.Game.Foundation.Buses;
using GusWorld.Game.Foundation.SceneManager;

namespace GusWorld.Game.Tools;

public partial class SceneManagerTest : SceneTree
{
    private int _errors;

    // Tracking de signals emitidos durante cada test
    private int _startedCount;
    private int _progressCount;
    private int _completedCount;
    private int _failedCount;
    private int _sceneChangedRelayCount;
    private string _lastStartedPath = string.Empty;
    private string _lastCompletedPath = string.Empty;
    private string _lastFailedPath = string.Empty;
    private string _lastFailedError = string.Empty;
    private string _lastRelayedPath = string.Empty;

    private const string TestSceneA = "res://tests/scenes/test_scene_a.tscn";
    private const string TestSceneB = "res://tests/scenes/test_scene_b.tscn";
    private const string InvalidPath = "res://tests/scenes/path_inexistente.tscn";

    public override void _Initialize()
    {
        // Dispara teste async e quit quando completar (CallDeferred via Task).
        _ = RunAllTestsAsync();
    }

    private async Task RunAllTestsAsync()
    {
        GD.Print("=== F2-E.4 SceneManager Test Suite ===");
        _errors = 0;

        // Aguarda 1 frame pra autoloads inicializarem
        await ToSignal(this, SceneTree.SignalName.ProcessFrame);

        var manager = Root.GetNodeOrNull<SceneManager>("SceneManager");
        if (manager == null)
        {
            Fail("SceneManager AutoLoad ausente — abortando suite");
            Quit(1);
            return;
        }

        ConnectSignals(manager);

        await TestLoadSceneInstantAsync(manager);
        await TestLoadSceneAsyncWithFade(manager);
        await TestReloadCurrentScene(manager);
        await TestInvalidPath(manager);
        await TestBusyRejectsConcurrent(manager);

        GD.Print($"=== Resultado: {_errors} erro(s) ===");
        Quit(_errors == 0 ? 0 : 1);
    }

    private void ConnectSignals(SceneManager manager)
    {
        manager.SceneLoadStarted += path =>
        {
            _startedCount++;
            _lastStartedPath = path;
        };
        manager.SceneLoadProgress += _ => _progressCount++;
        manager.SceneLoadCompleted += path =>
        {
            _completedCount++;
            _lastCompletedPath = path;
        };
        manager.SceneLoadFailed += (path, err) =>
        {
            _failedCount++;
            _lastFailedPath = path;
            _lastFailedError = err;
        };

        var gsb = Root.GetNodeOrNull<GameStateBus>("GameStateBus");
        if (gsb != null)
        {
            gsb.SceneChanged += path =>
            {
                _sceneChangedRelayCount++;
                _lastRelayedPath = path;
            };
        }
    }

    private void ResetCounters()
    {
        _startedCount = 0;
        _progressCount = 0;
        _completedCount = 0;
        _failedCount = 0;
        _sceneChangedRelayCount = 0;
        _lastStartedPath = string.Empty;
        _lastCompletedPath = string.Empty;
        _lastFailedPath = string.Empty;
        _lastFailedError = string.Empty;
        _lastRelayedPath = string.Empty;
    }

    private async Task TestLoadSceneInstantAsync(SceneManager manager)
    {
        GD.Print("-- Test 1: LoadSceneInstantAsync feliz path --");
        ResetCounters();

        var err = await manager.LoadSceneInstantAsync(TestSceneA);

        if (err == Error.Ok)
            Pass("LoadSceneInstantAsync retornou Error.Ok");
        else
            Fail($"LoadSceneInstantAsync esperava Error.Ok obteve {err}");

        if (_startedCount == 1 && _lastStartedPath == TestSceneA)
            Pass($"SceneLoadStarted emitido 1x com path correto");
        else
            Fail($"SceneLoadStarted count={_startedCount} path='{_lastStartedPath}'");

        if (_completedCount == 1 && _lastCompletedPath == TestSceneA)
            Pass($"SceneLoadCompleted emitido 1x com path correto");
        else
            Fail($"SceneLoadCompleted count={_completedCount} path='{_lastCompletedPath}'");

        if (_progressCount >= 1)
            Pass($"SceneLoadProgress emitido {_progressCount}x");
        else
            Fail($"SceneLoadProgress esperava >=1 obteve {_progressCount}");

        if (_sceneChangedRelayCount == 1 && _lastRelayedPath == TestSceneA)
            Pass("GameStateBus.SceneChanged relay emitido 1x");
        else
            Fail($"GameStateBus.SceneChanged relay count={_sceneChangedRelayCount} path='{_lastRelayedPath}'");

        if (!manager.IsLoading && Mathf.IsZeroApprox(manager.GetLoadProgress()))
            Pass("Estado pós-load: IsLoading=false, progress=0");
        else
            Fail($"Estado pós-load inesperado: IsLoading={manager.IsLoading} progress={manager.GetLoadProgress()}");

        // Verifica cena efetivamente trocou
        var currentPath = CurrentScene?.SceneFilePath ?? string.Empty;
        if (currentPath == TestSceneA)
            Pass($"CurrentScene.SceneFilePath = '{currentPath}' (swap efetivo)");
        else
            Fail($"CurrentScene esperava '{TestSceneA}' obteve '{currentPath}'");
    }

    private async Task TestLoadSceneAsyncWithFade(SceneManager manager)
    {
        GD.Print("-- Test 2: LoadSceneAsync com FadeConfig.Default --");
        ResetCounters();

        // Config rápido pra teste headless (10ms cada lado, ainda > 0 pra exercitar caminho fade)
        var fastConfig = new FadeConfig(Colors.Black, 0.01f, 0.01f, Tween.EaseType.InOut);
        var err = await manager.LoadSceneAsync(TestSceneB, fastConfig);

        if (err == Error.Ok)
            Pass($"LoadSceneAsync retornou Error.Ok");
        else
            Fail($"LoadSceneAsync esperava Error.Ok obteve {err}");

        if (_startedCount == 1 && _completedCount == 1 && _failedCount == 0)
            Pass($"Signals: started=1 completed=1 failed=0");
        else
            Fail($"Signals incorretos: started={_startedCount} completed={_completedCount} failed={_failedCount}");

        if (_sceneChangedRelayCount == 1 && _lastRelayedPath == TestSceneB)
            Pass("GameStateBus.SceneChanged relay emitido 1x");
        else
            Fail($"GameStateBus.SceneChanged relay count={_sceneChangedRelayCount}");

        var currentPath = CurrentScene?.SceneFilePath ?? string.Empty;
        if (currentPath == TestSceneB)
            Pass($"CurrentScene = '{currentPath}'");
        else
            Fail($"CurrentScene esperava '{TestSceneB}' obteve '{currentPath}'");
    }

    private async Task TestReloadCurrentScene(SceneManager manager)
    {
        GD.Print("-- Test 3: ReloadCurrentScene --");
        ResetCounters();

        var pathBefore = CurrentScene?.SceneFilePath ?? string.Empty;
        manager.ReloadCurrentScene();

        // Aguarda load completar (poll _completedCount + timeout 5s)
        const float timeoutSeconds = 5f;
        var elapsed = 0f;
        const float pollInterval = 0.05f;
        while (_completedCount == 0 && _failedCount == 0 && elapsed < timeoutSeconds)
        {
            await Task.Delay(TimeSpan.FromSeconds(pollInterval));
            elapsed += pollInterval;
        }

        if (_completedCount == 1 && _failedCount == 0)
            Pass($"ReloadCurrentScene completou (path={_lastCompletedPath})");
        else
            Fail($"ReloadCurrentScene timeout ou falhou: completed={_completedCount} failed={_failedCount}");

        var pathAfter = CurrentScene?.SceneFilePath ?? string.Empty;
        if (pathAfter == pathBefore)
            Pass($"CurrentScene path mantido após reload: '{pathAfter}'");
        else
            Fail($"CurrentScene mudou em reload: '{pathBefore}' → '{pathAfter}'");
    }

    private async Task TestInvalidPath(SceneManager manager)
    {
        GD.Print("-- Test 4: LoadSceneAsync path inválido --");
        ResetCounters();

        var err = await manager.LoadSceneInstantAsync(InvalidPath);

        if (err != Error.Ok)
            Pass($"LoadSceneInstantAsync(invalid) retornou erro {err} (esperado != Ok)");
        else
            Fail("LoadSceneInstantAsync(invalid) retornou Error.Ok inesperadamente");

        if (_failedCount == 1 && _lastFailedPath == InvalidPath)
            Pass($"SceneLoadFailed emitido 1x com path correto (err='{_lastFailedError}')");
        else
            Fail($"SceneLoadFailed count={_failedCount} path='{_lastFailedPath}'");

        if (_completedCount == 0 && _sceneChangedRelayCount == 0)
            Pass("SceneLoadCompleted/SceneChanged NÃO emitidos em error path");
        else
            Fail($"Signals indevidos em error: completed={_completedCount} relay={_sceneChangedRelayCount}");

        if (!manager.IsLoading)
            Pass("IsLoading=false após error path");
        else
            Fail("IsLoading deveria ser false após error path");
    }

    private async Task TestBusyRejectsConcurrent(SceneManager manager)
    {
        GD.Print("-- Test 5: Concurrent load retorna Error.Busy --");
        ResetCounters();

        // Config com fade longo pra criar janela de overlap
        var slowConfig = new FadeConfig(Colors.Black, 0.3f, 0.3f, Tween.EaseType.InOut);

        // Dispara primeira load sem await (background)
        var firstTask = manager.LoadSceneAsync(TestSceneA, slowConfig);

        // Pequeno delay pra primeira entrar no IsLoading=true
        await Task.Delay(TimeSpan.FromMilliseconds(20));

        // Segunda chamada DEVE retornar Busy
        var secondErr = await manager.LoadSceneAsync(TestSceneB, slowConfig);

        if (secondErr == Error.Busy)
            Pass("Concurrent LoadSceneAsync retorna Error.Busy");
        else
            Fail($"Concurrent esperava Error.Busy obteve {secondErr}");

        // Aguarda first task completar
        var firstErr = await firstTask;
        if (firstErr == Error.Ok)
            Pass($"Primeira load completou Error.Ok após concurrent rejection");
        else
            Fail($"Primeira load falhou inesperadamente: {firstErr}");
    }

    private static void Pass(string msg) => GD.Print($"OK: {msg}");

    private void Fail(string msg)
    {
        GD.PrintErr($"FAIL: {msg}");
        _errors++;
    }
}
