// controls_diff_test.cpp
//
// Spec executavel (Catch2 v3) do diff de controles (ADR-007 item 4). POCO puro,
// ZERO Qt, ZERO UI. diff_controls(was, now) -> ControlsDiff (lista de BindingChange
// por acao que mudou: action_name + label_i18n_key + was_human + now_human). A
// TRADUCAO final do rotulo e da UI (via label_i18n_key); aqui devolvemos dados +
// chave + uma representacao legivel do binding (ex.: "Espaco" -> "D").
//
// Exemplo do lider: "magia: era Espaco, esta D" =
//   BindingChange{action_name="combat_cast", was_human="Espaco", now_human="D"}.
//
// Contrato:
//   - So entram no diff as actions cuja lista de bindings MUDOU semanticamente.
//   - Ordem do diff = ordem do ActionRegistry.
//   - label_i18n_key vem do ActionRegistry (pra UI traduzir).
//   - was_human/now_human: rotulo legivel do PRIMEIRO key-binding (tabela
//     keycode->rotulo pura); "" quando a action nao tem key-binding.
//   - empty(): true quando nada mudou.
//
// Cross-ref: docs/tech/adr/ADR-007-controls-json-hash128-save-v4.md (item 4),
//            gus/domain/input/controls_diff.hpp, action_registry.hpp.

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "gus/domain/input/controls_diff.hpp"
#include "gus/domain/input/input_binding.hpp"

using namespace gus::domain::input;

namespace {

ActionBindings ab(const std::string& name, long long keycode) {
    ActionBindings a;
    a.action_name = name;
    a.keys = {KeyBinding{.keycode = keycode}};
    return a;
}

}  // namespace

// ---- nada mudou => diff vazio ----------------------------------------------

TEST_CASE("controls_diff: configs iguais produzem diff vazio",
          "[domain][input][controls_diff]") {
    InputRemapConfig cfg;
    cfg.actions = {ab("interact", 70), ab("combat_cast", 32)};
    const auto diff = diff_controls(cfg, cfg);
    REQUIRE(diff.empty());
    REQUIRE(diff.changes.empty());
}

// ---- troca de tecla: "magia: era Espaco, esta D" ---------------------------

TEST_CASE("controls_diff: troca de tecla vira um BindingChange com rotulos",
          "[domain][input][controls_diff]") {
    InputRemapConfig was;
    was.actions = {ab("combat_cast", 32)};  // Espaco
    InputRemapConfig now;
    now.actions = {ab("combat_cast", 68)};  // D

    const auto diff = diff_controls(was, now);
    REQUIRE_FALSE(diff.empty());
    REQUIRE(diff.changes.size() == 1);
    const auto& ch = diff.changes[0];
    REQUIRE(ch.action_name == "combat_cast");
    REQUIRE(ch.label_i18n_key == "ACTION_COMBAT_CAST");
    REQUIRE(ch.was_human == "Espaco");
    REQUIRE(ch.now_human == "D");
}

// ---- so as actions que mudaram entram no diff ------------------------------

TEST_CASE("controls_diff: so as actions alteradas entram (as iguais nao)",
          "[domain][input][controls_diff]") {
    InputRemapConfig was;
    was.actions = {ab("interact", 70), ab("combat_cast", 32)};
    InputRemapConfig now;
    now.actions = {ab("interact", 70), ab("combat_cast", 68)};  // so cast mudou

    const auto diff = diff_controls(was, now);
    REQUIRE(diff.changes.size() == 1);
    REQUIRE(diff.changes[0].action_name == "combat_cast");
}

// ---- ordem do diff = ordem do ActionRegistry -------------------------------

TEST_CASE("controls_diff: mudancas saem na ordem do ActionRegistry",
          "[domain][input][controls_diff]") {
    // interact (13o no registry) vem antes de combat_cast (categoria Combat).
    // Mudamos as duas; o diff deve listar interact antes de combat_cast,
    // independente da ordem nos vetores de entrada.
    InputRemapConfig was;
    was.actions = {ab("combat_cast", 32), ab("interact", 70)};
    InputRemapConfig now;
    now.actions = {ab("combat_cast", 68), ab("interact", 71)};

    const auto diff = diff_controls(was, now);
    REQUIRE(diff.changes.size() == 2);
    REQUIRE(diff.changes[0].action_name == "interact");
    REQUIRE(diff.changes[1].action_name == "combat_cast");
}

// ---- adicionar binding a uma action conta como mudanca ---------------------

TEST_CASE("controls_diff: alterar a lista de bindings (add) conta como mudanca",
          "[domain][input][controls_diff]") {
    InputRemapConfig was;
    was.actions = {ab("interact", 70)};
    InputRemapConfig now;
    now.actions = {ab("interact", 70)};
    now.actions[0].keys.push_back(KeyBinding{.keycode = 13});  // +Enter

    const auto diff = diff_controls(was, now);
    REQUIRE(diff.changes.size() == 1);
    REQUIRE(diff.changes[0].action_name == "interact");
}

// ---- tabela keycode->rotulo: teclas comuns ---------------------------------

TEST_CASE("controls_diff: rotulo humano cobre teclas comuns (letras/espaco)",
          "[domain][input][controls_diff]") {
    InputRemapConfig was;
    was.actions = {ab("interact", 87)};   // W
    InputRemapConfig now;
    now.actions = {ab("interact", 32)};   // Espaco
    const auto diff = diff_controls(was, now);
    REQUIRE(diff.changes.size() == 1);
    REQUIRE(diff.changes[0].was_human == "W");
    REQUIRE(diff.changes[0].now_human == "Espaco");
}

// ---- rotulo de tecla desconhecida = fallback nao-vazio ---------------------

TEST_CASE("controls_diff: keycode sem rotulo conhecido tem fallback legivel",
          "[domain][input][controls_diff]") {
    InputRemapConfig was;
    was.actions = {ab("interact", 70)};            // F
    InputRemapConfig now;
    now.actions = {ab("interact", 999999999LL)};   // keycode exotico
    const auto diff = diff_controls(was, now);
    REQUIRE(diff.changes.size() == 1);
    REQUIRE_FALSE(diff.changes[0].now_human.empty());  // nunca vazio
}
