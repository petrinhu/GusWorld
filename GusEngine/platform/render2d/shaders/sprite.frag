#version 440

// Fragment shader de SPRITE do render2d. Amostra a textura (NEAREST, definido no
// sampler do C++ -> pixel-art crisp) e multiplica pelo tint. O alpha vem da
// textura (PNG com fundo transparente) * tint.a; o blend alpha (definido no
// pipeline) recorta o fundo. Sem iluminacao/sRGB no M1 (placeholder).

layout(location = 0) in vec2 v_uv;
layout(location = 1) in vec4 v_tint;
layout(location = 0) out vec4 frag_color;

layout(binding = 0) uniform sampler2D sprite_tex;

void main()
{
    vec4 texel = texture(sprite_tex, v_uv);
    frag_color = texel * v_tint;
}
