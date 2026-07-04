// gus/app/screens/ui_hover.hpp
//
// COCKPIT-SFX-HOVER-CLIQUE: fatia PURA/testavel do "som de hover" de botoes de UI,
// GENERALIZADA a partir das duas funcoes POCO que o MENU DE SISTEMA ja usava
// (system_menu_hover_index / system_menu_hover_entered_new_item, ver
// gus/app/screens/system_menu.hpp). O VISUAL do hover e sempre NATIVO do glintfx
// (RCSS :hover) - so o SOM precisa de logica propria de edge-detection (tocar so
// quando o item hovered MUDA, nao a cada frame parado sobre o mesmo botao).
//
// POR QUE GENERALIZAR (reuso > duplicacao): o COCKPIT DA BATALHA (pills de verbo
// SCAN/ATACAR/... em battle_preview.cpp) precisava do MESMO comportamento do menu.
// As duas funcoes do menu eram quase reusaveis, mas a de indice era acoplada ao
// SystemMenuState (screen -> count). Aqui a versao GENERICA recebe o `count`
// diretamente (qualquer tela com N botoes: menu OU cockpit). O menu passou a
// DELEGAR pra estas (ver system_menu.cpp) - zero duplicacao, o teste do menu
// continua verde e um teste proprio (ui_hover_test.cpp) trava esta camada.
//
// 100% testavel SEM glintfx/GL/janela: a QUERY GL-heavy (get_element_box) fica no
// CHAMADOR, que converte a geometria pro tipo local UiHoverBox e chama aqui - o
// MESMO racional POCO documentado em system_menu.hpp.

#ifndef GUS_APP_SCREENS_UI_HOVER_HPP
#define GUS_APP_SCREENS_UI_HOVER_HPP

namespace gus::app::screens {

// Caixa retangular MINIMA pro hit-test de hover - MESMO layout de campos que
// glintfx::ElementBox (found/x/y/w/h) e que SystemMenuHoverBox, mas um tipo PROPRIO
// pra esta camada seguir testavel sem incluir glintfx. O CHAMADOR converte
// glintfx::ElementBox -> UiHoverBox campo a campo (trivial) antes de chamar.
struct UiHoverBox {
    bool found = false;
    float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
};

// HOVER (mouse, SEM clique): dado o ponto (mouse_x,mouse_y, espaco-janela), as
// caixas dos `count` botoes navegaveis da tela ATUAL e o `count`, devolve o INDICE
// (0..count-1) do PRIMEIRO botao cuja caixa contem o ponto, ou -1 se nenhum bater
// (ou count<=0, ou boxes nulo). box.found=false conta como "fora" (MESMO contrato
// do hit-test de clique). PURA: so decisao geometrica, sem GL/janela.
[[nodiscard]] int ui_hover_index(float mouse_x, float mouse_y,
                                 const UiHoverBox* boxes, int count) noexcept;

// EDGE-DETECT: true quando o hover ENTROU num item NOVO e VALIDO (current_index>=0
// && current_index != previous_index) - o chamador usa isto pra tocar o SFX de
// hover SO na TRANSICAO. Parado sobre o MESMO item (current==previous) nao redispara
// (evita o som repetir a cada frame/MouseMove parado). SAIR pra fora de qualquer
// botao (current_index==-1) tambem NAO dispara (so ENTRAR soa, como o "OnPointerEnter"
// das engines de UI). Sair e voltar pro MESMO item (com -1 no meio) redispara - a
// "memoria" e so o ULTIMO indice visto, nao um historico.
[[nodiscard]] bool ui_hover_entered_new_item(int previous_index,
                                             int current_index) noexcept;

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_UI_HOVER_HPP
