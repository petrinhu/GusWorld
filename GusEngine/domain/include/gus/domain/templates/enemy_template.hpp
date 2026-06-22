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

    // Igualdade por valor (semantica de record do C#).
    [[nodiscard]] bool operator==(const EnemyTemplate&) const = default;

    // Valida os invariantes (fail-fast). Chamado pelo serializer no load.
    // Lanca std::invalid_argument.
    void validate() const;
};

}  // namespace gus::domain::templates

#endif  // GUS_DOMAIN_TEMPLATES_ENEMY_TEMPLATE_HPP
