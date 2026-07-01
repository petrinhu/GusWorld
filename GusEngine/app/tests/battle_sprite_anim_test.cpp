// GusEngine/app/tests/battle_sprite_anim_test.cpp
//
// Catch2 (headless) do POCO de SPRITE ANIMADO na arena (M5, W3 - a troca
// placeholder -> sprite real prevista no battle-anim.md par.1/1.1/3.2). Prova a
// LOGICA com clips SINTETICOS (handles de textura falsos): nenhum teste depende de
// PNG no disco (CI/headless seguro):
//   - clip_frame_index: loop da a volta; one-shot TRAVA no ultimo frame; entradas
//     degeneradas devolvem 0 (seguro);
//   - clip_for_kind: mapa fase-do-director -> clip, incluindo o SWING no fim do
//     Approach (o attack_melee one-shot termina EXATAMENTE no contato - decisao
//     documentada no header: MeleeHold dura ~0 frame, entao o golpe anima na
//     cauda da aproximacao);
//   - BattleSpriteAnimator: relogio por ator; troca de CLIP reseta o relogio;
//     mesmo clip atravessando fases (Approach-cauda -> Hold) NAO reseta (o soco
//     nao recomeca no contato); ator desconhecido = Idle no frame 0;
//   - defaults por clip (fps/loop) sao sadios e o nome de pasta casa o disco.

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "gus/app/screens/battle_sprite_anim.hpp"

using gus::app::screens::ActorAnimKind;
using gus::app::screens::ActorSpriteSet;
using gus::app::screens::BattleClipId;
using gus::app::screens::BattleSpriteAnimator;
using gus::app::screens::clip_dir_name;
using gus::app::screens::clip_for_kind;
using gus::app::screens::clip_frame_index;
using gus::app::screens::default_clip_fps;
using gus::app::screens::default_clip_loop;
using gus::app::screens::kBattleClipCount;
using gus::app::screens::kMeleeSwingSeconds;

// ---------------------------------------------------------------------------
// clip_frame_index (selecao de frame por tempo, pura)
// ---------------------------------------------------------------------------

TEST_CASE("sprite: loop da a volta (wrap) e comeca no frame 0", "[battle_sprite]") {
    // 7 frames @ 10 fps => 1 frame a cada 0.1s; ciclo completo em 0.7s.
    REQUIRE(clip_frame_index(7, 10.0f, /*loop=*/true, 0.0f) == 0);
    REQUIRE(clip_frame_index(7, 10.0f, true, 0.05f) == 0);
    REQUIRE(clip_frame_index(7, 10.0f, true, 0.15f) == 1);
    REQUIRE(clip_frame_index(7, 10.0f, true, 0.65f) == 6);
    // Deu a volta: 0.71s ~ frame 7 -> wrap pro 0; 1.05s -> frame 10 % 7 = 3.
    REQUIRE(clip_frame_index(7, 10.0f, true, 0.71f) == 0);
    REQUIRE(clip_frame_index(7, 10.0f, true, 1.05f) == 3);
}

TEST_CASE("sprite: one-shot TRAVA no ultimo frame (nao recomeca)", "[battle_sprite]") {
    REQUIRE(clip_frame_index(7, 10.0f, /*loop=*/false, 0.65f) == 6);
    REQUIRE(clip_frame_index(7, 10.0f, false, 0.71f) == 6);   // passou do fim: trava
    REQUIRE(clip_frame_index(7, 10.0f, false, 10.0f) == 6);   // muito depois: trava
    REQUIRE(clip_frame_index(5, 16.0f, false, 99.0f) == 4);
}

TEST_CASE("sprite: entradas degeneradas devolvem frame 0 (seguro)", "[battle_sprite]") {
    REQUIRE(clip_frame_index(0, 10.0f, true, 1.0f) == 0);    // sem frames
    REQUIRE(clip_frame_index(-3, 10.0f, true, 1.0f) == 0);   // contagem invalida
    REQUIRE(clip_frame_index(1, 10.0f, true, 5.0f) == 0);    // 1 frame = estatico
    REQUIRE(clip_frame_index(7, 0.0f, true, 1.0f) == 0);     // fps invalido
    REQUIRE(clip_frame_index(7, -5.0f, true, 1.0f) == 0);
    REQUIRE(clip_frame_index(7, 10.0f, true, -0.5f) == 0);   // tempo negativo
}

// ---------------------------------------------------------------------------
// clip_for_kind (fase do BattleAnimDirector -> clip)
// ---------------------------------------------------------------------------

TEST_CASE("sprite: repouso (None) toca battle_idle", "[battle_sprite]") {
    REQUIRE(clip_for_kind(ActorAnimKind::None, 0.0f, kMeleeSwingSeconds) ==
            BattleClipId::Idle);
}

TEST_CASE("sprite: Approach LONGE do contato corre (run); a CAUDA vira o SWING",
          "[battle_sprite]") {
    // Longe do contato (restam 0.5s > swing 0.28s): correndo.
    REQUIRE(clip_for_kind(ActorAnimKind::MeleeApproach, 0.5f, kMeleeSwingSeconds) ==
            BattleClipId::Run);
    // Cauda da aproximacao (restam <= swing): o attack_melee one-shot comeca AGORA,
    // pra TERMINAR exatamente no contato (MeleeHold dura ~0 frame - o consumidor
    // resolve e pede o Return no mesmo update; ver header).
    REQUIRE(clip_for_kind(ActorAnimKind::MeleeApproach, kMeleeSwingSeconds,
                          kMeleeSwingSeconds) == BattleClipId::AttackMelee);
    REQUIRE(clip_for_kind(ActorAnimKind::MeleeApproach, 0.1f, kMeleeSwingSeconds) ==
            BattleClipId::AttackMelee);
}

TEST_CASE("sprite: Hold = attack_melee; Return = run; HitReact = hurt_physical; "
          "Cast = cast",
          "[battle_sprite]") {
    REQUIRE(clip_for_kind(ActorAnimKind::MeleeHold, 0.0f, kMeleeSwingSeconds) ==
            BattleClipId::AttackMelee);
    REQUIRE(clip_for_kind(ActorAnimKind::MeleeReturn, 0.3f, kMeleeSwingSeconds) ==
            BattleClipId::Run);
    REQUIRE(clip_for_kind(ActorAnimKind::HitReact, 0.2f, kMeleeSwingSeconds) ==
            BattleClipId::HurtPhysical);
    REQUIRE(clip_for_kind(ActorAnimKind::CastWindup, 0.4f, kMeleeSwingSeconds) ==
            BattleClipId::Cast);
}

// ---------------------------------------------------------------------------
// BattleSpriteAnimator (relogio por ator; reset na TROCA de clip)
// ---------------------------------------------------------------------------

TEST_CASE("sprite: ator desconhecido esta em Idle no frame 0", "[battle_sprite]") {
    const BattleSpriteAnimator anim;
    const auto pb = anim.playback_for("ninguem");
    REQUIRE(pb.clip == BattleClipId::Idle);
    REQUIRE(pb.elapsed == 0.0f);
}

TEST_CASE("sprite: tick no MESMO clip acumula o relogio", "[battle_sprite]") {
    BattleSpriteAnimator anim;
    // O 1o tick e a TROCA (instala o clip; o dt do frame da troca nao conta - o
    // frame 0 aparece inteiro). Os seguintes acumulam: 3 ticks = 2 dt.
    anim.tick("gus", BattleClipId::Idle, 1.0f / 60.0f);
    anim.tick("gus", BattleClipId::Idle, 1.0f / 60.0f);
    anim.tick("gus", BattleClipId::Idle, 1.0f / 60.0f);
    const auto pb = anim.playback_for("gus");
    REQUIRE(pb.clip == BattleClipId::Idle);
    REQUIRE(pb.elapsed > 0.03f);
    REQUIRE(pb.elapsed < 0.04f);
}

TEST_CASE("sprite: TROCA de clip reseta o relogio (o novo clip comeca do frame 0)",
          "[battle_sprite]") {
    BattleSpriteAnimator anim;
    for (int i = 0; i < 30; ++i) {
        anim.tick("gus", BattleClipId::Idle, 1.0f / 60.0f);  // 0.5s de idle
    }
    anim.tick("gus", BattleClipId::Run, 1.0f / 60.0f);  // troca de fase: reseta
    const auto pb = anim.playback_for("gus");
    REQUIRE(pb.clip == BattleClipId::Run);
    REQUIRE(pb.elapsed == 0.0f);  // o frame da troca zera (o dt conta a partir do prox)
}

TEST_CASE("sprite: o MESMO clip atravessando fases NAO reseta (soco nao recomeca "
          "no contato)",
          "[battle_sprite]") {
    // A cauda do Approach e o Hold mapeiam AMBOS pra AttackMelee: o one-shot que
    // comecou na cauda continua atravessando o contato sem repetir do zero.
    BattleSpriteAnimator anim;
    anim.tick("gus", BattleClipId::AttackMelee, 1.0f / 60.0f);  // cauda do Approach
    anim.tick("gus", BattleClipId::AttackMelee, 1.0f / 60.0f);  // (fase virou Hold)
    const auto pb = anim.playback_for("gus");
    REQUIRE(pb.clip == BattleClipId::AttackMelee);
    REQUIRE(pb.elapsed > 0.0f);  // NAO zerou
}

TEST_CASE("sprite: atores independentes (relogio por id)", "[battle_sprite]") {
    BattleSpriteAnimator anim;
    anim.tick("gus", BattleClipId::Run, 0.5f);
    anim.tick("caua", BattleClipId::Idle, 0.1f);
    REQUIRE(anim.playback_for("gus").clip == BattleClipId::Run);
    REQUIRE(anim.playback_for("caua").clip == BattleClipId::Idle);
}

// ---------------------------------------------------------------------------
// Defaults por clip + nomes de pasta (casam o disco) + ActorSpriteSet
// ---------------------------------------------------------------------------

TEST_CASE("sprite: defaults sadios pra TODO clip (fps > 0; nome de pasta nao-vazio)",
          "[battle_sprite]") {
    for (int c = 0; c < kBattleClipCount; ++c) {
        const auto id = static_cast<BattleClipId>(c);
        REQUIRE(default_clip_fps(id) > 0.0f);
        REQUIRE_FALSE(clip_dir_name(id).empty());
    }
}

TEST_CASE("sprite: loop/one-shot por clip (idle/run/victory loopam; golpe/dano/ko "
          "travam)",
          "[battle_sprite]") {
    REQUIRE(default_clip_loop(BattleClipId::Idle));
    REQUIRE(default_clip_loop(BattleClipId::Run));
    REQUIRE(default_clip_loop(BattleClipId::Victory));
    REQUIRE_FALSE(default_clip_loop(BattleClipId::AttackMelee));
    REQUIRE_FALSE(default_clip_loop(BattleClipId::HurtPhysical));
    REQUIRE_FALSE(default_clip_loop(BattleClipId::KO));
}

TEST_CASE("sprite: nomes de pasta casam o layout do disco (anims/<clip>/fN.png)",
          "[battle_sprite]") {
    REQUIRE(clip_dir_name(BattleClipId::Idle) == std::string("battle_idle"));
    REQUIRE(clip_dir_name(BattleClipId::Run) == std::string("run"));
    REQUIRE(clip_dir_name(BattleClipId::AttackMelee) == std::string("attack_melee"));
    REQUIRE(clip_dir_name(BattleClipId::HurtPhysical) == std::string("hurt_physical"));
    REQUIRE(clip_dir_name(BattleClipId::Cast) == std::string("cast"));
    REQUIRE(clip_dir_name(BattleClipId::KO) == std::string("ko"));
}

TEST_CASE("sprite: o SWING casa a duracao natural do attack_melee (termina no contato)",
          "[battle_sprite]") {
    // 7 frames @ fps do clip = a janela do swing: o one-shot que comeca quando restam
    // kMeleeSwingSeconds acaba de tocar o ultimo frame EXATAMENTE no contato.
    const float natural =
        7.0f / default_clip_fps(BattleClipId::AttackMelee);
    REQUIRE(kMeleeSwingSeconds > natural - 0.01f);
    REQUIRE(kMeleeSwingSeconds < natural + 0.01f);
}

TEST_CASE("sprite: ActorSpriteSet.find devolve nullptr pra clip vazio", "[battle_sprite]") {
    ActorSpriteSet set;
    REQUIRE(set.find(BattleClipId::Idle) == nullptr);  // sem frames = ausente
    auto& idle = set.clips[static_cast<std::size_t>(BattleClipId::Idle)];
    idle.frames = {1u, 2u, 3u};  // handles sinteticos (POCO: so numeros)
    REQUIRE(set.find(BattleClipId::Idle) != nullptr);
    REQUIRE(set.find(BattleClipId::Run) == nullptr);
    REQUIRE(set.any());
}
