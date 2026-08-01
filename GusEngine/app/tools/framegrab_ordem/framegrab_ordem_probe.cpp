// SPDX-License-Identifier: Apache-2.0
// FRAMEGRAB-ORDEM - EXPERIMENTO (nao producao, nao migracao). Prova/refuta POR PIXEL a
// hipotese que o glintfx declara no doc-comment de UiLayer::capture_frame()
// (include/glintfx/ui_layer.hpp, bloco "TIMING - A UNICA REGRA QUE IMPORTA"): que a
// chamada e LEITURA PURA do FBO 0 na regiao do viewport, sem tocar RmlUi, e que portanto
// chama-la ANTES de ui.render() devolve o quadro com SO o que o host desenhou - o que o
// "fundo congelado" atras do menu de pausa precisa.
//
// O QUE ESTE PROBE NAO E: nao altera nada de producao (SdlWindow::capture_frame_to_png
// segue intocado); nao e um workaround de lacuna; nao decide a migracao. Ele so MEDE.
//
// FRONTEIRA DO QUE ELE PROVA (leia antes de citar este probe como evidencia): ele prova a
// MECANICA do capture_frame() - que le o FBO 0 e que a ORDEM em relacao ao render() decide
// se a UI entra. Ele NAO prova a integracao com o pipeline real do jogo: a "cena" aqui e
// glClear de cor solida, sem Render2dGl3, sem OverworldSim::render, sem sprite/textura, sem
// blend, sem camera, sem o begin_frame/end_frame da casa. "O capture_frame funciona" e "o
// fundo congelado esta resolvido" sao afirmacoes DIFERENTES. O que a fatia seguinte precisa
// medir (proposta): no frame REAL de producao, chamar ui.capture_frame() e o
// gl3_read_backbuffer_rgba() que ja usamos lado a lado e exigir buffers BYTE-IDENTICOS - os
// dois leem o mesmo GL_BACK no mesmo instante, entao o criterio nao depende de conhecer a
// cor de nenhum sprite.
//
// POR QUE GL CRU EM VEZ DO NOSSO Draw2d/Render2dGl3: pintar com glClear e MAIS controlado
// pra esta pergunta - o valor do pixel e escolhido e a comparacao fica sem ambiguidade de
// blend/filtragem/premultiply, o que permite oraculo de contagem EXATA (sem tolerancia). O
// preco e exatamente a fronteira do paragrafo acima. A lei do framework unico nao e
// violada: ela vale pra PRODUCAO, e app/tools/ esta fora dela por construcao (e o que o
// escopo do GATE(sdl-ratchet) reconhece ao excluir app/tools/). O RmlUi_Include_GL3.h abaixo
// e NOSSO (GusEngine/platform/rmlui/), o mesmo glad que platform/rmlui/gl3_loader.cpp usa em
// producao - nao e vendor do glintfx.
//
// DESENHO (a UI tem de ser visualmente inconfundivel, senao o teste nao distingue nada):
//   - o "jogo" (nos) e pintado com GL cru: glClear + glScissor. So valores de canal 0.0 e
//     1.0, para que a cor no framebuffer seja EXATA (nada de arredondamento de float, e
//     imune a conversao sRGB, que e identidade em 0 e 1) - isso da um oraculo de pixel
//     sem tolerancia:
//         fundo inteiro   AZUL     (0,   0, 255)
//         bloco AMARELO   (255,255,  0) em x[40,200)  y[40,120)   <- FORA do painel
//         bloco VERDE     (0,  255,  0) em x[300,660) y[180,360)  <- SOB o painel
//   - a UI e um unico div opaco MAGENTA (#ff00ff) em x[280,680) y[160,380), que cobre o
//     bloco verde inteiro com 20px de folga em cada lado.
//   Assim cada uma das 4 perguntas tem regiao propria:
//     (1) "o antes NAO contem a UI"     -> zero pixel magenta na imagem inteira do antes
//     (2) "o depois CONTEM a UI"        -> painel virou magenta no depois
//     (3) "o antes NAO esta vazio/preto"-> o bloco verde SOB o painel esta 100% verde no
//                                          antes (conteudo de JOGO exatamente onde a UI
//                                          estaria), + amarelo/azul nas outras regioes
//     (4) "nao e imagem trocada/velha"  -> fora do retangulo do painel, antes e depois sao
//                                          pixel-a-pixel IDENTICOS (0 diferenca)
//   Bonus de orientacao: o bloco amarelo e assimetrico na vertical (linhas 40..120). Se a
//   imagem estivesse invertida, ele apareceria em 420..500 - a contagem na regiao espelhada
//   e reportada e tem de ser 0.
//
// FASE B (observacao, nao assercao): 1 quadro que NAO limpa e NAO desenha nada, so captura
// antes do render. Documenta o que o BACK buffer guarda de residuo (consequencia de double
// buffering, nao de capture_frame) - relevante pra migracao, porque um host que nao cobre
// o quadro inteiro leria residuo.
//
// SAIDA: 3 PNGs (pra olho) + 3 .rgba crus (pra o verificador independente em Python -
// check_pixels.py). As contagens que este .cpp imprime sao um cross-check; o veredito sai
// do Python lendo os artefatos, nao do programa que os produziu.
//
// ISOLAMENTO GRAFICO: nunca rodar direto. Usar run_framegrab.sh (Xvfb proprio +
// XDG_RUNTIME_DIR proprio chmod 700 + XDG_SESSION_TYPE=x11 + WAYLAND_DISPLAY removido +
// prova de isolamento por lsof/ss ANTES de qualquer coisa).

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

// UMA TU dona da implementacao do glad (header-only, GL 3.3 core) - MESMA receita de
// app/tools/msaa_glintfx_probe.cpp. O glintfx tem loader interno proprio (glx_*), zero
// conflito de simbolo.
#define GLAD_GL_IMPLEMENTATION
#include "RmlUi_Include_GL3.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <glintfx/font_face.hpp>
#include <glintfx/ui_layer.hpp>

namespace fs = std::filesystem;

namespace {

constexpr int kW = 960;
constexpr int kH = 540;

// --- geometria do padrao de teste (coordenadas de TELA: origem no topo-esquerda) ------
struct RectPx {
    int x, y, w, h;  // [x, x+w) x [y, y+h)
};

constexpr RectPx kYellow{40, 40, 160, 80};     // fora do painel
constexpr RectPx kGreen{300, 180, 360, 180};   // sob o painel
constexpr RectPx kPanel{280, 160, 400, 220};   // o div da UI (cobre kGreen com folga)

struct Rgb {
    unsigned char r, g, b;
};

constexpr Rgb kBg{0, 0, 255};        // azul
constexpr Rgb kYellowRgb{255, 255, 0};
constexpr Rgb kGreenRgb{0, 255, 0};
constexpr Rgb kMagentaRgb{255, 0, 255};

// glScissor usa origem bottom-left; converte um RectPx de tela pra ela.
void scissor_screen_rect(const RectPx& r) {
    glScissor(r.x, kH - (r.y + r.h), r.w, r.h);
}

// Pinta o "jogo": fundo + 2 blocos. Sem shader nenhum - glClear com scissor.
void paint_game_frame() {
    glDisable(GL_SCISSOR_TEST);
    glClearColor(static_cast<float>(kBg.r) / 255.0f, static_cast<float>(kBg.g) / 255.0f,
                 static_cast<float>(kBg.b) / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_SCISSOR_TEST);
    scissor_screen_rect(kYellow);
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    scissor_screen_rect(kGreen);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
}

// --- contagem (cross-check interno; o veredito vem do Python) --------------------------
std::size_t count_exact(const unsigned char* px, const RectPx& r, Rgb c) {
    std::size_t n = 0;
    for (int y = r.y; y < r.y + r.h; ++y) {
        for (int x = r.x; x < r.x + r.w; ++x) {
            const unsigned char* p = px + (static_cast<std::size_t>(y) * kW + x) * 4;
            if (p[0] == c.r && p[1] == c.g && p[2] == c.b) ++n;
        }
    }
    return n;
}

std::size_t count_exact_full(const unsigned char* px, Rgb c) {
    return count_exact(px, RectPx{0, 0, kW, kH}, c);
}

// Diferenca pixel-a-pixel entre duas capturas, restrita a DENTRO ou FORA do retangulo do
// painel (RGB; alpha e sintetico 255 nos dois).
std::size_t diff_count(const unsigned char* a, const unsigned char* b, bool inside_panel) {
    std::size_t n = 0;
    for (int y = 0; y < kH; ++y) {
        for (int x = 0; x < kW; ++x) {
            const bool in = (x >= kPanel.x && x < kPanel.x + kPanel.w && y >= kPanel.y &&
                             y < kPanel.y + kPanel.h);
            if (in != inside_panel) continue;
            const std::size_t off = (static_cast<std::size_t>(y) * kW + x) * 4;
            if (a[off] != b[off] || a[off + 1] != b[off + 1] || a[off + 2] != b[off + 2]) ++n;
        }
    }
    return n;
}

bool dump(const std::string& dir, const std::string& stem, const unsigned char* px) {
    const std::string png = dir + "/" + stem + ".png";
    const std::string raw = dir + "/" + stem + ".rgba";
    const int ok = stbi_write_png(png.c_str(), kW, kH, 4, px, kW * 4);
    std::ofstream f(raw, std::ios::binary);
    f.write(reinterpret_cast<const char*>(px), static_cast<std::streamsize>(kW) * kH * 4);
    const bool raw_ok = f.good();
    f.close();
    std::printf("  dump %-8s png=%d raw=%d (%s / %s)\n", stem.c_str(), ok, raw_ok ? 1 : 0,
                png.c_str(), raw.c_str());
    return ok != 0 && raw_ok;
}

void report(const char* tag, const unsigned char* px) {
    std::printf("[%s] magenta(imagem inteira)      = %zu\n", tag, count_exact_full(px, kMagentaRgb));
    std::printf("[%s] verde  (regiao SOB o painel) = %zu de %d\n", tag,
                count_exact(px, kGreen, kGreenRgb), kGreen.w * kGreen.h);
    std::printf("[%s] magenta(regiao SOB o painel) = %zu de %d\n", tag,
                count_exact(px, kGreen, kMagentaRgb), kGreen.w * kGreen.h);
    std::printf("[%s] amarelo(bloco amarelo)       = %zu de %d\n", tag,
                count_exact(px, kYellow, kYellowRgb), kYellow.w * kYellow.h);
    // Oraculo de ORIENTACAO: o bloco amarelo espelhado na vertical.
    const RectPx mirrored{kYellow.x, kH - (kYellow.y + kYellow.h), kYellow.w, kYellow.h};
    std::printf("[%s] amarelo(regiao ESPELHADA)    = %zu (precisa ser 0)\n", tag,
                count_exact(px, mirrored, kYellowRgb));
    std::printf("[%s] preto  (imagem inteira)      = %zu (precisa ser 0)\n", tag,
                count_exact_full(px, Rgb{0, 0, 0}));
}

}  // namespace

int main() {
    const char* out_env = std::getenv("GUSWORLD_FRAMEGRAB_OUT");
    const std::string out_dir =
        (out_env != nullptr && out_env[0] != '\0')
            ? std::string(out_env)
            : std::string("/var/tmp/builds/claude-1000/framegrab-ordem/out");
    std::error_code ec;
    fs::create_directories(out_dir, ec);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::fprintf(stderr, "SDL_Init falhou: %s\n", SDL_GetError());
        return 1;
    }

    // MESMA receita de contexto dos probes de producao (frozen_bg_probe.cpp): janela unica
    // com SDL_WINDOW_OPENGL desde o inicio, GL 3.3 core, double buffer, stencil 8 (RmlUi
    // usa stencil), swap interval 0.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window* window = SDL_CreateWindow("framegrab_ordem_probe", kW, kH, SDL_WINDOW_OPENGL);
    if (window == nullptr) {
        std::fprintf(stderr, "SDL_CreateWindow falhou: %s\n", SDL_GetError());
        return 1;
    }
    SDL_GLContext gl = SDL_GL_CreateContext(window);
    if (gl == nullptr) {
        std::fprintf(stderr, "SDL_GL_CreateContext falhou: %s\n", SDL_GetError());
        return 1;
    }
    SDL_GL_MakeCurrent(window, gl);
    SDL_GL_SetSwapInterval(0);
    if (gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress)) == 0) {
        std::fprintf(stderr, "gladLoadGL falhou\n");
        return 1;
    }
    int db = -1;
    SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &db);
    std::printf("GL_VERSION=%s DOUBLEBUFFER=%d\n",
                reinterpret_cast<const char*>(glGetString(GL_VERSION)), db);

    // --- o documento RML: UM div opaco magenta, sem texto -----------------------------
    // Sem texto de proposito: glifo antialiased poria pixel nao-magenta dentro do painel e
    // sujaria o oraculo. A pergunta aqui e ordem de captura, nao tipografia.
    const fs::path stage = fs::path(out_dir) / "stage";
    fs::create_directories(stage, ec);
    const fs::path rml_path = stage / "framegrab_panel.rml";
    {
        std::ofstream f(rml_path);
        f << "<rml>\n<head>\n<style>\n"
          << "body { width: " << kW << "px; height: " << kH
          << "px; background-color: transparent; }\n"
          << "#panel { position: absolute; left: " << kPanel.x << "px; top: " << kPanel.y
          << "px; width: " << kPanel.w << "px; height: " << kPanel.h
          << "px; background-color: #ff00ff; }\n"
          << "</style>\n</head>\n<body>\n<div id=\"panel\"></div>\n</body>\n</rml>\n";
    }

    glintfx::UiLayer ui(glintfx::UiLayer::Config{/*logical_width=*/kW, /*logical_height=*/kH,
                                                 /*load_gl=*/true, /*dp_ratio=*/1.0f});
    if (!ui.ok()) {
        std::fprintf(stderr, "glintfx::UiLayer::ok()=false\n");
        return 1;
    }

    // Fonte: opcional (o painel nao tem texto). Carregada so pra o documento nao ficar
    // sem nenhuma face - se falhar, nao muda o oraculo.
    const char* fonts_env = std::getenv("GUSWORLD_FONTS");
    if (fonts_env != nullptr && fonts_env[0] != '\0') {
        const std::string ttf = std::string(fonts_env) + "/PixelOperatorMono.ttf";
        if (fs::exists(ttf)) {
            glintfx::FontFaceDesc desc;
            desc.path = ttf.c_str();
            desc.family = "Pixel Operator Mono";
            std::printf("load_font_face(%s) = %d\n", ttf.c_str(), ui.load_font_face(desc) ? 1 : 0);
        }
    }

    ui.set_asset_base_url(stage.string().c_str());
    // PRE-REQUISITO do brief: set_viewport() e o que define a regiao que capture_frame le
    // (ui_layer.cpp:544-547 passa impl_->gl_offset_x/y + impl_->w/h). Sem isto o
    // experimento mediria o caso degenerado.
    ui.set_viewport(kW, kH);
    ui.set_dp_ratio(1.0f);
    if (!ui.load(rml_path.string().c_str())) {
        std::fprintf(stderr, "ui.load falhou: %s\n", rml_path.string().c_str());
        return 1;
    }

    // --- warmup: quadros completos, pra o RmlUi assentar layout/texturas ---------------
    constexpr int kWarmup = 12;
    for (int i = 0; i < kWarmup; ++i) {
        paint_game_frame();
        ui.update();
        ui.render();
        SDL_GL_SwapWindow(window);
    }

    int rc = 0;
    std::vector<unsigned char> before_copy;

    // ================================ FASE A ==========================================
    // O quadro medido. Pinta o jogo, captura ANTES do render, renderiza a UI, captura
    // DEPOIS - tudo no MESMO quadro, ANTES do swap (o engine.cpp le GL_BACK; capturar
    // depois do swap leria conteudo indefinido, e o proprio doc-comment deles avisa).
    {
        std::printf("\n=== FASE A: duas capturas no MESMO quadro ===\n");
        paint_game_frame();

        const glintfx::UiLayer::CapturedFrame before = ui.capture_frame();
        std::printf("antes : ok=%d %dx%d bytes=%zu\n", before.ok ? 1 : 0, before.width,
                    before.height, before.byte_count);

        // Captura do MEIO: entre update() e render(). Sem ela, o experimento provaria
        // "antes de update()+render()" e eu estaria atribuindo a render() um efeito que
        // poderia ser de update() - a hipotese e sobre render(), entao o par tem de ser
        // desmembrado.
        ui.update();
        const glintfx::UiLayer::CapturedFrame mid = ui.capture_frame();
        std::printf("meio  : ok=%d %dx%d bytes=%zu\n", mid.ok ? 1 : 0, mid.width, mid.height,
                    mid.byte_count);

        ui.render();

        const glintfx::UiLayer::CapturedFrame after = ui.capture_frame();
        std::printf("depois: ok=%d %dx%d bytes=%zu\n", after.ok ? 1 : 0, after.width,
                    after.height, after.byte_count);

        const std::size_t expect_bytes = static_cast<std::size_t>(kW) * kH * 4;
        if (!before.ok || !after.ok || before.byte_count != expect_bytes ||
            after.byte_count != expect_bytes || before.width != kW || before.height != kH) {
            std::fprintf(stderr, "captura invalida (ok/dimensao/bytes) - abortando\n");
            SDL_GL_SwapWindow(window);
            return 1;
        }

        if (!dump(out_dir, "before", before.pixels.get())) rc = 1;
        if (mid.ok) {
            if (!dump(out_dir, "mid_after_update", mid.pixels.get())) rc = 1;
        }
        if (!dump(out_dir, "after", after.pixels.get())) rc = 1;

        report("antes ", before.pixels.get());
        if (mid.ok) report("meio  ", mid.pixels.get());
        report("depois", after.pixels.get());
        std::printf("diff FORA do painel  = %zu (precisa ser 0)\n",
                    diff_count(before.pixels.get(), after.pixels.get(), /*inside_panel=*/false));
        std::printf("diff DENTRO do painel= %zu de %d\n",
                    diff_count(before.pixels.get(), after.pixels.get(), /*inside_panel=*/true),
                    kPanel.w * kPanel.h);

        before_copy.assign(before.pixels.get(), before.pixels.get() + expect_bytes);
        SDL_GL_SwapWindow(window);
    }

    // ================================ FASE B ==========================================
    // OBSERVACAO, nao assercao: um quadro que NAO limpa e NAO desenha nada. O que aparecer
    // aqui e residuo do BACK buffer (double buffering), nao efeito de capture_frame. Serve
    // pra documentar o cuidado que a migracao precisa: quem nao cobre o quadro inteiro
    // antes de capturar pode ler UI velha.
    {
        std::printf("\n=== FASE B: quadro SEM clear e SEM desenho (observacao) ===\n");
        const glintfx::UiLayer::CapturedFrame stale = ui.capture_frame();
        if (stale.ok) {
            if (!dump(out_dir, "stale_noclear", stale.pixels.get())) rc = 1;
            report("stale ", stale.pixels.get());
        } else {
            std::fprintf(stderr, "FASE B: captura ok=false\n");
            rc = 1;
        }
        SDL_GL_SwapWindow(window);
    }

    std::printf("\nGL erro final = 0x%x (0 = GL_NO_ERROR)\n", glGetError());

    SDL_GL_DestroyContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return rc;
}
