// gus/app/screens/player_sprites_loader.hpp
//
// Carrega o PlayerSpriteSet do Caua a partir de uma pasta base, via IRenderer
// (que resolve cada arquivo para um TextureId). Fica em app/ (orquestra a casca
// de I/O do renderer), NAO em core/ (toca caminho de arquivo). Sem Qt direto:
// monta os caminhos com std::string e delega o carregamento ao renderer.
//
// LAYOUT ESPERADO dos assets (PixelLab, ja no disco, fora do git):
//   <base>/south.png  north.png  east.png  west.png         (neutro/idle)
//   <base>/walk/<dir>/{0,1,2,3}.png   para dir em {south,north,east,west}
//
// RESOLUCAO DO <base>: a casca (main/GameWindow) decide. Ordem sugerida:
//   1) variavel de ambiente GUSWORLD_ASSETS (se setada);
//   2) caminho do repo embutido em tempo de compilacao (GUSWORLD_ASSETS_DIR);
//   3) "resources/sprites/caua_volt" relativo ao CWD (rodando da raiz do repo).
// Ver player_sprites_loader.cpp / main.cpp.
//
// DEGRADACAO: se um arquivo faltar ou o backend nao suportar textura (smoke
// offscreen Null), o slot fica kInvalidTexture; PlayerSpriteSet::loaded() vira
// false e o render cai pro contorno (fallback). Nunca crasha.

#ifndef GUS_APP_SCREENS_PLAYER_SPRITES_LOADER_HPP
#define GUS_APP_SCREENS_PLAYER_SPRITES_LOADER_HPP

#include <string>

#include "gus/app/screens/overworld_sim.hpp"  // PlayerSpriteSet
#include "gus/platform/render2d/i_renderer.hpp"

namespace gus::app::screens {

// Carrega os sprites do Caua de <base_dir> usando o renderer. Devolve o set (com
// slots invalidos onde faltou). Nao lanca.
[[nodiscard]] PlayerSpriteSet load_caua_sprites(
    gus::platform::render2d::IRenderer& renderer, const std::string& base_dir);

// Resolve o diretorio base dos sprites do Caua segundo a ordem documentada acima
// (env GUSWORLD_ASSETS > GUSWORLD_ASSETS_DIR de compilacao > relativo ao CWD).
[[nodiscard]] std::string resolve_caua_sprites_dir();

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_PLAYER_SPRITES_LOADER_HPP
