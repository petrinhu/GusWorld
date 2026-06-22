#version 440

// Fragment shader do render2d (M1). Cor lisa interpolada do vertice (retangulos
// placeholder, sem textura). Sem iluminacao, sem sRGB no M1 (placeholder).

// Nome casa com o output do vertex shader (v_color) - o GLSL pareia por nome.
layout(location = 0) in vec4 v_color;
layout(location = 0) out vec4 frag_color;

void main()
{
    frag_color = v_color;
}
