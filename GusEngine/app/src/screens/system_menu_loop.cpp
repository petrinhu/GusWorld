// gus/app/src/screens/system_menu_loop.cpp
//
// Implementacao do loop interativo do MENU DE SISTEMA. Ver header para o contrato
// completo. GL/glintfx-heavy (mesma familia de battle_preview.cpp) - sem unidade
// de teste direta (irredutivel, mesmo racional de run_battle_preview_embedded: a
// logica PURA testavel ja fica em system_menu.hpp/system_menu_test.cpp e
// system_menu_rml.hpp/system_menu_rml_test.cpp; este .cpp so orquestra SDL/GL em
// torno dela).
//
// EFEITO DE PRESS (MENU-PAUSA-CONFIG-SOM, onda arvore): quando o jogador aciona
// uma pill/categoria/Voltar (Enter/Espaco no TECLADO ou clique de MOUSE), o loop
// renderiza ALGUNS FRAMES com a classe "pressed" no item ativado (flash cyan
// intenso, ver .verb-pill.pressed/.btn-back.pressed em system_menu_rml.cpp) ANTES
// de aplicar a transicao de fato (trocar de tela/fechar o menu/pedir Sair). Isto e
// deliberadamente um efeito NOSSO (nao do glintfx - a lib nao tem estado "active"
// disparado por teclado, so :focus/:hover via classe) - flash_pressed() abaixo e o
// UNICO lugar que gera esse frame extra.
//
// HOVER NATIVO + SOM DE HOVER/CLIQUE (retoque ao vivo do lider, pos-ONDA ARVORE):
// o VISUAL do hover e 100% :hover nativo do glintfx (RCSS em system_menu_rml.cpp) -
// so precisamos injetar UiEvent::MouseMove em TODO SDL_EVENT_MOUSE_MOTION (nao so
// durante o arrasto do slider, como antes), MESMO pipeline ja em producao no
// cockpit da batalha (battle_preview.cpp: sdl_to_glintfx -> process_event ->
// Context::ProcessMouseMove -> :hover). O SOM, ao contrario, NAO tem equivalente
// nativo - handle_mouse_motion() abaixo faz hit-test leve (current_hover_index)
// nas MESMAS caixas/ids do clique, e so dispara audio.play_sfx(hover_sfx_id_)
// quando o item hovered MUDA pra um NOVO item valido (edge-detect PURO/testavel:
// system_menu_hover_index + system_menu_hover_entered_new_item em
// system_menu.hpp/.cpp - ver system_menu_test.cpp). O som de CLIQUE
// (click_sfx_id_) dispara dentro de flash_pressed() - o MESMO choke-point que ja
// gera o flash visual .pressed pra CONFIRMACAO de teclado OU mouse, garantindo 1
// unico lugar pros dois canais de entrada (sem duplicar a logica).

#include "gus/app/screens/system_menu_loop.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
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
#include "gus/platform/render2d/render2d_gl3.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"  // glad load (variante owning_gl)

// Pasta das fontes (.ttf), embutida pelo CMake (mesma macro que battle_preview.cpp
// ja usa - PRIVATE no CMakeLists do target app, aplica a TODO .cpp do target). SO usada
// pelo staging de fonte pro glintfx (write_system_menu_rml_file) - FORA do escopo do
// retrofit ASSETS-VFS-F1 (ADR-013 marca esse staging como ESCRITA, nao leitura).
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

// Diretorio de stage do RML do menu (mesma receita de glintfx_cockpit_stage_dir em
// battle_preview.cpp: tempfile - o glintfx carrega documento por PATH). Nome
// PROPRIO (nao colide com o stage do cockpit da batalha).
std::string menu_stage_dir() {
    return (fs::temp_directory_path() / "gusworld_glintfx_sysmenu").string();
}

// Escreve o RML do menu (build_system_menu_rml, ver system_menu_rml.hpp) num
// arquivo dentro do stage, com o @font-face injetado logo apos <style> (o
// glintfx::UiLayer NAO expoe Rml::LoadFontFace - so o doc registra a familia via
// @font-face, MESMA receita de write_baked_cockpit_rml/write_live_cockpit_rml em
// battle_preview.cpp). Copia as 2 fontes pro stage (fonte: GUSWORLD_FONTS_DIR, env
// GUSWORLD_FONTS tem prioridade - mesma ordem de resolucao de asset do resto do
// app/). Devolve o path do .rml escrito. `pressed_index` repassado direto pra
// build_system_menu_rml (ver seu header) - default -1 (nenhum item pressionado).
std::string write_system_menu_rml_file(const SystemMenuState& state,
                                        const gus::app::i18n::Translator& tr,
                                        int pressed_index = -1) {
    const fs::path stage = menu_stage_dir();
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

    std::string rml = build_system_menu_rml(state, tr, pressed_index);
    const std::string needle = "<style>\n";
    const std::size_t pos = rml.find(needle);
    if (pos != std::string::npos) {
        rml.insert(pos + needle.size(),
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "src: \"PixelOperatorMono.ttf\"; }\n"
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "font-weight: bold; src: \"PixelOperatorMono-Bold.ttf\"; }\n");
    }

    const fs::path out = stage / "system_menu.rml";
    std::ofstream f(out);
    f << rml;
    return out.string();
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
// ControlsApplied ("Aplicar" confirmado, ver handle_action) - remap/
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
// deste arquivo). box.found=false conta como "fora".
bool hit_test(const glintfx::ElementBox& box, float x, float y) {
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

// Preenche `boxes` com a geometria dos itens NAVEGAVEIS da tela ATUAL
// (MESMOS ids do hit-test de clique acima - pause_item_id/category_item_id/
// audio_item_id/kPlaceholderBackId) e devolve o indice hovered via
// system_menu_hover_index (POCO, testado em system_menu_test.cpp). So a
// CONSULTA a UI (get_element_box, GL-heavy) mora aqui; a decisao "qual bateu"
// e pura.
int current_hover_index(const glintfx::UiLayer& ui, const SystemMenuState& state,
                         float mouse_x, float mouse_y) {
    SystemMenuHoverBox boxes[kSystemMenuMaxHoverItems];
    auto fill = [&](int idx, const std::string& id) {
        const glintfx::ElementBox box = ui.get_element_box(id.c_str());
        boxes[idx] = SystemMenuHoverBox{box.found, box.x, box.y, box.w, box.h};
    };
    switch (state.screen) {
        case SystemMenuScreen::Pause:
            for (int i = 0; i < kPauseItemCount; ++i) fill(i, pause_item_id(i));
            break;
        case SystemMenuScreen::ConfigCategories:
            for (int i = 0; i < kConfigCategoriesItemCount; ++i) fill(i, category_item_id(i));
            break;
        case SystemMenuScreen::Audio:
            for (int i = 0; i < kAudioItemCount; ++i) fill(i, audio_item_id(i));
            break;
        case SystemMenuScreen::Controls:
            // Capturando: nenhum item hover-testavel (system_menu_hover_index ja
            // devolve count=0 pra essa combinacao - nada a preencher aqui).
            // Confirmando o restaurar-padrao OU o descarte (M2 STAGED CHANGES):
            // so as 2 pills do mini-dialogo correspondente.
            if (state.controls_confirming_restore) {
                fill(0, controls_confirm_id(0));
                fill(1, controls_confirm_id(1));
            } else if (state.controls_confirming_discard) {
                fill(0, controls_discard_confirm_id(0));
                fill(1, controls_discard_confirm_id(1));
            } else if (!state.controls_capturing) {
                // BUG-A: filtra as 30 actions (nao o rodape) pelo recorte
                // visivel de `.ctrl-list` - ver filter_offscreen_controls_row.
                const glintfx::ElementBox list_box = ui.get_element_box(kControlsListId);
                for (int i = 0; i < kControlsItemCount; ++i) {
                    const glintfx::ElementBox raw = ui.get_element_box(controls_item_id(i).c_str());
                    const glintfx::ElementBox box = filter_offscreen_controls_row(i, raw, list_box);
                    boxes[i] = SystemMenuHoverBox{box.found, box.x, box.y, box.w, box.h};
                }
            }
            break;
        case SystemMenuScreen::Video:
        case SystemMenuScreen::Language:
            fill(0, kPlaceholderBackId);
            break;
        case SystemMenuScreen::Hidden:
            break;  // sem itens - system_menu_hover_index devolve -1 de qualquer jeito
    }
    return system_menu_hover_index(state, mouse_x, mouse_y, boxes);
}

}  // namespace

SystemMenuLoopOutcome run_system_menu_loop_gl_current(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator, const std::string& settings_dir,
    const std::string& saves_dir,
    const std::function<gus::domain::save::SaveData()>& build_current_save_data,
    const std::function<void(const gus::domain::save::SaveData&)>&
        apply_loaded_save_data,
    const std::string& frozen_background_png) {
    SystemMenuLoopOutcome outcome;

    SystemMenuState state;
    state.music_volume = audio.music_volume();
    state.sfx_volume = audio.sfx_volume();
    // Tela Controles (M2 -> M2 STAGED CHANGES): carrega o remap persistido (ou
    // default_controls() se ausente/corrompido, ver controls_file_store.hpp) -
    // MESMO espirito de music_volume/sfx_volume acima (semeado do estado JA
    // carregado no boot, nao resetado por system_menu_open). Perfil UNICO
    // "default" nesta onda (ver persist_controls acima - nao ha selecao de
    // jogador na UI ainda). controls_applied_config = a MESMA leitura -
    // BASELINE (o que ja vale no jogo/disco agora, alvo do revert ao
    // descartar, ver o comentario STAGED CHANGES em system_menu.hpp); as duas
    // copias comecam IGUAIS, so controls_config (a copia de trabalho) muda com
    // remap/restaurar ate o jogador confirmar "Aplicar".
    state.controls_config = gus::platform::fs::load_controls(
        settings_dir, std::string(gus::domain::input::kDefaultProfile));
    state.controls_applied_config = state.controls_config;
    system_menu_open(state);

    int pw = 0, ph = 0;
    SDL_GetWindowSizeInPixels(window, &pw, &ph);
    if (pw < 1) pw = 1;
    if (ph < 1) ph = 1;
    float dp_ratio = static_cast<float>(pw) / 960.0f;

    // SAVE-LOAD-UI etapa 6 (FIX critico, ver o comentario grande no ramo
    // OpenSaveLoadSave/Load de handle_action mais abaixo): `ui` vive num
    // std::optional (NAO um glintfx::UiLayer direto) porque a tela de save/load
    // precisa abrir seu PROPRIO glintfx::UiLayer - e RmlUi NAO SUPORTA 2
    // contextos/UiLayer simultaneos no MESMO processo (crash real ja documentado,
    // ver o comentario historico em app/tests/battle_key_routing_test.cpp,
    // "Element meta pool not empty on shutdown" - MESMO sintoma reproduzido ao
    // vivo por este agente antes do fix). Por isso `ui` e DESTRUIDO (ui_opt.reset())
    // ANTES de abrir a tela de save/load e RECRIADO (ui_opt.emplace(...) + reload())
    // so DEPOIS dela fechar - nunca 2 instancias vivas ao mesmo tempo.
    std::optional<glintfx::UiLayer> ui_opt(glintfx::UiLayer::Config{
        /*logical_width=*/960, /*logical_height=*/540, /*load_gl=*/true,
        /*dp_ratio=*/dp_ratio});
    if (!ui_opt->ok()) {
        std::cerr << "SystemMenuLoop: glintfx::UiLayer::ok()=false (attach falhou) - "
                     "fechando o menu sem desenhar nada (degradacao segura).\n";
        return outcome;  // quit_app=false: o chamador so retoma a cena
    }

    const std::string stage = menu_stage_dir();
    ui_opt->set_asset_base_url(stage.c_str());
    std::string rml_path = write_system_menu_rml_file(state, translator);
    ui_opt->load(rml_path.c_str());
    ui_opt->set_viewport(pw, ph);
    ui_opt->set_dp_ratio(dp_ratio);

    gus::platform::render2d::Render2dGl3 backdrop(/*gl_active=*/true);

    // FUNDO REAL CONGELADO (retoque do lider via AskUserQuestion, MENU-PAUSA-
    // CONFIG-SOM): carrega a textura UMA VEZ (mesmo racional de "load_texture
    // NUNCA no frame" ja documentado pros SFX abaixo) - kInvalidTexture (path
    // vazio/asset ausente/backend headless) degrada com seguranca pra vinheta de
    // sempre (ver present_frame). Independe do reload() de estado (a imagem NAO
    // muda durante a sessao do menu).
    const gus::platform::render2d::TextureId frozen_bg_tex =
        frozen_background_png.empty()
            ? gus::platform::render2d::kInvalidTexture
            : backdrop.load_texture(frozen_background_png.c_str());

    // SFX de hover/clique (retoque ao vivo do lider): load_sfx UMA VEZ por sessao de
    // menu (a cada Esc que abre o menu de novo, ver o header - MESMO padrao de
    // hit_sfx_id em battle_preview.cpp, "load_sfx NUNCA no frame"). audio.available()
    // false (device indisponivel/CI) degrada com seguranca - play_sfx(id invalido)
    // ja e no-op, ver AudioEngine::play_sfx.
    const std::string hover_sfx_path =
        resolve_menu_sfx_path(gus::core::assets::kMenuHoverSfxFile);
    const std::string click_sfx_path =
        resolve_menu_sfx_path(gus::core::assets::kMenuClickSfxFile);
    const gus::platform::audio::SoundId hover_sfx_id = audio.load_sfx(hover_sfx_path.c_str());
    const gus::platform::audio::SoundId click_sfx_id = audio.load_sfx(click_sfx_path.c_str());

    // Reconstroi o RML/reflete no glintfx apos QUALQUER mutacao de estado (navegacao,
    // troca de tela, volume) - ver o comentario de build_system_menu_rml/
    // write_system_menu_rml_file: reload-on-change e simples e barato o bastante
    // (o menu muda so em input do jogador, nao a cada frame).
    //
    // SCROLL SEGUE A SELECAO (M2/GLINTFX-SCROLL, glintfx v0.4.0): apos QUALQUER
    // reload (navegacao por teclado UP/DOWN/W/S, entrar/sair de Controles,
    // Restaurar padrao, etc - TODOS passam por aqui), garante que a linha
    // state.controls_selected fique DENTRO do recorte visivel de `.ctrl-list`
    // via UiLayer::scroll_element_into_view - controls_scroll_target_index
    // (system_menu.hpp, POCO/testavel) decide SE deve rolar (so na tela
    // Controles, fora dos mini-dialogos Restaurar/Descartar - ver seu
    // comentario). 1 ui_opt->update() extra ANTES do scroll: o load() acabou de
    // trocar de DOCUMENTO (novo Rml::ElementDocument) - a geometria precisa de
    // 1 passo de layout assentado antes de ScrollIntoView le-la (MESMO
    // racional do "2o update() por seguranca" do self-test BUG-1 mais abaixo,
    // que ja fazia 2x present_frame() por desconfianca da mesma janela).
    // scroll_element_into_view(id, align_with_top=true) e no-op seguro quando
    // a linha ja esta visivel (RmlUi so escreve scrollTop quando precisa) -
    // por isso um UNICO ponto de chamada cobre tanto "linha nova entrando na
    // vista" (navegacao) quanto "linha que ja estava visivel" (sem custo
    // extra perceptivel).
    auto reload = [&] {
        rml_path = write_system_menu_rml_file(state, translator);
        ui_opt->load(rml_path.c_str());
        ui_opt->set_viewport(pw, ph);
        ui_opt->set_dp_ratio(dp_ratio);

        const int scroll_target = controls_scroll_target_index(state);
        if (scroll_target >= 0) {
            ui_opt->update();
            ui_opt->scroll_element_into_view(controls_item_id(scroll_target).c_str());
        }
    };

    // Desenha um frame do backdrop+UI e apresenta (MESMA sequencia do corpo do
    // loop principal abaixo) - fatorado pra ser reusado pelo flash de PRESS.
    auto present_frame = [&] {
        const gus::core::spatial::Rect cam{0.0f, 0.0f, static_cast<float>(pw),
                                            static_cast<float>(ph)};
        backdrop.begin_frame(cam, pw, ph);  // clear + vinheta radial (fallback abstrato)
        if (frozen_bg_tex != gus::platform::render2d::kInvalidTexture) {
            // FUNDO REAL CONGELADO (decisao do lider): cobre a vinheta com a CENA
            // REAL da cidade (1 frame estatico, capturado pela Maestro ANTES de
            // abrir - ver Maestro::open_pause_from_city/SdlWindow::capture_frame_
            // to_png), full-screen e opaca - mesmo padrao de Chrono Trigger/Zelda/
            // Stardew Valley (o mundo "pausa" atras da UI). O #sysmenu-scrim do
            // RML (system_menu_rml.cpp, ~66% preto) segue desenhado por CIMA disto
            // pelo glintfx (ui_opt->render() abaixo) - a legibilidade do painel nao muda.
            backdrop.draw_textured_rect(
                cam, frozen_bg_tex, gus::platform::render2d::UvRect{0.0f, 0.0f, 1.0f, 1.0f},
                gus::platform::render2d::DrawColor{1.0f, 1.0f, 1.0f, 1.0f});
        }
        backdrop.end_frame();
        ui_opt->update();
        ui_opt->render();
        SDL_GL_SwapWindow(window);
    };

    // EFEITO DE PRESS (ver comentario do topo do arquivo): renderiza a tela
    // `pre_action_state` (snapshot tirado ANTES da mutacao que ja aconteceu em
    // `state`) com o item `item_index` marcado ".pressed", por ~100ms (4 frames
    // de ~25ms - varios swaps garantem que o compositor/driver apresente pelo
    // menos 1 frame do flash mesmo sob vsync), e SO DEPOIS devolve - o chamador
    // segue com handle_action/reload usando o `state` JA MUTADO (a transicao
    // real acontece normalmente no proximo reload/return). SOM DE CLIQUE (retoque
    // ao vivo do lider): dispara AQUI, no MESMO choke-point do flash visual - e o
    // UNICO lugar chamado tanto por confirmacao de TECLADO (Enter/Espaco) quanto
    // por CLIQUE de mouse (ver is_confirming mais abaixo), entao 1 play_sfx cobre
    // os dois canais sem duplicar logica.
    auto flash_pressed = [&](const SystemMenuState& pre_action_state, int item_index) {
        audio.play_sfx(click_sfx_id);
        rml_path = write_system_menu_rml_file(pre_action_state, translator, item_index);
        ui_opt->load(rml_path.c_str());
        ui_opt->set_viewport(pw, ph);
        ui_opt->set_dp_ratio(dp_ratio);
        for (int frame = 0; frame < 4; ++frame) {
            present_frame();
            SDL_Delay(25);
        }
    };

    int drag_item = -1;      // -1 = nenhum arrasto em curso; 0=Music, 1=Sfx
    int hovered_index = -1;  // -1 = mouse fora de qualquer item navegavel (ver
                             // handle_mouse_motion abaixo - edge-detect do SFX de hover)

    // HOVER (mouse) - PEDIDO 2a/2b: injeta o MouseMove no glintfx (visual :hover
    // NATIVO, MESMO pipeline do cockpit da batalha) e faz o hit-test/edge-detect do
    // SOM de hover (current_hover_index + system_menu_hover_entered_new_item, ambas
    // POCO testadas em system_menu_test.cpp). Fatorada em lambda pra ser chamada
    // tanto pelo SDL_EVENT_MOUSE_MOTION real (abaixo) quanto pelo self-test
    // headless (GUSWORLD_SYSMENU_HOVER_SELFTEST, ver mais abaixo) - MESMO caminho
    // de codigo prova o comportamento real, sem duplicar.
    auto handle_mouse_motion = [&](float mx, float my) {
        glintfx::UiEvent hover_ev{};
        hover_ev.type = glintfx::UiEvent::Type::MouseMove;
        hover_ev.x = mx;
        hover_ev.y = my;
        ui_opt->process_event(hover_ev);

        const int new_hover = current_hover_index(*ui_opt, state, mx, my);
        if (system_menu_hover_entered_new_item(hovered_index, new_hover)) {
            audio.play_sfx(hover_sfx_id);
        }
        hovered_index = new_hover;

        if (drag_item >= 0) {
            const std::string id = track_id_for_item(drag_item);
            const glintfx::ElementBox box = ui_opt->get_element_box(id.c_str());
            if (box.found && box.w > 0.0f) {
                const float ratio = (mx - box.x) / box.w;
                system_menu_set_slider_ratio(state, drag_item, ratio);
                apply_and_persist(state, audio, settings_dir);
                reload();
            }
        }
    };

    // Roteia UMA action (vinda do teclado OU de um clique de mouse) pro mesmo
    // efeito de mundo (persistir volume, recarregar o RML) - compartilhado
    // pelos dois canais de entrada pra nao duplicar a logica de
    // Continue/RequestQuit/VolumeChanged/Navigated. Devolve true se o CHAMADOR
    // deve retornar `outcome` na hora (Continue/RequestQuit ja setaram outcome).
    auto handle_action = [&](SystemMenuAction action) -> bool {
        if (action == SystemMenuAction::Continue) {
            return true;  // quit_app=false: retoma a cena
        }
        if (action == SystemMenuAction::RequestQuit) {
            outcome.quit_app = true;
            return true;
        }
        if (action == SystemMenuAction::VolumeChanged) {
            apply_and_persist(state, audio, settings_dir);
        }
        // ControlsChanged (M2 STAGED CHANGES): remap/restaurar-padrao mutou SO
        // a COPIA DE TRABALHO (controls_config) - controls_dirty ja ficou true
        // do lado puro (system_menu.cpp). NAO persiste mais em disco aqui (era
        // o modelo antigo "aplica na hora") - so ControlsApplied persiste,
        // abaixo. O reload() generico no fim ja cobre a UI (botao Aplicar
        // acende, linha mostra o novo binding).
        if (action == SystemMenuAction::ControlsApplied) {
            // UNICO ponto de escrita em disco da tela Controles agora: a copia
            // de trabalho (ja promovida a baseline em controls_applied_config
            // pelo lado puro) e o que persiste.
            persist_controls(state, settings_dir);
        }
        // SAVE-LOAD-UI etapa 6: "Salvar"/"Carregar" confirmados no Pause abrem a
        // tela REAL, ANINHADA no MESMO contexto GL (state.screen continua Pause -
        // ver o comentario grande em system_menu.hpp). Ao voltar: BackToPause so
        // recarrega o Pause (fica no while(true) deste loop); ClosedAfterLoad
        // fecha o menu de pausa INTEIRO igual a Continuar (o Load ja aplicou no
        // jogo vivo, via apply_loaded_save_data, ANTES de devolver); QuitApp
        // propaga o fechamento da janela.
        if (action == SystemMenuAction::OpenSaveLoadSave ||
            action == SystemMenuAction::OpenSaveLoadLoad) {
            const SaveLoadMode mode = (action == SystemMenuAction::OpenSaveLoadSave)
                                           ? SaveLoadMode::Save
                                           : SaveLoadMode::Load;
            // FIX CRITICO (crash real reproduzido ao vivo por este agente, MESMO
            // sintoma ja documentado em battle_key_routing_test.cpp - "Element
            // meta pool not empty on shutdown"): RmlUi NAO SUPORTA 2 UiLayer
            // simultaneos no processo. DESTROI o UiLayer do Pause ANTES de abrir
            // o da tela de save/load (que cria o SEU PROPRIO) - nunca 2 vivos ao
            // mesmo tempo.
            ui_opt.reset();
            const SaveLoadLoopExit exit = run_save_load_menu_loop_gl_current(
                window, translator, mode, saves_dir, build_current_save_data,
                apply_loaded_save_data, frozen_background_png);
            switch (exit) {
                case SaveLoadLoopExit::QuitApp:
                    outcome.quit_app = true;
                    return true;
                case SaveLoadLoopExit::ClosedAfterLoad:
                    return true;  // quit_app=false: MESMO efeito de Continuar
                case SaveLoadLoopExit::BackToPause:
                    // RECRIA o UiLayer do Pause (a tela de save/load ja destruiu
                    // o dela ao retornar) ANTES do reload() generico abaixo, que
                    // dereferencia ui_opt.
                    ui_opt.emplace(glintfx::UiLayer::Config{
                        /*logical_width=*/960, /*logical_height=*/540,
                        /*load_gl=*/true, /*dp_ratio=*/dp_ratio});
                    ui_opt->set_asset_base_url(stage.c_str());
                    break;  // so recarrega o Pause abaixo, segue no while(true)
            }
        }
        // None/Navigated: o ESTADO pode ter mudado mesmo assim (navegacao/foco
        // move e devolve None, ou trocou de tela e devolve Navigated) - reload
        // sempre.
        reload();
        return false;
    };

    // NAVEGACAO POR TECLADO (nao confirm-key: setas/WASD/LEFT/RIGHT/ESC) - SOM
    // DE HOVER PARIDADE (retoque ao vivo do lider, pos-tela Controles
    // aprovada): move a selecao (system_menu_key_down) e, SO quando a selecao
    // mudou pra um item NOVO na MESMA tela (edge-detect via
    // system_menu_keyboard_focus_index + system_menu_hover_entered_new_item -
    // AS MESMAS 2 funcoes puras que handle_mouse_motion acima ja usa pro
    // hover de MOUSE, NADA duplicado), toca hover_sfx no MESMO choke-point
    // (audio.play_sfx(hover_sfx_id)) que o mouse ja usa. O guard `state.screen
    // == screen_before` barra TROCA DE TELA (ex.: ESC subindo um nivel, ou
    // ESC abrindo o mini-dialogo de descarte NAO muda de tela, entao esse
    // caso especifico passa pelo guard normalmente): trocar de tela NAO e
    // "navegar pra um item novo" (ela ja tem seu proprio feedback - o flash
    // .pressed de Enter/click, quando aplicavel; ESC hoje nao tem flash, fora
    // de escopo mudar aqui) e comparar indices ENTRE telas diferentes nao
    // faz sentido (pause_selected=2 e "Configuracoes", config_categories_
    // selected=2 e "Controles" - numeros iguais, significados diferentes).
    // LEFT/RIGHT/A/D no slider da tela Audio (ajusta volume, NAO navega -
    // audio_selected fica intocado) naturalmente NAO dispara (indice
    // antes==depois, edge-detect nao acha "item novo"). Fatorada em lambda
    // pra ser o UNICO choke-point tanto do SDL_EVENT_KEY_DOWN real (ramo
    // nao-confirm mais abaixo) quanto do self-test headless
    // (GUSWORLD_SYSMENU_KEYBOARD_HOVER_SELFTEST, ver mais abaixo) - MESMA
    // receita de handle_mouse_motion/flash_pressed acima (1 unico lugar,
    // reusado pelos dois canais/caminhos, sem duplicar logica de som).
    // Devolve o mesmo bool de handle_action (true = o CHAMADOR deve retornar
    // `outcome` na hora).
    auto handle_navigation_key = [&](SDL_Keycode key) -> bool {
        const SystemMenuScreen screen_before = state.screen;
        const int kb_index_before = system_menu_keyboard_focus_index(state);
        const SystemMenuAction action = system_menu_key_down(state, key);
        if (state.screen == screen_before) {
            const int kb_index_after = system_menu_keyboard_focus_index(state);
            if (system_menu_hover_entered_new_item(kb_index_before, kb_index_after)) {
                audio.play_sfx(hover_sfx_id);
            }
        }
        return handle_action(action);
    };

    // Confirma se `action` merece o flash de PRESS (ver topo do arquivo): SO as
    // acoes que de fato "acionam uma opcao" (pill/categoria/Voltar/Aplicar) -
    // nunca VolumeChanged (drag de slider nao pisca) nem None (nao aconteceu
    // nada). ControlsChanged (M2) SOMA aqui: confirmar "Sim" no
    // restaurar-padrao e uma acao destrutiva (reseta a copia de trabalho) -
    // merece o mesmo flash de confirmacao das demais (entrar em modo CAPTURA
    // tambem devolve None, entao nao pisca - o feedback dela e a propria linha
    // ciano "Pressione uma tecla...", ja imediato via reload()).
    // ControlsApplied (M2 STAGED CHANGES) SOMA aqui tambem: clicar/confirmar
    // "Aplicar" e a acao mais importante da tela (persiste de fato) - merece o
    // mesmo flash.
    auto is_confirming = [](SystemMenuAction action) {
        return action == SystemMenuAction::Continue ||
               action == SystemMenuAction::RequestQuit ||
               action == SystemMenuAction::Navigated ||
               action == SystemMenuAction::ControlsChanged ||
               action == SystemMenuAction::ControlsApplied;
    };

    // DIAGNOSTICO/PROVA (SOM DE HOVER/CLIQUE): GUSWORLD_SYSMENU_HOVER_SELFTEST=1
    // entra na tela Pause (ja aberta acima), MOVE o mouse SINTETICO sequencialmente
    // pelos 4 pills (item0->1->2->3->0 de novo, provando que sair-e-voltar redispara
    // o hover - MESMA logica de system_menu_hover_entered_new_item) via
    // handle_mouse_motion (o MESMO codigo do SDL_EVENT_MOUSE_MOTION real acima, sem
    // duplicar), e SIMULA 1 clique confirmando "Continuar" (system_menu_click_option
    // + flash_pressed, o MESMO caminho de um clique real) - tudo SEM SDL_PushEvent,
    // SEM input real, SEM tocar hardware de audio (o CHAMADOR decide device_active
    // do AudioEngine - este self-test so EXERCITA os call-sites; sfx_play_count() e
    // o hook de prova, ver AudioEngine::sfx_play_count). Imprime a contagem
    // observada e retorna ANTES do loop interativo (bypassa por completo - nunca
    // abre pra input real). MESMO espirito de GUSWORLD_BATTLE_HOVER_SELFTEST em
    // battle_preview.cpp (auto-contido, headless, Xvfb).
    const char* sysmenu_selftest = std::getenv("GUSWORLD_SYSMENU_HOVER_SELFTEST");
    if (sysmenu_selftest != nullptr && sysmenu_selftest[0] != '\0') {
        present_frame();  // assenta o layout (get_element_box precisa de 1 update())

        // Centro de cada pill (item0..3), na ORDEM 0,1,2,3,0 (o ultimo "0" de novo
        // prova o re-trigger apos sair pro item 3).
        const int hover_sequence[] = {0, 1, 2, 3, 0};
        for (const int item : hover_sequence) {
            const glintfx::ElementBox box = ui_opt->get_element_box(pause_item_id(item).c_str());
            const float cx = box.found ? box.x + box.w * 0.5f : -1.0f;
            const float cy = box.found ? box.y + box.h * 0.5f : -1.0f;
            handle_mouse_motion(cx, cy);
            present_frame();
        }
        std::cout << "SystemMenuLoop: [selftest] hover_sfx_play_count apos 5 "
                     "moves (0,1,2,3,0 - 5 entradas NOVAS esperadas) = "
                  << audio.sfx_play_count() << "\n";

        const int click_baseline = static_cast<int>(audio.sfx_play_count());
        const SystemMenuState pre_click_state = state;
        const SystemMenuAction click_action =
            system_menu_click_option(state, static_cast<int>(PauseItem::Continue));
        if (is_confirming(click_action)) {
            flash_pressed(pre_click_state, static_cast<int>(PauseItem::Continue));
        }
        std::cout << "SystemMenuLoop: [selftest] click_sfx disparou "
                  << (static_cast<int>(audio.sfx_play_count()) - click_baseline)
                  << "x (esperado 1) - total sfx_play_count()=" << audio.sfx_play_count()
                  << "\n";
        (void)handle_action(click_action);  // fecha o menu (Continuar) - outcome ja refletido
        return outcome;
    }

    // DIAGNOSTICO/PROVA (PARIDADE SOM DE HOVER TECLADO x MOUSE, retoque ao vivo
    // do lider pos-tela Controles aprovada):
    // GUSWORLD_SYSMENU_KEYBOARD_HOVER_SELFTEST=1 prova que handle_navigation_key
    // (MESMO caminho de codigo do SDL_EVENT_KEY_DOWN nao-confirm real acima) toca
    // hover_sfx EXATAMENTE quando a navegacao por TECLADO move a selecao pra um
    // item NOVO na MESMA tela (edge-detect), e NAO toca em (a) no-op de
    // navegacao (tecla sem efeito na selecao, ex. LEFT em Pause), (b) ajuste de
    // slider (LEFT/RIGHT no Audio muda volume, nao indice), nem (c) troca de
    // TELA (ESC subindo de nivel) - tudo SEM SDL_PushEvent/input real, headless,
    // SEM tocar hardware de audio, sfx_play_count() como hook de prova. MESMO
    // espirito de GUSWORLD_SYSMENU_HOVER_SELFTEST acima (canal MOUSE) - aqui o
    // canal e TECLADO.
    const char* keyboard_hover_selftest =
        std::getenv("GUSWORLD_SYSMENU_KEYBOARD_HOVER_SELFTEST");
    if (keyboard_hover_selftest != nullptr && keyboard_hover_selftest[0] != '\0') {
        present_frame();  // assenta o layout (mesma cautela dos demais self-tests)

        // Pause (4 itens, item 0 "Continuar" selecionado por system_menu_open ja
        // chamado acima): 3x DOWN move 0->1->2->3 - 3 itens NOVOS, 3 sons.
        const unsigned int baseline_down3 = audio.sfx_play_count();
        for (int i = 0; i < 3; ++i) {
            if (handle_navigation_key(SDLK_DOWN)) return outcome;
        }
        const unsigned int after_down3 = audio.sfx_play_count() - baseline_down3;
        std::cout << "SystemMenuLoop: [selftest][KEYBOARD-HOVER] Pause 3x DOWN: "
                     "pause_selected="
                  << state.pause_selected << " (esperado 3); hover_sfx tocou "
                  << after_down3 << "x (esperado 3) - "
                  << (state.pause_selected == 3 && after_down3 == 3 ? "OK" : "FALHOU")
                  << "\n";

        // NO-OP: LEFT nao tem efeito de navegacao em Pause (handle_pause_key so
        // trata UP/DOWN/ESC/RETURN/SPACE - default case, sem mudar
        // pause_selected) - selecao intocada, hover_sfx NAO deve tocar.
        const unsigned int baseline_left = audio.sfx_play_count();
        if (handle_navigation_key(SDLK_LEFT)) return outcome;
        const unsigned int after_left = audio.sfx_play_count() - baseline_left;
        std::cout << "SystemMenuLoop: [selftest][KEYBOARD-HOVER] Pause LEFT "
                     "(no-op, sem efeito de navegacao): pause_selected="
                  << state.pause_selected << " (esperado 3, intocado); hover_sfx "
                     "tocou "
                  << after_left << "x (esperado 0) - "
                  << (state.pause_selected == 3 && after_left == 0 ? "OK" : "FALHOU")
                  << "\n";

        // WRAP: mais 1 DOWN (3->0) - AINDA um item NOVO (diferente do anterior) -
        // edge-detect continua disparando atraves do wrap-around (wrap_move nunca
        // repete o indice anterior quando count>1).
        const unsigned int baseline_wrap = audio.sfx_play_count();
        if (handle_navigation_key(SDLK_DOWN)) return outcome;
        const unsigned int after_wrap = audio.sfx_play_count() - baseline_wrap;
        std::cout << "SystemMenuLoop: [selftest][KEYBOARD-HOVER] Pause DOWN (wrap "
                     "3->0): pause_selected="
                  << state.pause_selected << " (esperado 0); hover_sfx tocou "
                  << after_wrap << "x (esperado 1) - "
                  << (state.pause_selected == 0 && after_wrap == 1 ? "OK" : "FALHOU")
                  << "\n";

        // TROCA DE TELA nao conta como navegacao: forca "Configuracoes"
        // selecionado e ENTER (confirm-key, FORA de handle_navigation_key - MESMO
        // roteamento do while(true) de producao, ver o ramo is_confirm_key acima)
        // pra entrar em ConfigCategories.
        state.pause_selected = static_cast<int>(PauseItem::Settings);
        (void)handle_action(system_menu_key_down(state, SDLK_RETURN));
        const bool entered_config = state.screen == SystemMenuScreen::ConfigCategories;

        // ESC (NAO e confirm-key - passa por handle_navigation_key igual a
        // qualquer seta) sobe de volta pra Pause: TROCA DE TELA - o guard
        // `state.screen == screen_before` (dentro de handle_navigation_key) barra
        // o hover_sfx mesmo que os indices numericos difiram entre as duas telas
        // (comparar entre telas diferentes nao faz sentido, ver o comentario da
        // lambda acima).
        const unsigned int baseline_esc = audio.sfx_play_count();
        if (handle_navigation_key(SDLK_ESCAPE)) return outcome;
        const unsigned int after_esc = audio.sfx_play_count() - baseline_esc;
        std::cout << "SystemMenuLoop: [selftest][KEYBOARD-HOVER] ESC troca de "
                     "tela (ConfigCategories->Pause, entrou_em_config="
                  << entered_config << "): screen_apos="
                  << (state.screen == SystemMenuScreen::Pause ? "Pause" : "outro")
                  << " hover_sfx tocou " << after_esc
                  << "x (esperado 0, guard de tela barra) - "
                  << (entered_config && state.screen == SystemMenuScreen::Pause &&
                              after_esc == 0
                          ? "OK"
                          : "FALHOU")
                  << "\n";

        // SLIDER (Audio): LEFT/RIGHT ajustam volume (VolumeChanged), NAO navegam
        // (audio_selected fica intocado) - hover_sfx NAO deve tocar. Entra em
        // Configuracoes (Audio ja selecionado, indice 0 - reset por
        // handle_pause_key/SDLK_RETURN acima) -> Audio (Musica, indice 0), ambos
        // ENTER (confirm-key, fora de handle_navigation_key).
        (void)handle_action(system_menu_key_down(state, SDLK_RETURN));  // -> ConfigCategories
        (void)handle_action(system_menu_key_down(state, SDLK_RETURN));  // -> Audio
        const bool entered_audio = state.screen == SystemMenuScreen::Audio;
        const unsigned int baseline_slider = audio.sfx_play_count();
        if (handle_navigation_key(SDLK_LEFT)) return outcome;
        const unsigned int after_slider = audio.sfx_play_count() - baseline_slider;
        std::cout << "SystemMenuLoop: [selftest][KEYBOARD-HOVER] Audio LEFT "
                     "(ajusta volume, nao navega, entrou_em_audio="
                  << entered_audio << "): audio_selected=" << state.audio_selected
                  << " (esperado 0, Musica); hover_sfx tocou " << after_slider
                  << "x (esperado 0, e slider, nao navegacao) - "
                  << (entered_audio && state.audio_selected == 0 && after_slider == 0
                          ? "OK"
                          : "FALHOU")
                  << "\n";

        return outcome;
    }

    // DIAGNOSTICO/PROVA (TELA CONTROLES, M2, 3 bugs ao vivo reportados pelo lider):
    // GUSWORLD_SYSMENU_CONTROLS_SELFTEST=1 entra em Controles (navegacao REAL via
    // system_menu_key_down, MESMO caminho de codigo do teclado de producao - SEM
    // SDL_PushEvent, MESMO espirito do GUSWORLD_SYSMENU_HOVER_SELFTEST acima) e
    // MEDE/EXERCITA os 3 pontos que quebraram:
    //   BUG-1 (painel estourava a viewport, rodape cortado): confere que o
    //     bottom do #sysmenu-panel cabe dentro de `ph` (achado por probe Xvfb :99:
    //     `top:90dp` herdado + `.ctrl-list{height:270dp}` somavam ~525dp de
    //     conteudo, estourando o canvas de 540dp - FIX: `.wide{top:20dp}` +
    //     `.ctrl-list{height:220dp}`, ver os comentarios no <style> acima).
    //   BUG-A (achado NOVO nesta investigacao, causa raiz do "mouse nao
    //     seleciona nada"): `.ctrl-row` (display:flex, filho de `.ctrl-list` que
    //     tem `overflow-y:auto`) colapsava pra ~16px de largura (so o padding -
    //     o conteudo flex ficava com largura 0) em vez de ~750px - a caixa de
    //     hit-test (get_element_box, MESMA que o mouse do loop usa) ficava uma
    //     fresta de 12dp perto da borda esquerda; qualquer clique no resto da
    //     linha (onde o jogador VE o texto, que so visualmente transbordava a
    //     caixa colapsada) errava o hit-test. FIX: `box-sizing:border-box;
    //     width:558dp` explicito em .ctrl-row (sidesteps o bug de
    //     overflow-auto+flex sem width explicita - width:100% tambem falhava,
    //     so um comprimento ABSOLUTO fixou). Confere que a largura medida e
    //     grande o bastante pro clique acertar a linha inteira.
    //   BUG-2 (coluna Teclado mostrava "-" em vez de "W"): NAO reproduziu nem no
    //     teste unitario (system_menu_rml_test.cpp) nem neste self-test ao vivo -
    //     o keycap correto ("W") sempre apareceu no RML gerado com
    //     default_controls(). Mantido aqui como guarda de regressao (BUG-A podia
    //     ser a causa visual: com a linha colapsada a 16px, "W"/"-" ficavam
    //     espremidos quase um em cima do outro perto da borda esquerda - com a
    //     largura corrigida, as 3 colunas ficam nos offsets certos, sem overlap).
    const char* controls_selftest = std::getenv("GUSWORLD_SYSMENU_CONTROLS_SELFTEST");
    if (controls_selftest != nullptr && controls_selftest[0] != '\0') {
        // Pause -> ConfigCategories -> Controls (navegacao REAL via system_menu_key_down).
        system_menu_key_down(state, SDLK_DOWN);    // Continue->Save
        system_menu_key_down(state, SDLK_DOWN);    // Save->Settings
        system_menu_key_down(state, SDLK_RETURN);  // entra ConfigCategories
        system_menu_key_down(state, SDLK_DOWN);    // Audio->Video
        system_menu_key_down(state, SDLK_DOWN);    // Video->Controls
        system_menu_key_down(state, SDLK_RETURN);  // entra Controls
        reload();
        present_frame();
        present_frame();  // 2o update() por seguranca (layout assentado)

        // BUG-1: o painel (e o rodape Restaurar/Voltar) tem que caber na viewport.
        const glintfx::ElementBox panel = ui_opt->get_element_box("sysmenu-panel");
        const glintfx::ElementBox back_btn =
            ui_opt->get_element_box(controls_item_id(kControlsBackIndex).c_str());
        std::cout << "SystemMenuLoop: [selftest][BUG-1] viewport ph=" << ph
                  << " panel_bottom=" << (panel.y + panel.h)
                  << " (esperado <= " << ph << ") - "
                  << ((panel.y + panel.h) <= static_cast<float>(ph) ? "OK" : "FALHOU")
                  << "\n";
        std::cout << "SystemMenuLoop: [selftest][BUG-1] botao Voltar do rodape: found="
                  << back_btn.found << " bottom=" << (back_btn.y + back_btn.h)
                  << " (esperado found=1 e <= " << ph << ") - "
                  << (back_btn.found && (back_btn.y + back_btn.h) <= static_cast<float>(ph)
                          ? "OK"
                          : "FALHOU")
                  << "\n";

        // BUG-A: a caixa de hit-test da linha 0 precisa cobrir a linha INTEIRA
        // (nao so os ~16px de padding do bug original).
        const glintfx::ElementBox row0 = ui_opt->get_element_box(controls_item_id(0).c_str());
        std::cout << "SystemMenuLoop: [selftest][BUG-A] largura hit-test da linha 0: w="
                  << row0.w << " (esperado > 400px, era ~16px no bug) - "
                  << (row0.w > 400.0f ? "OK" : "FALHOU") << "\n";

        // BUG-2: o keycap de move_forward (1a linha) tem que mostrar "W" (default_controls()).
        {
            std::ifstream rml_in(rml_path);
            std::ostringstream ss;
            ss << rml_in.rdbuf();
            const std::string txt = ss.str();
            const bool has_w_keycap = txt.find(">W<") != std::string::npos;
            std::cout << "SystemMenuLoop: [selftest][BUG-2] keycap 'W' de move_forward "
                         "presente no RML: "
                      << (has_w_keycap ? "OK" : "FALHOU") << "\n";
        }

        // BUG-3 (teclado): DOWN 3x tem que avancar controls_selected 0->3 (WRAP
        // testado a parte em system_menu_test.cpp - aqui so prova o caminho REAL
        // do loop, com reload() reconstruindo o RML a cada passo).
        for (int i = 0; i < 3; ++i) {
            system_menu_key_down(state, SDLK_DOWN);
        }
        reload();
        present_frame();
        std::cout << "SystemMenuLoop: [selftest][BUG-3 teclado] apos 3x DOWN: "
                     "controls_selected="
                  << state.controls_selected << " (esperado 3) - "
                  << (state.controls_selected == 3 ? "OK" : "FALHOU") << "\n";

        // BUG-3 (mouse): hit-test + clique na linha 5 (MESMO par get_element_box +
        // system_menu_click_option que o handler de SDL_EVENT_MOUSE_BUTTON_DOWN
        // do while(true) de producao usa - so sem passar pela fila de eventos).
        const glintfx::ElementBox row5 = ui_opt->get_element_box(controls_item_id(5).c_str());
        const SystemMenuAction click_action = system_menu_click_option(state, 5);
        std::cout << "SystemMenuLoop: [selftest][BUG-3 mouse] hit-test linha 5: found="
                  << row5.found << " w=" << row5.w << "; apos clique: controls_selected="
                  << state.controls_selected << " controls_capturing="
                  << state.controls_capturing << " (esperado selected=5, capturing=1) - "
                  << (state.controls_selected == 5 && state.controls_capturing ? "OK"
                                                                                : "FALHOU")
                  << "\n";
        (void)click_action;

        // BUG-A (Voltar morto pro mouse, achado NOVO nesta investigacao - ver o
        // comentario extenso de controls_row_visible_in_list em
        // system_menu.hpp): cancela a captura aberta pelo teste BUG-3 acima (Esc,
        // nao muda config) antes de prosseguir.
        system_menu_controls_capture_key(state, /*is_escape=*/true, 0);
        {
            const glintfx::ElementBox list_box = ui_opt->get_element_box(kControlsListId);
            const glintfx::ElementBox raw_row6 =
                ui_opt->get_element_box(controls_item_id(6).c_str());
            std::cout << "SystemMenuLoop: [selftest][BUG-A] linha 6 (rolada pra fora "
                         "da vista) - caixa REAL: y="
                      << raw_row6.y << " h=" << raw_row6.h << "; recorte visivel de "
                         "ctrl-list: y="
                      << list_box.y << " h=" << list_box.h << " (linha 6 fora do "
                         "recorte, MESMA geometria que roubava o clique do rodape "
                         "antes do fix)\n";

            // Centro da caixa REAL de Voltar (kControlsBackIndex, rodape - sempre
            // fora da lista rolavel) - a coordenada de clique que o jogador
            // fisicamente usaria.
            const glintfx::ElementBox raw_back =
                ui_opt->get_element_box(controls_item_id(kControlsBackIndex).c_str());
            const float cx = raw_back.found ? raw_back.x + raw_back.w * 0.5f : -1.0f;
            const float cy = raw_back.found ? raw_back.y + raw_back.h * 0.5f : -1.0f;

            // REPLICA o loop de producao (SDL_EVENT_MOUSE_BUTTON_DOWN, ramo
            // Controles/navegacao normal acima): itera 0..kControlsItemCount-1 EM
            // ORDEM, filtra pelo recorte visivel (filter_offscreen_controls_row) e
            // para no PRIMEIRO que bater - prova QUAL indice de fato vence o
            // clique na posicao REAL de Voltar (antes do fix, era o indice 6, nao
            // o 31 esperado - ver o comentario do header).
            int winner = -1;
            for (int item = 0; item < kControlsItemCount; ++item) {
                const glintfx::ElementBox raw =
                    ui_opt->get_element_box(controls_item_id(item).c_str());
                const glintfx::ElementBox box =
                    filter_offscreen_controls_row(item, raw, list_box);
                if (hit_test(box, cx, cy)) {
                    winner = item;
                    break;
                }
            }
            const SystemMenuAction back_action =
                (winner == kControlsBackIndex) ? system_menu_click_option(state, winner)
                                                : SystemMenuAction::None;
            const bool ok = winner == kControlsBackIndex &&
                             state.screen == SystemMenuScreen::ConfigCategories &&
                             back_action == SystemMenuAction::Navigated;
            std::cout << "SystemMenuLoop: [selftest][BUG-A] clique na posicao REAL de "
                         "Voltar (loop de producao replicado 0.."
                      << (kControlsItemCount - 1) << "): indice vencedor=" << winner
                      << " (esperado " << kControlsBackIndex << ") screen_apos="
                      << (state.screen == SystemMenuScreen::ConfigCategories
                              ? "ConfigCategories"
                              : "outro")
                      << " action=" << static_cast<int>(back_action)
                      << " (esperado screen=ConfigCategories action=Navigated) - "
                      << (ok ? "OK" : "FALHOU") << "\n";
        }

        // GLINTFX-SCROLL (M2, bump v0.3.1 -> v0.4.0): a v0.4.0 entrega scroll DE
        // VERDADE em embed mode - antes disso `.ctrl-list` nunca rolava (scrollTop
        // sempre 0), so ~6 das 30 actions eram alcancaveis (ver BUG-A acima, que so
        // filtrava o HIT-TEST das linhas invisiveis, sem trazer nenhuma pra vista).
        // Re-entra em Controles (config_categories_selected ainda aponta pra ela -
        // ver o comentario de heranca de selecao no topo do arquivo/header, so 1
        // RETURN, sem repetir os DOWN/DOWN - Voltar em leave_controls_screen_or_
        // confirm_discard nunca mexe em config_categories_selected).
        system_menu_key_down(state, SDLK_RETURN);
        reload();
        present_frame();
        present_frame();

        // (b) WHEEL: a roda rola o elemento em HOVER (nao o com foco - ver o
        // comentario extenso de UiEvent::Type::MouseWheel no ui_event.hpp
        // vendorizado) - posiciona o cursor sintetico sobre `.ctrl-list`
        // (handle_mouse_motion, MESMO caminho do SDL_EVENT_MOUSE_MOTION real) e
        // mede scroll_top ANTES/DEPOIS de um MouseWheel sintetico
        // (system_menu_wheel_delta_to_rmlui, MESMA conversao do loop de producao).
        // Roda ANTES da navegacao por teclado abaixo (lista ainda no topo,
        // scrollTop=0 - controls_selected==0 acabou de reentrar - garante espaco
        // pra rolar PRA BAIXO sem bater no limite do fim do conteudo).
        {
            const glintfx::ElementBox list_box = ui_opt->get_element_box(kControlsListId);
            const float hover_x = list_box.found ? list_box.x + list_box.w * 0.5f : -1.0f;
            const float hover_y = list_box.found ? list_box.y + list_box.h * 0.5f : -1.0f;
            handle_mouse_motion(hover_x, hover_y);

            float scroll_before = -1.0f;
            ui_opt->get_element_scroll_top(kControlsListId, scroll_before);

            const float wheel_dy =
                system_menu_wheel_delta_to_rmlui(/*sdl_wheel_y=*/-3.0f, /*flipped=*/false);
            glintfx::UiEvent wheel_ev{};
            wheel_ev.type = glintfx::UiEvent::Type::MouseWheel;
            wheel_ev.x = 0.0f;
            wheel_ev.y = wheel_dy;
            ui_opt->process_event(wheel_ev);
            present_frame();

            float scroll_after = -1.0f;
            ui_opt->get_element_scroll_top(kControlsListId, scroll_after);
            std::cout << "SystemMenuLoop: [selftest][GLINTFX-SCROLL][wheel] "
                         "scroll_top antes="
                      << scroll_before << " depois=" << scroll_after
                      << " (esperado depois > antes - wheel_dy=" << wheel_dy << ") - "
                      << (scroll_after > scroll_before ? "OK" : "FALHOU") << "\n";
        }

        // (a) TECLADO: navega ATE uma action perto do FIM da lista (25a, dentro de
        // kControlsActionCount=30) - PROVA que scroll_element_into_view (chamado
        // DENTRO de reload(), ver seu comentario) traz a linha selecionada pra
        // dentro do recorte visivel de `.ctrl-list`, mesmo ela comecando MUITO
        // alem das ~6 linhas visiveis por vez (220dp de altura). reload() a CADA
        // DOWN (nao 1 so no fim) - MESMO caminho incremental que o loop de
        // producao roda a cada tecla real.
        for (int i = 0; i < 25; ++i) {
            system_menu_key_down(state, SDLK_DOWN);
            reload();
        }
        present_frame();
        present_frame();
        {
            const glintfx::ElementBox list_box = ui_opt->get_element_box(kControlsListId);
            const glintfx::ElementBox row25 = ui_opt->get_element_box(controls_item_id(25).c_str());
            const bool visible = row25.found && list_box.found &&
                                  controls_row_visible_in_list(row25.y, row25.h,
                                                                list_box.y, list_box.h);
            std::cout << "SystemMenuLoop: [selftest][GLINTFX-SCROLL][teclado] apos "
                         "DOWN x25: controls_selected="
                      << state.controls_selected << " (esperado 25); linha 25 - y="
                      << row25.y << " h=" << row25.h << "; recorte de ctrl-list - y="
                      << list_box.y << " h=" << list_box.h << " - "
                      << (state.controls_selected == 25 && visible ? "OK" : "FALHOU")
                      << "\n";
        }

        // (c) RECONCILIACAO com o hit-test de mouse: a linha 25, agora DENTRO do
        // recorte visivel (scroll_element_into_view ja rolou pra ela), tem que
        // continuar clicavel na sua posicao REAL (JA ROLADA) - get_element_box
        // reflete a geometria POS-scroll (a mesma que filter_offscreen_controls_
        // row/controls_row_visible_in_list consultam); nao mudou algoritmo, so a
        // geometria que ele filtra passou a ser DINAMICA (antes do scroll de
        // verdade, linhas fora da vista tinham uma posicao FIXA "de coluna longa"
        // que podia coincidir com o rodape - ver BUG-A acima; agora refletem
        // onde a linha REALMENTE esta, dentro ou fora do recorte).
        {
            const glintfx::ElementBox list_box = ui_opt->get_element_box(kControlsListId);
            const glintfx::ElementBox row25 = ui_opt->get_element_box(controls_item_id(25).c_str());
            const float cx = row25.found ? row25.x + row25.w * 0.5f : -1.0f;
            const float cy = row25.found ? row25.y + row25.h * 0.5f : -1.0f;

            int winner = -1;
            for (int item = 0; item < kControlsItemCount; ++item) {
                const glintfx::ElementBox raw = ui_opt->get_element_box(controls_item_id(item).c_str());
                const glintfx::ElementBox box = filter_offscreen_controls_row(item, raw, list_box);
                if (hit_test(box, cx, cy)) {
                    winner = item;
                    break;
                }
            }
            const SystemMenuAction click_action25 =
                (winner == 25) ? system_menu_click_option(state, winner) : SystemMenuAction::None;
            std::cout << "SystemMenuLoop: [selftest][GLINTFX-SCROLL][clique "
                         "pos-scroll] indice vencedor="
                      << winner << " (esperado 25); apos clique: controls_selected="
                      << state.controls_selected << " controls_capturing="
                      << state.controls_capturing << " (esperado selected=25 "
                         "capturing=1) - "
                      << (winner == 25 && state.controls_selected == 25 &&
                                  state.controls_capturing
                              ? "OK"
                              : "FALHOU")
                      << "\n";
            (void)click_action25;
            system_menu_controls_capture_key(state, /*is_escape=*/true, 0);  // cancela
        }

        return outcome;
    }

    // DIAGNOSTICO/PROVA (SAVE-LOAD-UI etapa 6, prova visual headless Xvfb :99):
    // GUSWORLD_SAVELOAD_SCREENSHOT_DIR=<dir> abre a tela REAL de save/load (a
    // MESMA alcancada pelo jogador via Pause > Salvar/Carregar) em modo Save
    // (semeando o slot 1 de verdade em disco, save_game() real - pra o modo Load
    // seguinte mostrar um slot OCUPADO de verdade, nao um mock) e depois em modo
    // Load, salvando 1 PNG de cada (save_load_save.png/save_load_load.png, ver
    // save_load_menu_loop.cpp) - bypassa por completo o jogo real (nunca abre pra
    // input real), MESMO espirito dos demais self-tests acima.
    const char* saveload_screenshot_dir =
        std::getenv("GUSWORLD_SAVELOAD_SCREENSHOT_DIR");
    if (saveload_screenshot_dir != nullptr && saveload_screenshot_dir[0] != '\0') {
        // MESMO fix critico do ramo OpenSaveLoadSave/Load de handle_action acima
        // (RmlUi nao suporta 2 UiLayer simultaneos) - destroi o UiLayer do Pause
        // (ainda vivo desde a construcao no topo desta funcao) ANTES de abrir a
        // tela de save/load. Sem re-emplace depois: este ramo so retorna
        // (nenhum reload()/present_frame() do Pause roda mais nesta chamada).
        ui_opt.reset();
        (void)run_save_load_menu_loop_gl_current(window, translator, SaveLoadMode::Save,
                                                  saves_dir, build_current_save_data,
                                                  apply_loaded_save_data,
                                                  frozen_background_png);
        if (build_current_save_data) {
            // Semeia o slot 1 de verdade (I/O real, MESMO save_game que o
            // jogador aciona) - o modo Load abaixo mostra esse slot OCUPADO.
            gus::domain::save::SaveData seed = build_current_save_data();
            seed.slot_id = 1;
            (void)gus::platform::fs::save_game(seed, 1, saves_dir);
        }
        (void)run_save_load_menu_loop_gl_current(window, translator, SaveLoadMode::Load,
                                                  saves_dir, build_current_save_data,
                                                  apply_loaded_save_data,
                                                  frozen_background_png);
        return outcome;
    }

    while (true) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) {
                outcome.quit_app = true;
                return outcome;
            }
            if (ev.type == SDL_EVENT_WINDOW_RESIZED ||
                ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                SDL_GetWindowSizeInPixels(window, &pw, &ph);
                if (pw < 1) pw = 1;
                if (ph < 1) ph = 1;
                dp_ratio = static_cast<float>(pw) / 960.0f;
                ui_opt->set_viewport(pw, ph);
                ui_opt->set_dp_ratio(dp_ratio);
                continue;
            }
            if (ev.type == SDL_EVENT_KEY_DOWN && !ev.key.repeat) {
                // MODO DE CAPTURA (tela Controles, M2): intercepta ANTES do
                // roteamento generico abaixo - toda tecla (nao so UP/DOWN/
                // ENTER/ESC com o significado especial de navegacao) e
                // candidata a virar o novo binding. Esc CANCELA (nao vira
                // binding); qualquer outra tecla e traduzida pro esquema Godot
                // (sdl_key_to_godot_keycode, MESMA tabela que o InputMapper de
                // gameplay usa) e aplicada via system_menu_controls_capture_key
                // (swap-on-conflict + persistencia se ControlsChanged).
                if (state.screen == SystemMenuScreen::Controls && state.controls_capturing) {
                    const bool is_escape = (ev.key.key == SDLK_ESCAPE);
                    const long long godot_keycode =
                        is_escape ? 0
                                  : gus::platform::input::sdl_key_to_godot_keycode(
                                        static_cast<int>(ev.key.key));
                    const SystemMenuAction action =
                        system_menu_controls_capture_key(state, is_escape, godot_keycode);
                    if (handle_action(action)) return outcome;
                    continue;
                }

                const bool is_confirm_key = (ev.key.key == SDLK_RETURN ||
                                              ev.key.key == SDLK_KP_ENTER ||
                                              ev.key.key == SDLK_SPACE);
                if (is_confirm_key) {
                    // Enter/Espaco: captura a tela+item ATUAIS (antes da mutacao)
                    // pra poder desenhar o flash de PRESS na tela DE ORIGEM caso a
                    // action resultante confirme algo (ver is_confirming acima).
                    const SystemMenuState pre_action_state = state;
                    int item_index = -1;
                    switch (state.screen) {
                        case SystemMenuScreen::Pause:
                            item_index = state.pause_selected;
                            break;
                        case SystemMenuScreen::ConfigCategories:
                            item_index = state.config_categories_selected;
                            break;
                        case SystemMenuScreen::Audio:
                            item_index = state.audio_selected;
                            break;
                        case SystemMenuScreen::Controls:
                            // Confirmando o restaurar-padrao OU o descarte (M2 STAGED
                            // CHANGES): o item pressionado e a pill Sim/Nao do
                            // mini-dialogo correspondente (0|1); caso contrario, a
                            // acao/rodape selecionado normalmente (inclui Aplicar,
                            // kControlsApplyIndex). controls_capturing nunca chega
                            // aqui (interceptado acima, ver o `continue`).
                            if (state.controls_confirming_restore) {
                                item_index = state.controls_restore_confirm_selected;
                            } else if (state.controls_confirming_discard) {
                                item_index = state.controls_discard_confirm_selected;
                            } else {
                                item_index = state.controls_selected;
                            }
                            break;
                        case SystemMenuScreen::Video:
                        case SystemMenuScreen::Language:
                            item_index = kPlaceholderBackIndex;
                            break;
                        case SystemMenuScreen::Hidden:
                            break;
                    }
                    const SystemMenuAction action =
                        system_menu_key_down(state, ev.key.key);
                    if (is_confirming(action)) flash_pressed(pre_action_state, item_index);
                    if (handle_action(action)) return outcome;
                } else {
                    // Nao-confirm-key (setas/WASD/LEFT/RIGHT/ESC): handle_navigation_key
                    // (ver seu comentario acima) ja cobre system_menu_key_down +
                    // handle_action + o SOM DE HOVER PARIDADE na navegacao.
                    if (handle_navigation_key(ev.key.key)) return outcome;
                }
            } else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                       ev.button.button == SDL_BUTTON_LEFT) {
                bool handled = false;
                if (state.screen == SystemMenuScreen::Pause) {
                    // Clicar numa pill (Continuar/Salvar/Configuracoes/Sair)
                    // SELECIONA E ACIONA na hora - equivalente a focar + ENTER.
                    for (int item = 0; item < kPauseItemCount && !handled; ++item) {
                        const glintfx::ElementBox box =
                            ui_opt->get_element_box(pause_item_id(item).c_str());
                        if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                        handled = true;
                        const SystemMenuState pre_action_state = state;
                        const SystemMenuAction action =
                            system_menu_click_option(state, item);
                        if (is_confirming(action)) flash_pressed(pre_action_state, item);
                        if (handle_action(action)) return outcome;
                    }
                } else if (state.screen == SystemMenuScreen::ConfigCategories) {
                    // Categorias (Audio/Video/Lingua/Voltar) - botoes simples, SEM
                    // slider: clicar SEMPRE seleciona E aciona na hora.
                    for (int item = 0; item < kConfigCategoriesItemCount && !handled;
                         ++item) {
                        const glintfx::ElementBox box =
                            ui_opt->get_element_box(category_item_id(item).c_str());
                        if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                        handled = true;
                        const SystemMenuState pre_action_state = state;
                        const SystemMenuAction action =
                            system_menu_click_option(state, item);
                        if (is_confirming(action)) flash_pressed(pre_action_state, item);
                        if (handle_action(action)) return outcome;
                    }
                } else if (state.screen == SystemMenuScreen::Audio) {
                    // (1) Tracks dos sliders (drag-start, receita PRE-EXISTENTE) -
                    // checado PRIMEIRO porque a caixa do track fica DENTRO da
                    // caixa do campo/rotulo (audio-item-<i>, ver (3) abaixo) - o
                    // mais especifico tem que vencer quando o clique cai nos dois.
                    for (int item = 0; item < 2 && !handled; ++item) {
                        const glintfx::ElementBox box =
                            ui_opt->get_element_box(track_id_for_item(item).c_str());
                        if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                        handled = true;
                        drag_item = item;
                        state.audio_selected = item;
                        if (box.w > 0.0f) {
                            const float ratio = (ev.button.x - box.x) / box.w;
                            system_menu_set_slider_ratio(state, item, ratio);
                            apply_and_persist(state, audio, settings_dir);
                        }
                        reload();
                    }
                    // (2) Botao Voltar - ACIONA na hora (equivalente a focar + ENTER).
                    if (!handled) {
                        const int back_index = static_cast<int>(AudioItem::Back);
                        const glintfx::ElementBox box =
                            ui_opt->get_element_box(audio_item_id(back_index).c_str());
                        if (hit_test(box, ev.button.x, ev.button.y)) {
                            handled = true;
                            const SystemMenuState pre_action_state = state;
                            const SystemMenuAction action =
                                system_menu_click_option(state, back_index);
                            if (is_confirming(action)) {
                                flash_pressed(pre_action_state, back_index);
                            }
                            if (handle_action(action)) return outcome;
                        }
                    }
                    // (3) Campo/rotulo do slider (fora do track) - SO FOCA (nao
                    // ajusta volume - isso e papel exclusivo do track, ver (1)).
                    for (int item = 0; item < 2 && !handled; ++item) {
                        const glintfx::ElementBox box =
                            ui_opt->get_element_box(audio_item_id(item).c_str());
                        if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                        handled = true;
                        const SystemMenuAction action =
                            system_menu_click_option(state, item);
                        if (handle_action(action)) return outcome;
                    }
                } else if (state.screen == SystemMenuScreen::Controls) {
                    // Capturando: mouse NAO participa (o jogador precisa apertar
                    // uma tecla FISICA - ver a interceptacao no ramo KEY_DOWN
                    // acima) - clique nao faz nada nesse modo.
                    if (state.controls_capturing) {
                        // no-op
                    } else if (state.controls_confirming_restore) {
                        // Mini-dialogo "tem certeza?": so as 2 pills Sim/Nao
                        // (system_menu_click_option reinterpreta o indice como
                        // 0=Sim/1=Nao neste sub-modo, ver system_menu.cpp).
                        for (int item = 0; item < 2 && !handled; ++item) {
                            const glintfx::ElementBox box =
                                ui_opt->get_element_box(controls_confirm_id(item).c_str());
                            if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                            handled = true;
                            const SystemMenuState pre_action_state = state;
                            const SystemMenuAction action = system_menu_click_option(state, item);
                            if (is_confirming(action)) flash_pressed(pre_action_state, item);
                            if (handle_action(action)) return outcome;
                        }
                    } else if (state.controls_confirming_discard) {
                        // Mini-dialogo "descartar alteracoes?" (M2 STAGED
                        // CHANGES) - MESMA mecanica do restaurar-padrao acima,
                        // ids proprios (controls_discard_confirm_id).
                        for (int item = 0; item < 2 && !handled; ++item) {
                            const glintfx::ElementBox box =
                                ui_opt->get_element_box(controls_discard_confirm_id(item).c_str());
                            if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                            handled = true;
                            const SystemMenuState pre_action_state = state;
                            const SystemMenuAction action = system_menu_click_option(state, item);
                            if (is_confirming(action)) flash_pressed(pre_action_state, item);
                            if (handle_action(action)) return outcome;
                        }
                    } else {
                        // Navegacao normal: clicar numa action FOCA+ENTRA em
                        // captura (system_menu_click_option, MESMA convencao
                        // "focar+ENTER" das outras telas); clicar em Restaurar
                        // abre o mini-dialogo; clicar em Voltar confirma na hora.
                        // BUG-A: filtra as 30 actions (nao o rodape) pelo
                        // recorte visivel de `.ctrl-list` ANTES do hit-test -
                        // sem isto, uma linha rolada pra fora da vista podia
                        // roubar o clique de Restaurar/Voltar (ver
                        // filter_offscreen_controls_row/controls_row_visible_in_list).
                        const glintfx::ElementBox list_box = ui_opt->get_element_box(kControlsListId);
                        for (int item = 0; item < kControlsItemCount && !handled; ++item) {
                            const glintfx::ElementBox raw =
                                ui_opt->get_element_box(controls_item_id(item).c_str());
                            const glintfx::ElementBox box =
                                filter_offscreen_controls_row(item, raw, list_box);
                            if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                            handled = true;
                            const SystemMenuState pre_action_state = state;
                            const SystemMenuAction action = system_menu_click_option(state, item);
                            if (is_confirming(action)) flash_pressed(pre_action_state, item);
                            if (handle_action(action)) return outcome;
                        }
                    }
                } else if (state.screen == SystemMenuScreen::Video ||
                           state.screen == SystemMenuScreen::Language) {
                    // Placeholder ("em breve"): so o Voltar e clicavel.
                    const glintfx::ElementBox box = ui_opt->get_element_box(kPlaceholderBackId);
                    if (hit_test(box, ev.button.x, ev.button.y)) {
                        handled = true;
                        const SystemMenuState pre_action_state = state;
                        const SystemMenuAction action = system_menu_click_option(
                            state, kPlaceholderBackIndex);
                        if (is_confirming(action)) {
                            flash_pressed(pre_action_state, kPlaceholderBackIndex);
                        }
                        if (handle_action(action)) return outcome;
                    }
                }
            } else if (ev.type == SDL_EVENT_MOUSE_MOTION) {
                // SEMPRE (nao so durante o arrasto, PEDIDO 2a): hover NATIVO (:hover
                // RCSS) + edge-detect do SOM de hover; o arrasto de slider (se
                // drag_item>=0) continua tratado DENTRO de handle_mouse_motion.
                handle_mouse_motion(ev.motion.x, ev.motion.y);
            } else if (ev.type == SDL_EVENT_MOUSE_BUTTON_UP &&
                       ev.button.button == SDL_BUTTON_LEFT) {
                drag_item = -1;
            } else if (ev.type == SDL_EVENT_MOUSE_WHEEL) {
                // WHEEL FORWARDING (M2/GLINTFX-SCROLL, glintfx v0.4.0): rola o
                // elemento em HOVER (NAO o com foco - ver o comentario extenso de
                // glintfx::UiEvent::Type::MouseWheel no vendored ui_event.hpp).
                // GOTCHA CRITICO da release note: um MouseWheel sem MouseMove
                // recente sobre o alvo e um no-op silencioso (hover fica o que o
                // ULTIMO MouseMove deixou - possivelmente nenhum, ex. logo apos
                // reload() trocar de DOCUMENTO, que reseta o hover interno do
                // RmlUi, ou o cursor parado ha varios frames sem novo
                // SDL_EVENT_MOUSE_MOTION). Nao contamos com "o loop ja manda
                // MouseMove todo frame" pra garantir isso (so manda em
                // SDL_EVENT_MOUSE_MOTION de verdade, nao a cada frame) - por
                // isso, SEMPRE mandamos 1 MouseMove sintetico pra posicao ATUAL
                // do cursor (SDL_GetMouseState, nao um cache proprio - pega o
                // valor mais fresco possivel) IMEDIATAMENTE antes do
                // MouseWheel, via handle_mouse_motion (o MESMO caminho de
                // codigo do SDL_EVENT_MOUSE_MOTION real acima - reusa hover
                // visual + som de hover + arrasto de slider, sem duplicar
                // logica). Hover fora de container overflow:auto (ou hover
                // nenhum) e um no-op seguro do lado do RmlUi (GetClosestScrollable
                // Container devolve nullptr) - nao gated por tela de proposito
                // (a tela Controles e a UNICA com lista rolavel hoje, mas
                // encaminhar sempre, incondicional, cobre qualquer lista rolavel
                // futura de graca).
                float mouse_x = 0.0f, mouse_y = 0.0f;
                SDL_GetMouseState(&mouse_x, &mouse_y);
                handle_mouse_motion(mouse_x, mouse_y);

                const float wheel_dy = system_menu_wheel_delta_to_rmlui(
                    ev.wheel.y, ev.wheel.direction == SDL_MOUSEWHEEL_FLIPPED);
                glintfx::UiEvent wheel_ev{};
                wheel_ev.type = glintfx::UiEvent::Type::MouseWheel;
                wheel_ev.x = 0.0f;
                wheel_ev.y = wheel_dy;
                ui_opt->process_event(wheel_ev);
            }
        }

        present_frame();
    }
}

bool run_system_menu_loop_owning_gl(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator, const std::string& settings_dir,
    const std::string& saves_dir, SystemMenuLoopOutcome* out_outcome,
    const std::function<gus::domain::save::SaveData()>& build_current_save_data,
    const std::function<void(const gus::domain::save::SaveData&)>&
        apply_loaded_save_data,
    const std::string& frozen_background_png) {
    // MESMA receita de run_battle_preview_embedded (battle_preview.cpp): os
    // atributos GL sao setados a CADA entrada (nao precisam ter sido setados na
    // criacao da janela) - viabilidade ja provada empiricamente pela troca
    // cidade<->batalha (ver maestro.cpp::to_battle).
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GLContext gl = SDL_GL_CreateContext(window);
    if (gl == nullptr) {
        std::cerr << "SystemMenuLoop: SDL_GL_CreateContext falhou: " << SDL_GetError()
                  << "\n";
        return false;
    }
    SDL_GL_MakeCurrent(window, gl);
    SDL_GL_SetSwapInterval(1);

    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        std::cerr << "SystemMenuLoop: falha ao carregar funcoes OpenGL (glad)\n";
        SDL_GL_DestroyContext(gl);
        return false;
    }

    const SystemMenuLoopOutcome outcome = run_system_menu_loop_gl_current(
        window, audio, translator, settings_dir, saves_dir, build_current_save_data,
        apply_loaded_save_data, frozen_background_png);
    if (out_outcome != nullptr) {
        *out_outcome = outcome;
    }

    SDL_GL_DestroyContext(gl);
    return true;
}

}  // namespace gus::app::screens
