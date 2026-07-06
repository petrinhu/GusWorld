// gus/app/src/screens/npc_dialogue_rml.cpp
//
// Implementacao de npc_dialogue_portrait_file/build_npc_dialogue_rml. Ver header.
// PURA (sem SDL/GL/I/O) - testada headless em npc_dialogue_rml_test.cpp, MESMO
// racional/estilo de system_menu_rml_test.cpp ("SO checagem ESTRUTURAL da string
// RML gerada... NAO valida pixel/renderizacao real").

#include "gus/app/screens/npc_dialogue_rml.hpp"

#include <sstream>

#include "gus/app/screens/npc_dialogue_overlay.hpp"  // npc_dialogue_actor_display_name

namespace gus::app::screens {

namespace {

// RCSS traduzido do mock aprovado (docs/design/mockups/05-dialogo-bertoldo-retrato-
// real.html + 03-.../04-...): MESMAS variaveis --warm-* / classes .warm-box/
// .warm-corner/.warm-portrait/.warm-name/.warm-line, so trocando `background:` por
// `decorator:`/`decorator: radial-gradient(...)` (nativo do glintfx, ver
// DecoratorGradient.cpp - "radial-gradient" e um decorator de 1a classe, nao so um
// fill de polygon()) e `left:50%;transform:translateX(-50%)` pelo MESMO truque de
// margin-left NEGATIVO (metade da largura) ja usado em #sysmenu-panel
// (system_menu_rml.cpp) - evita depender de `transform` (nao usado em nenhum outro
// RCSS deste projeto ate hoje). O body{width:100%;height:100%} vem de CAUSA RAIZ
// ja documentada em system_menu_rml.cpp (sem isto, left:50% resolve contra um body
// de tamanho 0x0 - bug ja fechado la, prevenido aqui de saida).
constexpr const char* kNpcDialogueStyle = R"RCSS(
body { font-family: "Pixel Operator Mono"; background: transparent; width: 100%; height: 100%; }

#npcdlg-scrim {
  position: absolute; top: 0dp; left: 0dp; right: 0dp; bottom: 0dp;
  background-color: #05070c66;
}

/* width:820dp -> margin-left:-410dp (metade), MESMO truque de #sysmenu-panel
   (system_menu_rml.cpp) pra centralizar sem depender de `transform`. */
#npcdlg-box {
  box-sizing: border-box;
  position: absolute; left: 50%; bottom: 36dp; margin-left: -410dp;
  width: 820dp;
  decorator: linear-gradient( 180deg, #241812 0%, #170F0B 100% );
  border: 1dp #7A5A2E; border-radius: 14dp;
  padding: 20dp 26dp;
  box-shadow: #000000ff 0dp 0dp 0dp 1dp, #E9A33D1a 0dp 0dp 46dp 0dp;
}

/* CANTOS DE LATAO: circulos (border-radius:999dp - MESMO truque de "totalmente
   redondo" ja usado em .verb-pill/.slider-node deste projeto, evita depender de
   border-radius percentual) com radial-gradient nativo, straddling a borda
   (offsets NEGATIVOS) - EXATAMENTE como o mock aprovado desenha (o "parafuso"
   meio-dentro/meio-fora da moldura). */
.warm-corner {
  position: absolute; width: 16dp; height: 16dp; border-radius: 999dp;
  decorator: radial-gradient( circle at 35% 30%, #F0D98C, #C9A24B 55%, #7A5A2E 100% );
  box-shadow: #00000080 0dp 1dp 3dp 0dp;
}
.warm-corner.tl { top: -8dp; left: -8dp; }
.warm-corner.tr { top: -8dp; right: -8dp; }
.warm-corner.bl { bottom: -8dp; left: -8dp; }
.warm-corner.br { bottom: -8dp; right: -8dp; }

/* RETRATO: decorator: image( <flat> cover ) - MESMA tecnica ja usada por #pic no
   cockpit da batalha (battle_preview.cpp), evita depender de <img>/object-fit
   (nao usado em nenhum outro RCSS deste projeto). */
#npcdlg-portrait {
  position: absolute; top: 20dp; left: 26dp; width: 96dp; height: 96dp;
  border: 3dp #C9A24B; border-radius: 12dp;
  background-color: #000000;
  box-shadow: #E9A33D59 0dp 0dp 22dp 0dp;
}

#npcdlg-body { position: relative; margin-left: 116dp; min-height: 96dp; }
#npcdlg-name {
  font-size: 13dp; letter-spacing: 2dp; color: #E8A33D; margin-bottom: 8dp;
}
#npcdlg-line { font-size: 19dp; line-height: 1.5; color: #F3E9DD; }

.warm-choices { margin-top: 14dp; }
.warm-choice {
  font-size: 15dp; line-height: 1.7; color: #B9A392;
}
.warm-choice.selected { color: #F3E9DD; }

.warm-continue-hint {
  margin-top: 10dp; font-size: 11dp; color: #B9A392; letter-spacing: .5dp;
}
)RCSS";

}  // namespace

std::string npc_dialogue_portrait_file(const std::string& speaker_id) {
    // EXCECOES CADASTRADAS (ver header): o arquivo real em disco nao bate 1:1 com
    // o speaker_id. Ponto UNICO - quando um novo NPC precisar de excecao,
    // acrescenta 1 linha aqui, documentada.
    if (speaker_id == "bertoldo") {
        return "retrato_seu_bertoldo_caim.png";
    }
    // DEFAULT generico: "retrato_<speaker_id>.png" (bate com ACTOR_GUS/retrato_gus.png,
    // ACTOR_CAUA/retrato_caua.png, ACTOR_JACI/retrato_jaci.png etc. sem excecao).
    return "retrato_" + speaker_id + ".png";
}

std::string build_npc_dialogue_rml(
    const gus::domain::dialogue::DialogueNode& node,
    const gus::app::i18n::Translator& translator, int selected_option,
    const std::string& portrait_file) {
    const std::string speaker_label =
        npc_dialogue_actor_display_name(node.speaker_id, translator);

    std::ostringstream body;
    body << "<div id=\"npcdlg-scrim\"></div>";
    body << "<div id=\"npcdlg-box\">";
    body << "<div class=\"warm-corner tl\"></div><div class=\"warm-corner tr\">"
            "</div><div class=\"warm-corner bl\"></div><div class=\"warm-corner br\">"
            "</div>";
    body << "<div id=\"npcdlg-portrait\" style=\"decorator: image( " << portrait_file
         << " cover );\"></div>";
    body << "<div id=\"npcdlg-body\">";
    body << "<div id=\"npcdlg-name\">" << speaker_label << "</div>";
    body << "<div id=\"npcdlg-line\">" << translator.tr(node.text_key) << "</div>";

    if (node.options.empty()) {
        body << "<div class=\"warm-continue-hint\">" << translator.tr("DIALOGUE_CONTINUE")
             << "</div>";
    } else {
        body << "<div class=\"warm-choices\">";
        for (std::size_t i = 0; i < node.options.size(); ++i) {
            const bool is_selected = static_cast<int>(i) == selected_option;
            body << "<div class=\"warm-choice" << (is_selected ? " selected" : "")
                 << "\">" << (is_selected ? "&gt; " : "&nbsp;&nbsp;")
                 << translator.tr(node.options[i].label_key) << "</div>";
        }
        body << "</div>";  // .warm-choices
    }

    body << "</div>";  // #npcdlg-body
    body << "</div>";  // #npcdlg-box

    std::string rml;
    rml += "<rml>\n<head>\n<style>\n";
    rml += kNpcDialogueStyle;
    rml += "\n</style>\n</head>\n<body>\n";
    rml += body.str();
    rml += "\n</body>\n</rml>\n";
    return rml;
}

}  // namespace gus::app::screens
