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
#include <string>
#include <vector>

#include <SDL3/SDL.h>

#include "gus/app/boot_pixel_overlay.hpp"  // sequencia de frames da transicao (M7-COSTURA Inc 2c)
#include "gus/app/screens/overworld_sim.hpp"
#include "gus/core/anim/fade_transition.hpp"  // FadeDirection (qual perna do boot pixelizado)
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

    // M7-COSTURA Inc 2 (ADR-012 decisao 5, "fade preto curto") / Inc 2c (sequencia de
    // frames pre-renderizada no lugar do glitch procedural vetado pelo lider): MESMO
    // passo de step() (poll -> N updates fixos -> 1 render), com o overlay da
    // transicao (BootPixelOverlay - retangulo solido de seguranca + o frame corrente
    // do boot pixelizado) desenhado POR CIMA do frame ja renderizado, ANTES do
    // present. overlay_alpha<=0.0f e BYTE-IDENTICO a
    // step() (nenhum overlay, nenhum present adiado): step() e literalmente
    // `return step_with_fade(0.0f);`. overlay_alpha>0 clampa em [0,1] e usa o mesmo
    // present-diferido (ADR-009, set_defer_present) que o HUD RmlUi ja usa pra compor
    // por cima da arena - aqui e o Maestro quem compoe o preto por cima, sem HUD. A
    // curva do alpha (crescente/decrescente ao longo do tempo) e responsabilidade do
    // CHAMADOR (gus::core::anim::fade_overlay_alpha); esta funcao so DESENHA o alpha
    // dado num frame. `direction` (Inc 2c, NOVO): a CIDADE serve as DUAS pernas do
    // boot pixelizado que tocam no lado dela (kOut = cidade escurecendo indo pra
    // batalha = BootPixelLeg::kToBattleDarkening; kIn = cidade revelando voltando da
    // batalha = BootPixelLeg::kFromBattleRevealing - ver o header do POCO
    // boot_pixel_sequence.hpp pro desenho completo das 4 pernas) - o valor default
    // (kOut) e inofensivo quando overlay_alpha<=0 (o direction nem chega a ser usado,
    // preserva o contrato "byte-identico" de step() acima). Mesmo contrato de retorno
    // de step() (false = janela fechou).
    [[nodiscard]] bool step_with_fade(
        float overlay_alpha,
        gus::core::anim::FadeDirection direction =
            gus::core::anim::FadeDirection::kOut);

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

    // M7-DIALOGO (NPC-MVP, integracao do sprite): define/reposiciona o marcador
    // visual do Bertoldo em `aabb` - carrega (ou recarrega) a textura ESTATICA
    // south.png (kBertoldoSpritesDir/kBertoldoSpriteSouthFile, a pose "de frente pro
    // jogador/camera" - o NPC nao anda, so essa 1 direcao e usada) no renderer_
    // CORRENTE e a entrega ao OverworldSim (mesmo padrao de set_enemy_marker acima,
    // slot PROPRIO no sim_ - ver OverworldSim::set_npc_bertoldo_marker). A Maestro
    // chama isto apos calcular a posicao logica do Bertoldo. Asset ausente/headless
    // => nada e desenhado (degradacao segura, mesmo racional do marcador de inimigo).
    void set_npc_bertoldo_marker(const gus::core::spatial::Aabb& aabb);

    // Some com o marcador do Bertoldo. No-op se nao havia marcador (paridade com
    // clear_enemy_marker acima; sem uso previsto hoje - o Bertoldo nunca "derrota" -
    // mas mantido por simetria/futuro-proofing barato).
    void clear_npc_bertoldo_marker();

    // M7-DIALOGO (NPC-MVP): desenha 1 frame ESTATICO da cidade (NAO avanca
    // step_fixed - o jogador fica PARADO durante o dialogo, mesma alpha=1.0 sempre,
    // sem interpolar) com um overlay funcional SIMPLES de texto (caixa semi-opaca +
    // `lines`, ancorada no rodape do enquadramento visivel via camera_view/cam.rect
    // - MESMA tecnica de composicao/present-diferido de step_with_fade, SEM GL/
    // glintfx) desenhado por cima via IRenderer::draw_filled_rect/draw_text. A
    // apresentacao visual FINA (RCSS terminal/caixa-quente) e o item paralelo
    // DIALOGO-TERMINAL; isto aqui so prova o CICLO. NAO pumpa eventos - o chamador
    // (o loop de dialogo, ver gus/app/screens/npc_dialogue_loop.hpp) faz o proprio
    // poll de tecla, MESMO padrao independente de SdlInput que system_menu_loop.cpp
    // ja usa pro menu de pausa (navegacao de UI != movimento do jogador). No-op de
    // desenho (mas nao crasha) se o renderer estiver liberado (release_renderer).
    void render_dialogue_overlay_frame(const std::vector<std::string>& lines);

    // FUNDO REAL CONGELADO (M7-DIALOGO/MENU-PAUSA-CONFIG-SOM, decisao do lider):
    // captura o frame ATUAL da cidade (o que esta na tela agora, sim_ PARADO -
    // nenhum step_fixed) em RGBA e escreve num PNG em `out_path` (SDL_
    // RenderReadPixels no SDL_Renderer vivo + stbi_write_png, ver .cpp pra a
    // tecnica completa). Chamar SEMPRE ANTES de release_renderer() (a captura
    // exige o SDL_Renderer da cidade ainda vivo) - a Maestro usa isto em
    // to_npc_dialogue()/open_pause_from_city() pra dar aos loops GL (dialogo/menu
    // de pausa) a CENA REAL da cidade como fundo estatico, no lugar da vinheta
    // abstrata (mesmo padrao de Chrono Trigger/Zelda/Stardew Valley). Devolve
    // false se o renderer estiver liberado ou a captura/escrita falhar
    // (degradacao segura - o chamador cai pro fundo abstrato de sempre).
    [[nodiscard]] bool capture_frame_to_png(const std::string& out_path);

    // MENU-PAUSA-CONFIG-SOM: repassa o EDGE do Esc drenado pelo input_ (ver
    // SdlInput::consume_escape_pressed) - o gancho unico do MENU DE PAUSA na
    // CIDADE (que, ao contrario da batalha, nao tem pilha de modais pra Esc
    // desempilhar antes). Chamar apos step()/step_with_fade() (que ja bombeou os
    // eventos deste frame via input_.pump_events()). Consome: uma 2a chamada sem
    // novo press devolve false.
    [[nodiscard]] bool consume_escape_pressed() noexcept;

    // FIX BUG (playtest ao vivo do lider, M7-DIALOGO NPC-MVP: "Gus anda sozinho
    // apos fechar o dialogo com o Bertoldo"): solta TODA tecla de movimento
    // segurada no estado de input da cidade (mesmo efeito de SDL_EVENT_WINDOW_
    // FOCUS_LOST -> SdlInput::clear(), ver sdl_input.hpp). CAUSA RAIZ: qualquer
    // loop MODAL que faz o proprio SDL_PollEvent independente (dialogo, menu de
    // pausa - ver npc_dialogue_loop.cpp/npc_dialogue_loop_gl.cpp/system_menu_loop.cpp)
    // NUNCA repassa os eventos pro input_ desta SdlWindow (input_.pump_events() so
    // roda dentro de step()/step_with_fade(), que a Maestro NAO chama enquanto o
    // modal esta aberto). Se o jogador estava segurando uma tecla de movimento ao
    // entrar no modal e a SOLTA durante a conversa, o SDL_EVENT_KEY_UP correspondente
    // e descartado pelo loop modal (nunca chega em input_) - o estado interno
    // continua "pressionada" para sempre, e o Gus retoma andando na cidade ate
    // esbarrar em algo. Chamar isto ao ENTRAR (congela o movimento em "nada
    // pressionado", coerente com o jogador olhando pra uma caixa de dialogo/menu -
    // nao faz sentido ele estar "tentando mover" nesse momento) E ao SAIR (garante
    // que o estado comeca limpo ao retomar a cidade, mesmo se algo mudasse
    // input_ no meio do caminho) fecha o bug pela raiz, sem exigir que cada loop
    // modal reimplemente o roteamento de KEY_UP.
    void clear_input() noexcept;

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

    // (Re)carrega a textura ESTATICA do marcador do Bertoldo (south.png) no renderer_
    // CORRENTE e a reaplica ao sim_ (ver set_npc_bertoldo_marker no header publico).
    // No-op se npc_bertoldo_marker_aabb_ ainda nao foi definida. Chamado por
    // set_npc_bertoldo_marker() e por reacquire_renderer() (mesmo racional de
    // load_enemy_marker_texture acima - os TextureId nao sobrevivem a troca de
    // SDL_Renderer).
    void load_npc_bertoldo_marker_texture();

    // Carrega os 20 frames do boot pixelizado (BootPixelOverlay, M7-COSTURA Inc 2c) no
    // renderer_ CORRENTE. Chamado por init()/init_attached() (1a carga) e por
    // reacquire_renderer() (os TextureId antigos nao sobrevivem a troca de
    // SDL_Renderer, mesmo motivo de load_player_sprites/load_enemy_marker_texture).
    // Asset ausente/headless: BootPixelOverlay::load() devolve false e step_with_fade
    // degrada com seguranca pro retangulo solido (ver o header do overlay).
    void load_boot_pixel_frames();

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

    // MARCADOR DO NPC BERTOLDO (M7-DIALOGO, NPC-MVP): AABB logica (dada pela
    // Maestro) + TextureId cacheado (local ao renderer_ vivo). nullopt = nenhum
    // marcador definido ainda (uso standalone/sem Maestro). Slot PROPRIO, distinto
    // do marcador de inimigo acima.
    std::optional<gus::core::spatial::Aabb> npc_bertoldo_marker_aabb_{};
    gus::platform::render2d::TextureId npc_bertoldo_marker_tex_ =
        gus::platform::render2d::kInvalidTexture;

    // M7-COSTURA Inc 2c: sequencia de frames do boot pixelizado (substitui o glitch
    // procedural vetado pelo lider). Carregada em init()/init_attached()/
    // reacquire_renderer() (load_boot_pixel_frames) - handles locais ao renderer_
    // vivo, mesmo racional do marcador de inimigo/sprites do Gus acima.
    gus::app::BootPixelOverlay boot_overlay_;
};

}  // namespace gus::app

#endif  // GUS_APP_SDL_WINDOW_HPP
