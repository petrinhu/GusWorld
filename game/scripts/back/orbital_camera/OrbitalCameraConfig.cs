// OrbitalCameraConfig.cs
//
// Resource customizado pra parametrizar OrbitalCamera. Tune via Inspector Godot.
//
// Uso:
//   1. Criar OrbitalCameraConfig.tres no editor (Inspector → New Resource → OrbitalCameraConfig).
//   2. Ajustar valores no Inspector.
//   3. Assign na propriedade Config do OrbitalCamera node.
//
// Cross-ref: docs/tech/engine-modules.md §3.1.

using Godot;

namespace GusWorld.Game.Back.OrbitalCamera;

/// <summary>
/// Configuração de comportamento da OrbitalCamera. Resource customizado tune-able.
/// </summary>
[GlobalClass]
public partial class OrbitalCameraConfig : Resource
{
    /// <summary>Modo rotação horizontal: true = snap em N stops, false = livre contínua.</summary>
    [Export] public bool RotationSnapEnabled { get; set; } = true;

    /// <summary>Número de stops snap (8 = Chrono Trigger canon, 45° cada).</summary>
    [Export(PropertyHint.Range, "2,16,1")] public int RotationStops { get; set; } = 8;

    /// <summary>Tempo lerp pra snap pro stop quando user solta input (segundos).</summary>
    [Export(PropertyHint.Range, "0.05,1.0,0.05")] public float SnapReturnTime { get; set; } = 0.3f;

    /// <summary>Velocidade rotação manual segurando input (graus/segundo).</summary>
    [Export(PropertyHint.Range, "30,360,10")] public float RotationSpeedDegreesPerSecond { get; set; } = 90f;

    /// <summary>Modo pitch vertical: true = livre 30-60°, false = volta 45° suave após soltar.</summary>
    [Export] public bool PitchFreeMode { get; set; } = false;

    /// <summary>Pitch default em graus (45° canon Chrono Trigger).</summary>
    [Export(PropertyHint.Range, "30,60,1")] public float PitchDefaultDegrees { get; set; } = 45f;

    /// <summary>Pitch min em graus (mais vertical).</summary>
    [Export(PropertyHint.Range, "20,45,1")] public float PitchMinDegrees { get; set; } = 30f;

    /// <summary>Pitch max em graus (mais horizontal).</summary>
    [Export(PropertyHint.Range, "45,75,1")] public float PitchMaxDegrees { get; set; } = 60f;

    /// <summary>Velocidade pitch input (graus/segundo).</summary>
    [Export(PropertyHint.Range, "30,180,10")] public float PitchSpeedDegreesPerSecond { get; set; } = 60f;

    /// <summary>Zoom minimum (spring length min em metros).</summary>
    [Export(PropertyHint.Range, "0.5,10.0,0.5")] public float ZoomMin { get; set; } = 2.0f;

    /// <summary>Zoom maximum (spring length max em metros).</summary>
    [Export(PropertyHint.Range, "5.0,30.0,0.5")] public float ZoomMax { get; set; } = 12.0f;

    /// <summary>Zoom default (spring length inicial em metros).</summary>
    [Export(PropertyHint.Range, "1.0,20.0,0.5")] public float ZoomDefault { get; set; } = 6.0f;

    /// <summary>Velocidade zoom (metros/segundo).</summary>
    [Export(PropertyHint.Range, "1.0,20.0,0.5")] public float ZoomSpeed { get; set; } = 5.0f;

    /// <summary>Half-life em segundos pro lerp do follow target (0.15s = canon).</summary>
    [Export(PropertyHint.Range, "0.05,1.0,0.05")] public float FollowHalfLife { get; set; } = 0.15f;

    /// <summary>Tempo animação reset view (segundos).</summary>
    [Export(PropertyHint.Range, "0.1,1.0,0.05")] public float ResetAnimationTime { get; set; } = 0.3f;

    /// <summary>SpringArm3D margin (distância segurança contra clipping).</summary>
    [Export(PropertyHint.Range, "0.01,0.5,0.01")] public float SpringMargin { get; set; } = 0.05f;
}
