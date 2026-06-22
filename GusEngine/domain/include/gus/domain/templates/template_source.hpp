// gus/domain/templates/template_source.hpp
//
// Politica de selecao de fonte de template: res:// e AUTORITATIVO (PCK); user:// e
// so uma copia de override potencialmente adulterada (F2-E.10-CONTRACT). Portado de
// engine/foundation/data/TemplateSource.cs.
//
// A LOGICA DE SELECAO e POCO pura e testavel sem Godot: recebe os bytes brutos dos
// dois lados como std::optional (ausencia = nullopt). O IO real (Godot/Qt FileAccess)
// fica numa camada a parte (FORA deste marco, platform/). Mesma separacao do loader i18n.
//
// Regra (res:// vence): se res:// presente -> usa res://, IGNORA user://. user:// so
// quando res:// ausente, e ainda assim passa pela MESMA validacao HMAC (sem fallback
// silencioso para dado adulterado). user:// NUNCA sobrescreve res:// valido (anti-tamper).
//
// POCO puro, ZERO Qt (invariante de domain/, engine-design.md secao 2).
//
// Cross-ref: engine/foundation/data/TemplateSource.cs, F2-E.10-CONTRACT, ADR-006.

#ifndef GUS_DOMAIN_TEMPLATES_TEMPLATE_SOURCE_HPP
#define GUS_DOMAIN_TEMPLATES_TEMPLATE_SOURCE_HPP

#include <cstdint>
#include <optional>
#include <vector>

#include "gus/domain/templates/character_template.hpp"
#include "gus/domain/templates/enemy_template.hpp"

namespace gus::domain::templates {

// De onde o template selecionado veio (telemetria/log).
enum class TemplateOrigin {
    Resource,  // res:// (PCK, autoritativo)
    User,      // user:// (override em disco do jogador)
};

// Resultado da selecao: bytes brutos + origem.
struct TemplateSelection {
    std::vector<std::uint8_t> bytes;
    TemplateOrigin origin;

    [[nodiscard]] bool operator==(const TemplateSelection&) const = default;
};

// Seleciona os bytes .gdt autoritativos. res_bytes (res://) vence sempre que
// presente; user_bytes (user://) so e usado quando res:// ausente (nullopt).
// Lanca TemplateCorruptError se ambas as fontes ausentes.
[[nodiscard]] TemplateSelection select_source(
    const std::optional<std::vector<std::uint8_t>>& res_bytes,
    const std::optional<std::vector<std::uint8_t>>& user_bytes);

// Carrega + desserializa um CharacterTemplate aplicando a politica res:// > user://.
// A validacao HMAC acontece dentro de deserialize_character (res:// adulterado lanca,
// nao cai silenciosamente para user://).
[[nodiscard]] CharacterTemplate resolve_character(
    const std::optional<std::vector<std::uint8_t>>& res_bytes,
    const std::optional<std::vector<std::uint8_t>>& user_bytes);

// Carrega + desserializa um EnemyTemplate aplicando a politica res:// > user://.
[[nodiscard]] EnemyTemplate resolve_enemy(
    const std::optional<std::vector<std::uint8_t>>& res_bytes,
    const std::optional<std::vector<std::uint8_t>>& user_bytes);

}  // namespace gus::domain::templates

#endif  // GUS_DOMAIN_TEMPLATES_TEMPLATE_SOURCE_HPP
