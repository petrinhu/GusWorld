// gus/domain/progression/enemy_knowledge_tracker.hpp
//
// Logica PURA de progressao de Knowledge por TIPO de inimigo (Eixo A, F2-E.9).
// Portado de engine/foundation/knowledge/EnemyKnowledgeTracker.cs.
//
// POCO puro, ZERO Qt (invariante de domain/, engine-design.md secao 2). Sem
// estado proprio, sem I/O: cada funcao e pura sobre seus argumentos.
//
// SEMANTICA CANONICA (knowledge-progression.md secao 1): knowledge_kills e
// "quantos inimigos DAQUELE TIPO o player derrotou", o conhecimento do PLAYER
// sobre o bestiario, NAO um stat de membro da party. O store e
// map<enemy_type_id,int> (chave = id do EnemyTemplate, ex.: "sentinela_bit";
// valor = kills acumulados). Alimenta o decaimento de variancia da formula de
// combate (combat.md secao 11).
//
// REGRAS DE INCREMENTO (secao 3, DA-2):
//   - Derrotar o inimigo em combate (Victory + HP 0) -> +1 para o tipo;
//   - Fugir (Flee)                                   -> 0 (ciclo nao observado);
//   - Party perdeu (Defeat)                          -> 0;
//   - Scan sem derrota                               -> 0 (kill = ciclo completo).
//
// PUREZA: apply_victory devolve um NOVO mapa, nao muta o input (espelha o padrao
// forward-only dos migrators, CONTRACT secao 7). O caller (game-side) persiste o
// resultado.
//
// FRONTEIRA (decisao de porte): o C# resolve defeated_enemy_types a partir de
// CombatActor (entidade mutavel de combate, ainda NAO portada em C++,
// domain/combat e .gitkeep no marco M3). A logica le apenas 3 campos do ator
// (id, is_player_side, is_alive); portamos uma VIEW minima POCO (DefeatedActor)
// em vez de acoplar progression a um tipo de combat inexistente, evitando inverter
// a ordem de portabilidade. Quando combat/CombatActor for portado, um adaptador
// trivial preenche a view (id/IsPlayerSide/IsAlive ja existem la). Mantem
// progression desacoplado e independentemente testavel.
//
// Cross-ref: docs/design/mecanicas/knowledge-progression.md secao 1/3/7,
//            combat.md secao 11; engine/foundation/knowledge/EnemyKnowledgeTracker.cs.

#ifndef GUS_DOMAIN_PROGRESSION_ENEMY_KNOWLEDGE_TRACKER_HPP
#define GUS_DOMAIN_PROGRESSION_ENEMY_KNOWLEDGE_TRACKER_HPP

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace gus::domain::progression {

// Store de conhecimento do player: enemy_type_id -> kills acumulados. std::map
// (ordenado) da iteracao deterministica de graca (util para diff/save estavel);
// a busca O(log n) e irrelevante para o tamanho de um bestiario. Espelha o
// SaveDataV1.EnemyKnowledge do C# (Dictionary<string,int>).
using KnowledgeStore = std::map<std::string, int>;

// Desfecho de um combate, para creditar (ou nao) Knowledge. Espelha o
// CombatOutcome do turn_combat C#; redefinido aqui (subconjunto necessario)
// para progression nao depender de domain/combat (ainda nao portado, M3).
// Quando combat/ for portado, este enum pode passar a referenciar o canonico.
enum class CombatOutcome {
    Victory,  // party venceu: tipos derrotados sao creditados.
    Fled,     // fuga: 0 kills (ciclo nao observado).
    Defeat,   // party perdeu: 0 kills.
};

// View POCO minima de um combatente para defeated_enemy_types. Contem apenas o
// que a regra le do CombatActor C# (id, is_player_side, is_alive). NAO e a
// entidade de combate completa: e um adaptador de leitura. Ver nota FRONTEIRA.
struct DefeatedActor {
    std::string id;             // id do tipo (EnemyTemplate.Id), chave do store.
    bool is_player_side = false;  // true = party; false = inimigo (bestiario).
    bool is_alive = true;       // false = derrotado (HP 0) ao fim do combate.
};

// Le o knowledge_kills do PLAYER contra um enemy_type_id. Chave ausente = 0
// (1o encontro, variancia maxima). Leitura defensiva (nao muta o store).
//
// Invariante (fail-fast): enemy_type_id nao pode ser vazio/so-espacos; senao
// std::invalid_argument (espelha o ArgumentException do C#).
[[nodiscard]] int knowledge_for(const KnowledgeStore& store,
                                std::string_view enemy_type_id);

// Aplica uma vitoria: incrementa +1 o Knowledge de CADA tipo derrotado em
// defeated_enemy_types (um id por corpo; o mesmo tipo repetido soma). Funcao
// PURA: clona o store, aplica os incrementos e devolve o novo mapa; o input fica
// intacto (forward-only, CONTRACT secao 7). Lista vazia e valida (combate sem
// kill de bestiario) e devolve um clone do store.
//
// Invariante (fail-fast): nenhum id pode ser vazio/so-espacos; senao
// std::invalid_argument.
[[nodiscard]] KnowledgeStore apply_victory(
    const KnowledgeStore& store,
    const std::vector<std::string>& defeated_enemy_types);

// Extrai os tipos de inimigo a CREDITAR a partir do desfecho do combate (secao
// 3). So CombatOutcome::Victory concede Knowledge; Fled e Defeat devolvem vazio.
// Considera apenas atores inimigos (is_player_side=false) que terminaram mortos
// (is_alive=false): companions caidos nao sao bestiario; inimigos vivos no fim
// nao foram derrotados. O id do ator e o id do tipo (EnemyTemplate.Id), coerente
// com o store por tipo. Preserva a ordem dos atores (um por corpo; tipos
// repetidos preservados).
[[nodiscard]] std::vector<std::string> defeated_enemy_types(
    CombatOutcome outcome,
    const std::vector<DefeatedActor>& actors);

}  // namespace gus::domain::progression

#endif  // GUS_DOMAIN_PROGRESSION_ENEMY_KNOWLEDGE_TRACKER_HPP
