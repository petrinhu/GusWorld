// gus/domain/infection/integrity_state.hpp
//
// PECA de integridade/virus de uma carta possuida (ATOM-1, decomposicao atomica de
// CardPhysicalState em pecas componiveis em modulos estreitos, generalizando
// ADR-019 ao nivel de modulo; CARDS-HARDWARE-ENGINE incremento 1, CARDS-HW-1):
// payload de virus oculto + diagnostico. POCO puro, ZERO SDL/GL/RmlUi/IO (invariante
// de domain/, engine-design.md secao 2).
//
// infection/ NAO inclui combat/ nem deck/ (gate de camadas; so cards/ e permitido -
// este header nao precisa nem disso).
//
// NOTA: is_burned_out (chip permanentemente destruido) NAO mora nesta peca - fica
// direto no agregado CardPhysicalState (gus/domain/deck/card_hardware.hpp). E
// resultado possivel da cura do Turing OU da arma-industrial (VirusKind::
// IndustrialWeapon desta peca), mas PERMANENTE e sem o mesmo ciclo de vida
// infectado/diagnosticado - nao e parte do invariante secao 6 inv.1 (que so
// correlaciona virus_kind/is_infected/is_diagnosed, os 3 campos DESTA peca).
//
// Cross-ref: docs/design/mecanicas/cartas-hardware-pirataria-energia.md secao 8/9/11;
//            docs/design/mecanicas/cartas-spec-dados.md secao 5/6;
//            gus/domain/deck/card_hardware.hpp (fachada agregada - CardPhysicalState
//            HERDA desta peca); card_hardware_test.cpp (oraculo do agregado).

#ifndef GUS_DOMAIN_INFECTION_INTEGRITY_STATE_HPP
#define GUS_DOMAIN_INFECTION_INTEGRITY_STATE_HPP

#include <cstdint>

namespace gus::domain::infection {

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

// Propagacao secundaria (carta JA infectada, ao ser conjurada) - "worm de deck".
// //PLAYTEST
inline constexpr std::uint8_t kWormPropagationChancePercent = 13;

// Estado de integridade MUTAVEL de uma copia especifica de carta. Default =
// IntegrityState{} = "sem infeccao" - o estado mais SEGURO/generoso, tanto para uma
// instancia nova quanto para um save V6 migrado (que nunca teve este campo) -
// cartas-spec-dados.md secao 5.2, "zero e seguro".
struct IntegrityState {
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

    [[nodiscard]] bool operator==(const IntegrityState&) const = default;

    // Fail-fast (cartas-spec-dados.md secao 6 inv.1): virus_kind != None exige
    // is_infected==true; is_diagnosed==true exige is_infected==true; virus_kind
    // dentro do dominio canonico (defesa em profundidade, mesmo padrao de
    // DifficultyLevel::validate() em save_data.cpp/SaveData::validate()). Lanca
    // std::invalid_argument.
    void validate() const;
};

}  // namespace gus::domain::infection

#endif  // GUS_DOMAIN_INFECTION_INTEGRITY_STATE_HPP
