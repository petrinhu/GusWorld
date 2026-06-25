// gus/app/src/screens/battle_menu.cpp
//
// Implementacao do modelo PURO do menu de verbos (ver header). Custos de AP, mapeamento
// verbo->acao, navegacao com wrap, habilitacao por AP e layout em coluna. Sem SDL.

#include "gus/app/screens/battle_menu.hpp"

namespace gus::app::screens {

namespace {
using gus::domain::combat::CombatActionType;
}  // namespace

int verb_ap_cost(BattleVerb verb) noexcept {
    switch (verb) {
        case BattleVerb::Scan:     return 1;
        case BattleVerb::Gambito:  return 1;  // Gambito-Prever (par.5); Reordenar = 2 AP
        case BattleVerb::Atacar:   return 1;
        case BattleVerb::Defender: return 1;
        case BattleVerb::Compilar: return 0;  // so abre o overlay (incremento 4)
        case BattleVerb::Flee:     return 1;
    }
    return 1;
}

std::string_view verb_key(BattleVerb verb) noexcept {
    switch (verb) {
        case BattleVerb::Scan:     return "scan";
        case BattleVerb::Gambito:  return "gambito";
        case BattleVerb::Atacar:   return "atacar";
        case BattleVerb::Defender: return "defender";
        case BattleVerb::Compilar: return "compilar";
        case BattleVerb::Flee:     return "flee";
    }
    return "atacar";
}

CombatActionType verb_action_type(BattleVerb verb) noexcept {
    switch (verb) {
        case BattleVerb::Scan:     return CombatActionType::Scan;
        case BattleVerb::Gambito:  return CombatActionType::GambitPredict;
        case BattleVerb::Atacar:   return CombatActionType::Attack;
        case BattleVerb::Defender: return CombatActionType::Defend;
        case BattleVerb::Compilar: return CombatActionType::Pass;  // sentinela (UI-only)
        case BattleVerb::Flee:     return CombatActionType::Flee;
    }
    return CombatActionType::Attack;
}

void BattleMenu::refresh(int available_ap) noexcept {
    for (int i = 0; i < kBattleVerbCount; ++i) {
        const BattleVerb v = static_cast<BattleVerb>(i);
        // Compilar (custo 0) sempre habilitado; os demais exigem AP suficiente.
        enabled_[static_cast<std::size_t>(i)] = verb_ap_cost(v) <= available_ap;
    }
}

void BattleMenu::move(int delta) noexcept {
    // Wrap nos dois sentidos sobre os 6 itens (navega inclusive os desabilitados).
    int n = selected_ + delta;
    n %= kBattleVerbCount;
    if (n < 0) {
        n += kBattleVerbCount;
    }
    selected_ = n;
}

std::array<MenuItem, kBattleVerbCount> BattleMenu::layout(
    const Rect& menu_zone) const noexcept {
    std::array<MenuItem, kBattleVerbCount> items{};
    // Coluna unica: 6 itens empilhados, retangulos iguais, do topo da zona pra baixo.
    // Pequena margem interna pra nao colar nas bordas do painel.
    constexpr float kPad = 2.0f;
    const float inner_x = menu_zone.x + kPad;
    const float inner_w = menu_zone.w - 2.0f * kPad;
    const float inner_top = menu_zone.y + kPad;
    const float inner_h = menu_zone.h - 2.0f * kPad;
    const float item_h = inner_h / static_cast<float>(kBattleVerbCount);

    for (int i = 0; i < kBattleVerbCount; ++i) {
        items[static_cast<std::size_t>(i)] = MenuItem{
            static_cast<BattleVerb>(i),
            enabled_[static_cast<std::size_t>(i)],
            Rect{inner_x, inner_top + static_cast<float>(i) * item_h, inner_w,
                 item_h}};
    }
    return items;
}

}  // namespace gus::app::screens
