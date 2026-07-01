// GusEngine/app/tests/battle_anim_test.cpp
//
// Catch2 (headless) do BattleAnimDirector (M5, W2 base funcional): prova o POCO de
// animacao de combate SEM SDL/janela (battle-anim.md par.2/3):
//   - melee: Approach vai de (0,0) ate o displacement (monotonico), vira Hold
//     (melee_arrived), e o Return termina EXATAMENTE em (0,0) - sem drift, mesmo
//     interrompido no meio (robusto a skip/jitter);
//   - hit-react: tranco pra tras (pico no meio) e volta exata a (0,0);
//   - cast + projetil (mecanismo dormante da magia): windup no lugar, bolinha viaja
//     em lerp e o impacto e reportado UMA vez (take_impacts);
//   - leitura segura: ator desconhecido = offset (0,0) / None.

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <string>
#include <vector>

#include "gus/app/screens/battle_anim.hpp"

using gus::app::screens::ActorAnimKind;
using gus::app::screens::BattleAnimDirector;
using gus::app::screens::kHitReactKnockbackPx;
using gus::app::screens::kHitReactSeconds;
using gus::app::screens::kMeleeReturnSeconds;
using gus::core::spatial::Vec2;

namespace {

// Bombeia o diretor por `seconds` em passos de 1/60 (o mesmo dt do host).
void pump(BattleAnimDirector& d, float seconds) {
    const float dt = 1.0f / 60.0f;
    for (float t = 0.0f; t < seconds; t += dt) {
        d.update(dt);
    }
}

constexpr float kEps = 0.0001f;

}  // namespace

TEST_CASE("anim: ator desconhecido esta em repouso (offset zero, kind None)",
          "[battle_anim]") {
    BattleAnimDirector d;
    const Vec2 off = d.offset_for("ninguem");
    REQUIRE(off.x == 0.0f);
    REQUIRE(off.y == 0.0f);
    REQUIRE(d.kind_for("ninguem") == ActorAnimKind::None);
    REQUIRE_FALSE(d.melee_arrived("ninguem"));
    REQUIRE_FALSE(d.melee_in_flight("ninguem"));
    REQUIRE_FALSE(d.any_active());
}

TEST_CASE("anim melee: Approach desloca MONOTONICO de (0,0) ate o displacement",
          "[battle_anim]") {
    BattleAnimDirector d;
    d.start_melee_approach("gus", Vec2{100.0f, -20.0f}, 0.6f);
    REQUIRE(d.kind_for("gus") == ActorAnimKind::MeleeApproach);
    REQUIRE(d.melee_in_flight("gus"));
    REQUIRE_FALSE(d.melee_arrived("gus"));  // ainda NAO chegou (contato nao pode)

    // O offset avanca monotonico em x (sem voltar) e nunca passa do alvo.
    float last_x = 0.0f;
    for (int i = 0; i < 36; ++i) {  // 36 frames = 0.6s
        d.update(1.0f / 60.0f);
        const Vec2 off = d.offset_for("gus");
        REQUIRE(off.x >= last_x - kEps);
        REQUIRE(off.x <= 100.0f + kEps);
        last_x = off.x;
    }
    // Terminou: chegou EXATAMENTE no displacement e virou Hold (contato pode).
    const Vec2 off = d.offset_for("gus");
    REQUIRE(std::fabs(off.x - 100.0f) < kEps);
    REQUIRE(std::fabs(off.y - (-20.0f)) < kEps);
    REQUIRE(d.kind_for("gus") == ActorAnimKind::MeleeHold);
    REQUIRE(d.melee_arrived("gus"));
    REQUIRE(d.melee_in_flight("gus"));
}

TEST_CASE("anim melee: Return volta EXATAMENTE ao repouso (sem drift)",
          "[battle_anim]") {
    BattleAnimDirector d;
    d.start_melee_approach("gus", Vec2{100.0f, 10.0f}, 0.5f);
    pump(d, 0.6f);  // chega (Hold)
    REQUIRE(d.melee_arrived("gus"));

    d.begin_melee_return("gus", kMeleeReturnSeconds);
    REQUIRE(d.kind_for("gus") == ActorAnimKind::MeleeReturn);
    REQUIRE_FALSE(d.melee_arrived("gus"));  // saiu do Hold

    pump(d, kMeleeReturnSeconds + 0.1f);
    const Vec2 off = d.offset_for("gus");
    REQUIRE(off.x == 0.0f);  // repouso EXATO (a fase final crava zero)
    REQUIRE(off.y == 0.0f);
    REQUIRE(d.kind_for("gus") == ActorAnimKind::None);
    REQUIRE_FALSE(d.melee_in_flight("gus"));
    REQUIRE_FALSE(d.any_active());
}

TEST_CASE("anim melee: Return INTERROMPIDO no meio do Approach parte do offset atual",
          "[battle_anim]") {
    // Robustez a skip/jitter (battle-anim.md par.3.2): mesmo que o Approach nao tenha
    // terminado quando o Beat 2 resolve, o Return parte de ONDE o ator esta e ainda
    // termina exatamente em (0,0).
    BattleAnimDirector d;
    d.start_melee_approach("drone", Vec2{-80.0f, 0.0f}, 0.7f);
    pump(d, 0.3f);  // no MEIO do approach
    const Vec2 mid = d.offset_for("drone");
    REQUIRE(mid.x < -kEps);          // ja saiu do lugar
    REQUIRE(mid.x > -80.0f + kEps);  // mas nao chegou

    d.begin_melee_return("drone", 0.3f);
    pump(d, 0.4f);
    const Vec2 off = d.offset_for("drone");
    REQUIRE(off.x == 0.0f);
    REQUIRE(off.y == 0.0f);
    REQUIRE(d.kind_for("drone") == ActorAnimKind::None);
}

TEST_CASE("anim melee: begin_melee_return e NO-OP fora do caminho de melee",
          "[battle_anim]") {
    BattleAnimDirector d;
    // Em repouso: nada acontece.
    d.begin_melee_return("gus", 0.3f);
    REQUIRE(d.kind_for("gus") == ActorAnimKind::None);
    // Em hit-react: NAO interrompe o tranco.
    d.start_hit_react("gus", -1.0f, kHitReactSeconds, kHitReactKnockbackPx);
    d.begin_melee_return("gus", 0.3f);
    REQUIRE(d.kind_for("gus") == ActorAnimKind::HitReact);
}

TEST_CASE("anim melee: ciclos repetidos NAO acumulam drift", "[battle_anim]") {
    BattleAnimDirector d;
    for (int ciclo = 0; ciclo < 5; ++ciclo) {
        d.start_melee_approach("gus", Vec2{60.0f, 8.0f}, 0.4f);
        pump(d, 0.5f);
        d.begin_melee_return("gus", 0.3f);
        pump(d, 0.4f);
        const Vec2 off = d.offset_for("gus");
        REQUIRE(off.x == 0.0f);
        REQUIRE(off.y == 0.0f);
    }
}

TEST_CASE("anim hit-react: tranco pra TRAS com pico no meio e volta exata",
          "[battle_anim]") {
    BattleAnimDirector d;
    // Inimigo (dir +1): recua pra DIREITA.
    d.start_hit_react("drone", +1.0f, kHitReactSeconds, kHitReactKnockbackPx);
    REQUIRE(d.kind_for("drone") == ActorAnimKind::HitReact);

    // No meio da vida o recuo esta perto do PICO (sin(pi/2) = 1).
    pump(d, kHitReactSeconds * 0.5f);
    const Vec2 mid = d.offset_for("drone");
    REQUIRE(mid.x > kHitReactKnockbackPx * 0.8f);
    REQUIRE(mid.x <= kHitReactKnockbackPx + kEps);
    REQUIRE(mid.y == 0.0f);  // tranco e horizontal

    // Termina EXATAMENTE em repouso.
    pump(d, kHitReactSeconds);
    const Vec2 off = d.offset_for("drone");
    REQUIRE(off.x == 0.0f);
    REQUIRE(off.y == 0.0f);
    REQUIRE(d.kind_for("drone") == ActorAnimKind::None);
}

TEST_CASE("anim hit-react: party recua pra ESQUERDA (dir -1)", "[battle_anim]") {
    BattleAnimDirector d;
    d.start_hit_react("caua", -1.0f, kHitReactSeconds, kHitReactKnockbackPx);
    pump(d, kHitReactSeconds * 0.5f);
    REQUIRE(d.offset_for("caua").x < -kHitReactKnockbackPx * 0.8f);
}

TEST_CASE("anim cast: conjura NO LUGAR (sem deslocamento horizontal) e termina",
          "[battle_anim]") {
    BattleAnimDirector d;
    d.start_cast("jaci", 0.5f);
    REQUIRE(d.is_casting("jaci"));
    REQUIRE(d.kind_for("jaci") == ActorAnimKind::CastWindup);

    // No meio: NAO se desloca em x (conjura no lugar); o bob e so vertical e sutil.
    pump(d, 0.25f);
    const Vec2 mid = d.offset_for("jaci");
    REQUIRE(mid.x == 0.0f);
    REQUIRE(mid.y <= 0.0f);  // bob pra CIMA (eixo +Y e pra baixo)

    pump(d, 0.35f);
    REQUIRE_FALSE(d.is_casting("jaci"));
    REQUIRE(d.kind_for("jaci") == ActorAnimKind::None);
    REQUIRE(d.offset_for("jaci").x == 0.0f);
    REQUIRE(d.offset_for("jaci").y == 0.0f);
}

TEST_CASE("anim projetil: viaja em lerp e reporta o impacto UMA vez",
          "[battle_anim]") {
    BattleAnimDirector d;
    d.spawn_projectile("drone", 100.0f, 200.0f, 300.0f, 240.0f, 0.4f);
    REQUIRE(d.projectiles().size() == 1);
    REQUIRE(d.any_active());

    // No meio da viagem: posicao entre origem e destino (lerp), avancando.
    pump(d, 0.2f);
    REQUIRE(d.projectiles().size() == 1);
    const Vec2 p = d.projectiles()[0].position();
    REQUIRE(p.x > 100.0f + kEps);
    REQUIRE(p.x < 300.0f - kEps);
    REQUIRE(p.y > 200.0f + kEps);
    REQUIRE(p.y < 240.0f - kEps);
    REQUIRE(d.take_impacts().empty());  // ainda NAO chegou

    // Chega: sai da lista de voo e o alvo aparece em take_impacts UMA vez.
    pump(d, 0.3f);
    REQUIRE(d.projectiles().empty());
    const std::vector<std::string> hits = d.take_impacts();
    REQUIRE(hits.size() == 1);
    REQUIRE(hits[0] == "drone");
    REQUIRE(d.take_impacts().empty());  // drenado: nao reporta de novo
}

TEST_CASE("anim: uma anim POR ATOR (a nova substitui a corrente); atores independentes",
          "[battle_anim]") {
    BattleAnimDirector d;
    d.start_melee_approach("gus", Vec2{50.0f, 0.0f}, 0.4f);
    d.start_hit_react("drone", +1.0f, kHitReactSeconds, kHitReactKnockbackPx);
    pump(d, 0.1f);
    // Os dois animam ao mesmo tempo, cada um no seu estado.
    REQUIRE(d.kind_for("gus") == ActorAnimKind::MeleeApproach);
    REQUIRE(d.kind_for("drone") == ActorAnimKind::HitReact);
    // Novo hit-react no gus SUBSTITUI o melee dele (uma anim por ator).
    d.start_hit_react("gus", -1.0f, kHitReactSeconds, kHitReactKnockbackPx);
    REQUIRE(d.kind_for("gus") == ActorAnimKind::HitReact);
    REQUIRE_FALSE(d.melee_in_flight("gus"));
}
