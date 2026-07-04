// gus/app/screens/system_menu_rml.hpp
//
// RML/RCSS do MENU DE SISTEMA (pausa + config de som/video/lingua/save),
// MENU-PAUSA-CONFIG-SOM (M7-COSTURA, onda arvore). Traduz a arvore de telas
// aprovada AO VIVO pelo lider (ver system_menu.hpp para o diagrama completo)
// pro RCSS do glintfx, no MESMO estilo de autoria de
// gus/app/screens/battle_preview.cpp::load_cockpit_rml() (RML/RCSS como string
// C++, efeitos nativos do backend GL3: vertical-gradient/radial-gradient/
// box-shadow-glow/border-radius/animation/polygon).
//
// 7 TELAS num UNICO documento por chamada (so a tela ATIVA de `state.screen`
// gera corpo - Hidden gera documento vazio): Pause (4 pills), ConfigCategories
// (4 pills: Audio/Video/Lingua/Voltar), Audio (2 sliders + Voltar, a antiga
// tela "Config"), e 3 placeholders (Save/Video/Language - "em breve" + Voltar).
// O SystemMenuState (system_menu.hpp) e a UNICA fonte de verdade da selecao/
// volume - esta funcao SO GERA a string RML a partir dele (nao decide nada).
//
// Cross-ref: gus/app/screens/system_menu.hpp (o estado/logica pura);
//            gus/app/screens/battle_preview.cpp (load_cockpit_rml, MESMO estilo
//            de autoria + a receita de stage dir/@font-face/achatamento de asset);
//            gus/app/i18n/translator.hpp (tr() dos rotulos).

#ifndef GUS_APP_SCREENS_SYSTEM_MENU_RML_HPP
#define GUS_APP_SCREENS_SYSTEM_MENU_RML_HPP

#include <string>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/system_menu.hpp"

namespace gus::app::screens {

// Gera o RML/RCSS AUTORADO (com {{bindings}} literais, NAO um data-model live) do
// menu de sistema, no estado ATUAL de `state` + os rotulos traduzidos de
// `translator`. Mesma receita de write_baked_cockpit_rml (battle_preview.cpp):
// GERA UM SNAPSHOT do estado dado (nao e reativo por si so) - o chamador GERA UM
// NOVO RML e chama UiLayer::load() de novo sempre que o estado mudar (navegacao/
// volume) dentro do loop do menu. Isto e deliberado: o menu de sistema muda com
// pouca frequencia (so em input do jogador, nao a cada frame como o HUD de
// combate), entao reload-on-change e simples e barato o bastante - NAO precisa
// do data-model live (bind_number/set_number) que o cockpit usa pro HUD de
// combate (que muda a cada frame de simulacao).
//
// `pressed_index` (default -1 = nenhum): quando >= 0, o item de indice
// `pressed_index` DA TELA ATUAL (mesma convencao de indices de
// system_menu_click_option: PauseItem em Pause, ConfigCategoryItem em
// ConfigCategories, AudioItem em Audio, kPlaceholderBackIndex nas telas
// placeholder) ganha a classe extra "pressed" (flash visual de PRESS, efeito
// NOSSO - ver system_menu_loop.cpp - NAO e feedback nativo do glintfx). Usado
// pelo loop pra renderizar alguns frames de "pressionado" ANTES de executar a
// transicao de fato (Enter/Espaco/clique numa pill/categoria/Voltar).
//
// Devolve a string RML completa (com <style> embutido). NAO inclui @font-face
// (o CHAMADOR injeta, mesma receita de string-replace de write_baked_cockpit_rml/
// write_live_cockpit_rml em battle_preview.cpp) nem resolve caminhos de asset.
[[nodiscard]] std::string build_system_menu_rml(
    const SystemMenuState& state, const gus::app::i18n::Translator& translator,
    int pressed_index = -1);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_SYSTEM_MENU_RML_HPP
