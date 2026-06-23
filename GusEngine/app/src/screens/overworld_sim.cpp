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
                              tuning.anim_run_px_per_frame}),
      idle_clock_(/*frame_count=*/1, tuning.idle_fps_for_loop(1)),
      stamina_(gus::core::player::StaminaConfig{
          tuning.stamina_max, tuning.run_drain_per_sec,
          tuning.recover_walk_per_sec, tuning.recover_idle_per_sec,
          tuning.tired_threshold}),
      breath_(tuning.idle_calm_breaths_per_minute) {}

void OverworldSim::set_player_sprites(const PlayerSpriteSet& sprites) noexcept {
    sprites_ = sprites;
    // Reconfigura as animacoes pelo numero REAL de quadros da arte recebida, sem
    // tocar no feel (px_per_frame do walk continua do tuning; fps do idle idem).
    // Ciclo unico do personagem: o WalkCycle usa o MAIOR walk_count entre as direcoes
    // (Caua 4, Gus 7); o render so indexa walk[dir][frame] dentro do que existe.
    walk_ = WalkCycle(WalkCycle::Config{tuning_.anim_walk_px_per_frame,
                                        tuning_.anim_run_px_per_frame},
                      sprites_.max_walk_count());
    // O loop de breathing tem agora o N REAL de quadros (Gus 5, Caua 1). Reconta o
    // frame_count E re-deriva o fps a partir dos ciclos/min do tuning: um loop inteiro
    // deve durar 60/bpm s, logo fps = N * bpm / 60 (idle_fps_for_loop). Sem isso o fps
    // ficaria no init de loop=1 e a respiracao sairia errada.
    const int idle_loop = sprites_.max_idle_count();
    idle_clock_.set_frame_count(idle_loop);
    idle_clock_.set_fps(tuning_.idle_fps_for_loop(idle_loop));
}

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

    // IDLE OFEGANTE (breathing rapido) e a respiracao CALMA procedural tocam por TEMPO,
    // sempre - so um deles e MOSTRADO quando parado (decide a stamina). Avancar tambem
    // andando mantem ambos vivos e evita "pulo" ao voltar pro idle.
    idle_clock_.advance(fixed_dt);
    breath_.advance(fixed_dt);

    // CARGA DO APARATO (seam de 3 estados, canon 2026-06-23 - stamina.md):
    //   - Running (Shift + movimento real): DRENA. "Correr no lugar" preso na parede
    //     NAO conta como sprint (sem movimento de input -> nao esgota a Carga).
    //   - Walking (anda sem Shift, ou Shift sem mover): regenera DEVAGAR.
    //   - Idle (parado): regenera RAPIDO.
    // O movimento real (apos colidir) e apurado abaixo; aqui usamos a INTENCAO de input
    // (dx,dy) + Shift, coerente com o resto do feel (encostar na parede nao zera a Carga).
    const bool moving_now = (dx != 0 || dy != 0);
    gus::core::player::MoveState move_state;
    if (run && moving_now) {
        move_state = gus::core::player::MoveState::Running;
    } else if (moving_now) {
        move_state = gus::core::player::MoveState::Walking;
    } else {
        move_state = gus::core::player::MoveState::Idle;
    }
    stamina_.tick(move_state, fixed_dt);

    if (dx == 0 && dy == 0) {
        // Parado: prev == curr (render nao interpola). A direcao MANTEM a ultima
        // (idle nao gira o boneco); o ciclo de walk volta ao neutro (idle congelado).
        // Memoria de input zerada: o proximo movimento sera "eixo recem-acionado".
        walk_.advance(0.0f, run);
        dx_prev_ = 0;
        dy_prev_ = 0;
        return;
    }

    // Direcao cardinal pela intencao de input (vetor cru), nao pelo movimento
    // resolvido: encostar na parede num eixo NAO deve girar o boneco. A politica de
    // diagonal (qual eixo manda o olhar) vem do tuning (ponto unico de feel). Passa a
    // MEMORIA DO INPUT do tick anterior (dx_prev_,dy_prev_) pra LastAxisWins decidir o
    // eixo recem-acionado pela mudanca do INPUT - estavel na diagonal sustentada
    // (sem flicker), em vez de derivar do facing anterior.
    facing_ = direction_from_move(dx, dy, dx_prev_, dy_prev_, facing_,
                                  tuning_.diagonal_facing);

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

    // Guarda o INPUT deste tick pra o proximo decidir o eixo recem-acionado.
    dx_prev_ = dx;
    dy_prev_ = dy;
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
        const bool moving =
            (frame != WalkCycle::kNeutralFrame && frame >= 0 &&
             frame < sprites_.walk_count[di]);

        // IDLE EM DOIS MODOS por STAMINA (lider 2026-06-23): CALMO quando descansado
        // (senoide procedural no quadro NEUTRO, sem trocar frame), OFEGANTE quando
        // cansado (troca os quadros do breathing rapido). So vale quando PARADO.
        const bool tired = stamina_.is_tired();
        // calm_breath != 0 so quando PARADO e DESCANSADO: liga o bob/escala procedural.
        bool calm_breathing = false;

        gus::platform::render2d::TextureId tex;
        if (moving) {
            // ANDANDO: quadro f de walk da direcao (Gus 7 / Caua 4 quadros).
            tex = sprites_.walk[di][frame];
        } else if (tired) {
            // PARADO + CANSADO: IDLE OFEGANTE. O AnimClock cicla os quadros do breathing
            // RAPIDO (Gus 5 ~6 fps), comunicando fadiga. Com 1 quadro (Caua) fica
            // congelado. Clampa o indice ao que existe (degrada pro frame 0 = idle[di]).
            int idle_f = idle_clock_.frame();
            if (idle_f < 0 || idle_f >= sprites_.idle_count[di]) {
                idle_f = 0;
            }
            tex = sprites_.idle_frames[di][idle_f];
        } else {
            // PARADO + DESCANSADO: IDLE CALMO. Quadro NEUTRO (frame 0) + respiracao
            // PROCEDURAL (senoide) aplicada no transform do desenho abaixo - FLUIDO,
            // SEM staccato (nao troca frame).
            tex = sprites_.idle_frames[di][0];
            calm_breathing = true;
        }
        // Se o quadro escolhido faltar (slot invalido), cai pro idle representativo.
        if (tex == gus::platform::render2d::kInvalidTexture) {
            tex = sprites_.idle[di];
        }

        const float sprite_h = tuning_.player_sprite_height_tiles * grid_.tile_size();
        const float sprite_w = sprite_h;  // PNG quadrado
        const float sx = shown.x + shown.w * 0.5f - sprite_w * 0.5f;  // centrado em X
        // ANCORAGEM PELOS PES (M1-BUG.SUL): desce o desenho ate o PE REAL coincidir
        // com a base da AABB. A margem inferior transparente do sprite IDLE daquela
        // direcao (foot.bottom_fraction, MEDIDA do alpha pelo loader) sobe a base do
        // canvas o tanto da sobra; o ajuste fino manual do lider
        // (sprite_foot_offset_tiles, default 0) e SOMADO por cima. Automatico = padrao;
        // manual = so refino opcional. base AABB = shown.y + shown.h.
        const float bottom_fraction = sprites_.foot.for_direction(facing_);
        const float manual_offset = tuning_.sprite_foot_offset_tiles * grid_.tile_size();
        const float sy = sprite_top_y(shown.y + shown.h, sprite_h, bottom_fraction,
                                      manual_offset);

        // RESPIRACAO CALMA PROCEDURAL (idle descansado): aplica um bob/escala SENOIDAL
        // suave no desenho do quadro NEUTRO - sem trocar frame, sem staccato. A senoide
        // (breath_.value() em [-1,1]) modula:
        //   - ESCALA vertical: estica/encolhe o sprite (peito subindo) mantendo o PE
        //     plantado (cresce SO pra cima: a base fica na ancoragem dos pes);
        //   - BOB: pequeno sobe-desce do desenho inteiro.
        // So no idle calmo; andando/ofegante o transform e identidade (intacto).
        float draw_y = sy;
        float draw_h = sprite_h;
        if (calm_breathing) {
            const float osc = breath_.value();  // [-1, 1], continuo no tempo
            // Escala vertical: 1 + amp * osc (amp em fracao, ex.: 0.025 = +-2.5%).
            const float scaled_h = sprite_h * (1.0f + tuning_.idle_calm_scale_amplitude * osc);
            // Cresce/encolhe pela base (pe plantado): topo sobe o tanto que a altura
            // aumentou (base = sy + sprite_h preservada).
            draw_y = (sy + sprite_h) - scaled_h;
            draw_h = scaled_h;
            // BOB: sobe-desce o desenho inteiro (amplitude em tiles -> mundo).
            const float bob = tuning_.idle_calm_bob_tiles * grid_.tile_size() * osc;
            draw_y -= bob;  // osc>0 (inspirado) sobe um pouquinho
        }
        const gus::core::spatial::Rect sprite_rect{sx, draw_y, sprite_w, draw_h};
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
