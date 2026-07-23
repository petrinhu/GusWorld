// gus/app/src/screens/title_menu_loop.cpp
//
// Implementacao do loop interativo da TELA DE TITULO. Ver header para o
// contrato completo. GL/glintfx-heavy (mesma familia de save_load_menu_loop.cpp/
// system_menu_loop.cpp) - sem unidade de teste direta (a logica PURA testavel ja
// fica em title_menu.hpp/title_menu_test.cpp e title_menu_rml.hpp/
// title_menu_rml_test.cpp; este .cpp so orquestra SDL/GL + o I/O de disco em
// torno delas).

#include "gus/app/screens/title_menu_loop.hpp"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

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
#include "gus/platform/render2d/render2d_gl3.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"  // glad load + gl3_read_backbuffer_rgba

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

    std::string rml = build_title_menu_rml(state, tr, pressed_index);
    const std::string needle = "<style>\n";
    const std::size_t pos = rml.find(needle);
    if (pos != std::string::npos) {
        rml.insert(pos + needle.size(),
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "src: \"PixelOperatorMono.ttf\"; }\n"
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "font-weight: bold; src: \"PixelOperatorMono-Bold.ttf\"; }\n");
    }

    const fs::path out = stage / "title_menu.rml";
    std::ofstream f(out);
    f << rml;
    return out.string();
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

// FLASH-CTX (A2, extracao behavior-preserving): NUCLEO que ASSUME um contexto
// GL JA CORRENTE + glad JA CARREGADO - mesmo corpo que antes vivia numa lambda
// `run_body` dentro de run_title_menu_loop_owning_gl (ver o header pro
// contrato). Extraida verbatim (zero mudanca de logica) pra permitir reuso
// futuro sem recriar o contexto (Opcao C do plano de contexto GL unico).
void run_title_menu_loop_gl_current(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator, const std::string& saves_dir,
    TitleLoopExit* out_exit, gus::domain::save::SaveData* out_loaded_save,
    gus::domain::save::DifficultyLevel* out_new_game_difficulty,
    const std::string& frozen_background_png) {
    {
        // DIAGNOSTICO/PROVA (MODOS-MORTE Fase 0, prova visual headless Xvfb :99):
        // GUSWORLD_DIFFICULTY_SCREENSHOT_DIR=<dir> pula a tela de titulo por
        // completo e abre DIRETO a tela de selecao de dificuldade (ANINHADA no
        // MESMO contexto GL que este owning_gl acabou de criar) - MESMO espirito
        // de GUSWORLD_TITLE_SCREENSHOT_DIR abaixo, so que mirando a tela seguinte
        // do fluxo de Novo Jogo. O proprio run_difficulty_menu_loop_gl_current
        // detecta a mesma variavel e salva o PNG (difficulty_menu.png) antes de
        // entrar no loop interativo - aqui so evitamos montar a tela de titulo
        // (que nao seria exercitada de qualquer forma).
        if (const char* diff_screenshot_dir =
                std::getenv("GUSWORLD_DIFFICULTY_SCREENSHOT_DIR");
            diff_screenshot_dir != nullptr && diff_screenshot_dir[0] != '\0') {
            gus::domain::save::DifficultyLevel dummy =
                gus::domain::save::DifficultyLevel::Medio;
            (void)run_difficulty_menu_loop_gl_current(window, audio, translator,
                                                       &dummy, frozen_background_png);
            *out_exit = TitleLoopExit::NewGame;
            return;
        }

        const TitleDiskScan scan = scan_saves(saves_dir);

        TitleMenuState state;
        title_menu_open(state, scan.any_save_exists);

        int pw = 0, ph = 0;
        SDL_GetWindowSizeInPixels(window, &pw, &ph);
        if (pw < 1) pw = 1;
        if (ph < 1) ph = 1;
        const float dp_ratio = static_cast<float>(pw) / 960.0f;

        // std::optional (nao um objeto direto): MODOS-MORTE Fase 0 precisa
        // DESTRUIR esta UiLayer ANTES de abrir a tela de dificuldade ANINHADA
        // (RmlUi so aceita 1 instancia viva no processo, ver o comentario
        // "FIX CRITICO" em system_menu_loop.cpp - MESMA causa raiz) e RECRIAR
        // se o jogador Cancelar e voltar pra esta lista.
        std::optional<glintfx::UiLayer> ui_opt;
        ui_opt.emplace(glintfx::UiLayer::Config{/*logical_width=*/960,
                                                 /*logical_height=*/540,
                                                 /*load_gl=*/true,
                                                 /*dp_ratio=*/dp_ratio});
        if (!ui_opt->ok()) {
            std::cerr << "TitleMenuLoop: glintfx::UiLayer::ok()=false (attach "
                         "falhou) - fechando sem desenhar (degradacao segura, "
                         "comeca fresco).\n";
            *out_exit = TitleLoopExit::NewGame;
            return;
        }

        const std::string stage = title_stage_dir();
        ui_opt->set_asset_base_url(stage.c_str());
        std::string rml_path = write_title_rml_file(state, translator);
        ui_opt->load(rml_path.c_str());
        ui_opt->set_viewport(pw, ph);
        ui_opt->set_dp_ratio(dp_ratio);
        // SFX-MIGRATE-V0.9: 1 update() de "assentamento" AQUI, ANTES do
        // while(true) - achado EMPIRICO (harness headless de
        // save_load_menu_loop.cpp, MESMA receita replicada aqui): o hover
        // NATIVO (Context::ProcessMouseMove -> UpdateHoverChain ->
        // GetElementAtPoint, fonte pinada do RmlUi) so resolve elemento sob o
        // cursor DEPOIS de pelo menos 1 Context::Update() ter rodado pro
        // documento RECEM-carregado. Sem isto, um MouseMove que chegue ANTES
        // do 1o present_frame() desta tela cairia num hover_cb mudo ate o
        // PROXIMO MouseMove. Idempotente/barato (present_frame() ja chama
        // ui.update() de novo a cada frame).
        ui_opt->update();

        gus::platform::render2d::Render2dGl3 backdrop(/*gl_active=*/true);

        // M7-FB3 (MENU-INICIAL-FUNDO): fundo VIVO da LISTA (Continuar/Novo Jogo/
        // Sair) - playtest do Gus Dragon ("menu inicial tem arte/animacao PROPRIA
        // por tras, nao a tela de onde o jogador estava") + decisao do lider: o
        // MESMO monitor CRT do boot pixelizado (ja aprovado pela transicao cidade<-
        // >batalha, M7-COSTURA Inc 2c), agora "assentado" (nao uma transicao com
        // duracao fixa - ver gus/core/anim/boot_pixel_sequence.hpp::
        // boot_pixel_idle_frame_index). `frozen_background_png` (cidade congelada)
        // NAO e mais usado AQUI - segue existindo so pra tela de dificuldade
        // aninhada (start_new_game_via_difficulty_menu abaixo, fora do escopo deste
        // feedback). load() JA loga no stderr se algum frame faltar (asset ausente/
        // headless) - draw_idle() degrada sozinho pro solido gunmetal nesse caso
        // (regra do projeto: todo efeito, bom ou ruim, loga - a falha e logada UMA
        // vez aqui, nao a cada frame desenhado).
        gus::app::BootPixelOverlay boot_bg;
        boot_bg.load(backdrop, gus::platform::assets::FilesystemAssetSource().resolve_path(
                                    gus::core::assets::kVfxBootPixelDir));
        const unsigned long long boot_bg_start_ns = SDL_GetTicksNS();

        // COCKPIT-SFX-HOVER-CLIQUE: SFX de hover/clique - load_sfx UMA VEZ por
        // sessao desta tela (MESMA cautela de "load_sfx NUNCA no frame" ja
        // documentada em system_menu_loop.cpp/battle_preview.cpp). MESMOS
        // arquivos do menu de pausa (kMenuHoverSfxFile/kMenuClickSfxFile) -
        // paridade sonora entre os menus de botoes. audio.available()==false
        // (device indisponivel/CI) degrada com seguranca (play_sfx com id
        // invalido ja e no-op, ver AudioEngine::play_sfx).
        const std::string hover_sfx_path =
            resolve_menu_sfx_path(gus::core::assets::kMenuHoverSfxFile);
        const std::string click_sfx_path =
            resolve_menu_sfx_path(gus::core::assets::kMenuClickSfxFile);
        const gus::platform::audio::SoundId hover_sfx_id =
            audio.load_sfx(hover_sfx_path.c_str());
        const gus::platform::audio::SoundId click_sfx_id =
            audio.load_sfx(click_sfx_path.c_str());
        auto reload = [&] {
            rml_path = write_title_rml_file(state, translator);
            ui_opt->load(rml_path.c_str());
            ui_opt->set_viewport(pw, ph);
            ui_opt->set_dp_ratio(dp_ratio);
            // SFX-MIGRATE-V0.9: MESMO update() de assentamento da construcao
            // inicial acima (ver o comentario la) - QUALQUER reload() troca de
            // DOCUMENTO, entao o hover NATIVO precisa do mesmo "layout
            // assentado" de novo a cada troca, nao so na primeira.
            ui_opt->update();
        };

        auto present_frame = [&] {
            const gus::core::spatial::Rect cam{0.0f, 0.0f, static_cast<float>(pw),
                                                static_cast<float>(ph)};
            backdrop.begin_frame(cam, pw, ph);
            // M7-FB3: fundo VIVO - indice calculado do tempo REAL corrido desde que
            // a tela abriu (SDL_GetTicksNS, MESMO padrao de elapsed de
            // battle_preview.cpp/sdl_window.cpp), ciclando devagar pelos ULTIMOS 3
            // frames do boot (ver o comentario de boot_pixel_idle_frame_index).
            const float boot_bg_elapsed_s =
                static_cast<float>(SDL_GetTicksNS() - boot_bg_start_ns) / 1.0e9f;
            const int boot_bg_frame = gus::core::anim::boot_pixel_idle_frame_index(
                boot_bg_elapsed_s, gus::core::anim::kBootPixelFrameCount);
            boot_bg.draw_idle(backdrop, cam, boot_bg_frame);
            backdrop.end_frame();
            ui_opt->update();
            ui_opt->render();
            SDL_GL_SwapWindow(window);
        };

        // SOM DE HOVER (mouse) - SFX-MIGRATE-V0.9: hover_cb e o callback NATIVO
        // (glintfx::UiLayer::set_hover_callback, v0.9.0) - a glintfx despacha
        // entered=true/false JA deduplicado por id (current_hover_id_ interno,
        // ver o doc-comment vendorizado em ui_layer.hpp/bootstrap.hpp: so invoca
        // o callback quando o id hovered de fato MUDA). `last_hover_sfx_id` e
        // uma 2a camada de dedup NOSSA (defesa em profundidade, redundante mas
        // barata com a da glintfx) - sincronizada nos DOIS sentidos
        // (entered=false TAMBEM atualiza, senao sair-e-voltar pro MESMO item
        // nunca redispararia). `id` (const char*) so e valido DURANTE esta
        // chamada (contrato do glintfx) - convertido pra std::string ANTES de
        // qualquer outra coisa. is_navigable_hover_id() filtra os ids que o
        // hover nativo TAMBEM resolve mas nunca devem soar. Registrado UMA VEZ
        // (`ui` aqui e uma UNICA glintfx::UiLayer pra vida inteira da tela).
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
        ui_opt->set_hover_callback(hover_cb);

        // HOVER (mouse) - COCKPIT-SFX-HOVER-CLIQUE: injeta o MouseMove no glintfx
        // (visual :hover NATIVO, ver title_menu_rml.cpp - e o QUE dispara
        // hover_cb acima por baixo dos panos). O SOM de hover agora e 100%
        // responsabilidade do callback nativo - esta lambda so injeta o evento.
        // Fatorada em lambda pra ser o UNICO choke-point do SDL_EVENT_MOUSE_MOTION
        // real.
        auto handle_mouse_motion = [&](float mx, float my) {
            glintfx::UiEvent hover_ev{};
            hover_ev.type = glintfx::UiEvent::Type::MouseMove;
            hover_ev.x = mx;
            hover_ev.y = my;
            ui_opt->process_event(hover_ev);
        };

        // Confirma "Continuar": o save mais recente JA esta no cache (scan_saves) -
        // nao le o disco de novo. Defensivo (nunca deveria acontecer - Continuar
        // so e selecionavel quando any_save_exists, que implica most_recent_slot
        // >= 0): se por algum motivo o cache estiver vazio, degrada pra NewGame
        // em vez de devolver um SaveData por default-construir (nunca finge um
        // load que nao aconteceu).
        auto confirm_continue = [&] {
            if (scan.most_recent_slot >= 0 &&
                scan.loaded[static_cast<std::size_t>(scan.most_recent_slot)]
                    .has_value()) {
                if (out_loaded_save != nullptr) {
                    *out_loaded_save =
                        *scan.loaded[static_cast<std::size_t>(scan.most_recent_slot)];
                }
                *out_exit = TitleLoopExit::ContinueGame;
            } else {
                std::cerr << "[title_menu_loop] BUG defensivo: ContinueGame sem "
                             "slot no cache - degradando pra Novo Jogo (nao finge "
                             "um load que nao aconteceu).\n";
                *out_exit = TitleLoopExit::NewGame;
            }
        };

        // MODOS-MORTE Fase 0 (docs/design/mecanicas/modos-morte.md §3.2):
        // "Novo Jogo" confirmado dispara a TELA DE SELECAO DE DIFICULDADE,
        // ANINHADA no MESMO contexto GL (MESMA tecnica de
        // run_save_load_menu_loop_gl_current - RmlUi so aceita 1 UiLayer viva por
        // processo, entao a desta tela e DESTRUIDA antes e RECRIADA se o
        // jogador Cancelar). Devolve true se o CHAMADOR (route_title_action) deve
        // retornar de run_body NA HORA.
        auto start_new_game_via_difficulty_menu = [&] {
            ui_opt.reset();
            gus::domain::save::DifficultyLevel chosen =
                gus::domain::save::DifficultyLevel::Medio;
            const DifficultyLoopExit dexit = run_difficulty_menu_loop_gl_current(
                window, audio, translator, &chosen, frozen_background_png);
            switch (dexit) {
                case DifficultyLoopExit::QuitApp:
                    *out_exit = TitleLoopExit::QuitApp;
                    return true;
                case DifficultyLoopExit::Chosen:
                    *out_exit = TitleLoopExit::NewGame;
                    if (out_new_game_difficulty != nullptr) {
                        *out_new_game_difficulty = chosen;
                    }
                    return true;
                case DifficultyLoopExit::Cancelled:
                    // RECRIA a UiLayer do titulo (MESMO padrao de
                    // system_menu_loop.cpp apos voltar de save/load, ver o "FIX
                    // CRITICO" la) e segue mostrando a lista de novo.
                    ui_opt.emplace(glintfx::UiLayer::Config{
                        /*logical_width=*/960, /*logical_height=*/540,
                        /*load_gl=*/true, /*dp_ratio=*/dp_ratio});
                    ui_opt->set_asset_base_url(stage.c_str());
                    ui_opt->set_hover_callback(hover_cb);
                    reload();
                    return false;
            }
            return false;
        };

        // Roteia UMA TitleMenuAction pro efeito de mundo comum aos pontos de
        // entrada (Enter, clique em item) - devolve true se o CHAMADOR deve
        // retornar de run_body NA HORA (ContinueGame/StartNewGame/RequestQuit ja
        // setaram *out_exit ou chamaram confirm_continue()).
        auto route_title_action = [&](TitleMenuAction action) -> bool {
            switch (action) {
                case TitleMenuAction::None:
                    reload();
                    return false;
                case TitleMenuAction::ContinueGame:
                    confirm_continue();
                    return true;
                case TitleMenuAction::StartNewGame:
                    return start_new_game_via_difficulty_menu();
                case TitleMenuAction::RequestQuit:
                    *out_exit = TitleLoopExit::QuitApp;
                    return true;
            }
            return false;
        };

        // DIAGNOSTICO/PROVA (SAVE-LOAD-UI etapa 4, prova visual headless Xvfb
        // :99): GUSWORLD_TITLE_SCREENSHOT_DIR=<dir> assenta alguns frames (bake
        // de fonte/layout, MESMA cautela dos demais self-tests) e salva 1 PNG
        // ANTES de entrar no loop interativo - bypassa por completo (nunca abre
        // pra input real), MESMO espirito de GUSWORLD_SAVELOAD_SCREENSHOT_DIR em
        // save_load_menu_loop.cpp. Nome do arquivo reflete o ESTADO REAL varrido
        // do disco (Continuar habilitado/desabilitado) - rodar o processo 2x com
        // 2 GUSWORLD_HOME diferentes (1 com save, 1 vazio) produz os 2 PNGs
        // pedidos sem precisar renomear nada por fora.
        const char* screenshot_dir = std::getenv("GUSWORLD_TITLE_SCREENSHOT_DIR");
        if (screenshot_dir != nullptr && screenshot_dir[0] != '\0') {
            for (int i = 0; i < 6; ++i) present_frame();
            std::vector<unsigned char> buf(static_cast<std::size_t>(pw) *
                                            static_cast<std::size_t>(ph) * 4);
            if (gus::platform::rmlui::gl3_read_backbuffer_rgba(pw, ph, buf.data())) {
                const std::string suffix = scan.any_save_exists
                                                ? "title_continue_enabled"
                                                : "title_continue_disabled";
                const std::string out =
                    join(std::string(screenshot_dir), suffix + ".png");
                stbi_write_png(out.c_str(), pw, ph, 4, buf.data(), pw * 4);
                std::cout << "TitleMenuLoop: [screenshot] " << out << " (" << pw
                          << "x" << ph << ")\n";
            } else {
                std::cerr << "TitleMenuLoop: [screenshot] gl3_read_backbuffer_rgba "
                             "falhou\n";
            }
            return;
        }

        while (true) {
            SDL_Event ev;
            while (SDL_PollEvent(&ev)) {
                if (ev.type == SDL_EVENT_QUIT) {
                    *out_exit = TitleLoopExit::QuitApp;
                    return;
                }
                if (ev.type == SDL_EVENT_WINDOW_RESIZED ||
                    ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                    SDL_GetWindowSizeInPixels(window, &pw, &ph);
                    if (pw < 1) pw = 1;
                    if (ph < 1) ph = 1;
                    ui_opt->set_viewport(pw, ph);
                    ui_opt->set_dp_ratio(static_cast<float>(pw) / 960.0f);
                    continue;
                }
                if (ev.type == SDL_EVENT_KEY_DOWN && !ev.key.repeat) {
                    const bool is_confirm_key = (ev.key.key == SDLK_RETURN ||
                                                  ev.key.key == SDLK_KP_ENTER ||
                                                  ev.key.key == SDLK_SPACE);
                    if (is_confirm_key) {
                        // SOM DE CLIQUE (COCKPIT-SFX-HOVER-CLIQUE): Enter/Espaco e
                        // sempre uma confirmacao intencional de item/pill - MESMO
                        // choke-point de flash_pressed em system_menu_loop.cpp (a
                        // tela de titulo nao tem flash visual, so o som).
                        audio.play_sfx(click_sfx_id);
                        const TitleMenuAction action =
                            title_menu_key_down(state, ev.key.key);
                        if (route_title_action(action)) return;
                    } else {
                        // Navegacao (setas/WASD/ESC) - SOM DE HOVER PARIDADE
                        // TECLADO x MOUSE (MESMA tecnica de handle_navigation_key
                        // em system_menu_loop.cpp): move a selecao e, SO se o
                        // MODO (lista vs mini-dialogo) NAO mudou E moveu pra um
                        // item NOVO, toca hover_sfx no MESMO choke-point do mouse.
                        // Trocar de MODO (ex.: ESC fechando o mini-dialogo de Novo
                        // Jogo) NAO conta como "hover num item novo" - MESMO guard
                        // de `state.screen == screen_before` do menu de pausa.
                        const bool confirming_before = state.confirming_new_game;
                        const int kb_index_before = title_keyboard_focus_index(state);
                        const TitleMenuAction action =
                            title_menu_key_down(state, ev.key.key);
                        if (state.confirming_new_game == confirming_before) {
                            const int kb_index_after = title_keyboard_focus_index(state);
                            if (ui_hover_entered_new_item(kb_index_before, kb_index_after)) {
                                audio.play_sfx(hover_sfx_id);
                            }
                        }
                        if (route_title_action(action)) return;
                    }
                } else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                           ev.button.button == SDL_BUTTON_LEFT) {
                    bool handled = false;
                    if (state.confirming_new_game) {
                        for (int i = 0; i < 2 && !handled; ++i) {
                            const glintfx::ElementBox box = ui_opt->get_element_box(
                                ("title-confirm-" + std::to_string(i)).c_str());
                            if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                            handled = true;
                            // as 2 pills do mini-dialogo sao SEMPRE validas (sem
                            // conceito de "desabilitada" aqui) - som sempre toca.
                            audio.play_sfx(click_sfx_id);
                            const TitleMenuAction action =
                                title_menu_click_option(state, i);
                            if (route_title_action(action)) return;
                        }
                    } else {
                        for (int i = 0; i < kTitleItemCount && !handled; ++i) {
                            const glintfx::ElementBox box = ui_opt->get_element_box(
                                ("title-item-" + std::to_string(i)).c_str());
                            if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                            handled = true;
                            // Clicar num item DESABILITADO (Continuar sem save) e
                            // no-op TOTAL (ver title_menu_click_option) - sem som
                            // tambem, MESMA semantica "nao reage a nada".
                            if (title_item_selectable(state, i)) {
                                audio.play_sfx(click_sfx_id);
                            }
                            const TitleMenuAction action =
                                title_menu_click_option(state, i);
                            if (route_title_action(action)) return;
                        }
                    }
                } else if (ev.type == SDL_EVENT_MOUSE_MOTION) {
                    handle_mouse_motion(ev.motion.x, ev.motion.y);
                }
            }
            present_frame();
        }
    }
}

bool run_title_menu_loop_owning_gl(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator, const std::string& saves_dir,
    TitleLoopExit* out_exit, gus::domain::save::SaveData* out_loaded_save,
    gus::domain::save::DifficultyLevel* out_new_game_difficulty,
    const std::string& frozen_background_png) {
    if (out_exit != nullptr) *out_exit = TitleLoopExit::QuitApp;

    // MESMA receita de run_system_menu_loop_owning_gl/run_battle_preview_embedded:
    // os atributos GL sao setados a CADA entrada (viabilidade ja provada
    // empiricamente pela troca cidade<->batalha/cidade<->menu).
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GLContext gl = SDL_GL_CreateContext(window);
    if (gl == nullptr) {
        std::cerr << "TitleMenuLoop: SDL_GL_CreateContext falhou: " << SDL_GetError()
                  << "\n";
        return false;
    }
    SDL_GL_MakeCurrent(window, gl);
    SDL_GL_SetSwapInterval(1);

    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        std::cerr << "TitleMenuLoop: falha ao carregar funcoes OpenGL (glad)\n";
        SDL_GL_DestroyContext(gl);
        return false;
    }

    run_title_menu_loop_gl_current(window, audio, translator, saves_dir, out_exit,
                                    out_loaded_save, out_new_game_difficulty,
                                    frozen_background_png);

    SDL_GL_DestroyContext(gl);
    return true;
}

}  // namespace gus::app::screens
