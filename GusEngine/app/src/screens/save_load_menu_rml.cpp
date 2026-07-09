// gus/app/src/screens/save_load_menu_rml.cpp
//
// Implementacao de build_save_load_menu_rml. Ver header para o contrato.
//
// RECEITA VISUAL: MESMA de system_menu_rml.cpp (painel gradiente 3-stop +
// moldura latao hexagonal + pills com glow cyan de selecao) - ver os
// comentarios longos la (HEXAGONO DO SLIDER, CAUSA RAIZ DO DESLOCAMENTO/body
// sem width:100%, box-sizing:border-box) para o racional COMPLETO de cada
// truque de RCSS; aqui so o SUBCONJUNTO necessario pra esta tela (painel +
// corners + linhas de slot + mini-dialogo Sim/Nao), auto-contido (arquivo
// proprio, sem incluir system_menu_rml.cpp - MESMO espirito de nao-acoplamento
// que separa battle_preview.cpp de system_menu_rml.cpp, cada um com seu
// <style> completo).

#include "gus/app/screens/save_load_menu_rml.hpp"

#include <sstream>
#include <string>

#include "gus/domain/save/save_slots.hpp"

namespace gus::app::screens {

namespace {

using gus::domain::save::kSlotCount;

// Monta o envelope RML completo (<rml><head><style>kSharedStyle</style></head>
// <body>body_html</body></rml>) - MESMA receita de build_system_menu_rml
// (system_menu_rml.cpp). Extraido pra evitar duplicar o envelope nos 2 pontos
// de retorno da funcao publica (mini-dialogo vs lista normal).
std::string wrap_document(const std::string& body_html);

// Sentinela de pressed_index pro botao "Voltar" (fora do range 0..kSlotCount-1
// dos slots) - MESMA convencao de kPlaceholderBackIndex em system_menu.hpp.
constexpr int kBackPressedIndex = kSlotCount;

std::string pressed_class(int index, int pressed_index) {
    return (pressed_index >= 0 && index == pressed_index) ? " pressed" : "";
}

// Substitui a PRIMEIRA ocorrencia de "{0}" por `value` - Translator::tr(key) so
// resolve KEY->string, NAO interpola (MESMA limitacao/receita manual ja usada
// em system_menu_rml.cpp::build_pause_body pro MENU_PAUSE_HINT).
std::string interpolate(std::string text, const std::string& value) {
    const auto pos = text.find("{0}");
    if (pos != std::string::npos) text.replace(pos, 3, value);
    return text;
}

// Local textual de exibicao (i18n) a partir de scene_path. Tabela MINIMA (uma
// entrada, o UNICO mapa que existe hoje - "distritos_inferiores", ver M4 no
// TODO.md) - PLACEHOLDER deliberado: quando novos mapas existirem, cada um
// ganha 1 linha aqui (ou uma tabela dedicada, se a lista crescer o bastante
// pra justificar mover pra domain/ com dono proprio - fora do escopo desta
// dispatch). scene_path desconhecido cai em LOCATION_UNKNOWN.
std::string location_key_for_scene(const std::string& scene_path) {
    if (scene_path == "distritos_inferiores") return "LOCATION_PRACA_COMPILACAO";
    return "LOCATION_UNKNOWN";
}

// RCSS compartilhado (MESMO conteudo de paleta/moldura pras 2 telas). Extraido
// pra constante (nao inline no body) porque o documento RML precisa da
// sequencia <rml><head><style>...</style></head><body>...</body></rml> - a
// CAUSA RAIZ de o probe headless nao renderizar NADA (get_element_box de todo
// id devolvia found=0) era EXATAMENTE a falta desse envelope: a 1a versao
// deste arquivo devolvia so "<style>...</style><div ...>" sem <rml>/<head>/
// <body>, que o RmlUi (via Rml::LoadDocument, chamado por
// glintfx::UiLayer::load) simplesmente nao reconhece como documento valido -
// MESMA receita de envelope de build_system_menu_rml (system_menu_rml.cpp,
// linhas finais: "<rml>\n<head>\n<style>\n" + kSharedStyle + "\n</style>\n
// </head>\n<body>\n" + body + "\n</body>\n</rml>\n").
constexpr const char* kSharedStyle = R"RCSS(
body { font-family: "Pixel Operator Mono"; background: transparent; width: 100%; height: 100%; }
#slmenu-scrim {
  position: absolute; top: 0dp; left: 0dp; right: 0dp; bottom: 0dp;
  background-color: #05070ca8;
}
#slmenu-panel {
  box-sizing: border-box;
  position: absolute; top: 40dp; left: 50%; margin-left: -230dp;
  width: 460dp;
  decorator: linear-gradient( 180deg, #3A4566 0%, #1B2238 42%, #0A0E1A 100% );
  border: 1dp #7A5A2E;
  border-radius: 20dp;
  padding: 26dp 26dp 22dp 26dp;
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
.title { text-align: center; font-size: 20dp; line-height: 30dp; color: #F0D98C; margin: 2dp 0dp 4dp 0dp; padding-top: 4dp; letter-spacing: 3dp; }
.subtitle { text-align: center; color: #9AA5C0; font-size: 10dp; line-height: 15dp; margin-bottom: 14dp; }
.slot-list { height: 300dp; overflow-x: hidden; overflow-y: auto; margin-top: 2dp; padding-right: 6dp; }
#slmenu-list scrollbarvertical { width: 8dp; }
#slmenu-list scrollbarvertical slidertrack { background-color: #0A0E1A80; border-radius: 999dp; }
#slmenu-list scrollbarvertical sliderbar { background-color: #C9A24Bb3; border-radius: 999dp; min-height: 24dp; }
#slmenu-list scrollbarvertical sliderbar:hover { background-color: #C9A24Bff; }
#slmenu-list scrollbarvertical sliderarrowdec,
#slmenu-list scrollbarvertical sliderarrowinc { height: 0dp; }
.slot {
  box-sizing: border-box; display: flex; align-items: center;
  width: 400dp; padding: 10dp 12dp; margin: 6dp 0dp; border-radius: 10dp;
  decorator: vertical-gradient( #1a2238 #141b2e );
  border: 1dp #33405e;
}
.slot.readonly { opacity: 0.85; }
.slot.empty { border: 1dp #3a4256; color: #6B6F7A; }
.slot.focused {
  border: 1dp #22D3EE; box-shadow: #22D3EE 0dp 0dp 0dp 1dp inset;
}
.slot.pressed {
  decorator: vertical-gradient( #22D3EE #0EA5C9 ); border: 1dp #ffffff;
  box-shadow: #ffffff 0dp 0dp 24dp 3dp;
}
.slot-num {
  flex: 0 0 auto; width: 32dp; height: 32dp; margin-right: 12dp; border-radius: 7dp;
  text-align: center; line-height: 32dp;
  decorator: vertical-gradient( #0d1320 #0d1320 );
  border: 1dp #7A5A2E; color: #F0D98C; font-size: 14dp;
}
.slot-body { flex: 1; }
.slot-title { color: #E7ECF5; font-size: 12dp; margin-bottom: 2dp; }
.slot-title .loc { color: #F0D98C; }
.slot-title .ro { color: #6B6F7A; font-size: 10dp; }
.slot-meta { color: #9AA5C0; font-size: 10dp; }
.slot-empty-label { color: #6B6F7A; font-size: 11dp; }
.footer {
  display: flex; justify-content: space-between; align-items: center;
  margin-top: 12dp; padding-top: 10dp; border-top: 1dp #33281a;
}
.footer-hint { color: #9AA5C0; font-size: 10dp; }
.btn-back {
  box-sizing: border-box; text-align: center; width: 100dp; padding: 6dp 10dp;
  border: 1dp #3A4566; border-radius: 999dp; color: #9AA5C0; font-size: 11dp; letter-spacing: 1dp;
}
.btn-back:hover { color: #c3cadb; border-color: #6a7aa2; }
.btn-back.focused { color: #ffffff; border: 1dp #22D3EE; box-shadow: #22D3EE 0dp 0dp 14dp 1dp; }
.btn-back.pressed {
  decorator: vertical-gradient( #22D3EE #0EA5C9 ); color: #071019; border: 1dp #ffffff;
  box-shadow: #ffffff 0dp 0dp 22dp 3dp;
}
.confirm-title { text-align: center; color: #E7ECF5; font-size: 13dp; line-height: 20dp; margin: 20dp 0dp 20dp 0dp; }
.confirm-pill {
  box-sizing: border-box; width: 300dp; padding: 8dp 16dp; margin: 6dp auto;
  text-align: center; decorator: vertical-gradient( #3A4566 #1B2238 );
  border: 1dp #ffffff12; border-radius: 999dp; font-size: 12dp; color: #E7ECF5; letter-spacing: 1dp;
}
.confirm-pill.focused { color: #ffffff; border: 1dp #22D3EE; box-shadow: #22D3EE 0dp 0dp 18dp 1dp; }
)RCSS";

std::string wrap_document(const std::string& body_html) {
    std::string rml;
    rml += "<rml>\n<head>\n<style>\n";
    rml += kSharedStyle;
    rml += "\n</style>\n</head>\n<body>\n";
    rml += body_html;
    rml += "\n</body>\n</rml>\n";
    return rml;
}

}  // namespace

std::string build_save_load_menu_rml(const SaveLoadMenuState& state,
                                      const gus::app::i18n::Translator& tr,
                                      int pressed_index) {
    std::ostringstream body;

    body << "<div id=\"slmenu-scrim\"></div>";
    body << "<div id=\"slmenu-panel\">";
    body << "<div class=\"corner tl\"></div><div class=\"corner tr\"></div>"
            "<div class=\"corner bl\"></div><div class=\"corner br\"></div>";

    const bool is_save = (state.mode == SaveLoadMode::Save);
    body << "<div class=\"title\">"
         << tr.tr(is_save ? "SAVE_SCREEN_TITLE_SAVE" : "SAVE_SCREEN_TITLE_LOAD")
         << "</div>";
    body << "<div class=\"subtitle\">"
         << interpolate(tr.tr(is_save ? "SAVE_SCREEN_SUBTITLE_SAVE"
                                       : "SAVE_SCREEN_SUBTITLE_LOAD"),
                        std::to_string(kSlotCount))
         << "</div>";

    if (state.confirming_overwrite) {
        // Mini-dialogo Sim/Nao (MESMA mecanica de controls_confirming_restore em
        // system_menu_rml.cpp) - substitui a lista enquanto aberto.
        body << "<div class=\"confirm-title\">" << tr.tr("SAVE_CONFIRM_OVERWRITE")
             << "</div>";
        body << "<div class=\"confirm-pill" << (state.confirm_selected == 0 ? " focused" : "")
             << "\" id=\"slmenu-confirm-yes\">" << tr.tr("SAVE_OVERWRITE_CONFIRM_YES")
             << "</div>";
        body << "<div class=\"confirm-pill" << (state.confirm_selected == 1 ? " focused" : "")
             << "\" id=\"slmenu-confirm-no\">" << tr.tr("SAVE_OVERWRITE_CONFIRM_NO")
             << "</div>";
        body << "</div>";  // #slmenu-panel
        return wrap_document(body.str());
    }

    body << "<div class=\"slot-list\" id=\"slmenu-list\">";
    for (int i = 0; i < kSlotCount; ++i) {
        const SaveSlotPreview& slot = state.slots[static_cast<std::size_t>(i)];
        const bool focused = (state.selected == i) && slot_selectable(state, i);
        const bool readonly = (state.mode == SaveLoadMode::Save) && slot.is_autosave;

        std::string classes = "slot";
        if (readonly) classes += " readonly";
        if (!slot.occupied) classes += " empty";
        if (focused) classes += " focused";
        classes += pressed_class(i, pressed_index);

        body << "<div class=\"" << classes << "\" id=\"slmenu-slot-" << i << "\">";
        body << "<div class=\"slot-num\">"
             << (slot.is_autosave ? tr.tr("SAVE_SLOT_AUTO_NAME") : std::to_string(i))
             << "</div>";
        body << "<div class=\"slot-body\">";
        if (!slot.occupied) {
            body << "<div class=\"slot-empty-label\">"
                 << interpolate(tr.tr("SAVE_SLOT_EMPTY"), std::to_string(i)) << "</div>";
        } else {
            const std::string slot_label = slot.is_autosave
                                                ? tr.tr("SAVE_SLOT_AUTO_NAME")
                                                : interpolate(tr.tr("SAVE_SLOT_LABEL"),
                                                              std::to_string(i));
            body << "<div class=\"slot-title\">" << slot_label << " <span class=\"loc\">- "
                 << tr.tr(location_key_for_scene(slot.scene_path)) << "</span>";
            if (readonly) {
                body << " <span class=\"ro\">" << tr.tr("SAVE_SLOT_READONLY_TAG")
                     << "</span>";
            }
            body << "</div>";
            body << "<div class=\"slot-meta\">" << format_timestamp_ms(slot.timestamp_ms)
                 << "  -  " << format_playtime_seconds(slot.playtime_seconds) << "  -  "
                 << interpolate(tr.tr("SAVE_XP_LABEL"), std::to_string(slot.xp))
                 << "  -  " << interpolate(tr.tr("SAVE_CHAPTER_LABEL"),
                                            std::to_string(slot.chapter))
                 << "</div>";
        }
        body << "</div>";  // .slot-body
        body << "</div>";  // .slot
    }
    body << "</div>";  // .slot-list

    body << "<div class=\"footer\">";
    body << "<span class=\"footer-hint\">"
         << tr.tr(is_save ? "SAVE_SCREEN_FOOTER_SAVE" : "SAVE_SCREEN_FOOTER_LOAD")
         << "</span>";
    body << "<div class=\"btn-back focused" << pressed_class(kBackPressedIndex, pressed_index)
         << "\" id=\"slmenu-back\">" << tr.tr("SETTINGS_BACK") << "</div>";
    body << "</div>";  // .footer

    body << "</div>";  // #slmenu-panel
    return wrap_document(body.str());
}

}  // namespace gus::app::screens
