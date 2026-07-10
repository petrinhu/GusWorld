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

#include "gus/app/screens/save_load_menu_loop.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
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
bool hit_test(const glintfx::ElementBox& box, float x, float y) {
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

    std::string rml = build_save_load_menu_rml(state, tr);
    const std::string needle = "<style>\n";
    const std::size_t pos = rml.find(needle);
    if (pos != std::string::npos) {
        rml.insert(pos + needle.size(),
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "src: \"PixelOperatorMono.ttf\"; }\n"
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "font-weight: bold; src: \"PixelOperatorMono-Bold.ttf\"; }\n");
    }

    const fs::path out = stage / "save_load_menu.rml";
    std::ofstream f(out);
    f << rml;
    return out.string();
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
// slot) - "Tentar recuperar" e tratado em handle_action abaixo via
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

}  // namespace

SaveLoadLoopExit run_save_load_menu_loop_gl_current(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator, SaveLoadMode mode,
    const std::string& saves_dir,
    const std::function<gus::domain::save::SaveData()>& build_current_save_data,
    const std::function<void(const gus::domain::save::SaveData&)>&
        apply_loaded_save_data,
    const std::string& frozen_background_png) {
    std::array<std::optional<gus::domain::save::SaveData>, gus::domain::save::kSlotCount>
        loaded_cache{};
    const std::array<SaveSlotPreview, gus::domain::save::kSlotCount> previews =
        build_previews_and_cache(saves_dir, loaded_cache);

    SaveLoadMenuState state;
    save_load_menu_open(state, mode, previews);

    int pw = 0, ph = 0;
    SDL_GetWindowSizeInPixels(window, &pw, &ph);
    if (pw < 1) pw = 1;
    if (ph < 1) ph = 1;
    const float dp_ratio = static_cast<float>(pw) / 960.0f;

    glintfx::UiLayer ui(glintfx::UiLayer::Config{/*logical_width=*/960,
                                                  /*logical_height=*/540,
                                                  /*load_gl=*/true,
                                                  /*dp_ratio=*/dp_ratio});
    if (!ui.ok()) {
        std::cerr << "SaveLoadMenuLoop: glintfx::UiLayer::ok()=false (attach "
                     "falhou) - fechando sem desenhar (degradacao segura).\n";
        return SaveLoadLoopExit::BackToPause;
    }

    const std::string stage = save_load_stage_dir();
    ui.set_asset_base_url(stage.c_str());
    std::string rml_path = write_save_load_rml_file(state, translator);
    ui.load(rml_path.c_str());
    ui.set_viewport(pw, ph);
    ui.set_dp_ratio(dp_ratio);
    // SFX-MIGRATE-V0.9: 1 update() de "assentamento" AQUI, ANTES do while(true) -
    // achado EMPIRICO (harness headless, save_load_menu_interaction_test.cpp): o
    // hover NATIVO (Context::ProcessMouseMove -> UpdateHoverChain -> GetElementAtPoint,
    // fonte pinada do RmlUi) so resolve elemento sob o cursor DEPOIS de pelo menos 1
    // Context::Update() ter rodado pro documento RECEM-carregado (diferente de
    // get_element_box, que le a geometria do elemento direto e ja funciona logo
    // apos load()). Sem isto, um MouseMove que chegue ANTES do 1o present_frame()
    // desta tela (raro mas real: o jogador podia estar com o mouse em movimento no
    // EXATO frame em que a tela abriu) cairia num hover_cb mudo ate o PROXIMO
    // MouseMove. Idempotente/barato (present_frame() ja chama ui.update() de novo a
    // cada frame normalmente).
    ui.update();

    gus::platform::render2d::Render2dGl3 backdrop(/*gl_active=*/true);
    const gus::platform::render2d::TextureId frozen_bg_tex =
        frozen_background_png.empty()
            ? gus::platform::render2d::kInvalidTexture
            : backdrop.load_texture(frozen_background_png.c_str());

    // SFX de hover/clique (paridade com system_menu_loop.cpp/title_menu_loop.cpp -
    // "todo menu de botoes soa"): load_sfx UMA VEZ por sessao desta tela (MESMO
    // padrao "load_sfx NUNCA no frame"). audio.available()==false (device
    // indisponivel/CI) degrada com seguranca (play_sfx(id invalido) e no-op).
    const std::string hover_sfx_path =
        resolve_menu_sfx_path(gus::core::assets::kMenuHoverSfxFile);
    const std::string click_sfx_path =
        resolve_menu_sfx_path(gus::core::assets::kMenuClickSfxFile);
    const gus::platform::audio::SoundId hover_sfx_id = audio.load_sfx(hover_sfx_path.c_str());
    const gus::platform::audio::SoundId click_sfx_id = audio.load_sfx(click_sfx_path.c_str());

    auto reload = [&] {
        rml_path = write_save_load_rml_file(state, translator);
        ui.load(rml_path.c_str());
        ui.set_viewport(pw, ph);
        ui.set_dp_ratio(dp_ratio);
    };

    auto present_frame = [&] {
        const gus::core::spatial::Rect cam{0.0f, 0.0f, static_cast<float>(pw),
                                            static_cast<float>(ph)};
        backdrop.begin_frame(cam, pw, ph);
        if (frozen_bg_tex != gus::platform::render2d::kInvalidTexture) {
            backdrop.draw_textured_rect(
                cam, frozen_bg_tex, gus::platform::render2d::UvRect{0.0f, 0.0f, 1.0f, 1.0f},
                gus::platform::render2d::DrawColor{1.0f, 1.0f, 1.0f, 1.0f});
        }
        backdrop.end_frame();
        ui.update();
        ui.render();
        SDL_GL_SwapWindow(window);
    };

    // Confirma um slot em modo SAVE: pede o SaveData VIVO ao CHAMADOR (timestamp
    // fresco), grava de fato, e ATUALIZA o preview do slot NA HORA (sem fechar a
    // tela - o jogador ve o novo timestamp/playtime imediatamente).
    auto do_save = [&](int slot) {
        if (!build_current_save_data) return;  // defensivo: chamador nao forneceu
        gus::domain::save::SaveData data = build_current_save_data();
        data.slot_id = slot;
        const bool ok = gus::platform::fs::save_game(data, slot, saves_dir);
        if (!ok) {
            std::cerr << "[save_load_menu_loop] falha ao gravar slot " << slot
                      << " (I/O - disco cheio/permissao?) - estado em memoria "
                         "intocado, nada persistiu.\n";
            return;
        }
        state.slots[static_cast<std::size_t>(slot)] = build_slot_preview(data, slot);
        loaded_cache[static_cast<std::size_t>(slot)] = data;
    };

    // Apaga de fato o slot (feature "Apagar", aprovada pelo lider): I/O real via
    // gus::platform::fs::delete_save (primario + cadeia INTEIRA de backup), e
    // ATUALIZA o preview local NA HORA (o slot vira vazio sem fechar a tela,
    // MESMO padrao "efeito imediato" de do_save acima) + re-ancora a selecao se
    // o slot apagado era o focado e deixou de ser selecionavel (ver
    // save_load_menu_reselect_if_needed, ex.: apagar o ULTIMO slot ocupado em
    // modo Load).
    auto do_delete = [&](int slot) {
        const bool ok = gus::platform::fs::delete_save(slot, saves_dir);
        if (!ok) {
            std::cerr << "[save_load_menu_loop] falha ao apagar slot " << slot
                      << " (I/O - permissao negada?) - preview local NAO "
                         "atualizado (o arquivo pode continuar em disco).\n";
            return;
        }
        state.slots[static_cast<std::size_t>(slot)] = empty_slot_preview(slot);
        loaded_cache[static_cast<std::size_t>(slot)].reset();
        save_load_menu_reselect_if_needed(state);
    };

    // SAVE-LOAD-AVISOS: "Tentar recuperar" do aviso Damaged - tenta a cadeia de
    // backup de fato (gus::platform::fs::load_game_from_backup, JA prova a 1a
    // geracao Ok). Sucesso: MESMO caminho de um Load normal bem-sucedido
    // (apply_loaded_save_data + fecha a tela, ClosedAfterLoad) - o jogador nao
    // precisa saber que veio do backup, so que o jogo carregou. Falha: NENHUMA
    // geracao de backup era Ok - transita state.warning_kind pra RecoverFailed
    // (mensagem "nao foi possivel recuperar", so Cancelar) e MANTEM a tela
    // aberta (reload, nao fecha) - devolve nullopt igual aos outros ramos que
    // "so tratam e continuam".
    auto do_recover = [&](int slot) -> std::optional<SaveLoadLoopExit> {
        const auto recovered = gus::platform::fs::load_game_from_backup(slot, saves_dir);
        if (recovered.has_value() && recovered->result == gus::domain::save::LoadResult::Ok &&
            apply_loaded_save_data) {
            apply_loaded_save_data(recovered->data);
            return SaveLoadLoopExit::ClosedAfterLoad;
        }
        std::cerr << "[save_load_menu_loop] recuperacao do slot " << slot
                  << " falhou (nenhuma geracao de backup carregou Ok) - abrindo "
                     "o aviso RecoverFailed.\n";
        state.warning_kind = SaveLoadMenuState::WarningKind::RecoverFailed;
        state.warning_selected = 1;
        reload();
        return std::nullopt;
    };

    // Roteia UMA SaveLoadMenuAction (vinda do teclado OU de um clique de mouse)
    // pro MESMO efeito de mundo - compartilhado pelos dois canais de entrada pra
    // nao duplicar do_save/do_delete/reload (MESMO racional de handle_action em
    // system_menu_loop.cpp). Devolve o SaveLoadLoopExit se o CHAMADOR deve
    // retornar NA HORA; nullopt = ja tratou (reload incluso) e o loop continua.
    auto handle_action =
        [&](SaveLoadMenuAction action) -> std::optional<SaveLoadLoopExit> {
        switch (action) {
            case SaveLoadMenuAction::None:
                reload();
                return std::nullopt;
            case SaveLoadMenuAction::Back:
                return SaveLoadLoopExit::BackToPause;
            case SaveLoadMenuAction::SlotChosen:
                if (mode == SaveLoadMode::Save) {
                    do_save(state.selected);
                    reload();
                    return std::nullopt;
                }
                {
                    const auto& cached =
                        loaded_cache[static_cast<std::size_t>(state.selected)];
                    if (cached.has_value() && apply_loaded_save_data) {
                        apply_loaded_save_data(*cached);
                        return SaveLoadLoopExit::ClosedAfterLoad;
                    }
                    // Defensivo: slot selecionavel em Load SEMPRE tem cache (ver
                    // build_previews_and_cache) - se nao tiver, no-op seguro
                    // (fica na lista) em vez de fingir um load.
                    std::cerr << "[save_load_menu_loop] BUG defensivo: slot "
                              << state.selected
                              << " selecionavel em Load sem cache - ignorando "
                                 "(nao finge um load).\n";
                    reload();
                    return std::nullopt;
                }
            case SaveLoadMenuAction::OverwriteConfirmed:
                do_save(state.selected);
                reload();
                return std::nullopt;
            case SaveLoadMenuAction::OverwriteCancelled:
                reload();
                return std::nullopt;
            case SaveLoadMenuAction::DeleteConfirmed:
                do_delete(state.delete_target_slot);
                reload();
                return std::nullopt;
            case SaveLoadMenuAction::DeleteCancelled:
                reload();
                return std::nullopt;
            case SaveLoadMenuAction::RecoverRequested:
                return do_recover(state.selected);
            case SaveLoadMenuAction::WarningCancelled:
                reload();
                return std::nullopt;
        }
        reload();  // defensivo (enum exaustivo acima - nunca deveria cair aqui)
        return std::nullopt;
    };

    // SOM DE HOVER (mouse) - SFX-MIGRATE-V0.9: hover_cb e o callback NATIVO
    // (glintfx::UiLayer::set_hover_callback, v0.9.0) - a glintfx despacha
    // entered=true/false JA deduplicado por id (current_hover_id_ interno, ver o
    // doc-comment vendorizado em ui_layer.hpp/bootstrap.hpp: so invoca o
    // callback quando o id hovered de fato MUDA). `last_hover_sfx_id` e uma 2a
    // camada de dedup NOSSA (defesa em profundidade, redundante mas barata com
    // a da glintfx) - sincronizada nos DOIS sentidos (entered=false TAMBEM
    // atualiza, senao sair-e-voltar pro MESMO item nunca redispararia). `id`
    // (const char*) so e valido DURANTE esta chamada (contrato do glintfx) -
    // convertido pra std::string ANTES de qualquer outra coisa.
    // is_navigable_hover_id() filtra os containers que o hover nativo TAMBEM
    // resolve mas nunca devem soar (ex.: `.slot-list`). Registrado UMA VEZ (ao
    // contrario de system_menu_loop.cpp, `ui` aqui e uma UNICA glintfx::UiLayer
    // pra vida inteira da tela - nunca resetada/recriada).
    std::string last_hover_sfx_id;
    auto hover_cb = [&](const char* raw_id, bool entered) {
        const std::string id = raw_id != nullptr ? raw_id : "";
        if (!entered) {
            if (id == last_hover_sfx_id) last_hover_sfx_id.clear();
            return;
        }
        if (id == last_hover_sfx_id || !is_navigable_hover_id(state, id)) return;
        last_hover_sfx_id = id;
        audio.play_sfx(hover_sfx_id);
    };
    ui.set_hover_callback(hover_cb);

    // HOVER (mouse) - MESMO pipeline de system_menu_loop.cpp::handle_mouse_motion:
    // injeta MouseMove no glintfx (visual :hover NATIVO, e o QUE dispara hover_cb
    // acima por baixo dos panos). O SOM de hover agora e 100% responsabilidade do
    // callback nativo - esta lambda so injeta o evento.
    auto handle_mouse_motion = [&](float mx, float my) {
        glintfx::UiEvent hover_ev{};
        hover_ev.type = glintfx::UiEvent::Type::MouseMove;
        hover_ev.x = mx;
        hover_ev.y = my;
        ui.process_event(hover_ev);
    };

    // DIAGNOSTICO/PROVA (SAVE-LOAD-UI etapa 6, prova visual headless Xvfb :99):
    // GUSWORLD_SAVELOAD_SCREENSHOT_DIR=<dir> assenta alguns frames (bake de
    // fonte/layout - MESMA cautela de todos os outros self-tests com captura
    // desta base de codigo, ex. HOVER-SELFTEST em battle_preview.cpp) e salva 1
    // PNG (save_load_save.png OU save_load_load.png, conforme `mode`) ANTES de
    // entrar no loop interativo - bypassa por completo (nunca abre pra input
    // real, MESMO espirito de GUSWORLD_SYSMENU_HOVER_SELFTEST em
    // system_menu_loop.cpp).
    const char* screenshot_dir = std::getenv("GUSWORLD_SAVELOAD_SCREENSHOT_DIR");
    if (screenshot_dir != nullptr && screenshot_dir[0] != '\0') {
        for (int i = 0; i < 6; ++i) present_frame();
        std::vector<unsigned char> buf(static_cast<std::size_t>(pw) *
                                        static_cast<std::size_t>(ph) * 4);
        if (gus::platform::rmlui::gl3_read_backbuffer_rgba(pw, ph, buf.data())) {
            const std::string suffix = (mode == SaveLoadMode::Save) ? "save" : "load";
            const std::string out =
                join(std::string(screenshot_dir), "save_load_" + suffix + ".png");
            stbi_write_png(out.c_str(), pw, ph, 4, buf.data(), pw * 4);
            std::cout << "SaveLoadMenuLoop: [screenshot] " << out << " (" << pw << "x"
                      << ph << ")\n";
        } else {
            std::cerr << "SaveLoadMenuLoop: [screenshot] gl3_read_backbuffer_rgba "
                         "falhou\n";
        }
        return SaveLoadLoopExit::BackToPause;
    }

    while (true) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) {
                return SaveLoadLoopExit::QuitApp;
            }
            if (ev.type == SDL_EVENT_WINDOW_RESIZED ||
                ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                SDL_GetWindowSizeInPixels(window, &pw, &ph);
                if (pw < 1) pw = 1;
                if (ph < 1) ph = 1;
                ui.set_viewport(pw, ph);
                ui.set_dp_ratio(static_cast<float>(pw) / 960.0f);
                continue;
            }
            if (ev.type == SDL_EVENT_KEY_DOWN && !ev.key.repeat) {
                if (const auto exit = handle_action(save_load_menu_key_down(state, ev.key.key))) {
                    return *exit;
                }
            } else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                       ev.button.button == SDL_BUTTON_LEFT) {
                bool handled = false;
                // O icone de apagar (unico alvo que precisa tocar o click_sfx
                // ANTES do handle_action generico abaixo, ver o comentario dele)
                // marca este flag pra o trailer nao tocar 2x.
                bool played_click_sfx = false;
                if (state.warning_kind != SaveLoadMenuState::WarningKind::None) {
                    // "Tentar recuperar" (so existe quando Damaged) - checado
                    // ANTES do Cancelar, MESMO padrao "mais especifico vence" do
                    // icone de apagar abaixo (aqui nao ha sobreposicao real, mas
                    // mantem a ordem consistente).
                    if (hit_test(ui.get_element_box(kWarnRecoverId), ev.button.x, ev.button.y)) {
                        handled = true;
                        if (const auto exit =
                                handle_action(save_load_menu_click_warning_recover(state))) {
                            return *exit;
                        }
                    } else if (hit_test(ui.get_element_box(kWarnCancelId), ev.button.x,
                                         ev.button.y)) {
                        handled = true;
                        if (const auto exit =
                                handle_action(save_load_menu_click_warning_cancel(state))) {
                            return *exit;
                        }
                    }
                } else if (state.confirming_delete) {
                    for (int i = 0; i < 2 && !handled; ++i) {
                        if (!hit_test(ui.get_element_box(kDeleteConfirmId[i]), ev.button.x,
                                       ev.button.y)) {
                            continue;
                        }
                        handled = true;
                        if (const auto exit = handle_action(
                                save_load_menu_click_delete_confirm(state, i))) {
                            return *exit;
                        }
                    }
                } else if (state.confirming_overwrite) {
                    for (int i = 0; i < 2 && !handled; ++i) {
                        if (!hit_test(ui.get_element_box(kOverwriteConfirmId[i]), ev.button.x,
                                       ev.button.y)) {
                            continue;
                        }
                        handled = true;
                        if (const auto exit = handle_action(
                                save_load_menu_click_overwrite_confirm(state, i))) {
                            return *exit;
                        }
                    }
                } else {
                    // Voltar (bug 1/6/9: antes desta onda, so o teclado fechava a
                    // tela) - id fixo, SEMPRE presente na lista normal.
                    if (hit_test(ui.get_element_box(kBackId), ev.button.x, ev.button.y)) {
                        handled = true;
                        if (const auto exit = handle_action(SaveLoadMenuAction::Back)) {
                            return *exit;
                        }
                    }
                    // Icone de apagar por-linha (feature "Apagar") - checado ANTES
                    // dos slots: a caixa do icone fica DENTRO da linha do slot (ver
                    // save_load_menu_rml.cpp), o mais especifico vence.
                    for (int i = 0; i < gus::domain::save::kSlotCount && !handled; ++i) {
                        if (!state.slots[static_cast<std::size_t>(i)].occupied) continue;
                        if (!hit_test(ui.get_element_box(delete_item_id(i).c_str()),
                                      ev.button.x, ev.button.y)) {
                            continue;
                        }
                        handled = true;
                        played_click_sfx = true;
                        save_load_menu_request_delete(state, i);
                        audio.play_sfx(click_sfx_id);
                        reload();
                    }
                    // Clicar num slot (bug 3: antes desta onda, nao fazia nada) -
                    // "focar + Enter" (MESMA convencao de system_menu_click_option).
                    for (int i = 0; i < gus::domain::save::kSlotCount && !handled; ++i) {
                        if (!hit_test(ui.get_element_box(slot_item_id(i).c_str()),
                                      ev.button.x, ev.button.y)) {
                            continue;
                        }
                        handled = true;
                        if (const auto exit =
                                handle_action(save_load_menu_click_slot(state, i))) {
                            return *exit;
                        }
                    }
                }
                // SOM DE CLIQUE (slot/Voltar/pill-de-dialogo): qualquer clique que
                // ACERTOU um alvo real soa - MESMO espirito de "todo menu de
                // botoes soa"; SIMPLIFICACAO deliberada vs. system_menu_loop.cpp
                // (que so toca em acoes "confirming" por causa do caso do
                // drag-de-slider, que nao existe nesta tela) - clicar numa linha
                // readonly/ja-vazia (que devolve None) ainda soa um clique
                // "reconhecido", aceitavel aqui. O icone de apagar ja tocou o seu
                // proprio click_sfx acima (played_click_sfx evita duplicar).
                if (handled && !played_click_sfx) audio.play_sfx(click_sfx_id);
            } else if (ev.type == SDL_EVENT_MOUSE_MOTION) {
                handle_mouse_motion(ev.motion.x, ev.motion.y);
            } else if (ev.type == SDL_EVENT_MOUSE_WHEEL) {
                // WHEEL FORWARDING (`.slot-list`) - MESMA receita de
                // system_menu_loop.cpp: 1 MouseMove sintetico pra posicao ATUAL do
                // cursor IMEDIATAMENTE antes do MouseWheel (rola o elemento em
                // HOVER, nao o focado - gotcha do glintfx v0.4.0, ver o header).
                float mouse_x = 0.0f, mouse_y = 0.0f;
                SDL_GetMouseState(&mouse_x, &mouse_y);
                handle_mouse_motion(mouse_x, mouse_y);

                const float wheel_dy = system_menu_wheel_delta_to_rmlui(
                    ev.wheel.y, ev.wheel.direction == SDL_MOUSEWHEEL_FLIPPED);
                glintfx::UiEvent wheel_ev{};
                wheel_ev.type = glintfx::UiEvent::Type::MouseWheel;
                wheel_ev.x = 0.0f;
                wheel_ev.y = wheel_dy;
                ui.process_event(wheel_ev);
            }
        }
        present_frame();
    }
}

}  // namespace gus::app::screens
