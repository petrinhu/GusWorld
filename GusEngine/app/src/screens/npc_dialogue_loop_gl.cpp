// gus/app/src/screens/npc_dialogue_loop_gl.cpp
//
// Implementacao do loop GL real do dialogo do NPC. Ver header para o contrato
// completo. GL/glintfx-heavy (mesma familia de battle_preview.cpp/
// system_menu_loop.cpp) - sem unidade de teste direta (irredutivel, mesmo
// racional documentado no topo de npc_dialogue_loop.cpp/system_menu_loop.cpp): a
// logica PURA testavel ja fica em npc_dialogue_overlay.hpp/.cpp (input/selecao,
// testada em npc_dialogue_overlay_test.cpp) e npc_dialogue_rml.hpp/.cpp (a string
// RML, testada em npc_dialogue_rml_test.cpp); este .cpp so orquestra SDL/GL/
// glintfx em torno delas, MESMO padrao de run_system_menu_loop_owning_gl.

#include "gus/app/screens/npc_dialogue_loop_gl.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

#include <glintfx/ui_layer.hpp>

#include "gus/app/screens/npc_dialogue_overlay.hpp"
#include "gus/app/screens/npc_dialogue_rml.hpp"
#include "gus/core/asset_paths.hpp"  // kRetratosDir
#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Rect
#include "gus/platform/render2d/render2d_gl3.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"  // glad load (variante owning_gl)

// Pasta das fontes (.ttf), embutida pelo CMake (mesma macro que battle_preview.cpp/
// system_menu_loop.cpp ja usam - PRIVATE no CMakeLists do target app).
#ifndef GUSWORLD_FONTS_DIR
#define GUSWORLD_FONTS_DIR ""
#endif

// Raiz ABSOLUTA de resources/ do repo (mesma macro que sdl_window.cpp/
// battle_preview.cpp resolvem).
#ifndef GUSWORLD_ASSETS_DIR
#define GUSWORLD_ASSETS_DIR ""
#endif

namespace gus::app::screens {

namespace {

namespace fs = std::filesystem;

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

// Resolve a pasta resources/ (env GUSWORLD_ASSETS > macro de compilacao > CWD) -
// MESMA receita de resolve_assets_subdir_local (sdl_window.cpp)/resolve_asset_dir
// (battle_preview.cpp), duplicada aqui pra este arquivo ficar self-contained
// (MESMO padrao dos demais .cpp desta suite).
std::string resolve_assets_subdir(std::string_view rel) {
    const std::string sub(rel);
    if (const char* env = std::getenv("GUSWORLD_ASSETS")) {
        if (env[0] != '\0') return join(env, sub);
    }
    const std::string compiled = GUSWORLD_ASSETS_DIR;
    if (!compiled.empty()) return join(compiled, sub);
    return join("resources", sub);
}

// Diretorio de STAGE do dialogo (tempfile) - MESMA receita/motivo de
// menu_stage_dir (system_menu_loop.cpp)/glintfx_cockpit_stage_dir
// (battle_preview.cpp): o glintfx carrega o doc por PATH e resolve os
// caminhos RELATIVOS (fontes + retrato) contra o DIR do documento - juntar
// doc+assets num unico dir evita o problema de arvores diferentes (fontes em
// GusEngine/assets/, retratos em resources/sprites/icons-m5/retratos/). Nome
// PROPRIO (nao colide com os stages do menu/cockpit).
std::string npc_dialogue_stage_dir() {
    return (fs::temp_directory_path() / "gusworld_glintfx_npcdialogue").string();
}

// Escreve o RML do dialogo (build_npc_dialogue_rml) no stage dir, com o
// @font-face injetado (MESMA receita de write_system_menu_rml_file) e o
// retrato do speaker_id ATUAL copiado flat (MESMA receita de RETRATO-VIVO do
// cockpit, battle_preview.cpp: copy_file pro stage + referencia por nome
// flat). GENERICO: o arquivo-fonte do retrato vem de npc_dialogue_portrait_file
// (speaker_id -> nome; excecoes cadastradas la, ex. bertoldo) - funciona pra
// QUALQUER speaker_id, nao so o Bertoldo. Se o arquivo-fonte nao existir em
// disco (asset ausente/headless), copy_file falha silenciosamente (ec setado,
// ignorado) - o glintfx so nao desenha a imagem (decorator sem textura valida),
// MESMA degradacao segura do resto do app/ (nunca crasha).
std::string write_npc_dialogue_rml_file(
    const gus::domain::dialogue::DialogueNode& node,
    const gus::app::i18n::Translator& tr, int selected_option) {
    const fs::path stage = npc_dialogue_stage_dir();
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

    const std::string portrait_file = npc_dialogue_portrait_file(node.speaker_id);
    const std::string retratos_dir =
        resolve_assets_subdir(gus::core::assets::kRetratosDir);
    fs::copy_file(join(retratos_dir, portrait_file), stage / portrait_file,
                  fs::copy_options::overwrite_existing, ec);

    std::string rml = build_npc_dialogue_rml(node, tr, selected_option, portrait_file);
    const std::string needle = "<style>\n";
    const std::size_t pos = rml.find(needle);
    if (pos != std::string::npos) {
        rml.insert(pos + needle.size(),
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "src: \"PixelOperatorMono.ttf\"; }\n"
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "font-weight: bold; src: \"PixelOperatorMono-Bold.ttf\"; }\n");
    }

    const fs::path out = stage / "npc_dialogue.rml";
    std::ofstream f(out);
    f << rml;
    return out.string();
}

}  // namespace

bool run_npc_dialogue_loop_gl(SDL_Window* window, gus::app::SdlWindow& city,
                               gus::domain::dialogue::DialogueRuntime& runtime,
                               const gus::app::i18n::Translator& translator) {
    // FIX BUG (ver header) - solta qualquer tecla de movimento segurada ANTES de
    // entrar no modal (o jogador nao pode estar "tentando mover" olhando pra uma
    // caixa de dialogo).
    city.clear_input();

    // MESMOS atributos GL de run_system_menu_loop_owning_gl/run_battle_preview_
    // embedded (viabilidade ja provada empiricamente pela troca cidade<->batalha/
    // cidade<->menu-de-pausa).
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GLContext gl = SDL_GL_CreateContext(window);
    if (gl == nullptr) {
        SDL_Log("NpcDialogueLoopGl: SDL_GL_CreateContext falhou: %s", SDL_GetError());
        city.clear_input();
        return false;  // degradacao segura: a Maestro so retoma a cidade
    }
    SDL_GL_MakeCurrent(window, gl);
    SDL_GL_SetSwapInterval(1);

    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        SDL_Log("NpcDialogueLoopGl: falha ao carregar funcoes OpenGL (glad)");
        SDL_GL_DestroyContext(gl);
        city.clear_input();
        return false;
    }

    bool window_closed = false;
    {
        int pw = 0, ph = 0;
        SDL_GetWindowSizeInPixels(window, &pw, &ph);
        if (pw < 1) pw = 1;
        if (ph < 1) ph = 1;
        float dp_ratio = static_cast<float>(pw) / 960.0f;

        glintfx::UiLayer ui(glintfx::UiLayer::Config{/*logical_width=*/960,
                                                      /*logical_height=*/540,
                                                      /*load_gl=*/true,
                                                      /*dp_ratio=*/dp_ratio});
        if (!ui.ok()) {
            SDL_Log(
                "NpcDialogueLoopGl: glintfx::UiLayer::ok()=false (attach falhou) - "
                "encerrando o dialogo sem desenhar nada (degradacao segura).");
        } else {
            const std::string stage = npc_dialogue_stage_dir();
            ui.set_asset_base_url(stage.c_str());

            int selected_option = 0;
            std::string rml_path =
                write_npc_dialogue_rml_file(runtime.current(), translator, selected_option);
            ui.load(rml_path.c_str());
            ui.set_viewport(pw, ph);
            ui.set_dp_ratio(dp_ratio);

            gus::platform::render2d::Render2dGl3 backdrop(/*gl_active=*/true);

            auto reload = [&] {
                rml_path = write_npc_dialogue_rml_file(runtime.current(), translator,
                                                        selected_option);
                ui.load(rml_path.c_str());
                ui.set_viewport(pw, ph);
                ui.set_dp_ratio(dp_ratio);
            };

            auto present_frame = [&] {
                const gus::core::spatial::Rect cam{0.0f, 0.0f, static_cast<float>(pw),
                                                    static_cast<float>(ph)};
                backdrop.begin_frame(cam, pw, ph);  // clear + vinheta radial (fundo
                                                     // abstrato, MESMO padrao do
                                                     // menu de pausa)
                backdrop.end_frame();
                ui.update();
                ui.render();
                SDL_GL_SwapWindow(window);
            };

            present_frame();

            // DIAGNOSTICO/PROVA (MESMO padrao de GUSWORLD_SYSMENU_HOVER_SELFTEST em
            // system_menu_loop.cpp e GUSWORLD_BATTLE_HOVER_SELFTEST em
            // battle_preview.cpp): avanca a conversa INTEIRA chamando
            // apply_npc_dialogue_input DIRETO (MESMA funcao pura ja testada em
            // npc_dialogue_overlay_test.cpp - bypassa o poll de evento real, SEM
            // SDL_PushEvent/xdotool/wmctrl, SEM tocar hardware nenhum) e chamando
            // present_frame() a cada no - prova que o contexto GL real + o stage
            // dir (fontes+retrato copiados) + o glintfx::UiLayer::load/update/render
            // desta caixa quente NAO CRASHAM em nenhum no do grafo real do Bertoldo,
            // rodando 100% headless sob Xvfb (nunca display :0). Imprime a contagem
            // de nos visitados e retorna ANTES do loop interativo (bypassa por
            // completo - nunca abre pra input real).
            const char* selftest = std::getenv("GUSWORLD_NPCDLG_SELFTEST");
            if (selftest != nullptr && selftest[0] != '\0') {
                int nodes_visited = 1;
                int guard = 0;
                while (!runtime.finished() && guard < 64) {
                    ++guard;
                    const bool is_choice = !runtime.current().options.empty();
                    const NpcDialogueInputAction action =
                        is_choice ? NpcDialogueInputAction::Confirm
                                  : NpcDialogueInputAction::Confirm;
                    selected_option =
                        apply_npc_dialogue_input(runtime, action, selected_option);
                    ++nodes_visited;
                    if (!runtime.finished()) {
                        reload();
                        present_frame();
                    }
                }
                SDL_Log(
                    "NpcDialogueLoopGl: [selftest] conversa inteira avancada ate "
                    "finished()=%d em %d nos visitados (guard=%d) - GL/glintfx/stage "
                    "dir OK, sem crash.",
                    runtime.finished() ? 1 : 0, nodes_visited, guard);
            } else {

            while (!runtime.finished()) {
                SDL_Event ev;
                bool changed = false;
                while (SDL_PollEvent(&ev)) {
                    if (ev.type == SDL_EVENT_QUIT) {
                        window_closed = true;
                        break;
                    }
                    if (ev.type == SDL_EVENT_WINDOW_RESIZED ||
                        ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                        SDL_GetWindowSizeInPixels(window, &pw, &ph);
                        if (pw < 1) pw = 1;
                        if (ph < 1) ph = 1;
                        dp_ratio = static_cast<float>(pw) / 960.0f;
                        ui.set_viewport(pw, ph);
                        ui.set_dp_ratio(dp_ratio);
                        continue;
                    }
                    if (ev.type != SDL_EVENT_KEY_DOWN || ev.key.repeat) {
                        continue;
                    }
                    NpcDialogueInputAction action = NpcDialogueInputAction::None;
                    switch (ev.key.key) {
                        case SDLK_UP:
                        case SDLK_W:
                            action = NpcDialogueInputAction::MoveUp;
                            break;
                        case SDLK_DOWN:
                        case SDLK_S:
                            action = NpcDialogueInputAction::MoveDown;
                            break;
                        case SDLK_RETURN:
                        case SDLK_KP_ENTER:
                        case SDLK_SPACE:
                            action = NpcDialogueInputAction::Confirm;
                            break;
                        default:
                            break;
                    }
                    if (action == NpcDialogueInputAction::None) {
                        continue;
                    }
                    selected_option =
                        apply_npc_dialogue_input(runtime, action, selected_option);
                    changed = true;
                    if (runtime.finished()) {
                        break;
                    }
                }
                if (window_closed || runtime.finished()) {
                    break;
                }
                if (changed) {
                    reload();
                }
                present_frame();
            }
            }  // else (caminho interativo real)
        }
    }  // ui (glintfx::UiLayer) destruida aqui, ANTES do contexto GL - mesma ordem
       // de run_system_menu_loop_gl_current/owning_gl.

    SDL_GL_DestroyContext(gl);

    // FIX BUG (ver header) - garante estado limpo ao RETOMAR a cidade.
    city.clear_input();

    return window_closed;
}

}  // namespace gus::app::screens
