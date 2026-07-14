// master_cards_test.cpp
//
// Spec executavel (Catch2 v3) do catalogo data-driven das cartas ESPECIAIS dos mestres
// suportadas pelo executor techMagic (ADR-016, item TECHMAGIC-EXECUTOR / MVP step 4):
// volta, newton, pythagoras, godel, faraday, euler, turing, menger. POCO puro, ZERO Qt.
//
// Cobre: (1) as 8 cartas existem, ids unicos, nenhuma usa EffectKind::CloneAlly (guarda
// que von Neumann/Bruno NAO entraram nesta leva); (2) campos canonicos por-carta (tier/
// category/mana/ignores_weakness_wheel); (3) as 3 executaveis (volta/newton/pythagoras)
// resolvem via techMagic::execute SEM logic_error num contexto minimo; (4) as 4
// fora-de-combate + godel tem effects vazio (exceto godel, cujo trunfo e so a flag); (5)
// paridade i18n das 8 chaves CARD_EXEC_<FIGURA>_NAME (2 locales).
//
// Cross-ref: gus/domain/combat/master_cards.hpp; techmagic.hpp; placeholder_cards_test.cpp
//            (mesmo padrao de spec de registry); docs/design/roster-analogos/
//            _EFEITOS-ESCOLHIDOS.md; ADR-016.

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/master_cards.hpp"
#include "gus/domain/combat/techmagic.hpp"

using namespace gus::domain::combat;

namespace {

CombatActor make_actor(const std::string& id, bool player_side, int hp = 100, int atk = 8,
                       int def = 0, int spd = 20) {
    return CombatActor(id, id, hp, atk, def, spd, CardFamily::Eletrico, player_side);
}

}  // namespace

// ===== 1. As 8 cartas existem, ids unicos, guarda anti-CloneAlly =====

TEST_CASE("master_cards: build_registry tem exatamente as 8 cartas suportadas",
          "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    REQUIRE(reg.size() == 8);
    for (const char* id : {"volta", "newton", "pythagoras", "godel", "faraday", "euler",
                           "turing", "menger"})
        REQUIRE(reg.count(id) == 1);
}

TEST_CASE("master_cards: nenhuma carta usa EffectKind::CloneAlly (von Neumann/Bruno "
         "ficam de fora desta leva)",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    for (const auto& [id, card] : reg) {
        INFO("carta: " << id);
        for (const EffectSpec& spec : card.effects)
            REQUIRE(spec.kind != EffectKind::CloneAlly);
    }
}

// ===== 2. Campos canonicos por-carta =====

TEST_CASE("master_cards: todas as 8 sao tier Especial", "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    for (const auto& [id, card] : reg) {
        INFO("carta: " << id);
        REQUIRE(card.tier == CardTier::Especial);
    }
}

TEST_CASE("master_cards: volta = Ativa/Eletrico/mana 6, effects [OnDamageDealt Leech]",
          "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("volta");
    REQUIRE(c.category == CardCategory::Ativa);
    REQUIRE(c.family == CardFamily::Eletrico);
    REQUIRE(c.mana_cost == 6);
    REQUIRE(c.ignores_weakness_wheel == false);
    REQUIRE(c.effects.size() == 1);
    REQUIRE(c.effects[0].trigger == TriggerHook::OnDamageDealt);
    REQUIRE(c.effects[0].kind == EffectKind::Leech);
    REQUIRE(c.effects[0].percent == 50);
}

TEST_CASE("master_cards: newton = Hibrida/Universal/mana 6, effects [OnCast ApplyStatus "
         "Stun] + [OnDamageReceived Reflect]",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("newton");
    REQUIRE(c.category == CardCategory::Hibrida);
    REQUIRE(c.family == CardFamily::Universal);
    REQUIRE(c.mana_cost == 6);
    REQUIRE(c.ignores_weakness_wheel == false);
    REQUIRE(c.effects.size() == 2);

    const auto& stun = c.effects[0];
    REQUIRE(stun.trigger == TriggerHook::OnCast);
    REQUIRE(stun.kind == EffectKind::ApplyStatus);
    REQUIRE(stun.status == StatusId::Stun);
    REQUIRE(stun.duration == 1);

    const auto& reflect = c.effects[1];
    REQUIRE(reflect.trigger == TriggerHook::OnDamageReceived);
    REQUIRE(reflect.kind == EffectKind::Reflect);
    REQUIRE(reflect.percent == 30);
}

TEST_CASE("master_cards: pythagoras = Passiva/Universal/mana 0, effects [OnRoundEnd "
         "HypotenuseCombo]",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("pythagoras");
    REQUIRE(c.category == CardCategory::Passiva);
    REQUIRE(c.family == CardFamily::Universal);
    REQUIRE(c.mana_cost == 0);
    REQUIRE(c.ignores_weakness_wheel == false);
    REQUIRE(c.effects.size() == 1);
    REQUIRE(c.effects[0].trigger == TriggerHook::OnRoundEnd);
    REQUIRE(c.effects[0].kind == EffectKind::HypotenuseCombo);
}

TEST_CASE("master_cards: godel = Passiva/Universal/mana 0/effects vazio, trunfo "
         "ignores_weakness_wheel=true",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("godel");
    REQUIRE(c.category == CardCategory::Passiva);
    REQUIRE(c.family == CardFamily::Universal);
    REQUIRE(c.mana_cost == 0);
    REQUIRE(c.effects.empty());
    REQUIRE(c.ignores_weakness_wheel == true);
}

TEST_CASE("master_cards: as 4 fora-de-combate (faraday/euler/turing/menger) sao "
         "ForaDeCombate/mana 0/effects vazio; SOMENTE godel tem "
         "ignores_weakness_wheel=true",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();

    const Card& faraday = reg.at("faraday");
    REQUIRE(faraday.category == CardCategory::ForaDeCombate);
    REQUIRE(faraday.family == CardFamily::Eletrico);
    REQUIRE(faraday.mana_cost == 0);
    REQUIRE(faraday.effects.empty());
    REQUIRE(faraday.ignores_weakness_wheel == false);

    const Card& euler = reg.at("euler");
    REQUIRE(euler.category == CardCategory::ForaDeCombate);
    REQUIRE(euler.family == CardFamily::Eletrico);
    REQUIRE(euler.mana_cost == 0);
    REQUIRE(euler.effects.empty());
    REQUIRE(euler.ignores_weakness_wheel == false);

    const Card& turing = reg.at("turing");
    REQUIRE(turing.category == CardCategory::ForaDeCombate);
    REQUIRE(turing.family == CardFamily::Universal);
    REQUIRE(turing.mana_cost == 0);
    REQUIRE(turing.effects.empty());
    REQUIRE(turing.ignores_weakness_wheel == false);

    const Card& menger = reg.at("menger");
    REQUIRE(menger.category == CardCategory::ForaDeCombate);
    REQUIRE(menger.family == CardFamily::Universal);
    REQUIRE(menger.mana_cost == 0);
    REQUIRE(menger.effects.empty());
    REQUIRE(menger.ignores_weakness_wheel == false);

    // SO godel tem o trunfo fora-da-roda; nenhuma das 8 alem dela.
    int flagged = 0;
    for (const auto& [id, card] : reg)
        if (card.ignores_weakness_wheel) ++flagged;
    REQUIRE(flagged == 1);
}

// ===== 3. As 3 executaveis resolvem SEM logic_error num contexto minimo =====

TEST_CASE("master_cards: volta (Leech) executa via techMagic::execute sem lancar, "
         "caster ganha HP e mana",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("volta");

    CombatActor caster = make_actor("h", /*player_side=*/true, /*hp=*/50);
    // Estabelece capacidade de mana do turno (max_mana so existe apos o refresh da FSM),
    // depois gasta tudo: assim o restore_mana do Leech tem para onde subir (senao clampa
    // em max_mana_=0). heal idem: abrimos espaco de HP com dano previo.
    caster.refresh_resources_for_turn(/*round_index=*/0);  // mana_ = max_mana_ = 2.
    caster.spend_mana(caster.mana());                      // mana 2 -> 0.
    caster.take_damage(30);                                // hp 50 -> 20.
    const int mana_before = caster.mana();                 // 0.
    const int hp_before = caster.hp();                     // 20.
    CombatActor target = make_actor("e", /*player_side=*/false, /*hp=*/300);

    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &target;
    ctx.damage = 20;  // evento de dano ja resolvido (OnDamageDealt); Leech le daqui.

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnDamageDealt, c, ctx));
    REQUIRE(caster.hp() > hp_before);      // Leech curou.
    REQUIRE(caster.mana() > mana_before);  // Leech tambem restaura mana (clamp em max_mana).
}

TEST_CASE("master_cards: newton (ApplyStatus Stun em OnCast + Reflect em "
         "OnDamageReceived) executa sem lancar",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("newton");

    CombatActor caster = make_actor("h", /*player_side=*/true);
    CombatActor foe = make_actor("e", /*player_side=*/false);

    // OnCast: aplica Stun no counterpart (alvo da carta).
    techMagic::TechMagicContext cast_ctx;
    cast_ctx.caster = &caster;
    cast_ctx.counterpart = &foe;
    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, c, cast_ctx));
    bool stunned = false;
    for (const auto& s : foe.status_effects())
        if (s.id == StatusId::Stun) stunned = true;
    REQUIRE(stunned);

    // OnDamageReceived: caster sofreu dano de counterpart (o atacante original); Reflect
    // devolve fracao pro atacante.
    CombatActor attacker = make_actor("atk", /*player_side=*/false, /*hp=*/100);
    techMagic::TechMagicContext hit_ctx;
    hit_ctx.caster = &caster;       // dono da passiva (quem SOFREU o dano).
    hit_ctx.counterpart = &attacker;  // quem CAUSOU.
    hit_ctx.damage = 10;
    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnDamageReceived, c, hit_ctx));
    REQUIRE(attacker.hp() < 100);  // Reflect acertou o atacante.
}

TEST_CASE("master_cards: pythagoras (HypotenuseCombo em OnRoundEnd) executa sem "
         "lancar com um ledger minimo",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("pythagoras");

    CombatActor h1 = make_actor("h1", /*player_side=*/true);
    CombatActor h2 = make_actor("h2", /*player_side=*/true);
    CombatActor e = make_actor("e", /*player_side=*/false, /*hp=*/500);

    const std::vector<techMagic::RoundHitEntry> hits = {
        techMagic::RoundHitEntry{&h1, &e, 8}, techMagic::RoundHitEntry{&h2, &e, 5}};

    techMagic::TechMagicContext ctx;
    ctx.caster = &h1;  // dono da passiva precisa estar entre os atacantes (Q4).
    ctx.round_hits = &hits;

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnRoundEnd, c, ctx));
    // Nao valida o numero exato do bonus (ja coberto em techmagic_roundend_test.cpp) -
    // so que o handler existe e roda ate o fim sem lancar.
}

// ===== 4. Paridade i18n das 8 chaves (2 locales) =====

namespace {

// Le um .md de traducao e devolve o conjunto de chaves "## CHAVE" encontradas. Parser
// minimo local (nao reusa tools/i18n_parity.py, que e Python fora do escopo C++ deste
// teste); so verifica presenca, mesmo contrato de MdTranslationLoader (secao 5).
std::vector<std::string> keys_in(const std::string& path) {
    std::ifstream in(path);
    std::vector<std::string> out;
    std::string line;
    while (std::getline(in, line)) {
        if (line.rfind("## ", 0) == 0) out.push_back(line.substr(3));
    }
    return out;
}

bool has_key(const std::vector<std::string>& keys, const std::string& key) {
    for (const auto& k : keys)
        if (k == key) return true;
    return false;
}

}  // namespace

// GUSWORLD_TRANSLATIONS_DIR e injetado por tests/CMakeLists.txt
// (${CMAKE_SOURCE_DIR}/../game/translations) - caminho ABSOLUTO, robusto ao CWD do
// ctest (catch_discover_tests roda o binario a partir do build dir).
#ifndef GUSWORLD_TRANSLATIONS_DIR
#define GUSWORLD_TRANSLATIONS_DIR "../../../../game/translations"
#endif

TEST_CASE("master_cards: as 8 chaves CARD_EXEC_<FIGURA>_NAME existem em pt_br.md e "
         "en_intl.md",
         "[domain][combat][cards][techmagic][mastercards][i18n]") {
    const std::vector<std::string> pt =
        keys_in(std::string(GUSWORLD_TRANSLATIONS_DIR) + "/pt_br.md");
    const std::vector<std::string> en =
        keys_in(std::string(GUSWORLD_TRANSLATIONS_DIR) + "/en_intl.md");
    REQUIRE_FALSE(pt.empty());
    REQUIRE_FALSE(en.empty());

    for (const char* figura : {"VOLTA", "NEWTON", "PYTHAGORAS", "GODEL", "FARADAY",
                               "EULER", "TURING", "MENGER"}) {
        const std::string key = std::string("CARD_EXEC_") + figura + "_NAME";
        INFO("chave: " << key);
        REQUIRE(has_key(pt, key));
        REQUIRE(has_key(en, key));
    }
}
