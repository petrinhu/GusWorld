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
// FLASH-CTX (A1, contexto GL UNICO - docs/tech/pivot/menu-flash-contexto-unico-plano.md):
// a cidade desenhava com Render2dSdl (SDL_Renderer). Agora desenha com Render2dGl3, no
// MESMO contexto GL que a Maestro cria UMA vez no boot (gus/app/maestro.cpp) e mantem vivo
// ate o shutdown - ZERO SDL_CreateRenderer/SDL_DestroyRenderer na cidade. init_attached()
// (o caminho de PRODUCAO, usado pela Maestro) ASSUME que o contexto GL ja esta CORRENTE
// (SDL_GL_MakeCurrent) quando e chamado - nao cria nem faz make-current sozinho, MESMO
// padrao "assume contexto corrente" de gus/app/screens/system_menu_loop.hpp::
// run_system_menu_loop_gl_current. init() (caminho STANDALONE, sem Maestro - HOJE SEM
// NENHUM CHAMADOR de producao/teste/tool, mantido por simetria de API) e dono do PROPRIO
// contexto GL (cria/faz current/destroi, MESMA receita da Maestro).
//
// PONTE TEMPORARIA (bridge, ate o A3 podar release_renderer/reacquire_renderer -
// ver o comentario grande dos dois metodos abaixo): NENHUM dos dois destroi o
// Render2dGl3/contexto - viram um GUARD booleano (renderer_paused_) que impede
// GL enquanto uma casca "owning" (menu/dialogo/titulo/batalha, que ainda cria um
// contexto GL PROPRIO por cima do da Maestro nesta etapa) pode estar com o SEU
// contexto corrente. Restaurar o contexto da Maestro como corrente (SDL_GL_
// MakeCurrent) ao VOLTAR de uma casca owning e responsabilidade de QUEM CHAMA
// (gus/app/maestro.cpp, dono do SDL_GLContext) - a SdlWindow nao conhece o
// handle do contexto da Maestro.
//
// Inclui <SDL3/SDL.h> (camada app/, SDL permitido). O irredutivel (criar janela/
// contexto GL, apresentar) e coberto pelo smoke headless do main (--smoke com
// SDL_VIDEODRIVER=dummy - Render2dGl3 em modo headless, gl_active=false, todo draw
// vira no-op contabilizado, mesma degradacao que Render2dSdl(nullptr) ja tinha).

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
#include "gus/platform/render2d/render2d_gl3.hpp"  // FLASH-CTX: GL3 (era render2d_sdl.hpp)

namespace gus::app {

class SdlWindow {
public:
    SdlWindow();
    ~SdlWindow();

    SdlWindow(const SdlWindow&) = delete;
    SdlWindow& operator=(const SdlWindow&) = delete;

    // Cria janela + contexto GL PROPRIO (3.3 core/doublebuffer/stencil 8, MESMA receita
    // de gus/app/maestro.cpp) + carrega os sprites do Gus. Devolve false se o SDL/
    // janela/contexto falharem (o main reporta e sai != 0). A SdlWindow POSSUI a janela
    // E o contexto GL criados aqui (o dtor destroi os dois, contexto primeiro). FLASH-CTX:
    // caminho STANDALONE (sem Maestro) - SEM NENHUM CHAMADOR de producao/teste/tool hoje
    // (a Maestro sempre usa init_attached() abaixo); mantido por simetria de API/futuro-
    // proofing barato, nao dead-code-removido de proposito (decisao de remocao e do M9).
    [[nodiscard]] bool init();

    // FLASH-CTX (A1, era M7-COSTURA Onda 1): cria o Render2dGl3 (arena/cidade em GL, o
    // MESMO backend que a batalha ja usa) numa janela JA CRIADA por quem chama, com um
    // CONTEXTO GL JA CORRENTE (SDL_GL_MakeCurrent) - a Maestro cria/faz-current o contexto
    // GL UNICO (vivo do boot ao shutdown) ANTES de chamar isto (ver gus/app/maestro.cpp::
    // init()). Este metodo NAO cria contexto nem faz make-current (mesmo padrao "assume
    // contexto corrente" de run_system_menu_loop_gl_current/run_battle_preview_embedded_
    // gl_current) - so constroi o Render2dGl3(gl_active=true) no contexto que ja esta
    // current. A SdlWindow NAO possui a janela neste modo: o dtor NUNCA a destroi (nem o
    // contexto GL - que tambem NAO e desta SdlWindow, e da Maestro). Carrega a cidade + os
    // sprites do Gus, como init() faz. Devolve false se a criacao do Render2dGl3 falhar
    // (programa GL nao compilou/linkou - degrada pra headless, ver Render2dGl3::Render2dGl3).
    [[nodiscard]] bool init_attached(SDL_Window* window);

    // FLASH-CTX PONTE TEMPORARIA (A1; poda definitiva e o A3, passo 6 do plano): NAO
    // destroi mais nada (o Render2dGl3/contexto GL agora e UNICO e vive do boot ao
    // shutdown, dono e a Maestro) - vira um GUARD booleano (renderer_paused_=true) que
    // impede QUALQUER chamada GL enquanto uma casca "owning" (menu de pausa/dialogo/
    // titulo/batalha) pode estar com o SEU PROPRIO contexto GL corrente por cima do da
    // Maestro (nesta etapa do refactor essas cascas AINDA criam um contexto proprio -
    // ver o comentario grande em gus/app/maestro.cpp). Chamado ao ENTRAR num desses
    // fluxos, MESMO NOME/local de chamada de antes (nao quebra os call-sites da Maestro).
    // O OverworldSim (posicao/animacao do Gus) e os handles de textura (TextureId) NAO
    // sao mais afetados (nada e destruido) - reacquire_renderer() so precisa DESPAUSAR.
    void release_renderer();

    // FLASH-CTX PONTE TEMPORARIA (A1): despausa (renderer_paused_=false) - NAO recria
    // nada (nao ha mais o que recriar: o Render2dGl3/contexto sobreviveu inteiro). Ainda
    // assim RECARREGA os marcadores de inimigo/Bertoldo incondicionalmente (load_enemy_
    // marker_texture/load_npc_bertoldo_marker_texture) - eles PODEM ter sido pedidos
    // ENQUANTO pausado (ex.: o callback de LOAD do menu de Save/Load reposiciona/limpa o
    // marcador via Maestro::apply_loaded_save_data -> set_enemy_marker, e o guard de
    // renderer_paused_ tera adiado a carga da TEXTURA - so o AABB logico foi guardado).
    // load_player_sprites()/load_boot_pixel_frames() TAMBEM sao chamados aqui (mesma
    // chamada de sempre) mas agora sao BARATOS/no-op de fato: Render2dGl3::load_texture
    // cacheia por caminho (o TextureId de um path ja carregado volta na hora, sem GL novo)
    // - o contexto nunca morreu, entao os handles de sempre continuam validos. IMPORTANTE:
    // exige que o CONTEXTO DA MAESTRO ja esteja CORRENTE quando chamado (a Maestro
    // restaura via SDL_GL_MakeCurrent ANTES de chamar isto, ver maestro.cpp) - esta funcao
    // NAO faz make-current sozinha (nao conhece o handle do contexto da Maestro). Sempre
    // devolve true nesta ponte (nao ha mais criacao de SDL_Renderer pra falhar); a
    // assinatura fica bool por compatibilidade de API com os call-sites existentes (o A3
    // decide se simplifica pra void na poda final).
    [[nodiscard]] bool reacquire_renderer();

    // Um FRAME do loop (poll -> N updates fixos -> 1 render), extraido de run() pra
    // permitir que a Maestro intercale a checagem de colisao/trigger de batalha entre
    // frames. Devolve false quando o usuario fechou a janela (pump_events devolveu
    // false) - o chamador encerra o app nesse caso. NAO renderiza (so faz poll+update)
    // se o renderer estiver pausado (release_renderer, FLASH-CTX) - a Maestro so chama
    // step() com o renderer da cidade ativo.
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

    // TELEPORTE (SAVE-LOAD-UI etapa 6, Carregar REAL): reexpoe OverworldSim::
    // set_player_position (ja publico) sem a Maestro precisar conhecer o
    // OverworldSim por dentro (mesmo espirito de player_aabb acima). A Maestro
    // chama isto SO ao aplicar um SaveData carregado do disco.
    void set_player_position(const gus::core::spatial::Aabb& aabb) noexcept;

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
    // desenho (mas nao crasha) se o renderer estiver pausado (release_renderer, FLASH-CTX).
    void render_dialogue_overlay_frame(const std::vector<std::string>& lines);

    // FUNDO REAL CONGELADO (M7-DIALOGO/MENU-PAUSA-CONFIG-SOM, decisao do lider). FLASH-CTX
    // (A1, passo 4 do plano - DEFAULT escolhido: PNG congelado via glReadPixels, paridade
    // visual estrita; a alternativa "redesenhar a cidade sob a UI a cada frame" fica
    // registrada como decisao do LIDER pendente, nao tomada aqui): captura o frame ATUAL
    // da cidade (o que esta no backbuffer agora, sim_ PARADO - nenhum step_fixed) em RGBA
    // e escreve num PNG em `out_path` - desenha 1 frame (Render2dGl3, contexto GL
    // corrente), le o BACKBUFFER via gus::platform::rmlui::gl3_read_backbuffer_rgba
    // (glReadPixels + flip vertical, MESMA funcao que a batalha ja usa pro smoke visual)
    // ANTES do swap, escreve o PNG (stb_image_write), e SO ENTAO apresenta esse MESMO
    // frame (SDL_GL_SwapWindow - byte-identico ao que o jogador ja estava vendo, so
    // desenhado 1x a mais, mesmo racional de antes com SDL_RenderReadPixels). Chamar
    // SEMPRE ANTES de release_renderer() (a captura exige GL ativo - a Maestro usa isto em
    // to_npc_dialogue()/open_pause_from_city() pra dar aos loops GL (dialogo/menu de
    // pausa) a CENA REAL da cidade como fundo estatico, no lugar da vinheta abstrata
    // (mesmo padrao de Chrono Trigger/Zelda/Stardew Valley). Devolve false se o renderer
    // estiver pausado (renderer_paused_) ou a captura/escrita falhar (degradacao segura -
    // o chamador cai pro fundo abstrato de sempre).
    [[nodiscard]] bool capture_frame_to_png(const std::string& out_path);

    // MENU-PAUSA-FLASH-FIX (achado de playtest ao vivo do lider - o filho dele, 11 anos,
    // notou um flash rapido ao FECHAR o menu de pausa). FLASH-CTX (A1): este era o
    // MASCARAMENTO do sintoma que a Opcao C ataca na RAIZ (zero troca de contexto na
    // cidade) - o metodo continua existindo POR ORA (a ponte temporaria desta etapa AINDA
    // deixa as cascas owning trocarem contexto por cima do da Maestro, ver o comentario de
    // release_renderer() acima; o A3 remove esta chamada quando a raiz sumir de vez).
    // Redesenha e APRESENTA (SDL_GL_SwapWindow POR ITERACAO - Render2dGl3::end_frame() NAO
    // apresenta sozinho, ao contrario do Render2dSdl antigo; ver o .cpp) a cena da cidade
    // PARADA (sim_ NAO avanca - alpha=1.0 sem interpolar, MESMA tecnica de render_
    // dialogue_overlay_frame/capture_frame_to_png) `frames` vezes seguidas, cobrindo TODAS
    // as imagens do swapchain com o conteudo CORRETO antes de devolver o controle ao loop
    // normal. Chamar APOS reacquire_renderer() ter sucesso (E apos a Maestro ja ter
    // restaurado o contexto GL dela como corrente - ver o comentario de reacquire_
    // renderer()). No-op seguro se o renderer estiver pausado (mesmo guard de render_
    // dialogue_overlay_frame).
    void hold_frozen_frame(int frames = 2);

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

    // M2 (ligando a tela Controles ao input REAL): repassa `config` pro
    // SdlInput::set_controls interno - a Maestro chama isto (1) no BOOT, com o
    // controls.json carregado do disco (ou default_controls() se ausente/
    // corrompido), e (2) ao FECHAR o menu de pausa (Esc -> Controles -> remapear
    // -> Voltar), com o controls.json RELIDO do disco - assim o remap do
    // jogador vale IMEDIATAMENTE, sem reiniciar o jogo. Ver
    // gus::platform::input::SdlInput::set_controls pro efeito exato (reconstroi
    // o InputMapper; nenhuma tecla de movimento fica "presa" na troca).
    void set_controls(gus::domain::input::InputRemapConfig config);

private:
    // Carrega os sprites do Gus no render2d_ corrente e os entrega ao sim_. Extraido
    // de init() pra ser reusado por init_attached() e reacquire_renderer() (mesma
    // receita, 3 pontos de chamada). FLASH-CTX: Render2dGl3::load_texture cacheia por
    // caminho - reusar em reacquire_renderer() (contexto que nunca morreu) e barato
    // (cache hit), nao GL novo.
    void load_player_sprites();

    // (Re)carrega a textura do marcador de inimigo no render2d_ CORRENTE e a reaplica ao
    // sim_ (ver set_enemy_marker no header publico). No-op se enemy_marker_aabb_ ainda
    // nao foi definida (uso standalone da cidade sem Maestro, ou antes do 1o
    // set_enemy_marker) - o mesmo motivo de load_player_sprites nao depender de posicao.
    // FLASH-CTX: TAMBEM no-op (defer) se renderer_paused_ (ver o guard no .cpp e o
    // comentario de release_renderer() no header publico - uma casca owning pode estar
    // com o SEU PROPRIO contexto GL corrente; tocar GL aqui criaria a textura no
    // contexto ERRADO). Chamado por set_enemy_marker() e por reacquire_renderer()
    // (incondicional, cobre o caso "pedido enquanto pausado" - ver o comentario de
    // reacquire_renderer() no header publico).
    void load_enemy_marker_texture();

    // (Re)carrega a textura ESTATICA do marcador do Bertoldo (south.png) no render2d_
    // CORRENTE e a reaplica ao sim_ (ver set_npc_bertoldo_marker no header publico).
    // No-op se npc_bertoldo_marker_aabb_ ainda nao foi definida. FLASH-CTX: mesmo guard
    // de renderer_paused_ de load_enemy_marker_texture acima (mesmo racional). Chamado
    // por set_npc_bertoldo_marker() e por reacquire_renderer().
    void load_npc_bertoldo_marker_texture();

    // Carrega os 20 frames do boot pixelizado (BootPixelOverlay, M7-COSTURA Inc 2c) no
    // render2d_ CORRENTE. Chamado por init()/init_attached() (1a carga) e por
    // reacquire_renderer() (idempotente/barato - ver load_player_sprites acima, mesmo
    // cache por caminho). Asset ausente/headless: BootPixelOverlay::load() devolve
    // false e step_with_fade degrada com seguranca pro retangulo solido (ver o header
    // do overlay).
    void load_boot_pixel_frames();

    SDL_Window* window_ = nullptr;  // dono SO se owns_window_ (ver init() vs init_attached())
    bool owns_window_ = false;

    // FLASH-CTX (A1): contexto GL PROPRIO, so existe/e destruido no caminho STANDALONE
    // (init(), owns_window_==true - HOJE sem chamador de producao, ver o comentario de
    // init() no header publico). No caminho ANEXADO (init_attached(), a Maestro) o
    // contexto e da MAESTRO - esta SdlWindow nunca cria nem destroi um contexto ali.
    SDL_GLContext gl_context_ = nullptr;

    // FLASH-CTX PONTE TEMPORARIA (A1, ver release_renderer()/reacquire_renderer() acima):
    // true enquanto uma casca "owning" pode ter o PROPRIO contexto GL corrente - todo
    // metodo que toca GL (load_*_texture, step/step_with_fade/render_dialogue_overlay_
    // frame/hold_frozen_frame/capture_frame_to_png) checa isto e degrada pra no-op, MESMO
    // papel que "render2d_ == nullptr" tinha na era SDL_Renderer (o guard mudou de
    // "ponteiro nulo" pra "flag booleana" porque o Render2dGl3 agora NUNCA e destruido).
    bool renderer_paused_ = false;

    std::unique_ptr<gus::platform::render2d::Render2dGl3> render2d_;
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
