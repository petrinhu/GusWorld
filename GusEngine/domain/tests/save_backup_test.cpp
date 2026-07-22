// save_backup_test.cpp
//
// Spec executavel (Catch2 v3) da POLITICA DE BACKUP CHAIN do save (domain/save).
// Camada PURA: NAO toca disco. A politica opera sobre um STORE ABSTRATO (port
// injetavel, gus::domain::save::SaveStore) com FAKE em memoria no teste. A camada
// de I/O real (platform/, futura) implementa o mesmo port sobre arquivos.
//
// Portado de game/scripts/foundation/save_system/SaveManager.cs::RotateBackups
// (C#: renomeia arquivos no disco). Aqui a politica e PURA sobre nomes logicos:
//   ao gravar o slot S, o conteudo anterior de S vira backup1; backup1 vira
//   backup2; ... ate a profundidade da chain (N=3, igual ao C#).
//
// Nomes logicos do store: "<slot>" (primario) e "<slot>.backupK" (K=1..N). A
// traducao nome-logico -> path e do platform/, fora daqui.
//
// Cross-ref: game/scripts/foundation/save_system/SaveManager.cs (RotateBackups,
//            referencia), ADR-006 (backup chain).
// (o C# original nao existe mais: repo do submodulo engine/ apagado no M8;
//  referencia historica, tambem vale pra mencao na linha 8)

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>
#include <vector>

#include "gus/domain/save/save_backup.hpp"
#include "gus/domain/save/save_slots.hpp"
#include "gus/domain/save/save_store.hpp"

using gus::domain::save::backup_logical_name;
using gus::domain::save::InMemorySaveStore;
using gus::domain::save::kBackupChainDepth;
using gus::domain::save::primary_logical_name;
using gus::domain::save::write_with_backup_rotation;

namespace {

// Bytes-sentinela faceis de distinguir por "geracao".
std::vector<std::uint8_t> gen(std::uint8_t tag) {
    return {tag, tag, tag};
}

}  // namespace

// ---- nomes logicos de primario e backups -----------------------------------

TEST_CASE("backup: profundidade da chain N=3 (igual ao C#)",
          "[domain][save][backup]") {
    REQUIRE(kBackupChainDepth == 3);
}

TEST_CASE("backup: nome do primario e o nome logico do slot",
          "[domain][save][backup]") {
    REQUIRE(primary_logical_name(0) == "autosave");
    REQUIRE(primary_logical_name(1) == "save_1");
}

TEST_CASE("backup: nomes de backup sufixados .backupK",
          "[domain][save][backup]") {
    REQUIRE(backup_logical_name(0, 1) == "autosave.backup1");
    REQUIRE(backup_logical_name(1, 3) == "save_1.backup3");
}

// ---- primeira gravacao: sem backup anterior --------------------------------

TEST_CASE("backup: primeira gravacao escreve so o primario",
          "[domain][save][backup]") {
    InMemorySaveStore store;
    write_with_backup_rotation(store, 1, gen(0xA1));

    REQUIRE(store.read("save_1") == gen(0xA1));
    REQUIRE_FALSE(store.exists("save_1.backup1"));
}

// ---- rotacao: gravacoes sucessivas empurram a chain ------------------------

TEST_CASE("backup: gravacoes sucessivas rotacionam primary->b1->b2->b3",
          "[domain][save][backup]") {
    InMemorySaveStore store;
    write_with_backup_rotation(store, 1, gen(0x01));  // primary=01
    write_with_backup_rotation(store, 1, gen(0x02));  // primary=02, b1=01
    write_with_backup_rotation(store, 1, gen(0x03));  // primary=03, b1=02, b2=01
    write_with_backup_rotation(store, 1, gen(0x04));  // primary=04, b1=03, b2=02, b3=01

    REQUIRE(store.read("save_1") == gen(0x04));
    REQUIRE(store.read("save_1.backup1") == gen(0x03));
    REQUIRE(store.read("save_1.backup2") == gen(0x02));
    REQUIRE(store.read("save_1.backup3") == gen(0x01));
}

// ---- chain saturada: o mais antigo cai fora (N=3) --------------------------

TEST_CASE("backup: chain saturada descarta a geracao mais antiga",
          "[domain][save][backup]") {
    InMemorySaveStore store;
    for (std::uint8_t g = 1; g <= 5; ++g)
        write_with_backup_rotation(store, 0, gen(g));  // autosave, 5 gravacoes

    // Mantem primary + 3 backups = 4 geracoes mais recentes (5,4,3,2). A 1a some.
    REQUIRE(store.read("autosave") == gen(5));
    REQUIRE(store.read("autosave.backup1") == gen(4));
    REQUIRE(store.read("autosave.backup2") == gen(3));
    REQUIRE(store.read("autosave.backup3") == gen(2));
    // Nao existe backup4 (profundidade 3).
    REQUIRE_FALSE(store.exists("autosave.backup4"));
}

// ---- isolamento entre slots ------------------------------------------------

TEST_CASE("backup: rotacao de um slot nao afeta outro slot",
          "[domain][save][backup]") {
    InMemorySaveStore store;
    write_with_backup_rotation(store, 1, gen(0x11));
    write_with_backup_rotation(store, 2, gen(0x22));
    write_with_backup_rotation(store, 1, gen(0x12));  // rotaciona so o slot 1

    REQUIRE(store.read("save_1") == gen(0x12));
    REQUIRE(store.read("save_1.backup1") == gen(0x11));
    REQUIRE(store.read("save_2") == gen(0x22));
    REQUIRE_FALSE(store.exists("save_2.backup1"));  // slot 2 intacto
}

// ---- slot invalido rejeitado (fail-fast) -----------------------------------

TEST_CASE("backup: gravar em slot invalido lanca",
          "[domain][save][backup]") {
    InMemorySaveStore store;
    // 7 (nao mais 6 - bump SAVE-LOAD-UI etapa 6, kManualSlotCount 5->6 tornou o
    // slot 6 VALIDO, ver save_slots.hpp): o 1o slot fora do teto agora.
    REQUIRE_THROWS_AS(write_with_backup_rotation(store, 7, gen(0x00)),
                      std::out_of_range);
}
