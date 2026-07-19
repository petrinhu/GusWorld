// gus/domain/hardware/card_provenance.hpp
//
// PECA de proveniencia fisica de uma carta possuida (ATOM-1, decomposicao atomica
// de CardPhysicalState em pecas componiveis em modulos estreitos, generalizando
// ADR-019 ao nivel de modulo; CARDS-HARDWARE-ENGINE incremento 1, CARDS-HW-1):
// origem da copia fisica (ROM de fabrica / EPROM homebrew / clone pirata) + o
// conector RSB DERIVADO da origem. POCO puro, ZERO SDL/GL/RmlUi/IO (invariante de
// domain/, engine-design.md secao 2).
//
// hardware/ NAO inclui combat/ nem deck/ (gate de camadas; so cards/ e permitido,
// pra CardTier em hardware_class.hpp - este header nao precisa nem disso).
//
// Cross-ref: docs/design/mecanicas/cartas-hardware-pirataria-energia.md secao 2/3;
//            docs/design/mecanicas/cartas-spec-dados.md secao 5/6;
//            gus/domain/deck/card_hardware.hpp (fachada agregada - CardPhysicalState
//            HERDA desta peca); card_hardware_test.cpp (oraculo do agregado).

#ifndef GUS_DOMAIN_HARDWARE_CARD_PROVENANCE_HPP
#define GUS_DOMAIN_HARDWARE_CARD_PROVENANCE_HPP

#include <cstdint>

namespace gus::domain::hardware {

// Origem fisica da copia (cartas-hardware-pirataria-energia.md secao 2/3).
enum class CardOrigin : std::uint32_t {
    OriginalRom = 0,    // ROM de fabrica, nao regravavel, "queima na hora" se adulterada
    HomebrewEprom = 1,  // EPROM vazia gravada em bancada de mercado negro, regravavel-ate-queimar
    PirateClone = 2,    // clone pirata, ROM travada apos gravar 1x
};

// Numero de valores canonicos de CardOrigin (0..kCardOriginCount-1). Mesmo padrao
// de kDifficultyLevelCount (save_data.hpp) - defesa em profundidade no validate().
inline constexpr std::uint32_t kCardOriginCount = 3;

// Conector RSB - DERIVADO de CardOrigin (cartas-hardware-pirataria-energia.md
// secao 3, 1:1: Original=nenhum, Homebrew=externo visivel, Pirata=interno
// escondido). NUNCA armazenado - existe so como tipo de retorno de connector_of().
enum class RsbConnector : std::uint32_t {
    None = 0,
    ExternalVisible = 1,
    InternalHidden = 2,
};

// Proveniencia fisica MUTAVEL de uma copia especifica de carta. Default =
// CardProvenance{} = "ROM original legitima" - o estado mais SEGURO/generoso, tanto
// para uma instancia nova quanto para um save V6 migrado (cartas-spec-dados.md
// secao 5.2, "zero e seguro").
struct CardProvenance {
    CardOrigin origin = CardOrigin::OriginalRom;

    [[nodiscard]] bool operator==(const CardProvenance&) const = default;

    // Fail-fast (defesa em profundidade, mesmo padrao de DifficultyLevel::validate()
    // em save_data.cpp): um payload selado mas schema-divergente (ordinal fora do
    // dominio canonico do enum) nao e aceito silenciosamente. Lanca
    // std::invalid_argument.
    void validate() const;
};

// Conector RSB - DERIVADO 1:1 de origin, nunca armazenado.
[[nodiscard]] RsbConnector connector_of(CardOrigin) noexcept;

}  // namespace gus::domain::hardware

#endif  // GUS_DOMAIN_HARDWARE_CARD_PROVENANCE_HPP
