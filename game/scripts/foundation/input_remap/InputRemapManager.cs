// InputRemapManager.cs
//
// AutoLoad global de input remap. CONTRACT §6 Gate 1 a11y D1.
//
// API public:
//   - LoadConfig() / SaveConfig() persist user://config.json
//   - GetBindingsForAction(actionName)
//   - SetBindingsForAction(actionName, ActionBindings)
//   - ApplyToGodotInputMap() : aplica bindings em runtime
//   - ResetActionToDefault(actionName) / ResetAllToDefaults()
//   - DetectConflict(InputEvent, excludeAction) : retorna action conflictante OU null
//   - CaptureNextInputAsync(timeoutSec) : modal capture
//
// Persist: user://config.json (sem HMAC, decisão canon F2-E.7 batch A).
//
// Cross-ref: docs/tech/engine-modules.md §2.4 + CONTRACT.md §6 Gate 1.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Threading.Tasks;
using Godot;
using GusDragon.Engine.Foundation.InputRemap;

namespace GusWorld.Game.Foundation.InputRemap;

/// <summary>
/// Manager AutoLoad de input remap a11y.
/// </summary>
public partial class InputRemapManager : Node
{
    public static InputRemapManager? Instance { get; private set; }

    private const string ConfigPath = "user://config.json";

    private static readonly JsonSerializerOptions s_jsonOptions = new() { WriteIndented = true };

    private InputRemapConfig _currentConfig = new();
    private bool _isCapturing;
    private InputEvent? _capturedEvent;

    [Signal] public delegate void BindingsChangedEventHandler(string actionName);
    [Signal] public delegate void InputCapturedEventHandler();
    [Signal] public delegate void InputCaptureCanceledEventHandler();

    public override void _Ready()
    {
        Instance = this;
        LoadConfig();
        ApplyToGodotInputMap();
    }

    public override void _ExitTree()
    {
        if (Instance == this) Instance = null;
    }

    public override void _UnhandledInput(InputEvent @event)
    {
        if (_isCapturing && IsCapturable(@event))
        {
            _capturedEvent = @event;
            _isCapturing = false;
            GetViewport().SetInputAsHandled();
            EmitSignal(SignalName.InputCaptured);
        }
    }

    private static bool IsCapturable(InputEvent @event)
    {
        return @event is InputEventKey { Pressed: true } ||
               @event is InputEventJoypadButton { Pressed: true } ||
               @event is InputEventMouseButton { Pressed: true } ||
               @event is InputEventJoypadMotion j && Math.Abs(j.AxisValue) > 0.5f;
    }

    /// <summary>Carrega config de user://config.json. Se inexistente, popula com defaults.</summary>
    public void LoadConfig()
    {
        if (!Godot.FileAccess.FileExists(ConfigPath))
        {
            _currentConfig = BuildDefaultConfig();
            return;
        }

        try
        {
            using var file = Godot.FileAccess.Open(ConfigPath, Godot.FileAccess.ModeFlags.Read);
            var json = file?.GetAsText() ?? string.Empty;
            if (string.IsNullOrEmpty(json))
            {
                _currentConfig = BuildDefaultConfig();
                return;
            }

            var loaded = JsonSerializer.Deserialize<InputRemapConfig>(json);
            _currentConfig = loaded ?? BuildDefaultConfig();
        }
        catch (Exception ex)
        {
            GD.PushError($"InputRemapManager.LoadConfig falhou: {ex.Message}. Usando defaults.");
            _currentConfig = BuildDefaultConfig();
        }
    }

    /// <summary>Persiste config em user://config.json.</summary>
    public void SaveConfig()
    {
        try
        {
            var json = JsonSerializer.Serialize(_currentConfig, s_jsonOptions);
            using var file = Godot.FileAccess.Open(ConfigPath, Godot.FileAccess.ModeFlags.Write);
            if (file == null)
                throw new IOException($"Falha abrir {ConfigPath} para write");
            file.StoreString(json);
        }
        catch (Exception ex)
        {
            GD.PushError($"InputRemapManager.SaveConfig falhou: {ex.Message}");
        }
    }

    /// <summary>Constrói config default usando ActionRegistry + DefaultBindings.</summary>
    private static InputRemapConfig BuildDefaultConfig()
    {
        var actions = ActionRegistry.Actions
            .Select(def => DefaultBindings.GetDefault(def.ActionName))
            .Where(b => b != null)
            .Select(b => b!)
            .ToList();
        return new InputRemapConfig { ConfigVersion = 1, Actions = actions };
    }

    /// <summary>Retorna bindings da action OU null se não registrada.</summary>
    public ActionBindings? GetBindingsForAction(string actionName) =>
        _currentConfig.Actions.FirstOrDefault(a => a.ActionName == actionName);

    /// <summary>Substitui bindings da action e re-aplica ao Godot InputMap.</summary>
    public void SetBindingsForAction(ActionBindings bindings)
    {
        ArgumentNullException.ThrowIfNull(bindings);
        var newActions = _currentConfig.Actions
            .Where(a => a.ActionName != bindings.ActionName)
            .Append(bindings)
            .ToList();
        _currentConfig = _currentConfig with { Actions = newActions };
        ApplyActionToGodotInputMap(bindings);
        EmitSignal(SignalName.BindingsChanged, bindings.ActionName);
    }

    /// <summary>Aplica TODAS bindings do _currentConfig ao Godot InputMap.</summary>
    public void ApplyToGodotInputMap()
    {
        foreach (var binding in _currentConfig.Actions)
            ApplyActionToGodotInputMap(binding);
    }

    private static void ApplyActionToGodotInputMap(ActionBindings bindings)
    {
        var actionName = new StringName(bindings.ActionName);

        if (!InputMap.HasAction(actionName))
            InputMap.AddAction(actionName, bindings.Deadzone);
        else
            InputMap.ActionEraseEvents(actionName);

        InputMap.ActionSetDeadzone(actionName, bindings.Deadzone);

        foreach (var k in bindings.Keys)
        {
            var ev = new InputEventKey
            {
                PhysicalKeycode = (Key)k.Keycode,
                CtrlPressed = k.CtrlPressed,
                ShiftPressed = k.ShiftPressed,
                AltPressed = k.AltPressed,
            };
            InputMap.ActionAddEvent(actionName, ev);
        }

        foreach (var b in bindings.GamepadButtons)
        {
            var ev = new InputEventJoypadButton { ButtonIndex = (JoyButton)b.ButtonIndex };
            InputMap.ActionAddEvent(actionName, ev);
        }

        foreach (var m in bindings.MouseButtons)
        {
            var ev = new InputEventMouseButton { ButtonIndex = (MouseButton)m.ButtonIndex };
            InputMap.ActionAddEvent(actionName, ev);
        }

        foreach (var ax in bindings.GamepadAxes)
        {
            var ev = new InputEventJoypadMotion { Axis = (JoyAxis)ax.Axis, AxisValue = ax.AxisValue };
            InputMap.ActionAddEvent(actionName, ev);
        }
    }

    /// <summary>Reset action pra default. Persist + apply.</summary>
    public void ResetActionToDefault(string actionName)
    {
        var def = DefaultBindings.GetDefault(actionName);
        if (def != null) SetBindingsForAction(def);
    }

    /// <summary>Reset TODAS actions pra defaults. Persist + apply.</summary>
    public void ResetAllToDefaults()
    {
        _currentConfig = BuildDefaultConfig();
        ApplyToGodotInputMap();
        SaveConfig();
        EmitSignal(SignalName.BindingsChanged, "*");
    }

    /// <summary>
    /// Detecta se InputEvent já está bound em outra action (excludeAction excluído).
    /// Retorna action name conflictante OU null.
    /// </summary>
    public string? DetectConflict(InputEvent @event, string excludeAction)
    {
        foreach (var binding in _currentConfig.Actions)
        {
            if (binding.ActionName == excludeAction) continue;
            if (BindingMatchesEvent(binding, @event))
                return binding.ActionName;
        }
        return null;
    }

    private static bool BindingMatchesEvent(ActionBindings binding, InputEvent @event)
    {
        if (@event is InputEventKey ek)
        {
            foreach (var k in binding.Keys)
            {
                if (k.Keycode == (long)ek.PhysicalKeycode &&
                    k.CtrlPressed == ek.CtrlPressed &&
                    k.ShiftPressed == ek.ShiftPressed &&
                    k.AltPressed == ek.AltPressed)
                    return true;
            }
        }
        else if (@event is InputEventJoypadButton ej)
        {
            foreach (var b in binding.GamepadButtons)
                if (b.ButtonIndex == (int)ej.ButtonIndex) return true;
        }
        else if (@event is InputEventMouseButton em)
        {
            foreach (var m in binding.MouseButtons)
                if (m.ButtonIndex == (int)em.ButtonIndex) return true;
        }
        else if (@event is InputEventJoypadMotion ea)
        {
            foreach (var ax in binding.GamepadAxes)
                if (ax.Axis == (int)ea.Axis && Math.Sign(ax.AxisValue) == Math.Sign(ea.AxisValue))
                    return true;
        }
        return false;
    }

    /// <summary>
    /// Capture próximo InputEvent. Modal pattern. Timeout em segundos.
    /// Retorna InputEvent capturado OU null se cancel/timeout.
    /// </summary>
    public async Task<InputEvent?> CaptureNextInputAsync(float timeoutSeconds = 5f)
    {
        _isCapturing = true;
        _capturedEvent = null;
        var elapsed = 0f;
        const float pollInterval = 0.05f;

        while (_isCapturing && elapsed < timeoutSeconds)
        {
            await Task.Delay(TimeSpan.FromSeconds(pollInterval));
            elapsed += pollInterval;
        }

        if (_isCapturing)
        {
            // Timeout
            _isCapturing = false;
            EmitSignal(SignalName.InputCaptureCanceled);
            return null;
        }

        return _capturedEvent;
    }

    /// <summary>Cancela capture em andamento.</summary>
    public void CancelCapture()
    {
        if (_isCapturing)
        {
            _isCapturing = false;
            _capturedEvent = null;
            EmitSignal(SignalName.InputCaptureCanceled);
        }
    }

    /// <summary>Retorna config atual (read-only) pra debug ou UI binding inspector.</summary>
    public InputRemapConfig GetCurrentConfig() => _currentConfig;
}
