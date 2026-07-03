// gus/app/screens/battle_cockpit_verb_ids.hpp
//
// GLINTFX-CLICK: mapeamento ESTAVEL entre o `id` RCSS de cada um dos 6 pills de verbo do
// cockpit LIVE (SCAN/GAMBITO/ATACAR/DEFENDER/COMPILAR/FUGIR) e o INDICE do verbo (0..5,
// ordem = BattleVerb). POCO 100% testavel SEM SDL/glintfx: so string compare.
//
// POR QUE ISTO EXISTE (mata a divida do hit-test manual): ate a v0.2.4 o glintfx nao
// expunha hit-test/geometria de elemento (so data-model + process_event), entao o clique
// nos pills era resolvido por uma geometria ESPELHADA A MAO da RCSS (gus/app/screens/
// battle_cockpit_pills.hpp, aposentado neste incremento - ver historico git). A v0.2.5
// entrega UiLayer::set_click_callback: o glintfx faz o hit-test ELE MESMO (o MESMO
// hit-test que ja move o :hover nativo) e devolve o `id` do elemento clicado (sobe os
// ancestrais ate achar um id nao-vazio; "" se nenhum). Dando um `id` estavel a cada pill
// no RML (ver load_cockpit_rml() em battle_preview.cpp), o clique volta resolvido pelo
// PROPRIO glintfx - este arquivo e so o ponto unico que traduz esse `id` pro indice do
// verbo que o motor (BattleScene::menu_move/menu_confirm) entende.
//
// Cross-ref: app/src/screens/battle_preview.cpp (load_cockpit_rml / write_live_cockpit_rml,
//            onde os ids sao escritos no RML; battle_cockpit_verb_click, o wiring real do
//            callback); gus/app/screens/battle_menu.hpp (BattleVerb / verb_key, fonte da
//            ordem e do nome curto de cada verbo - os ids abaixo sao "verb-" + verb_key()).

#ifndef GUS_APP_SCREENS_BATTLE_COCKPIT_VERB_IDS_HPP
#define GUS_APP_SCREENS_BATTLE_COCKPIT_VERB_IDS_HPP

#include "gus/app/screens/battle_menu.hpp"  // kBattleVerbCount / BattleVerb / verb_key

namespace gus::app::screens {

// Os `id` RCSS estaveis dos 6 pills de verbo, na ORDEM do enum BattleVerb (indice i == o
// verbo static_cast<BattleVerb>(i)). "verb-" + verb_key(verb) (verb_key ja e a fonte unica
// do nome curto/estavel de cada verbo - reusado aqui, nao reinventado). MANTIDOS EM SINCRONIA
// A MAO com load_cockpit_rml()/write_live_cockpit_rml() (um teste dedicado prova a sincronia
// com verb_key(); ver battle_cockpit_verb_ids_test.cpp).
inline constexpr const char* kCockpitVerbElementIds[kBattleVerbCount] = {
    "verb-scan",      // BattleVerb::Scan
    "verb-gambito",   // BattleVerb::Gambito
    "verb-atacar",    // BattleVerb::Atacar
    "verb-defender",  // BattleVerb::Defender
    "verb-compilar",  // BattleVerb::Compilar
    "verb-flee",      // BattleVerb::Flee
};

// Mapeia o `id` devolvido por UiLayer::set_click_callback pro indice do verbo
// (0..kBattleVerbCount-1), ou -1 se o id nao for de nenhum pill (elemento vizinho do
// cockpit - #cockpit/#combat/#actor/#vitals/#log/etc, todos com id proprio - "" quando o
// clique nao acha nenhum ancestral com id, ou id nulo). PURA: so strcmp, sem estado.
[[nodiscard]] int cockpit_verb_index_for_click_id(const char* element_id) noexcept;

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_COCKPIT_VERB_IDS_HPP
