// gus/domain/settings/system_settings_json.hpp
//
// Serializer + parser JSON PROPRIO MINIMO do schema de SystemSettings
// (MENU-PAUSA-CONFIG-SOM, M7-COSTURA). POCO puro, ZERO Qt, ZERO I/O, ZERO
// dependencia externa (mesmo espirito/receita de gus/domain/input/controls_json.hpp
// - objeto JSON flat, numeros float, forward-compat, parse robusto que NUNCA lanca).
//
// Schema (flat, 3 campos):
//   { "schema_version": <int>, "music_volume": <float>, "sfx_volume": <float> }
//
// Parse ROBUSTO: parse_system_settings devolve um resultado por VALOR (nunca
// lanca/crasha). JSON malformado/ausente = falha graciosa - o chamador (platform/
// fs/settings_file_store.hpp) cai nos defaults do SystemSettings, igual ao "sem
// settings.json ainda" (1a execucao do jogo).
//
// Cross-ref: gus/domain/settings/system_settings.hpp (o struct),
//            gus/domain/input/controls_json.hpp (mesmo padrao de parser/serializer).

#ifndef GUS_DOMAIN_SETTINGS_SYSTEM_SETTINGS_JSON_HPP
#define GUS_DOMAIN_SETTINGS_SYSTEM_SETTINGS_JSON_HPP

#include <string>

#include "gus/domain/settings/system_settings.hpp"

namespace gus::domain::settings {

// Resultado de parse (sinaliza por valor, NUNCA lanca na borda). None = sucesso.
enum class SystemSettingsParseError {
    None,            // parse OK
    Empty,           // entrada vazia
    UnexpectedChar,  // token inesperado na sintaxe
    UnexpectedEnd,   // fim prematuro (chave/objeto nao fechado)
    BadNumber,       // numero malformado ou fora de faixa (ex.: notacao cientifica)
    BadLiteral,      // true/false/null quebrado
    NotAnObject,     // raiz nao e um objeto JSON
    TypeMismatch,    // campo com tipo errado (ex.: "music_volume" nao e numero)
};

// Resultado do parse: settings (valido mesmo em erro - defaults preservados nos
// campos que nao resolveram) + o erro.
struct SystemSettingsParseResult {
    SystemSettings settings;
    SystemSettingsParseError error = SystemSettingsParseError::None;
};

// Serializa na forma PRETTY (indentada, legivel para edicao manual em disco -
// settings.json e um arquivo pequeno que o jogador pode abrir num editor).
[[nodiscard]] std::string serialize_system_settings_pretty(const SystemSettings& s);

// Parseia o JSON de settings. NUNCA lanca: sinaliza erro por
// SystemSettingsParseError. Forward-compat (chave desconhecida ignorada; campo
// ausente usa o default do struct). Valores de volume fora de [0,1] sao
// CLAMPADOS (mesmo contrato de AudioEngine::set_music_volume/set_sfx_volume) -
// um settings.json editado a mao com um valor absurdo nao produz um volume
// invalido.
[[nodiscard]] SystemSettingsParseResult parse_system_settings(const std::string& json);

}  // namespace gus::domain::settings

#endif  // GUS_DOMAIN_SETTINGS_SYSTEM_SETTINGS_JSON_HPP
