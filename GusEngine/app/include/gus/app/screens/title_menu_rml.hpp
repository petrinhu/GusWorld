// gus/app/screens/title_menu_rml.hpp
//
// RML/RCSS da TELA DE TITULO (SAVE-LOAD-UI etapa 4), fiel ao mock APROVADO
// docs/design/mockups/07-save-load.html (Tela 1 - TITULO). MESMO estilo de
// autoria de gus/app/screens/save_load_menu_rml.cpp/system_menu_rml.cpp (RML
// como string C++, efeitos nativos do backend GL3 do glintfx:
// linear-gradient/polygon/box-shadow/drop-shadow) e MESMA paleta azul+latao
// (moldura hexagonal, painel gradiente, glow cyan de selecao).
//
// build_title_menu_rml gera um SNAPSHOT (nao reativo) do TitleMenuState dado -
// o CHAMADOR (title_menu_loop.cpp) gera uma nova string e chama
// glintfx::UiLayer::load() de novo sempre que o estado mudar (navegacao/
// confirmacao), MESMA receita de reload-on-change das demais telas.
//
// Cross-ref: gus/app/screens/title_menu.hpp (o estado/logica pura);
//            gus/app/screens/save_load_menu_rml.cpp (mesma receita de autoria +
//            moldura/corners/painel); gus/app/i18n/translator.hpp (tr()).

#ifndef GUS_APP_SCREENS_TITLE_MENU_RML_HPP
#define GUS_APP_SCREENS_TITLE_MENU_RML_HPP

#include <string>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/title_menu.hpp"

namespace gus::app::screens {

// Gera o RML/RCSS completo (com <style> embutido) da tela de titulo no estado
// ATUAL de `state` + os rotulos traduzidos de `translator`. `pressed_index`
// (default -1): quando >= 0, o item de indice `pressed_index` (0..
// kTitleItemCount-1, MESMA convencao de indices de TitleMenuState::selected)
// ganha a classe extra "pressed" (MESMO flash visual de PRESS de
// system_menu_rml.cpp/save_load_menu_rml.cpp - efeito NOSSO, o chamador
// renderiza alguns frames pressionado ANTES de aplicar a transicao real).
//
// Nao inclui @font-face (o CHAMADOR injeta, mesma receita de
// write_baked_cockpit_rml em battle_preview.cpp) nem resolve caminhos de asset.
[[nodiscard]] std::string build_title_menu_rml(
    const TitleMenuState& state, const gus::app::i18n::Translator& translator,
    int pressed_index = -1);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_TITLE_MENU_RML_HPP
