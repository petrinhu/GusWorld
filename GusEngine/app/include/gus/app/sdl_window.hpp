// gus/app/sdl_window.hpp
//
// SdlWindow: a CASCA SDL (janela + loop PROPRIO + bombeamento de input). Vive em
// app/ por ser o SHELL DE APLICACAO (cria janela/renderer, possui o loop de frame),
// nao um modulo de engine reutilizavel - o renderer reutilizavel (Render2dSdl atras
// de IRenderer) e a regra de jogo (OverworldSim) estao FORA daqui. Mantida fina:
// nenhuma regra de jogo, so orquestracao SDL.
//
// POS REPIVOT ADR-008: aqui esta a diferenca-chave do Qt - NOS possuimos o loop.
// Nao ha event loop de framework para brigar: o FixedTimestep (core/time, POCO)
// dirige o ritmo (poll de eventos SDL -> N updates fixos -> 1 render -> repeat),
// com SDL_GetTicksNS medindo o dt real. Isso casa exatamente com o modelo de loop
// que o engine-design pediu.
//
// Inclui <SDL3/SDL.h> (camada app/, SDL permitido). O irredutivel (criar janela/
// renderer, apresentar) e coberto pelo smoke headless do main (--smoke com
// SDL_VIDEODRIVER=dummy).

#ifndef GUS_APP_SDL_WINDOW_HPP
#define GUS_APP_SDL_WINDOW_HPP

#include <memory>

#include <SDL3/SDL.h>

#include "gus/app/screens/overworld_sim.hpp"
#include "gus/core/time/fixed_timestep.hpp"
#include "gus/platform/input/sdl_input.hpp"
#include "gus/platform/render2d/render2d_sdl.hpp"

namespace gus::app {

class SdlWindow {
public:
    SdlWindow();
    ~SdlWindow();

    SdlWindow(const SdlWindow&) = delete;
    SdlWindow& operator=(const SdlWindow&) = delete;

    // Cria janela + renderer SDL + carrega os sprites do Caua. Devolve false se o
    // SDL/janela/renderer falharem (o main reporta e sai != 0). A SdlWindow POSSUI a
    // janela criada aqui (o dtor a destroi).
    [[nodiscard]] bool init();

    // M7-COSTURA (ADR-012 Onda 1): cria SO o renderer (Render2dSdl) numa janela JA
    // CRIADA por quem chama (a Maestro, dona da janela COMPARTILHADA entre cidade e
    // batalha - "trocar escondido atras do preto"). A SdlWindow NAO possui a janela
    // neste modo: o dtor NUNCA a destroi. Carrega a cidade + os sprites do Gus, como
    // init() faz. Devolve false se a criacao do renderer falhar.
    [[nodiscard]] bool init_attached(SDL_Window* window);

    // M7-COSTURA: solta o SDL_Renderer/Render2dSdl (a janela NAO e tocada) - chamado ao
    // ENTRAR na batalha, pra liberar a janela pro contexto GL da BattleScene (o design
    // de troca-de-backend na MESMA janela). O OverworldSim (posicao/animacao do Gus)
    // segue intacto - so os handles de textura (TextureId) que ele guarda ficam
    // OBSOLETOS ate reacquire_renderer() recarrega-los.
    void release_renderer();

    // M7-COSTURA: reconstroi o SDL_Renderer/Render2dSdl na MESMA janela (apos
    // release_renderer, ao VOLTAR da batalha) e RECARREGA os sprites do Gus - os
    // TextureId antigos nao sobrevivem a destruicao do renderer anterior (handles de
    // uma tabela de texturas que nao existe mais; custo aceito do design, documentado
    // no relatorio da Onda 1). Devolve false se SDL_CreateRenderer falhar.
    [[nodiscard]] bool reacquire_renderer();

    // Um FRAME do loop (poll -> N updates fixos -> 1 render), extraido de run() pra
    // permitir que a Maestro intercale a checagem de colisao/trigger de batalha entre
    // frames. Devolve false quando o usuario fechou a janela (pump_events devolveu
    // false) - o chamador encerra o app nesse caso. NAO renderiza (so faz poll+update)
    // se o renderer foi liberado (release_renderer) - a Maestro so chama step() com o
    // renderer da cidade ativo.
    [[nodiscard]] bool step();

    // Roda o loop ate o usuario fechar a janela (pump_events devolver false). Cada
    // iteracao: poll de input -> N updates fixos (FixedTimestep) -> 1 render. O
    // lider joga: move o Caua com WASD/setas/gamepad, desliza nas paredes, camera
    // presa ao mapa, animacao de walk por distancia. Equivale a `while (step()) {}`.
    void run();

    // Posicao ATUAL do jogador (leitura) - a Maestro checa colisao com o inimigo fixo
    // contra isto, sem precisar conhecer o OverworldSim por dentro.
    [[nodiscard]] const gus::core::spatial::Aabb& player_aabb() const noexcept;

    // Grade de colisao da cidade corrente (leitura) - a Maestro usa isto SO na
    // inicializacao, pra escolher uma celula LIVRE pro inimigo fixo (nunca dentro de
    // parede, qualquer que seja o mapa real/fallback carregado).
    [[nodiscard]] const gus::core::spatial::TileGrid& grid() const noexcept;

    // Tuning vigente do OverworldSim (leitura) - a Maestro usa isto (M7-COSTURA, fix
    // BUG-1 do playtest ao vivo: "a hitbox do inimigo so ativa vindo do sul") pra
    // derivar o AABB REAL do inimigo (colisao) a partir do MESMO player_sprite_height_
    // tiles que o marcador visual usa pra desenhar o quad do androide (overworld_sim.cpp,
    // MARCADOR DE INIMIGO FIXO) - so assim hitbox e sprite COINCIDEM. Reexpoe
    // OverworldSim::tuning() (ja publico) sem a Maestro precisar conhecer o OverworldSim
    // por dentro (mesmo espirito de player_aabb/grid acima).
    [[nodiscard]] const gus::app::screens::OverworldTuning& tuning() const noexcept;

    // M7-COSTURA Inc 2 (o inimigo fixo agora e VISIVEL): define/reposiciona o marcador
    // visual do inimigo fixo em `aabb` - carrega (ou recarrega) a MESMA textura de
    // androide que a tela de BATALHA usa pros inimigos (retrato_inimigo.png,
    // kRetratoInimigoFile) no renderer_ CORRENTE e a entrega ao OverworldSim (mesmo
    // padrao de load_player_sprites: os TextureId sao locais ao SDL_Renderer vivo). A
    // Maestro chama isto apos calcular a posicao logica do inimigo (pick_fixed_enemy_
    // position). Asset ausente/headless => nada e desenhado (degradacao segura).
    void set_enemy_marker(const gus::core::spatial::Aabb& aabb);

    // Some com o marcador (Victory, item 4 do escopo M7: "o inimigo derrotado some do
    // mapa"). No-op se nao havia marcador.
    void clear_enemy_marker();

private:
    // Carrega os sprites do Gus no renderer_ corrente e os entrega ao sim_. Extraido
    // de init() pra ser reusado por init_attached() e reacquire_renderer() (mesma
    // receita, 3 pontos de chamada).
    void load_player_sprites();

    // (Re)carrega a textura do marcador de inimigo no renderer_ CORRENTE e a reaplica ao
    // sim_ (ver set_enemy_marker no header publico). No-op se enemy_marker_aabb_ ainda
    // nao foi definida (uso standalone da cidade sem Maestro, ou antes do 1o
    // set_enemy_marker) - o mesmo motivo de load_player_sprites nao depender de posicao.
    // Chamado por set_enemy_marker() e por reacquire_renderer() (os handles de textura
    // NAO sobrevivem a troca de SDL_Renderer, mesmo racional do Gus).
    void load_enemy_marker_texture();

    SDL_Window* window_ = nullptr;      // dono SO se owns_window_ (ver init() vs init_attached())
    SDL_Renderer* renderer_ = nullptr;  // sempre owner (destruido em release_renderer/dtor)
    bool owns_window_ = false;

    std::unique_ptr<gus::platform::render2d::Render2dSdl> render2d_;
    std::unique_ptr<gus::app::screens::OverworldSim> sim_;
    gus::platform::input::SdlInput input_;
    gus::core::time::FixedTimestep clock_;
    bool have_last_time_ = false;
    unsigned long long last_ns_ = 0;

    // MARCADOR DE INIMIGO FIXO (M7-COSTURA Inc 2): AABB logica (dada pela Maestro) +
    // TextureId cacheado (local ao renderer_ vivo). nullopt = nenhum marcador definido
    // ainda (uso standalone/sem Maestro).
    std::optional<gus::core::spatial::Aabb> enemy_marker_aabb_{};
    gus::platform::render2d::TextureId enemy_marker_tex_ =
        gus::platform::render2d::kInvalidTexture;
};

}  // namespace gus::app

#endif  // GUS_APP_SDL_WINDOW_HPP
