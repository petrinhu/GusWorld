// gus/platform/rmlui/gl3_loader.cpp
//
// Ver header. Carrega o glad (GL 3.3 core) via gladLoadGL, usando o loader de
// proc-address da casca.
//
// DONO DO GLAD (ADR-010 R-dup-backend): o glad e header-only e precisa de UMA TU que defina
// GLAD_GL_IMPLEMENTATION na lib gusengine_platform.
//  - Build OFF (default): a impl vive no RmlUi_Renderer_GL3.cpp (backend vendorizado); aqui
//    entram so as DECLARACOES (GLAD_API_CALL extern, resolvido no link com aquela TU).
//  - Build GUSWORLD_GLINTFX=ON: o RmlUi_Renderer_GL3.cpp vendorizado NAO e compilado, mas a
//    ARENA (render2d_gl3) e este loader ainda usam o glad. Como este gl3_loader fica nos DOIS
//    builds, ele assume a IMPLEMENTACAO (CMake passa GUSWORLD_OWN_GLAD_IMPL no ON). O glintfx
//    usa gl3w (tabela PROPRIA), entao nao ha conflito de simbolos GL.

#include "gus/platform/rmlui/gl3_loader.hpp"

// Declaracoes do glad (+ impl quando este loader e o dono - ver bloco acima).
#ifdef GUSWORLD_OWN_GLAD_IMPL
#define GLAD_GL_IMPLEMENTATION
#endif
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
