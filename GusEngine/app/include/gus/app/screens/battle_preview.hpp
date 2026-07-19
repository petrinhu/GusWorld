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
int run_battle_preview_embedded(
    SDL_Window* window, gus::domain::combat::CombatOutcome* out_outcome,
    bool* out_quit_requested = nullptr,
    gus::platform::audio::AudioEngine* external_audio = nullptr,
    float fade_in_seconds = 0.0f, float fade_out_seconds = 0.0f);

// FLASH-CTX (extracao behavior-preserving, A2): NUCLEO que ASSUME um contexto
// GL JA CORRENTE e com os ponteiros de funcao (glad) JA CARREGADOS - MESMO
// espirito de run_system_menu_loop_gl_current (system_menu_loop.hpp). Mesmos
// parametros/contrato de run_battle_preview_embedded (ver acima), so sem a
// posse do contexto GL (nao cria nem destroi). Hoje SO chamada internamente
// por run_battle_preview_embedded (unico chamador de producao continua sendo
// a Maestro, via a variante que possui o contexto); exposta pra futura
// reutilizacao aninhada (Opcao C, contexto GL unico).
void run_battle_preview_embedded_gl_current(
    SDL_Window* window, gus::domain::combat::CombatOutcome* out_outcome,
    bool* out_quit_requested = nullptr,
    gus::platform::audio::AudioEngine* external_audio = nullptr,
    float fade_in_seconds = 0.0f, float fade_out_seconds = 0.0f);

// Roda o viewer da BattleScene: SDL_Init proprio, janela PROPRIA, loop de render do
// esqueleto (camera logica 960x540 escalada por inteiro x2 = 1080p), Esc/fechar encerra.
// WRAPPER fino sobre run_battle_preview_embedded (outcome descartado). Devolve 0 ok.
int run_battle_preview();

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_PREVIEW_HPP
