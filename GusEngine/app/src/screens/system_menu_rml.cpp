// gus/app/src/screens/system_menu_rml.cpp
//
// Implementacao de build_system_menu_rml. Ver header para o contrato.
//
// HEXAGONO DO SLIDER (MENU-PAUSA-CONFIG-SOM, glintfx v0.3.0): a DIVERGENCIA antiga
// (circulo place-holder, glintfx v0.2.5 nao tinha decorator de poligono) esta
// FECHADA - `decorator: polygon(<sides>, <color>[, <rotation>])` chegou na v0.3.0
// (ver docs/effects.md do glintfx vendorizado). O `.slider-node` agora e um
// hexagono NATIVO fiel ao mock (docs/design/mockups/
// 01-menu-sistema-proposta-a-console-centralizado.html: SVG <polygon
// points="11,2 19,7 19,15 11,20 3,15 3,7" fill="#0A0E1A" stroke="#22D3EE"
// stroke-width="1.6">, vertice apontando pra cima - MESMA orientacao default do
// polygon() do glintfx). Como o decorator so pinta 1 cor SOLIDA (sem stroke
// separado - a API deliberadamente NAO duplica isso, ver docs/effects.md "no
// separate polygon-glow or polygon-clip API"), o "contorno cyan + preenchimento
// escuro" do mock e reproduzido com DOIS hexagonos CONCENTRICOS (mesmo truque que
// um border-radius de 2 camadas faria com retangulos): `.slider-node` (o de FORA,
// 22dp, cor cyan/branco conforme foco - faz as vezes do "stroke") e
// `.slider-node-hex-inner` (o de DENTRO, 18dp, centralizado com 2dp de inset por
// lado - faz as vezes do "fill" escuro). `filter: drop-shadow(...)` no hexagono de
// FORA da o glow que contorna a forma exata (nao um retangulo) - zero API nova,
// exatamente a receita do proprio docs/effects.md ("glow que abraca o contorno do
// hexagono").
//
// ONDA ARVORE (MENU-PAUSA-CONFIG-SOM, retoques ao vivo do lider apos ver a versao
// de 2 telas planas rodando):
//   (1) ARVORE HIERARQUICA: Pause ganhou o item "Salvar" e o antigo item unico
//       "Config" virou uma tela INTERMEDIARIA "ConfigCategories" (Audio/Video/
//       Lingua/Voltar); a tela de sliders (Musica/SFX) e agora "Audio", filha de
//       ConfigCategories; Save/Video/Language sao placeholders "em breve" - ver
//       system_menu.hpp para o diagrama completo e parent_screen_of(). 7 corpos
//       possiveis (build_pause_body/build_config_categories_body/build_audio_body/
//       build_placeholder_body x3), 1 por chamada (state.screen decide qual).
//   (2) BOTOES MAIS COMPACTOS (2a leva): a 1a leva ja tinha reduzido
//       padding-vertical (14dp->9dp); esta reduz mais (9dp->7dp) e a LARGURA
//       (364dp->340dp) - o lider agora tem 4 pills em Pause/ConfigCategories (era
//       3), entao a economia de altura por item importa mais.
//   (3) PADDING-BOTTOM do painel (24dp->40dp): o lider reportou o botao Voltar (e
//       o conteudo em geral) TOCANDO os hexagonos dos CANTOS INFERIORES (.corner.bl/
//       .corner.br, ancorados em bottom:8dp/height:20dp - zona [painel_bottom-28dp,
//       painel_bottom-8dp]). Com padding-bottom:24dp o conteudo terminava em
//       painel_bottom-24dp, DENTRO dessa zona (overlap de ~4dp). padding-bottom:40dp
//       move o fim do conteudo pra painel_bottom-40dp, 12dp ACIMA do topo da zona do
//       corner (painel_bottom-28dp) - folga real, nao so cosmetica.
//   (4) EFEITO DE PRESS (`pressed_index`, ver header): NAO e feedback nativo do
//       glintfx (a lib so tem :focus/:hover via classe, sem estado de "active"
//       tocado por Enter/Espaco de TECLADO) - e um flash construido pelo NOSSO lado
//       (system_menu_loop.cpp gera este RML com pressed_index setado, renderiza
//       alguns frames, SO ENTAO aplica a transicao real). `.pressed` inverte as
//       cores (fundo cyan solido + texto escuro + glow branco) por ~100ms - "brilho
//       intenso", pedido do lider - reaproveitando decorator: vertical-gradient
//       (MESMA API de .verb-pill, sem decorator novo).
//
// Traducao do mock pro RCSS do glintfx:
//   .panel (gradiente vertical 3-stop + borda latao + box-shadow glow cyan sutil)
//     -> decorator: linear-gradient(180deg, ...) (native RmlUi, MESMA familia de
//        shader do radial-gradient ja shipado no cockpit - ver DecoratorGradient.cpp)
//   .corner (moldura latao, hexagono solido + drop-shadow) -- REVISADO ao vivo
//     (MENU-PAUSA-CONFIG-SOM): a 1a versao usava radial-gradient(circle at 35%
//     30%, ...) + box-shadow, POSICIONADA NA QUINA (top/left -10dp, metade
//     dentro/metade fora do painel) - o lider reportou "fica estranho" (a linha
//     da moldura cruzando o centro do circulo). Trocado por decorator:
//     polygon(6, #C9A24B) (hexagono de latao SOLIDO, MESMA API do
//     .slider-node acima, verificada empiricamente antes desta troca - probe
//     Xvfb :99 confirmou geometria hexagonal real via glReadPixels) +
//     filter: drop-shadow (nao box-shadow: precisa abracar o contorno
//     hexagonal, nao desenhar sombra retangular atras dele). Posicao agora
//     POSITIVA (top/left/right/bottom: 8dp) - INTEIRA DENTRO do painel, perto
//     de cada quina mas sem cruzar a moldura. FECHADO (glintfx v0.3.1, bump
//     nesta onda): polygon() ganhou fill radial-gradient(...)/linear-gradient(...)
//     real (per-pixel), entao .corner agora e polygon(6, radial-gradient(circle
//     at 40% 35%, #F0D98C, #C9A24B 55%, #7A5A2E 100%)) - volume de "parafuso de
//     latao com luz" igual ao mock original, sem hexagonos duplicados.
//   .verb-pill / .verb-pill.focused (glow neon do selecionado)
//     -> MESMA receita de .verb/.verb.sel do cockpit (border + box-shadow spread
//        positivo + decorator: vertical-gradient). Padding/largura reduzidos em
//        2 levas ao vivo (14dp/12dp/364dp -> 9dp/8dp/364dp -> 7dp/6dp/340dp,
//        ver "ONDA ARVORE (2)" acima) - lider: "grandes demais" (1a leva),
//        depois mais compacto ainda (2a leva, mais itens por tela agora).
//        Reusada tambem pelas categorias de ConfigCategories (Audio/Video/
//        Lingua) - MESMO estilo visual de "pill de acao" que os verbos de Pause.
//        .btn-back seguiu a MESMA leva (13dp -> 9dp -> 7dp de padding).
//   .track/.fill (slider) -> decorator: vertical-gradient (track escuro) +
//     horizontal-gradient (fill cyan) + box-shadow glow
//   .slider-node/.slider-node-hex-inner (thumb hexagonal) -> decorator: polygon()
//     em 2 camadas concentricas, ver acima.
//   .title-glow (text-shadow neon no titulo) -> RmlUi nao tem 'text-shadow' CSS
//     (so box-shadow/drop-shadow em CAIXAS, nao em glifos de texto - o glow do
//     TITULO em si fica de fora nesta versao; o resto do painel mantem o glow
//     neon completo). Nota secundaria, reportada mas nao bloqueia.
//
// CENTRALIZACAO REAL de .verb-pill/.btn-back (retoque ao vivo do lider, pos-ONDA
// ARVORE): a causa raiz era `width:340dp` FIXO sem `margin-left/right:auto` - o
// item ficava flush-LEFT dentro do content-box de #sysmenu-panel (420dp - 2*28dp
// padding - 2*1dp border = 362dp, box-sizing:border-box), sobrando 22dp de folga
// TODA a direita. Ao lado das barras cheias do slider (tela Audio) a assimetria
// aparecia (reportado ao vivo). CONFIRMADO por leitura do FONTE do RmlUi
// vendorizado (nao so probe empirico desta vez): LayoutDetails::BuildBoxWidth
// (Source/Core/Layout/LayoutDetails.cpp, glintfx-src/build) resolve margin auto
// para block-boxes NORMAIS (nao so absolutamente posicionados) - quando o width
// e explicito (nao-auto) e ha margens 'auto' declaradas, o metodo distribui
// `(containing_block.x - width) / num_auto_margins` entre elas (linhas ~447-457):
// com containing_block.x=362dp e width=340dp, margin-left=margin-right=11dp cada
// - centralizacao EXATA, generica pra qualquer largura de painel/janela (nao um
// valor -Xdp hardcoded feito a mao, como o `margin-left:-210dp` do
// #sysmenu-panel acima - aqui um `auto` de verdade basta, confirmado pelo motor
// de layout aceitar a keyword nas 4 propriedades margin-* via
// StyleSheetSpecification.cpp). FIX: `margin-left: auto; margin-right: auto;`
// em .verb-pill/.btn-back (preservando margin-bottom/margin-top existentes).

#include "gus/app/screens/system_menu_rml.hpp"

#include <cmath>
#include <sstream>
#include <string>

#include "gus/domain/input/action_registry.hpp"  // ActionDefinition (label i18n key + categoria)
#include "gus/domain/input/controls_diff.hpp"    // human_label_for_keycode (rotulo legivel da tecla)

namespace gus::app::screens {

namespace {

std::string pct_string(float v01) {
    const int pct = static_cast<int>(std::lround(v01 * 100.0f));
    return std::to_string(pct) + "%";
}

// " pressed" se `index == pressed_index` (pressed_index >= 0), senao "" - usado
// pelos builders abaixo pra decorar o item ATIVADO com o flash de PRESS (ver
// header/comentario "ONDA ARVORE (4)" acima). pressed_index<0 (default) nunca
// bate com nenhum index>=0: nenhuma tela tem item pressionado.
std::string pressed_class(int index, int pressed_index) {
    return (pressed_index >= 0 && index == pressed_index) ? " pressed" : "";
}

// Header RCSS comum as 7 telas (paleta canonica + moldura latao + pills).
// dp = px logico do mock (canvas 960x540, mesma convencao do cockpit).
constexpr const char* kSharedStyle = R"RCSS(
/* CAUSA RAIZ DO DESLOCAMENTO (MENU-PAUSA-CONFIG-SOM, achado por probe headless
   2026-07-04): o <body> do RmlUi, quando so tem filhos position:absolute e NAO
   declara a propria largura/altura, colapsa pra uma content-box de tamanho 0dp x
   0dp (shrink-to-fit sem conteudo em fluxo normal - confirmado lendo
   ElementDocument::UpdateLayout, que usa GetParentNode()->GetBox() como
   containing block do body, mas o body em si so herda ESSE tamanho se tiver uma
   regra de largura/altura que o force a preenche-lo). Com isso, QUALQUER
   porcentagem resolvida contra o body (containing block dos filhos absolutos,
   ja que body fica position:static) vira 0: `left: 50%` = 50% de 0dp = 0dp - so
   o `margin-left: -210dp` (valor FIXO, nao percentual) sobra, empurrando o
   painel todo pra fora da borda esquerda (exatamente o bug relatado ao vivo: o
   centro do painel caindo perto do x=0 da janela, cortado). O cockpit da
   batalha (battle_preview.cpp) NUNCA bateu nesse caminho porque todo elemento
   dele usa offsets FIXOS em dp (ex. "top:20dp;left:20dp") - nao percentual nem
   0dp+0dp simetrico - entao o bug ficou latente e invisivel ate aqui. Provado
   por probe standalone (glintfx::UiLayer isolado, Xvfb headless, 3 larguras de
   janela): com body sem width/height, left:50% sempre resolvia a 0
   independente de dp_ratio/viewport; com width:100%;height:100% no body, o
   mesmo RCSS centraliza EXATO em qualquer largura testada (960/1330/1920px).
   FIX: dar ao body a largura/altura cheia do viewport (100%, contra o ROOT que
   SetDimensions() ja tamanho corretamente) - assim ele vira o containing block
   correto pros filhos absolutos, com ou sem porcentagem. Bonus: o
   #sysmenu-scrim (left/right/top/bottom:0dp, pensado pra cobrir a tela toda)
   sofria do MESMO bug (virava 0x0, invisivel) - corrigido de graca pelo mesmo
   fix, sem mexer na regra dele. NAO e bug do glintfx: e uso incompleto do RCSS
   deste documento (falta de width/height no body) - nao ha nada a reportar ao
   dev da lib. */
body { font-family: "Pixel Operator Mono"; background: transparent; width: 100%; height: 100%; }

#sysmenu-scrim {
  position: absolute; top: 0dp; left: 0dp; right: 0dp; bottom: 0dp;
  background-color: #05070ca8;
}

#sysmenu-panel {
  /* box-sizing:border-box (residuo do MESMO probe acima): sem isto, "width:420dp" e
     CONTEUDO (default content-box), e a border(1dp*2)+padding(28dp*2) somam 58dp que o
     "margin-left:-210dp" (metade de 420, nao de 478) nao compensa - o centro ficava
     ~29-58px a DIREITA do centro real da janela (escalando com dp_ratio), pequeno mas
     mensuravel. border-box faz "width:420dp" jah incluir border+padding, batendo com a
     conta de metade (210dp) usada no margin-left - centralizacao exata confirmada pelo
     probe headless (delta < 0.01px em 960/1330/1920/800px de janela). */
  box-sizing: border-box;
  position: absolute; top: 90dp; left: 50%; margin-left: -210dp;
  width: 420dp;
  decorator: linear-gradient( 180deg, #3A4566 0%, #1B2238 42%, #0A0E1A 100% );
  border: 1dp #7A5A2E;
  border-radius: 20dp;
  /* padding-bottom 24dp->40dp (ONDA ARVORE (3), ver header): folga real contra os
     cantos hexagonais inferiores - detalhe na conta no header deste arquivo. */
  padding: 30dp 28dp 40dp 28dp;
  box-shadow: #22D3EE1a 0dp 0dp 50dp 0dp;
}

/* CANTOS HEXAGONAIS: decorator: polygon(6, <fill>) do glintfx v0.3.1 (bump
   ver CMakeLists.txt/GIT_TAG). O fill agora aceita radial-gradient(...) real
   (per-pixel, mesmo shader do radial-gradient nativo do RmlUi - ver
   docs/effects.md do glintfx vendorizado, secao "a polygon with a gradient
   fill"), reproduzindo o "parafuso de latao com luz" do mock original
   (01-menu-sistema-proposta-a-console-centralizado.html, radial-gradient num
   circulo). "at 40% 35%" desloca o highlight pra cima-esquerda (luz
   off-center); o ultimo stop (100%) cai exatamente na borda do circulo
   inscrito do hexagono. */
.corner {
  position: absolute; width: 20dp; height: 20dp;
  decorator: polygon( 6, radial-gradient( circle at 40% 35%, #F0D98C, #C9A24B 55%, #7A5A2E 100% ) );
  filter: drop-shadow( #00000080 0dp 1dp 3dp );
}
.corner.tl { top: 8dp; left: 8dp; }
.corner.tr { top: 8dp; right: 8dp; }
.corner.bl { bottom: 8dp; left: 8dp; }
.corner.br { bottom: 8dp; right: 8dp; }

.kicker { text-align: center; color: #9AA5C0; font-size: 11dp; letter-spacing: 4dp; margin-bottom: 2dp; }
/* line-height 36dp (era o default herdado "1.2", ~31dp) + padding-top 4dp
   (achado da analise das capturas headless 2026-07-04, PROBLEMA 2): maiuscula
   acentuada ("Á" de "Áudio", ver menu_audio.png) tinha o topo do acento CORTADO
   - a caixa da linha nao tinha folga suficiente acima do cap-height pro
   diacritico da fonte pixel (Pixel Operator Mono) render inteiro. Aumentar
   line-height da folga vertical ao redor da linha (RmlUi distribui o excesso
   simetricamente acima/abaixo da baseline - RegisterProperty("line-height")
   em StyleSheetSpecification.cpp) e o padding-top da uma folga adicional fixa
   SO no topo (onde o corte acontecia) sem mexer no espacamento existente
   ate o divider (margin-bottom:18dp preservado - nao desalinha o resto). */
.title { text-align: center; font-size: 26dp; line-height: 36dp; color: #ffffff; margin: 2dp 0dp 18dp 0dp; padding-top: 4dp; letter-spacing: 4dp; }
.divider { height: 1dp; background-color: #ffffff2e; margin-bottom: 20dp; }

/* BOTOES COMPACTOS, 2a LEVA (ONDA ARVORE (2), ver header): 340dp de largura
   (era 364dp) + padding vertical 7dp (era 9dp) + margin-bottom 6dp (era 8dp) -
   Pause/ConfigCategories agora tem 4 itens (era 3), a economia de altura por
   pill importa mais. Reusada pelas categorias de ConfigCategories tambem (MESMO
   estilo de "pill de acao" que os verbos de Pause).
   text-align:center + box-sizing:border-box (retoque ao vivo do lider): rotulo
   centralizado (era esquerda) pra ficar uniforme com .btn-back (que ja nasceu
   centralizado); border-box pelo MESMO motivo documentado em #sysmenu-panel
   acima - garante que "width:340dp" seja a caixa RENDERIZADA final (incluindo
   padding+border), nao so o conteudo, pra bater exatamente com .btn-back. */
.verb-pill {
  box-sizing: border-box;
  position: relative; width: 340dp; padding: 7dp 16dp;
  margin-top: 0dp; margin-bottom: 6dp; margin-left: auto; margin-right: auto;
  text-align: center;
  decorator: vertical-gradient( #3A4566 #1B2238 );
  border: 1dp #ffffff12; border-radius: 999dp;
  font-size: 13dp; color: #E7ECF5; letter-spacing: 2dp;
}
/* HOVER (mouse, PEDIDO 2a - MESMA receita/ORDEM de .verb:hover em
   battle_preview.cpp:352-360, ver o comentario de la): DECLARADO ANTES de
   .focused/.danger/.pressed abaixo de proposito. :hover conta como 1
   pseudo-classe - a MESMA especificidade de uma classe (.focused/.danger/
   .pressed tem cada 2: .verb-pill + a propria), entao .verb-pill:hover (tambem
   2: .verb-pill + :hover) EMPATA com elas; o empate cai pra ORDEM DE FONTE, e
   quem vem DEPOIS vence. Declarando :hover PRIMEIRO garante que
   focado/pressionado sempre "ganham" o hover simultaneo (o glow FORTE da
   selecao real nunca e ofuscado pelo feedback discreto do mouse). Fundo mais
   claro + borda neutra clara, SEM box-shadow (zero glow que dispute com o
   .focused) - mesma paleta discreta usada no cockpit. */
.verb-pill:hover {
  decorator: vertical-gradient( #47547a #232b46 );
  border-color: #5a6a92;
}
.verb-pill.focused {
  color: #ffffff; border: 1dp #22D3EE;
  box-shadow: #22D3EE 0dp 0dp 22dp 2dp;
}
.verb-pill.danger { border: 1dp #E11D7459; }
/* PRESS (ONDA ARVORE (4), ver header): flash de ~100ms ANTES da transicao real -
   inverte pra fundo cyan solido + texto escuro + glow branco intenso. Efeito
   NOSSO (system_menu_loop.cpp gera o RML com pressed_index setado, renderiza
   alguns frames, SO ENTAO aplica a acao) - nao e estado nativo do glintfx. */
.verb-pill.pressed {
  decorator: vertical-gradient( #22D3EE #0EA5C9 );
  color: #071019; border: 1dp #ffffff;
  box-shadow: #ffffff 0dp 0dp 30dp 4dp;
}

.footer-hint { text-align: center; color: #9AA5C0; font-size: 11dp; margin-top: 6dp; }

.field { margin-bottom: 20dp; }
/* .field-row (achado da analise das capturas headless 2026-07-04, PROBLEMA 3):
   nome do campo e percentual vinham COLADOS (ex. "70%" grudado no rotulo
   sem nenhum espaco visivel, ver menu_audio.png) - o espaco literal entre os
   2 <span> no RML nao bastava (padrao de tela de configuracao pede
   nome-esquerda/valor-direita claramente separados, nao so 1 espaco).
   display:flex + justify-content:
   space-between joga o nome pra ESQUERDA e o valor pra DIREITA da linha (a
   largura cheia do .field, MESMA API flexbox ja suportada pelo RmlUi 6.3 -
   RegisterProperty("justify-content") em StyleSheetSpecification.cpp inclui
   "space-between"); align-items:baseline alinha as duas fontes de tamanho
   diferente (11dp/13dp) pela linha de base, nao pelo topo/centro da caixa.
   Wrapper PROPRIO (nao o .field inteiro) porque .field tambem e pai do
   .track/slider logo abaixo - se o flex fosse no .field, o track viraria um
   3o item da MESMA linha flex (quebraria o layout vertical nome+valor / barra
   embaixo). */
.field-row { display: flex; justify-content: space-between; align-items: baseline; }
.field .name { color: #9AA5C0; font-size: 11dp; letter-spacing: 1dp; }
.field .val { color: #22D3EE; font-size: 13dp; }
.track {
  position: relative; height: 14dp; border-radius: 999dp; margin-top: 10dp;
  decorator: vertical-gradient( #0A0E1A #000000 );
  box-shadow: #00000080 0dp 2dp 5dp 0dp;
}
.fill {
  position: absolute; top: 0dp; left: 0dp; bottom: 0dp; border-radius: 999dp;
  decorator: horizontal-gradient( #0891A8 #22D3EE );
  box-shadow: #22D3EE8c 0dp 0dp 12dp 0dp;
}
/* HEXAGONO NATIVO (glintfx v0.3.0, ver header): 2 camadas concentricas - a de FORA
   (cyan/branco conforme foco) faz as vezes do "stroke" do mock, a de DENTRO
   (.slider-node-hex-inner, escura, 18dp/2dp de inset) faz as vezes do "fill". */
.slider-node {
  position: absolute; top: -4dp; width: 22dp; height: 22dp; margin-left: -11dp;
  decorator: polygon( 6, #22D3EE );
  filter: drop-shadow( #22D3EEd9 0dp 0dp 6dp );
}
.slider-node.focused {
  decorator: polygon( 6, #ffffff );
  filter: drop-shadow( #22D3EEff 0dp 0dp 10dp );
}
.slider-node-hex-inner {
  position: absolute; top: 2dp; left: 2dp; width: 18dp; height: 18dp;
  decorator: polygon( 6, #0A0E1A );
}

/* Voltar (Audio/ConfigCategories/placeholders) - MESMA leva de compactacao do
   .verb-pill acima (largura 340dp, padding 7dp).
   box-sizing:border-box + padding horizontal 16dp (retoque ao vivo do lider):
   antes o padding era so "7dp" (7dp nos 4 lados, 7dp de horizontal) contra os
   "7dp 16dp" (16dp de horizontal) do .verb-pill - em content-box (default) as
   duas caixas RENDERIZADAS ficavam com larguras totais diferentes (356dp vs
   374dp) mesmo com o mesmo "width:340dp" declarado, entao a coluna nao
   alinhava (Voltar parecia mais estreito/deslocado). Copiando box-sizing +
   padding horizontal do .verb-pill garante caixa IDENTICA (mesma borda
   esquerda/direita) - so o estilo (border mais sutil, sem preenchimento,
   cor mais apagada) continua distinto. */
.btn-back {
  box-sizing: border-box;
  text-align: center; width: 340dp; padding: 7dp 16dp;
  margin-top: 6dp; margin-left: auto; margin-right: auto;
  border: 1dp #3A4566; border-radius: 999dp; color: #9AA5C0;
  font-size: 12dp; letter-spacing: 2dp;
}
/* HOVER (mouse, PEDIDO 2a) - MESMA ordem/logica de .verb-pill:hover acima
   (declarado ANTES de .focused/.pressed pra empate de especificidade cair a
   favor do foco/press real). */
.btn-back:hover { color: #c3cadb; border-color: #6a7aa2; }
.btn-back.focused { color: #ffffff; border: 1dp #22D3EE; box-shadow: #22D3EE 0dp 0dp 18dp 1dp; }
.btn-back.pressed {
  decorator: vertical-gradient( #22D3EE #0EA5C9 );
  color: #071019; border: 1dp #ffffff;
  box-shadow: #ffffff 0dp 0dp 26dp 3dp;
}

/* Telas placeholder (Save/Video/Language, ONDA ARVORE (1)): so o texto "em breve" +
   Voltar - sem controle nenhum (a feature real e uma peca futura). */
.placeholder-text {
  text-align: center; color: #9AA5C0; font-size: 13dp;
  margin: 24dp 0dp 30dp 0dp;
}

/* TELA CONTROLES (M2, mock docs/design/mockups/06-controles-remap.html): painel
   MAIS LARGO (o mock usa 3 colunas Acao/Teclado/Controle - nao cabe nos 420dp
   das demais 7 telas). #sysmenu-panel.wide sobrescreve width/margin-left
   (mesma formula -metade da largura, ver o comentario de #sysmenu-panel acima).
   ALTURA (BUG-1 ao vivo, achado por probe headless Xvfb :99): as 7 outras telas
   usam auto-height (cabem sobrando) e top:90dp fixo; a tela Controles, com
   cabecalho de colunas + lista + rodape, tinha altura de conteudo ~525dp -
   ESTOURANDO o canvas de 540dp quando somada ao top:90dp herdado (bottom ~615dp,
   75dp ALEM do rodape do canvas - os botoes Restaurar/Voltar ficavam fora da
   viewport, EXATAMENTE o bug relatado ao vivo pelo lider: "botoes do rodape fora
   da tela"). FIX: `top: 20dp` (em vez do 90dp herdado - o painel largo precisa de
   mais espaco vertical, nao faz sentido comecar tao baixo) + `.ctrl-list` com
   altura REDUZIDA (270dp -> 220dp, ver abaixo) - o total passa a caber com folga
   dentro do canvas 540dp (medido empiricamente: bottom final < 500dp, > 30dp de
   folga antes do fim do canvas).

   BUG-B (moldura "totalmente errada", achado por probe headless Xvfb :99 nesta
   investigacao M2 - NAO era um dos 3 bugs originais, mas a causa raiz do
   "painel de Controles parece diferente das telas irmas"): o #sysmenu-panel
   herda `box-shadow: #22D3EE1a 0dp 0dp 50dp 0dp` (glow suave, ver a regra
   acima) - nas 7 telas normais (top:90dp, painel raso ~315dp de altura) esse
   glow cabe folgado dentro do canvas 540dp. No painel LARGO (top:20dp, altura
   ~477dp - quase o canvas inteiro, ver o paragrafo acima), o MESMO blur de
   50dp faz RmlUi pedir uma textura de sombra de ~627px de altura
   (GeometryBoxShadow::Resolve: extend = 1.5*blur_radius = 75px pra cada lado)
   contra um canvas de so 540px - RmlUi ENTAO CLAMPA a textura pro tamanho da
   janela (RenderManager::GetScissorRegion, log "desired box-shadow texture
   dimensions (770, 627) are larger than the current window region (770,
   540)") e o resultado sai CORROMPIDO (cantos hexagonais topo/base
   deformados + uma emenda vertical visivel na borda esquerda - achado
   comparando screenshot headless Controles vs Audio lado a lado, PROVADO
   fechando o box-shadow do painel largo e vendo o artefato sumir). NAO e bug
   do glintfx nem do RmlUi (comportamento documentado - a lib avisa e clampa
   em vez de crashar); e o MESMO glow generico batendo num painel geometricamente
   incompativel (alto demais perto da borda do canvas). FIX: box-shadow PROPRIO
   pro painel largo com blur REDUZIDO (10dp em vez de 50dp - extend=15px, cabe
   com folga nos 20dp de margem acima do painel) - MESMA cor/alpha (#22D3EE1a,
   10% opacidade) e MESMO offset/spread (0dp/0dp) da regra original, so o raio
   do blur muda (a 10% de opacidade o glow ja e quase imperceptivel nas telas
   normais - reduzir o raio no painel largo nao muda a leitura visual "moldura
   consistente", so elimina o clamp/corrupcao). Corners/borda/gradiente/header
   continuam 100% herdados de #sysmenu-panel, INTOCADOS - so este 1 valor
   (box-shadow) precisou de override. */
#sysmenu-panel.wide {
  width: 620dp; margin-left: -310dp; top: 20dp;
  box-shadow: #22D3EE1a 0dp 0dp 10dp 0dp;
}

/* padding-right 6dp->18dp (GLINTFX-SCROLL, corredor da scrollbar, ver o comentario
   completo em cima de `.ctrl-row` abaixo): `.ctrl-row` perdeu 12dp de largura pra
   nao colidir mais com o thumb da scrollbar - sem este ajuste EQUIVALENTE aqui, o
   cabecalho de colunas (fora de `.ctrl-list`, nao encolhe sozinho) ficaria 12dp
   mais LARGO que as linhas de baixo, desalinhando "Teclado"/"Controle" do
   cabecalho contra os keycaps das linhas (a coluna .c-act, flex:1, absorve
   qualquer diferenca de largura total entre cabecalho/linha - MESMA logica que
   fez o cabecalho ficar 4dp mais largo que a linha ANTES desta onda, ver o -4dp
   ja existente aqui vs a antiga width:558dp de baixo). +12dp de padding-right
   preserva esse MESMO gap relativo de 4dp (nao desfaz o alinhamento pre-existente,
   so acompanha o corte novo da linha). */
.ctrl-cols-head {
  display: flex; color: #9AA5C0; font-size: 10dp; letter-spacing: 2dp;
  text-transform: uppercase; padding: 0dp 18dp 6dp 6dp; border-bottom: 1dp #33281a;
}
.ctrl-cols-head .c-act { flex: 1; }
.ctrl-cols-head .c-key { width: 130dp; text-align: center; }
.ctrl-cols-head .c-pad { width: 100dp; text-align: center; }

/* Lista ROLAVEL (30 actions nao cabem sem scroll - overflow-y:auto, MESMO
   espirito do mock, mas com ALTURA FIXA em vez de flex:1 - o #sysmenu-panel
   das outras telas usa fluxo de bloco normal, nao flexbox, entao "flex:1"
   nao teria container flex pra resolver contra; altura fixa e mais simples e
   coerente com o resto do arquivo). ALTURA 220dp (era 270dp, BUG-1 ao vivo, ver
   o comentario de #sysmenu-panel.wide acima) - reduzida pra o painel inteiro
   caber no canvas 540dp com o novo `top:20dp`. */
.ctrl-list {
  height: 220dp; overflow-x: hidden; overflow-y: auto; margin-top: 4dp; padding-right: 4dp;
}
/* SCROLLBAR ESTILIZADA (M2, GLINTFX-SCROLL, glintfx v0.6.0): a v0.6.0 do
   glintfx passou a embutir defaults de UA-stylesheet pra scrollbarvertical/
   slidertrack/sliderbar/sliderarrowdec/sliderarrowinc (antes eram tamanho 0,
   invisiveis - ver CHANGELOG glintfx [0.6.0]) - sem isto a lista de 30 actions
   rolava mas SEM NENHUM indicador visual de posicao/tamanho. O default vem
   generico (cinza) - fora da paleta azul+latao do resto do painel; override
   abaixo pinta o thumb/track na MESMA identidade visual (latao/brass sobre
   fundo escuro), coerente com `.keycap.pad`/`.corner`/`.ctrl-btn`.

   PEGADINHA DE ESPECIFICIDADE (documentada no CHANGELOG glintfx [0.6.0] e em
   docs/effects.md "How-to: style scrollbars" do glintfx vendorizado): os
   defaults da UA usam seletor COMPOSTO de 2 tags (ex. `scrollbarvertical
   sliderbar`, ~20000 de especificidade) - um override de 1 tag ISOLADA
   (`sliderbar { ... }` solto, ~10000) PERDE pro default e simplesmente NAO
   aplicaria (RmlUi resolve por especificidade, nao por "autor vs. builtin").
   Por isso toda regra abaixo usa `#ctrl-list scrollbarvertical ...`
   (id + composto de 2 tags - especificidade MAIOR que o default generico,
   nunca perde). CONFIRMADO que o override PEGA de verdade por probe headless
   (Xvfb :99, screenshot antes/depois: thumb generico cinza -> thumb
   latao/brass translucido). */
#ctrl-list scrollbarvertical { width: 8dp; }
#ctrl-list scrollbarvertical slidertrack {
  background-color: #0A0E1A80; border-radius: 999dp;
}
#ctrl-list scrollbarvertical sliderbar {
  background-color: #C9A24Bb3; border-radius: 999dp; min-height: 24dp;
}
#ctrl-list scrollbarvertical sliderbar:hover { background-color: #C9A24Bff; }
/* Setas ESCONDIDAS (height:0dp): o par decremento/incremento generico da UA
   nao combina com a paleta latao/brass e e ruido visual desnecessario - a
   lista ja rola por wheel/drag-do-thumb/teclado (setas Cima/Baixo, ver
   CONTROLS_NAV_HINT), track+thumb discretos bastam. */
#ctrl-list scrollbarvertical sliderarrowdec,
#ctrl-list scrollbarvertical sliderarrowinc { height: 0dp; }
.ctrl-group {
  color: #C9A24B; font-size: 10dp; letter-spacing: 2dp; text-transform: uppercase;
  margin: 10dp 0dp 4dp 6dp; opacity: 0.8;
}
/* CAUSA RAIZ DO "MOUSE NAO SELECIONA NADA" (BUG-A, achado NOVO por probe Xvfb
   :99 nesta investigacao - nao era um dos 3 relatados originalmente, mas era a
   causa raiz de um deles): SEM `width` explicita, `.ctrl-row` (display:flex,
   filho de `.ctrl-list` que tem `overflow-y:auto`) colapsava pra ~16px de
   largura (so o padding esquerdo+direito, 6dp+6dp=12dp*dp_ratio - o CONTEUDO
   flex ficava com largura 0) em vez de ~750px (a largura real disponivel,
   confirmada medindo `.ctrl-cols-head` - MESMO display:flex, MESMA largura de
   conteudo, mas FORA do `overflow-y:auto` - la o flex funcionava normal). A
   caixa de hit-test que o mouse do loop usa (get_element_box) reportava essa
   MESMA largura colapsada - qualquer clique fora da fresta de 12dp perto da
   borda esquerda ERRAVA o hit-test (o texto da linha so aparecia visualmente
   porque o conteudo flex TRANSBORDAVA a caixa colapsada, sem overflow:hidden
   na propria .ctrl-row - por isso a linha "parecia" normal na tela mas o
   clique nao registrava em quase nenhum ponto dela). `width:100%` (percentual)
   tambem falhou (mesmo colapso) - so um comprimento ABSOLUTO (558dp, a largura
   de conteudo real de #sysmenu-panel.wide menos padding/border/gutter da
   lista) contornou o bug. `box-sizing:border-box` pareia com o padding:6dp
   (mesma receita de .verb-pill/.btn-back acima - width JA inclui padding). */
/* CORREDOR DA SCROLLBAR (GLINTFX-SCROLL, bug reportado ao vivo pelo lider: o
   thumb dourado da scrollbar SOBREPUNHA o contorno de selecao/captura da linha
   em foco): `.ctrl-list` (overflow-y:auto) reserva o `#ctrl-list scrollbarvertical
   { width: 8dp; }` (regra acima) DENTRO do proprio padding-box do container - a
   LayoutDetails::GetContainingBlock do RmlUi confirma isto subtraindo
   GetScrollbarSize(VERTICAL) do containing block usado por filhos static/relative
   (Source/Core/Layout/LayoutDetails.cpp:178-188, lido no vendorizado) - MAS essa
   subtracao so vale pra filhos com largura AUTO/percentual; `.ctrl-row` usa um
   valor ABSOLUTO fixo (558dp, ver BUG-A acima - width:100%/auto colapsava o
   flex) que NUNCA reagia a essa reserva. Nas contas: `.ctrl-list` tem
   padding-box=562dp e padding-right:4dp -> content-box=558dp (o `width:558dp`
   antigo enchia esse content-box INTEIRO); o thumb (8dp), ancorado na borda
   DIREITA do padding-box, ocupa a faixa [554dp, 562dp] - 4dp dela CAIA
   dentro do content-box (554 a 558), exatamente onde `.ctrl-row` (e o
   box-shadow inset de selecao/captura, desenhado na PROPRIA borda da linha)
   terminava - confirmado por probe headless (Xvfb :99, zoom em pixel real: o
   contorno dourado da linha 0 selecionada se FUNDIA com o thumb dourado no
   canto superior-direito, virando 1 blob so).
   FIX: `width: 546dp` (558dp - 12dp = 8dp do thumb + 4dp de folga extra, MESMA
   margem de seguranca pedida - nao so o minimo matematico) - a linha (e seu anel
   de selecao/captura) agora sempre termina 8dp ANTES do inicio do corredor do
   thumb, em QUALQUER posicao de scroll (o calculo acima nao depende de onde a
   lista esta rolada, so da largura fixa do container/scrollbar). `.ctrl-cols-head`
   (cabecalho de colunas, IRMAO de `.ctrl-list`, NAO um filho dela - nao reage a
   esse corredor sozinho) ganhou o padding-right EQUIVALENTE (+12dp, ver acima)
   pra as colunas Teclado/Controle continuarem alinhadas com os keycaps de baixo. */
.ctrl-row {
  display: flex; align-items: center; padding: 6dp 6dp; border-radius: 6dp;
  box-sizing: border-box; width: 546dp;
}
.ctrl-row .c-act { flex: 1; color: #E7ECF5; font-size: 12dp; }
.ctrl-row .c-key { width: 130dp; text-align: center; }
.ctrl-row .c-pad { width: 100dp; text-align: center; }
/* CONTORNO INSET (M2, achado por probe Xvfb :99 nesta investigacao - bug do
   contorno ciano "cortado"): `.ctrl-list` (overflow-y:auto) so declara
   `padding-right:4dp` - top/left/bottom ficam com 0dp de padding, entao o
   CLIP de overflow (RmlUi clip_area=BoxArea::Padding, ver Element.cpp) cai
   EXATAMENTE na borda de conteudo da lista nesses 3 lados (zero folga). O
   box-shadow ORIGINAL (`#22D3EE 0dp 0dp 0dp 1dp`, sem `inset`) e um anel
   EXTERNO - spread positivo empurra 1dp pra FORA do border-box da linha
   (GeometryBoxShadow::Resolve usa BoxArea::Border pro anel externo) - e
   "fora" so cabe se houver folga na direcao certa. QUALQUER linha
   selecionada/capturada e SEMPRE realinhada ao TOPO do recorte por
   `UiLayer::scroll_element_into_view(id)` (chamado incondicionalmente a cada
   reload() em system_menu_loop.cpp, `align_with_top=true` por default -
   RmlUi::ScrollAlignment::Start SEMPRE forca o alinhamento, mesmo quando a
   linha ja estava visivel - Element::GetScrollOffsetDelta, caso Start
   devolve begin_offset incondicional, NUNCA "so se precisar"). O efeito:
   toda linha .sel/.capturing acaba com o TOPO do proprio border-box colado
   no topo do recorte de `.ctrl-list` (0dp de folga) - o 1dp do anel externo
   que deveria desenhar ACIMA da linha e cortado pelo clip, sobrando so os
   3 outros lados (achado CONFIRMADO por leitura de pixel real via
   glReadPixels: linha reta do topo cai de ~736px cyan pra 6px, os outros 3
   lados continuam 100% solidos - screenshot headless comparativo
   before/after, Xvfb :99). FIX: `inset` no box-shadow - GeometryBoxShadow
   usa BoxArea::Padding e spread NEGATIVO pro anel inset (desenha 1dp PRA
   DENTRO do proprio padding-box, nunca sai da caixa da linha) - o anel fica
   IMUNE ao clip do ancestral, em QUALQUER posicao de scroll (topo, meio,
   fim), sem tocar `.ctrl-list`/scroll/glintfx. MESMA identidade visual (cor/
   opacidade), so o lado do desenho muda (dentro em vez de fora) -
   `.ctrl-row.sel` recebeu o MESMO tratamento (idêntico problema estrutural,
   so menos visivel pela cor gold/opacidade mais baixa que a captura ciano
   que o lider circulou).

   SPREAD 2dp (nao 1dp): a linha 0 (a PRIMEIRA da lista) e um caso-limite
   RESIDUAL do mesmo mecanismo - quando ELA e o alvo do align-to-top,
   `ScrollAlignment::Start` (Element.cpp) por vezes fecha com um residuo
   sub-pixel (~0.3px medido no probe, arredondamento de float acumulado do
   `.ctrl-group` acima dela) que deixa o proprio TOPO do border-box da linha
   (nao so o anel) uma fresta ACIMA do clip - `padding-top` em `.ctrl-list`
   NAO resolve isso (tentado e descartado nesta investigacao): o alvo do
   scroll usa `GetClientTop()` no calculo de `ScrollIntoView`, que segue a
   convencao DOM (largura da BORDA, nao do padding) - qualquer padding-top
   so desloca o conteudo em repouso, e o align-to-top persegue o MESMO ponto
   de recorte de qualquer forma, "engolindo" o padding via scroll (medido:
   list.h cresceu com o padding, mas controls-item-0.y pos-scroll ficou
   IDENTICO). Como o anel INSET desenha relativo a PROPRIA borda da linha
   (imune ao jogo de scroll), aumentar o spread pra 2dp da ~1dp/1.3px REAL de
   folga a mais entre a borda da linha e o traco do anel - >4x o residuo de
   0.3px medido, cobre a linha 0 (e qualquer residuo futuro semelhante) sem
   depender de padding do container. Confirmado por probe (linhas 0/1/5/15/
   25/29 - PRIMEIRA e ULTIMA incluidas - todas com os 4 lados fechados). */
.ctrl-row.sel {
  box-shadow: #C9A24B66 0dp 0dp 0dp 2dp inset; decorator: vertical-gradient( #C9A24B24 #C9A24B0a );
}
.ctrl-row.capturing {
  box-shadow: #22D3EE 0dp 0dp 0dp 2dp inset; decorator: vertical-gradient( #22D3EE1a #22D3EE0a );
}
.ctrl-row.capturing .ctrl-capture-text { color: #22D3EE; font-size: 11dp; }
.ctrl-conflict { color: #F43F5E; font-size: 10dp; margin-top: 2dp; }

.keycap {
  display: inline-block; min-width: 40dp; padding: 3dp 8dp; border-radius: 5dp;
  background-color: #0d1320; border: 1dp #33405e; color: #E7ECF5; font-size: 11dp;
}
.keycap.pad { border: 1dp #3a2e18; background-color: #1a140b; color: #F0D98C; }
.keycap.none { color: #6B6F7A; border: 1dp #3a4256; }

/* RODAPE (M2 STAGED CHANGES, reforma de UX aprovada pelo lider): passou de 2
   pra 3 botoes - Restaurar padrao (ESQUERDA, sozinho) + Aplicar/Voltar
   (DIREITA, agrupados) - "aplica na hora" foi trocado por mudancas preparadas
   + Aplicar explicito (ver o comentario STAGED CHANGES em system_menu.hpp).
   .ctrl-foot continua space-between (Restaurar de um lado, o GRUPO
   Aplicar+Voltar do outro); .ctrl-foot-right e um flex ANINHADO (gap nativo do
   RmlUi 6.3 - shorthand "row-gap, column-gap", ver StyleSheetSpecification.cpp)
   so pra manter Aplicar e Voltar coladinhos, sem competir com o
   space-between externo. */
.ctrl-foot {
  display: flex; justify-content: space-between; align-items: center;
  margin-top: 10dp; padding-top: 10dp; border-top: 1dp #33281a;
}
.ctrl-foot-right { display: flex; gap: 8dp; align-items: center; }
.ctrl-btn {
  font-size: 11dp; letter-spacing: 1dp; padding: 6dp 14dp; border-radius: 8dp;
  border: 1dp #7A5A2E; decorator: vertical-gradient( #2a2113 #1a140b ); color: #F0D98C;
}
.ctrl-btn.ghost { color: #9AA5C0; border: 1dp #3a4256; }
/* "Aplicar" (M2 STAGED CHANGES): cor cyan (mesma familia do resto da UI de
   confirmacao/foco) pra se destacar como a acao PRIMARIA do rodape - e o
   UNICO botao que de fato persiste em disco/vale no jogo. */
.ctrl-btn.apply { color: #22D3EE; border: 1dp #0e6a7d; }
/* Sinal visual (OPCIONAL, ver o pedido) de "nada a aplicar ainda" -
   esmaece o botao quando !controls_dirty; continua clicavel (no-op seguro,
   persistir a MESMA config que ja esta em disco nao faz mal nenhum). */
.ctrl-btn.apply.disabled { opacity: 0.45; }
.ctrl-btn.focused { border: 1dp #22D3EE; box-shadow: #22D3EE 0dp 0dp 14dp 1dp; color: #ffffff; }
.ctrl-btn.pressed {
  decorator: vertical-gradient( #22D3EE #0EA5C9 ); color: #071019; border: 1dp #ffffff;
  box-shadow: #ffffff 0dp 0dp 22dp 3dp;
}

/* Mini-dialogo de confirmacao do Restaurar padrao (decisao 4 do lider: pede
   confirmacao) E do descarte de alteracoes nao aplicadas (M2 STAGED CHANGES,
   MESMA mecanica/linguagem visual). Substitui a lista por um prompt + 2 pills
   (reusa .verb-pill/.focused do resto do arquivo). */
.ctrl-confirm-title {
  text-align: center; color: #E7ECF5; font-size: 13dp; line-height: 20dp;
  margin: 20dp 0dp 20dp 0dp;
}
)RCSS";

// Corpo dos 4 pontos de latao (idempotente entre as telas).
constexpr const char* kCorners =
    "<div class=\"corner tl\"></div><div class=\"corner tr\"></div>"
    "<div class=\"corner bl\"></div><div class=\"corner br\"></div>";

// Preambulo comum (scrim + abertura do painel + kicker/titulo/divisor) - as 8
// telas repetem esta moldura, so o CONTEUDO abaixo do divisor muda.
// `extra_panel_class` (default vazio): classe extra no #sysmenu-panel - so a
// tela Controles usa (".wide", ver kSharedStyle) pra caber as 3 colunas do
// mock (Acao/Teclado/Controle) sem espremer o texto.
void append_panel_open(std::ostringstream& body, const gus::app::i18n::Translator& tr,
                        const char* title_key, const char* extra_panel_class = "") {
    body << "<div id=\"sysmenu-scrim\"></div>";
    body << "<div id=\"sysmenu-panel\" class=\"" << extra_panel_class << "\">" << kCorners;
    body << "<div class=\"kicker\">" << tr.tr("MENU_SYSTEM_KICKER") << "</div>";
    body << "<div class=\"title\">" << tr.tr(title_key) << "</div>";
    body << "<div class=\"divider\"></div>";
}

std::string build_pause_body(const SystemMenuState& state,
                              const gus::app::i18n::Translator& tr, int pressed_index) {
    std::ostringstream body;
    append_panel_open(body, tr, "MENU_PAUSE_TITLE");

    struct Item {
        int index;
        const char* key;
        const char* extra_class;
    };
    // Ordem SAVE-LOAD-UI etapa 6: Continuar / Salvar / Carregar / Configuracoes /
    // Sair (MENU_LOAD_GAME ja existia no catalogo, ate agora so consumida pela
    // tela de titulo legada - reusada aqui sem duplicar chave).
    const Item items[kPauseItemCount] = {
        {0, "MENU_CONTINUE", ""},
        {1, "MENU_SAVE_GAME", ""},
        {2, "MENU_LOAD_GAME", ""},
        {3, "SETTINGS_TITLE", ""},
        {4, "MENU_QUIT", " danger"},
    };
    for (const Item& item : items) {
        const bool focused = (state.pause_selected == item.index);
        body << "<div class=\"verb-pill" << item.extra_class
             << (focused ? " focused" : "") << pressed_class(item.index, pressed_index)
             << "\" id=\"pause-item-" << item.index << "\">" << tr.tr(item.key)
             << "</div>";
    }

    // MENU_PAUSE_HINT tem 2 placeholders posicionais ("{0} confirma, {1} volta ao
    // jogo" no catalogo pt_br) - o Translator::tr(key) so resolve KEY->string, NAO
    // interpola (mesma limitacao de COMBAT_DEFEAT_BARK). Mesmo padrao manual
    // find/replace usado em battle_scene.cpp:2071 pro bark de derrota (AUD-M7-COSTURA
    // ACH-2: sem isso o jogador via os tokens "{0}/{1}" crus no rodape).
    std::string hint = tr.tr("MENU_PAUSE_HINT");
    if (const auto pos0 = hint.find("{0}"); pos0 != std::string::npos) {
        hint.replace(pos0, 3, "Enter");
    }
    if (const auto pos1 = hint.find("{1}"); pos1 != std::string::npos) {
        hint.replace(pos1, 3, "Esc");
    }
    body << "<div class=\"footer-hint\">" << hint << "</div>";
    body << "</div>";  // #sysmenu-panel
    return body.str();
}

std::string build_config_categories_body(const SystemMenuState& state,
                                          const gus::app::i18n::Translator& tr,
                                          int pressed_index) {
    std::ostringstream body;
    append_panel_open(body, tr, "SETTINGS_TITLE");

    struct Item {
        int index;
        const char* key;
        bool is_back;
    };
    // Ordem da arvore aprovada: Audio / Video / Controles / Lingua / Voltar
    // (Controles inserido entre Video e Lingua, M2 - ver ConfigCategoryItem no
    // header pro racional de posicionamento).
    const Item items[kConfigCategoriesItemCount] = {
        {0, "SETTINGS_AUDIO", false},
        {1, "SETTINGS_VIDEO", false},
        {2, "SETTINGS_CONTROLS", false},
        {3, "SETTINGS_LANGUAGE", false},
        {4, "SETTINGS_BACK", true},
    };
    for (const Item& item : items) {
        const bool focused = (state.config_categories_selected == item.index);
        const char* base_class = item.is_back ? "btn-back" : "verb-pill";
        body << "<div class=\"" << base_class << (focused ? " focused" : "")
             << pressed_class(item.index, pressed_index) << "\" id=\"category-item-"
             << item.index << "\">" << tr.tr(item.key) << "</div>";
    }

    body << "</div>";  // #sysmenu-panel
    return body.str();
}

std::string build_audio_body(const SystemMenuState& state,
                              const gus::app::i18n::Translator& tr, int pressed_index) {
    std::ostringstream body;
    append_panel_open(body, tr, "SETTINGS_AUDIO");

    struct Row {
        int index;
        const char* name_key;
        float volume;
    };
    const Row rows[2] = {
        {0, "SETTINGS_MUSIC_VOLUME", state.music_volume},
        {1, "SETTINGS_SFX_VOLUME", state.sfx_volume},
    };
    for (const Row& row : rows) {
        const bool focused = (state.audio_selected == row.index);
        const std::string pct = pct_string(row.volume);
        // id="audio-item-<indice>" (clique de mouse no NOME/rotulo do slider): o
        // loop faz hit-test nisto SO SE o clique nao caiu no track
        // (slider-track-<indice>, ja tratado a parte) - clicar no rotulo FOCA o
        // item (system_menu_click_option), nao ajusta volume.
        // .field-row envolve SO nome+valor (ver comentario do .field-row no
        // <style>): o .track do slider fica DE FORA dele, como IRMAO seguinte
        // dentro do MESMO .field - preserva a pilha vertical "nome+valor" em
        // cima / "barra" embaixo.
        body << "<div class=\"field\" id=\"audio-item-" << row.index << "\">"
             << "<div class=\"field-row\">"
             << "<span class=\"name\">" << tr.tr(row.name_key) << "</span>"
             << "<span class=\"val\">" << pct << "</span>"
             << "</div>"
             << "<div class=\"track\" id=\"slider-track-" << row.index << "\">"
             << "<div class=\"fill\" style=\"width:" << pct << ";\"></div>"
             << "<div class=\"slider-node" << (focused ? " focused" : "")
             << "\" id=\"slider-node-" << row.index << "\" style=\"left:" << pct
             << ";\">"
             << "<div class=\"slider-node-hex-inner\"></div>"
             << "</div>"
             << "</div></div>";
    }

    const int back_index = static_cast<int>(AudioItem::Back);
    const bool back_focused = (state.audio_selected == back_index);
    body << "<div class=\"btn-back" << (back_focused ? " focused" : "")
         << pressed_class(back_index, pressed_index) << "\" id=\"audio-item-"
         << back_index << "\">" << tr.tr("SETTINGS_BACK") << "</div>";

    body << "</div>";  // #sysmenu-panel
    return body.str();
}

// Corpo compartilhado das 3 telas placeholder (Save/Video/Language, ONDA ARVORE
// (1)): titulo proprio (`title_key`) + texto "em breve" + Voltar (unico item
// navegavel, sempre focado - kPlaceholderBackIndex). `back_id` fixo
// ("placeholder-back") porque so 1 placeholder fica carregado por vez (nunca 2
// no mesmo documento) - system_menu_loop.cpp so precisa de 1 id pras 3 telas.
std::string build_placeholder_body(const gus::app::i18n::Translator& tr,
                                    const char* title_key, int pressed_index) {
    std::ostringstream body;
    append_panel_open(body, tr, title_key);

    body << "<div class=\"placeholder-text\">" << tr.tr("MENU_PLACEHOLDER_TEXT")
         << "</div>";
    body << "<div class=\"btn-back focused"
         << pressed_class(kPlaceholderBackIndex, pressed_index) << "\" id=\"placeholder-back\">"
         << tr.tr("SETTINGS_BACK") << "</div>";

    body << "</div>";  // #sysmenu-panel
    return body.str();
}

// Rotulo legivel do PRIMEIRO key binding de `action_name` dentro de `config`
// (string vazia se a action nao existe ali ou nao tem nenhuma key - "sem
// tecla", renderizado como CONTROLS_NO_BINDING pelo chamador). Reusa a tabela
// PURA de controls_diff.hpp (human_label_for_keycode) - duplicar aqui a busca
// linear por nome (em vez de expor a funcao interna do domain) e barato o
// bastante (30 actions, 1x por render).
std::string keyboard_label_for(const gus::domain::input::InputRemapConfig& config,
                                std::string_view action_name) {
    for (const auto& a : config.actions) {
        if (a.action_name == action_name) {
            if (a.keys.empty()) return std::string();
            return gus::domain::input::human_label_for_keycode(a.keys.front().keycode);
        }
    }
    return std::string();
}

// Corpo da tela Controles (M2 -> M2 STAGED CHANGES, mock docs/design/mockups/
// 06-controles-remap.html): painel LARGO (".wide") com 3 colunas (Acao/
// Teclado/Controle) e as 30 actions curadas/agrupadas
// (controls_action_name_at/controls_group_at, ver system_menu.hpp) + rodape
// (Restaurar padrao / Aplicar - NOVO / Voltar). A coluna "Controle" (gamepad)
// SEMPRE mostra CONTROLS_NO_BINDING nesta onda: decisao 2 do lider ("so
// teclado agora, controle fica read-only") + default_controls() nao popula
// NENHUM gamepad_buttons ainda (nao ha o que exibir de verdade; inventar um
// glifo seria mentir sobre o dado). Enquanto state.controls_confirming_restore
// (decisao 4 do lider: pede confirmacao antes de restaurar) OU
// state.controls_confirming_discard (reforma STAGED CHANGES: pede
// confirmacao antes de descartar mudancas nao aplicadas ao Voltar/Esc), a
// lista inteira e SUBSTITUIDA por um mini-dialogo "tem certeza?" com 2 pills
// (Sim/Nao) - mais simples que sobrepor um modal por cima da lista, mesma
// linguagem visual (.verb-pill/.btn-back) ja usada no resto do arquivo.
std::string build_controls_body(const SystemMenuState& state,
                                 const gus::app::i18n::Translator& tr, int pressed_index) {
    std::ostringstream body;
    append_panel_open(body, tr, "SETTINGS_CONTROLS", "wide");

    if (state.controls_confirming_restore) {
        body << "<div class=\"ctrl-confirm-title\">"
             << tr.tr("CONTROLS_RESTORE_CONFIRM_TITLE") << "</div>";
        const bool yes_focused = (state.controls_restore_confirm_selected == 0);
        const bool no_focused = (state.controls_restore_confirm_selected == 1);
        body << "<div class=\"verb-pill" << (yes_focused ? " focused" : "")
             << pressed_class(0, pressed_index) << "\" id=\"controls-confirm-0\">"
             << tr.tr("CONTROLS_RESTORE_CONFIRM_YES") << "</div>";
        body << "<div class=\"btn-back" << (no_focused ? " focused" : "")
             << pressed_class(1, pressed_index) << "\" id=\"controls-confirm-1\">"
             << tr.tr("CONTROLS_RESTORE_CONFIRM_NO") << "</div>";
        body << "</div>";  // #sysmenu-panel
        return body.str();
    }

    // M2 STAGED CHANGES: mini-dialogo "descartar alteracoes nao aplicadas?"
    // (Voltar/Esc com controls_dirty=true) - MESMA estrutura visual do
    // restaurar-padrao acima, ids PROPRIOS ("controls-discard-confirm-<0|1>")
    // pra nao colidir (embora mutuamente exclusivos - so 1 dialogo por vez).
    if (state.controls_confirming_discard) {
        body << "<div class=\"ctrl-confirm-title\">"
             << tr.tr("CONTROLS_DISCARD_CONFIRM_TITLE") << "</div>";
        const bool yes_focused = (state.controls_discard_confirm_selected == 0);
        const bool no_focused = (state.controls_discard_confirm_selected == 1);
        body << "<div class=\"verb-pill" << (yes_focused ? " focused" : "")
             << pressed_class(0, pressed_index) << "\" id=\"controls-discard-confirm-0\">"
             << tr.tr("CONTROLS_DISCARD_CONFIRM_YES") << "</div>";
        body << "<div class=\"btn-back" << (no_focused ? " focused" : "")
             << pressed_class(1, pressed_index) << "\" id=\"controls-discard-confirm-1\">"
             << tr.tr("CONTROLS_DISCARD_CONFIRM_NO") << "</div>";
        body << "</div>";  // #sysmenu-panel
        return body.str();
    }

    body << "<div class=\"ctrl-cols-head\" id=\"ctrl-cols-head\">"
         << "<span class=\"c-act\">" << tr.tr("CONTROLS_COL_ACTION") << "</span>"
         << "<span class=\"c-key\">" << tr.tr("CONTROLS_COL_KEYBOARD") << "</span>"
         << "<span class=\"c-pad\">" << tr.tr("CONTROLS_COL_GAMEPAD") << "</span>"
         << "</div>";

    body << "<div class=\"ctrl-list\" id=\"ctrl-list\">";
    static constexpr const char* kGroupKeys[4] = {
        "CONTROLS_GROUP_MOVEMENT",
        "CONTROLS_GROUP_WORLD",
        "CONTROLS_GROUP_COMBAT",
        "CONTROLS_GROUP_MENU_DIALOGUE",
    };
    int last_group = -1;
    for (int i = 0; i < kControlsActionCount; ++i) {
        const int group = controls_group_at(i);
        if (group != last_group && group >= 0 && group < 4) {
            body << "<div class=\"ctrl-group\">" << tr.tr(kGroupKeys[group]) << "</div>";
            last_group = group;
        }

        const std::string_view action_name = controls_action_name_at(i);
        const gus::domain::input::ActionDefinition* def =
            gus::domain::input::ActionRegistry::get_by_name(action_name);
        const std::string label =
            (def != nullptr) ? tr.tr(def->label_i18n_key) : std::string(action_name);

        const bool selected = (state.controls_selected == i);
        const bool capturing_here = selected && state.controls_capturing;
        const std::string key_label = keyboard_label_for(state.controls_config, action_name);

        std::string row_class = "ctrl-row";
        if (capturing_here) {
            row_class += " capturing";
        } else if (selected) {
            row_class += " sel";
        }

        body << "<div class=\"" << row_class << "\" id=\"controls-item-" << i << "\">";
        body << "<span class=\"c-act\">" << label;
        if (selected && state.controls_last_action_swapped) {
            // Aviso de troca (decisao 1 do lider): CONTROLS_SWAP_NOTICE tem 1
            // placeholder posicional "{0}" (mesmo padrao manual de MENU_PAUSE_HINT
            // acima) - substitui pelo rotulo traduzido da OUTRA action.
            std::string notice = tr.tr("CONTROLS_SWAP_NOTICE");
            const std::string other = tr.tr(state.controls_last_swapped_with_label_key);
            if (const auto pos = notice.find("{0}"); pos != std::string::npos) {
                notice.replace(pos, 3, other);
            }
            body << "<span class=\"ctrl-conflict\">" << notice << "</span>";
        }
        body << "</span>";

        body << "<span class=\"c-key\">";
        if (capturing_here) {
            body << "<span class=\"ctrl-capture-text\">" << tr.tr("CONTROLS_CAPTURE_PROMPT")
                 << "</span>";
        } else if (key_label.empty()) {
            body << "<span class=\"keycap none\">" << tr.tr("CONTROLS_NO_BINDING") << "</span>";
        } else {
            body << "<span class=\"keycap\">" << key_label << "</span>";
        }
        body << "</span>";

        // Controle (gamepad): SEMPRE "sem binding" nesta onda (ver comentario da
        // funcao acima - decisao 2 do lider + default_controls() nao popula
        // gamepad ainda).
        body << "<span class=\"c-pad\"><span class=\"keycap none\">"
             << tr.tr("CONTROLS_NO_BINDING") << "</span></span>";

        body << "</div>";  // .ctrl-row
    }
    body << "</div>";  // .ctrl-list

    // RODAPE (M2 STAGED CHANGES): Restaurar padrao sozinho a ESQUERDA;
    // Aplicar+Voltar agrupados a DIREITA (.ctrl-foot-right, ver <style>) -
    // layout aprovado pelo lider pro novo botao "Aplicar".
    body << "<div class=\"ctrl-foot\">";
    const bool restore_focused = (state.controls_selected == kControlsRestoreIndex);
    const bool apply_focused = (state.controls_selected == kControlsApplyIndex);
    const bool back_focused = (state.controls_selected == kControlsBackIndex);
    body << "<div class=\"ctrl-btn ghost" << (restore_focused ? " focused" : "")
         << pressed_class(kControlsRestoreIndex, pressed_index) << "\" id=\"controls-item-"
         << kControlsRestoreIndex << "\">" << tr.tr("SETTINGS_RESET_DEFAULTS") << "</div>";
    body << "<div class=\"ctrl-foot-right\">";
    // "Aplicar" esmaecido (sinal visual OPCIONAL) quando nao ha mudanca staged
    // pendente (!controls_dirty) - continua clicavel (no-op seguro).
    body << "<div class=\"ctrl-btn apply" << (state.controls_dirty ? "" : " disabled")
         << (apply_focused ? " focused" : "") << pressed_class(kControlsApplyIndex, pressed_index)
         << "\" id=\"controls-item-" << kControlsApplyIndex << "\">" << tr.tr("SETTINGS_APPLY")
         << "</div>";
    body << "<div class=\"ctrl-btn" << (back_focused ? " focused" : "")
         << pressed_class(kControlsBackIndex, pressed_index) << "\" id=\"controls-item-"
         << kControlsBackIndex << "\">" << tr.tr("SETTINGS_BACK") << "</div>";
    body << "</div>";  // .ctrl-foot-right
    body << "</div>";  // .ctrl-foot

    body << "<div class=\"footer-hint\">" << tr.tr("CONTROLS_NAV_HINT") << "</div>";

    body << "</div>";  // #sysmenu-panel
    return body.str();
}

}  // namespace

std::string build_system_menu_rml(const SystemMenuState& state,
                                   const gus::app::i18n::Translator& translator,
                                   int pressed_index) {
    std::string body;
    switch (state.screen) {
        case SystemMenuScreen::Pause:
            body = build_pause_body(state, translator, pressed_index);
            break;
        case SystemMenuScreen::ConfigCategories:
            body = build_config_categories_body(state, translator, pressed_index);
            break;
        case SystemMenuScreen::Audio:
            body = build_audio_body(state, translator, pressed_index);
            break;
        case SystemMenuScreen::Controls:
            body = build_controls_body(state, translator, pressed_index);
            break;
        case SystemMenuScreen::Video:
            body = build_placeholder_body(translator, "SETTINGS_VIDEO", pressed_index);
            break;
        case SystemMenuScreen::Language:
            body = build_placeholder_body(translator, "SETTINGS_LANGUAGE", pressed_index);
            break;
        case SystemMenuScreen::Hidden:
            break;  // documento minimo, sem paineis
    }

    std::string rml;
    rml += "<rml>\n<head>\n<style>\n";
    rml += kSharedStyle;
    rml += "\n</style>\n</head>\n<body>\n";
    rml += body;
    rml += "\n</body>\n</rml>\n";
    return rml;
}

}  // namespace gus::app::screens
