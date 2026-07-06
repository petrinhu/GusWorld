// gus/app/src/screens/overworld_sim.cpp
//
// Implementacao do OverworldSim (M1). Ver header. Travado por
// app/tests/overworld_sim_test.cpp (TEST-FIRST).

#include "gus/app/screens/overworld_sim.hpp"

#include <cmath>    // std::sqrt
#include <cstdint>  // std::uint16_t (tile-id do TileMap)
#include <utility>  // std::move

#include "gus/core/spatial/depth_sort.hpp"  // Y-SORT (M7-COSTURA): ordem de desenho por profundidade

namespace gus::app::screens {

namespace {

// 1/sqrt(2): fator de normalizacao do passo diagonal (modulo do vetor (1,1)).
constexpr float kInvSqrt2 = 0.70710678f;

// true se os retangulos de mundo a e b se sobrepoem (meio-aberto).
bool overlaps(const gus::core::spatial::Rect& a, const gus::core::spatial::Rect& b) {
    return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}

// Converte a cadencia do walk de FRACAO DE TILE (tuning, imune a escala) para UNIDADES
// DE MUNDO por troca de quadro (o que o WalkCycle consome, ja em unidades de mundo, do
// mesmo jeito que o "moved" do step_fixed). Multiplica pelo tile_size REAL do mapa:
// assim o passo do sprite acompanha a escala (tile 16 do M1 OU tile 2.0 do .gmap) e o
// Gus da passos visiveis e na cadencia natural em qualquer escala. tile_size degenerado
// (<= 0) cai em 1.0 para nao zerar o passo (fica equivalente a fracao crua).
WalkCycle::Config make_walk_config(const OverworldTuning& t,
                                   const gus::core::spatial::TileGrid& grid) noexcept {
    const float ts = grid.tile_size() > 0.0f ? grid.tile_size() : 1.0f;
    // coast_seconds e em TEMPO (s), imune a escala: nao multiplica pelo tile_size.
    return WalkCycle::Config{t.anim_walk_tiles_per_frame * ts,
                             t.anim_run_tiles_per_frame * ts,
                             t.anim_walk_coast_seconds};
}

}  // namespace

gus::core::spatial::Aabb OverworldSim::solid_obstacle_from_footprint(
    const gus::core::spatial::Aabb& footprint) const noexcept {
    // MESMA ancoragem do feet_trigger_aabb (maestro_logic.hpp): centro em X sobre o
    // footprint, base = base do footprint - so o TAMANHO difere (ver rationale em
    // overworld_tuning.hpp::npc_solid_box_tiles).
    const float sz = tuning_.npc_solid_box_tiles * grid_.tile_size();
    gus::core::spatial::Aabb solid;
    solid.w = sz;
    solid.h = sz;
    solid.x = footprint.x + footprint.w * 0.5f - sz * 0.5f;
    solid.y = (footprint.y + footprint.h) - sz;
    return solid;
}

OverworldSim::OverworldSim(gus::core::spatial::TileGrid grid,
                           gus::core::spatial::Aabb player_start,
                           OverworldTuning tuning)
    : grid_(std::move(grid)),
      prev_(player_start),
      curr_(player_start),
      tuning_(tuning),
      walk_(make_walk_config(tuning, grid_)),
      idle_clock_(/*frame_count=*/1, tuning.idle_fps_for_loop(1)),
      stamina_(gus::core::player::StaminaConfig{
          tuning.stamina_max, tuning.run_drain_per_sec,
          tuning.recover_walk_per_sec, tuning.recover_idle_per_sec,
          tuning.tired_threshold}),
      breath_(tuning.idle_calm_breaths_per_minute),
      winded_(gus::core::player::WindedConfig{
          tuning.winded_min_seconds, tuning.winded_max_seconds,
          tuning.winded_run_for_max_seconds, tuning.winded_run_threshold_seconds}) {}

void OverworldSim::set_player_sprites(const PlayerSpriteSet& sprites) noexcept {
    sprites_ = sprites;
    // Reconfigura as animacoes pelo numero REAL de quadros da arte recebida, sem
    // tocar no feel (a cadencia do walk em fracao de tile do tuning, escalada pelo
    // tile_size do mapa, continua a mesma; fps do idle idem).
    // Ciclo unico do personagem: o WalkCycle usa o MAIOR walk_count entre as direcoes
    // (Caua 4, Gus 7); o render so indexa walk[dir][frame] dentro do que existe.
    walk_ = WalkCycle(make_walk_config(tuning_, grid_), sprites_.max_walk_count());
    // O loop de breathing tem agora o N REAL de quadros (Gus 5, Caua 1). Reconta o
    // frame_count E re-deriva o fps a partir dos ciclos/min do tuning: um loop inteiro
    // deve durar 60/bpm s, logo fps = N * bpm / 60 (idle_fps_for_loop). Sem isso o fps
    // ficaria no init de loop=1 e a respiracao sairia errada.
    const int idle_loop = sprites_.max_idle_count();
    idle_clock_.set_frame_count(idle_loop);
    idle_clock_.set_fps(tuning_.idle_fps_for_loop(idle_loop));
}

OverworldSim::OverworldSim(gus::domain::map::TileMap map,
                           gus::core::spatial::Aabb player_start,
                           OverworldTuning tuning)
    // A colisao vem do TileGrid derivado do mapa (so Parede bloqueia, regra do
    // dominio). Delega ao ctor principal pra montar todo o estado de uma vez, depois
    // guarda o TileMap pro render pintar por TileKind.
    : OverworldSim(map.to_tile_grid(), player_start, tuning) {
    map_ = std::move(map);
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

    // FOLEGO DO CORPO (timer separado da Carga, lider 2026-06-23): acumula enquanto
    // CORRE; em qualquer outro estado (anda/parado) "para de correr" -> ao cruzar a
    // transicao dispara a ofegancia escalada (>= 5 s) e decai parado ate zerar. Isso
    // mantem o Gus ofegante por >= 5 s mesmo com a Carga ja recarregada. Dirigido pelo
    // MESMO move_state (correr = sprint real, com movimento de input).
    if (move_state == gus::core::player::MoveState::Running) {
        winded_.tick_running(fixed_dt);
    } else {
        winded_.tick_stopped(fixed_dt);
    }

    if (dx == 0 && dy == 0) {
        // Parado: prev == curr (render nao interpola). A direcao MANTEM a ultima
        // (idle nao gira o boneco). O ciclo de walk NAO corta seco pro neutro: a
        // HISTERESE (coast, sobrecarga com dt) segura o estado "andando" por um buffer
        // curto antes de cair pro idle - assim SPAMMAR a direcao (taps com micro-gaps)
        // mantem a anim rodando em vez de deslizar. Sem deslocamento real, o quadro e
        // segurado (nao marcha parado). So vai pro idle quando o buffer expira.
        // Memoria de input zerada: o proximo movimento sera "eixo recem-acionado".
        walk_.advance(0.0f, run, fixed_dt);
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

    // OBSTACULOS PONTUAIS (M7-COSTURA/M7-DIALOGO, colisao SOLIDA de NPC/inimigo):
    // enemy_solid_aabb_/npc_bertoldo_solid_aabb_ (derivados em set_enemy_marker/
    // set_npc_bertoldo_marker - ver solid_obstacle_from_footprint) entram como
    // "paredes pontuais" adicionais, NAO fazem parte da TileGrid estatica. Array
    // FIXO de no maximo 2 (sem heap, mesmo espirito data-driven do resto do sim).
    gus::core::spatial::Aabb obstacle_storage[2];
    int obstacle_count = 0;
    if (enemy_solid_aabb_.has_value()) {
        obstacle_storage[obstacle_count++] = *enemy_solid_aabb_;
    }
    if (npc_bertoldo_solid_aabb_.has_value()) {
        obstacle_storage[obstacle_count++] = *npc_bertoldo_solid_aabb_;
    }
    const gus::core::spatial::ObstacleSpan obstacles{obstacle_storage, obstacle_count};

    // Colisao que desliza nas paredes (resolucao por eixo: X depois Y), agora com
    // corner-assist quando ligado no tuning (escorrega na quina se ha abertura) E os
    // obstaculos pontuais acima (o jogador nunca ocupa a mesma posicao do NPC/
    // inimigo, mas contorna livre pelos tiles adjacentes).
    const gus::core::spatial::MoveResult r =
        gus::core::spatial::resolve_move_with_corner_assist(
            grid_, curr_, move_x, move_y, tuning_.corner, obstacles);
    curr_ = r.box;

    // Anima o walk pela distancia REALMENTE percorrida (apos a colisao): bater na
    // parede num eixo reduz o avanco e, portanto, a troca de quadro - o pe nao
    // "patina". Distancia euclidiana do deslocamento resolvido neste passo.
    const float adx = curr_.x - prev_.x;
    const float ady = curr_.y - prev_.y;
    const float moved = std::sqrt(adx * adx + ady * ady);
    // Sobrecarga com HISTERESE (coast): moved > 0 avanca os quadros e recarrega o
    // buffer; moved == 0 (preso na parede num eixo, sem deslocamento real) gasta o
    // buffer como no ramo parado - segura o estado por um instante e depois cai pro
    // idle (parede = parado). Cura o deslize do spam mantendo o resto do feel.
    walk_.advance(moved, run, fixed_dt);

    // Guarda o INPUT deste tick pra o proximo decidir o eixo recem-acionado.
    dx_prev_ = dx;
    dy_prev_ = dy;
}

float OverworldSim::px_per_world_unit() const noexcept {
    // Zoom em px-por-TILE (tuning, intuicao do lider) -> px-por-UNIDADE de mundo,
    // dividindo pelo tile_size real do mapa (o .gmap da cidade usa 2.0). Guarda em
    // tile_size degenerado: cai pro proprio px-por-tile (1 unidade == 1 tile).
    const float ts = grid_.tile_size();
    if (ts <= 0.0f) {
        return tuning_.camera_zoom_px_per_tile;
    }
    return tuning_.camera_zoom_px_per_tile / ts;
}

gus::core::spatial::CameraView OverworldSim::camera_view(
    float viewport_px_w, float viewport_px_h) const noexcept {
    const float map_w = static_cast<float>(grid_.width()) * grid_.tile_size();
    const float map_h = static_cast<float>(grid_.height()) * grid_.tile_size();
    const gus::core::spatial::Vec2 center{curr_.x + curr_.w * 0.5f,
                                          curr_.y + curr_.h * 0.5f};
    // ZOOM (M4-BUG.CAMERA): converte os PIXELS da viewport em UNIDADES DE MUNDO pelo
    // zoom (px-por-unidade). Antes os pixels iam crus pro clamp como se fossem mundo,
    // e o mapa de 60x40 unidades cabia inteiro num retangulo minusculo. Agora a visao
    // mostra so a porcao ao redor do Gus, AMPLIADA, e o clamp ao mapa segue valendo.
    const float ppu = px_per_world_unit();
    const float world_w = gus::core::spatial::world_span_from_pixels(viewport_px_w, ppu);
    const float world_h = gus::core::spatial::world_span_from_pixels(viewport_px_h, ppu);
    return gus::core::spatial::clamp_camera(center, world_w, world_h, map_w, map_h);
}

gus::core::spatial::Aabb OverworldSim::interpolated_player(float alpha) const noexcept {
    // lerp(prev, curr, alpha). w/h preservados (nao mudam).
    gus::core::spatial::Aabb p = curr_;
    p.x = prev_.x + (curr_.x - prev_.x) * alpha;
    p.y = prev_.y + (curr_.y - prev_.y) * alpha;
    return p;
}

void OverworldSim::render(gus::platform::render2d::IRenderer& renderer,
                          float viewport_px_w, float viewport_px_h, float alpha,
                          float screen_px_w, float screen_px_h) const {
    // Camera centrada no jogador INTERPOLADO (camera segue suave junto do boneco).
    const gus::core::spatial::Aabb shown = interpolated_player(alpha);
    const float map_w = static_cast<float>(grid_.width()) * grid_.tile_size();
    const float map_h = static_cast<float>(grid_.height()) * grid_.tile_size();
    const gus::core::spatial::Vec2 cam_center{shown.x + shown.w * 0.5f,
                                              shown.y + shown.h * 0.5f};
    // ZOOM (M4-BUG.CAMERA): a visao da camera (em MUNDO) vem dos PIXELS da viewport
    // divididos pelo zoom (px-por-unidade). begin_frame recebe os PIXELS REAIS pra a
    // projecao mundo->tela (build_quad_screen) mapear a visao-mundo na janela cheia.
    const float ppu = px_per_world_unit();
    const float world_w =
        gus::core::spatial::world_span_from_pixels(viewport_px_w, ppu);
    const float world_h =
        gus::core::spatial::world_span_from_pixels(viewport_px_h, ppu);
    const gus::core::spatial::CameraView view = gus::core::spatial::clamp_camera(
        cam_center, world_w, world_h, map_w, map_h);

    // DUNGEON-SCALING: screen_px_w/h (se>0) e o tamanho REAL da janela - o retangulo de
    // camera (calculado acima a partir de viewport_px_w/h, o zoom LOGICO) e mapeado pra
    // ele em vez de pra viewport_px_w/h. Sentinela <=0 (default) preserva o legado
    // (screen == viewport, o MESMO tamanho pros dois papeis).
    const float out_w = screen_px_w > 0.0f ? screen_px_w : viewport_px_w;
    const float out_h = screen_px_h > 0.0f ? screen_px_h : viewport_px_h;
    renderer.begin_frame(view.rect, static_cast<int>(out_w), static_cast<int>(out_h));

    // TILES: so as celulas que intersectam a janela da camera (culling simples; a
    // camera ja clampa). Duas vias:
    //   (a) MAPA REAL (map_ presente): pinta CADA celula da grade pela cor do seu
    //       TileKind (graybox) via a TilePalette - Chao/Parede/Marco/Entrada/Saida.
    //       Legivel pro blockout; arte de tileset vem depois e isto sai.
    //   (b) FALLBACK (so TileGrid, sem TileMap): comportamento legado do M1 - desenha
    //       so as celulas BLOQUEADAS na wall_color (cena de teste / mapa que nao
    //       carregou). A borda externa do mapa e parede implicita (sem celula).
    const float ts = grid_.tile_size();
    for (int cy = 0; cy < grid_.height(); ++cy) {
        for (int cx = 0; cx < grid_.width(); ++cx) {
            gus::core::spatial::Rect cell{static_cast<float>(cx) * ts,
                                          static_cast<float>(cy) * ts, ts, ts};
            if (!overlaps(cell, view.rect)) {
                continue;
            }
            if (map_.has_value()) {
                const std::uint16_t tile_id = map_->at(cx, cy);
                renderer.draw_filled_rect(cell, color_for_tile(palette_, tile_id));
            } else if (grid_.is_blocked(cx, cy)) {
                renderer.draw_filled_rect(cell, tuning_.wall_color);
            }
        }
    }

    // Y-SORT (M7-COSTURA colisao solida + profundidade, playtest ao vivo do lider:
    // "o Gus anda POR CIMA/ATRAVES do Bertoldo, o NPC fica escondido debaixo do
    // sprite ao aproximar pelo norte"). ANTES (BUG): ordem de desenho FIXA (sempre
    // inimigo -> NPC -> jogador, o jogador SEMPRE por cima) - correto so enquanto o
    // jogador se aproxima "de baixo" (Zelda/Stardew: quem esta mais "embaixo" na
    // tela, Y maior/mais perto da camera, fica na FRENTE). Com a colisao SOLIDA nova
    // (ver ObstacleSpan/enemy_solid_aabb_/npc_bertoldo_solid_aabb_ no header) o
    // jogador NUNCA mais ocupa a MESMA posicao do NPC/inimigo, mas pode ficar
    // ADJACENTE (ao lado) - a ORDEM de desenho entao PRECISA responder a posicao
    // relativa em vez de ser fixa. Os 3 desenhaveis (inimigo, NPC, jogador) entram
    // na MESMA lista ordenavel por profundidade (base/pe = aabb.y+aabb.h); a logica
    // de CADA desenho fica INTACTA (mesmas formulas de sempre) - so a SEQUENCIA de
    // invocacao muda. depth_sort.hpp e POCO puro (core/spatial), testado sem GL em
    // depth_sort_test.cpp.
    enum class DrawableId : int { kEnemy = 0, kNpc = 1, kPlayer = 2 };

    const bool draw_enemy_marker =
        has_enemy_marker() &&
        overlaps(gus::core::spatial::Rect{enemy_marker_aabb_->x, enemy_marker_aabb_->y,
                                          enemy_marker_aabb_->w, enemy_marker_aabb_->h},
                view.rect);
    const bool draw_npc_marker =
        has_npc_bertoldo_marker() &&
        overlaps(gus::core::spatial::Rect{
                     npc_bertoldo_marker_aabb_->x, npc_bertoldo_marker_aabb_->y,
                     npc_bertoldo_marker_aabb_->w, npc_bertoldo_marker_aabb_->h},
                view.rect);

    // MARCADOR DE INIMIGO FIXO (M7-COSTURA Inc 2): o placeholder do androide (a MESMA
    // textura que a tela de BATALHA usa pros inimigos). MESMA escala/ancoragem do
    // sprite do Gus (player_sprite_height_tiles): quad quadrado, centrado em X sobre a
    // AABB do inimigo, base do quad = base da AABB (sem foot-inset - e um busto/icone,
    // nao um sprite de corpo com pes medidos). Culling ja resolvido acima
    // (draw_enemy_marker); kInvalidTexture/sem AABB (ver has_enemy_marker) => nada e
    // desenhado (fallback seguro).
    const auto do_draw_enemy_marker = [&]() {
        const gus::core::spatial::Aabb& ea = *enemy_marker_aabb_;
        const float esprite_h =
            tuning_.player_sprite_height_tiles * grid_.tile_size();
        const float esprite_w = esprite_h;  // retrato quadrado
        const float ex = ea.x + ea.w * 0.5f - esprite_w * 0.5f;
        const float ey = sprite_top_y(ea.y + ea.h, esprite_h,
                                      /*bottom_fraction=*/0.0f,
                                      /*manual_offset_world=*/0.0f);
        const gus::core::spatial::Rect enemy_rect{ex, ey, esprite_w, esprite_h};
        const gus::platform::render2d::UvRect euv{0.0f, 0.0f, 1.0f, 1.0f};
        const gus::platform::render2d::DrawColor ewhite{1.0f, 1.0f, 1.0f, 1.0f};
        renderer.draw_textured_rect(enemy_rect, enemy_marker_tex_, euv, ewhite);
    };

    // MARCADOR DO NPC BERTOLDO (M7-DIALOGO, NPC-MVP): sprite ESTATICO (Seu Bertoldo
    // Caim, pose "south" - de frente pro jogador/camera, sem locomocao) na posicao
    // fixa calculada pela Maestro (npc_bertoldo_aabb_). MESMA formula/culling do
    // marcador de inimigo acima ("busto simples": quad quadrado, centrado em X,
    // base do quad = base da AABB, sem foot-inset - o NPC nao anda), so que com
    // ESCALA PROPRIA (tuning_.npc_bertoldo_sprite_height_tiles, NAO player_sprite_
    // height_tiles - FIX BUG do lider "Bertoldo menor que o Gus": o retrato do
    // Bertoldo tem margem transparente maior que o do Gus, reusar a mesma altura-
    // de-canvas do jogador fazia o adulto renderizar mais baixo que a crianca; ver
    // o comentario completo em overworld_tuning.hpp). Slot PROPRIO
    // (npc_bertoldo_marker_*), independente do marcador de inimigo. `na` (a AABB
    // recebida) e EXATAMENTE a mesma que a Maestro usa pra disparar o dialogo
    // (aabb_overlaps) - o quad abaixo e derivado dela pela MESMA formula
    // (sprite_top_y) que a Maestro usou pra computa-la (enemy_sprite_footprint_
    // aabb), entao trigger e visual COINCIDEM por construcao (nao ha 2 formulas
    // divergentes pra manter em sincronia).
    const auto do_draw_npc_marker = [&]() {
        const gus::core::spatial::Aabb& na = *npc_bertoldo_marker_aabb_;
        const float nsprite_h =
            tuning_.npc_bertoldo_sprite_height_tiles * grid_.tile_size();
        const float nsprite_w = nsprite_h;  // sprite quadrado
        const float nx = na.x + na.w * 0.5f - nsprite_w * 0.5f;
        const float ny = sprite_top_y(na.y + na.h, nsprite_h,
                                      /*bottom_fraction=*/0.0f,
                                      /*manual_offset_world=*/0.0f);
        const gus::core::spatial::Rect npc_rect{nx, ny, nsprite_w, nsprite_h};
        const gus::platform::render2d::UvRect nuv{0.0f, 0.0f, 1.0f, 1.0f};
        const gus::platform::render2d::DrawColor nwhite{1.0f, 1.0f, 1.0f, 1.0f};
        renderer.draw_textured_rect(npc_rect, npc_bertoldo_marker_tex_, nuv, nwhite);
    };

    // Monta as entradas ordenaveis (so as ATIVAS/visiveis - o jogador SEMPRE entra).
    // depth_key = base/pe (y+h): MAIOR = mais "embaixo" na tela = desenha por
    // ULTIMO = fica na FRENTE (ver depth_sort.hpp).
    gus::core::spatial::DepthEntry depth_entries[3];
    int depth_count = 0;
    if (draw_enemy_marker) {
        depth_entries[depth_count++] = {enemy_marker_aabb_->y + enemy_marker_aabb_->h,
                                        static_cast<int>(DrawableId::kEnemy)};
    }
    if (draw_npc_marker) {
        depth_entries[depth_count++] = {
            npc_bertoldo_marker_aabb_->y + npc_bertoldo_marker_aabb_->h,
            static_cast<int>(DrawableId::kNpc)};
    }
    depth_entries[depth_count++] = {shown.y + shown.h,
                                    static_cast<int>(DrawableId::kPlayer)};
    gus::core::spatial::sort_by_depth(depth_entries, depth_count);

    // Desenha na ORDEM ja resolvida por profundidade. O bloco do jogador (sprite OU
    // fallback de contorno) e chamado atraves da MESMA lambda de sempre - a logica de
    // escolha de quadro/anim/respiracao fica 100% intacta, so a POSICAO na sequencia
    // de desenho muda.
    const auto do_draw_player = [&]() {
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
        // OFEGANTE = Carga baixa (overflow do aparato) OU folego do corpo ainda ativo
        // (timer separado, lider 2026-06-23): o Gus ofega por >= 5 s ao parar de correr,
        // mesmo com a Carga ja recarregada. Une as duas leituras (show_winded_idle).
        const bool tired = show_winded_idle();
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
    };  // fim de do_draw_player

    for (int i = 0; i < depth_count; ++i) {
        switch (static_cast<DrawableId>(depth_entries[i].id)) {
            case DrawableId::kEnemy:
                do_draw_enemy_marker();
                break;
            case DrawableId::kNpc:
                do_draw_npc_marker();
                break;
            case DrawableId::kPlayer:
                do_draw_player();
                break;
        }
    }

    renderer.end_frame();
}

}  // namespace gus::app::screens
