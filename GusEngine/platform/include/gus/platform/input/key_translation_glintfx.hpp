// SPDX-License-Identifier: GPL-3.0-or-later
// gus/platform/input/key_translation_glintfx.hpp
//
// Traducao BIDIRECIONAL glintfx::Key <-> keycode do vocabulario Godot Key persistido
// (o MESMO vocabulario fossil que key_translation.hpp traduz pro backend SDL - ver
// aquele header pro "porque" do vocabulario Godot ter virado o formato persistido).
// Irma de key_translation.{hpp,cpp}: MESMO padrao (recebe/devolve tipos primitivos +
// glintfx::Key, cobre o sentinela 0 = "sem binding conhecido"), so que para o SEGUNDO
// backend de evento (glintfx, F4-2/HOSTIN-1) que a costura de input (onda F4) esta
// preparando.
//
// POR QUE BIDIRECIONAL (key_translation.hpp e so sdl->godot e chega): a decisao do
// lider de POLLING POR NIVEL (App::is_key_down(glintfx::Key), nao so callback de edge)
// exige a direcao REVERSA: dado um keycode godot ja gravado em controls.json/save V4,
// descobrir qual glintfx::Key perguntar pro host a CADA quadro. Ambas direcoes vem de
// UMA UNICA tabela de pares (kPairs, no .cpp) - gerar as duas funcoes por FORA dessa
// fonte unica (ex.: duplicar a lista a mao em cada direcao) e como as duas divergem
// silenciosamente quando alguem edita so um lado.
//
// COBERTURA: todo o conjunto NOMEAVEL de glintfx::Key do pin atual (glintfx v0.19.0,
// HOSTIN-1) - None..PageDown (bloco locked por static_assert no proprio glintfx) +
// letras + digitos + F1-F12 + setas + modificadores (Shift/Ctrl/Alt/Super, cada um com
// variante Left/Right) + teclas nomeadas (Insert/CapsLock/ScrollLock/NumLock/
// PrintScreen/Pause/Menu) + pontuacao + numpad. None (sentinela "nenhuma tecla") e
// Count (sentinela "um-passado-do-fim", NUNCA uma tecla real - doc-comment do proprio
// glintfx) ficam DE FORA da tabela de proposito: nao sao teclas nomeaveis.
//
// 4 TECLAS FICAM DE FORA MESMO SENDO NOMEAVEIS (sentinela 0 nas duas direcoes) - NAO
// SAO OMISSAO, SAO EXCLUSAO DOCUMENTADA (regra do lider: "nao invente valor, use a
// sentinela e documente"):
//   - Backspace: o keycode REAL do Godot 4 pra Backspace (SPECIAL|0x04 = 4194308) JA E
//     o mesmo valor que este projeto usa pro seu Tab (ver kGodotTab em
//     key_translation.cpp/controls_restore.cpp - um desvio PRE-EXISTENTE do Godot 4
//     real, congelado no fossil, que NAO e desta fatia consertar). Inventar OUTRO
//     valor pra Backspace violaria a regra "nao invente"; a sentinela e honesta. NOTA
//     DE PARIDADE: o backend SDL ATUAL (key_translation.cpp) TAMBEM nao suporta
//     Backspace (SDLK_BACKSPACE=0x08 nao cai em nenhum case nem no passthrough ASCII
//     0x20-0x7E - vira sentinela 0 la tambem) - entao esta exclusao E paridade, nao
//     uma regressao glintfx-only.
//   - World1/World2: o Key enum do Godot 4 (core/os/keyboard.h, conferido na fonte
//     upstream) NAO tem entrada equivalente a essas duas teclas ISO-extra do GLFW (o
//     proprio doc-comment do glintfx explica que sao a tecla extra de teclados
//     ABNT2/europeus - ver ui_event.hpp). Sem nome no vocabulario Godot -> sentinela.
//   - KpEqual: o Key enum do Godot 4 tambem NAO define KP_EQUAL (conferido: so
//     KP_MULTIPLY/DIVIDE/SUBTRACT/PERIOD/ADD/0-9). Sem nome no vocabulario -> sentinela.
//
// PARA OS DEMAIS NAO-ANCORADOS AINDA (F1-F12, Ctrl/Alt/Super, CapsLock/NumLock/
// ScrollLock/PrintScreen/Pause/Menu, Insert/Delete/Home/End/PageUp/PageDown, numpad,
// pontuacao): os valores usados SAO os valores REAIS do enum Key do Godot 4 upstream
// (core/os/keyboard.h, SPECIAL=1<<22 | offset - conferido na fonte oficial, nao
// inventados), verificados SEM colisao contra os 9 anchors ja congelados neste
// projeto (Escape/Tab/Enter/Left/Up/Right/Down/Shift/Space) nem entre si. Isto AMPLIA
// o vocabulario efetivamente bindavel alem do que o backend SDL cobre hoje (o
// key_translation.cpp so tem case explicito pra setas/shift/enter/escape/tab/space +
// passthrough ASCII 0x20-0x7E - nunca teve F-keys/ctrl/alt/numpad) - aditivo, ZERO
// mudanca no que ja existe.
//
// MODIFICADORES LEFT/RIGHT FUNDEM NO MESMO GODOT (LeftShift+RightShift -> o UNICO
// kGodotShift, idem Control/Alt/Super): MESMA semantica que key_translation.cpp ja usa
// pra LSHIFT/RSHIFT (o esquema de fabrica so tem UM codigo por "familia" de
// modificador). Para a direcao REVERSA (godot->glintfx) de um valor fundido, a
// variante Left e SEMPRE a canonica (ordem da tabela de pares - Left listado antes de
// Right). LIMITACAO CONHECIDA: um binding gravado como "Shift" so faz is_key_down
// consultar LeftShift; um jogador que so tem a tecla Shift DIREITA fisicamente
// (raro, mas existe em teclados split/acessibilidade) nao acionaria o polling por
// este caminho sozinho - o CHAMADOR futuro (F4-3, cutover) que precisar do estado
// FUNDIDO L+R de um binding "Shift" tem que consultar godot_keycode_to_glintfx_key()
// E, alem disso, tratar Left/Right como sinonimos no is_key_down (OR manual) - fora do
// escopo desta fatia (so a TRADUCAO, nao o consumo no maestro/InputMapper glintfx).
//
// NOTA DE PROJETO (append-only do glintfx): glintfx::Key e POSICIONAL (tecla FISICA
// de um teclado US-QWERTY), enquanto o keycode Godot persistido e por ROTULO (LABEL).
// Em ABNT2 o layout de fabrica (WASD, setas, Enter/Escape/Tab/Espaco, digitos 1-4)
// COINCIDE nas duas convencoes (sao todas teclas cuja posicao fisica bate com o
// rotulo em ambos os layouts) - a DERIVA so aparece em pontuacao (ex.: a tecla fisica
// que um teclado US rotula "'" fica na posicao "C" (cedilha) num ABNT2; glintfx::Key::
// Apostrophe sempre nomeia a POSICAO, nao o simbolo que aparece impresso na tecla
// ABNT2 real). Isto e uma LIMITACAO CONHECIDA herdada do proprio glintfx (ver o
// doc-comment de Key em ui_event.hpp, secao ABNT2), nao introduzida por esta tabela -
// registrada aqui pra quem for debugar "o jogador brasileiro remapeou uma tecla de
// pontuacao e o rotulo na UI nao bate com o que ele apertou".

#ifndef GUS_PLATFORM_INPUT_KEY_TRANSLATION_GLINTFX_HPP
#define GUS_PLATFORM_INPUT_KEY_TRANSLATION_GLINTFX_HPP

#include <optional>

#include <glintfx/ui_event.hpp>  // glintfx::Key, glintfx::Mod_* (enum leve, sem outras deps)

#include "gus/domain/input/input_binding.hpp"  // gus::domain::input::KeyBinding

namespace gus::platform::input {

// Converte um glintfx::Key para o keycode equivalente no esquema Godot persistido
// (default_controls()/controls.json/save V4). Tecla fora da tabela (None, Count, cast
// fora de faixa, ou uma das 4 exclusoes documentadas no header acima) -> sentinela 0
// ("sem binding conhecido"), MESMA convencao de sdl_key_to_godot_keycode. Pura,
// deterministica.
[[nodiscard]] long long glintfx_key_to_godot_keycode(glintfx::Key key) noexcept;

// Direcao REVERSA: dado um keycode ja persistido (vindo de KeyBinding::keycode,
// carregado do disco), devolve o glintfx::Key correspondente para alimentar
// App::is_key_down() a cada quadro (polling por nivel, decisao do lider). keycode
// desconhecido (0, ou qualquer valor que nao aparece na tabela) -> std::nullopt. Para
// um keycode fundido L/R (Shift/Ctrl/Alt/Super), devolve a variante Left (canonica -
// ver nota de limitacao conhecida acima). Pura, deterministica.
[[nodiscard]] std::optional<glintfx::Key> godot_keycode_to_glintfx_key(
    long long godot_keycode) noexcept;

// Payload puro da captura de remap no backend glintfx - espelha os DOIS parametros
// que o caminho SDL (system_menu_loop.cpp, bloco "MODO DE CAPTURA") computa ANTES de
// chamar system_menu_controls_capture_key(state, is_escape, godot_keycode): is_escape
// (Esc SEMPRE cancela a captura, checado ANTES de qualquer traducao de tecla - a MESMA
// prioridade que o caminho SDL ja tem) e o KeyBinding pronto pra alimentar
// apply_key_remap/system_menu_controls_capture_key sem o CHAMADOR ter que desmontar
// bitmask nenhum.
//
// DIFERENCA ADITIVA (nao uma divergencia de comportamento a corrigir): o caminho SDL
// ATUAL (system_menu.cpp, system_menu_controls_capture_key) HARDCODA
// ctrl_pressed=shift_pressed=alt_pressed=false na hora de montar o KeyBinding, mesmo
// que o SDL_Event carregue o modifier real (ev.key.mod) - uma lacuna PRE-EXISTENTE,
// fora do escopo desta fatia (nao mexemos em system_menu.cpp/domain/). Esta funcao,
// alimentada pelo bitmask glintfx::Mod_* que o UiEvent::Type::Key JA carrega (ver
// ui_event.hpp, campo `modifiers`), captura os modifiers de verdade - fecha essa
// lacuna PRA QUEM for consumir este caminho no futuro (F4-3), sem tocar no caminho
// SDL existente.
struct GlintfxCapturedKey {
    bool is_escape = false;
    // keycode==0 quando a tecla nao tem correspondente conhecido (ver
    // glintfx_key_to_godot_keycode) - MESMA semantica de sentinela "continua
    // capturando, tenta outra tecla" que system_menu_controls_capture_key ja trata
    // pro caminho SDL. Quando is_escape==true, o binding fica no default (keycode=0,
    // modifiers ignorados) - Esc NUNCA vira um binding valido.
    gus::domain::input::KeyBinding binding{};
};

// Monta o payload de captura a partir de um evento de tecla do glintfx (Key + bitmask
// de modifiers, glintfx::Mod_Shift|Mod_Ctrl|Mod_Alt combinaveis por OR - ver
// UiEvent::modifiers em ui_event.hpp). Pura, deterministica, ZERO side effect (o
// CHAMADOR decide o que fazer com o resultado, igual ao par is_escape/godot_keycode
// que o caminho SDL ja calcula antes de mutar o SystemMenuState).
[[nodiscard]] GlintfxCapturedKey glintfx_capture_key(glintfx::Key key, int modifiers) noexcept;

}  // namespace gus::platform::input

#endif  // GUS_PLATFORM_INPUT_KEY_TRANSLATION_GLINTFX_HPP
