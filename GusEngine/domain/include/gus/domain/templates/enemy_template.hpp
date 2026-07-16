// gus/domain/templates/enemy_template.hpp
//
// Template imutavel de inimigo. Dado PURO. Portado de
// engine/foundation/data/EnemyTemplate.cs (sealed record + enum BrainKind + Validate()).
//
// O brain e modelado como o enum BrainKind (referencia tipada a fabrica de brain), NAO
// como a classe IEnemyBrain (nao serializavel). A construcao do brain concreto a partir
// do BrainKind e responsabilidade do CharacterRepository futuro (FORA deste marco).
//
// Invariantes (fail-fast, validate()): id nao vazio; max_hp>0; atk/def/spd>=0;
// base_deck sem id vazio.
//
// POCO puro, ZERO Qt (invariante de domain/, engine-design.md secao 2).
//
// Cross-ref: engine/foundation/data/EnemyTemplate.cs, combat.md secao 13/17, ADR-006.

#ifndef GUS_DOMAIN_TEMPLATES_ENEMY_TEMPLATE_HPP
#define GUS_DOMAIN_TEMPLATES_ENEMY_TEMPLATE_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "gus/domain/templates/card_family.hpp"

namespace gus::domain::templates {

// Identidade tipada da AI de um inimigo (combat.md secao 13). Mapeia para uma classe
// IEnemyBrain concreta no repositorio futuro. A ORDEM e contrato binario (ordinal 0..1).
enum class BrainKind : std::uint32_t {
    // Deterministico, roteiro fixo, intent 100% legivel (Trash). secao 13.
    Scripted = 0,
    // Pontuacao de utilidade por acao (Elite / Mini-boss). Jogo posterior. secao 13.
    Utility = 1,
};

// Numero de valores canonicos de BrainKind (0..kBrainKindCount-1). Usado pela validacao
// de ordinal (A1): rejeita brain fora do dominio no validate(). BrainKind permanece
// definido aqui (conceito so-de-template; a FSM/brains do combate nao o usam).
inline constexpr std::uint32_t kBrainKindCount = 2;

// Natureza DIEGETICA do inimigo (MODOS-MORTE Fase 0, docs/design/mecanicas/
// modos-morte.md §3.1): decide o LOCAL de respawn na morte de Gus no modo Dificil
// (criatura -> Selve Sombria; humano -> casa destruida) - a LEITURA/consumo dessa
// decisao no combate e fase futura (§6 Fase 3), aqui e so o dado persistido no
// template. Ordinal EXPLICITO (contrato binario, mesmo padrao de BrainKind).
enum class EnemyKind : std::uint32_t {
    Creature = 0,  // default - a maioria dos encontros do VS e fauna da Selve
    Human = 1,
};

// Numero de valores canonicos de EnemyKind (0..kEnemyKindCount-1). Usado pela
// validacao de ordinal (mesmo padrao de kBrainKindCount acima).
inline constexpr std::uint32_t kEnemyKindCount = 2;

// Template imutavel de inimigo. Source de stats + identidade de AI (secao 13/17).
struct EnemyTemplate {
    // Identidade estavel (chave de repositorio/i18n). Invariante: nao vazia.
    std::string id;

    // HP maximo. Invariante: > 0.
    int max_hp = 0;

    // Ataque base. TBD em secao 17; default 0. Invariante: >= 0.
    int atk = 0;

    // Defesa base. Invariante: >= 0.
    int def = 0;

    // Velocidade. TBD em secao 17; default 0. Invariante: >= 0.
    int spd = 0;

    // Familia da roda de fraqueza secao 6.
    CardFamily family = CardFamily::Eletrico;

    // Identidade da AI (secao 13). Resolvida em IEnemyBrain concreto pelo repositorio futuro.
    BrainKind brain = BrainKind::Scripted;

    // true = boss/mini-boss (bloqueia Flee secao 14). Default false.
    bool is_boss = false;

    // Deck base do inimigo: IDs de carta. Lista vazia e valida. Sem id vazio.
    std::vector<std::string> base_deck;

    // Natureza diegetica (MODOS-MORTE Fase 0, ver EnemyKind acima). Campo NOVO no
    // FINAL da struct (nao no meio) de proposito: preserva os aggregate-inits
    // posicionais existentes de canonical_templates.cpp/testes (9 campos
    // explicitos continuam validos, este 10o usa o default).
    EnemyKind kind = EnemyKind::Creature;

    // Tag "comando central" (Mises/Calc-Edge, CARD-ENGINE-MANIFESTO item 9,
    // EffectKind::ApEfficiency face 2, combat_enums.hpp): inimigos marcados sofrem atraso
    // de fila + erro de mira quando a party porta a passiva Mises equipada. Espelhado em
    // CombatActor::central_command() (combat_actor.hpp) - o wiring template->actor fica
    // pro sitio que constroi o CombatActor a partir deste template (nao existe ainda em
    // producao; ver AMB-10, _EFEITOS-ESCOLHIDOS.md). Campo NOVO no FINAL da struct (mesmo
    // padrao de `kind` acima) - preserva os aggregate-inits posicionais existentes
    // (canonical_templates.cpp/testes com 10 campos explicitos continuam validos, este 11o
    // usa o default false). Curadoria de QUAIS inimigos canônicos levam a tag e decisao de
    // design/lore do criador, FORA de escopo desta implementacao - o bestiario canonico
    // fica intocado; so templates de TESTE ganham a tag.
    bool central_command = false;

    // Igualdade por valor (semantica de record do C#).
    [[nodiscard]] bool operator==(const EnemyTemplate&) const = default;

    // Valida os invariantes (fail-fast). Chamado pelo serializer no load.
    // Lanca std::invalid_argument.
    void validate() const;
};

}  // namespace gus::domain::templates

#endif  // GUS_DOMAIN_TEMPLATES_ENEMY_TEMPLATE_HPP
