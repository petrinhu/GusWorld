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
// Traducao do mock pro RCSS do glintfx:
//   .panel (gradiente vertical 3-stop + borda latao + box-shadow glow cyan sutil)
//     -> decorator: linear-gradient(180deg, ...) (native RmlUi, MESMA familia de
//        shader do radial-gradient ja shipado no cockpit - ver DecoratorGradient.cpp)
//   .corner (moldura latao, radial-gradient off-center + box-shadow)
//     -> decorator: radial-gradient(circle at 35% 30%, ...) (native, ja usado em
//        outra forma - circle closest-side - no cockpit; aqui so com "at X% Y%")
//   .verb-pill / .verb-pill.focused (glow neon do selecionado)
//     -> MESMA receita de .verb/.verb.sel do cockpit (border + box-shadow spread
//        positivo + decorator: vertical-gradient)
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

// Header RCSS comum as duas telas (paleta canonica + moldura latao + pills).
// dp = px logico do mock (canvas 960x540, mesma convencao do cockpit).
constexpr const char* kSharedStyle = R"RCSS(
body { font-family: "Pixel Operator Mono"; background: transparent; }

#sysmenu-scrim {
  position: absolute; top: 0dp; left: 0dp; right: 0dp; bottom: 0dp;
  background-color: #05070ca8;
}

#sysmenu-panel {
  position: absolute; top: 90dp; left: 50%; margin-left: -210dp;
  width: 420dp;
  decorator: linear-gradient( 180deg, #3A4566 0%, #1B2238 42%, #0A0E1A 100% );
  border: 1dp #7A5A2E;
  border-radius: 20dp;
  padding: 30dp 28dp 24dp 28dp;
  box-shadow: #22D3EE1a 0dp 0dp 50dp 0dp;
}

.corner {
  position: absolute; width: 20dp; height: 20dp; border-radius: 10dp;
  decorator: radial-gradient( circle at 35% 30%, #F0D98C, #C9A24B 55%, #7A5A2E 100% );
  box-shadow: #00000080 0dp 1dp 3dp 0dp;
}
.corner.tl { top: -10dp; left: -10dp; }
.corner.tr { top: -10dp; right: -10dp; }
.corner.bl { bottom: -10dp; left: -10dp; }
.corner.br { bottom: -10dp; right: -10dp; }

.kicker { text-align: center; color: #9AA5C0; font-size: 11dp; letter-spacing: 4dp; margin-bottom: 2dp; }
.title { text-align: center; font-size: 26dp; color: #ffffff; margin: 2dp 0dp 18dp 0dp; letter-spacing: 4dp; }
.divider { height: 1dp; background-color: #ffffff2e; margin-bottom: 20dp; }

.verb-pill {
  position: relative; width: 364dp; padding: 14dp 18dp; margin-bottom: 12dp;
  decorator: vertical-gradient( #3A4566 #1B2238 );
  border: 1dp #ffffff12; border-radius: 999dp;
  font-size: 14dp; color: #E7ECF5; letter-spacing: 2dp;
}
.verb-pill.focused {
  color: #ffffff; border: 1dp #22D3EE;
  box-shadow: #22D3EE 0dp 0dp 22dp 2dp;
}
.verb-pill.danger { border: 1dp #E11D7459; }

.footer-hint { text-align: center; color: #9AA5C0; font-size: 11dp; margin-top: 6dp; }

.field { margin-bottom: 24dp; }
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

.btn-back {
  text-align: center; width: 364dp; padding: 13dp; margin-top: 8dp;
  border: 1dp #3A4566; border-radius: 999dp; color: #9AA5C0;
  font-size: 12dp; letter-spacing: 2dp;
}
.btn-back.focused { color: #ffffff; border: 1dp #22D3EE; box-shadow: #22D3EE 0dp 0dp 18dp 1dp; }
)RCSS";

// Corpo dos 4 pontos de latao (idempotente entre as 2 telas).
constexpr const char* kCorners =
    "<div class=\"corner tl\"></div><div class=\"corner tr\"></div>"
    "<div class=\"corner bl\"></div><div class=\"corner br\"></div>";

std::string build_pause_body(const SystemMenuState& state,
                              const gus::app::i18n::Translator& tr) {
    std::ostringstream body;
    body << "<div id=\"sysmenu-scrim\"></div>";
    body << "<div id=\"sysmenu-panel\">" << kCorners;
    body << "<div class=\"kicker\">" << tr.tr("MENU_SYSTEM_KICKER") << "</div>";
    body << "<div class=\"title\">" << tr.tr("MENU_PAUSE_TITLE") << "</div>";
    body << "<div class=\"divider\"></div>";

    struct Item {
        int index;
        const char* key;
        const char* extra_class;
    };
    const Item items[3] = {
        {0, "MENU_CONTINUE", ""},
        {1, "SETTINGS_TITLE", ""},
        {2, "MENU_QUIT", " danger"},
    };
    for (const Item& item : items) {
        const bool focused = (state.pause_selected == item.index);
        body << "<div class=\"verb-pill" << item.extra_class
             << (focused ? " focused" : "") << "\" id=\"pause-item-" << item.index
             << "\">" << tr.tr(item.key) << "</div>";
    }

    body << "<div class=\"footer-hint\">" << tr.tr("MENU_PAUSE_HINT") << "</div>";
    body << "</div>";  // #sysmenu-panel
    return body.str();
}

std::string build_config_body(const SystemMenuState& state,
                               const gus::app::i18n::Translator& tr) {
    std::ostringstream body;
    body << "<div id=\"sysmenu-scrim\"></div>";
    body << "<div id=\"sysmenu-panel\">" << kCorners;
    body << "<div class=\"kicker\">" << tr.tr("MENU_SYSTEM_KICKER") << "</div>";
    body << "<div class=\"title\">" << tr.tr("SETTINGS_TITLE") << "</div>";
    body << "<div class=\"divider\"></div>";

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
        const bool focused = (state.config_selected == row.index);
        const std::string pct = pct_string(row.volume);
        body << "<div class=\"field\">"
             << "<span class=\"name\">" << tr.tr(row.name_key) << "</span> "
             << "<span class=\"val\">" << pct << "</span>"
             << "<div class=\"track\" id=\"slider-track-" << row.index << "\">"
             << "<div class=\"fill\" style=\"width:" << pct << ";\"></div>"
             << "<div class=\"slider-node" << (focused ? " focused" : "")
             << "\" id=\"slider-node-" << row.index << "\" style=\"left:" << pct
             << ";\">"
             << "<div class=\"slider-node-hex-inner\"></div>"
             << "</div>"
             << "</div></div>";
    }

    const bool back_focused =
        (state.config_selected == static_cast<int>(ConfigItem::Back));
    body << "<div class=\"btn-back" << (back_focused ? " focused" : "")
         << "\" id=\"config-back\">" << tr.tr("SETTINGS_BACK") << "</div>";

    body << "</div>";  // #sysmenu-panel
    return body.str();
}

}  // namespace

std::string build_system_menu_rml(const SystemMenuState& state,
                                   const gus::app::i18n::Translator& translator) {
    std::string body;
    if (state.screen == SystemMenuScreen::Pause) {
        body = build_pause_body(state, translator);
    } else if (state.screen == SystemMenuScreen::Config) {
        body = build_config_body(state, translator);
    }
    // Hidden (ou qualquer outro): body fica vazio - documento minimo, sem paineis.

    std::string rml;
    rml += "<rml>\n<head>\n<style>\n";
    rml += kSharedStyle;
    rml += "\n</style>\n</head>\n<body>\n";
    rml += body;
    rml += "\n</body>\n</rml>\n";
    return rml;
}

}  // namespace gus::app::screens
