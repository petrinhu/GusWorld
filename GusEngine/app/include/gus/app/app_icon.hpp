// gus/app/app_icon.hpp
//
// APP-ICONE: aplica o icone do app (opcao C do mock docs/design/mockups/08-app-icon.html
// - "Combate + glow cyan": Gus de combate/fundo transparente sobre glow cyan + fundo
// escuro, cantos arredondados) na SDL_Window ja criada. O PNG (kAppIconFile256, ver
// gus/core/asset_paths.hpp) e composto DETERMINISTICO via Pillow (sem IA) a partir da
// arte JA EXISTENTE do retrato de combate sem fundo - nao gerado aqui, so carregado.
//
// UM SO CONSUMIDOR REAL hoje: Maestro::init() (dona da UNICA janela do modo normal,
// ver maestro.cpp) - a antiga SdlWindow::init() (standalone, sem Maestro) tambem chama
// isto, pela simetria "quem cria a janela decide o icone", mas nao e o caminho do app
// rodando de verdade (main.cpp usa Maestro).
//
// Extraido pra este header PROPRIO (em vez de ficar estatico dentro de sdl_window.cpp)
// justamente pra ser chamavel de maestro.cpp TAMBEM, sem duplicar a logica de
// decode+SDL_Surface+SDL_SetWindowIcon nos dois lugares.
//
// DEGRADACAO SEGURA: qualquer falha (asset ausente, decode invalido, SDL_SetWindowIcon
// recusando) loga via SDL_Log e retorna sem tocar a janela - NUNCA crasha o boot. Isto
// cobre tambem o smoke headless (--smoke, SDL_VIDEODRIVER=dummy): o driver dummy nao tem
// SDL_SetWindowIcon com efeito visual nenhum, mas a chamada em si e segura (mesma
// degradacao se o asset nao existir no ambiente do smoke).

#ifndef GUS_APP_APP_ICON_HPP
#define GUS_APP_APP_ICON_HPP

#include <SDL3/SDL.h>

namespace gus::app {

// Carrega kAppIconDir/kAppIconFile256 (via AssetSource, mesma cadeia env > macro > CWD
// que os demais assets genericos usam) e chama SDL_SetWindowIcon(window, ...). No-op
// seguro (so loga) se qualquer passo falhar. `window` nao pode ser nullptr (contrato:
// chamar DEPOIS de SDL_CreateWindow/SDL_CreateWindowAndRenderer terem sucedido).
void set_window_icon_if_available(SDL_Window* window);

}  // namespace gus::app

#endif  // GUS_APP_APP_ICON_HPP
