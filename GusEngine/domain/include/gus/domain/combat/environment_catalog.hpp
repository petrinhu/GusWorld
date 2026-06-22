// gus/domain/combat/environment_catalog.hpp
//
// Catalogo data-driven dos ambientes de combate (secao 18), portado de
// engine/foundation/turn_combat/EnvironmentCatalog.cs. Fonte UNICA de instancias
// canonicas de EnvironmentModifier. Todos os numeros (mults de familia, deltas de status,
// duracoes de periodo, hooks de hardware) sao transcritos DIRETO das tabelas
// secao 18.2/18.3/18.4/18.5 - gameplay_engineer NAO inventa valor (balance e canon).
//
// Tambem expoe o calculo de mult_ambiente (secao 11): produto das camadas ATIVAS que
// afetam a familia da carta, com cap final [0.44, 2.25]. mult_ambiente e o ULTIMO fator
// da formula secao 11 (ADR-004); aqui o catalogo so devolve o fator (a multiplicacao na
// cadeia de dano e da CombatStateMachine, chunk 4).
//
// O C# modela como `static class` de membros estaticos. Aqui adotamos um namespace
// EnvironmentCatalog com funcoes livres (equivalente C++ idiomatico). POCO puro, ZERO Qt.
//
// MAPEAMENTO de excecoes C# -> C++: KeyNotFoundException -> std::out_of_range.
//
// Cross-ref: engine/foundation/turn_combat/EnvironmentCatalog.cs;
//            engine/tests/turn_combat/environments/EnvironmentCatalogTests.cs;
//            docs/design/mecanicas/combat.md secao 18 (catalogo)/11 (stacking/cap)/9;
//            ADR-004; ADR-006.

#ifndef GUS_DOMAIN_COMBAT_ENVIRONMENT_CATALOG_HPP
#define GUS_DOMAIN_COMBAT_ENVIRONMENT_CATALOG_HPP

#include <initializer_list>
#include <map>
#include <vector>

#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/environment_enums.hpp"
#include "gus/domain/combat/environment_modifier.hpp"

namespace gus::domain::combat::EnvironmentCatalog {

// Todas as entradas canonicas do catalogo (read-only). std::map: ordem por EnvironmentId
// estavel e deterministica (espelha a iteracao do dicionario do C# de forma reproduzivel).
[[nodiscard]] const std::map<EnvironmentId, EnvironmentModifier>& all();

// Ambiente "vazio" (None): mult_ambiente neutro, sem efeito. Retrocompat secao 11.
[[nodiscard]] const EnvironmentModifier& none();

// Lookup canonico por id. Lanca std::out_of_range se o id nao existe (bug de chamador;
// espelha KeyNotFoundException do C#).
[[nodiscard]] const EnvironmentModifier& get(EnvironmentId id);

// mult_ambiente (secao 11): produto dos multiplicadores das camadas ATIVAS (terreno +
// clima + periodo) que afetam a familia dada, com cap final [0.44, 2.25]. Conjunto vazio
// (ou so None) => 1.0 (retrocompat: combate sem ambiente INALTERADO). NUNCA toca
// mult_fraqueza. None e descartado do produto.
[[nodiscard]] float mult_ambiente(CardFamily family,
                                  const std::vector<EnvironmentModifier>& active);

// Sobrecarga por initializer_list (ergonomia de teste/chamada inline).
[[nodiscard]] float mult_ambiente(CardFamily family,
                                  std::initializer_list<EnvironmentModifier> active);

// Sobrecarga por ids (le do catalogo). Nome distinto (mult_ambiente_ids) por C++ nao
// resolver bem a sobrecarga entre vector<EnvironmentModifier> e vector<EnvironmentId>.
[[nodiscard]] float mult_ambiente_ids(CardFamily family,
                                      const std::vector<EnvironmentId>& active_ids);

}  // namespace gus::domain::combat::EnvironmentCatalog

#endif  // GUS_DOMAIN_COMBAT_ENVIRONMENT_CATALOG_HPP
