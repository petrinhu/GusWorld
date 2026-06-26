// gus/platform/rmlui/rmlui_hud.cpp
//
// Ver header. Implementacao do andaime RmlUi (ADR-009, adendo GL3) com PImpl: nenhum tipo
// do RmlUi/GL/SDL vaza pra interface publica. Encapsula o backend OpenGL 3.3 oficial
// (RenderInterface_GL3 = shaders) + o sistema SDL3 (SystemInterface_SDL) e o ciclo do
// nucleo do RmlUi.
//
// COMPOSICAO (coexistencia arena+HUD): o GL3 BeginFrame desenha o HUD numa LAYER propria
// (framebuffer offscreen, transparente onde nao ha UI); o EndFrame BLITA essa layer no
// backbuffer (framebuffer 0) com blend premultiplied-alpha. Como a arena (Render2dGl3) ja
// desenhou no backbuffer ANTES, o HUD compoe POR CIMA sem apagar a arena. O swap
// (SDL_GL_SwapWindow) e da casca app/. Logo compose() = SetViewport + BeginFrame +
// Context::Render + EndFrame.
//
// ESCALA LOGICA->REAL (pixel-perfect): o cockpit e autorado em 960x540 (D1) usando
// unidades 'dp'. Definimos o contexto com as dimensoes em PIXELS REAIS (ex.: 1920x1080) e
// o dp_ratio = pixel_w/960, de modo que 1dp = (real/logico) px: o layout logico preenche o
// alvo real escalado por inteiro (x2 em 1080p), nitido.

#include "gus/platform/rmlui/rmlui_hud.hpp"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/RenderInterface.h>

#include "RmlUi_Platform_SDL.h"
#include "RmlUi_Renderer_GL3.h"

#include <cstdlib>
#include <string>

// Caminho da pasta de fontes da engine (.ttf), embutido pelo CMake. A fonte do chrome
// (Pixel Operator Mono) e rasterizada pelo FreeType (RMLUI_FONT_ENGINE=freetype).
#ifndef GUSWORLD_FONTS_DIR
#define GUSWORLD_FONTS_DIR ""
#endif

namespace gus::platform::rmlui {

namespace {

// Render interface NULA para o modo headless: o RmlUi exige uma render interface antes de
// Initialise(); sem contexto GL nao ha o que desenhar, entao todos os metodos sao no-op
// seguros. Permite parse de RML/RCSS e data-model no CI sem GPU.
class NullRenderInterface : public Rml::RenderInterface {
public:
    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex>,
                                                Rml::Span<const int>) override {
        return 0;
    }
    void ReleaseGeometry(Rml::CompiledGeometryHandle) override {}
    void RenderGeometry(Rml::CompiledGeometryHandle, Rml::Vector2f,
                        Rml::TextureHandle) override {}
    Rml::TextureHandle LoadTexture(Rml::Vector2i& dims, const Rml::String&) override {
        dims = {0, 0};
        return 0;
    }
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte>,
                                       Rml::Vector2i) override {
        return 0;
    }
    void ReleaseTexture(Rml::TextureHandle) override {}
    void EnableScissorRegion(bool) override {}
    void SetScissorRegion(Rml::Rectanglei) override {}
};

// Contagem de Initialise()/Shutdown() do nucleo do RmlUi (singleton global). Varios
// RmlUiHud (ex.: TEST_CASEs em sequencia) sobem e descem o nucleo; so o primeiro
// Initialise e o ultimo Shutdown valem. As INTERFACES (render/system/font) sao por-Hud,
// mas o nucleo e unico; logo refcontamos. Catch2 roda single-thread, sem corrida.
int g_core_refs = 0;

std::string resolve_fonts_dir() {
    if (const char* env = std::getenv("GUSWORLD_FONTS")) {
        if (env[0] != '\0') {
            return std::string(env);
        }
    }
    return std::string(GUSWORLD_FONTS_DIR);
}

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

constexpr int kLogicalW = 960;  // resolucao logica de autoria (D1)

}  // namespace

struct RmlUiHud::Impl {
    bool gl_active = false;  // ha contexto GL corrente?
    bool initialized = false;

    // Interfaces do backend (donas; vivas ate Shutdown). Em headless, render_gl3 fica nulo
    // e usamos a NullRenderInterface.
    std::unique_ptr<RenderInterface_GL3> render_gl3;
    std::unique_ptr<NullRenderInterface> render_null;
    std::unique_ptr<SystemInterface_SDL> system_iface;

    Rml::Context* context = nullptr;  // dono via RemoveContext no shutdown
    Rml::ElementDocument* document = nullptr;

    int pixel_w = kLogicalW;
    int pixel_h = 540;

    std::string demo_nome;
    std::string demo_model_name;
    std::string base_url;  // base para resolver image() do RCSS (assets relativos)

    // ESTADO do HUD exposto ao RCSS via data-model (atualizado por frame pela casca).
    // intro = true => mostra o brasao (abertura); false => mostra o cockpit de combate.
    bool hud_intro = true;
    // Valores de combate vivos (lidos dos view-models POCO pela casca).
    int hud_hp = 0, hud_hp_max = 1;
    int hud_ap = 0, hud_ap_max = 0;
    int hud_mana = 0, hud_mana_max = 0;
    std::string hud_role;            // papel/role do ator (ex.: "VETOR DO GAMBITO")
    Rml::DataModelHandle model_handle;  // pra marcar variaveis sujas por frame

    Rml::RenderInterface* active_render() {
        if (render_gl3) return render_gl3.get();
        return render_null.get();
    }

    // Aplica dimensoes (pixels reais) + dp_ratio (logico->real) ao contexto e a viewport.
    void apply_dimensions() {
        if (!context) return;
        context->SetDimensions(Rml::Vector2i(pixel_w, pixel_h));
        const float ratio = static_cast<float>(pixel_w) / static_cast<float>(kLogicalW);
        context->SetDensityIndependentPixelRatio(ratio);
        if (render_gl3) {
            render_gl3->SetViewport(pixel_w, pixel_h);
        }
    }

    bool init_core() {
        system_iface = std::make_unique<SystemInterface_SDL>();
        Rml::SetSystemInterface(system_iface.get());

        if (gl_active) {
            render_gl3 = std::make_unique<RenderInterface_GL3>();
            if (!*render_gl3) {
                return false;  // GL3 nao construiu (sem contexto GL valido / glad)
            }
            Rml::SetRenderInterface(render_gl3.get());
        } else {
            render_null = std::make_unique<NullRenderInterface>();
            Rml::SetRenderInterface(render_null.get());
        }

        if (g_core_refs == 0) {
            if (!Rml::Initialise()) {
                return false;
            }
        }
        ++g_core_refs;

        // Fonte do chrome via FreeType.
        const std::string fonts = resolve_fonts_dir();
        if (!fonts.empty()) {
            Rml::LoadFontFace(join(fonts, "PixelOperatorMono.ttf"), /*fallback=*/true);
            Rml::LoadFontFace(join(fonts, "PixelOperatorMono-Bold.ttf"));
        }

        context = Rml::CreateContext("gusworld_hud",
                                     Rml::Vector2i(pixel_w, pixel_h), active_render());
        if (!context) {
            return false;
        }
        apply_dimensions();
        initialized = true;
        return true;
    }

    ~Impl() {
        // ORDEM DE DESTRUICAO (ADR-009). A render interface (render_gl3) e o system_iface
        // sao MEMBROS deste Impl, destruidos DEPOIS deste corpo (ordem reversa de
        // declaracao) - logo continuam VIVOS enquanto o teardown do RmlUi roda (ele chama
        // a render interface pra liberar texturas/geometria). Isso e o que evita o UAF da
        // interface durante a limpeza.
        //
        // NAO chamamos Update() aqui (ele dispararia ReleaseUnloadedDocuments no meio do
        // teardown, mexendo na ordem interna de destruicao dos elementos do RmlUi). Em vez
        // disso: se este e o ULTIMO Hud, Rml::Shutdown() limpa TODOS os contextos e
        // documentos na ordem controlada do proprio RmlUi (Core.cpp: "Clear out all
        // contexts, which should also clean up all attached elements"). Se ha outro Hud
        // vivo, RemoveContext tira so o meu (a destruicao diferida e resolvida pelo proximo
        // Update do outro Hud).
        const bool last = initialized && (g_core_refs == 1);
        if (initialized) {
            --g_core_refs;
        }
        if (last) {
            context = nullptr;   // o Shutdown e dono da limpeza
            document = nullptr;
            Rml::Shutdown();
        } else if (context) {
            Rml::RemoveContext("gusworld_hud");
            context = nullptr;
            document = nullptr;
        }
    }
};

RmlUiHud::RmlUiHud() : impl_(std::make_unique<Impl>()) {}
RmlUiHud::~RmlUiHud() = default;

bool RmlUiHud::init(bool gl_active, int pixel_w, int pixel_h, int /*logical_w*/,
                    int /*logical_h*/) {
    impl_->gl_active = gl_active;
    impl_->pixel_w = pixel_w > 0 ? pixel_w : kLogicalW;
    impl_->pixel_h = pixel_h > 0 ? pixel_h : 540;
    return impl_->init_core();
}

bool RmlUiHud::init_headless(int logical_w, int logical_h) {
    // Headless: sem GL; o contexto usa as dimensoes logicas direto (dp_ratio = 1).
    return init(/*gl_active=*/false, logical_w, logical_h, logical_w, logical_h);
}

bool RmlUiHud::is_initialized() const noexcept { return impl_->initialized; }

void RmlUiHud::set_asset_base_url(const std::string& base_url) {
    impl_->base_url = base_url;
}

bool RmlUiHud::bind_demo_model(const std::string& model_name,
                               const std::string& nome) {
    if (!impl_->context) return false;
    impl_->demo_nome = nome;
    impl_->demo_model_name = model_name;
    Rml::DataModelConstructor c = impl_->context->CreateDataModel(model_name);
    if (!c) return false;
    // Liga TODOS os campos que o RCSS consome: nome/role + intro (abertura vs combate) +
    // HP/AP/Mana. Os ponteiros apontam pros membros do Impl (atualizados por frame pela
    // casca; DirtyAllVariables marca tudo sujo em update()).
    bool ok = true;
    ok = ok && c.Bind("nome", &impl_->demo_nome);
    ok = ok && c.Bind("role", &impl_->hud_role);
    ok = ok && c.Bind("intro", &impl_->hud_intro);
    ok = ok && c.Bind("hp", &impl_->hud_hp);
    ok = ok && c.Bind("hp_max", &impl_->hud_hp_max);
    ok = ok && c.Bind("ap", &impl_->hud_ap);
    ok = ok && c.Bind("ap_max", &impl_->hud_ap_max);
    ok = ok && c.Bind("mana", &impl_->hud_mana);
    ok = ok && c.Bind("mana_max", &impl_->hud_mana_max);
    impl_->model_handle = c.GetModelHandle();
    return ok;
}

void RmlUiHud::set_intro(bool intro) { impl_->hud_intro = intro; }

void RmlUiHud::set_hud_values(const std::string& nome, const std::string& role, int hp,
                              int hp_max, int ap, int ap_max, int mana, int mana_max) {
    impl_->demo_nome = nome;
    impl_->hud_role = role;
    impl_->hud_hp = hp;
    impl_->hud_hp_max = hp_max > 0 ? hp_max : 1;
    impl_->hud_ap = ap;
    impl_->hud_ap_max = ap_max;
    impl_->hud_mana = mana;
    impl_->hud_mana_max = mana_max;
}

bool RmlUiHud::load_document_from_memory(const std::string& rml_source) {
    if (!impl_->context) return false;
    if (impl_->document) {
        impl_->context->UnloadDocument(impl_->document);
        impl_->document = nullptr;
        impl_->context->Update();
    }
    // source_url: base para resolver caminhos de asset (image() do RCSS) relativos. Quando
    // vazio, o doc da memoria nao tem base e caminhos absolutos perdem a barra inicial
    // (canonicalizacao de URL do RmlUi). Passar um source_url absoluto (ex.: um .rml ficticio
    // no diretorio dos assets) faz image(rel/x.png) resolver corretamente.
    const Rml::String url = impl_->base_url.empty() ? Rml::String("[document from memory]")
                                                    : Rml::String(impl_->base_url);
    impl_->document = impl_->context->LoadDocumentFromMemory(rml_source, url);
    if (!impl_->document) return false;
    impl_->document->Show();
    return true;
}

bool RmlUiHud::load_document(const std::string& rml_path) {
    if (!impl_->context) return false;
    if (impl_->document) {
        impl_->context->UnloadDocument(impl_->document);
        impl_->document = nullptr;
        impl_->context->Update();
    }
    impl_->document = impl_->context->LoadDocument(rml_path);
    if (!impl_->document) return false;
    impl_->document->Show();
    return true;
}

bool RmlUiHud::has_element(const std::string& id) const {
    if (!impl_->document) return false;
    return impl_->document->GetElementById(id) != nullptr;
}

void RmlUiHud::update() {
    if (impl_->context) {
        // Marca o data-model sujo: os valores (intro/HP/AP/Mana/nome) mudam por frame na
        // casca; sem DirtyAllVariables o RmlUi nao reavalia os data-* bindings.
        if (impl_->model_handle) {
            impl_->model_handle.DirtyAllVariables();
        }
        impl_->context->Update();
    }
}

void RmlUiHud::set_pixel_size(int pixel_w, int pixel_h) {
    if (pixel_w <= 0 || pixel_h <= 0) return;
    impl_->pixel_w = pixel_w;
    impl_->pixel_h = pixel_h;
    impl_->apply_dimensions();
}

void RmlUiHud::compose() {
    if (!impl_->context || !impl_->render_gl3) {
        return;  // headless ou nao iniciado: nada a desenhar
    }
    // GUARD 0x0 (robustez): em Wayland a janela pode ficar minimizada/ocluida e reportar
    // tamanho 0. O GL3 do RmlUi ASSERTA viewport >= 1; em Release o assert e no-op, mas
    // glViewport(0,0,0,0) + FBO de tamanho 0 levaria a erro de GL/crash em alguns drivers.
    // Sem dimensao valida, NAO compoe este frame (a arena ja desenhou; so pulamos o HUD).
    if (impl_->pixel_w < 1 || impl_->pixel_h < 1) {
        return;
    }
    // GL3: BeginFrame desenha o HUD numa layer offscreen (limpa SO a layer, nao o
    // backbuffer); EndFrame blita a layer no backbuffer (framebuffer 0) com premultiplied
    // alpha, compondo POR CIMA da arena ja desenhada. Nao limpa a arena, nao faz swap.
    impl_->render_gl3->SetViewport(impl_->pixel_w, impl_->pixel_h);
    impl_->render_gl3->BeginFrame();
    impl_->context->Render();
    impl_->render_gl3->EndFrame();
}

void RmlUiHud::compose_headless() {
    // Sem contexto GL: no-op (o Render() chamaria a NullRenderInterface, tudo no-op).
}

}  // namespace gus::platform::rmlui
