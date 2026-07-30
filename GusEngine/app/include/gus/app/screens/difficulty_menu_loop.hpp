// SPDX-License-Identifier: GPL-3.0-or-later
// gus/app/screens/difficulty_menu_loop.hpp
//
// LOOP INTERATIVO da TELA DE SELECAO DE DIFICULDADE (MODOS-MORTE Fase 0). Roda a
// maquina de estado pura de difficulty_menu.hpp + o RML/RCSS de
// difficulty_menu_rml.hpp numa glintfx::UiLayer PROPRIA, ANINHADA no MESMO
// contexto GL que o CHAMADOR (gus/app/screens/title_menu_loop.cpp) ja deixou
// corrente - MESMO padrao de aninhamento de run_save_load_menu_loop_gl_current
// (gus/app/screens/save_load_menu_loop.hpp): o CHAMADOR DESTROI o proprio
// UiLayer ANTES de chamar esta funcao (RmlUi NAO suporta 2 UiLayer simultaneos
// no processo) e recria o dele DEPOIS se precisar seguir mostrando algo (ver a
// causa raiz historica no commit 89fb380 "BATTLE-ESC-PAUSE-ACTOR-LIST", 2026-07-05:
// crash real ao vivo - "Element meta pool not empty on shutdown, 75 object(s)
// leaked" - ao abrir um SEGUNDO UiLayer/contexto RmlUi enquanto o primeiro
// ainda estava vivo).
//
// ESTE LOOP NAO FAZ I/O DE DISCO (ao contrario de title_menu_loop.cpp/
// save_load_menu_loop.cpp) - a dificuldade escolhida so e GRAVADA no SaveData
// quando o CHAMADOR (Maestro, via title_menu_loop.cpp) de fato inicia o jogo novo
// (StartNewGame); esta tela so decide QUAL DifficultyLevel.
//
// Cross-ref: gus/app/screens/difficulty_menu.hpp (estado/navegacao PURA);
//            gus/app/screens/difficulty_menu_rml.hpp (RML/RCSS data-driven);
//            gus/app/screens/title_menu_loop.cpp (o UNICO chamador de producao -
//            dispara esta tela ao confirmar "Novo Jogo"); gus/app/screens/
//            save_load_menu_loop.hpp (MESMA tecnica de aninhamento no mesmo
//            contexto GL, precedente historico).

#ifndef GUS_APP_SCREENS_DIFFICULTY_MENU_LOOP_HPP
#define GUS_APP_SCREENS_DIFFICULTY_MENU_LOOP_HPP

#include <optional>
#include <string>

#include <SDL3/SDL.h>
#include <glintfx/element_box.hpp>  // F4-1b: glintfx::ElementBox, INPUT de difficulty_screen_step

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screen_state.hpp"  // F4-1b: gus::app::EventSyncHook (MESMO contrato de F4-1a)
#include "gus/app/screens/difficulty_menu.hpp"  // DifficultyMenuState/kDifficultyItemCount
#include "gus/domain/save/save_data.hpp"
#include "gus/platform/audio/audio_engine.hpp"  // COCKPIT-SFX-HOVER-CLIQUE: paridade sonora

namespace gus::app::screens {

// Desfecho do loop: o que o CHAMADOR (title_menu_loop.cpp) deve fazer a seguir.
enum class DifficultyLoopExit {
    Chosen,     // dificuldade ESCOLHIDA + confirmada no Aviso #2 -
                // `*out_difficulty` foi preenchido - o CHAMADOR prossegue como
                // StartNewGame, gravando a dificuldade no SaveData novo.
    Cancelled,  // ESC/Voltar na LISTA (fora do splash) - o jogador desistiu de
                // Novo Jogo; o CHAMADOR volta pra tela de titulo (recria o
                // PROPRIO UiLayer, que ja foi destruido antes desta chamada).
    QuitApp,    // o jogador fechou a JANELA durante a tela - o chamador encerra
                // o programa (mesmo contrato de TitleLoopExit::QuitApp).
};

// Roda a tela de selecao de dificuldade ATE o jogador escolher+confirmar, desistir
// (Cancelled) ou fechar a janela. NAO cria/destroi contexto GL (ANINHADA no
// contexto JA CORRENTE do CHAMADOR, MESMO padrao de
// run_save_load_menu_loop_gl_current) - cria a PROPRIA glintfx::UiLayer (o
// CHAMADOR ja destruiu a dele antes de chamar, RmlUi so aceita 1 instancia viva).
//
// `audio`: AudioEngine da Maestro (nao-dono) - hover/clique, MESMOS sons dos
//   demais menus (kMenuHoverSfxFile/kMenuClickSfxFile).
// `out_difficulty`: SO valido (preenchido) quando o retorno e Chosen; intocado
//   nos demais casos. Nao pode ser nullptr.
// `frozen_background_png` (default vazio): MESMA tecnica de fundo real congelado
//   das demais telas - vazio degrada pro fundo abstrato (a vinheta do painel).
//
// Devolve Cancelled por DEFAULT se a criacao da UiLayer falhar (degradacao segura -
// o CHAMADOR volta pra tela de titulo em vez de travar o boot).
//
// F4-1b: `sync_hook` (opcional - nullptr = nao sincroniza nada, MESMO
// comportamento de antes desta fatia) e repassado direto a
// gus::app::run_screen_state() por dentro - MESMO papel/contrato de
// npc_dialogue_loop_gl.hpp:155-160 (F4-1a). Esta tela e ANINHADA (chamada de
// dentro de title_menu_loop.cpp, que NAO foi convertido nesta fatia) - nenhum
// chamador de producao passa `sync_hook` ainda; o parametro existe pra esta tela
// ja seguir o MESMO contrato de ScreenState/EventSyncHook que o resto da onda F4
// vai adotar tela a tela.
[[nodiscard]] DifficultyLoopExit run_difficulty_menu_loop_gl_current(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator,
    gus::domain::save::DifficultyLevel* out_difficulty,
    const std::string& frozen_background_png = std::string(),
    const gus::app::EventSyncHook& sync_hook = nullptr);

// F4-1b (onda F4 "casca SDL -> App mode do glintfx", fatia 1b - PRIMEIRA tela
// convertida ao contrato ScreenState apos F4-1a/npc_dialogue_loop_gl.cpp - esta
// conversao ESTABELECE o template pras telas seguintes da fatia): decisao PURA
// de "o que fazer com 1 SDL_Event" da tela de dificuldade, extraida
// behavior-preserving do while(true) antigo (ver difficulty_menu_loop.cpp) pra
// ficar 100% testavel headless (SEM SDL_Init/janela/glintfx::UiLayer real - ver
// difficulty_screen_step_test.cpp). NAO consulta a UiLayer - as caixas de
// hit-test entram como PARAMETRO (`boxes`), resolvidas pelo CHAMADOR via
// glintfx::UiLayer::get_element_box (MESMO racional POCO ja usado por
// difficulty_hover_index em difficulty_menu.hpp, agora tambem cobrindo
// clique/teclado, nao so hover puro de mouse).
enum class DifficultySfxKind {
    None,     // nenhum som (evento nao roteado por esta tela, ou clique fora de
              // qualquer box)
    Hover,    // hover_sfx_id (normal) - o CHAMADOR mapeia pro SoundId real
    Click,    // click_sfx_id (normal)
    Blocked,  // blocked_sfx_id - item NAO selecionavel (Hardcore nesta Fase 0,
              // ver difficulty_item_selectable)
};

// Resultado de UM difficulty_screen_step(): o que o CHAMADOR (DifficultyScreen::
// handle_event, difficulty_menu_loop.cpp) deve fazer a seguir - TODOS os side
// effects REAIS (SDL/GL/audio) ficam do lado de fora desta struct/funcao, que so
// decide.
struct DifficultyStepResult {
    bool window_closed = false;  // SDL_EVENT_QUIT
    bool resize = false;         // WINDOW_RESIZED/PIXEL_SIZE_CHANGED - o estado da
                                  // tela (DifficultyMenuState) NAO muda; o
                                  // CHAMADOR reconsulta o tamanho real da janela
                                  // (SDL_GetWindowSizeInPixels) e reposiciona a
                                  // UiLayer (efeito fora do POCO).
    bool mouse_move = false;      // MOUSE_MOTION - o CHAMADOR encaminha
                                  // (mouse_x,mouse_y) pro glintfx::UiEvent::
                                  // MouseMove (hover NATIVO/RCSS); o SOM de hover
                                  // de mouse continua no callback nativo
                                  // (ui.set_hover_callback), FORA do escopo desta
                                  // funcao (MESMO id-based hover_cb de sempre).
    float mouse_x = 0.0f;
    float mouse_y = 0.0f;
    bool reload = false;          // DifficultyMenuAction::None - o CHAMADOR
                                  // reescreve o RML (o estado pode ter mudado
                                  // foco/aberto o splash, mesmo sem Chosen/
                                  // Cancelled) e re-renderiza.
    DifficultySfxKind sfx = DifficultySfxKind::None;
    std::optional<DifficultyLoopExit> exit;  // Chosen/Cancelled - o CHAMADOR
                                              // retorna NA HORA (ver route_action
                                              // antigo); Chosen tambem exige
                                              // preencher *out_difficulty - o
                                              // CHAMADOR faz isso (nao ha ponteiro
                                              // de saida nesta funcao PURA).
};

// O NUCLEO PURO do roteamento de evento desta tela. Muta `state` via as MESMAS
// gus::app::screens::difficulty_menu_key_down/difficulty_menu_click_option de
// sempre e devolve o QUE FAZER a seguir (acima). `boxes` tem
// kDifficultyItemCount posicoes - MESMO contrato de difficulty_hover_index: fora
// do splash, 0..3 = os 4 itens da lista; dentro do splash, SO 0..1 sao
// consultadas (os 2 pills Confirmar/Cancelar) - o CHAMADOR decide QUAIS ids
// resolver antes de chamar (ver collect_click_boxes_ em difficulty_menu_loop.cpp),
// olhando pra `state.confirming` ANTES desta chamada (esta funcao tambem le
// `state.confirming` internamente pra escolher entre as duas semanticas de
// `boxes`, MESMA leitura). Eventos que esta tela nao roteia (ex.: TEXT_INPUT)
// devolvem o resultado default (tudo false/None/nullopt) - no-op TOTAL.
[[nodiscard]] DifficultyStepResult difficulty_screen_step(
    DifficultyMenuState& state, const SDL_Event& ev,
    const glintfx::ElementBox boxes[kDifficultyItemCount]) noexcept;

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_DIFFICULTY_MENU_LOOP_HPP
