// gus/app/src/screens/title_menu_rml.cpp
//
// Implementacao de build_title_menu_rml. Ver header para o contrato.
//
// RECEITA VISUAL: mock docs/design/mockups/07-save-load.html (Tela 1 - TITULO) +
// MESMA paleta/moldura de save_load_menu_rml.cpp/system_menu_rml.cpp (painel
// gradiente 3-stop + corners latao hexagonais + glow cyan de selecao) - arquivo
// AUTO-CONTIDO (proprio <style>, sem incluir os outros - MESMO nao-acoplamento
// que ja separa save_load_menu_rml.cpp de system_menu_rml.cpp).

#include "gus/app/screens/title_menu_rml.hpp"

#include <sstream>
#include <string>

namespace gus::app::screens {

namespace {

// Monta o envelope RML completo - MESMA receita de wrap_document em
// save_load_menu_rml.cpp (a CAUSA RAIZ historica de "nada renderiza" era
// exatamente a falta desse envelope <rml><head><style>...</style></head>
// <body>...</body></rml>, ver o comentario grande la).
std::string wrap_document(const std::string& body_html) {
    std::string rml;
    rml += "<rml>\n<head>\n<style>\n";
    rml += R"RCSS(
body { font-family: "Pixel Operator Mono"; background: transparent; width: 100%; height: 100%; }
#title-scrim {
  position: absolute; top: 0dp; left: 0dp; right: 0dp; bottom: 0dp;
  background-color: #05070ca8;
}
#title-panel {
  box-sizing: border-box;
  position: absolute; top: 50%; left: 50%; margin-left: -190dp; margin-top: -170dp;
  width: 380dp;
  decorator: linear-gradient( 180deg, #3A4566 0%, #1B2238 42%, #0A0E1A 100% );
  border: 1dp #7A5A2E;
  border-radius: 20dp;
  padding: 26dp 20dp 20dp 20dp;
  text-align: center;
  box-shadow: #22D3EE1a 0dp 0dp 20dp 0dp;
}
.corner {
  position: absolute; width: 18dp; height: 18dp;
  decorator: polygon( 6, radial-gradient( circle at 40% 35%, #F0D98C, #C9A24B 55%, #7A5A2E 100% ) );
  filter: drop-shadow( #00000080 0dp 1dp 3dp );
}
.corner.tl { top: 8dp; left: 8dp; }
.corner.tr { top: 8dp; right: 8dp; }
.corner.bl { bottom: 8dp; left: 8dp; }
.corner.br { bottom: 8dp; right: 8dp; }
.game-logo { font-size: 32dp; line-height: 38dp; letter-spacing: 4dp; color: #ffffff; margin: 6dp 0dp 2dp 0dp; }
.game-logo .w { color: #F0D98C; }
.game-sub { color: #9AA5C0; font-size: 10dp; line-height: 15dp; letter-spacing: 3dp; margin-bottom: 22dp; }
.title-item {
  box-sizing: border-box; display: block; width: 230dp; margin: 8dp auto; padding: 10dp 0dp;
  border-radius: 9dp; border: 1dp #7A5A2E;
  decorator: vertical-gradient( #1e2740 #141b2e );
  color: #E7ECF5; font-size: 13dp; letter-spacing: 2dp;
}
.title-item.sel {
  border: 1dp #22D3EE; box-shadow: #22D3EE 0dp 0dp 0dp 1dp inset, #22D3EE26 0dp 0dp 16dp 0dp;
  color: #ffffff;
}
.title-item.disabled { color: #6B6F7A; }
.title-item.pressed {
  decorator: vertical-gradient( #22D3EE #0EA5C9 ); border: 1dp #ffffff; color: #071019;
  box-shadow: #ffffff 0dp 0dp 22dp 3dp;
}
.title-item:hover { border: 1dp #22D3EE88; }
.title-foot { color: #9AA5C0; font-size: 9dp; margin-top: 18dp; letter-spacing: 0.5dp; }
.confirm-title { color: #E7ECF5; font-size: 12dp; line-height: 18dp; margin: 16dp 0dp 18dp 0dp; }
.confirm-pill {
  box-sizing: border-box; width: 260dp; padding: 8dp 14dp; margin: 6dp auto;
  decorator: vertical-gradient( #3A4566 #1B2238 ); border: 1dp #ffffff12; border-radius: 999dp;
  font-size: 11dp; color: #E7ECF5; letter-spacing: 1dp;
}
.confirm-pill.focused { color: #ffffff; border: 1dp #22D3EE; box-shadow: #22D3EE 0dp 0dp 18dp 1dp; }
)RCSS";
    rml += "\n</style>\n</head>\n<body>\n";
    rml += body_html;
    rml += "\n</body>\n</rml>\n";
    return rml;
}

// Classe extra "pressed" (MESMO efeito de FLASH de confirmacao das demais telas -
// so aplicavel a lista de itens, NAO ao mini-dialogo de Novo Jogo, ver o
// comentario do header pressed_index).
std::string pressed_class(int index, int pressed_index) {
    return (pressed_index >= 0 && index == pressed_index) ? " pressed" : "";
}

// Chave i18n de rotulo de cada item, na MESMA ordem de TitleMenuItem. Reusa as
// chaves JA EXISTENTES no catalogo (MENU_CONTINUE/MENU_NEW_GAME/MENU_QUIT -
// consumidas hoje pelo menu de pausa, system_menu_rml.cpp) - o rotulo e
// IDENTICO ("Continuar"/"Novo Jogo"/"Sair"), nao ha motivo pra duplicar chave.
constexpr const char* kItemKeys[kTitleItemCount] = {"MENU_CONTINUE", "MENU_NEW_GAME",
                                                     "MENU_QUIT"};

}  // namespace

std::string build_title_menu_rml(const TitleMenuState& state,
                                  const gus::app::i18n::Translator& translator,
                                  int pressed_index) {
    std::ostringstream body;

    body << "<div id=\"title-scrim\"></div>";
    body << "<div id=\"title-panel\">";
    body << "<div class=\"corner tl\"></div><div class=\"corner tr\"></div>"
            "<div class=\"corner bl\"></div><div class=\"corner br\"></div>";

    body << "<div class=\"game-logo\">" << translator.tr("TITLE_LOGO_PREFIX")
         << "<span class=\"w\">" << translator.tr("TITLE_LOGO_SUFFIX") << "</span></div>";
    body << "<div class=\"game-sub\">" << translator.tr("TITLE_SUBTITLE") << "</div>";

    if (state.confirming_new_game) {
        // Mini-dialogo Sim/Nao (MESMA mecanica de confirming_overwrite em
        // save_load_menu_rml.cpp) - substitui a lista de itens enquanto aberto.
        body << "<div class=\"confirm-title\">" << translator.tr("TITLE_NEW_GAME_CONFIRM")
             << "</div>";
        body << "<div class=\"confirm-pill"
             << (state.confirm_selected == 0 ? " focused" : "")
             << "\" id=\"title-confirm-0\">" << translator.tr("TITLE_NEW_GAME_CONFIRM_YES")
             << "</div>";
        body << "<div class=\"confirm-pill"
             << (state.confirm_selected == 1 ? " focused" : "")
             << "\" id=\"title-confirm-1\">" << translator.tr("TITLE_NEW_GAME_CONFIRM_NO")
             << "</div>";
        body << "</div>";  // #title-panel
        return wrap_document(body.str());
    }

    for (int i = 0; i < kTitleItemCount; ++i) {
        const bool selectable = title_item_selectable(state, i);
        const bool focused = (state.selected == i) && selectable;

        std::string classes = "title-item";
        if (!selectable) classes += " disabled";
        if (focused) classes += " sel";
        classes += pressed_class(i, pressed_index);

        body << "<div class=\"" << classes << "\" id=\"title-item-" << i << "\">"
             << translator.tr(kItemKeys[i]) << "</div>";
    }
    body << "<div class=\"title-foot\">" << translator.tr("TITLE_FOOTER_HINT") << "</div>";

    body << "</div>";  // #title-panel
    return wrap_document(body.str());
}

}  // namespace gus::app::screens
