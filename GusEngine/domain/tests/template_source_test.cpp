// template_source_test.cpp
//
// Spec executavel (Catch2 v3) da politica res:// > user:// (F2-E.10-CONTRACT),
// portada de engine/tests/data/TemplateSourceTests.cs. PARTE PURA: a logica de
// selecao recebe os bytes dos dois lados como std::optional (sem Godot IO). res://
// vence SEMPRE que presente; user:// so quando res:// ausente; ambos ausentes = erro.
//
// Subsistema: domain/templates (marco M3). POCO puro, ZERO Qt.
//
// Cross-ref: engine/foundation/data/TemplateSource.cs, F2-E.10-CONTRACT, ADR-006.

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <vector>

#include "gus/domain/templates/card_family.hpp"
#include "gus/domain/templates/character_template.hpp"
#include "gus/domain/templates/enemy_template.hpp"
#include "gus/domain/templates/template_serializer.hpp"
#include "gus/domain/templates/template_source.hpp"

using gus::domain::templates::BrainKind;
using gus::domain::templates::CardFamily;
using gus::domain::templates::CharacterTemplate;
using gus::domain::templates::EnemyTemplate;
using gus::domain::templates::resolve_character;
using gus::domain::templates::resolve_enemy;
using gus::domain::templates::select_source;
using gus::domain::templates::serialize_character;
using gus::domain::templates::serialize_enemy;
using gus::domain::templates::TemplateCorruptError;
using gus::domain::templates::TemplateIntegrityError;
using gus::domain::templates::TemplateOrigin;

namespace {

CharacterTemplate res_tpl() {
    return CharacterTemplate{"gus", 34, 8, 5, 9, CardFamily::Eletrico, false,
                             {"deck_res"}};
}

CharacterTemplate user_tpl() {
    // "adulterado": jogador inflou stats.
    return CharacterTemplate{"gus",  9999, 999, 999, 999, CardFamily::Eletrico,
                             false,  {"deck_user_cheat"}};
}

}  // namespace

// ---- select_source: politica de selecao -----------------------------------

TEST_CASE("source: ambos presentes escolhe res://",
          "[domain][templates][source]") {
    const auto res = serialize_character(res_tpl());
    const auto user = serialize_character(user_tpl());

    const auto sel = select_source(res, user);

    REQUIRE(sel.origin == TemplateOrigin::Resource);
    REQUIRE(sel.bytes == res);
}

TEST_CASE("source: so user:// faz fallback para user://",
          "[domain][templates][source]") {
    const auto user = serialize_character(user_tpl());

    const auto sel = select_source(std::nullopt, user);

    REQUIRE(sel.origin == TemplateOrigin::User);
    REQUIRE(sel.bytes == user);
}

TEST_CASE("source: ambos ausentes lanca corrupcao",
          "[domain][templates][source]") {
    REQUIRE_THROWS_AS(select_source(std::nullopt, std::nullopt),
                      TemplateCorruptError);
}

TEST_CASE("source: res:// vence mesmo quando user:// e byte-identico",
          "[domain][templates][source]") {
    // "Divergente" e irrelevante: res:// vence por construcao, sem diff.
    const auto res = serialize_character(res_tpl());
    const auto user = res;  // copia identica

    const auto sel = select_source(res, user);

    REQUIRE(sel.origin == TemplateOrigin::Resource);
}

// ---- resolve_character: aplica a politica + desserializa -------------------

TEST_CASE("source: res:// autoritativo ignora user:// adulterado",
          "[domain][templates][source]") {
    const auto res = serialize_character(res_tpl());
    const auto user = serialize_character(user_tpl());

    const auto resolved = resolve_character(res, user);

    REQUIRE(resolved.max_hp == 34);  // canonico, nao o cheat 9999
    REQUIRE(resolved.base_deck.size() == 1u);
    REQUIRE(resolved.base_deck[0] == "deck_res");
}

TEST_CASE("source: so user:// presente ainda valida HMAC (tamper rejeitado)",
          "[domain][templates][source]") {
    auto user = serialize_character(user_tpl());
    user[user.size() / 2] ^= 0xFF;  // tamper: sem fallback silencioso

    REQUIRE_THROWS_AS(resolve_character(std::nullopt, user),
                      TemplateIntegrityError);
}

TEST_CASE("source: resolve_enemy com res:// funciona",
          "[domain][templates][source]") {
    const EnemyTemplate res{"sentinela_bit",     55,    6, 8, 4,
                            CardFamily::Cinetico, BrainKind::Scripted, false, {}};
    const auto bytes = serialize_enemy(res);

    const auto resolved = resolve_enemy(bytes, std::nullopt);

    REQUIRE(resolved.id == "sentinela_bit");
    REQUIRE(resolved.max_hp == 55);
    REQUIRE(resolved.brain == BrainKind::Scripted);
}
