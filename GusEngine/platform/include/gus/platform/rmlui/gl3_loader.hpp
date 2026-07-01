// gus/platform/rmlui/gl3_loader.hpp
//
// gl3_load_functions: carrega os ponteiros de funcao OpenGL 3.3 (glad, embarcado no
// backend GL3 do RmlUi) usando o loader de proc-address da casca (ex.:
// SDL_GL_GetProcAddress). Deve ser chamado DEPOIS de tornar o contexto GL corrente e
// ANTES de qualquer chamada GL (Render2dGl3 / RmlUiHud). Devolve true se carregou.
//
// HEADER LIMPO: nao inclui o glad nem GL. app/ chama isto passando o
// SDL_GL_GetProcAddress (reinterpretado para void*(*)(const char*)), sem arrastar
// <GL...> pros headers.

#ifndef GUS_PLATFORM_RMLUI_GL3_LOADER_HPP
#define GUS_PLATFORM_RMLUI_GL3_LOADER_HPP

namespace gus::platform::rmlui {

// Carrega o glad (GL 3.3 core) via o loader de proc-address dado (ex.:
// SDL_GL_GetProcAddress). true = ok; false = falhou (loader nulo ou versao GL
// insuficiente). Chamar uma vez por contexto, com o contexto ja corrente.
bool gl3_load_functions(void* (*proc_loader)(const char*));

// Le o BACKBUFFER GL (w x h pixels, RGBA8) para out (deve ter w*h*4 bytes), ja com a
// origem no TOPO-esquerda (glReadPixels le de baixo pra cima; aqui invertemos a verticais
// pra casar com PNG). Usado pelo smoke visual --battle pra capturar o frame composto
// (arena + HUD) num PNG. true = ok; false = parametros invalidos. Exige glad carregado.
bool gl3_read_backbuffer_rgba(int w, int h, unsigned char* out);

}  // namespace gus::platform::rmlui

#endif  // GUS_PLATFORM_RMLUI_GL3_LOADER_HPP
