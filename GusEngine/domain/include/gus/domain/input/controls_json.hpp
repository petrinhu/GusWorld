// gus/domain/input/controls_json.hpp
//
// Serializer + parser JSON PROPRIO MINIMO do schema de controles (ADR-007 itens 1 e
// 2). POCO puro, ZERO Qt, ZERO I/O, ZERO dependencia externa (respeita o zero-dep do
// ADR-006). Cobre SO o schema flat de InputRemapConfig (NAO um JSON generico):
// objetos, arrays, strings com escape basico (\" \\ \n \t), inteiros, float,
// true/false. SEM \uXXXX, SEM notacao cientifica, SEM comentarios.
//
// Duas formas de saida:
//   - CANONICA (serialize_controls_canonical): compacta, actions na ORDEM do
//     ActionRegistry, ordem de chaves fixa, sem espaco significativo. E a forma que
//     entra no HASH 128 (controls_hash.hpp). Reformatar/reordenar o arquivo a mao
//     NAO muda a canonica (logo nao dispara o warning; decisao do lider, opcao 1).
//   - PRETTY (serialize_controls_pretty): indentada/legivel, para o arquivo de disco
//     que o jogador edita. NAO afeta a deteccao (o hash e sobre a canonica).
//
// Parse ROBUSTO que NUNCA lanca/crasha: parse_controls devolve ParseResult com um
// ControlsParseError. JSON malformado (jogador quebrou a sintaxe) = falha graciosa,
// igual a "arquivo ausente": o fluxo (platform/app) cai em restaurar/default. Entrada
// de usuario malformada e ESPERADA, nao corrupcao de save selado (por isso valor de
// retorno, nao excecao).
//
// Forward-compat: action_name desconhecido = action ignorada; chave JSON desconhecida
// = ignorada; campo ausente usa o default do struct.
//
// Cross-ref: docs/tech/adr/ADR-007-controls-json-hash128-save-v4.md (1, 2),
//            gus/domain/input/input_binding.hpp, action_registry.hpp.

#ifndef GUS_DOMAIN_INPUT_CONTROLS_JSON_HPP
#define GUS_DOMAIN_INPUT_CONTROLS_JSON_HPP

#include <string>

#include "gus/domain/input/input_binding.hpp"

namespace gus::domain::input {

// Resultado de parse (sinaliza por valor, NUNCA lanca na borda). None = sucesso.
enum class ControlsParseError {
    None,            // parse OK
    Empty,           // entrada vazia
    UnexpectedChar,  // token inesperado na sintaxe
    UnexpectedEnd,   // fim prematuro (chave/array/string nao fechado)
    BadNumber,       // numero malformado ou fora de faixa
    BadLiteral,      // true/false quebrado
    NotAnObject,     // raiz nao e um objeto JSON
    TypeMismatch,    // campo com tipo errado (ex.: "actions" nao e array)
    TooDeep,         // profundidade de aninhamento excedida (anti stack-overflow)
};

// Resultado do parse: o config (valido sse error == None) + o erro + o offset
// aproximado onde a falha ocorreu (debug; 0 quando irrelevante).
struct ControlsParseResult {
    InputRemapConfig config;
    ControlsParseError error = ControlsParseError::None;
    std::size_t error_offset = 0;
};

// Serializa o config na forma CANONICA (compacta, actions na ordem do ActionRegistry,
// chaves em ordem fixa). E a forma que entra no hash 128. Funcao pura.
[[nodiscard]] std::string serialize_controls_canonical(const InputRemapConfig& cfg);

// Serializa o config na forma PRETTY (indentada, legivel para edicao manual). Mesma
// ordem canonica de actions/chaves, so com espacos/quebras. Funcao pura.
[[nodiscard]] std::string serialize_controls_pretty(const InputRemapConfig& cfg);

// Parseia o JSON de controles. NUNCA lanca: sinaliza erro por ControlsParseError.
// Forward-compat (action/chave desconhecida ignorada; default em campo ausente).
[[nodiscard]] ControlsParseResult parse_controls(const std::string& json);

}  // namespace gus::domain::input

#endif  // GUS_DOMAIN_INPUT_CONTROLS_JSON_HPP
