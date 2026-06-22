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

}  // namespace

int current_schema_version() noexcept {
    // Fonte unica: o ancora do dominio. A chain abaixo (passos 1->2->3) DEVE alcancar
    // exatamente esta versao; o test-guarda current_schema_version()==kSaveSchemaVersion
    // trava qualquer divergencia (ex.: somar passo sem bumpar o ancora).
    return gus::domain::kSaveSchemaVersion;  // 4
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
