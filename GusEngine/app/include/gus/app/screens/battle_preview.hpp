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

#include <SDL3/SDL.h>  // SDL_Keycode (roteamento de teclado, testavel headless - ver abaixo)

#include "gus/app/screens/battle_scene.hpp"
#include "gus/domain/combat/combat_enums.hpp"  // CombatOutcome (out-param do embedded)

namespace gus::app::screens {

// Resolve a pasta dos retratos 48px (resources/sprites/icons-m5/retratos), na MESMA
// ordem do resolver de sprites do Gus: env GUSWORLD_ASSETS > macro embutido > relativo
// ao CWD. So monta a STRING (nao abre arquivo).
[[nodiscard]] std::string resolve_retratos_dir();

// Resolve a pasta dos icones de status (resources/sprites/icons-m5/status), na mesma
// ordem do resolver de retratos. So monta a STRING (nao abre arquivo).
[[nodiscard]] std::string resolve_status_icons_dir();

// Digito 1-9 de uma tecla numerica (fileira OU numpad); 0 se nao for numerica 1-9. Fonte
// unica do mapeamento tecla->N pros atalhos numericos (mira e escolha de ator). Exposto
// (implementacao em battle_preview.cpp) pra ser CHAMAVEL pelo self-test sintetico E pelos
// testes Catch2 do roteamento de teclado (battle_key_routing_test.cpp) - headless, sem SDL_Init.
[[nodiscard]] int battle_digit_for_key(SDL_Keycode key) noexcept;

// Roteamento de TECLADO do host (extraido do loop de eventos - ver definicao/comentario
// completo em battle_preview.cpp). Esc DESEMPILHA 1 nivel de modal por vez (FIX bug2 do
// playtest do lider: "Esc fecha a tela" com um picker/preview aberto) - mira > preview de
// ator > picker > (pilha vazia) sai do viewer. `running` so vira false na pilha vazia.
// Exposto pra ser testavel headless (Catch2), sem abrir janela SDL nem chamar SDL_Init.
void battle_key_down(BattleScene& scene, SDL_Keycode key, bool& running);

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
int run_battle_preview_embedded(SDL_Window* window,
                                 gus::domain::combat::CombatOutcome* out_outcome,
                                 bool* out_quit_requested = nullptr);

// Roda o viewer da BattleScene: SDL_Init proprio, janela PROPRIA, loop de render do
// esqueleto (camera logica 960x540 escalada por inteiro x2 = 1080p), Esc/fechar encerra.
// WRAPPER fino sobre run_battle_preview_embedded (outcome descartado). Devolve 0 ok.
int run_battle_preview();

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_PREVIEW_HPP
