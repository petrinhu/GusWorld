// gus/domain/hardware/hardware_class.hpp
//
// Classificacao DERIVADA de hardware pra lookup de config (ATOM-1, decomposicao
// atomica ao nivel de modulo, generalizando ADR-019; CARDS-HARDWARE-ENGINE
// incremento 1, CARDS-HW-1). HardwareClass NAO e um campo armazenado em nenhuma das
// 3 pecas de estado (CardProvenance/BatteryState/IntegrityState) - e a CHAVE
// derivada de (CardTier do catalogo, CardOrigin, mimics_special_id da carta), por
// isso mora num modulo PROPRIO ao lado do vocabulario de carta (gus::domain::cards::
// CardTier), nao dentro de nenhuma peca de estado. POCO puro, ZERO SDL/GL/RmlUi/IO
// (invariante de domain/, engine-design.md secao 2).
//
// hardware/ NAO inclui combat/ nem deck/ (gate de camadas); so cards/ e permitido,
// exatamente pra CardTier aqui.
//
// Cross-ref: docs/design/mecanicas/cartas-spec-dados.md secao 5.3;
//            docs/design/mecanicas/cartas-numeros-proposta.md secao 1a/3;
//            gus/domain/cards/card_enums.hpp (CardTier);
//            gus/domain/hardware/card_provenance.hpp (CardOrigin);
//            gus/domain/deck/card_hardware.hpp (fachada, re-exporta pra deck::);
//            gus/domain/deck/card_hardware_constants.hpp (battery_capacity_for,
//            cruza HardwareClass com save::DifficultyLevel - fora deste modulo,
//            umbrella deck/, pois hardware/ nao pode incluir save/).

#ifndef GUS_DOMAIN_HARDWARE_HARDWARE_CLASS_HPP
#define GUS_DOMAIN_HARDWARE_HARDWARE_CLASS_HPP

#include <array>
#include <cstddef>
#include <cstdint>

#include "gus/domain/cards/card_enums.hpp"
#include "gus/domain/hardware/card_provenance.hpp"

namespace gus::domain::hardware {

// Classe de hardware para lookup de config (capacidade de bateria por dificuldade,
// % de contaminacao - cartas-numeros-proposta.md secao 1a/3). NAO e um campo
// armazenado: e a CHAVE derivada de (CardTier do catalogo, CardOrigin,
// mimics_special_id da carta) - ver hardware_class_of() abaixo. 5 classes porque
// "pirata especial" (clone-falso) tem risco de contaminacao PROPRIO (8%), distinto
// de "pirata comum" (21%), mesmo os dois sendo CardTier::Comum no catalogo.
enum class HardwareClass : std::uint32_t {
    ComumOriginal = 0,
    HomebrewEprom = 1,
    PirataComum = 2,
    PirataEspecialFalso = 3,  // clone-falso - Card::mimics_special_id preenchido
    EspecialSelada = 4,        // CardTier::Especial/Super real (sempre CardOrigin::OriginalRom)
};

// Classe de hardware pra lookup de config (capacidade de bateria / % de
// contaminacao). catalog_tier vem de um TierLookup (mesmo padrao de
// card_collection.hpp); mimics_special vem de Card::mimics_special_id.has_value()
// do catalogo. CardTier::Especial OU CardTier::Super (classe PROTEGIDA, mesmo
// agrupamento de CardCollection::guard_protected_tier, deck-mao-sistema.md secao 7
// inv.9) SEMPRE mapeia para EspecialSelada, independente de origin/mimics_special -
// a validacao de origin==OriginalRom p/ Especial e invariante SEPARADO (secao 6.5
// do doc-fonte); esta funcao nao valida, so classifica.
[[nodiscard]] HardwareClass hardware_class_of(gus::domain::cards::CardTier catalog_tier,
                                               CardOrigin origin,
                                               bool mimics_special) noexcept;

// ---- Contaminacao por virus (%) - cartas-numeros-proposta.md secao 3 -------------
//
// FECHADO PELO LIDER 2026-07-18: fixo em TODO modo de dificuldade (nao escala, ao
// contrario da bateria - por isso vive aqui, indexada SO por HardwareClass, e nao
// no umbrella deck/ junto de battery_capacity_for que tambem cruza com
// DifficultyLevel). Ordem ordinal de HardwareClass. //PLAYTEST
inline constexpr std::array<std::uint8_t, 5> kContaminationPercentTable = {
    1,   // ComumOriginal
    55,  // HomebrewEprom
    21,  // PirataComum
    8,   // PirataEspecialFalso
    0,   // EspecialSelada (canon fixo - so gatilho narrativo, nunca RNG)
};

// % de risco de contaminacao na aquisicao (loot/compra/upload homebrew), por
// HardwareClass. Table lookup puro - o chamador SEMPRE passa um HardwareClass ja
// resolvido por hardware_class_of() (dominio fechado por construcao, 5 valores).
[[nodiscard]] inline std::uint8_t contamination_percent_for(
    HardwareClass hardware_class) noexcept {
    return kContaminationPercentTable[static_cast<std::size_t>(hardware_class)];
}

}  // namespace gus::domain::hardware

#endif  // GUS_DOMAIN_HARDWARE_HARDWARE_CLASS_HPP
