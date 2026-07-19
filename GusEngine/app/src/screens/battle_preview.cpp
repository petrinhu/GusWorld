// gus/app/src/screens/battle_preview.cpp
//
// Ver header. Casca SDL do viewer da BattleScene (esqueleto M5). Reusa Render2dSdl
// (atras de IRenderer) e o mesmo padrao de loop do anim_preview (poll -> render). A
// cena LE o motor de combate; aqui so abrimos janela, carregamos os retratos 48px e
// desenhamos o esqueleto a cada frame. Esc/fechar encerra.

#include "gus/app/screens/battle_preview.hpp"

#include <cstdlib>  // std::getenv
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <SDL3/SDL.h>

#include "gus/app/screens/battle_cockpit_rml.hpp"  // AC-E11 A2: montagem RML do cockpit
#include "gus/app/screens/battle_cockpit_verb_ids.hpp"  // GLINTFX-CLICK: id->indice de verbo
#include "gus/app/screens/battle_hud_model.hpp"  // status_icon_file/index
#include "gus/app/screens/battle_layout.hpp"     // arena_layout (selftest de mouse A2)
#include "gus/app/boot_pixel_overlay.hpp"  // sequencia de frames da transicao (M7-COSTURA Inc 2c)
#include "gus/app/screens/battle_scene.hpp"
// REVERT (BATTLE-ESC-PAUSE-ACTOR-LIST, 2026-07-05): o HOST REAL nao chama mais
// run_system_menu_loop_gl_current() (crash real ao vivo, 2o UiLayer RmlUi aninhado - ver
// battle_key_down abaixo). Include MANTIDO de proposito (nao-orfao, sem warning/erro de
// build - so um header, nao gera unused-var): o modulo system_menu_loop.* fica intacto e
// reutilizavel se o nesting da lib for resolvido no futuro.
#include "gus/app/screens/system_menu_loop.hpp"
#include "gus/app/screens/ui_hover.hpp"  // COCKPIT-SFX-HOVER-CLIQUE: edge-detect POCO do som de hover
#include "gus/core/asset_paths.hpp"             // caminhos de asset centralizados
#include "gus/domain/combat/combat_enums.hpp"  // StatusId
#include "gus/platform/audio/audio_engine.hpp"     // AudioEngine (M6 F3, ADR-011)
#include "gus/platform/fs/settings_file_store.hpp"  // MENU-PAUSA-CONFIG-SOM: resolve_settings_dir
#include "gus/platform/render2d/render2d_gl3.hpp"  // ADR-009 GL3: backend OpenGL da arena
#include "gus/platform/rmlui/gl3_loader.hpp"  // glad load + read_backbuffer (captura)

// ADR-010 F3: glintfx::UiLayer (embed mode) e o UNICO motor de UI/HUD - o backend RmlUi
// vendorizado (RmlUiHud) foi aposentado. Compilado e linkado SEMPRE (app/ linka
// glintfx::glintfx incondicional). A arena (Render2dGl3) e o gl3_loader continuam.
#include <optional>
#include <glintfx/ui_event.hpp>
#include <glintfx/ui_layer.hpp>

// stb_image_write: captura de frame (PNG) para o SMOKE VISUAL do ADR-009 (comparar o
// jogo com o mock). IMPLEMENTACAO definida UMA vez aqui (camada app/, fora do hot path).
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// AC-E11 A2 (ADR-019): GUSWORLD_FONTS_DIR (a macro do @font-face do cockpit BAKED/LIVE)
// e <filesystem>/<fstream> (tempfile do RML) MORARAM pra battle_cockpit_rml.cpp - so quem
// escreve o .rml precisa deles agora.

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

// FLASH-CTX (A2, extracao behavior-preserving): NUCLEO que ASSUME um contexto
// GL JA CORRENTE + glad JA CARREGADO - mesmo corpo que antes vivia direto em
// run_battle_preview_embedded (ver o header pro contrato, e o comentario
// M7-COSTURA acima da casca owning mais abaixo pra tecnica de "trocar
// escondido atras do preto"). `quit_requested` e `fxtest` passam a ser locais
// deste nucleo (antes viviam no escopo da funcao dona do contexto, mas so
// eram usados dentro deste corpo - mudanca de escopo pura, zero mudanca de
// valor/logica). Extraida verbatim pra permitir reuso futuro sem recriar o
// contexto (Opcao C do plano de contexto GL unico).
void run_battle_preview_embedded_gl_current(
    SDL_Window* window, gus::domain::combat::CombatOutcome* out_outcome,
    bool* out_quit_requested, gus::platform::audio::AudioEngine* external_audio,
    float fade_in_seconds, float fade_out_seconds) {
    // FIX BUG-3 (playtest ao vivo do lider: fechar a janela durante a batalha reabria a
    // cidade em LOOP INFINITO): comeca false; so vira true no MESMO handler de
    // SDL_EVENT_QUIT que ja existia (ver o `while (running)` abaixo) - um sinal DISTINTO
    // de qualquer CombatOutcome, gravado no choke-point de saida junto do outcome.
    bool quit_requested = false;

    // DIAGNOSTICO DE EFEITOS (ADR-009 #1): GUSWORLD_RMLUI_FXTEST=1 carrega um doc com
    // efeitos MAXIMAMENTE OBVIOS (gradiente vermelho->azul, box-shadow magenta forte) e
    // renderiza SO o HUD (sem arena, fundo preto), pra provar SE os shaders de
    // gradiente/box-shadow chegam a tela neste setup GL. Se aparecerem aqui, o problema
    // sao os VALORES (sutis) ou a composicao com a arena; se NAO, e o pipeline GL3.
    const bool fxtest = [] {
        const char* e = std::getenv("GUSWORLD_RMLUI_FXTEST");
        return e != nullptr && e[0] == '1';
    }();

    {
        gus::platform::render2d::Render2dGl3 renderer(/*gl_active=*/true);

        // M7-COSTURA Inc 2c: sequencia de frames do boot pixelizado (substitui o
        // glitch procedural - o lider VETOU o visual ao vivo, "pareceu bug").
        // Carregada UMA VEZ por entrada na batalha (TextureId locais a ESTE
        // Render2dGl3, recriado a cada entrada - ver o comentario da funcao) e
        // reusada nos 2 loops de fade abaixo (entrada kIn + saida kOut).
        gus::app::BootPixelOverlay boot_overlay;
        boot_overlay.load(renderer, resolve_asset_dir(gus::core::assets::kVfxBootPixelDir));

        // ====================================================================
        // ADR-010 F3: glintfx::UiLayer (embed mode) e o UNICO motor de UI/HUD, compondo
        // POR CIMA da arena com efeitos nativos (gradiente/box-shadow/glow). A arena desenha
        // primeiro (backbuffer), o glintfx compoe por cima (layer -> backbuffer), o swap e
        // unico (SDL_GL_SwapWindow). Opt-out (debug so-arena): GUSWORLD_RMLUI_OFF=1.
        // ====================================================================
        const bool rmlui_opt_out = [] {
            const char* e = std::getenv("GUSWORLD_RMLUI_OFF");
            return e != nullptr && e[0] == '1';
        }();
        int pw0 = kWindowW, ph0 = kWindowH;
        SDL_GetWindowSizeInPixels(window, &pw0, &ph0);
        {
            int ww = 0, wh = 0;
            SDL_GetWindowSize(window, &ww, &wh);
            std::cout << "BattlePreview: [scale] janela logica=" << ww << "x" << wh
                      << " pixels=" << pw0 << "x" << ph0
                      << " dp_ratio=" << (static_cast<float>(pw0) / 960.0f) << "\n";
        }

        // ====================================================================
        // ADR-010 F3: glintfx::UiLayer (embed mode) - UNICO motor de UI/HUD.
        // Anexa ao contexto GL JA corrente (SDL_GL_MakeCurrent acima); compose-only no
        // loop (sem clear, sem swap), no slot do hud.compose(). load_gl=true: o glintfx usa
        // gl3w (tabela de ponteiros PROPRIA, independente do glad que o GusEngine carregou
        // em gl3_load_functions); por isso PRECISA carregar a sua (false deixaria os
        // ponteiros gl3w NULL -> crash). Ver R-glad-owner no relatorio.
        // ====================================================================
        bool glintfx_on = false;
        std::optional<glintfx::UiLayer> ui;
        bool glintfx_live = false;  // F2b: cockpit dirigido por data-model (valores vivos)
        // F2b DEBUG: GUSWORLD_GLINTFX_DP=<float> FORCA o dp_ratio (em vez de pw/960). Util
        // pra INSPECIONAR a coluna inteira do cockpit num shot: o canvas e autorado ~625dp
        // de alto e o backbuffer (~1006px) so mostra ~503dp a dp_ratio=2 (a base do menu +
        // o log caem abaixo da dobra). dp_ratio menor (ex. 1.5 => mostra ~670dp) revela o
        // log/now-line. 0/ausente => comportamento normal (pw/960). Nao altera o motor.
        const float glintfx_dp_override = [] {
            const char* e = std::getenv("GUSWORLD_GLINTFX_DP");
            return (e != nullptr && e[0] != '\0') ? static_cast<float>(std::atof(e)) : 0.0f;
        }();
        if (!rmlui_opt_out) {
            // ADR-010 F2b (DEFAULT): cockpit REAL pelo UiLayer dirigido por DATA-MODEL
            // (valores VIVOS por frame: HP/nome/role/selecao-de-verbo/log). RCSS autorado em
            // 'dp' num canvas logico 960x540; o dp_ratio (= pixels reais / 960) escala pro
            // backbuffer. Modos de debug por env:
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
            const float dp_ratio = glintfx_dp_override > 0.0f
                                       ? glintfx_dp_override
                                       : static_cast<float>(pw0) / 960.0f;
            ui.emplace(glintfx::UiLayer::Config{/*logical_width=*/960,
                                                /*logical_height=*/540,
                                                /*load_gl=*/true,
                                                /*dp_ratio=*/dp_ratio});
            if (ui->ok()) {
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
                    // os set_* vivos vem no loop (a cena so existe depois deste bloco).
                    glintfx_live = true;
                    ui->create_data_model("hud");
                    ui->bind_bool("intro", true);
                    ui->bind_string("nome", "Gus");
                    ui->bind_string("role", "VETOR DO GAMBITO");
                    ui->bind_number("hp", 55);
                    ui->bind_number("hp_max", 55);
                    ui->bind_number("sel", 2);  // Atacar (default do BattleMenu)
                    ui->bind_string("verb", "ATACAR");
                    ui->bind_string("alvo", "-");
                    // RETRATO-VIVO: caminho flat do retrato da moldura. Inicial = Gus no-bg
                    // (1o frame/intro); o alimentador troca por frame conforme o ator ativo.
                    ui->bind_string("retrato_src", "retrato_gus_combate_nobg.png");
                    ui->bind_list("log");
                    rml_path = write_live_cockpit_rml();
                    base_url = glintfx_cockpit_stage_dir();  // dir com doc+fontes+sprites
                }
                // base-url ANTES do load (pra o doc tambem resolver via ele). Caminhos
                // relativos -> base_url/path; absolutos passam direto.
                if (!base_url.empty()) {
                    ui->set_asset_base_url(base_url.c_str());
                }
                ui->load(rml_path.c_str());
                ui->set_viewport(pw0, ph0);    // pixels reais do backbuffer
                ui->set_dp_ratio(dp_ratio);    // logico 960x540 -> pixels reais
                glintfx_on = true;
                std::cout << "BattlePreview: [glintfx] UiLayer ATIVO (embed, load_gl=true) "
                          << (glintfx_smoke   ? "[SMOKE]"
                              : glintfx_baked ? (glintfx_intro
                                                     ? "[cockpit BAKED: intro/brasao]"
                                                     : "[cockpit BAKED: combate]")
                                              : "[cockpit LIVE: data-model]")
                          << " viewport=" << pw0 << "x" << ph0
                          << " dp_ratio=" << dp_ratio << " RML=" << rml_path << "\n";
            } else {
                std::cerr << "BattlePreview: [glintfx] UiLayer::ok()=false (attach falhou) "
                             "- caindo SEM UI neste run\n";
                ui.reset();
            }
        }

        // AUDIO (M6 F3/F4, ADR-011; ownership revisada M7-COSTURA Inc 2, ADR-012 decisao
        // 5): dois modos.
        //   external_audio == nullptr (--battle STANDALONE + todo selftest/captura, ver
        //     header): o AudioEngine e DONO da CASCA (comportamento de SEMPRE, INTOCADO)
        //     - local_audio_engine construido AQUI, device_active=true tenta o hardware
        //     real, degradacao graciosa se indisponivel. Toca a musica (loop+fade-in) e
        //     para no choke-point de saida (ver mais abaixo).
        //   external_audio != nullptr (a Maestro, M7-COSTURA Inc 2): usa o engine DELA
        //     (ponteiro nao-dono) - NAO cria device novo (a divida do ADR-011 "AudioEngine
        //     e dono da battle_preview" fica paga: o device nao e mais reaberto a cada
        //     entrada na batalha) e NAO toca/para musica aqui - a Maestro cronometra o
        //     crossfade com o fade preto por fora (gus/app/maestro_logic.hpp::
        //     crossfade_music), ANTES/DEPOIS desta funcao rodar.
        std::optional<gus::platform::audio::AudioEngine> local_audio_engine;
        gus::platform::audio::AudioEngine* audio_ptr = external_audio;
        if (audio_ptr == nullptr) {
            local_audio_engine.emplace(/*device_active=*/true);
            audio_ptr = &(*local_audio_engine);
        }

        // SFX do hit (M6 F3): load_sfx UMA vez por entrada na batalha (NUNCA no frame do
        // contato - decodificar e caro), em AMBOS os modos - preserva a variante A/B
        // GUSWORLD_HIT_SFX=alt mesmo com o engine EXTERNO.
        const std::string hit_sfx_path = resolve_hit_sfx_path();
        const gus::platform::audio::SoundId hit_sfx_id =
            audio_ptr->load_sfx(hit_sfx_path.c_str());

        // COCKPIT-SFX-HOVER-CLIQUE: blips de UI do cockpit (hover + clique nos pills de
        // verbo) - REUSA os MESMOS 2 arquivos do menu de sistema (kMenuHoverSfxFile/
        // kMenuClickSfxFile: identidade sonora unica de UI, pedido do lider). load_sfx UMA
        // VEZ por entrada na batalha (NUNCA no frame - decodificar e caro), MESMO padrao do
        // hit acima e do menu (system_menu_loop.cpp). Em AMBOS os modos de audio (local/
        // externo). kInvalidSound degrada com seguranca (play_sfx no-op).
        const std::string ui_hover_sfx_path =
            resolve_ui_sfx_path(gus::core::assets::kMenuHoverSfxFile);
        const std::string ui_click_sfx_path =
            resolve_ui_sfx_path(gus::core::assets::kMenuClickSfxFile);
        const gus::platform::audio::SoundId ui_hover_sfx_id =
            audio_ptr->load_sfx(ui_hover_sfx_path.c_str());
        const gus::platform::audio::SoundId ui_click_sfx_id =
            audio_ptr->load_sfx(ui_click_sfx_path.c_str());
        std::cout << "BattlePreview: [audio] SFX de UI (cockpit) hover "
                  << (ui_hover_sfx_id != gus::platform::audio::kInvalidSound ? "carregado"
                                                                             : "AUSENTE")
                  << " / clique "
                  << (ui_click_sfx_id != gus::platform::audio::kInvalidSound ? "carregado"
                                                                             : "AUSENTE")
                  << " (reuso dos blips do menu de sistema)\n";

        std::cout << "BattlePreview: [audio] device "
                  << (audio_ptr->available() ? "disponivel" : "INDISPONIVEL (mudo)")
                  << " - SFX de hit "
                  << (hit_sfx_id != gus::platform::audio::kInvalidSound
                          ? "carregado"
                          : "AUSENTE (silencioso)")
                  << " de " << hit_sfx_path
                  << (external_audio != nullptr ? " [engine EXTERNO, Maestro]"
                                                 : " [engine LOCAL, standalone]")
                  << "\n";

        // MUSICA (M6 F4, ADR-011): SO no modo LOCAL (standalone) - toca em LOOP com
        // FADE-IN ao entrar, comportamento INTOCADO. No modo EXTERNO (Maestro), a
        // musica e responsabilidade INTEIRA da Maestro (crossfade cronometrado com o
        // fade preto) - tocar/parar aqui de novo brigaria com esse crossfade.
        //
        // NOTA HONESTA (kCityThemeFile, ver comentario em asset_paths.hpp): e um tema de
        // CIDADE tocando na BATALHA porque e a UNICA faixa do kit CC0 provisorio (F2) -
        // serve pra PROVAR loop+fade tecnicamente, NAO pra vender o feel de combate. NAO
        // mudar o timbre/curadoria aqui (fora de escopo desta fase, ADR-011).
        if (external_audio == nullptr) {
            const std::string music_path = resolve_music_path();
            const gus::platform::audio::SoundId music_id =
                audio_ptr->load_music(music_path.c_str());
            constexpr float kMusicFadeInSeconds = 2.0f;
            audio_ptr->play_music(music_id, /*loop=*/true, kMusicFadeInSeconds);
            std::cout << "BattlePreview: [audio] musica "
                      << (music_id != gus::platform::audio::kInvalidSound
                              ? "carregada (loop, fade-in " +
                                    std::to_string(kMusicFadeInSeconds) + "s)"
                              : "AUSENTE (silenciosa)")
                      << " de " << music_path << "\n";
        }

        // A cena monta o encontro de demo e ja le a fila do motor.
        BattleScene scene;
        // Ponteiro NAO-DONO (mesmo padrao de set_translator/set_portraits): a cena so
        // dispara play_sfx no evento de CONTATO do golpe (F3) - nunca decodifica, nunca
        // possui o engine. hit_sfx_id pode ser kInvalidSound (asset ausente/device
        // indisponivel) - play_sfx() ja degrada com seguranca nesse caso.
        scene.set_audio(audio_ptr, hit_sfx_id);
        // (A) Com o HUD externo (glintfx::UiLayer) ATIVO, a cena NAO desenha o cockpit/log a
        // mao - so arena/banner/floaters/fila. Evita cockpits sobrepostos.
        scene.set_hud_external(glintfx_on);

        // GLINTFX-CLICK (v0.2.5): registra o callback de clique da UI - o glintfx faz o
        // hit-test ele mesmo (o MESMO que ja move o :hover) e devolve o `id` do elemento
        // clicado; battle_cockpit_verb_click traduz esse id pra acao do motor (ver defs
        // acima). Sem restricao de ordem vs load() (a doc do glintfx garante isso) - registra
        // aqui, DEPOIS da BattleScene existir, porque o callback CAPTURA `scene` por
        // referencia (nao dava pra registrar antes, no bloco de setup do UiLayer, onde a
        // cena ainda nao existia). `scene` sobrevive ate o fim deste escopo, junto com `ui`.
        if (glintfx_on && ui) {
            // COCKPIT-SFX-HOVER-CLIQUE: o glintfx JA fez o hit-test nativo (o mesmo do
            // :hover) e devolveu o `id`; aqui so somamos o SFX de CLIQUE quando a pill de
            // fato ACIONOU um verbo (battle_cockpit_verb_click devolve true). NAO duplica
            // hit-test - o unico choke-point do clique real E deste callback. audio_ptr/
            // ui_click_sfx_id capturados por valor (ponteiro nao-dono + handle); vivem ate
            // o fim deste escopo, junto de `scene`/`ui`.
            ui->set_click_callback(
                [&scene, audio_ptr, ui_click_sfx_id](const char* element_id) {
                    if (battle_cockpit_verb_click(scene, element_id)) {
                        audio_ptr->play_sfx(ui_click_sfx_id);
                    }
                });
        }

        // Carrega os retratos 48px da fila CTB (handles resolvidos pelo renderer) e os
        // entrega a cena. Cada id de ator -> seu retrato; ausencia degrada pro retangulo.
        const std::string dir = resolve_retratos_dir();
        BattlePortraitSet portraits;
        for (const auto* actor : scene.machine().queue().order()) {
            if (actor == nullptr) {
                continue;
            }
            const std::string path = join(dir, retrato_file_for(actor->id()));
            const gus::platform::render2d::TextureId tex =
                renderer.load_texture(path.c_str());
            portraits.by_id.emplace_back(actor->id(), tex);
        }
        scene.set_portraits(std::move(portraits));

        // SPRITE SET do GUS (W3): frames de anims/ resolvidos em TextureId e entregues
        // a cena (mesmo padrao dos retratos). Ausencia (headless/sem assets) degrada
        // pro retrato placeholder - a cena decide por ator.
        if (auto gus_set = load_gus_sprite_set(renderer)) {
            int nframes = 0;
            for (const auto& c : gus_set->clips) {
                nframes += static_cast<int>(c.frames.size());
            }
            scene.set_actor_sprites("gus", std::move(*gus_set));
            std::cout << "BattlePreview: sprite set do Gus carregado (" << nframes
                      << " frames de " << gus::core::assets::kGusBattleAnimsDir
                      << ")\n";
        } else {
            std::cout << "BattlePreview: sprite set do Gus AUSENTE (retrato "
                         "placeholder)\n";
        }

        // Carrega os icones de status (14px), indexados por StatusId (status_icon_index),
        // e os entrega a cena. Ausencia degrada pro quadradinho placeholder.
        const std::string sdir = resolve_status_icons_dir();
        BattleStatusIconSet status_icons;
        for (int i = 0; i < static_cast<int>(status_icons.by_index.size()); ++i) {
            const auto id = static_cast<gus::domain::combat::StatusId>(i);
            const std::string spath = join(sdir, std::string(status_icon_file(id)));
            status_icons.by_index[static_cast<std::size_t>(i)] =
                renderer.load_texture(spath.c_str());
        }
        scene.set_status_icons(status_icons);

        // Carrega os icones de INTENT (telegraph, incremento 5) e os entrega a cena.
        // Ausencia => marca placeholder ambar sobre o inimigo.
        const std::string sdir_intent = resolve_intent_icons_dir();
        BattleIntentIconSet intent_icons;
        intent_icons.atacar =
            renderer.load_texture(join(sdir_intent, "intent_atacar.png").c_str());
        intent_icons.defender =
            renderer.load_texture(join(sdir_intent, "intent_defender.png").c_str());
        intent_icons.aplicar_status = renderer.load_texture(
            join(sdir_intent, "intent_aplicar_status.png").c_str());
        intent_icons.ruido = renderer.load_texture(
            join(sdir_intent, "intent_ruido_patchzero.png").c_str());
        scene.set_intent_icons(intent_icons);

        // Carrega o catalogo de traducao (pt_br.md) e o entrega a cena, pra os verbos do
        // menu aparecerem com NOME legivel (incremento 3.5). Ausencia => fallback (caixa
        // colorida sem nome, mas nao crasha). O Translator vive aqui (casca), a cena so
        // aponta pra ele (nao-dono): mantemos o objeto vivo ate o fim do loop.
        gus::app::i18n::Translator translator;
        const std::string tr_path = gus::app::i18n::resolve_translations_path();
        const bool tr_ok = translator.load_from_file(tr_path);
        scene.set_translator(&translator);

        std::cout << "BattlePreview: traducao "
                  << (tr_ok ? "carregada" : "AUSENTE (fallback)") << " de " << tr_path
                  << "\n  party=" << scene.party_count()
                  << " inimigos=" << scene.enemy_count()
                  << " fila=" << scene.queue_len() << " retratos em " << dir
                  << "\n  ABERTURA: Enter = Encarar (comeca a luta) | Q = Resolver sem "
                     "encarar (placeholder)"
                  << "\n  COMBATE: Cima/Baixo navega o menu | Enter/Espaco: na sua vez "
                     "confirma o verbo, senao ACELERA o ritmo | Esc: sai\n";

        // SMOKE VISUAL (ADR-009): se GUSWORLD_RMLUI_CAPTURE=<arquivo.png>, renderiza
        // alguns frames (deixa o pacing assentar) e salva 1 PNG do framebuffer, depois
        // sai. Sem janela interativa - serve pra COMPARAR o jogo com o mock lado a lado.
        const char* capture_path = std::getenv("GUSWORLD_RMLUI_CAPTURE");
        int frame_no = 0;
        int capture_at_frame = 20;  // ~assenta o 1o layout/fonte antes do shot
        // DEBUG: GUSWORLD_RMLUI_CAPTURE_FRAME=<N> adia o shot p/ o frame N (pra pegar um
        // turno especifico - ex. jogador vs inimigo - sem driver de input). >0 sobrescreve.
        if (const char* cf = std::getenv("GUSWORLD_RMLUI_CAPTURE_FRAME")) {
            const int v = std::atoi(cf);
            if (v > 0) capture_at_frame = v;
        }

        // DIAGNOSTICO (ADR-009): GUSWORLD_RMLUI_FRAMES=N roda o LOOP INTERATIVO COMPLETO
        // (com Update/compose do HUD por frame) e sai apos N frames. Diferente da captura
        // (1 frame), exercita a composicao repetida sob ASan pra pegar leak/UAF-apos-N.
        int max_frames = 0;
        if (const char* mf = std::getenv("GUSWORLD_RMLUI_FRAMES")) {
            max_frames = std::atoi(mf);
        }
        // DIAGNOSTICO: GUSWORLD_RMLUI_AUTOSTART=1 da Encarar automaticamente no 1o frame
        // (pra capturar o estado de COMBATE sem input). So pra smoke visual/teste.
        const bool autostart = [] {
            const char* e = std::getenv("GUSWORLD_RMLUI_AUTOSTART");
            return e != nullptr && e[0] == '1';
        }();

        // DIAGNOSTICO/PROVA (HOVER dos pills): GUSWORLD_BATTLE_HOVER_SELFTEST=<prefixo> assenta a
        // cena ate a vez do jogador, FORCA a selecao em [ATACAR] e roda 4 FASES injetando um
        // UiEvent::MouseMove SINTETICO (mesmo pipeline do mouse real: process_event ->
        // Context::ProcessMouseMove -> pseudo-classe :hover), capturando 1 PNG por fase pra
        // PROVAR o hover surgindo e SUMINDO pelo movimento do ponteiro, sem mouse fisico (que nao
        // da pra simular numa captura estatica). Fases (sufixos do prefixo):
        //   _a_none        ponteiro FORA do cockpit           -> nenhum pill em hover (normal)
        //   _b_hover_unsel ponteiro sobre SCAN (nao-selec.)   -> hover PURO (fundo+borda)
        //   _c_hover_sel   ponteiro sobre ATACAR (SELECIONADO)-> sel+hover: .sel (cyan) DOMINA
        //   _d_none_again  ponteiro FORA de novo              -> hover REMOVIDO (== _a: toggle)
        // So diagnostico: dirige a cena/UI pela MESMA API publica; nao muda o motor nem o render.
        const char* hover_selftest_prefix = std::getenv("GUSWORLD_BATTLE_HOVER_SELFTEST");
        const bool hover_selftest =
            hover_selftest_prefix != nullptr && hover_selftest_prefix[0] != '\0';
        int hover_phase = 0;        // 0=none 1=unsel 2=sel 3=none-again
        int hover_phase_frame = 0;  // frames assentados na fase atual (antes de capturar)

        // DIAGNOSTICO/PROVA (COCKPIT-SFX-HOVER-CLIQUE): GUSWORLD_BATTLE_UI_SFX_SELFTEST=1 prova
        // HEADLESS, sem mouse fisico nem SDL_PushEvent, que (1) o SOM DE HOVER dispara com
        // EDGE-DETECT (1 play por pill NOVO; parado no mesmo pill NAO repica; sair-e-voltar
        // redispara) e (2) o SOM DE CLIQUE dispara ao acionar uma pill. Roda DENTRO do loop
        // (get_element_box exige a geometria assentada por >=1 ui->update()): apos ~12 frames
        // varre um MouseMove SINTETICO pelos 6 pills via handle_cockpit_hover (o MESMO caminho
        // do mouse real) e aciona o callback de clique (o MESMO do glintfx), imprimindo
        // sfx_play_count() (hook de prova, AudioEngine) - N hovers => N plays. Analogo ao
        // GUSWORLD_SYSMENU_HOVER_SELFTEST do menu; encerra ao terminar (running=false).
        const bool ui_sfx_selftest = [] {
            const char* e = std::getenv("GUSWORLD_BATTLE_UI_SFX_SELFTEST");
            return e != nullptr && e[0] == '1';
        }();
        int ui_sfx_settle_frame = 0;  // frames assentados antes de disparar a varredura
        bool ui_sfx_done = false;     // varredura ja rodou (1 vez so)

        // DIAGNOSTICO/PROVA (ANIMACAO DE COMBATE, W2): GUSWORLD_BATTLE_ANIM_SELFTEST=
        // <prefixo> roda um SCRIPT POR FRAME (dt FIXO 1/60, deterministico) que captura 5
        // PNGs num UNICO launch, provando o battle-anim.md par.2 com os placeholders:
        //   _a_cast_travel   demo de cast (dormante): bolinha em VOO caster -> alvo
        //   _b_cast_react    impacto do projetil: alvo no TRANCO do hit-react (cosmetico)
        //   _c_melee_windup  [Atacar] do jogador: atacante DESLOCADO no meio do dash
        //   _d_melee_contact contato: atacante NO alvo + floater + tranco do alvo
        //   _e_melee_rest    recovery concluida: TODOS de volta ao repouso exato
        // Dirige a cena pela MESMA API publica (debug_cast_demo e a UNICA porta de
        // diagnostico, tambem publica); nao muda o motor nem o render.
        const char* anim_selftest_prefix = std::getenv("GUSWORLD_BATTLE_ANIM_SELFTEST");
        const bool anim_selftest =
            anim_selftest_prefix != nullptr && anim_selftest_prefix[0] != '\0';
        int anim_f0 = -1;              // frame do aim_confirm (inicio do windup melee)
        std::string anim_attacker_id;  // pro log de offsets (prova textual)
        std::string anim_target_id;
        bool anim_done_cast = false;   // fases ja disparadas (1 vez cada)
        bool anim_done_melee = false;

        // DIAGNOSTICO/PROVA (SPRITE ANIMADO, W3): GUSWORLD_BATTLE_SPRITE_SELFTEST=
        // <prefixo> roda um SCRIPT POR FRAME (dt FIXO 1/60, deterministico) que FORCA o
        // GUS como atacante (navega o PICKER §4.1 ate ele - nao o pre-selecionado por
        // SPD) e captura 4 PNGs num unico launch, provando a troca placeholder->sprite
        // (battle-anim.md par.1.1/3.2) com os frames REAIS do disco:
        //   _a_idle_rest      battle_idle no REPOUSO (antes do Encarar)
        //   _b_run_dash       frame de RUN no MEIO do dash (ida do melee)
        //   _c_attack_swing   attack_melee na CAUDA da aproximacao (swing -> contato)
        //   _d_idle_back      de volta ao battle_idle no repouso exato (pos-Return)
        // Cada captura loga o offset do director + o clip/frame do sprite (prova
        // textual). Dirige pela MESMA API publica do jogador; zero motor/render novo.
        const char* sprite_selftest_prefix =
            std::getenv("GUSWORLD_BATTLE_SPRITE_SELFTEST");
        const bool sprite_selftest =
            sprite_selftest_prefix != nullptr && sprite_selftest_prefix[0] != '\0';
        int sprite_f0 = -1;              // frame do aim_confirm do GUS (inicio do dash)
        bool sprite_done_drive = false;  // drive disparado (1 vez)

        // DIAGNOSTICO/CAPTURA: GUSWORLD_BATTLE_PUMP_TO=<actor_id> conduz o combate ate esse
        // ator ser o ATIVO, ANTES do loop de exibicao - pra CAPTURAR a fila CTB na vez de um
        // ator especifico (ex. um de SPD BAIXA como "jaci") sem driver de input. Encara,
        // bombeia o ritmo nos turnos de inimigo e AUTO-RESOLVE os turnos de jogador (Atacar
        // no alvo sugerido) ate chegar no alvo. So diagnostico: LE/dirige a cena pela MESMA
        // API publica do jogador, nao muda o motor.
        if (const char* pump_to = std::getenv("GUSWORLD_BATTLE_PUMP_TO")) {
            if (pump_to[0] != '\0') {
                const std::string want(pump_to);
                if (scene.is_intro()) {
                    scene.start_combat();
                }
                for (int i = 0; i < 600; ++i) {
                    const auto* a = scene.active_actor();
                    if ((a != nullptr && a->id() == want) || scene.combat_over()) {
                        break;
                    }
                    if (scene.waiting_player_input()) {
                        // W1 item 4: a vez da party abre no PICKER (§4.1); confirma o
                        // pre-selecionado pra chegar ao menu de verbos e seguir o pump.
                        if (scene.is_choosing_actor()) {
                            scene.actor_picker_confirm();
                        }
                        for (int k = 0; k < 8 &&
                                        scene.menu().selected_verb() != BattleVerb::Atacar;
                             ++k) {
                            scene.menu_move(+1);
                        }
                        scene.menu_confirm();  // Atacar -> entra na mira
                        if (scene.is_aiming()) {
                            scene.aim_confirm();  // confirma o alvo -> inicia o WINDUP
                        }
                        // W2 (battle-anim.md par.3.1): a resolucao do [Atacar] e
                        // DEFERIDA ate o CONTATO (fim da aproximacao). Bombeia o
                        // windup ate o motor resolver antes de seguir o pump.
                        for (int k = 0; k < 120 && scene.player_action_in_flight();
                             ++k) {
                            scene.update(1.0f / 60.0f);
                        }
                    } else {
                        scene.skip();
                        scene.update(1.0f / 60.0f);  // bombeia 1 beat de inimigo
                    }
                }
                const auto* a = scene.active_actor();
                std::cout << "BattlePreview: [pump] alvo=" << want << " ator ativo agora="
                          << (a != nullptr ? a->id() : "?")
                          << " fila=" << scene.queue_len() << "\n";
            }
        }

        // DIAGNOSTICO/CAPTURA: GUSWORLD_BATTLE_AIM=1 deixa a cena PARADA no MODO-MIRA de
        // [Atacar] do jogador ativo (assenta o pacing ate a vez do jogador, seleciona Atacar
        // e ENTRA na mira SEM confirmar) - pra CAPTURAR a previa de dano "-N" sobre o alvo,
        // sem driver de input. GUSWORLD_BATTLE_AIM_MOVE=<n> navega n passos entre os inimigos
        // (mostra o "-N" ATUALIZANDO por alvo). Combina com PUMP_TO (ex.: mira na vez do Gus).
        // Dirige pela MESMA API publica do jogador; NAO muda o motor nem o render.
        const bool stop_in_aim = [] {
            const char* e = std::getenv("GUSWORLD_BATTLE_AIM");
            return e != nullptr && e[0] == '1';
        }();
        if (stop_in_aim) {
            if (scene.is_intro()) {
                scene.start_combat();  // Encarar
            }
            // Assenta o pacing ate um turno de JOGADOR esperando input (skip/update nao
            // resolvem o turno do jogador; so avancam os beats de inimigo/anuncio).
            for (int i = 0; i < 240 && !scene.combat_over() &&
                            !scene.waiting_player_input();
                 ++i) {
                scene.skip();
                scene.update(1.0f / 60.0f);
            }
            // W1 item 4: atravessa o PICKER de ator (§4.1) pra chegar ao menu de verbos.
            if (scene.is_choosing_actor()) {
                scene.actor_picker_confirm();
            }
            if (scene.waiting_player_input() && !scene.is_aiming()) {
                for (int k = 0; k < 8 &&
                                scene.menu().selected_verb() != BattleVerb::Atacar;
                     ++k) {
                    scene.menu_move(+1);
                }
                scene.menu_confirm();  // Atacar -> ENTRA na mira (nao confirma)
                if (const char* mv = std::getenv("GUSWORLD_BATTLE_AIM_MOVE")) {
                    const int steps = std::atoi(mv);
                    for (int k = 0; k < steps && scene.is_aiming(); ++k) {
                        scene.aim_move(+1);
                    }
                }
                // GUSWORLD_BATTLE_AIM_SHIELD=<mag>: aplica um Shield de pool <mag> no ALVO
                // mirado (ponteiro mutavel da fila) so pra CAPTURAR o "-N" REDUZIDO. Usa a
                // MESMA API de status do jogo (add_status); nao muda o motor nem o render.
                if (const char* sh = std::getenv("GUSWORLD_BATTLE_AIM_SHIELD")) {
                    const int mag = std::atoi(sh);
                    const auto* aimed = scene.aim_target();
                    if (mag > 0 && aimed != nullptr) {
                        for (gus::domain::combat::CombatActor* act :
                             scene.machine().queue().order()) {
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
            const auto* t = scene.aim_target();
            const auto* atk = scene.active_actor();
            const int previsto =
                (t != nullptr && atk != nullptr)
                    ? scene.machine().preview_basic_attack_damage(*atk, *t)
                    : -1;
            std::cout << "BattlePreview: [aim] modo-mira="
                      << (scene.is_aiming() ? "on" : "off")
                      << " atacante=" << (atk != nullptr ? atk->id() : "?")
                      << " alvo=" << (t != nullptr ? t->id() : "?")
                      << " dano_previsto=" << previsto << " (badge deve mostrar \"-"
                      << previsto << "\")\n";
        }

        // HOVER-SELFTEST (setup): assenta ate a vez do jogador (skip/update so avancam beats de
        // inimigo/anuncio; NAO resolvem o turno do jogador) e FORCA a selecao em [ATACAR]
        // (indice 2, verbo cyan). ATACAR e o PIOR CASO pra "hover nao ofuscar": seu .sel carrega
        // o glow cyan mais forte (.verb.cyan.sel), entao provar que o hover-sobre-o-selecionado
        // nao o apaga vale pros demais. A cena FICA parada em waiting_player_input (o menu segue
        // exibido); nada no loop muda a selecao (nao injeto navegacao), so o MouseMove sintetico.
        if (hover_selftest) {
            if (scene.is_intro()) {
                scene.start_combat();
            }
            for (int i = 0; i < 240 && !scene.combat_over() &&
                            !scene.waiting_player_input();
                 ++i) {
                scene.skip();
                scene.update(1.0f / 60.0f);
            }
            // W1 item 4: atravessa o PICKER de ator (§4.1) pra chegar ao menu de verbos.
            if (scene.is_choosing_actor()) {
                scene.actor_picker_confirm();
            }
            if (scene.waiting_player_input() && !scene.is_aiming()) {
                for (int k = 0; k < 8 &&
                                scene.menu().selected_verb() != BattleVerb::Atacar;
                     ++k) {
                    scene.menu_move(+1);
                }
            }
            std::cout << "BattlePreview: [hover-selftest] waiting_player="
                      << scene.waiting_player_input()
                      << " sel_index=" << scene.menu().selected_index()
                      << " (esperado 2=ATACAR); prefixo=" << hover_selftest_prefix << "\n";
        }

        // COCKPIT-SFX-HOVER-CLIQUE (setup do self-test de SOM): MESMA assent do hover-selftest
        // visual - assenta ate a vez do jogador (menu de verbos interativo) pra o gate de
        // handle_cockpit_hover liberar o som. A varredura em si roda no loop (get_element_box).
        if (ui_sfx_selftest) {
            if (scene.is_intro()) {
                scene.start_combat();
            }
            for (int i = 0; i < 240 && !scene.combat_over() &&
                            !scene.waiting_player_input();
                 ++i) {
                scene.skip();
                scene.update(1.0f / 60.0f);
            }
            if (scene.is_choosing_actor()) {
                scene.actor_picker_confirm();  // atravessa o picker (§4.1) ate o menu de verbos
            }
            std::cout << "BattlePreview: [ui-sfx-selftest] waiting_player="
                      << scene.waiting_player_input()
                      << " (menu de verbos interativo esperado)\n";
        }

        bool running = true;
        bool have_last = false;
        unsigned long long last_ns = 0;
        int glintfx_injected = 0;  // SMOKE: conta eventos injetados na UI (prova do pipeline)

        // COCKPIT-SFX-HOVER-CLIQUE: ULTIMO pill de verbo sob o ponteiro (-1 = fora de
        // qualquer pill). Alimenta o edge-detect do SOM de hover (toca so ao ENTRAR num
        // pill NOVO, nunca a cada frame parado sobre o mesmo - ui_hover_entered_new_item).
        int hovered_verb = -1;

        // Hit-test/edge-detect do SOM de hover dos pills de verbo, fatorado em lambda pra ser
        // chamado tanto pelo SDL_EVENT_MOUSE_MOTION real quanto pelo self-test headless
        // (GUSWORLD_BATTLE_UI_SFX_SELFTEST) - MESMO caminho de codigo prova o comportamento,
        // sem duplicar (espelha handle_mouse_motion de system_menu_loop.cpp). O VISUAL do
        // hover ja e' nativo do glintfx (process_event(MouseMove) mais abaixo); AQUI so o SOM.
        // Consulta a geometria REAL de cada pill via UiLayer::get_element_box (ids estaveis
        // kCockpitVerbElementIds) e delega a decisao "qual bateu" ao POCO ui_hover_index.
        // GATE: so soa quando o MENU DE VERBOS esta de fato interativo (vez do jogador, sem
        // mira/escolha-de-ator/abertura) - mesma condicao em que a pill responde ao clique;
        // evita blip de hover durante turno de inimigo / animacao.
        auto handle_cockpit_hover = [&](float mx, float my) {
            if (!glintfx_on || !ui) return;
            if (!scene.waiting_player_input() || scene.is_aiming() ||
                scene.is_choosing_actor() || scene.is_intro()) {
                hovered_verb = -1;  // menu nao-interativo: zera (nao "prende" o ultimo pill)
                return;
            }
            gus::app::screens::UiHoverBox boxes[gus::app::screens::kBattleVerbCount];
            for (int i = 0; i < gus::app::screens::kBattleVerbCount; ++i) {
                const glintfx::ElementBox b =
                    ui->get_element_box(gus::app::screens::kCockpitVerbElementIds[i]);
                boxes[i] = gus::app::screens::UiHoverBox{b.found, b.x, b.y, b.w, b.h};
            }
            const int new_hover = gus::app::screens::ui_hover_index(
                mx, my, boxes, gus::app::screens::kBattleVerbCount);
            if (gus::app::screens::ui_hover_entered_new_item(hovered_verb, new_hover)) {
                audio_ptr->play_sfx(ui_hover_sfx_id);
            }
            hovered_verb = new_hover;
        };

        // DIAGNOSTICO/PROVA (GLINTFX-CLICK, ex-Incremento A2): GUSWORLD_BATTLE_MOUSE_SELFTEST=1
        // exercita o roteamento clique->acao SEM mouse fisico. O clique nos pills de verbo ja
        // NAO passa por hit-test de pixel algum (o glintfx resolve o `id` internamente, o mesmo
        // hit-test que move o :hover); entao (1) prova o round-trip id->indice pra cada um dos 6
        // verbos chamando battle_cockpit_verb_click DIRETO (a MESMA funcao que o
        // set_click_callback do UiLayer chama no clique real); (2) CHAMA o pill [ATACAR] e mostra
        // que entrou na MIRA; (3) CLICA o slot de um inimigo (esse SIM em coordenadas de MUNDO,
        // via battle_mouse_click) e mostra que a mira confirmou naquele alvo. So diagnostico
        // (API publica).
        const bool mouse_selftest = [] {
            const char* e = std::getenv("GUSWORLD_BATTLE_MOUSE_SELFTEST");
            return e != nullptr && e[0] == '1';
        }();
        if (mouse_selftest) {
            if (scene.is_intro()) {
                scene.start_combat();
            }
            for (int i = 0; i < 240 && !scene.combat_over() &&
                            !scene.waiting_player_input();
                 ++i) {
                scene.skip();
                scene.update(1.0f / 60.0f);
            }
            // W1 item 4: a 1a vez da party agora ABRE no PICKER de ator (§4.1) antes do menu
            // de verbos. Este diagnostico (pre-picker) espera o MENU: atravessa o picker
            // confirmando o pre-selecionado (maior SPD) pra chegar ao menu, como antes.
            if (scene.is_choosing_actor()) {
                scene.actor_picker_confirm();
            }
            std::cout << "BattlePreview: [mouse-selftest] pw0xph0=" << pw0 << "x" << ph0
                      << " waiting_player=" << scene.waiting_player_input() << "\n";
            // (1) round-trip: o `id` de cada pill (fonte unica: kCockpitVerbElementIds, ordem
            // = BattleVerb) -> cockpit_verb_index_for_click_id -> deve devolver o MESMO indice.
            for (int v = 0; v < gus::app::screens::kBattleVerbCount; ++v) {
                const char* id = gus::app::screens::kCockpitVerbElementIds[v];
                const int back = gus::app::screens::cockpit_verb_index_for_click_id(id);
                std::cout << "  pill[" << v << "] "
                          << kVerbLabels[static_cast<std::size_t>(v)] << " id=" << id
                          << " -> indice=" << back << (back == v ? " OK" : " MISMATCH") << "\n";
            }
            // (2) ACIONA o pill ATACAR (o MESMO caminho do callback real). Espera: entra na
            // MIRA. Chama battle_cockpit_verb_click DIRETO (nao ha pixel/evento SDL a simular:
            // o glintfx ja teria resolvido o id antes de chamar o callback registrado).
            if (scene.waiting_player_input() && !scene.is_aiming()) {
                battle_cockpit_verb_click(
                    scene,
                    gus::app::screens::kCockpitVerbElementIds[static_cast<int>(
                        BattleVerb::Atacar)]);
                std::cout << "  CLIQUE (callback) pill ATACAR -> is_aiming="
                          << (scene.is_aiming() ? "on" : "off") << " (esperado on)\n";
            }
            // (3) CLICA o slot de um inimigo (o 2o miravel, se houver). Espera: confirma nele.
            if (scene.is_aiming()) {
                const int want = scene.aim_count() >= 2 ? 1 : 0;
                const gus::app::screens::ArenaLayout arena = gus::app::screens::arena_layout(
                    scene.party_count(), scene.enemy_count(), scene.gus_party_index());
                const gus::core::spatial::Rect s =
                    arena.enemies[static_cast<std::size_t>(want)].rect;
                const float wcx = s.x + s.w * 0.5f, wcy = s.y + s.h * 0.5f;
                const float pxcx = wcx / 960.0f * static_cast<float>(pw0);
                const float pxcy = wcy / 540.0f * static_cast<float>(ph0);
                const std::string alvo_antes =
                    scene.aim_target() != nullptr ? scene.aim_target()->id() : "?";
                const int hit = scene.aim_index_at_arena(wcx, wcy);
                battle_mouse_click(scene, pxcx, pxcy, pw0, ph0);
                std::cout << "  CLIQUE inimigo#" << want << " (alvo pre-clique=" << alvo_antes
                          << ") world(" << wcx << "," << wcy << ") px(" << pxcx << "," << pxcy
                          << ") hit_idx=" << hit
                          << " -> is_aiming=" << (scene.is_aiming() ? "on" : "off")
                          << " (esperado off: confirmou; W2 - o contato resolve no fim "
                             "do windup)\n";
            }
            std::cout << "BattlePreview: [mouse-selftest] concluido; encerrando.\n";
            running = false;
        }

        // DIAGNOSTICO/PROVA (Escolha de ator, §4.1): GUSWORLD_BATTLE_ACTOR_SELFTEST=1 dirige uma
        // cena FRESCA ate a vez do BLOCO da party (onde is_choosing_actor()==true) e injeta INPUT
        // SINTETICO pelo MESMO roteamento do host (battle_key_down / battle_mouse_click), pra
        // PROVAR o wiring sem input fisico (dificil de simular em captura estatica):
        //   (A) SETA (battle_key_down DOWN) MOVE o cursor do picker (SEM confirmar);
        //   (B) CLIQUE no slot de um membro (battle_mouse_click) ESCOLHE e CONFIRMA aquele membro;
        //   (C) TECLA 1/2/3 (battle_key_down) escolhe e CONFIRMA o n-esimo membro na hora.
        // Cada proof usa sua PROPRIA BattleScene (o confirm consome o modo). So diagnostico:
        // dirige pela MESMA API publica do host; nao muda o motor nem o render.
        const bool actor_selftest = [] {
            const char* e = std::getenv("GUSWORLD_BATTLE_ACTOR_SELFTEST");
            return e != nullptr && e[0] == '1';
        }();
        if (actor_selftest) {
            // Assenta uma cena ate o picker abrir (party-block com >1 elegivel). skip()+update()
            // avancam os beats de inimigo/anuncio; NADA auto-confirma o picker -> para no modo.
            auto drive_to_picker = [](BattleScene& s) {
                if (s.is_intro()) {
                    s.start_combat();
                }
                for (int i = 0; i < 600 && !s.combat_over() && !s.is_choosing_actor(); ++i) {
                    s.skip();
                    s.update(1.0f / 60.0f);
                }
            };
            // id do i-esimo elegivel (0-based) na escolha corrente; "?" fora de faixa.
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
                battle_key_down(sa, SDLK_DOWN, dummy);  // roteamento real do host
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
                const int want_idx =
                    sb.actor_pick_count() >= 2 ? 1 : 0;  // 2o elegivel (NAO o pre-selecionado)
                const std::string want_id = choice_id(sb, want_idx);
                const gus::app::screens::ArenaLayout arena = gus::app::screens::arena_layout(
                    sb.party_count(), sb.enemy_count(), sb.gus_party_index());
                // Acha o SLOT da party cujo centro casa o elegivel want_idx pelo MESMO hit-test do
                // host (actor_pick_index_at_arena) -> nao presume o mapeamento slot->elegivel.
                float pxc = -1.0f, pyc = -1.0f, wcx = -1.0f, wcy = -1.0f;
                for (int k = 0; k < sb.party_count(); ++k) {
                    const gus::core::spatial::Rect r =
                        arena.party[static_cast<std::size_t>(k)].rect;
                    const float cx = r.x + r.w * 0.5f, cy = r.y + r.h * 0.5f;
                    if (sb.actor_pick_index_at_arena(cx, cy) == want_idx) {
                        wcx = cx;
                        wcy = cy;
                        pxc = cx / 960.0f * static_cast<float>(pw0);
                        pyc = cy / 540.0f * static_cast<float>(ph0);
                        break;
                    }
                }
                if (pxc >= 0.0f) {
                    battle_mouse_click(sb, pxc, pyc, pw0, ph0);  // roteamento real do host
                    const std::string got =
                        sb.active_actor() != nullptr ? sb.active_actor()->id() : "?";
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
                const int nth = sc.actor_pick_count() >= 2 ? 2 : 1;  // 2a hotkey se houver
                const std::string want_id = choice_id(sc, nth - 1);
                const SDL_Keycode kc = nth == 1 ? SDLK_1 : (nth == 2 ? SDLK_2 : SDLK_3);
                bool dummy = true;
                battle_key_down(sc, kc, dummy);  // roteamento real -> actor_picker_hotkey(nth)
                const std::string got =
                    sc.active_actor() != nullptr ? sc.active_actor()->id() : "?";
                const bool ok = !sc.is_choosing_actor() && got == want_id;
                std::cout << "BattlePreview: [actor-selftest] (C tecla " << nth << ") elegivel#"
                          << (nth - 1) << "=" << want_id << " -> choosing=" << sc.is_choosing_actor()
                          << " ator_ativo=" << got << "  "
                          << (ok ? "OK (escolheu+confirmou)" : "FALHA") << "\n";
            }

            std::cout << "BattlePreview: [actor-selftest] concluido; encerrando.\n";
            running = false;
        }

        // M7-COSTURA Inc 2 (ADR-012 decisao 5): FADE-IN de ENTRADA - a tela CLAREIA
        // (overlay preto 1->0) sobre o 1o frame REAL da arena+HUD ja prontos (nao um
        // placeholder). SO roda quando quem chamou pediu (fade_in_seconds>0 - a Maestro
        // pede; o --battle standalone e todo selftest/captura acima pedem 0, ver os
        // defaults do header) E a cena ainda esta RODANDO (nenhum selftest ja zerou
        // `running` e encerrou tudo antes do 1o frame interativo). Pump de eventos
        // LIMITADO a SDL_EVENT_QUIT - a batalha ainda nao "comecou" pro jogador enquanto
        // a tela esta preta (teclado/mouse ficam mudos ate o fade acabar e o loop
        // principal comecar).
        if (fade_in_seconds > 0.0f && running) {
            const unsigned long long fade_start_ns = SDL_GetTicksNS();
            bool fading = true;
            while (fading) {
                SDL_Event fev;
                while (SDL_PollEvent(&fev)) {
                    if (fev.type == SDL_EVENT_QUIT) {
                        running = false;
                        quit_requested = true;
                    }
                }
                if (!running) {
                    break;
                }
                int fpw = kWindowW, fph = kWindowH;
                SDL_GetWindowSizeInPixels(window, &fpw, &fph);
                if (fpw < 1 || fph < 1) {
                    SDL_Delay(16);
                    continue;
                }
                const float elapsed =
                    static_cast<float>(SDL_GetTicksNS() - fade_start_ns) / 1.0e9f;
                scene.render(renderer, static_cast<float>(fpw), static_cast<float>(fph));
                if (glintfx_on && ui) {
                    if (fpw != pw0 || fph != ph0) {
                        ui->set_viewport(fpw, fph);
                        if (glintfx_dp_override <= 0.0f) {
                            ui->set_dp_ratio(static_cast<float>(fpw) / 960.0f);
                        }
                        pw0 = fpw;
                        ph0 = fph;
                    }
                    ui->update();
                    ui->render();
                }
                // M7-COSTURA Inc 2c: sequencia de frames do boot pixelizado - esta E
                // a metade "Revealing indo pra batalha" da transicao inteira (a arena
                // aparecendo; continua de onde o lado da CIDADE parou - ver o header
                // extenso em gus/core/anim/boot_pixel_sequence.hpp) no lugar do
                // glitch procedural (aposentado - o lider VETOU o visual, "pareceu
                // bug"). `t` = elapsed/fade_in_seconds, a MESMA fracao que
                // fade_overlay_alpha usava internamente antes do flip de kIn (ver
                // gus/app/boot_pixel_overlay.hpp).
                {
                    const float t = fade_in_seconds > 0.0f
                                        ? std::min(1.0f, elapsed / fade_in_seconds)
                                        : 1.0f;
                    boot_overlay.draw(renderer, gus::app::screens::battle_screen_rect(),
                                      gus::core::anim::BootPixelLeg::kToBattleRevealing,
                                      t);
                }
                SDL_GL_SwapWindow(window);
                fading = elapsed < fade_in_seconds;
            }
            std::cout << "BattlePreview: [fade] entrada (kIn) concluido em "
                      << fade_in_seconds << "s.\n";
        }

        while (running) {
            SDL_Event ev;
            while (SDL_PollEvent(&ev)) {
                // ADR-010 F1 SMOKE: injeta o evento na UI glintfx (caminho NOVO; a UI e
                // display-only por ora, mas ja recebe input). Em paralelo ao handler de
                // cena abaixo (ambos veem o mesmo evento). Loga os PRIMEIROS eventos
                // injetados (de qualquer tipo) + toda tecla, p/ provar que o evento SDL
                // atravessa sdl_to_glintfx -> process_event ate o motor de UI.
                if (glintfx_on && ui) {
                    glintfx::UiEvent ge{};
                    if (sdl_to_glintfx(ev, window, &ge)) {
                        ui->process_event(ge);
                        const bool is_key = ge.type == glintfx::UiEvent::Type::Key;
                        if (glintfx_injected < 6 || (is_key && ge.pressed)) {
                            std::cout << "BattlePreview: [glintfx] input injetado #"
                                      << glintfx_injected
                                      << " type=" << static_cast<int>(ge.type)
                                      << " key=" << static_cast<int>(ge.key)
                                      << " x=" << ge.x << " y=" << ge.y
                                      << " mods=" << ge.modifiers << "\n";
                        }
                        ++glintfx_injected;
                    }
                }
                if (ev.type == SDL_EVENT_QUIT) {
                    // FIX BUG-3: o jogador clicou no X da janela DURANTE a batalha. Isto
                    // NAO e um CombatOutcome (nao e vitoria/derrota/fuga) - e um pedido
                    // pra encerrar o PROGRAMA INTEIRO. Sinaliza via quit_requested (grava
                    // no choke-point de saida, junto do outcome) - a Maestro usa isto pra
                    // NAO voltar a cidade (ver run_battle_preview_embedded no header).
                    running = false;
                    quit_requested = true;
                } else if (ev.type == SDL_EVENT_KEY_DOWN) {
                    // Roteamento de teclado EXTRAIDO pra battle_key_down (funcao-livre, testavel
                    // pelo self-test sintetico; espelha battle_mouse_click). Cobre menu de verbos,
                    // MODO-MIRA (§3.5), ESCOLHA DE ATOR (§4.1, prioridade + teclas 1/2/3) e Esc.
                    //
                    // REVERT (BATTLE-ESC-PAUSE-ACTOR-LIST, 2026-07-05): o HOST REAL volta a
                    // chamar SEM `out_effect` (nullptr implicito) - o comportamento ORIGINAL de
                    // sempre, pre-integracao do menu de pausa: mira cancela / preview volta pra
                    // lista / lista e no-op / PILHA VAZIA fecha o viewer (running=false). O
                    // CRASH real ao vivo (lider testou): abrir o menu de pausa AQUI criava um
                    // SEGUNDO contexto/UiLayer RmlUi (run_system_menu_loop_gl_current) ANINHADO
                    // enquanto o cockpit da batalha (o PRIMEIRO UiLayer, `ui` acima) ainda estava
                    // VIVO - RmlUi tem estado global (element pool) que NAO suporta 2 contextos
                    // simultaneos no mesmo processo; o log capturado foi "Element meta pool not
                    // empty on shutdown, 75 object(s) leaked" ao fechar/trocar de contexto. Isto
                    // e um LIMITE DE NESTING DA LIB, nao um bug de logica nossa - decisao do lider:
                    // reverter, SEM menu de pausa na arena por ora (ver TODO.md). A FUNCAO
                    // battle_key_down/BattleEscEffect/OpenPauseMenu CONTINUAM existindo e
                    // funcionando (testadas em battle_key_routing_test.cpp com out_effect
                    // explicito) - so este HOST DE PRODUCAO para de passar o ponteiro. O caminho
                    // da CIDADE (Maestro::open_pause_from_city) usa uma tecnica DIFERENTE (contexto
                    // GL proprio e dedicado, nao aninhado sobre um UiLayer vivo) e NAO foi tocado.
                    battle_key_down(scene, ev.key.key, running);
                } else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                           ev.button.button == SDL_BUTTON_LEFT) {
                    // MOUSE (A2): clique ESQUERDO aciona verbo (menu) ou alvo (mira). ADITIVO
                    // ao teclado (que segue igual). O forward pro glintfx (acima) ja rolou (so
                    // visual); a ACAO real vem do hit-test do host em px de janela.
                    int pw = kWindowW, ph = kWindowH;
                    SDL_GetWindowSizeInPixels(window, &pw, &ph);
                    battle_mouse_click(scene, ev.button.x, ev.button.y, pw, ph);
                } else if (ev.type == SDL_EVENT_MOUSE_MOTION) {
                    // MOUSE (A2, hover): na mira, passar sobre um inimigo pre-seleciona (realce).
                    int pw = kWindowW, ph = kWindowH;
                    SDL_GetWindowSizeInPixels(window, &pw, &ph);
                    battle_mouse_hover(scene, ev.motion.x, ev.motion.y, pw, ph);
                    // COCKPIT-SFX-HOVER-CLIQUE: SOM de hover dos pills de verbo (visual :hover
                    // ja saiu no process_event nativo acima). Edge-detect: toca so ao ENTRAR
                    // num pill NOVO. Em px de janela == px do viewport (sem HiDPI neste alvo,
                    // mesmo pressuposto do forward glintfx / battle_mouse_click).
                    handle_cockpit_hover(ev.motion.x, ev.motion.y);
                }
            }
            if (!running) {
                break;
            }

            // dt real desde o ultimo frame (segundos): envelhece os numeros flutuantes
            // e dirige o pacing (2 beats por turno). CLAMP anti-salto: o 1o frame apos o
            // setup pesado (bake do atlas de fonte + load de texturas) chega com dt
            // ENORME (1-2s); sem teto, o pacing avancaria intro+anuncio+resolucao de uma
            // vez e a tela "abriria com o ataque ja feito" (bug pego pelo lider 3x). O
            // teto de 50ms (~3 frames a 60fps) garante que nenhum frame pule um beat.
            const unsigned long long now_ns = SDL_GetTicksNS();
            float dt = 0.0f;
            if (have_last) {
                dt = static_cast<float>(now_ns - last_ns) / 1.0e9f;
                if (dt > 0.05f) {
                    dt = 0.05f;  // clamp anti-salto (spiral-of-death / 1o frame lento)
                }
            }
            have_last = true;
            last_ns = now_ns;
            // ANIM/SPRITE-SELFTEST: dt FIXO 1/60 (script por frame deterministico; o
            // relogio real varia por maquina/driver e desalinharia as capturas).
            if (anim_selftest || sprite_selftest) {
                dt = 1.0f / 60.0f;
            }
            scene.update(dt);  // anima os floaters + pacing; nao toca a FSM

            // FIX BUG-2 (playtest ao vivo do lider: "perdi a batalha e ficou preso, so
            // tocando musica e mais nada"): o loop ANTES so saia via Esc explicito
            // (SDLK_ESCAPE, ver battle_key_down) ou fechando a janela - Victory/Defeat/
            // Fled NUNCA eram checados aqui, entao ao perder (ou fugir) a tela ficava
            // PARADA esperando um Esc que o jogador nao sabia que precisava apertar (a
            // musica seguia tocando pq nada disparava o choke-point de saida/fade-out
            // abaixo). FIX SIMETRICO: qualquer desfecho TERMINAL (Victory/Defeat/Fled -
            // scene.combat_over(), ver BattleScene::combat_over) encerra o loop sozinho.
            // O ultimo frame (com o floater/narracao da acao final, ja tocados durante o
            // proprio turno em BattleScene::resolve_one_turn) ainda RENDERIZA e faz o
            // SWAP mais abaixo nesta MESMA iteracao antes do loop checar `running` de
            // novo - nao corta o quadro final no meio. NAO interfere nos self-tests
            // acima: todos eles usam scripts que param de propósito ANTES de
            // scene.combat_over() virar true (`for (...&& !scene.combat_over()...)`).
            //
            // M7-COSTURA Inc 3 (flavor da derrota): com Defeat, `defeat_flavor_active()`
            // segura este `running=false` por kDefeatFlavorSeconds (battle_scene.cpp) -
            // o loop CONTINUA rodando (scene.update/render acima ja envelhecem e desenham
            // o overlay "reboot" sozinhos, nao precisa de nada extra aqui) ate o timer
            // esgotar; SO ENTAO trata como combat_over() de fato e sai (mesmo choke-point
            // de saida/fade-out da Maestro, agora um pouco mais tarde). Victory/Fled nunca
            // ativam defeat_flavor_active (gate por CombatOutcome::Defeat em si), entao
            // saem IMEDIATOS como sempre - comportamento inalterado pros dois outcomes.
            if (scene.combat_over() && !scene.defeat_flavor_active()) {
                running = false;
            }

            // ANIM-SELFTEST (script de ACOES por frame; as capturas ficam no fim do
            // frame, apos render+compose). Timeline (dt 1/60): frame 1 dispara o demo
            // de cast (windup 0.5s -> projetil ~f31 -> impacto ~f52); frame 62 comeca o
            // MELEE do jogador (pump ate a vez dele + [Atacar] confirmado = windup
            // 0.7s: contato em F0+42, Return ate F0+66, delay ate F0+90).
            if (anim_selftest) {
                if (frame_no == 1 && !anim_done_cast) {
                    anim_done_cast = true;
                    scene.debug_cast_demo();  // cosmetico (intro): cast + bolinha
                    std::cout << "BattlePreview: [anim-selftest] f1 cast demo iniciado\n";
                }
                if (frame_no == 62 && !anim_done_melee) {
                    anim_done_melee = true;
                    if (scene.is_intro()) {
                        scene.start_combat();
                    }
                    for (int i = 0; i < 240 && !scene.combat_over() &&
                                    !scene.waiting_player_input();
                         ++i) {
                        scene.skip();
                        scene.update(1.0f / 60.0f);
                    }
                    if (scene.is_choosing_actor()) {
                        scene.actor_picker_confirm();  // atravessa o picker (§4.1)
                    }
                    if (scene.waiting_player_input() && !scene.is_aiming()) {
                        for (int k = 0; k < 8 && scene.menu().selected_verb() !=
                                                     BattleVerb::Atacar;
                             ++k) {
                            scene.menu_move(+1);
                        }
                        const auto* atk = scene.active_actor();
                        anim_attacker_id = atk != nullptr ? atk->id() : "?";
                        scene.menu_confirm();  // entra na mira
                        const auto* tgt = scene.aim_target();
                        anim_target_id = tgt != nullptr ? tgt->id() : "?";
                        scene.aim_confirm();  // COMANDA: windup parte agora
                        anim_f0 = frame_no;
                        std::cout << "BattlePreview: [anim-selftest] f" << frame_no
                                  << " melee confirmado: atacante=" << anim_attacker_id
                                  << " alvo=" << anim_target_id << " in_flight="
                                  << scene.player_action_in_flight() << "\n";
                    } else {
                        std::cout << "BattlePreview: [anim-selftest] FALHA: nao chegou "
                                     "na vez do jogador\n";
                        running = false;
                    }
                }
            }

            // SPRITE-SELFTEST (drive): apos capturar o idle-em-repouso (frame 8, na
            // secao de capturas), o frame 10 conduz a cena ate a vez da party, navega
            // o PICKER ate o GUS (forca ele como atacante) e confirma [Atacar] + alvo.
            // O dash parte AGORA (sprite_f0) e as capturas seguintes provam run/golpe/
            // volta-ao-idle. MESMA API publica do jogador (padrao dos self-tests).
            if (sprite_selftest && frame_no == 10 && !sprite_done_drive) {
                sprite_done_drive = true;
                if (scene.is_intro()) {
                    scene.start_combat();
                }
                for (int i = 0; i < 240 && !scene.combat_over() &&
                                !scene.waiting_player_input();
                     ++i) {
                    scene.skip();
                    scene.update(1.0f / 60.0f);
                }
                if (scene.is_choosing_actor()) {
                    // Navega o cursor ate o GUS (§4.1: o jogador comanda QUEM age).
                    for (int k = 0; k < scene.actor_pick_count() &&
                                    (scene.actor_pick_target() == nullptr ||
                                     scene.actor_pick_target()->id() != "gus");
                         ++k) {
                        scene.actor_picker_move(+1);
                    }
                    scene.actor_picker_confirm();
                }
                const auto* atk = scene.active_actor();
                if (scene.waiting_player_input() && atk != nullptr &&
                    atk->id() == "gus" && !scene.is_aiming()) {
                    for (int k = 0; k < 8 && scene.menu().selected_verb() !=
                                                 BattleVerb::Atacar;
                         ++k) {
                        scene.menu_move(+1);
                    }
                    scene.menu_confirm();  // entra na mira
                    scene.aim_confirm();   // confirma o alvo: o dash parte AGORA
                    sprite_f0 = frame_no;
                    std::cout << "BattlePreview: [sprite-selftest] f" << frame_no
                              << " melee do GUS confirmado; in_flight="
                              << scene.player_action_in_flight() << "\n";
                } else {
                    std::cout << "BattlePreview: [sprite-selftest] FALHA: nao chegou "
                                 "na vez do GUS (ativo="
                              << (atk != nullptr ? atk->id() : "?") << ")\n";
                    running = false;
                }
            }

            // DIAGNOSTICO: auto-Encarar (captura do estado de combate sem input).
            if (autostart && scene.is_intro()) {
                scene.start_combat();
            }

            int pw = kWindowW, ph = kWindowH;
            SDL_GetWindowSizeInPixels(window, &pw, &ph);
            // GUARD janela minimizada (Wayland pode reportar 0): pula render+compose deste
            // frame (so pumpa eventos no proximo), evitando viewport/FBO de tamanho 0.
            if (pw < 1 || ph < 1) {
                SDL_Delay(16);
                continue;
            }
            // A arena desenha no backbuffer GL (clear + draws). O HUD RmlUi-GL3 compoe POR
            // CIMA (layer -> backbuffer). O SWAP e UNICO (SDL_GL_SwapWindow), depois do HUD.
            // FXTEST: pula a arena (so o HUD sobre fundo preto) pra isolar os efeitos.
            if (fxtest) {
                renderer.begin_frame(gus::core::spatial::Rect{0, 0, 960, 540}, pw, ph);
                renderer.end_frame();  // so o clear (fundo preto-ish); HUD compoe por cima
            } else {
                scene.render(renderer, static_cast<float>(pw), static_cast<float>(ph));
            }
            // ADR-010 F3: glintfx compoe no slot de HUD - depois da arena, antes do swap.
            // render() e compose-only (sem clear, sem swap; salva e restaura o estado GL
            // internamente). O viewport segue os pixels reais.
            if (glintfx_on && ui) {
                if (pw != pw0 || ph != ph0) {
                    ui->set_viewport(pw, ph);
                    if (glintfx_dp_override <= 0.0f) {
                        ui->set_dp_ratio(static_cast<float>(pw) / 960.0f);  // reescala logico
                    }
                    pw0 = pw;
                    ph0 = ph;
                }
                // ADR-010 F2b: ALIMENTA o data-model com os valores VIVOS do motor (POCO),
                // a cada frame. O MOTOR (scene) e a fonte de verdade: estado intro/combate,
                // ator ativo (nome/HP), selecao de verbo (scene.menu().selected_index ->
                // binding 'sel' -> classe .sel via data-class-sel), alvo, e o log narrado.
                if (glintfx_live) {
                    ui->set_bool("intro", scene.is_intro());
                    if (!scene.is_intro()) {
                        if (const auto* a = scene.active_actor(); a != nullptr) {
                            ui->set_string("nome", a->display_name().c_str());
                            // POLISH 2: o label segue o LADO do ator ativo. Party = role
                            // de party ("VETOR DO GAMBITO"); inimigo = designacao generica
                            // "INIMIGO" (o CombatActor nao expoe tipo/arquetipo - so lado).
                            // NUNCA pintar um inimigo com o role da party. Motor = autoridade.
                            ui->set_string("role", a->is_player_side() ? "VETOR DO GAMBITO"
                                                                       : "INIMIGO");
                            ui->set_number("hp", a->hp());
                            ui->set_number("hp_max", a->max_hp());
                            // RETRATO-VIVO: a moldura segue o ator ATIVO. Inimigo -> cabeca
                            // generica do inimigo (mesma fonte da coluna/CTB); party -> seu
                            // retrato. Motor = autoridade (le active_actor, nao inventa).
                            ui->set_string("retrato_src",
                                           cockpit_retrato_flat_for(*a).c_str());
                        }
                        // Selecao do menu (motor = autoridade). O foco do glintfx (tab/nav no
                        // RML) e navegacao inerte; a classe .sel VISIVEL segue este indice.
                        const int sel = scene.menu().selected_index();
                        ui->set_number("sel", sel);
                        ui->set_string("verb",
                                       std::string(kVerbLabels[static_cast<std::size_t>(
                                                       sel < 0 || sel > 5 ? 2 : sel)])
                                           .c_str());
                        // Alvo: 1o inimigo VIVO da fila (alvo default das acoes ofensivas,
                        // canon 1B). "-" se nenhum. Le o motor (queue), nao inventa estado.
                        const char* alvo = "-";
                        std::string alvo_buf;
                        for (const auto* act : scene.machine().queue().order()) {
                            if (act != nullptr && !act->is_player_side() && act->is_alive()) {
                                alvo_buf = act->display_name();
                                alvo = alvo_buf.c_str();
                                break;
                            }
                        }
                        ui->set_string("alvo", alvo);
                        // Log VIVO: ultimas linhas narradas pelo motor (data-for "line:log").
                        // As strings precisam VIVER ate o set_list copiar (mantemos o vector).
                        // CAP 2 entradas (as mais recentes) + now-line = EXATAS 3 linhas de
                        // log. Com o RCSS white-space:nowrap+ellipsis do #log .ln cada entrada
                        // ocupa 1 linha fixa (nao quebra mais em 2), entao 2*12dp + now-line =
                        // 36dp e a coluna cabe em 540dp com folga do rodape; now-line visivel.
                        const std::vector<gus::app::screens::LogLine> lines =
                            scene.log_lines(2);
                        std::vector<const char*> ptrs;
                        ptrs.reserve(lines.size());
                        for (const auto& l : lines) ptrs.push_back(l.text.c_str());
                        ui->set_list("log", ptrs.data(), ptrs.size());
                    }
                }
                // HOVER-SELFTEST (injecao): a cada frame, coloca o ponteiro SINTETICO onde a
                // fase pede, ANTES do update() (o Context::Update reaplica o hover chain sob a
                // ultima posicao). Fases 0/3: FORA do cockpit (arena, x=80% da largura) => nenhum
                // pill em hover. Fases 1/2: no CENTRO do pill-alvo. GLINTFX-CLICK: a posicao vem
                // de UiLayer::get_element_box (v0.2.5, geometria REAL do doc carregado) em vez da
                // geometria manual aposentada (mais precisa: o glintfx sabe onde o pill
                // REALMENTE esta, nao uma copia espelhada da RCSS) -- ESTE diagnostico e' o UNICO
                // uso de get_element_box no projeto (o hit-test de CLIQUE usa o id direto via
                // set_click_callback, nunca geometria; ver battle_cockpit_verb_click).
                if (hover_selftest) {
                    float mx = static_cast<float>(pw) * 0.80f;  // fora da coluna (fases 0 e 3)
                    float my = static_cast<float>(ph) * 0.50f;
                    const int hover_pill = hover_phase == 1   ? 0    // SCAN (nao-selecionado)
                                           : hover_phase == 2 ? 2    // ATACAR (selecionado)
                                                              : -1;  // 0 e 3: sem pill
                    if (hover_pill >= 0) {
                        const glintfx::ElementBox box = ui->get_element_box(
                            gus::app::screens::kCockpitVerbElementIds[hover_pill]);
                        if (box.found) {
                            mx = box.x + box.w * 0.5f;
                            my = box.y + box.h * 0.5f;
                        }
                    }
                    glintfx::UiEvent ge{};
                    ge.type = glintfx::UiEvent::Type::MouseMove;
                    ge.x = mx;
                    ge.y = my;
                    ui->process_event(ge);
                }
                ui->update();
                ui->render();  // UI por cima da arena, mesmo contexto GL
            }

            // HOVER-SELFTEST (captura por fase): deixa ~12 frames assentarem (bake do atlas de
            // fonte + hover chain estavel) e salva 1 PNG por fase, lendo o backbuffer ANTES do
            // swap. Depois avanca a fase; encerra apos a 4a. Nao colide com a captura single-shot
            // (GUSWORLD_RMLUI_CAPTURE) porque aquela so dispara com capture_path != nullptr.
            if (hover_selftest) {
                constexpr int kHoverSettleFrames = 12;
                ++hover_phase_frame;
                if (hover_phase_frame >= kHoverSettleFrames) {
                    const char* suffix = hover_phase == 0   ? "_a_none.png"
                                         : hover_phase == 1 ? "_b_hover_unsel.png"
                                         : hover_phase == 2 ? "_c_hover_sel.png"
                                                            : "_d_none_again.png";
                    const std::string out = std::string(hover_selftest_prefix) + suffix;
                    std::vector<unsigned char> buf(
                        static_cast<std::size_t>(pw) * static_cast<std::size_t>(ph) * 4);
                    if (gus::platform::rmlui::gl3_read_backbuffer_rgba(pw, ph, buf.data())) {
                        stbi_write_png(out.c_str(), pw, ph, 4, buf.data(), pw * 4);
                        std::cout << "BattlePreview: [hover-selftest] fase " << hover_phase
                                  << " -> " << out << " (" << pw << "x" << ph << ")\n";
                    } else {
                        std::cerr
                            << "BattlePreview: [hover-selftest] gl3_read_backbuffer falhou\n";
                    }
                    ++hover_phase;
                    hover_phase_frame = 0;
                    if (hover_phase > 3) {
                        running = false;  // 4 fases capturadas; encerra
                    }
                }
            }

            // COCKPIT-SFX-HOVER-CLIQUE (self-test HEADLESS): apos a geometria assentar (~12
            // frames de ui->update, como o hover-selftest visual), varre um MouseMove SINTETICO
            // pelos 6 pills e prova o edge-detect do SOM DE HOVER + o SOM DE CLIQUE, medindo
            // sfx_play_count() (hook do AudioEngine). MESMOS caminhos de codigo do mouse real
            // (handle_cockpit_hover + o callback de clique). Roda 1 vez e encerra.
            if (ui_sfx_selftest && !ui_sfx_done) {
                constexpr int kUiSfxSettleFrames = 12;
                ++ui_sfx_settle_frame;
                if (ui_sfx_settle_frame >= kUiSfxSettleFrames && glintfx_on && ui) {
                    ui_sfx_done = true;

                    // Centro de cada pill via get_element_box (geometria REAL do doc).
                    auto pill_center = [&](int v, float& cx, float& cy) -> bool {
                        const glintfx::ElementBox b = ui->get_element_box(
                            gus::app::screens::kCockpitVerbElementIds[v]);
                        if (!b.found) return false;
                        cx = b.x + b.w * 0.5f;
                        cy = b.y + b.h * 0.5f;
                        return true;
                    };

                    // (1) HOVER: varre os 6 pills 0..5 (6 ENTRADAS novas => 6 plays).
                    const unsigned int hover_base = audio_ptr->sfx_play_count();
                    hovered_verb = -1;  // comeca fora de qualquer pill
                    int swept = 0;
                    for (int v = 0; v < gus::app::screens::kBattleVerbCount; ++v) {
                        float cx = 0.0f, cy = 0.0f;
                        if (!pill_center(v, cx, cy)) continue;
                        handle_cockpit_hover(cx, cy);
                        ++swept;
                    }
                    const unsigned int after_sweep = audio_ptr->sfx_play_count();

                    // (2) REPIQUE: parado no ULTIMO pill 2x seguidas => 0 plays extras.
                    float lx = 0.0f, ly = 0.0f;
                    if (pill_center(gus::app::screens::kBattleVerbCount - 1, lx, ly)) {
                        handle_cockpit_hover(lx, ly);
                        handle_cockpit_hover(lx, ly);
                    }
                    const unsigned int after_still = audio_ptr->sfx_play_count();

                    // (3) SAIR-E-VOLTAR: fora (arena, x=80%) e reentra no pill 0 => +1 play.
                    handle_cockpit_hover(static_cast<float>(pw) * 0.80f,
                                         static_cast<float>(ph) * 0.50f);
                    float z0x = 0.0f, z0y = 0.0f;
                    if (pill_center(0, z0x, z0y)) handle_cockpit_hover(z0x, z0y);
                    const unsigned int after_reenter = audio_ptr->sfx_play_count();

                    // (4) CLIQUE: aciona ATACAR pelo MESMO caminho do callback do glintfx
                    // (battle_cockpit_verb_click + play_sfx do click), => +1 play.
                    const unsigned int click_base = audio_ptr->sfx_play_count();
                    if (scene.waiting_player_input() && !scene.is_aiming()) {
                        if (battle_cockpit_verb_click(
                                scene, gus::app::screens::kCockpitVerbElementIds[static_cast<int>(
                                           BattleVerb::Atacar)])) {
                            audio_ptr->play_sfx(ui_click_sfx_id);
                        }
                    }
                    const unsigned int after_click = audio_ptr->sfx_play_count();

                    std::cout
                        << "BattlePreview: [ui-sfx-selftest] hover: " << swept
                        << " pills varridos, " << (after_sweep - hover_base)
                        << " plays (esperado " << swept << " = N hovers -> N plays)\n"
                        << "  repique (parado 2x no mesmo pill): "
                        << (after_still - after_sweep) << " plays extras (esperado 0)\n"
                        << "  sair-e-voltar ao pill 0: " << (after_reenter - after_still)
                        << " play (esperado 1)\n"
                        << "  clique ATACAR: " << (after_click - click_base)
                        << " play (esperado 1); is_aiming="
                        << (scene.is_aiming() ? "on" : "off") << "\n"
                        << "  device audio="
                        << (audio_ptr->available() ? "disponivel" : "INDISPONIVEL (mudo)")
                        << " sfx_play_count total=" << after_click << "\n";
                    running = false;  // prova concluida; encerra
                }
            }

            // ANIM-SELFTEST (capturas): le o backbuffer ANTES do swap (mesma tecnica do
            // hover-selftest) nos frames-chave da timeline. Loga o OFFSET do atacante
            // (prova textual do desloca-golpeia-volta) junto de cada shot.
            if (anim_selftest) {
                const auto capture = [&](const char* suffix) {
                    const std::string out = std::string(anim_selftest_prefix) + suffix;
                    std::vector<unsigned char> buf(
                        static_cast<std::size_t>(pw) * static_cast<std::size_t>(ph) * 4);
                    if (gus::platform::rmlui::gl3_read_backbuffer_rgba(pw, ph,
                                                                       buf.data())) {
                        stbi_write_png(out.c_str(), pw, ph, 4, buf.data(), pw * 4);
                        const auto aoff = anim_attacker_id.empty()
                                              ? gus::core::spatial::Vec2{}
                                              : scene.anim().offset_for(anim_attacker_id);
                        const auto toff = anim_target_id.empty()
                                              ? gus::core::spatial::Vec2{}
                                              : scene.anim().offset_for(anim_target_id);
                        std::cout << "BattlePreview: [anim-selftest] f" << frame_no
                                  << " -> " << out << " | atacante_off=(" << aoff.x << ","
                                  << aoff.y << ") alvo_off=(" << toff.x << "," << toff.y
                                  << ") projeteis=" << scene.anim().projectiles().size()
                                  << " floaters=" << scene.floaters().size() << "\n";
                    } else {
                        std::cerr << "BattlePreview: [anim-selftest] gl3_read_backbuffer "
                                     "falhou\n";
                    }
                };
                if (frame_no == 40) {
                    capture("_a_cast_travel.png");  // bolinha em voo (meio da viagem)
                } else if (frame_no == 58) {
                    capture("_b_cast_react.png");   // alvo no tranco pos-impacto
                }
                if (anim_f0 > 0) {
                    if (frame_no == anim_f0 + 21) {
                        capture("_c_melee_windup.png");   // meio do dash (deslocado)
                    } else if (frame_no == anim_f0 + 49) {
                        capture("_d_melee_contact.png");  // no alvo + floater + tranco
                    } else if (frame_no == anim_f0 + 87) {
                        capture("_e_melee_rest.png");     // todos de volta ao repouso
                        std::cout << "BattlePreview: [anim-selftest] concluido; "
                                     "encerrando.\n";
                        running = false;
                    }
                }
            }

            // SPRITE-SELFTEST (capturas): backbuffer antes do swap (mesma tecnica dos
            // demais self-tests) nos frames-chave, logando offset do director + clip/
            // frame do sprite do GUS (prova textual: qual animacao esta tocando).
            if (sprite_selftest) {
                const auto capture = [&](const char* suffix) {
                    const std::string out =
                        std::string(sprite_selftest_prefix) + suffix;
                    std::vector<unsigned char> buf(
                        static_cast<std::size_t>(pw) * static_cast<std::size_t>(ph) *
                        4);
                    if (gus::platform::rmlui::gl3_read_backbuffer_rgba(pw, ph,
                                                                       buf.data())) {
                        stbi_write_png(out.c_str(), pw, ph, 4, buf.data(), pw * 4);
                    } else {
                        std::cerr << "BattlePreview: [sprite-selftest] "
                                     "gl3_read_backbuffer falhou\n";
                    }
                    const auto off = scene.anim().offset_for("gus");
                    const auto sf = scene.actor_sprite_frame("gus");
                    std::cout << "BattlePreview: [sprite-selftest] f" << frame_no
                              << " -> " << out << " | gus_off=(" << off.x << ","
                              << off.y << ") kind="
                              << static_cast<int>(scene.anim().kind_for("gus"))
                              << " clip="
                              << (sf.has_value()
                                      ? std::string(gus::app::screens::clip_dir_name(
                                            sf->first))
                                      : std::string("<sem sprite set>"))
                              << " frame=" << (sf.has_value() ? sf->second : -1)
                              << "\n";
                };
                if (frame_no == 8) {
                    capture("_a_idle_rest.png");     // battle_idle no repouso
                }
                if (sprite_f0 > 0) {
                    // Duracoes reais (dt fixo 1/60; +0.5f arredonda p/ evitar truncamento
                    // de float, ex. 0.7*60 vira 41.99...): Approach kPlayerMeleeApproach
                    // Seconds (1.3s ~= 78f), Return kPlayerMeleeReturnSeconds (0.7s ~= 42f),
                    // delay do Beat 2 kPacingStepDelaySeconds (0.8s ~= 48f).
                    const int approach_f = static_cast<int>(
                        gus::app::screens::kPlayerMeleeApproachSeconds * 60.0f + 0.5f);
                    const int return_f = static_cast<int>(
                        gus::app::screens::kPlayerMeleeReturnSeconds * 60.0f + 0.5f);
                    const int rel = frame_no - sprite_f0;
                    // FLIPBOOK do APPROACH inteiro (diagnostico do lider 2026-07-02): captura
                    // DENSA a cada 4 frames de jogo, do inicio do dash (rel 0) ao contato
                    // (rel approach_f). ~20 quadros pra montar o flip e VER se o ciclo de
                    // perna le pra-FRENTE (aliasing curado pela duracao maior) ou reverso
                    // (problema real de ordem/pose). Nomes _flip_NN zero-pad p/ ordenar.
                    if (rel >= 0 && rel <= approach_f && (rel % 4) == 0) {
                        const int fi = rel / 4;
                        const std::string suf = std::string("_flip_") +
                                                (fi < 10 ? "0" : "") +
                                                std::to_string(fi) + ".png";
                        capture(suf.c_str());
                    }
                    // Marcos MACRO (nomes semanticos; rel escolhidos %4==3, nao colidem com
                    // o flip): swing cravado na cauda, meio da volta, repouso final.
                    if (rel == approach_f - 3) {
                        capture("_c_attack_swing.png");  // murro de perfil cravado (<= f5)
                    } else if (rel == approach_f + return_f / 2) {
                        capture("_e_run_back.png");      // run_west no meio da volta
                    } else if (rel == approach_f + return_f + 3) {
                        capture("_d_idle_back.png");     // de volta ao idle no repouso
                        std::cout << "BattlePreview: [sprite-selftest] concluido; "
                                     "encerrando.\n";
                        running = false;
                    }
                }
            }

            // SMOKE VISUAL: captura 1 PNG no frame alvo (ANTES do swap, lendo o backbuffer)
            // e encerra. Em modo interativo, o swap apresenta na janela.
            if (capture_path != nullptr && frame_no + 1 >= capture_at_frame) {
                std::vector<unsigned char> buf(
                    static_cast<std::size_t>(pw) * static_cast<std::size_t>(ph) * 4);
                if (gus::platform::rmlui::gl3_read_backbuffer_rgba(pw, ph, buf.data())) {
                    stbi_write_png(capture_path, pw, ph, 4, buf.data(), pw * 4);
                    std::cout << "BattlePreview: [capture] PNG salvo em " << capture_path
                              << " (" << pw << "x" << ph << ")\n";
                } else {
                    std::cerr << "BattlePreview: [capture] gl3_read_backbuffer falhou\n";
                }
                running = false;
            }

            SDL_GL_SwapWindow(window);  // swap unico (arena + HUD compostos)
            ++frame_no;

            // DIAGNOSTICO: sai apos N frames (exercita o loop completo sob ASan/UBSan).
            if (max_frames > 0 && frame_no >= max_frames) {
                std::cout << "BattlePreview: [frames] limite de " << max_frames
                          << " frames atingido, encerrando.\n";
                running = false;
            }
        }

        // M7-COSTURA Inc 2 (ADR-012 decisao 5): FADE-OUT de SAIDA - a tela ESCURECE
        // (overlay preto 0->1) sobre o ULTIMO ESTADO congelado da arena+HUD (sem avancar
        // logica de jogo, so re-desenha o mesmo frame com alpha crescente). SO roda
        // quando quem chamou pediu (fade_out_seconds>0 - a Maestro pede; o --battle
        // standalone e todo selftest/captura pedem 0, ver os defaults do header) E o
        // motivo da saida NAO foi o jogador fechando a JANELA (quit_requested) - fechar
        // deve ser IMEDIATO, sem segurar o jogador pra um fade que ele nao pediu. Pump de
        // eventos LIMITADO a SDL_EVENT_QUIT (se o jogador fechar DURANTE o proprio fade,
        // interrompe o fade e marca quit_requested - o choke-point abaixo ja usa a
        // variavel atualizada).
        if (fade_out_seconds > 0.0f && !quit_requested) {
            const unsigned long long fade_start_ns = SDL_GetTicksNS();
            bool fading = true;
            while (fading) {
                SDL_Event fev;
                while (SDL_PollEvent(&fev)) {
                    if (fev.type == SDL_EVENT_QUIT) {
                        quit_requested = true;
                    }
                }
                if (quit_requested) {
                    break;
                }
                int fpw = kWindowW, fph = kWindowH;
                SDL_GetWindowSizeInPixels(window, &fpw, &fph);
                if (fpw < 1 || fph < 1) {
                    SDL_Delay(16);
                    continue;
                }
                const float elapsed =
                    static_cast<float>(SDL_GetTicksNS() - fade_start_ns) / 1.0e9f;
                scene.render(renderer, static_cast<float>(fpw), static_cast<float>(fph));
                if (glintfx_on && ui) {
                    if (fpw != pw0 || fph != ph0) {
                        ui->set_viewport(fpw, fph);
                        if (glintfx_dp_override <= 0.0f) {
                            ui->set_dp_ratio(static_cast<float>(fpw) / 960.0f);
                        }
                        pw0 = fpw;
                        ph0 = fph;
                    }
                    ui->update();
                    ui->render();
                }
                // M7-COSTURA Inc 2c: sequencia de frames do boot pixelizado - esta E
                // a metade "Darkening voltando pra cidade" da transicao inteira (a
                // arena escurecendo; continua REVERSO de onde a proxima metade - a
                // cidade revelando - vai pegar, ver o header extenso em gus/core/anim/
                // boot_pixel_sequence.hpp) no lugar do glitch procedural (aposentado -
                // o lider VETOU o visual, "pareceu bug"). `t` = elapsed/
                // fade_out_seconds, a MESMA fracao que fade_overlay_alpha usava
                // internamente pra kOut (ver gus/app/boot_pixel_overlay.hpp).
                {
                    const float t = fade_out_seconds > 0.0f
                                        ? std::min(1.0f, elapsed / fade_out_seconds)
                                        : 1.0f;
                    boot_overlay.draw(renderer, gus::app::screens::battle_screen_rect(),
                                      gus::core::anim::BootPixelLeg::kFromBattleDarkening,
                                      t);
                }
                SDL_GL_SwapWindow(window);
                fading = elapsed < fade_out_seconds;
            }
            std::cout << "BattlePreview: [fade] saida (kOut) concluido em "
                      << fade_out_seconds << "s.\n";
        }

        // M7-COSTURA (ADR-012 Onda 1): MESMO choke-point (comentario original abaixo) -
        // grava o CombatOutcome final pra Maestro decidir vitoria/derrota/fuga. Se a
        // janela foi fechada no meio do combate (Esc/fechar), a FSM nunca chegou a
        // CombatEnd e outcome() ainda e Ongoing - a Maestro trata isso como "abortou",
        // nao como derrota (ver maestro.cpp).
        if (out_outcome != nullptr) {
            *out_outcome = scene.machine().outcome();
        }
        // FIX BUG-3: grava se o motivo da saida foi ESPECIFICAMENTE o jogador fechando a
        // janela (distinto de qualquer CombatOutcome - ver o handler de SDL_EVENT_QUIT
        // acima e o comentario no header). Default false (nullptr-safe).
        if (out_quit_requested != nullptr) {
            *out_quit_requested = quit_requested;
        }

        // MUSICA: fade-out ao SAIR da batalha (M6 F4, ADR-011) - SO no modo LOCAL
        // (standalone); no modo EXTERNO (Maestro) a musica e 100% dela (crossfade
        // cronometrado com o fade preto, disparado por FORA desta funcao - ver
        // gus/app/maestro_logic.hpp::crossfade_music). O CROSSFADE completo tela-a-tela
        // (M7-COSTURA Inc 2) fecha o M6 - ver ADR-012 e o relatorio do Inc 2.
        if (external_audio == nullptr) {
            const bool music_was_playing_before_exit = audio_ptr->music_is_playing();
            constexpr float kMusicFadeOutSeconds = 1.5f;
            audio_ptr->stop_music(kMusicFadeOutSeconds);
            std::cout << "BattlePreview: [audio] musica: play_music count="
                      << audio_ptr->music_play_count() << " estava_tocando_ao_sair="
                      << (music_was_playing_before_exit ? "sim" : "nao") << " fade-out de "
                      << kMusicFadeOutSeconds << "s disparado.\n";
        }

        // DIAGNOSTICO (M6 F3, ADR-011): quantos SFX de hit TOCARAM de fato nesta sessao -
        // prova rapida no console (sem precisar ouvir) de que o gancho disparou no evento
        // de contato certo, tanto em playtest manual quanto sob os selftests acima.
        std::cout << "BattlePreview: [audio] play_sfx(hit) disparou "
                  << audio_ptr->sfx_play_count() << "x nesta sessao.\n";
    }  // Render2dGl3 destruido (libera recursos GL) antes de destruir o contexto
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
                                 float fade_in_seconds, float fade_out_seconds) {
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
        std::cerr << "BattlePreview: SDL_GL_CreateContext falhou: " << SDL_GetError()
                  << "\n";
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
                                            fade_out_seconds);

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
