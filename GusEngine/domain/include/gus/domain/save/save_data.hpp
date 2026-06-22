// gus/domain/save/save_data.hpp
//
// Estado de SAVE versionado (schema atual V2 = gus::domain::kSaveSchemaVersion).
// Dado PURO, ZERO Qt. Portado de engine/foundation/save_system/SaveDataV1.cs +
// CharacterSaveState.cs (sealed records C#).
//
// EVOLUCAO de schema por CAMPO aditivo + migrator forward-only (NAO renomeia o
// tipo a cada bump): o numero de schema vive em SaveData::schema_version + no
// envelope, nao no nome C++. O nome SaveData (sem sufixo de versao) e proposital.
//
// DIVERGENCIAS vs C# (ADR-006 + ancora kSaveSchemaVersion):
//   - Vector3 do Godot (PlayerPosition/Rotation) vira Vec3 = 3 doubles POCO (sem Qt,
//     sem Godot). Mesma semantica (x/y/z), formato NOSSO.
//   - O C# foi a V3 (EnemyKnowledge). O C++ para em V2 de proposito
//     (kSaveSchemaVersion=2): EnemyKnowledge e o bump V2->V3 NAO sao portados neste
//     marco (mexeriam no ancora, fora do escopo). CharacterSaveState aqui NAO tem
//     KnowledgeKills (campo que so existe a partir de V3 no C#).
//
// CARIMBO (ADR-006 item 4): timestamp_ms (epoch ms, data+hora+ms) gravado como
// metadado para listar/ordenar saves e exibir "salvo em X". PUREZA: o domain NUNCA
// chama o relogio; quem grava INJETA o timestamp (testes passam valor fixo).
//
// Invariantes (fail-fast, validate()): espelha o Validate() do C#.
//
// Cross-ref: engine/foundation/save_system/SaveDataV1.cs + CharacterSaveState.cs,
//            CONTRACT.md §7, ADR-006.

#ifndef GUS_DOMAIN_SAVE_SAVE_DATA_HPP
#define GUS_DOMAIN_SAVE_SAVE_DATA_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace gus::domain::save {

// Vetor 3D POCO (substitui Godot.Vector3 do C#; ZERO dependencia). Igualdade exata
// por valor (roundtrip binario fiel: gravamos o bit-pattern dos doubles).
struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    [[nodiscard]] bool operator==(const Vec3&) const = default;
};

// Estado MUTAVEL-no-tempo de um personagem persistido per-character (V2).
// Portado de CharacterSaveState.cs SEM KnowledgeKills (campo so existe em V3 do C#;
// o C++ para em V2). Invariantes em validate(): current_hp>=0; xp>=0; deck sem id
// vazio (lista vazia e valida; em C++ nao ha vector null).
struct CharacterSaveState {
    // HP atual (0 = nocauteado). Invariante: >= 0. Teto MaxHp validado no consumidor.
    int current_hp = 0;
    // XP acumulado (gating de zona §11). Invariante: >= 0. Nao vira nivel (GDD §5.4).
    int xp = 0;
    // Deck atual: IDs de carta (Card.Id). Lista vazia e valida. Sem id vazio.
    std::vector<std::string> deck;

    [[nodiscard]] bool operator==(const CharacterSaveState&) const = default;

    // Valida invariantes (fail-fast). Lanca std::invalid_argument.
    void validate() const;
};

// Save data versionado (schema atual V2). Imutavel por convencao (struct de valor).
struct SaveData {
    // Versao do schema deste save. Save novo nasce na versao atual
    // (kSaveSchemaVersion); saves antigos sobem pela chain antes de materializar.
    int schema_version = 2;

    // CARIMBO injetado (epoch ms). Metadado de listagem/ordenacao (ADR-006 item 4).
    std::int64_t timestamp_ms = 0;

    // Tempo de jogo acumulado em segundos.
    double playtime_seconds = 0.0;

    // Cena atual carregada.
    std::string current_scene_path;

    // Posicao e rotacao do player no mundo (yaw/pitch/roll em graus na rotacao).
    Vec3 player_position;
    Vec3 player_rotation;

    // Roster da party (ids dos companions recrutados, em ordem).
    std::vector<std::string> party_roster;
    // Active party (ate 3 membros em combate), em ordem.
    std::vector<std::string> party_active;

    // Flags booleanas de world state (ordenado: std::map = serializacao determinista).
    std::map<std::string, bool> flags;
    // Inventory (item_id -> quantidade).
    std::map<std::string, int> inventory;
    // Quest progress (quest_id -> stage_index).
    std::map<std::string, int> quest_progress;
    // Afinidade de companions (companion_id -> score).
    std::map<std::string, int> relations;

    // Estado per-character (V2). Chave = CharacterId ("gus", "caua"...). Default
    // vazio: party recem-iniciada usa stats do template. std::map = ordem
    // deterministica (chave de determinismo do selo).
    std::map<std::string, CharacterSaveState> character_states;

    [[nodiscard]] bool operator==(const SaveData&) const = default;

    // Valida invariantes (fail-fast): timestamp_ms>=0; playtime>=0; cada
    // CharacterSaveState valido. Lanca std::invalid_argument.
    void validate() const;
};

}  // namespace gus::domain::save

#endif  // GUS_DOMAIN_SAVE_SAVE_DATA_HPP
