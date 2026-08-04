// SPDX-License-Identifier: Apache-2.0
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
#include <vector>

#include "gus/app/screens/overworld_tuning.hpp"
#include "gus/app/screens/sprite_anchor.hpp"  // FootInset (ancoragem pelos pes)
#include "gus/app/screens/sprite_animation.hpp"
#include "gus/app/screens/world_entities.hpp"  // pecas de cenario + atores (listas)
#include "gus/app/screens/tile_palette.hpp"  // TilePalette (cor por TileKind, graybox)
#include "gus/domain/map/tile_map.hpp"  // TileMap (mapa real, pinta por TileKind)
#include "gus/core/anim/anim_clock.hpp"  // idle OFEGANTE (breathing) por TEMPO
#include "gus/core/anim/breath_oscillator.hpp"  // idle CALMO (senoide procedural)
#include "gus/core/player/stamina.hpp"  // Carga do aparato: drena correndo
#include "gus/core/player/winded_timer.hpp"  // folego do corpo: ofega ao parar (>= 5 s)
#include "gus/core/spatial/camera_clamp.hpp"
#include "gus/core/spatial/depth_sort.hpp"  // DepthEntry (Y-sort de N entidades)
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
    //
    // DUNGEON-SCALING (fix do letterbox ao maximizar): screen_px_w/h (NOVO, opcional)
    // e o tamanho REAL da tela pra onde o retangulo de camera e mapeado (begin_frame) -
    // DESACOPLADO de viewport_px_w/h (que so alimenta o ZOOM/quanto-mundo-mostra).
    // Default (<=0, sentinela) = usa viewport_px_w/h pros dois papeis, o comportamento
    // LEGADO byte-identico (TODOS os testes/chamadores existentes, que passam so 1
    // tamanho de viewport, continuam identicos). O caller de producao (SdlWindow)
    // passa um viewport LOGICO FIXO (kWindowW/kWindowH, a resolucao onde o zoom foi
    // calibrado, ver overworld_tuning.hpp) + o screen_px_w/h REAL da janela (que cresce
    // ao maximizar) - a camera nunca revela MAIS mundo (nao esbarra no limite do mapa e
    // sobra preto), so ESTICA o mesmo enquadramento pro tamanho real da tela, igual a
    // tela de batalha (Render2dGl3, camera fixa 960x540 D1 esticada pra janela).
    void render(gus::platform::render2d::IRenderer& renderer, float viewport_px_w,
                float viewport_px_h, float alpha, float screen_px_w = -1.0f,
                float screen_px_h = -1.0f) const;

    // Fator de conversao do zoom: PIXELS de tela por UNIDADE de mundo (=
    // camera_zoom_px_per_tile / tile_size do mapa). Leitura/teste. Guarda tile_size<=0.
    [[nodiscard]] float px_per_world_unit() const noexcept;

    // Posicao ATUAL do jogador (canto sup-esq + w/h).
    [[nodiscard]] const gus::core::spatial::Aabb& player() const noexcept { return curr_; }

    // TELEPORTE (SAVE-LOAD-UI etapa 6, wiring do Carregar REAL no menu de pausa):
    // reposiciona o jogador INSTANTANEAMENTE pra `aabb`, SEM interpolacao visual -
    // curr_ E prev_ recebem o MESMO valor, entao o proximo render() nao produz
    // nenhum "deslize" fantasma do ponto antigo pro novo (a interpolacao normal de
    // step_fixed so faz sentido entre 2 posicoes de UM MESMO passo de movimento).
    // Usado pelo CHAMADOR (Maestro) SO ao aplicar um SaveData carregado do disco -
    // NUNCA durante o step_fixed normal de gameplay. NAO mexe em facing_/walk_/
    // stamina_ (o save nao guarda direcao/animacao - o jogador aparece parado na
    // pose corrente, mesma degradacao "sem info extra" de outros campos ausentes
    // do SaveData).
    void set_player_position(const gus::core::spatial::Aabb& aabb) noexcept {
        curr_ = aabb;
        prev_ = aabb;
    }

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

    // =======================================================================
    //  PECAS DE CENARIO (DEMO-CIDADE-VESTIDA B1) - a cidade deixa de ser so chao
    //  colorido. A peca chega JA RESOLVIDA (retangulo em unidades de mundo +
    //  textura da casca): quem traduz o dado do catalogo em instancia e
    //  city_props.hpp, e o dado em si mora em gus/domain/world/scene_prop.hpp.
    //  O sim so desenha (com Y-sort) e colide.
    // =======================================================================

    // Poe uma peca no mundo. Devolve o indice dela (leitura/teste); a lista so
    // cresce ate um clear_scene_props.
    int add_scene_prop(const ScenePropInstance& prop);

    // Tira TODAS as pecas (troca de area, recarregar cidade).
    void clear_scene_props() noexcept;

    [[nodiscard]] const std::vector<ScenePropInstance>& scene_props() const noexcept {
        return props_;
    }

    // =======================================================================
    //  ATORES (DEMO-CIDADE-VESTIDA B2) - VARIOS NPCs e VARIOS inimigos.
    //
    //  ANTES desta fatia existiam DOIS SLOTS FIXOS (um inimigo, um NPC), cada um
    //  com par proprio de AABB/textura/caixa solida, e o Y-sort era um array de 3
    //  posicoes. Agora e uma lista so: o papel (NPC/inimigo) e dado do ator, nao
    //  estrutura de codigo, e a ordenacao por profundidade vale para N.
    //
    //  HANDLE = INDICE, e indice NUNCA escorrega: remover um ator apaga o
    //  conteudo mas preserva o slot, senao todo handle guardado por quem chamou
    //  (a Maestro, um save) passaria a apontar para o vizinho.
    // =======================================================================

    // Poe um personagem no mundo. Devolve o handle. A ronda (spec.route) e
    // opcional: rota invalida = personagem parado, o comportamento de sempre.
    int add_actor(const WorldActorSpec& spec);

    // (Re)arma a ronda de um ator ja existente. A rota e aplicada como
    // DESLOCAMENTO a partir de onde o ator esta AGORA - armar nunca teleporta.
    // Handle invalido e no-op seguro.
    void set_actor_patrol(int handle,
                          const gus::domain::world::PatrolRoute& route) noexcept;

    // Tira o ator do desenho E da colisao (inimigo derrotado, NPC que foi embora).
    // Handle invalido e no-op seguro.
    void remove_actor(int handle) noexcept;

    // Posicao logica CORRENTE do ator (nullopt se o handle nao existe ou o ator
    // foi removido). E por aqui que a Maestro le onde o inimigo em ronda esta
    // AGORA para recalcular a caixa de ativacao da batalha.
    [[nodiscard]] std::optional<gus::core::spatial::Aabb> actor_anchor(
        int handle) const noexcept;

    [[nodiscard]] const std::vector<WorldActor>& actors() const noexcept {
        return actors_;
    }

    // =======================================================================
    //  FACHADA LEGADA dos dois marcadores (M7-COSTURA/M7-DIALOGO).
    //
    //  Continua valendo palavra por palavra para quem ja chamava (Maestro,
    //  SdlWindow, app/tools) - o que mudou por baixo e que cada marcador virou UM
    //  ATOR RESERVADO da lista, em vez de um slot paralelo. Mesma escala, mesma
    //  ancoragem, mesma caixa solida, mesma degradacao com textura invalida.
    //  Reposicionar NAO empilha ator novo: reusa o slot reservado e REARMA a ronda
    //  a partir da posicao nova (carregar um save reposiciona o inimigo).
    // =======================================================================

    // MARCADOR DE INIMIGO FIXO (M7-COSTURA Inc 2): posiciona (ou reposiciona) um
    // marcador visivel de inimigo no mapa, desenhado por cima do chao na MESMA
    // escala/ancoragem do sprite do Gus (tuning_.player_sprite_height_tiles). `tex`
    // e o TextureId JA RESOLVIDO pela casca SDL (mesmo padrao de set_player_sprites)
    // - a MESMA textura (retrato_inimigo.png) que a tela de BATALHA usa pros
    // inimigos, pra o jogador reconhecer "e o mesmo bicho". kInvalidTexture => nada
    // e desenhado (fallback seguro/headless).
    // COLISAO SOLIDA (M7-COSTURA, ver overworld_tuning.hpp::npc_solid_box_tiles):
    // deriva TAMBEM a caixa de bloqueio FISICO a partir do footprint recebido -
    // mesma ancoragem do feet_trigger_aabb (centro em X, base = base do footprint),
    // so que MAIOR (~1 tile, nao a faixa fina do trigger). step_fixed passa essa
    // caixa como ObstacleSpan pro resolve_move_with_corner_assist: o jogador NUNCA
    // ocupa a mesma posicao do inimigo (mas contorna livre pelos tiles adjacentes).
    void set_enemy_marker(const gus::core::spatial::Aabb& aabb,
                          gus::platform::render2d::TextureId tex) noexcept;

    // Some com o marcador (Victory, item 4 do escopo M7 Inc 1: "o inimigo derrotado
    // some do mapa"). No-op seguro se nao havia marcador. TAMBEM libera o bloqueio
    // FISICO (o inimigo derrotado nao deve mais colidir com o jogador).
    void clear_enemy_marker() noexcept;

    // true se ha um marcador de inimigo ATIVO e desenhavel (posicao definida +
    // textura valida). Leitura/teste.
    [[nodiscard]] bool has_enemy_marker() const noexcept;

    // Posicao CORRENTE do marcador de inimigo (nullopt se nao ha). Diferente do
    // valor que a Maestro passou em set_enemy_marker assim que uma ronda esteja
    // armada: e daqui que se le onde ele esta AGORA.
    [[nodiscard]] std::optional<gus::core::spatial::Aabb> enemy_marker_aabb()
        const noexcept {
        return actor_anchor(enemy_marker_handle_);
    }

    // Handle do ator reservado ao marcador de inimigo (kInvalidWorldActor enquanto
    // nenhum set_enemy_marker tiver acontecido). Serve para armar a ronda dele
    // pelas APIs genericas de ator, sem uma segunda via so para o slot legado.
    [[nodiscard]] int enemy_marker_handle() const noexcept {
        return enemy_marker_handle_;
    }

    // MARCADOR DO NPC BERTOLDO (M7-DIALOGO, NPC-MVP): posiciona/reposiciona o sprite
    // ESTATICO do Seu Bertoldo Caim (south.png, sem locomocao/direcao dinamica) no
    // mapa. Ator PROPRIO (nao compartilha slot com o marcador de inimigo) - o
    // Bertoldo e um NPC amigavel, sprite distinto do androide-inimigo. ESCALA
    // PROPRIA (tuning_.npc_bertoldo_sprite_height_tiles): o retrato dele tem margem
    // transparente maior que o do Gus, e reusar a altura de canvas do jogador fazia
    // o adulto renderizar mais baixo que a crianca (bug do playtest ao vivo).
    // `tex` ja resolvido pela casca SDL. COLISAO SOLIDA: MESMA tecnica do inimigo.
    void set_npc_bertoldo_marker(const gus::core::spatial::Aabb& aabb,
                                 gus::platform::render2d::TextureId tex) noexcept;

    // Some com o marcador (asset ausente/headless degrada com seguranca). No-op se
    // nao havia marcador. TAMBEM libera o bloqueio FISICO.
    void clear_npc_bertoldo_marker() noexcept;

    // true se o marcador do Bertoldo esta ATIVO e desenhavel. Leitura/teste.
    [[nodiscard]] bool has_npc_bertoldo_marker() const noexcept;

    // Posicao CORRENTE e handle do Bertoldo (espelho do par do inimigo acima).
    [[nodiscard]] std::optional<gus::core::spatial::Aabb> npc_bertoldo_marker_aabb()
        const noexcept {
        return actor_anchor(npc_bertoldo_handle_);
    }
    [[nodiscard]] int npc_bertoldo_marker_handle() const noexcept {
        return npc_bertoldo_handle_;
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

    // COLISAO SOLIDA (M7-COSTURA/M7-DIALOGO): deriva a caixa de bloqueio FISICO de
    // um NPC/inimigo a partir do seu footprint VISUAL (o mesmo AABB passado a
    // set_enemy_marker/set_npc_bertoldo_marker) - MESMA ancoragem do feet_trigger_
    // aabb (maestro_logic.hpp): centrada em X sobre o footprint, base = base do
    // footprint. Tamanho = tuning_.npc_solid_box_tiles (ver rationale la, NAO o
    // footprint visual inteiro - isso bloquearia tiles demais). Usa grid_.tile_size()
    // e tuning_ (membros da propria classe) - por isso e metodo, nao funcao livre.
    [[nodiscard]] gus::core::spatial::Aabb solid_obstacle_from_footprint(
        const gus::core::spatial::Aabb& footprint) const noexcept;

    // Quad DESENHADO de um ator a partir da sua ancora: quadrado de
    // sprite_height_tiles, centrado em X, base na base da ancora. A MESMA formula
    // que os dois marcadores usavam separadamente antes desta fatia - uma so, agora,
    // parametrizada pela altura (que e dado do ator).
    [[nodiscard]] gus::core::spatial::Aabb actor_sprite_rect(
        const gus::core::spatial::Aabb& anchor,
        float sprite_height_tiles) const noexcept;

    // (Re)arma a ronda de um ator ja resolvido, ancorando a rota na posicao ATUAL
    // dele. Usada por add_actor, set_actor_patrol e pela fachada legada (que rearma
    // ao reposicionar).
    void rearm_patrol(WorldActor& actor,
                      const gus::domain::world::PatrolRoute& route) const noexcept;

    // Avanca a ronda de TODOS os atores por um passo fixo (posicao + corpo solido).
    // Roda ANTES de qualquer saida antecipada do step_fixed: a cidade tem que
    // continuar viva com o jogador parado.
    void advance_actor_patrols(float fixed_dt) noexcept;

    // Slot reservado da fachada legada: cria na primeira chamada e reusa depois.
    int ensure_reserved_actor(int& handle, WorldActorRole role,
                              const gus::core::spatial::Aabb& aabb,
                              gus::platform::render2d::TextureId tex,
                              float sprite_height_tiles) noexcept;

    [[nodiscard]] bool valid_actor_handle(int handle) const noexcept {
        return handle >= 0 && handle < static_cast<int>(actors_.size());
    }

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

    // PECAS DE CENARIO e ATORES do mundo (DEMO-CIDADE-VESTIDA B1/B2). Substituem os
    // dois slots fixos de marcador que existiam aqui ate esta fatia (um inimigo, um
    // NPC, com par proprio de AABB/textura/caixa solida cada). Vazias por default:
    // uma cidade sem nada continua sendo uma cidade valida (headless/sem Maestro).
    std::vector<ScenePropInstance> props_{};
    std::vector<WorldActor> actors_{};

    // Slots RESERVADOS da fachada legada. kInvalidWorldActor ate a primeira chamada
    // de set_enemy_marker/set_npc_bertoldo_marker - dai em diante o MESMO indice e
    // reusado, para reposicionar nao empilhar ator novo a cada save carregado.
    int enemy_marker_handle_ = kInvalidWorldActor;
    int npc_bertoldo_handle_ = kInvalidWorldActor;

    // Rascunhos reaproveitados entre quadros (sem alocar no laco de jogo depois do
    // aquecimento): obstaculos pontuais do passo fixo e entradas do Y-sort. O
    // segundo e mutable porque o render e const - ele nao muda o mundo, so precisa
    // de um lugar para ordenar quem desenha antes de quem.
    std::vector<gus::core::spatial::Aabb> obstacle_scratch_{};
    mutable std::vector<gus::core::spatial::DepthEntry> depth_scratch_{};

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
