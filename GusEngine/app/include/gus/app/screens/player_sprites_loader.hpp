// gus/app/screens/player_sprites_loader.hpp
//
// Carrega o PlayerSpriteSet de UM personagem a partir de uma pasta base, via
// IRenderer (que resolve cada arquivo para um TextureId). Fica em app/ (orquestra a
// casca de I/O do renderer), NAO em core/ (toca caminho de arquivo). Sem SDL direto:
// monta os caminhos com std::string e delega o carregamento ao renderer.
//
// GENERALIZADO (M1, Gus): o loader e DATA-DRIVEN por um SpriteLayout, que descreve o
// numero de quadros e a convencao de nome de cada personagem (o Caua e o Gus tem
// layouts diferentes). Mantem os dois caminhos sem if espalhado:
//
//   CAUA (legado):                          GUS:
//     <base>/south.png  north.png ...          <base>/walk/<dir>/f0..f6.png  (7 walk)
//     <base>/walk/<dir>/0..3.png  (4 walk)      <base>/anims/breathing_idle/f0..f4.png
//     idle = 1 quadro congelado por direcao      (5 quadros de breathing, NAO direcional)
//
// RESOLUCAO DO <base>: a casca (main/SdlWindow) decide. Ordem sugerida:
//   1) variavel de ambiente GUSWORLD_ASSETS (se setada): <env>/sprites/<subdir>;
//   2) caminho do repo embutido em compilacao (GUSWORLD_ASSETS_DIR): <repo>/sprites/<subdir>;
//   3) "resources/sprites/<subdir>" relativo ao CWD (rodando da raiz do repo).
//
// DEGRADACAO: se um arquivo faltar ou o backend nao suportar textura (smoke
// offscreen / headless), o slot fica kInvalidTexture; PlayerSpriteSet::loaded() vira
// false e o render cai pro contorno (fallback). Nunca crasha.

#ifndef GUS_APP_SCREENS_PLAYER_SPRITES_LOADER_HPP
#define GUS_APP_SCREENS_PLAYER_SPRITES_LOADER_HPP

#include <array>
#include <string>

#include "gus/app/screens/overworld_sim.hpp"  // PlayerSpriteSet
#include "gus/platform/render2d/i_renderer.hpp"

namespace gus::app::screens {

// Convencao de nomes/quadros de UM personagem. Header-only (dados puros). O loader
// le isto pra montar os caminhos - trocar de personagem = trocar o layout, sem if.
struct SpriteLayout {
    // Subpasta dentro de resources/sprites/ (ex.: "gus", "caua_volt").
    const char* subdir = "gus";

    // --- WALK ---
    int walk_frames = 7;        // quadros por direcao (Gus 7, Caua 4)
    // Prefixo do arquivo de walk: "f" -> f0.png..f6.png (Gus); "" -> 0.png..3.png (Caua).
    const char* walk_prefix = "f";

    // Subpasta de walk POR DIRECAO, indexada pela ORDEM do enum Direction
    // (Sul=0, Norte=1, Leste=2, Oeste=3). DATA-DRIVEN pra absorver convencoes de
    // nomenclatura por personagem SEM mexer no input nem no facing compartilhado.
    //
    // Default = nomes diretos {south,north,east,west} (Caua: a arte do leste mostra o
    // perfil pra DIREITA, entao "east" no slot Leste esta correto).
    //
    // GUS (bug do lider 2026-06-23): a arte veio do generate-8-rotations com os ROTULOS
    // leste/oeste TROCADOS na fonte (o PNG "east"/"6_east" mostra o perfil pra ESQUERDA;
    // o "west"/"2_west" mostra pra DIREITA). gus_layout() corrige AQUI trocando os dois
    // slots -> {south,north,west,east}: ao pedir Direction::East o Gus carrega a pasta
    // "west" (que de fato aponta pra direita = +X). Fix ESPECIFICO do Gus; Sul/Norte
    // intactos; Caua intacto.
    std::array<const char*, 4> walk_dir_names = {"south", "north", "east", "west"};

    // --- IDLE ---
    // true: idle ANIMADO num caminho UNICO (mesmo loop pras 4 direcoes), ex.: Gus
    //       <base>/anims/breathing_idle/f0..f(N-1).png.
    // false: idle CONGELADO direcional, 1 quadro por direcao, ex.: Caua <base>/<dir>.png.
    bool idle_animated = true;
    int idle_frames = 5;                          // quadros do breathing (Gus 5)
    const char* idle_dir = "anims/breathing_idle";  // subpasta do idle animado
    const char* idle_prefix = "f";                // prefixo dos quadros do idle animado

    // BUG 1 (lider 2026-06-23): o idle animado do Gus (breathing) so existe de FRENTE
    // (Sul). Replicar esse loop nas 4 direcoes fazia o Gus PARECER virar pra Sul ao
    // parar (a logica de facing ja estava certa; o problema era a arte). Com este flag,
    // o idle animado (breathing) e usado SO na direcao idle_animated_facing; as OUTRAS
    // direcoes recebem o walk f0 DAQUELA direcao como idle de 1 quadro (arte que ja
    // existe), preservando o olhar visualmente sem arte nova. Quando houver breathing
    // direcional completo, basta desligar este flag.
    bool idle_animated_only_one_facing = true;
    // Direcao que TEM o breathing animado (default Sul = a arte de frente do Gus).
    Direction idle_animated_facing = Direction::South;
};

// Layouts canonicos prontos.
[[nodiscard]] SpriteLayout gus_layout() noexcept;
[[nodiscard]] SpriteLayout caua_layout() noexcept;

// Carrega os sprites de <base_dir> segundo o layout, usando o renderer. Devolve o set
// (com slots invalidos onde faltou). Preenche walk_count/idle_count e o FootInset
// medido (alpha-bbox). Nao lanca.
[[nodiscard]] PlayerSpriteSet load_player_sprites(
    gus::platform::render2d::IRenderer& renderer, const std::string& base_dir,
    const SpriteLayout& layout);

// Resolve um caminho RELATIVO de asset (env GUSWORLD_ASSETS > macro de compilacao > CWD).
// rel_subpath vem do header central gus/core/asset_paths.hpp (ex.: kCauaSpritesDir =
// "sprites/caua_volt"); NAO prefixa nada - o caminho ja vem completo. Ver doc no topo.
[[nodiscard]] std::string resolve_sprites_dir(const std::string& rel_subpath);

// --- ATALHOS (compat + conveniencia) ----------------------------------------
// Gus e o player default do overworld agora. (A resolucao da pasta do Gus reusa
// resolve_gus_sprites_dir() de anim_catalog.hpp - sub-caminho do header central
// gus/core/asset_paths.hpp, kGusSpritesDir; nao duplicar aqui.)
[[nodiscard]] PlayerSpriteSet load_gus_sprites(
    gus::platform::render2d::IRenderer& renderer, const std::string& base_dir);

// Caua (mantido pra quem quiser trocar o player; nao usado por default).
[[nodiscard]] PlayerSpriteSet load_caua_sprites(
    gus::platform::render2d::IRenderer& renderer, const std::string& base_dir);
[[nodiscard]] std::string resolve_caua_sprites_dir();

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_PLAYER_SPRITES_LOADER_HPP
