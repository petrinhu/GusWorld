// SPDX-License-Identifier: Apache-2.0
// gus/app/src/screens/difficulty_menu_loop.cpp
//
// Implementacao do loop interativo da tela de selecao de dificuldade. Ver header
// para o contrato completo.
//
// F4-1b (onda F4 "casca SDL -> App mode do glintfx", fatia 1b - PRIMEIRA tela
// convertida ao contrato ScreenState apos F4-1a/npc_dialogue_loop_gl.cpp - esta
// conversao ESTABELECE o template pras telas seguintes da fatia): o caminho de
// PRODUCAO (run_difficulty_menu_loop_gl_current, unico chamador de producao e
// title_menu_loop.cpp) foi convertido de um while(true){SDL_PollEvent...} PROPRIO
// pra classe DifficultyScreen (gus::app::ScreenState) + gus::app::run_screen_state
// (gus/app/screen_state.hpp) - MESMA tecnica de NpcDialogueScreen (F4-1a). O
// roteamento de evento (o "o que fazer" - navegar/confirmar/tocar SFX/reload/
// sair) foi extraido pra uma FUNCAO LIVRE PURA nova, difficulty_screen_step
// (declarada no .hpp, testada headless SEM SDL_Init/glintfx real em
// difficulty_screen_step_test.cpp) - ela recebe as glintfx::ElementBox JA
// RESOLVIDAS como parametro (`boxes`), nunca consulta a UiLayer diretamente;
// DifficultyScreen::handle_event() e o UNICO chamador de producao dela (resolve
// as boxes quando o evento precisa, delega a decisao, aplica os side effects
// REAIS - SDL/GL/audio - que a funcao pura devolve pedidos).
//
// ANINHADO (NAO owning_gl): ao contrario de title_menu_loop.cpp, esta funcao NAO
// cria/destroi o contexto GL - ela roda DENTRO do contexto que o CHAMADOR
// (title_menu_loop.cpp) ja deixou corrente, MESMA tecnica de
// run_save_load_menu_loop_gl_current (o CHAMADOR ja destruiu a PROPRIA UiLayer
// ANTES de chamar esta funcao - RmlUi so aceita 1 instancia viva por processo).

#include "gus/app/screens/difficulty_menu_loop.hpp"

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

#include "gus/app/screens/difficulty_menu_rml.hpp"
#include "gus/core/asset_paths.hpp"  // kMenuHoverSfxFile/kMenuClickSfxFile/kMenuBlockedSfxFile/kSfxDir
#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Rect
#include "gus/platform/assets/asset_source.hpp"  // FilesystemAssetSource (resolve do SFX)
#include "gus/platform/input/key_translation.hpp"  // sdl_key_to_godot_keycode (Fatia 2)
#include "gus/platform/input/key_translation_glintfx.hpp"  // godot_keycode_to_glintfx_key
#include "gus/platform/render2d/render2d_gl3.hpp"
// FRAMEGRAB-7-SITIOS (2026-07-30): gl3_loader.hpp/gl3_read_backbuffer_rgba
// (leitura crua de backbuffer via glad) saiu daqui - a unica captura deste
// arquivo migrou pra glintfx::UiLayer::capture_frame() (ver render_frame_()/
// enter()). Nao ha mais nenhuma outra necessidade de gl3_loader.hpp neste
// arquivo (ao contrario de battle_preview.cpp, que ainda usa
// gl3_load_functions pra outro fim, fora do escopo desta migracao).

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

// Stage PROPRIO (nao colide com o stage de title_menu_loop.cpp/system_menu_loop.cpp).
std::string difficulty_stage_dir() {
    return (fs::temp_directory_path() / "gusworld_glintfx_difficulty").string();
}

std::string write_difficulty_rml_file(const DifficultyMenuState& state,
                                       const gus::app::i18n::Translator& tr,
                                       int pressed_index = -1) {
    const fs::path stage = difficulty_stage_dir();
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
    // de string - DifficultyScreen::enter() registra a familia 1x via
    // glintfx::UiLayer::load_font_face (API v0.24.0) logo apos set_asset_base_url. Os
    // .ttf continuam copiados pro stage acima (load_font_face resolve o path pela MESMA
    // BaseUrlFileInterface que o @font-face antigo usava).
    std::string rml = build_difficulty_menu_rml(state, tr, pressed_index);

    const fs::path out = stage / "difficulty_menu.rml";
    std::ofstream f(out);
    f << rml;
    return out.string();
}

// FONT-EXTEND-GLITCH (2026-07-29): registra "Pixel Operator Mono" (regular+bold) via
// glintfx::UiLayer::load_font_face (API v0.24.0) - mata o @font-face por string que
// write_difficulty_rml_file injetava antes. Chamado 1x em enter() (registro e
// process-wide RmlUi state, ver ui_layer.hpp), DEPOIS do stage ja ter os .ttf (o
// PRIMEIRO write_difficulty_rml_file os copia) e ANTES de ui_->load(). `family`
// SEMPRE explicito (armadilha de assimetria de motor Own-vs-FreeType documentada em
// font_face.hpp - o mesmo literal que o RCSS ja referencia neutraliza os dois).
void register_pixel_operator_mono_fonts(glintfx::UiLayer& ui) {
    if (!ui.load_font_face(glintfx::FontFaceDesc{/*path=*/"PixelOperatorMono.ttf",
                                                  /*family=*/"Pixel Operator Mono"})) {
        std::cerr << "DifficultyMenuLoop: load_font_face(PixelOperatorMono.ttf) falhou "
                     "(arquivo ausente/invalido no stage) - cai no fallback de fonte do "
                     "RmlUi.\n";
    }
    if (!ui.load_font_face(glintfx::FontFaceDesc{
            /*path=*/"PixelOperatorMono-Bold.ttf",
            /*family=*/"Pixel Operator Mono",
            /*style=*/glintfx::FontStyle::Normal,
            /*weight=*/glintfx::FontWeight::Bold})) {
        std::cerr << "DifficultyMenuLoop: load_font_face(PixelOperatorMono-Bold.ttf) "
                     "falhou (arquivo ausente/invalido no stage) - cai no fallback de "
                     "fonte do RmlUi.\n";
    }
}

// Hit-test simples (MESMA receita de title_menu_loop.cpp/system_menu_loop.cpp):
// cursor (espaco-janela) dentro da caixa border-box devolvida por
// glintfx::UiLayer::get_element_box. PURA - usada tanto por difficulty_screen_step
// (roteamento de clique) quanto (indiretamente, via boxes ja resolvidas) pelo
// resto deste arquivo.
bool hit_test(const glintfx::ElementBox& box, float x, float y) noexcept {
    if (!box.found) return false;
    return x >= box.x && x <= box.x + box.w && y >= box.y && y <= box.y + box.h;
}

std::string resolve_menu_sfx_path(std::string_view file) {
    const std::string id = join(std::string(gus::core::assets::kSfxDir), std::string(file));
    return gus::platform::assets::FilesystemAssetSource().resolve_path(id);
}

// SFX-MIGRATE-V0.9: filtro NAVEGAVEL pro callback NATIVO de hover (MESMA receita de
// is_navigable_hover_id em title_menu_loop.cpp) - ids "difficulty-item-<i>" (lista)
// ou "difficulty-confirm-<i>" (splash), conforme o modo ATUAL.
bool is_navigable_hover_id(const DifficultyMenuState& state, const std::string& id) {
    if (state.confirming) {
        return id == "difficulty-confirm-0" || id == "difficulty-confirm-1";
    }
    for (int i = 0; i < kDifficultyItemCount; ++i) {
        if (id == "difficulty-item-" + std::to_string(i)) return true;
    }
    return false;
}

// AJUSTE polish playtest 2026-07-10 ("card Hardcore bloqueado toca SFX grave/
// abafado"): extrai o indice do item de LISTA (nao-splash) do id do elemento
// hover-native, ou nullopt se `id` nao for um "difficulty-item-<i>" (splash, ou
// id desconhecido). So faz sentido fora do splash (os 2 pills de confirmacao sao
// SEMPRE selecionaveis - nunca tocam o SFX bloqueado).
std::optional<int> parse_difficulty_list_item_index(const std::string& id) {
    for (int i = 0; i < kDifficultyItemCount; ++i) {
        if (id == "difficulty-item-" + std::to_string(i)) return i;
    }
    return std::nullopt;
}

// F4-1b: roteia UM "kind normal" (Hover/Click) pro Blocked quando `index` e um
// item de LISTA (nao-splash) NAO selecionavel (Hardcore nesta Fase 0) - MESMA
// decisao que o antigo `sfx_for_item` lambda tomava, agora 100% PURA (so
// `state` + `difficulty_item_selectable`, sem SoundId nenhum) e COMPARTILHADA
// entre difficulty_screen_step (teclado/clique) e o callback nativo de hover
// (mouse) abaixo - um UNICO lugar decide "isto e o card bloqueado?".
DifficultySfxKind sfx_kind_for_item(const DifficultyMenuState& state, int index,
                                    DifficultySfxKind normal_kind) noexcept {
    if (!state.confirming && index >= 0 && index < kDifficultyItemCount &&
        !difficulty_item_selectable(state, index)) {
        return DifficultySfxKind::Blocked;
    }
    return normal_kind;
}

// F4-1b: aplica o DESFECHO de um DifficultyMenuAction (None/Chosen/Cancelled) no
// DifficultyStepResult - MESMO dispatch do antigo `route_action` lambda, so que
// gravando em campos da struct de retorno (reload/exit) em vez de chamar
// reload()/retornar direto (esses side effects ficam com o CHAMADOR, ver
// DifficultyScreen::handle_event).
void apply_action_to_result(DifficultyStepResult& result, DifficultyMenuAction action) noexcept {
    switch (action) {
        case DifficultyMenuAction::None:
            result.reload = true;
            return;
        case DifficultyMenuAction::Chosen:
            result.exit = DifficultyLoopExit::Chosen;
            return;
        case DifficultyMenuAction::Cancelled:
            result.exit = DifficultyLoopExit::Cancelled;
            return;
    }
}

// Traduz SDL_Keycode -> glintfx::Key REUSANDO a ponte SDL->Godot->glintfx JA
// EXISTENTE e testada adversarialmente (F4-2, 8/8 mutantes mortos):
// platform/input/key_translation.hpp (SDL_Keycode -> keycode Godot) +
// platform/input/key_translation_glintfx.hpp (keycode Godot -> glintfx::Key).
// Evita duplicar uma 3a tabela SDLK_*->glintfx::Key so pra esta tela
// (M9-CAMADAS-SDL Fatia 2, docs/tech/plano-camadas-sdl.md) - difficulty_menu.hpp
// (a logica PURA) so conhece glintfx::Key, o SDL fica 100% aqui no loop.
//
// SDLK_RETURN e SDLK_KP_ENTER colapsam no MESMO Key::Enter pela ponte Godot
// (key_translation.cpp ja fundia as duas no MESMO kGodotEnter, ANTES desta
// fatia) - preserva EXATAMENTE o comportamento anterior (difficulty_menu.cpp ja
// tratava as duas teclas como sinonimo em is_confirm_key), nao e regressao.
// glintfx::Key nao tem KpEnter dedicado - lacuna ja reportada ao glintfx.
glintfx::Key sdl_keycode_to_menu_key(SDL_Keycode sdl_key) noexcept {
    const long long godot =
        gus::platform::input::sdl_key_to_godot_keycode(static_cast<int>(sdl_key));
    return gus::platform::input::godot_keycode_to_glintfx_key(godot)
        .value_or(glintfx::Key::None);
}

}  // namespace

// F4-1b: implementacao de difficulty_screen_step (declarada no .hpp) - ver o
// comentario grande la pro contrato completo. Extracao BEHAVIOR-PRESERVING do
// corpo do while(true) antigo (o mesmo racional/ordem de checagem de tipo de
// evento, MESMAS chamadas a difficulty_menu_key_down/difficulty_menu_click_option,
// MESMO uso de sfx_kind_for_item/ui_hover_entered_new_item) - so devolvendo a
// DECISAO em vez de executar os side effects na hora.
DifficultyStepResult difficulty_screen_step(
    DifficultyMenuState& state, const SDL_Event& ev,
    const glintfx::ElementBox boxes[kDifficultyItemCount]) noexcept {
    DifficultyStepResult result;

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
            // AJUSTE polish playtest 2026-07-10: Enter/Espaco com o foco no
            // Hardcore BLOQUEADO (item de LISTA) toca o SFX bloqueado em vez do
            // click normal - a ACAO em si segue no-op TOTAL de estado
            // (difficulty_menu_key_down, ver o header) - result.reload ainda vira
            // true (MESMO comportamento antigo: route_action recarrega o RML
            // mesmo quando o estado nao mudou de fato).
            result.sfx = sfx_kind_for_item(state, difficulty_keyboard_focus_index(state),
                                           DifficultySfxKind::Click);
            const DifficultyMenuAction action =
                difficulty_menu_key_down(state, sdl_keycode_to_menu_key(ev.key.key));
            apply_action_to_result(result, action);
        } else {
            // Navegacao (setas/WASD/ESC) - SOM DE HOVER paridade teclado x mouse
            // (MESMA tecnica de title_menu_loop.cpp): so toca se o MODO (lista vs
            // splash) NAO mudou E moveu pra um item NOVO.
            const bool confirming_before = state.confirming;
            const int kb_index_before = difficulty_keyboard_focus_index(state);
            const DifficultyMenuAction action =
                difficulty_menu_key_down(state, sdl_keycode_to_menu_key(ev.key.key));
            if (state.confirming == confirming_before) {
                const int kb_index_after = difficulty_keyboard_focus_index(state);
                if (ui_hover_entered_new_item(kb_index_before, kb_index_after)) {
                    result.sfx =
                        sfx_kind_for_item(state, kb_index_after, DifficultySfxKind::Hover);
                }
            }
            apply_action_to_result(result, action);
        }
        return result;
    }

    if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN && ev.button.button == SDL_BUTTON_LEFT) {
        if (state.confirming) {
            for (int i = 0; i < 2; ++i) {
                if (!hit_test(boxes[i], ev.button.x, ev.button.y)) continue;
                // Os 2 pills de confirmacao sao SEMPRE selecionaveis - nunca
                // tocam o SFX bloqueado (MESMO comentario do lambda antigo).
                result.sfx = DifficultySfxKind::Click;
                const DifficultyMenuAction action = difficulty_menu_click_option(state, i);
                apply_action_to_result(result, action);
                return result;
            }
            return result;  // clique fora de qualquer pill - no-op TOTAL
        }
        for (int i = 0; i < kDifficultyItemCount; ++i) {
            if (!hit_test(boxes[i], ev.button.x, ev.button.y)) continue;
            // Clicar no Hardcore BLOQUEADO e no-op TOTAL DE ESTADO (ver
            // difficulty_menu_click_option), mas AJUSTE polish playtest
            // 2026-07-10: toca o SFX bloqueado (grave/abafado) em vez de ficar
            // mudo - da feedback sonoro de "bloqueado" ao jogador. Facil/Medio/
            // Dificil sempre selecionaveis, click normal toca.
            result.sfx = sfx_kind_for_item(state, i, DifficultySfxKind::Click);
            const DifficultyMenuAction action = difficulty_menu_click_option(state, i);
            apply_action_to_result(result, action);
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

namespace {

// F4-1b: o ScreenState de PRODUCAO da tela de dificuldade (unico chamador:
// run_difficulty_menu_loop_gl_current, abaixo) - MESMO padrao de
// NpcDialogueScreen (npc_dialogue_loop_gl.cpp, F4-1a). Todo o estado que antes
// vivia em variaveis locais fechadas por lambdas (ui/backdrop/ids de SFX/
// rml_path/etc) agora e MEMBRO; enter() cria (glintfx::UiLayer + Render2dGl3 +
// ids de SFX), exit() libera (a EXCLUSIVIDADE do UiLayer descrita em
// gus/app/screen_state.hpp: exit() e o UNICO lugar onde ui_/backdrop_ sao
// destruidos, chamado pelo runner logo apos o ultimo tick()).
class DifficultyScreen final : public gus::app::ScreenState {
public:
    DifficultyScreen(SDL_Window* window, gus::platform::audio::AudioEngine& audio,
                      const gus::app::i18n::Translator& translator,
                      gus::domain::save::DifficultyLevel* out_difficulty,
                      std::string frozen_background_png)
        : window_(window),
          audio_(audio),
          translator_(translator),
          out_difficulty_(out_difficulty),
          frozen_background_png_(std::move(frozen_background_png)) {}

    void enter() override {
        // TODO Fase 4 - ler da ANCORA selada (ADR-015 + modos-morte.md §2.3b: o
        // unlock do Hardcore mora DENTRO da mesma ancora selada+machine-bound do
        // save-crypto, setado na vitoria do Dificil - nao ha profile.json/flag
        // soft, proposta rejeitada pelo lider). Nesta Fase 0 e o UNICO estado
        // alcancavel: hardcoded false - Hardcore fica visivel mas SEMPRE
        // bloqueado (scope-add REVISAO 2026-07-10, "cenoura" - ver o comentario
        // grande em difficulty_menu.hpp).
        constexpr bool kHardcoreUnlockedFase0 = false;
        difficulty_menu_open(state_, kHardcoreUnlockedFase0);

        SDL_GetWindowSizeInPixels(window_, &pw_, &ph_);
        if (pw_ < 1) pw_ = 1;
        if (ph_ < 1) ph_ = 1;
        dp_ratio_ = static_cast<float>(pw_) / 960.0f;

        ui_.emplace(glintfx::UiLayer::Config{/*logical_width=*/960,
                                              /*logical_height=*/540,
                                              /*load_gl=*/true,
                                              /*dp_ratio=*/dp_ratio_});
        if (!ui_->ok()) {
            std::cerr << "DifficultyMenuLoop: glintfx::UiLayer::ok()=false (attach "
                         "falhou) - abortando Novo Jogo, volta pra tela de titulo "
                         "(degradacao segura).\n";
            result_ = DifficultyLoopExit::Cancelled;
            bailed_ = true;
            return;
        }

        stage_ = difficulty_stage_dir();
        ui_->set_asset_base_url(stage_.c_str());
        rml_path_ = write_difficulty_rml_file(state_, translator_);
        register_pixel_operator_mono_fonts(*ui_);
        ui_->load(rml_path_.c_str());
        ui_->set_viewport(pw_, ph_);
        ui_->set_dp_ratio(dp_ratio_);
        // SFX-MIGRATE-V0.9: 1 update() de "assentamento" (MESMO achado empirico de
        // title_menu_loop.cpp/save_load_menu_loop.cpp - o hover NATIVO so resolve
        // elemento sob o cursor apos pelo menos 1 Context::Update() do documento
        // recem-carregado).
        ui_->update();

        backdrop_.emplace(/*gl_active=*/true);
        frozen_bg_tex_ = frozen_background_png_.empty()
                             ? gus::platform::render2d::kInvalidTexture
                             : backdrop_->load_texture(frozen_background_png_.c_str());

        // AJUSTE polish playtest 2026-07-10: SFX proprio (grave/abafado) do card
        // Hardcore BLOQUEADO - toca no lugar do hover/click normal quando a
        // interacao mira um item NAO-selecionavel (ver sfx_kind_for_item acima).
        // Degradacao segura: se o asset ainda nao existir no disco, load_sfx
        // devolve kInvalidSound e audio.play_sfx() vira no-op (nunca crasha).
        const std::string hover_sfx_path =
            resolve_menu_sfx_path(gus::core::assets::kMenuHoverSfxFile);
        const std::string click_sfx_path =
            resolve_menu_sfx_path(gus::core::assets::kMenuClickSfxFile);
        const std::string blocked_sfx_path =
            resolve_menu_sfx_path(gus::core::assets::kMenuBlockedSfxFile);
        hover_sfx_id_ = audio_.load_sfx(hover_sfx_path.c_str());
        click_sfx_id_ = audio_.load_sfx(click_sfx_path.c_str());
        blocked_sfx_id_ = audio_.load_sfx(blocked_sfx_path.c_str());

        ui_->set_hover_callback([this](const char* raw_id, bool entered) {
            native_hover_callback_(raw_id, entered);
        });

        // DIAGNOSTICO/PROVA (prova visual headless Xvfb :99): GUSWORLD_DIFFICULTY_
        // SCREENSHOT_DIR=<dir> assenta alguns frames e salva 1 PNG ANTES de entrar
        // no loop interativo - bypassa por completo (MESMO espirito de
        // GUSWORLD_TITLE_SCREENSHOT_DIR em title_menu_loop.cpp).
        const char* screenshot_dir = std::getenv("GUSWORLD_DIFFICULTY_SCREENSHOT_DIR");
        if (screenshot_dir != nullptr && screenshot_dir[0] != '\0') {
            // GUSWORLD_DIFFICULTY_SCREENSHOT_CONFIRM=1 (opcional): captura o
            // SPLASH de confirmacao (Aviso #2) em vez da lista - abre via a MESMA
            // maquina de estado PURA (Enter na lista, mesma logica de um clique
            // real) antes do reload() que gera o RML novo.
            const char* confirm_flag = std::getenv("GUSWORLD_DIFFICULTY_SCREENSHOT_CONFIRM");
            const bool want_confirm = confirm_flag != nullptr && confirm_flag[0] != '\0';
            if (want_confirm) {
                (void)difficulty_menu_key_down(state_, glintfx::Key::Enter);  // abre o splash
                reload_();
            }
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
                const std::string filename =
                    want_confirm ? "difficulty_menu_confirm.png" : "difficulty_menu.png";
                const std::string out = join(std::string(screenshot_dir), filename);
                stbi_write_png(out.c_str(), captured.width, captured.height, 4,
                               captured.pixels.get(), captured.width * 4);
                std::cout << "DifficultyMenuLoop: [screenshot] " << out << " (" << captured.width
                          << "x" << captured.height << ")\n";
            } else {
                std::cerr << "DifficultyMenuLoop: [screenshot] capture_frame() falhou\n";
            }
            SDL_GL_SwapWindow(window_);  // completa o 6o frame (capturado logo acima)
            result_ = DifficultyLoopExit::Cancelled;
            bailed_ = true;
            return;
        }

        // NAO ha present_frame_() explicito aqui (ao contrario de
        // NpcDialogueScreen): o while(true) ORIGINAL desta tela so desenhava no
        // FIM de cada iteracao, DEPOIS de drenar a rajada de eventos daquele
        // frame - o 1o tick() do runner reproduz exatamente essa 1a iteracao
        // (drena o que houver, desenha em seguida).
    }

    void handle_event(const SDL_Event& ev) override {
        if (bailed_) {
            return;  // defensivo: nao deveria ser chamado (finished()==true).
        }

        // Resolve as boxes SO quando o evento precisa (MESMO custo do while(true)
        // antigo, que so chamava get_element_box dentro do ramo de clique) -
        // demais eventos passam um array zerado (found=false), que
        // difficulty_screen_step so consulta no ramo de MOUSE_BUTTON_DOWN.
        std::array<glintfx::ElementBox, kDifficultyItemCount> boxes{};
        if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN && ev.button.button == SDL_BUTTON_LEFT) {
            boxes = collect_click_boxes_();
        }

        const DifficultyStepResult step = difficulty_screen_step(state_, ev, boxes.data());

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
        if (step.sfx != DifficultySfxKind::None) {
            audio_.play_sfx(sound_id_for_(step.sfx));
        }
        if (step.exit.has_value()) {
            if (*step.exit == DifficultyLoopExit::Chosen && out_difficulty_ != nullptr) {
                *out_difficulty_ = difficulty_level_for_item(state_.selected);
            }
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
        // ANTES de backdrop_ (MESMA ordem de sempre) - depois de exit(), a
        // PROXIMA tela pode chamar seu proprio enter() com seguranca.
        ui_.reset();
        backdrop_.reset();
    }

    [[nodiscard]] bool window_closed() const override { return window_closed_; }

    // Exposto SO pro wrapper run_difficulty_menu_loop_gl_current (nao faz parte
    // do contrato ScreenState) - o desfecho Chosen/Cancelled decidido pelo
    // ultimo difficulty_screen_step com exit preenchido, OU o default Cancelled
    // (ui_->ok()==false / screenshot mode / nunca terminou por acao real).
    [[nodiscard]] DifficultyLoopExit result() const { return result_; }

private:
    void reload_() {
        rml_path_ = write_difficulty_rml_file(state_, translator_);
        ui_->load(rml_path_.c_str());
        ui_->set_viewport(pw_, ph_);
        ui_->set_dp_ratio(dp_ratio_);
        ui_->update();  // MESMO assentamento a cada troca de documento
    }

    // FRAMEGRAB-7-SITIOS: extraido de present_frame_() (que so agrega o swap
    // por cima) - o corpo isolado permite capturar via glintfx::UiLayer::
    // capture_frame() DEPOIS do render() e ANTES do proprio swap (contrato
    // do pin, ui_layer.hpp), sem duplicar arena+UI a mao no chamador.
    void render_frame_() {
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

    // Callback NATIVO de hover do glintfx (id-based, RCSS :hover) - MESMA receita
    // do hover_cb lambda antigo, agora metodo (captura `this` em vez de fechar
    // sobre variaveis locais). Reusa sfx_kind_for_item (a MESMA decisao PURA que
    // difficulty_screen_step usa pro teclado/clique) - garante paridade: hover de
    // mouse no Hardcore bloqueado toca o MESMO SFX que navegar ate ele por
    // teclado.
    void native_hover_callback_(const char* raw_id, bool entered) {
        const std::string id = raw_id != nullptr ? raw_id : "";
        if (!entered) {
            if (id == last_hover_sfx_id_) last_hover_sfx_id_.clear();
            return;
        }
        if (id == last_hover_sfx_id_ || !is_navigable_hover_id(state_, id)) return;
        last_hover_sfx_id_ = id;
        const std::optional<int> list_index = parse_difficulty_list_item_index(id);
        const DifficultySfxKind kind =
            sfx_kind_for_item(state_, list_index.value_or(-1), DifficultySfxKind::Hover);
        audio_.play_sfx(sound_id_for_(kind));
    }

    // Resolve as boxes de CLIQUE do frame ATUAL - "difficulty-confirm-<i>" (2
    // posicoes, splash) OU "difficulty-item-<i>" (kDifficultyItemCount posicoes,
    // lista), conforme state_.confirming - MESMA leitura que
    // difficulty_screen_step faz internamente pra decidir a semantica de
    // `boxes` (ver o comentario do .hpp). SO chamado do ramo MOUSE_BUTTON_DOWN de
    // handle_event() (get_element_box e uma query da UiLayer - nao roda em todo
    // evento, MESMO custo do while(true) antigo).
    [[nodiscard]] std::array<glintfx::ElementBox, kDifficultyItemCount>
    collect_click_boxes_() const {
        std::array<glintfx::ElementBox, kDifficultyItemCount> boxes{};
        if (state_.confirming) {
            for (int i = 0; i < 2; ++i) {
                boxes[static_cast<std::size_t>(i)] =
                    ui_->get_element_box(("difficulty-confirm-" + std::to_string(i)).c_str());
            }
        } else {
            for (int i = 0; i < kDifficultyItemCount; ++i) {
                boxes[static_cast<std::size_t>(i)] =
                    ui_->get_element_box(("difficulty-item-" + std::to_string(i)).c_str());
            }
        }
        return boxes;
    }

    [[nodiscard]] gus::platform::audio::SoundId sound_id_for_(DifficultySfxKind kind) const {
        switch (kind) {
            case DifficultySfxKind::Hover:
                return hover_sfx_id_;
            case DifficultySfxKind::Click:
                return click_sfx_id_;
            case DifficultySfxKind::Blocked:
                return blocked_sfx_id_;
            case DifficultySfxKind::None:
                return gus::platform::audio::kInvalidSound;
        }
        return gus::platform::audio::kInvalidSound;
    }

    SDL_Window* window_;
    gus::platform::audio::AudioEngine& audio_;
    const gus::app::i18n::Translator& translator_;
    gus::domain::save::DifficultyLevel* out_difficulty_;
    std::string frozen_background_png_;

    DifficultyMenuState state_;

    int pw_ = 0;
    int ph_ = 0;
    float dp_ratio_ = 1.0f;

    bool window_closed_ = false;
    bool bailed_ = false;  // ui_->ok()==false OU o modo screenshot rodou (ver enter())
    bool done_ = false;    // Chosen/Cancelled decidido por um difficulty_screen_step

    // std::optional (nao um objeto direto): enter()/exit() controlam o ciclo de
    // vida (EXCLUSIVIDADE DO UILAYER, ver gus/app/screen_state.hpp).
    std::optional<glintfx::UiLayer> ui_;
    std::optional<gus::platform::render2d::Render2dGl3> backdrop_;

    std::string stage_;
    std::string rml_path_;
    gus::platform::render2d::TextureId frozen_bg_tex_ =
        gus::platform::render2d::kInvalidTexture;

    gus::platform::audio::SoundId hover_sfx_id_ = gus::platform::audio::kInvalidSound;
    gus::platform::audio::SoundId click_sfx_id_ = gus::platform::audio::kInvalidSound;
    gus::platform::audio::SoundId blocked_sfx_id_ = gus::platform::audio::kInvalidSound;
    std::string last_hover_sfx_id_;

    // Default Cancelled (MESMO fallback do while(true) antigo: qualquer saida
    // que nao seja um Chosen/Cancelled explicito por acao real degrada pra
    // Cancelled - ui_->ok()==false e o modo screenshot tambem setam isto
    // explicitamente em enter()).
    DifficultyLoopExit result_ = DifficultyLoopExit::Cancelled;
};

}  // namespace

DifficultyLoopExit run_difficulty_menu_loop_gl_current(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator,
    gus::domain::save::DifficultyLevel* out_difficulty,
    const std::string& frozen_background_png, const gus::app::EventSyncHook& sync_hook) {
    DifficultyScreen screen(window, audio, translator, out_difficulty, frozen_background_png);
    gus::app::run_screen_state(screen, sync_hook);
    if (screen.window_closed()) return DifficultyLoopExit::QuitApp;
    return screen.result();
}

}  // namespace gus::app::screens
