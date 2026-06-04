// AudioDirector.cs
//
// AutoLoad global "maestro" de áudio (F2-AU.4). SÓ encanamento: toca/silencia e mexe em
// volume de bus. ZERO conteúdo (música/SFX entram depois, audio-designer-composer AU.1-AU.7).
//
// É o 10º AutoLoad. Mora no game project (não na engine) porque é um Node C# e o source-gen
// do Godot exige Node AutoLoad em game/ (padrão F2-S.MIG.1). A matemática reutilizável
// (curva de fade) e o espelho de bus (AudioBusConfig) vivem na engine (POCO testável).
//
// API mínima:
//   void PlaySfx(AudioStream stream, float volumeDb = 0f)   // one-shot, auto-free
//   void PlayMusic(AudioStream stream)                      // troca a track de música (loop)
//   void StopMusic()
//   void SetBusVolume(string busName, float volumeDb)       // duck/crossfade ao vivo
//
// O "abaixar a música em combate" (duck) é SetBusVolume("Music", -X) — quem dirige o tempo
// do fade é o caller (F2-AU.7), aplicando AudioFadeCurve.VolumeAt frame-a-frame. O Director
// só aplica o valor instantâneo: mantém-se thin e sem estado de animação.
//
// HONESTIDADE: tocar som de verdade + os buses resolverem exigem o runtime Godot + o .tres
// (F2-AU.3) carregado. Isto COMPILA e está estruturado; validar PlaySfx/PlayMusic no editor.
//
// Cross-ref: game/default_bus_layout.tres (F2-AU.3);
//            engine/foundation/audio/AudioBusConfig.cs, AudioFadeCurve.cs; TODO F2-AU.4.

using System.Diagnostics.CodeAnalysis;
using Godot;

namespace GusWorld.Game.Foundation.Audio;

/// <summary>
/// Maestro de áudio AutoLoad. Toca SFX one-shot, gerencia 1 track de música e ajusta
/// volume de bus (duck/crossfade). Thin: sem mixagem, sem pooling pesado, sem conteúdo.
/// </summary>
public partial class AudioDirector : Node
{
    public static AudioDirector? Instance { get; private set; }

    /// <summary>Nomes canônicos dos buses (espelham o .tres F2-AU.3). Evita string mágica no caller.</summary>
    public const string BusMaster = "Master";
    public const string BusMusic = "Music";
    public const string BusSfx = "SFX";
    public const string BusUi = "UI";
    public const string BusVoice = "Voice";

    /// <summary>Player dedicado à música (uma track por vez; troca = stop + play).</summary>
    private AudioStreamPlayer? _musicPlayer;

    public override void _Ready()
    {
        Instance = this;

        _musicPlayer = new AudioStreamPlayer
        {
            Name = "MusicPlayer",
            Bus = BusMusic,
        };
        AddChild(_musicPlayer);
    }

    public override void _ExitTree()
    {
        if (Instance == this) Instance = null;
    }

    /// <summary>
    /// Toca um SFX one-shot no bus SFX. Cria um AudioStreamPlayer descartável que se
    /// auto-libera ao terminar (sem pool no VS — anti-OE; pooling entra se o profiler pedir).
    /// </summary>
    /// <param name="stream">Stream do efeito. Null = no-op.</param>
    /// <param name="volumeDb">Ganho do one-shot em dB (default 0 = sem ganho).</param>
    public void PlaySfx(AudioStream? stream, float volumeDb = 0f)
    {
        if (stream is null) return;

        var player = new AudioStreamPlayer
        {
            Stream = stream,
            Bus = BusSfx,
            VolumeDb = volumeDb,
        };
        AddChild(player);
        // Auto-free quando o one-shot termina (evita acúmulo de Nodes mortos).
        player.Finished += player.QueueFree;
        player.Play();
    }

    /// <summary>
    /// Troca a track de música atual pela informada (toca do início). Null = no-op.
    /// Crossfade real é responsabilidade do caller via <see cref="SetBusVolume"/> (F2-AU.7).
    /// </summary>
    public void PlayMusic(AudioStream? stream)
    {
        if (stream is null || _musicPlayer is null) return;

        _musicPlayer.Stream = stream;
        _musicPlayer.Play();
    }

    /// <summary>Para a música atual (se houver). Idempotente.</summary>
    public void StopMusic()
    {
        _musicPlayer?.Stop();
    }

    /// <summary>
    /// Ajusta o volume (em dB) de um bus pelo nome. Base do duck/crossfade: passar o valor
    /// já interpolado por <c>AudioFadeCurve</c>. No-op silencioso se o bus não existe (o
    /// layout vem do .tres F2-AU.3; logamos um aviso pra pegar typo cedo).
    /// </summary>
    /// <param name="busName">Nome do bus (use as constantes Bus*).</param>
    /// <param name="volumeDb">Volume alvo em dB.</param>
    // Instance (não static) por simetria de API: todos os métodos do Director são chamados
    // via AudioDirector.Instance.*; transformar só este em static quebraria o call-site.
    [SuppressMessage("Performance", "CA1822:Mark members as static",
        Justification = "Mantido instance pra simetria da API pública do AutoLoad (Instance.*).")]
    public void SetBusVolume(string busName, float volumeDb)
    {
        var idx = AudioServer.GetBusIndex(busName);
        if (idx < 0)
        {
            GD.PushWarning($"AudioDirector.SetBusVolume: bus '{busName}' não existe no layout.");
            return;
        }
        AudioServer.SetBusVolumeDb(idx, volumeDb);
    }
}
