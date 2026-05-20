// DefaultBindings.cs
//
// Defaults canônicos pra cada action do ActionRegistry.
// WASD + arrows keyboard + standard gamepad (Xbox layout).
//
// Cross-ref: ActionRegistry.cs (lista canônica de actions).

using Godot;
using GusDragon.Engine.Foundation.InputRemap;

namespace GusWorld.Game.Foundation.InputRemap;

/// <summary>
/// Defaults canon pra todas actions. Static.
/// </summary>
public static class DefaultBindings
{
    /// <summary>Retorna default binding pra actionName OU null se não tem default registrado.</summary>
    public static ActionBindings? GetDefault(string actionName) => actionName switch
    {
        // §1. Movement
        "move_forward" => MakeKeyAxis(actionName, Key.W, Key.Up, JoyAxis.LeftY, -1f),
        "move_backward" => MakeKeyAxis(actionName, Key.S, Key.Down, JoyAxis.LeftY, 1f),
        "move_left" => MakeKeyAxis(actionName, Key.A, Key.Left, JoyAxis.LeftX, -1f),
        "move_right" => MakeKeyAxis(actionName, Key.D, Key.Right, JoyAxis.LeftX, 1f),
        "move_run" => MakeKeyButton(actionName, Key.Shift, JoyButton.LeftStick),

        // §2. Camera (defaults já em project.godot F2-E.1, replicados aqui pra consistency)
        "camera_rotate_left" => MakeKeyAxis(actionName, Key.Q, null, JoyAxis.RightX, -1f),
        "camera_rotate_right" => MakeKeyAxis(actionName, Key.E, null, JoyAxis.RightX, 1f),
        "camera_zoom_in" => MakeMouseGamepad(actionName, MouseButton.WheelUp, JoyButton.DpadUp),
        "camera_zoom_out" => MakeMouseGamepad(actionName, MouseButton.WheelDown, JoyButton.DpadDown),
        "camera_pitch_up" => MakeKeyAxis(actionName, Key.Pageup, null, JoyAxis.RightY, -1f),
        "camera_pitch_down" => MakeKeyAxis(actionName, Key.Pagedown, null, JoyAxis.RightY, 1f),
        "camera_reset_view" => MakeKeyButton(actionName, Key.R, JoyButton.RightShoulder),

        // §3. Interact
        "interact" => MakeKeyButton(actionName, Key.F, JoyButton.A),

        // §4. Menu
        "menu_open" => MakeKeyButton(actionName, Key.Escape, JoyButton.Start),
        "menu_close" => MakeKeyButton(actionName, Key.Escape, JoyButton.B),
        "menu_confirm" => MakeKeyButton(actionName, Key.Enter, JoyButton.A),
        "menu_cancel" => MakeKeyButton(actionName, Key.Escape, JoyButton.B),
        "menu_nav_up" => MakeKeyButton(actionName, Key.Up, JoyButton.DpadUp),
        "menu_nav_down" => MakeKeyButton(actionName, Key.Down, JoyButton.DpadDown),
        "menu_nav_left" => MakeKeyButton(actionName, Key.Left, JoyButton.DpadLeft),
        "menu_nav_right" => MakeKeyButton(actionName, Key.Right, JoyButton.DpadRight),

        // §5. Combat
        "combat_attack_basic" => MakeKeyButton(actionName, Key.Space, JoyButton.A),
        "combat_defend" => MakeKeyButton(actionName, Key.Shift, JoyButton.RightShoulder),
        "combat_cast" => MakeKeyButton(actionName, Key.C, JoyButton.X),
        "combat_card_1" => MakeKeyButton(actionName, Key.Key1, JoyButton.DpadUp),
        "combat_card_2" => MakeKeyButton(actionName, Key.Key2, JoyButton.DpadRight),
        "combat_card_3" => MakeKeyButton(actionName, Key.Key3, JoyButton.DpadDown),
        "combat_end_turn" => MakeKeyButton(actionName, Key.Enter, JoyButton.Start),

        // §6. Dialogue
        "dialogue_continue" => MakeKeyButton(actionName, Key.Space, JoyButton.A),
        "dialogue_skip" => MakeKeyButton(actionName, Key.Tab, JoyButton.Y),
        "dialogue_choice_1" => MakeKeyButton(actionName, Key.Key1, JoyButton.A),
        "dialogue_choice_2" => MakeKeyButton(actionName, Key.Key2, JoyButton.B),
        "dialogue_choice_3" => MakeKeyButton(actionName, Key.Key3, JoyButton.X),
        "dialogue_choice_4" => MakeKeyButton(actionName, Key.Key4, JoyButton.Y),

        // §7. Inventory
        "inventory_open" => MakeKeyButton(actionName, Key.I, JoyButton.Y),
        "inventory_close" => MakeKeyButton(actionName, Key.I, JoyButton.B),

        // §8. Diary
        "diary_open" => MakeKeyButton(actionName, Key.J, JoyButton.Back),

        _ => null,
    };

    private static ActionBindings MakeKeyAxis(string action, Key key1, Key? key2, JoyAxis axis, float axisValue)
    {
        var bindings = new ActionBindings
        {
            ActionName = action,
            Deadzone = 0.5f,
        };
        bindings.Keys.Add(new KeyBinding { Keycode = (long)key1 });
        if (key2.HasValue) bindings.Keys.Add(new KeyBinding { Keycode = (long)key2.Value });
        bindings.GamepadAxes.Add(new GamepadAxisBinding { Axis = (int)axis, AxisValue = axisValue });
        return bindings;
    }

    private static ActionBindings MakeKeyButton(string action, Key key, JoyButton button)
    {
        var bindings = new ActionBindings
        {
            ActionName = action,
            Deadzone = 0.5f,
        };
        bindings.Keys.Add(new KeyBinding { Keycode = (long)key });
        bindings.GamepadButtons.Add(new GamepadButtonBinding { ButtonIndex = (int)button });
        return bindings;
    }

    private static ActionBindings MakeMouseGamepad(string action, MouseButton mouse, JoyButton button)
    {
        var bindings = new ActionBindings
        {
            ActionName = action,
            Deadzone = 0.5f,
        };
        bindings.MouseButtons.Add(new MouseButtonBinding { ButtonIndex = (int)mouse });
        bindings.GamepadButtons.Add(new GamepadButtonBinding { ButtonIndex = (int)button });
        return bindings;
    }
}
