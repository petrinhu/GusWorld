// gus/domain/deck/card_hardware.hpp
//
// Camada FISICA de uma carta possuida (origem ROM/EPROM/pirata, bateria CR2032,
// integridade/virus oculto, conector RSB) - CARDS-HARDWARE-ENGINE incremento 1
// (CARDS-HW-1, TODO.md). POCO puro, ZERO SDL/GL/RmlUi/IO (invariante de domain/,
// engine-design.md secao 2). Fonte de verdade: docs/design/mecanicas/
// cartas-spec-dados.md secao 5 (enums + CardPhysicalState + funcoes puras
// derivadas) + secao 6 (invariantes) + secao 7 (clone-falso).
//
// ESCOPO deste incremento (secao 1 do doc-fonte, "fora do escopo" explicito):
//   - Rolagem de contaminacao na aquisicao (loot/compra/upload homebrew) - onda
//     FUTURA de deck_transactions.hpp (acquire-com-origem).
//   - Gate de bateria-vazia/is_burned_out impedindo cast em combate, propagacao de
//     worm, payload de virus (logic bomb/backdoor/etc) - onda FUTURA do
//     gameplay_engineer/executor techMagic.
//   - UI/RunaDex, gate "nao vaza is_infected antes de is_diagnosed" (contrato de
//     FRONTEIRA nomeado em secao 6 inv.3 do doc-fonte, NAO implementacao aqui).
//   - CardCollection::add_to_active(..., initial_physical) - extensao sugerida na
//     secao 11 do doc-fonte, NAO implementada nesta onda (CardCollection continua
//     intocada; quem cria a CardInstance seta physical diretamente por ora).
//
// Cross-ref: docs/design/mecanicas/cartas-spec-dados.md,
//            docs/design/mecanicas/cartas-numeros-proposta.md,
//            gus/domain/deck/card_hardware_constants.hpp (os NUMEROS, TABELA
//            config - battery_capacity_for/contamination_percent_for),
//            gus/domain/deck/deck_records.hpp (CardInstance::physical),
//            gus/domain/combat/combat_records.hpp (Card::mimics_special_id).

#ifndef GUS_DOMAIN_DECK_CARD_HARDWARE_HPP
#define GUS_DOMAIN_DECK_CARD_HARDWARE_HPP

#include <cstdint>

#include "gus/domain/combat/combat_enums.hpp"  // CardTier

namespace gus::domain::deck {

// Origem fisica da copia (cartas-hardware-pirataria-energia.md secao 2/3).
enum class CardOrigin : std::uint32_t {
    OriginalRom = 0,    // ROM de fabrica, nao regravavel, "queima na hora" se adulterada
    HomebrewEprom = 1,  // EPROM vazia gravada em bancada de mercado negro, regravavel-ate-queimar
    PirateClone = 2,    // clone pirata, ROM travada apos gravar 1x
};

// Numero de valores canonicos de CardOrigin (0..kCardOriginCount-1). Mesmo padrao
// de kDifficultyLevelCount (save_data.hpp) - defesa em profundidade no validate().
inline constexpr std::uint32_t kCardOriginCount = 3;

// Payload oculto de virus (cartas-spec-dados.md secao 5.1/cartas-hardware-
// pirataria-energia.md secao 8). None = sem infeccao.
//
// IndustrialWeapon (7o valor, CARDS-HW-1, decisao do lider sobre AMB-DADOS-02): a
// arma dedicada do Dante/Sterling contra a Gaiola de Faraday no climax - fora da
// lista de virus "civis" do mercado negro (LogicBomb..ZipBomb, que continuam so
// para a rolagem de contaminacao generica, secao 6 inv.6). O EFEITO final e QUEIMA
// (CardPhysicalState::is_burned_out=true), NAO uma infeccao curavel pelo Turing -
// so a redencao scriptada do Dante restaura a carta. O DISPARO/gatilho narrativo em
// si e onda FUTURA (gameplay_engineer/narrativa); aqui so o VALOR do enum existe,
// sem nenhuma logica de disparo.
enum class VirusKind : std::uint32_t {
    None = 0,
    LogicBomb = 1,        // "sabotador de combate" - falha/vira contra o jogador
    Backdoor = 2,          // spyware - reporta mao/deck/posicao ao inimigo
    Worm = 3,               // espalha pro deck em cadeia + degrada performance
    FalseBenign = 4,       // isca Bastiat - parece buff, cobra custo oculto retardado
    AdwareSterling = 5,    // propaganda intransponivel antes do efeito
    ZipBomb = 6,            // incha na hora de soltar, estoura memoria/bateria
    IndustrialWeapon = 7,  // arma-industrial Dante/Sterling vs Faraday (AMB-DADOS-02)
};

// Numero de valores canonicos de VirusKind (0..kVirusKindCount-1).
inline constexpr std::uint32_t kVirusKindCount = 8;

// Conector RSB - DERIVADO de CardOrigin (cartas-hardware-pirataria-energia.md
// secao 3, 1:1: Original=nenhum, Homebrew=externo visivel, Pirata=interno
// escondido). NUNCA armazenado - existe so como tipo de retorno de connector_of().
enum class RsbConnector : std::uint32_t {
    None = 0,
    ExternalVisible = 1,
    InternalHidden = 2,
};

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

// Estado fisico MUTAVEL de UMA copia especifica de carta (CardInstance::physical,
// deck_records.hpp). Default = CardPhysicalState{} = "ROM original legitima,
// bateria cheia, sem infeccao" - o estado mais SEGURO/generoso, tanto para uma
// instancia nova quanto para um save V6 migrado (que nunca teve este campo) -
// cartas-spec-dados.md secao 5.2, "zero e seguro".
struct CardPhysicalState {
    CardOrigin origin = CardOrigin::OriginalRom;

    // ---- Bateria (cartas-hardware-pirataria-energia.md secao 5/cartas-numeros-
    // proposta.md secao 1) ----

    // Fonte de verdade da degradacao: cada RECARGA EM-LUGAR (mesma bateria fisica)
    // incrementa 1. SoH = 100 - 13xcycles (clamp 0), derivado - ver
    // state_of_health_percent() abaixo. Trocar de bateria (item novo, onda futura)
    // ZERA este campo; recarga-em-lugar INCREMENTA.
    std::uint16_t battery_recharge_cycles = 0;

    // Deficit de carga em relacao a capacidade ATUAL (nao a carga absoluta). 0 =
    // bateria CHEIA. Carga restante utilizavel = capacidade(hardware_class,
    // dificuldade) - battery_charge_deficit, clamp >= 0 (nunca negativo). Modelado
    // como DEFICIT de proposito: o valor-zero (default de struct, e o valor que um
    // save V6 migrado ganha por nao ter este campo) significa "bateria cheia" - o
    // estado mais SEGURO/generoso pra retrocompatibilidade.
    std::uint32_t battery_charge_deficit = 0;

    // ---- Integridade / virus (cartas-hardware-pirataria-energia.md secao 8/9/11) ----

    // OCULTO ao jogador por regra de design (nunca exposto cru na UI antes de
    // is_diagnosed==true - contrato de FRONTEIRA, secao 6 inv.3 do doc-fonte, NAO
    // implementado aqui). Setado 1x na rolagem de contaminacao na AQUISICAO (onda
    // futura) OU pelo gatilho narrativo scriptado do Dante/arma-Sterling em
    // CardTier::Especial (nunca por rolagem aleatoria nessas).
    bool is_infected = false;

    // O Turing diagnosticou (ou o payload ja disparou e se revelou sozinho).
    // Controla o que a UI pode mostrar; NAO cura nada por si.
    bool is_diagnosed = false;

    VirusKind virus_kind = VirusKind::None;

    // Chip permanentemente destruido (queima): resultado possivel da tentativa de
    // cura do Turing OU da arma-industrial (VirusKind::IndustrialWeapon). Carta com
    // is_burned_out=true fica inutilizavel em combate (gate futuro do
    // gameplay_engineer). Distinto de bateria morta (SoH<=piso): aquilo e
    // recuperavel trocando bateria; isto e PERMANENTE (exceto redencao scriptada).
    bool is_burned_out = false;

    [[nodiscard]] bool operator==(const CardPhysicalState&) const = default;

    // Fail-fast (cartas-spec-dados.md secao 6): virus_kind != None exige
    // is_infected==true (inv.1); is_diagnosed==true exige is_infected==true
    // (inv.1); origin/virus_kind dentro do dominio canonico (defesa em
    // profundidade, mesmo padrao de DifficultyLevel::validate() em
    // save_data.cpp/SaveData::validate() - um payload selado mas schema-
    // divergente nao e aceito silenciosamente). Lanca std::invalid_argument.
    void validate() const;
};

// ---- funcoes puras derivadas (cartas-spec-dados.md secao 5.3) ---------------------
//
// Mesmo estilo de TierLookup (card_collection.hpp) - funcoes livres, sem I/O.

// SoH em pontos percentuais, clamp [0,100]. 13pp por recarga
// (kBatteryDegradationPerRechargeCyclePp, card_hardware_constants.hpp).
[[nodiscard]] std::uint8_t state_of_health_percent(const CardPhysicalState&) noexcept;

// SoH <= kBatteryDeadSohFloorPercent (21%, piso de descarte) - "nao serve mais pra
// recarregar em-lugar", so vender/reciclar OU trocar de bateria.
[[nodiscard]] bool is_battery_dead(const CardPhysicalState&) noexcept;

// Carga restante = capacidade_atual - battery_charge_deficit, clamp >= 0 (SEM
// underflow de unsigned mesmo com deficit > capacidade). Recebe a capacidade JA
// RESOLVIDA (chamador calcula via battery_capacity_for, card_hardware_constants.hpp)
// - esta funcao NAO conhece dificuldade/classe, so faz a subtracao/clamp.
[[nodiscard]] std::uint32_t battery_charge_remaining(const CardPhysicalState&,
                                                      std::uint32_t capacity) noexcept;

// Conector RSB - DERIVADO 1:1 de origin, nunca armazenado.
[[nodiscard]] RsbConnector connector_of(CardOrigin) noexcept;

// Classe de hardware pra lookup de config (capacidade de bateria / % de
// contaminacao). catalog_tier vem de um TierLookup (mesmo padrao de
// card_collection.hpp); mimics_special vem de Card::mimics_special_id.has_value()
// do catalogo. CardTier::Especial OU CardTier::Super (classe PROTEGIDA, mesmo
// agrupamento de CardCollection::guard_protected_tier, deck-mao-sistema.md secao 7
// inv.9) SEMPRE mapeia para EspecialSelada, independente de origin/mimics_special -
// a validacao de origin==OriginalRom p/ Especial e invariante SEPARADO (secao 6.5
// do doc-fonte); esta funcao nao valida, so classifica.
[[nodiscard]] HardwareClass hardware_class_of(gus::domain::combat::CardTier catalog_tier,
                                               CardOrigin origin,
                                               bool mimics_special) noexcept;

}  // namespace gus::domain::deck

#endif  // GUS_DOMAIN_DECK_CARD_HARDWARE_HPP
