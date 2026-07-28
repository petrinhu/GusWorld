// SPDX-License-Identifier: GPL-3.0-or-later
// gus/app/screens/battle_preview.hpp
//
// VIEWER da BattleScreen (M5, incremento 1): abre uma janela SDL direto na BattleScene
// pra o lider VER o esqueleto navegavel (arena + fila CTB + HUD) rodando na engine, sem
// passar pelo overworld. Mesmo padrao do --anim-preview (viewer dedicado, casca SDL
// fina, Esc/fechar sai). NAO conduz combate ainda; e validacao visual do layout.
//
// FORMA DE ENTRAR/SAIR (item 4 do incremento): o main pluga este viewer no modo
// "--battle". Entrar = rodar o app com --battle; sair = Esc ou fechar a janela (volta
// ao shell). Quando o loop de telas (Overworld<->Battle) existir (incremento futuro), a
// transicao real par.3.3 substitui este atalho; por ora ele isola a tela pra inspecao.
//
// M7-COSTURA (ADR-012 Onda 1): run_battle_preview_embedded() e a forma REUSAVEL que a
// Maestro chama - roda a MESMA BattleScene/loop numa janela JA EXISTENTE (a da cidade,
// no design "trocar escondido atras do preto": SDL_Renderer <-> contexto GL na MESMA
// janela) e devolve o CombatOutcome final via out-param. run_battle_preview() (o
// --battle standalone) virou um WRAPPER fino: cria sua PROPRIA janela + SDL_Init/Quit e
// delega pra run_battle_preview_embedded (outcome descartado - ninguem consome aqui).
//
// Cross-ref: gus/app/screens/battle_scene.hpp (a cena renderizada);
//            gus/app/screens/anim_preview.hpp (viewer irmao, mesmo padrao);
//            gus/app/maestro.hpp (dono da janela compartilhada, M7-COSTURA).

#ifndef GUS_APP_SCREENS_BATTLE_PREVIEW_HPP
#define GUS_APP_SCREENS_BATTLE_PREVIEW_HPP

#include <string>
#include <string_view>

#include <SDL3/SDL.h>  // SDL_Window (dono da janela do host, ver run_battle_preview_embedded)

#include "gus/app/screen_state.hpp"  // F4-1b.5: gus::app::EventSyncHook (MESMO contrato da onda F4)
#include "gus/app/screens/battle_assets.hpp"  // AC-E11 A3: resolve_retratos_dir/
                                               // resolve_music_path/etc moraram pra ca
                                               // (ADR-019); incluido aqui pra call-sites
                                               // existentes (ex. maestro.cpp) continuarem
                                               // intactos.
#include "gus/app/screens/battle_input.hpp"  // AC-E11 A1: battle_key_down/BattleEscEffect/
                                              // battle_digit_for_key moraram pra ca (ADR-019);
                                              // incluido aqui pra call-site existentes que so
                                              // incluem battle_preview.hpp continuarem intactos.
#include "gus/app/screens/battle_scene.hpp"
#include "gus/domain/combat/combat_enums.hpp"  // CombatOutcome (out-param do embedded)
#include "gus/platform/audio/audio_engine.hpp"  // AudioEngine externo (M7-COSTURA Inc 2)

namespace gus::app::screens {

// AC-E11 A1/A3 (ADR-019): resolve_retratos_dir/resolve_status_icons_dir/
// resolve_music_path/resolve_intent_icons_dir MORARAM pra gus/app/screens/
// battle_assets.hpp; battle_digit_for_key/BattleEscEffect/battle_key_down MORARAM pra
// gus/app/screens/battle_input.hpp (ambos incluidos no topo deste arquivo) - ficam
// acessiveis via gus::app::screens::* pra quem so inclui battle_preview.hpp, ZERO
// mudanca de call-site.

// M7-COSTURA: roda o loop de batalha (mesma BattleScene/mesmo esqueleto) numa janela JA
// CRIADA por quem chama (a Maestro). NAO chama SDL_Init/SDL_Quit nem cria/destroi a
// janela - so o CONTEXTO GL (criado na entrada, destruido na saida; ver ADR-012 Onda 1,
// "trocar escondido atras do preto"). O loop sai por 3 motivos DISTINTOS: (1) o combate
// chegou a um desfecho TERMINAL (Victory/Defeat/Fled - fix BUG-2 do playtest ao vivo do
// lider: "perdi a batalha e ficou preso, so tocando musica" - o loop antigo so saia via
// Esc explicito, entao Defeat/Fled nunca eram detectados aqui e a tela ficava parada);
// (2) o jogador apertou Esc (pilha de modais vazia); (3) o jogador FECHOU A JANELA
// (SDL_EVENT_QUIT) - este ultimo e um sinal DISTINTO de qualquer CombatOutcome (fechar a
// janela nao e "vitoria/derrota/fuga", e "encerrar o PROGRAMA INTEIRO"; fix BUG-3: sem
// isto o quit era absorvido aqui e a Maestro reabria a cidade num LOOP INFINITO se o
// jogador ainda estivesse sobre o inimigo). Todos os 3 motivos convergem no MESMO
// choke-point: grava o CombatOutcome final (Victory/Defeat/Fled/Ongoing se a janela foi
// fechada no meio) em *out_outcome (se nao-nulo) E se o motivo foi especificamente (3) em
// *out_quit_requested (se nao-nulo; default false). A Maestro usa out_quit_requested pra
// decidir "encerrar o app" (nao "voltar pra cidade"). Devolve 0 ok, !=0 se a criacao do
// contexto GL ou o load de funcoes GL falhar (a janela segue viva - quem chamou decide o
// que fazer).
//
// M7-COSTURA Inc 2 (ADR-012 decisao 5, paga a divida do ADR-011 "AudioEngine e dono da
// battle_preview"):
//   external_audio: nullptr (default) = comportamento de SEMPRE - cria/possui um
//     AudioEngine LOCAL (destruido ao sair; o kit CC0 de musica/SFX toca/para aqui
//     mesmo, com fade-in/fade-out proprios) - o caminho do --battle STANDALONE
//     (run_battle_preview() abaixo) e de todo selftest/captura, INTOCADOS. Nao-nulo =
//     a Maestro passa o SEU AudioEngine (dono real, M7-COSTURA Inc 2) - esta funcao
//     SO usa (ponteiro nao-dono, mesmo padrao de BattleScene::set_audio), NUNCA toca
//     musica (a Maestro cronometra o crossfade com o fade preto por fora, ver
//     gus/app/maestro_logic.hpp::crossfade_music) - so carrega/dispara o SFX do hit
//     no engine externo (preserva a variante A/B GUSWORLD_HIT_SFX=alt).
//   fade_in_seconds/fade_out_seconds: <=0 (default) = SEM fade visual (comportamento
//     de sempre). >0 = desenha o overlay preto (gus/core/anim/fade_transition.hpp) por
//     cima da arena+HUD na ENTRADA (kIn, tela clareando sobre o 1o frame pronto, antes
//     do loop interativo comecar) e na SAIDA (kOut, tela escurecendo sobre o ULTIMO
//     frame congelado, depois que o loop interativo termina) - SO a Maestro pede isto
//     (o --battle standalone e os selftests pedem 0, preservando 100% o comportamento
//     anterior). O fade de SAIDA e PULADO se o motivo foi o jogador fechar a janela
//     (quit_requested) - fechar e imediato, sem segurar o jogador pra um fade que ele
//     nao pediu.
//
// `sync_hook` (F4-1b.5, opcional - nullptr = nao sincroniza nada, MESMO comportamento
// de sempre): repassado direto pro gus::app::run_screen_state() interno (ver o .cpp) -
// MESMO papel/contrato de title_menu_loop.hpp/system_menu_loop.hpp. O UNICO chamador de
// producao (Maestro::to_battle) NAO passa sync_hook ainda (MESMO estado de antes desta
// fatia) - existe pra esta tela ja seguir o MESMO contrato do resto da onda F4.
int run_battle_preview_embedded(
    SDL_Window* window, gus::domain::combat::CombatOutcome* out_outcome,
    bool* out_quit_requested = nullptr,
    gus::platform::audio::AudioEngine* external_audio = nullptr,
    float fade_in_seconds = 0.0f, float fade_out_seconds = 0.0f,
    const gus::app::EventSyncHook& sync_hook = nullptr);

// FLASH-CTX (extracao behavior-preserving, A2): NUCLEO que ASSUME um contexto
// GL JA CORRENTE e com os ponteiros de funcao (glad) JA CARREGADOS - MESMO
// espirito de run_system_menu_loop_gl_current (system_menu_loop.hpp). Mesmos
// parametros/contrato de run_battle_preview_embedded (ver acima), so sem a
// posse do contexto GL (nao cria nem destroi). Hoje SO chamada internamente
// por run_battle_preview_embedded (unico chamador de producao continua sendo
// a Maestro, via a variante que possui o contexto); exposta pra futura
// reutilizacao aninhada (Opcao C, contexto GL unico).
//
// F4-1b.5 (onda F4 "casca SDL -> App mode do glintfx", fatia 1b.5 - a conversao MAIS
// ARRISCADA da onda): o HOST REAL da batalha (~1831 linhas, 3 SDL_PollEvent PROPRIOS -
// fade-in/loop principal/fade-out) foi convertido pra uma classe BattleScreen
// (gus::app::ScreenState) + gus::app::run_screen_state, MESMA tecnica das telas
// anteriores (Difficulty/SaveLoad/Title/System) - so que os 3 PollEvent viram 3 FASES de
// UMA UNICA tela (gus::app::screens::BattlePhase: FadeIn -> Main -> FadeOut), decididas
// pela FSM PURA battle_phase_initial/battle_phase_next abaixo (testavel headless, SEM
// SDL_Init/janela/BattleScene - ver battle_screen_step_test.cpp). Esta funcao (o NUCLEO)
// e agora um WRAPPER FINO: constroi a BattleScreen, chama run_screen_state(), e SO
// ENTAO escreve *out_outcome/*out_quit_requested (MESMO choke-point/ORDEM das linhas
// ~1681-1689 do while(true) antigo - out_outcome PRIMEIRO, out_quit_requested DEPOIS;
// intocavel, a Maestro distingue "abortou" de "derrota" por esses 2 out-params).
void run_battle_preview_embedded_gl_current(
    SDL_Window* window, gus::domain::combat::CombatOutcome* out_outcome,
    bool* out_quit_requested = nullptr,
    gus::platform::audio::AudioEngine* external_audio = nullptr,
    float fade_in_seconds = 0.0f, float fade_out_seconds = 0.0f,
    const gus::app::EventSyncHook& sync_hook = nullptr);

// Roda o viewer da BattleScene: SDL_Init proprio, janela PROPRIA, loop de render do
// esqueleto (camera logica 960x540 escalada por inteiro x2 = 1080p), Esc/fechar encerra.
// WRAPPER fino sobre run_battle_preview_embedded (outcome descartado). Devolve 0 ok.
int run_battle_preview();

// ---------------------------------------------------------------------------------
// F4-1b.5: FSM DE FASE PURA do host da batalha - decide FadeIn -> Main -> FadeOut ->
// Done SEM tocar SDL/GL/BattleScene/audio nenhum (testavel headless, ZERO setup - ver
// battle_screen_step_test.cpp). BattleScreen (privada no .cpp) e o UNICO consumidor de
// producao; exposta aqui SO pra ser testavel (mesmo espirito de difficulty_screen_step/
// title_screen_step, que vivem no header PUBLICO da tela pra o teste incluir).

// As 3 fases + o estado terminal. Cada fase corresponde a UM dos 3 SDL_PollEvent
// antigos: FadeIn/FadeOut = os 2 `while(fading)` (Pump LIMITADO a QUIT); Main = o
// `while(running)` principal (roteamento completo). Done = a tela terminou (ScreenState::
// finished()==true) - run_screen_state() para de chamar handle_event()/tick() e roda
// exit() em seguida.
enum class BattlePhase {
    FadeIn,
    Main,
    FadeOut,
    Done,
};

// Fase INICIAL de uma BattleScreen nova - `fade_in_enabled` e SEMPRE
// `fade_in_seconds > 0.0f && running` (running so pode ja estar false aqui se um
// self-test como GUSWORLD_BATTLE_MOUSE_SELFTEST/GUSWORLD_BATTLE_ACTOR_SELFTEST rodou
// e terminou por completo DENTRO do proprio enter() - MESMO comportamento do
// `if (fade_in_seconds > 0.0f && running)` antigo: guard falso pula o FadeIn
// inteiro). fade_in_enabled==false comeca DIRETO em Main (o --battle standalone e
// quase todo self-test/captura, que pedem fade_in_seconds<=0 - ver os defaults acima).
[[nodiscard]] BattlePhase battle_phase_initial(bool fade_in_enabled) noexcept;

// Decisao PURA de proxima fase, chamada UMA vez por tick() (ou uma vez extra em
// enter(), pro caso Main-nasce-ja-terminado dos 2 self-tests acima) apos a fase
// corrente terminar seu proprio trabalho:
//   `quit`: SDL_EVENT_QUIT ja chegou (em QUALQUER fase, ver battle_screen_should_close_
//     on_event abaixo) - forca Done na hora, de QUALQUER fase corrente (o "quit pula o
//     resto" do FIX BUG-3: FadeIn->Done pula Main+FadeOut; Main->Done pula FadeOut;
//     FadeOut->Done so encerra o proprio fade mais cedo). Na BattleScreen real isto e
//     redundante com ScreenState::window_closed() (que ja para run_screen_state() de
//     chamar tick() de novo, ver screen_state.hpp) - a FSM fica correta/testavel do
//     mesmo jeito, sem depender dessa redundancia externa.
//   `phase_done`: a fase CORRENTE terminou seu proprio criterio - FadeIn/FadeOut:
//     elapsed>=duration (o tempo acumulado desta fase alcancou fade_in/out_seconds);
//     Main: o `running` interno virou false (combate terminou/Esc na pilha vazia/
//     algum self-test concluiu/max_frames/captura de 1 frame).
//   `fade_out_enabled`: SEMPRE `fade_out_seconds > 0.0f` (avaliado 1 vez, MESMO papel
//     de fade_in_enabled acima) - Main->Done direto se false (--battle standalone/
//     selftests, que pedem fade_out_seconds<=0).
// `current` inalterado se `phase_done==false` e `quit==false` (a fase segue rodando).
[[nodiscard]] BattlePhase battle_phase_next(BattlePhase current, bool quit,
                                            bool phase_done,
                                            bool fade_out_enabled) noexcept;

// Decisao PURA de fechamento de janela - MESMA condicao que os 3 handlers de evento
// antigos checavam (`ev.type == SDL_EVENT_QUIT`), extraida em UMA funcao pra ser
// testavel isoladamente e reusada tanto pelas fases de fade (que SO ouvem isto,
// ignorando qualquer outro tipo de evento - "Pump de eventos LIMITADO a SDL_EVENT_QUIT"
// dos 2 `while(fading)` antigos) quanto pela fase Main (que ouve tudo, mas trata QUIT
// da MESMA forma - ver BattleScreen::handle_event_main_ no .cpp).
[[nodiscard]] bool battle_screen_should_close_on_event(const SDL_Event& ev) noexcept;

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_PREVIEW_HPP
