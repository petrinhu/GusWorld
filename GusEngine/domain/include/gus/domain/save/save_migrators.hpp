// gus/domain/save/save_migrators.hpp
//
// MIGRATORS forward-only do save sobre STRUCTS VERSIONADAS (ADR-006), NAO sobre
// arvore JSON (como era o C#). Cada bump de schema (VN -> VN+1) e um passo da chain;
// CurrentVersion = maior versao alcancavel = gus::domain::kSaveSchemaVersion.
//
// Schema atual = V2:
//   V1 = inicial, SEM character_states (per-character).
//   V2 = + character_states. MigrateV1ToV2 popula character_states VAZIO (semantica
//        honesta de um save v1: party usa stats do template, sem delta registrado).
//
// Por que para em V2: o ancora gus::domain::kSaveSchemaVersion = 2. O C# ja foi a
// V3 (EnemyKnowledge), mas portar aquele bump mexeria no ancora (fora do escopo
// deste marco). A chain e extensivel: somar o passo V2->V3 + bumpar o ancora quando
// o marco chegar (test-guarda current_schema_version()==kSaveSchemaVersion trava).
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

// Forja um envelope selado (HMAC valido) cujo payload declara schema_version =
// version, com o restante dos campos minimos (layout V1). Usado para testar a
// rejeicao de versao futura (version > atual). Determinista.
[[nodiscard]] std::vector<std::uint8_t> make_v1_payload(int version);

}  // namespace gus::domain::save

#endif  // GUS_DOMAIN_SAVE_SAVE_MIGRATORS_HPP
