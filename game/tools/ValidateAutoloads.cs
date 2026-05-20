// ValidateAutoloads.cs
//
// Script standalone pra validar AutoLoads C# no boot.
// Substitui validate_autoloads.gd (removido em F2-S.MIG.1).
//
// Uso:
//   cd game/
//   godot --headless -s tools/ValidateAutoloads.cs
//
// Testa:
// - GameStateBus AutoLoad existe + signals esperados
// - PlayerBus AutoLoad existe + signals esperados
// - CombatBus AutoLoad existe + signals esperados
// - UIBus AutoLoad existe + signals esperados
// - Localization AutoLoad existe + carrega pt_br.md + en_intl.md
// - tr_md funcional + interpolação + fallback literal
//
// Exit code 0 = sucesso, 1 = falha.
//
// TDD pattern: cada novo AutoLoad migrado adiciona seção de validação aqui.

using Godot;
using GusDragon.Engine.Foundation;
using GusDragon.Engine.Foundation.InputRemap;
using GusDragon.Engine.Foundation.SaveSystem;
using GusWorld.Game.Back.OrbitalCamera;
using GusWorld.Game.Foundation.Buses;
using GusWorld.Game.Foundation.InputRemap;
using GusWorld.Game.Foundation.Localization;
using GusWorld.Game.Foundation.SaveSystem;

namespace GusWorld.Game.Tools;

public partial class ValidateAutoloads : SceneTree
{
    private int _errors;

    public override void _Initialize()
    {
        // SceneTree -s scripts rodam _Initialize antes de AutoLoads _Ready.
        // Force carregamento explícito Localization + InputRemap antes de validar.
        var locEarly = Root.GetNodeOrNull<Localization>("Localization");
        locEarly?.LoadAllLocales();
        var inputEarly = Root.GetNodeOrNull<InputRemapManager>("InputRemapManager");
        inputEarly?.LoadConfig();

        GD.Print("=== Validação AutoLoads + i18n ===");
        _errors = 0;

        ValidateGameStateBus();
        ValidatePlayerBus();
        ValidateCombatBus();
        ValidateUIBus();
        ValidateLocalization();
        ValidateMathHelpers();
        ValidateOrbitalCameraClass();
        ValidateSaveSystem();
        ValidateInputRemap();

        GD.Print($"=== Resultado: {_errors} erro(s) ===");
        Quit(_errors == 0 ? 0 : 1);
    }

    private void ValidateGameStateBus()
    {
        var node = Root.GetNodeOrNull<GameStateBus>("GameStateBus");
        if (node == null)
        {
            Fail("GameStateBus AutoLoad não registrado");
            return;
        }
        Pass("GameStateBus AutoLoad presente");

        string[] expectedSignals = {
            "GameStarted", "GamePaused", "GameResumed", "GameSaved", "GameLoaded"
        };
        foreach (var sig in expectedSignals)
        {
            if (!node.HasSignal(sig))
                Fail($"GameStateBus signal '{sig}' não encontrado");
        }
        if (_errors == 0)
            Pass("GameStateBus 5 signals canon verificados");
    }

    private void ValidatePlayerBus()
    {
        var node = Root.GetNodeOrNull<PlayerBus>("PlayerBus");
        if (node == null)
        {
            Fail("PlayerBus AutoLoad não registrado");
            return;
        }
        Pass("PlayerBus AutoLoad presente");

        string[] expectedSignals = {
            "PlayerMoved", "PlayerInteracted", "PlayerHpChanged", "PlayerDied"
        };
        foreach (var sig in expectedSignals)
        {
            if (!node.HasSignal(sig))
                Fail($"PlayerBus signal '{sig}' não encontrado");
        }
    }

    private void ValidateCombatBus()
    {
        var node = Root.GetNodeOrNull<CombatBus>("CombatBus");
        if (node == null)
        {
            Fail("CombatBus AutoLoad não registrado");
            return;
        }
        Pass("CombatBus AutoLoad presente");

        string[] expectedSignals = {
            "CombatStarted", "CombatEnded", "TurnStarted", "TurnEnded", "ActionResolved"
        };
        foreach (var sig in expectedSignals)
        {
            if (!node.HasSignal(sig))
                Fail($"CombatBus signal '{sig}' não encontrado");
        }
    }

    private void ValidateUIBus()
    {
        var node = Root.GetNodeOrNull<UIBus>("UIBus");
        if (node == null)
        {
            Fail("UIBus AutoLoad não registrado");
            return;
        }
        Pass("UIBus AutoLoad presente");

        string[] expectedSignals = {
            "DialogueShown", "DialogueChoiceMade", "MenuOpened", "MenuClosed"
        };
        foreach (var sig in expectedSignals)
        {
            if (!node.HasSignal(sig))
                Fail($"UIBus signal '{sig}' não encontrado");
        }
    }

    private void ValidateLocalization()
    {
        var node = Root.GetNodeOrNull<Localization>("Localization");
        if (node == null)
        {
            Fail("Localization AutoLoad não registrado");
            return;
        }
        Pass("Localization AutoLoad presente");

        var locales = node.GetAvailableLocales();
        GD.Print($"    Locales carregados: {string.Join(", ", locales)}");
        if (!locales.Contains("pt_br"))
            Fail("pt_br não carregado");
        if (!locales.Contains("en_intl"))
            Fail("en_intl não carregado");

        var testValue = node.TrMd("MENU_START_GAME");
        if (testValue == "Iniciar jogo")
            Pass($"TrMd MENU_START_GAME → '{testValue}'");
        else
            Fail($"TrMd MENU_START_GAME esperava 'Iniciar jogo' mas obteve '{testValue}'");

        var missing = node.TrMd("CHAVE_QUE_NAO_EXISTE_XYZ");
        if (missing == "CHAVE_QUE_NAO_EXISTE_XYZ")
            Pass("Fallback chave inexistente retorna literal");
        else
            Fail($"Fallback inesperado: '{missing}'");

        var interpolated = node.TrMd("SAVE_SLOT_LABEL", new Variant[] { "3" });
        if (interpolated == "Slot 3")
            Pass($"Interpolação {{0}} → '{interpolated}'");
        else
            Fail($"Interpolação esperava 'Slot 3' obteve '{interpolated}'");
    }

    private void ValidateMathHelpers()
    {
        // Snap 30° em 8 stops (45° cada) → 45°
        var snapped = MathHelpers.SnapToStops(30f, 8);
        if (Mathf.IsEqualApprox(snapped, 45f))
            Pass($"MathHelpers.SnapToStops(30°, 8) → {snapped}°");
        else
            Fail($"SnapToStops esperava 45° obteve {snapped}°");

        // Normalize -90° → 270°
        var norm = MathHelpers.NormalizeAngle(-90f);
        if (Mathf.IsEqualApprox(norm, 270f))
            Pass($"MathHelpers.NormalizeAngle(-90°) → {norm}°");
        else
            Fail($"NormalizeAngle esperava 270° obteve {norm}°");

        // Clamp01
        if (Mathf.IsEqualApprox(MathHelpers.Clamp01(1.5f), 1f) && Mathf.IsEqualApprox(MathHelpers.Clamp01(-0.5f), 0f))
            Pass("MathHelpers.Clamp01 boundaries OK");
        else
            Fail("Clamp01 falhou em boundaries");
    }

    private void ValidateOrbitalCameraClass()
    {
        // Check class existence via reflection-free pattern (compile-time link)
        var type = typeof(OrbitalCamera);
        if (type != null)
            Pass($"OrbitalCamera class type {type.FullName} carregado");
        else
            Fail("OrbitalCamera class type não encontrado");

        var configType = typeof(OrbitalCameraConfig);
        if (configType != null)
            Pass($"OrbitalCameraConfig Resource type {configType.FullName} carregado");
        else
            Fail("OrbitalCameraConfig type não encontrado");

        // Defaults config (instance)
        var config = new OrbitalCameraConfig();
        if (Mathf.IsEqualApprox(config.PitchDefaultDegrees, 45f) && Mathf.IsEqualApprox(config.ZoomDefault, 6f))
            Pass($"OrbitalCameraConfig defaults: pitch=45° zoom=6m");
        else
            Fail($"Config defaults inesperados: pitch={config.PitchDefaultDegrees}° zoom={config.ZoomDefault}m");
    }

    private void ValidateSaveSystem()
    {
        // SaveManager AutoLoad
        var sm = Root.GetNodeOrNull<SaveManager>("SaveManager");
        if (sm == null)
        {
            Fail("SaveManager AutoLoad não registrado");
            return;
        }
        Pass("SaveManager AutoLoad presente");

        // HMAC round-trip POCO test
        var sample = new SaveDataV1
        {
            Timestamp = "2026-05-19T00:00:00Z",
            PlaytimeSeconds = 42.0,
            CurrentScenePath = "res://test.tscn",
            PlayerPosition = new Vector3(1f, 2f, 3f),
            PlayerRotation = new Vector3(0f, 90f, 0f),
        };
        try
        {
            var envelope = SaveSerializer.SerializeWithHmac(sample);
            var roundtrip = SaveSerializer.DeserializeWithHmacValidation(envelope);
            if (roundtrip.PlaytimeSeconds == 42.0 && Mathf.IsEqualApprox(roundtrip.PlayerPosition.X, 1f))
                Pass("SaveSerializer HMAC roundtrip OK (Vector3 + double + string preservados)");
            else
                Fail($"SaveSerializer roundtrip lost data");
        }
        catch (System.Exception ex)
        {
            Fail($"SaveSerializer roundtrip threw: {ex.Message}");
        }

        // HMAC tamper detection (replace string única dentro do Payload escapado)
        try
        {
            var envelope = SaveSerializer.SerializeWithHmac(sample);
            // Tamper: substitui scene path no payload (string única, presente no JSON)
            var tampered = envelope.Replace("test.tscn", "evil.tscn", System.StringComparison.Ordinal);
            if (tampered == envelope)
            {
                Fail($"Tamper detection: replace falhou (envelope nao continha 'test.tscn'). Envelope sample: {envelope.Substring(0, System.Math.Min(200, envelope.Length))}");
            }
            else
            {
                SaveSerializer.DeserializeWithHmacValidation(tampered);
                Fail("Tamper detection: esperava SaveIntegrityException mas não foi lançada");
            }
        }
        catch (SaveIntegrityException)
        {
            Pass("Tamper detection: SaveIntegrityException lançada corretamente");
        }
        catch (System.Exception ex)
        {
            Fail($"Tamper detection: exception inesperada {ex.GetType().Name}: {ex.Message}");
        }

        // Constants canon (compile-time constants, valor canonizado)
        Pass($"SaveManager constants: autosave={SaveManager.AutosaveSlot}, slots={SaveManager.ManualSlotsCount}, backups={SaveManager.BackupChainDepth}");
    }

    private void ValidateInputRemap()
    {
        var manager = Root.GetNodeOrNull<InputRemapManager>("InputRemapManager");
        if (manager == null)
        {
            Fail("InputRemapManager AutoLoad não registrado");
            return;
        }
        Pass("InputRemapManager AutoLoad presente");

        // ActionRegistry count canon (movement 5 + camera 7 + interact 1 + menu 8 + combat 7 + dialogue 6 + inventory 2 + diary 1 = 37)
        var expectedActionCount = 37;
        if (ActionRegistry.Count == expectedActionCount)
            Pass($"ActionRegistry tem {expectedActionCount} actions canon");
        else
            Fail($"ActionRegistry esperava {expectedActionCount} actions, obteve {ActionRegistry.Count}");

        // DefaultBindings preenchidos pra todas actions
        var missingDefaults = 0;
        foreach (var def in ActionRegistry.Actions)
        {
            if (DefaultBindings.GetDefault(def.ActionName) == null)
            {
                Fail($"DefaultBindings ausente pra action '{def.ActionName}'");
                missingDefaults++;
            }
        }
        if (missingDefaults == 0)
            Pass($"DefaultBindings cobre todas {ActionRegistry.Count} actions");

        // Config atual existe
        var config = manager.GetCurrentConfig();
        if (config.Actions.Count == expectedActionCount)
            Pass($"InputRemapManager config atual tem {config.Actions.Count} actions bound");
        else
            Fail($"Config atual esperava {expectedActionCount} actions, obteve {config.Actions.Count}");

        // Lookup específico
        var moveBindings = manager.GetBindingsForAction("move_forward");
        if (moveBindings != null && moveBindings.Keys.Count >= 1)
            Pass($"move_forward bindings: {moveBindings.Keys.Count} keys, {moveBindings.GamepadAxes.Count} axes");
        else
            Fail("move_forward bindings ausentes");
    }

    private static void Pass(string msg) => GD.Print($"OK: {msg}");

    private void Fail(string msg)
    {
        GD.PrintErr($"FAIL: {msg}");
        _errors++;
    }
}
