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
#include <vector>

#include <glintfx/element_box.hpp>
#include <glintfx/ui_layer.hpp>

#include "gus/app/screens/npc_dialogue_overlay.hpp"
#include "gus/app/screens/npc_dialogue_rml.hpp"
#include "gus/app/screens/ui_hover.hpp"  // COCKPIT-SFX-HOVER-CLIQUE: fatia GENERICA de
                                         // edge-detect de hover, REUSADA aqui (MESMA
                                         // que o menu de sistema/cockpit ja usam)
#include "gus/core/asset_paths.hpp"  // kRetratosDir/kSfxDir/kMenuHoverSfxFile/kMenuClickSfxFile
#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Rect
#include "gus/platform/assets/asset_source.hpp"  // ASSETS-VFS-F1b (ADR-013): porteiro
#include "gus/platform/render2d/render2d_gl3.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"  // glad load (variante owning_gl)

// Pasta das fontes (.ttf), embutida pelo CMake (mesma macro que battle_preview.cpp/
// system_menu_loop.cpp ja usam - PRIVATE no CMakeLists do target app). Fica FORA do
// escopo do retrofit ASSETS-VFS-F1/F1b (ADR-013 marca o staging de fonte pro glintfx,
// abaixo em write_npc_dialogue_rml_file, como ESCRITA - fs::copy_file - nao leitura;
// MESMA excecao ja registrada em battle_preview.cpp/system_menu_loop.cpp).
#ifndef GUSWORLD_FONTS_DIR
#define GUSWORLD_FONTS_DIR ""
#endif
// (GUSWORLD_ASSETS_DIR/GUSWORLD_SFX_DIR foram REMOVIDOS daqui - ASSETS-VFS-F1b/
// ADR-013: a resolucao dessas 2 raizes agora mora dentro de FilesystemAssetSource,
// platform/assets/; este arquivo so consome via resolve_path(), MESMO padrao ja
// aplicado em battle_preview.cpp/system_menu_loop.cpp.)

namespace gus::app::screens {

namespace {

namespace fs = std::filesystem;

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

// Resolve a pasta resources/ (id relativo, familia GENERICA). ASSETS-VFS-F1b
// (ADR-013): a cadeia `env GUSWORLD_ASSETS > macro de compilacao > CWD ("resources/")`
// foi CONSOLIDADA em FilesystemAssetSource::resolve_path (o `rel` JA E o id logico, sem
// prefixo de familia especifica - cai no dispatch generico, MESMO padrao de
// resolve_asset_dir em battle_preview.cpp/resolve_assets_subdir_local em
// sdl_window.cpp). Assinatura INTOCADA.
std::string resolve_assets_subdir(std::string_view rel) {
    return gus::platform::assets::FilesystemAssetSource().resolve_path(rel);
}

// Id fixo do botao "Continuar" (npc_dialogue_rml.cpp: "npcdlg-continue-btn") - so
// 1 no LINEAR fica carregado por vez (MESMO racional de kPlaceholderBackId em
// system_menu_loop.cpp), entao 1 id fixo basta pro hit-test de hover/clique.
constexpr const char* kContinueBtnId = "npcdlg-continue-btn";

// Id de uma opcao (npc_dialogue_rml.cpp: "npcdlg-choice-<indice>", MESMA
// convencao de id-por-indice de pause_item_id/category_item_id em
// system_menu_loop.cpp) - fonte UNICA do nome pro hit-test de clique/hover POR
// OPCAO abaixo (evita o id divergir entre quem GERA o RML e quem CONSULTA).
std::string npc_dialogue_choice_id(int index) {
    return "npcdlg-choice-" + std::to_string(index);
}

// Hit-test simples: cursor (x,y, espaco-janela) dentro da caixa border-box
// devolvida por glintfx::UiLayer::get_element_box - MESMA receita/contrato de
// hit_test em system_menu_loop.cpp (box.found=false conta como "fora").
bool hit_test(const glintfx::ElementBox& box, float x, float y) {
    if (!box.found) return false;
    return x >= box.x && x <= box.x + box.w && y >= box.y && y <= box.y + box.h;
}

// Resolve o caminho de um SFX (hover/click do botao "Continuar") - MESMA familia SFX
// de resolve_menu_sfx_path (system_menu_loop.cpp)/resolve_ui_sfx_path
// (battle_preview.cpp). ASSETS-VFS-F1b (ADR-013): a cadeia `env GUSWORLD_SFX > macro
// GUSWORLD_SFX_DIR > CWD (kSfxDir)` foi CONSOLIDADA em FilesystemAssetSource (dispatch
// pelo prefixo "assets/sfx/" do id). Assinatura INTOCADA.
std::string resolve_dialogue_sfx_path(std::string_view file) {
    const std::string id = join(std::string(gus::core::assets::kSfxDir), std::string(file));
    return gus::platform::assets::FilesystemAssetSource().resolve_path(id);
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
    const gus::app::i18n::Translator& tr, int selected_option,
    bool continue_pressed = false, int pressed_option = -1) {
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

    std::string rml = build_npc_dialogue_rml(node, tr, selected_option, portrait_file,
                                              continue_pressed, pressed_option);
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

// Digito 1-9 de uma tecla numerica (fileira OU numpad); 0 se nao for numerica
// 1-9. MESMA tecnica de gus::app::screens::battle_digit_for_key (battle_
// preview.cpp) - ver o header (npc_dialogue_loop_gl.hpp) pro motivo de
// duplicar em vez de reusar (peso de include de battle_scene.hpp).
int npc_dialogue_digit_for_key(SDL_Keycode key) noexcept {
    switch (key) {
        case SDLK_1: case SDLK_KP_1: return 1;
        case SDLK_2: case SDLK_KP_2: return 2;
        case SDLK_3: case SDLK_KP_3: return 3;
        case SDLK_4: case SDLK_KP_4: return 4;
        case SDLK_5: case SDLK_KP_5: return 5;
        case SDLK_6: case SDLK_KP_6: return 6;
        case SDLK_7: case SDLK_KP_7: return 7;
        case SDLK_8: case SDLK_KP_8: return 8;
        case SDLK_9: case SDLK_KP_9: return 9;
        default: return 0;
    }
}

bool run_npc_dialogue_loop_gl(SDL_Window* window, gus::app::SdlWindow& city,
                               gus::domain::dialogue::DialogueRuntime& runtime,
                               const gus::app::i18n::Translator& translator,
                               gus::platform::audio::AudioEngine& audio,
                               const std::string& frozen_background_png) {
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

            // FUNDO REAL CONGELADO (retoque do lider via AskUserQuestion,
            // M7-DIALOGO): carrega a textura UMA VEZ (mesmo racional de
            // "load_texture NUNCA no frame" ja documentado pros SFX abaixo) -
            // kInvalidTexture (path vazio/asset ausente/backend headless) degrada
            // com seguranca pra vinheta de sempre (ver present_frame).
            const gus::platform::render2d::TextureId frozen_bg_tex =
                frozen_background_png.empty()
                    ? gus::platform::render2d::kInvalidTexture
                    : backdrop.load_texture(frozen_background_png.c_str());

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
                backdrop.begin_frame(cam, pw, ph);  // clear + vinheta radial (fallback
                                                     // abstrato)
                if (frozen_bg_tex != gus::platform::render2d::kInvalidTexture) {
                    // FUNDO REAL CONGELADO (decisao do lider): cobre a vinheta com
                    // a CENA REAL da cidade (1 frame estatico, capturado pela
                    // Maestro ANTES de abrir - ver Maestro::to_npc_dialogue/
                    // SdlWindow::capture_frame_to_png), full-screen e opaca -
                    // mesmo padrao de Chrono Trigger/Zelda/Stardew Valley (o mundo
                    // "pausa" atras da UI). O #npcdlg-scrim do RML (npc_dialogue_
                    // rml.cpp, ~40% preto) segue desenhado por CIMA disto pelo
                    // glintfx (ui.render() abaixo) - a legibilidade da caixa nao
                    // muda.
                    backdrop.draw_textured_rect(
                        cam, frozen_bg_tex,
                        gus::platform::render2d::UvRect{0.0f, 0.0f, 1.0f, 1.0f},
                        gus::platform::render2d::DrawColor{1.0f, 1.0f, 1.0f, 1.0f});
                }
                backdrop.end_frame();
                ui.update();
                ui.render();
                SDL_GL_SwapWindow(window);
            };

            // SFX de hover/clique do botao "Continuar" (pedido do lider): MESMOS 2
            // arquivos que o menu de sistema/cockpit ja usam (kMenuHoverSfxFile/
            // kMenuClickSfxFile) - load_sfx UMA VEZ por sessao de dialogo (MESMO
            // padrao de hover_sfx_id/click_sfx_id em system_menu_loop.cpp,
            // "load_sfx NUNCA no frame"). audio.available()==false (device
            // indisponivel/CI) degrada com seguranca - play_sfx(id invalido) ja e
            // no-op.
            const std::string hover_sfx_path =
                resolve_dialogue_sfx_path(gus::core::assets::kMenuHoverSfxFile);
            const std::string click_sfx_path =
                resolve_dialogue_sfx_path(gus::core::assets::kMenuClickSfxFile);
            const gus::platform::audio::SoundId hover_sfx_id =
                audio.load_sfx(hover_sfx_path.c_str());
            const gus::platform::audio::SoundId click_sfx_id =
                audio.load_sfx(click_sfx_path.c_str());

            int hovered_index = -1;  // -1 = mouse fora de qualquer item navegavel
                                     // (botao "Continuar" OU 1 das opcoes, ver
                                     // abaixo); edge-detect PURO do SOM de hover
                                     // (ver ui_hover.hpp), MESMA logica do
                                     // menu/cockpit.

            // HOVER (mouse) - injeta o MouseMove no glintfx (visual :hover NATIVO,
            // MESMO pipeline do menu de sistema/cockpit) e faz o hit-test/edge-
            // detect do SOM de hover: no LINEAR (node.options.empty()) so ha o
            // botao "Continuar" (1 caixa); no de ESCOLHA (pedido do lider) ha 1
            // caixa POR OPCAO ("npcdlg-choice-<indice>", ver npc_dialogue_rml.hpp) -
            // MESMA dupla ui_hover_index/UiHoverBox do menu/cockpit, so com N>1
            // caixas dinamico (options.size(), nao um maximo fixo). Fatorada em
            // lambda pra ser chamada tanto pelo SDL_EVENT_MOUSE_MOTION real
            // (abaixo) quanto pelos self-tests headless (GUSWORLD_NPCDLG_HOVER_
            // SELFTEST/GUSWORLD_NPCDLG_OPTIONS_SELFTEST) - MESMO caminho de
            // codigo prova o comportamento real.
            auto handle_mouse_motion = [&](float mx, float my) {
                glintfx::UiEvent hover_ev{};
                hover_ev.type = glintfx::UiEvent::Type::MouseMove;
                hover_ev.x = mx;
                hover_ev.y = my;
                ui.process_event(hover_ev);

                int new_hover = -1;
                if (runtime.current().options.empty()) {
                    const glintfx::ElementBox box = ui.get_element_box(kContinueBtnId);
                    const UiHoverBox hb{box.found, box.x, box.y, box.w, box.h};
                    new_hover = ui_hover_index(mx, my, &hb, 1);
                } else {
                    const auto& options = runtime.current().options;
                    std::vector<UiHoverBox> boxes(options.size());
                    for (std::size_t i = 0; i < options.size(); ++i) {
                        const glintfx::ElementBox box = ui.get_element_box(
                            npc_dialogue_choice_id(static_cast<int>(i)).c_str());
                        boxes[i] = UiHoverBox{box.found, box.x, box.y, box.w, box.h};
                    }
                    new_hover =
                        ui_hover_index(mx, my, boxes.data(), static_cast<int>(boxes.size()));
                }
                if (ui_hover_entered_new_item(hovered_index, new_hover)) {
                    audio.play_sfx(hover_sfx_id);
                }
                hovered_index = new_hover;
            };

            // EFEITO DE PRESS (MESMO padrao de flash_pressed() em
            // system_menu_loop.cpp, ver header): renderiza o no ATUAL (ainda NAO
            // avancado - o CHAMADOR chama isto ANTES de apply_npc_dialogue_input)
            // com o item que vai ser confirmado marcado ".pressed", por ~100ms
            // (4 frames de ~25ms), e SO DEPOIS devolve - o chamador segue
            // avancando o dialogo normalmente. SOM DE CLIQUE: dispara AQUI, no
            // MESMO choke-point chamado por CLIQUE de mouse, TECLA DE NUMERO OU
            // Enter/Espaco de TECLADO (ver o loop abaixo) - 1 play_sfx cobre os
            // 3 canais sem duplicar logica. GENERALIZADO (pedido do lider,
            // opcoes clicaveis): olha o no ATUAL pra decidir QUEM marcar - no
            // LINEAR marca o botao "Continuar" (continue_pressed=true), no de
            // ESCOLHA marca a opcao `selected_option` (pressed_option) - o
            // CHAMADOR so precisa garantir que `selected_option` ja aponta pra
            // opcao certa ANTES de chamar isto (ver confirm_choice abaixo).
            auto flash_pressed = [&] {
                audio.play_sfx(click_sfx_id);
                const bool is_linear = runtime.current().options.empty();
                rml_path = write_npc_dialogue_rml_file(
                    runtime.current(), translator, selected_option,
                    /*continue_pressed=*/is_linear,
                    /*pressed_option=*/is_linear ? -1 : selected_option);
                ui.load(rml_path.c_str());
                ui.set_viewport(pw, ph);
                ui.set_dp_ratio(dp_ratio);
                for (int frame = 0; frame < 4; ++frame) {
                    present_frame();
                    SDL_Delay(25);
                }
            };

            // CONFIRMA DIRETO uma opcao do no de ESCOLHA (indice `option_index`,
            // pedido do lider): seleciona (selected_option = option_index, MESMO
            // efeito de navegar ate ela por Up/Down) e confirma no MESMO
            // choke-point de flash+som acima (flash_pressed, ja generalizado).
            // Usada pelo CLIQUE de mouse numa `.warm-choice`, pela TECLA DE
            // NUMERO (handle_number_key abaixo) e pelos self-tests headless -
            // MESMO caminho de codigo pros 3 canais, sem duplicar logica (MESMA
            // razao de ser de handle_mouse_motion acima).
            auto confirm_choice = [&](int option_index) {
                selected_option = option_index;
                flash_pressed();
                selected_option = apply_npc_dialogue_input(
                    runtime, NpcDialogueInputAction::Confirm, selected_option);
            };

            // TECLA DE NUMERO (1-9, fileira+numpad - pedido do lider): SO no de
            // ESCOLHA (node.options nao vazio), SELECIONA e CONFIRMA DIRETO a
            // N-esima opcao (sem precisar navegar com Up/Down antes) via
            // confirm_choice acima. Devolve false (NO-OP explicito, nada
            // mexido) se o no ATUAL for LINEAR (sem opcoes - so o botao
            // "Continuar", que nao tem atalho numerico) OU se o digito estiver
            // FORA do range disponivel (ex. tecla "9" com so 3 opcoes - guarda
            // pedida pelo lider). Fatorada em lambda pra ser chamada tanto pelo
            // SDL_EVENT_KEY_DOWN real (abaixo) quanto pelo self-test headless
            // (GUSWORLD_NPCDLG_OPTIONS_SELFTEST) - MESMO caminho de codigo
            // prova o comportamento real, MESMO espirito de handle_mouse_motion.
            auto handle_number_key = [&](SDL_Keycode key) -> bool {
                if (runtime.current().options.empty()) return false;
                const int nth = npc_dialogue_digit_for_key(key);
                const int option_count =
                    static_cast<int>(runtime.current().options.size());
                if (nth < 1 || nth > option_count) return false;
                confirm_choice(nth - 1);
                return true;
            };

            // CLIQUE numa opcao (pedido do lider): hit-test POR OPCAO
            // ("npcdlg-choice-<indice>") - a PRIMEIRA caixa que bater SELECIONA
            // e CONFIRMA a opcao DIRETO via confirm_choice acima (MESMO
            // comportamento "1 clique ja aciona" de uma pill do menu de
            // sistema). Devolve false (NO-OP) se o clique cair fora de
            // qualquer caixa - o CHAMADOR (evento real abaixo) so invoca isto
            // quando runtime.current().options nao esta vazio (ha caixas pra
            // testar). Fatorada em lambda pra ser chamada tanto pelo
            // SDL_EVENT_MOUSE_BUTTON_DOWN real (abaixo) quanto pelo self-test
            // headless (GUSWORLD_NPCDLG_OPTIONS_SELFTEST) - MESMO caminho de
            // codigo prova o comportamento real, MESMO espirito de
            // handle_mouse_motion/handle_number_key.
            auto handle_choice_click = [&](float x, float y) -> bool {
                const auto& options = runtime.current().options;
                for (std::size_t i = 0; i < options.size(); ++i) {
                    const glintfx::ElementBox box = ui.get_element_box(
                        npc_dialogue_choice_id(static_cast<int>(i)).c_str());
                    if (hit_test(box, x, y)) {
                        confirm_choice(static_cast<int>(i));
                        return true;
                    }
                }
                return false;
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
            } else if (const char* hover_selftest =
                           std::getenv("GUSWORLD_NPCDLG_HOVER_SELFTEST");
                       hover_selftest != nullptr && hover_selftest[0] != '\0') {
                // DIAGNOSTICO/PROVA (HOVER/CLIQUE do botao "Continuar", ver header):
                // MESMO espirito de GUSWORLD_SYSMENU_HOVER_SELFTEST (system_menu_
                // loop.cpp) - move o mouse SINTETICO fora->dentro->dentro(repete)->
                // fora->dentro do botao (prova ui_hover_entered_new_item: 2 entradas
                // NOVAS esperadas, a repeticao sobre o MESMO item nao redispara) via
                // handle_mouse_motion (O MESMO codigo do SDL_EVENT_MOUSE_MOTION real
                // abaixo, sem duplicar), e SIMULA 1 clique (flash_pressed + avanca o
                // dialogo) - tudo SEM SDL_PushEvent/xdotool/wmctrl, SEM tocar
                // hardware de audio (audio.available() decide - este self-test so
                // EXERCITA os call-sites; sfx_play_count() e o hook de prova).
                // Degrada com seguranca se o no ATUAL nao for LINEAR (sem botao pra
                // achar - kContinueBtnId nao existe no doc) - nunca crasha.
                present_frame();  // assenta o layout (get_element_box precisa de 1 update())

                if (runtime.current().options.empty()) {
                    const glintfx::ElementBox box = ui.get_element_box(kContinueBtnId);
                    const float cx = box.found ? box.x + box.w * 0.5f : -1.0f;
                    const float cy = box.found ? box.y + box.h * 0.5f : -1.0f;

                    handle_mouse_motion(-100.0f, -100.0f);  // fora de qualquer botao
                    present_frame();
                    handle_mouse_motion(cx, cy);  // ENTRA - 1a entrada NOVA
                    present_frame();
                    handle_mouse_motion(cx, cy);  // MESMO item - nao redispara
                    present_frame();
                    handle_mouse_motion(-100.0f, -100.0f);  // sai
                    present_frame();
                    handle_mouse_motion(cx, cy);  // volta - 2a entrada NOVA
                    present_frame();
                    SDL_Log(
                        "NpcDialogueLoopGl: [hover-selftest] hover_sfx_play_count apos "
                        "fora->dentro->dentro(repete)->fora->dentro (2 entradas NOVAS "
                        "esperadas) = %u",
                        audio.sfx_play_count());

                    const unsigned int click_baseline = audio.sfx_play_count();
                    const std::string node_before = runtime.current().id;
                    flash_pressed();
                    selected_option = apply_npc_dialogue_input(
                        runtime, NpcDialogueInputAction::Confirm, selected_option);
                    SDL_Log(
                        "NpcDialogueLoopGl: [hover-selftest] click_sfx disparou %u x "
                        "(esperado 1) - total sfx_play_count()=%u; no avancou de "
                        "'%s' para '%s' (finished()=%d)",
                        audio.sfx_play_count() - click_baseline, audio.sfx_play_count(),
                        node_before.c_str(),
                        runtime.finished() ? "(fim)" : runtime.current().id.c_str(),
                        runtime.finished() ? 1 : 0);
                } else {
                    SDL_Log(
                        "NpcDialogueLoopGl: [hover-selftest] no ATUAL e de ESCOLHA "
                        "(sem botao 'Continuar') - nada a provar aqui, encerrando "
                        "sem crash (degradacao segura).");
                }
            } else if (const char* options_selftest =
                           std::getenv("GUSWORLD_NPCDLG_OPTIONS_SELFTEST");
                       options_selftest != nullptr && options_selftest[0] != '\0') {
                // DIAGNOSTICO/PROVA (as 3 melhorias de OPCOES clicaveis - pedido
                // do lider, ver header): entra em n0_greet (LINEAR, ja aberto
                // acima), avanca DIRETO via apply_npc_dialogue_input (MESMA
                // funcao pura ja testada - o botao "Continuar" ja tem PROVA
                // propria em HOVER_SELFTEST, nao repetida aqui) ate n1_hook (no
                // de ESCOLHA, 3 opcoes reais do grafo do Bertoldo) - dai
                // exercita HOVER multi-opcao + edge-detect (2), a GUARDA de
                // tecla-numero fora do range e o CLIQUE/tecla-de-numero
                // confirmando a opcao CORRETA (1/3), tudo via os MESMOS
                // handle_mouse_motion/handle_number_key/confirm_choice do
                // caminho real - SEM SDL_PushEvent/xdotool/wmctrl, SEM tocar
                // hardware de audio (sfx_play_count() e o hook de prova).
                present_frame();  // assenta o layout inicial (n0_greet)

                if (!runtime.current().options.empty()) {
                    SDL_Log(
                        "NpcDialogueLoopGl: [options-selftest] entry node JA e de "
                        "ESCOLHA (esperava LINEAR n0_greet) - grafo mudou, "
                        "abortando self-test sem crash.");
                } else {
                    // Avanca n0_greet -> n1_hook SEM flash/som (prova propria ja
                    // em HOVER_SELFTEST) - so posiciona o runtime no de ESCOLHA.
                    selected_option = apply_npc_dialogue_input(
                        runtime, NpcDialogueInputAction::Confirm, selected_option);
                    reload();
                    present_frame();  // assenta o layout do no de ESCOLHA

                    const int option_count =
                        static_cast<int>(runtime.current().options.size());
                    if (option_count < 3) {
                        SDL_Log(
                            "NpcDialogueLoopGl: [options-selftest] n1_hook tem %d "
                            "opcoes (esperava 3) - grafo do Bertoldo mudou, "
                            "self-test parcial.",
                            option_count);
                    }

                    // (1) HOVER multi-opcao + edge-detect (item 2 - pedido do
                    // lider): fora->0->1->2->2(repete)->fora->0, MESMO espirito
                    // de hover_sequence={0,1,2,3,0} em
                    // GUSWORLD_SYSMENU_HOVER_SELFTEST (system_menu_loop.cpp) - 4
                    // entradas NOVAS esperadas (0,1,2,0-de-novo); a repeticao
                    // sobre o item 2 e a saida pro vazio NAO disparam.
                    auto choice_center = [&](int idx, float* out_x, float* out_y) {
                        const glintfx::ElementBox box =
                            ui.get_element_box(npc_dialogue_choice_id(idx).c_str());
                        *out_x = box.found ? box.x + box.w * 0.5f : -100.0f;
                        *out_y = box.found ? box.y + box.h * 0.5f : -100.0f;
                    };
                    const int hover_sequence[] = {0, 1, 2, 2, -1, 0};
                    for (const int idx : hover_sequence) {
                        if (idx < 0) {
                            handle_mouse_motion(-100.0f, -100.0f);
                        } else {
                            float cx = -100.0f, cy = -100.0f;
                            choice_center(idx, &cx, &cy);
                            handle_mouse_motion(cx, cy);
                        }
                        present_frame();
                    }
                    SDL_Log(
                        "NpcDialogueLoopGl: [options-selftest] hover_sfx_play_count "
                        "apos 0->1->2->2(repete)->fora->0 (4 entradas NOVAS "
                        "esperadas) = %u",
                        audio.sfx_play_count());

                    // (2) CLIQUE de mouse (item 1 - pedido do lider) NUMA CAIXA
                    // FORA de qualquer opcao PRIMEIRO (ex. canto vazio da tela) -
                    // handle_choice_click devolve false, NADA e confirmado (prova
                    // que o clique NAO dispara em falso fora das caixas).
                    const unsigned int miss_baseline = audio.sfx_play_count();
                    const std::string node_before_miss = runtime.current().id;
                    const bool consumed_miss = handle_choice_click(1.0f, 1.0f);
                    SDL_Log(
                        "NpcDialogueLoopGl: [options-selftest] clique FORA de "
                        "qualquer opcao (canto 1,1) consumed=%d (esperado 0), no "
                        "continua '%s' (era '%s'), click_sfx tocou %u x "
                        "(esperado 0)",
                        consumed_miss ? 1 : 0, runtime.current().id.c_str(),
                        node_before_miss.c_str(),
                        audio.sfx_play_count() - miss_baseline);

                    // (3) CLIQUE de mouse NA CAIXA da opcao de INDICE 1
                    // (pragmatico, NAO a 0 - prova que confirma a CORRETA, nao
                    // sempre a primeira) SELECIONA e CONFIRMA DIRETO (item 1) -
                    // 1 clique ja aciona, sem passo de foco separado.
                    float cx1 = -100.0f, cy1 = -100.0f;
                    choice_center(1, &cx1, &cy1);
                    const unsigned int click_baseline = audio.sfx_play_count();
                    const std::string node_before_confirm = runtime.current().id;
                    const bool consumed = handle_choice_click(cx1, cy1);
                    SDL_Log(
                        "NpcDialogueLoopGl: [options-selftest] clique na opcao "
                        "indice 1 (pragmatico) consumed=%d (esperado 1), no "
                        "avancou de '%s' para '%s' (esperava 'n2b_pragmatico'), "
                        "click_sfx tocou %u x (esperado 1), finished()=%d",
                        consumed ? 1 : 0, node_before_confirm.c_str(),
                        runtime.finished() ? "(fim)" : runtime.current().id.c_str(),
                        audio.sfx_play_count() - click_baseline,
                        runtime.finished() ? 1 : 0);
                }
            } else if (const char* hotkey_selftest =
                           std::getenv("GUSWORLD_NPCDLG_OPTIONS_HOTKEY_SELFTEST");
                       hotkey_selftest != nullptr && hotkey_selftest[0] != '\0') {
                // DIAGNOSTICO/PROVA (item 3 - TECLA DE NUMERO, pedido do lider):
                // MESMO roteiro de entrada de GUSWORLD_NPCDLG_OPTIONS_SELFTEST
                // (n0_greet -> n1_hook), execucao SEPARADA (env var propria,
                // MESMA independencia de GUSWORLD_NPCDLG_SELFTEST vs
                // GUSWORLD_NPCDLG_HOVER_SELFTEST) porque o grafo do Bertoldo so
                // permite UMA confirmacao real de n1_hook por execucao - aqui a
                // confirmacao "de verdade" e gasta pela TECLA DE NUMERO (o
                // CLIQUE de mouse ja tem a sua propria acima).
                present_frame();

                if (!runtime.current().options.empty()) {
                    SDL_Log(
                        "NpcDialogueLoopGl: [hotkey-selftest] entry node JA e de "
                        "ESCOLHA (esperava LINEAR n0_greet) - grafo mudou, "
                        "abortando self-test sem crash.");
                } else {
                    selected_option = apply_npc_dialogue_input(
                        runtime, NpcDialogueInputAction::Confirm, selected_option);
                    reload();
                    present_frame();

                    const int option_count =
                        static_cast<int>(runtime.current().options.size());

                    // (1) GUARDA: tecla "9" fora do range disponivel (so ha 3
                    // opcoes) - handle_number_key devolve false, NADA e
                    // selecionado/confirmado (o no continua n1_hook, sem som).
                    const unsigned int guard_baseline = audio.sfx_play_count();
                    const std::string node_before_guard = runtime.current().id;
                    const bool consumed_out_of_range = handle_number_key(SDLK_9);
                    SDL_Log(
                        "NpcDialogueLoopGl: [hotkey-selftest] tecla '9' (fora do "
                        "range, so ha %d opcoes) consumed=%d (esperado 0), no "
                        "continua '%s' (era '%s'), click_sfx tocou %u x "
                        "(esperado 0)",
                        option_count, consumed_out_of_range ? 1 : 0,
                        runtime.current().id.c_str(), node_before_guard.c_str(),
                        audio.sfx_play_count() - guard_baseline);

                    // (2) TECLA "2" SELECIONA e CONFIRMA DIRETO a opcao de
                    // INDICE 1 (pragmatico, NAO a 0) sem navegar com Up/Down
                    // antes - o pedido central do item 3.
                    const unsigned int click_baseline = audio.sfx_play_count();
                    const std::string node_before_confirm = runtime.current().id;
                    const bool consumed = handle_number_key(SDLK_2);
                    SDL_Log(
                        "NpcDialogueLoopGl: [hotkey-selftest] tecla '2' "
                        "consumed=%d (esperado 1), no avancou de '%s' para '%s' "
                        "(esperava 'n2b_pragmatico'), click_sfx tocou %u x "
                        "(esperado 1), finished()=%d",
                        consumed ? 1 : 0, node_before_confirm.c_str(),
                        runtime.finished() ? "(fim)" : runtime.current().id.c_str(),
                        audio.sfx_play_count() - click_baseline,
                        runtime.finished() ? 1 : 0);
                }
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
                    if (ev.type == SDL_EVENT_KEY_DOWN && !ev.key.repeat) {
                        // TECLA DE NUMERO (1-9, fileira+numpad - pedido do lider):
                        // testada ANTES do switch generico de Up/Down/Enter -
                        // handle_number_key ja e NO-OP seguro (devolve false) em no
                        // LINEAR ou fora do range de opcoes disponiveis, entao cai
                        // pro switch abaixo sem efeito nenhum (nenhuma tecla
                        // numerica mapeia pra uma NpcDialogueInputAction de
                        // qualquer forma - MESMA prioridade de battle_digit_for_key
                        // em battle_key_down, battle_preview.cpp).
                        if (handle_number_key(ev.key.key)) {
                            changed = true;
                            if (runtime.finished()) {
                                break;
                            }
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
                        // FLASH DE CLIQUE (teclado): na CONFIRMACAO (Enter/Espaco),
                        // TANTO no LINEAR (botao "Continuar") QUANTO no de ESCOLHA (a
                        // opcao ja navegada por Up/Down, `selected_option`) -
                        // flash_pressed() generalizado decide QUEM marcar olhando o
                        // no ATUAL (ver comentario la). MoveUp/MoveDown (navegacao)
                        // nunca piscam. MESMO choke-point do clique de mouse/tecla de
                        // numero abaixo, MESMA razao de ser de is_confirming/
                        // flash_pressed em system_menu_loop.cpp.
                        if (action == NpcDialogueInputAction::Confirm) {
                            flash_pressed();
                        }
                        selected_option =
                            apply_npc_dialogue_input(runtime, action, selected_option);
                        changed = true;
                        if (runtime.finished()) {
                            break;
                        }
                    } else if (ev.type == SDL_EVENT_MOUSE_MOTION) {
                        // SEMPRE (nao so em no LINEAR - handle_mouse_motion ja decide
                        // internamente se ha botao OU opcoes pra hover): hover NATIVO
                        // (:hover RCSS) + edge-detect do SOM de hover.
                        handle_mouse_motion(ev.motion.x, ev.motion.y);
                    } else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                               ev.button.button == SDL_BUTTON_LEFT) {
                        if (runtime.current().options.empty()) {
                            // CLIQUE no botao "Continuar" - SO existe em no LINEAR
                            // (ver npc_dialogue_rml.cpp: node.options.empty()).
                            // Equivalente a Enter/Espaco de teclado - MESMO
                            // choke-point (flash_pressed).
                            const glintfx::ElementBox box =
                                ui.get_element_box(kContinueBtnId);
                            if (hit_test(box, ev.button.x, ev.button.y)) {
                                flash_pressed();
                                selected_option = apply_npc_dialogue_input(
                                    runtime, NpcDialogueInputAction::Confirm,
                                    selected_option);
                                changed = true;
                                if (runtime.finished()) {
                                    break;
                                }
                            }
                        } else {
                            // CLIQUE numa opcao (pedido do lider): delega pro
                            // MESMO handle_choice_click chamado pelo self-test
                            // headless (ver acima, MESMO espirito de
                            // handle_mouse_motion/handle_number_key).
                            if (handle_choice_click(ev.button.x, ev.button.y)) {
                                changed = true;
                            }
                            if (runtime.finished()) {
                                break;
                            }
                        }
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
