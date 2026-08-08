// SPDX-License-Identifier: Apache-2.0
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

// Idem para duas Aabb (mesma convencao meio-aberta do overlaps_aabb interno do
// resolve_move - encostar NAO e sobrepor).
bool aabb_overlaps(const gus::core::spatial::Aabb& a,
                   const gus::core::spatial::Aabb& b) noexcept {
    return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}

// Comprimento de uma perna da rota, em CELULAS.
float leg_length_cells(const gus::domain::world::PatrolRoute& route, int from,
                       int to) noexcept {
    const float dx = route.waypoints[to].x - route.waypoints[from].x;
    const float dy = route.waypoints[to].y - route.waypoints[from].y;
    return std::sqrt(dx * dx + dy * dy);
}

// Quao longe (em CELULAS) o ator pode estar da rota e ainda ser considerado EM
// CIMA dela. Ver rearm_patrol: dentro desta folga a ronda comeca no ponto da rota
// mais proximo dele; fora dela, vale a degradacao legada (a rota como
// deslocamento cru a partir do waypoint 0), que existe para o caso de a tabela do
// level design ter apontado uma celula que nao e a do ator.
constexpr float kOnRouteToleranceCells = 1.0f;

// Passo pedido pela rota abaixo disto (em unidades de mundo) conta como "a rota
// nao pediu movimento nenhum" - fim de perna, pausa, rota degenerada. Serve para
// nao classificar ruido de float como bloqueio.
constexpr float kPatrolNoStepSq = 1e-8f;

// Folga na fracao do passo cumprida: abaixo de (1 - isto) o ator conta como
// BARRADO no quadro. Larga o bastante para nao acusar arredondamento de float.
constexpr float kPatrolProgressEps = 1e-3f;

// A borda em p+size e EXCLUSIVA: o mesmo epsilon do overlaps_blocked do
// resolve_move, para a varredura de celulas nao vazar para a celula seguinte.
constexpr float kCellEdgeEps = 1e-4f;

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

// ===========================================================================
//  PECAS DE CENARIO e ATORES (DEMO-CIDADE-VESTIDA B1/B2/B3)
// ===========================================================================

int OverworldSim::add_scene_prop(const ScenePropInstance& prop) {
    props_.push_back(prop);
    return static_cast<int>(props_.size()) - 1;
}

void OverworldSim::clear_scene_props() noexcept { props_.clear(); }

bool OverworldSim::is_cell_dressed(int cx, int cy) const noexcept {
    if (cx < 0 || cy < 0) {
        return false;  // celula invalida nunca esta vestida
    }
    // Varredura linear sobre a lista de pecas: uma cidade tem dezenas, e so as
    // celulas de Parede visiveis chegam aqui (o chamador ja filtrou). Se um dia
    // forem milhares, o indice entra AQUI, num lugar so.
    for (const ScenePropInstance& p : props_) {
        if (p.cell_x == cx && p.cell_y == cy && p.drawable()) {
            return true;
        }
    }
    return false;
}

gus::core::spatial::Aabb OverworldSim::actor_sprite_rect(
    const gus::core::spatial::Aabb& anchor,
    float sprite_height_tiles) const noexcept {
    const float h = (sprite_height_tiles > 0.0f
                         ? sprite_height_tiles
                         : tuning_.player_sprite_height_tiles) *
                    grid_.tile_size();
    gus::core::spatial::Aabb rect;
    rect.w = h;  // retrato quadrado (mesma convencao dos dois marcadores legados)
    rect.h = h;
    rect.x = anchor.x + anchor.w * 0.5f - rect.w * 0.5f;
    // sprite_top_y com bottom_fraction/offset zerados: e um busto/icone, nao um
    // sprite de corpo com pes medidos pelo alpha-bbox (o jogador tem isso, o
    // marcador nunca teve).
    rect.y = sprite_top_y(anchor.y + anchor.h, rect.h, /*bottom_fraction=*/0.0f,
                          /*manual_offset_world=*/0.0f);
    return rect;
}

void OverworldSim::rearm_patrol(
    WorldActor& actor,
    const gus::domain::world::PatrolRoute& route) const noexcept {
    actor.route = route;
    actor.patrol = gus::domain::world::PatrolState{};
    actor.blocked_seconds = 0.0f;
    // A rota e aplicada como DESLOCAMENTO a partir de onde o ator esta AGORA (ver
    // world_entities.hpp): armar/rearmar nunca teleporta ninguem, e um erro de
    // celula na tabela vira um desvio, nao um sumico.
    actor.route_origin_anchor = actor.anchor;
    actor.route_origin_point =
        route.count >= 1 ? route.waypoints[0] : gus::domain::world::PatrolWaypoint{};

    // =======================================================================
    //  D4 (CLIPPING-ATOR-RONDA-SEM-COLISAO, 2026-08-07): a ronda comeca no ponto
    //  da rota MAIS PROXIMO do ator, e nao no waypoint 0.
    //
    //  O DEFEITO QUE ISTO CONSERTA, medido contra o .gmap real: a rota do demo
    //  nasce de build_horizontal_patrol_route(grid, celula_do_ator, alcance),
    //  que escreve waypoints[0] = celula - alcance. Como o estado zerado tambem
    //  comeca no waypoint 0, o par (origem_da_ancora, waypoint 0) casava a
    //  posicao do ator com a PONTA OESTE da rota - e ele passava a percorrer
    //  [celula, celula + 2*alcance] em vez de [celula - alcance, celula +
    //  alcance]. Um alcance inteiro de deslocamento, sistematico, em todo ator
    //  do jogo. O efeito colateral e grave e silencioso: a rota e conferida
    //  contra as paredes no trecho ESCRITO (city_patrol.hpp encurta onde bate),
    //  e essa garantia nao valia para o trecho ANDADO. No demo isso punha a
    //  Vanda em [30..38] (conferido [26..34]) e o androide da FIR em [74..86]
    //  (conferido [68..80]), sendo que a celula (86,50) e PAREDE - ele
    //  terminava a ida dentro de um predio. E a explicacao medida do "o
    //  androide entra e sai de objetos livremente" do playtest do lider.
    //
    //  A DEGRADACAO LEGADA FICA DE PE, e de proposito: se o ator estiver LONGE
    //  da rota (acima de kOnRouteToleranceCells), nada disto se aplica e vale o
    //  deslocamento cru a partir do waypoint 0. Esse e o caso que o comentario
    //  acima descreve - a tabela do level design apontou uma celula que nao e a
    //  do ator -, e ali "aplicar a rota como desvio" continua sendo a leitura
    //  certa: projetar teleportaria a ronda para um trilho que ninguem pediu.
    // =======================================================================
    const float ts = grid_.tile_size();
    if (!route.valid() || ts <= 0.0f) {
        return;
    }

    // Onde o ator esta, na MESMA unidade dos waypoints (indice de celula). O
    // centro do corpo cai no centro da celula (pick_actor_position_at_cell) e o
    // waypoint NOMEIA a celula, dai o -0.5.
    const float cx = (actor.anchor.x + actor.anchor.w * 0.5f) / ts - 0.5f;
    const float cy = (actor.anchor.y + actor.anchor.h * 0.5f) / ts - 0.5f;

    int best_leg = -1;
    float best_t = 0.0f;
    float best_len = 0.0f;
    float best_d2 = 0.0f;
    gus::domain::world::PatrolWaypoint best_point{};
    for (int i = 0; i + 1 < route.count; ++i) {
        const gus::domain::world::PatrolWaypoint& a = route.waypoints[i];
        const gus::domain::world::PatrolWaypoint& b = route.waypoints[i + 1];
        const float ex = b.x - a.x;
        const float ey = b.y - a.y;
        const float len2 = ex * ex + ey * ey;
        float t = 0.0f;
        if (len2 > 0.0f) {
            t = ((cx - a.x) * ex + (cy - a.y) * ey) / len2;
            if (t < 0.0f) {
                t = 0.0f;
            } else if (t > 1.0f) {
                t = 1.0f;
            }
        }
        const float px = a.x + ex * t;
        const float py = a.y + ey * t;
        const float d2 = (cx - px) * (cx - px) + (cy - py) * (cy - py);
        if (best_leg < 0 || d2 < best_d2) {
            best_leg = i;
            best_t = t;
            best_d2 = d2;
            best_len = std::sqrt(len2);
            best_point = gus::domain::world::PatrolWaypoint{px, py};
        }
    }
    if (best_leg < 0 ||
        best_d2 > kOnRouteToleranceCells * kOnRouteToleranceCells) {
        return;  // longe da rota: degradacao legada, intacta
    }
    actor.patrol.from_index = best_leg;
    actor.patrol.to_index = best_leg + 1;
    actor.patrol.traveled_tiles = best_t * best_len;
    actor.patrol.forward = true;
    actor.route_origin_point = best_point;
}

void OverworldSim::reverse_patrol(WorldActor& actor) const noexcept {
    const gus::domain::world::PatrolRoute& route = actor.route;
    if (!route.valid()) {
        return;
    }
    const int from = actor.patrol.from_index;
    const int to = actor.patrol.to_index;
    if (from < 0 || from >= route.count || to < 0 || to >= route.count ||
        from == to) {
        return;  // estado corrompido: o proprio advance_patrol reancora
    }
    // MEIA-VOLTA NO PONTO, sem teleporte: as pontas da perna corrente trocam de
    // papel e o que ja foi andado nela e REFLETIDO (o resto da perna vira o
    // andado). A posicao amostrada antes e depois disto e a MESMA - so o sentido
    // muda. `forward` acompanha para o ping-pong das pontas continuar coerente.
    const float len = leg_length_cells(route, from, to);
    float traveled = len - actor.patrol.traveled_tiles;
    if (traveled < 0.0f) {
        traveled = 0.0f;
    }
    actor.patrol.from_index = to;
    actor.patrol.to_index = from;
    actor.patrol.traveled_tiles = traveled;
    actor.patrol.forward = !actor.patrol.forward;
    actor.patrol.pause_left = 0.0f;
    // A ancora tem de continuar casando com a rota depois da troca de sentido: o
    // par (origem da ancora, ponto de origem) e reancorado AQUI, na posicao
    // corrente, senao o proximo quadro mediria o deslocamento a partir de um
    // ponto que a nova perna nao usa mais.
    const gus::domain::world::PatrolSample here =
        gus::domain::world::sample_patrol(route, actor.patrol);
    actor.route_origin_anchor = actor.anchor;
    actor.route_origin_point = gus::domain::world::PatrolWaypoint{here.x, here.y};
}

int OverworldSim::add_actor(const WorldActorSpec& spec) {
    WorldActor a;
    a.role = spec.role;
    a.anchor = spec.anchor;
    a.prev_anchor = spec.anchor;
    a.tex = spec.tex;
    a.sprite_height_tiles = spec.sprite_height_tiles > 0.0f
                                ? spec.sprite_height_tiles
                                : tuning_.player_sprite_height_tiles;
    a.solid = solid_obstacle_from_footprint(spec.anchor);
    a.active = true;
    rearm_patrol(a, spec.route);
    actors_.push_back(a);
    return static_cast<int>(actors_.size()) - 1;
}

void OverworldSim::set_actor_patrol(
    int handle, const gus::domain::world::PatrolRoute& route) noexcept {
    if (!valid_actor_handle(handle)) {
        return;
    }
    rearm_patrol(actors_[static_cast<std::size_t>(handle)], route);
}

void OverworldSim::remove_actor(int handle) noexcept {
    if (!valid_actor_handle(handle)) {
        return;
    }
    // O SLOT permanece (handle e indice; indice nao pode escorregar). Zera o que
    // faz o ator existir no mundo: desenho, colisao e ronda.
    WorldActor& a = actors_[static_cast<std::size_t>(handle)];
    a.active = false;
    a.tex = gus::platform::render2d::kInvalidTexture;
    a.solid = gus::core::spatial::Aabb{};
    a.route = gus::domain::world::PatrolRoute{};
}

std::optional<gus::core::spatial::Aabb> OverworldSim::actor_anchor(
    int handle) const noexcept {
    if (!valid_actor_handle(handle)) {
        return std::nullopt;
    }
    const WorldActor& a = actors_[static_cast<std::size_t>(handle)];
    if (!a.active) {
        return std::nullopt;
    }
    return a.anchor;
}

gus::core::spatial::ObstacleSpan OverworldSim::rebuild_obstacles(
    int skip_actor, bool include_player) noexcept {
    // Rascunho reaproveitado entre quadros: depois do aquecimento nao ha mais
    // alocacao no laco de jogo, so refill. UM SO para o passo inteiro (ronda de
    // cada ator + jogador) - quem chama por ultimo o reescreve, e ninguem guarda
    // o span entre chamadas.
    obstacle_scratch_.clear();
    for (const ScenePropInstance& p : props_) {
        if (p.blocks()) {
            obstacle_scratch_.push_back(p.solid);
        }
    }
    if (include_player) {
        // O corpo do jogador e a propria hitbox dele - a MESMA caixa que ele move.
        obstacle_scratch_.push_back(curr_);
    }
    for (std::size_t i = 0; i < actors_.size(); ++i) {
        if (static_cast<int>(i) == skip_actor) {
            continue;  // ninguem e obstaculo de si mesmo
        }
        if (actors_[i].blocks()) {
            obstacle_scratch_.push_back(actors_[i].solid);
        }
    }
    return gus::core::spatial::ObstacleSpan{
        obstacle_scratch_.data(), static_cast<int>(obstacle_scratch_.size())};
}

// ===========================================================================
//  RONDA COM COLISAO SOLIDA (CLIPPING-ATOR-RONDA-SEM-COLISAO, playtest ao vivo
//  do lider por Gus Dragon, 2026-08-07).
//
//  ANTES: esta funcao punha o ator na posicao que a rota mandava e pronto - sem
//  nunca chamar resolve_move. A resolucao contra obstaculos so rodava do lado do
//  JOGADOR, e so quando ELE se movia. Consequencias vistas no playtest: o NPC em
//  ronda andava POR CIMA do jogador parado (e a sobreposicao ficava de pe ate ele
//  apertar uma direcao livre - com o jogador encostado num prop, "preso"), e o
//  androide entrava e saia de objetos livremente.
//
//  AGORA: a posicao que a rota pede vira um DESLOCAMENTO, que passa pelo mesmo
//  resolve_move do jogador, contra a grade + as pecas + o jogador + os demais
//  atores.
//
//  QUAL CAIXA SE MOVE: a caixa SOLIDA do ator - a MESMA que os outros enxergam
//  dele. Nao a ancora. Isso e o que mantem a fisica simetrica: um corpo tem UM
//  tamanho. Mover a ancora (0.6 tile) enquanto os outros veem o solido (1 tile)
//  deixaria o ator encostar a ancora no jogador e, com isso, meter 0.2 tile do
//  corpo dentro dele - o mesmo defeito de novo, so que menor e mais dificil de
//  ver. A ancora acompanha pelo deslocamento REALMENTE cumprido.
//
//  SEM CORNER-ASSIST, de proposito: o corner-assist existe para perdoar a
//  imprecisao de quem TECLA (empurra o corpo de lado para escorregar na quina).
//  Um ator scripted nao tecla, e o empurrao lateral o tiraria da linha da rota -
//  justamente o que a politica "continua de onde parou" nao quer.
//
//  POLITICA AO SER BARRADO (D1, decisao do lider): "continua de onde parou, sem
//  pressa". O relogio da rota so avanca na fracao do passo que ele DE FATO
//  cumpriu, entao ele nunca acelera para recuperar atraso e nunca salta para
//  onde a rota diria que ele deveria estar. Barrado tempo demais (D2), da
//  meia-volta em vez de esperar para sempre.
// ===========================================================================
void OverworldSim::advance_actor_patrols(float fixed_dt) noexcept {
    const float ts = grid_.tile_size();
    for (std::size_t i = 0; i < actors_.size(); ++i) {
        WorldActor& a = actors_[i];
        // prev_anchor SEMPRE acompanha (mesmo parado): o render interpola entre os
        // dois, e um prev_ desatualizado faria o ator "saltar" ao comecar a andar.
        a.prev_anchor = a.anchor;
        if (!a.patrolling()) {
            continue;
        }

        const gus::core::spatial::ObstacleSpan obstacles =
            rebuild_obstacles(static_cast<int>(i), /*include_player=*/true);

        // O estado ANTES do passo: se o passo for barrado, o relogio volta para
        // ca e so a fracao cumprida e reaplicada.
        const gus::domain::world::PatrolState before = a.patrol;
        const gus::domain::world::PatrolSample s =
            gus::domain::world::advance_patrol(a.route, a.patrol, fixed_dt);

        // Onde a rota quer que ele esteja, como DESLOCAMENTO a partir de onde ele
        // esta (celulas -> unidades de mundo).
        const float want_x =
            a.route_origin_anchor.x + (s.x - a.route_origin_point.x) * ts - a.anchor.x;
        const float want_y =
            a.route_origin_anchor.y + (s.y - a.route_origin_point.y) * ts - a.anchor.y;

        const gus::core::spatial::MoveResult r = gus::core::spatial::resolve_move(
            grid_, a.solid, want_x, want_y, obstacles);
        const float got_x = r.box.x - a.solid.x;
        const float got_y = r.box.y - a.solid.y;
        a.anchor.x += got_x;
        a.anchor.y += got_y;

        const float want2 = want_x * want_x + want_y * want_y;
        if (want2 > kPatrolNoStepSq) {
            // Quanto do caminho PRETENDIDO foi cumprido: a projecao do que andou
            // sobre o que queria andar. Projecao, e nao razao de modulos, porque
            // o resolve_move DESLIZA - a componente perpendicular anda, mas nao e
            // progresso na rota.
            float progress = (got_x * want_x + got_y * want_y) / want2;
            if (progress < 0.0f) {
                progress = 0.0f;
            } else if (progress > 1.0f) {
                progress = 1.0f;
            }
            if (progress < 1.0f - kPatrolProgressEps) {
                // BARRADO: o relogio da rota volta e anda so a fracao cumprida.
                a.patrol = before;
                if (progress > 0.0f) {
                    gus::domain::world::advance_patrol(a.route, a.patrol,
                                                       fixed_dt * progress);
                }
                a.blocked_seconds += fixed_dt;
                if (a.blocked_seconds >= tuning_.actor_blocked_turnaround_seconds) {
                    reverse_patrol(a);
                    a.blocked_seconds = 0.0f;
                }
            } else {
                a.blocked_seconds = 0.0f;
            }
        } else {
            // A rota nao pediu passo nenhum (pausa na ponta, fim de perna): isso
            // NAO e estar barrado, e nao pode acumular para a meia-volta.
            a.blocked_seconds = 0.0f;
        }

        // O corpo solido acompanha: sem isto o inimigo andaria e deixaria a
        // colisao para tras (o jogador trombaria no ar e atravessaria o sprite).
        a.solid = solid_obstacle_from_footprint(a.anchor);
    }
}

// ===========================================================================
//  DEPENETRACAO (blindagem anti-exploit, pedido do lider 2026-08-07).
//
//  Quem achou o clipping estuda speedrun/glitch hunting, e "um corpo empurra o
//  jogador para dentro da parede, o jogador atravessa" e uma classe de exploit
//  documentada em jogos comerciais. A defesa padrao da industria nao tapa o caso
//  particular: e uma passada que, a CADA quadro e INDEPENDENTE de input, tira
//  qualquer corpo sobreposto a um bloqueador, empurrando pela MENOR distancia de
//  saida.
//
//  POR QUE A MENOR: e a unica escolha que nao inventa uma direcao. Empurrar por
//  um eixo fixo (ou pelo "lado de onde ele veio") atravessa a parede quando a
//  penetracao daquele lado e maior que a espessura do bloqueador - que e
//  exatamente o atravessamento que se quer barrar.
//
//  NO-OP NO QUADRO NORMAL: o resolve_move deixa o corpo com a borda COINCIDINDO
//  com a do obstaculo, e sobreposicao aqui e meio-aberta - encostar nao e
//  sobrepor. Sem isso, o jogador seria empurrado para longe de toda parede em
//  que encostasse, em todo quadro.
//
//  SO O JOGADOR, e isto e conclusao da implementacao, nao suposicao: os atores
//  ja passam pelo resolve_move acima com a MESMA caixa que apresentam aos
//  outros, entao nao entram em sobreposicao por movimento proprio. Aplicar a
//  mesma passada a eles seria ativamente ERRADO num caso legitimo do jogo - a
//  caixa solida do ator e maior que a ancora e ancorada pela base, entao ela
//  invade a celula de cima; um ator andando colado numa fachada teria o corpo
//  parcialmente dentro dela por construcao, e a depenetracao ficaria empurrando
//  ele para fora da propria rua, todo quadro.
// ===========================================================================
gus::core::spatial::Aabb OverworldSim::depenetrate(
    const gus::core::spatial::Aabb& box,
    gus::core::spatial::ObstacleSpan obstacles) const noexcept {
    // Sair de um bloqueador pode meter o corpo noutro (quina de dois props). O
    // teto existe para o pior caso (corpo enfiado bem fundo num amontoado) sair
    // com a posicao inalterada em vez de pendurar o passo fixo.
    constexpr int kMaxPasses = 4;

    const float ts = grid_.tile_size();
    gus::core::spatial::Aabb out = box;
    if (ts <= 0.0f || out.w <= 0.0f || out.h <= 0.0f) {
        return out;
    }

    for (int pass = 0; pass < kMaxPasses; ++pass) {
        float push_x = 0.0f;
        float push_y = 0.0f;
        float best = 0.0f;
        bool found = false;

        const auto consider = [&](const gus::core::spatial::Aabb& ob) noexcept {
            if (!aabb_overlaps(out, ob)) {
                return;
            }
            // As 4 saidas possiveis, com sinal. A de menor MODULO vence.
            const float cand[4] = {(ob.x + ob.w) - out.x,   // para leste
                                   ob.x - (out.x + out.w),  // para oeste
                                   (ob.y + ob.h) - out.y,   // para sul
                                   ob.y - (out.y + out.h)};  // para norte
            const bool on_x[4] = {true, true, false, false};
            for (int k = 0; k < 4; ++k) {
                const float mag = cand[k] < 0.0f ? -cand[k] : cand[k];
                // `>=` mantem o PRIMEIRO da ordem em caso de empate: a escolha
                // fica deterministica (o corpo no centro exato de um bloqueador
                // quadrado tem os 4 empatados).
                if (found && mag >= best) {
                    continue;
                }
                found = true;
                best = mag;
                push_x = on_x[k] ? cand[k] : 0.0f;
                push_y = on_x[k] ? 0.0f : cand[k];
            }
        };

        // (a) PAREDES DA GRADE sobrepostas, cada celula como um retangulo. A
        // borda do mapa e parede implicita (is_blocked cobre fora dos limites),
        // entao um corpo que vazou para fora tambem e trazido de volta.
        const int cx0 = grid_.world_to_cell(out.x);
        const int cy0 = grid_.world_to_cell(out.y);
        const int cx1 = grid_.world_to_cell(out.x + out.w - kCellEdgeEps);
        const int cy1 = grid_.world_to_cell(out.y + out.h - kCellEdgeEps);
        for (int cy = cy0; cy <= cy1; ++cy) {
            for (int cx = cx0; cx <= cx1; ++cx) {
                if (!grid_.is_blocked(cx, cy)) {
                    continue;
                }
                consider(gus::core::spatial::Aabb{static_cast<float>(cx) * ts,
                                                  static_cast<float>(cy) * ts, ts,
                                                  ts});
            }
        }
        // (b) OBSTACULOS PONTUAIS (pecas e corpos de personagem).
        for (int i = 0; i < obstacles.count; ++i) {
            consider(obstacles.items[i]);
        }

        if (!found) {
            break;  // livre - o caso de todo quadro normal, e a saida barata
        }
        out.x += push_x;
        out.y += push_y;
    }
    return out;
}

int OverworldSim::ensure_reserved_actor(
    int& handle, WorldActorRole role, const gus::core::spatial::Aabb& aabb,
    gus::platform::render2d::TextureId tex, float sprite_height_tiles) noexcept {
    if (!valid_actor_handle(handle)) {
        WorldActorSpec spec;
        spec.role = role;
        spec.anchor = aabb;
        spec.tex = tex;
        spec.sprite_height_tiles = sprite_height_tiles;
        handle = add_actor(spec);
        return handle;
    }
    WorldActor& a = actors_[static_cast<std::size_t>(handle)];
    const gus::domain::world::PatrolRoute route = a.route;  // preserva a ronda
    a.role = role;
    a.anchor = aabb;
    a.prev_anchor = aabb;
    a.tex = tex;
    a.sprite_height_tiles = sprite_height_tiles;
    a.solid = solid_obstacle_from_footprint(aabb);
    a.active = true;
    // REARMA a ronda na posicao nova: reposicionar acontece ao carregar um save, e
    // a ronda tem que recomecar dali - nao continuar medindo do lugar antigo.
    rearm_patrol(a, route);
    return handle;
}

void OverworldSim::set_enemy_marker(
    const gus::core::spatial::Aabb& aabb,
    gus::platform::render2d::TextureId tex) noexcept {
    ensure_reserved_actor(enemy_marker_handle_, WorldActorRole::Enemy, aabb, tex,
                          tuning_.player_sprite_height_tiles);
}

void OverworldSim::clear_enemy_marker() noexcept {
    remove_actor(enemy_marker_handle_);
}

bool OverworldSim::has_enemy_marker() const noexcept {
    return valid_actor_handle(enemy_marker_handle_) &&
           actors_[static_cast<std::size_t>(enemy_marker_handle_)].drawable();
}

void OverworldSim::set_npc_bertoldo_marker(
    const gus::core::spatial::Aabb& aabb,
    gus::platform::render2d::TextureId tex) noexcept {
    ensure_reserved_actor(npc_bertoldo_handle_, WorldActorRole::Npc, aabb, tex,
                          tuning_.npc_bertoldo_sprite_height_tiles);
}

void OverworldSim::clear_npc_bertoldo_marker() noexcept {
    remove_actor(npc_bertoldo_handle_);
}

bool OverworldSim::has_npc_bertoldo_marker() const noexcept {
    return valid_actor_handle(npc_bertoldo_handle_) &&
           actors_[static_cast<std::size_t>(npc_bertoldo_handle_)].drawable();
}

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

    // RONDA DOS ATORES (DEMO-CIDADE-VESTIDA B3) - LOGO NO INICIO, ANTES de
    // qualquer saida antecipada. A cidade tem que continuar viva com o jogador
    // parado: se isto ficasse depois do `return` do ramo "sem input", o inimigo so
    // andaria enquanto o jogador andasse - e ninguem notaria no playtest, porque o
    // jogador esta quase sempre andando. Tambem precisa vir antes da colisao do
    // jogador, para que os corpos solidos usados abaixo sejam os deste quadro.
    advance_actor_patrols(fixed_dt);

    // OBSTACULOS PONTUAIS do JOGADOR (M7-COSTURA/M7-DIALOGO, colisao SOLIDA de
    // NPC/inimigo; estendido em DEMO-CIDADE-VESTIDA B1/B2 para pecas de cenario e
    // N atores): entram como "paredes pontuais" adicionais, NAO fazem parte da
    // TileGrid estatica. Montados AQUI, antes de qualquer saida antecipada,
    // porque a depenetracao logo abaixo tambem precisa deles - e ela roda com o
    // jogador PARADO, que e justamente o caso do defeito.
    const gus::core::spatial::ObstacleSpan obstacles =
        rebuild_obstacles(/*skip_actor=*/kInvalidWorldActor, /*include_player=*/false);

    // DEPENETRACAO (blindagem anti-exploit; ver o metodo). Roda TODO quadro, com
    // ou sem tecla apertada: sobreposicao que so se resolve quando o jogador se
    // move foi exatamente o defeito do playtest. Depois do `prev_ = curr_` acima,
    // de proposito - assim o empurrao aparece interpolado no render em vez de
    // saltar. No quadro normal e um no-op (encostar nao e sobrepor).
    curr_ = depenetrate(curr_, obstacles);

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
                // A CELULA VESTIDA nao pinta (fatia E, achado A3 do laudo visual).
                // A caixa de cobertura nao fica AO LADO de um obstaculo: ela E a
                // celula de Parede. Pintar a parede de graybox atras dela deixa a
                // barricada "pairando sobre um buraco" escuro, porque o conteudo
                // do PNG termina 0,19 tile antes da base do quadro. Na leitura de
                // PRODUCAO a celula sai na cor do Chao e quem faz o papel de
                // parede ali e a arte; a COLISAO nao muda (a celula continua
                // Parede na TileGrid). Na leitura de BLOCKOUT a parede volta,
                // porque quem traça o nivel precisa ver o solido de verdade.
                const bool vestida =
                    palette_.reading == TileReading::Production &&
                    static_cast<gus::domain::map::TileKind>(tile_id) ==
                        gus::domain::map::TileKind::Parede &&
                    is_cell_dressed(cx, cy);
                renderer.draw_filled_rect(
                    cell, vestida ? palette_.chao : color_for_tile(palette_, tile_id));
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
    //
    // GENERALIZACAO (DEMO-CIDADE-VESTIDA B1/B2): a lista deixou de ser um array de
    // 3 posicoes com um enum de 3 valores. Agora entram o jogador, TODOS os atores
    // (N NPCs e N inimigos) e TODAS as pecas de cenario de pe - a casa cibergotica
    // esconde quem passa atras dela pela MESMA regra que o Bertoldo ja escondia.
    // O `id` de DepthEntry e opaco (ver depth_sort.hpp): aqui ele codifica QUEM
    // desenhar - jogador, peca `i` ou ator `i`.
    constexpr int kPlayerDepthId = -1;
    // Base do intervalo de ator. Separa os dois espacos de indice sem par de
    // vetores paralelos; o valor e folgado o bastante para nao colidir com
    // nenhuma contagem de pecas plausivel (uma cidade tem dezenas, nao um milhao).
    constexpr int kActorDepthIdBase = 1 << 20;

    const gus::platform::render2d::UvRect kFullUv{0.0f, 0.0f, 1.0f, 1.0f};
    const gus::platform::render2d::DrawColor kNoTint{1.0f, 1.0f, 1.0f, 1.0f};

    // PECAS DE CHAO (tabuleiro de puzzle, marcacao de calcada): pintadas JUNTO com
    // os tiles, antes de tudo. Nao entram no Y-sort de proposito - a base delas e
    // baixa na tela e um Y-sort ingenuo as desenharia POR CIMA do jogador que esta
    // pisando nelas.
    for (const ScenePropInstance& p : props_) {
        if (!p.ground || !p.drawable()) {
            continue;
        }
        const gus::core::spatial::Rect r{p.footprint.x, p.footprint.y, p.footprint.w,
                                         p.footprint.h};
        if (!overlaps(r, view.rect)) {
            continue;
        }
        renderer.draw_textured_rect(r, p.tex, kFullUv, kNoTint);
    }

    // Desenha UMA peca de cenario de pe (o retangulo ja vem resolvido do dado).
    const auto do_draw_prop = [&](int index) {
        const ScenePropInstance& p = props_[static_cast<std::size_t>(index)];
        renderer.draw_textured_rect(
            gus::core::spatial::Rect{p.footprint.x, p.footprint.y, p.footprint.w,
                                     p.footprint.h},
            p.tex, kFullUv, kNoTint);
    };

    // Desenha UM ator (NPC ou inimigo). Mesma formula que os dois marcadores
    // legados usavam separadamente - quad quadrado, centrado em X sobre a ancora,
    // base do quad na base dela, sem foot-inset (e um busto/icone, nao um sprite de
    // corpo com pes medidos pelo alpha-bbox). A ALTURA vem do proprio ator, o que
    // preserva a escala propria do Bertoldo (retrato com margem transparente maior
    // que a do Gus; reusar a altura do jogador fazia o adulto sair mais baixo que a
    // crianca, bug do playtest ao vivo).
    //
    // POSICAO INTERPOLADA entre o passo anterior e o atual, igual ao jogador: um
    // inimigo em ronda desenhado so na posicao do passo fixo andaria aos trancos
    // enquanto o jogador desliza liso.
    const auto do_draw_actor = [&](int index) {
        const WorldActor& a = actors_[static_cast<std::size_t>(index)];
        gus::core::spatial::Aabb anchor = a.anchor;
        anchor.x = a.prev_anchor.x + (a.anchor.x - a.prev_anchor.x) * alpha;
        anchor.y = a.prev_anchor.y + (a.anchor.y - a.prev_anchor.y) * alpha;
        const gus::core::spatial::Aabb rect =
            actor_sprite_rect(anchor, a.sprite_height_tiles);
        renderer.draw_textured_rect(
            gus::core::spatial::Rect{rect.x, rect.y, rect.w, rect.h}, a.tex, kFullUv,
            kNoTint);
    };

    // Monta as entradas ordenaveis (so as ATIVAS/visiveis - o jogador SEMPRE entra).
    //
    // CADA entrada leva DUAS coisas (ver depth_sort.hpp): a FATIA DE CHAO que o
    // desenhavel ocupa e a FAIXA LATERAL do DESENHO. Ate a fatia I ia so um numero
    // (a base/pe), e um numero nao consegue responder "quem esta atras de quem"
    // quando o desenho e MUITO maior que a celula: com scene_prop_scale 1,83 uma
    // casa de 3 celulas ocupa 5,49 de largura e cobre vizinho a duas celulas de
    // distancia, ainda que o vizinho esteja NIVELADO com a base dela.
    depth_scratch_.clear();
    for (std::size_t i = 0; i < props_.size(); ++i) {
        const ScenePropInstance& p = props_[i];
        if (p.ground || !p.drawable()) {
            continue;
        }
        const gus::core::spatial::Rect r{p.footprint.x, p.footprint.y, p.footprint.w,
                                         p.footprint.h};
        if (!overlaps(r, view.rect)) {
            continue;
        }
        gus::core::spatial::DepthEntry e;
        // A fatia de chao da peca e a CELULA em que ela foi plantada - a propria
        // definicao de "onde a peca pisa" (ver ScenePropPlacement) -, NAO o
        // retangulo alto do desenho, que sobe pelo ar e nao encosta em nada.
        //
        // POR QUE A CELULA E NAO A CAIXA QUE BLOQUEIA: a caixa foi a primeira
        // escolha e o mutation testing a derrubou. Ela e mais funda que a celula
        // (1,83 celula na escala do lider), mas a diferenca so muda resposta onde
        // um personagem esteja DENTRO da caixa e ao norte da celula - e ali ninguem
        // consegue estar, porque a caixa e justamente o que barra a passagem.
        // Sabotar aquele termo nao reprovava teste NENHUM (mutante sobrevivente na
        // suite real do app), que e a definicao de trecho que nao decide nada. Com
        // a celula sozinha o modelo cabe numa frase, cobre a peca ATRAVESSAVEL (o
        // holograma nao tem caixa: entraria como uma LINHA e voltaria a esconder
        // quem esta ao lado dele) e da a leitura certa para quem esta ao NORTE da
        // peca - esse fica atras, como tem de ser.
        //
        // PREMISSA: o corpo de um personagem e MENOR que uma celula (a hitbox e
        // 0,6 do tile). Se um dia existir ator com corpo mais fundo que uma celula,
        // ele deixa de caber na fatia da peca e cai na comparacao de borda - vale
        // remedir aqui. Peca montada a mao em teste (sem celula de origem) entra
        // como linha na base, que e o comportamento antigo.
        e.ground_front = p.footprint.y + p.footprint.h;
        e.ground_back = e.ground_front;
        if (p.cell_y >= 0) {
            e.ground_back = static_cast<float>(p.cell_y) * grid_.tile_size();
        }
        e.draw_left = p.footprint.x;
        e.draw_right = p.footprint.x + p.footprint.w;
        e.id = static_cast<int>(i);
        depth_scratch_.push_back(e);
    }
    for (std::size_t i = 0; i < actors_.size(); ++i) {
        const WorldActor& a = actors_[i];
        if (!a.drawable()) {
            continue;
        }
        gus::core::spatial::Aabb anchor = a.anchor;
        anchor.x = a.prev_anchor.x + (a.anchor.x - a.prev_anchor.x) * alpha;
        anchor.y = a.prev_anchor.y + (a.anchor.y - a.prev_anchor.y) * alpha;
        const gus::core::spatial::Aabb rect =
            actor_sprite_rect(anchor, a.sprite_height_tiles);
        if (!overlaps(gus::core::spatial::Rect{rect.x, rect.y, rect.w, rect.h},
                      view.rect)) {
            continue;
        }
        gus::core::spatial::DepthEntry e;
        // A profundidade e a da ANCORA (onde o ator pisa), nao a do quad desenhado -
        // o quad "vaza" para cima e daria leitura errada para sprites de alturas
        // diferentes. Ja a faixa LATERAL e a do QUAD, porque e o quad que cobre
        // pixel de vizinho.
        e.ground_back = anchor.y;
        e.ground_front = anchor.y + anchor.h;
        e.draw_left = rect.x;
        e.draw_right = rect.x + rect.w;
        e.id = kActorDepthIdBase + static_cast<int>(i);
        depth_scratch_.push_back(e);
    }
    {
        gus::core::spatial::DepthEntry e;
        e.ground_back = shown.y;
        e.ground_front = shown.y + shown.h;
        if (sprites_.loaded()) {
            // MESMA formula do desenho logo abaixo (quadrado de N tiles, centrado
            // em X sobre a hitbox): a faixa lateral tem de ser a do que e PINTADO.
            const float sprite_w =
                tuning_.player_sprite_height_tiles * grid_.tile_size();
            e.draw_left = shown.x + shown.w * 0.5f - sprite_w * 0.5f;
            e.draw_right = e.draw_left + sprite_w;
        } else {
            // Sem arte o desenho e o contorno da propria hitbox.
            e.draw_left = shown.x;
            e.draw_right = shown.x + shown.w;
        }
        e.id = kPlayerDepthId;
        depth_scratch_.push_back(e);
    }
    gus::core::spatial::sort_by_occlusion(depth_scratch_.data(),
                                          static_cast<int>(depth_scratch_.size()),
                                          depth_graph_scratch_);

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

    for (const gus::core::spatial::DepthEntry& e : depth_scratch_) {
        if (e.id == kPlayerDepthId) {
            do_draw_player();
        } else if (e.id >= kActorDepthIdBase) {
            do_draw_actor(e.id - kActorDepthIdBase);
        } else {
            do_draw_prop(e.id);
        }
    }

    renderer.end_frame();
}

}  // namespace gus::app::screens
