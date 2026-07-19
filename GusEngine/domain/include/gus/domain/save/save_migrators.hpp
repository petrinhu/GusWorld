// gus/domain/save/save_migrators.hpp
//
// MIGRATORS forward-only do save sobre STRUCTS VERSIONADAS (ADR-006), NAO sobre
// arvore JSON (como era o C#). Cada bump de schema (VN -> VN+1) e um passo da chain;
// CurrentVersion = maior versao alcancavel = gus::domain::kSaveSchemaVersion.
//
// Schema atual = V7 (fonte unica: gus::domain::kSaveSchemaVersion). NAO confiar
// neste comentario como autoridade da versao; a ancora kSaveSchemaVersion e que manda.
//   V1 = inicial, SEM character_states (per-character).
//   V2 = + character_states. MigrateV1ToV2 popula character_states VAZIO (semantica
//        honesta de um save v1: party usa stats do template, sem delta registrado).
//   V3 = + enemy_knowledge (conhecimento de bestiario por TIPO). MigrateV2ToV3 vazio.
//   V4 = + input_remap_backup + controls_hash128 + slot_id (ADR-007). MigrateV3ToV4
//        popula backup = default canonico, hash = hash do default, slot_id = o slot
//        lido (passado no load).
//   V5 = + difficulty + difficult_recovery_stage (MODOS-MORTE Fase 0, docs/design/
//        mecanicas/modos-morte.md §3.2). MigrateV4ToV5 popula difficulty = Medio
//        (default canonico §2.1: saves pre-dificuldade sobem no "meio-termo", nao
//        havia escolha explicita antes) e difficult_recovery_stage = 0 (sem
//        penalidade ativa).
//   V6 = + CardCollectionState (deck ativo/morto) + hand_selection por
//        personagem, EM SUBSTITUICAO ao campo legado CharacterSaveState::deck
//        (DECK-4, docs/design/mecanicas/deck-mao-sistema.md); + credits (i64) UMA
//        VEZ no nivel do SaveData (carteira UNICA da party, docs/design/mecanicas/
//        economia.md - economia single-currency, NAO per-character). MigrateV5ToV6
//        converte o deck legado (vector<string> de card_id) de cada personagem em
//        instancias sequenciais no deck ATIVO novo (instance_id 1..N, deterministico,
//        LOCAL a cada personagem - CardCollection nao compartilha espaco de IDs
//        entre companions), deck morto vazio, hand_selection vazia (mao nao
//        persistia antes; bancada comeca vazia); credits=0 no SaveData (nao havia
//        carteira antes desta onda). O campo deck legado e ESVAZIADO apos a
//        conversao (fonte de
//        verdade migrou pra card_collection.active).
//   V7 = + CardPhysicalState (gus/domain/deck/card_hardware.hpp) dentro de CADA
//        CardInstance (ativo E morto) de CardCollectionState (CARDS-HARDWARE-
//        ENGINE incremento 1, CARDS-HW-1, docs/design/mecanicas/
//        cartas-spec-dados.md secao 10). MigrateV6ToV7 popula physical =
//        CardPhysicalState{} (ROM original, bateria cheia, sem infeccao - o
//        estado mais SEGURO/generoso, "zero e seguro") em TODA instancia ja
//        existente - semantica honesta de um save V6: "toda carta pre-existente e
//        legitima", sem tentar adivinhar contaminacao retroativa (funcao PURA,
//        sem RNG/relogio, CONTRACT.md secao 7).
//
// A chain e extensivel: somar o passo VN->VN+1 + bumpar a ancora quando o marco
// chegar (test-guarda current_schema_version()==kSaveSchemaVersion trava o esquecimento).
//
// Forward-only: versao > atual e rejeitada no load (SaveVersionTooNewError); versao
// < atual sobe pela chain antes de materializar; == atual materializa direto.
//
// Funcoes PURAS (CONTRACT §7): sem efeito colateral, sem relogio, sem disco.
//
// Cross-ref: engine/foundation/save_system/Migrators/* (ref semantica), CONTRACT §7,
//            gus/domain/save/save_serializer.hpp, ADR-006.

#ifndef GUS_DOMAIN_SAVE_SAVE_MIGRATORS_HPP
#define GUS_DOMAIN_SAVE_SAVE_MIGRATORS_HPP

#include <cstdint>
#include <vector>

#include "gus/domain/save/save_data.hpp"
#include "gus/domain/save/save_serializer.hpp"  // erros + pack_save (re-export)

namespace gus::domain::save {

// Versao atual do schema alcancada pela chain (= maior ToVersion). DEVE bater com
// gus::domain::kSaveSchemaVersion (test-guarda). Forward-only: o load nunca produz
// um SaveData de versao diferente desta.
[[nodiscard]] int current_schema_version() noexcept;

// Migra um SaveData lido de uma versao antiga (from_version) ate a versao atual,
// aplicando a chain passo a passo. Funcao pura. Lanca std::logic_error se houver
// GAP na chain (versao sem migrator). NAO valida HMAC (o load ja validou antes).
[[nodiscard]] SaveData migrate_to_current(SaveData data, int from_version);

// ---- helpers de FIXTURE (usados por testes e pela camada de migracao) ------
//
// Serializa um SaveData no layout do payload V1 (SEM character_states) dentro do
// envelope binario selado (MAGIC || LEN || payloadV1 || HMAC). Representa a
// "geracao antiga" do jogo gravando um save v1, para a fixture de migracao. NAO
// valida invariantes de V2 (V1 nao os tinha).
[[nodiscard]] std::vector<std::uint8_t> serialize_save_v1(const SaveData& data);

// Serializa um SaveData no layout do payload V2 (comuns + character_states, SEM
// enemy_knowledge) dentro do envelope selado. Representa a "geracao V2" do jogo, para
// a fixture de migracao V2->V3. NAO valida invariantes de V3.
[[nodiscard]] std::vector<std::uint8_t> serialize_save_v2(const SaveData& data);

// Serializa um SaveData no layout do payload V3 (comuns + character_states +
// enemy_knowledge, SEM os campos V4: backup/hash128/slot_id) dentro do envelope
// selado. Representa a "geracao V3" do jogo, para a fixture de migracao V3->V4. NAO
// valida invariantes de V4.
[[nodiscard]] std::vector<std::uint8_t> serialize_save_v3(const SaveData& data);

// Serializa um SaveData no layout do payload V4 (comuns + character_states +
// enemy_knowledge + input_remap_backup + controls_hash128 + slot_id, SEM os campos
// V5: difficulty/difficult_recovery_stage) dentro do envelope selado. Representa a
// "geracao V4" do jogo (ADR-007, pre-MODOS-MORTE), para a fixture de migracao
// V4->V5. NAO valida invariantes de V5.
[[nodiscard]] std::vector<std::uint8_t> serialize_save_v4(const SaveData& data);

// Serializa um SaveData no layout do payload V5 (comuns + character_states COM O
// DECK LEGADO (list<str>, sem card_collection/credits/hand_selection) +
// enemy_knowledge + input_remap_backup + controls_hash128 + slot_id + difficulty +
// difficult_recovery_stage) dentro do envelope selado. Representa a "geracao V5"
// do jogo (MODOS-MORTE Fase 0, pre-DECK-4), para a fixture de migracao V5->V6. NAO
// valida invariantes de V6 (CharacterSaveState::deck e o campo lido pelo migrator;
// card_collection/credits/hand_selection nao existem neste layout).
[[nodiscard]] std::vector<std::uint8_t> serialize_save_v5(const SaveData& data);

// Serializa um SaveData no layout do payload V6 (comuns + character_states_v6 COM
// card_collection/hand_selection MAS SEM CardInstance::physical + enemy_knowledge +
// input_remap_backup + controls_hash128 + slot_id + difficulty +
// difficult_recovery_stage + credits) dentro do envelope selado. Representa a
// "geracao V6" do jogo (DECK-4, pre-CARDS-HW-1), para a fixture de migracao
// V6->V7 (CARDS-HARDWARE-ENGINE incremento 1). NAO valida invariantes de V7
// (CardInstance::physical nao existe neste layout - o decoder de V6 ja deixa
// physical no default ao materializar, e o migrator so confirma/documenta isso).
[[nodiscard]] std::vector<std::uint8_t> serialize_save_v6(const SaveData& data);

// Forja um envelope selado (HMAC valido) cujo payload declara schema_version =
// version, com o restante dos campos minimos (layout V1). Usado para testar a
// rejeicao de versao futura (version > atual). Determinista.
[[nodiscard]] std::vector<std::uint8_t> make_v1_payload(int version);

}  // namespace gus::domain::save

#endif  // GUS_DOMAIN_SAVE_SAVE_MIGRATORS_HPP
