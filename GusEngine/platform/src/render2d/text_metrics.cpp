// gus/platform/src/render2d/text_metrics.cpp
//
// Implementacao das metricas de texto puras (ver header). Aritmetica monospace, sem
// SDL nem stb_truetype.

#include "gus/platform/render2d/text_metrics.hpp"

namespace gus::platform::render2d {

std::vector<std::uint32_t> decode_utf8(std::string_view text) {
    std::vector<std::uint32_t> out;
    out.reserve(text.size());
    std::size_t i = 0;
    const std::size_t n = text.size();
    while (i < n) {
        const std::uint8_t b0 = static_cast<std::uint8_t>(text[i]);
        std::uint32_t cp = 0;
        int extra = 0;  // bytes de continuacao esperados
        if (b0 < 0x80) {            // 0xxxxxxx: ASCII (1 byte)
            cp = b0;
        } else if ((b0 & 0xE0) == 0xC0) {  // 110xxxxx: 2 bytes (cobre Latin-1)
            cp = b0 & 0x1F;
            extra = 1;
        } else if ((b0 & 0xF0) == 0xE0) {  // 1110xxxx: 3 bytes
            cp = b0 & 0x0F;
            extra = 2;
        } else if ((b0 & 0xF8) == 0xF0) {  // 11110xxx: 4 bytes
            cp = b0 & 0x07;
            extra = 3;
        } else {
            // Continuation byte solto / invalido: pula 1 byte com seguranca (U+FFFD).
            out.push_back(0xFFFD);
            ++i;
            continue;
        }
        // Consome os bytes de continuacao (10xxxxxx). Se faltar/invalido, U+FFFD.
        if (i + static_cast<std::size_t>(extra) >= n + 0) {
            // possivel truncamento; checa cada um abaixo
        }
        bool ok = true;
        std::size_t j = i + 1;
        for (int k = 0; k < extra; ++k, ++j) {
            if (j >= n || (static_cast<std::uint8_t>(text[j]) & 0xC0) != 0x80) {
                ok = false;
                break;
            }
            cp = (cp << 6) | (static_cast<std::uint8_t>(text[j]) & 0x3F);
        }
        if (!ok) {
            out.push_back(0xFFFD);
            ++i;  // avanca 1 e tenta re-sincronizar
            continue;
        }
        out.push_back(cp);
        i = j;  // proximo code point
    }
    return out;
}

int text_char_count(std::string_view text) noexcept {
    // Conta CODE POINTS (UTF-8), nao bytes. Reimplementa o decode SEM alocar (noexcept):
    // conta os bytes que NAO sao continuacao (10xxxxxx) - cada code point comeca num
    // byte-lider. Equivalente a decode_utf8().size() pros textos de UI bem-formados.
    int count = 0;
    for (char c : text) {
        if ((static_cast<std::uint8_t>(c) & 0xC0) != 0x80) {
            ++count;  // byte-lider (ASCII ou inicio de sequencia multibyte)
        }
    }
    return count;
}

float text_width(std::string_view text, float px_size) noexcept {
    if (px_size <= 0.0f) {
        return 0.0f;
    }
    return static_cast<float>(text_char_count(text)) * kMonoAdvanceRatio * px_size;
}

float text_height(float px_size) noexcept {
    return px_size > 0.0f ? px_size : 0.0f;
}

float glyph_advance(float px_size) noexcept {
    return px_size > 0.0f ? kMonoAdvanceRatio * px_size : 0.0f;
}

}  // namespace gus::platform::render2d
