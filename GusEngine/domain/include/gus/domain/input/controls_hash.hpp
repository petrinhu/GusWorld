// gus/domain/input/controls_hash.hpp
//
// Hash de 128 bits dos controles (ADR-007 item 2). POCO puro, ZERO Qt. Mora em
// domain/ (depende de InputRemapConfig) e CONSOME o SHA-256 de core/crypto (core nao
// conhece InputRemapConfig, por isso NAO mora em core/).
//
// O hash = primeiros 16 bytes (128 bits) do SHA-256 sobre a forma CANONICA do
// controls.json (NAO os bytes crus do arquivo). Reformatar/reindentar/reordenar o
// arquivo a mao NAO muda o hash; so uma troca SEMANTICA de binding muda (decisao do
// lider, opcao 1: menos falso-positivo).
//
// NAO e seguranca: usa SHA-256 CRU (nao HMAC). A chave/algoritmo sao publicos por
// design (o controls.json e deliberadamente reproduzivel e editavel). E deteccao
// CASUAL de edicao manual: dizer ao jogador "alguem mexeu nisso fora do jogo".
//
// ATENCAO (ADR-007 risco a): a forma canonica de controls_json e agora parte do
// CONTRATO de hash. Mudar a forma canonica muda o hash de configs identicos
// (breaking para a deteccao); tratar como bump de config_version. Travado por
// tests/controls_hash_test.cpp (vetor "16 primeiros bytes do SHA-256 do canonico").
//
// Cross-ref: docs/tech/adr/ADR-007-controls-json-hash128-save-v4.md (item 2),
//            gus/domain/input/controls_json.hpp, gus/core/crypto/sha256.hpp.

#ifndef GUS_DOMAIN_INPUT_CONTROLS_HASH_HPP
#define GUS_DOMAIN_INPUT_CONTROLS_HASH_HPP

#include <array>
#include <cstdint>

#include "gus/domain/input/input_binding.hpp"

namespace gus::domain::input {

// Tamanho do hash de controles em bytes (128 bits).
inline constexpr std::size_t kControlsHashSize = 16;

// Hash 128 do config: primeiros 16 bytes do SHA-256 da forma canonica. Pura.
[[nodiscard]] std::array<std::uint8_t, kControlsHashSize> controls_hash128(
    const InputRemapConfig& cfg);

// true sse os dois hashes diferem (= o arquivo de controles foi modificado em
// relacao ao esperado-no-save). Sem timing-safety (nao e seguranca).
[[nodiscard]] bool controls_were_modified(
    const std::array<std::uint8_t, kControlsHashSize>& current,
    const std::array<std::uint8_t, kControlsHashSize>& saved);

}  // namespace gus::domain::input

#endif  // GUS_DOMAIN_INPUT_CONTROLS_HASH_HPP
