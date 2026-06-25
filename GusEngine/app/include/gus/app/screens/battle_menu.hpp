// gus/app/screens/battle_menu.hpp
//
// MODELO PURO do menu de verbos comando-first da BattleScreen (M5, incremento 3): POCO
// 100% testavel SEM SDL. Define os 6 verbos (battle-screen.md par.1 decisao 3), o
// INDICE selecionado, quais verbos estao HABILITADOS pelo AP/estado do ator ativo, e o
// layout dos itens (retangulos em px logico). A navegacao (cima/baixo/confirmar) e
// abstrata: o viewer traduz tecla/controller -> intent e chama os metodos aqui.
//
// COMANDO-FIRST: so o ATOR ATIVO mostra o menu (CTB por-ator). As cartas/overlay NAO
// aparecem aqui (incremento 4): COMPILAR e so um verbo que, por ora, sinaliza "abriria
// o overlay" (o controller loga). Os custos de AP seguem combat.md par.5.
//
// SEM FONTE (decisao do criador, mantida): os verbos sao caixas-placeholder COLORIDAS
// (1 cor por verbo) - NAO ha set de icones de verbo no projeto (so intent_atacar/
// intent_defender existem; Scan/Gambito/COMPILAR/Flee nao tem). Reportado ao
// coordenador. O modelo ja expoe um "icon hint" por verbo pra trocar caixa->icone sem
// mexer no layout quando o set de icones de verbo existir.
//
// Cross-ref: docs/design/mecanicas/battle-screen.md par.1/2; combat.md par.5 (AP);
//            gus/app/screens/battle_layout.hpp (zona do painel do ator);
//            gus/domain/combat/combat_enums.hpp (CombatActionType).

#ifndef GUS_APP_SCREENS_BATTLE_MENU_HPP
#define GUS_APP_SCREENS_BATTLE_MENU_HPP

#include <array>
#include <string_view>

#include "gus/core/spatial/camera_clamp.hpp"  // Rect
#include "gus/domain/combat/combat_enums.hpp"  // CombatActionType

namespace gus::app::screens {

using gus::core::spatial::Rect;

// Os 6 verbos do menu comando-first (ordem de exibicao). Mapeiam pra CombatActionType
// (Gambito = GambitPredict no slice; COMPILAR e um verbo de UI que abre o overlay no
// incremento 4, sem CombatActionType proprio).
enum class BattleVerb : int {
    Scan = 0,
    Gambito = 1,
    Atacar = 2,
    Defender = 3,
    Compilar = 4,
    Flee = 5,
};

inline constexpr int kBattleVerbCount = 6;

// Custo de AP de um verbo (combat.md par.5). Scan/Atacar/Defender/Flee/Gambito-Prever =
// 1 AP; Compilar = 0 aqui (so abre o overlay; o custo real vem ao disparar a pipeline,
// incremento 4). Determinismo total.
[[nodiscard]] int verb_ap_cost(BattleVerb verb) noexcept;

// Nome curto estavel do verbo (chave/diagnostico; NAO e texto de UI traduzido). Util
// pra log/teste e pra quando a fonte entrar (vira a label via tr()).
[[nodiscard]] std::string_view verb_key(BattleVerb verb) noexcept;

// CombatActionType correspondente ao verbo (pra montar a CombatAction real). Compilar
// nao tem tipo de acao (retorna Pass como sentinela; o controller trata Compilar a
// parte, sem submeter acao ao motor no incremento 3).
[[nodiscard]] gus::domain::combat::CombatActionType verb_action_type(
    BattleVerb verb) noexcept;

// Item do menu ja resolvido: o verbo, se esta HABILITADO (AP suficiente) e seu
// retangulo em px logico.
struct MenuItem {
    BattleVerb verb = BattleVerb::Scan;
    bool enabled = true;
    Rect rect;
};

// Estado do menu de verbos do ator ativo. Guarda o indice selecionado e recalcula
// habilitacao/layout a partir do AP corrente. POCO: sem SDL, deterministico.
class BattleMenu {
public:
    BattleMenu() = default;

    // Recalcula a habilitacao dos verbos pelo AP disponivel do ator ativo (chamado a
    // cada turno/acao). Mantem o indice selecionado dentro dos limites; se o item
    // selecionado ficou desabilitado, NAO move sozinho (o jogador ve e escolhe outro;
    // o confirm de um item desabilitado e no-op no controller).
    void refresh(int available_ap) noexcept;

    // Move a selecao (delta tipicamente -1 cima / +1 baixo). Faz WRAP nos extremos.
    // Pula itens? NAO: navega por TODOS os 6 (inclusive desabilitados, que ficam
    // visiveis acinzentados); o confirm e que respeita o enabled.
    void move(int delta) noexcept;

    // Indice selecionado (0..kBattleVerbCount-1).
    [[nodiscard]] int selected_index() const noexcept { return selected_; }
    // Verbo selecionado.
    [[nodiscard]] BattleVerb selected_verb() const noexcept {
        return static_cast<BattleVerb>(selected_);
    }
    // true se o verbo selecionado esta habilitado (AP suficiente).
    [[nodiscard]] bool selected_enabled() const noexcept {
        return enabled_[static_cast<std::size_t>(selected_)];
    }
    // Habilitacao de um verbo (por indice de enum).
    [[nodiscard]] bool is_enabled(BattleVerb verb) const noexcept {
        return enabled_[static_cast<std::size_t>(verb)];
    }

    // Layout dos 6 itens DENTRO da zona do menu (faixa do painel do ator). A zona vem
    // do battle_layout (active_panel_rect). Distribui os itens em coluna unica
    // (empilhados), retangulos iguais, do topo da zona pra baixo. Pura/deterministica.
    [[nodiscard]] std::array<MenuItem, kBattleVerbCount> layout(
        const Rect& menu_zone) const noexcept;

private:
    int selected_ = static_cast<int>(BattleVerb::Atacar);  // Atacar como default util
    std::array<bool, kBattleVerbCount> enabled_{true, true, true, true, true, true};
};

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_MENU_HPP
