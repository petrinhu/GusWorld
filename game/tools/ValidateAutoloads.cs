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
using GusWorld.Game.Back.OrbitalCamera;
using GusWorld.Game.Foundation.Buses;
using GusWorld.Game.Foundation.Localization;

namespace GusWorld.Game.Tools;

public partial class ValidateAutoloads : SceneTree
{
    private int _errors;

    public override void _Initialize()
    {
        // SceneTree -s scripts rodam _Initialize antes de AutoLoads _Ready.
        // Force carregamento explícito Localization antes de validar.
        var locEarly = Root.GetNodeOrNull<Localization>("Localization");
        locEarly?.LoadAllLocales();

        GD.Print("=== Validação AutoLoads + i18n ===");
        _errors = 0;

        ValidateGameStateBus();
        ValidatePlayerBus();
        ValidateCombatBus();
        ValidateUIBus();
        ValidateLocalization();
        ValidateMathHelpers();
        ValidateOrbitalCameraClass();

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

    private static void Pass(string msg) => GD.Print($"OK: {msg}");

    private void Fail(string msg)
    {
        GD.PrintErr($"FAIL: {msg}");
        _errors++;
    }
}
