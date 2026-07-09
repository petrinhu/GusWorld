// gus/app/screens/save_load_menu_rml.hpp
//
// RML/RCSS da TELA DE SALVAR/CARREGAR (SAVE-LOAD-UI), fiel ao mock APROVADO
// docs/design/mockups/07-save-load.html (Telas 2 SALVAR e 3 CARREGAR). MESMO
// estilo de autoria de gus/app/screens/system_menu_rml.cpp::build_system_menu_rml
// (RML como string C++, efeitos nativos do backend GL3 do glintfx:
// linear-gradient/polygon/box-shadow/drop-shadow) e MESMA paleta azul+latao
// (moldura hexagonal, painel gradiente, glow cyan de selecao).
//
// build_save_load_menu_rml gera um SNAPSHOT (nao reativo) do SaveLoadMenuState
// dado - o CHAMADOR gera uma nova string e chama glintfx::UiLayer::load() de
// novo sempre que o estado mudar (navegacao/confirmacao), MESMA receita de
// reload-on-change de build_system_menu_rml (o menu de save/load tambem so muda
// em input do jogador, nao a cada frame).
//
// Cross-ref: gus/app/screens/save_load_menu.hpp (o estado/logica pura);
//            gus/app/screens/system_menu_rml.cpp (mesma receita de autoria +
//            moldura/corners/painel); gus/app/i18n/translator.hpp (tr()).

#ifndef GUS_APP_SCREENS_SAVE_LOAD_MENU_RML_HPP
#define GUS_APP_SCREENS_SAVE_LOAD_MENU_RML_HPP

#include <string>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/save_load_menu.hpp"

namespace gus::app::screens {

// Gera o RML/RCSS completo (com <style> embutido) da tela de save/load no
// estado ATUAL de `state` + os rotulos traduzidos de `translator`.
// `pressed_index` (default -1): quando >= 0, o slot de indice `pressed_index`
// (0..kSlotCount-1, mesma convencao de indices de SaveLoadMenuState::selected)
// OU o botao "Voltar" (index == gus::domain::save::kSlotCount, sentinela)
// ganha a classe extra "pressed" (MESMO flash visual de PRESS de
// system_menu_rml.cpp, efeito NOSSO - o chamador renderiza alguns frames
// pressionado ANTES de aplicar a transicao real).
//
// Nao inclui @font-face (o CHAMADOR injeta, mesma receita de
// write_baked_cockpit_rml em battle_preview.cpp) nem resolve caminhos de asset.
[[nodiscard]] std::string build_save_load_menu_rml(
    const SaveLoadMenuState& state, const gus::app::i18n::Translator& translator,
    int pressed_index = -1);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_SAVE_LOAD_MENU_RML_HPP
