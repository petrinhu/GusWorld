// SPDX-License-Identifier: Apache-2.0
// gus/app/screens/system_menu_loop.hpp
//
// LOOP INTERATIVO do MENU DE SISTEMA (pausa + config de som), MENU-PAUSA-CONFIG-SOM
// (M7-COSTURA, INTEGRACAO FINAL). Roda a maquina de estado pura de system_menu.hpp +
// o RML/RCSS de system_menu_rml.hpp numa glintfx::UiLayer PROPRIA (independente de
// qualquer HUD do chamador), num CONTEXTO GL. Aplica volume no AudioEngine em tempo
// real e PERSISTE em settings.json a cada mudanca (fundacao ja pronta: gus/platform/
// fs/settings_file_store.hpp).
//
// UM UNICO JEITO DE ENTRAR (FLASH-CTX, contexto GL unico do boot ao shutdown):
//   run_system_menu_loop_gl_current: ASSUME um contexto GL JA CORRENTE e com os
//     ponteiros de funcao (glad) JA CARREGADOS (o chamador e dono - cria/destroi por
//     fora). Uso: BATALHA (a battle_preview ja roda dentro do MESMO contexto GL -
//     Esc na pilha vazia do combate abre este loop ANINHADO, sem sair do contexto)
//     e CIDADE (a Maestro tambem desenha DIRETO no contexto GL unico - ver o
//     comentario FLASH-CTX em maestro.cpp). Existiu uma 2a forma
//     (run_system_menu_loop_owning_gl, que criava/destruia um contexto GL PROPRIO
//     pra CIDADE - Render2dSdl puro, exigindo city_->release_renderer()/
//     reacquire_renderer() por fora) - DECOMMISSIONADA por ser codigo morto desde
//     FLASH-CTX (M9-CAMADAS-SDL Fatia 0, zero call-site confirmado,
//     docs/tech/plano-camadas-sdl.md).
//
// FUNDO (retoque do lider via AskUserQuestion, M7-DIALOGO/MENU-PAUSA-CONFIG-SOM):
// quando o CHAMADOR fornece `frozen_background_png` (PNG de 1 frame, capturado por
// SdlWindow::capture_frame_to_png ANTES de abrir o menu - ver Maestro::open_pause_
// from_city), o loop desenha essa CENA REAL da cidade CONGELADA como fundo
// estatico, cobrindo a vinheta - mesmo padrao de Chrono Trigger/Zelda/Stardew
// Valley (o mundo "pausa" visualmente atras da UI). `frozen_background_png` vazio
// (default, uso de dentro da BATALHA hoje inexistente na producao - ver
// battle_preview.cpp) degrada pro fundo ABSTRATO de sempre: Render2dGl3::
// begin_frame (clear + vinheta radial, MESMA ambientacao usada na arena).
//
// SAVE-LOAD-UI etapa 6 (wiring REAL): "Salvar"/"Carregar" confirmados no Pause
// (SystemMenuAction::OpenSaveLoadSave/OpenSaveLoadLoad) abrem a tela REAL de
// save/load (gus/app/screens/save_load_menu_loop.hpp), ANINHADA no MESMO
// contexto GL (MESMA tecnica do proprio menu de sistema quando aninhado dentro
// da batalha). `saves_dir` + os 2 callbacks abaixo sao repassados direto pra
// essa tela - ela e quem faz o I/O de disco de fato; este loop so intercepta a
// action e delega.
//
// F4-1b.4 (onda F4 "casca SDL -> App mode do glintfx", fatia 1b.4 - SEGUNDA
// tela PAI da onda, apos F4-1b.3/title_menu_loop.cpp): o caminho de PRODUCAO
// (run_system_menu_loop_gl_current, chamado pela batalha E pela Maestro - as
// duas via a MESMA variante gl_current desde FLASH-CTX) foi convertido de um
// while(true){SDL_PollEvent...} PROPRIO pra uma classe SystemMenuScreen (gus::
// app::ScreenState) + gus::app::run_screen_state - MESMA tecnica de
// TitleScreen/DifficultyScreen/SaveLoadScreen. O roteamento de evento (decisao
// PURA) foi extraido pra system_screen_step (testada headless em
// system_screen_step_test.cpp).
//
// O PROBLEMA NOVO desta fatia (MESMO racional de F4-1b.3): o Pause abre a tela
// de SALVAR/CARREGAR (save_load_menu_loop.hpp) ANINHADA quando o jogador
// confirma "Salvar"/"Carregar" - ANTES desta fatia isso acontecia de DENTRO do
// handler do Pause (o antigo handle_action lambda, chamando
// run_save_load_menu_loop_gl_current no MEIO do proprio corpo) - o que
// VIOLARIA a guarda de reentrada de gus::app::run_screen_state()
// (gus/app/screen_state.hpp:29-39). A partir desta fatia,
// SystemMenuScreen::handle_event() SO devolve um SystemMenuScreenExit (ver
// abaixo) - QUEM decide abrir a tela de save/load e o MINI-DRIVER dentro do
// WRAPPER (run_system_menu_loop_gl_current, ver o .cpp): ele chama
// run_screen_state(system_screen) ATE ela terminar (system_screen.exit() ja
// rodou, a UiLayer do Pause ja foi destruida - SO ENTAO e seguro abrir a de
// save/load), decide o proximo passo via a funcao PURA pause_flow_next(), e
// RE-USA o MESMO objeto SystemMenuScreen quando a tela de save/load devolve
// BackToPause (o estado - tela atual/selecoes/copia de trabalho de Controles -
// persiste porque system_menu_open()/seed de volume/load de controls.json
// rodam UMA VEZ no CONSTRUTOR de SystemMenuScreen, NUNCA em enter() - MESMO
// racional de TitleScreen).
//
// Cross-ref: gus/app/screens/system_menu.hpp (a maquina de estado pura);
//            gus/app/screens/system_menu_rml.hpp (o RML/RCSS gerado do estado);
//            gus/app/screens/save_load_menu_loop.hpp (a tela REAL de save/load,
//            SAVE-LOAD-UI etapa 6, MESMO template ScreenState, F4-1b.2);
//            gus/app/screens/title_menu_loop.hpp (o TEMPLATE do MINI-DRIVER,
//            F4-1b.3); gus/app/screens/battle_preview.cpp
//            (run_battle_preview_embedded, o chamador da variante gl_current -
//            Esc na pilha vazia do combate); gus/app/maestro.cpp (tambem
//            chamador da variante gl_current - Esc na cidade, FLASH-CTX);
//            gus/platform/fs/settings_file_store.hpp (I/O real do
//            settings.json).

#ifndef GUS_APP_SCREENS_SYSTEM_MENU_LOOP_HPP
#define GUS_APP_SCREENS_SYSTEM_MENU_LOOP_HPP

#include <array>
#include <functional>
#include <optional>
#include <string>

#include <SDL3/SDL.h>
#include <glintfx/element_box.hpp>  // F4-1b.4: glintfx::ElementBox, INPUT de system_screen_step

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screen_state.hpp"  // F4-1b.4: gus::app::EventSyncHook (MESMO contrato da onda F4)
#include "gus/app/screens/save_load_menu_loop.hpp"  // SaveLoadLoopExit (pause_flow_next, driver PURO)
#include "gus/app/screens/system_menu.hpp"  // SystemMenuState/SystemMenuAction/contadores de item
#include "gus/domain/save/save_data.hpp"
#include "gus/platform/audio/audio_engine.hpp"

namespace gus::app::screens {

// Desfecho do loop: o chamador decide o que fazer a seguir (fechar a janela real ou
// so retomar a cena de onde parou).
struct SystemMenuLoopOutcome {
    // true = o jogador confirmou "Sair" no menu OU fechou a janela (SDL_EVENT_QUIT)
    // enquanto o menu estava aberto. Sinal DISTINTO de "Continuar" (que devolve
    // false - o chamador so retoma a cena, sem encerrar nada). Mesmo espirito do
    // quit_requested de run_battle_preview_embedded.
    bool quit_app = false;

    // MENU-INICIAL: true = o jogador confirmou "Sim" no mini-dialogo "voltar ao
    // menu inicial?" (SystemMenuAction::RequestToTitle) - o CHAMADOR (Maestro::
    // open_pause_from_city) deve chamar show_title_screen() em vez de encerrar
    // o processo. Mutuamente exclusivo com quit_app (so 1 dos dois fica true por
    // chamada - RequestToTitle e RequestQuit nunca coexistem no mesmo desfecho).
    bool to_title = false;
};

// F4-1b.4: desfecho de UMA rodada do PROPRIO SystemMenuScreen - diferente de
// SystemMenuLoopOutcome (desfecho do WRAPPER inteiro, que pode incluir a tela
// de save/load aninhada), SystemMenuScreenExit e so o que o Pause/arvore de
// config decidiu, ANTES do MINI-DRIVER (no .cpp) decidir o proximo passo.
enum class SystemMenuScreenExit {
    Continue,          // "Continuar" confirmado OU ESC na raiz (Pause) - o
                       // DRIVER fecha o menu (outcome default, quit_app=false).
    RequestQuit,        // "Sair" confirmado - o DRIVER encerra o programa.
    RequestToTitle,      // "Sim" confirmado no mini-dialogo de Menu Inicial - o
                       // DRIVER volta pra tela de titulo (outcome.to_title=true).
    OpenSaveLoadSave,    // "Salvar" confirmado no Pause - o DRIVER deve abrir a
                       // tela de SALVAR/CARREGAR (modo Save) ANINHADA logo em
                       // seguida (run_screen_state(system_screen) ja retornou,
                       // SystemMenuScreen::exit() ja rodou - seguro criar a 2a
                       // UiLayer agora).
    OpenSaveLoadLoad,    // "Carregar" confirmado - idem, modo Load.
};

// SFX que pode resultar de UM system_screen_step(): so o HOVER de navegacao por
// TECLADO precisa de um kind PROPRIO aqui (SOM DE HOVER PARIDADE TECLADO x
// MOUSE, ver system_menu.hpp::system_menu_hover_entered_new_item) - o SOM DE
// CLIQUE desta tela SEMPRE vive dentro do EFEITO DE PRESS (ver `flash` em
// SystemMenuStepResult abaixo: o CHAMADOR - SystemMenuScreen::flash_pressed_ -
// toca o click_sfx como PRIMEIRA coisa do proprio flash, MESMO choke-point do
// while(true) antigo) - nao ha um SystemMenuSfxKind::Click separado (ao
// contrario de TitleSfxKind/DifficultySfxKind) porque esta tela nunca toca
// clique SEM flash.
enum class SystemMenuSfxKind {
    None,
    Hover,
};

// Caixas de hit-test resolvidas pelo CHAMADOR (SystemMenuScreen::
// collect_click_boxes_, system_menu_loop.cpp) via glintfx::UiLayer::
// get_element_box - SO as do ramo/estado ATUAL de SystemMenuState sao
// preenchidas (found=true); as demais ficam no default (found=false, MESMO
// "nao resolvida neste frame" de TitleStepResult/DifficultyStepResult) -
// system_screen_step le `state.screen`/`state.pause_confirming_to_title`/
// `state.controls_capturing`/`state.controls_confirming_restore`/
// `state.controls_confirming_discard` (MESMA leitura que o CHAMADOR faz pra
// escolher QUAIS ids resolver antes de chamar) pra decidir qual sub-array
// consultar. `controls_items` chega JA FILTRADO (BUG-A, ver system_menu.hpp::
// controls_row_visible_in_list) - o CALLER e quem consulta a caixa de
// `ctrl-list` e aplica o filtro ANTES de preencher este array (esta funcao
// nunca faz essa query GL-heavy).
struct SystemMenuStepBoxes {
    std::array<glintfx::ElementBox, kPauseItemCount> pause_items{};
    std::array<glintfx::ElementBox, 2> pause_totitle_confirm{};  // "pause-totitle-confirm-<0|1>"
    std::array<glintfx::ElementBox, kConfigCategoriesItemCount> category_items{};
    std::array<glintfx::ElementBox, 2> audio_tracks{};  // "slider-track-<0|1>" (Musica/SFX)
    std::array<glintfx::ElementBox, kAudioItemCount> audio_items{};  // campo/rotulo + Voltar
    std::array<glintfx::ElementBox, kControlsItemCount> controls_items{};  // JA FILTRADO pelo CALLER
    std::array<glintfx::ElementBox, 2> controls_confirm{};          // restaurar-padrao
    std::array<glintfx::ElementBox, 2> controls_discard_confirm{};  // descartar alteracoes
    glintfx::ElementBox placeholder_back;  // "placeholder-back" (Video/Language)
};

// Info do EFEITO DE PRESS (flash, ver o comentario grande no topo de
// system_menu_loop.cpp): o CHAMADOR (SystemMenuScreen::flash_pressed_) precisa
// do ESTADO ANTES da mutacao (pra renderizar a tela DE ORIGEM com o item
// pressed) + o indice do item pressionado - MESMO par que o while(true) antigo
// passava pra flash_pressed(pre_action_state, item_index). Copia de
// SystemMenuState (nao GL) - pura o bastante pra viver aqui.
struct SystemMenuFlashInfo {
    SystemMenuState pre_action_state;
    int item_index = -1;
};

// Resultado de UM system_screen_step(): o que o CHAMADOR (SystemMenuScreen::
// handle_event, system_menu_loop.cpp) deve fazer a seguir - TODO o efeito de
// mundo REAL (audio/GL/I-O em disco: apply_and_persist/persist_controls/
// flash_pressed/abrir a tela de save-load) fica FORA desta struct/funcao, que
// so decide.
struct SystemMenuStepResult {
    bool window_closed = false;  // SDL_EVENT_QUIT
    bool resize = false;         // WINDOW_RESIZED/PIXEL_SIZE_CHANGED - o estado
                                  // da tela NAO muda; o CHAMADOR reconsulta o
                                  // tamanho real da janela e reposiciona a UiLayer.
    bool mouse_move = false;     // MOUSE_MOTION - o CHAMADOR encaminha
                                  // (mouse_x,mouse_y) pro glintfx::UiEvent::
                                  // MouseMove (hover NATIVO) + continua o
                                  // arrasto de slider se houver (FORA do
                                  // escopo desta funcao - exige get_element_box
                                  // do track, uma query GL).
    float mouse_x = 0.0f;
    float mouse_y = 0.0f;
    bool reload = false;   // o CHAMADOR reescreve o RML - o estado pode ter
                           // mudado (navegacao/troca de tela/volume/remap) mesmo
                           // sem `exit` preenchido.
    bool volume_changed = false;    // music_volume OU sfx_volume mudou (tecla
                                    // LEFT/RIGHT OU clique no track do slider) -
                                    // o CHAMADOR deve aplicar no AudioEngine e
                                    // persistir settings.json (apply_and_persist_).
    bool controls_applied = false;  // "Aplicar" confirmado - controls_config ja
                                    // foi promovido a baseline (efeito em
                                    // MEMORIA, feito por esta funcao); o
                                    // CHAMADOR deve persistir em controls.json
                                    // (persist_controls_).
    std::optional<int> start_drag_item;  // clique no track do slider (0=Musica,
                                         // 1=Sfx) - o CHAMADOR deve setar o
                                         // MEMBRO drag_item_ pra este valor (o
                                         // arrasto CONTINUA em handle_mouse_
                                         // motion_, fora do escopo desta funcao).
    bool end_drag = false;               // MOUSE_BUTTON_UP LEFT - o CHAMADOR
                                         // deve setar drag_item_ = -1.
    SystemMenuSfxKind sfx = SystemMenuSfxKind::None;
    std::optional<SystemMenuFlashInfo> flash;  // EFEITO DE PRESS pendente - o
                                               // CHAMADOR chama flash_pressed_
                                               // (toca o click_sfx + renderiza
                                               // alguns frames com o item
                                               // ".pressed") ANTES de aplicar
                                               // volume_changed/controls_applied/
                                               // exit (MESMA ordem do while(true)
                                               // antigo).
    std::optional<SystemMenuScreenExit> exit;  // Continue/RequestQuit/
                                               // RequestToTitle/OpenSaveLoadSave/
                                               // OpenSaveLoadLoad - o CHAMADOR
                                               // encerra esta rodada (done_=true).
};

// O NUCLEO PURO do roteamento de evento do Pause/arvore de configuracoes -
// extraido behavior-preserving do while(true) antigo (ver o .cpp), 100%
// testavel headless (SEM SDL_Init/janela/glintfx::UiLayer real - ver
// system_screen_step_test.cpp). PONTOS CRITICOS preservados EXATAMENTE (ver o
// comentario grande no topo do .cpp): (1) a interceptacao de captura de tecla
// da tela Controles (state.controls_capturing) roda ANTES de qualquer
// roteamento generico de teclado; (2) o clique no track do slider (Audio) e
// verificado ANTES do campo/rotulo (o mais especifico vence).
[[nodiscard]] SystemMenuStepResult system_screen_step(SystemMenuState& state, const SDL_Event& ev,
                                                        const SystemMenuStepBoxes& boxes) noexcept;

// F4-1b.4: desfecho do MINI-DRIVER (dentro do wrapper run_system_menu_loop_gl_
// current, ver o comentario grande no .cpp) apos UMA iteracao - consome o
// desfecho do Pause (`exit`) e, SE ele pediu Salvar/Carregar, o desfecho da
// tela de save/load aninhada que o driver acabou de rodar (`saveload_exit` -
// IRRELEVANTE/ignorado quando `exit` nao e OpenSaveLoadSave/OpenSaveLoadLoad;
// o driver passa um valor qualquer nesses casos - os testes de pause_flow_next
// provam EXPLICITAMENTE que o valor nao importa fora desses 2 casos).
enum class SystemMenuFlowStep {
    Continue,       // o driver devolve SystemMenuLoopOutcome{} (default,
                    // quit_app=false/to_title=false) na hora.
    RequestQuit,    // o driver devolve outcome.quit_app=true.
    RequestToTitle,  // o driver devolve outcome.to_title=true.
    RetryPause,     // o driver RE-ENTRA no MESMO objeto SystemMenuScreen -
                    // MESMO comportamento do antigo ui_opt.reset()/emplace()
                    // manual, agora consequencia ESTRUTURAL de
                    // SystemMenuScreen::exit() ja ter rodado antes desta
                    // decisao (ver o comentario grande no topo do .cpp).
};

[[nodiscard]] SystemMenuFlowStep pause_flow_next(SystemMenuScreenExit exit,
                                                   SaveLoadLoopExit saveload_exit) noexcept;

// Variante que ASSUME um contexto GL corrente (ver header). Seed inicial do volume
// = audio.music_volume()/audio.sfx_volume() (o estado em MEMORIA do AudioEngine,
// ja carregado do settings.json no boot pela Maestro - ver maestro.cpp::init()).
// A cada SystemMenuAction::VolumeChanged (teclado LEFT/RIGHT) OU arrasto de mouse
// no track do slider, aplica IMEDIATAMENTE em `audio` (set_music_volume/
// set_sfx_volume) e persiste em settings_dir via save_system_settings - o ouvinte
// escuta a mudanca em tempo real, sem esperar fechar o menu. `translator` resolve
// os rotulos i18n (o chamador mantem vivo; nao-dono, mesmo padrao de
// BattleScene::set_translator). `frozen_background_png` (ver o header, NOVO):
// caminho de um PNG de 1 frame capturado pelo chamador (ex.: cidade CONGELADA) pra
// desenhar como fundo estatico; vazio (default) = fundo abstrato de sempre (a
// vinheta radial). Sai (devolve) quando o jogador confirma Continuar/ESC na tela
// Pause (quit_app=false), confirma Sair (quit_app=true), ou fecha a janela
// (quit_app=true).
// `saves_dir` (SAVE-LOAD-UI etapa 6): diretorio real dos saves (gus::platform::
// fs::resolve_saves_dir(), MESMA convencao de injecao de settings_dir acima -
// testabilidade/override). `build_current_save_data`/`apply_loaded_save_data`
// (default vazio = "sem capacidade de save/load" nesta entrada - hoje so a
// Maestro na CIDADE fornece os 2; a entrada futura de dentro da BATALHA, se
// vier a existir, pode legitimamente deixar vazio e "Salvar"/"Carregar" viram
// no-op degradado, NUNCA fingem persistir): repassados direto pra
// run_save_load_menu_loop_gl_current (ver seu header pro contrato de cada um).
// `sync_hook` (F4-1b.4, opcional - nullptr = nao sincroniza nada, MESMO
// comportamento de antes desta fatia): repassado direto pro gus::app::
// run_screen_state() do MINI-DRIVER, a CADA iteracao (inclusive quando o
// driver re-entra o Pause apos um BackToPause da tela de save/load) - MESMO
// papel/contrato de title_menu_loop.hpp. NAO repassado pra
// run_save_load_menu_loop_gl_current (a tela de save/load aninhada continua
// sem sync_hook de producao, MESMO estado de antes desta fatia).
// `apply_controls_config` (aviso #2, SAVE-LOAD-AVISOS - decisao do lider
// 2026-08-06/07, default nullptr = "sem capacidade de trocar controles ao
// vivo", MESMA convencao de build_current_save_data/apply_loaded_save_data
// vazios acima): repassado direto pra run_save_load_menu_loop_gl_current
// (junto de `settings_dir`, ja recebido acima e agora TAMBEM repassado pra
// la) - ver o header de save_load_menu_loop.hpp pro contrato completo.
[[nodiscard]] SystemMenuLoopOutcome run_system_menu_loop_gl_current(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator, const std::string& settings_dir,
    const std::string& saves_dir,
    const std::function<gus::domain::save::SaveData()>& build_current_save_data =
        {},
    const std::function<void(const gus::domain::save::SaveData&)>&
        apply_loaded_save_data = {},
    const std::string& frozen_background_png = std::string(),
    const gus::app::EventSyncHook& sync_hook = nullptr,
    const std::function<void(const gus::domain::input::InputRemapConfig&)>&
        apply_controls_config = nullptr);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_SYSTEM_MENU_LOOP_HPP
