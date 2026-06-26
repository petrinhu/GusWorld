// gus/platform/rmlui/rmlui_hud.hpp
//
// RmlUiHud: o ANDAIME do RmlUi (ADR-009, adendo GL3). Encapsula o nucleo do RmlUi e o
// backend OpenGL 3.3 (RenderInterface_GL3 = shaders: gradiente/box-shadow/blur/mask) +
// o sistema SDL3 (SystemInterface_SDL: clock/eventos), vendorizados em platform/rmlui/,
// atras de uma interface FINA da engine. Vive SO na camada platform/ (com app/): core/ e
// domain/ permanecem POCO (o GATE proibe <RmlUi e Rml:: la).
//
// REGRA DE OURO DA COMPOSICAO: o RmlUi desenha o HUD/chrome POR CIMA da arena
// (Render2dGl3) NO MESMO contexto GL, e NUNCA limpa a tela nem faz swap. A arena desenha
// primeiro (e dona do clear); o HUD compoe depois (BeginFrame do GL3 NAO limpa o
// backbuffer - so prepara estado); o dono do frame (a casca app/) faz o swap unico
// (SDL_GL_SwapWindow).
//
// HEADER LIMPO: nao expoe nenhum tipo do RmlUi nem do GL/SDL na interface publica
// (PImpl). app/ consome o wrapper sem arrastar <RmlUi...> nem <GL/SDL...> pros headers.
//
// MODO HEADLESS (espelha Render2dGl3(false)): init_headless inicializa o nucleo do RmlUi
// com um RenderInterface nulo-seguro (sem contexto GL); parseia RML/RCSS e monta
// data-model SEM GPU (o que o CI testa). compose_headless e no-op. Prova offscreen.

#ifndef GUS_PLATFORM_RMLUI_RMLUI_HUD_HPP
#define GUS_PLATFORM_RMLUI_RMLUI_HUD_HPP

#include <memory>
#include <string>

namespace gus::platform::rmlui {

class RmlUiHud {
public:
    RmlUiHud();
    ~RmlUiHud();

    RmlUiHud(const RmlUiHud&) = delete;
    RmlUiHud& operator=(const RmlUiHud&) = delete;

    // --- inicializacao ---

    // Modo COM GL (janela real): exige um contexto GL JA criado e make-current pela casca
    // (SDL_GL_CreateContext + glad carregado). Inicializa o nucleo do RmlUi sobre o
    // RenderInterface_GL3 e cria um contexto de dimensoes logicas (logical_w x logical_h,
    // ex.: 960x540). pixel_w/h sao os PIXELS REAIS do alvo (viewport GL). Carrega a fonte
    // do chrome (FreeType). false se o RmlUi/GL nao inicializar.
    [[nodiscard]] bool init(bool gl_active, int pixel_w, int pixel_h, int logical_w,
                            int logical_h);

    // Modo HEADLESS (sem janela/GPU, CI): inicializa o nucleo com um RenderInterface
    // nulo-seguro; permite parse de RML/RCSS e data-model sem desenhar.
    [[nodiscard]] bool init_headless(int logical_w, int logical_h);

    [[nodiscard]] bool is_initialized() const noexcept;

    // Define a URL BASE para resolver caminhos de asset (image() do RCSS) dos documentos
    // carregados da memoria. Sem isto, caminhos absolutos perdem a barra inicial na
    // canonicalizacao de URL do RmlUi. Passar um caminho absoluto ficticio no diretorio
    // dos assets (ex.: "/abs/dir/cockpit.rml") faz image(rel/x.png) resolver certo. Chamar
    // ANTES de load_document_from_memory.
    void set_asset_base_url(const std::string& base_url);

    // --- documentos ---

    // Carrega um documento RML de um ARQUIVO (path resolvido pela casca). Substitui o
    // documento corrente (F0/F1 trabalham com 1 doc de cockpit por vez). false se faltar
    // ou o parse falhar.
    [[nodiscard]] bool load_document(const std::string& rml_path);

    // Carrega um documento RML direto de uma STRING em memoria (usado pelos testes e por
    // docs embarcados). Substitui o documento corrente. false se o parse falhar.
    [[nodiscard]] bool load_document_from_memory(const std::string& rml_source);

    // true se o documento corrente contem um elemento com este id (sanity de parse/teste).
    [[nodiscard]] bool has_element(const std::string& id) const;

    // --- data-model (F0: demo; F1 liga aos view-models POCO) ---

    // Cria o data-model do HUD e liga os campos que o RCSS consome: nome/role + intro
    // (abertura vs combate) + HP/AP/Mana. Deve ser chamado ANTES de load_document (o doc
    // referencia data-model="<name>"). Os valores sao atualizados por frame via set_intro
    // / set_hud_values; update() marca tudo sujo.
    [[nodiscard]] bool bind_demo_model(const std::string& model_name,
                                       const std::string& nome);

    // ESTADO abertura vs combate (C): true => o RCSS mostra o brasao (data-if intro);
    // false => mostra o cockpit de combate. Chamar por frame conforme scene.is_intro().
    void set_intro(bool intro);

    // Valores VIVOS do cockpit (lidos dos view-models POCO pela casca), atualizados por
    // frame. nome/role do ator ativo + HP/AP/Mana (atual e max). update() reflete na UI.
    void set_hud_values(const std::string& nome, const std::string& role, int hp,
                        int hp_max, int ap, int ap_max, int mana, int mana_max);

    // --- ciclo de frame ---

    // Atualiza o contexto (layout/animacoes/data-model). Chamar 1x por frame antes de
    // compose. Headless tambem roda (so nao desenha).
    void update();

    // Informa o tamanho REAL em pixels do alvo (a janela pode ser redimensionada). O
    // wrapper ajusta a viewport GL e a escala logica->real ao compor (cockpit autorado em
    // 960x540, escalado pro alvo real, ex.: 1080p).
    void set_pixel_size(int pixel_w, int pixel_h);

    // COMPOE o HUD por cima do que a arena (Render2dGl3) ja desenhou no backbuffer GL.
    // NAO limpa, NAO faz swap: so emite a geometria do RmlUi (GL3). Chamar DEPOIS da arena
    // e ANTES do swap (SDL_GL_SwapWindow, feito pela casca app/).
    void compose();

    // Versao headless explicita (no-op de desenho) usada pelos testes.
    void compose_headless();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace gus::platform::rmlui

#endif  // GUS_PLATFORM_RMLUI_RMLUI_HUD_HPP
