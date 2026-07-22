// GusEngine/tests/asset_paths_test.cpp
//
// Catch2 (headless) do header CENTRAL de caminhos de asset (gus/core/asset_paths.hpp).
// Trava a fonte unica da verdade dos sub-caminhos: nenhum esta vazio, sao RELATIVOS (sem
// raiz absoluta nem barra inicial), e a MOVIDA do Gus (sprites/gus ->
// sprites/personagens_inspirados/gus, 2026-06-25) esta refletida. Se uma reorg quebrar um
// caminho, este teste acende (e e o UNICO lugar a corrigir).

#include <catch2/catch_test_macros.hpp>

#include <string_view>

#include "gus/core/asset_paths.hpp"

namespace ap = gus::core::assets;

TEST_CASE("asset_paths: a movida do Gus esta refletida (personagens_inspirados)",
          "[core][assets]") {
    // O Gus saiu de "sprites/gus" pra dentro de "sprites/personagens_inspirados/".
    REQUIRE(ap::kGusSpritesDir == std::string_view("sprites/personagens_inspirados/gus"));
    // A raiz da pasta nova existe como constante e e prefixo do caminho do Gus (e dos
    // demais inspirados em pessoas reais).
    REQUIRE(ap::kGusSpritesDir.rfind(ap::kPersonagensInspiradosDir, 0) == 0);
    REQUIRE(ap::kYakovSpritesDir.rfind(ap::kPersonagensInspiradosDir, 0) == 0);
    // Frames de batalha do Gus (W3): sob a pasta do Gus, em anims/.
    REQUIRE(ap::kGusBattleAnimsDir ==
            std::string_view("sprites/personagens_inspirados/gus/anims"));
    REQUIRE(ap::kGusBattleAnimsDir.rfind(ap::kGusSpritesDir, 0) == 0);
    REQUIRE(ap::kPyotorVanceSpritesDir.rfind(ap::kPersonagensInspiradosDir, 0) == 0);
    REQUIRE(ap::kBrunusVetorialSpritesDir.rfind(ap::kPersonagensInspiradosDir, 0) == 0);
}

TEST_CASE("asset_paths: companions continuam na RAIZ de sprites/ (nao moveram)",
          "[core][assets]") {
    // So o Gus (e os inspirados) foram pra personagens_inspirados. Os companions ficam
    // em sprites/<nome> direto.
    REQUIRE(ap::kCauaSpritesDir == std::string_view("sprites/caua_volt"));
    REQUIRE(ap::kJaciSpritesDir == std::string_view("sprites/jaci_proxy"));
    REQUIRE(ap::kBentoSpritesDir == std::string_view("sprites/bento_requiem"));
    REQUIRE(ap::kDanteSpritesDir == std::string_view("sprites/dante_grid"));
    REQUIRE(ap::kLindaSpritesDir == std::string_view("sprites/linda_siren"));
    REQUIRE(ap::kIaraSpritesDir == std::string_view("sprites/iara_lumen"));
    // NAO estao sob personagens_inspirados (nao tem esse prefixo).
    REQUIRE(ap::kCauaSpritesDir.rfind(ap::kPersonagensInspiradosDir, 0) != 0);
}

TEST_CASE("asset_paths: icones de batalha apontam pra icons-m5 (nao mudaram)",
          "[core][assets]") {
    REQUIRE(ap::kRetratosDir == std::string_view("sprites/icons-m5/retratos"));
    REQUIRE(ap::kStatusIconsDir == std::string_view("sprites/icons-m5/status"));
    REQUIRE(ap::kIntentIconsDir == std::string_view("sprites/icons-m5/intent"));
}

TEST_CASE("asset_paths: fonte/mapa/traducao centralizados", "[core][assets]") {
    REQUIRE(ap::kFontsDir == std::string_view("assets/fonts"));
    REQUIRE(ap::kFontMonoRegularFile == std::string_view("PixelOperatorMono.ttf"));
    REQUIRE(ap::kFontMonoBoldFile == std::string_view("PixelOperatorMono-Bold.ttf"));
    REQUIRE(ap::kMapsCompiledDir == std::string_view("assets/maps/compiled"));
    REQUIRE(ap::kDistritosInferioresGmapFile ==
            std::string_view("distritos_inferiores.gmap"));
    REQUIRE(ap::kTranslationsDir == std::string_view("resources/translations"));
    REQUIRE(ap::kTranslationPtBrFile == std::string_view("pt_br.md"));
}

TEST_CASE("asset_paths: SFX do hit (M6 F3, ADR-011) - principal + variante A/B",
          "[core][assets]") {
    REQUIRE(ap::kSfxDir == std::string_view("assets/sfx"));
    REQUIRE(ap::kHitSfxFile == std::string_view("hit_digital_provisorio.wav"));
    REQUIRE(ap::kHitSfxAltFile == std::string_view("hit_digital_alt_provisorio.wav"));
    // Principal e variante sao arquivos DIFERENTES (o A/B so faz sentido assim).
    REQUIRE(ap::kHitSfxFile != ap::kHitSfxAltFile);
}

TEST_CASE("asset_paths: todos os caminhos sao RELATIVOS e nao-vazios",
          "[core][assets]") {
    const std::string_view all[] = {
        ap::kPersonagensInspiradosDir, ap::kGusSpritesDir, ap::kYakovSpritesDir,
        ap::kPyotorVanceSpritesDir, ap::kBrunusVetorialSpritesDir,
        ap::kCauaSpritesDir, ap::kJaciSpritesDir, ap::kBentoSpritesDir,
        ap::kDanteSpritesDir, ap::kLindaSpritesDir, ap::kIaraSpritesDir,
        ap::kRetratosDir, ap::kStatusIconsDir, ap::kIntentIconsDir, ap::kFontsDir,
        ap::kFontMonoRegularFile, ap::kFontMonoBoldFile, ap::kMapsCompiledDir,
        ap::kDistritosInferioresGmapFile, ap::kTranslationsDir,
        ap::kTranslationPtBrFile, ap::kSfxDir, ap::kHitSfxFile, ap::kHitSfxAltFile,
        ap::kVfxBootPixelDir,
    };
    for (std::string_view p : all) {
        REQUIRE_FALSE(p.empty());
        REQUIRE(p.front() != '/');   // relativo (sem raiz absoluta)
        REQUIRE(p.back() != '/');    // sem barra final (o join cuida do separador)
    }
}

TEST_CASE("asset_paths: pasta do boot pixelizado (M7-COSTURA Inc 2c)",
          "[core][assets]") {
    REQUIRE(ap::kVfxBootPixelDir == std::string_view("vfx/boot_pixel"));
}
