// SPDX-License-Identifier: GPL-3.0-or-later
// GusEngine/platform/tests/key_translation_glintfx_test.cpp
//
// Catch2 da traducao BIDIRECIONAL glintfx::Key <-> keycode Godot (platform/input,
// F4-2.1) + captura de remap do backend glintfx (F4-2.2). TEST-FIRST. Headless: nao
// abre janela, nao inicializa SDL nem glintfx::App - so exercita as duas funcoes de
// traducao e a montagem pura do KeyBinding.
//
// LICAO DURA da onda F4-1 (ver CLAUDE.md/TODO.md): a peca nova precisa ser CHAMADA e
// EXERCITADA, senao o mutante sobrevive. Este arquivo cobre, nesta ordem:
//   1. Round-trip (godot_to_key(key_to_godot(k)) == k) pra toda tecla NAO fundida L/R.
//   2. Fusao Left/Right (Shift/Ctrl/Alt/Super): mesmo godot dos dois lados; reverso e
//      SEMPRE a variante Left (canonica).
//   3. PARIDADE com o backend SDL - a mesma tecla fisica produz o MESMO keycode
//      godot nos dois backends (garante que controls.json significa a mesma coisa
//      nas duas cascas).
//   4. Sentinelas: desconhecida, Key::Count, cast fora de faixa, keycode inexistente
//      na direcao reversa.
//   5. Cobertura total: percorre None+1..Count e falha se alguma tecla nomeavel (fora
//      da lista de exclusao DOCUMENTADA no header) ficou sem traducao - pega "o
//      glintfx adicionou tecla nova e ninguem mapeou".
//   6. Captura (glintfx_capture_key): monta o KeyBinding certo com/sem modifier, Esc
//      cancela, tecla desconhecida fica com keycode 0; a igualdade ESTRUTURAL
//      Ctrl+A != A (input_binding.hpp) sobrevive; paridade de KEYCODE com o caminho
//      SDL (system_menu_loop.cpp).

#include <set>

#include <catch2/catch_test_macros.hpp>

#include "gus/domain/input/input_binding.hpp"
#include "gus/platform/input/key_translation.hpp"
#include "gus/platform/input/key_translation_glintfx.hpp"

using gus::domain::input::KeyBinding;
using gus::platform::input::GlintfxCapturedKey;
using gus::platform::input::glintfx_capture_key;
using gus::platform::input::glintfx_key_to_godot_keycode;
using gus::platform::input::godot_keycode_to_glintfx_key;
using gus::platform::input::sdl_key_to_godot_keycode;
using K = glintfx::Key;

namespace {

// SDL_Keycode das teclas exercitadas na paridade (MESMOS literais de
// key_translation_test.cpp/sdl_input_test.cpp - convencao do platform/tests/: literal
// local em vez de incluir <SDL3/SDL_keycode.h>, pra o teste continuar headless/leve).
constexpr int kSdlUp = 0x40000052;      // SDLK_UP
constexpr int kSdlDown = 0x40000051;    // SDLK_DOWN
constexpr int kSdlLeft = 0x40000050;    // SDLK_LEFT
constexpr int kSdlRight = 0x4000004F;   // SDLK_RIGHT
constexpr int kSdlLShift = 0x400000E1;  // SDLK_LSHIFT
constexpr int kSdlRShift = 0x400000E5;  // SDLK_RSHIFT
constexpr int kSdlReturn = 0x0D;        // SDLK_RETURN ('\r')
constexpr int kSdlKpEnter = 0x40000058; // SDLK_KP_ENTER
constexpr int kSdlEscape = 0x1B;        // SDLK_ESCAPE
constexpr int kSdlTab = 0x09;           // SDLK_TAB
constexpr int kSdlSpace = 0x20;         // SDLK_SPACE
constexpr int kSdlBackspace = 0x08;     // SDLK_BACKSPACE

// 4 exclusoes DOCUMENTADAS (ver key_translation_glintfx.hpp): teclas nomeaveis do
// glintfx sem correspondente seguro/existente no vocabulario Godot. Reusada pelos
// testes de round-trip e de cobertura total.
const std::set<K>& excluded_keys() {
    static const std::set<K> kExcluded = {K::Backspace, K::World1, K::World2, K::KpEqual};
    return kExcluded;
}

// As 4 variantes Right* que fundem no MESMO godot da variante Left* correspondente -
// round-trip generico NAO vale pra elas (coberto pelo teste de fusao dedicado).
const std::set<K>& right_modifier_variants() {
    static const std::set<K> kRightVariants = {K::RightShift, K::RightControl, K::RightAlt,
                                                K::RightSuper};
    return kRightVariants;
}

}  // namespace

TEST_CASE("key_translation_glintfx: round-trip para toda tecla nao fundida",
          "[key_translation_glintfx]") {
    for (int i = 1; i < static_cast<int>(K::Count); ++i) {  // comeca em 1: None nao e tecla real
        const auto key = static_cast<K>(i);
        if (excluded_keys().count(key) != 0 || right_modifier_variants().count(key) != 0) {
            continue;
        }
        const long long godot = glintfx_key_to_godot_keycode(key);
        INFO("indice do enum: " << i);
        REQUIRE(godot != 0);
        const std::optional<K> back = godot_keycode_to_glintfx_key(godot);
        REQUIRE(back.has_value());
        REQUIRE(*back == key);
    }
}

TEST_CASE("key_translation_glintfx: modificadores Left/Right fundem no mesmo godot; "
          "reverso e sempre a variante Left (canonica)",
          "[key_translation_glintfx]") {
    REQUIRE(glintfx_key_to_godot_keycode(K::LeftShift) ==
            glintfx_key_to_godot_keycode(K::RightShift));
    REQUIRE(glintfx_key_to_godot_keycode(K::LeftControl) ==
            glintfx_key_to_godot_keycode(K::RightControl));
    REQUIRE(glintfx_key_to_godot_keycode(K::LeftAlt) == glintfx_key_to_godot_keycode(K::RightAlt));
    REQUIRE(glintfx_key_to_godot_keycode(K::LeftSuper) ==
            glintfx_key_to_godot_keycode(K::RightSuper));

    REQUIRE(godot_keycode_to_glintfx_key(glintfx_key_to_godot_keycode(K::RightShift)) ==
            std::optional<K>{K::LeftShift});
    REQUIRE(godot_keycode_to_glintfx_key(glintfx_key_to_godot_keycode(K::RightControl)) ==
            std::optional<K>{K::LeftControl});
    REQUIRE(godot_keycode_to_glintfx_key(glintfx_key_to_godot_keycode(K::RightAlt)) ==
            std::optional<K>{K::LeftAlt});
    REQUIRE(godot_keycode_to_glintfx_key(glintfx_key_to_godot_keycode(K::RightSuper)) ==
            std::optional<K>{K::LeftSuper});

    // Ctrl e Alt/Shift/Super sao 4 codigos DISTINTOS entre si (nao colapsam uns nos
    // outros - so a fusao Left/Right dentro da MESMA familia).
    REQUIRE(glintfx_key_to_godot_keycode(K::LeftShift) != glintfx_key_to_godot_keycode(K::LeftControl));
    REQUIRE(glintfx_key_to_godot_keycode(K::LeftControl) != glintfx_key_to_godot_keycode(K::LeftAlt));
    REQUIRE(glintfx_key_to_godot_keycode(K::LeftAlt) != glintfx_key_to_godot_keycode(K::LeftSuper));
}

TEST_CASE("key_translation_glintfx: PARIDADE com o backend SDL - mesma tecla fisica "
          "produz o MESMO keycode godot nos dois backends",
          "[key_translation_glintfx]") {
    // Setas.
    REQUIRE(glintfx_key_to_godot_keycode(K::Up) == sdl_key_to_godot_keycode(kSdlUp));
    REQUIRE(glintfx_key_to_godot_keycode(K::Down) == sdl_key_to_godot_keycode(kSdlDown));
    REQUIRE(glintfx_key_to_godot_keycode(K::Left) == sdl_key_to_godot_keycode(kSdlLeft));
    REQUIRE(glintfx_key_to_godot_keycode(K::Right) == sdl_key_to_godot_keycode(kSdlRight));

    // Shift: os DOIS backends fundem L/R no MESMO godot (mesma semantica).
    REQUIRE(glintfx_key_to_godot_keycode(K::LeftShift) == sdl_key_to_godot_keycode(kSdlLShift));
    REQUIRE(glintfx_key_to_godot_keycode(K::RightShift) == sdl_key_to_godot_keycode(kSdlRShift));

    // Enter: SDL funde Return+KP_Enter; glintfx tem um Enter generico so - os dois
    // convergem pro mesmo godot.
    REQUIRE(glintfx_key_to_godot_keycode(K::Enter) == sdl_key_to_godot_keycode(kSdlReturn));
    REQUIRE(glintfx_key_to_godot_keycode(K::Enter) == sdl_key_to_godot_keycode(kSdlKpEnter));

    // Escape/Tab/Space.
    REQUIRE(glintfx_key_to_godot_keycode(K::Escape) == sdl_key_to_godot_keycode(kSdlEscape));
    REQUIRE(glintfx_key_to_godot_keycode(K::Tab) == sdl_key_to_godot_keycode(kSdlTab));
    REQUIRE(glintfx_key_to_godot_keycode(K::Space) == sdl_key_to_godot_keycode(kSdlSpace));

    // Letras: SDL entrega minuscula e normaliza pra maiuscula no passthrough; glintfx
    // ja e por posicao (A..Z) - os dois convergem pro godot maiusculo.
    REQUIRE(glintfx_key_to_godot_keycode(K::A) == sdl_key_to_godot_keycode(static_cast<int>('a')));
    REQUIRE(glintfx_key_to_godot_keycode(K::W) == sdl_key_to_godot_keycode(static_cast<int>('w')));
    REQUIRE(glintfx_key_to_godot_keycode(K::Z) == sdl_key_to_godot_keycode(static_cast<int>('z')));

    // Digitos.
    REQUIRE(glintfx_key_to_godot_keycode(K::Digit0) ==
            sdl_key_to_godot_keycode(static_cast<int>('0')));
    REQUIRE(glintfx_key_to_godot_keycode(K::Digit9) ==
            sdl_key_to_godot_keycode(static_cast<int>('9')));

    // Pontuacao: o SDL traduz via passthrough ASCII generico (0x20-0x7E) - sem case
    // nomeado, mas ainda assim produz o mesmo valor (SDLK_* == o proprio caractere).
    REQUIRE(glintfx_key_to_godot_keycode(K::Comma) == sdl_key_to_godot_keycode(static_cast<int>(',')));
    REQUIRE(glintfx_key_to_godot_keycode(K::Period) ==
            sdl_key_to_godot_keycode(static_cast<int>('.')));
    REQUIRE(glintfx_key_to_godot_keycode(K::Slash) == sdl_key_to_godot_keycode(static_cast<int>('/')));
    REQUIRE(glintfx_key_to_godot_keycode(K::Semicolon) ==
            sdl_key_to_godot_keycode(static_cast<int>(';')));
    REQUIRE(glintfx_key_to_godot_keycode(K::Equal) == sdl_key_to_godot_keycode(static_cast<int>('=')));
    REQUIRE(glintfx_key_to_godot_keycode(K::Minus) == sdl_key_to_godot_keycode(static_cast<int>('-')));
    REQUIRE(glintfx_key_to_godot_keycode(K::Apostrophe) ==
            sdl_key_to_godot_keycode(static_cast<int>('\'')));
    REQUIRE(glintfx_key_to_godot_keycode(K::LeftBracket) ==
            sdl_key_to_godot_keycode(static_cast<int>('[')));
    REQUIRE(glintfx_key_to_godot_keycode(K::Backslash) ==
            sdl_key_to_godot_keycode(static_cast<int>('\\')));
    REQUIRE(glintfx_key_to_godot_keycode(K::RightBracket) ==
            sdl_key_to_godot_keycode(static_cast<int>(']')));
    REQUIRE(glintfx_key_to_godot_keycode(K::GraveAccent) ==
            sdl_key_to_godot_keycode(static_cast<int>('`')));

    // Backspace: PARIDADE NEGATIVA - os dois backends concordam que e sentinela 0
    // (ver header pro motivo: SDL nunca teve suporte, e o valor real do Godot pra
    // Backspace colide com o Tab ja congelado neste projeto).
    REQUIRE(glintfx_key_to_godot_keycode(K::Backspace) == sdl_key_to_godot_keycode(kSdlBackspace));
    REQUIRE(glintfx_key_to_godot_keycode(K::Backspace) == 0LL);
}

TEST_CASE("key_translation_glintfx: sentinelas - desconhecida/Count/fora de faixa -> "
          "0; reverso de keycode inexistente -> nullopt",
          "[key_translation_glintfx]") {
    REQUIRE(glintfx_key_to_godot_keycode(K::None) == 0LL);
    REQUIRE(glintfx_key_to_godot_keycode(K::Count) == 0LL);
    REQUIRE(glintfx_key_to_godot_keycode(static_cast<K>(99999)) == 0LL);
    REQUIRE(glintfx_key_to_godot_keycode(static_cast<K>(-1)) == 0LL);

    REQUIRE_FALSE(godot_keycode_to_glintfx_key(0).has_value());
    REQUIRE_FALSE(godot_keycode_to_glintfx_key(-1).has_value());
    REQUIRE_FALSE(godot_keycode_to_glintfx_key(999999999LL).has_value());  // nunca usado na tabela
}

TEST_CASE("key_translation_glintfx: cobertura total - toda tecla nomeavel do enum tem "
          "um par ou esta na lista de exclusao documentada",
          "[key_translation_glintfx]") {
    // Pega "o glintfx adicionou tecla nova e ninguem mapeou": qualquer indice entre
    // None+1 e Count-1 que NAO esteja na lista de exclusao documentada TEM que ter
    // traducao != 0; e a lista de exclusao TEM que continuar sentinela (se algum dia
    // ganhar valor, o teste avisa pra atualizar a lista/o header junto).
    for (int i = 1; i < static_cast<int>(K::Count); ++i) {
        const auto key = static_cast<K>(i);
        const long long godot = glintfx_key_to_godot_keycode(key);
        INFO("indice do enum: " << i);
        if (excluded_keys().count(key) != 0) {
            REQUIRE(godot == 0);
        } else {
            REQUIRE(godot != 0);
        }
    }
}

TEST_CASE("key_translation_glintfx: glintfx_capture_key monta o KeyBinding certo, com "
          "e sem modifier",
          "[key_translation_glintfx]") {
    // Tecla simples, sem modifier.
    {
        const GlintfxCapturedKey captured = glintfx_capture_key(K::A, glintfx::Mod_None);
        REQUIRE_FALSE(captured.is_escape);
        REQUIRE(captured.binding == KeyBinding{.keycode = static_cast<long long>('A')});
    }
    // Ctrl+A - a igualdade ESTRUTURAL (input_binding.hpp) exige Ctrl+A != A.
    {
        const GlintfxCapturedKey captured = glintfx_capture_key(K::A, glintfx::Mod_Ctrl);
        const KeyBinding expected{.keycode = static_cast<long long>('A'), .ctrl_pressed = true};
        REQUIRE(captured.binding == expected);
        REQUIRE_FALSE(captured.binding == KeyBinding{.keycode = static_cast<long long>('A')});
    }
    // Shift+Alt combinados (OR de bits).
    {
        const GlintfxCapturedKey captured =
            glintfx_capture_key(K::F1, glintfx::Mod_Shift | glintfx::Mod_Alt);
        const KeyBinding expected{.keycode = glintfx_key_to_godot_keycode(K::F1),
                                   .shift_pressed = true,
                                   .alt_pressed = true};
        REQUIRE(captured.binding == expected);
    }
    // Escape cancela: is_escape=true, binding fica no default (keycode 0), modifiers
    // ignorados (Esc nunca vira um binding valido).
    {
        const GlintfxCapturedKey captured = glintfx_capture_key(K::Escape, glintfx::Mod_Ctrl);
        REQUIRE(captured.is_escape);
        REQUIRE(captured.binding == KeyBinding{});
    }
    // Tecla sem correspondente conhecido (Backspace): keycode fica 0 (sentinela),
    // is_escape=false (o CHAMADOR trata como "continua capturando").
    {
        const GlintfxCapturedKey captured = glintfx_capture_key(K::Backspace, glintfx::Mod_None);
        REQUIRE_FALSE(captured.is_escape);
        REQUIRE(captured.binding.keycode == 0);
    }
}

TEST_CASE("key_translation_glintfx: captura tem paridade de KEYCODE com o caminho SDL "
          "(system_menu_loop.cpp) para tecla com e sem modifier",
          "[key_translation_glintfx]") {
    REQUIRE(glintfx_capture_key(K::W, glintfx::Mod_None).binding.keycode ==
            sdl_key_to_godot_keycode(static_cast<int>('w')));
    REQUIRE(glintfx_capture_key(K::W, glintfx::Mod_Shift).binding.keycode ==
            sdl_key_to_godot_keycode(static_cast<int>('w')));  // modifier nao muda o keycode base
    REQUIRE(glintfx_capture_key(K::Escape, glintfx::Mod_None).is_escape);
    REQUIRE(glintfx_capture_key(K::Escape, glintfx::Mod_Ctrl).is_escape);  // Esc cancela sempre
}
