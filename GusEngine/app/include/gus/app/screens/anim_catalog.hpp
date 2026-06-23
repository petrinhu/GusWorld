// gus/app/screens/anim_catalog.hpp
//
// AnimCatalog: monta, EM RUNTIME, a lista de animacoes do Gus varrendo
// resources/sprites/gus/. Fica em app/ por TOCAR FILESYSTEM (std::filesystem) -
// fora da invariante pura de core/. Nao toca SDL nem GPU: so descobre os CAMINHOS
// dos frames; quem carrega PNG e desenha e o anim_preview (via IRenderer).
//
// O QUE VARRE (dinamico, le a pasta na hora - novas anims aparecem sozinhas):
//   - anims/<NOME>/f0..fN.png        -> 1 animacao por subpasta (NOME = rotulo);
//   - walk/<dir>/f0..fN.png          -> 1 animacao "walk_<dir>" por direcao;
//   - rotations/<i>_<nome>.png       -> 1 animacao SINTETICA "turntable" que cicla
//                                       as 8 rotacoes estaticas em loop (1 frame
//                                       por rotacao, ordenadas pelo indice).
//
// Cada AnimEntry guarda o rotulo + os caminhos ABSOLUTOS dos frames JA ORDENADOS
// (f0,f1,...; turntable por indice 0..7). A lista vem ordenada por rotulo pra dar
// uma navegacao estavel no viewer. Se a pasta nao existir, devolve lista vazia (o
// viewer reporta "nenhuma animacao" e sai limpo, sem crashar).

#ifndef GUS_APP_SCREENS_ANIM_CATALOG_HPP
#define GUS_APP_SCREENS_ANIM_CATALOG_HPP

#include <string>
#include <vector>

namespace gus::app::screens {

// Uma animacao descoberta: um rotulo pra HUD + os caminhos dos frames em ordem.
struct AnimEntry {
    std::string label;                // ex.: "attack_melee", "walk_south", "turntable"
    std::vector<std::string> frames;  // caminhos absolutos, ja ordenados
};

// Resolve a pasta resources/sprites/gus do mesmo jeito que o loader do Caua
// resolve a dele: env GUSWORLD_ASSETS > GUSWORLD_ASSETS_DIR (compilacao) >
// relativo ao CWD ("resources/sprites/gus").
[[nodiscard]] std::string resolve_gus_sprites_dir();

// Varre <gus_dir> e devolve as animacoes encontradas (anims/, walk/, turntable das
// rotations/), ordenadas por rotulo. Lista vazia se a pasta nao existir ou nada for
// achado. Nao lanca: erros de I/O sao tratados como "pasta ausente".
[[nodiscard]] std::vector<AnimEntry> build_gus_anim_catalog(
    const std::string& gus_dir);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_ANIM_CATALOG_HPP
