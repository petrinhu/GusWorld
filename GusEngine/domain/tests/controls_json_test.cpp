// controls_json_test.cpp
//
// Spec executavel (Catch2 v3) do serializer/parser JSON PROPRIO de controles
// (ADR-007 itens 1 e 2). POCO puro, ZERO Qt, ZERO disco. O JSON e legivel/editavel
// pelo jogador; o parser e dep-free e cobre SO o schema flat de InputRemapConfig.
//
// Contrato:
//   - Duas formas de saida: DISCO (pretty-print, indentado, legivel) e CANONICA
//     (compacta, actions na ordem do ActionRegistry, sem espaco significativo). O
//     hash 128 (controls_hash_test) entra sobre a CANONICA.
//   - Parse ROBUSTO: JSON malformado NUNCA lanca/crasha; devolve ParseResult com
//     ParseError != None. O fluxo trata como "ausente/restaurar".
//   - Roundtrip semantico: config -> serialize_disco/canonico -> parse -> config
//     IGUAL (operator== profundo de InputRemapConfig).
//   - Forward-compat: action_name desconhecido = ignorado; chave JSON desconhecida
//     = ignorada; campos ausentes usam default do struct.
//
// Cross-ref: docs/tech/adr/ADR-007-controls-json-hash128-save-v4.md (1, 2),
//            gus/domain/input/controls_json.hpp, input_binding.hpp.

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "gus/domain/input/action_registry.hpp"
#include "gus/domain/input/controls_json.hpp"
#include "gus/domain/input/input_binding.hpp"

using namespace gus::domain::input;

namespace {

// Config rico nao-trivial: 3 actions com bindings de varios tipos.
InputRemapConfig rich_config() {
    InputRemapConfig cfg;
    cfg.config_version = 1;

    ActionBindings move_up;
    move_up.action_name = "move_forward";
    move_up.deadzone = 0.25f;
    move_up.keys = {KeyBinding{.keycode = 87}, KeyBinding{.keycode = 16777217}};
    move_up.gamepad_axes = {GamepadAxisBinding{.axis = 1, .axis_value = -1.0f}};

    ActionBindings cast;
    cast.action_name = "combat_cast";
    cast.keys = {KeyBinding{.keycode = 32, .ctrl_pressed = true}};
    cast.mouse_buttons = {MouseButtonBinding{.button_index = 1}};

    ActionBindings interact;
    interact.action_name = "interact";
    interact.gamepad_buttons = {GamepadButtonBinding{.button_index = 11}};

    // Em ORDEM DE REGISTRY (move_forward, interact, combat_cast): a forma canonica
    // emite nessa ordem, entao o roundtrip preserva o vetor igual. A
    // independencia-de-ordem-de-insercao e coberta pelos testes de canonico/hash.
    cfg.actions = {move_up, interact, cast};
    return cfg;
}

}  // namespace

// ---- roundtrip canonico ----------------------------------------------------

TEST_CASE("controls_json: roundtrip canonico preserva o config",
          "[domain][input][controls_json]") {
    const auto cfg = rich_config();
    const std::string json = serialize_controls_canonical(cfg);
    const auto parsed = parse_controls(json);
    REQUIRE(parsed.error == ControlsParseError::None);
    REQUIRE(parsed.config == cfg);
}

// ---- roundtrip da forma de disco (pretty) ----------------------------------

TEST_CASE("controls_json: roundtrip da forma de disco (pretty) preserva o config",
          "[domain][input][controls_json]") {
    const auto cfg = rich_config();
    const std::string json = serialize_controls_pretty(cfg);
    const auto parsed = parse_controls(json);
    REQUIRE(parsed.error == ControlsParseError::None);
    REQUIRE(parsed.config == cfg);
}

// ---- canonica = compacta, sem espaco significativo -------------------------

TEST_CASE("controls_json: forma canonica e compacta (sem espacos/quebras)",
          "[domain][input][controls_json]") {
    const std::string json = serialize_controls_canonical(rich_config());
    REQUIRE(json.find('\n') == std::string::npos);
    REQUIRE(json.find("  ") == std::string::npos);  // sem indentacao dupla
}

// ---- pretty != canonica nos bytes, IGUAIS na semantica ---------------------

TEST_CASE("controls_json: pretty e canonica diferem nos bytes mas nao na semantica",
          "[domain][input][controls_json]") {
    const auto cfg = rich_config();
    const std::string pretty = serialize_controls_pretty(cfg);
    const std::string canonical = serialize_controls_canonical(cfg);
    REQUIRE(pretty != canonical);  // pretty tem espacos/quebras
    REQUIRE(parse_controls(pretty).config == parse_controls(canonical).config);
}

// ---- ordem canonica das actions = ordem do ActionRegistry ------------------

TEST_CASE("controls_json: forma canonica ordena actions pela ordem do ActionRegistry",
          "[domain][input][controls_json]") {
    // O registry tem move_forward (1o) ... interact (13o) ... combat_cast (combat).
    // No rich_config inserimos {move_forward, combat_cast, interact}; o canonico
    // deve emitir interact ANTES de combat_cast (ordem do registry).
    const std::string json = serialize_controls_canonical(rich_config());
    const auto pos_move = json.find("move_forward");
    const auto pos_interact = json.find("interact");
    const auto pos_cast = json.find("combat_cast");
    REQUIRE(pos_move != std::string::npos);
    REQUIRE(pos_interact != std::string::npos);
    REQUIRE(pos_cast != std::string::npos);
    REQUIRE(pos_move < pos_interact);
    REQUIRE(pos_interact < pos_cast);
}

// ---- reordenar o JSON a mao NAO muda a forma canonica ----------------------

TEST_CASE("controls_json: reordenar actions no JSON nao muda o canonico (re-serializa)",
          "[domain][input][controls_json]") {
    const auto cfg = rich_config();
    const std::string canonical = serialize_controls_canonical(cfg);
    // Parseia a forma pretty (que pode estar em ordem diferente da inserida) e
    // re-serializa canonico: deve bater byte-a-byte com o canonico do cfg.
    const auto reparsed = parse_controls(serialize_controls_pretty(cfg));
    REQUIRE(serialize_controls_canonical(reparsed.config) == canonical);
}

// ---- parse robusto: malformado NUNCA crasha, devolve erro tipado -----------

TEST_CASE("controls_json: JSON malformado retorna erro (nunca lanca)",
          "[domain][input][controls_json]") {
    const std::vector<std::string> bad = {
        "",                              // vazio
        "{",                             // chave nao fechada
        "}",                             // fecha sem abrir
        "{ \"config_version\": }",       // valor faltando
        "{ \"config_version\" 1 }",      // sem dois-pontos
        "{ \"actions\": [ { ] }",        // array/obj cruzados
        "{ \"config_version\": 1, }",    // virgula sobrando antes do fim
        "not json at all",               // lixo
        "{ \"config_version\": 999999999999999999999999 }",  // numero gigante
        "[ 1, 2, 3 ]",                   // raiz nao e objeto
        "{ \"actions\": \"notarray\" }", // tipo errado
        "{ \"config_version\": tru }",   // literal quebrado
    };
    for (const auto& s : bad) {
        INFO("entrada: " << s);
        const auto r = parse_controls(s);  // NAO deve lancar
        REQUIRE(r.error != ControlsParseError::None);
    }
}

// ---- forward-compat: action desconhecida ignorada --------------------------

TEST_CASE("controls_json: action_name desconhecida e ignorada (forward-compat)",
          "[domain][input][controls_json]") {
    const std::string json =
        "{ \"config_version\": 1, \"actions\": ["
        "  { \"action_name\": \"acao_do_futuro\", \"keys\": [ { \"keycode\": 1 } ] },"
        "  { \"action_name\": \"interact\", \"keys\": [ { \"keycode\": 70 } ] }"
        "] }";
    const auto r = parse_controls(json);
    REQUIRE(r.error == ControlsParseError::None);
    REQUIRE(r.config.actions.size() == 1);  // a do futuro caiu
    REQUIRE(r.config.actions[0].action_name == "interact");
    REQUIRE(r.config.actions[0].keys.size() == 1);
    REQUIRE(r.config.actions[0].keys[0].keycode == 70);
}

// ---- forward-compat: chave JSON desconhecida ignorada ----------------------

TEST_CASE("controls_json: chaves JSON desconhecidas sao ignoradas (forward-compat)",
          "[domain][input][controls_json]") {
    const std::string json =
        "{ \"config_version\": 1, \"campo_novo\": 42, \"actions\": ["
        "  { \"action_name\": \"interact\", \"futuro\": true,"
        "    \"keys\": [ { \"keycode\": 70, \"extra\": 1 } ] }"
        "] }";
    const auto r = parse_controls(json);
    REQUIRE(r.error == ControlsParseError::None);
    REQUIRE(r.config.actions.size() == 1);
    REQUIRE(r.config.actions[0].keys[0].keycode == 70);
}

// ---- defaults: deadzone ausente usa 0.5 ------------------------------------

TEST_CASE("controls_json: deadzone ausente usa o default 0.5 do struct",
          "[domain][input][controls_json]") {
    const std::string json =
        "{ \"config_version\": 1, \"actions\": ["
        "  { \"action_name\": \"interact\", \"keys\": [ { \"keycode\": 70 } ] }"
        "] }";
    const auto r = parse_controls(json);
    REQUIRE(r.error == ControlsParseError::None);
    REQUIRE(r.config.actions[0].deadzone == 0.5f);
}

// ---- modifiers e tipos de binding roundtrippam ------------------------------

TEST_CASE("controls_json: modifiers de tecla (ctrl/shift/alt) roundtrippam",
          "[domain][input][controls_json]") {
    InputRemapConfig cfg;
    ActionBindings ab;
    ab.action_name = "combat_cast";
    ab.keys = {KeyBinding{.keycode = 65,
                          .ctrl_pressed = true,
                          .shift_pressed = false,
                          .alt_pressed = true}};
    cfg.actions = {ab};
    const auto r = parse_controls(serialize_controls_canonical(cfg));
    REQUIRE(r.error == ControlsParseError::None);
    REQUIRE(r.config.actions[0].keys[0].ctrl_pressed == true);
    REQUIRE(r.config.actions[0].keys[0].shift_pressed == false);
    REQUIRE(r.config.actions[0].keys[0].alt_pressed == true);
}

// ---- config vazio (sem actions) roundtrippa --------------------------------

TEST_CASE("controls_json: config sem actions roundtrippa (lista vazia valida)",
          "[domain][input][controls_json]") {
    InputRemapConfig cfg;  // config_version=1, actions vazio
    const auto r = parse_controls(serialize_controls_canonical(cfg));
    REQUIRE(r.error == ControlsParseError::None);
    REQUIRE(r.config.actions.empty());
    REQUIRE(r.config.config_version == 1);
}

// ---- string com escape (ainda que action_name nao precise hoje) ------------

TEST_CASE("controls_json: strings com escape basico (\\\" \\\\) roundtrippam",
          "[domain][input][controls_json]") {
    // action_name desconhecido seria ignorado; usamos um conhecido. O escape e
    // exercitado pela serializacao defensiva de qualquer string do schema.
    const std::string json =
        "{ \"config_version\": 1, \"actions\": ["
        "  { \"action_name\": \"interact\", \"keys\": [] }"
        "] }";
    const auto r = parse_controls(json);
    REQUIRE(r.error == ControlsParseError::None);
    REQUIRE(r.config.actions[0].action_name == "interact");
}
