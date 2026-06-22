// controls_hash_test.cpp
//
// Spec executavel (Catch2 v3) do hash 128 de controles (ADR-007 item 2). POCO puro,
// ZERO Qt. O hash = primeiros 16 bytes do SHA-256 (core/crypto) sobre a forma
// CANONICA do controls.json (NAO os bytes crus do arquivo). NAO e seguranca: e
// deteccao casual de edicao manual (chave/algoritmo publicos).
//
// Contrato:
//   - controls_hash128(cfg) -> std::array<uint8_t,16> = sha256(canonico(cfg))[0..15].
//   - Determinismo: mesmo config -> mesmo hash.
//   - Sensivel a troca de tecla (semantica): trocar keycode muda o hash.
//   - INsensivel a reformatacao cosmetica: o hash e sobre o canonico, entao
//     reindentar/reordenar o arquivo a mao NAO muda o hash (decisao do lider, opcao 1).
//   - controls_were_modified(a, b) = (a != b): true sse os hashes diferem.
//
// Cross-ref: docs/tech/adr/ADR-007-controls-json-hash128-save-v4.md (item 2),
//            gus/domain/input/controls_hash.hpp, core/crypto/sha256.hpp.

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <string>

#include "gus/domain/input/controls_hash.hpp"
#include "gus/domain/input/controls_json.hpp"
#include "gus/domain/input/input_binding.hpp"
#include "gus/core/crypto/sha256.hpp"

using namespace gus::domain::input;

namespace {

InputRemapConfig config_a() {
    InputRemapConfig cfg;
    ActionBindings cast;
    cast.action_name = "combat_cast";
    cast.keys = {KeyBinding{.keycode = 32}};  // Espaco
    cfg.actions = {cast};
    return cfg;
}

InputRemapConfig config_b_changed_key() {
    InputRemapConfig cfg = config_a();
    cfg.actions[0].keys[0].keycode = 68;  // D (troca semantica de tecla)
    return cfg;
}

}  // namespace

// ---- tamanho = 128 bits (16 bytes) -----------------------------------------

TEST_CASE("controls_hash: hash tem 16 bytes (128 bits)",
          "[domain][input][controls_hash]") {
    const std::array<std::uint8_t, 16> h = controls_hash128(config_a());
    REQUIRE(h.size() == 16);
}

// ---- determinismo ----------------------------------------------------------

TEST_CASE("controls_hash: mesmo config produz o mesmo hash",
          "[domain][input][controls_hash]") {
    REQUIRE(controls_hash128(config_a()) == controls_hash128(config_a()));
}

// ---- truncamento honesto do SHA-256 sobre o canonico -----------------------

TEST_CASE("controls_hash: e os 16 primeiros bytes do SHA-256 do JSON canonico",
          "[domain][input][controls_hash]") {
    const auto cfg = config_a();
    const std::string canonical = serialize_controls_canonical(cfg);
    const auto full = gus::core::crypto::sha256(
        std::vector<std::uint8_t>(canonical.begin(), canonical.end()));
    const auto h = controls_hash128(cfg);
    for (int i = 0; i < 16; ++i) {
        INFO("byte " << i);
        REQUIRE(h[i] == full[i]);
    }
}

// ---- troca de tecla (semantica) muda o hash --------------------------------

TEST_CASE("controls_hash: trocar tecla muda o hash (era Espaco, esta D)",
          "[domain][input][controls_hash]") {
    REQUIRE(controls_hash128(config_a()) != controls_hash128(config_b_changed_key()));
}

// ---- reformatacao cosmetica NAO muda o hash (hash sobre canonico) ----------

TEST_CASE("controls_hash: reformatar/reordenar NAO muda o hash (canonico)",
          "[domain][input][controls_hash]") {
    // Dois configs semanticamente iguais mas com actions inseridas em ordens
    // diferentes produzem o MESMO hash (a forma canonica reordena pelo registry).
    InputRemapConfig ordem1;
    InputRemapConfig ordem2;
    ActionBindings interact;
    interact.action_name = "interact";
    interact.keys = {KeyBinding{.keycode = 70}};
    ActionBindings cast;
    cast.action_name = "combat_cast";
    cast.keys = {KeyBinding{.keycode = 32}};
    ordem1.actions = {interact, cast};
    ordem2.actions = {cast, interact};  // ordem inversa de insercao
    REQUIRE(controls_hash128(ordem1) == controls_hash128(ordem2));
}

// ---- controls_were_modified -------------------------------------------------

TEST_CASE("controls_were_modified: hashes iguais => nao modificado",
          "[domain][input][controls_hash]") {
    const auto h = controls_hash128(config_a());
    REQUIRE_FALSE(controls_were_modified(h, h));
}

TEST_CASE("controls_were_modified: hashes diferentes => modificado",
          "[domain][input][controls_hash]") {
    const auto a = controls_hash128(config_a());
    const auto b = controls_hash128(config_b_changed_key());
    REQUIRE(controls_were_modified(a, b));
}
