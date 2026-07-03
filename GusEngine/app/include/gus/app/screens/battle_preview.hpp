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
// Cross-ref: gus/app/screens/battle_scene.hpp (a cena renderizada);
//            gus/app/screens/anim_preview.hpp (viewer irmao, mesmo padrao).

#ifndef GUS_APP_SCREENS_BATTLE_PREVIEW_HPP
#define GUS_APP_SCREENS_BATTLE_PREVIEW_HPP

#include <string>

#include <SDL3/SDL.h>  // SDL_Keycode (roteamento de teclado, testavel headless - ver abaixo)

#include "gus/app/screens/battle_scene.hpp"

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

// Roda o viewer da BattleScene: SDL_Init proprio, janela, loop de render do esqueleto
// (camera logica 960x540 escalada por inteiro x2 = 1080p), Esc/fechar encerra. Devolve 0 ok.
int run_battle_preview();

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_PREVIEW_HPP
