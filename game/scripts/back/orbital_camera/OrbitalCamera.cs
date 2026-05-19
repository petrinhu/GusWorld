// OrbitalCamera.cs
//
// Câmera orbital 3/4 rotacional + zoom (Chrono Trigger reference 3D).
//
// Hierarquia esperada na cena:
//   Node3D (OrbitalCamera, este script)
//   └── SpringArm3D
//       └── Camera3D
//
// Uso típico:
//   1. Adicionar cena OrbitalCamera ou criar manualmente hierarquia acima.
//   2. Assign FollowTarget propriedade pra Node3D do player.
//   3. Assign Config OrbitalCameraConfig.tres (opcional, defaults sensatos).
//
// InputMap actions esperadas (criar via project.godot ou InputMap remap):
//   - camera_rotate_left, camera_rotate_right
//   - camera_zoom_in, camera_zoom_out
//   - camera_pitch_up, camera_pitch_down
//   - camera_reset_view
//
// Cross-ref: docs/tech/engine-modules.md §3.1, ADR-002 batch 7 standalone publishing.

using Godot;
using GusDragon.Engine.Foundation;

namespace GusWorld.Game.Back.OrbitalCamera;

/// <summary>
/// Camera3D orbital 3/4 com SpringArm3D collision-aware.
/// </summary>
public partial class OrbitalCamera : Node3D
{
    /// <summary>Target a seguir (player Node3D normalmente).</summary>
    [Export] public Node3D? FollowTarget { get; set; }

    /// <summary>Configuração de comportamento. Se null, usa defaults.</summary>
    [Export] public OrbitalCameraConfig? Config { get; set; }

    /// <summary>Stop index inicial (0..N-1 conforme Config.RotationStops). 0 = 0° (Norte).</summary>
    [Export(PropertyHint.Range, "0,15,1")] public int InitialStopIndex { get; set; }

    private SpringArm3D? _springArm;
    private Camera3D? _camera;

    // Estado runtime
    private float _yaw; // rotação Y atual em graus
    private float _yawTarget; // alvo snap quando user solta input
    private float _pitch; // pitch atual graus
    private float _pitchTarget; // pitch alvo
    private float _zoom; // spring length atual
    private bool _isRotatingFree; // user segurando input rotação?
    private bool _isPitchingFree; // user segurando input pitch?
    private bool _isResetAnimating;
    private float _resetTime;

    public override void _Ready()
    {
        Config ??= new OrbitalCameraConfig();

        _springArm = GetNodeOrNull<SpringArm3D>("SpringArm3D");
        if (_springArm == null)
        {
            GD.PushError("OrbitalCamera: SpringArm3D child node não encontrado");
            return;
        }
        _camera = _springArm.GetNodeOrNull<Camera3D>("Camera3D");
        if (_camera == null)
        {
            GD.PushError("OrbitalCamera: Camera3D grandchild não encontrado");
            return;
        }

        // Setup SpringArm
        _springArm.Margin = Config.SpringMargin;
        _springArm.SpringLength = Config.ZoomDefault;
        _zoom = Config.ZoomDefault;

        // Setup inicial yaw + pitch
        var stopSize = 360f / Config.RotationStops;
        _yaw = InitialStopIndex * stopSize;
        _yawTarget = _yaw;
        _pitch = Config.PitchDefaultDegrees;
        _pitchTarget = _pitch;

        ApplyTransform();
    }

    public override void _Process(double delta)
    {
        if (_springArm == null || Config == null) return;
        var deltaF = (float)delta;

        HandleResetView(deltaF);
        HandleRotationInput(deltaF);
        HandlePitchInput(deltaF);
        HandleZoomInput(deltaF);
        FollowTargetSmooth(deltaF);
        ApplyTransform();
    }

    private void HandleResetView(float delta)
    {
        if (Config == null) return;

        if (Input.IsActionJustPressed("camera_reset_view"))
        {
            _isResetAnimating = true;
            _resetTime = 0f;
            _yawTarget = InitialStopIndex * (360f / Config.RotationStops);
            _pitchTarget = Config.PitchDefaultDegrees;
        }

        if (_isResetAnimating)
        {
            _resetTime += delta;
            var t = _resetTime / Config.ResetAnimationTime;
            _yaw = MathHelpers.LerpAngle(_yaw, _yawTarget, t);
            _pitch = MathHelpers.Lerp(_pitch, _pitchTarget, t);
            _zoom = MathHelpers.Lerp(_zoom, Config.ZoomDefault, t);
            if (t >= 1f)
            {
                _isResetAnimating = false;
                _yaw = _yawTarget;
                _pitch = _pitchTarget;
                _zoom = Config.ZoomDefault;
            }
        }
    }

    private void HandleRotationInput(float delta)
    {
        if (Config == null || _isResetAnimating) return;

        var rotateLeft = Input.IsActionPressed("camera_rotate_left");
        var rotateRight = Input.IsActionPressed("camera_rotate_right");
        var wasRotating = _isRotatingFree;
        _isRotatingFree = rotateLeft || rotateRight;

        if (_isRotatingFree)
        {
            // Livre enquanto segura input
            var direction = (rotateRight ? 1f : 0f) - (rotateLeft ? 1f : 0f);
            _yaw += direction * Config.RotationSpeedDegreesPerSecond * delta;
            _yaw = MathHelpers.NormalizeAngle(_yaw);
            _yawTarget = _yaw;
        }
        else if (wasRotating && Config.RotationSnapEnabled)
        {
            // Acabou de soltar: snap target pro stop mais próximo
            _yawTarget = MathHelpers.SnapToStops(_yaw, Config.RotationStops);
        }
        else if (!_isRotatingFree && Config.RotationSnapEnabled)
        {
            // Lerp pro target snap
            _yaw = MathHelpers.SmoothDamp(_yaw, _yawTarget, Config.SnapReturnTime, delta);
        }
    }

    private void HandlePitchInput(float delta)
    {
        if (Config == null || _isResetAnimating) return;

        var pitchUp = Input.IsActionPressed("camera_pitch_up");
        var pitchDown = Input.IsActionPressed("camera_pitch_down");
        var wasPitching = _isPitchingFree;
        _isPitchingFree = pitchUp || pitchDown;

        if (_isPitchingFree)
        {
            var direction = (pitchUp ? -1f : 0f) + (pitchDown ? 1f : 0f);
            _pitch += direction * Config.PitchSpeedDegreesPerSecond * delta;
            _pitch = System.Math.Clamp(_pitch, Config.PitchMinDegrees, Config.PitchMaxDegrees);
            _pitchTarget = _pitch;
        }
        else if (wasPitching && !Config.PitchFreeMode)
        {
            // Solta input: volta pro default 45°
            _pitchTarget = Config.PitchDefaultDegrees;
        }
        else if (!_isPitchingFree && !Config.PitchFreeMode)
        {
            // Lerp suave pro target
            _pitch = MathHelpers.SmoothDamp(_pitch, _pitchTarget, Config.SnapReturnTime, delta);
        }
    }

    private void HandleZoomInput(float delta)
    {
        if (Config == null || _springArm == null) return;

        var zoomIn = Input.IsActionPressed("camera_zoom_in");
        var zoomOut = Input.IsActionPressed("camera_zoom_out");
        var direction = (zoomOut ? 1f : 0f) - (zoomIn ? 1f : 0f);
        if (direction != 0f)
        {
            _zoom += direction * Config.ZoomSpeed * delta;
            _zoom = System.Math.Clamp(_zoom, Config.ZoomMin, Config.ZoomMax);
        }
        _springArm.SpringLength = _zoom;
    }

    private void FollowTargetSmooth(float delta)
    {
        if (FollowTarget == null || Config == null) return;

        var target = FollowTarget.GlobalPosition;
        var current = GlobalPosition;
        var newPos = new Vector3(
            MathHelpers.SmoothDamp(current.X, target.X, Config.FollowHalfLife, delta),
            MathHelpers.SmoothDamp(current.Y, target.Y, Config.FollowHalfLife, delta),
            MathHelpers.SmoothDamp(current.Z, target.Z, Config.FollowHalfLife, delta)
        );
        GlobalPosition = newPos;
    }

    private void ApplyTransform()
    {
        if (_springArm == null) return;
        // Yaw aplicado no pivot Node3D (este)
        Rotation = new Vector3(0f, Mathf.DegToRad(_yaw), 0f);
        // Pitch aplicado no SpringArm3D (rotaciona ao redor do pivot)
        _springArm.Rotation = new Vector3(Mathf.DegToRad(-_pitch), 0f, 0f);
    }

    /// <summary>
    /// Reset programático (alternativa ao input action). Anima até default state.
    /// </summary>
    public void TriggerResetView()
    {
        _isResetAnimating = true;
        _resetTime = 0f;
        if (Config != null)
        {
            _yawTarget = InitialStopIndex * (360f / Config.RotationStops);
            _pitchTarget = Config.PitchDefaultDegrees;
        }
    }

    /// <summary>Estado atual yaw em graus (debug).</summary>
    public float CurrentYaw => _yaw;

    /// <summary>Estado atual pitch em graus (debug).</summary>
    public float CurrentPitch => _pitch;

    /// <summary>Estado atual zoom em metros (debug).</summary>
    public float CurrentZoom => _zoom;
}
