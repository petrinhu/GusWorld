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

namespace gus::app::screens {

// Resolve a pasta dos retratos 48px (resources/sprites/icons-m5/retratos), na MESMA
// ordem do resolver de sprites do Gus: env GUSWORLD_ASSETS > macro embutido > relativo
// ao CWD. So monta a STRING (nao abre arquivo).
[[nodiscard]] std::string resolve_retratos_dir();

// Resolve a pasta dos icones de status (resources/sprites/icons-m5/status), na mesma
// ordem do resolver de retratos. So monta a STRING (nao abre arquivo).
[[nodiscard]] std::string resolve_status_icons_dir();

// Roda o viewer da BattleScene: SDL_Init proprio, janela, loop de render do esqueleto
// (camera logica 640x360 escalada por inteiro), Esc/fechar encerra. Devolve 0 ok.
int run_battle_preview();

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_PREVIEW_HPP
