// gus/app/screens/overworld_sim.hpp
//
// OverworldSim: a regra de jogo do overworld do M1, POCO "fora da casca Qt"
// (engine-design.md secao 2/4). Junta os contratos puros ja prontos:
//   - core::spatial::resolve_move  -> colisao AABB que DESLIZA nas paredes;
//   - core::spatial::clamp_camera  -> camera ortografica presa ao mapa;
//   - interpolacao de render (alpha do FixedTimestep) -> movimento suave.
//
// NAO conhece Qt nem eventos: recebe (dx,dy) CRUS (do InputMapper) + dt fixo, e
// desenha atraves da interface IRenderer. Por isso e testavel sem janela (ver
// app/tests/overworld_sim_test.cpp) e o feel (desliza, camera clampa, anda liso)
// fica deterministico. A casca GameWindow/main so o alimenta e o desenha.
//
// NAO formaliza a state machine de telas (Overworld/Battle/Menu) - YAGNI, isso e
// M5. E so a tela do overworld.
//
// TODO o feel ajustavel (velocidade, corrida, normalize_diagonal, corner-assist,
// zoom, cores) mora no OverworldTuning (overworld_tuning.hpp) - PONTO UNICO de
// tuning. O OverworldSim so consome esse struct; mexer no feel nao reescreve nada.

#ifndef GUS_APP_SCREENS_OVERWORLD_SIM_HPP
#define GUS_APP_SCREENS_OVERWORLD_SIM_HPP

#include "gus/app/screens/overworld_tuning.hpp"
#include "gus/app/screens/sprite_anchor.hpp"  // FootInset (ancoragem pelos pes)
#include "gus/app/screens/sprite_animation.hpp"
#include "gus/core/spatial/camera_clamp.hpp"
#include "gus/core/spatial/grid_collision.hpp"
#include "gus/core/spatial/tile_grid.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

namespace gus::app::screens {

// Conjunto de sprites de locomocao do jogador, ja resolvidos para TextureId pelo
// renderer (a casca Qt chama IRenderer::load_texture e preenche isto). POCO sem
// Qt: o OverworldSim so guarda os handles e escolhe qual mostrar por
// (direcao, quadro). Indices: [direcao] = Direction (Sul/Norte/Leste/Oeste).
//   - idle[d]    = sprite neutro daquela direcao (parado E quadro neutro do walk);
//   - walk[d][f] = quadro f de walk (f em [0, kWalkFrameCount)).
// kInvalidTexture em qualquer slot => o render DEGRADA para o contorno da AABB
// (caminho do smoke/headless e de "ainda sem arte"). loaded() = true so se os 4
// idle estiverem validos (minimo para desenhar algo).
struct PlayerSpriteSet {
    gus::platform::render2d::TextureId idle[kDirectionCount] = {};
    gus::platform::render2d::TextureId walk[kDirectionCount][kWalkFrameCount] = {};

    // ANCORAGEM AUTOMATICA (M1-BUG.SUL): margem inferior transparente de cada sprite
    // IDLE, em fracao do canvas, MEDIDA pelo loader via IRenderer::texture_content_bbox.
    // Ancora-se pelo IDLE (estavel: o tronco/cabeca nao "pula" entre quadros de walk).
    // Tudo zero (headless/Null, sem decode) => anchor legado (base do canvas == base
    // da AABB), preservando o comportamento e os testes antigos.
    FootInset foot{};

    [[nodiscard]] bool loaded() const noexcept {
        for (int d = 0; d < kDirectionCount; ++d) {
            if (idle[d] == gus::platform::render2d::kInvalidTexture) {
                return false;
            }
        }
        return true;
    }
};

class OverworldSim {
public:
    // Ctor principal: grid (copiado), AABB inicial do jogador e o tuning completo
    // (movimento + corner-assist + camera + cores). O lider ajusta tudo via o
    // OverworldTuning, sem tocar nesta classe.
    OverworldSim(gus::core::spatial::TileGrid grid,
                 gus::core::spatial::Aabb player_start, OverworldTuning tuning);

    // Ctor de conveniencia: so a velocidade de caminhada (tiles/s); o resto do
    // tuning fica no default. Mantido para chamadas/testes simples.
    OverworldSim(gus::core::spatial::TileGrid grid,
                 gus::core::spatial::Aabb player_start,
                 float walk_speed_tiles_per_sec);

    // Um passo de simulacao a dt FIXO. dx,dy em {-1,0,1} (cardinal cru). run liga
    // o multiplicador de corrida (tuning). Aplica a colisao (com corner-assist se
    // ligado no tuning) e guarda a posicao anterior pra interpolacao do render.
    void step_fixed(int dx, int dy, bool run, float fixed_dt) noexcept;

    // Visao da camera (clampada ao mapa) centrada no jogador ATUAL.
    [[nodiscard]] gus::core::spatial::CameraView camera_view(
        float viewport_w, float viewport_h) const noexcept;

    // Desenha o frame: abre com a camera (interpolada), emite as paredes visiveis
    // (preenchidas) e o jogador (contorno da AABB, interpolado por alpha em [0,1]
    // entre a posicao anterior e a atual), fecha. viewport_w/h em mundo.
    void render(gus::platform::render2d::IRenderer& renderer, float viewport_w,
                float viewport_h, float alpha) const;

    // Posicao ATUAL do jogador (canto sup-esq + w/h).
    [[nodiscard]] const gus::core::spatial::Aabb& player() const noexcept { return curr_; }

    // Mapa (pra inspecao/desenho externo, se preciso).
    [[nodiscard]] const gus::core::spatial::TileGrid& grid() const noexcept { return grid_; }

    // Tuning vigente (leitura). O lider/main podem inspecionar/ajustar a copia
    // antes de construir; aqui e so leitura do que esta em uso.
    [[nodiscard]] const OverworldTuning& tuning() const noexcept { return tuning_; }

    // Define os sprites do jogador (handles ja resolvidos pelo renderer). Se nao
    // for chamado (ou vier incompleto), o render usa o contorno da AABB (fallback
    // do headless / "sem arte"). Chamado pela casca Qt apos load_texture.
    void set_player_sprites(const PlayerSpriteSet& sprites) noexcept { sprites_ = sprites; }

    // Direcao e quadro de walk correntes (leitura/teste).
    [[nodiscard]] Direction facing() const noexcept { return facing_; }
    [[nodiscard]] const WalkCycle& walk_cycle() const noexcept { return walk_; }

private:
    // Posicao do jogador interpolada entre prev_ e curr_ por alpha.
    [[nodiscard]] gus::core::spatial::Aabb interpolated_player(float alpha) const noexcept;

    gus::core::spatial::TileGrid grid_;
    gus::core::spatial::Aabb prev_;  // posicao no passo anterior (pra interpolar)
    gus::core::spatial::Aabb curr_;  // posicao atual
    OverworldTuning tuning_;         // PONTO UNICO de feel (velocidade/corner/zoom/cores)

    // Estado de animacao (logica POCO em sprite_animation.hpp). facing_ e a ultima
    // direcao cardinal; walk_ cicla os quadros por distancia. Avancados em
    // step_fixed; lidos em render. sprites_ sao os handles (vazio = fallback).
    Direction facing_ = Direction::South;
    WalkCycle walk_;
    PlayerSpriteSet sprites_{};

    // MEMORIA DO INPUT do tick anterior (cru, em {-1,0,1}). Necessaria pra politica
    // LastAxisWins decidir o eixo RECEM-acionado pela mudanca do INPUT (e nao pelo
    // facing anterior), eliminando o flicker da diagonal sustentada. Init (0,0).
    int dx_prev_ = 0;
    int dy_prev_ = 0;
};

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_OVERWORLD_SIM_HPP
