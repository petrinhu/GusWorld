// gus/platform/rmlui/gl3_loader.cpp
//
// Ver header. Carrega o glad (GL 3.3 core) via gladLoadGL, usando o loader de
// proc-address da casca. O glad e header-only com a IMPLEMENTACAO definida no
// RmlUi_Renderer_GL3.cpp (GLAD_GL_IMPLEMENTATION); aqui incluimos so as DECLARACOES
// (gladLoadGL e GLAD_API_CALL extern, resolvido no link com aquela TU).

#include "gus/platform/rmlui/gl3_loader.hpp"

// So as declaracoes do glad (a impl vem do RmlUi_Renderer_GL3.cpp, mesma lib).
#include "RmlUi_Include_GL3.h"

#include <cstring>  // std::memcpy
#include <vector>

namespace gus::platform::rmlui {

bool gl3_load_functions(void* (*proc_loader)(const char*)) {
    if (proc_loader == nullptr) {
        return false;  // sem loader: nada a resolver (headless/CI)
    }
    // GLADloadfunc = GLADapiproc(*)(const char*), GLADapiproc = void(*)(void). O loader da
    // casca devolve void* (SDL_GL_GetProcAddress); reinterpretamos para o tipo do glad.
    const auto glad_loader = reinterpret_cast<GLADloadfunc>(proc_loader);
    const int version = gladLoadGL(glad_loader);
    return version != 0;  // 0 = falhou (GL insuficiente)
}

bool gl3_read_backbuffer_rgba(int w, int h, unsigned char* out) {
    if (w <= 0 || h <= 0 || out == nullptr) {
        return false;
    }
    glReadBuffer(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, out);
    // glReadPixels devolve a 1a linha = base da tela (origem GL embaixo). Inverte a
    // vertical no lugar (swap linha-a-linha) para a origem ficar no topo (formato PNG).
    const int row = w * 4;
    std::vector<unsigned char> tmp(static_cast<std::size_t>(row));
    for (int y = 0; y < h / 2; ++y) {
        unsigned char* top = out + static_cast<std::size_t>(y) * row;
        unsigned char* bot = out + static_cast<std::size_t>(h - 1 - y) * row;
        std::memcpy(tmp.data(), top, row);
        std::memcpy(top, bot, row);
        std::memcpy(bot, tmp.data(), row);
    }
    return true;
}

}  // namespace gus::platform::rmlui
