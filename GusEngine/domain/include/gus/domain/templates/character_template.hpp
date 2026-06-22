// gus/domain/templates/character_template.hpp
//
// Template imutavel de personagem jogavel (party). Dado PURO. Portado de
// engine/foundation/data/CharacterTemplate.cs (sealed record + Validate()).
//
// NAO e o estado de combate (CombatActor, mutavel, derivado por um
// CharacterRepository futuro, FORA deste marco por depender de turn_combat).
//
// Gus = "compilador universal" (combat.md secao 6.1): a universalidade e a FLAG
// is_universal_compiler (default false; Gus=true), NAO um valor de CardFamily. O
// campo family continua valido (familia ancora / cor de jogo); a flag e ortogonal.
//
// Invariantes (fail-fast, validate()): id nao vazio; max_hp>0; atk/def/spd>=0;
// base_deck sem id vazio (lista vazia e valida). C++ nao tem null para vector, logo
// o caso "BaseDeck null" do C# nao existe aqui.
//
// POCO puro, ZERO Qt (invariante de domain/, engine-design.md secao 2). Struct de
// campos publicos (semantica de record por valor); validacao explicita em validate()
// para espelhar o fail-fast do C# no load do serializer.
//
// Cross-ref: engine/foundation/data/CharacterTemplate.cs, combat.md secao 17/6.1, ADR-006.

#ifndef GUS_DOMAIN_TEMPLATES_CHARACTER_TEMPLATE_HPP
#define GUS_DOMAIN_TEMPLATES_CHARACTER_TEMPLATE_HPP

#include <string>
#include <vector>

#include "gus/domain/templates/card_family.hpp"

namespace gus::domain::templates {

// Template imutavel de personagem jogavel. Source de stats base (combat.md secao 17).
struct CharacterTemplate {
    // Identidade estavel (chave de save/i18n/repositorio). Invariante: nao vazia.
    std::string id;

    // HP maximo. Invariante: > 0.
    int max_hp = 0;

    // Ataque base (formula de dano secao 11). Invariante: >= 0.
    int atk = 0;

    // Defesa base. Invariante: >= 0.
    int def = 0;

    // Velocidade (posicao na fila de iniciativa secao 4). Invariante: >= 0.
    int spd = 0;

    // Familia-ancora da roda de fraqueza secao 6. Ortogonal a is_universal_compiler.
    CardFamily family = CardFamily::Eletrico;

    // true = compilador universal (Gus, secao 6.1): deck multi-familia + defesa neutra.
    // Default false (companions). Semantica consumida pelo combate (futuro).
    bool is_universal_compiler = false;

    // Deck base: IDs de carta (Card.Id), resolvidos por um CardRepository futuro.
    // Lista vazia e valida. Sem id vazio.
    std::vector<std::string> base_deck;

    // Igualdade por valor (semantica de record do C#).
    [[nodiscard]] bool operator==(const CharacterTemplate&) const = default;

    // Valida os invariantes (fail-fast). Chamado pelo serializer no load (defesa
    // contra .gdt malformado, mesmo apos HMAC valido: schema drift e possivel).
    // Lanca std::invalid_argument (espelha ArgumentException/ArgumentOutOfRange do C#).
    void validate() const;
};

}  // namespace gus::domain::templates

#endif  // GUS_DOMAIN_TEMPLATES_CHARACTER_TEMPLATE_HPP
