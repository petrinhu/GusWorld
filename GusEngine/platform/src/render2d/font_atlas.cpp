// gus/platform/src/render2d/font_atlas.cpp
//
// Implementacao do FONT ATLAS (ver header). Le o .ttf (I/O, fronteira platform/) e
// rasteriza o ASCII printable via stb_truetype num bitmap grayscale em grade de celulas.
// Sem SDL: e bake na CPU (testavel headless). O Render2dSdl sobe os pixels pra textura.
//
// stb_truetype: ESTA TU define a implementacao (STB_TRUETYPE_IMPLEMENTATION). Outras TU
// que usem o header so incluem a interface. Convive com stb_image (definido no
// render2d_sdl.cpp, TU separada): macros de implementacao distintas, sem simbolo dup.

#include "gus/platform/render2d/font_atlas.hpp"

#include <cstdio>      // std::fopen/fread/fclose
#include <cstdlib>     // std::getenv
#include <cstring>     // std::memset
#include <filesystem>  // std::filesystem::exists (escolhe o candidato que existe)
#include <vector>

#include "gus/core/asset_paths.hpp"  // caminhos de asset centralizados (kFontsDir)

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

// Pasta dos .ttf embarcados, embutida pelo CMake (= GusEngine/assets/fonts).
#ifndef GUSWORLD_FONTS_DIR
#define GUSWORLD_FONTS_DIR ""
#endif

namespace gus::platform::render2d {

namespace {

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) {
        return b;
    }
    if (a.back() == '/') {
        return a + b;
    }
    return a + "/" + b;
}

// Le um arquivo binario inteiro pra um vetor de bytes. Vazio se nao abrir.
std::vector<unsigned char> read_file(const std::string& path) {
    std::vector<unsigned char> bytes;
    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (f == nullptr) {
        return bytes;
    }
    std::fseek(f, 0, SEEK_END);
    const long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (size > 0) {
        bytes.resize(static_cast<std::size_t>(size));
        const std::size_t got =
            std::fread(bytes.data(), 1, static_cast<std::size_t>(size), f);
        if (got != static_cast<std::size_t>(size)) {
            bytes.clear();  // leitura incompleta: trata como falha
        }
    }
    std::fclose(f);
    return bytes;
}

}  // namespace

int glyph_slot(int codepoint) noexcept {
    // ASCII printable -> slots 0..94; Latin-1 (160..255) -> slots 95..190. Fora = -1.
    if (codepoint >= kFontAsciiFirst && codepoint <= kFontAsciiLast) {
        return codepoint - kFontAsciiFirst;
    }
    if (codepoint >= kFontLatin1First && codepoint <= kFontLatin1Last) {
        return kFontAsciiCount + (codepoint - kFontLatin1First);
    }
    return -1;
}

bool FontAtlas::has_glyph(int codepoint) const noexcept {
    return valid() && glyph_slot(codepoint) >= 0;
}

UvRect FontAtlas::glyph_uv(int codepoint) const noexcept {
    const int slot = valid() ? glyph_slot(codepoint) : -1;
    if (slot < 0) {
        return UvRect{0.0f, 0.0f, 0.0f, 0.0f};  // vazio
    }
    const int cx = slot % cols;
    const int cy = slot / cols;
    const float u = static_cast<float>(cx * cell_px) / static_cast<float>(atlas_w);
    const float v = static_cast<float>(cy * cell_px) / static_cast<float>(atlas_h);
    const float w = static_cast<float>(cell_px) / static_cast<float>(atlas_w);
    const float h = static_cast<float>(cell_px) / static_cast<float>(atlas_h);
    return UvRect{u, v, w, h};
}

bool FontAtlas::glyph_has_ink(int codepoint) const noexcept {
    const int slot = valid() ? glyph_slot(codepoint) : -1;
    if (slot < 0) {
        return false;
    }
    const int cx = (slot % cols) * cell_px;
    const int cy = (slot / cols) * cell_px;
    for (int y = 0; y < cell_px; ++y) {
        const int row = (cy + y) * atlas_w + cx;
        for (int x = 0; x < cell_px; ++x) {
            if (pixels[static_cast<std::size_t>(row + x)] != 0) {
                return true;
            }
        }
    }
    return false;
}

std::string resolve_font_path(const std::string& ttf_file) {
    // Tenta os candidatos EM ORDEM e retorna o PRIMEIRO QUE EXISTE no disco. A fonte
    // vive em GusEngine/assets/fonts (asset de engine), separada dos sprites/icons que
    // ficam em <GUSWORLD_ASSETS>/. Por isso checamos existencia: setar GUSWORLD_ASSETS
    // (pra arte) nao pode "sequestrar" a fonte pra um <assets>/fonts inexistente.
    std::error_code ec;
    const auto exists = [&ec](const std::string& p) {
        return !p.empty() && std::filesystem::exists(p, ec);
    };

    // 1) Override por ambiente, SE a fonte de fato estiver la (<GUSWORLD_ASSETS>/fonts).
    if (const char* env = std::getenv("GUSWORLD_ASSETS")) {
        if (env[0] != '\0') {
            const std::string p = join(join(env, "fonts"), ttf_file);
            if (exists(p)) {
                return p;
            }
        }
    }
    // 2) Pasta de fontes do repo embutida em compilacao (= GusEngine/assets/fonts).
    const std::string compiled = join(GUSWORLD_FONTS_DIR, ttf_file);
    if (exists(compiled)) {
        return compiled;
    }
    // 3) Relativo ao CWD (rodando da raiz do GusEngine). Sub-caminho do header central.
    const std::string cwd_rel =
        join(std::string(gus::core::assets::kFontsDir), ttf_file);
    if (exists(cwd_rel)) {
        return cwd_rel;
    }
    // Nenhum existe: devolve o melhor chute (compilado) pra o erro ser claro no log.
    return compiled.empty() ? cwd_rel : compiled;
}

FontAtlas bake_font_atlas(const std::string& ttf_path, int cell_px) {
    FontAtlas atlas;
    if (cell_px <= 0) {
        return atlas;  // invalido
    }

    const std::vector<unsigned char> ttf = read_file(ttf_path);
    if (ttf.empty()) {
        return atlas;  // arquivo ausente/ilegivel: degrada
    }

    stbtt_fontinfo font;
    if (stbtt_InitFont(&font, ttf.data(),
                       stbtt_GetFontOffsetForIndex(ttf.data(), 0)) == 0) {
        return atlas;  // stb_truetype recusou o arquivo
    }

    // Grade quadrada-ish que cabe os kFontGlyphCount glifos (ASCII + Latin-1 = 191).
    // 14 colunas da uma folha compacta (14x14 = 196 >= 191).
    const int cols = 14;
    const int rows = (kFontGlyphCount + cols - 1) / cols;
    const int aw = cols * cell_px;
    const int ah = rows * cell_px;

    std::vector<std::uint8_t> bitmap(static_cast<std::size_t>(aw) * ah, 0);

    // Escala pro tamanho de celula pedido (altura). stb rasteriza top-down.
    const float scale = stbtt_ScaleForPixelHeight(&font, static_cast<float>(cell_px));
    int ascent = 0, descent = 0, line_gap = 0;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);
    const int baseline = static_cast<int>(static_cast<float>(ascent) * scale);

    // Mapeia cada SLOT linear (0..190) -> codepoint da sua faixa (ASCII ou Latin-1).
    const auto slot_to_codepoint = [](int slot) -> int {
        if (slot < kFontAsciiCount) {
            return kFontAsciiFirst + slot;
        }
        return kFontLatin1First + (slot - kFontAsciiCount);
    };

    for (int i = 0; i < kFontGlyphCount; ++i) {
        const int codepoint = slot_to_codepoint(i);
        // Espaco e similares sem contorno: stbtt devolve bitmap vazio (ink=false), ok.
        int gw = 0, gh = 0, gx = 0, gy = 0;
        unsigned char* glyph = stbtt_GetCodepointBitmap(
            &font, scale, scale, codepoint, &gw, &gh, &gx, &gy);
        if (glyph == nullptr || gw <= 0 || gh <= 0) {
            if (glyph != nullptr) {
                stbtt_FreeBitmap(glyph, nullptr);
            }
            continue;  // celula fica vazia (ex.: espaco)
        }

        const int cell_x = (i % cols) * cell_px;
        const int cell_y = (i / cols) * cell_px;
        // Posiciona o glifo na celula pela baseline + bearing do stb (gx,gy negativos
        // acima da baseline). Clampa pra nao vazar a celula.
        const int ox = cell_x + (gx > 0 ? gx : 0);
        const int oy = cell_y + baseline + gy;  // gy tipicamente negativo

        for (int yy = 0; yy < gh; ++yy) {
            const int py = oy + yy;
            if (py < cell_y || py >= cell_y + cell_px) {
                continue;  // fora da celula vertical
            }
            for (int xx = 0; xx < gw; ++xx) {
                const int px = ox + xx;
                if (px < cell_x || px >= cell_x + cell_px) {
                    continue;  // fora da celula horizontal
                }
                const std::uint8_t v =
                    glyph[static_cast<std::size_t>(yy) * gw + xx];
                if (v != 0) {
                    bitmap[static_cast<std::size_t>(py) * aw + px] = v;
                }
            }
        }
        stbtt_FreeBitmap(glyph, nullptr);
    }

    atlas.cell_px = cell_px;
    atlas.cols = cols;
    atlas.rows = rows;
    atlas.atlas_w = aw;
    atlas.atlas_h = ah;
    atlas.pixels = std::move(bitmap);
    return atlas;
}

}  // namespace gus::platform::render2d
