// gus/domain/save/save_data.hpp
//
// Estado de SAVE versionado (schema atual V5 = gus::domain::kSaveSchemaVersion).
// Dado PURO, ZERO Qt. Portado de engine/foundation/save_system/SaveDataV1.cs +
// CharacterSaveState.cs (sealed records C#); campos V4 (input_remap_backup,
// controls_hash128, slot_id) sao novos do ADR-007 (sem origem C#).
//
// EVOLUCAO de schema por CAMPO aditivo + migrator forward-only (NAO renomeia o
// tipo a cada bump): o numero de schema vive em SaveData::schema_version + no
// envelope, nao no nome C++. O nome SaveData (sem sufixo de versao) e proposital.
//
// DIVERGENCIAS vs C# (ADR-006 + ancora kSaveSchemaVersion):
//   - Vector3 do Godot (PlayerPosition/Rotation) vira Vec3 = 3 doubles POCO (sem Qt,
//     sem Godot). Mesma semantica (x/y/z), formato NOSSO.
//   - EnemyKnowledge (V3): reusa progression::KnowledgeStore (= std::map<string,int>),
//     o MESMO tipo do EnemyKnowledgeTracker (chave = enemy_type_id, valor = kills do
//     player contra aquele tipo). NAO duplica o tipo. Espelha o Dictionary<string,int>
//     EnemyKnowledge do C# (SaveDataV1.cs).
//   - CharacterSaveState aqui NAO tem KnowledgeKills: no C# esse campo per-character
//     virou VESTIGIAL ao chegar V3 (o consumidor le de EnemyKnowledge, keyed por TIPO,
//     nao por companion). Nao portamos o campo morto.
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

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "gus/domain/input/input_binding.hpp"  // InputRemapConfig (V4 backup)
#include "gus/domain/progression/enemy_knowledge_tracker.hpp"  // KnowledgeStore

namespace gus::domain::save {

// MODOS-MORTE Fase 0 (docs/design/mecanicas/modos-morte.md §2.2/§3.2): dificuldade
// FIXA do save, escolhida 1x na criacao (Novo Jogo) e NUNCA reescrita depois -
// nenhuma funcao de dominio deve expor um "setter" pos-criacao (a unica escrita
// legitima e no momento de StartNewGame, antes do 1o save_game). Hardcore nao
// aparece na tela de selecao (§2.3, e unlock separado, fase futura) mas ja tem
// ordinal reservado aqui (contrato binario estavel, mesmo padrao de BrainKind). A
// ORDEM e contrato binario (ordinal explicito 0..3).
enum class DifficultyLevel : std::uint32_t {
    Facil = 0,
    Medio = 1,     // default canonico (§2.1): jogo novo nasce aqui
    Dificil = 2,
    Hardcore = 3,
};

// Numero de valores canonicos de DifficultyLevel (0..kDifficultyLevelCount-1).
// Usado pela validacao de ordinal (mesmo padrao de kBrainKindCount/kCardFamilyCount
// em enemy_template.hpp): rejeita dificuldade fora do dominio no validate().
inline constexpr std::uint32_t kDifficultyLevelCount = 4;

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

// Save data versionado (schema atual V3). Imutavel por convencao (struct de valor).
struct SaveData {
    // Versao do schema deste save. Save novo nasce na versao atual
    // (kSaveSchemaVersion); saves antigos sobem pela chain antes de materializar.
    int schema_version = 5;

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

    // Conhecimento de bestiario do PLAYER por TIPO de inimigo (V3). Reusa o tipo do
    // progression (enemy_type_id -> kills acumulados); std::map = serializacao
    // deterministica. Vazio = "nenhum tipo conhecido" (variancia maxima no 1o
    // encontro). Alimenta o decaimento de variancia da formula de combate. O caller
    // game-side grava aqui o resultado de EnemyKnowledgeTracker::apply_victory.
    progression::KnowledgeStore enemy_knowledge;

    // ---- campos V4 (ADR-007) ----------------------------------------------

    // Backup integral do esquema de controles vigente no momento do save. Fonte de
    // restauracao quando o controls.json foi adulterado a mao. Gravado em TODO save.
    // Default = config vazio (config_version=1, actions vazio); o migrator V3->V4
    // popula com default_controls() e os saves novos com o config vigente.
    gus::domain::input::InputRemapConfig input_remap_backup;

    // Hash 128 do controls.json canonico vigente no momento do save (ADR-007 item 2).
    // E o "valor esperado" contra o qual a deteccao de adulteracao compara. Default
    // = todos-zero (sem controles registrados).
    std::array<std::uint8_t, 16> controls_hash128{};

    // ID/origem do slot deste save, SELADO dentro do payload (T1.2). Detecta troca de
    // arquivo entre slots no gerenciador de arquivos: se o arquivo do slot 2 for
    // copiado para a posicao do slot 5, o slot_id selado (2) diverge do slot lido (5).
    // -1 = origem desconhecida (save importado / legado pre-V4 sem slot conhecido).
    int slot_id = -1;

    // ---- campos V5 (MODOS-MORTE Fase 0) -----------------------------------

    // Dificuldade FIXA deste save (§2.2) - setada 1x na criacao, NUNCA reescrita
    // depois. Migrator V4->V5: saves antigos (pre-dificuldade) sobem como Medio
    // (default canonico §2.1 - nao havia escolha explicita antes, Medio e o
    // "meio-termo" mais coerente pra nao punir nem trivializar saves legados).
    DifficultyLevel difficulty = DifficultyLevel::Medio;

    // Estagio de recuperacao pos-morte no Dificil (§2.4). 0 = sem penalidade ativa
    // (ou modo != Dificil); 1 = acabou de respawnar (5%); 2 = cruzou Marco 1 (34%);
    // 3 = cruzou Marco 2 (89%); 4 = Marco 3 completo, 100%, volta a 0 (sem
    // penalidade). Irrelevante fora do modo Dificil. A LEITURA/consumo dos marcos
    // e fase futura (§6 Fase 3) - aqui e so o campo persistido.
    int difficult_recovery_stage = 0;

    [[nodiscard]] bool operator==(const SaveData&) const = default;

    // Valida invariantes (fail-fast): timestamp_ms>=0; playtime>=0; cada
    // CharacterSaveState valido; enemy_knowledge sem chave vazia e com kills>=0;
    // difficulty com ordinal no dominio canonico (V5, hardening MESMO padrao de
    // BrainKind/CardFamily em enemy_template.hpp); difficult_recovery_stage em
    // [0, 4] (V5, os 5 estagios documentados no campo acima). Lanca
    // std::invalid_argument.
    void validate() const;
};

}  // namespace gus::domain::save

#endif  // GUS_DOMAIN_SAVE_SAVE_DATA_HPP
