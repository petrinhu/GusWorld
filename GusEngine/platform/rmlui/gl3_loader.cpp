// gus/platform/rmlui/gl3_loader.cpp
//
// Ver header. Carrega o glad (GL 3.3 core) via gladLoadGL, usando o loader de
// proc-address da casca.
//
// DONO DO GLAD (ADR-010 F3): o glad e header-only e precisa de UMA TU que defina
// GLAD_GL_IMPLEMENTATION na lib gusengine_platform. Antes o dono era o RmlUi_Renderer_GL3.cpp
// vendorizado; com o backend RmlUi aposentado (F3), este gl3_loader.cpp - o loader GL da
// ARENA (Render2dGl3), que fica SEMPRE - assume a IMPLEMENTACAO (o CMake passa sempre
// GUSWORLD_OWN_GLAD_IMPL). O glintfx (motor de UI) usa gl3w (tabela de ponteiros PROPRIA),
// independente do glad, entao nao ha conflito de simbolos GL.

#include "gus/platform/rmlui/gl3_loader.hpp"

// Impl do glad (este loader e o dono unico - ver bloco acima). GUSWORLD_OWN_GLAD_IMPL vem
// sempre do CMake; o #ifdef fica como cinto de seguranca caso a macro suma.
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
