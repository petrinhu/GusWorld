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
//     de cada quina mas sem cruzar a moldura. AINDA SOLIDO (sem volume/gradiente -
//     ver o TODO no proprio bloco .corner abaixo: a API polygon() do glintfx so
//     pinta 1 cor solida hoje; gradiente/volume no hexagono e um pedido em aberto
//     ao dev da lib, NAO resolvido aqui).
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

#include "gus/app/screens/system_menu_rml.hpp"

#include <cmath>
#include <sstream>
#include <string>

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

/* CANTOS HEXAGONAIS: decorator: polygon(6, <cor>) do glintfx v0.3.0 (verificado
   empiricamente, probe Xvfb :99, glReadPixels confirmou geometria hexagonal real -
   ver header deste arquivo). AINDA SOLIDO/PROVISORIO: a API so pinta 1 cor (sem
   gradiente nem stroke separado) - o volume/gradiente do mock (radial, latao com
   luz) fica de fora nesta versao.
   TODO(glintfx): hexagono solido provisorio; trocar por
   polygon(6, radial-gradient(...)) (ou API equivalente de gradiente dentro de
   polygon()) quando o glintfx >v0.3.0 ganhar essa feature (pedido feito ao dev da
   lib - NAO mexer na lib por aqui, so consumir o pin quando a tag sair). */
.corner {
  position: absolute; width: 20dp; height: 20dp;
  decorator: polygon( 6, #C9A24B );
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
  position: relative; width: 340dp; padding: 7dp 16dp; margin-bottom: 6dp;
  text-align: center;
  decorator: vertical-gradient( #3A4566 #1B2238 );
  border: 1dp #ffffff12; border-radius: 999dp;
  font-size: 13dp; color: #E7ECF5; letter-spacing: 2dp;
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
  text-align: center; width: 340dp; padding: 7dp 16dp; margin-top: 6dp;
  border: 1dp #3A4566; border-radius: 999dp; color: #9AA5C0;
  font-size: 12dp; letter-spacing: 2dp;
}
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
)RCSS";

// Corpo dos 4 pontos de latao (idempotente entre as telas).
constexpr const char* kCorners =
    "<div class=\"corner tl\"></div><div class=\"corner tr\"></div>"
    "<div class=\"corner bl\"></div><div class=\"corner br\"></div>";

// Preambulo comum (scrim + abertura do painel + kicker/titulo/divisor) - as 7
// telas repetem esta moldura, so o CONTEUDO abaixo do divisor muda.
void append_panel_open(std::ostringstream& body, const gus::app::i18n::Translator& tr,
                        const char* title_key) {
    body << "<div id=\"sysmenu-scrim\"></div>";
    body << "<div id=\"sysmenu-panel\">" << kCorners;
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
    // Ordem da arvore aprovada: Continuar / Salvar / Configuracoes / Sair.
    const Item items[kPauseItemCount] = {
        {0, "MENU_CONTINUE", ""},
        {1, "MENU_SAVE_GAME", ""},
        {2, "SETTINGS_TITLE", ""},
        {3, "MENU_QUIT", " danger"},
    };
    for (const Item& item : items) {
        const bool focused = (state.pause_selected == item.index);
        body << "<div class=\"verb-pill" << item.extra_class
             << (focused ? " focused" : "") << pressed_class(item.index, pressed_index)
             << "\" id=\"pause-item-" << item.index << "\">" << tr.tr(item.key)
             << "</div>";
    }

    body << "<div class=\"footer-hint\">" << tr.tr("MENU_PAUSE_HINT") << "</div>";
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
    // Ordem da arvore aprovada: Audio / Video / Lingua / Voltar.
    const Item items[kConfigCategoriesItemCount] = {
        {0, "SETTINGS_AUDIO", false},
        {1, "SETTINGS_VIDEO", false},
        {2, "SETTINGS_LANGUAGE", false},
        {3, "SETTINGS_BACK", true},
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
        case SystemMenuScreen::Save:
            body = build_placeholder_body(translator, "MENU_SAVE_GAME", pressed_index);
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
