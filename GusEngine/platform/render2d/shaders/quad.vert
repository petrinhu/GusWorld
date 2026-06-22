#version 440

// Vertex shader do render2d (M1). Os vertices JA chegam em NDC (a projecao
// mundo->NDC e feita na CPU por viewport_transform, que e testada sem GPU). Aqui
// so passa a posicao e a cor adiante. 2D puro: z fixo = 0, w = 1.

layout(location = 0) in vec2 pos_ndc;
layout(location = 1) in vec4 color_attr;

// O NOME do varying tem que casar com a entrada do fragment shader: no OpenGL
// (GLSL) o linker pareia varyings POR NOME, nao so por location. Por isso ambos
// usam 'v_color' (um nome != das entradas/saidas de vertice resolve o warning de
// 'output name does not match input').
layout(location = 0) out vec4 v_color;

void main()
{
    v_color = color_attr;
    gl_Position = vec4(pos_ndc, 0.0, 1.0);
}
