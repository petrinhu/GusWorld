// SPDX-License-Identifier: Apache-2.0
// gus/app/src/screens/title_menu_loop.cpp
//
// Implementacao do loop interativo da TELA DE TITULO. Ver header para o
// contrato completo.
//
// F4-1b.3 (onda F4 "casca SDL -> App mode do glintfx", fatia 1b.3 - PRIMEIRA
// tela PAI da onda apos F4-1b.1/difficulty_menu_loop.cpp e F4-1b.2/
// save_load_menu_loop.cpp): o caminho de PRODUCAO (run_title_menu_loop_gl_
// current, chamado DIRETO por Maestro::show_title_screen) foi convertido de um
// while(true){SDL_PollEvent...} PROPRIO pra uma classe TitleScreen (gus::app::
// ScreenState) + gus::app::run_screen_state - MESMA tecnica de
// DifficultyScreen/SaveLoadScreen. O roteamento de evento (decisao PURA) foi
// extraido pra title_screen_step (declarada no .hpp, testada headless em
// title_screen_step_test.cpp) - MESMO racional de difficulty_screen_step.
//
// O PROBLEMA NOVO desta fatia (por isso e a PRIMEIRA tela PAI): o titulo
// dispara a tela de SELECAO DE DIFICULDADE (difficulty_menu_loop.hpp) ANINHADA
// quando o jogador confirma "Novo Jogo". ANTES desta fatia, isso acontecia de
// DENTRO do handler da tela de titulo (o antigo start_new_game_via_difficulty_
// menu, chamado de dentro de route_title_action) - o que agora VIOLARIA a
// guarda de reentrada de gus::app::run_screen_state() (screen_state.hpp:29-39:
// run_screen_state(pai) precisa RETORNAR - rodando pai.exit(), que libera a
// UiLayer do pai - ANTES de qualquer codigo chamar run_screen_state(filha)).
//
// A SOLUCAO (canonizada pelo lider): MINI-DRIVER no WRAPPER (run_title_menu_
// loop_gl_current), NAO dentro da tela. TitleScreen::handle_event() SO devolve
// um TitleScreenExit (ver o .hpp) - nunca chama a tela de dificuldade. O
// DRIVER, dentro do wrapper:
//   1) chama gus::app::run_screen_state(title_screen) - ela SO retorna DEPOIS
//      que title_screen.exit() ja rodou (garantia ESTRUTURAL do proprio
//      run_screen_state, nao depende de ninguem "lembrar" de nada);
//   2) SO ENTAO, se o desfecho foi NewGameRequested, chama
//      run_difficulty_menu_loop_gl_current (a UiLayer do titulo ja foi
//      destruida - seguro criar a 2a);
//   3) decide o proximo passo via a funcao PURA title_flow_next() (testada
//      headless em title_screen_step_test.cpp: as 3x3 combinacoes de
//      TitleScreenExit x DifficultyLoopExit);
//   4) se o jogador Cancelou na dificuldade, RE-USA o MESMO objeto
//      TitleScreen, chamando run_screen_state(title_screen) de novo.
//
// TitleScreen PERSISTE entre iteracoes do driver (e um objeto local da PILHA
// do wrapper, nao recriado a cada volta) - o ESTADO de negocio (TitleMenuState
// state_, o TitleDiskScan scan_ dos saves) e inicializado no CONSTRUTOR, NAO
// em enter(): so ASSIM "voltar da dificuldade cancelando" preserva o foco/
// estado ATUAL (nao re-escaneia saves, nao perde o item selecionado) - MESMO
// comportamento observavel do while(true) antigo (que so recriava a UiLayer,
// nunca o TitleMenuState, ao cancelar). enter()/exit() SO cuidam de recursos
// GL (glintfx::UiLayer, Render2dGl3, boot overlay, SFX ids) - a exclusividade
// da UiLayer (gus/app/screen_state.hpp) vira consequencia ESTRUTURAL de
// title_screen.exit() acontecer (dentro de run_screen_state) ANTES do driver
// sequer cogitar abrir a tela de dificuldade, substituindo o antigo
// ui_opt.reset()/emplace() manual cruzando a fronteira de telas.

#include "gus/app/screens/title_menu_loop.hpp"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

#include <glintfx/clock.hpp>  // FW-CLOCK (bump v0.26.0): substitui SDL_GetTicksNS
#include <glintfx/element_box.hpp>
#include <glintfx/ui_layer.hpp>

#include "gus/app/boot_pixel_overlay.hpp"  // M7-FB3: fundo VIVO da tela de titulo
#include "gus/app/screens/difficulty_menu_loop.hpp"  // MODOS-MORTE Fase 0: aninhada no mesmo GL
#include "gus/app/screens/save_load_menu.hpp"  // SaveSlotPreview/most_recent_occupied_slot
#include "gus/app/screens/title_menu_rml.hpp"
#include "gus/app/screens/ui_hover.hpp"  // COCKPIT-SFX-HOVER-CLIQUE: edge-detect generico
#include "gus/core/anim/boot_pixel_sequence.hpp"  // M7-FB3: boot_pixel_idle_frame_index
#include "gus/core/asset_paths.hpp"  // kMenuHoverSfxFile/kMenuClickSfxFile/kSfxDir/kVfxBootPixelDir
#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Rect
#include "gus/domain/save/save_serializer.hpp"  // LoadResult
#include "gus/platform/assets/asset_source.hpp"  // FilesystemAssetSource (resolve do SFX)
#include "gus/platform/fs/save_file_store.hpp"  // has_save/load_game
#include "gus/platform/input/key_translation.hpp"  // sdl_key_to_godot_keycode (Fatia 2)
#include "gus/platform/input/key_translation_glintfx.hpp"  // godot_keycode_to_glintfx_key
#include "gus/platform/render2d/render2d_gl3.hpp"
// FRAMEGRAB-7-SITIOS (2026-07-30): gl3_loader.hpp/gl3_read_backbuffer_rgba
// (leitura crua de backbuffer via glad) saiu daqui - a unica captura deste
// arquivo migrou pra glintfx::UiLayer::capture_frame() (ver render_frame_()/
// enter()). gl3_load_functions ja nao era usado aqui desde a decommissao de
// run_title_menu_loop_owning_gl (M9-CAMADAS-SDL Fatia 0) - zero necessidade
// remanescente de gl3_loader.hpp neste arquivo.

// stb_image_write: SO a declaracao aqui (a IMPLEMENTACAO ja vive UMA vez em
// battle_preview.cpp, MESMA lib gusengine_app - nao redefinir
// STB_IMAGE_WRITE_IMPLEMENTATION aqui, senao da symbol duplicado no link).
#include "stb_image_write.h"

// ASSETS-FONTE-TELAS-GEMEO (2026-08-05): a macro GUSWORLD_FONTS_DIR NAO e mais lida aqui
// (nem o getenv("GUSWORLD_FONTS") a mao) - o staging das 2 fontes pro stage do glintfx
// migrou pra stage_ui_fonts, que resolve o caminho pelo porteiro de assets e reporta a
// falha de copia. Ver gus/app/screens/font_stage.hpp.
#include "gus/app/screens/font_stage.hpp"

namespace gus::app::screens {

namespace {

namespace fs = std::filesystem;
using gus::domain::save::kSlotCount;

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

// Stage PROPRIO (nao colide com o stage do menu de sistema/save-load).
std::string title_stage_dir() {
    return (fs::temp_directory_path() / "gusworld_glintfx_title").string();
}

std::string write_title_rml_file(const TitleMenuState& state,
                                  const gus::app::i18n::Translator& tr,
                                  int pressed_index = -1) {
    const fs::path stage = title_stage_dir();
    std::error_code ec;
    fs::create_directories(stage, ec);

    // ASSETS-FONTE-TELAS-GEMEO (2026-08-05): as 2 fontes vao pro stage por
    // stage_ui_fonts (font_stage.hpp), que resolve o caminho pelo PORTEIRO de assets
    // (cascata checada `GUSWORLD_FONTS > GUSWORLD_ASSETS+"/fonts" > macro > CWD`) e LOGA
    // a falha de copia. Antes, este bloco lia a macro GUSWORLD_FONTS_DIR crua - o caminho
    // absoluto da MAQUINA DE BUILD - sem checar existencia e engolindo o error_code, o que
    // deixava a tela sem fonte EM SILENCIO em qualquer maquina que nao a de build.
    // Retorno ignorado de proposito: fonte ausente degrada (o RmlUi usa o fallback dele) e
    // stage_ui_fonts ja logou; nao ha o que decidir aqui.
    stage_ui_fonts(stage);

    // FONT-EXTEND-GLITCH (2026-07-29): a fonte NAO e mais injetada aqui via @font-face
    // de string - TitleScreen::enter() registra a familia 1x via
    // glintfx::UiLayer::load_font_face (API v0.24.0) logo apos set_asset_base_url. Os
    // .ttf continuam copiados pro stage acima (load_font_face resolve o path pela MESMA
    // BaseUrlFileInterface que o @font-face antigo usava).
    std::string rml = build_title_menu_rml(state, tr, pressed_index);

    const fs::path out = stage / "title_menu.rml";
    std::ofstream f(out);
    f << rml;
    return out.string();
}

// FONT-EXTEND-GLITCH (2026-07-29): registra "Pixel Operator Mono" (regular+bold) via
// glintfx::UiLayer::load_font_face (API v0.24.0) - mata o @font-face por string que
// write_title_rml_file injetava antes. Chamado 1x em enter() (registro e process-wide
// RmlUi state, ver ui_layer.hpp), DEPOIS do stage ja ter os .ttf (o PRIMEIRO
// write_title_rml_file os copia) e ANTES de ui_->load(). `family` SEMPRE explicito
// (armadilha de assimetria de motor Own-vs-FreeType documentada em font_face.hpp - o
// mesmo literal que o RCSS ja referencia neutraliza os dois).
void register_pixel_operator_mono_fonts(glintfx::UiLayer& ui) {
    if (!ui.load_font_face(glintfx::FontFaceDesc{/*path=*/"PixelOperatorMono.ttf",
                                                  /*family=*/"Pixel Operator Mono"})) {
        std::cerr << "TitleMenuLoop: load_font_face(PixelOperatorMono.ttf) falhou "
                     "(arquivo ausente/invalido no stage) - cai no fallback de fonte do "
                     "RmlUi.\n";
    }
    if (!ui.load_font_face(glintfx::FontFaceDesc{
            /*path=*/"PixelOperatorMono-Bold.ttf",
            /*family=*/"Pixel Operator Mono",
            /*style=*/glintfx::FontStyle::Normal,
            /*weight=*/glintfx::FontWeight::Bold})) {
        std::cerr << "TitleMenuLoop: load_font_face(PixelOperatorMono-Bold.ttf) falhou "
                     "(arquivo ausente/invalido no stage) - cai no fallback de fonte do "
                     "RmlUi.\n";
    }
}

// Hit-test simples (MESMA receita de system_menu_loop.cpp/battle_preview.cpp):
// cursor (espaco-janela) dentro da caixa border-box devolvida por
// glintfx::UiLayer::get_element_box.
bool hit_test(const glintfx::ElementBox& box, float x, float y) {
    if (!box.found) return false;
    return x >= box.x && x <= box.x + box.w && y >= box.y && y <= box.y + box.h;
}

// Resolve o caminho de um SFX do menu (hover/click) - MESMA familia de
// resolve_menu_sfx_path em system_menu_loop.cpp (paridade sonora: os DOIS
// menus de botoes resolvem pelo MESMO caminho/arquivo).
std::string resolve_menu_sfx_path(std::string_view file) {
    const std::string id = join(std::string(gus::core::assets::kSfxDir), std::string(file));
    return gus::platform::assets::FilesystemAssetSource().resolve_path(id);
}

// SFX-MIGRATE-V0.9: filtro NAVEGAVEL pro callback NATIVO de hover
// (glintfx::UiLayer::set_hover_callback) - dado o `id` que o hover nativo
// reportou (entered=true) e o estado ATUAL, devolve true SO se `id` e um dos
// botoes hover-testaveis por CLIQUE (lista de itens OU as 2 pills do
// mini-dialogo de Novo Jogo, MESMOS ids do hit-test de clique mais abaixo).
// Substitui o antigo title_current_hover_index (que devolvia um INDICE via
// title_hover_index/ui_hover_index) - o dedup agora e por ID, interno ao
// proprio hover nativo (ver hover_cb mais abaixo). 100% string/estado, sem GL.
bool is_navigable_hover_id(const TitleMenuState& state, const std::string& id) {
    if (state.confirming_new_game) {
        return id == "title-confirm-0" || id == "title-confirm-1";
    }
    for (int i = 0; i < kTitleItemCount; ++i) {
        if (id == "title-item-" + std::to_string(i)) return true;
    }
    return false;
}

// title_keyboard_focus_index agora e PUBLICA/testavel (title_menu.hpp/.cpp,
// COCKPIT-SFX-HOVER-CLIQUE) - MESMO racional de system_menu_keyboard_focus_index;
// nao duplicada aqui.

// Varredura de disco (o UNICO I/O desta tela): monta os previews de TODOS os
// slots + um cache do SaveData ja carregado por slot (evita ler o arquivo 2x ao
// confirmar "Continuar") + any_save_exists + most_recent_occupied_slot. MESMA
// politica de degradacao de build_previews_and_cache (save_load_menu_loop.cpp):
// um save PRESENTE mas NAO Ok (adulterado/corrompido/versao incompativel/slot
// trocado) degrada, POR ORA, como slot vazio (os avisos dedicados sao etapa
// futura, fora do escopo desta dispatch) - logado pra nao silenciar o caso.
struct TitleDiskScan {
    std::array<SaveSlotPreview, kSlotCount> previews{};
    std::array<std::optional<gus::domain::save::SaveData>, kSlotCount> loaded{};
    bool any_save_exists = false;
    int most_recent_slot = -1;
};

TitleDiskScan scan_saves(const std::string& saves_dir) {
    TitleDiskScan scan;
    for (int slot = 0; slot < kSlotCount; ++slot) {
        if (!gus::platform::fs::has_save(slot, saves_dir)) {
            scan.previews[static_cast<std::size_t>(slot)] = empty_slot_preview(slot);
            continue;
        }
        const auto outcome = gus::platform::fs::load_game(slot, saves_dir);
        if (outcome.has_value() &&
            outcome->result == gus::domain::save::LoadResult::Ok) {
            scan.previews[static_cast<std::size_t>(slot)] =
                build_slot_preview(outcome->data, slot);
            scan.loaded[static_cast<std::size_t>(slot)] = outcome->data;
            scan.any_save_exists = true;
        } else {
            std::cerr << "[title_menu_loop] aviso: slot " << slot
                      << " tem arquivo mas NAO carregou Ok (adulterado/corrompido/"
                         "versao incompativel/slot trocado) - degradando como "
                         "vazio nesta onda (avisos dedicados sao etapa futura).\n";
            scan.previews[static_cast<std::size_t>(slot)] = empty_slot_preview(slot);
        }
    }
    scan.most_recent_slot = most_recent_occupied_slot(scan.previews);
    return scan;
}

}  // namespace

namespace {

// F4-1b.3: aplica o DESFECHO de uma TitleMenuAction (None/ContinueGame/
// StartNewGame/RequestQuit) no TitleStepResult - MESMO dispatch do antigo
// `route_title_action` lambda, so que gravando em campos da struct de retorno
// (reload/exit) em vez de chamar reload()/confirm_continue()/retornar direto
// (esses side effects ficam com o CHAMADOR, ver TitleScreen::handle_event -
// inclusive confirm_continue(), que precisa do cache scan_ que esta funcao
// PURA nao tem).
void apply_title_action_to_result(TitleStepResult& result, TitleMenuAction action) noexcept {
    switch (action) {
        case TitleMenuAction::None:
            result.reload = true;
            return;
        case TitleMenuAction::ContinueGame:
            result.exit = TitleScreenExit::ContinueGame;
            return;
        case TitleMenuAction::StartNewGame:
            result.exit = TitleScreenExit::NewGameRequested;
            return;
        case TitleMenuAction::RequestQuit:
            result.exit = TitleScreenExit::QuitApp;
            return;
    }
}

// Traduz SDL_Keycode -> glintfx::Key REUSANDO a ponte SDL->Godot->glintfx JA
// EXISTENTE e testada adversarialmente (F4-2, 8/8 mutantes mortos):
// platform/input/key_translation.hpp (SDL_Keycode -> keycode Godot) +
// platform/input/key_translation_glintfx.hpp (keycode Godot -> glintfx::Key).
// Evita duplicar uma 3a tabela SDLK_*->glintfx::Key so pra esta tela
// (M9-CAMADAS-SDL Fatia 2, docs/tech/plano-camadas-sdl.md) - title_menu.hpp
// (a logica PURA) so conhece glintfx::Key, o SDL fica 100% aqui no loop.
//
// SDLK_RETURN e SDLK_KP_ENTER colapsam no MESMO Key::Enter pela ponte Godot
// (key_translation.cpp ja fundia as duas no MESMO kGodotEnter, ANTES desta
// fatia) - preserva EXATAMENTE o comportamento anterior (title_menu.cpp ja
// tratava as duas teclas como sinonimo em is_confirm_key), nao e regressao.
// glintfx::Key nao tem KpEnter dedicado - lacuna ja reportada ao glintfx.
glintfx::Key sdl_keycode_to_menu_key(SDL_Keycode sdl_key) noexcept {
    const long long godot =
        gus::platform::input::sdl_key_to_godot_keycode(static_cast<int>(sdl_key));
    return gus::platform::input::godot_keycode_to_glintfx_key(godot)
        .value_or(glintfx::Key::None);
}

}  // namespace

// F4-1b.3: implementacao de title_screen_step (declarada no .hpp) - ver o
// comentario grande la pro contrato completo. Extracao BEHAVIOR-PRESERVING do
// corpo do while(true) antigo (MESMO racional/ordem de checagem de tipo de
// evento, MESMAS chamadas a title_menu_key_down/title_menu_click_option,
// MESMO uso de title_item_selectable/ui_hover_entered_new_item) - so devolvendo
// a DECISAO em vez de executar os side effects na hora.
TitleStepResult title_screen_step(TitleMenuState& state, const SDL_Event& ev,
                                   const glintfx::ElementBox boxes[kTitleItemCount]) noexcept {
    TitleStepResult result;

    if (ev.type == SDL_EVENT_QUIT) {
        result.window_closed = true;
        return result;
    }

    if (ev.type == SDL_EVENT_WINDOW_RESIZED ||
        ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
        result.resize = true;  // estado da tela intocado - o CHAMADOR reposiciona.
        return result;
    }

    if (ev.type == SDL_EVENT_KEY_DOWN && !ev.key.repeat) {
        const bool is_confirm_key = (ev.key.key == SDLK_RETURN ||
                                      ev.key.key == SDLK_KP_ENTER ||
                                      ev.key.key == SDLK_SPACE);
        if (is_confirm_key) {
            // SOM DE CLIQUE (COCKPIT-SFX-HOVER-CLIQUE): Enter/Espaco e sempre
            // uma confirmacao intencional de item/pill - toca INCONDICIONAL
            // (MESMO comportamento antigo: nao ha "item bloqueado" na tela de
            // titulo, ao contrario da tela de dificuldade).
            // EFEITO DE PRESS (B2, decisao do lider 2026-08-01): captura o
            // estado+indice ANTES da mutacao - Enter/Espaco AQUI e SEMPRE uma
            // confirmacao intencional (diferente do system_menu, nao precisa
            // de is_confirming(action): a distincao ja e feita pelo TIPO de
            // tecla, nao pelo resultado - mesmo quando o resultado e None
            // (abrir o mini-dialogo de Novo Jogo, ou cancelar com Nao) o item
            // FOI confirmado/ativado e merece o flash visual).
            result.sfx = TitleSfxKind::Click;
            const TitleMenuState pre_action_state = state;
            const int item_index = title_keyboard_focus_index(state);
            const TitleMenuAction action =
                title_menu_key_down(state, sdl_keycode_to_menu_key(ev.key.key));
            result.flash = TitleFlashInfo{pre_action_state, item_index};
            apply_title_action_to_result(result, action);
        } else {
            // Navegacao (setas/WASD/ESC) - SOM DE HOVER PARIDADE TECLADO x
            // MOUSE: move a selecao e, SO se o MODO (lista vs mini-dialogo)
            // NAO mudou E moveu pra um item NOVO, toca hover_sfx.
            const bool confirming_before = state.confirming_new_game;
            const int kb_index_before = title_keyboard_focus_index(state);
            const TitleMenuAction action =
                title_menu_key_down(state, sdl_keycode_to_menu_key(ev.key.key));
            if (state.confirming_new_game == confirming_before) {
                const int kb_index_after = title_keyboard_focus_index(state);
                if (ui_hover_entered_new_item(kb_index_before, kb_index_after)) {
                    result.sfx = TitleSfxKind::Hover;
                }
            }
            apply_title_action_to_result(result, action);
        }
        return result;
    }

    if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN && ev.button.button == SDL_BUTTON_LEFT) {
        if (state.confirming_new_game) {
            for (int i = 0; i < 2; ++i) {
                if (!hit_test(boxes[i], ev.button.x, ev.button.y)) continue;
                // as 2 pills do mini-dialogo sao SEMPRE validas (sem conceito
                // de "desabilitada" aqui) - som sempre toca. EFEITO DE PRESS
                // (B2): MESMO racional do ramo de teclado - clique numa pill
                // e SEMPRE confirmacao intencional.
                result.sfx = TitleSfxKind::Click;
                const TitleMenuState pre_action_state = state;
                const TitleMenuAction action = title_menu_click_option(state, i);
                result.flash = TitleFlashInfo{pre_action_state, i};
                apply_title_action_to_result(result, action);
                return result;
            }
            return result;  // clique fora de qualquer pill - no-op TOTAL
        }
        for (int i = 0; i < kTitleItemCount; ++i) {
            if (!hit_test(boxes[i], ev.button.x, ev.button.y)) continue;
            // Clicar num item DESABILITADO (Continuar sem save) e no-op TOTAL
            // (ver title_menu_click_option) - sem som nem flash, MESMA
            // semantica "nao reage a nada".
            const TitleMenuState pre_action_state = state;
            if (title_item_selectable(state, i)) {
                result.sfx = TitleSfxKind::Click;
            }
            const TitleMenuAction action = title_menu_click_option(state, i);
            if (title_item_selectable(pre_action_state, i)) {
                result.flash = TitleFlashInfo{pre_action_state, i};
            }
            apply_title_action_to_result(result, action);
            return result;
        }
        return result;  // clique fora de qualquer item - no-op TOTAL
    }

    if (ev.type == SDL_EVENT_MOUSE_MOTION) {
        result.mouse_move = true;
        result.mouse_x = ev.motion.x;
        result.mouse_y = ev.motion.y;
        return result;
    }

    return result;  // tipo de evento nao roteado por esta tela - no-op TOTAL
}

// F4-1b.3: implementacao de title_flow_next (declarada no .hpp) - a MATRIZ DE
// DECISAO do mini-driver, 100% testavel sem GL (title_screen_step_test.cpp
// exercita as 3x3 combinacoes). `difficulty_exit` SO importa quando
// `title_exit == NewGameRequested` (nos outros 2 casos o driver nem chega a
// rodar a tela de dificuldade - ver run_title_menu_loop_gl_current abaixo).
TitleFlowStep title_flow_next(TitleScreenExit title_exit,
                               DifficultyLoopExit difficulty_exit) noexcept {
    switch (title_exit) {
        case TitleScreenExit::ContinueGame:
            return TitleFlowStep::ContinueGame;
        case TitleScreenExit::QuitApp:
            return TitleFlowStep::QuitApp;
        case TitleScreenExit::NewGameRequested:
            switch (difficulty_exit) {
                case DifficultyLoopExit::QuitApp:
                    return TitleFlowStep::QuitApp;
                case DifficultyLoopExit::Chosen:
                    return TitleFlowStep::StartNewGame;
                case DifficultyLoopExit::Cancelled:
                    return TitleFlowStep::RetryTitle;
            }
            break;
    }
    return TitleFlowStep::QuitApp;  // inalcancavel (defensivo - todo
                                     // TitleScreenExit/DifficultyLoopExit
                                     // relevante ja foi coberto acima).
}

namespace {

// F4-1b.3: o ScreenState de PRODUCAO da tela de titulo (unico chamador: o
// MINI-DRIVER dentro de run_title_menu_loop_gl_current, abaixo) - MESMO padrao
// de DifficultyScreen (difficulty_menu_loop.cpp, F4-1b.1). Todo o estado que
// antes vivia em variaveis locais fechadas por lambdas (ui/backdrop/boot_bg/
// ids de SFX/rml_path/etc) agora e MEMBRO; enter() cria os recursos GL
// (glintfx::UiLayer + Render2dGl3 + BootPixelOverlay + ids de SFX), exit()
// libera (a EXCLUSIVIDADE do UiLayer descrita em gus/app/screen_state.hpp).
//
// DIFERENCA-CHAVE em relacao a DifficultyScreen: o ESTADO DE NEGOCIO (state_/
// scan_) NAO nasce em enter() - nasce no CONSTRUTOR, UMA UNICA VEZ. O
// MINI-DRIVER reusa o MESMO objeto TitleScreen entre a lista e a volta da
// tela de dificuldade (Cancelled) - se state_/scan_ fossem recriados em
// enter(), o jogador perderia o foco/re-escanearia os saves a cada volta
// (regressao do comportamento antigo, ver o comentario grande no topo deste
// arquivo).
class TitleScreen final : public gus::app::ScreenState {
   public:
    TitleScreen(SDL_Window* window, gus::platform::audio::AudioEngine& audio,
                const gus::app::i18n::Translator& translator, std::string saves_dir,
                gus::domain::save::SaveData* out_loaded_save)
        : window_(window),
          audio_(audio),
          translator_(translator),
          saves_dir_(std::move(saves_dir)),
          out_loaded_save_(out_loaded_save) {
        scan_ = scan_saves(saves_dir_);
        title_menu_open(state_, scan_.any_save_exists);
    }

    // ATENCAO - CALLBACK-DTOR-DESARME (2026-08-05): SEGUNDA linha de defesa, que se
    // SOMA a (nao substitui) a ordem de declaracao de `ui_` no fim da lista de
    // membros (CALLBACK-DTOR-ORDER, gate tools/callback_dtor_order.py). A ordem e
    // estrutural mas fragil socialmente - depende do comentario ser lido, e o gate
    // dela so conhece os membros de HOJE (um membro NOVO declarado depois de `ui_`
    // passa batido). O desarme e local, explicito e INSENSIVEL a ordem de membro.
    // PRIMEIRA instrucao de proposito: ~UiLayer() descarrega os documentos do RmlUi,
    // o que recalcula a hover chain e pode emitir UM ultimo evento de hover - a
    // lambda de set_hover_callback() chama native_hover_callback_, que toca
    // last_hover_sfx_id_/state_, entao ela nao pode rodar depois que o teardown
    // comecou. Contrato do glintfx v0.30.0, explicito deles: "nenhum consumidor e
    // chamado de volta depois de pedir para ser desligado" - passar nullptr e
    // suportado, nao gambiarra (o proprio ~UiLayer::Impl() deles faz
    // clock.set_cursor_callback(nullptr) como 1a instrucao, glintfx/src/
    // ui_layer.cpp:138; o listener guarda com `if (!cb_ || !*cb_) return;`,
    // glintfx/src/rml/bootstrap.cpp, ou seja std::function vazio = no-op identico a
    // "nunca houve callback"). `if (ui_)`: exit() ja faz ui_.reset() no caminho
    // normal e enter() pode nem ter chegado ao emplace() - desarmar tem de ser
    // seguro com o optional VAZIO.
    // Gate automatico: tools/callback_dtor_disarm.py (roda no tools/check.sh).
    ~TitleScreen() override {
        if (ui_) ui_->set_hover_callback(nullptr);
    }

    void enter() override {
        // Flags TRANSIENTES de UMA rodada - resetadas a CADA enter(),
        // diferente de state_/scan_ (persistem entre rodadas, ver o
        // comentario da classe/construtor acima).
        bailed_ = false;
        done_ = false;
        window_closed_ = false;
        ui_init_failed_ = false;
        screenshot_only_ = false;

        SDL_GetWindowSizeInPixels(window_, &pw_, &ph_);
        if (pw_ < 1) pw_ = 1;
        if (ph_ < 1) ph_ = 1;
        dp_ratio_ = static_cast<float>(pw_) / 960.0f;

        ui_.emplace(glintfx::UiLayer::Config{/*logical_width=*/960,
                                              /*logical_height=*/540,
                                              /*load_gl=*/true,
                                              /*dp_ratio=*/dp_ratio_});
        if (!ui_->ok()) {
            std::cerr << "TitleMenuLoop: glintfx::UiLayer::ok()=false (attach "
                         "falhou) - fechando sem desenhar (degradacao segura, "
                         "comeca fresco).\n";
            // MESMO degrade do while(true) antigo: NewGame direto (Medio
            // default), SEM passar pela tela de dificuldade (abrir uma 2a
            // UiLayer logo apos a 1a ter falhado provavelmente falharia de
            // novo - nao ha ganho em tentar) - flag PROPRIA (nao
            // TitleScreenExit::NewGameRequested), o driver nao deve abrir a
            // dificuldade neste caso (ver ui_init_failed() abaixo).
            ui_init_failed_ = true;
            bailed_ = true;
            return;
        }

        stage_ = title_stage_dir();
        ui_->set_asset_base_url(stage_.c_str());
        rml_path_ = write_title_rml_file(state_, translator_);
        register_pixel_operator_mono_fonts(*ui_);
        ui_->load(rml_path_.c_str());
        ui_->set_viewport(pw_, ph_);
        ui_->set_dp_ratio(dp_ratio_);
        // SFX-MIGRATE-V0.9: 1 update() de "assentamento" (MESMO achado
        // empirico de difficulty_menu_loop.cpp/save_load_menu_loop.cpp - o
        // hover NATIVO so resolve elemento sob o cursor apos pelo menos 1
        // Context::Update() do documento recem-carregado).
        ui_->update();

        backdrop_.emplace(/*gl_active=*/true);

        // M7-FB3 (MENU-INICIAL-FUNDO): fundo VIVO da LISTA - MESMO monitor CRT
        // do boot pixelizado, "assentado" (ver gus/core/anim/
        // boot_pixel_sequence.hpp::boot_pixel_idle_frame_index). boot_bg_ e um
        // MEMBRO reusado entre rodadas (nao um std::optional) mas load()
        // PRECISA rodar de novo a CADA enter() - os TextureId antigos nao
        // sobrevivem a destruicao do Render2dGl3 anterior (ver o comentario de
        // BootPixelOverlay::load() no header - "chamar de novo apos
        // reacquire_renderer()", MESMO racional aqui: backdrop_ e recriado a
        // cada enter()).
        boot_bg_.load(*backdrop_, gus::platform::assets::FilesystemAssetSource().resolve_path(
                                       gus::core::assets::kVfxBootPixelDir));
        boot_bg_start_ns_ = glintfx::monotonic_now_ns();  // FW-CLOCK (bump v0.26.0)

        // COCKPIT-SFX-HOVER-CLIQUE: SFX de hover/clique - load_sfx a CADA
        // enter() (MESMA cautela de "load_sfx NUNCA no frame", os SoundId
        // antigos tambem nao sobrevivem entre rodadas do AudioEngine se ele
        // fosse recriado - aqui audio_ e injetada/nao-dona, sobrevive, mas
        // recarregar o MESMO arquivo e barato e idempotente).
        const std::string hover_sfx_path =
            resolve_menu_sfx_path(gus::core::assets::kMenuHoverSfxFile);
        const std::string click_sfx_path =
            resolve_menu_sfx_path(gus::core::assets::kMenuClickSfxFile);
        hover_sfx_id_ = audio_.load_sfx(hover_sfx_path.c_str());
        click_sfx_id_ = audio_.load_sfx(click_sfx_path.c_str());

        ui_->set_hover_callback([this](const char* raw_id, bool entered) {
            native_hover_callback_(raw_id, entered);
        });

        // DIAGNOSTICO/PROVA (SAVE-LOAD-UI etapa 4, prova visual headless Xvfb
        // :99): GUSWORLD_TITLE_SCREENSHOT_DIR=<dir> assenta alguns frames e
        // salva 1 PNG ANTES de entrar no loop interativo - bypassa por
        // completo, MESMO espirito de GUSWORLD_DIFFICULTY_SCREENSHOT_DIR em
        // DifficultyScreen::enter().
        const char* screenshot_dir = std::getenv("GUSWORLD_TITLE_SCREENSHOT_DIR");
        if (screenshot_dir != nullptr && screenshot_dir[0] != '\0') {
            // FRAMEGRAB-7-SITIOS: 5 frames de assentamento (COM swap, via
            // present_frame_() de sempre) + o 6o frame renderizado SEM swap
            // ainda (render_frame_()) - a captura via glintfx::UiLayer::
            // capture_frame() PRECISA rodar DEPOIS do render() e ANTES do
            // swap (contrato do pin, ui_layer.hpp) - ler o backbuffer DEPOIS
            // do swap e UNDEFINED em muitas implementacoes GL. O
            // gl3_read_backbuffer_rgba antigo lia DEPOIS do 6o swap e so
            // "funcionava" por um comportamento especifico do Mesa/llvmpipe
            // (swap se comporta como copia, retendo o quadro anterior no
            // back buffer) - nao e um contrato garantido, so um acidente de
            // driver. Mesma contagem de 6 swaps no total (5 aqui + 1 abaixo,
            // apos a captura) - comportamento de apresentacao preservado.
            for (int i = 0; i < 5; ++i) present_frame_();
            render_frame_();
            const glintfx::UiLayer::CapturedFrame captured = ui_->capture_frame();
            if (captured.ok) {
                const std::string suffix = scan_.any_save_exists
                                                ? "title_continue_enabled"
                                                : "title_continue_disabled";
                const std::string out = join(std::string(screenshot_dir), suffix + ".png");
                stbi_write_png(out.c_str(), captured.width, captured.height, 4,
                               captured.pixels.get(), captured.width * 4);
                std::cout << "TitleMenuLoop: [screenshot] " << out << " (" << captured.width
                          << "x" << captured.height << ")\n";
            } else {
                std::cerr << "TitleMenuLoop: [screenshot] capture_frame() falhou\n";
            }
            SDL_GL_SwapWindow(window_);  // completa o 6o frame (capturado logo acima)
            screenshot_only_ = true;
            bailed_ = true;
            return;
        }

        // NAO ha present_frame_() explicito aqui (ao contrario de
        // NpcDialogueScreen): o while(true) ORIGINAL desta tela so desenhava
        // no FIM de cada iteracao, DEPOIS de drenar a rajada de eventos
        // daquele frame - o 1o tick() do runner reproduz exatamente essa 1a
        // iteracao.
    }

    void handle_event(const SDL_Event& ev) override {
        if (bailed_) {
            return;  // defensivo: nao deveria ser chamado (finished()==true).
        }

        std::array<glintfx::ElementBox, kTitleItemCount> boxes{};
        if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN && ev.button.button == SDL_BUTTON_LEFT) {
            boxes = collect_click_boxes_();
        }

        const TitleStepResult step = title_screen_step(state_, ev, boxes.data());

        if (step.window_closed) {
            window_closed_ = true;
            return;
        }
        if (step.resize) {
            SDL_GetWindowSizeInPixels(window_, &pw_, &ph_);
            if (pw_ < 1) pw_ = 1;
            if (ph_ < 1) ph_ = 1;
            dp_ratio_ = static_cast<float>(pw_) / 960.0f;
            ui_->set_viewport(pw_, ph_);
            ui_->set_dp_ratio(dp_ratio_);
            return;
        }
        if (step.mouse_move) {
            handle_mouse_motion_(step.mouse_x, step.mouse_y);
            return;
        }
        // EFEITO DE PRESS (B2, decisao do lider 2026-08-01): quando ha flash
        // pendente, o SOM DE CLIQUE toca DENTRO de flash_pressed_ (MESMO
        // choke-point de system_menu_loop.cpp) - so o Hover (sem flash
        // associado) toca aqui fora, senao o clique soaria 2x.
        if (step.sfx == TitleSfxKind::Hover) {
            audio_.play_sfx(hover_sfx_id_);
        }
        if (step.flash.has_value()) {
            flash_pressed_(step.flash->pre_action_state, step.flash->item_index);
        }
        if (step.exit.has_value()) {
            apply_exit_(*step.exit);
            done_ = true;
            return;
        }
        if (step.reload) {
            reload_();
        }
    }

    void tick(float /*dt*/) override {
        if (bailed_ || done_ || window_closed_) {
            return;  // defensivo: run_screen_state() nao chama tick() quando
                      // finished()/window_closed() ja e true, mas guarda mesmo assim.
        }
        present_frame_();
    }

    [[nodiscard]] bool finished() const override { return bailed_ || done_; }

    void exit() override {
        // EXCLUSIVIDADE DO UILAYER (ver gus/app/screen_state.hpp): destroi
        // ui_ ANTES de backdrop_ (MESMA ordem de sempre) - depois de exit(),
        // o MINI-DRIVER pode abrir a tela de dificuldade (OU re-entrar este
        // MESMO TitleScreen) com seguranca.
        ui_.reset();
        backdrop_.reset();
    }

    [[nodiscard]] bool window_closed() const override { return window_closed_; }

    // Exposto SO pro DRIVER (run_title_menu_loop_gl_current, abaixo) - nao faz
    // parte do contrato ScreenState. O desfecho ContinueGame/NewGameRequested/
    // QuitApp decidido pelo ultimo title_screen_step com exit preenchido
    // (ContinueGame ja passou por apply_exit_, que pode degradar pra
    // NewGameRequested - ver o comentario la).
    [[nodiscard]] TitleScreenExit result() const { return result_; }

    // idem - 2 degradacoes que NAO abrem a tela de dificuldade (ui_->ok()==
    // false / modo screenshot), MESMO contrato do while(true) antigo.
    [[nodiscard]] bool ui_init_failed() const { return ui_init_failed_; }
    [[nodiscard]] bool screenshot_only() const { return screenshot_only_; }

   private:
    // Aplica o desfecho de UM title_screen_step (ContinueGame/NewGameRequested/
    // QuitApp) em result_ - ContinueGame precisa do cache scan_ (que a funcao
    // PURA title_screen_step nao tem) pra de fato preencher *out_loaded_save_;
    // se o cache estiver vazio (BUG defensivo, nunca deveria acontecer -
    // Continuar so e selecionavel quando any_save_exists, que implica
    // most_recent_slot >= 0), degrada pra NewGameRequested (MESMO racional do
    // antigo confirm_continue lambda - nunca finge um load que nao aconteceu).
    void apply_exit_(TitleScreenExit step_exit) {
        if (step_exit != TitleScreenExit::ContinueGame) {
            result_ = step_exit;
            return;
        }
        if (scan_.most_recent_slot >= 0 &&
            scan_.loaded[static_cast<std::size_t>(scan_.most_recent_slot)].has_value()) {
            if (out_loaded_save_ != nullptr) {
                *out_loaded_save_ =
                    *scan_.loaded[static_cast<std::size_t>(scan_.most_recent_slot)];
            }
            result_ = TitleScreenExit::ContinueGame;
        } else {
            std::cerr << "[title_menu_loop] BUG defensivo: ContinueGame sem "
                         "slot no cache - degradando pra Novo Jogo (nao finge "
                         "um load que nao aconteceu).\n";
            result_ = TitleScreenExit::NewGameRequested;
        }
    }

    void reload_() {
        rml_path_ = write_title_rml_file(state_, translator_);
        ui_->load(rml_path_.c_str());
        ui_->set_viewport(pw_, ph_);
        ui_->set_dp_ratio(dp_ratio_);
        ui_->update();  // MESMO assentamento a cada troca de documento
    }

    // EFEITO DE PRESS (B2, decisao do lider 2026-08-01 - paridade com o menu
    // de pausa/system_menu_loop.cpp): renderiza a tela `pre_action_state`
    // (snapshot tirado ANTES da mutacao que ja aconteceu em state_) com o
    // item `item_index` marcado ".pressed", por ~100ms (4 frames de ~25ms) -
    // SO DEPOIS o CHAMADOR (handle_event) segue com exit/reload (ja
    // decididos por title_screen_step, usando state_ JA MUTADO). SOM DE
    // CLIQUE: dispara AQUI (MESMO choke-point do system_menu) - nao ha um
    // 2o play fora disto, ver o comentario em handle_event.
    void flash_pressed_(const TitleMenuState& pre_action_state, int item_index) {
        audio_.play_sfx(click_sfx_id_);
        rml_path_ = write_title_rml_file(pre_action_state, translator_, item_index);
        ui_->load(rml_path_.c_str());
        ui_->set_viewport(pw_, ph_);
        ui_->set_dp_ratio(dp_ratio_);
        for (int frame = 0; frame < 4; ++frame) {
            present_frame_();
            SDL_Delay(25);
        }
    }

    // FRAMEGRAB-7-SITIOS: extraido de present_frame_() (que so agrega o swap
    // por cima) - o corpo isolado permite capturar via glintfx::UiLayer::
    // capture_frame() DEPOIS do render() e ANTES do proprio swap (contrato
    // do pin, ui_layer.hpp), sem duplicar arena+UI a mao no chamador.
    void render_frame_() {
        const gus::core::spatial::Rect cam{0.0f, 0.0f, static_cast<float>(pw_),
                                            static_cast<float>(ph_)};
        backdrop_->begin_frame(cam, pw_, ph_);
        const float boot_bg_elapsed_s =  // FW-CLOCK (bump v0.26.0)
            static_cast<float>(glintfx::monotonic_now_ns() - boot_bg_start_ns_) / 1.0e9f;
        const int boot_bg_frame = gus::core::anim::boot_pixel_idle_frame_index(
            boot_bg_elapsed_s, gus::core::anim::kBootPixelFrameCount);
        boot_bg_.draw_idle(*backdrop_, cam, boot_bg_frame);
        backdrop_->end_frame();
        ui_->update();
        ui_->render();
    }

    void present_frame_() {
        render_frame_();
        SDL_GL_SwapWindow(window_);
    }

    void handle_mouse_motion_(float mx, float my) {
        glintfx::UiEvent hover_ev{};
        hover_ev.type = glintfx::UiEvent::Type::MouseMove;
        hover_ev.x = mx;
        hover_ev.y = my;
        ui_->process_event(hover_ev);
    }

    // Callback NATIVO de hover do glintfx (id-based, RCSS :hover) - MESMA
    // receita do hover_cb lambda antigo, agora metodo (captura `this` em vez
    // de fechar sobre variaveis locais).
    void native_hover_callback_(const char* raw_id, bool entered) {
        const std::string id = raw_id != nullptr ? raw_id : "";
        if (!entered) {
            if (id == last_hover_sfx_id_) last_hover_sfx_id_.clear();
            return;
        }
        if (id == last_hover_sfx_id_ || !is_navigable_hover_id(state_, id)) return;
        last_hover_sfx_id_ = id;
        audio_.play_sfx(hover_sfx_id_);
    }

    // Resolve as boxes de CLIQUE do frame ATUAL - "title-confirm-<i>" (2
    // posicoes, mini-dialogo) OU "title-item-<i>" (kTitleItemCount posicoes,
    // lista), conforme state_.confirming_new_game - SO chamado do ramo
    // MOUSE_BUTTON_DOWN de handle_event() (MESMO custo do while(true) antigo).
    [[nodiscard]] std::array<glintfx::ElementBox, kTitleItemCount> collect_click_boxes_() const {
        std::array<glintfx::ElementBox, kTitleItemCount> boxes{};
        if (state_.confirming_new_game) {
            for (int i = 0; i < 2; ++i) {
                boxes[static_cast<std::size_t>(i)] =
                    ui_->get_element_box(("title-confirm-" + std::to_string(i)).c_str());
            }
        } else {
            for (int i = 0; i < kTitleItemCount; ++i) {
                boxes[static_cast<std::size_t>(i)] =
                    ui_->get_element_box(("title-item-" + std::to_string(i)).c_str());
            }
        }
        return boxes;
    }

    SDL_Window* window_;
    gus::platform::audio::AudioEngine& audio_;
    const gus::app::i18n::Translator& translator_;
    std::string saves_dir_;
    gus::domain::save::SaveData* out_loaded_save_;

    // ESTADO DE NEGOCIO - inicializado UMA VEZ no construtor, PERSISTE entre
    // rodadas do driver (ver o comentario grande da classe acima).
    TitleDiskScan scan_;
    TitleMenuState state_;

    int pw_ = 0;
    int ph_ = 0;
    float dp_ratio_ = 1.0f;

    bool window_closed_ = false;
    bool bailed_ = false;         // ui_init_failed_ OU screenshot_only_ (ver enter())
    bool done_ = false;           // um TitleScreenExit foi decidido por um title_screen_step
    bool ui_init_failed_ = false;
    bool screenshot_only_ = false;

    std::optional<gus::platform::render2d::Render2dGl3> backdrop_;
    // MEMBRO direto (nao optional): so guarda TextureId (ints) - load() roda
    // de novo a cada enter() (ver o comentario la), nao precisa de RAII de
    // ciclo de vida proprio.
    gus::app::BootPixelOverlay boot_bg_;
    unsigned long long boot_bg_start_ns_ = 0;

    std::string stage_;
    std::string rml_path_;

    gus::platform::audio::SoundId hover_sfx_id_ = gus::platform::audio::kInvalidSound;
    gus::platform::audio::SoundId click_sfx_id_ = gus::platform::audio::kInvalidSound;
    std::string last_hover_sfx_id_;

    // Default QuitApp (MESMO fallback documentado no .hpp) - so lido pelo
    // driver quando done_==true (nunca em bailed_==true, ver ui_init_failed()/
    // screenshot_only() acima, que o driver checa ANTES de ler result()).
    TitleScreenExit result_ = TitleScreenExit::QuitApp;

    // std::optional (nao um objeto direto): enter()/exit() controlam o ciclo
    // de vida (EXCLUSIVIDADE DO UILAYER, ver gus/app/screen_state.hpp).
    //
    // ATENCAO - CALLBACK-DTOR-ORDER (2026-08-05): ui_ e o ULTIMO membro DE
    // PROPOSITO. Nao mova pra cima "por organizacao" - a posicao E o conserto.
    // Membro morre na ordem INVERSA da declaracao, entao declarado por ultimo o
    // ui_ e o PRIMEIRO a ser destruido, ANTES de last_hover_sfx_id_/state_ -
    // exatamente o que a lambda registrada em set_hover_callback() alcanca (via
    // native_hover_callback_). Importa porque ~UiLayer() descarrega os
    // documentos do RmlUi, o que RECALCULA a hover chain e pode emitir UM
    // ultimo evento de hover: o glintfx levou esse mesmo use-after-free ao vivo
    // (achado so pelo ASan; passou na revisao, na suite local e em 3 dos 4 jobs
    // do CI pesado deles). No caminho normal exit() ja destroi ui_ com tudo
    // vivo; a janela e o caminho em que exit() NAO roda - ele esta solto na
    // ultima linha de run_screen_state() (app/src/screen_state.cpp), FORA de
    // guard RAII, e uma excecao escapando de enter()/tick()/handle_event()
    // desenrola a pilha, pula o exit() e vai direto pra ~TitleScreen. Ali so a
    // ordem de declaracao protege. Bonus: alinha a destruicao implicita com o
    // que exit() ja faz a mao (ui_.reset() ANTES de backdrop_.reset()).
    // Gate automatico: tools/callback_dtor_order.py (roda no tools/check.sh).
    std::optional<glintfx::UiLayer> ui_;
};

}  // namespace

// F4-1b.3 MINI-DRIVER: o NUCLEO que ASSUME um contexto GL JA CORRENTE + glad
// JA CARREGADO (FLASH-CTX, A2) - ver o comentario grande no topo deste arquivo
// pro racional completo de POR QUE existe um driver aqui (a guarda de
// reentrada de run_screen_state PROIBE abrir a tela de dificuldade de DENTRO
// do handler da tela de titulo).
void run_title_menu_loop_gl_current(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator, const std::string& saves_dir,
    TitleLoopExit* out_exit, gus::domain::save::SaveData* out_loaded_save,
    gus::domain::save::DifficultyLevel* out_new_game_difficulty,
    const std::string& frozen_background_png, const gus::app::EventSyncHook& sync_hook) {
    // DIAGNOSTICO/PROVA (MODOS-MORTE Fase 0, prova visual headless Xvfb :99):
    // GUSWORLD_DIFFICULTY_SCREENSHOT_DIR=<dir> pula a tela de titulo por
    // completo e abre DIRETO a tela de selecao de dificuldade (ANINHADA no
    // MESMO contexto GL) - roda ANTES de sequer construir o TitleScreen (nunca
    // entra no par enter()/exit() da tela de titulo). MESMO espirito de
    // GUSWORLD_TITLE_SCREENSHOT_DIR (checado dentro de TitleScreen::enter()),
    // so que mirando a tela seguinte do fluxo de Novo Jogo.
    if (const char* diff_screenshot_dir = std::getenv("GUSWORLD_DIFFICULTY_SCREENSHOT_DIR");
        diff_screenshot_dir != nullptr && diff_screenshot_dir[0] != '\0') {
        gus::domain::save::DifficultyLevel dummy = gus::domain::save::DifficultyLevel::Medio;
        (void)run_difficulty_menu_loop_gl_current(window, audio, translator, &dummy,
                                                   frozen_background_png);
        *out_exit = TitleLoopExit::NewGame;
        return;
    }

    // TitleScreen PERSISTE entre iteracoes deste driver (objeto local da
    // PILHA do wrapper, NAO recriado a cada volta) - scan_saves()/
    // title_menu_open() rodaram UMA VEZ no construtor (ver a classe acima).
    TitleScreen title_screen(window, audio, translator, saves_dir, out_loaded_save);

    for (;;) {
        // run_screen_state SO retorna DEPOIS que title_screen.exit() ja rodou
        // (garantia ESTRUTURAL, ver gus/app/screen_state.hpp:29-39) - so
        // ENTAO e seguro abrir a tela de dificuldade ANINHADA logo abaixo (2a
        // UiLayer viva ao mesmo tempo e o crash real que essa exclusividade
        // existe pra prevenir).
        gus::app::run_screen_state(title_screen, sync_hook);

        if (title_screen.window_closed()) {
            *out_exit = TitleLoopExit::QuitApp;
            return;
        }
        if (title_screen.ui_init_failed()) {
            *out_exit = TitleLoopExit::NewGame;
            return;
        }
        if (title_screen.screenshot_only()) {
            *out_exit = TitleLoopExit::QuitApp;
            return;
        }

        const TitleScreenExit texit = title_screen.result();
        gus::domain::save::DifficultyLevel chosen = gus::domain::save::DifficultyLevel::Medio;
        // Irrelevante fora de NewGameRequested (title_flow_next ignora este
        // valor nos outros 2 casos, ver os testes de title_flow_next) - valor
        // qualquer (Cancelled) pra nao deixar a variavel indeterminada.
        DifficultyLoopExit dexit = DifficultyLoopExit::Cancelled;
        if (texit == TitleScreenExit::NewGameRequested) {
            // ANINHADA (MESMA tecnica do antigo start_new_game_via_difficulty_
            // menu): a UiLayer do titulo JA foi destruida (title_screen.exit()
            // rodou dentro do run_screen_state acima) - seguro criar a da
            // dificuldade agora.
            dexit = run_difficulty_menu_loop_gl_current(window, audio, translator, &chosen,
                                                         frozen_background_png);
        }

        switch (title_flow_next(texit, dexit)) {
            case TitleFlowStep::ContinueGame:
                *out_exit = TitleLoopExit::ContinueGame;
                return;
            case TitleFlowStep::QuitApp:
                *out_exit = TitleLoopExit::QuitApp;
                return;
            case TitleFlowStep::StartNewGame:
                *out_exit = TitleLoopExit::NewGame;
                if (out_new_game_difficulty != nullptr) {
                    *out_new_game_difficulty = chosen;
                }
                return;
            case TitleFlowStep::RetryTitle:
                break;  // RE-ENTRA no MESMO title_screen (topo do for(;;)).
        }
    }
}

}  // namespace gus::app::screens
