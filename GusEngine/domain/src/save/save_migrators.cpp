// gus/domain/src/save/save_migrators.cpp
//
// Chain forward-only de migracao sobre STRUCTS VERSIONADAS (ADR-006). POCO puro,
// ZERO Qt, funcoes PURAS (sem relogio, disco, efeito colateral). Ver header.
//
// Os helpers de FIXTURE (serialize_save_v1 / make_v1_payload) sao definidos em
// save_serializer.cpp (junto do codec binario, para nao duplicar o writer V1).

#include "gus/domain/save/save_migrators.hpp"

#include <stdexcept>

#include "gus/domain/domain_info.hpp"
#include "gus/domain/input/controls_hash.hpp"
#include "gus/domain/input/controls_restore.hpp"

namespace gus::domain::save {

namespace {

// Passo V1 -> V2: adiciona character_states (vazio) e bumpa a versao. Espelha
// MigrateV1ToV2.cs (C#): semantica honesta de um save v1 (party sem delta
// registrado). Funcao pura. O decoder ja deixou character_states vazio ao ler um
// payload V1; aqui so confirmamos o invariante e bumpamos a versao.
SaveData migrate_v1_to_v2(SaveData data) {
    data.character_states.clear();  // V1 nao tinha o conceito: vazio e o correto
    data.schema_version = 2;
    return data;
}

// Passo V2 -> V3: adiciona enemy_knowledge (vazio) e bumpa a versao. Espelha
// MigrateV2ToV3.cs (C#). Funcao pura. EnemyKnowledge VAZIO e a semantica honesta de um
// save v2: nunca rastreou Knowledge por TIPO de inimigo (variancia maxima no 1o
// encontro). NAO se deriva de character_states: aquele era keyed por COMPANION, este
// por enemy_type_id; as chaves nao se mapeiam (derivar inventaria dado errado). O
// decoder ja deixou enemy_knowledge vazio ao ler um payload V2; aqui so confirmamos o
// invariante e bumpamos a versao.
SaveData migrate_v2_to_v3(SaveData data) {
    data.enemy_knowledge.clear();  // V2 nao tinha o conceito: vazio e o correto
    data.schema_version = 3;
    return data;
}

// Passo V3 -> V4 (ADR-007): adiciona input_remap_backup + controls_hash128 + slot_id.
// Semantica honesta de um save V3: "nao havia backup de controles; assume-se o
// default canonico" (mesma forma do migrate_v1_to_v2: campo novo com valor neutro
// honesto). backup = default_controls(); hash = hash 128 desse default; slot_id fica
// -1 (origem nao-selada): o load por slot (load_save) atribui o slot lido. Funcao
// pura.
SaveData migrate_v3_to_v4(SaveData data) {
    data.input_remap_backup = gus::domain::input::default_controls();
    data.controls_hash128 =
        gus::domain::input::controls_hash128(data.input_remap_backup);
    data.slot_id = -1;  // origem nao-selada num save V3; o load por slot define
    data.schema_version = 4;
    return data;
}

// Passo V4 -> V5 (MODOS-MORTE Fase 0, docs/design/mecanicas/modos-morte.md §3.2):
// adiciona difficulty + difficult_recovery_stage. Semantica honesta de um save V4:
// "nao havia escolha de dificuldade ainda" - sobe como Medio (default canonico
// §2.1, o "meio-termo" que nao pune nem trivializa saves legados), MESMA forma dos
// passos anteriores (campo novo com valor neutro honesto). difficult_recovery_stage
// = 0 (sem penalidade ativa - so relevante no modo Dificil, que nao existia ainda
// pra este save). Funcao pura.
SaveData migrate_v4_to_v5(SaveData data) {
    data.difficulty = DifficultyLevel::Medio;
    data.difficult_recovery_stage = 0;
    data.schema_version = 5;
    return data;
}

// Passo V5 -> V6 (DECK-4, docs/design/mecanicas/deck-mao-sistema.md): converte o
// deck legado (vector<string> de card_id) de CADA personagem no deck ATIVO novo
// (CardCollectionState). instance_id e sequencial deterministico comecando em 1,
// LOCAL a cada personagem (mesma semantica de CardCollection::add_to_active - sem
// RNG, sem timestamp; cada CardCollection e um espaco de IDs proprio, companions
// nao compartilham contador). Semantica honesta de um save V5: "as cartas do deck
// legado sao a UNICA fonte de verdade que existia" - viram instancias novas no
// ativo, deck morto vazio (V5 nao tinha o conceito), e hand_selection vazia (mao
// nao persistia; a bancada comeca vazia, o jogador remonta). O campo deck legado
// e ESVAZIADO apos a conversao - documento COM o resto do bump: manter os dois
// populados seria estado duplicado incoerente (fonte de verdade migrou pra
// card_collection.active). credits nasce em 0 UMA VEZ no nivel do SaveData
// (carteira UNICA da party, docs/design/mecanicas/economia.md - nao havia
// carteira registrada antes desta onda; NAO e per-character). Funcao pura.
SaveData migrate_v5_to_v6(SaveData data) {
    for (auto& [character_id, state] : data.character_states) {
        CardCollectionState collection;
        std::uint64_t next_id = 1;
        for (const auto& card_id : state.deck) {
            collection.active.push_back(
                gus::domain::deck::CardInstance{next_id, card_id});
            ++next_id;
        }
        collection.next_instance_id = next_id;
        state.card_collection = std::move(collection);
        state.deck.clear();
        state.hand_selection.clear();
    }
    data.credits = 0;
    data.schema_version = 6;
    return data;
}

// Passo V6 -> V7 (CARDS-HARDWARE-ENGINE incremento 1, CARDS-HW-1, docs/design/
// mecanicas/cartas-spec-dados.md secao 10): adiciona CardPhysicalState
// (CardInstance::physical) a CADA CardInstance de card_collection.active E .dead,
// de CADA personagem. Semantica honesta de um save V6: "toda carta pre-existente e
// legitima" - nenhuma tentativa de "adivinhar" retroativamente se uma carta ja
// salva seria pirata/infectada. physical = CardPhysicalState{} (ROM original,
// bateria cheia, sem infeccao - secao 5.2 "zero e seguro", o estado mais
// SEGURO/generoso). Nenhum outro campo de SaveData/CharacterSaveState muda neste
// bump (so o SHAPE de CardInstance ganhou physical). Funcao pura (sem RNG/
// relogio, CONTRACT.md secao 7) - o decoder do layout V6 (read_character_states_v6,
// save_serializer.cpp) ja materializa CardInstance com physical no default (o
// campo nao existe nos bytes V6); este passo so CONFIRMA/documenta esse estado
// honesto, mesmo padrao dos passos anteriores (ex.: migrate_v1_to_v2 confirmando
// character_states vazio).
SaveData migrate_v6_to_v7(SaveData data) {
    for (auto& [character_id, state] : data.character_states) {
        for (auto& inst : state.card_collection.active)
            inst.physical = gus::domain::deck::CardPhysicalState{};
        for (auto& inst : state.card_collection.dead)
            inst.physical = gus::domain::deck::CardPhysicalState{};
    }
    data.schema_version = 7;
    return data;
}

}  // namespace

int current_schema_version() noexcept {
    // Fonte unica: o ancora do dominio. A chain abaixo (passos 1->2->3->4->5->6)
    // DEVE alcancar exatamente esta versao; o test-guarda
    // current_schema_version()==kSaveSchemaVersion trava qualquer divergencia (ex.:
    // somar passo sem bumpar o ancora).
    return gus::domain::kSaveSchemaVersion;  // 7
}

SaveData migrate_to_current(SaveData data, int from_version) {
    const int target = current_schema_version();

    if (from_version > target)
        throw std::logic_error(
            "migrate_to_current: from_version > atual (forward-only); a checagem "
            "de versao futura e do load.");

    int version = from_version;
    while (version < target) {
        switch (version) {
            case 1:
                data = migrate_v1_to_v2(std::move(data));
                version = 2;
                break;
            case 2:
                data = migrate_v2_to_v3(std::move(data));
                version = 3;
                break;
            case 3:
                data = migrate_v3_to_v4(std::move(data));
                version = 4;
                break;
            case 4:
                data = migrate_v4_to_v5(std::move(data));
                version = 5;
                break;
            case 5:
                data = migrate_v5_to_v6(std::move(data));
                version = 6;
                break;
            case 6:
                data = migrate_v6_to_v7(std::move(data));
                version = 7;
                break;
            default:
                // GAP na chain: versao sem migrator registrado. Bug de schema.
                throw std::logic_error(
                    "migrate_to_current: gap na chain, sem migrator a partir da "
                    "versao " + std::to_string(version));
        }
    }
    return data;
}

}  // namespace gus::domain::save
