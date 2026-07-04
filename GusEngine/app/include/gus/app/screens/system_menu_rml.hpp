// gus/app/screens/system_menu_rml.hpp
//
// RML/RCSS do MENU DE SISTEMA (pausa + config de som), MENU-PAUSA-CONFIG-SOM
// (M7-COSTURA). Traduz o mock APROVADO
// (docs/design/mockups/01-menu-sistema-proposta-a-console-centralizado.html,
// "Console Centralizado") pro RCSS do glintfx, no MESMO estilo de autoria de
// gus/app/screens/battle_preview.cpp::load_cockpit_rml() (RML/RCSS como string
// C++, efeitos nativos do backend GL3: vertical-gradient/radial-gradient/
// box-shadow-glow/border-radius/animation).
//
// DUAS TELAS num UNICO documento (data-if, mesmo padrao de opening/combat do
// cockpit): "pause" (3 verb-pills: Continuar/Configuracoes/Sair) e "config" (2
// sliders: Musica/SFX + Voltar). O SystemMenuState (system_menu.hpp) e a UNICA
// fonte de verdade da selecao/volume - esta funcao SO GERA a string RML a partir
// dele (nao decide nada).
//
// >>> DIVERGENCIA CONHECIDA DO MOCK (reportada ao lider, NAO resolvida em
// silencio - ver o relatorio da tarefa MENU-PAUSA-CONFIG-SOM): o mock desenha o
// "no" (thumb) do slider como um HEXAGONO (SVG <polygon>, 6 lados, contorno
// cyan + preenchimento escuro + drop-shadow). RmlUi 6.3 / glintfx NAO tem
// decorator de poligono/vetor arbitrario (so retangulo com border-radius,
// elipse/circulo via radial-gradient, sem clip-path nem SVG inline - conferido
// no fonte vendorizado, Source/Core/Decorator*.cpp). Esta funcao usa um CIRCULO
// (native, border-radius + box-shadow glow, MESMA cor/tamanho/glow do mock) como
// place-holder EXPLICITO ate o lider decidir entre: (a) aceitar o circulo como
// forma final (baixo esforco, ja funciona); (b) encomendar um asset PNG do
// hexagono (como o medalhao .mono/.disc do brasao em load_cockpit_rml - decorator:
// image(), MESMA tecnica ja usada e aprovada no cockpit) e trocar so a regra
// #slider-node abaixo; (c) aceitar uma outra forma nativa (ex.: losango via
// transform: rotate(45deg) num quadrado). NENHUMA das 3 foi escolhida por mim -
// o circulo abaixo e so pra o menu ficar FUNCIONALMENTE testavel (drag/clique)
// enquanto a decisao de forma final nao vem.
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
// Devolve a string RML completa (com <style> embutido). NAO inclui @font-face
// (o CHAMADOR injeta, mesma receita de string-replace de write_baked_cockpit_rml/
// write_live_cockpit_rml em battle_preview.cpp) nem resolve caminhos de asset
// (moldura latao reusa a MESMA imagem de moldura_carta_frame.png do cockpit, se
// o lider optar por manter a moldura de latao identica a carta-retrato - ver
// comentario da regra .corner abaixo).
[[nodiscard]] std::string build_system_menu_rml(
    const SystemMenuState& state, const gus::app::i18n::Translator& translator);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_SYSTEM_MENU_RML_HPP
