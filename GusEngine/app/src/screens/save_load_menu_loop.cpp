// SPDX-License-Identifier: GPL-3.0-or-later
// gus/app/src/screens/save_load_menu_loop.cpp
//
// Implementacao do loop interativo da tela de save/load. Ver header para o
// contrato completo. GL/glintfx-heavy (mesma familia de system_menu_loop.cpp) -
// sem unidade de teste direta pro loop em si (a logica PURA testavel ja fica em
// save_load_menu.hpp/save_load_menu_test.cpp e save_load_menu_rml.hpp/
// save_load_menu_rml_test.cpp; este .cpp so orquestra SDL/GL + o I/O de disco em
// torno delas) - o hit-test/geometria de layout E coberto pelo harness headless
// (app/tests/save_load_menu_interaction_test.cpp, Xvfb :99).
//
// MOUSE (retoque ao vivo do lider, bugs 1/3/6/9): ANTES desta onda, este arquivo
// NAO tratava NENHUM evento de mouse (so SDL_EVENT_KEY_DOWN) - "Voltar" nao
// respondia a clique, slots nao selecionavam, o icone de apagar nao existia. A
// receita de hit-test/SFX/wheel abaixo e a MESMA de system_menu_loop.cpp
// (get_element_box + hit_test pro CLIQUE), adaptada ao estado mais simples da
// tela de save/load (1 lista + 2 mini-dialogos, sem sub-telas). O SOM DE HOVER
// (mouse) migrou pro callback NATIVO glintfx::UiLayer::set_hover_callback em
// SFX-MIGRATE-V0.9 (ver hover_cb/is_navigable_hover_id mais abaixo) - o CLIQUE
// continua no hit-test manual de sempre.
//
// F4-1b.2 (onda F4 "casca SDL -> App mode do glintfx", fatia 1b.2 - SEGUNDA
// tela convertida ao contrato ScreenState, MESMO template de F4-1b.1/
// difficulty_menu_loop.cpp): o caminho de PRODUCAO
// (run_save_load_menu_loop_gl_current, unico chamador de producao
// system_menu_loop.cpp) foi convertido de um while(true){SDL_PollEvent...}
// PROPRIO pra classe SaveLoadScreen (gus::app::ScreenState) +
// gus::app::run_screen_state (gus/app/screen_state.hpp) - MESMA tecnica de
// DifficultyScreen (F4-1b.1). O roteamento de evento (decidir QUAL
// SaveLoadMenuAction resultou de 1 SDL_Event, mais SFX/resize/mouse_move) foi
// extraido pra uma FUNCAO LIVRE PURA nova, save_load_screen_step (declarada no
// .hpp, testada headless SEM SDL_Init/glintfx real em
// save_load_screen_step_test.cpp) - ela recebe as glintfx::ElementBox JA
// RESOLVIDAS como parametro (`boxes`), nunca consulta a UiLayer diretamente.
// TODO o I/O real (do_save_/do_delete_/do_recover_/apply_loaded_save_data_)
// fica FORA da funcao pura, em metodos da classe (ver
// SaveLoadScreen::apply_action_side_effects_ abaixo) - a funcao pura so decide
// QUAL SaveLoadMenuAction resultou, e o exit SO quando e 100% deterministico
// sem I/O (Back->BackToPause; SlotChosen em modo Load->ClosedAfterLoad, ver o
// comentario de SaveLoadStepResult no .hpp).
//
// ANINHADA (NAO owning_gl): MESMA nota de difficulty_menu_loop.cpp - esta
// funcao NAO cria/destroi o contexto GL, roda DENTRO do contexto que o
// CHAMADOR (system_menu_loop.cpp) ja deixou corrente (ele destroi a PROPRIA
// UiLayer ANTES de chamar esta funcao - RmlUi so aceita 1 instancia viva por
// processo).

#include "gus/app/screens/save_load_menu_loop.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

#include <glintfx/element_box.hpp>
#include <glintfx/ui_layer.hpp>

#include "gus/app/screens/save_load_menu_rml.hpp"
#include "gus/app/screens/system_menu.hpp"  // system_menu_wheel_delta_to_rmlui (REUSO, POCO generico)
#include "gus/core/asset_paths.hpp"  // kSfxDir/kMenuHoverSfxFile/kMenuClickSfxFile
#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Rect
#include "gus/domain/save/save_serializer.hpp"  // LoadResult
#include "gus/platform/assets/asset_source.hpp"  // FilesystemAssetSource (resolve SFX)
#include "gus/platform/fs/save_file_store.hpp"  // has_save/save_game/load_game/delete_save
#include "gus/platform/render2d/render2d_gl3.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"  // gl3_read_backbuffer_rgba (prova visual)

// stb_image_write: SO a declaracao aqui (a IMPLEMENTACAO ja vive UMA vez em
// battle_preview.cpp, MESMA lib gusengine_app - nao redefinir
// STB_IMAGE_WRITE_IMPLEMENTATION aqui, senao da symbol duplicado no link).
#include "stb_image_write.h"

#ifndef GUSWORLD_FONTS_DIR
#define GUSWORLD_FONTS_DIR ""
#endif

namespace gus::app::screens {

namespace {

namespace fs = std::filesystem;

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

// Stage PROPRIO (nao colide com o do menu de sistema, ver system_menu_loop.cpp).
std::string save_load_stage_dir() {
    return (fs::temp_directory_path() / "gusworld_glintfx_saveload").string();
}

// Ids (save_load_menu_rml.cpp): "slmenu-slot-<i>" (linha do slot), "slmenu-delete-
// <i>" (icone de apagar por-linha, so em slots OCUPADOS), "slmenu-back" (Voltar,
// id fixo), "slmenu-confirm-yes/no" (mini-dialogo de sobrescrita), "slmenu-delete-
// confirm-yes/no" (mini-dialogo de exclusao), "slmenu-warn-recover"/"slmenu-warn-
// cancel" (AVISO de slot ilegivel, SAVE-LOAD-AVISOS - o botao recover so existe
// quando warning_kind==Damaged, ver save_load_menu_rml.cpp).
std::string slot_item_id(int slot) { return "slmenu-slot-" + std::to_string(slot); }
std::string delete_item_id(int slot) { return "slmenu-delete-" + std::to_string(slot); }
constexpr const char* kBackId = "slmenu-back";
constexpr const char* kOverwriteConfirmId[2] = {"slmenu-confirm-yes", "slmenu-confirm-no"};
constexpr const char* kDeleteConfirmId[2] = {"slmenu-delete-confirm-yes",
                                              "slmenu-delete-confirm-no"};
constexpr const char* kWarnRecoverId = "slmenu-warn-recover";
constexpr const char* kWarnCancelId = "slmenu-warn-cancel";

// Hit-test simples: cursor (x,y, espaco-janela) dentro da caixa border-box
// devolvida por glintfx::UiLayer::get_element_box (MESMA receita de
// system_menu_loop.cpp::hit_test). box.found=false conta como "fora".
bool hit_test(const glintfx::ElementBox& box, float x, float y) noexcept {
    if (!box.found) return false;
    return x >= box.x && x <= box.x + box.w && y >= box.y && y <= box.y + box.h;
}

// Resolve o caminho de um SFX do menu (hover/click) - MESMA receita/fronteira
// (ASSETS-VFS-F1/ADR-013) de resolve_menu_sfx_path em system_menu_loop.cpp.
std::string resolve_menu_sfx_path(std::string_view file) {
    const std::string id = join(std::string(gus::core::assets::kSfxDir), std::string(file));
    return gus::platform::assets::FilesystemAssetSource().resolve_path(id);
}

// SFX-MIGRATE-V0.9: filtro NAVEGAVEL pro callback NATIVO de hover
// (glintfx::UiLayer::set_hover_callback) - dado o `id` que o hover nativo
// reportou (entered=true) e o estado ATUAL, devolve true SO se `id` e um dos
// alvos hover-testaveis por CLIQUE (MESMOS ids do roteamento de
// SDL_EVENT_MOUSE_BUTTON_DOWN mais abaixo: warning/confirming_delete/
// confirming_overwrite/slot+icone-de-apagar+Voltar). Substitui o antigo
// current_hover_index (que devolvia um INDICE linear pra
// system_menu_hover_entered_new_item comparar) - o dedup agora e por ID,
// interno ao proprio hover nativo (ver hover_cb mais abaixo). 100%
// string/estado, sem GL.
bool is_navigable_hover_id(const SaveLoadMenuState& state, const std::string& id) {
    if (state.warning_kind != SaveLoadMenuState::WarningKind::None) {
        // Damaged tem 2 botoes (0=recover, 1=cancel); Version/RecoverFailed so
        // tem o Cancelar (o id recover nem existe na RML nesse caso - o hover
        // nativo nunca reporta um id que nao existe no documento carregado).
        return id == kWarnRecoverId || id == kWarnCancelId;
    }
    if (state.confirming_delete) {
        return id == kDeleteConfirmId[0] || id == kDeleteConfirmId[1];
    }
    if (state.confirming_overwrite) {
        return id == kOverwriteConfirmId[0] || id == kOverwriteConfirmId[1];
    }
    for (int i = 0; i < gus::domain::save::kSlotCount; ++i) {
        if (slot_selectable(state, i) && id == slot_item_id(i)) return true;
    }
    for (int i = 0; i < gus::domain::save::kSlotCount; ++i) {
        if (state.slots[static_cast<std::size_t>(i)].occupied && id == delete_item_id(i)) {
            return true;
        }
    }
    return id == kBackId;
}

std::string write_save_load_rml_file(const SaveLoadMenuState& state,
                                      const gus::app::i18n::Translator& tr) {
    const fs::path stage = save_load_stage_dir();
    std::error_code ec;
    fs::create_directories(stage, ec);

    std::string fonts_dir = GUSWORLD_FONTS_DIR;
    if (const char* envf = std::getenv("GUSWORLD_FONTS")) {
        if (envf[0] != '\0') fonts_dir = envf;
    }
    if (!fonts_dir.empty()) {
        fs::copy_file(join(fonts_dir, "PixelOperatorMono.ttf"),
                      stage / "PixelOperatorMono.ttf",
                      fs::copy_options::overwrite_existing, ec);
        fs::copy_file(join(fonts_dir, "PixelOperatorMono-Bold.ttf"),
                      stage / "PixelOperatorMono-Bold.ttf",
                      fs::copy_options::overwrite_existing, ec);
    }

    // FONT-EXTEND-GLITCH (2026-07-29): a fonte NAO e mais injetada aqui via @font-face
    // de string - SaveLoadMenuLoopScreen::enter() registra a familia 1x via
    // glintfx::UiLayer::load_font_face (API v0.24.0) logo apos set_asset_base_url (ver
    // register_pixel_operator_mono_fonts abaixo). Os .ttf continuam copiados pro stage
    // acima (load_font_face resolve o path pela MESMA BaseUrlFileInterface que a
    // @font-face antiga usava).
    std::string rml = build_save_load_menu_rml(state, tr);

    const fs::path out = stage / "save_load_menu.rml";
    std::ofstream f(out);
    f << rml;
    return out.string();
}

// FONT-EXTEND-GLITCH (2026-07-29): registra "Pixel Operator Mono" (regular+bold) via
// glintfx::UiLayer::load_font_face (API v0.24.0) - mata o @font-face por string que
// write_save_load_rml_file injetava antes. Chamado 1x em enter() (registro e
// process-wide RmlUi state, ver ui_layer.hpp), DEPOIS do stage ja ter os .ttf (o
// PRIMEIRO write_save_load_rml_file os copia) e ANTES de ui_->load(). `family` SEMPRE
// explicito (armadilha de assimetria de motor Own-vs-FreeType documentada em
// font_face.hpp - o mesmo literal que o RCSS ja referencia neutraliza os dois).
void register_pixel_operator_mono_fonts(glintfx::UiLayer& ui) {
    if (!ui.load_font_face(glintfx::FontFaceDesc{/*path=*/"PixelOperatorMono.ttf",
                                                  /*family=*/"Pixel Operator Mono"})) {
        std::cerr << "SaveLoadMenuLoop: load_font_face(PixelOperatorMono.ttf) falhou "
                     "(arquivo ausente/invalido no stage) - cai no fallback de fonte do "
                     "RmlUi.\n";
    }
    if (!ui.load_font_face(glintfx::FontFaceDesc{
            /*path=*/"PixelOperatorMono-Bold.ttf",
            /*family=*/"Pixel Operator Mono",
            /*style=*/glintfx::FontStyle::Normal,
            /*weight=*/glintfx::FontWeight::Bold})) {
        std::cerr << "SaveLoadMenuLoop: load_font_face(PixelOperatorMono-Bold.ttf) "
                     "falhou (arquivo ausente/invalido no stage) - cai no fallback de "
                     "fonte do RmlUi.\n";
    }
}

// Le TODOS os slots do disco e monta os previews + (modo Load) um cache do
// SaveData ja carregado por slot (evita ler o arquivo 2x ao confirmar). Um save
// PRESENTE mas NAO Ok (HmacInvalid/Corrupt/VersionTooNew/Invalid/WrongSlot)
// continua "occupied=false" (a lista NUNCA mostra dado nao confiavel) mas ganha
// unreadable_slot_preview com o LoadResult REAL - CRIT-1 (auditoria AUD-SAVE-
// LOAD-UI-2026-07-09): present_unreadable=true PEDE confirmacao de sobrescrita
// em modo Save; SAVE-LOAD-AVISOS (aviso #1): em modo Load, o slot vira
// SELECIONAVEL com o rotulo "! Danificado"/"! Versao incompativel" e, ao ser
// confirmado, abre o aviso dedicado (ver save_load_menu.hpp::confirm_selected_
// slot) - "Tentar recuperar" e tratado em SaveLoadScreen::do_recover_ abaixo via
// gus::platform::fs::load_game_from_backup.
std::array<SaveSlotPreview, gus::domain::save::kSlotCount> build_previews_and_cache(
    const std::string& saves_dir,
    std::array<std::optional<gus::domain::save::SaveData>, gus::domain::save::kSlotCount>&
        loaded_cache) {
    std::array<SaveSlotPreview, gus::domain::save::kSlotCount> previews{};
    for (int slot = 0; slot < gus::domain::save::kSlotCount; ++slot) {
        loaded_cache[static_cast<std::size_t>(slot)].reset();
        if (!gus::platform::fs::has_save(slot, saves_dir)) {
            previews[static_cast<std::size_t>(slot)] = empty_slot_preview(slot);
            continue;
        }
        const auto outcome = gus::platform::fs::load_game(slot, saves_dir);
        if (outcome.has_value() &&
            outcome->result == gus::domain::save::LoadResult::Ok) {
            previews[static_cast<std::size_t>(slot)] =
                build_slot_preview(outcome->data, slot);
            loaded_cache[static_cast<std::size_t>(slot)] = outcome->data;
        } else {
            // SAVE-LOAD-AVISOS: o motivo REAL (LoadResult) alimenta o aviso
            // dedicado - VersionTooNew vira o aviso "so Cancelar" (forward-only);
            // qualquer outro (inclusive o degradado por falha de I/O pura, sem
            // outcome, tratado como Corrupt) vira "Danificado" (RECUPERAVEL via
            // "Tentar recuperar", ver load_game_from_backup).
            const gus::domain::save::LoadResult reason =
                outcome.has_value() ? outcome->result : gus::domain::save::LoadResult::Corrupt;
            std::cerr << "[save_load_menu_loop] aviso: slot " << slot
                      << " tem arquivo mas NAO carregou Ok (adulterado/corrompido/"
                         "versao incompativel/slot trocado) - marcado "
                         "present_unreadable=true (CRIT-1: sobrescrita pede "
                         "confirmacao; SAVE-LOAD-AVISOS: selecionavel em Load, "
                         "abre o aviso dedicado).\n";
            previews[static_cast<std::size_t>(slot)] = unreadable_slot_preview(slot, reason);
        }
    }
    return previews;
}

// F4-1b.2: aplica o DESFECHO de uma SaveLoadMenuAction (vinda do teclado OU de
// um clique de mouse) no SaveLoadStepResult - a versao PURA/SEM-I/O do
// handle_action antigo: so decide `reload`/`exit`/`action` quando isso NAO
// exige tocar disco (Back->BackToPause sempre; SlotChosen em modo Load->
// ClosedAfterLoad, assumindo o INVARIANTE "slot selecionavel em Load SEMPRE
// tem cache" - ver save_load_menu.hpp). Os casos que EXIGEM I/O real
// (SlotChosen em modo Save/do_save, OverwriteConfirmed/do_save,
// DeleteConfirmed/do_delete, RecoverRequested/do_recover) so marcam `reload`
// (quando aplicavel) e devolvem o `action` cru pro CHAMADOR
// (SaveLoadScreen::apply_action_side_effects_) executar de fato. Compartilhada
// entre o roteamento de teclado (save_load_screen_step) e de mouse
// (route_mouse_click abaixo) - MESMO racional de nao duplicar do_save/
// do_delete/reload que o handle_action antigo ja tinha. Helper INTERNO deste
// .cpp (nao faz parte do contrato do .hpp - save_load_screen_step e a UNICA
// funcao exportada/testada).
void apply_action_to_result(SaveLoadStepResult& result, const SaveLoadMenuState& state,
                             SaveLoadMenuAction action) noexcept {
    result.action = action;
    switch (action) {
        case SaveLoadMenuAction::None:
            result.reload = true;
            return;
        case SaveLoadMenuAction::Back:
            // BackToPause NAO precisa de I/O nenhum - decidido aqui, 100% PURO.
            result.exit = SaveLoadLoopExit::BackToPause;
            return;
        case SaveLoadMenuAction::SlotChosen:
            if (state.mode == SaveLoadMode::Load) {
                // INVARIANTE (ver save_load_menu.hpp/confirm_selected_slot):
                // slot_selectable em Load SEMPRE tem cache - o CHAMADOR ainda
                // aplica apply_loaded_save_data(cached) de fato e mantem o
                // guard defensivo pro caso teorico do cache faltar (nunca
                // deveria acontecer na pratica).
                result.exit = SaveLoadLoopExit::ClosedAfterLoad;
            } else {
                // Modo Save: confirm_selected_slot SEMPRE abre
                // confirming_overwrite antes de devolver SlotChosen (ver
                // save_load_menu.cpp) - este ramo e defensivo/inalcancavel na
                // pratica, preservado do handle_action antigo (que tambem o
                // tinha) sem mudar comportamento.
                result.reload = true;
            }
            return;
        case SaveLoadMenuAction::OverwriteConfirmed:
        case SaveLoadMenuAction::DeleteConfirmed:
            // do_save/do_delete (I/O real) ficam com o CHAMADOR - aqui so liga
            // o reload (o CHAMADOR recarrega o RML DEPOIS do I/O, MESMO
            // comportamento do handle_action antigo).
            result.reload = true;
            return;
        case SaveLoadMenuAction::OverwriteCancelled:
        case SaveLoadMenuAction::DeleteCancelled:
        case SaveLoadMenuAction::WarningCancelled:
            result.reload = true;
            return;
        case SaveLoadMenuAction::RecoverRequested:
            // do_recover (I/O real, load_game_from_backup) decide o exit de
            // fato - NUNCA seta reload/exit aqui (o CHAMADOR, apos tentar,
            // sabe se foi ClosedAfterLoad ou se precisa recarregar com
            // warning_kind=RecoverFailed).
            return;
    }
}

// F4-1b.2: roteia UM clique de mouse (x,y, espaco-janela) pela ORDEM de
// prioridade EXATA do while(true) antigo (warning > confirming_delete >
// confirming_overwrite > lista normal - dentro da lista normal: Voltar >
// icone-de-apagar > slot), mutando `state` via as MESMAS
// save_load_menu_click_*/save_load_menu_request_delete de sempre. QUALQUER
// clique que acertou um alvo real (handled=true) toca Click - MESMA
// simplificacao "todo clique reconhecido soa" do while(true) antigo (ver o
// comentario de "SOM DE CLIQUE" que existia ali).
void route_mouse_click(SaveLoadMenuState& state, float x, float y,
                        const SaveLoadStepBoxes& boxes, SaveLoadStepResult& result) noexcept {
    bool handled = false;

    if (state.warning_kind != SaveLoadMenuState::WarningKind::None) {
        // "Tentar recuperar" (so existe quando Damaged) - checado ANTES do
        // Cancelar, MESMO padrao "mais especifico vence" do icone de apagar
        // abaixo (aqui nao ha sobreposicao real, mas mantem a ordem consistente).
        if (hit_test(boxes.warn_recover, x, y)) {
            handled = true;
            apply_action_to_result(result, state, save_load_menu_click_warning_recover(state));
        } else if (hit_test(boxes.warn_cancel, x, y)) {
            handled = true;
            apply_action_to_result(result, state, save_load_menu_click_warning_cancel(state));
        }
    } else if (state.confirming_delete) {
        for (int i = 0; i < 2 && !handled; ++i) {
            if (!hit_test(boxes.delete_confirm[static_cast<std::size_t>(i)], x, y)) continue;
            handled = true;
            apply_action_to_result(result, state,
                                    save_load_menu_click_delete_confirm(state, i));
        }
    } else if (state.confirming_overwrite) {
        for (int i = 0; i < 2 && !handled; ++i) {
            if (!hit_test(boxes.overwrite_confirm[static_cast<std::size_t>(i)], x, y)) continue;
            handled = true;
            apply_action_to_result(result, state,
                                    save_load_menu_click_overwrite_confirm(state, i));
        }
    } else {
        // Voltar (bug 1/6/9: antes desta onda, so o teclado fechava a tela) -
        // id fixo, SEMPRE presente na lista normal.
        if (hit_test(boxes.back, x, y)) {
            handled = true;
            apply_action_to_result(result, state, SaveLoadMenuAction::Back);
        }
        // Icone de apagar por-linha (feature "Apagar") - checado ANTES dos
        // slots: a caixa do icone fica DENTRO da linha do slot (ver
        // save_load_menu_rml.cpp), o mais especifico vence. Muta o estado
        // DIRETO via save_load_menu_request_delete (nao passa por uma
        // SaveLoadMenuAction) - `result.action` fica nullopt, so `reload` liga.
        for (int i = 0; i < gus::domain::save::kSlotCount && !handled; ++i) {
            if (!state.slots[static_cast<std::size_t>(i)].occupied) continue;
            if (!hit_test(boxes.delete_icons[static_cast<std::size_t>(i)], x, y)) continue;
            handled = true;
            save_load_menu_request_delete(state, i);
            result.reload = true;
        }
        // Clicar num slot (bug 3: antes desta onda, nao fazia nada) - "focar +
        // Enter" (MESMA convencao de system_menu_click_option).
        for (int i = 0; i < gus::domain::save::kSlotCount && !handled; ++i) {
            if (!hit_test(boxes.slots[static_cast<std::size_t>(i)], x, y)) continue;
            handled = true;
            apply_action_to_result(result, state, save_load_menu_click_slot(state, i));
        }
    }

    if (handled) result.sfx = SaveLoadSfxKind::Click;
}

}  // namespace

// F4-1b.2: implementacao de save_load_screen_step (declarada no .hpp) - ver o
// comentario grande la pro contrato completo. Extracao BEHAVIOR-PRESERVING do
// corpo do while(true) antigo (MESMO racional/ordem de checagem de tipo de
// evento) - so devolvendo a DECISAO (via apply_action_to_result/
// route_mouse_click) em vez de executar do_save/do_delete/do_recover/
// apply_loaded_save_data na hora.
SaveLoadStepResult save_load_screen_step(SaveLoadMenuState& state, const SDL_Event& ev,
                                          const SaveLoadStepBoxes& boxes) noexcept {
    SaveLoadStepResult result;

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
        const SaveLoadMenuAction action = save_load_menu_key_down(state, ev.key.key);
        apply_action_to_result(result, state, action);
        return result;
    }

    if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN && ev.button.button == SDL_BUTTON_LEFT) {
        route_mouse_click(state, ev.button.x, ev.button.y, boxes, result);
        return result;
    }

    if (ev.type == SDL_EVENT_MOUSE_MOTION) {
        result.mouse_move = true;
        result.mouse_x = ev.motion.x;
        result.mouse_y = ev.motion.y;
        return result;
    }

    // SDL_EVENT_MOUSE_WHEEL (forwarding pra `.slot-list`) e qualquer outro tipo
    // de evento NAO sao roteados aqui - MOUSE_WHEEL exige SDL_GetMouseState
    // (consulta IMPURA de cursor real), tratado direto em
    // SaveLoadScreen::handle_event (save_load_menu_loop.cpp) ANTES de chamar
    // esta funcao.
    return result;  // no-op TOTAL
}

namespace {

// F4-1b.2: o ScreenState de PRODUCAO da tela de save/load (unico chamador:
// run_save_load_menu_loop_gl_current, abaixo) - MESMO padrao de
// DifficultyScreen (difficulty_menu_loop.cpp, F4-1b.1). Todo o estado que
// antes vivia em variaveis locais fechadas por lambdas (ui/backdrop/cache de
// saves carregados/ids de SFX/rml_path/etc) agora e MEMBRO; enter() cria
// (glintfx::UiLayer + Render2dGl3 + ids de SFX), exit() libera (a
// EXCLUSIVIDADE do UiLayer descrita em gus/app/screen_state.hpp).
class SaveLoadScreen final : public gus::app::ScreenState {
public:
    SaveLoadScreen(SDL_Window* window, gus::platform::audio::AudioEngine& audio,
                   const gus::app::i18n::Translator& translator, SaveLoadMode mode,
                   const std::string& saves_dir,
                   const std::function<gus::domain::save::SaveData()>& build_current_save_data,
                   const std::function<void(const gus::domain::save::SaveData&)>&
                       apply_loaded_save_data,
                   std::string frozen_background_png)
        : window_(window),
          audio_(audio),
          translator_(translator),
          mode_(mode),
          saves_dir_(saves_dir),
          build_current_save_data_(build_current_save_data),
          apply_loaded_save_data_(apply_loaded_save_data),
          frozen_background_png_(std::move(frozen_background_png)) {}

    void enter() override {
        const std::array<SaveSlotPreview, gus::domain::save::kSlotCount> previews =
            build_previews_and_cache(saves_dir_, loaded_cache_);
        save_load_menu_open(state_, mode_, previews);

        SDL_GetWindowSizeInPixels(window_, &pw_, &ph_);
        if (pw_ < 1) pw_ = 1;
        if (ph_ < 1) ph_ = 1;
        dp_ratio_ = static_cast<float>(pw_) / 960.0f;

        ui_.emplace(glintfx::UiLayer::Config{/*logical_width=*/960,
                                              /*logical_height=*/540,
                                              /*load_gl=*/true,
                                              /*dp_ratio=*/dp_ratio_});
        if (!ui_->ok()) {
            std::cerr << "SaveLoadMenuLoop: glintfx::UiLayer::ok()=false (attach "
                         "falhou) - fechando sem desenhar (degradacao segura).\n";
            result_ = SaveLoadLoopExit::BackToPause;
            bailed_ = true;
            return;
        }

        stage_ = save_load_stage_dir();
        ui_->set_asset_base_url(stage_.c_str());
        rml_path_ = write_save_load_rml_file(state_, translator_);
        register_pixel_operator_mono_fonts(*ui_);
        ui_->load(rml_path_.c_str());
        ui_->set_viewport(pw_, ph_);
        ui_->set_dp_ratio(dp_ratio_);
        // SFX-MIGRATE-V0.9: 1 update() de "assentamento" AQUI, ANTES do loop -
        // achado EMPIRICO (harness headless, save_load_menu_interaction_test.cpp):
        // ver o comentario extenso que vivia no while(true) antigo (o hover NATIVO
        // so resolve elemento sob o cursor DEPOIS de pelo menos 1
        // Context::Update() ter rodado pro documento RECEM-carregado).
        ui_->update();

        backdrop_.emplace(/*gl_active=*/true);
        frozen_bg_tex_ = frozen_background_png_.empty()
                             ? gus::platform::render2d::kInvalidTexture
                             : backdrop_->load_texture(frozen_background_png_.c_str());

        // SFX de hover/clique (paridade com system_menu_loop.cpp/
        // title_menu_loop.cpp - "todo menu de botoes soa"): load_sfx UMA VEZ por
        // sessao desta tela. audio.available()==false (device indisponivel/CI)
        // degrada com seguranca (play_sfx(id invalido) e no-op).
        const std::string hover_sfx_path =
            resolve_menu_sfx_path(gus::core::assets::kMenuHoverSfxFile);
        const std::string click_sfx_path =
            resolve_menu_sfx_path(gus::core::assets::kMenuClickSfxFile);
        hover_sfx_id_ = audio_.load_sfx(hover_sfx_path.c_str());
        click_sfx_id_ = audio_.load_sfx(click_sfx_path.c_str());

        // SOM DE HOVER (mouse) - SFX-MIGRATE-V0.9: hover_cb e o callback NATIVO
        // (glintfx::UiLayer::set_hover_callback) - ver o comentario extenso que
        // vivia no while(true) antigo (last_hover_sfx_id_ + is_navigable_hover_id
        // filtram os containers que o hover nativo TAMBEM resolve mas nunca devem
        // soar).
        ui_->set_hover_callback(
            [this](const char* raw_id, bool entered) { native_hover_callback_(raw_id, entered); });

        // DIAGNOSTICO/PROVA (SAVE-LOAD-UI etapa 6, prova visual headless Xvfb
        // :99): GUSWORLD_SAVELOAD_SCREENSHOT_DIR=<dir> assenta alguns frames e
        // salva 1 PNG ANTES de entrar no loop interativo - bypassa por completo
        // (MESMO espirito de GUSWORLD_DIFFICULTY_SCREENSHOT_DIR em
        // difficulty_menu_loop.cpp).
        const char* screenshot_dir = std::getenv("GUSWORLD_SAVELOAD_SCREENSHOT_DIR");
        if (screenshot_dir != nullptr && screenshot_dir[0] != '\0') {
            for (int i = 0; i < 6; ++i) present_frame_();
            std::vector<unsigned char> buf(static_cast<std::size_t>(pw_) *
                                            static_cast<std::size_t>(ph_) * 4);
            if (gus::platform::rmlui::gl3_read_backbuffer_rgba(pw_, ph_, buf.data())) {
                const std::string suffix = (mode_ == SaveLoadMode::Save) ? "save" : "load";
                const std::string out =
                    join(std::string(screenshot_dir), "save_load_" + suffix + ".png");
                stbi_write_png(out.c_str(), pw_, ph_, 4, buf.data(), pw_ * 4);
                std::cout << "SaveLoadMenuLoop: [screenshot] " << out << " (" << pw_ << "x" << ph_
                          << ")\n";
            } else {
                std::cerr << "SaveLoadMenuLoop: [screenshot] gl3_read_backbuffer_rgba "
                             "falhou\n";
            }
            result_ = SaveLoadLoopExit::BackToPause;
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

        // WHEEL FORWARDING (`.slot-list`) - MESMA receita de
        // system_menu_loop.cpp/o while(true) antigo desta tela: 1 MouseMove
        // sintetico pra posicao ATUAL do cursor IMEDIATAMENTE antes do
        // MouseWheel (rola o elemento em HOVER, nao o focado - gotcha do
        // glintfx v0.4.0). SDL_GetMouseState e uma consulta IMPURA de cursor
        // real - FICA FORA de save_load_screen_step (que so recebe o SDL_Event
        // ja pumpado, sem tocar hardware), tratado direto aqui.
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

        // Resolve as boxes SO quando o evento precisa (MESMO custo do
        // while(true) antigo, que so chamava get_element_box dentro do ramo de
        // clique) - demais eventos passam um SaveLoadStepBoxes default (todas
        // found=false), que save_load_screen_step so consulta no ramo de
        // MOUSE_BUTTON_DOWN.
        SaveLoadStepBoxes boxes{};
        if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN && ev.button.button == SDL_BUTTON_LEFT) {
            boxes = collect_click_boxes_();
        }

        const SaveLoadStepResult step = save_load_screen_step(state_, ev, boxes);

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
        if (step.sfx == SaveLoadSfxKind::Click) {
            audio_.play_sfx(click_sfx_id_);
        }

        if (step.action.has_value()) {
            // apply_action_side_effects_ executa TODO o I/O real (do_save_/
            // do_delete_/do_recover_/apply_loaded_save_data_) e devolve true
            // quando ja tratou o reload/exit por conta propria (TODOS os
            // ramos exceto None/*Cancelled, que ja vieram com `reload=true`
            // do lado PURO) - so nesse caso o CHAMADOR (aqui) ainda aplica
            // step.reload.
            if (!apply_action_side_effects_(*step.action) && step.reload) {
                reload_();
            }
            return;
        }
        if (step.reload) {
            // Ramo sem `action` (ex.: clique no icone de apagar, que muta o
            // estado DIRETO via save_load_menu_request_delete) - so recarrega.
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
        // ANTES de backdrop_ (MESMA ordem de sempre).
        ui_.reset();
        backdrop_.reset();
    }

    [[nodiscard]] bool window_closed() const override { return window_closed_; }

    // Exposto SO pro wrapper run_save_load_menu_loop_gl_current (nao faz parte
    // do contrato ScreenState) - o desfecho decidido pelo ultimo
    // save_load_screen_step/apply_action_side_effects_ com exit preenchido, OU
    // o default BackToPause (ui_->ok()==false / screenshot mode).
    [[nodiscard]] SaveLoadLoopExit result() const { return result_; }

private:
    // Confirma um slot em modo SAVE: pede o SaveData VIVO ao CHAMADOR
    // (timestamp fresco), grava de fato, e ATUALIZA o preview do slot NA HORA
    // (sem fechar a tela - o jogador ve o novo timestamp/playtime
    // imediatamente).
    void do_save_(int slot) {
        if (!build_current_save_data_) return;  // defensivo: chamador nao forneceu
        gus::domain::save::SaveData data = build_current_save_data_();
        data.slot_id = slot;
        const bool ok = gus::platform::fs::save_game(data, slot, saves_dir_);
        if (!ok) {
            std::cerr << "[save_load_menu_loop] falha ao gravar slot " << slot
                      << " (I/O - disco cheio/permissao?) - estado em memoria "
                         "intocado, nada persistiu.\n";
            return;
        }
        state_.slots[static_cast<std::size_t>(slot)] = build_slot_preview(data, slot);
        loaded_cache_[static_cast<std::size_t>(slot)] = data;
    }

    // Apaga de fato o slot (feature "Apagar", aprovada pelo lider): I/O real
    // via gus::platform::fs::delete_save, e ATUALIZA o preview local NA HORA +
    // re-ancora a selecao se o slot apagado era o focado e deixou de ser
    // selecionavel.
    void do_delete_(int slot) {
        const bool ok = gus::platform::fs::delete_save(slot, saves_dir_);
        if (!ok) {
            std::cerr << "[save_load_menu_loop] falha ao apagar slot " << slot
                      << " (I/O - permissao negada?) - preview local NAO "
                         "atualizado (o arquivo pode continuar em disco).\n";
            return;
        }
        state_.slots[static_cast<std::size_t>(slot)] = empty_slot_preview(slot);
        loaded_cache_[static_cast<std::size_t>(slot)].reset();
        save_load_menu_reselect_if_needed(state_);
    }

    // SAVE-LOAD-AVISOS: "Tentar recuperar" do aviso Damaged - tenta a cadeia
    // de backup de fato. Sucesso: MESMO caminho de um Load normal
    // bem-sucedido (ClosedAfterLoad). Falha: transita state_.warning_kind pra
    // RecoverFailed e MANTEM a tela aberta (reload_(), nao fecha) - devolve
    // nullopt igual aos outros ramos que "so tratam e continuam".
    std::optional<SaveLoadLoopExit> do_recover_(int slot) {
        const auto recovered = gus::platform::fs::load_game_from_backup(slot, saves_dir_);
        if (recovered.has_value() && recovered->result == gus::domain::save::LoadResult::Ok &&
            apply_loaded_save_data_) {
            apply_loaded_save_data_(recovered->data);
            return SaveLoadLoopExit::ClosedAfterLoad;
        }
        std::cerr << "[save_load_menu_loop] recuperacao do slot " << slot
                  << " falhou (nenhuma geracao de backup carregou Ok) - abrindo "
                     "o aviso RecoverFailed.\n";
        state_.warning_kind = SaveLoadMenuState::WarningKind::RecoverFailed;
        state_.warning_selected = 1;
        reload_();
        return std::nullopt;
    }

    // F4-1b.2: executa o I/O real (quando aplicavel) pra UMA SaveLoadMenuAction
    // ja devolvida por save_load_screen_step/route_mouse_click - MESMO
    // dispatch do handle_action lambda antigo, so que como metodo. Devolve
    // true quando ja tratou reload/exit por conta propria (TODOS os ramos
    // exceto None/*Cancelled, que sao no-op de I/O e ja vieram com
    // `reload=true` do lado PURO - o CHAMADOR (handle_event) ainda aplica esse
    // reload).
    bool apply_action_side_effects_(SaveLoadMenuAction action) {
        switch (action) {
            case SaveLoadMenuAction::Back:
                done_ = true;
                result_ = SaveLoadLoopExit::BackToPause;
                return true;
            case SaveLoadMenuAction::SlotChosen:
                if (state_.mode == SaveLoadMode::Save) {
                    // Ramo defensivo/inalcancavel na pratica (ver o comentario
                    // de apply_action_to_result) - preservado do
                    // handle_action antigo, sem mudar comportamento.
                    do_save_(state_.selected);
                    reload_();
                    return true;
                }
                {
                    const auto& cached =
                        loaded_cache_[static_cast<std::size_t>(state_.selected)];
                    if (cached.has_value() && apply_loaded_save_data_) {
                        apply_loaded_save_data_(*cached);
                        done_ = true;
                        result_ = SaveLoadLoopExit::ClosedAfterLoad;
                        return true;
                    }
                    // Defensivo: slot selecionavel em Load SEMPRE tem cache
                    // (ver build_previews_and_cache) - se nao tiver, no-op
                    // seguro (fica na lista) em vez de fingir um load.
                    std::cerr << "[save_load_menu_loop] BUG defensivo: slot "
                              << state_.selected
                              << " selecionavel em Load sem cache - ignorando "
                                 "(nao finge um load).\n";
                    reload_();
                    return true;
                }
            case SaveLoadMenuAction::OverwriteConfirmed:
                do_save_(state_.selected);
                reload_();
                return true;
            case SaveLoadMenuAction::DeleteConfirmed:
                do_delete_(state_.delete_target_slot);
                reload_();
                return true;
            case SaveLoadMenuAction::RecoverRequested: {
                const std::optional<SaveLoadLoopExit> outcome = do_recover_(state_.selected);
                if (outcome.has_value()) {
                    done_ = true;
                    result_ = *outcome;
                }
                // do_recover_ ja chama reload_() no caminho de falha
                // (RecoverFailed) - nao reload_() de novo aqui.
                return true;
            }
            case SaveLoadMenuAction::None:
            case SaveLoadMenuAction::OverwriteCancelled:
            case SaveLoadMenuAction::DeleteCancelled:
            case SaveLoadMenuAction::WarningCancelled:
                return false;  // sem I/O - o CHAMADOR aplica step.reload
        }
        return false;
    }

    void reload_() {
        rml_path_ = write_save_load_rml_file(state_, translator_);
        ui_->load(rml_path_.c_str());
        ui_->set_viewport(pw_, ph_);
        ui_->set_dp_ratio(dp_ratio_);
    }

    void present_frame_() {
        const gus::core::spatial::Rect cam{0.0f, 0.0f, static_cast<float>(pw_),
                                            static_cast<float>(ph_)};
        backdrop_->begin_frame(cam, pw_, ph_);
        if (frozen_bg_tex_ != gus::platform::render2d::kInvalidTexture) {
            backdrop_->draw_textured_rect(
                cam, frozen_bg_tex_, gus::platform::render2d::UvRect{0.0f, 0.0f, 1.0f, 1.0f},
                gus::platform::render2d::DrawColor{1.0f, 1.0f, 1.0f, 1.0f});
        }
        backdrop_->end_frame();
        ui_->update();
        ui_->render();
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
    // receita do hover_cb lambda antigo, agora metodo (captura `this` em vez de
    // fechar sobre variaveis locais).
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

    // Resolve as boxes de CLIQUE do frame ATUAL - SO as do ramo/estado ATUAL de
    // state_ (warning/confirming_delete/confirming_overwrite/lista normal),
    // MESMA leitura que save_load_screen_step faz internamente pra escolher
    // qual sub-array de SaveLoadStepBoxes consultar. SO chamado do ramo
    // MOUSE_BUTTON_DOWN de handle_event() (get_element_box e uma query da
    // UiLayer - nao roda em todo evento, MESMO custo do while(true) antigo).
    [[nodiscard]] SaveLoadStepBoxes collect_click_boxes_() const {
        SaveLoadStepBoxes boxes{};
        if (state_.warning_kind != SaveLoadMenuState::WarningKind::None) {
            boxes.warn_recover = ui_->get_element_box(kWarnRecoverId);
            boxes.warn_cancel = ui_->get_element_box(kWarnCancelId);
        } else if (state_.confirming_delete) {
            for (int i = 0; i < 2; ++i) {
                boxes.delete_confirm[static_cast<std::size_t>(i)] =
                    ui_->get_element_box(kDeleteConfirmId[i]);
            }
        } else if (state_.confirming_overwrite) {
            for (int i = 0; i < 2; ++i) {
                boxes.overwrite_confirm[static_cast<std::size_t>(i)] =
                    ui_->get_element_box(kOverwriteConfirmId[i]);
            }
        } else {
            boxes.back = ui_->get_element_box(kBackId);
            for (int i = 0; i < gus::domain::save::kSlotCount; ++i) {
                if (!state_.slots[static_cast<std::size_t>(i)].occupied) continue;
                boxes.delete_icons[static_cast<std::size_t>(i)] =
                    ui_->get_element_box(delete_item_id(i).c_str());
            }
            for (int i = 0; i < gus::domain::save::kSlotCount; ++i) {
                boxes.slots[static_cast<std::size_t>(i)] =
                    ui_->get_element_box(slot_item_id(i).c_str());
            }
        }
        return boxes;
    }

    SDL_Window* window_;
    gus::platform::audio::AudioEngine& audio_;
    const gus::app::i18n::Translator& translator_;
    SaveLoadMode mode_;
    const std::string& saves_dir_;
    const std::function<gus::domain::save::SaveData()>& build_current_save_data_;
    const std::function<void(const gus::domain::save::SaveData&)>& apply_loaded_save_data_;
    std::string frozen_background_png_;

    SaveLoadMenuState state_;
    std::array<std::optional<gus::domain::save::SaveData>, gus::domain::save::kSlotCount>
        loaded_cache_{};

    int pw_ = 0;
    int ph_ = 0;
    float dp_ratio_ = 1.0f;

    bool window_closed_ = false;
    bool bailed_ = false;  // ui_->ok()==false OU o modo screenshot rodou (ver enter())
    bool done_ = false;    // BackToPause/ClosedAfterLoad decidido por uma acao real

    // std::optional (nao um objeto direto): enter()/exit() controlam o ciclo
    // de vida (EXCLUSIVIDADE DO UILAYER, ver gus/app/screen_state.hpp).
    std::optional<glintfx::UiLayer> ui_;
    std::optional<gus::platform::render2d::Render2dGl3> backdrop_;

    std::string stage_;
    std::string rml_path_;
    gus::platform::render2d::TextureId frozen_bg_tex_ =
        gus::platform::render2d::kInvalidTexture;

    gus::platform::audio::SoundId hover_sfx_id_ = gus::platform::audio::kInvalidSound;
    gus::platform::audio::SoundId click_sfx_id_ = gus::platform::audio::kInvalidSound;
    std::string last_hover_sfx_id_;

    // Default BackToPause (MESMO fallback do while(true) antigo: qualquer
    // saida que nao seja um Back/ClosedAfterLoad explicito por acao real
    // degrada pra BackToPause - ui_->ok()==false e o modo screenshot tambem
    // setam isto explicitamente).
    SaveLoadLoopExit result_ = SaveLoadLoopExit::BackToPause;
};

}  // namespace

SaveLoadLoopExit run_save_load_menu_loop_gl_current(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator, SaveLoadMode mode,
    const std::string& saves_dir,
    const std::function<gus::domain::save::SaveData()>& build_current_save_data,
    const std::function<void(const gus::domain::save::SaveData&)>&
        apply_loaded_save_data,
    const std::string& frozen_background_png, const gus::app::EventSyncHook& sync_hook) {
    SaveLoadScreen screen(window, audio, translator, mode, saves_dir, build_current_save_data,
                          apply_loaded_save_data, frozen_background_png);
    gus::app::run_screen_state(screen, sync_hook);
    if (screen.window_closed()) return SaveLoadLoopExit::QuitApp;
    return screen.result();
}

}  // namespace gus::app::screens
