// gus/app/screens/difficulty_menu_rml.hpp
//
// RML/RCSS da TELA DE SELECAO DE DIFICULDADE (MODOS-MORTE Fase 0). MESMO estilo
// de autoria de gus/app/screens/title_menu_rml.cpp/save_load_menu_rml.cpp (RML
// como string C++, efeitos nativos do backend GL3 do glintfx:
// linear-gradient/polygon/box-shadow) e MESMA paleta azul+latao (moldura
// hexagonal, painel gradiente, glow cyan de selecao) - reusa literalmente a MESMA
// folha de estilo de title_menu_rml.cpp (.title-item/.confirm-pill/.confirm-title/
// corners) pra continuidade visual (a tela abre LOGO em seguida da tela de
// titulo, no MESMO contexto GL).
//
// build_difficulty_menu_rml gera um SNAPSHOT (nao reativo) do DifficultyMenuState
// dado - o CHAMADOR (difficulty_menu_loop.cpp) gera uma nova string e chama
// glintfx::UiLayer::load() de novo sempre que o estado mudar, MESMA receita de
// reload-on-change das demais telas.
//
// Cross-ref: gus/app/screens/difficulty_menu.hpp (o estado/logica pura);
//            gus/app/screens/title_menu_rml.cpp (MESMA paleta/moldura, folha
//            de estilo compartilhada por copia deliberada - AUTO-CONTIDO, sem
//            incluir o outro, mesmo nao-acoplamento ja estabelecido entre as
//            telas do menu); gus/app/i18n/translator.hpp (tr()).

#ifndef GUS_APP_SCREENS_DIFFICULTY_MENU_RML_HPP
#define GUS_APP_SCREENS_DIFFICULTY_MENU_RML_HPP

#include <string>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/difficulty_menu.hpp"

namespace gus::app::screens {

// Gera o RML/RCSS completo (com <style> embutido) da tela de selecao de
// dificuldade no estado ATUAL de `state` + os rotulos traduzidos de `translator`.
// `pressed_index` (default -1): quando >= 0, o item de indice `pressed_index`
// ganha a classe extra "pressed" (MESMO flash visual de PRESS das demais telas).
//
// Nao inclui @font-face (o CHAMADOR injeta, mesma receita de
// write_baked_cockpit_rml em battle_preview.cpp) nem resolve caminhos de asset.
[[nodiscard]] std::string build_difficulty_menu_rml(
    const DifficultyMenuState& state, const gus::app::i18n::Translator& translator,
    int pressed_index = -1);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_DIFFICULTY_MENU_RML_HPP
