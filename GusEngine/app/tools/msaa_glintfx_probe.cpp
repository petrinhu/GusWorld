// PROBE EFEMERO (NAO COMMITADO - app/tools/ e UNTRACKED). Descartar apos a prova.
//
// PERGUNTA UNICA: se o jogo habilitar MSAA (SDL_GL_SetAttribute MULTISAMPLEBUFFERS=1 +
// MULTISAMPLESAMPLES=4) no contexto GL ANTES de criar a janela, o composite da UI do
// glintfx (embed mode, glintfx::UiLayer) continua funcionando, ou quebra?
//
// CONTEXTO (ver GusEngine/build/linux-release/_deps/glintfx-src/glintfx/CMakeLists.txt
// linha ~294-302 e AGENTS.md "Gotchas criticos"): o glintfx compila RmlUi com
// RMLUI_NUM_MSAA_SAMPLES=0 DE PROPOSITO - workaround do bug Mesa/llvmpipe onde
// glBlitFramebuffer MSAA->nao-MSAA produz textura de pos-processo preta SILENCIOSA. Lendo
// o source do RmlUi (Backends/RmlUi_Renderer_GL3.cpp):
//   - RenderInterface_GL3::EndFrame() faz um glBlitFramebuffer "Resolve MSAA to postprocess
//     framebuffer" ENTRE 2 FBOs INTERNOS DO PROPRIO RmlUi (fb_active -> fb_postprocess,
//     linha ~902-906). Com RMLUI_NUM_MSAA_SAMPLES=0 esse blit vira 0->0 amostras (nunca
//     dispara o bug).
//   - O COMPOSITE FINAL pro FBO 0 (o backbuffer da JANELA, ou seja, o FBO do HOST) e um
//     DESENHO NORMAL via shader (glBindFramebuffer(GL_FRAMEBUFFER, 0); Gfx::BindTexture(
//     fb_postprocess); UseProgram(Passthrough); DrawFullscreenQuad(); linha ~908-917) - NAO
//     um glBlitFramebuffer. Um draw call normal escrevendo num FBO 0 multisample e GL
//     100% legal e portavel (todo hardware, nao so GPU real) - NADA na cadeia interna do
//     glintfx/RmlUi jamais faz blit CRUZANDO contagem-de-amostras contra o FBO 0 do host.
//
// HIPOTESE (fundamentada em leitura de source, testada empiricamente abaixo): o MSAA do
// framebuffer default do HOST e ORTOGONAL ao pipeline interno do glintfx. O UNICO ponto de
// atrito e NOSSO: nao existe forma portavel/valida em GL de dar glReadPixels() direto num
// FBO 0 MULTISAMPLE (a spec exige GL_SAMPLE_BUFFERS==0 no framebuffer de leitura) - ISSO
// exige um resolve (glBlitFramebuffer MSAA->non-MSAA) por conta NOSSA pra tirar o screenshot
// de prova, o que reproduz EXATAMENTE o padrao que o llvmpipe tem bug conhecido. Por isso
// rodamos 3 cenarios (ver run_scenario) pra DISTINGUIR "quebrou por causa do glintfx" de
// "quebrou por causa do MEU proprio resolve-blit de leitura, sob llvmpipe, independente do
// glintfx":
//
//   S0 baseline_msaa0_with_ui   : samples=0, WITH ui.render(). Leitura DIRETA (legal).
//   S1 msaa4_backdrop_only      : samples=4, SEM glintfx (so o "jogo": glClear). Leitura via
//                                 resolve-blit MEU. CONTROLE: isola se o bug e do MEU blit.
//   S2 msaa4_with_ui            : samples=4, COM ui.render() por cima. Leitura via
//                                 resolve-blit MEU. O TESTE REAL.
//
// Se S1 (sem glintfx) ja falhar (ficar preto) sob este Xvfb/llvmpipe, isso e um artefato
// do MEU proprio resolve-blit de leitura (mesma classe de bug documentada pelo glintfx),
// NAO evidencia contra o glintfx - nesse caso S2 falhar tambem nao prova nada sobre o
// glintfx (ambos os cenarios MSAA ficam cegos por igual, pela MESMA causa). Se S1 passar
// (cores corretas) mas S2 falhar, ISSO SIM seria evidencia real de quebra causada pela
// interacao com o glintfx.
//
// Uso: ./msaa_glintfx_probe [dir_de_saida_dos_pngs]  (default: /tmp)

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <SDL3/SDL.h>
#include <glintfx/ui_layer.hpp>

// Loader glad (GL 3.3 core), MESMO header que gus::platform::rmlui::gl3_loader.cpp usa em
// producao. UMA TU dona da implementacao (GLAD_GL_IMPLEMENTATION) - so este .cpp no probe.
// O glintfx usa SEU PROPRIO loader interno (gl3w/glloader), zero conflito de simbolo (ver
// comentario em platform/rmlui/gl3_loader.cpp).
#define GLAD_GL_IMPLEMENTATION
#include "RmlUi_Include_GL3.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace {

constexpr int kW = 640;
constexpr int kH = 480;

// Regiao do box da UI (RCSS px == GL px fisico aqui: janela 640x480, logical_width/height
// iguais, dp_ratio=1.0 - sem escala). Mesma convencao top-left/y-down de get_element_box.
constexpr int kBoxLeft = 100, kBoxTop = 100, kBoxW = 300, kBoxH = 150;
// Cor esperada do box da UI: background-color: #3388ffff (R=51 G=136 B=255, alpha=255).
constexpr int kBoxR = 51, kBoxG = 136, kBoxB = 255;
// Cor de "clear" do jogo (o "mundo" por baixo da UI, fora do box): verde medio opaco.
constexpr float kBackdropR = 60.0f / 255.0f, kBackdropG = 150.0f / 255.0f, kBackdropB = 70.0f / 255.0f;
constexpr int kBackdropR8 = 60, kBackdropG8 = 150, kBackdropB8 = 70;

void log_gl_errors(const char* where) {
    int n = 0;
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::fprintf(stderr, "  [glGetError @ %s] 0x%04x\n", where, static_cast<unsigned>(err));
        ++n;
    }
    if (n == 0) std::printf("  [glGetError @ %s] limpo (GL_NO_ERROR)\n", where);
}

// Flip vertical in-place (glReadPixels devolve origem embaixo-esquerda; troca pra
// topo-esquerda, convencao PNG/get_element_box).
void flip_vertical_rgba(std::vector<unsigned char>& px, int w, int h) {
    const int row = w * 4;
    std::vector<unsigned char> tmp(static_cast<std::size_t>(row));
    for (int y = 0; y < h / 2; ++y) {
        unsigned char* top = px.data() + static_cast<std::size_t>(y) * row;
        unsigned char* bot = px.data() + static_cast<std::size_t>(h - 1 - y) * row;
        std::memcpy(tmp.data(), top, row);
        std::memcpy(top, bot, row);
        std::memcpy(bot, tmp.data(), row);
    }
}

struct PixelStats {
    double mean_luminance = 0.0;   // 0..255, media de (r+g+b)/3 no frame inteiro
    double pct_black = 0.0;        // % de pixels com r+g+b < 10 (praticamente preto)
    int box_r = 0, box_g = 0, box_b = 0;       // media da regiao do box da UI
    int backdrop_r = 0, backdrop_g = 0, backdrop_b = 0;  // media de um canto FORA do box
    bool box_matches_ui = false;         // box perto de (51,136,255)?
    bool backdrop_matches_game = false;  // canto perto de (60,150,70)?
};

PixelStats analyze(const std::vector<unsigned char>& px_top_left, int w, int h) {
    PixelStats s;
    long long lum_sum = 0;
    long long black_count = 0;
    const long long total = static_cast<long long>(w) * h;
    for (long long i = 0; i < total; ++i) {
        const unsigned char r = px_top_left[static_cast<std::size_t>(i) * 4 + 0];
        const unsigned char g = px_top_left[static_cast<std::size_t>(i) * 4 + 1];
        const unsigned char b = px_top_left[static_cast<std::size_t>(i) * 4 + 2];
        lum_sum += (static_cast<int>(r) + g + b);
        if (static_cast<int>(r) + g + b < 10) ++black_count;
    }
    s.mean_luminance = static_cast<double>(lum_sum) / (static_cast<double>(total) * 3.0);
    s.pct_black = 100.0 * static_cast<double>(black_count) / static_cast<double>(total);

    // Amostra o CENTRO do box da UI (deve ser a cor do background-color do RCSS).
    auto sample_region = [&](int cx, int cy, int half, int& out_r, int& out_g, int& out_b) {
        long long sr = 0, sg = 0, sb = 0, n = 0;
        for (int y = cy - half; y <= cy + half; ++y) {
            for (int x = cx - half; x <= cx + half; ++x) {
                if (x < 0 || y < 0 || x >= w || y >= h) continue;
                const std::size_t idx = (static_cast<std::size_t>(y) * w + x) * 4;
                sr += px_top_left[idx + 0];
                sg += px_top_left[idx + 1];
                sb += px_top_left[idx + 2];
                ++n;
            }
        }
        out_r = n ? static_cast<int>(sr / n) : -1;
        out_g = n ? static_cast<int>(sg / n) : -1;
        out_b = n ? static_cast<int>(sb / n) : -1;
    };
    sample_region(kBoxLeft + kBoxW / 2, kBoxTop + kBoxH / 2, 10, s.box_r, s.box_g, s.box_b);
    sample_region(20, 20, 10, s.backdrop_r, s.backdrop_g, s.backdrop_b);

    auto close = [](int a, int b) { return std::abs(a - b) <= 25; };
    s.box_matches_ui = close(s.box_r, kBoxR) && close(s.box_g, kBoxG) && close(s.box_b, kBoxB);
    s.backdrop_matches_game =
        close(s.backdrop_r, kBackdropR8) && close(s.backdrop_g, kBackdropG8) && close(s.backdrop_b, kBackdropB8);
    return s;
}

void save_png(const std::string& path, std::vector<unsigned char> px_top_left, int w, int h) {
    const int ok = stbi_write_png(path.c_str(), w, h, 4, px_top_left.data(), w * 4);
    std::printf("  stbi_write_png(%s) = %d\n", path.c_str(), ok);
}

// Cria janela+contexto GL com o NUMERO DE SAMPLES pedido (0 = desliga MSAA). Retorna
// false em falha (loga SDL_GetError). Mesmos atributos de contexto que a producao usa
// (maestro.cpp / battle_preview.cpp: core profile 3.3, doublebuffer, stencil=8).
bool create_window_and_context(const char* title, int samples, SDL_Window** out_window,
                                SDL_GLContext* out_ctx) {
    if (samples > 0) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, samples);
    } else {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window* window = SDL_CreateWindow(title, kW, kH, SDL_WINDOW_OPENGL);
    if (window == nullptr) {
        std::fprintf(stderr, "  SDL_CreateWindow(%s, samples=%d) falhou: %s\n", title, samples,
                     SDL_GetError());
        return false;
    }
    SDL_GLContext ctx = SDL_GL_CreateContext(window);
    if (ctx == nullptr) {
        std::fprintf(stderr, "  SDL_GL_CreateContext(%s, samples=%d) falhou: %s\n", title, samples,
                     SDL_GetError());
        SDL_DestroyWindow(window);
        return false;
    }
    SDL_GL_MakeCurrent(window, ctx);
    SDL_GL_SetSwapInterval(0);

    const auto glad_loader = reinterpret_cast<GLADloadfunc>(
        reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress));
    if (gladLoadGL(glad_loader) == 0) {
        std::fprintf(stderr, "  gladLoadGL falhou (samples=%d)\n", samples);
        SDL_GL_DestroyContext(ctx);
        SDL_DestroyWindow(window);
        return false;
    }

    GLint sample_buffers = -1, achieved_samples = -1;
    glGetIntegerv(GL_SAMPLE_BUFFERS, &sample_buffers);
    glGetIntegerv(GL_SAMPLES, &achieved_samples);
    std::printf("  [%s] pedido samples=%d -> GL_SAMPLE_BUFFERS=%d GL_SAMPLES=%d\n", title, samples,
                sample_buffers, achieved_samples);
    log_gl_errors("apos create_window_and_context/gladLoadGL");

    *out_window = window;
    *out_ctx = ctx;
    return true;
}

// Cria um FBO texture-backed NAO-multisample (samples=0) do tamanho da janela, pra
// resolver (blit) o FBO 0 multisample antes de ler pixels (glReadPixels de um FBO
// multisample e invalido pela spec GL). Retorna 0 em falha.
GLuint create_resolve_fbo(GLuint* out_tex) {
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kW, kH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::fprintf(stderr, "  create_resolve_fbo: glCheckFramebufferStatus=0x%04x (incompleto)\n",
                     static_cast<unsigned>(status));
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &tex);
        return 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    *out_tex = tex;
    return fbo;
}

std::string write_rml_stage(const std::string& dir) {
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    // Sem @font-face / sem texto (MESMO padrao de glintfx/tests/embed_scene.rml - "seguro
    // sob Mesa llvmpipe / Xvfb"): so 1 box opaco pra medir composite por cor, sem
    // dependencia de arquivo de fonte.
    const std::string rcss =
        "body { display: block; width: 100%; height: 100%; background-color: transparent; }\n"
        "#box { display: block; position: absolute; left: " + std::to_string(kBoxLeft) +
        "px; top: " + std::to_string(kBoxTop) + "px; width: " + std::to_string(kBoxW) +
        "px; height: " + std::to_string(kBoxH) + "px; background-color: #3388ffff; }\n";
    const std::string rml =
        "<rml><head><link type=\"text/rcss\" href=\"probe.rcss\"/>"
        "<title>msaa_glintfx_probe</title></head>"
        "<body><div id=\"box\"></div></body></rml>\n";
    {
        std::ofstream f(dir + "/probe.rcss");
        f << rcss;
    }
    const std::string rml_path = dir + "/probe.rml";
    {
        std::ofstream f(rml_path);
        f << rml;
    }
    return rml_path;
}

struct ScenarioResult {
    std::string name;
    bool ok = false;
    PixelStats stats;
};

// with_ui=false: so o "jogo" (clear verde), sem glintfx nenhum - CONTROLE isolando o MEU
// resolve-blit de leitura sob MSAA. with_ui=true: pipeline completo (clear + UiLayer).
ScenarioResult run_scenario(const std::string& name, int samples, bool with_ui,
                            const std::string& rml_path, const std::string& out_dir) {
    ScenarioResult result;
    result.name = name;
    std::printf("\n=== cenario: %s (samples=%d, with_ui=%s) ===\n", name.c_str(), samples,
                with_ui ? "true" : "false");

    SDL_Window* window = nullptr;
    SDL_GLContext ctx = nullptr;
    if (!create_window_and_context(name.c_str(), samples, &window, &ctx)) {
        return result;
    }

    // "jogo": limpa o backbuffer (FBO 0) com uma cor solida e opaca (representa o mundo
    // renderizado ANTES da UI, MESMO contrato de producao: host desenha a cena, depois
    // chama ui.render() compose-only, depois faz o swap).
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, kW, kH);
    glClearColor(kBackdropR, kBackdropG, kBackdropB, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    log_gl_errors("apos backdrop glClear");

    if (with_ui) {
        glintfx::UiLayer ui(glintfx::UiLayer::Config{/*logical_width=*/kW, /*logical_height=*/kH,
                                                      /*load_gl=*/true, /*dp_ratio=*/1.0f});
        if (!ui.ok()) {
            std::fprintf(stderr, "  glintfx::UiLayer::ok()=false (attach falhou)\n");
            SDL_GL_DestroyContext(ctx);
            SDL_DestroyWindow(window);
            return result;
        }
        const std::string dir_of_rml = rml_path.substr(0, rml_path.find_last_of('/'));
        ui.set_asset_base_url(dir_of_rml.c_str());
        const bool loaded = ui.load(rml_path.c_str());
        std::printf("  ui.load(%s) = %s\n", rml_path.c_str(), loaded ? "true" : "false");
        ui.set_viewport(kW, kH);

        // Igual save_load_screenshot_probe.cpp: alguns frames de assentamento (layout +
        // render-layer interno do RmlUi), redesenhando o backdrop a cada frame (o compose
        // do glintfx e nao-destrutivo do FBO 0 fora do que a UI cobre, mas o proprio host
        // teria que redesenhar a cena TODO frame de qualquer forma - mesmo contrato real).
        for (int frame = 0; frame < 5; ++frame) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, kW, kH);
            glClearColor(kBackdropR, kBackdropG, kBackdropB, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ui.update();
            ui.render();
        }
        log_gl_errors("apos ui.update()/ui.render() (5 frames)");
    }

    // --- leitura ---
    std::vector<unsigned char> px(static_cast<std::size_t>(kW) * kH * 4);
    if (samples == 0) {
        // FBO 0 single-sample: leitura DIRETA e legal pela spec GL.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glReadBuffer(GL_BACK);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, kW, kH, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
        log_gl_errors("apos glReadPixels direto (samples=0)");
    } else {
        // FBO 0 multisample: glReadPixels direto e INVALIDO pela spec (GL_SAMPLE_BUFFERS>0
        // no read framebuffer). Resolve via glBlitFramebuffer pra um FBO texture-backed
        // nao-multisample - EXATAMENTE o padrao que o glintfx documenta como bugado sob
        // Mesa/llvmpipe (glBlitFramebuffer MSAA->non-MSAA = textura preta silenciosa). Se
        // ISSO falhar, e um artefato do MEU proprio codigo de leitura, nao do glintfx (ver
        // cabecalho do arquivo).
        GLuint resolve_tex = 0;
        const GLuint resolve_fbo = create_resolve_fbo(&resolve_tex);
        if (resolve_fbo == 0) {
            std::fprintf(stderr, "  create_resolve_fbo falhou\n");
            SDL_GL_DestroyContext(ctx);
            SDL_DestroyWindow(window);
            return result;
        }
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolve_fbo);
        glBlitFramebuffer(0, 0, kW, kH, 0, 0, kW, kH, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        log_gl_errors("apos glBlitFramebuffer (resolve MSAA->non-MSAA, MEU codigo)");

        glBindFramebuffer(GL_READ_FRAMEBUFFER, resolve_fbo);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, kW, kH, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
        log_gl_errors("apos glReadPixels do resolve FBO");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &resolve_fbo);
        glDeleteTextures(1, &resolve_tex);
    }

    flip_vertical_rgba(px, kW, kH);
    result.stats = analyze(px, kW, kH);
    save_png(out_dir + "/msaa_probe_" + name + ".png", px, kW, kH);

    std::printf(
        "  stats: mean_luminance=%.1f pct_black=%.1f%% box=(%d,%d,%d)[match=%s] "
        "backdrop=(%d,%d,%d)[match=%s]\n",
        result.stats.mean_luminance, result.stats.pct_black, result.stats.box_r, result.stats.box_g,
        result.stats.box_b, result.stats.box_matches_ui ? "OK" : "FALHOU", result.stats.backdrop_r,
        result.stats.backdrop_g, result.stats.backdrop_b,
        result.stats.backdrop_matches_game ? "OK" : "FALHOU");

    SDL_GL_DestroyContext(ctx);
    SDL_DestroyWindow(window);
    result.ok = true;
    return result;
}

}  // namespace

int main(int argc, char** argv) {
    const std::string out_dir = (argc > 1) ? argv[1] : "/tmp";

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::fprintf(stderr, "SDL_Init falhou: %s\n", SDL_GetError());
        return 1;
    }

    const std::string stage_dir = out_dir + "/msaa_probe_stage";
    const std::string rml_path = write_rml_stage(stage_dir);

    std::vector<ScenarioResult> results;
    results.push_back(run_scenario("S0_baseline_msaa0_with_ui", /*samples=*/0, /*with_ui=*/true,
                                    rml_path, out_dir));
    results.push_back(run_scenario("S1_msaa4_backdrop_only", /*samples=*/4, /*with_ui=*/false,
                                    rml_path, out_dir));
    results.push_back(run_scenario("S2_msaa4_with_ui", /*samples=*/4, /*with_ui=*/true, rml_path,
                                    out_dir));

    SDL_Quit();

    std::printf("\n=== RESUMO ===\n");
    int failures = 0;
    for (const auto& r : results) {
        std::printf("%-28s ok=%s box_match=%s backdrop_match=%s mean_lum=%.1f\n", r.name.c_str(),
                    r.ok ? "sim" : "NAO", r.ok && r.stats.box_matches_ui ? "sim" : "NAO",
                    r.ok && r.stats.backdrop_matches_game ? "sim" : "NAO",
                    r.ok ? r.stats.mean_luminance : -1.0);
        if (!r.ok || !r.stats.box_matches_ui || !r.stats.backdrop_matches_game) ++failures;
    }
    return failures;
}
