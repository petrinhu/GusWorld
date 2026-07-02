// gus/app/screens/battle_sprite_anim.hpp
//
// SPRITE ANIMADO NA ARENA (M5, W3): POCO 100% testavel SEM SDL/IO. E a troca
// placeholder -> sprite real prevista no battle-anim.md par.1/1.1/3.2: os GANCHOS de
// timing (BattleAnimDirector, W2) ficam INTOCADOS - este modulo so escolhe QUAL
// animacao (clip) tocar a partir da FASE corrente do director, e QUAL frame do clip
// mostrar pelo tempo decorrido. Zero mudanca de motor/pacing.
//
// PECAS:
//   - BattleClipId: os 12 conjuntos de frames do disco (anims/<clip>/f0..fN.png).
//     Nesta onda so Idle/Run/AttackMelee/HurtPhysical TOCAM (melee do Gus); os demais
//     ficam MAPEADOS mas dormentes (cast entra com o COMPILAR; victory com a tela de
//     resultado; ko quando a cena renderizar mortos) - o mapa ja esta pronto.
//   - SpriteClip / ActorSpriteSet: frames como TextureId JA RESOLVIDOS pela casca
//     (mesmo padrao de BattlePortraitSet: struct de handles, cena agnostica de I/O;
//     1 PNG = 1 textura - frames separados no disco, sem atlas nesta onda).
//   - clip_frame_index(): selecao de frame por tempo (loop da a volta; one-shot trava
//     no ultimo frame).
//   - clip_for_kind(): fase do director -> clip. DECISAO DOCUMENTADA (swing na cauda):
//     o MeleeHold dura ~0 frame na pratica (o consumidor resolve o contato e dispara o
//     Return no MESMO update - ver battle_scene.cpp), entao mapear "contato ->
//     attack_melee" nunca apareceria. O golpe anima na CAUDA da aproximacao: quando
//     restam <= kMeleeSwingSeconds da fase Approach, o attack_melee one-shot comeca -
//     anticipation classica - e TERMINA exatamente no contato (floater + hit-react
//     nascem com o soco cravado). Hold segue mapeado pra AttackMelee (o one-shot
//     clampa no ultimo frame, robusto a jitter).
//   - BattleSpriteAnimator: relogio POR ATOR. O reset e keyado na TROCA DE CLIP (nao
//     de fase): Approach-cauda -> Hold mantem AttackMelee e NAO reseta (o soco nao
//     recomeca no contato); Run -> AttackMelee reseta (o swing parte do frame 0).
//
// Pillar 3 (SEM flip): os clips sao DADOS; nenhum frame e espelhado em codigo.
// PERFIS (2026-07-01): o melee ganhou clipes LATERAIS dedicados - run_east (ida,
// encarando o inimigo a direita), attack_melee_east (murro de perfil) e run_west
// (volta; desenho PROPRIO de perfil-esquerda, gerado da rotacao canonica 2_west -
// NAO e espelho do east). Idle/hurt/demais seguem front-facing por ora. Clipe de
// perfil ausente no disco degrada pro front-facing equivalente (clip_fallback) e
// por fim pro Idle - a cena nunca mostra buraco.
//
// TIMING DO MURRO (decisao 2026-07-01): o clipe attack_melee_east do disco tem 9
// frames, mas f6-f8 DERIVAM (o Gus gira pra camera). clip_frame_cap = 6: o loader
// carrega SO f0..f5 (os derivados nem entram na memoria). 6 frames @ 25 fps =
// 0.24s <= janela de swing 0.28s: o one-shot chega no PICO (f5, soco esticado)
// ~0.04s ANTES do contato e CRAVA - segura o pico atraves do contato e do Hold,
// ate o Return trocar pro run_west. Frame derivado e inalcancavel por construcao.
//
// Cross-ref: gus/app/screens/battle_anim.hpp (fases, W2);
//            gus/app/screens/battle_scene.hpp (consumidor: update tick + render);
//            gus/core/asset_paths.hpp (kGusBattleAnimsDir);
//            docs/design/mecanicas/battle-anim.md par.1.1/3.2 (spec da troca).

#ifndef GUS_APP_SCREENS_BATTLE_SPRITE_ANIM_HPP
#define GUS_APP_SCREENS_BATTLE_SPRITE_ANIM_HPP

#include <array>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "gus/app/screens/battle_anim.hpp"        // ActorAnimKind (fase do director)
#include "gus/platform/render2d/i_renderer.hpp"   // TextureId (handle opaco)

namespace gus::app::screens {

// Os 12 conjuntos de frames de batalha do disco (anims/<pasta>/f0..fN.png). A ordem e
// so indexacao interna (nao e contrato com o disco - o NOME da pasta e, via
// clip_dir_name).
enum class BattleClipId : int {
    Idle = 0,       // battle_idle (repouso em combate; loop)
    Run,            // run (ida E volta do melee; loop)
    AttackMelee,    // attack_melee (o golpe; one-shot, termina no contato)
    Cast,           // cast (DORMANTE: entra com o COMPILAR)
    Defend,         // defend (DORMANTE)
    HurtPhysical,   // hurt_physical (hit-react de golpe; one-shot)
    HurtMagic,      // hurt_magic (DORMANTE: hit-react de carta quando o canal existir)
    KO,             // ko (DORMANTE: a cena so desenha vivos hoje)
    Revive,         // revive (DORMANTE)
    Victory,        // victory (DORMANTE: tela de resultado)
    DragonVictory,  // dragon_victory (DORMANTE: mecanica epica, canon Dragon Victory)
    BreathingIdle,  // breathing_idle (DORMANTE: idle alternativo/overworld)
    // Perfis laterais (2026-07-01; desenhos DISTINTOS por direcao - Pillar 3 nativo):
    RunEast,         // run_east (ida do melee: corre de perfil ENCARANDO o inimigo)
    RunWest,         // run_west (volta do melee: perfil-esquerda proprio, sem espelho)
    AttackMeleeEast, // attack_melee_east (murro de perfil; one-shot com frame cap 6)
    Count
};
inline constexpr int kBattleClipCount = static_cast<int>(BattleClipId::Count);

// Janela do SWING (s): quando restam <= isto da fase Approach, o attack_melee comeca
// (e termina exatamente no contato). = duracao natural do clip (7 frames @ 25 fps).
inline constexpr float kMeleeSwingSeconds = 0.28f;

// Escala do quad do sprite em relacao ao SLOT da arena (54px): o corpo-inteiro chibi
// precisa ler MAIOR que os retratos-busto vizinhos (o conteudo util do canvas 256 e
// ~88% da altura). Ajustavel em playtest (decisao documentada; ver W3).
inline constexpr float kActorSpriteScale = 1.55f;

// Nome da PASTA do clip no disco (anims/<nome>/f0..fN.png). Contrato com o asset.
[[nodiscard]] std::string_view clip_dir_name(BattleClipId id) noexcept;

// fps default do clip (cadencia escolhida por feel; ver tabela no .cpp).
[[nodiscard]] float default_clip_fps(BattleClipId id) noexcept;

// true = loop (idle/run/victory); false = one-shot que TRAVA no ultimo frame.
[[nodiscard]] bool default_clip_loop(BattleClipId id) noexcept;

// Maximo de frames a CARREGAR do disco (0 = sem cap, carrega f0..fN inteiro).
// So attack_melee_east capa (6): f6-f8 derivam (o Gus gira pra camera) e ficam
// FORA da memoria - nunca renderizam (ver decisao de timing no topo).
[[nodiscard]] int clip_frame_cap(BattleClipId id) noexcept;

// Degrau de fallback quando o clip nao tem frames no disco: direcional -> o
// front-facing equivalente (run_east/run_west -> run; attack_melee_east ->
// attack_melee); demais -> Idle (ultimo degrau; o consumidor ainda tenta Idle
// explicitamente e por fim degrada pro retrato placeholder).
[[nodiscard]] BattleClipId clip_fallback(BattleClipId id) noexcept;

// Um clip: frames (TextureId ja resolvidos; vazio = clip ausente no disco) + cadencia.
struct SpriteClip {
    std::vector<gus::platform::render2d::TextureId> frames;
    float fps = 10.0f;
    bool loop = true;

    [[nodiscard]] bool valid() const noexcept { return !frames.empty(); }
};

// Conjunto de clips de UM ator (mesmo padrao de BattlePortraitSet: handles prontos,
// cena agnostica de I/O). Clip sem frames = ausente (o consumidor degrada: Idle ->
// retrato placeholder).
struct ActorSpriteSet {
    std::array<SpriteClip, kBattleClipCount> clips{};

    // O clip, ou nullptr se ausente/vazio (o caller escolhe o fallback).
    [[nodiscard]] const SpriteClip* find(BattleClipId id) const noexcept;

    // true se QUALQUER clip tem frames (o set vale a pena ser instalado).
    [[nodiscard]] bool any() const noexcept;
};

// Frame corrente de um clip pelo tempo decorrido (s). PURA: loop da a volta
// (elapsed*fps % n); one-shot clampa no ultimo frame. Entradas degeneradas
// (n <= 1, fps <= 0, elapsed <= 0) devolvem 0 (seguro).
[[nodiscard]] int clip_frame_index(int frame_count, float fps, bool loop,
                                   float elapsed_seconds) noexcept;

// Fase do director -> clip a tocar. remaining_seconds = tempo restante da fase
// corrente (BattleAnimDirector::phase_remaining_seconds); swing_seconds = janela do
// golpe na cauda do Approach (kMeleeSwingSeconds). Ver decisao "swing na cauda" no
// topo do header.
[[nodiscard]] BattleClipId clip_for_kind(ActorAnimKind kind, float remaining_seconds,
                                         float swing_seconds) noexcept;

// Relogio de clip POR ATOR. POCO: o consumidor (BattleScene::update) chama tick() a
// cada frame com o clip que a fase pede; a troca de clip reseta o relogio (o novo
// clip parte do frame 0). playback_for() e a leitura pro render/testes.
class BattleSpriteAnimator {
public:
    struct Playback {
        BattleClipId clip = BattleClipId::Idle;
        float elapsed = 0.0f;  // tempo (s) DENTRO do clip corrente
    };

    // Avanca o relogio do ator: mesmo clip acumula dt; clip DIFERENTE reseta pra 0
    // (o dt do frame da troca nao conta - o novo clip mostra o frame 0 inteiro).
    void tick(const std::string& id, BattleClipId clip, float dt_seconds);

    // Estado corrente (Idle/0.0 pra ator desconhecido: repouso seguro).
    [[nodiscard]] Playback playback_for(const std::string& id) const;

private:
    std::unordered_map<std::string, Playback> state_;
};

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_SPRITE_ANIM_HPP
