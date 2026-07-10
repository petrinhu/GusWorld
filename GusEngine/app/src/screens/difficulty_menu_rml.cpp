// gus/app/src/screens/difficulty_menu_rml.cpp
//
// Implementacao de build_difficulty_menu_rml. Ver header para o contrato.
//
// RECEITA VISUAL: MESMA paleta/moldura de title_menu_rml.cpp/save_load_menu_rml.cpp
// (painel gradiente 3-stop + corners latao hexagonais + glow cyan de selecao) -
// arquivo AUTO-CONTIDO (proprio <style>, sem incluir os outros - MESMO
// nao-acoplamento que ja separa as demais telas do menu).

#include "gus/app/screens/difficulty_menu_rml.hpp"

#include <array>
#include <sstream>
#include <string>

namespace gus::app::screens {

namespace {

// Monta o envelope RML completo - MESMA receita de wrap_document em
// title_menu_rml.cpp/save_load_menu_rml.cpp.
std::string wrap_document(const std::string& body_html) {
    std::string rml;
    rml += "<rml>\n<head>\n<style>\n";
    rml += R"RCSS(
body { font-family: "Pixel Operator Mono"; background: transparent; width: 100%; height: 100%; }
#difficulty-scrim {
  position: absolute; top: 0dp; left: 0dp; right: 0dp; bottom: 0dp;
  background-color: #05070ca8;
}
#difficulty-panel {
  box-sizing: border-box;
  position: absolute; top: 50%; left: 50%; margin-left: -220dp; margin-top: -190dp;
  width: 440dp;
  decorator: linear-gradient( 180deg, #3A4566 0%, #1B2238 42%, #0A0E1A 100% );
  border: 1dp #7A5A2E;
  border-radius: 20dp;
  padding: 26dp 22dp 20dp 22dp;
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
.difficulty-title { font-size: 18dp; letter-spacing: 2dp; color: #ffffff; margin: 4dp 0dp 18dp 0dp; }
.difficulty-item {
  box-sizing: border-box; display: block; width: 100%; margin: 8dp 0dp; padding: 10dp 14dp;
  border-radius: 9dp; border: 1dp #7A5A2E;
  decorator: vertical-gradient( #1e2740 #141b2e );
  text-align: left;
}
.difficulty-item.sel {
  border: 1dp #22D3EE; box-shadow: #22D3EE 0dp 0dp 0dp 1dp inset, #22D3EE26 0dp 0dp 16dp 0dp;
}
.difficulty-item.pressed {
  decorator: vertical-gradient( #22D3EE #0EA5C9 ); border: 1dp #ffffff;
  box-shadow: #ffffff 0dp 0dp 22dp 3dp;
}
.difficulty-item:hover { border: 1dp #22D3EE88; }
.difficulty-item-label { color: #E7ECF5; font-size: 13dp; letter-spacing: 1dp; margin-bottom: 3dp; }
.difficulty-item.sel .difficulty-item-label { color: #ffffff; }
.difficulty-badge {
  display: inline-block; margin-left: 8dp; padding: 1dp 7dp; border-radius: 999dp;
  decorator: vertical-gradient( #22D3EE #0EA5C9 ); font-size: 8dp; color: #071019;
  letter-spacing: 0.5dp;
}
.difficulty-item-desc { color: #9AA5C0; font-size: 10dp; line-height: 14dp; }
.difficulty-hint { color: #9AA5C0; font-size: 9dp; line-height: 13dp; margin-top: 16dp; }
.difficulty-foot { color: #9AA5C0; font-size: 9dp; margin-top: 10dp; letter-spacing: 0.5dp; }
.confirm-title { color: #E7ECF5; font-size: 13dp; line-height: 18dp; margin: 10dp 0dp 10dp 0dp; }
.confirm-body { color: #9AA5C0; font-size: 10dp; line-height: 15dp; margin: 0dp 0dp 18dp 0dp; }
.confirm-pill {
  box-sizing: border-box; width: 280dp; padding: 8dp 14dp; margin: 6dp auto;
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

// Classe extra "pressed" (MESMO efeito de FLASH das demais telas - so aplicavel a
// lista de itens, NAO ao splash de confirmacao).
std::string pressed_class(int index, int pressed_index) {
    return (pressed_index >= 0 && index == pressed_index) ? " pressed" : "";
}

// Chave i18n do LABEL + DESC de cada item, na MESMA ordem de DifficultyMenuItem.
constexpr std::array<const char*, kDifficultyItemCount> kLabelKeys = {
    "SAVE_DIFFICULTY_FACIL_LABEL", "SAVE_DIFFICULTY_MEDIO_LABEL",
    "SAVE_DIFFICULTY_DIFICIL_LABEL"};
constexpr std::array<const char*, kDifficultyItemCount> kDescKeys = {
    "SAVE_DIFFICULTY_FACIL_DESC", "SAVE_DIFFICULTY_MEDIO_DESC",
    "SAVE_DIFFICULTY_DIFICIL_DESC"};

// Copy final aprovada pelo lider (2026-07-10, via ux-writer): o splash de
// confirmacao (Aviso #2) tem TITULO e botao "Sim" PROPRIOS por dificuldade (o
// parser i18n NAO interpola placeholders - "Jogar no Fácil?"/"Sim, jogar no
// Fácil" sao chaves INTEIRAS, nao um prefixo+label concatenado como no
// rascunho). O corpo (SAVE_DIFFICULTY_CONFIRM_BODY) e o "Cancelar"
// (SAVE_DIFFICULTY_CONFIRM_NO) sao UNICOS, independem da dificuldade.
constexpr std::array<const char*, kDifficultyItemCount> kConfirmTitleKeys = {
    "SAVE_DIFFICULTY_CONFIRM_TITLE_FACIL", "SAVE_DIFFICULTY_CONFIRM_TITLE_MEDIO",
    "SAVE_DIFFICULTY_CONFIRM_TITLE_DIFICIL"};
constexpr std::array<const char*, kDifficultyItemCount> kConfirmYesKeys = {
    "SAVE_DIFFICULTY_CONFIRM_YES_FACIL", "SAVE_DIFFICULTY_CONFIRM_YES_MEDIO",
    "SAVE_DIFFICULTY_CONFIRM_YES_DIFICIL"};

}  // namespace

std::string build_difficulty_menu_rml(const DifficultyMenuState& state,
                                       const gus::app::i18n::Translator& translator,
                                       int pressed_index) {
    std::ostringstream body;

    body << "<div id=\"difficulty-scrim\"></div>";
    body << "<div id=\"difficulty-panel\">";
    body << "<div class=\"corner tl\"></div><div class=\"corner tr\"></div>"
            "<div class=\"corner bl\"></div><div class=\"corner br\"></div>";
    body << "<div class=\"difficulty-title\">" << translator.tr("SAVE_DIFFICULTY_TITLE")
         << "</div>";

    if (state.confirming) {
        // Splash Aviso #2 (§2.2): titulo/botao "Sim" PROPRIOS da dificuldade
        // escolhida ("Jogar no Fácil?"/"Sim, jogar no Fácil") + corpo unico +
        // Cancelar - substitui a lista enquanto aberto (MESMA mecanica de
        // confirming_new_game em title_menu_rml.cpp).
        const std::size_t idx = static_cast<std::size_t>(
            state.selected >= 0 && state.selected < kDifficultyItemCount
                ? state.selected
                : static_cast<int>(DifficultyMenuItem::Medio));
        body << "<div class=\"confirm-title\">" << translator.tr(kConfirmTitleKeys[idx])
             << "</div>";
        body << "<div class=\"confirm-body\">"
             << translator.tr("SAVE_DIFFICULTY_CONFIRM_BODY") << "</div>";
        body << "<div class=\"confirm-pill" << (state.confirm_selected == 0 ? " focused" : "")
             << "\" id=\"difficulty-confirm-0\">" << translator.tr(kConfirmYesKeys[idx])
             << "</div>";
        body << "<div class=\"confirm-pill" << (state.confirm_selected == 1 ? " focused" : "")
             << "\" id=\"difficulty-confirm-1\">"
             << translator.tr("SAVE_DIFFICULTY_CONFIRM_NO") << "</div>";
        body << "</div>";  // #difficulty-panel
        return wrap_document(body.str());
    }

    for (int i = 0; i < kDifficultyItemCount; ++i) {
        const bool focused = (state.selected == i);
        std::string classes = "difficulty-item";
        if (focused) classes += " sel";
        classes += pressed_class(i, pressed_index);

        body << "<div class=\"" << classes << "\" id=\"difficulty-item-" << i << "\">";
        body << "<div class=\"difficulty-item-label\">"
             << translator.tr(kLabelKeys[static_cast<std::size_t>(i)]);
        // Badge "Recomendado" SO no Medio (§2.1 default canonico).
        if (static_cast<DifficultyMenuItem>(i) == DifficultyMenuItem::Medio) {
            body << "<span class=\"difficulty-badge\">"
                 << translator.tr("SAVE_DIFFICULTY_MEDIO_BADGE") << "</span>";
        }
        body << "</div>";
        body << "<div class=\"difficulty-item-desc\">"
             << translator.tr(kDescKeys[static_cast<std::size_t>(i)]) << "</div>";
        body << "</div>";
    }
    // Aviso #1 (§2.2): aviso fixo, sempre visivel enquanto navega a lista.
    body << "<div class=\"difficulty-hint\">" << translator.tr("SAVE_DIFFICULTY_HINT")
         << "</div>";
    body << "<div class=\"difficulty-foot\">"
         << translator.tr("SAVE_DIFFICULTY_FOOTER_HINT") << "</div>";

    body << "</div>";  // #difficulty-panel
    return wrap_document(body.str());
}

}  // namespace gus::app::screens
