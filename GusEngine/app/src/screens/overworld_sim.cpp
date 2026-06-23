// gus/app/src/screens/overworld_sim.cpp
//
// Implementacao do OverworldSim (M1). Ver header. Travado por
// app/tests/overworld_sim_test.cpp (TEST-FIRST).

#include "gus/app/screens/overworld_sim.hpp"

#include <cmath>    // std::sqrt
#include <utility>  // std::move

namespace gus::app::screens {

namespace {

// 1/sqrt(2): fator de normalizacao do passo diagonal (modulo do vetor (1,1)).
constexpr float kInvSqrt2 = 0.70710678f;

// true se os retangulos de mundo a e b se sobrepoem (meio-aberto).
bool overlaps(const gus::core::spatial::Rect& a, const gus::core::spatial::Rect& b) {
    return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}

}  // namespace

OverworldSim::OverworldSim(gus::core::spatial::TileGrid grid,
                           gus::core::spatial::Aabb player_start,
                           OverworldTuning tuning)
    : grid_(std::move(grid)),
      prev_(player_start),
      curr_(player_start),
      tuning_(tuning),
      walk_(WalkCycle::Config{tuning.anim_walk_px_per_frame,
                              tuning.anim_run_px_per_frame}) {}

OverworldSim::OverworldSim(gus::core::spatial::TileGrid grid,
                           gus::core::spatial::Aabb player_start,
                           float walk_speed_tiles_per_sec)
    : OverworldSim(std::move(grid), player_start,
                   [walk_speed_tiles_per_sec] {
                       OverworldTuning t;
                       t.walk_speed_tiles_per_sec = walk_speed_tiles_per_sec;
                       return t;
                   }()) {}

void OverworldSim::step_fixed(int dx, int dy, bool run, float fixed_dt) noexcept {
    // A posicao atual vira a "anterior" deste frame (base da interpolacao).
    prev_ = curr_;

    if (dx == 0 && dy == 0) {
        // Parado: prev == curr (render nao interpola). A direcao MANTEM a ultima
        // (idle nao gira o boneco); o ciclo de walk volta ao neutro (idle congelado).
        walk_.advance(0.0f, run);
        return;
    }

    // Direcao cardinal pela intencao de input (vetor cru), nao pelo movimento
    // resolvido: encostar na parede num eixo NAO deve girar o boneco.
    facing_ = direction_from_move(dx, dy, facing_);

    // Velocidade em unidades de mundo/s = tiles/s * tile_size, com a corrida.
    const float speed = tuning_.walk_speed_tiles_per_sec * grid_.tile_size() *
                        (run ? tuning_.run_multiplier : 1.0f);
    const float dist = speed * fixed_dt;  // distancia neste passo

    float fx = static_cast<float>(dx);
    float fy = static_cast<float>(dy);
    // GANCHO normalize_diagonal: se ligado E for diagonal, normaliza o vetor pra a
    // diagonal ter a MESMA velocidade das cardinais (senao (1,1) anda ~1.41x).
    // Desligado (default): mantem cru (cada eixo recebe o passo cheio).
    if (tuning_.normalize_diagonal && dx != 0 && dy != 0) {
        fx *= kInvSqrt2;
        fy *= kInvSqrt2;
    }
    const float move_x = fx * dist;
    const float move_y = fy * dist;

    // Colisao que desliza nas paredes (resolucao por eixo: X depois Y), agora com
    // corner-assist quando ligado no tuning (escorrega na quina se ha abertura).
    const gus::core::spatial::MoveResult r =
        gus::core::spatial::resolve_move_with_corner_assist(grid_, curr_, move_x,
                                                            move_y, tuning_.corner);
    curr_ = r.box;

    // Anima o walk pela distancia REALMENTE percorrida (apos a colisao): bater na
    // parede num eixo reduz o avanco e, portanto, a troca de quadro - o pe nao
    // "patina". Distancia euclidiana do deslocamento resolvido neste passo.
    const float adx = curr_.x - prev_.x;
    const float ady = curr_.y - prev_.y;
    const float moved = std::sqrt(adx * adx + ady * ady);
    walk_.advance(moved, run);
}

gus::core::spatial::CameraView OverworldSim::camera_view(
    float viewport_w, float viewport_h) const noexcept {
    const float map_w = static_cast<float>(grid_.width()) * grid_.tile_size();
    const float map_h = static_cast<float>(grid_.height()) * grid_.tile_size();
    const gus::core::spatial::Vec2 center{curr_.x + curr_.w * 0.5f,
                                          curr_.y + curr_.h * 0.5f};
    // ZOOM: gancho pronto, INERTE (tuning_.camera_zoom == 1.0 hoje). Pra ligar o
    // zoom, passar (viewport_w / tuning_.camera_zoom, viewport_h / tuning_.camera_zoom)
    // aqui (viewport menor em mundo = mais perto). NAO aplicado agora.
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
    // ZOOM: gancho pronto, INERTE (tuning_.camera_zoom == 1.0). Pra ligar, dividir
    // viewport_w/viewport_h por tuning_.camera_zoom aqui. NAO aplicado agora.
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
                renderer.draw_filled_rect(cell, tuning_.wall_color);
            }
        }
    }

    // Jogador por cima, na posicao interpolada.
    if (sprites_.loaded()) {
        // SPRITE ancorado nos PES sobre a AABB de colisao. A AABB e a hitbox dos
        // pes; o sprite (corpo+cabeca) e maior e "vaza" pra cima. Quadrado (PNG
        // 68x68): largura = altura. Altura = N tiles; base do sprite = base da AABB;
        // centrado em X sobre a AABB.
        const int di = static_cast<int>(facing_);
        const int frame = walk_.current_frame();  // kNeutralFrame = parado
        gus::platform::render2d::TextureId tex =
            (frame == WalkCycle::kNeutralFrame || frame < 0 || frame >= kWalkFrameCount)
                ? sprites_.idle[di]
                : sprites_.walk[di][frame];
        // Se faltar o quadro de walk (so idle carregado), cai pro idle.
        if (tex == gus::platform::render2d::kInvalidTexture) {
            tex = sprites_.idle[di];
        }

        const float sprite_h = tuning_.player_sprite_height_tiles * grid_.tile_size();
        const float sprite_w = sprite_h;  // PNG quadrado
        const float sx = shown.x + shown.w * 0.5f - sprite_w * 0.5f;  // centrado em X
        const float sy = shown.y + shown.h - sprite_h;  // base do sprite = base da AABB
        const gus::core::spatial::Rect sprite_rect{sx, sy, sprite_w, sprite_h};
        const gus::platform::render2d::UvRect uv{0.0f, 0.0f, 1.0f, 1.0f};
        const gus::platform::render2d::DrawColor white{1.0f, 1.0f, 1.0f, 1.0f};
        renderer.draw_textured_rect(sprite_rect, tex, uv, white);
    } else {
        // FALLBACK (headless/smoke ou "sem arte"): contorno da hitbox.
        const gus::core::spatial::Rect player_rect{shown.x, shown.y, shown.w, shown.h};
        renderer.draw_rect_outline(player_rect, tuning_.player_color,
                                   tuning_.player_outline_world);
    }

    renderer.end_frame();
}

}  // namespace gus::app::screens
