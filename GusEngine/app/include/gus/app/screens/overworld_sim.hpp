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

#include <optional>

#include "gus/app/screens/overworld_tuning.hpp"
#include "gus/app/screens/sprite_anchor.hpp"  // FootInset (ancoragem pelos pes)
#include "gus/app/screens/sprite_animation.hpp"
#include "gus/app/screens/tile_palette.hpp"  // TilePalette (cor por TileKind, graybox)
#include "gus/domain/map/tile_map.hpp"  // TileMap (mapa real, pinta por TileKind)
#include "gus/core/anim/anim_clock.hpp"  // idle OFEGANTE (breathing) por TEMPO
#include "gus/core/anim/breath_oscillator.hpp"  // idle CALMO (senoide procedural)
#include "gus/core/player/stamina.hpp"  // Carga do aparato: drena correndo
#include "gus/core/player/winded_timer.hpp"  // folego do corpo: ofega ao parar (>= 5 s)
#include "gus/core/spatial/camera_clamp.hpp"
#include "gus/core/spatial/grid_collision.hpp"
#include "gus/core/spatial/tile_grid.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

namespace gus::app::screens {

// Conjunto de sprites de locomocao do jogador, ja resolvidos para TextureId pelo
// renderer (a casca SDL chama IRenderer::load_texture e preenche isto). POCO sem
// SDL: o OverworldSim so guarda os handles e escolhe qual mostrar por
// (direcao, quadro). Indices: [direcao] = Direction (Sul/Norte/Leste/Oeste).
//   - idle[d]          = quadro REPRESENTATIVO/neutro (frame 0 do breathing) daquela
//                        direcao - acesso direto preservado pra compat de testes;
//   - idle_frames[d][i]= quadro i do IDLE ANIMADO (breathing); i em [0, idle_count[d]);
//   - walk[d][f]       = quadro f de walk; f em [0, walk_count[d]).
//
// GENERALIZACAO (Gus): walk e idle aceitam N quadros POR DIRECAO (Caua: 4 walk e 1
// idle congelado; Gus: 7 walk e 5 breathing). Arrays FIXOS dimensionados pelos tetos
// (kMaxWalkFrameCount/kMaxIdleFrameCount), SEM heap; as contagens reais ficam em
// walk_count[d]/idle_count[d]. idle_count[d] >= 1 sempre (frame 0 == idle[d]).
//
// kInvalidTexture em qualquer slot usado => o render DEGRADA para o contorno da AABB
// (caminho do smoke/headless e de "ainda sem arte"). loaded() = true so se os 4
// idle representativos estiverem validos (minimo para desenhar algo).
struct PlayerSpriteSet {
    // Quadro representativo (frame 0 do idle/breathing) por direcao. Mantido pra
    // compat de acesso direto (testes legados: s.idle[d]).
    gus::platform::render2d::TextureId idle[kDirectionCount] = {};
    // Quadros do walk por direcao (N reais em walk_count[d]).
    gus::platform::render2d::TextureId walk[kDirectionCount][kMaxWalkFrameCount] = {};
    // Quadros do IDLE ANIMADO (breathing) por direcao (N reais em idle_count[d]).
    gus::platform::render2d::TextureId idle_frames[kDirectionCount][kMaxIdleFrameCount] = {};

    // Contagem REAL de quadros por direcao. walk_count default = kWalkFrameCount (4,
    // legado do Caua, mantido pra make_fake_sprites antigo); idle_count default = 1
    // (idle congelado legado). O loader sobrescreve com o que achou no disco.
    int walk_count[kDirectionCount] = {kWalkFrameCount, kWalkFrameCount,
                                       kWalkFrameCount, kWalkFrameCount};
    int idle_count[kDirectionCount] = {1, 1, 1, 1};

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

    // Maior contagem de walk entre as 4 direcoes (>= 1). Dirige o frame_count do
    // WalkCycle (ciclo unico pro personagem; todas as direcoes do Gus tem 7).
    [[nodiscard]] int max_walk_count() const noexcept {
        int m = 1;
        for (int d = 0; d < kDirectionCount; ++d) {
            if (walk_count[d] > m) m = walk_count[d];
        }
        return m;
    }
    // Idem pro idle animado (breathing). >= 1.
    [[nodiscard]] int max_idle_count() const noexcept {
        int m = 1;
        for (int d = 0; d < kDirectionCount; ++d) {
            if (idle_count[d] > m) m = idle_count[d];
        }
        return m;
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

    // Ctor a partir do MAPA REAL (TileMap do dominio): a colisao vem de
    // map.to_tile_grid() (so Parede bloqueia), o player_start ja deve estar derivado
    // do map.spawn() (ver city_scene.hpp). GUARDA o TileMap pra o render pintar cada
    // celula pela cor do seu TileKind (graybox) via a TilePalette. O ctor de TileGrid
    // continua valendo (cena de teste / fallback) e cai no render de paredes legado.
    OverworldSim(gus::domain::map::TileMap map,
                 gus::core::spatial::Aabb player_start, OverworldTuning tuning);

    // Paleta graybox (cor por TileKind) usada quando ha TileMap. Ponto unico de cor;
    // o lider ajusta em tile_palette.hpp ou troca aqui antes/depois de construir.
    void set_tile_palette(const TilePalette& palette) noexcept { palette_ = palette; }
    [[nodiscard]] const TilePalette& tile_palette() const noexcept { return palette_; }

    // O TileMap em uso (nullopt quando construido so do TileGrid). Leitura/teste.
    [[nodiscard]] const std::optional<gus::domain::map::TileMap>& tile_map()
        const noexcept {
        return map_;
    }

    // Um passo de simulacao a dt FIXO. dx,dy em {-1,0,1} (cardinal cru). run liga
    // o multiplicador de corrida (tuning). Aplica a colisao (com corner-assist se
    // ligado no tuning) e guarda a posicao anterior pra interpolacao do render.
    void step_fixed(int dx, int dy, bool run, float fixed_dt) noexcept;

    // Visao da camera (clampada ao mapa) centrada no jogador ATUAL. viewport_px_w/h
    // em PIXELS de tela; o ZOOM (tuning camera_zoom_px_per_tile) converte pra unidades
    // de mundo internamente. A camera AMPLIA a porcao ao redor do Gus e rola no mapa.
    [[nodiscard]] gus::core::spatial::CameraView camera_view(
        float viewport_px_w, float viewport_px_h) const noexcept;

    // Desenha o frame: abre com a camera (interpolada + zoom), emite as paredes
    // visiveis (preenchidas) e o jogador (interpolado por alpha em [0,1] entre a
    // posicao anterior e a atual), fecha. viewport_px_w/h em PIXELS de tela (o zoom
    // do tuning os converte pra unidades de mundo da visao).
    void render(gus::platform::render2d::IRenderer& renderer, float viewport_px_w,
                float viewport_px_h, float alpha) const;

    // Fator de conversao do zoom: PIXELS de tela por UNIDADE de mundo (=
    // camera_zoom_px_per_tile / tile_size do mapa). Leitura/teste. Guarda tile_size<=0.
    [[nodiscard]] float px_per_world_unit() const noexcept;

    // Posicao ATUAL do jogador (canto sup-esq + w/h).
    [[nodiscard]] const gus::core::spatial::Aabb& player() const noexcept { return curr_; }

    // Mapa (pra inspecao/desenho externo, se preciso).
    [[nodiscard]] const gus::core::spatial::TileGrid& grid() const noexcept { return grid_; }

    // Tuning vigente (leitura). O lider/main podem inspecionar/ajustar a copia
    // antes de construir; aqui e so leitura do que esta em uso.
    [[nodiscard]] const OverworldTuning& tuning() const noexcept { return tuning_; }

    // Define os sprites do jogador (handles ja resolvidos pelo renderer). Se nao
    // for chamado (ou vier incompleto), o render usa o contorno da AABB (fallback
    // do headless / "sem arte"). Chamado pela casca SDL apos load_texture.
    //
    // RECONFIGURA as duas animacoes pelo numero REAL de quadros da arte recebida:
    //   - WalkCycle: frame_count = sprites.max_walk_count() (Caua 4, Gus 7), preservando
    //     o feel por DISTANCIA (mesmo px_per_frame do tuning);
    //   - AnimClock do idle: frame_count = sprites.max_idle_count() (Caua 1 = congelado,
    //     Gus 5 = breathing), tocado por TEMPO no step_fixed (loop em loop).
    void set_player_sprites(const PlayerSpriteSet& sprites) noexcept;

    // MARCADOR DE INIMIGO FIXO (M7-COSTURA Inc 2): posiciona (ou reposiciona) um
    // marcador visivel de inimigo ESTATICO no mapa, desenhado por cima do chao na MESMA
    // escala/ancoragem do sprite do Gus (tuning_.player_sprite_height_tiles), pra ficar
    // do tamanho certo e visivel na celula. `tex` e o TextureId JA RESOLVIDO pela casca
    // SDL (mesmo padrao de set_player_sprites) - a MESMA textura (retrato_inimigo.png)
    // que a tela de BATALHA usa pros inimigos, pra o jogador reconhecer "e o mesmo
    // bicho". kInvalidTexture => nada e desenhado (fallback seguro/headless). A Maestro
    // (dona da posicao logica do inimigo) chama isto apos calcular a posicao; NAO muda a
    // colisao/regra de jogo, so o desenho.
    void set_enemy_marker(const gus::core::spatial::Aabb& aabb,
                          gus::platform::render2d::TextureId tex) noexcept {
        enemy_marker_aabb_ = aabb;
        enemy_marker_tex_ = tex;
    }

    // Some com o marcador (Victory, item 4 do escopo M7 Inc 1: "o inimigo derrotado some
    // do mapa"). No-op seguro se nao havia marcador.
    void clear_enemy_marker() noexcept {
        enemy_marker_aabb_.reset();
        enemy_marker_tex_ = gus::platform::render2d::kInvalidTexture;
    }

    // true se ha um marcador de inimigo ATIVO e desenhavel (AABB definida + textura
    // valida). Leitura/teste.
    [[nodiscard]] bool has_enemy_marker() const noexcept {
        return enemy_marker_aabb_.has_value() &&
               enemy_marker_tex_ != gus::platform::render2d::kInvalidTexture;
    }

    // Direcao e quadro de walk correntes (leitura/teste).
    [[nodiscard]] Direction facing() const noexcept { return facing_; }
    [[nodiscard]] const WalkCycle& walk_cycle() const noexcept { return walk_; }
    // Quadro corrente do idle OFEGANTE (breathing rapido) - leitura/teste.
    [[nodiscard]] const gus::core::anim::AnimClock& idle_clock() const noexcept {
        return idle_clock_;
    }
    // Folego corrente do jogador (drena correndo, recupera parado/andando) - dirige a
    // escolha entre idle CALMO (descansado) e OFEGANTE (cansado). Leitura/teste.
    [[nodiscard]] const gus::core::player::Stamina& stamina() const noexcept {
        return stamina_;
    }
    // Oscilador da respiracao CALMA (senoide procedural do idle descansado) - leitura/teste.
    [[nodiscard]] const gus::core::anim::BreathOscillator& breath() const noexcept {
        return breath_;
    }
    // true quando a CARGA do aparato esta abaixo do limiar (leitura do hardware).
    [[nodiscard]] bool is_tired() const noexcept { return stamina_.is_tired(); }
    // true enquanto o FOLEGO do corpo (timer separado) esta ativo: ao parar de correr,
    // o Gus ofega por >= 5 s INDEPENDENTE da Carga ja ter recarregado (lider 2026-06-23).
    [[nodiscard]] bool is_winded() const noexcept { return winded_.is_winded(); }
    // O idle OFEGANTE e mostrado quando QUALQUER um dispara: Carga baixa (overflow do
    // aparato) OU folego do corpo ainda alto (peito acalmando). Une as duas leituras.
    [[nodiscard]] bool show_winded_idle() const noexcept {
        return stamina_.is_tired() || winded_.is_winded();
    }
    // Timer de folego do corpo (leitura/teste).
    [[nodiscard]] const gus::core::player::WindedTimer& winded() const noexcept {
        return winded_;
    }

private:
    // Posicao do jogador interpolada entre prev_ e curr_ por alpha.
    [[nodiscard]] gus::core::spatial::Aabb interpolated_player(float alpha) const noexcept;

    gus::core::spatial::TileGrid grid_;
    // MAPA REAL opcional: presente quando construido do TileMap. Da ao render a
    // identidade (TileKind) de cada celula pra pintar por cor (graybox). Ausente
    // (nullopt) = cena de teste/fallback do TileGrid -> render de paredes legado.
    std::optional<gus::domain::map::TileMap> map_;
    TilePalette palette_{};  // cor por TileKind (so usada quando ha map_)
    gus::core::spatial::Aabb prev_;  // posicao no passo anterior (pra interpolar)
    gus::core::spatial::Aabb curr_;  // posicao atual
    OverworldTuning tuning_;         // PONTO UNICO de feel (velocidade/corner/zoom/cores)

    // Estado de animacao (logica POCO em sprite_animation.hpp). facing_ e a ultima
    // direcao cardinal; walk_ cicla os quadros por distancia. Avancados em
    // step_fixed; lidos em render. sprites_ sao os handles (vazio = fallback).
    Direction facing_ = Direction::South;
    WalkCycle walk_;
    PlayerSpriteSet sprites_{};

    // MARCADOR DE INIMIGO FIXO (M7-COSTURA Inc 2): AABB + textura do marcador visivel
    // (ver set_enemy_marker acima). nullopt/kInvalidTexture = nada desenhado (default,
    // headless/sem Maestro).
    std::optional<gus::core::spatial::Aabb> enemy_marker_aabb_{};
    gus::platform::render2d::TextureId enemy_marker_tex_ =
        gus::platform::render2d::kInvalidTexture;

    // IDLE OFEGANTE (cansado): troca os QUADROS do breathing por TEMPO (AnimClock),
    // num ritmo RAPIDO (idle_tired_breaths_per_minute). So e mostrado quando parado E
    // cansado (stamina < limiar). Avanca no step_fixed pelo fixed_dt (respira parado).
    // frame_count = quadros do breathing (1 = congelado, legado Caua). O fps e DERIVADO
    // do tuning quando os sprites chegam (set_player_sprites sabe o N real do loop).
    gus::core::anim::AnimClock idle_clock_{
        1, OverworldTuning{}.idle_fps_for_loop(1)};

    // FOLEGO do jogador: drena correndo, recupera parado/andando. Decide se o idle e
    // CALMO (descansado) ou OFEGANTE (cansado). Avanca no step_fixed pelo fixed_dt.
    gus::core::player::Stamina stamina_{};

    // IDLE CALMO (descansado): respiracao PROCEDURAL - senoide continua e suave (bob/
    // escala), SEM trocar quadro (fim do staccato). Avanca no step_fixed pelo fixed_dt;
    // o render usa breath_.value() pra esticar/deslocar levemente o sprite NEUTRO.
    gus::core::anim::BreathOscillator breath_{
        OverworldTuning{}.idle_calm_breaths_per_minute};

    // FOLEGO do corpo (lider 2026-06-23): acumula tempo enquanto CORRE; ao parar de
    // correr (apos correr o bastante), ofega por >= 5 s escalado, decaindo parado ate
    // zerar - INDEPENDENTE da Carga. Avancado no step_fixed pelo move_state real; o
    // render forca o idle ofegante enquanto is_winded() (ou a Carga estiver baixa).
    gus::core::player::WindedTimer winded_{};

    // MEMORIA DO INPUT do tick anterior (cru, em {-1,0,1}). Necessaria pra politica
    // LastAxisWins decidir o eixo RECEM-acionado pela mudanca do INPUT (e nao pelo
    // facing anterior), eliminando o flicker da diagonal sustentada. Init (0,0).
    int dx_prev_ = 0;
    int dy_prev_ = 0;
};

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_OVERWORLD_SIM_HPP
