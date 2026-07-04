// gus/core/anim/glitch_dissolve.hpp
//
// GlitchDissolve: matematica PURA (POCO, ZERO SDL/GL/I-O) do efeito de GLITCH
// DIGITAL/"PROCESSANDO" que substitui o fade preto liso da transicao cidade<->
// batalha (M7-COSTURA Inc 2b, pedido ao vivo do Gus Dragon + veredito do lider: "o
// sistema runico do Gus REBOOTANDO pro modo de combate" - Pillar "magia = software").
//
// A curva TEMPORAL continua sendo core::anim::fade_overlay_alpha (kOut 0->1, kIn
// 1->0) - este modulo NAO reimplementa isso. O que ele acrescenta e a
// "DISSOLVE EM BLOCOS": em vez de um retangulo preto crescendo uniforme, a tela e
// dividida numa grade fixa (kGlitchGridCols x kGlitchGridRows) e cada CELULA tem um
// LIMIAR pseudo-aleatorio ESTAVEL (glitch_cell_threshold - determinista, mesma
// celula sempre devolve o mesmo numero, sem <random>/estado/tempo real). A celula
// "acende" (opaca) quando o envelope geral (a saida de fade_overlay_alpha) ultrapassa
// o limiar dela - visual de "compilando/processando bloco a bloco", nao um escurecer
// uniforme. glitch_cell_is_wavefront marca a FRANJA de celulas recem-acendidas (pra
// o desenho colori-las de cyan/magenta em vez de preto solido - o "flash" de energia
// da compilacao) e glitch_cell_shift_fraction da um deslocamento horizontal FIXO por
// celula (pro desenho aplicar SO nas celulas da franja - o efeito de "bloco
// deslocado/datamosh" pedido, sem precisar ler o framebuffer: a celula inteira e
// desenhada num x levemente deslocado, revelando uma tira da cena original de um
// lado e sobrepondo o vizinho do outro).
//
// QUEM DESENHA (draw_filled_rect por celula, cor preta ou accent cyan/magenta na
// franja) fica na camada app/ (gus/app/glitch_overlay.hpp::draw_glitch_overlay) -
// aqui e so a matematica determinista, testavel sem janela. Funciona IDENTICO em
// qualquer IRenderer (SDL_Renderer classico ou GL3): so usa draw_filled_rect, a
// mesma primitiva que o fade preto original ja usava.
//
// INVARIANTE DE SEGURANCA (herdado do fade_transition original): no PICO do
// envelope (alpha<=0 -> 0 celulas acesas / alpha>=1 -> TODAS as celulas acesas,
// SEM franja, SEM deslocamento), a grade cobre a tela inteira com preto solido -
// e o instante em que a Maestro troca o SDL_Renderer pelo contexto GL da batalha
// (release_renderer/reacquire_renderer) escondida atras do preto. glitch_cell_is_
// wavefront devolve false nos extremos de proposito (nunca ha bloco deslocado ou
// colorido exatamente quando a tela deveria estar 100% opaca).

#ifndef GUS_CORE_ANIM_GLITCH_DISSOLVE_HPP
#define GUS_CORE_ANIM_GLITCH_DISSOLVE_HPP

namespace gus::core::anim {

// Dimensoes da grade do glitch. 16x9 casa a proporcao 16:9 da tela/mundo (celulas
// ~quadradas em qualquer resolucao 16:9 usada no projeto) - grande o bastante pra
// "ler" como pixelizacao/dissolve digital (nao um fade liso), pequena o bastante
// pra caber em ~150 draw_filled_rect por frame em QUALQUER backend, sem custo
// perceptivel (a transicao inteira dura so ~0.4s, ver kTransitionFadeSeconds em
// maestro.cpp).
inline constexpr int kGlitchGridCols = 16;
inline constexpr int kGlitchGridRows = 9;

// Limiar pseudo-aleatorio ESTAVEL (determinista, hash inteiro simples tipo
// splitmix - sem <random>, sem estado, sem tempo real) da celula (col,row), em
// [0,1). A MESMA celula SEMPRE devolve o MESMO limiar entre chamadas/processos -
// da a impressao de uma "ordem de compilacao" fixa (celulas de limiar baixo
// acendem primeiro) e torna o efeito 100% reproduzivel (testavel, sem flakiness).
// col/row fora de [0,kGlitchGridCols)/[0,kGlitchGridRows) ainda devolvem um valor
// bem definido (o hash aceita qualquer int) - sem UB, so sem sentido visual.
[[nodiscard]] float glitch_cell_threshold(int col, int row) noexcept;

// Alpha [0,1] do bloco preto da celula (col,row), dado o envelope geral `alpha`
// (a saida JA CALCULADA de fade_overlay_alpha). A celula acende (1.0, opaca) assim
// que `alpha` ultrapassa glitch_cell_threshold(col,row); antes disso fica 0.0 (nada
// desenhado - a cena original/nova aparece por baixo, sem overlay). alpha<=0.0f
// devolve 0.0f pra QUALQUER celula (nenhum bloco aceso - tela limpa) e alpha>=1.0f
// devolve 1.0f pra QUALQUER celula (grade inteira opaca - tela 100% "compilada"),
// os dois extremos exatos que fade_overlay_alpha ja garantia pro retangulo liso
// original (mesmo invariante de seguranca, ver o comentario do header).
[[nodiscard]] float glitch_block_alpha(int col, int row, float alpha) noexcept;

// true se a celula (col,row) esta na FRANJA (wavefront) do glitch neste instante:
// ja acendeu (alpha >= o limiar dela) mas ha MENOS de `band` de folga entre o
// envelope corrente e o limiar (isto e, "acendeu ha pouquissimo tempo/poucos
// frames"). Usado pelo desenho pra colorir SO essa franja fina de cyan/magenta (o
// "flash" de energia) e aplicar o deslocamento horizontal (glitch_cell_shift_
// fraction) - as celulas ja "assentadas" (fora da franja, alpha bem alem do limiar)
// ficam pretas e no lugar, escondendo a cena com seguranca. Devolve false nos
// EXTREMOS do envelope (alpha<=0 ou alpha>=1) e para band<=0 - nunca ha franja
// exatamente quando a tela deveria estar limpa ou 100% opaca solida (o invariante
// de seguranca da troca de renderer nunca ve bloco deslocado/colorido).
[[nodiscard]] bool glitch_cell_is_wavefront(int col, int row, float alpha,
                                             float band) noexcept;

// Deslocamento horizontal ESTAVEL da celula (col,row), como FRACAO da largura da
// celula, em [-1,1]. Determinista (mesma celula sempre o mesmo valor, hash
// independente de glitch_cell_threshold - proposito: o limiar de quando a celula
// acende e o quanto ela desloca nao devem correlacionar, senao o padrao ficaria
// previsivel demais). O desenho (camada app/) SO aplica isto as celulas na franja
// (glitch_cell_is_wavefront) - desenhar a celula inteira num x levemente deslocado
// revela uma tira da cena original de um lado e sobrepoe o vizinho preto do outro:
// o efeito de "bloco deslocado" (datamosh-lite) pedido, sem precisar ler pixels do
// framebuffer (impossivel via draw_filled_rect puro em ambos os backends).
[[nodiscard]] float glitch_cell_shift_fraction(int col, int row) noexcept;

}  // namespace gus::core::anim

#endif  // GUS_CORE_ANIM_GLITCH_DISSOLVE_HPP
