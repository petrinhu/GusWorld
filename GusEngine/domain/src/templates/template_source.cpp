// gus/domain/src/templates/template_source.cpp
//
// Implementacao da politica res:// > user://. Ver header. POCO puro, ZERO Qt.

#include "gus/domain/templates/template_source.hpp"

#include "gus/domain/templates/template_serializer.hpp"

namespace gus::domain::templates {

TemplateSelection select_source(
    const std::optional<std::vector<std::uint8_t>>& res_bytes,
    const std::optional<std::vector<std::uint8_t>>& user_bytes) {
    // res:// vence por construcao. Nao compara, nao faz diff: se existe, e a verdade.
    if (res_bytes.has_value()) {
        return TemplateSelection{*res_bytes, TemplateOrigin::Resource};
    }
    // res:// ausente: fallback para user:// (sera validado por HMAC a jusante).
    if (user_bytes.has_value()) {
        return TemplateSelection{*user_bytes, TemplateOrigin::User};
    }
    throw TemplateCorruptError(
        "Nenhuma fonte de template disponivel: res:// e user:// ambos ausentes.");
}

CharacterTemplate resolve_character(
    const std::optional<std::vector<std::uint8_t>>& res_bytes,
    const std::optional<std::vector<std::uint8_t>>& user_bytes) {
    const auto sel = select_source(res_bytes, user_bytes);
    return deserialize_character(sel.bytes);
}

EnemyTemplate resolve_enemy(
    const std::optional<std::vector<std::uint8_t>>& res_bytes,
    const std::optional<std::vector<std::uint8_t>>& user_bytes) {
    const auto sel = select_source(res_bytes, user_bytes);
    return deserialize_enemy(sel.bytes);
}

}  // namespace gus::domain::templates
