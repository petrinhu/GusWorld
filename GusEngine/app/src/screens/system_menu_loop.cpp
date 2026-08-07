// SPDX-License-Identifier: Apache-2.0
// gus/app/src/screens/system_menu_loop.cpp
//
// Implementacao do loop interativo do MENU DE SISTEMA. Ver header para o contrato
// completo.
//
// F4-1b.4 (onda F4 "casca SDL -> App mode do glintfx", fatia 1b.4 - SEGUNDA tela
// PAI da onda, apos F4-1b.3/title_menu_loop.cpp): o caminho de PRODUCAO
// (run_system_menu_loop_gl_current) foi convertido de um while(true){SDL_
// PollEvent...} PROPRIO pra uma classe SystemMenuScreen (gus::app::ScreenState) +
// gus::app::run_screen_state - MESMA tecnica de TitleScreen/DifficultyScreen/
// SaveLoadScreen. O roteamento de evento (decisao PURA) foi extraido pra
// system_screen_step (declarada no .hpp, testada headless em
// system_screen_step_test.cpp) - MESMO racional de title_screen_step.
//
// O PROBLEMA NOVO desta fatia (MESMO racional de F4-1b.3): o Pause dispara a
// tela de SALVAR/CARREGAR (save_load_menu_loop.hpp) ANINHADA quando o jogador
// confirma "Salvar"/"Carregar" (SystemMenuAction::OpenSaveLoadSave/Load). ANTES
// desta fatia, isso acontecia de DENTRO do handler do Pause (o antigo
// handle_action lambda, chamando run_save_load_menu_loop_gl_current no MEIO do
// proprio corpo, incluindo o velho par ui_opt.reset()/ui_opt.emplace() cruzando a
// fronteira de telas) - o que agora VIOLARIA a guarda de reentrada de
// gus::app::run_screen_state() (screen_state.hpp:29-39: run_screen_state(pai)
// precisa RETORNAR - rodando pai.exit(), que libera a UiLayer do pai - ANTES de
// qualquer codigo chamar run_screen_state(filha)).
//
// A SOLUCAO (MESMO MINI-DRIVER de title_menu_loop.cpp): SystemMenuScreen::
// handle_event() SO devolve um SystemMenuScreenExit (ver o .hpp) - nunca chama a
// tela de save/load. O DRIVER, dentro do wrapper (run_system_menu_loop_gl_current):
//   1) chama gus::app::run_screen_state(system_screen) - ela SO retorna DEPOIS
//      que system_screen.exit() ja rodou (garantia ESTRUTURAL);
//   2) SO ENTAO, se o desfecho foi OpenSaveLoadSave/OpenSaveLoadLoad, chama
//      run_save_load_menu_loop_gl_current (a UiLayer do Pause ja foi destruida -
//      seguro criar a 2a);
//   3) decide o proximo passo via a funcao PURA pause_flow_next() (testada
//      headless em system_screen_step_test.cpp);
//   4) se a tela de save/load devolveu BackToPause, RE-USA o MESMO objeto
//      SystemMenuScreen, chamando run_screen_state(system_screen) de novo.
//
// SystemMenuScreen PERSISTE entre iteracoes do driver (objeto local da PILHA do
// wrapper, NAO recriado a cada volta) - o ESTADO DE NEGOCIO (SystemMenuState
// state_: tela atual/selecoes/volume/copia de trabalho de Controles) e
// inicializado no CONSTRUTOR, NAO em enter() - so ASSIM "voltar do save/load"
// preserva a tela/selecao/staged changes ATUAIS (nao reabre sempre em Pause com
// foco em Continuar, nao perde um remap de tecla ainda nao aplicado) - MESMO
// comportamento observavel do while(true) antigo. enter()/exit() SO cuidam de
// recursos GL (glintfx::UiLayer, Render2dGl3, ids de SFX) - a exclusividade da
// UiLayer (gus/app/screen_state.hpp) vira consequencia ESTRUTURAL de
// system_screen.exit() acontecer (dentro de run_screen_state) ANTES do driver
// sequer cogitar abrir a tela de save/load, substituindo o antigo
// ui_opt.reset()/emplace() manual cruzando a fronteira de telas.
//
// EFEITO DE PRESS (MENU-PAUSA-CONFIG-SOM, onda arvore): quando o jogador aciona
// uma pill/categoria/Voltar (Enter/Espaco no TECLADO ou clique de MOUSE), o loop
// renderiza ALGUNS FRAMES com a classe "pressed" no item ativado (flash cyan
// intenso, ver .verb-pill.pressed/.btn-back.pressed em system_menu_rml.cpp) ANTES
// de aplicar a transicao de fato (trocar de tela/fechar o menu/pedir Sair). Isto e
// deliberadamente um efeito NOSSO (nao do glintfx - a lib nao tem estado "active"
// disparado por teclado, so :focus/:hover via classe) - flash_pressed_() abaixo e
// o UNICO lugar que gera esse frame extra. O SOM DE CLIQUE dispara DENTRO dele -
// nao ha um SystemMenuSfxKind::Click separado (ver o .hpp).
//
// HOVER NATIVO + SOM DE HOVER/CLIQUE (retoque ao vivo do lider, pos-ONDA ARVORE;
// SOM DE HOVER de MOUSE migrado pro callback NATIVO em SFX-MIGRATE-V0.9): o
// VISUAL do hover e 100% :hover nativo do glintfx (RCSS em system_menu_rml.cpp) -
// so precisamos injetar UiEvent::MouseMove em TODO SDL_EVENT_MOUSE_MOTION, MESMO
// pipeline ja em producao no cockpit da batalha. O SOM DE HOVER (mouse) usa
// glintfx::UiLayer::set_hover_callback - a glintfx JA deduplica o proprio fan-out
// de Mouseover internamente e so invoca o callback na TRANSICAO real;
// is_navigable_hover_id (POCO, abaixo) filtra pra so os ids de item NAVEGAVEL da
// tela ATUAL soarem. O hover de TECLADO (sem mouse) continua na MESMA dupla
// system_menu_hover_entered_new_item/system_menu_keyboard_focus_index de antes -
// agora decidido DENTRO de system_screen_step (SystemMenuSfxKind::Hover).

#include "gus/app/screens/system_menu_loop.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include <glintfx/element_box.hpp>
#include <glintfx/ui_layer.hpp>

#include "gus/app/screens/save_load_menu_loop.hpp"  // SAVE-LOAD-UI etapa 6: tela real
#include "gus/app/screens/system_menu.hpp"
#include "gus/app/screens/system_menu_rml.hpp"
#include "gus/core/asset_paths.hpp"            // kMenuHoverSfxFile/kMenuClickSfxFile/kSfxDir
#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Rect
#include "gus/domain/input/controls_name.hpp"     // kDefaultProfile (tela Controles, M2)
#include "gus/domain/settings/system_settings.hpp"
#include "gus/platform/assets/asset_source.hpp"  // ASSETS-VFS-F1 (ADR-013): porteiro
#include "gus/platform/fs/controls_file_store.hpp"  // load_controls/save_controls (M2)
#include "gus/platform/fs/save_file_store.hpp"  // SAVE-LOAD-UI etapa 6: save_game (selftest)
#include "gus/platform/fs/settings_file_store.hpp"
#include "gus/platform/input/key_translation.hpp"  // sdl_key_to_godot_keycode (captura, M2)
#include "gus/platform/input/key_translation_glintfx.hpp"  // godot_keycode_to_glintfx_key (Fatia 2)
#include "gus/platform/render2d/render2d_gl3.hpp"

// stb_image_write: SO a declaracao aqui (a IMPLEMENTACAO ja vive UMA vez em
// battle_preview.cpp, MESMA lib gusengine_app - nao redefinir
// STB_IMAGE_WRITE_IMPLEMENTATION aqui, senao da symbol duplicado no link).
#include "stb_image_write.h"

// ASSETS-FONTE-TELAS-GEMEO (2026-08-05): a macro GUSWORLD_FONTS_DIR NAO e mais lida aqui
// (nem o getenv("GUSWORLD_FONTS") a mao). O staging continua sendo ESCRITA (e portanto
// fora do porteiro, que e so-leitura por decisao do ADR-013), mas quem RESOLVE o caminho
// de origem passou a ser o porteiro - via stage_ui_fonts. Ver gus/app/screens/font_stage.hpp.
#include "gus/app/screens/font_stage.hpp"

namespace gus::app::screens {

namespace {

namespace fs = std::filesystem;

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

// Diretorio de stage do RML do menu (mesma receita de glintfx_cockpit_stage_dir em
// battle_preview.cpp: tempfile - o glintfx carrega documento por PATH). Nome
// PROPRIO (nao colide com o stage do cockpit da batalha).
std::string menu_stage_dir() {
    return (fs::temp_directory_path() / "gusworld_glintfx_sysmenu").string();
}

// Escreve o RML do menu (build_system_menu_rml, ver system_menu_rml.hpp) num arquivo
// dentro do stage. Copia as 2 fontes pro stage via stage_ui_fonts (font_stage.hpp), que
// resolve a origem pelo PORTEIRO de assets - MESMA cadeia do resto do app/, agora de
// verdade e nao por copia (ASSETS-FONTE-TELAS-GEMEO). Devolve o path do .rml escrito. `pressed_index` repassado direto pra
// build_system_menu_rml (ver seu header) - default -1 (nenhum item pressionado).
//
// FONT-EXTEND-GLITCH (2026-07-29): a fonte NAO e mais injetada aqui via @font-face de
// string - SystemMenuLoopScreen::enter() registra a familia 1x via
// glintfx::UiLayer::load_font_face (API v0.24.0) logo apos set_asset_base_url (ver
// register_pixel_operator_mono_fonts abaixo). Os .ttf continuam copiados pro stage
// (load_font_face resolve o path pela MESMA BaseUrlFileInterface que a @font-face
// antiga usava).
std::string write_system_menu_rml_file(const SystemMenuState& state,
                                        const gus::app::i18n::Translator& tr,
                                        int pressed_index = -1) {
    const fs::path stage = menu_stage_dir();
    std::error_code ec;
    fs::create_directories(stage, ec);

    // ASSETS-FONTE-TELAS-GEMEO: staging das 2 fontes pelo helper unico (font_stage.hpp),
    // que resolve pelo PORTEIRO (cascata checada) e LOGA a falha de copia. Antes, este
    // bloco lia a macro GUSWORLD_FONTS_DIR crua (caminho da maquina de BUILD) sem checar
    // existencia e com o error_code descartado - falha 100% silenciosa. Retorno ignorado
    // de proposito: fonte ausente degrada e o helper ja logou.
    stage_ui_fonts(stage);

    std::string rml = build_system_menu_rml(state, tr, pressed_index);

    const fs::path out = stage / "system_menu.rml";
    std::ofstream f(out);
    f << rml;
    return out.string();
}

// FONT-EXTEND-GLITCH (2026-07-29): registra "Pixel Operator Mono" (regular+bold) via
// glintfx::UiLayer::load_font_face (API v0.24.0) - mata o @font-face por string que
// write_system_menu_rml_file injetava antes. Chamado 1x em enter() (registro e
// process-wide RmlUi state, ver ui_layer.hpp), DEPOIS do stage ja ter os .ttf (o
// PRIMEIRO write_system_menu_rml_file os copia) e ANTES de ui_->load(). `family`
// SEMPRE explicito (armadilha de assimetria de motor Own-vs-FreeType documentada em
// font_face.hpp - o mesmo literal que o RCSS ja referencia neutraliza os dois).
void register_pixel_operator_mono_fonts(glintfx::UiLayer& ui) {
    if (!ui.load_font_face(glintfx::FontFaceDesc{/*path=*/"PixelOperatorMono.ttf",
                                                  /*family=*/"Pixel Operator Mono"})) {
        std::cerr << "SystemMenuLoop: load_font_face(PixelOperatorMono.ttf) falhou "
                     "(arquivo ausente/invalido no stage) - cai no fallback de fonte do "
                     "RmlUi.\n";
    }
    if (!ui.load_font_face(glintfx::FontFaceDesc{
            /*path=*/"PixelOperatorMono-Bold.ttf",
            /*family=*/"Pixel Operator Mono",
            /*style=*/glintfx::FontStyle::Normal,
            /*weight=*/glintfx::FontWeight::Bold})) {
        std::cerr << "SystemMenuLoop: load_font_face(PixelOperatorMono-Bold.ttf) "
                     "falhou (arquivo ausente/invalido no stage) - cai no fallback de "
                     "fonte do RmlUi.\n";
    }
}

void apply_and_persist(const SystemMenuState& state,
                        gus::platform::audio::AudioEngine& audio,
                        const std::string& settings_dir) {
    audio.set_music_volume(state.music_volume);
    audio.set_sfx_volume(state.sfx_volume);
    gus::domain::settings::SystemSettings settings;
    settings.music_volume = state.music_volume;
    settings.sfx_volume = state.sfx_volume;
    if (!gus::platform::fs::save_system_settings(settings, settings_dir)) {
        // Best-effort: o volume ja vale nesta sessao (aplicado no AudioEngine acima);
        // so nao persistiu (ex. disco cheio / permissao). Nao e fatal.
        std::cerr << "[system_menu] aviso: falha ao salvar settings.json "
                     "(volume vale nesta sessao, mas nao persistiu)\n";
    }
}

// Persiste state.controls_config em "<perfil>_controls.json" (tela Controles,
// M2 STAGED CHANGES - MESMO padrao best-effort de apply_and_persist acima:
// falha de I/O so loga, a copia de trabalho em MEMORIA continua valendo pro
// resto da sessao). Chamada SO quando o CHAMADOR ve SystemMenuAction::
// ControlsApplied ("Aplicar" confirmado, ver system_screen_step) - remap/
// restaurar-padrao isolados (ControlsChanged) NAO chegam mais aqui (o modelo
// antigo "aplica na hora" foi trocado por mudancas preparadas + Aplicar
// explicito). Perfil UNICO "default" nesta onda (nao ha selecao de jogador na
// UI ainda - ADR-007 fork 3 preve multi-perfil, mas a tela nao expoe essa
// escolha; residuo sinalizado).
void persist_controls(const SystemMenuState& state, const std::string& settings_dir) {
    if (!gus::platform::fs::save_controls(
            state.controls_config, settings_dir,
            std::string(gus::domain::input::kDefaultProfile))) {
        std::cerr << "[system_menu] aviso: falha ao salvar controls.json (Aplicar "
                     "vale nesta sessao/em memoria, mas nao persistiu em disco)\n";
    }
}

// Track ids (system_menu_rml.cpp: "slider-track-<indice>", indice = AudioItem).
std::string track_id_for_item(int item) {
    return "slider-track-" + std::to_string(item);
}

// Ids das PILLS do Pause / categorias de ConfigCategories / campos+Voltar do
// Audio (system_menu_rml.cpp: "pause-item-<indice>"/"category-item-<indice>"/
// "audio-item-<indice>" - clique de mouse aciona/foca a opcao). O Voltar das 3
// telas placeholder usa 1 UNICO id fixo ("placeholder-back", ver
// build_placeholder_body): so 1 placeholder fica carregado por vez.
std::string pause_item_id(int item) {
    return "pause-item-" + std::to_string(item);
}
// MENU-INICIAL: ids das pills Sim/Cancelar do mini-dialogo "voltar ao menu
// inicial?" (system_menu_rml.cpp: build_pause_body, ramo pause_confirming_to_title)
// - MESMA convencao de controls_confirm_id abaixo.
std::string pause_totitle_confirm_id(int item) {
    return "pause-totitle-confirm-" + std::to_string(item);
}
std::string category_item_id(int item) {
    return "category-item-" + std::to_string(item);
}
std::string audio_item_id(int item) {
    return "audio-item-" + std::to_string(item);
}
constexpr const char* kPlaceholderBackId = "placeholder-back";

// Ids da tela Controles (system_menu_rml.cpp: build_controls_body):
// "controls-item-<indice>" (0..kControlsActionCount-1 = action,
// kControlsRestoreIndex/kControlsApplyIndex/kControlsBackIndex = rodape) na
// navegacao normal; "controls-confirm-<0|1>" (Sim/Nao) no mini-dialogo de
// restaurar-padrao; "controls-discard-confirm-<0|1>" (Sim/Nao, M2 STAGED
// CHANGES) no mini-dialogo de descartar alteracoes nao aplicadas (Voltar/Esc
// com mudanca pendente).
std::string controls_item_id(int item) {
    return "controls-item-" + std::to_string(item);
}
std::string controls_confirm_id(int item) {
    return "controls-confirm-" + std::to_string(item);
}
std::string controls_discard_confirm_id(int item) {
    return "controls-discard-confirm-" + std::to_string(item);
}

// Id de `.ctrl-list` (system_menu_rml.cpp: build_controls_body) - consultado
// pra filtrar hit-test/hover das 30 actions pelo recorte VISIVEL da lista
// rolavel (ver controls_row_visible_in_list/BUG-A no header de system_menu.hpp).
constexpr const char* kControlsListId = "ctrl-list";

// BUG-A (ver o comentario extenso de controls_row_visible_in_list em
// system_menu.hpp): filtra `box` pra "nao encontrado" (found=false) se `index`
// e uma ACTION da lista rolavel (0..kControlsActionCount-1 - o rodape,
// kControlsRestoreIndex/kControlsBackIndex, fica FORA da lista e nunca e
// filtrado) E a caixa REAL nao tem nenhuma sobreposicao com o recorte visivel
// de `.ctrl-list` no momento (linha rolada pra fora da vista, cuja geometria
// de layout pode coincidir com a posicao do rodape - ver o achado empirico no
// header). Devolve `box` inalterado se list_box.found==false (defensivo - sem
// referencia, nao filtra nada) ou se `index` for do rodape.
glintfx::ElementBox filter_offscreen_controls_row(int index, glintfx::ElementBox box,
                                                   const glintfx::ElementBox& list_box) {
    if (index >= kControlsActionCount || !box.found || !list_box.found) {
        return box;  // rodape (sempre valido) OU ja nao encontrado OU sem lista pra comparar
    }
    if (!controls_row_visible_in_list(box.y, box.h, list_box.y, list_box.h)) {
        box.found = false;  // rolada pra fora da vista - nao conta hit-test/hover (BUG-A)
    }
    return box;
}

// Hit-test simples: cursor (x,y, espaco-janela) dentro da caixa border-box
// devolvida por glintfx::UiLayer::get_element_box (MESMO espaco de coordenadas
// - ver docs/embed-integration.md secao 10, ja citado em outros comentarios
// deste arquivo). box.found=false conta como "fora". PURA - usada tanto por
// system_screen_step (roteamento de clique) quanto (indiretamente, via boxes ja
// resolvidas) pelo resto deste arquivo.
bool hit_test(const glintfx::ElementBox& box, float x, float y) noexcept {
    if (!box.found) return false;
    return x >= box.x && x <= box.x + box.w && y >= box.y && y <= box.y + box.h;
}

// Resolve o caminho de um SFX do menu (hover/click) - familia SFX, MESMO destino de
// resolve_hit_sfx_path em battle_preview.cpp. ASSETS-VFS-F1 (ADR-013): a cadeia `env
// GUSWORLD_SFX > macro GUSWORLD_SFX_DIR > CWD (kSfxDir)` foi CONSOLIDADA em
// FilesystemAssetSource (dispatch pelo prefixo "assets/sfx/"). Assinatura INTOCADA.
std::string resolve_menu_sfx_path(std::string_view file) {
    const std::string id = join(std::string(gus::core::assets::kSfxDir), std::string(file));
    return gus::platform::assets::FilesystemAssetSource().resolve_path(id);
}

// COCKPIT-SFX-HOVER-CLIQUE / SFX-MIGRATE-V0.9: filtro NAVEGAVEL pro callback
// NATIVO de hover (glintfx::UiLayer::set_hover_callback) - dado o `id` que o
// hover nativo reportou (entered=true) e a tela ATUAL, devolve true SO se `id`
// e um dos itens hover-testaveis por CLIQUE (MESMOS ids de pause_item_id/
// category_item_id/audio_item_id/kPlaceholderBackId/controls_item_id/
// controls_confirm_id/controls_discard_confirm_id) - o hover nativo do RmlUi
// tambem resolve ids de CONTAINER (#sysmenu-panel, .ctrl-list, slider-track-N,
// ver system_menu_rml.cpp) que NUNCA devem tocar o SFX de hover. NAO precisa
// filtrar linhas roladas pra fora da vista (BUG-A): o hover NATIVO so resolve
// `id` pra um elemento de fato PINTADO sob o cursor.
bool is_navigable_hover_id(const SystemMenuState& state, const std::string& id) {
    switch (state.screen) {
        case SystemMenuScreen::Pause:
            // MENU-INICIAL: enquanto o mini-dialogo esta aberto, so as 2 pills
            // Sim/Cancelar sao navegaveis (MESMA convencao de Controls abaixo).
            if (state.pause_confirming_to_title) {
                return id == pause_totitle_confirm_id(0) ||
                       id == pause_totitle_confirm_id(1);
            }
            for (int i = 0; i < kPauseItemCount; ++i) {
                if (id == pause_item_id(i)) return true;
            }
            return false;
        case SystemMenuScreen::ConfigCategories:
            for (int i = 0; i < kConfigCategoriesItemCount; ++i) {
                if (id == category_item_id(i)) return true;
            }
            return false;
        case SystemMenuScreen::Audio:
            for (int i = 0; i < kAudioItemCount; ++i) {
                if (id == audio_item_id(i)) return true;
            }
            return false;
        case SystemMenuScreen::Controls:
            // Confirmando o restaurar-padrao OU o descarte (M2 STAGED CHANGES):
            // so as 2 pills do mini-dialogo correspondente. Capturando: nenhum
            // item navegavel ("Pressione uma tecla...").
            if (state.controls_confirming_restore) {
                return id == controls_confirm_id(0) || id == controls_confirm_id(1);
            }
            if (state.controls_confirming_discard) {
                return id == controls_discard_confirm_id(0) ||
                       id == controls_discard_confirm_id(1);
            }
            if (state.controls_capturing) return false;
            for (int i = 0; i < kControlsItemCount; ++i) {
                if (id == controls_item_id(i)) return true;
            }
            return false;
        case SystemMenuScreen::Video:
        case SystemMenuScreen::Language:
            return id == kPlaceholderBackId;
        case SystemMenuScreen::Hidden:
            return false;  // menu fechado - nada navegavel
    }
    return false;
}

// F4-1b.4: indice do item que RECEBE o flash de PRESS quando um confirm-key
// (Enter/Espaco) e apertado, ATUAL (ANTES da mutacao) - MESMO switch do antigo
// bloco is_confirm_key do while(true), agora extraido pra funcao PURA usada por
// system_screen_step.
int confirm_item_index(const SystemMenuState& state) noexcept {
    switch (state.screen) {
        case SystemMenuScreen::Pause:
            // MENU-INICIAL: enquanto o mini-dialogo esta aberto, o item
            // pressionado e a pill Sim/Cancelar (MESMA convencao de
            // Controls::controls_confirming_restore abaixo).
            return state.pause_confirming_to_title ? state.pause_to_title_confirm_selected
                                                    : state.pause_selected;
        case SystemMenuScreen::ConfigCategories:
            return state.config_categories_selected;
        case SystemMenuScreen::Audio:
            return state.audio_selected;
        case SystemMenuScreen::Controls:
            // Confirmando o restaurar-padrao OU o descarte (M2 STAGED CHANGES):
            // o item pressionado e a pill Sim/Nao do mini-dialogo
            // correspondente (0|1); caso contrario, a acao/rodape selecionado
            // normalmente (inclui Aplicar, kControlsApplyIndex).
            // controls_capturing nunca chega aqui (interceptado ANTES, ver
            // system_screen_step).
            if (state.controls_confirming_restore) return state.controls_restore_confirm_selected;
            if (state.controls_confirming_discard) return state.controls_discard_confirm_selected;
            return state.controls_selected;
        case SystemMenuScreen::Video:
        case SystemMenuScreen::Language:
            return kPlaceholderBackIndex;
        case SystemMenuScreen::Hidden:
            return -1;
    }
    return -1;
}

// Confirma se `action` merece o flash de PRESS (ver topo do arquivo): SO as
// acoes que de fato "acionam uma opcao" (pill/categoria/Voltar/Aplicar) -
// nunca VolumeChanged (drag de slider nao pisca) nem None (nao aconteceu
// nada). ControlsChanged (M2) SOMA aqui: confirmar "Sim" no restaurar-padrao e
// uma acao destrutiva (reseta a copia de trabalho) - merece o mesmo flash de
// confirmacao das demais. ControlsApplied (M2 STAGED CHANGES) SOMA aqui
// tambem: "Aplicar" e a acao mais importante da tela - merece o mesmo flash.
bool is_confirming(SystemMenuAction action) noexcept {
    return action == SystemMenuAction::Continue ||
           action == SystemMenuAction::RequestQuit ||
           action == SystemMenuAction::RequestToTitle ||
           action == SystemMenuAction::Navigated ||
           action == SystemMenuAction::ControlsChanged ||
           action == SystemMenuAction::ControlsApplied;
}

// F4-1b.4: aplica o DESFECHO de uma SystemMenuAction em SystemMenuStepResult -
// MESMO dispatch do antigo `handle_action` lambda, so que gravando em campos da
// struct de retorno (reload/volume_changed/controls_applied/exit) em vez de
// executar os side effects na hora (esses ficam com o CHAMADOR, ver
// SystemMenuScreen::handle_event). NUNCA mexe em `result.sfx`/`result.flash` -
// quem decide isso e o CHAMADOR de system_screen_step (edge-detect de hover na
// navegacao, is_confirming no confirm-key/clique).
void apply_system_menu_action_to_result(SystemMenuStepResult& result,
                                          SystemMenuAction action) noexcept {
    switch (action) {
        case SystemMenuAction::None:
            result.reload = true;
            return;
        case SystemMenuAction::Continue:
            result.exit = SystemMenuScreenExit::Continue;
            return;
        case SystemMenuAction::RequestQuit:
            result.exit = SystemMenuScreenExit::RequestQuit;
            return;
        case SystemMenuAction::VolumeChanged:
            result.volume_changed = true;
            result.reload = true;
            return;
        case SystemMenuAction::Navigated:
            result.reload = true;
            return;
        case SystemMenuAction::ControlsChanged:
            result.reload = true;
            return;
        case SystemMenuAction::ControlsApplied:
            result.controls_applied = true;
            result.reload = true;
            return;
        case SystemMenuAction::OpenSaveLoadSave:
            result.exit = SystemMenuScreenExit::OpenSaveLoadSave;
            return;
        case SystemMenuAction::OpenSaveLoadLoad:
            result.exit = SystemMenuScreenExit::OpenSaveLoadLoad;
            return;
        case SystemMenuAction::RequestToTitle:
            result.exit = SystemMenuScreenExit::RequestToTitle;
            return;
    }
}

// Traduz SDL_Keycode -> glintfx::Key REUSANDO a ponte SDL->Godot->glintfx JA
// EXISTENTE e testada adversarialmente (F4-2, 8/8 mutantes mortos):
// platform/input/key_translation.hpp (SDL_Keycode -> keycode Godot) +
// platform/input/key_translation_glintfx.hpp (keycode Godot -> glintfx::Key).
// Evita duplicar uma 3a tabela SDLK_*->glintfx::Key so pra esta tela
// (M9-CAMADAS-SDL Fatia 2, docs/tech/plano-camadas-sdl.md) - system_menu.hpp
// (a logica PURA) so conhece glintfx::Key, o SDL fica 100% aqui no loop.
//
// SDLK_RETURN e SDLK_KP_ENTER colapsam no MESMO Key::Enter pela ponte Godot
// (key_translation.cpp ja fundia as duas no MESMO kGodotEnter, ANTES desta
// fatia) - preserva EXATAMENTE o comportamento anterior (system_menu.cpp ja
// tratava as duas teclas como sinonimo, fallthrough de switch - 8 sites).
// glintfx::Key nao tem KpEnter dedicado - lacuna ja reportada ao glintfx.
glintfx::Key sdl_keycode_to_menu_key(SDL_Keycode sdl_key) noexcept {
    const long long godot =
        gus::platform::input::sdl_key_to_godot_keycode(static_cast<int>(sdl_key));
    return gus::platform::input::godot_keycode_to_glintfx_key(godot)
        .value_or(glintfx::Key::None);
}

}  // namespace

// F4-1b.4: implementacao de system_screen_step (declarada no .hpp) - ver o
// comentario grande la pro contrato completo. Extracao BEHAVIOR-PRESERVING do
// corpo do while(true) antigo (MESMA ordem de checagem de tipo de evento, MESMAS
// chamadas a system_menu_key_down/system_menu_click_option/system_menu_
// controls_capture_key/system_menu_set_slider_ratio) - so devolvendo a DECISAO
// em vez de executar os side effects na hora.
//
// PONTO CRITICO #1 (preservado EXATAMENTE): a interceptacao de captura de tecla
// da tela Controles (state.screen==Controls && state.controls_capturing) roda
// ANTES de qualquer roteamento generico de KEY_DOWN - um mutante classico e
// inverter essa ordem (ver system_screen_step_test.cpp, caso "ORDEM captura").
SystemMenuStepResult system_screen_step(SystemMenuState& state, const SDL_Event& ev,
                                          const SystemMenuStepBoxes& boxes) noexcept {
    SystemMenuStepResult result;

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
        // PONTO CRITICO #1: MODO DE CAPTURA (tela Controles, M2) intercepta
        // ANTES do roteamento generico abaixo - toda tecla (nao so UP/DOWN/
        // ENTER/ESC com o significado especial de navegacao) e candidata a
        // virar o novo binding.
        if (state.screen == SystemMenuScreen::Controls && state.controls_capturing) {
            const bool is_escape = (ev.key.key == SDLK_ESCAPE);
            const long long godot_keycode =
                is_escape ? 0
                          : gus::platform::input::sdl_key_to_godot_keycode(
                                static_cast<int>(ev.key.key));
            const SystemMenuAction action =
                system_menu_controls_capture_key(state, is_escape, godot_keycode);
            apply_system_menu_action_to_result(result, action);
            return result;
        }

        const bool is_confirm_key = (ev.key.key == SDLK_RETURN || ev.key.key == SDLK_KP_ENTER ||
                                      ev.key.key == SDLK_SPACE);
        if (is_confirm_key) {
            // Enter/Espaco: captura a tela+item ATUAIS (antes da mutacao) pra
            // poder desenhar o flash de PRESS na tela DE ORIGEM caso a action
            // resultante confirme algo (ver is_confirming acima).
            const SystemMenuState pre_action_state = state;
            const int item_index = confirm_item_index(state);
            const SystemMenuAction action =
                system_menu_key_down(state, sdl_keycode_to_menu_key(ev.key.key));
            if (is_confirming(action)) {
                result.flash = SystemMenuFlashInfo{pre_action_state, item_index};
            }
            apply_system_menu_action_to_result(result, action);
        } else {
            // Navegacao (setas/WASD/LEFT/RIGHT/ESC) - SOM DE HOVER PARIDADE
            // TECLADO x MOUSE: move a selecao e, SO se a TELA nao mudou E moveu
            // pra um item NOVO, toca hover_sfx. Trocar de TELA (ex.: ESC subindo
            // um nivel) NAO conta como "navegar pra um item novo" (comparar
            // indices ENTRE telas diferentes nao faz sentido).
            const SystemMenuScreen screen_before = state.screen;
            const int kb_index_before = system_menu_keyboard_focus_index(state);
            const SystemMenuAction action =
                system_menu_key_down(state, sdl_keycode_to_menu_key(ev.key.key));
            if (state.screen == screen_before) {
                const int kb_index_after = system_menu_keyboard_focus_index(state);
                if (system_menu_hover_entered_new_item(kb_index_before, kb_index_after)) {
                    result.sfx = SystemMenuSfxKind::Hover;
                }
            }
            apply_system_menu_action_to_result(result, action);
        }
        return result;
    }

    if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN && ev.button.button == SDL_BUTTON_LEFT) {
        switch (state.screen) {
            case SystemMenuScreen::Pause: {
                if (state.pause_confirming_to_title) {
                    for (int item = 0; item < 2; ++item) {
                        if (!hit_test(boxes.pause_totitle_confirm[static_cast<std::size_t>(item)],
                                      ev.button.x, ev.button.y)) {
                            continue;
                        }
                        const SystemMenuState pre_action_state = state;
                        const SystemMenuAction action = system_menu_click_option(state, item);
                        if (is_confirming(action)) {
                            result.flash = SystemMenuFlashInfo{pre_action_state, item};
                        }
                        apply_system_menu_action_to_result(result, action);
                        break;
                    }
                } else {
                    for (int item = 0; item < kPauseItemCount; ++item) {
                        if (!hit_test(boxes.pause_items[static_cast<std::size_t>(item)],
                                      ev.button.x, ev.button.y)) {
                            continue;
                        }
                        const SystemMenuState pre_action_state = state;
                        const SystemMenuAction action = system_menu_click_option(state, item);
                        if (is_confirming(action)) {
                            result.flash = SystemMenuFlashInfo{pre_action_state, item};
                        }
                        apply_system_menu_action_to_result(result, action);
                        break;
                    }
                }
                break;
            }
            case SystemMenuScreen::ConfigCategories: {
                for (int item = 0; item < kConfigCategoriesItemCount; ++item) {
                    if (!hit_test(boxes.category_items[static_cast<std::size_t>(item)],
                                  ev.button.x, ev.button.y)) {
                        continue;
                    }
                    const SystemMenuState pre_action_state = state;
                    const SystemMenuAction action = system_menu_click_option(state, item);
                    if (is_confirming(action)) {
                        result.flash = SystemMenuFlashInfo{pre_action_state, item};
                    }
                    apply_system_menu_action_to_result(result, action);
                    break;
                }
                break;
            }
            case SystemMenuScreen::Audio: {
                // (1) Tracks dos sliders (drag-start) - checado PRIMEIRO porque a
                // caixa do track fica DENTRO da caixa do campo/rotulo (mais
                // especifico vence quando o clique cai nos dois, PONTO CRITICO #2).
                bool handled = false;
                for (int item = 0; item < 2 && !handled; ++item) {
                    const glintfx::ElementBox& box =
                        boxes.audio_tracks[static_cast<std::size_t>(item)];
                    if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                    handled = true;
                    result.start_drag_item = item;
                    state.audio_selected = item;
                    if (box.w > 0.0f) {
                        const float ratio = (ev.button.x - box.x) / box.w;
                        system_menu_set_slider_ratio(state, item, ratio);
                        result.volume_changed = true;
                    }
                    result.reload = true;
                }
                // (2) Botao Voltar - ACIONA na hora (equivalente a focar + ENTER).
                if (!handled) {
                    const int back_index = static_cast<int>(AudioItem::Back);
                    if (hit_test(boxes.audio_items[static_cast<std::size_t>(back_index)],
                                 ev.button.x, ev.button.y)) {
                        handled = true;
                        const SystemMenuState pre_action_state = state;
                        const SystemMenuAction action = system_menu_click_option(state, back_index);
                        if (is_confirming(action)) {
                            result.flash = SystemMenuFlashInfo{pre_action_state, back_index};
                        }
                        apply_system_menu_action_to_result(result, action);
                    }
                }
                // (3) Campo/rotulo do slider (fora do track) - SO FOCA.
                for (int item = 0; item < 2 && !handled; ++item) {
                    if (!hit_test(boxes.audio_items[static_cast<std::size_t>(item)], ev.button.x,
                                  ev.button.y)) {
                        continue;
                    }
                    handled = true;
                    const SystemMenuAction action = system_menu_click_option(state, item);
                    apply_system_menu_action_to_result(result, action);
                }
                break;
            }
            case SystemMenuScreen::Controls: {
                if (state.controls_capturing) {
                    // no-op: mouse nao participa neste modo (o jogador precisa
                    // apertar uma tecla FISICA - interceptado no ramo KEY_DOWN).
                } else if (state.controls_confirming_restore) {
                    for (int item = 0; item < 2; ++item) {
                        if (!hit_test(boxes.controls_confirm[static_cast<std::size_t>(item)],
                                      ev.button.x, ev.button.y)) {
                            continue;
                        }
                        const SystemMenuState pre_action_state = state;
                        const SystemMenuAction action = system_menu_click_option(state, item);
                        if (is_confirming(action)) {
                            result.flash = SystemMenuFlashInfo{pre_action_state, item};
                        }
                        apply_system_menu_action_to_result(result, action);
                        break;
                    }
                } else if (state.controls_confirming_discard) {
                    for (int item = 0; item < 2; ++item) {
                        if (!hit_test(boxes.controls_discard_confirm[static_cast<std::size_t>(item)],
                                      ev.button.x, ev.button.y)) {
                            continue;
                        }
                        const SystemMenuState pre_action_state = state;
                        const SystemMenuAction action = system_menu_click_option(state, item);
                        if (is_confirming(action)) {
                            result.flash = SystemMenuFlashInfo{pre_action_state, item};
                        }
                        apply_system_menu_action_to_result(result, action);
                        break;
                    }
                } else {
                    // Navegacao normal - `boxes.controls_items` ja chega FILTRADO
                    // (BUG-A) pelo CALLER (collect_click_boxes_).
                    for (int item = 0; item < kControlsItemCount; ++item) {
                        if (!hit_test(boxes.controls_items[static_cast<std::size_t>(item)],
                                      ev.button.x, ev.button.y)) {
                            continue;
                        }
                        const SystemMenuState pre_action_state = state;
                        const SystemMenuAction action = system_menu_click_option(state, item);
                        if (is_confirming(action)) {
                            result.flash = SystemMenuFlashInfo{pre_action_state, item};
                        }
                        apply_system_menu_action_to_result(result, action);
                        break;
                    }
                }
                break;
            }
            case SystemMenuScreen::Video:
            case SystemMenuScreen::Language: {
                if (hit_test(boxes.placeholder_back, ev.button.x, ev.button.y)) {
                    const SystemMenuState pre_action_state = state;
                    const SystemMenuAction action =
                        system_menu_click_option(state, kPlaceholderBackIndex);
                    if (is_confirming(action)) {
                        result.flash = SystemMenuFlashInfo{pre_action_state, kPlaceholderBackIndex};
                    }
                    apply_system_menu_action_to_result(result, action);
                }
                break;
            }
            case SystemMenuScreen::Hidden:
                break;
        }
        return result;
    }

    if (ev.type == SDL_EVENT_MOUSE_MOTION) {
        result.mouse_move = true;
        result.mouse_x = ev.motion.x;
        result.mouse_y = ev.motion.y;
        return result;
    }

    if (ev.type == SDL_EVENT_MOUSE_BUTTON_UP && ev.button.button == SDL_BUTTON_LEFT) {
        result.end_drag = true;
        return result;
    }

    // SDL_EVENT_MOUSE_WHEEL (forwarding pra `.ctrl-list`) e qualquer outro tipo
    // de evento NAO sao roteados aqui - MOUSE_WHEEL exige SDL_GetMouseState
    // (consulta IMPURA de cursor real), tratado direto em
    // SystemMenuScreen::handle_event (system_menu_loop.cpp) ANTES de chamar
    // esta funcao.
    return result;  // no-op TOTAL
}

// F4-1b.4: implementacao de pause_flow_next (declarada no .hpp) - a MATRIZ DE
// DECISAO do mini-driver, 100% testavel sem GL (system_screen_step_test.cpp
// exercita os casos relevantes). `saveload_exit` SO importa quando `exit` e
// OpenSaveLoadSave/OpenSaveLoadLoad (nos demais casos o driver nem chega a
// rodar a tela de save/load - ver run_system_menu_loop_gl_current abaixo).
SystemMenuFlowStep pause_flow_next(SystemMenuScreenExit exit,
                                     SaveLoadLoopExit saveload_exit) noexcept {
    switch (exit) {
        case SystemMenuScreenExit::Continue:
            return SystemMenuFlowStep::Continue;
        case SystemMenuScreenExit::RequestQuit:
            return SystemMenuFlowStep::RequestQuit;
        case SystemMenuScreenExit::RequestToTitle:
            return SystemMenuFlowStep::RequestToTitle;
        case SystemMenuScreenExit::OpenSaveLoadSave:
        case SystemMenuScreenExit::OpenSaveLoadLoad:
            switch (saveload_exit) {
                case SaveLoadLoopExit::BackToPause:
                    return SystemMenuFlowStep::RetryPause;
                case SaveLoadLoopExit::ClosedAfterLoad:
                    return SystemMenuFlowStep::Continue;  // MESMO efeito de Continuar
                case SaveLoadLoopExit::QuitApp:
                    return SystemMenuFlowStep::RequestQuit;  // propaga
            }
            break;
    }
    return SystemMenuFlowStep::RequestQuit;  // inalcancavel (defensivo - todo
                                              // SystemMenuScreenExit/
                                              // SaveLoadLoopExit relevante ja
                                              // foi coberto acima).
}

namespace {

// F4-1b.4: o ScreenState de PRODUCAO do menu de sistema (unico chamador: o
// MINI-DRIVER dentro de run_system_menu_loop_gl_current, abaixo) - MESMO padrao
// de TitleScreen (title_menu_loop.cpp, F4-1b.3). Todo o estado que antes vivia
// em variaveis locais fechadas por lambdas (ui/backdrop/drag_item/ids de SFX/
// rml_path/etc) agora e MEMBRO; enter() cria os recursos GL (glintfx::UiLayer +
// Render2dGl3 + ids de SFX), exit() libera (a EXCLUSIVIDADE do UiLayer descrita
// em gus/app/screen_state.hpp).
//
// DIFERENCA-CHAVE em relacao a recriar do zero: o ESTADO DE NEGOCIO (state_) NAO
// nasce em enter() - nasce no CONSTRUTOR, UMA UNICA VEZ. O MINI-DRIVER reusa o
// MESMO objeto SystemMenuScreen entre o Pause e a volta da tela de save/load
// (BackToPause) - se state_ fosse recriado em enter(), o jogador perderia a
// tela/selecao atual (ex.: uma copia de trabalho de Controles ainda nao
// aplicada) toda vez que voltasse de Salvar/Carregar (regressao do
// comportamento antigo, ver o comentario grande no topo deste arquivo).
class SystemMenuLoopScreen final : public gus::app::ScreenState {
   public:
    SystemMenuLoopScreen(SDL_Window* window, gus::platform::audio::AudioEngine& audio,
                      const gus::app::i18n::Translator& translator, std::string settings_dir,
                      std::string saves_dir,
                      std::function<gus::domain::save::SaveData()> build_current_save_data,
                      std::function<void(const gus::domain::save::SaveData&)>
                          apply_loaded_save_data,
                      std::string frozen_background_png)
        : window_(window),
          audio_(audio),
          translator_(translator),
          settings_dir_(std::move(settings_dir)),
          saves_dir_(std::move(saves_dir)),
          build_current_save_data_(std::move(build_current_save_data)),
          apply_loaded_save_data_(std::move(apply_loaded_save_data)),
          frozen_background_png_(std::move(frozen_background_png)) {
        state_.music_volume = audio_.music_volume();
        state_.sfx_volume = audio_.sfx_volume();
        // Tela Controles (M2 -> M2 STAGED CHANGES): carrega o remap persistido
        // (ou default_controls() se ausente/corrompido) - controls_applied_
        // config = a MESMA leitura (BASELINE, alvo do revert ao descartar); as
        // duas copias comecam IGUAIS.
        state_.controls_config = gus::platform::fs::load_controls(
            settings_dir_, std::string(gus::domain::input::kDefaultProfile));
        state_.controls_applied_config = state_.controls_config;
        system_menu_open(state_);
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
    ~SystemMenuLoopScreen() override {
        if (ui_) ui_->set_hover_callback(nullptr);
    }

    void enter() override {
        // Flags TRANSIENTES de UMA rodada do driver - resetadas a CADA enter(),
        // diferente de state_ (persiste entre rodadas, ver o comentario da
        // classe/construtor acima).
        bailed_ = false;
        done_ = false;
        window_closed_ = false;
        drag_item_ = -1;
        last_hover_sfx_id_.clear();

        SDL_GetWindowSizeInPixels(window_, &pw_, &ph_);
        if (pw_ < 1) pw_ = 1;
        if (ph_ < 1) ph_ = 1;
        dp_ratio_ = static_cast<float>(pw_) / 960.0f;

        ui_.emplace(glintfx::UiLayer::Config{/*logical_width=*/960,
                                              /*logical_height=*/540,
                                              /*load_gl=*/true,
                                              /*dp_ratio=*/dp_ratio_});
        if (!ui_->ok()) {
            std::cerr << "SystemMenuLoop: glintfx::UiLayer::ok()=false (attach "
                         "falhou) - fechando o menu sem desenhar nada (degradacao "
                         "segura).\n";
            result_ = SystemMenuScreenExit::Continue;  // quit_app=false: o
                                                        // driver so retoma a cena
            bailed_ = true;
            return;
        }

        stage_ = menu_stage_dir();
        ui_->set_asset_base_url(stage_.c_str());
        rml_path_ = write_system_menu_rml_file(state_, translator_);
        register_pixel_operator_mono_fonts(*ui_);
        ui_->load(rml_path_.c_str());
        ui_->set_viewport(pw_, ph_);
        ui_->set_dp_ratio(dp_ratio_);
        // SFX-MIGRATE-V0.9: 1 update() de "assentamento" (achado empirico: o
        // hover NATIVO so resolve elemento sob o cursor apos pelo menos 1
        // Context::Update() do documento recem-carregado).
        ui_->update();

        backdrop_.emplace(/*gl_active=*/true);

        // FUNDO REAL CONGELADO (retoque do lider via AskUserQuestion, MENU-PAUSA-
        // CONFIG-SOM): carrega a textura A CADA enter() (os TextureId antigos nao
        // sobrevivem a destruicao do Render2dGl3 anterior) - kInvalidTexture
        // (path vazio/asset ausente/backend headless) degrada com seguranca pra
        // vinheta de sempre (ver present_frame_).
        frozen_bg_tex_ = frozen_background_png_.empty()
                             ? gus::platform::render2d::kInvalidTexture
                             : backdrop_->load_texture(frozen_background_png_.c_str());

        // SFX de hover/clique: load_sfx a CADA enter() (os SoundId antigos nao
        // sobrevivem entre rodadas - recarregar o MESMO arquivo e barato e
        // idempotente).
        const std::string hover_sfx_path =
            resolve_menu_sfx_path(gus::core::assets::kMenuHoverSfxFile);
        const std::string click_sfx_path =
            resolve_menu_sfx_path(gus::core::assets::kMenuClickSfxFile);
        hover_sfx_id_ = audio_.load_sfx(hover_sfx_path.c_str());
        click_sfx_id_ = audio_.load_sfx(click_sfx_path.c_str());

        ui_->set_hover_callback([this](const char* raw_id, bool entered) {
            native_hover_callback_(raw_id, entered);
        });

        // DIAGNOSTICO/PROVA (SOM DE HOVER/CLIQUE): GUSWORLD_SYSMENU_HOVER_
        // SELFTEST=1 entra na tela Pause (ja aberta acima), MOVE o mouse
        // SINTETICO sequencialmente pelos 4 pills e SIMULA 1 clique confirmando
        // "Continuar" - tudo SEM SDL_PushEvent, SEM input real, SEM tocar
        // hardware de audio. Bypassa por completo o loop interativo (MESMO
        // espirito de GUSWORLD_TITLE_SCREENSHOT_DIR em title_menu_loop.cpp -
        // bailed_=true, nunca entra no loop de tick()).
        if (const char* sysmenu_selftest = std::getenv("GUSWORLD_SYSMENU_HOVER_SELFTEST");
            sysmenu_selftest != nullptr && sysmenu_selftest[0] != '\0') {
            run_hover_selftest_();
            bailed_ = true;
            return;
        }

        // DIAGNOSTICO/PROVA (PARIDADE SOM DE HOVER TECLADO x MOUSE):
        // GUSWORLD_SYSMENU_KEYBOARD_HOVER_SELFTEST=1 prova que a navegacao por
        // TECLADO (via handle_event(), o MESMO caminho de codigo do SDL_EVENT_
        // KEY_DOWN real) toca hover_sfx exatamente quando move pra um item NOVO
        // na MESMA tela.
        if (const char* keyboard_hover_selftest =
                std::getenv("GUSWORLD_SYSMENU_KEYBOARD_HOVER_SELFTEST");
            keyboard_hover_selftest != nullptr && keyboard_hover_selftest[0] != '\0') {
            run_keyboard_hover_selftest_();
            bailed_ = true;
            return;
        }

        // DIAGNOSTICO/PROVA (TELA CONTROLES, M2, 3 bugs ao vivo reportados pelo
        // lider): GUSWORLD_SYSMENU_CONTROLS_SELFTEST=1 entra em Controles
        // (navegacao REAL via system_menu_key_down) e MEDE/EXERCITA os pontos
        // que quebraram (BUG-1/BUG-A/BUG-2/BUG-3/GLINTFX-SCROLL).
        if (const char* controls_selftest = std::getenv("GUSWORLD_SYSMENU_CONTROLS_SELFTEST");
            controls_selftest != nullptr && controls_selftest[0] != '\0') {
            run_controls_selftest_();
            bailed_ = true;
            return;
        }

        // NAO ha present_frame_() explicito aqui (ao contrario de
        // NpcDialogueScreen): o while(true) ORIGINAL desta tela so desenhava no
        // FIM de cada iteracao, DEPOIS de drenar a rajada de eventos daquele
        // frame - o 1o tick() do runner reproduz exatamente essa 1a iteracao.
    }

    void handle_event(const SDL_Event& ev) override {
        if (bailed_) {
            return;  // defensivo: nao deveria ser chamado (finished()==true).
        }

        // WHEEL FORWARDING (M2/GLINTFX-SCROLL): exige SDL_GetMouseState
        // (consulta IMPURA de cursor real) - FORA do escopo de
        // system_screen_step, tratado direto aqui (MESMO racional de
        // SaveLoadScreen::handle_event).
        if (ev.type == SDL_EVENT_MOUSE_WHEEL) {
            float mouse_x = 0.0f, mouse_y = 0.0f;
            SDL_GetMouseState(&mouse_x, &mouse_y);
            handle_mouse_motion_(mouse_x, mouse_y);

            const float wheel_dy = system_menu_wheel_delta_to_rmlui(
                ev.wheel.y, ev.wheel.direction == SDL_MOUSEWHEEL_FLIPPED);
            glintfx::UiEvent wheel_ev{};
            wheel_ev.type = glintfx::UiEvent::Type::MouseWheel;
            wheel_ev.x = 0.0f;
            wheel_ev.y = wheel_dy;
            ui_->process_event(wheel_ev);
            return;
        }

        SystemMenuStepBoxes boxes;
        if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN && ev.button.button == SDL_BUTTON_LEFT) {
            boxes = collect_click_boxes_();
        }

        const SystemMenuStepResult step = system_screen_step(state_, ev, boxes);

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
        if (step.end_drag) {
            drag_item_ = -1;
            return;
        }
        if (step.start_drag_item.has_value()) {
            drag_item_ = *step.start_drag_item;
        }
        if (step.sfx == SystemMenuSfxKind::Hover) {
            audio_.play_sfx(hover_sfx_id_);
        }
        if (step.flash.has_value()) {
            flash_pressed_(step.flash->pre_action_state, step.flash->item_index);
        }
        if (step.volume_changed) {
            apply_and_persist_();
        }
        if (step.controls_applied) {
            persist_controls_();
        }
        if (step.exit.has_value()) {
            result_ = *step.exit;
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
        // EXCLUSIVIDADE DO UILAYER (ver gus/app/screen_state.hpp): destroi ui_
        // ANTES de backdrop_ (MESMA ordem de sempre) - depois de exit(), o
        // MINI-DRIVER pode abrir a tela de save/load (OU re-entrar este MESMO
        // SystemMenuScreen) com seguranca.
        ui_.reset();
        backdrop_.reset();
    }

    [[nodiscard]] bool window_closed() const override { return window_closed_; }

    // Exposto SO pro DRIVER (run_system_menu_loop_gl_current, abaixo) - nao faz
    // parte do contrato ScreenState. O desfecho decidido pelo ultimo
    // system_screen_step com exit preenchido, OU o default Continue setado em
    // enter() (ui_->ok()==false).
    [[nodiscard]] SystemMenuScreenExit result() const { return result_; }

   private:
    void reload_() {
        rml_path_ = write_system_menu_rml_file(state_, translator_);
        ui_->load(rml_path_.c_str());
        ui_->set_viewport(pw_, ph_);
        ui_->set_dp_ratio(dp_ratio_);
        ui_->update();  // MESMO assentamento a cada troca de documento

        // SCROLL SEGUE A SELECAO (M2/GLINTFX-SCROLL): garante que a linha
        // state.controls_selected fique DENTRO do recorte visivel de
        // `.ctrl-list`.
        //
        // ATENCAO (achado 2026-08-01, item SCROLL-REANCORA-AO-TOPO do
        // TODO.md): NAO e no-op quando a linha ja esta visivel - este
        // comentario mentia isso, e a receita foi copiada pra save/load
        // (save_load_menu_loop.cpp) antes de o erro ser achado. A API do
        // glintfx REANCORA a linha ao TOPO da area visivel toda vez que roda
        // (align_with_top=true e o unico modo hoje), mesmo se ela ja estivesse
        // visivel em outra posicao (medido: -68px de deslocamento so por
        // chamar a funcao com o item ja selecionado/visivel). Efeito: a lista
        // treme a cada mudanca de selecao, mesmo sem precisar rolar.
        // BLOQUEADO PELO GLINTFX: aguardando ScrollAlignment::Nearest (ja
        // existe no RmlUi 6.x que eles embrulham, uma camada abaixo do
        // wrapper) - pedido no bus (thread api-glintfx). Quando sair, a troca
        // e uma linha aqui E em save_load_menu_loop.cpp, no mesmo toque. Guard
        // [!shouldfail] em save_load_menu_interaction_test.cpp avisa sozinho.
        const int scroll_target = controls_scroll_target_index(state_);
        if (scroll_target >= 0) {
            ui_->scroll_element_into_view(controls_item_id(scroll_target).c_str());
        }
    }

    void present_frame_() {
        const gus::core::spatial::Rect cam{0.0f, 0.0f, static_cast<float>(pw_),
                                            static_cast<float>(ph_)};
        backdrop_->begin_frame(cam, pw_, ph_);  // clear + vinheta radial (fallback abstrato)
        if (frozen_bg_tex_ != gus::platform::render2d::kInvalidTexture) {
            // FUNDO REAL CONGELADO: cobre a vinheta com a CENA REAL da cidade (1
            // frame estatico), full-screen e opaca. O #sysmenu-scrim do RML
            // segue desenhado por CIMA disto pelo glintfx.
            backdrop_->draw_textured_rect(
                cam, frozen_bg_tex_, gus::platform::render2d::UvRect{0.0f, 0.0f, 1.0f, 1.0f},
                gus::platform::render2d::DrawColor{1.0f, 1.0f, 1.0f, 1.0f});
        }
        backdrop_->end_frame();
        ui_->update();
        ui_->render();
        SDL_GL_SwapWindow(window_);
    }

    // EFEITO DE PRESS (ver comentario do topo do arquivo): renderiza a tela
    // `pre_action_state` (snapshot tirado ANTES da mutacao que ja aconteceu em
    // state_) com o item `item_index` marcado ".pressed", por ~100ms (4 frames
    // de ~25ms) - SO DEPOIS o CHAMADOR (handle_event) segue com o efeito de
    // mundo (volume/controls_applied/exit ja decididos por system_screen_step,
    // usando state_ JA MUTADO). SOM DE CLIQUE: dispara AQUI, no MESMO
    // choke-point do flash visual - o UNICO lugar chamado tanto por
    // confirmacao de TECLADO quanto por CLIQUE de mouse (ver
    // system_screen_step), entao 1 play_sfx cobre os dois canais sem duplicar
    // logica.
    void flash_pressed_(const SystemMenuState& pre_action_state, int item_index) {
        audio_.play_sfx(click_sfx_id_);
        rml_path_ = write_system_menu_rml_file(pre_action_state, translator_, item_index);
        ui_->load(rml_path_.c_str());
        ui_->set_viewport(pw_, ph_);
        ui_->set_dp_ratio(dp_ratio_);
        for (int frame = 0; frame < 4; ++frame) {
            present_frame_();
            SDL_Delay(25);
        }
    }

    // HOVER (mouse): injeta o MouseMove no glintfx (visual :hover NATIVO, o QUE
    // dispara native_hover_callback_ por baixo dos panos) + trata o arrasto de
    // slider (drag_item_ - PONTO CRITICO #2, o estado de arrasto vive ENTRE
    // MOUSE_MOTIONs, por isso e MEMBRO, nao variavel local). O SOM de hover e
    // 100% responsabilidade do callback nativo.
    void handle_mouse_motion_(float mx, float my) {
        glintfx::UiEvent hover_ev{};
        hover_ev.type = glintfx::UiEvent::Type::MouseMove;
        hover_ev.x = mx;
        hover_ev.y = my;
        ui_->process_event(hover_ev);

        if (drag_item_ >= 0) {
            const std::string id = track_id_for_item(drag_item_);
            const glintfx::ElementBox box = ui_->get_element_box(id.c_str());
            if (box.found && box.w > 0.0f) {
                const float ratio = (mx - box.x) / box.w;
                system_menu_set_slider_ratio(state_, drag_item_, ratio);
                apply_and_persist_();
                reload_();
            }
        }
    }

    void apply_and_persist_() { apply_and_persist(state_, audio_, settings_dir_); }
    void persist_controls_() { persist_controls(state_, settings_dir_); }

    // Callback NATIVO de hover do glintfx (id-based, RCSS :hover) - precisa ser
    // re-registrado em CADA glintfx::UiLayer NOVA (o callback vive no
    // Bootstrap::Impl da instancia, nao sobrevive a ui_.reset()+emplace() - ver
    // enter() acima, que registra de novo a CADA rodada).
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

    // Resolve as boxes de CLIQUE do frame ATUAL - SO os sub-arrays do
    // ramo/estado ATUAL de state_.screen sao preenchidos (found=true), MESMA
    // convencao de DifficultyScreen::collect_click_boxes_. Controls: consulta
    // `ctrl-list` UMA VEZ e filtra as 33 caixas (BUG-A) - as demais telas nao
    // precisam de filtro.
    [[nodiscard]] SystemMenuStepBoxes collect_click_boxes_() const {
        SystemMenuStepBoxes boxes;
        switch (state_.screen) {
            case SystemMenuScreen::Pause:
                if (state_.pause_confirming_to_title) {
                    for (int i = 0; i < 2; ++i) {
                        boxes.pause_totitle_confirm[static_cast<std::size_t>(i)] =
                            ui_->get_element_box(pause_totitle_confirm_id(i).c_str());
                    }
                } else {
                    for (int i = 0; i < kPauseItemCount; ++i) {
                        boxes.pause_items[static_cast<std::size_t>(i)] =
                            ui_->get_element_box(pause_item_id(i).c_str());
                    }
                }
                break;
            case SystemMenuScreen::ConfigCategories:
                for (int i = 0; i < kConfigCategoriesItemCount; ++i) {
                    boxes.category_items[static_cast<std::size_t>(i)] =
                        ui_->get_element_box(category_item_id(i).c_str());
                }
                break;
            case SystemMenuScreen::Audio:
                for (int i = 0; i < 2; ++i) {
                    boxes.audio_tracks[static_cast<std::size_t>(i)] =
                        ui_->get_element_box(track_id_for_item(i).c_str());
                }
                for (int i = 0; i < kAudioItemCount; ++i) {
                    boxes.audio_items[static_cast<std::size_t>(i)] =
                        ui_->get_element_box(audio_item_id(i).c_str());
                }
                break;
            case SystemMenuScreen::Controls:
                if (state_.controls_capturing) {
                    // no-op: boxes ficam found=false (mouse nao participa).
                } else if (state_.controls_confirming_restore) {
                    for (int i = 0; i < 2; ++i) {
                        boxes.controls_confirm[static_cast<std::size_t>(i)] =
                            ui_->get_element_box(controls_confirm_id(i).c_str());
                    }
                } else if (state_.controls_confirming_discard) {
                    for (int i = 0; i < 2; ++i) {
                        boxes.controls_discard_confirm[static_cast<std::size_t>(i)] =
                            ui_->get_element_box(controls_discard_confirm_id(i).c_str());
                    }
                } else {
                    const glintfx::ElementBox list_box = ui_->get_element_box(kControlsListId);
                    for (int i = 0; i < kControlsItemCount; ++i) {
                        const glintfx::ElementBox raw =
                            ui_->get_element_box(controls_item_id(i).c_str());
                        boxes.controls_items[static_cast<std::size_t>(i)] =
                            filter_offscreen_controls_row(i, raw, list_box);
                    }
                }
                break;
            case SystemMenuScreen::Video:
            case SystemMenuScreen::Language:
                boxes.placeholder_back = ui_->get_element_box(kPlaceholderBackId);
                break;
            case SystemMenuScreen::Hidden:
                break;
        }
        return boxes;
    }

    // ---------------------------------------------------------------- selftests
    //
    // Os 3 diagnosticos abaixo (SOM DE HOVER/CLIQUE do mouse; PARIDADE SOM DE
    // HOVER TECLADO x MOUSE; TELA CONTROLES) migraram do antigo while(true)
    // (que rodava ANTES do loop, usando as variaveis locais desta funcao) pra
    // metodos desta classe (MESMO padrao de bailed_=true de TitleScreen/
    // DifficultyScreen pro screenshot self-test) - chamados por enter() acima,
    // rodam UMA VEZ e terminam a rodada (bailed_=true, nunca entram no loop
    // interativo de tick()).

    void run_hover_selftest_() {
        present_frame_();  // assenta o layout (get_element_box precisa de 1 update())

        // Centro de cada pill (item0..3), na ORDEM 0,1,2,3,0 (o ultimo "0" de
        // novo prova o re-trigger apos sair pro item 3).
        const int hover_sequence[] = {0, 1, 2, 3, 0};
        for (const int item : hover_sequence) {
            const glintfx::ElementBox box = ui_->get_element_box(pause_item_id(item).c_str());
            const float cx = box.found ? box.x + box.w * 0.5f : -1.0f;
            const float cy = box.found ? box.y + box.h * 0.5f : -1.0f;
            handle_mouse_motion_(cx, cy);
            present_frame_();
        }
        std::cout << "SystemMenuLoop: [selftest] hover_sfx_play_count apos 5 "
                     "moves (0,1,2,3,0 - 5 entradas NOVAS esperadas) = "
                  << audio_.sfx_play_count() << "\n";

        const int click_baseline = static_cast<int>(audio_.sfx_play_count());
        const SystemMenuState pre_click_state = state_;
        const SystemMenuAction click_action =
            system_menu_click_option(state_, static_cast<int>(PauseItem::Continue));
        if (is_confirming(click_action)) {
            flash_pressed_(pre_click_state, static_cast<int>(PauseItem::Continue));
        }
        std::cout << "SystemMenuLoop: [selftest] click_sfx disparou "
                  << (static_cast<int>(audio_.sfx_play_count()) - click_baseline)
                  << "x (esperado 1) - total sfx_play_count()=" << audio_.sfx_play_count()
                  << "\n";

        SystemMenuStepResult tmp;
        apply_system_menu_action_to_result(tmp, click_action);  // fecha o menu (Continuar)
        if (tmp.exit.has_value()) result_ = *tmp.exit;
    }

    void run_keyboard_hover_selftest_() {
        present_frame_();  // assenta o layout (mesma cautela dos demais self-tests)

        // Pause (4 itens, item 0 "Continuar" selecionado por system_menu_open ja
        // chamado no construtor): 3x DOWN move 0->1->2->3 - 3 itens NOVOS, 3
        // sons. handle_event(down_ev) e o MESMO caminho de codigo do SDL_EVENT_
        // KEY_DOWN real (nao-confirm-key -> ramo de navegacao de
        // system_screen_step).
        const unsigned int baseline_down3 = audio_.sfx_play_count();
        for (int i = 0; i < 3; ++i) {
            handle_event(key_down_event_(SDLK_DOWN));
            if (done_ || window_closed_) return;
        }
        const unsigned int after_down3 = audio_.sfx_play_count() - baseline_down3;
        std::cout << "SystemMenuLoop: [selftest][KEYBOARD-HOVER] Pause 3x DOWN: "
                     "pause_selected="
                  << state_.pause_selected << " (esperado 3); hover_sfx tocou "
                  << after_down3 << "x (esperado 3) - "
                  << (state_.pause_selected == 3 && after_down3 == 3 ? "OK" : "FALHOU")
                  << "\n";

        // NO-OP: LEFT nao tem efeito de navegacao em Pause - selecao intocada,
        // hover_sfx NAO deve tocar.
        const unsigned int baseline_left = audio_.sfx_play_count();
        handle_event(key_down_event_(SDLK_LEFT));
        if (done_ || window_closed_) return;
        const unsigned int after_left = audio_.sfx_play_count() - baseline_left;
        std::cout << "SystemMenuLoop: [selftest][KEYBOARD-HOVER] Pause LEFT "
                     "(no-op, sem efeito de navegacao): pause_selected="
                  << state_.pause_selected << " (esperado 3, intocado); hover_sfx "
                     "tocou "
                  << after_left << "x (esperado 0) - "
                  << (state_.pause_selected == 3 && after_left == 0 ? "OK" : "FALHOU")
                  << "\n";

        // WRAP: mais 1 DOWN (3->0) - AINDA um item NOVO - edge-detect continua
        // disparando atraves do wrap-around.
        const unsigned int baseline_wrap = audio_.sfx_play_count();
        handle_event(key_down_event_(SDLK_DOWN));
        if (done_ || window_closed_) return;
        const unsigned int after_wrap = audio_.sfx_play_count() - baseline_wrap;
        std::cout << "SystemMenuLoop: [selftest][KEYBOARD-HOVER] Pause DOWN (wrap "
                     "3->0): pause_selected="
                  << state_.pause_selected << " (esperado 0); hover_sfx tocou "
                  << after_wrap << "x (esperado 1) - "
                  << (state_.pause_selected == 0 && after_wrap == 1 ? "OK" : "FALHOU")
                  << "\n";

        // TROCA DE TELA nao conta como navegacao: forca "Configuracoes"
        // selecionado e ENTER (confirm-key) pra entrar em ConfigCategories -
        // via mutacao direta + selftest_route_only_ (MESMA receita do antigo
        // `(void)handle_action(system_menu_key_down(...))`: so aplica volume/
        // controls_applied/reload, sem flash - o selftest historico nunca
        // exercitou o flash nestas transicoes de tela).
        state_.pause_selected = static_cast<int>(PauseItem::Settings);
        selftest_route_only_(system_menu_key_down(state_, glintfx::Key::Enter));
        const bool entered_config = state_.screen == SystemMenuScreen::ConfigCategories;

        // ESC (NAO e confirm-key - passa pelo ramo de navegacao igual a
        // qualquer seta) sobe de volta pra Pause: TROCA DE TELA - o guard
        // `state.screen == screen_before` (dentro de system_screen_step) barra
        // o hover_sfx mesmo que os indices numericos difiram entre as 2 telas.
        const unsigned int baseline_esc = audio_.sfx_play_count();
        handle_event(key_down_event_(SDLK_ESCAPE));
        if (done_ || window_closed_) return;
        const unsigned int after_esc = audio_.sfx_play_count() - baseline_esc;
        std::cout << "SystemMenuLoop: [selftest][KEYBOARD-HOVER] ESC troca de "
                     "tela (ConfigCategories->Pause, entrou_em_config="
                  << entered_config << "): screen_apos="
                  << (state_.screen == SystemMenuScreen::Pause ? "Pause" : "outro")
                  << " hover_sfx tocou " << after_esc
                  << "x (esperado 0, guard de tela barra) - "
                  << (entered_config && state_.screen == SystemMenuScreen::Pause &&
                              after_esc == 0
                          ? "OK"
                          : "FALHOU")
                  << "\n";

        // SLIDER (Audio): LEFT/RIGHT ajustam volume (VolumeChanged), NAO navegam
        // - hover_sfx NAO deve tocar. Entra em Configuracoes (Audio ja
        // selecionado, indice 0) -> Audio (Musica, indice 0), ambos ENTER
        // (confirm-key).
        selftest_route_only_(system_menu_key_down(state_, glintfx::Key::Enter));  // -> ConfigCategories
        selftest_route_only_(system_menu_key_down(state_, glintfx::Key::Enter));  // -> Audio
        const bool entered_audio = state_.screen == SystemMenuScreen::Audio;
        const unsigned int baseline_slider = audio_.sfx_play_count();
        handle_event(key_down_event_(SDLK_LEFT));
        if (done_ || window_closed_) return;
        const unsigned int after_slider = audio_.sfx_play_count() - baseline_slider;
        std::cout << "SystemMenuLoop: [selftest][KEYBOARD-HOVER] Audio LEFT "
                     "(ajusta volume, nao navega, entrou_em_audio="
                  << entered_audio << "): audio_selected=" << state_.audio_selected
                  << " (esperado 0, Musica); hover_sfx tocou " << after_slider
                  << "x (esperado 0, e slider, nao navegacao) - "
                  << (entered_audio && state_.audio_selected == 0 && after_slider == 0
                          ? "OK"
                          : "FALHOU")
                  << "\n";
    }

    // Constroi um SDL_Event KEY_DOWN sintetico (repeat=0) - usado SO pelos
    // selftests acima, pra rotear pelo MESMO caminho de codigo de producao
    // (handle_event) sem precisar de SDL_PushEvent/fila de eventos real.
    static SDL_Event key_down_event_(SDL_Keycode key) {
        SDL_Event ev{};
        ev.type = SDL_EVENT_KEY_DOWN;
        ev.key.key = key;
        ev.key.repeat = 0;
        return ev;
    }

    // MESMO racional do antigo `(void)handle_action(system_menu_key_down(...))`
    // dos selftests: aplica volume_changed/controls_applied/reload, SEM flash -
    // usado so pelas transicoes de tela que os selftests provocam de proposito
    // (nunca pelo roteamento de producao, que sempre passa por
    // system_screen_step via handle_event).
    void selftest_route_only_(SystemMenuAction action) {
        if (action == SystemMenuAction::VolumeChanged) apply_and_persist_();
        if (action == SystemMenuAction::ControlsApplied) persist_controls_();
        reload_();
    }

    void run_controls_selftest_() {
        // Pause -> ConfigCategories -> Controls (navegacao REAL via
        // system_menu_key_down). (void): so a mutacao de state_ importa aqui.
        (void)system_menu_key_down(state_, glintfx::Key::Down);    // Continue->Save
        (void)system_menu_key_down(state_, glintfx::Key::Down);    // Save->Settings
        (void)system_menu_key_down(state_, glintfx::Key::Enter);  // entra ConfigCategories
        (void)system_menu_key_down(state_, glintfx::Key::Down);    // Audio->Video
        (void)system_menu_key_down(state_, glintfx::Key::Down);    // Video->Controls
        (void)system_menu_key_down(state_, glintfx::Key::Enter);  // entra Controls
        reload_();
        present_frame_();
        present_frame_();  // 2o update() por seguranca (layout assentado)

        // BUG-1: o painel (e o rodape Restaurar/Voltar) tem que caber na viewport.
        const glintfx::ElementBox panel = ui_->get_element_box("sysmenu-panel");
        const glintfx::ElementBox back_btn =
            ui_->get_element_box(controls_item_id(kControlsBackIndex).c_str());
        std::cout << "SystemMenuLoop: [selftest][BUG-1] viewport ph=" << ph_
                  << " panel_bottom=" << (panel.y + panel.h)
                  << " (esperado <= " << ph_ << ") - "
                  << ((panel.y + panel.h) <= static_cast<float>(ph_) ? "OK" : "FALHOU")
                  << "\n";
        std::cout << "SystemMenuLoop: [selftest][BUG-1] botao Voltar do rodape: found="
                  << back_btn.found << " bottom=" << (back_btn.y + back_btn.h)
                  << " (esperado found=1 e <= " << ph_ << ") - "
                  << (back_btn.found && (back_btn.y + back_btn.h) <= static_cast<float>(ph_)
                          ? "OK"
                          : "FALHOU")
                  << "\n";

        // BUG-A: a caixa de hit-test da linha 0 precisa cobrir a linha INTEIRA.
        const glintfx::ElementBox row0 = ui_->get_element_box(controls_item_id(0).c_str());
        std::cout << "SystemMenuLoop: [selftest][BUG-A] largura hit-test da linha 0: w="
                  << row0.w << " (esperado > 400px, era ~16px no bug) - "
                  << (row0.w > 400.0f ? "OK" : "FALHOU") << "\n";

        // BUG-2: o keycap de move_forward (1a linha) tem que mostrar "W".
        {
            std::ifstream rml_in(rml_path_);
            std::ostringstream ss;
            ss << rml_in.rdbuf();
            const std::string txt = ss.str();
            const bool has_w_keycap = txt.find(">W<") != std::string::npos;
            std::cout << "SystemMenuLoop: [selftest][BUG-2] keycap 'W' de move_forward "
                         "presente no RML: "
                      << (has_w_keycap ? "OK" : "FALHOU") << "\n";
        }

        // BUG-3 (teclado): DOWN 3x tem que avancar controls_selected 0->3.
        for (int i = 0; i < 3; ++i) {
            (void)system_menu_key_down(state_, glintfx::Key::Down);
        }
        reload_();
        present_frame_();
        std::cout << "SystemMenuLoop: [selftest][BUG-3 teclado] apos 3x DOWN: "
                     "controls_selected="
                  << state_.controls_selected << " (esperado 3) - "
                  << (state_.controls_selected == 3 ? "OK" : "FALHOU") << "\n";

        // BUG-3 (mouse): hit-test + clique na linha 5.
        const glintfx::ElementBox row5 = ui_->get_element_box(controls_item_id(5).c_str());
        const SystemMenuAction click_action = system_menu_click_option(state_, 5);
        std::cout << "SystemMenuLoop: [selftest][BUG-3 mouse] hit-test linha 5: found="
                  << row5.found << " w=" << row5.w << "; apos clique: controls_selected="
                  << state_.controls_selected << " controls_capturing="
                  << state_.controls_capturing << " (esperado selected=5, capturing=1) - "
                  << (state_.controls_selected == 5 && state_.controls_capturing ? "OK"
                                                                                  : "FALHOU")
                  << "\n";
        (void)click_action;

        // BUG-A (Voltar morto pro mouse): cancela a captura aberta pelo teste
        // BUG-3 acima (Esc, nao muda config) antes de prosseguir.
        (void)system_menu_controls_capture_key(state_, /*is_escape=*/true, 0);
        {
            const glintfx::ElementBox list_box = ui_->get_element_box(kControlsListId);
            const glintfx::ElementBox raw_row6 =
                ui_->get_element_box(controls_item_id(6).c_str());
            std::cout << "SystemMenuLoop: [selftest][BUG-A] linha 6 (rolada pra fora "
                         "da vista) - caixa REAL: y="
                      << raw_row6.y << " h=" << raw_row6.h << "; recorte visivel de "
                         "ctrl-list: y="
                      << list_box.y << " h=" << list_box.h << " (linha 6 fora do "
                         "recorte, MESMA geometria que roubava o clique do rodape "
                         "antes do fix)\n";

            const glintfx::ElementBox raw_back =
                ui_->get_element_box(controls_item_id(kControlsBackIndex).c_str());
            const float cx = raw_back.found ? raw_back.x + raw_back.w * 0.5f : -1.0f;
            const float cy = raw_back.found ? raw_back.y + raw_back.h * 0.5f : -1.0f;

            int winner = -1;
            for (int item = 0; item < kControlsItemCount; ++item) {
                const glintfx::ElementBox raw =
                    ui_->get_element_box(controls_item_id(item).c_str());
                const glintfx::ElementBox box = filter_offscreen_controls_row(item, raw, list_box);
                if (hit_test(box, cx, cy)) {
                    winner = item;
                    break;
                }
            }
            const SystemMenuAction back_action =
                (winner == kControlsBackIndex) ? system_menu_click_option(state_, winner)
                                                : SystemMenuAction::None;
            const bool ok = winner == kControlsBackIndex &&
                             state_.screen == SystemMenuScreen::ConfigCategories &&
                             back_action == SystemMenuAction::Navigated;
            std::cout << "SystemMenuLoop: [selftest][BUG-A] clique na posicao REAL de "
                         "Voltar (loop de producao replicado 0.."
                      << (kControlsItemCount - 1) << "): indice vencedor=" << winner
                      << " (esperado " << kControlsBackIndex << ") screen_apos="
                      << (state_.screen == SystemMenuScreen::ConfigCategories
                              ? "ConfigCategories"
                              : "outro")
                      << " action=" << static_cast<int>(back_action)
                      << " (esperado screen=ConfigCategories action=Navigated) - "
                      << (ok ? "OK" : "FALHOU") << "\n";
        }

        // GLINTFX-SCROLL (M2): re-entra em Controles (config_categories_selected
        // ainda aponta pra ela). (void): so a mutacao importa.
        (void)system_menu_key_down(state_, glintfx::Key::Enter);
        reload_();
        present_frame_();
        present_frame_();

        // (b) WHEEL: a roda rola o elemento em HOVER.
        {
            const glintfx::ElementBox list_box = ui_->get_element_box(kControlsListId);
            const float hover_x = list_box.found ? list_box.x + list_box.w * 0.5f : -1.0f;
            const float hover_y = list_box.found ? list_box.y + list_box.h * 0.5f : -1.0f;
            handle_mouse_motion_(hover_x, hover_y);

            float scroll_before = -1.0f;
            ui_->get_element_scroll_top(kControlsListId, scroll_before);

            const float wheel_dy =
                system_menu_wheel_delta_to_rmlui(/*sdl_wheel_y=*/-3.0f, /*flipped=*/false);
            glintfx::UiEvent wheel_ev{};
            wheel_ev.type = glintfx::UiEvent::Type::MouseWheel;
            wheel_ev.x = 0.0f;
            wheel_ev.y = wheel_dy;
            ui_->process_event(wheel_ev);
            present_frame_();

            float scroll_after = -1.0f;
            ui_->get_element_scroll_top(kControlsListId, scroll_after);
            std::cout << "SystemMenuLoop: [selftest][GLINTFX-SCROLL][wheel] "
                         "scroll_top antes="
                      << scroll_before << " depois=" << scroll_after
                      << " (esperado depois > antes - wheel_dy=" << wheel_dy << ") - "
                      << (scroll_after > scroll_before ? "OK" : "FALHOU") << "\n";
        }

        // (a) TECLADO: navega ate uma action perto do FIM da lista (25a).
        for (int i = 0; i < 25; ++i) {
            (void)system_menu_key_down(state_, glintfx::Key::Down);
            reload_();
        }
        present_frame_();
        present_frame_();
        {
            const glintfx::ElementBox list_box = ui_->get_element_box(kControlsListId);
            const glintfx::ElementBox row25 = ui_->get_element_box(controls_item_id(25).c_str());
            const bool visible = row25.found && list_box.found &&
                                  controls_row_visible_in_list(row25.y, row25.h,
                                                                list_box.y, list_box.h);
            std::cout << "SystemMenuLoop: [selftest][GLINTFX-SCROLL][teclado] apos "
                         "DOWN x25: controls_selected="
                      << state_.controls_selected << " (esperado 25); linha 25 - y="
                      << row25.y << " h=" << row25.h << "; recorte de ctrl-list - y="
                      << list_box.y << " h=" << list_box.h << " - "
                      << (state_.controls_selected == 25 && visible ? "OK" : "FALHOU")
                      << "\n";
        }

        // (c) RECONCILIACAO com o hit-test de mouse.
        {
            const glintfx::ElementBox list_box = ui_->get_element_box(kControlsListId);
            const glintfx::ElementBox row25 = ui_->get_element_box(controls_item_id(25).c_str());
            const float cx = row25.found ? row25.x + row25.w * 0.5f : -1.0f;
            const float cy = row25.found ? row25.y + row25.h * 0.5f : -1.0f;

            int winner = -1;
            for (int item = 0; item < kControlsItemCount; ++item) {
                const glintfx::ElementBox raw = ui_->get_element_box(controls_item_id(item).c_str());
                const glintfx::ElementBox box = filter_offscreen_controls_row(item, raw, list_box);
                if (hit_test(box, cx, cy)) {
                    winner = item;
                    break;
                }
            }
            const SystemMenuAction click_action25 =
                (winner == 25) ? system_menu_click_option(state_, winner) : SystemMenuAction::None;
            std::cout << "SystemMenuLoop: [selftest][GLINTFX-SCROLL][clique "
                         "pos-scroll] indice vencedor="
                      << winner << " (esperado 25); apos clique: controls_selected="
                      << state_.controls_selected << " controls_capturing="
                      << state_.controls_capturing << " (esperado selected=25 "
                         "capturing=1) - "
                      << (winner == 25 && state_.controls_selected == 25 &&
                                  state_.controls_capturing
                              ? "OK"
                              : "FALHOU")
                      << "\n";
            (void)click_action25;
            (void)system_menu_controls_capture_key(state_, /*is_escape=*/true, 0);  // cancela
        }
    }

    SDL_Window* window_;
    gus::platform::audio::AudioEngine& audio_;
    const gus::app::i18n::Translator& translator_;
    std::string settings_dir_;
    std::string saves_dir_;
    std::function<gus::domain::save::SaveData()> build_current_save_data_;
    std::function<void(const gus::domain::save::SaveData&)> apply_loaded_save_data_;
    std::string frozen_background_png_;

    // ESTADO DE NEGOCIO - inicializado UMA VEZ no construtor, PERSISTE entre
    // rodadas do driver (ver o comentario grande da classe acima).
    SystemMenuState state_;

    int pw_ = 0;
    int ph_ = 0;
    float dp_ratio_ = 1.0f;

    bool window_closed_ = false;
    bool bailed_ = false;  // ui_->ok()==false OU um selftest rodou ate o fim
    bool done_ = false;    // um SystemMenuScreenExit foi decidido por um system_screen_step

    // PONTO CRITICO #2: estado de arrasto de slider - vive ENTRE MOUSE_MOTIONs
    // (por isso e MEMBRO, resetado a CADA enter(), nao variavel local de
    // iteracao). -1 = nenhum arrasto em curso; 0=Music, 1=Sfx.
    int drag_item_ = -1;

    std::optional<gus::platform::render2d::Render2dGl3> backdrop_;
    gus::platform::render2d::TextureId frozen_bg_tex_ = gus::platform::render2d::kInvalidTexture;

    std::string stage_;
    std::string rml_path_;

    gus::platform::audio::SoundId hover_sfx_id_ = gus::platform::audio::kInvalidSound;
    gus::platform::audio::SoundId click_sfx_id_ = gus::platform::audio::kInvalidSound;
    std::string last_hover_sfx_id_;

    // Default Continue (MESMO fallback documentado no .hpp) - so lido pelo
    // driver quando done_==true (ou bailed_==true por ui_->ok()==false /
    // selftest, que tambem setam este campo explicitamente).
    SystemMenuScreenExit result_ = SystemMenuScreenExit::Continue;

    // std::optional (nao um objeto direto): enter()/exit() controlam o ciclo de
    // vida (EXCLUSIVIDADE DO UILAYER, ver gus/app/screen_state.hpp).
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
    // desenrola a pilha, pula o exit() e vai direto pra ~SystemMenuLoopScreen.
    // Ali so a ordem de declaracao protege. Bonus: alinha a destruicao
    // implicita com o que exit() ja faz a mao (ui_.reset() ANTES do resto).
    // Gate automatico: tools/callback_dtor_order.py (roda no tools/check.sh).
    std::optional<glintfx::UiLayer> ui_;
};

}  // namespace

// F4-1b.4 MINI-DRIVER: o NUCLEO que ASSUME um contexto GL JA CORRENTE + glad JA
// CARREGADO - ver o comentario grande no topo deste arquivo pro racional
// completo de POR QUE existe um driver aqui (a guarda de reentrada de
// run_screen_state PROIBE abrir a tela de save/load de DENTRO do handler do
// Pause).
SystemMenuLoopOutcome run_system_menu_loop_gl_current(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator, const std::string& settings_dir,
    const std::string& saves_dir,
    const std::function<gus::domain::save::SaveData()>& build_current_save_data,
    const std::function<void(const gus::domain::save::SaveData&)>&
        apply_loaded_save_data,
    const std::string& frozen_background_png, const gus::app::EventSyncHook& sync_hook,
    const std::function<void(const gus::domain::input::InputRemapConfig&)>&
        apply_controls_config) {
    SystemMenuLoopOutcome outcome;

    // DIAGNOSTICO/PROVA (SAVE-LOAD-UI etapa 6, prova visual headless Xvfb :99):
    // GUSWORLD_SAVELOAD_SCREENSHOT_DIR=<dir> pula o Pause por COMPLETO - MESMO
    // espirito de GUSWORLD_DIFFICULTY_SCREENSHOT_DIR em title_menu_loop.cpp
    // (roda ANTES de sequer construir a SystemMenuScreen - este selftest nunca
    // reusa a UiLayer/estado do Pause, so abre a tela REAL de save/load direto
    // em modo Save e depois Load, salvando 1 PNG de cada).
    if (const char* saveload_screenshot_dir = std::getenv("GUSWORLD_SAVELOAD_SCREENSHOT_DIR");
        saveload_screenshot_dir != nullptr && saveload_screenshot_dir[0] != '\0') {
        (void)run_save_load_menu_loop_gl_current(
            window, audio, translator, SaveLoadMode::Save, saves_dir, build_current_save_data,
            apply_loaded_save_data, frozen_background_png, /*sync_hook=*/nullptr, settings_dir,
            apply_controls_config);
        if (build_current_save_data) {
            // Semeia o slot 1 de verdade (I/O real, MESMO save_game que o
            // jogador aciona) - o modo Load abaixo mostra esse slot OCUPADO.
            gus::domain::save::SaveData seed = build_current_save_data();
            seed.slot_id = 1;
            const bool seed_ok = gus::platform::fs::save_game(seed, 1, saves_dir);
            if (!seed_ok) {
                std::cerr << "[system_menu] selftest: seed do slot 1 FALHOU (save_game=false) "
                             "- resultado do modo Load abaixo nao e representativo.\n";
            }
        }
        (void)run_save_load_menu_loop_gl_current(
            window, audio, translator, SaveLoadMode::Load, saves_dir, build_current_save_data,
            apply_loaded_save_data, frozen_background_png, /*sync_hook=*/nullptr, settings_dir,
            apply_controls_config);
        return outcome;
    }

    // SystemMenuScreen PERSISTE entre iteracoes deste driver (objeto local da
    // PILHA do wrapper, NAO recriado a cada volta) - o estado de negocio
    // (state_) roda UMA VEZ no construtor (ver a classe acima).
    SystemMenuLoopScreen system_screen(window, audio, translator, settings_dir, saves_dir,
                                    build_current_save_data, apply_loaded_save_data,
                                    frozen_background_png);

    for (;;) {
        // run_screen_state SO retorna DEPOIS que system_screen.exit() ja rodou
        // (garantia ESTRUTURAL, ver gus/app/screen_state.hpp:29-39) - so ENTAO
        // e seguro abrir a tela de save/load ANINHADA logo abaixo (2a UiLayer
        // viva ao mesmo tempo e o crash real que essa exclusividade existe pra
        // prevenir - "Element meta pool not empty on shutdown").
        gus::app::run_screen_state(system_screen, sync_hook);

        if (system_screen.window_closed()) {
            outcome.quit_app = true;
            return outcome;
        }

        const SystemMenuScreenExit sexit = system_screen.result();
        // Irrelevante fora de OpenSaveLoadSave/OpenSaveLoadLoad (pause_flow_next
        // ignora este valor nos outros 3 casos, ver os testes de
        // pause_flow_next) - valor qualquer (BackToPause) pra nao deixar a
        // variavel indeterminada.
        SaveLoadLoopExit saveload_exit = SaveLoadLoopExit::BackToPause;
        if (sexit == SystemMenuScreenExit::OpenSaveLoadSave ||
            sexit == SystemMenuScreenExit::OpenSaveLoadLoad) {
            const SaveLoadMode mode = (sexit == SystemMenuScreenExit::OpenSaveLoadSave)
                                           ? SaveLoadMode::Save
                                           : SaveLoadMode::Load;
            // ANINHADA (MESMA tecnica do antigo handle_action lambda, agora sem
            // o ui_opt.reset()/emplace() manual - a UiLayer do Pause JA foi
            // destruida, system_screen.exit() rodou dentro do run_screen_state
            // acima).
            saveload_exit = run_save_load_menu_loop_gl_current(
                window, audio, translator, mode, saves_dir, build_current_save_data,
                apply_loaded_save_data, frozen_background_png, /*sync_hook=*/nullptr,
                settings_dir, apply_controls_config);
        }

        switch (pause_flow_next(sexit, saveload_exit)) {
            case SystemMenuFlowStep::Continue:
                return outcome;  // quit_app=false/to_title=false (default)
            case SystemMenuFlowStep::RequestQuit:
                outcome.quit_app = true;
                return outcome;
            case SystemMenuFlowStep::RequestToTitle:
                outcome.to_title = true;
                return outcome;
            case SystemMenuFlowStep::RetryPause:
                break;  // RE-ENTRA no MESMO system_screen (topo do for(;;)).
        }
    }
}

}  // namespace gus::app::screens
