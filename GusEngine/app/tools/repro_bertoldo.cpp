// DIAGNOSTIC-ONLY (nao commitado): reproduz headless a MESMA sequencia do
// Maestro::init() para a cidade + marcador do inimigo + marcador do Bertoldo,
// com um SDL_Renderer REAL (driver "software", SDL_VIDEODRIVER=dummy - sem
// janela visivel), e despeja um PNG via SDL_RenderReadPixels para inspecao.
#include <SDL3/SDL.h>

#include <iostream>
#include <vector>

#include "gus/app/maestro_logic.hpp"
#include "gus/app/screens/city_loader.hpp"
#include "gus/app/screens/overworld_sim.hpp"
#include "gus/app/screens/player_sprites_loader.hpp"
#include "gus/core/asset_paths.hpp"
#include "gus/platform/render2d/render2d_sdl.hpp"

#include "stb_image_write.h"

namespace {
constexpr int kEnemyOffsetTilesX = -5;
constexpr int kEnemyOffsetTilesY = 4;
constexpr int kNpcBertoldoOffsetTilesX = -5;
constexpr int kNpcBertoldoOffsetTilesY = 13;
}  // namespace

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Window* window =
        SDL_CreateWindow("repro-bertoldo", 1280, 720, SDL_WINDOW_HIDDEN);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == nullptr) {
        std::cerr << "SDL_CreateRenderer falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    std::cout << "renderer driver = " << SDL_GetRendererName(renderer) << "\n";

    gus::platform::render2d::Render2dSdl r2d(renderer);

    gus::app::screens::CityLoadOutcome city =
        gus::app::screens::load_city_or_fallback();
    gus::app::screens::OverworldSim sim(std::move(city.sim));

    const std::string gus_dir = gus::app::screens::resolve_sprites_dir(
        std::string(gus::core::assets::kGusSpritesDir));
    sim.set_player_sprites(
        gus::app::screens::load_gus_sprites(r2d, gus_dir));

    // Marcador do INIMIGO (mesma receita de maestro.cpp).
    const gus::core::spatial::Aabb enemy_anchor = gus::app::pick_fixed_enemy_position(
        sim.grid(), sim.player(), kEnemyOffsetTilesX, kEnemyOffsetTilesY);
    const gus::core::spatial::Aabb enemy_aabb = gus::app::enemy_sprite_footprint_aabb(
        enemy_anchor, sim.tuning().player_sprite_height_tiles, sim.grid().tile_size());
    const std::string retrato_path = gus::app::screens::resolve_sprites_dir(
        std::string(gus::core::assets::kRetratosDir)) + "/" +
        std::string(gus::core::assets::kRetratoInimigoFile);
    const auto enemy_tex = r2d.load_texture(retrato_path.c_str());
    std::cout << "enemy tex valid=" << (enemy_tex != gus::platform::render2d::kInvalidTexture)
              << " path=" << retrato_path << "\n";
    if (enemy_tex != gus::platform::render2d::kInvalidTexture) {
        sim.set_enemy_marker(enemy_aabb, enemy_tex);
    }

    // Marcador do BERTOLDO (mesma receita de maestro.cpp).
    const gus::core::spatial::Aabb npc_anchor = gus::app::pick_fixed_enemy_position(
        sim.grid(), sim.player(), kNpcBertoldoOffsetTilesX, kNpcBertoldoOffsetTilesY);
    const gus::core::spatial::Aabb npc_aabb = gus::app::enemy_sprite_footprint_aabb(
        npc_anchor, sim.tuning().player_sprite_height_tiles, sim.grid().tile_size());
    const std::string bertoldo_path = gus::app::screens::resolve_sprites_dir(
        std::string(gus::core::assets::kBertoldoSpritesDir)) + "/" +
        std::string(gus::core::assets::kBertoldoSpriteSouthFile);
    const auto bertoldo_tex = r2d.load_texture(bertoldo_path.c_str());
    std::cout << "bertoldo tex valid=" << (bertoldo_tex != gus::platform::render2d::kInvalidTexture)
              << " path=" << bertoldo_path << "\n";
    if (bertoldo_tex != gus::platform::render2d::kInvalidTexture) {
        sim.set_npc_bertoldo_marker(npc_aabb, bertoldo_tex);
    }

    std::cout << "player aabb = (" << sim.player().x << "," << sim.player().y << ","
              << sim.player().w << "," << sim.player().h << ")\n";
    std::cout << "enemy aabb  = (" << enemy_aabb.x << "," << enemy_aabb.y << ","
              << enemy_aabb.w << "," << enemy_aabb.h << ")\n";
    std::cout << "npc aabb    = (" << npc_aabb.x << "," << npc_aabb.y << ","
              << npc_aabb.w << "," << npc_aabb.h << ")\n";
    std::cout << "tile_size = " << sim.grid().tile_size()
              << " player_sprite_height_tiles = " << sim.tuning().player_sprite_height_tiles
              << "\n";

    // Camera CENTRADA no NPC (nao no player) pra enquadrar o Bertoldo cheio na tela,
    // igual o lider veria chegando perto dele. Fazemos isso teleportando o player pro
    // pe do NPC (a camera do OverworldSim segue sempre o player).
    // Set player position directly via repeated step towards npc? Simpler: since sim
    // doesn't expose a teleport, we just render with the REAL player start (far away)
    // and ALSO do a second render after moving the player near the npc via step_fixed.
    const float dt = 1.0f / 60.0f;
    // Move o player em direcao ao NPC por um numero grande de ticks (o corner-assist/
    // colisao vai fazer ele andar pela cidade real ate perto do Bertoldo).
    for (int i = 0; i < 6000; ++i) {
        const float dx = npc_anchor.x - sim.player().x;
        const float dy = npc_anchor.y - sim.player().y;
        const int ix = dx > 0.05f ? 1 : (dx < -0.05f ? -1 : 0);
        const int iy = dy > 0.05f ? 1 : (dy < -0.05f ? -1 : 0);
        if (ix == 0 && iy == 0) break;
        sim.step_fixed(ix, iy, /*run=*/true, dt);
    }
    std::cout << "player aabb (apos andar) = (" << sim.player().x << "," << sim.player().y
              << ")\n";

    const int pw = 640;
    const int ph = 480;
    sim.render(r2d, static_cast<float>(pw), static_cast<float>(ph), 0.0f);

    SDL_Surface* surf = SDL_RenderReadPixels(renderer, nullptr);
    if (surf == nullptr) {
        std::cerr << "SDL_RenderReadPixels falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Surface* conv = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32);
    if (conv != nullptr) {
        stbi_write_png("/tmp/claude-1000/-home-petrus-IDrive-Documentos-projetos-claudebrain-Projects-gusworld/"
                        "cb06df2e-ba27-4b40-a6ec-4a14597113cd/scratchpad/repro_bertoldo.png",
                        conv->w, conv->h, 4, conv->pixels, conv->pitch);
        std::cout << "PNG salvo: " << conv->w << "x" << conv->h << "\n";
        SDL_DestroySurface(conv);
    }
    SDL_DestroySurface(surf);

    return 0;
}
