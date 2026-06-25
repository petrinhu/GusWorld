// GusEngine/app/tests/battle_floaters_test.cpp
//
// Catch2 (headless) do MODELO PURO dos numeros flutuantes de dano (M5, incremento 5).
// Prova, SEM SDL: a vida/posicao/alpha do floater ao longo do tempo, a cor por CANAL
// (combat.md par.11: COMUM/CRIT/FALHA/CURA), e o parser do resultado do golpe a partir
// da message + value que o motor ja acumula (sufixos [CRITICO]/FALHA DE COMPILACAO).

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "gus/app/screens/battle_floaters.hpp"

using Catch::Matchers::WithinAbs;
using gus::app::screens::floater_alive;
using gus::app::screens::floater_alpha;
using gus::app::screens::floater_color_for_channel;
using gus::app::screens::floater_offset_y;
using gus::app::screens::HitChannel;
using gus::app::screens::HitResult;
using gus::app::screens::kFloaterLifeSeconds;
using gus::app::screens::parse_hit;

TEST_CASE("floater vive ~700ms e morre depois", "[battle_floaters]") {
    REQUIRE(floater_alive(0.0f));
    REQUIRE(floater_alive(kFloaterLifeSeconds * 0.5f));
    REQUIRE_FALSE(floater_alive(kFloaterLifeSeconds + 0.01f));
    REQUIRE_FALSE(floater_alive(-0.1f));  // idade negativa = nao vivo
}

TEST_CASE("floater SOBE com a idade (offset_y cresce pra cima = negativo)",
          "[battle_floaters]") {
    const float y0 = floater_offset_y(0.0f);
    const float y_mid = floater_offset_y(kFloaterLifeSeconds * 0.5f);
    const float y_end = floater_offset_y(kFloaterLifeSeconds);
    REQUIRE_THAT(y0, WithinAbs(0.0f, 1e-4f));  // comeca sem deslocamento
    REQUIRE(y_mid < y0);   // subiu (y pra cima e negativo no nosso eixo +Y baixo)
    REQUIRE(y_end < y_mid);  // continua subindo
}

TEST_CASE("floater faz FADE (alpha 1 -> 0)", "[battle_floaters]") {
    REQUIRE_THAT(floater_alpha(0.0f), WithinAbs(1.0f, 1e-4f));
    REQUIRE(floater_alpha(kFloaterLifeSeconds * 0.5f) < 1.0f);
    REQUIRE(floater_alpha(kFloaterLifeSeconds * 0.5f) > 0.0f);
    REQUIRE_THAT(floater_alpha(kFloaterLifeSeconds), WithinAbs(0.0f, 0.05f));
    // Fora da vida, alpha clampa em 0 (nao negativo).
    REQUIRE(floater_alpha(kFloaterLifeSeconds * 2.0f) >= 0.0f);
}

TEST_CASE("cor por canal: COMUM claro, CRIT ciano, FALHA vermelho-erro, CURA verde",
          "[battle_floaters]") {
    const auto common = floater_color_for_channel(HitChannel::Common);
    const auto crit = floater_color_for_channel(HitChannel::Crit);
    const auto fail = floater_color_for_channel(HitChannel::Fail);
    const auto heal = floater_color_for_channel(HitChannel::Heal);
    // COMUM ~ branco/claro (todos os canais altos).
    REQUIRE(common.r > 0.8f);
    REQUIRE(common.g > 0.8f);
    REQUIRE(common.b > 0.8f);
    // CRIT ciano: azul/verde altos, vermelho baixo.
    REQUIRE(crit.b > 0.7f);
    REQUIRE(crit.r < crit.b);
    // FALHA vermelho-erro #F43F5E: vermelho dominante.
    REQUIRE(fail.r > 0.8f);
    REQUIRE(fail.r > fail.g);
    // CURA verde: verde dominante.
    REQUIRE(heal.g > 0.6f);
    REQUIRE(heal.g > heal.r);
}

TEST_CASE("parse_hit le valor + canal do que o motor produz", "[battle_floaters]") {
    // Hit comum: value 25, sem sufixo de canal.
    const HitResult common = parse_hit("caua ataca inimigo3 por 25.", 25);
    REQUIRE(common.channel == HitChannel::Common);
    REQUIRE(common.value == 25);
    REQUIRE(common.text == "25");

    // Critico: sufixo [CRITICO].
    const HitResult crit = parse_hit("gus compila X em alvo por 40. [CRITICO]", 40);
    REQUIRE(crit.channel == HitChannel::Crit);
    REQUIRE(crit.value == 40);

    // Falha de compilacao: dano 0.
    const HitResult fail =
        parse_hit("gus compila X em alvo por 0. FALHA DE COMPILACAO", 0);
    REQUIRE(fail.channel == HitChannel::Fail);
    REQUIRE(fail.text == "FALHA");  // texto especial (nao "0")
}

TEST_CASE("parse_hit: cura vem com sinal + e canal Heal", "[battle_floaters]") {
    // O caller marca cura passando is_heal=true (o motor nao sufixa cura; a deteccao
    // vem do tipo de acao/efeito). value positivo => "+N".
    const HitResult heal = parse_hit("jaci cura gus por 12.", 12, /*is_heal=*/true);
    REQUIRE(heal.channel == HitChannel::Heal);
    REQUIRE(heal.text == "+12");
}
