// gus/app/src/screens/overworld_sim.cpp
//
// Implementacao do OverworldSim (M1). Ver header. Travado por
// app/tests/overworld_sim_test.cpp (TEST-FIRST).

#include "gus/app/screens/overworld_sim.hpp"

#include <utility>  // std::move

namespace gus::app::screens {

namespace {

// Multiplicador de corrida (PLACEHOLDER; o lider ajusta vendo). 1.6x walk.
constexpr float kRunMultiplier = 1.6f;

// Cores placeholder de bom contraste (RGBA em [0,1]).
//   parede: cinza-azulado escuro; jogador: ciano vivo.
constexpr gus::platform::render2d::DrawColor kWallColor{0.18f, 0.20f, 0.28f, 1.0f};
constexpr gus::platform::render2d::DrawColor kPlayerColor{0.20f, 0.85f, 0.90f, 1.0f};

// Espessura do contorno do jogador em mundo (placeholder; ~1/8 de um tile de 16).
constexpr float kPlayerOutlineWorld = 2.0f;

// true se os retangulos de mundo a e b se sobrepoem (meio-aberto).
bool overlaps(const gus::core::spatial::Rect& a, const gus::core::spatial::Rect& b) {
    return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}

}  // namespace

OverworldSim::OverworldSim(gus::core::spatial::TileGrid grid,
                           gus::core::spatial::Aabb player_start,
                           float walk_speed_tiles_per_sec)
    : grid_(std::move(grid)),
      prev_(player_start),
      curr_(player_start),
      walk_speed_world_(walk_speed_tiles_per_sec * grid_.tile_size()) {}

void OverworldSim::step_fixed(int dx, int dy, bool run, float fixed_dt) noexcept {
    // A posicao atual vira a "anterior" deste frame (base da interpolacao).
    prev_ = curr_;

    if (dx == 0 && dy == 0) {
        return;  // parado: prev == curr, render nao interpola movimento
    }

    const float speed = walk_speed_world_ * (run ? kRunMultiplier : 1.0f);
    // Deslocamento desejado neste passo (cardinal cru; diagonal NAO normalizada -
    // decisao de feel pro lider). dx,dy em {-1,0,1}.
    const float move_x = static_cast<float>(dx) * speed * fixed_dt;
    const float move_y = static_cast<float>(dy) * speed * fixed_dt;

    // Colisao que desliza nas paredes (resolucao por eixo: X depois Y).
    const gus::core::spatial::MoveResult r =
        gus::core::spatial::resolve_move(grid_, curr_, move_x, move_y);
    curr_ = r.box;
}

gus::core::spatial::CameraView OverworldSim::camera_view(
    float viewport_w, float viewport_h) const noexcept {
    const float map_w = static_cast<float>(grid_.width()) * grid_.tile_size();
    const float map_h = static_cast<float>(grid_.height()) * grid_.tile_size();
    const gus::core::spatial::Vec2 center{curr_.x + curr_.w * 0.5f,
                                          curr_.y + curr_.h * 0.5f};
    return gus::core::spatial::clamp_camera(center, viewport_w, viewport_h, map_w,
                                            map_h);
}

gus::core::spatial::Aabb OverworldSim::interpolated_player(float alpha) const noexcept {
    // lerp(prev, curr, alpha). w/h preservados (nao mudam).
    gus::core::spatial::Aabb p = curr_;
    p.x = prev_.x + (curr_.x - prev_.x) * alpha;
    p.y = prev_.y + (curr_.y - prev_.y) * alpha;
    return p;
}

void OverworldSim::render(gus::platform::render2d::IRenderer& renderer,
                          float viewport_w, float viewport_h, float alpha) const {
    // Camera centrada no jogador INTERPOLADO (camera segue suave junto do boneco).
    const gus::core::spatial::Aabb shown = interpolated_player(alpha);
    const float map_w = static_cast<float>(grid_.width()) * grid_.tile_size();
    const float map_h = static_cast<float>(grid_.height()) * grid_.tile_size();
    const gus::core::spatial::Vec2 cam_center{shown.x + shown.w * 0.5f,
                                              shown.y + shown.h * 0.5f};
    const gus::core::spatial::CameraView view = gus::core::spatial::clamp_camera(
        cam_center, viewport_w, viewport_h, map_w, map_h);

    renderer.begin_frame(view.rect, static_cast<int>(viewport_w),
                         static_cast<int>(viewport_h));

    // Paredes: desenha so as celulas bloqueadas que intersectam a janela da camera
    // (culling simples). A borda do mapa e parede implicita (nao tem celula
    // armazenada); aqui desenhamos as celulas DENTRO de [0,width)x[0,height) que
    // estao bloqueadas - o limite externo aparece como o "vazio" alem da ultima
    // fileira de paredes do mapa de teste.
    const float ts = grid_.tile_size();
    for (int cy = 0; cy < grid_.height(); ++cy) {
        for (int cx = 0; cx < grid_.width(); ++cx) {
            if (!grid_.is_blocked(cx, cy)) {
                continue;
            }
            gus::core::spatial::Rect cell{static_cast<float>(cx) * ts,
                                          static_cast<float>(cy) * ts, ts, ts};
            if (overlaps(cell, view.rect)) {
                renderer.draw_filled_rect(cell, kWallColor);
            }
        }
    }

    // Jogador por cima (contorno da hitbox, na posicao interpolada).
    const gus::core::spatial::Rect player_rect{shown.x, shown.y, shown.w, shown.h};
    renderer.draw_rect_outline(player_rect, kPlayerColor, kPlayerOutlineWorld);

    renderer.end_frame();
}

}  // namespace gus::app::screens
