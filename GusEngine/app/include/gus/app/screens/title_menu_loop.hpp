// gus/app/screens/title_menu_loop.hpp
//
// LOOP INTERATIVO da TELA DE TITULO (SAVE-LOAD-UI etapa 4, wiring REAL do boot).
// Roda a maquina de estado pura de title_menu.hpp + o RML/RCSS de
// title_menu_rml.hpp numa glintfx::UiLayer PROPRIA, num CONTEXTO GL DONO (owning -
// MESMA receita de run_system_menu_loop_owning_gl/run_battle_preview_embedded): o
// UNICO chamador de producao (Maestro::show_title_screen, chamado no INICIO do
// boot, antes do loop cidade<->batalha) ja fez city_->release_renderer() ANTES e
// faz city_->reacquire_renderer() DEPOIS - a janela fica livre pro contexto GL
// enquanto esta tela roda.
//
// ESTE ARQUIVO E O UNICO PONTO QUE FAZ I/O REAL DE DISCO desta tela (gus::
// platform::fs::has_save/load_game, JA EXISTENTES desde M2-SAVE-IO): varre TODOS
// os slots pra decidir any_save_exists (Continuar habilitado ou nao) e achar
// most_recent_occupied_slot (save_load_menu.hpp, SAVE-LOAD-UI etapa 4) - a tela
// PURA (title_menu.hpp) nunca toca disco. Um save PRESENTE mas nao Ok
// (adulterado/corrompido/versao incompativel) degrada, POR ORA, como slot vazio -
// MESMA politica (e MESMO racional) de build_previews_and_cache em
// save_load_menu_loop.cpp (os avisos dedicados sao etapa futura, fora do escopo
// desta dispatch).
//
// F4-1b.3 (onda F4 "casca SDL -> App mode do glintfx", fatia 1b.3 - PRIMEIRA
// tela PAI da onda, apos F4-1b.1/difficulty_menu_loop.cpp e F4-1b.2/
// save_load_menu_loop.cpp): o caminho de PRODUCAO (run_title_menu_loop_gl_
// current, chamado DIRETO por Maestro::show_title_screen - ver o comentario
// FLASH-CTX la) foi convertido de um while(true){SDL_PollEvent...} PROPRIO pra
// uma classe TitleScreen (gus::app::ScreenState) + gus::app::run_screen_state,
// MESMA tecnica de DifficultyScreen/SaveLoadScreen - PORTO, INTRODUZ o padrao
// do MINI-DRIVER (novo nesta fatia): o titulo abre a tela de SELECAO DE
// DIFICULDADE (gus/app/screens/difficulty_menu_loop.hpp) ANINHADA quando o
// jogador confirma "Novo Jogo" - antes desta fatia isso acontecia de DENTRO do
// handler da tela de titulo (start_new_game_via_difficulty_menu, chamando
// run_difficulty_menu_loop_gl_current no MEIO do proprio route_title_action) -
// o que VIOLARIA a guarda de reentrada de gus::app::run_screen_state()
// (gus/app/screen_state.hpp:29-39: run_screen_state(pai) precisa RETORNAR
// antes de qualquer codigo chamar run_screen_state(filha)). A partir desta
// fatia, TitleScreen::handle_event() SO devolve um TitleScreenExit (ver
// abaixo) - QUEM decide abrir a tela de dificuldade e o MINI-DRIVER dentro do
// WRAPPER (run_title_menu_loop_gl_current, ver o .cpp): ele chama
// run_screen_state(title_screen) ATE ela terminar (title_screen.exit() ja
// rodou, a UiLayer do titulo ja foi destruida - SO ENTAO e seguro abrir a da
// dificuldade), decide o proximo passo via a funcao PURA title_flow_next(), e
// RE-USA o MESMO objeto TitleScreen se o jogador Cancelar na dificuldade (o
// estado - foco/scan de saves - persiste porque title_menu_open()/scan_saves()
// rodam UMA VEZ no CONSTRUTOR de TitleScreen, NUNCA em enter() - ver o
// comentario la).
//
// Cross-ref: gus/app/screens/title_menu.hpp (estado/navegacao PURA);
//            gus/app/screens/title_menu_rml.hpp (RML/RCSS data-driven);
//            gus/app/screens/save_load_menu.hpp (SaveSlotPreview/
//            most_recent_occupied_slot, REUSADOS sem alteracao);
//            gus/app/screens/difficulty_menu_loop.hpp (tela ANINHADA, MESMO
//            template ScreenState, F4-1b.1);
//            gus/platform/fs/save_file_store.hpp (has_save/load_game reais);
//            gus/app/maestro.cpp (Maestro::show_title_screen, o UNICO chamador de
//            producao).

#ifndef GUS_APP_SCREENS_TITLE_MENU_LOOP_HPP
#define GUS_APP_SCREENS_TITLE_MENU_LOOP_HPP

#include <optional>
#include <string>

#include <SDL3/SDL.h>
#include <glintfx/element_box.hpp>  // F4-1b.3: glintfx::ElementBox, INPUT de title_screen_step

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screen_state.hpp"  // F4-1b.3: gus::app::EventSyncHook (MESMO contrato de F4-1a/F4-1b)
#include "gus/app/screens/difficulty_menu_loop.hpp"  // DifficultyLoopExit (title_flow_next, driver PURO)
#include "gus/app/screens/title_menu.hpp"  // TitleMenuState/kTitleItemCount
#include "gus/domain/save/save_data.hpp"
#include "gus/platform/audio/audio_engine.hpp"  // COCKPIT-SFX-HOVER-CLIQUE: paridade sonora

namespace gus::app::screens {

// Desfecho do loop: o que o CHAMADOR (Maestro) deve fazer a seguir.
enum class TitleLoopExit {
    ContinueGame,  // "Continuar" confirmado - `*out_loaded_save` foi preenchido
                   // com o save mais recente (most_recent_occupied_slot entre
                   // TODOS os slots ocupados) - o CHAMADOR aplica no jogo VIVO.
    NewGame,       // "Novo Jogo" confirmado (direto, sem save nenhum; OU via "Sim"
                   // no mini-dialogo) - o CHAMADOR comeca do estado FRESCO que
                   // Maestro::init() ja deixou pronto (sem retrabalho - nao ha
                   // "sessao anterior" carregada ainda nesta fatia).
    QuitApp,       // "Sair" confirmado OU o jogador fechou a JANELA durante a tela
                   // - o chamador encerra o programa SEM entrar no loop de jogo
                   // (nada foi jogado ainda - nenhum autosave faz sentido aqui).
};

// F4-1b.3: desfecho de UMA rodada do PROPRIO TitleScreen - diferente de
// TitleLoopExit (desfecho do WRAPPER inteiro, que pode incluir a tela de
// dificuldade aninhada), TitleScreenExit e so o que a LISTA/mini-dialogo de
// titulo decidiu, ANTES do MINI-DRIVER (no .cpp) decidir o proximo passo.
enum class TitleScreenExit {
    ContinueGame,      // "Continuar" confirmado - TitleScreen::handle_event ja
                        // tentou preencher *out_loaded_save a partir do cache
                        // de scan_saves(); se o cache estiver vazio (BUG
                        // defensivo, nunca deveria acontecer), o PROPRIO
                        // TitleScreen degrada o `result()` pra
                        // NewGameRequested (ver o .cpp) - o driver nunca ve
                        // essa distincao.
    NewGameRequested,   // "Novo Jogo" confirmado (direto ou via "Sim" no mini-
                        // dialogo) - o DRIVER (no wrapper) deve abrir a tela de
                        // selecao de dificuldade ANINHADA logo em seguida
                        // (run_screen_state(title_screen) ja retornou,
                        // TitleScreen::exit() ja rodou - seguro criar a 2a
                        // UiLayer agora).
    QuitApp,            // "Sair" confirmado - o DRIVER encerra o programa. NAO
                        // confundir com ScreenState::window_closed() (janela
                        // fechada via X/Alt+F4) - os DOIS viram
                        // TitleLoopExit::QuitApp no wrapper, mas por caminhos
                        // DIFERENTES (ver o .cpp).
};

// SFX desta tela - mais simples que DifficultySfxKind (difficulty_menu_loop.hpp):
// nao ha item "bloqueado" aqui - confirmar/clicar num item nao selecionavel (so
// Continuar sem save existente) e no-op TOTAL, inclusive de som, MESMA
// semantica do while(true) antigo.
enum class TitleSfxKind {
    None,
    Hover,
    Click,
};

// Resultado de UM title_screen_step(): o que o CHAMADOR (TitleScreen::
// handle_event, title_menu_loop.cpp) deve fazer a seguir - MESMO espirito de
// DifficultyStepResult (difficulty_menu_loop.hpp); TODOS os side effects REAIS
// (SDL/GL/audio/disco) ficam FORA desta struct/funcao, que so decide.
struct TitleStepResult {
    bool window_closed = false;  // SDL_EVENT_QUIT
    bool resize = false;         // WINDOW_RESIZED/PIXEL_SIZE_CHANGED - o estado
                                  // da tela (TitleMenuState) NAO muda; o
                                  // CHAMADOR reconsulta o tamanho real da
                                  // janela e reposiciona a UiLayer.
    bool mouse_move = false;      // MOUSE_MOTION - o CHAMADOR encaminha
                                  // (mouse_x,mouse_y) pro glintfx::UiEvent::
                                  // MouseMove (hover NATIVO); o SOM de hover de
                                  // mouse continua no callback nativo, FORA do
                                  // escopo desta funcao.
    float mouse_x = 0.0f;
    float mouse_y = 0.0f;
    bool reload = false;          // TitleMenuAction::None - o CHAMADOR
                                  // reescreve o RML e re-renderiza.
    TitleSfxKind sfx = TitleSfxKind::None;
    std::optional<TitleScreenExit> exit;  // ContinueGame/NewGameRequested/
                                           // QuitApp - o CHAMADOR retorna NA
                                           // HORA (equivalente ao antigo
                                           // route_title_action(action)==true).
};

// O NUCLEO PURO do roteamento de evento da tela de titulo - extraido
// behavior-preserving do while(true) antigo (ver title_menu_loop.cpp), 100%
// testavel headless (SEM SDL_Init/janela/glintfx::UiLayer real - ver
// title_screen_step_test.cpp). `boxes` tem kTitleItemCount posicoes: fora do
// mini-dialogo, 0..2 correspondem aos 3 itens (title-item-0..2); dentro dele,
// SO 0..1 sao consultadas (as 2 pills title-confirm-0/1) - MESMO contrato de
// difficulty_screen_step (o CHAMADOR decide QUAIS ids resolver, olhando pra
// state.confirming_new_game ANTES de chamar, MESMA leitura que esta funcao faz
// internamente).
[[nodiscard]] TitleStepResult title_screen_step(
    TitleMenuState& state, const SDL_Event& ev,
    const glintfx::ElementBox boxes[kTitleItemCount]) noexcept;

// F4-1b.3: desfecho do MINI-DRIVER (dentro do wrapper run_title_menu_loop_gl_
// current, ver o comentario grande no .cpp) apos UMA iteracao - consome o
// desfecho da tela de titulo (`title_exit`) e, SE ela pediu Novo Jogo, o
// desfecho da tela de dificuldade aninhada que o driver acabou de rodar
// (`difficulty_exit` - IRRELEVANTE/ignorado quando `title_exit !=
// NewGameRequested`; o driver passa um valor qualquer nesses casos - os testes
// de title_flow_next provam EXPLICITAMENTE, testando as 3x3 combinacoes, que o
// valor realmente nao importa fora de NewGameRequested).
enum class TitleFlowStep {
    ContinueGame,   // o driver devolve TitleLoopExit::ContinueGame na hora.
    StartNewGame,   // dificuldade Chosen - o driver devolve TitleLoopExit::
                    // NewGame com a dificuldade escolhida.
    QuitApp,        // Sair (na tela de titulo OU dentro da dificuldade) - o
                    // driver devolve TitleLoopExit::QuitApp.
    RetryTitle,     // dificuldade Cancelled - o driver RE-ENTRA no MESMO
                    // objeto TitleScreen (run_screen_state de novo) - MESMO
                    // comportamento do antigo ui_opt.reset()/emplace() manual,
                    // agora consequencia ESTRUTURAL de TitleScreen::exit() ja
                    // ter rodado antes desta decisao (ver o comentario grande
                    // no topo deste header).
};

[[nodiscard]] TitleFlowStep title_flow_next(
    TitleScreenExit title_exit, DifficultyLoopExit difficulty_exit) noexcept;

// FLASH-CTX (extracao behavior-preserving, A2): NUCLEO que ASSUME um contexto
// GL JA CORRENTE e com os ponteiros de funcao (glad) JA CARREGADOS - MESMO
// espirito de run_system_menu_loop_gl_current (system_menu_loop.hpp, ver o
// comentario la pro contrato completo das 2 formas de entrar). Devolve por
// `*out_exit`/`*out_loaded_save`/`*out_new_game_difficulty`, MESMO contrato de
// run_title_menu_loop_owning_gl abaixo (parametros identicos, so sem window
// ownership). Hoje SO chamada internamente por run_title_menu_loop_owning_gl E
// diretamente pela Maestro (FLASH-CTX, contexto GL unico - ver
// Maestro::show_title_screen).
//
// `sync_hook` (F4-1b.3, opcional - nullptr = nao sincroniza nada, MESMO
// comportamento de antes desta fatia): repassado direto pro
// gus::app::run_screen_state() do MINI-DRIVER, a CADA iteracao (inclusive
// quando o driver re-entra a lista apos um Cancelled na dificuldade) - MESMO
// papel/contrato de difficulty_menu_loop.hpp (nenhum chamador de producao
// passa `sync_hook` ainda; existe pra esta tela ja seguir o MESMO contrato que
// o resto da onda F4 vai adotar tela a tela). NAO repassado pra
// run_difficulty_menu_loop_gl_current (a tela de dificuldade aninhada continua
// sem sync_hook de producao, MESMO estado de antes desta fatia).
void run_title_menu_loop_gl_current(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator, const std::string& saves_dir,
    TitleLoopExit* out_exit, gus::domain::save::SaveData* out_loaded_save,
    gus::domain::save::DifficultyLevel* out_new_game_difficulty = nullptr,
    const std::string& frozen_background_png = std::string(),
    const gus::app::EventSyncHook& sync_hook = nullptr);

// Roda a tela de titulo ATE o jogador escolher Continuar/Novo Jogo/Sair (ou
// fechar a janela). CRIA seu PROPRIO contexto GL na janela dada (mesma
// receita/atributos de run_system_menu_loop_owning_gl - profile core 3.3,
// double-buffer, stencil 8), carrega o glad, chama run_title_menu_loop_gl_current
// (o nucleo acima) e DESTROI o contexto ao sair.
//
// `audio`: AudioEngine da Maestro (nao-dono, MESMO padrao de injecao de
//   run_system_menu_loop_owning_gl/BattleScene::set_audio) - COCKPIT-SFX-HOVER-
//   CLIQUE: hover (edge-detect via gus/app/screens/ui_hover.hpp) e clique nos
//   botoes/pills da tela, MESMOS sons do menu de pausa (kMenuHoverSfxFile/
//   kMenuClickSfxFile) - paridade sonora entre TODOS os menus de botoes.
// `saves_dir`: diretorio real dos saves (o CHAMADOR passa gus::platform::fs::
//   resolve_saves_dir() - MESMA convencao de injecao de outros loops,
//   testabilidade/override via GUSWORLD_HOME).
// `out_exit`: preenchido com o desfecho (ver TitleLoopExit acima). Nao pode ser
//   nullptr.
// `out_loaded_save`: SO valido (preenchido) quando `*out_exit ==
//   TitleLoopExit::ContinueGame`; ignorado/intocado nos demais casos. Pode ser
//   nullptr SE o chamador so precisar saber QUE o jogador quer continuar (sem
//   uso previsto - o unico chamador de producao sempre fornece).
// `out_new_game_difficulty` (MODOS-MORTE Fase 0): SO valido (preenchido) quando
//   `*out_exit == TitleLoopExit::NewGame` - "Novo Jogo" agora passa pela TELA DE
//   SELECAO DE DIFICULDADE (gus/app/screens/difficulty_menu_loop.hpp, ANINHADA no
//   MESMO contexto GL, disparada ao confirmar StartNewGame - ver o .cpp) ANTES de
//   devolver NewGame; a dificuldade ESCOLHIDA sai aqui. Ignorado/intocado nos
//   demais casos. Pode ser nullptr (degrada: o CHAMADOR usa o default Medio do
//   proprio SaveData).
// `frozen_background_png` (default vazio, M7-FB3 MUDOU O USO): a LISTA da tela de
//   titulo (Continuar/Novo Jogo/Sair) NAO desenha mais este PNG - playtest do Gus
//   Dragon ("menu inicial de jogo tem arte/animacao PROPRIA por tras, nao a tela de
//   onde o jogador estava") + decisao do lider: o fundo da LISTA agora e SEMPRE o
//   boot pixelizado VIVO (gus::app::BootPixelOverlay::draw_idle + gus::core::anim::
//   boot_pixel_idle_frame_index, resolvido via gus::core::assets::kVfxBootPixelDir -
//   ZERO dependencia deste parametro). `frozen_background_png` CONTINUA existindo e
//   sendo repassado, INTOCADO, pra tela de SELECAO DE DIFICULDADE aninhada (aberta
//   pelo MINI-DRIVER ao confirmar "Novo Jogo", ver run_title_menu_loop_gl_current
//   no .cpp) - essa tela fora do escopo deste feedback, segue mostrando a cidade
//   congelada (vazio degrada pro fundo abstrato, MESMO contrato de antes).
//
// Devolve false se a criacao do contexto GL ou o load do glad falhar (a janela
// segue viva; `*out_exit` fica no default QuitApp definido pela IMPLEMENTACAO -
// o CHAMADOR decide como degradar; o unico chamador de producao trata false como
// "sem tela de titulo, comeca fresco" em vez de fechar o app, ver
// Maestro::show_title_screen).
[[nodiscard]] bool run_title_menu_loop_owning_gl(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator, const std::string& saves_dir,
    TitleLoopExit* out_exit, gus::domain::save::SaveData* out_loaded_save,
    gus::domain::save::DifficultyLevel* out_new_game_difficulty = nullptr,
    const std::string& frozen_background_png = std::string());

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_TITLE_MENU_LOOP_HPP
