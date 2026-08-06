// SPDX-License-Identifier: Apache-2.0
// gus/app/src/screens/battle_preview.cpp
//
// Ver header. Casca SDL do viewer da BattleScene (esqueleto M5). Reusa Render2dSdl
// (atras de IRenderer) e o mesmo padrao de loop do anim_preview (poll -> render). A
// cena LE o motor de combate; aqui so abrimos janela, carregamos os retratos 48px e
// desenhamos o esqueleto a cada frame. Esc/fechar encerra.
//
// F4-1b.5 (onda F4 "casca SDL -> App mode do glintfx", fatia 1b.5 - a conversao MAIS
// ARRISCADA da onda): o HOST REAL da batalha (run_battle_preview_embedded_gl_current)
// tinha 3 SDL_PollEvent PROPRIOS (fade-in ~linha 953, loop principal ~linha 1008,
// fade-out ~linha 1624 do arquivo pre-conversao) - viraram 3 FASES de UMA UNICA
// BattleScreen (gus::app::ScreenState): FadeIn -> Main -> FadeOut -> Done, decididas
// pela FSM PURA battle_phase_initial/battle_phase_next (declaradas no .hpp, testadas
// headless em battle_screen_step_test.cpp). MESMA tecnica de DifficultyScreen/
// SaveLoadScreen/TitleScreen/SystemMenuLoopScreen (F4-1b.1..4): tudo que era variavel
// LOCAL do while(true) antigo virou MEMBRO; enter() cria os recursos GL/UiLayer/audio/
// sprites (E roda TODOS os self-tests de diagnostico que antes assentavam a cena ANTES
// do 1o loop - GUSWORLD_BATTLE_*_SELFTEST, GUSWORLD_BATTLE_PUMP_TO/AIM - ZERO mudanca
// de comportamento, so de escopo); tick()/handle_event() despacham pela FASE corrente;
// exit() libera (E cacheia o CombatOutcome final ANTES de destruir a BattleScene - ver
// o comentario de exit() abaixo).
//
// CHOKE-POINT DE SAIDA INTOCADO: *out_outcome/*out_quit_requested continuam escritos
// pelo WRAPPER (run_battle_preview_embedded_gl_current, agora fino), DEPOIS que
// run_screen_state() retorna - MESMA ordem das linhas ~1681-1689 do while(true) antigo
// (out_outcome PRIMEIRO, out_quit_requested DEPOIS; a Maestro distingue "abortou" de
// "derrota" por esses 2 out-params).

#include "gus/app/screens/battle_preview.hpp"

#include <algorithm>  // std::min (fracao t dos fades)
#include <cstdlib>  // std::getenv
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <SDL3/SDL.h>

#include "gus/app/screen_state.hpp"  // F4-1b.5: ScreenState/run_screen_state/EventSyncHook
#include "gus/app/screens/battle_cockpit_rml.hpp"  // AC-E11 A2: montagem RML do cockpit
#include "gus/app/screens/battle_cockpit_verb_ids.hpp"  // GLINTFX-CLICK: id->indice de verbo
#include "gus/app/screens/battle_hud_model.hpp"  // status_icon_file/index
#include "gus/app/screens/battle_layout.hpp"     // arena_layout (selftest de mouse A2)
#include "gus/app/boot_pixel_overlay.hpp"  // sequencia de frames da transicao (M7-COSTURA Inc 2c)
#include "gus/app/screens/battle_scene.hpp"
// REVERT (BATTLE-ESC-PAUSE-ACTOR-LIST, 2026-07-05): o HOST REAL nao chama mais
// run_system_menu_loop_gl_current() (crash real ao vivo, 2o UiLayer RmlUi aninhado - ver
// BattleScreen::handle_event_main_ abaixo). Include MANTIDO de proposito (nao-orfao, sem
// warning/erro de build - so um header, nao gera unused-var): o modulo system_menu_loop.*
// fica intacto e reutilizavel se o nesting da lib for resolvido no futuro.
#include "gus/app/screens/system_menu_loop.hpp"
#include "gus/app/screens/battle_ui_sfx.hpp"  // SFX-COCKPIT: dono dos blips de UI do cockpit
#include "gus/app/screens/ui_hover.hpp"  // COCKPIT-SFX-HOVER-CLIQUE: edge-detect POCO do som de hover
#include "gus/core/asset_paths.hpp"             // caminhos de asset centralizados
#include "gus/domain/combat/combat_enums.hpp"  // StatusId
#include "gus/platform/audio/audio_engine.hpp"     // AudioEngine (M6 F3, ADR-011)
#include "gus/platform/fs/settings_file_store.hpp"  // MENU-PAUSA-CONFIG-SOM: resolve_settings_dir
#include "gus/platform/render2d/render2d_gl3.hpp"  // ADR-009 GL3: backend OpenGL da arena
// FRAMEGRAB-7-SITIOS (2026-07-30): so gl3_load_functions (glad, ~linha 1745)
// ainda vem daqui - as 4 capturas de backbuffer deste arquivo (hover/anim/
// sprite-selftest + SMOKE VISUAL) migraram pra glintfx::UiLayer::
// capture_frame(), removendo gl3_read_backbuffer_rgba. gl3_loader.hpp
// PERMANECE incluido (diferente de difficulty_menu_loop.cpp/title_menu_loop.cpp/
// save_load_menu_loop.cpp, onde o include saiu por completo) porque
// gl3_load_functions carrega os ponteiros GL (glad) que a arena (Render2dGl3)
// precisa - preocupacao TOTALMENTE separada da leitura de backbuffer, fora do
// escopo desta migracao.
#include "gus/platform/rmlui/gl3_loader.hpp"

// ADR-010 F3: glintfx::UiLayer (embed mode) e o UNICO motor de UI/HUD - o backend RmlUi
// vendorizado (RmlUiHud) foi aposentado. Compilado e linkado SEMPRE (app/ linka
// glintfx::glintfx incondicional). A arena (Render2dGl3) e o gl3_loader continuam.
#include <glintfx/ui_event.hpp>
#include <glintfx/ui_layer.hpp>

// stb_image_write: captura de frame (PNG) para o SMOKE VISUAL do ADR-009 (comparar o
// jogo com o mock). IMPLEMENTACAO definida UMA vez aqui (camada app/, fora do hot path).
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// AC-E11 A2 (ADR-019): o staging da pasta de fontes do cockpit BAKED/LIVE e
// <filesystem>/<fstream> (tempfile do RML) MORARAM pra battle_cockpit_rml.cpp - so quem
// escreve o .rml precisa deles agora. (ASSETS-FONTE-TELAS-GEMEO, 2026-08-05: la a macro
// GUSWORLD_FONTS_DIR deixou de ser lida a mao - o caminho vem do porteiro de assets via
// gus::app::screens::stage_ui_fonts.) A familia "Pixel
// Operator Mono" e registrada AQUI (load_font_face, ver mais abaixo) - FONT-EXTEND-GLITCH,
// 2026-07-29.

namespace gus::app::screens {

namespace {

constexpr int kWindowW = 1920;  // 960x540 * 2 (escala inteira x2 = 1080p, D1)
constexpr int kWindowH = 1080;

}  // namespace

// AC-E11 A2/A3 (ADR-019): a montagem RML/RCSS do cockpit (F1, load_cockpit_rml e as 3
// variantes smoke/baked/live) MOROU pra battle_cockpit_rml.cpp, e a resolucao de paths/
// loaders de asset (join/resolve_asset_dir/retrato_file_for/etc) MOROU pra
// battle_assets.cpp - ver gus/app/screens/battle_cockpit_rml.hpp e battle_assets.hpp.
// battle_preview.cpp inclui os headers e so CONSOME essas funcoes daqui pra baixo - zero
// mudanca de comportamento, so de arquivo.

// AC-E11 A1 (ADR-019): sdl_to_glintfx/battle_cockpit_verb_click/battle_mouse_click/
// battle_mouse_hover/battle_digit_for_key/battle_key_down MORARAM pra battle_input.cpp
// (roteamento de input isolado da casca de render/setup, ver gus/app/screens/
// battle_input.hpp). battle_preview.cpp inclui o header e so CONSOME essas funcoes daqui
// pra baixo - zero mudanca de comportamento, so de arquivo.

// ---------------------------------------------------------------------------------
// F4-1b.5: FSM DE FASE PURA (declarada no .hpp) - ver o comentario grande la pro
// contrato completo. Implementacao TRIVIAL de proposito (2 switch/if puros) - toda a
// complexidade de "o QUE cada fase faz" fica em BattleScreen::tick_*/handle_event_*
// abaixo; esta FSM SO decide "QUANDO trocar de fase".

BattlePhase battle_phase_initial(bool fade_in_enabled) noexcept {
    return fade_in_enabled ? BattlePhase::FadeIn : BattlePhase::Main;
}

BattlePhase battle_phase_next(BattlePhase current, bool quit, bool phase_done,
                              bool fade_out_enabled) noexcept {
    if (quit) {
        return BattlePhase::Done;
    }
    if (!phase_done) {
        return current;
    }
    switch (current) {
        case BattlePhase::FadeIn:
            return BattlePhase::Main;
        case BattlePhase::Main:
            return fade_out_enabled ? BattlePhase::FadeOut : BattlePhase::Done;
        case BattlePhase::FadeOut:
            return BattlePhase::Done;
        case BattlePhase::Done:
            return BattlePhase::Done;
    }
    return BattlePhase::Done;
}

bool battle_screen_should_close_on_event(const SDL_Event& ev) noexcept {
    return ev.type == SDL_EVENT_QUIT;
}

namespace {

// F4-1b.5: o ScreenState de PRODUCAO do host da batalha (unico chamador de producao:
// run_battle_preview_embedded_gl_current, abaixo). Extracao BEHAVIOR-PRESERVING do
// corpo de run_battle_preview_embedded_gl_current pre-conversao (ver o historico git) -
// TODO estado que antes era variavel local de uma unica funcao GIGANTE agora e MEMBRO;
// enter() faz TODO o setup (arena/glintfx/audio/sprites/traducao) + roda os self-tests
// de diagnostico que antes assentavam a cena ANTES do 1o loop; tick()/handle_event()
// despacham pela FASE corrente (phase_); exit() cacheia o outcome final e libera os
// recursos GL/UiLayer (EXCLUSIVIDADE DO UILAYER, ver gus/app/screen_state.hpp).
class BattleScreen final : public gus::app::ScreenState {
public:
    BattleScreen(SDL_Window* window, gus::platform::audio::AudioEngine* external_audio,
                 float fade_in_seconds, float fade_out_seconds)
        : window_(window),
          external_audio_(external_audio),
          fade_in_seconds_(fade_in_seconds),
          fade_out_seconds_(fade_out_seconds) {}

    // ATENCAO - CALLBACK-DTOR-DESARME (2026-08-05): SEGUNDA linha de defesa, que se
    // SOMA a (nao substitui) a ordem de declaracao de `ui_` no fim da lista de membros
    // (CALLBACK-DTOR-ORDER, gate tools/callback_dtor_order.py). A ordem e estrutural
    // mas fragil socialmente - depende do comentario ser lido, e o gate dela so conhece
    // os membros de HOJE (um membro NOVO declarado depois de `ui_` passa batido). O
    // desarme e local, explicito e INSENSIVEL a ordem de membro.
    // PRIMEIRA instrucao de proposito: ~UiLayer() descarrega os documentos do RmlUi, o
    // que recalcula a hover chain e pode emitir UM ultimo evento de clique - a lambda de
    // set_click_callback() derreferencia *scene_ e audio_ptr_, entao ela nao pode rodar
    // depois que o teardown comecou. Contrato do glintfx v0.30.0, explicito deles:
    // "nenhum consumidor e chamado de volta depois de pedir para ser desligado" - passar
    // nullptr e suportado, nao gambiarra (o proprio ~UiLayer::Impl() deles faz
    // clock.set_cursor_callback(nullptr) como 1a instrucao, glintfx/src/ui_layer.cpp:138;
    // o listener guarda com `if (!cb_ || !*cb_) return;`, glintfx/src/rml/bootstrap.cpp,
    // ou seja std::function vazio = no-op identico a "nunca houve callback").
    // `if (ui_)`: exit() ja faz ui_.reset() no caminho normal e enter() pode nem ter
    // chegado ao emplace() - desarmar tem de ser seguro com o optional VAZIO.
    // Gate automatico: tools/callback_dtor_disarm.py (roda no tools/check.sh).
    ~BattleScreen() override {
        if (ui_) ui_->set_click_callback(nullptr);
    }

    void enter() override;
    void handle_event(const SDL_Event& ev) override;
    void tick(float dt) override;
    [[nodiscard]] bool finished() const override { return phase_ == BattlePhase::Done; }
    void exit() override;
    [[nodiscard]] bool window_closed() const override { return window_closed_; }

    // Exposto SO pro wrapper run_battle_preview_embedded_gl_current (nao faz parte do
    // contrato ScreenState) - o CombatOutcome final, CACHEADO em exit() (a BattleScene e
    // destruida la, ver o comentario de exit()). O wrapper le isto DEPOIS que
    // run_screen_state() retorna - preserva o choke-point de saida original (ordem das
    // linhas ~1681-1689 do while(true) antigo, INTOCADA).
    [[nodiscard]] gus::domain::combat::CombatOutcome outcome() const {
        return cached_outcome_;
    }

private:
    // ---- fases (ver o header pro contrato da FSM) ----
    void tick_fade_in_(float dt);
    void tick_main_(float dt);
    void tick_fade_out_(float dt);
    void handle_event_fade_(const SDL_Event& ev);
    void handle_event_main_(const SDL_Event& ev);

    // battle_phase_next() + reseta o acumulador de tempo da fase NOVA.
    void advance_phase_(bool phase_done) {
        phase_ = battle_phase_next(phase_, window_closed_, phase_done,
                                   fade_out_seconds_ > 0.0f);
        phase_elapsed_ = 0.0f;
    }

    // COCKPIT-SFX-HOVER-CLIQUE: hit-test/edge-detect do SOM de hover dos pills de verbo -
    // chamado tanto pelo SDL_EVENT_MOUSE_MOTION real (fase Main) quanto pelo self-test
    // headless (ui_sfx_selftest_, dentro de tick_main_) - MESMO caminho de codigo prova o
    // comportamento, sem duplicar (espelha handle_mouse_motion de system_menu_loop.cpp).
    // O VISUAL do hover ja e nativo do glintfx (process_event(MouseMove)); AQUI so o SOM.
    // GATE: so soa quando o MENU DE VERBOS esta de fato interativo (vez do jogador, sem
    // mira/escolha-de-ator/abertura) - mesma condicao em que a pill responde ao clique.
    //
    // SFX-COCKPIT (2026-08-06): o edge-detect + o play_sfx SAIRAM daqui pra
    // gus/app/screens/battle_ui_sfx.hpp; o que sobrou nesta funcao e' a unica parte que
    // PRECISA de GL/glintfx (perguntar as caixas dos pills ao UiLayer). Motivo: enquanto
    // o play_sfx morava aqui dentro (classe em namespace anonimo, exigindo janela +
    // contexto GL vivos), NENHUM teste o alcancava - apaga-lo deixava a suite verde. O
    // GATE de "menu interativo" tambem passou a ser lido de battle_focus_of, fonte UNICA
    // compartilhada com o som de foco (teclado/mira/picker) - antes eram duas copias da
    // mesma condicao, que podiam divergir em silencio.
    void handle_cockpit_hover_(float mx, float my) {
        if (!glintfx_on_ || !ui_) return;
        if (gus::app::screens::battle_focus_of(*scene_).surface !=
            gus::app::screens::BattleFocusSurface::VerbMenu) {
            ui_sfx_.reset_pill_hover();  // menu nao-interativo: zera (nao "prende" o ultimo pill)
            return;
        }
        gus::app::screens::UiHoverBox boxes[gus::app::screens::kBattleVerbCount];
        for (int i = 0; i < gus::app::screens::kBattleVerbCount; ++i) {
            const glintfx::ElementBox b =
                ui_->get_element_box(gus::app::screens::kCockpitVerbElementIds[i]);
            boxes[i] = gus::app::screens::UiHoverBox{b.found, b.x, b.y, b.w, b.h};
        }
        ui_sfx_.on_pill_motion(mx, my, boxes, gus::app::screens::kBattleVerbCount);
    }

    // ---- config de construcao (imutaveis apos o ctor) ----
    SDL_Window* window_;
    gus::platform::audio::AudioEngine* external_audio_;
    float fade_in_seconds_;
    float fade_out_seconds_;

    // ---- FSM de fase ----
    BattlePhase phase_ = BattlePhase::Main;  // battle_phase_initial() decide o real em enter()
    float phase_elapsed_ = 0.0f;  // segundos acumulados NESTA fase (soma de dt por tick)

    // ---- recursos de render/GL (enter() cria, exit() libera) ----
    std::optional<gus::platform::render2d::Render2dGl3> renderer_;
    gus::app::BootPixelOverlay boot_overlay_;
    int pw0_ = kWindowW;
    int ph0_ = kWindowH;

    // ---- glintfx (ADR-010 F3) ----
    // ui_ NAO mora aqui: ver CALLBACK-DTOR-ORDER no FIM da lista de membros.
    bool glintfx_on_ = false;
    bool glintfx_live_ = false;
    float glintfx_dp_override_ = 0.0f;

    // ---- audio (M6/M7-COSTURA Inc 2, ADR-011) ----
    std::optional<gus::platform::audio::AudioEngine> local_audio_engine_;
    gus::platform::audio::AudioEngine* audio_ptr_ = nullptr;
    gus::platform::audio::SoundId ui_hover_sfx_id_ = gus::platform::audio::kInvalidSound;
    gus::platform::audio::SoundId ui_click_sfx_id_ = gus::platform::audio::kInvalidSound;
    // SFX-COCKPIT-AJUSTES: blip de RECUSA (verbo sem AP) - kMenuBlockedSfxFile, o mesmo
    // do card Hardcore bloqueado da tela de dificuldade.
    gus::platform::audio::SoundId ui_blocked_sfx_id_ = gus::platform::audio::kInvalidSound;
    // SFX-ABERTURA-TIMBRE: confirmacao PESADA (kUiConfirmSfxFile) - o timbre que o lider
    // escolheu em 2026-08-06 pro Enter que "Encara" na abertura. Quem decide QUE slot a
    // abertura toca e' kBattleOpeningSfxSlot (battle_ui_sfx.hpp), nao esta linha.
    gus::platform::audio::SoundId ui_confirm_sfx_id_ = gus::platform::audio::kInvalidSound;
    // SFX-COCKPIT: dono de TODO play_sfx de UI desta tela (hover de pill, hover de foco -
    // mira/picker/menu por teclado -, e clique em qualquer canal). Ver battle_ui_sfx.hpp.
    gus::app::screens::BattleUiSfx ui_sfx_;

    // ---- traducao + cena ----
    gus::app::i18n::Translator translator_;
    std::optional<BattleScene> scene_;

    // ---- estado do loop principal (Main) ----
    bool running_ = true;
    bool window_closed_ = false;
    int frame_no_ = 0;
    bool fxtest_ = false;

    // ---- diagnosticos/self-tests (env-gated; ver o header original pro racional de
    // CADA um - MESMOS nomes/semantica, so viraram membro em vez de variavel local) ----
    const char* capture_path_ = nullptr;
    int capture_at_frame_ = 20;
    int max_frames_ = 0;
    bool autostart_ = false;

    const char* hover_selftest_prefix_ = nullptr;
    bool hover_selftest_ = false;
    int hover_phase_ = 0;
    int hover_phase_frame_ = 0;

    bool ui_sfx_selftest_ = false;
    int ui_sfx_settle_frame_ = 0;
    bool ui_sfx_done_ = false;

    const char* anim_selftest_prefix_ = nullptr;
    bool anim_selftest_ = false;
    int anim_f0_ = -1;
    std::string anim_attacker_id_;
    std::string anim_target_id_;
    bool anim_done_cast_ = false;
    bool anim_done_melee_ = false;

    const char* sprite_selftest_prefix_ = nullptr;
    bool sprite_selftest_ = false;
    int sprite_f0_ = -1;
    bool sprite_done_drive_ = false;

    // ---- desfecho (cacheado em exit(), ANTES de scene_ ser destruida) ----
    gus::domain::combat::CombatOutcome cached_outcome_ =
        gus::domain::combat::CombatOutcome::Ongoing;

    // ---- glintfx: a UiLayer (declarada por ULTIMO de proposito) ----
    // ATENCAO - CALLBACK-DTOR-ORDER (2026-08-05): ui_ e o ULTIMO membro DE
    // PROPOSITO. Nao mova pra cima "por organizacao" (o lugar "natural" seria
    // junto de glintfx_on_/glintfx_live_, la em cima) - a posicao E o conserto.
    // Membro morre na ordem INVERSA da declaracao, entao declarado por ultimo o
    // ui_ e o PRIMEIRO a ser destruido, ANTES de scene_/local_audio_engine_/
    // audio_ptr_/ui_click_sfx_id_ - exatamente o que a lambda registrada em
    // set_click_callback() alcanca (ela captura `this` e derreferencia *scene_
    // e audio_ptr_). Importa porque ~UiLayer() descarrega os documentos do
    // RmlUi, o que RECALCULA a hover chain e pode emitir UM ultimo evento: o
    // glintfx levou esse mesmo use-after-free ao vivo (achado so pelo ASan;
    // passou na revisao, na suite local e em 3 dos 4 jobs do CI pesado deles).
    // No caminho normal exit() ja destroi ui_ com tudo vivo; a janela e o
    // caminho em que exit() NAO roda - ele esta solto na ultima linha de
    // run_screen_state() (app/src/screen_state.cpp), FORA de guard RAII, e uma
    // excecao escapando de enter()/tick()/handle_event() desenrola a pilha,
    // pula o exit() e vai direto pra ~BattleScreen. Ali so a ordem de
    // declaracao protege. Bonus: alinha a destruicao implicita com a ordem que
    // exit() ja faz a mao (ui_ ANTES de scene_/renderer_/local_audio_engine_).
    // Gate automatico: tools/callback_dtor_order.py (roda no tools/check.sh).
    std::optional<glintfx::UiLayer> ui_;
};

void BattleScreen::enter() {
    // FIX BUG-3 (playtest ao vivo do lider: fechar a janela durante a batalha reabria a
    // cidade em LOOP INFINITO): running_ comeca true; quem vira false e window_closed_
    // (via handle_event, no MESMO handler de SDL_EVENT_QUIT de sempre).

    // DIAGNOSTICO DE EFEITOS (ADR-009 #1): GUSWORLD_RMLUI_FXTEST=1 carrega um doc com
    // efeitos MAXIMAMENTE OBVIOS e renderiza SO o HUD (sem arena, fundo preto), pra
    // provar SE os shaders de gradiente/box-shadow chegam a tela neste setup GL.
    fxtest_ = [] {
        const char* e = std::getenv("GUSWORLD_RMLUI_FXTEST");
        return e != nullptr && e[0] == '1';
    }();

    renderer_.emplace(/*gl_active=*/true);

    // M7-COSTURA Inc 2c: sequencia de frames do boot pixelizado (substitui o glitch
    // procedural - o lider VETOU o visual ao vivo, "pareceu bug"). Carregada UMA VEZ
    // por entrada na batalha (TextureId locais a ESTE Render2dGl3) e reusada nos 2
    // loops de fade (entrada kIn + saida kOut).
    boot_overlay_.load(*renderer_, resolve_asset_dir(gus::core::assets::kVfxBootPixelDir));

    // ====================================================================
    // ADR-010 F3: glintfx::UiLayer (embed mode) e o UNICO motor de UI/HUD, compondo POR
    // CIMA da arena com efeitos nativos (gradiente/box-shadow/glow). A arena desenha
    // primeiro (backbuffer), o glintfx compoe por cima (layer -> backbuffer), o swap e
    // unico (SDL_GL_SwapWindow). Opt-out (debug so-arena): GUSWORLD_RMLUI_OFF=1.
    // ====================================================================
    const bool rmlui_opt_out = [] {
        const char* e = std::getenv("GUSWORLD_RMLUI_OFF");
        return e != nullptr && e[0] == '1';
    }();
    SDL_GetWindowSizeInPixels(window_, &pw0_, &ph0_);
    {
        int ww = 0, wh = 0;
        SDL_GetWindowSize(window_, &ww, &wh);
        std::cout << "BattlePreview: [scale] janela logica=" << ww << "x" << wh
                  << " pixels=" << pw0_ << "x" << ph0_
                  << " dp_ratio=" << (static_cast<float>(pw0_) / 960.0f) << "\n";
    }

    // ====================================================================
    // ADR-010 F3: glintfx::UiLayer (embed mode) - UNICO motor de UI/HUD. Anexa ao
    // contexto GL JA corrente; compose-only no loop (sem clear, sem swap). load_gl=true:
    // o glintfx usa gl3w (tabela de ponteiros PROPRIA, independente do glad que o
    // GusEngine carregou em gl3_load_functions); por isso PRECISA carregar a sua.
    // ====================================================================
    if (!rmlui_opt_out) {
        // ADR-010 F2b (DEFAULT): cockpit REAL pelo UiLayer dirigido por DATA-MODEL
        // (valores VIVOS por frame). Modos de debug por env:
        //   GUSWORLD_GLINTFX_SMOKE=1 -> smoke trivial (compose puro, sem assets).
        //   GUSWORLD_GLINTFX_BAKED=1 -> cockpit F2a com valores LITERAIS (sem binding).
        //   GUSWORLD_GLINTFX_INTRO=1 -> (so no BAKED) baka o brasao no lugar do combate.
        const bool glintfx_smoke = [] {
            const char* e = std::getenv("GUSWORLD_GLINTFX_SMOKE");
            return e != nullptr && e[0] == '1';
        }();
        const bool glintfx_baked = [] {
            const char* e = std::getenv("GUSWORLD_GLINTFX_BAKED");
            return e != nullptr && e[0] == '1';
        }();
        const bool glintfx_intro = [] {
            const char* e = std::getenv("GUSWORLD_GLINTFX_INTRO");
            return e != nullptr && e[0] == '1';
        }();
        glintfx_dp_override_ = [] {
            const char* e = std::getenv("GUSWORLD_GLINTFX_DP");
            return (e != nullptr && e[0] != '\0') ? static_cast<float>(std::atof(e)) : 0.0f;
        }();
        const float dp_ratio = glintfx_dp_override_ > 0.0f
                                   ? glintfx_dp_override_
                                   : static_cast<float>(pw0_) / 960.0f;
        ui_.emplace(glintfx::UiLayer::Config{/*logical_width=*/960,
                                             /*logical_height=*/540,
                                             /*load_gl=*/true,
                                             /*dp_ratio=*/dp_ratio});
        if (ui_->ok()) {
            std::string rml_path;
            std::string base_url;
            if (glintfx_smoke) {
                rml_path = write_smoke_glintfx_rml();  // px puros, sem assets/base-url
            } else if (glintfx_baked) {
                rml_path = write_baked_cockpit_rml(glintfx_intro);
                base_url = glintfx_cockpit_stage_dir();  // dir com doc+fontes+sprites
            } else {
                // LIVE: cria o data-model + LIGA os bindings ANTES do load (ordem
                // obrigatoria, secao 6 da doc de embed: create_data_model -> bind_* ->
                // load -> set_*). As views de data-binding sao compiladas no load; bind_*
                // apos o load retornaria false. Initials cobrem o 1o frame (intro=true);
                // os set_* vivos vem na fase Main (a cena so existe depois deste bloco).
                glintfx_live_ = true;
                ui_->create_data_model("hud");
                ui_->bind_bool("intro", true);
                ui_->bind_string("nome", "Gus");
                ui_->bind_string("role", "VETOR DO GAMBITO");
                ui_->bind_number("hp", 55);
                ui_->bind_number("hp_max", 55);
                ui_->bind_number("sel", 2);  // Atacar (default do BattleMenu)
                ui_->bind_string("verb", "ATACAR");
                ui_->bind_string("alvo", "-");
                // RETRATO-VIVO: caminho flat do retrato da moldura. Inicial = Gus no-bg
                // (1o frame/intro); o alimentador troca por frame conforme o ator ativo.
                ui_->bind_string("retrato_src", "retrato_gus_combate_nobg.png");
                ui_->bind_list("log");
                rml_path = write_live_cockpit_rml();
                base_url = glintfx_cockpit_stage_dir();  // dir com doc+fontes+sprites
            }
            // base-url ANTES do load (pra o doc tambem resolver via ele). Caminhos
            // relativos -> base_url/path; absolutos passam direto.
            if (!base_url.empty()) {
                ui_->set_asset_base_url(base_url.c_str());
                // FONT-EXTEND-GLITCH (2026-07-29): registra a familia "Pixel Operator
                // Mono" (regular+bold) UMA vez via glintfx::UiLayer::load_font_face (API
                // nova, v0.24.0) - mata o @font-face injetado por string que
                // write_baked_cockpit_rml/write_live_cockpit_rml faziam antes (ver
                // battle_cockpit_rml.cpp). Registro e process-wide RmlUi state (sem
                // ordem vs. load() - ver ui_layer.hpp), mas chamamos ANTES pra manter a
                // mesma ordem visual do @font-face-logo-apos-<style> antigo. `family`
                // SEMPRE explicito (armadilha de assimetria de motor documentada em
                // font_face.hpp: sem family, FontEngine::Own derivaria do stem do
                // arquivo enquanto o FreeType que rodamos le a tabela 'name' do SFNT -
                // passar o mesmo literal que o RCSS ja referencia neutraliza os dois).
                // Os .ttf ja foram copiados pro stage por write_baked_cockpit_rml/
                // write_live_cockpit_rml (SMOKE nao entra aqui - base_url vazio).
                if (!ui_->load_font_face(glintfx::FontFaceDesc{
                        /*path=*/"PixelOperatorMono.ttf",
                        /*family=*/"Pixel Operator Mono"})) {
                    std::cerr << "BattlePreview: [glintfx] load_font_face(PixelOperator"
                                 "Mono.ttf) falhou (arquivo ausente/invalido no stage) - "
                                 "cockpit cai no fallback de fonte do RmlUi.\n";
                }
                if (!ui_->load_font_face(glintfx::FontFaceDesc{
                        /*path=*/"PixelOperatorMono-Bold.ttf",
                        /*family=*/"Pixel Operator Mono",
                        /*style=*/glintfx::FontStyle::Normal,
                        /*weight=*/glintfx::FontWeight::Bold})) {
                    std::cerr << "BattlePreview: [glintfx] load_font_face(PixelOperator"
                                 "Mono-Bold.ttf) falhou (arquivo ausente/invalido no "
                                 "stage) - cockpit cai no fallback de fonte do RmlUi.\n";
                }
            }
            ui_->load(rml_path.c_str());
            ui_->set_viewport(pw0_, ph0_);    // pixels reais do backbuffer
            ui_->set_dp_ratio(dp_ratio);      // logico 960x540 -> pixels reais
            glintfx_on_ = true;
            std::cout << "BattlePreview: [glintfx] UiLayer ATIVO (embed, load_gl=true) "
                      << (glintfx_smoke   ? "[SMOKE]"
                          : glintfx_baked ? (glintfx_intro
                                                 ? "[cockpit BAKED: intro/brasao]"
                                                 : "[cockpit BAKED: combate]")
                                          : "[cockpit LIVE: data-model]")
                      << " viewport=" << pw0_ << "x" << ph0_
                      << " dp_ratio=" << dp_ratio << " RML=" << rml_path << "\n";
        } else {
            std::cerr << "BattlePreview: [glintfx] UiLayer::ok()=false (attach falhou) "
                         "- caindo SEM UI neste run\n";
            ui_.reset();
        }
    }

    // AUDIO (M6 F3/F4, ADR-011; ownership revisada M7-COSTURA Inc 2, ADR-012 decisao
    // 5): dois modos.
    //   external_audio_ == nullptr (--battle STANDALONE + todo selftest/captura, ver
    //     header): o AudioEngine e DONO da CASCA (comportamento de SEMPRE, INTOCADO)
    //     - local_audio_engine_ construido AQUI, device_active=true tenta o hardware
    //     real, degradacao graciosa se indisponivel. Toca a musica (loop+fade-in) e
    //     para no choke-point de saida (ver exit()).
    //   external_audio_ != nullptr (a Maestro, M7-COSTURA Inc 2): usa o engine DELA
    //     (ponteiro nao-dono) - NAO cria device novo e NAO toca/para musica aqui.
    if (external_audio_ == nullptr) {
        local_audio_engine_.emplace(/*device_active=*/true);
        audio_ptr_ = &(*local_audio_engine_);
    } else {
        audio_ptr_ = external_audio_;
    }

    // SFX do hit (M6 F3): load_sfx UMA vez por entrada na batalha (NUNCA no frame do
    // contato - decodificar e caro), em AMBOS os modos.
    const std::string hit_sfx_path = resolve_hit_sfx_path();
    const gus::platform::audio::SoundId hit_sfx_id =
        audio_ptr_->load_sfx(hit_sfx_path.c_str());

    // COCKPIT-SFX-HOVER-CLIQUE: blips de UI do cockpit (hover + clique nos pills de
    // verbo) - REUSA os MESMOS 2 arquivos do menu de sistema. load_sfx UMA VEZ por
    // entrada na batalha, em AMBOS os modos de audio.
    const std::string ui_hover_sfx_path =
        resolve_ui_sfx_path(gus::core::assets::kMenuHoverSfxFile);
    const std::string ui_click_sfx_path =
        resolve_ui_sfx_path(gus::core::assets::kMenuClickSfxFile);
    // SFX-COCKPIT-AJUSTES: 3o blip - o de RECUSA (verbo sem AP), decisao do lider
    // 2026-08-06. ZERO asset novo: e' o MESMO kMenuBlockedSfxFile (grave/abafado) que a
    // tela de dificuldade ja toca no card Hardcore bloqueado.
    //
    // SFX-ABERTURA-TIMBRE: 4o blip - a confirmacao PESADA da ABERTURA (kUiConfirmSfxFile),
    // timbre escolhido pelo lider em 2026-08-06. ZERO asset novo: o wav ja existia no kit
    // provisorio (curadoria F2) e nao era carregado por tela nenhuma ate aqui.
    //
    // A ORDEM destes load_sfx e' CONTRATO com app/tests/battle_preview_interaction_test.cpp
    // (SoundId e' 1-based na ordem de load_sfx): 1=hit, 2=hover, 3=clique, 4=bloqueado,
    // 5=confirmacao. Cada novo entra no FIM de proposito - inserir antes deslocaria os ids
    // que aqueles testes ja comparam contra last_sfx_id().
    const std::string ui_blocked_sfx_path =
        resolve_ui_sfx_path(gus::core::assets::kMenuBlockedSfxFile);
    const std::string ui_confirm_sfx_path =
        resolve_ui_sfx_path(gus::core::assets::kUiConfirmSfxFile);
    ui_hover_sfx_id_ = audio_ptr_->load_sfx(ui_hover_sfx_path.c_str());
    ui_click_sfx_id_ = audio_ptr_->load_sfx(ui_click_sfx_path.c_str());
    ui_blocked_sfx_id_ = audio_ptr_->load_sfx(ui_blocked_sfx_path.c_str());
    ui_confirm_sfx_id_ = audio_ptr_->load_sfx(ui_confirm_sfx_path.c_str());
    // SFX-COCKPIT: liga o dono dos blips de UI. Ponteiro NAO-DONO do engine (mesmo padrao
    // de BattleScene::set_audio); daqui pra frente NENHUM play_sfx de UI sai desta casca.
    ui_sfx_.bind(audio_ptr_, ui_hover_sfx_id_, ui_click_sfx_id_, ui_blocked_sfx_id_,
                 ui_confirm_sfx_id_);
    std::cout << "BattlePreview: [audio] SFX de UI (cockpit) hover "
              << (ui_hover_sfx_id_ != gus::platform::audio::kInvalidSound ? "carregado"
                                                                          : "AUSENTE")
              << " / clique "
              << (ui_click_sfx_id_ != gus::platform::audio::kInvalidSound ? "carregado"
                                                                          : "AUSENTE")
              << " / bloqueado "
              << (ui_blocked_sfx_id_ != gus::platform::audio::kInvalidSound ? "carregado"
                                                                            : "AUSENTE")
              << " / confirmacao (abertura) "
              << (ui_confirm_sfx_id_ != gus::platform::audio::kInvalidSound ? "carregado"
                                                                            : "AUSENTE")
              << " (blips do menu de sistema + o confirm do kit provisorio)\n";

    std::cout << "BattlePreview: [audio] device "
              << (audio_ptr_->available() ? "disponivel" : "INDISPONIVEL (mudo)")
              << " - SFX de hit "
              << (hit_sfx_id != gus::platform::audio::kInvalidSound ? "carregado"
                                                                    : "AUSENTE (silencioso)")
              << " de " << hit_sfx_path
              << (external_audio_ != nullptr ? " [engine EXTERNO, Maestro]"
                                              : " [engine LOCAL, standalone]")
              << "\n";

    // MUSICA (M6 F4, ADR-011): SO no modo LOCAL (standalone) - toca em LOOP com
    // FADE-IN ao entrar, comportamento INTOCADO. No modo EXTERNO (Maestro), a musica e
    // responsabilidade INTEIRA da Maestro (crossfade cronometrado com o fade preto).
    //
    // NOTA HONESTA (kCityThemeFile): tema de CIDADE tocando na BATALHA porque e a UNICA
    // faixa do kit CC0 provisorio (F2) - serve pra PROVAR loop+fade tecnicamente.
    if (external_audio_ == nullptr) {
        const std::string music_path = resolve_music_path();
        const gus::platform::audio::SoundId music_id =
            audio_ptr_->load_music(music_path.c_str());
        constexpr float kMusicFadeInSeconds = 2.0f;
        audio_ptr_->play_music(music_id, /*loop=*/true, kMusicFadeInSeconds);
        std::cout << "BattlePreview: [audio] musica "
                  << (music_id != gus::platform::audio::kInvalidSound
                          ? "carregada (loop, fade-in " +
                                std::to_string(kMusicFadeInSeconds) + "s)"
                          : "AUSENTE (silenciosa)")
                  << " de " << music_path << "\n";
    }

    // A cena monta o encontro de demo e ja le a fila do motor.
    scene_.emplace();
    // Ponteiro NAO-DONO (mesmo padrao de set_translator/set_portraits): a cena so
    // dispara play_sfx no evento de CONTATO do golpe (F3) - nunca decodifica, nunca
    // possui o engine.
    scene_->set_audio(audio_ptr_, hit_sfx_id);
    // (A) Com o HUD externo (glintfx::UiLayer) ATIVO, a cena NAO desenha o cockpit/log a
    // mao - so arena/banner/floaters/fila. Evita cockpits sobrepostos.
    scene_->set_hud_external(glintfx_on_);

    // GLINTFX-CLICK (v0.2.5): registra o callback de clique da UI - o glintfx faz o
    // hit-test ele mesmo (o MESMO que ja move o :hover) e devolve o `id` do elemento
    // clicado; battle_cockpit_verb_click traduz esse id pra acao do motor. Captura
    // `this` (scene_/audio_ptr_/ui_click_sfx_id_ vivem tanto quanto a BattleScreen, que
    // sobrevive ate exit() rodar - MESMA garantia da lambda antiga capturando `scene`
    // por referencia + audio_ptr/ui_click_sfx_id por valor).
    if (glintfx_on_ && ui_) {
        // COCKPIT-SFX-HOVER-CLIQUE: o glintfx JA fez o hit-test nativo (o mesmo do
        // :hover) e devolveu o `id`; aqui so somamos o SFX que o clique MERECE.
        // SFX-COCKPIT-AJUSTES: um pill de verbo SEM AP passa a tocar o som de RECUSA em
        // vez do clique de confirmacao - mesma regra do teclado, mesmo choke-point.
        ui_->set_click_callback([this](const char* element_id) {
            ui_sfx_.play_outcome(battle_cockpit_verb_click(*scene_, element_id));
        });
    }

    // Carrega os retratos 48px da fila CTB (handles resolvidos pelo renderer) e os
    // entrega a cena. Cada id de ator -> seu retrato; ausencia degrada pro retangulo.
    const std::string dir = resolve_retratos_dir();
    BattlePortraitSet portraits;
    for (const auto* actor : scene_->machine().queue().order()) {
        if (actor == nullptr) {
            continue;
        }
        const std::string path = join(dir, retrato_file_for(actor->id()));
        const gus::platform::render2d::TextureId tex = renderer_->load_texture(path.c_str());
        portraits.by_id.emplace_back(actor->id(), tex);
    }
    scene_->set_portraits(std::move(portraits));

    // SPRITE SET do GUS (W3): frames de anims/ resolvidos em TextureId e entregues a
    // cena (mesmo padrao dos retratos). Ausencia (headless/sem assets) degrada pro
    // retrato placeholder - a cena decide por ator.
    if (auto gus_set = load_gus_sprite_set(*renderer_)) {
        int nframes = 0;
        for (const auto& c : gus_set->clips) {
            nframes += static_cast<int>(c.frames.size());
        }
        scene_->set_actor_sprites("gus", std::move(*gus_set));
        std::cout << "BattlePreview: sprite set do Gus carregado (" << nframes
                  << " frames de " << gus::core::assets::kGusBattleAnimsDir << ")\n";
    } else {
        std::cout << "BattlePreview: sprite set do Gus AUSENTE (retrato placeholder)\n";
    }

    // Carrega os icones de status (14px), indexados por StatusId (status_icon_index), e
    // os entrega a cena. Ausencia degrada pro quadradinho placeholder.
    const std::string sdir = resolve_status_icons_dir();
    BattleStatusIconSet status_icons;
    for (int i = 0; i < static_cast<int>(status_icons.by_index.size()); ++i) {
        const auto id = static_cast<gus::domain::combat::StatusId>(i);
        const std::string spath = join(sdir, std::string(status_icon_file(id)));
        status_icons.by_index[static_cast<std::size_t>(i)] =
            renderer_->load_texture(spath.c_str());
    }
    scene_->set_status_icons(status_icons);

    // Carrega os icones de INTENT (telegraph, incremento 5) e os entrega a cena.
    // Ausencia => marca placeholder ambar sobre o inimigo.
    const std::string sdir_intent = resolve_intent_icons_dir();
    BattleIntentIconSet intent_icons;
    intent_icons.atacar = renderer_->load_texture(join(sdir_intent, "intent_atacar.png").c_str());
    intent_icons.defender =
        renderer_->load_texture(join(sdir_intent, "intent_defender.png").c_str());
    intent_icons.aplicar_status =
        renderer_->load_texture(join(sdir_intent, "intent_aplicar_status.png").c_str());
    intent_icons.ruido =
        renderer_->load_texture(join(sdir_intent, "intent_ruido_patchzero.png").c_str());
    scene_->set_intent_icons(intent_icons);

    // Carrega o catalogo de traducao (pt_br.md) e o entrega a cena, pra os verbos do
    // menu aparecerem com NOME legivel. Ausencia => fallback (caixa colorida sem nome,
    // mas nao crasha). O Translator vive na BattleScreen (membro, sobrevive tanto
    // quanto scene_), a cena so aponta pra ele (nao-dono).
    const std::string tr_path = gus::app::i18n::resolve_translations_path();
    const bool tr_ok = translator_.load_from_file(tr_path);
    scene_->set_translator(&translator_);

    std::cout << "BattlePreview: traducao " << (tr_ok ? "carregada" : "AUSENTE (fallback)")
              << " de " << tr_path << "\n  party=" << scene_->party_count()
              << " inimigos=" << scene_->enemy_count() << " fila=" << scene_->queue_len()
              << " retratos em " << dir
              << "\n  ABERTURA: Enter = Encarar (comeca a luta) | Q = Resolver sem "
                 "encarar (placeholder)"
              << "\n  COMBATE: Cima/Baixo navega o menu | Enter/Espaco: na sua vez "
                 "confirma o verbo, senao ACELERA o ritmo | Esc: sai\n";

    // SMOKE VISUAL (ADR-009): se GUSWORLD_RMLUI_CAPTURE=<arquivo.png>, renderiza alguns
    // frames (deixa o pacing assentar) e salva 1 PNG do framebuffer, depois sai.
    capture_path_ = std::getenv("GUSWORLD_RMLUI_CAPTURE");
    // DEBUG: GUSWORLD_RMLUI_CAPTURE_FRAME=<N> adia o shot p/ o frame N.
    if (const char* cf = std::getenv("GUSWORLD_RMLUI_CAPTURE_FRAME")) {
        const int v = std::atoi(cf);
        if (v > 0) capture_at_frame_ = v;
    }

    // DIAGNOSTICO (ADR-009): GUSWORLD_RMLUI_FRAMES=N roda o LOOP INTERATIVO COMPLETO e
    // sai apos N frames.
    if (const char* mf = std::getenv("GUSWORLD_RMLUI_FRAMES")) {
        max_frames_ = std::atoi(mf);
    }
    // DIAGNOSTICO: GUSWORLD_RMLUI_AUTOSTART=1 da Encarar automaticamente no 1o frame.
    autostart_ = [] {
        const char* e = std::getenv("GUSWORLD_RMLUI_AUTOSTART");
        return e != nullptr && e[0] == '1';
    }();

    // DIAGNOSTICO/PROVA (HOVER dos pills): GUSWORLD_BATTLE_HOVER_SELFTEST=<prefixo>.
    hover_selftest_prefix_ = std::getenv("GUSWORLD_BATTLE_HOVER_SELFTEST");
    hover_selftest_ = hover_selftest_prefix_ != nullptr && hover_selftest_prefix_[0] != '\0';

    // DIAGNOSTICO/PROVA (COCKPIT-SFX-HOVER-CLIQUE): GUSWORLD_BATTLE_UI_SFX_SELFTEST=1.
    ui_sfx_selftest_ = [] {
        const char* e = std::getenv("GUSWORLD_BATTLE_UI_SFX_SELFTEST");
        return e != nullptr && e[0] == '1';
    }();

    // DIAGNOSTICO/PROVA (ANIMACAO DE COMBATE, W2): GUSWORLD_BATTLE_ANIM_SELFTEST=<prefixo>.
    anim_selftest_prefix_ = std::getenv("GUSWORLD_BATTLE_ANIM_SELFTEST");
    anim_selftest_ = anim_selftest_prefix_ != nullptr && anim_selftest_prefix_[0] != '\0';

    // DIAGNOSTICO/PROVA (SPRITE ANIMADO, W3): GUSWORLD_BATTLE_SPRITE_SELFTEST=<prefixo>.
    sprite_selftest_prefix_ = std::getenv("GUSWORLD_BATTLE_SPRITE_SELFTEST");
    sprite_selftest_ =
        sprite_selftest_prefix_ != nullptr && sprite_selftest_prefix_[0] != '\0';

    // DIAGNOSTICO/CAPTURA: GUSWORLD_BATTLE_PUMP_TO=<actor_id> conduz o combate ate esse
    // ator ser o ATIVO, ANTES do loop de exibicao.
    if (const char* pump_to = std::getenv("GUSWORLD_BATTLE_PUMP_TO")) {
        if (pump_to[0] != '\0') {
            const std::string want(pump_to);
            if (scene_->is_intro()) {
                scene_->start_combat();
            }
            for (int i = 0; i < 600; ++i) {
                const auto* a = scene_->active_actor();
                if ((a != nullptr && a->id() == want) || scene_->combat_over()) {
                    break;
                }
                if (scene_->waiting_player_input()) {
                    if (scene_->is_choosing_actor()) {
                        scene_->actor_picker_confirm();
                    }
                    for (int k = 0;
                         k < 8 && scene_->menu().selected_verb() != BattleVerb::Atacar; ++k) {
                        scene_->menu_move(+1);
                    }
                    scene_->menu_confirm();  // Atacar -> entra na mira
                    if (scene_->is_aiming()) {
                        scene_->aim_confirm();  // confirma o alvo -> inicia o WINDUP
                    }
                    for (int k = 0; k < 120 && scene_->player_action_in_flight(); ++k) {
                        scene_->update(1.0f / 60.0f);
                    }
                } else {
                    scene_->skip();
                    scene_->update(1.0f / 60.0f);  // bombeia 1 beat de inimigo
                }
            }
            const auto* a = scene_->active_actor();
            std::cout << "BattlePreview: [pump] alvo=" << want << " ator ativo agora="
                      << (a != nullptr ? a->id() : "?") << " fila=" << scene_->queue_len()
                      << "\n";
        }
    }

    // DIAGNOSTICO/CAPTURA: GUSWORLD_BATTLE_AIM=1 deixa a cena PARADA no MODO-MIRA de
    // [Atacar] do jogador ativo.
    const bool stop_in_aim = [] {
        const char* e = std::getenv("GUSWORLD_BATTLE_AIM");
        return e != nullptr && e[0] == '1';
    }();
    if (stop_in_aim) {
        if (scene_->is_intro()) {
            scene_->start_combat();  // Encarar
        }
        for (int i = 0; i < 240 && !scene_->combat_over() && !scene_->waiting_player_input();
             ++i) {
            scene_->skip();
            scene_->update(1.0f / 60.0f);
        }
        if (scene_->is_choosing_actor()) {
            scene_->actor_picker_confirm();
        }
        if (scene_->waiting_player_input() && !scene_->is_aiming()) {
            for (int k = 0; k < 8 && scene_->menu().selected_verb() != BattleVerb::Atacar;
                 ++k) {
                scene_->menu_move(+1);
            }
            scene_->menu_confirm();  // Atacar -> ENTRA na mira (nao confirma)
            if (const char* mv = std::getenv("GUSWORLD_BATTLE_AIM_MOVE")) {
                const int steps = std::atoi(mv);
                for (int k = 0; k < steps && scene_->is_aiming(); ++k) {
                    scene_->aim_move(+1);
                }
            }
            // GUSWORLD_BATTLE_AIM_SHIELD=<mag>: aplica um Shield de pool <mag> no ALVO
            // mirado (ponteiro mutavel da fila) so pra CAPTURAR o "-N" REDUZIDO.
            if (const char* sh = std::getenv("GUSWORLD_BATTLE_AIM_SHIELD")) {
                const int mag = std::atoi(sh);
                const auto* aimed = scene_->aim_target();
                if (mag > 0 && aimed != nullptr) {
                    for (gus::domain::combat::CombatActor* act :
                         scene_->machine().queue().order()) {
                        if (act != nullptr && act->id() == aimed->id()) {
                            act->add_status(gus::domain::combat::StatusEffect{
                                gus::domain::combat::StatusId::Shield, mag,
                                /*duration=*/1, gus::domain::combat::StackRule::Replace,
                                gus::domain::combat::CardFamily::Eletrico});
                            break;
                        }
                    }
                }
            }
        }
        const auto* t = scene_->aim_target();
        const auto* atk = scene_->active_actor();
        const int previsto = (t != nullptr && atk != nullptr)
                                 ? scene_->machine().preview_basic_attack_damage(*atk, *t)
                                 : -1;
        std::cout << "BattlePreview: [aim] modo-mira=" << (scene_->is_aiming() ? "on" : "off")
                  << " atacante=" << (atk != nullptr ? atk->id() : "?")
                  << " alvo=" << (t != nullptr ? t->id() : "?") << " dano_previsto="
                  << previsto << " (badge deve mostrar \"-" << previsto << "\")\n";
    }

    // HOVER-SELFTEST (setup): assenta ate a vez do jogador e FORCA a selecao em
    // [ATACAR] (indice 2, verbo cyan) - o PIOR CASO pra "hover nao ofuscar".
    if (hover_selftest_) {
        if (scene_->is_intro()) {
            scene_->start_combat();
        }
        for (int i = 0; i < 240 && !scene_->combat_over() && !scene_->waiting_player_input();
             ++i) {
            scene_->skip();
            scene_->update(1.0f / 60.0f);
        }
        if (scene_->is_choosing_actor()) {
            scene_->actor_picker_confirm();
        }
        if (scene_->waiting_player_input() && !scene_->is_aiming()) {
            for (int k = 0; k < 8 && scene_->menu().selected_verb() != BattleVerb::Atacar;
                 ++k) {
                scene_->menu_move(+1);
            }
        }
        std::cout << "BattlePreview: [hover-selftest] waiting_player="
                  << scene_->waiting_player_input()
                  << " sel_index=" << scene_->menu().selected_index()
                  << " (esperado 2=ATACAR); prefixo=" << hover_selftest_prefix_ << "\n";
    }

    // COCKPIT-SFX-HOVER-CLIQUE (setup do self-test de SOM): MESMA assent do hover-
    // selftest visual - assenta ate a vez do jogador.
    if (ui_sfx_selftest_) {
        if (scene_->is_intro()) {
            scene_->start_combat();
        }
        for (int i = 0; i < 240 && !scene_->combat_over() && !scene_->waiting_player_input();
             ++i) {
            scene_->skip();
            scene_->update(1.0f / 60.0f);
        }
        if (scene_->is_choosing_actor()) {
            scene_->actor_picker_confirm();
        }
        std::cout << "BattlePreview: [ui-sfx-selftest] waiting_player="
                  << scene_->waiting_player_input() << " (menu de verbos interativo esperado)\n";
    }

    // DIAGNOSTICO/PROVA (GLINTFX-CLICK, ex-Incremento A2): GUSWORLD_BATTLE_MOUSE_
    // SELFTEST=1 exercita o roteamento clique->acao SEM mouse fisico e ENCERRA sozinho
    // (running_=false ANTES de qualquer fade/loop principal rodar - MESMO
    // comportamento do while(true) antigo).
    const bool mouse_selftest = [] {
        const char* e = std::getenv("GUSWORLD_BATTLE_MOUSE_SELFTEST");
        return e != nullptr && e[0] == '1';
    }();
    if (mouse_selftest) {
        if (scene_->is_intro()) {
            scene_->start_combat();
        }
        for (int i = 0; i < 240 && !scene_->combat_over() && !scene_->waiting_player_input();
             ++i) {
            scene_->skip();
            scene_->update(1.0f / 60.0f);
        }
        if (scene_->is_choosing_actor()) {
            scene_->actor_picker_confirm();
        }
        std::cout << "BattlePreview: [mouse-selftest] pw0xph0=" << pw0_ << "x" << ph0_
                  << " waiting_player=" << scene_->waiting_player_input() << "\n";
        for (int v = 0; v < gus::app::screens::kBattleVerbCount; ++v) {
            const char* id = gus::app::screens::kCockpitVerbElementIds[v];
            const int back = gus::app::screens::cockpit_verb_index_for_click_id(id);
            std::cout << "  pill[" << v << "] " << kVerbLabels[static_cast<std::size_t>(v)]
                      << " id=" << id << " -> indice=" << back
                      << (back == v ? " OK" : " MISMATCH") << "\n";
        }
        if (scene_->waiting_player_input() && !scene_->is_aiming()) {
            // SFX-COCKPIT-AJUSTES: este self-test prova o ROTEAMENTO id->verbo, nao o som
            // - descarta o outcome de proposito (o gemeo do (void) dos self-tests de
            // mira/picker mais abaixo).
            (void)battle_cockpit_verb_click(
                *scene_, gus::app::screens::kCockpitVerbElementIds[static_cast<int>(
                             BattleVerb::Atacar)]);
            std::cout << "  CLIQUE (callback) pill ATACAR -> is_aiming="
                      << (scene_->is_aiming() ? "on" : "off") << " (esperado on)\n";
        }
        if (scene_->is_aiming()) {
            const int want = scene_->aim_count() >= 2 ? 1 : 0;
            const gus::app::screens::ArenaLayout arena = gus::app::screens::arena_layout(
                scene_->party_count(), scene_->enemy_count(), scene_->gus_party_index());
            const gus::core::spatial::Rect s = arena.enemies[static_cast<std::size_t>(want)].rect;
            const float wcx = s.x + s.w * 0.5f, wcy = s.y + s.h * 0.5f;
            const float pxcx = wcx / 960.0f * static_cast<float>(pw0_);
            const float pxcy = wcy / 540.0f * static_cast<float>(ph0_);
            const std::string alvo_antes =
                scene_->aim_target() != nullptr ? scene_->aim_target()->id() : "?";
            const int hit = scene_->aim_index_at_arena(wcx, wcy);
            // SFX-COCKPIT: este self-test SINTETICO prova o hit-test, nao o som (o
            // AudioEngine deste modo pode nem ter device) - descarta o "acionou?" de
            // proposito. O caminho de PRODUCAO que toca o blip e' handle_event_main_.
            (void)battle_mouse_click(*scene_, pxcx, pxcy, pw0_, ph0_);
            std::cout << "  CLIQUE inimigo#" << want << " (alvo pre-clique=" << alvo_antes
                      << ") world(" << wcx << "," << wcy << ") px(" << pxcx << "," << pxcy
                      << ") hit_idx=" << hit << " -> is_aiming="
                      << (scene_->is_aiming() ? "on" : "off")
                      << " (esperado off: confirmou; W2 - o contato resolve no fim "
                         "do windup)\n";
        }
        std::cout << "BattlePreview: [mouse-selftest] concluido; encerrando.\n";
        running_ = false;
    }

    // DIAGNOSTICO/PROVA (Escolha de ator, §4.1): GUSWORLD_BATTLE_ACTOR_SELFTEST=1
    // dirige cenas FRESCAS (PROPRIAS, descartadas ao sair do bloco) ate a vez do BLOCO
    // da party e injeta INPUT SINTETICO pelo MESMO roteamento do host, ENCERRA sozinho
    // (running_=false) - MESMA independencia de scene_ (a BattleScene principal da
    // BattleScreen nao e tocada por este self-test).
    const bool actor_selftest = [] {
        const char* e = std::getenv("GUSWORLD_BATTLE_ACTOR_SELFTEST");
        return e != nullptr && e[0] == '1';
    }();
    if (actor_selftest) {
        auto drive_to_picker = [](BattleScene& s) {
            if (s.is_intro()) {
                s.start_combat();
            }
            for (int i = 0; i < 600 && !s.combat_over() && !s.is_choosing_actor(); ++i) {
                s.skip();
                s.update(1.0f / 60.0f);
            }
        };
        auto choice_id = [](const BattleScene& s, int i) -> std::string {
            const auto cs = s.actor_choices();
            return (i >= 0 && i < static_cast<int>(cs.size()) &&
                    cs[static_cast<std::size_t>(i)] != nullptr)
                       ? cs[static_cast<std::size_t>(i)]->id()
                       : std::string("?");
        };

        // ---- (A) SETA move o cursor do picker (NAO confirma) ----
        {
            BattleScene sa;
            drive_to_picker(sa);
            bool dummy = true;
            const int n = sa.actor_pick_count();
            const std::string c0 =
                sa.actor_pick_target() != nullptr ? sa.actor_pick_target()->id() : "?";
            battle_key_down(sa, SDLK_DOWN, dummy);
            const std::string c1 =
                sa.actor_pick_target() != nullptr ? sa.actor_pick_target()->id() : "?";
            const bool ok = sa.is_choosing_actor() && (n < 2 || c0 != c1);
            std::cout << "BattlePreview: [actor-selftest] (A seta) choosing="
                      << sa.is_choosing_actor() << " elegiveis=" << n << " cursor: " << c0
                      << " --[DOWN]--> " << c1 << "  " << (ok ? "OK" : "FALHA")
                      << (n < 2 ? " (n<2: sem 2o alvo p/ mover)" : "") << "\n";
        }

        // ---- (B) CLIQUE no slot de um membro ESCOLHE + CONFIRMA ----
        {
            BattleScene sb;
            drive_to_picker(sb);
            const int want_idx = sb.actor_pick_count() >= 2 ? 1 : 0;
            const std::string want_id = choice_id(sb, want_idx);
            const gus::app::screens::ArenaLayout arena = gus::app::screens::arena_layout(
                sb.party_count(), sb.enemy_count(), sb.gus_party_index());
            float pxc = -1.0f, pyc = -1.0f, wcx = -1.0f, wcy = -1.0f;
            for (int k = 0; k < sb.party_count(); ++k) {
                const gus::core::spatial::Rect r = arena.party[static_cast<std::size_t>(k)].rect;
                const float cx = r.x + r.w * 0.5f, cy = r.y + r.h * 0.5f;
                if (sb.actor_pick_index_at_arena(cx, cy) == want_idx) {
                    wcx = cx;
                    wcy = cy;
                    pxc = cx / 960.0f * static_cast<float>(pw0_);
                    pyc = cy / 540.0f * static_cast<float>(ph0_);
                    break;
                }
            }
            if (pxc >= 0.0f) {
                // SFX-COCKPIT: self-test do hit-test do PICKER; o "acionou?" e' descartado
                // aqui de proposito (ver o gemeo no selftest de mira, acima).
                (void)battle_mouse_click(sb, pxc, pyc, pw0_, ph0_);
                const std::string got = sb.active_actor() != nullptr ? sb.active_actor()->id() : "?";
                const bool ok = !sb.is_choosing_actor() && got == want_id;
                std::cout << "BattlePreview: [actor-selftest] (B clique) elegivel#" << want_idx
                          << "=" << want_id << " world(" << wcx << "," << wcy << ") px(" << pxc
                          << "," << pyc << ") -> choosing=" << sb.is_choosing_actor()
                          << " ator_ativo=" << got << "  "
                          << (ok ? "OK (escolheu+confirmou)" : "FALHA") << "\n";
            } else {
                std::cout << "BattlePreview: [actor-selftest] (B clique) FALHA: nenhum slot "
                             "casou o elegivel #"
                          << want_idx << " (choosing=" << sb.is_choosing_actor()
                          << " elegiveis=" << sb.actor_pick_count() << ")\n";
            }
        }

        // ---- (C) TECLA 1/2/3 escolhe + CONFIRMA na hora ----
        {
            BattleScene sc;
            drive_to_picker(sc);
            const int nth = sc.actor_pick_count() >= 2 ? 2 : 1;
            const std::string want_id = choice_id(sc, nth - 1);
            const SDL_Keycode kc = nth == 1 ? SDLK_1 : (nth == 2 ? SDLK_2 : SDLK_3);
            bool dummy = true;
            battle_key_down(sc, kc, dummy);
            const std::string got = sc.active_actor() != nullptr ? sc.active_actor()->id() : "?";
            const bool ok = !sc.is_choosing_actor() && got == want_id;
            std::cout << "BattlePreview: [actor-selftest] (C tecla " << nth << ") elegivel#"
                      << (nth - 1) << "=" << want_id << " -> choosing=" << sc.is_choosing_actor()
                      << " ator_ativo=" << got << "  " << (ok ? "OK (escolheu+confirmou)" : "FALHA")
                      << "\n";
        }

        std::cout << "BattlePreview: [actor-selftest] concluido; encerrando.\n";
        running_ = false;
    }

    // F4-1b.5: decisao INICIAL de fase (FSM pura, ver o .hpp) - MESMO guard
    // `if (fade_in_seconds > 0.0f && running)` do while(true) antigo.
    phase_ = battle_phase_initial(fade_in_seconds_ > 0.0f && running_);
    if (phase_ == BattlePhase::Main && !running_) {
        // Main NASCE ja terminado (mouse-selftest/actor-selftest concluiram DENTRO do
        // proprio enter(), running_ ja false ANTES do 1o tick()) - MESMO comportamento
        // do while(running) antigo com running=false desde o inicio (0 iteracoes: zero
        // render/swap) - avanca direto pra FadeOut (se pedido) ou Done, SEM tick() rodar.
        advance_phase_(/*phase_done=*/true);
    }

    // NAO ha present_frame explicito aqui (ao contrario de NpcDialogueScreen): o
    // while(true) ORIGINAL desta tela so desenhava no FIM de cada iteracao - o 1o
    // tick() do runner reproduz exatamente essa 1a iteracao.
}

void BattleScreen::handle_event_fade_(const SDL_Event& ev) {
    // Pump de eventos LIMITADO a SDL_EVENT_QUIT (MESMO dos 2 `while(fading)` antigos) -
    // a batalha ainda nao "comecou"/ja "terminou" pro jogador enquanto a tela esta
    // clareando/escurecendo: teclado/mouse ficam mudos durante o fade.
    if (battle_screen_should_close_on_event(ev)) {
        window_closed_ = true;
    }
}

void BattleScreen::handle_event_main_(const SDL_Event& ev) {
    // Injeta o evento na UI glintfx (em paralelo ao roteamento de cena abaixo,
    // ambos veem o mesmo evento). O log de smoke do ADR-010 F1 (contagem de
    // eventos injetados) provou o pipeline SDL -> sdl_to_glintfx -> process_event
    // ha semanas e foi removido 2026-08-01 (LOG-RUIDO-INPUT-INJETADO): afogava o
    // terminal com uma linha por tecla, sem ser efeito de jogo nem mensagem
    // diegetica.
    if (glintfx_on_ && ui_) {
        glintfx::UiEvent ge{};
        if (sdl_to_glintfx(ev, window_, &ge)) {
            ui_->process_event(ge);
        }
    }
    if (battle_screen_should_close_on_event(ev)) {
        // FIX BUG-3: o jogador clicou no X da janela DURANTE a batalha. Isto NAO e um
        // CombatOutcome (nao e vitoria/derrota/fuga) - e um pedido pra encerrar o
        // PROGRAMA INTEIRO. window_closed_=true grava no choke-point de saida (junto
        // do outcome) - a Maestro usa isto pra NAO voltar a cidade.
        running_ = false;
        window_closed_ = true;
    } else if (ev.type == SDL_EVENT_KEY_DOWN) {
        // Roteamento de teclado EXTRAIDO pra battle_key_down (funcao-livre, testavel
        // pelo self-test sintetico; espelha battle_mouse_click). Cobre menu de verbos,
        // MODO-MIRA (§3.5), ESCOLHA DE ATOR (§4.1, prioridade + teclas 1/2/3) e Esc.
        //
        // REVERT (BATTLE-ESC-PAUSE-ACTOR-LIST, 2026-07-05): o HOST REAL volta a chamar
        // SEM `out_effect` (nullptr implicito) - o comportamento ORIGINAL de sempre,
        // pre-integracao do menu de pausa: mira cancela / preview volta pra lista /
        // lista e no-op / PILHA VAZIA fecha o viewer (running_=false). O CRASH real ao
        // vivo (lider testou): abrir o menu de pausa AQUI criava um SEGUNDO contexto/
        // UiLayer RmlUi (run_system_menu_loop_gl_current) ANINHADO enquanto o cockpit
        // da batalha (o PRIMEIRO UiLayer, ui_ acima) ainda estava VIVO - RmlUi tem
        // estado global (element pool) que NAO suporta 2 contextos simultaneos no
        // mesmo processo. Decisao do lider: reverter, SEM menu de pausa na arena por
        // ora (ver TODO.md). battle_key_down/BattleEscEffect/OpenPauseMenu CONTINUAM
        // existindo e funcionando (testadas em battle_key_routing_test.cpp com
        // out_effect explicito) - so este HOST DE PRODUCAO para de passar o ponteiro.
        //
        // SFX-COCKPIT: PARIDADE TECLADO x MOUSE (o defeito 3 da auditoria - quem joga de
        // teclado ouvia blip nos 6 outros menus do jogo e SILENCIO na batalha). Duas
        // medidas em volta do MESMO roteamento, sem duplicar regra nenhuma:
        //   CLIQUE: predicado PURO avaliado ANTES (a cena ainda no estado em que a tecla
        //     foi apertada) - depois do roteamento ja e' tarde, o estado mudou.
        //   HOVER: foco ANTES x DEPOIS (mesma receita de save_load_menu_loop.cpp) - pega
        //     setas no menu de verbos, na mira E no picker de uma vez so.
        const gus::app::screens::BattleClickOutcome outcome =
            battle_key_click_outcome(*scene_, ev.key.key);
        const gus::app::screens::BattleFocus focus_before =
            gus::app::screens::battle_focus_of(*scene_);
        battle_key_down(*scene_, ev.key.key, running_);
        ui_sfx_.play_outcome(outcome);
        ui_sfx_.on_focus_change(focus_before, gus::app::screens::battle_focus_of(*scene_));
    } else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN && ev.button.button == SDL_BUTTON_LEFT) {
        // MOUSE (A2): clique ESQUERDO aciona verbo (menu) ou alvo (mira). ADITIVO ao
        // teclado (que segue igual). O forward pro glintfx (acima) ja rolou (so
        // visual); a ACAO real vem do hit-test do host em px de janela.
        int pw = kWindowW, ph = kWindowH;
        SDL_GetWindowSizeInPixels(window_, &pw, &ph);
        // SFX-COCKPIT: clique na MIRA / no PICKER agora soa (antes: mudos - grep play_sfx
        // battle_input.cpp devolvia zero). Sem risco de som DUPLO com o pill: o callback
        // do glintfx (acima, no process_event) so aciona verbo quando NAO ha mira/picker
        // aberto, e battle_mouse_click so devolve true quando HA - as duas guardas sao
        // complementares por construcao.
        if (battle_mouse_click(*scene_, ev.button.x, ev.button.y, pw, ph)) {
            ui_sfx_.play_click();
        }
    } else if (ev.type == SDL_EVENT_MOUSE_MOTION) {
        // MOUSE (A2, hover): na mira, passar sobre um inimigo pre-seleciona (realce).
        int pw = kWindowW, ph = kWindowH;
        SDL_GetWindowSizeInPixels(window_, &pw, &ph);
        // SFX-COCKPIT: o realce que o mouse MOVE na mira/no picker agora soa - mesmo
        // choke-point de foco do teclado, logo mesma regra de dedup (parado sobre o mesmo
        // alvo nao redispara).
        const gus::app::screens::BattleFocus focus_before =
            gus::app::screens::battle_focus_of(*scene_);
        battle_mouse_hover(*scene_, ev.motion.x, ev.motion.y, pw, ph);
        ui_sfx_.on_focus_change(focus_before, gus::app::screens::battle_focus_of(*scene_));
        // COCKPIT-SFX-HOVER-CLIQUE: SOM de hover dos pills de verbo (visual :hover ja
        // saiu no process_event nativo acima). Edge-detect: toca so ao ENTRAR num pill
        // NOVO. Em px de janela == px do viewport (sem HiDPI neste alvo). NAO soma com o
        // foco acima: mouse sobre pill nao mexe em menu().selected_index(), e o gate de
        // superficie deste caminho e' exatamente VerbMenu.
        handle_cockpit_hover_(ev.motion.x, ev.motion.y);
    }
}

void BattleScreen::handle_event(const SDL_Event& ev) {
    switch (phase_) {
        case BattlePhase::FadeIn:
        case BattlePhase::FadeOut:
            handle_event_fade_(ev);
            return;
        case BattlePhase::Main:
            handle_event_main_(ev);
            return;
        case BattlePhase::Done:
            return;  // defensivo: run_screen_state nao chama apos finished()==true.
    }
}

void BattleScreen::tick_fade_in_(float dt) {
    // M7-COSTURA Inc 2 (ADR-012 decisao 5): a tela CLAREIA (overlay preto 1->0) sobre o
    // 1o frame REAL da arena+HUD ja prontos (nao um placeholder). phase_elapsed_ soma o
    // dt de CADA tick desta fase (== elapsed relativo ao SDL_GetTicksNS() de entrada do
    // `while(fading)` antigo - dt ja e tempo real desde o ultimo tick, injetado por
    // run_screen_state, MESMA semantica de wall-clock).
    phase_elapsed_ += dt;
    int fpw = kWindowW, fph = kWindowH;
    SDL_GetWindowSizeInPixels(window_, &fpw, &fph);
    if (fpw < 1 || fph < 1) {
        SDL_Delay(16);
        return;  // GUARD janela minimizada: nao desenha, nao avanca a fase ainda.
    }
    const float elapsed = phase_elapsed_;
    scene_->render(*renderer_, static_cast<float>(fpw), static_cast<float>(fph));
    if (glintfx_on_ && ui_) {
        if (fpw != pw0_ || fph != ph0_) {
            ui_->set_viewport(fpw, fph);
            if (glintfx_dp_override_ <= 0.0f) {
                ui_->set_dp_ratio(static_cast<float>(fpw) / 960.0f);
            }
            pw0_ = fpw;
            ph0_ = fph;
        }
        ui_->update();
        ui_->render();
    }
    // M7-COSTURA Inc 2c: sequencia de frames do boot pixelizado - a metade "Revealing
    // indo pra batalha" da transicao inteira, no lugar do glitch procedural (aposentado
    // - o lider VETOU o visual, "pareceu bug"). `t` = elapsed/fade_in_seconds_.
    {
        const float t = fade_in_seconds_ > 0.0f ? std::min(1.0f, elapsed / fade_in_seconds_)
                                                : 1.0f;
        boot_overlay_.draw(*renderer_, battle_screen_rect(),
                           gus::core::anim::BootPixelLeg::kToBattleRevealing, t);
    }
    SDL_GL_SwapWindow(window_);
    if (elapsed >= fade_in_seconds_) {
        std::cout << "BattlePreview: [fade] entrada (kIn) concluido em " << fade_in_seconds_
                  << "s.\n";
        advance_phase_(/*phase_done=*/true);
    }
}

void BattleScreen::tick_fade_out_(float dt) {
    // M7-COSTURA Inc 2 (ADR-012 decisao 5): a tela ESCURECE (overlay preto 0->1) sobre
    // o ULTIMO ESTADO congelado da arena+HUD (sem avancar logica de jogo - scene_ NAO e
    // atualizada; so re-desenha o mesmo frame com alpha crescente). glintfx_live_ NAO
    // alimenta os bindings vivos aqui (MESMO comportamento do `while(fading)` antigo de
    // saida - so ui_->update()/render(), sem nenhum ui_->set_*): os valores ficam
    // congelados no que a fase Main deixou.
    phase_elapsed_ += dt;
    int fpw = kWindowW, fph = kWindowH;
    SDL_GetWindowSizeInPixels(window_, &fpw, &fph);
    if (fpw < 1 || fph < 1) {
        SDL_Delay(16);
        return;
    }
    const float elapsed = phase_elapsed_;
    scene_->render(*renderer_, static_cast<float>(fpw), static_cast<float>(fph));
    if (glintfx_on_ && ui_) {
        if (fpw != pw0_ || fph != ph0_) {
            ui_->set_viewport(fpw, fph);
            if (glintfx_dp_override_ <= 0.0f) {
                ui_->set_dp_ratio(static_cast<float>(fpw) / 960.0f);
            }
            pw0_ = fpw;
            ph0_ = fph;
        }
        ui_->update();
        ui_->render();
    }
    // M7-COSTURA Inc 2c: a metade "Darkening voltando pra cidade" da transicao inteira.
    {
        const float t = fade_out_seconds_ > 0.0f
                            ? std::min(1.0f, elapsed / fade_out_seconds_)
                            : 1.0f;
        boot_overlay_.draw(*renderer_, battle_screen_rect(),
                           gus::core::anim::BootPixelLeg::kFromBattleDarkening, t);
    }
    SDL_GL_SwapWindow(window_);
    if (elapsed >= fade_out_seconds_) {
        std::cout << "BattlePreview: [fade] saida (kOut) concluido em " << fade_out_seconds_
                  << "s.\n";
        advance_phase_(/*phase_done=*/true);
    }
}

void BattleScreen::tick_main_(float dt) {
    if (!running_) {
        // MESMO guard `if (!running) break;` do while(true) antigo (checado logo apos
        // drenar a rajada de eventos): nao desenha 1 frame extra apos o evento (Esc/
        // Quit) que encerrou o loop - so avanca a fase.
        advance_phase_(/*phase_done=*/true);
        return;
    }

    // dt real desde o ultimo tick (segundos): envelhece os numeros flutuantes e dirige
    // o pacing (2 beats por turno). CLAMP anti-salto: um dt ENORME (setup pesado/1o
    // frame) faria o pacing avancar intro+anuncio+resolucao de uma vez e a tela "abriria
    // com o ataque ja feito" (bug pego pelo lider 3x). O teto de 50ms garante que nenhum
    // frame pule um beat.
    if (dt > 0.05f) {
        dt = 0.05f;
    }
    // ANIM/SPRITE-SELFTEST: dt FIXO 1/60 (script por frame deterministico; o relogio
    // real varia por maquina/driver e desalinharia as capturas).
    if (anim_selftest_ || sprite_selftest_) {
        dt = 1.0f / 60.0f;
    }
    scene_->update(dt);  // anima os floaters + pacing; nao toca a FSM

    // FIX BUG-2 (playtest ao vivo do lider: "perdi a batalha e ficou preso, so tocando
    // musica e mais nada"): qualquer desfecho TERMINAL (Victory/Defeat/Fled) encerra o
    // loop sozinho. M7-COSTURA Inc 3 (flavor da derrota): com Defeat, defeat_flavor_
    // active() segura este running_=false por kDefeatFlavorSeconds - o loop CONTINUA
    // rodando ate o timer esgotar; SO ENTAO trata como combat_over() de fato e sai.
    if (scene_->combat_over() && !scene_->defeat_flavor_active()) {
        running_ = false;
    }

    // ANIM-SELFTEST (script de ACOES por frame; as capturas ficam no fim do frame, apos
    // render+compose). Timeline (dt 1/60): frame 1 dispara o demo de cast; frame 62
    // comeca o MELEE do jogador.
    if (anim_selftest_) {
        if (frame_no_ == 1 && !anim_done_cast_) {
            anim_done_cast_ = true;
            scene_->debug_cast_demo();  // cosmetico (intro): cast + bolinha
            std::cout << "BattlePreview: [anim-selftest] f1 cast demo iniciado\n";
        }
        if (frame_no_ == 62 && !anim_done_melee_) {
            anim_done_melee_ = true;
            if (scene_->is_intro()) {
                scene_->start_combat();
            }
            for (int i = 0; i < 240 && !scene_->combat_over() &&
                            !scene_->waiting_player_input();
                 ++i) {
                scene_->skip();
                scene_->update(1.0f / 60.0f);
            }
            if (scene_->is_choosing_actor()) {
                scene_->actor_picker_confirm();
            }
            if (scene_->waiting_player_input() && !scene_->is_aiming()) {
                for (int k = 0; k < 8 && scene_->menu().selected_verb() != BattleVerb::Atacar;
                     ++k) {
                    scene_->menu_move(+1);
                }
                const auto* atk = scene_->active_actor();
                anim_attacker_id_ = atk != nullptr ? atk->id() : "?";
                scene_->menu_confirm();  // entra na mira
                const auto* tgt = scene_->aim_target();
                anim_target_id_ = tgt != nullptr ? tgt->id() : "?";
                scene_->aim_confirm();  // COMANDA: windup parte agora
                anim_f0_ = frame_no_;
                std::cout << "BattlePreview: [anim-selftest] f" << frame_no_
                          << " melee confirmado: atacante=" << anim_attacker_id_
                          << " alvo=" << anim_target_id_
                          << " in_flight=" << scene_->player_action_in_flight() << "\n";
            } else {
                std::cout << "BattlePreview: [anim-selftest] FALHA: nao chegou na vez "
                             "do jogador\n";
                running_ = false;
            }
        }
    }

    // SPRITE-SELFTEST (drive): apos capturar o idle-em-repouso (frame 8), o frame 10
    // conduz a cena ate a vez da party, navega o PICKER ate o GUS e confirma [Atacar] +
    // alvo. O dash parte AGORA (sprite_f0_).
    if (sprite_selftest_ && frame_no_ == 10 && !sprite_done_drive_) {
        sprite_done_drive_ = true;
        if (scene_->is_intro()) {
            scene_->start_combat();
        }
        for (int i = 0; i < 240 && !scene_->combat_over() && !scene_->waiting_player_input();
             ++i) {
            scene_->skip();
            scene_->update(1.0f / 60.0f);
        }
        if (scene_->is_choosing_actor()) {
            for (int k = 0; k < scene_->actor_pick_count() &&
                            (scene_->actor_pick_target() == nullptr ||
                             scene_->actor_pick_target()->id() != "gus");
                 ++k) {
                scene_->actor_picker_move(+1);
            }
            scene_->actor_picker_confirm();
        }
        const auto* atk = scene_->active_actor();
        if (scene_->waiting_player_input() && atk != nullptr && atk->id() == "gus" &&
            !scene_->is_aiming()) {
            for (int k = 0; k < 8 && scene_->menu().selected_verb() != BattleVerb::Atacar; ++k) {
                scene_->menu_move(+1);
            }
            scene_->menu_confirm();  // entra na mira
            scene_->aim_confirm();   // confirma o alvo: o dash parte AGORA
            sprite_f0_ = frame_no_;
            std::cout << "BattlePreview: [sprite-selftest] f" << frame_no_
                      << " melee do GUS confirmado; in_flight="
                      << scene_->player_action_in_flight() << "\n";
        } else {
            std::cout << "BattlePreview: [sprite-selftest] FALHA: nao chegou na vez do "
                         "GUS (ativo="
                      << (atk != nullptr ? atk->id() : "?") << ")\n";
            running_ = false;
        }
    }

    // DIAGNOSTICO: auto-Encarar (captura do estado de combate sem input).
    if (autostart_ && scene_->is_intro()) {
        scene_->start_combat();
    }

    int pw = kWindowW, ph = kWindowH;
    SDL_GetWindowSizeInPixels(window_, &pw, &ph);
    // GUARD janela minimizada (Wayland pode reportar 0): pula render+compose deste
    // frame (so pumpa eventos no proximo), evitando viewport/FBO de tamanho 0.
    if (pw < 1 || ph < 1) {
        SDL_Delay(16);
        if (!running_) {
            advance_phase_(/*phase_done=*/true);
        }
        return;
    }

    // A arena desenha no backbuffer GL (clear + draws). O HUD RmlUi-GL3 compoe POR
    // CIMA. O SWAP e UNICO, depois do HUD. FXTEST: pula a arena (so o HUD sobre fundo
    // preto) pra isolar os efeitos.
    if (fxtest_) {
        renderer_->begin_frame(gus::core::spatial::Rect{0, 0, 960, 540}, pw, ph);
        renderer_->end_frame();  // so o clear (fundo preto-ish); HUD compoe por cima
    } else {
        scene_->render(*renderer_, static_cast<float>(pw), static_cast<float>(ph));
    }
    // ADR-010 F3: glintfx compoe no slot de HUD - depois da arena, antes do swap.
    if (glintfx_on_ && ui_) {
        if (pw != pw0_ || ph != ph0_) {
            ui_->set_viewport(pw, ph);
            if (glintfx_dp_override_ <= 0.0f) {
                ui_->set_dp_ratio(static_cast<float>(pw) / 960.0f);
            }
            pw0_ = pw;
            ph0_ = ph;
        }
        // ADR-010 F2b: ALIMENTA o data-model com os valores VIVOS do motor (POCO), a
        // cada frame. O MOTOR (scene_) e a fonte de verdade.
        if (glintfx_live_) {
            ui_->set_bool("intro", scene_->is_intro());
            if (!scene_->is_intro()) {
                if (const auto* a = scene_->active_actor(); a != nullptr) {
                    ui_->set_string("nome", a->display_name().c_str());
                    // POLISH 2: o label segue o LADO do ator ativo.
                    ui_->set_string("role",
                                    a->is_player_side() ? "VETOR DO GAMBITO" : "INIMIGO");
                    ui_->set_number("hp", a->hp());
                    ui_->set_number("hp_max", a->max_hp());
                    // RETRATO-VIVO: a moldura segue o ator ATIVO.
                    ui_->set_string("retrato_src", cockpit_retrato_flat_for(*a).c_str());
                }
                const int sel = scene_->menu().selected_index();
                ui_->set_number("sel", sel);
                ui_->set_string(
                    "verb",
                    std::string(kVerbLabels[static_cast<std::size_t>(sel < 0 || sel > 5 ? 2 : sel)])
                        .c_str());
                // Alvo: 1o inimigo VIVO da fila (alvo default das acoes ofensivas).
                const char* alvo = "-";
                std::string alvo_buf;
                for (const auto* act : scene_->machine().queue().order()) {
                    if (act != nullptr && !act->is_player_side() && act->is_alive()) {
                        alvo_buf = act->display_name();
                        alvo = alvo_buf.c_str();
                        break;
                    }
                }
                ui_->set_string("alvo", alvo);
                // Log VIVO: ultimas linhas narradas pelo motor. CAP 2 entradas + now-line
                // = EXATAS 3 linhas de log.
                const std::vector<gus::app::screens::LogLine> lines = scene_->log_lines(2);
                std::vector<const char*> ptrs;
                ptrs.reserve(lines.size());
                for (const auto& l : lines) ptrs.push_back(l.text.c_str());
                ui_->set_list("log", ptrs.data(), ptrs.size());
            }
        }
        // HOVER-SELFTEST (injecao): a cada frame, coloca o ponteiro SINTETICO onde a
        // fase pede, ANTES do update().
        if (hover_selftest_) {
            float mx = static_cast<float>(pw) * 0.80f;  // fora da coluna (fases 0 e 3)
            float my = static_cast<float>(ph) * 0.50f;
            const int hover_pill = hover_phase_ == 1   ? 0    // SCAN (nao-selecionado)
                                   : hover_phase_ == 2 ? 2    // ATACAR (selecionado)
                                                       : -1;  // 0 e 3: sem pill
            if (hover_pill >= 0) {
                const glintfx::ElementBox box =
                    ui_->get_element_box(gus::app::screens::kCockpitVerbElementIds[hover_pill]);
                if (box.found) {
                    mx = box.x + box.w * 0.5f;
                    my = box.y + box.h * 0.5f;
                }
            }
            glintfx::UiEvent ge{};
            ge.type = glintfx::UiEvent::Type::MouseMove;
            ge.x = mx;
            ge.y = my;
            ui_->process_event(ge);
        }
        ui_->update();
        ui_->render();  // UI por cima da arena, mesmo contexto GL
    }

    // HOVER-SELFTEST (captura por fase): deixa ~12 frames assentarem e salva 1 PNG por
    // fase, lendo o backbuffer ANTES do swap. Depois avanca a fase; encerra apos a 4a.
    if (hover_selftest_) {
        constexpr int kHoverSettleFrames = 12;
        ++hover_phase_frame_;
        if (hover_phase_frame_ >= kHoverSettleFrames) {
            const char* suffix = hover_phase_ == 0   ? "_a_none.png"
                                 : hover_phase_ == 1 ? "_b_hover_unsel.png"
                                 : hover_phase_ == 2 ? "_c_hover_sel.png"
                                                     : "_d_none_again.png";
            const std::string out = std::string(hover_selftest_prefix_) + suffix;
            // FRAMEGRAB-7-SITIOS: guarda glintfx_on_ && ui_ (MESMO idioma de
            // handle_cockpit_hover_ acima) - capture_frame() e metodo de
            // UiLayer, nao chamavel se ui_ nunca foi criado (GUSWORLD_RMLUI_OFF=1)
            // ou o attach falhou. gl3_read_backbuffer_rgba nao tinha essa
            // exigencia (leitura crua de GL, indiferente a UiLayer).
            if (glintfx_on_ && ui_) {
                const glintfx::UiLayer::CapturedFrame captured = ui_->capture_frame();
                if (captured.ok) {
                    stbi_write_png(out.c_str(), captured.width, captured.height, 4,
                                   captured.pixels.get(), captured.width * 4);
                    std::cout << "BattlePreview: [hover-selftest] fase " << hover_phase_ << " -> "
                              << out << " (" << captured.width << "x" << captured.height << ")\n";
                } else {
                    std::cerr << "BattlePreview: [hover-selftest] capture_frame() falhou\n";
                }
            } else {
                std::cerr << "BattlePreview: [hover-selftest] capture_frame() indisponivel "
                             "(ui_ inativo - GUSWORLD_RMLUI_OFF/attach falhou)\n";
            }
            ++hover_phase_;
            hover_phase_frame_ = 0;
            if (hover_phase_ > 3) {
                running_ = false;  // 4 fases capturadas; encerra
            }
        }
    }

    // COCKPIT-SFX-HOVER-CLIQUE (self-test HEADLESS): apos a geometria assentar (~12
    // frames), varre um MouseMove SINTETICO pelos 6 pills e prova o edge-detect do SOM
    // DE HOVER + o SOM DE CLIQUE.
    if (ui_sfx_selftest_ && !ui_sfx_done_) {
        constexpr int kUiSfxSettleFrames = 12;
        ++ui_sfx_settle_frame_;
        if (ui_sfx_settle_frame_ >= kUiSfxSettleFrames && glintfx_on_ && ui_) {
            ui_sfx_done_ = true;

            auto pill_center = [&](int v, float& cx, float& cy) -> bool {
                const glintfx::ElementBox b =
                    ui_->get_element_box(gus::app::screens::kCockpitVerbElementIds[v]);
                if (!b.found) return false;
                cx = b.x + b.w * 0.5f;
                cy = b.y + b.h * 0.5f;
                return true;
            };

            const unsigned int hover_base = audio_ptr_->sfx_play_count();
            ui_sfx_.reset_pill_hover();
            int swept = 0;
            for (int v = 0; v < gus::app::screens::kBattleVerbCount; ++v) {
                float cx = 0.0f, cy = 0.0f;
                if (!pill_center(v, cx, cy)) continue;
                handle_cockpit_hover_(cx, cy);
                ++swept;
            }
            const unsigned int after_sweep = audio_ptr_->sfx_play_count();

            float lx = 0.0f, ly = 0.0f;
            if (pill_center(gus::app::screens::kBattleVerbCount - 1, lx, ly)) {
                handle_cockpit_hover_(lx, ly);
                handle_cockpit_hover_(lx, ly);
            }
            const unsigned int after_still = audio_ptr_->sfx_play_count();

            handle_cockpit_hover_(static_cast<float>(pw) * 0.80f, static_cast<float>(ph) * 0.50f);
            float z0x = 0.0f, z0y = 0.0f;
            if (pill_center(0, z0x, z0y)) handle_cockpit_hover_(z0x, z0y);
            const unsigned int after_reenter = audio_ptr_->sfx_play_count();

            const unsigned int click_base = audio_ptr_->sfx_play_count();
            if (scene_->waiting_player_input() && !scene_->is_aiming()) {
                // MESMO choke-point do callback de producao (play_outcome), pra o
                // self-test medir o som que o jogador ouviria de verdade - inclusive o de
                // RECUSA, se [Atacar] estivesse sem AP.
                ui_sfx_.play_outcome(battle_cockpit_verb_click(
                    *scene_, gus::app::screens::kCockpitVerbElementIds[static_cast<int>(
                                 BattleVerb::Atacar)]));
            }
            const unsigned int after_click = audio_ptr_->sfx_play_count();

            std::cout << "BattlePreview: [ui-sfx-selftest] hover: " << swept
                      << " pills varridos, " << (after_sweep - hover_base)
                      << " plays (esperado " << swept << " = N hovers -> N plays)\n"
                      << "  repique (parado 2x no mesmo pill): " << (after_still - after_sweep)
                      << " plays extras (esperado 0)\n"
                      << "  sair-e-voltar ao pill 0: " << (after_reenter - after_still)
                      << " play (esperado 1)\n"
                      << "  clique ATACAR: " << (after_click - click_base)
                      << " play (esperado 1); is_aiming="
                      << (scene_->is_aiming() ? "on" : "off") << "\n"
                      << "  device audio="
                      << (audio_ptr_->available() ? "disponivel" : "INDISPONIVEL (mudo)")
                      << " sfx_play_count total=" << after_click << "\n";
            running_ = false;  // prova concluida; encerra
        }
    }

    // ANIM-SELFTEST (capturas): le o backbuffer ANTES do swap nos frames-chave da
    // timeline. Loga o OFFSET do atacante junto de cada shot.
    if (anim_selftest_) {
        const auto capture = [&](const char* suffix) {
            const std::string out = std::string(anim_selftest_prefix_) + suffix;
            // FRAMEGRAB-7-SITIOS: mesma guarda glintfx_on_ && ui_ do site de
            // hover-selftest acima (capture_frame() e metodo de UiLayer).
            if (glintfx_on_ && ui_) {
                const glintfx::UiLayer::CapturedFrame captured = ui_->capture_frame();
                if (captured.ok) {
                    stbi_write_png(out.c_str(), captured.width, captured.height, 4,
                                   captured.pixels.get(), captured.width * 4);
                    const auto aoff = anim_attacker_id_.empty()
                                          ? gus::core::spatial::Vec2{}
                                          : scene_->anim().offset_for(anim_attacker_id_);
                    const auto toff = anim_target_id_.empty()
                                          ? gus::core::spatial::Vec2{}
                                          : scene_->anim().offset_for(anim_target_id_);
                    std::cout << "BattlePreview: [anim-selftest] f" << frame_no_ << " -> " << out
                              << " | atacante_off=(" << aoff.x << "," << aoff.y << ") alvo_off=("
                              << toff.x << "," << toff.y
                              << ") projeteis=" << scene_->anim().projectiles().size()
                              << " floaters=" << scene_->floaters().size() << "\n";
                } else {
                    std::cerr << "BattlePreview: [anim-selftest] capture_frame() falhou\n";
                }
            } else {
                std::cerr << "BattlePreview: [anim-selftest] capture_frame() indisponivel "
                             "(ui_ inativo - GUSWORLD_RMLUI_OFF/attach falhou)\n";
            }
        };
        if (frame_no_ == 40) {
            capture("_a_cast_travel.png");  // bolinha em voo (meio da viagem)
        } else if (frame_no_ == 58) {
            capture("_b_cast_react.png");   // alvo no tranco pos-impacto
        }
        if (anim_f0_ > 0) {
            if (frame_no_ == anim_f0_ + 21) {
                capture("_c_melee_windup.png");   // meio do dash (deslocado)
            } else if (frame_no_ == anim_f0_ + 49) {
                capture("_d_melee_contact.png");  // no alvo + floater + tranco
            } else if (frame_no_ == anim_f0_ + 87) {
                capture("_e_melee_rest.png");      // todos de volta ao repouso
                std::cout << "BattlePreview: [anim-selftest] concluido; encerrando.\n";
                running_ = false;
            }
        }
    }

    // SPRITE-SELFTEST (capturas): backbuffer antes do swap nos frames-chave, logando
    // offset do director + clip/frame do sprite do GUS.
    if (sprite_selftest_) {
        const auto capture = [&](const char* suffix) {
            const std::string out = std::string(sprite_selftest_prefix_) + suffix;
            // FRAMEGRAB-7-SITIOS: mesma guarda glintfx_on_ && ui_ dos 2 sites acima.
            if (glintfx_on_ && ui_) {
                const glintfx::UiLayer::CapturedFrame captured = ui_->capture_frame();
                if (captured.ok) {
                    stbi_write_png(out.c_str(), captured.width, captured.height, 4,
                                   captured.pixels.get(), captured.width * 4);
                } else {
                    std::cerr << "BattlePreview: [sprite-selftest] capture_frame() falhou\n";
                }
            } else {
                std::cerr << "BattlePreview: [sprite-selftest] capture_frame() indisponivel "
                             "(ui_ inativo - GUSWORLD_RMLUI_OFF/attach falhou)\n";
            }
            const auto off = scene_->anim().offset_for("gus");
            const auto sf = scene_->actor_sprite_frame("gus");
            std::cout << "BattlePreview: [sprite-selftest] f" << frame_no_ << " -> " << out
                      << " | gus_off=(" << off.x << "," << off.y
                      << ") kind=" << static_cast<int>(scene_->anim().kind_for("gus"))
                      << " clip="
                      << (sf.has_value()
                              ? std::string(gus::app::screens::clip_dir_name(sf->first))
                              : std::string("<sem sprite set>"))
                      << " frame=" << (sf.has_value() ? sf->second : -1) << "\n";
        };
        if (frame_no_ == 8) {
            capture("_a_idle_rest.png");     // battle_idle no repouso
        }
        if (sprite_f0_ > 0) {
            const int approach_f = static_cast<int>(
                gus::app::screens::kPlayerMeleeApproachSeconds * 60.0f + 0.5f);
            const int return_f =
                static_cast<int>(gus::app::screens::kPlayerMeleeReturnSeconds * 60.0f + 0.5f);
            const int rel = frame_no_ - sprite_f0_;
            // FLIPBOOK do APPROACH inteiro: captura DENSA a cada 4 frames de jogo.
            if (rel >= 0 && rel <= approach_f && (rel % 4) == 0) {
                const int fi = rel / 4;
                const std::string suf =
                    std::string("_flip_") + (fi < 10 ? "0" : "") + std::to_string(fi) + ".png";
                capture(suf.c_str());
            }
            // Marcos MACRO (nomes semanticos; rel escolhidos %4==3, nao colidem com o flip).
            if (rel == approach_f - 3) {
                capture("_c_attack_swing.png");  // murro de perfil cravado (<= f5)
            } else if (rel == approach_f + return_f / 2) {
                capture("_e_run_back.png");      // run_west no meio da volta
            } else if (rel == approach_f + return_f + 3) {
                capture("_d_idle_back.png");      // de volta ao idle no repouso
                std::cout << "BattlePreview: [sprite-selftest] concluido; encerrando.\n";
                running_ = false;
            }
        }
    }

    // SMOKE VISUAL: captura 1 PNG no frame alvo (ANTES do swap, lendo o backbuffer) e
    // encerra. Em modo interativo, o swap apresenta na janela.
    if (capture_path_ != nullptr && frame_no_ + 1 >= capture_at_frame_) {
        // FRAMEGRAB-7-SITIOS: mesma guarda glintfx_on_ && ui_ dos 3 sites acima.
        if (glintfx_on_ && ui_) {
            const glintfx::UiLayer::CapturedFrame captured = ui_->capture_frame();
            if (captured.ok) {
                stbi_write_png(capture_path_, captured.width, captured.height, 4,
                               captured.pixels.get(), captured.width * 4);
                std::cout << "BattlePreview: [capture] PNG salvo em " << capture_path_ << " ("
                          << captured.width << "x" << captured.height << ")\n";
            } else {
                std::cerr << "BattlePreview: [capture] capture_frame() falhou\n";
            }
        } else {
            std::cerr << "BattlePreview: [capture] capture_frame() indisponivel (ui_ inativo "
                         "- GUSWORLD_RMLUI_OFF/attach falhou)\n";
        }
        running_ = false;
    }

    SDL_GL_SwapWindow(window_);  // swap unico (arena + HUD compostos)
    ++frame_no_;

    // DIAGNOSTICO: sai apos N frames (exercita o loop completo sob ASan/UBSan).
    if (max_frames_ > 0 && frame_no_ >= max_frames_) {
        std::cout << "BattlePreview: [frames] limite de " << max_frames_
                  << " frames atingido, encerrando.\n";
        running_ = false;
    }

    if (!running_) {
        advance_phase_(/*phase_done=*/true);
    }
}

void BattleScreen::tick(float dt) {
    switch (phase_) {
        case BattlePhase::FadeIn:
            tick_fade_in_(dt);
            return;
        case BattlePhase::Main:
            tick_main_(dt);
            return;
        case BattlePhase::FadeOut:
            tick_fade_out_(dt);
            return;
        case BattlePhase::Done:
            return;  // defensivo: run_screen_state nao chama apos finished()==true.
    }
}

void BattleScreen::exit() {
    // M7-COSTURA (ADR-012 Onda 1): MESMO choke-point de sempre - CACHEIA o
    // CombatOutcome final (pra Maestro decidir vitoria/derrota/fuga) ANTES de destruir
    // scene_ logo abaixo. O WRAPPER (run_battle_preview_embedded_gl_current) le
    // outcome()/window_closed() DEPOIS que run_screen_state() retorna (exit() ja rodou
    // aqui) - preserva a MESMA ordem "out_outcome primeiro, out_quit_requested depois"
    // das linhas ~1681-1689 do while(true) antigo, so que num MOMENTO diferente do C++
    // (o cache acontece agora; a ESCRITA nos out-params do chamador acontece depois).
    // Se a janela foi fechada no meio do combate, a FSM nunca chegou a CombatEnd e
    // outcome() ainda e Ongoing - a Maestro trata isso como "abortou", nao derrota.
    if (scene_) {
        cached_outcome_ = scene_->machine().outcome();
    }

    // MUSICA: fade-out ao SAIR da batalha (M6 F4, ADR-011) - SO no modo LOCAL
    // (standalone); no modo EXTERNO (Maestro) a musica e 100% dela.
    if (external_audio_ == nullptr && audio_ptr_ != nullptr) {
        const bool music_was_playing_before_exit = audio_ptr_->music_is_playing();
        constexpr float kMusicFadeOutSeconds = 1.5f;
        audio_ptr_->stop_music(kMusicFadeOutSeconds);
        std::cout << "BattlePreview: [audio] musica: play_music count="
                  << audio_ptr_->music_play_count()
                  << " estava_tocando_ao_sair=" << (music_was_playing_before_exit ? "sim" : "nao")
                  << " fade-out de " << kMusicFadeOutSeconds << "s disparado.\n";
    }

    // DIAGNOSTICO (M6 F3, ADR-011): quantos SFX de hit TOCARAM de fato nesta sessao.
    if (audio_ptr_ != nullptr) {
        std::cout << "BattlePreview: [audio] play_sfx(hit) disparou "
                  << audio_ptr_->sfx_play_count() << "x nesta sessao.\n";
    }

    // EXCLUSIVIDADE DO UILAYER (ver gus/app/screen_state.hpp): destroi ui_ ANTES do
    // resto - depois de exit(), a PROXIMA tela pode chamar seu proprio enter() com
    // seguranca.
    ui_.reset();
    scene_.reset();
    renderer_.reset();
    local_audio_engine_.reset();
}

}  // namespace

void run_battle_preview_embedded_gl_current(
    SDL_Window* window, gus::domain::combat::CombatOutcome* out_outcome,
    bool* out_quit_requested, gus::platform::audio::AudioEngine* external_audio,
    float fade_in_seconds, float fade_out_seconds, const gus::app::EventSyncHook& sync_hook) {
    BattleScreen screen(window, external_audio, fade_in_seconds, fade_out_seconds);
    gus::app::run_screen_state(screen, sync_hook);

    // M7-COSTURA (ADR-012 Onda 1): MESMO choke-point de saida (INTOCADO, ver o
    // comentario grande no .hpp) - grava o CombatOutcome final PRIMEIRO, depois se o
    // motivo foi ESPECIFICAMENTE o jogador fechando a janela (FIX BUG-3) - a Maestro
    // distingue "abortou" de "derrota" por esses 2 out-params, nesta ORDEM.
    if (out_outcome != nullptr) {
        *out_outcome = screen.outcome();
    }
    if (out_quit_requested != nullptr) {
        *out_quit_requested = screen.window_closed();
    }
}

// M7-COSTURA (ADR-012 Onda 1): "trocar escondido atras do preto" - viabilidade validada
// EMPIRICAMENTE (probes standalone fora da arvore, nao commitados) num build SDL3 real
// (X11/Mesa, GL 4.6 core): destruir o SDL_Renderer da cidade, criar um contexto GL na
// MESMA janela (mesmo sem SDL_WINDOW_OPENGL setado na criacao original), rodar a
// batalha, destruir o contexto, recriar o SDL_Renderer - tudo na MESMA SDL_Window, em
// ciclo repetido (cidade->batalha->cidade->batalha), sem crash/hang. Os atributos GL
// (profile/versao/stencil) sao setados de novo a CADA entrada na batalha (nao precisam
// ter sido setados na criacao da janela) - por isso run_battle_preview_embedded() os
// seta aqui, e nao so no wrapper standalone. Custo real aceito (nao e "pesadelo
// tecnico", e um tradeoff conhecido): TextureId da SDL_Renderer antiga NAO sobrevivem a
// destruicao do renderer - a Maestro RECARREGA os sprites da cidade ao reconstruir o
// SDL_Renderer no retorno (ver Maestro::reacquire_city_renderer em maestro.cpp).
//
// FLASH-CTX (A2): casca fina que cria/destroi o contexto GL e delega o corpo pra
// run_battle_preview_embedded_gl_current (o nucleo acima) - MESMO padrao de
// run_system_menu_loop_owning_gl/run_system_menu_loop_gl_current.
int run_battle_preview_embedded(SDL_Window* window,
                                 gus::domain::combat::CombatOutcome* out_outcome,
                                 bool* out_quit_requested,
                                 gus::platform::audio::AudioEngine* external_audio,
                                 float fade_in_seconds, float fade_out_seconds,
                                 const gus::app::EventSyncHook& sync_hook) {
    // ADR-009 adendo GL3: a janela usa contexto OpenGL 3.3 core (nao SDL_Renderer), pois o
    // HUD RmlUi-GL3 precisa de shaders (gradiente/box-shadow/blur). A arena (Render2dGl3) e
    // o HUD compartilham o MESMO contexto GL; swap unico (SDL_GL_SwapWindow).
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);  // o GL3 do RmlUi usa stencil (clip mask)

    SDL_GLContext gl = SDL_GL_CreateContext(window);
    if (gl == nullptr) {
        std::cerr << "BattlePreview: SDL_GL_CreateContext falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_GL_MakeCurrent(window, gl);
    SDL_GL_SetSwapInterval(1);  // VSync

    // Carrega os ponteiros de funcao GL (glad, via o backend GL3 do RmlUi). PRECISA vir
    // depois do contexto corrente e ANTES de qualquer chamada GL (Render2dGl3/RmlUiHud).
    // Chamado a CADA entrada na batalha (o contexto e recriado toda vez - ver nota acima).
    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        std::cerr << "BattlePreview: falha ao carregar funcoes OpenGL (glad)\n";
        SDL_GL_DestroyContext(gl);
        return 1;
    }

    run_battle_preview_embedded_gl_current(window, out_outcome, out_quit_requested,
                                            external_audio, fade_in_seconds,
                                            fade_out_seconds, sync_hook);

    // M7-COSTURA: SO o contexto GL e desta funcao (criado no topo). A janela e o SDL_Init/
    // Quit pertencem a quem chamou (a Maestro, dona da janela compartilhada; ou o wrapper
    // run_battle_preview() abaixo, no uso standalone --battle).
    SDL_GL_DestroyContext(gl);
    return 0;
}

// M7-COSTURA (ADR-012 Onda 1): wrapper fino do --battle STANDALONE. Cria SUA PROPRIA
// janela (escalada 16:9 pra caber na area util do desktop) + SDL_Init/Quit, delega o
// loop inteiro pra run_battle_preview_embedded (outcome descartado - nada consome fora
// deste fluxo isolado) e destroi a janela ao sair. Preserva o comportamento de sempre.
int run_battle_preview() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "BattlePreview: SDL_Init falhou: " << SDL_GetError() << "\n";
        return 1;
    }

    // FIX W1 item 2 (lider: "maximizada, a base da cena desliza pra tras da barra de
    // tarefas"): a janela INICIAL deve caber na AREA UTIL do desktop (que desconta a barra
    // de tarefas/paineis via struts), senao a base (log/rodape do cockpit) nasce escondida.
    // SDL_GetDisplayUsableBounds ja desconta os paineis. Preservamos a proporcao 16:9 (a
    // arena estica 960x540 -> janela; 16:9 = sem distorcao) escolhendo a MAIOR janela 16:9
    // que cabe na area util (com margem p/ a decoracao da janela), limitada ao alvo
    // 1920x1080. Sob Xvfb/headless (sem barra) os usable bounds = display inteiro -> escala
    // 1.0 -> janela 1920x1080 como antes (self-tests de mouse/hover intactos: as coordenadas
    // derivam de pw0/ph0 REAIS da janela, nao de constantes). NAO ha offset de letterbox
    // aqui -> os hit-tests de mouse (A2/picker) seguem validos sem desconto.
    int win_w = kWindowW, win_h = kWindowH;
    {
        SDL_Rect usable{};
        const SDL_DisplayID disp = SDL_GetPrimaryDisplay();
        if (disp != 0 && SDL_GetDisplayUsableBounds(disp, &usable) && usable.w > 0 &&
            usable.h > 0) {
            constexpr float kMargin = 0.95f;  // folga p/ bordas/titlebar da janela
            const float avail_w = static_cast<float>(usable.w) * kMargin;
            const float avail_h = static_cast<float>(usable.h) * kMargin;
            float scale = 1.0f;
            scale = std::min(scale, avail_w / static_cast<float>(kWindowW));
            scale = std::min(scale, avail_h / static_cast<float>(kWindowH));
            if (scale < 1.0f) {
                win_w = static_cast<int>(static_cast<float>(kWindowW) * scale);
                win_h = static_cast<int>(static_cast<float>(kWindowH) * scale);
            }
            std::cout << "BattlePreview: [win] area util=" << usable.w << "x" << usable.h
                      << " -> janela inicial=" << win_w << "x" << win_h << " (16:9)\n";
        }
    }

    SDL_Window* window =
        SDL_CreateWindow("GusWorld BattlePreview (M5, GL3)", win_w, win_h,
                         SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        std::cerr << "BattlePreview: SDL_CreateWindow falhou: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    const int rc = run_battle_preview_embedded(window, /*out_outcome=*/nullptr);

    SDL_DestroyWindow(window);
    SDL_Quit();
    return rc;
}

}  // namespace gus::app::screens
