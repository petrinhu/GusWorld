// GusEngine/app/tests/ui_box_assertions.hpp
//
// Helpers PUROS (sem GL, sem depender de nenhum estado de tela especifico) pra
// verificacao de POSICIONAMENTO/INTERACAO de telas glintfx em testes Catch2.
// Nasceu do harness da tela de Salvar/Carregar (SAVE-LOAD-UI, save_load_menu_
// interaction_test.cpp - bugs 2/8 de slot encostando na scrollbar, "Voltar
// morto" pro mouse) mas e GENERICO o bastante pra qualquer tela que use
// glintfx::UiLayer::get_element_box - reusar em vez de reescrever a cada nova
// tela (exigencia do lider: harness reutilizavel).
//
// Opera sobre glintfx::ElementBox (o MESMO tipo que UiLayer::get_element_box
// devolve, {x,y,w,h,found}) - nao precisa de contexto GL pra COMPILAR/RODAR (so
// os TESTES que alimentam os boxes de verdade precisam de GL real, ver o
// bootstrap em save_load_menu_interaction_test.cpp).

#ifndef GUS_APP_TESTS_UI_BOX_ASSERTIONS_HPP
#define GUS_APP_TESTS_UI_BOX_ASSERTIONS_HPP

#include <glintfx/element_box.hpp>

namespace gus::app::testing {

// Espaco horizontal LIVRE entre `left` e `right` (espera-se `right` a DIREITA de
// `left`, mesma convencao usada pra "slot x scrollbar"): right.x - (left.x +
// left.w). Negativo/zero = OVERLAP ou encostados (o bug 2/8 original: slot.right
// == scrollbar.left, gap==0). Devolve -1e9f (sentinela obviamente invalido, NUNCA
// um gap real) se `left` ou `right` nao foram encontrados - o CHAMADOR deve
// REQUIRE(box.found) separadamente ANTES de chamar isto (fail-fast explicito no
// teste, nao escondido aqui).
[[nodiscard]] inline float horizontal_gap(const glintfx::ElementBox& left,
                                           const glintfx::ElementBox& right) noexcept {
    if (!left.found || !right.found) return -1e9f;
    return right.x - (left.x + left.w);
}

// true se `box` e um alvo de CLIQUE valido: encontrado, area > 0, e inteiramente
// DENTRO da janela (0..window_w, 0..window_h). Pega tres classes de bug numa so
// checagem: "elemento sumiu do layout" (found=false), "elemento com area zero"
// (ex.: flex colapsado pra ~0px de largura - MESMA classe do BUG-A documentado
// em system_menu_loop.cpp), e "elemento fora da viewport visivel" (nao deveria
// ser clicavel de fato, mesmo que a caixa exista).
[[nodiscard]] inline bool box_hittable(const glintfx::ElementBox& box, float window_w,
                                        float window_h) noexcept {
    if (!box.found) return false;
    if (box.w <= 0.0f || box.h <= 0.0f) return false;
    if (box.x < 0.0f || box.y < 0.0f) return false;
    if (box.x + box.w > window_w || box.y + box.h > window_h) return false;
    return true;
}

// true se `inner` esta INTEIRAMENTE dentro do recorte VERTICAL visivel de
// `container` (ambos found) - util pra linhas de listas roláveis: uma linha
// rolada pra fora da vista nao deveria contar como "visivel/clicavel" mesmo que
// a caixa REAL ainda exista no layout. Generalizacao SEM estado de tela do
// mesmo racional de controls_row_visible_in_list (system_menu.hpp).
[[nodiscard]] inline bool box_within_container(const glintfx::ElementBox& inner,
                                                const glintfx::ElementBox& container) noexcept {
    if (!inner.found || !container.found) return false;
    return inner.y >= container.y && (inner.y + inner.h) <= (container.y + container.h);
}

}  // namespace gus::app::testing

#endif  // GUS_APP_TESTS_UI_BOX_ASSERTIONS_HPP
