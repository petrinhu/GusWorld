// SPDX-License-Identifier: Apache-2.0
// FRAMEGRAB-7-SITIOS - EXPERIMENTO DE VERIFICACAO (nao producao, nao migracao nova). Mede
// diretamente a pergunta que framegrab_ordem_probe.cpp deixou em aberto no proprio
// comentario (linhas 18-21 daquele arquivo): no MESMO instante (depois de ui.render(),
// antes do swap), gus::platform::rmlui::gl3_read_backbuffer_rgba() (a funcao que os 7
// sitios desta fatia deixaram de chamar) e glintfx::UiLayer::capture_frame() (a funcao
// que passaram a chamar) leem o MESMO GL_BACK? Exigimos os 3 canais RGB BYTE-IDENTICOS
// (o alpha diverge por DESIGN documentado no proprio ui_layer.hpp - capture_frame()
// sintetiza 255 sempre; gl3_read_backbuffer_rgba le o alpha REAL do framebuffer - por
// isso o alpha fica de fora do oraculo, de proposito, nao por displicencia).
//
// O QUE ESTE PROBE NAO E: nao altera nada de producao; nao decide migracao nenhuma (ja
// decidida e feita nos 7 sitios); so MEDE, com um oraculo de pixel sem ambiguidade.
//
// DIFERENCA em relacao a framegrab_ordem_probe.cpp: aquele prova a MECANICA de
// capture_frame() (antes/depois de render(), UI entra ou nao) usando SEU PROPRIO glad
// (GLAD_GL_IMPLEMENTATION na propria TU, header-only, isolado). Este aqui em vez disso
// LINKA gusengine_platform.a de verdade (o gl3_loader.cpp que os 7 sitios usavam) e chama
// gl3_load_functions()/gl3_read_backbuffer_rgba() PRODUCAO - nao uma reimplementacao. Por
// isso NAO define GLAD_GL_IMPLEMENTATION aqui: a definicao (a UNICA TU dona, ADR-010 F3)
// ja mora dentro de gl3_loader.cpp (linkado do .a) - RmlUi_Include_GL3.h incluido aqui SEM
// a macro so DECLARA os ponteiros (extern), resolvidos pelo linker contra o .o do .a.
//
// DESENHO DA CENA (MESMO padrao de framegrab_ordem_probe.cpp, pra imagem visualmente
// inconfundivel e oraculo de orientacao gratis):
//   fundo AZUL, bloco AMARELO fora do painel, bloco VERDE sob o painel, painel RmlUi
//   MAGENTA opaco. Cores em 0/255 puros (imunes a arredondamento/sRGB).
//
// ISOLAMENTO GRAFICO: nunca rodar direto. Usar run_framegrab_byte_identical.sh (MESMA
// receita, copiada de framegrab_ordem/run_framegrab.sh).

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

// SEM GLAD_GL_IMPLEMENTATION aqui de proposito (ver comentario acima) - so declaracoes
// extern; a definicao vem do .o de gl3_loader.cpp dentro de libgusengine_platform.a.
#include "RmlUi_Include_GL3.h"

#include "gus/platform/rmlui/gl3_loader.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <glintfx/font_face.hpp>
#include <glintfx/ui_layer.hpp>

namespace fs = std::filesystem;

namespace {

constexpr int kW = 960;
constexpr int kH = 540;

struct RectPx {
    int x, y, w, h;
};

// MESMA geometria de framegrab_ordem_probe.cpp (reuso deliberado do oraculo ja provado).
constexpr RectPx kYellow{40, 40, 160, 80};
constexpr RectPx kGreen{300, 180, 360, 180};
constexpr RectPx kPanel{280, 160, 400, 220};

struct Rgb {
    unsigned char r, g, b;
};

constexpr Rgb kBg{0, 0, 255};
constexpr Rgb kYellowRgb{255, 255, 0};
constexpr Rgb kGreenRgb{0, 255, 0};
constexpr Rgb kMagentaRgb{255, 0, 255};

void scissor_screen_rect(const RectPx& r) {
    glScissor(r.x, kH - (r.y + r.h), r.w, r.h);
}

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

// Diferenca pixel-a-pixel SO em RGB (alpha excluido de proposito - ver o comentario de
// topo do arquivo: capture_frame() sintetiza 255, gl3_read_backbuffer_rgba le o real).
std::size_t rgb_diff_count(const unsigned char* a, const unsigned char* b) {
    std::size_t n = 0;
    for (int y = 0; y < kH; ++y) {
        for (int x = 0; x < kW; ++x) {
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
    std::printf("  dump %-12s png=%d raw=%d (%s / %s)\n", stem.c_str(), ok, raw_ok ? 1 : 0,
                png.c_str(), raw.c_str());
    return ok != 0 && raw_ok;
}

// Orientacao: bloco amarelo assimetrico na vertical (y=[40,120), kH=540 -> espelhado
// seria y=[420,500)). ANTIDOTO (achado do glintfx, retransmitido pelo lider apos este
// probe ja estar escrito): um teste que so compara old==new pixel a pixel PASSA MESMO
// SE OS DOIS LADOS ESTIVEREM IGUALMENTE INVERTIDOS - a comparacao relativa nao prova
// orientacao ABSOLUTA. Por isso esta checagem e feita SEPARADAMENTE, na regiao FIXA
// esperada (kYellow) e na regiao ESPELHADA (mirror_of(kYellow)), pra cada buffer
// INDEPENDENTEMENTE - nao contra o outro buffer. Amostra de CENTRO/area uniforme nao
// serviria (uma cena simetrica passa nos dois sentidos); a assimetria e o que importa.
RectPx mirror_of(const RectPx& r) {
    return RectPx{r.x, kH - (r.y + r.h), r.w, r.h};
}

void report(const char* tag, const unsigned char* px) {
    std::printf("[%s] magenta(imagem inteira)      = %zu\n", tag,
                count_exact_full(px, kMagentaRgb));
    std::printf("[%s] verde  (regiao SOB o painel) = %zu de %d\n", tag,
                count_exact(px, kGreen, kGreenRgb), kGreen.w * kGreen.h);
    std::printf("[%s] magenta(regiao SOB o painel) = %zu de %d\n", tag,
                count_exact(px, kGreen, kMagentaRgb), kGreen.w * kGreen.h);
    std::printf("[%s] amarelo(bloco amarelo)       = %zu de %d\n", tag,
                count_exact(px, kYellow, kYellowRgb), kYellow.w * kYellow.h);
    const RectPx mirrored = mirror_of(kYellow);
    std::printf("[%s] amarelo(regiao ESPELHADA)    = %zu (precisa ser 0 - prova orientacao "
                "ABSOLUTA, nao so relativa old-vs-new)\n",
                tag, count_exact(px, mirrored, kYellowRgb));
}

}  // namespace

int main() {
    const char* out_env = std::getenv("GUSWORLD_FRAMEGRAB_BI_OUT");
    const std::string out_dir =
        (out_env != nullptr && out_env[0] != '\0')
            ? std::string(out_env)
            : std::string("/var/tmp/builds/claude-1000/framegrab-byte-identical/out");
    std::error_code ec;
    fs::create_directories(out_dir, ec);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::fprintf(stderr, "SDL_Init falhou: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window* window =
        SDL_CreateWindow("framegrab_byte_identical_probe", kW, kH, SDL_WINDOW_OPENGL);
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

    // PRODUCAO de verdade: MESMA chamada que battle_preview.cpp/sdl_window.cpp fazem.
    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        std::fprintf(stderr, "gl3_load_functions falhou\n");
        return 1;
    }
    std::printf("GL_VERSION=%s\n", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

    const fs::path stage = fs::path(out_dir) / "stage";
    fs::create_directories(stage, ec);
    const fs::path rml_path = stage / "panel.rml";
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
    ui.set_asset_base_url(stage.string().c_str());
    ui.set_viewport(kW, kH);
    ui.set_dp_ratio(1.0f);
    if (!ui.load(rml_path.string().c_str())) {
        std::fprintf(stderr, "ui.load falhou: %s\n", rml_path.string().c_str());
        return 1;
    }

    constexpr int kWarmup = 12;
    for (int i = 0; i < kWarmup; ++i) {
        paint_game_frame();
        ui.update();
        ui.render();
        SDL_GL_SwapWindow(window);
    }

    int rc = 0;

    // ================= O QUADRO MEDIDO: as 2 leituras no MESMO instante =================
    paint_game_frame();
    ui.update();
    ui.render();  // MESMA posicao dos 7 sitios migrados: depois do render(), antes do swap.

    const std::size_t expect_bytes = static_cast<std::size_t>(kW) * kH * 4;
    std::vector<unsigned char> old_buf(expect_bytes);
    const bool old_ok = gus::platform::rmlui::gl3_read_backbuffer_rgba(kW, kH, old_buf.data());
    const glintfx::UiLayer::CapturedFrame captured = ui.capture_frame();

    std::printf("old (gl3_read_backbuffer_rgba): ok=%d\n", old_ok ? 1 : 0);
    std::printf("new (capture_frame)           : ok=%d %dx%d bytes=%zu\n", captured.ok ? 1 : 0,
                captured.width, captured.height, captured.byte_count);

    if (!old_ok || !captured.ok || captured.byte_count != expect_bytes ||
        captured.width != kW || captured.height != kH) {
        std::fprintf(stderr, "captura invalida (ok/dimensao/bytes) - abortando\n");
        SDL_GL_SwapWindow(window);
        return 1;
    }

    if (!dump(out_dir, "old_gl3_readback", old_buf.data())) rc = 1;
    if (!dump(out_dir, "new_capture_frame", captured.pixels.get())) rc = 1;

    report("old", old_buf.data());
    report("new", captured.pixels.get());

    const std::size_t rgb_diff = rgb_diff_count(old_buf.data(), captured.pixels.get());
    std::printf("\nDIFF RGB (old vs new, alpha excluido de proposito) = %zu de %d pixels "
                "(precisa ser 0)\n",
                rgb_diff, kW * kH);

    // Alpha: reportado a parte (esperado DIVERGIR - documentado, nao e falha).
    std::size_t alpha_old_255 = 0, alpha_new_255 = 0;
    for (int i = 0; i < kW * kH; ++i) {
        if (old_buf[static_cast<std::size_t>(i) * 4 + 3] == 255) ++alpha_old_255;
        if (captured.pixels[static_cast<std::size_t>(i) * 4 + 3] == 255) ++alpha_new_255;
    }
    std::printf("alpha==255: old=%zu/%d new=%zu/%d (new precisa ser TODOS 255 - sintetico "
                "por design; old pode divergir, e o alpha REAL do framebuffer)\n",
                alpha_old_255, kW * kH, alpha_new_255, kW * kH);

    if (rgb_diff != 0) {
        std::fprintf(stderr, "FALHA: %zu pixel(s) divergem em RGB entre old e new\n", rgb_diff);
        rc = 1;
    } else {
        std::printf("OK: RGB byte-identico entre gl3_read_backbuffer_rgba e capture_frame() "
                    "no mesmo instante.\n");
    }

    SDL_GL_SwapWindow(window);
    std::printf("\nGL erro final = 0x%x (0 = GL_NO_ERROR)\n", glGetError());

    SDL_GL_DestroyContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return rc;
}
