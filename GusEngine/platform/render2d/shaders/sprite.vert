#version 440

// Vertex shader de SPRITE do render2d. Como o quad colorido, os vertices ja chegam
// em NDC (projecao mundo->NDC feita na CPU por viewport_transform, testada sem GPU).
// Aqui passa a posicao adiante e repassa UV + tint pro fragment. 2D puro (z=0, w=1).

layout(location = 0) in vec2 pos_ndc;
layout(location = 1) in vec2 uv_attr;
layout(location = 2) in vec4 tint_attr;

layout(location = 0) out vec2 v_uv;
layout(location = 1) out vec4 v_tint;

void main()
{
    v_uv = uv_attr;
    v_tint = tint_attr;
    gl_Position = vec4(pos_ndc, 0.0, 1.0);
}
