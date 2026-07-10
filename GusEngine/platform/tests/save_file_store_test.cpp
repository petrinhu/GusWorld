// GusEngine/platform/tests/save_file_store_test.cpp
//
// Catch2 (TEST-FIRST) do FsSaveStore/save_game/load_game (M2-SAVE-IO, ADR-012 Onda
// 2): I/O REAL em disco de ~/.gusworld/saves/<nome>.sav (perms 0700 dir / 0600
// arquivo), consumindo gus::domain::save::load_save (T1.1, nao-lancante) + a cadeia
// de backup (write_with_backup_rotation, N=3) e a politica de slots ja provadas em
// domain/tests/. Hermetico: cada teste usa um diretorio TEMPORARIO proprio (nunca o
// $HOME real do host que roda o teste).

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

#include "gus/domain/save/save_data.hpp"
#include "gus/domain/save/save_serializer.hpp"
#include "gus/domain/save/save_slots.hpp"
#include "gus/domain/save/save_store.hpp"
#include "gus/platform/fs/save_file_store.hpp"

using gus::domain::save::LoadResult;
using gus::domain::save::SaveData;
using gus::platform::fs::delete_save;
using gus::platform::fs::FsSaveStore;
using gus::platform::fs::has_save;
using gus::platform::fs::load_game;
using gus::platform::fs::load_game_from_backup;
using gus::platform::fs::save_game;

namespace {

std::filesystem::path make_temp_dir(const char* suffix) {
    auto dir = std::filesystem::temp_directory_path() /
               (std::string("gusworld_save_io_test_") + suffix);
    std::filesystem::remove_all(dir);
    return dir;
}

// SaveData minimo mas valido (validate() nao lanca), com slot_id coerente com o
// slot fisico onde sera gravado (contrato de save_game).
SaveData make_valid_save(int slot) {
    SaveData data;
    data.timestamp_ms = 1000;
    data.playtime_seconds = 42.5;
    data.current_scene_path = "city_intro";
    data.party_roster = {"gus"};
    data.party_active = {"gus"};
    data.slot_id = slot;
    return data;
}

}  // namespace

// ---- resolve_saves_dir ------------------------------------------------------

TEST_CASE("resolve_saves_dir: contem o sufixo canonico .gusworld/saves",
          "[platform][fs][save]") {
    const std::string dir = gus::platform::fs::resolve_saves_dir();
    REQUIRE(dir.find(".gusworld") != std::string::npos);
    REQUIRE(dir.find("saves") != std::string::npos);
}

// ---- slot vazio: degradacao segura como "sem save" --------------------------

TEST_CASE("has_save: slot sem arquivo devolve false (1a execucao do jogo)",
          "[platform][fs][save]") {
    const auto dir = make_temp_dir("ausente");
    REQUIRE_FALSE(std::filesystem::exists(dir));
    REQUIRE_FALSE(has_save(1, dir.string()));
    std::filesystem::remove_all(dir);
}

TEST_CASE("load_game: slot sem arquivo devolve nullopt (fluxo de novo jogo)",
          "[platform][fs][save]") {
    const auto dir = make_temp_dir("load_ausente");
    const auto outcome = load_game(1, dir.string());
    REQUIRE_FALSE(outcome.has_value());
    std::filesystem::remove_all(dir);
}

// ---- roundtrip real em disco -------------------------------------------------

TEST_CASE("save_game + load_game: roundtrip real em disco (slot manual)",
          "[platform][fs][save]") {
    const auto dir = make_temp_dir("roundtrip");
    const SaveData original = make_valid_save(1);

    REQUIRE(save_game(original, 1, dir.string()));
    REQUIRE(has_save(1, dir.string()));

    const auto outcome = load_game(1, dir.string());
    REQUIRE(outcome.has_value());
    REQUIRE(outcome->result == LoadResult::Ok);
    REQUIRE(outcome->data == original);

    std::filesystem::remove_all(dir);
}

TEST_CASE("save_game + load_game: roundtrip real em disco (autosave, slot 0)",
          "[platform][fs][save]") {
    const auto dir = make_temp_dir("roundtrip_autosave");
    const SaveData original = make_valid_save(gus::domain::save::kAutosaveSlot);

    REQUIRE(save_game(original, gus::domain::save::kAutosaveSlot, dir.string()));
    const auto outcome = load_game(gus::domain::save::kAutosaveSlot, dir.string());
    REQUIRE(outcome.has_value());
    REQUIRE(outcome->result == LoadResult::Ok);
    REQUIRE(outcome->data.current_scene_path == "city_intro");

    std::filesystem::remove_all(dir);
}

// ---- permissoes LGPD ---------------------------------------------------------

TEST_CASE("save_game: cria o diretorio com permissao 0700 e o arquivo com "
          "permissao 0600 (LGPD - dado do jogador so ele le/escreve)",
          "[platform][fs][save]") {
    const auto dir = make_temp_dir("perms");
    const SaveData data = make_valid_save(1);
    REQUIRE(save_game(data, 1, dir.string()));

    const auto dir_perms = std::filesystem::status(dir).permissions();
    const auto file_perms =
        std::filesystem::status(dir / "save_1.sav").permissions();

    using std::filesystem::perms;
    REQUIRE((dir_perms & perms::owner_all) == perms::owner_all);
    REQUIRE((dir_perms & perms::group_all) == perms::none);
    REQUIRE((dir_perms & perms::others_all) == perms::none);

    REQUIRE((file_perms & perms::owner_read) == perms::owner_read);
    REQUIRE((file_perms & perms::owner_write) == perms::owner_write);
    REQUIRE((file_perms & perms::group_all) == perms::none);
    REQUIRE((file_perms & perms::others_all) == perms::none);

    std::filesystem::remove_all(dir);
}

// ---- arquivo corrompido: PRESENTE mas malformado != ausente ------------------

TEST_CASE("load_game: arquivo PRESENTE mas corrompido devolve LoadResult::Corrupt "
          "(nao confundir com slot vazio)",
          "[platform][fs][save]") {
    const auto dir = make_temp_dir("corrompido");
    std::filesystem::create_directories(dir);
    {
        std::ofstream out(dir / "save_1.sav", std::ios::binary | std::ios::trunc);
        out << "isso nao e um envelope de save valido";
    }

    const auto outcome = load_game(1, dir.string());
    REQUIRE(outcome.has_value());  // arquivo existe -> NAO e nullopt
    REQUIRE(outcome->result == LoadResult::Corrupt);

    std::filesystem::remove_all(dir);
}

// ---- adulteracao: HMAC nao bate ----------------------------------------------

TEST_CASE("load_game: byte do envelope adulterado devolve LoadResult::HmacInvalid",
          "[platform][fs][save]") {
    const auto dir = make_temp_dir("tamper");
    const SaveData data = make_valid_save(1);
    REQUIRE(save_game(data, 1, dir.string()));

    const auto path = dir / "save_1.sav";
    {
        std::fstream f(path, std::ios::in | std::ios::out | std::ios::binary);
        REQUIRE(f.is_open());
        f.seekp(10);
        char byte = 0;
        f.seekg(10);
        f.read(&byte, 1);
        byte = static_cast<char>(byte ^ 0xFF);
        f.seekp(10);
        f.write(&byte, 1);
    }

    const auto outcome = load_game(1, dir.string());
    REQUIRE(outcome.has_value());
    REQUIRE(outcome->result == LoadResult::HmacInvalid);

    std::filesystem::remove_all(dir);
}

// ---- backup chain sobrevive ao disco real ------------------------------------

TEST_CASE("save_game: gravacoes sucessivas no mesmo slot rotacionam backup1 em "
          "disco (write_with_backup_rotation real, nao so em memoria)",
          "[platform][fs][save]") {
    const auto dir = make_temp_dir("backup");
    SaveData first = make_valid_save(1);
    first.playtime_seconds = 1.0;
    REQUIRE(save_game(first, 1, dir.string()));

    SaveData second = make_valid_save(1);
    second.playtime_seconds = 2.0;
    REQUIRE(save_game(second, 1, dir.string()));

    REQUIRE(std::filesystem::exists(dir / "save_1.sav"));
    REQUIRE(std::filesystem::exists(dir / "save_1.backup1.sav"));

    // backup1 em disco e a geracao ANTERIOR (primeira gravacao), lida via
    // deserialize_save direto (uso interno de teste - a app usa load_save).
    std::ifstream in(dir / "save_1.backup1.sav", std::ios::binary);
    const std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)),
                                           std::istreambuf_iterator<char>());
    const SaveData backup_data = gus::domain::save::deserialize_save(bytes);
    REQUIRE(backup_data.playtime_seconds == 1.0);

    std::filesystem::remove_all(dir);
}

// ---- T1.2: arquivo trocado de slot -------------------------------------------

TEST_CASE("load_game: arquivo do slot 1 copiado pro slot 2 devolve "
          "LoadResult::WrongSlot (slot_id selado diverge do slot fisico)",
          "[platform][fs][save]") {
    const auto dir = make_temp_dir("wrong_slot");
    const SaveData data = make_valid_save(1);  // slot_id selado = 1
    REQUIRE(save_game(data, 1, dir.string()));

    // Simula troca de arquivo entre slots no gerenciador de arquivos (fora do jogo):
    // copia o BYTE-A-BYTE do slot 1 pro nome logico do slot 2.
    std::filesystem::copy_file(dir / "save_1.sav", dir / "save_2.sav",
                                std::filesystem::copy_options::overwrite_existing);

    const auto outcome = load_game(2, dir.string());
    REQUIRE(outcome.has_value());
    REQUIRE(outcome->result == LoadResult::WrongSlot);
    // Mesmo em WrongSlot, os dados estao integros (T1.2: so a origem diverge).
    REQUIRE(outcome->data.current_scene_path == "city_intro");

    std::filesystem::remove_all(dir);
}

// ---- fail-fast: slot invalido e erro de programacao, nao I/O -----------------

TEST_CASE("save_game: slot invalido lanca std::out_of_range (fail-fast, nao "
          "degradacao de I/O)",
          "[platform][fs][save]") {
    const auto dir = make_temp_dir("slot_invalido_save");
    const SaveData data = make_valid_save(99);
    REQUIRE_THROWS_AS(save_game(data, 99, dir.string()), std::out_of_range);
    std::filesystem::remove_all(dir);
}

TEST_CASE("load_game: slot invalido lanca std::out_of_range (fail-fast, nao "
          "degradacao de I/O)",
          "[platform][fs][save]") {
    const auto dir = make_temp_dir("slot_invalido_load");
    REQUIRE_THROWS_AS(load_game(99, dir.string()), std::out_of_range);
    std::filesystem::remove_all(dir);
}

// ---- delete_save (feature "Apagar", SAVE-LOAD-UI etapa 6) --------------------

TEST_CASE("delete_save: apaga o primario + a cadeia INTEIRA de backup em disco "
          "(slot fica vazio de fato, nao so o primario)",
          "[platform][fs][save]") {
    const auto dir = make_temp_dir("delete_com_backup");
    SaveData first = make_valid_save(1);
    first.playtime_seconds = 1.0;
    REQUIRE(save_game(first, 1, dir.string()));
    SaveData second = make_valid_save(1);
    second.playtime_seconds = 2.0;
    REQUIRE(save_game(second, 1, dir.string()));  // gera save_1.backup1.sav
    REQUIRE(std::filesystem::exists(dir / "save_1.sav"));
    REQUIRE(std::filesystem::exists(dir / "save_1.backup1.sav"));

    REQUIRE(delete_save(1, dir.string()));

    REQUIRE_FALSE(std::filesystem::exists(dir / "save_1.sav"));
    REQUIRE_FALSE(std::filesystem::exists(dir / "save_1.backup1.sav"));
    REQUIRE_FALSE(has_save(1, dir.string()));
    REQUIRE_FALSE(load_game(1, dir.string()).has_value());  // volta a "slot vazio"

    std::filesystem::remove_all(dir);
}

TEST_CASE("delete_save: slot ja vazio e no-op seguro (idempotente, devolve true)",
          "[platform][fs][save]") {
    const auto dir = make_temp_dir("delete_ja_vazio");
    REQUIRE_FALSE(has_save(1, dir.string()));
    REQUIRE(delete_save(1, dir.string()));
    REQUIRE_FALSE(has_save(1, dir.string()));
    std::filesystem::remove_all(dir);
}

TEST_CASE("delete_save: apaga o autosave (slot 0) igual a qualquer manual (decisao "
          "do lider: Auto tambem apagavel)",
          "[platform][fs][save]") {
    const auto dir = make_temp_dir("delete_autosave");
    const SaveData data = make_valid_save(gus::domain::save::kAutosaveSlot);
    REQUIRE(save_game(data, gus::domain::save::kAutosaveSlot, dir.string()));
    REQUIRE(delete_save(gus::domain::save::kAutosaveSlot, dir.string()));
    REQUIRE_FALSE(has_save(gus::domain::save::kAutosaveSlot, dir.string()));
    std::filesystem::remove_all(dir);
}

TEST_CASE("delete_save: nao mexe em OUTROS slots (so o alvo)",
          "[platform][fs][save]") {
    const auto dir = make_temp_dir("delete_isola_outros_slots");
    REQUIRE(save_game(make_valid_save(1), 1, dir.string()));
    REQUIRE(save_game(make_valid_save(2), 2, dir.string()));

    REQUIRE(delete_save(1, dir.string()));

    REQUIRE_FALSE(has_save(1, dir.string()));
    REQUIRE(has_save(2, dir.string()));
    std::filesystem::remove_all(dir);
}

TEST_CASE("delete_save: slot invalido lanca std::out_of_range (fail-fast, nao "
          "degradacao de I/O)",
          "[platform][fs][save]") {
    const auto dir = make_temp_dir("delete_slot_invalido");
    REQUIRE_THROWS_AS(delete_save(99, dir.string()), std::out_of_range);
    std::filesystem::remove_all(dir);
}

// ---- secure_wipe_save (MODOS-MORTE Fase 0, Camada 3 essencial) ---------------

TEST_CASE("secure_wipe_save: apaga primario + cadeia INTEIRA de backup - "
          "load_game e load_game_from_backup NAO recuperam mais",
          "[platform][fs][save][secure_wipe]") {
    using gus::platform::fs::secure_wipe_save;

    const auto dir = make_temp_dir("secure_wipe_com_backup");
    SaveData first = make_valid_save(1);
    first.playtime_seconds = 1.0;
    REQUIRE(save_game(first, 1, dir.string()));
    SaveData second = make_valid_save(1);
    second.playtime_seconds = 2.0;
    REQUIRE(save_game(second, 1, dir.string()));  // gera save_1.backup1.sav
    REQUIRE(std::filesystem::exists(dir / "save_1.sav"));
    REQUIRE(std::filesystem::exists(dir / "save_1.backup1.sav"));
    // sanity ANTES do wipe: o slot carrega normalmente (prova que o teste nao
    // esta testando um slot ja quebrado por outro motivo).
    REQUIRE(load_game(1, dir.string()).has_value());
    REQUIRE(load_game(1, dir.string())->result == LoadResult::Ok);

    REQUIRE(secure_wipe_save(1, dir.string()));

    // Os 4 arquivos (primario + os N backups, so 1 gerado aqui) sumiram de disco.
    REQUIRE_FALSE(std::filesystem::exists(dir / "save_1.sav"));
    REQUIRE_FALSE(std::filesystem::exists(dir / "save_1.backup1.sav"));
    REQUIRE_FALSE(has_save(1, dir.string()));
    // O CORE do pedido: nem o load direto nem a recuperacao via backup revivem o
    // slot depois do wipe (a morte no Hardcore e final).
    REQUIRE_FALSE(load_game(1, dir.string()).has_value());
    REQUIRE_FALSE(load_game_from_backup(1, dir.string()).has_value());

    std::filesystem::remove_all(dir);
}

TEST_CASE("secure_wipe_save: cadeia de backup completa (3 geracoes) some inteira",
          "[platform][fs][save][secure_wipe]") {
    using gus::platform::fs::secure_wipe_save;

    const auto dir = make_temp_dir("secure_wipe_cadeia_completa");
    // 4 gravacoes sucessivas: primario + backup1 + backup2 + backup3 (a cadeia
    // inteira, kBackupChainDepth=3) ficam ocupados.
    for (int i = 0; i < 4; ++i) {
        SaveData s = make_valid_save(1);
        s.playtime_seconds = static_cast<double>(i);
        REQUIRE(save_game(s, 1, dir.string()));
    }
    REQUIRE(std::filesystem::exists(dir / "save_1.sav"));
    REQUIRE(std::filesystem::exists(dir / "save_1.backup1.sav"));
    REQUIRE(std::filesystem::exists(dir / "save_1.backup2.sav"));
    REQUIRE(std::filesystem::exists(dir / "save_1.backup3.sav"));

    REQUIRE(secure_wipe_save(1, dir.string()));

    REQUIRE_FALSE(std::filesystem::exists(dir / "save_1.sav"));
    REQUIRE_FALSE(std::filesystem::exists(dir / "save_1.backup1.sav"));
    REQUIRE_FALSE(std::filesystem::exists(dir / "save_1.backup2.sav"));
    REQUIRE_FALSE(std::filesystem::exists(dir / "save_1.backup3.sav"));
    REQUIRE_FALSE(load_game_from_backup(1, dir.string()).has_value());

    std::filesystem::remove_all(dir);
}

TEST_CASE("secure_wipe_save: slot ja vazio e no-op seguro (idempotente, devolve "
          "true)",
          "[platform][fs][save][secure_wipe]") {
    using gus::platform::fs::secure_wipe_save;

    const auto dir = make_temp_dir("secure_wipe_ja_vazio");
    REQUIRE_FALSE(has_save(1, dir.string()));
    REQUIRE(secure_wipe_save(1, dir.string()));
    REQUIRE_FALSE(has_save(1, dir.string()));
    std::filesystem::remove_all(dir);
}

TEST_CASE("secure_wipe_save: nao mexe em OUTROS slots (so o alvo)",
          "[platform][fs][save][secure_wipe]") {
    using gus::platform::fs::secure_wipe_save;

    const auto dir = make_temp_dir("secure_wipe_isola_outros_slots");
    REQUIRE(save_game(make_valid_save(1), 1, dir.string()));
    REQUIRE(save_game(make_valid_save(2), 2, dir.string()));

    REQUIRE(secure_wipe_save(1, dir.string()));

    REQUIRE_FALSE(has_save(1, dir.string()));
    REQUIRE(has_save(2, dir.string()));
    REQUIRE(load_game(2, dir.string()).has_value());
    std::filesystem::remove_all(dir);
}

TEST_CASE("secure_wipe_save: slot invalido lanca std::out_of_range (fail-fast, "
          "nao degradacao de I/O)",
          "[platform][fs][save][secure_wipe]") {
    using gus::platform::fs::secure_wipe_save;

    const auto dir = make_temp_dir("secure_wipe_slot_invalido");
    REQUIRE_THROWS_AS(secure_wipe_save(99, dir.string()), std::out_of_range);
    std::filesystem::remove_all(dir);
}

// ---- load_game_from_backup (SAVE-LOAD-AVISOS, "Tentar recuperar") ------------

TEST_CASE("load_game_from_backup: primario corrompido + backup1 bom recupera "
          "via backup1 (1a geracao boa, a mais fresca)",
          "[platform][fs][save][save-load-avisos]") {
    const auto dir = make_temp_dir("recover_backup1_bom");
    SaveData first = make_valid_save(1);
    first.playtime_seconds = 1.0;
    REQUIRE(save_game(first, 1, dir.string()));  // vira backup1 na 2a gravacao

    SaveData second = make_valid_save(1);
    second.playtime_seconds = 2.0;
    REQUIRE(save_game(second, 1, dir.string()));  // gera save_1.backup1.sav (= first)

    // Corrompe o PRIMARIO (mesma tecnica de "adulteracao: HMAC nao bate" acima).
    {
        std::fstream f(dir / "save_1.sav", std::ios::in | std::ios::out | std::ios::binary);
        REQUIRE(f.is_open());
        char byte = 0;
        f.seekg(10);
        f.read(&byte, 1);
        byte = static_cast<char>(byte ^ 0xFF);
        f.seekp(10);
        f.write(&byte, 1);
    }
    REQUIRE(load_game(1, dir.string())->result != LoadResult::Ok);  // confirma a falha do primario

    const auto recovered = load_game_from_backup(1, dir.string());
    REQUIRE(recovered.has_value());
    REQUIRE(recovered->result == LoadResult::Ok);
    REQUIRE(recovered->data.playtime_seconds == 1.0);  // backup1 = a geracao ANTERIOR (first)

    std::filesystem::remove_all(dir);
}

TEST_CASE("load_game_from_backup: backup1 TAMBEM corrompido cai pro backup2 bom",
          "[platform][fs][save][save-load-avisos]") {
    const auto dir = make_temp_dir("recover_backup2_bom");
    SaveData gen1 = make_valid_save(1);
    gen1.playtime_seconds = 1.0;
    REQUIRE(save_game(gen1, 1, dir.string()));

    SaveData gen2 = make_valid_save(1);
    gen2.playtime_seconds = 2.0;
    REQUIRE(save_game(gen2, 1, dir.string()));  // gen1 -> backup1

    SaveData gen3 = make_valid_save(1);
    gen3.playtime_seconds = 3.0;
    REQUIRE(save_game(gen3, 1, dir.string()));  // gen2 -> backup1, gen1 -> backup2

    // Corrompe o PRIMARIO (gen3) e o backup1 (gen2) - so o backup2 (gen1) segue Ok.
    for (const char* name : {"save_1.sav", "save_1.backup1.sav"}) {
        std::fstream f(dir / name, std::ios::in | std::ios::out | std::ios::binary);
        REQUIRE(f.is_open());
        char byte = 0;
        f.seekg(10);
        f.read(&byte, 1);
        byte = static_cast<char>(byte ^ 0xFF);
        f.seekp(10);
        f.write(&byte, 1);
    }

    const auto recovered = load_game_from_backup(1, dir.string());
    REQUIRE(recovered.has_value());
    REQUIRE(recovered->result == LoadResult::Ok);
    REQUIRE(recovered->data.playtime_seconds == 1.0);  // backup2 = gen1

    std::filesystem::remove_all(dir);
}

TEST_CASE("load_game_from_backup: TODAS as geracoes ruins (ou ausentes) devolve "
          "nullopt (recuperacao falhou)",
          "[platform][fs][save][save-load-avisos]") {
    const auto dir = make_temp_dir("recover_falha_total");
    const SaveData data = make_valid_save(1);
    REQUIRE(save_game(data, 1, dir.string()));  // so o primario existe, sem backup ainda

    REQUIRE_FALSE(load_game_from_backup(1, dir.string()).has_value());

    std::filesystem::remove_all(dir);
}

TEST_CASE("load_game_from_backup: slot sem NENHUM arquivo (nem primario) devolve "
          "nullopt",
          "[platform][fs][save][save-load-avisos]") {
    const auto dir = make_temp_dir("recover_slot_vazio");
    REQUIRE_FALSE(load_game_from_backup(1, dir.string()).has_value());
    std::filesystem::remove_all(dir);
}

TEST_CASE("load_game_from_backup: slot invalido lanca std::out_of_range "
          "(fail-fast, nao degradacao de I/O)",
          "[platform][fs][save][save-load-avisos]") {
    const auto dir = make_temp_dir("recover_slot_invalido");
    REQUIRE_THROWS_AS(load_game_from_backup(99, dir.string()), std::out_of_range);
    std::filesystem::remove_all(dir);
}

// ---- FsSaveStore diretamente (o port sobre arquivos reais) -------------------

TEST_CASE("FsSaveStore: write + read + exists + move + remove sobre arquivos "
          "reais (o port SaveStore, nao so o wrapper save_game/load_game)",
          "[platform][fs][save]") {
    const auto dir = make_temp_dir("store_direto");
    std::filesystem::create_directories(dir);
    FsSaveStore store(dir.string());

    REQUIRE_FALSE(store.exists("foo"));
    store.write("foo", {1, 2, 3});
    REQUIRE(store.exists("foo"));
    REQUIRE(store.read("foo") == std::vector<std::uint8_t>{1, 2, 3});

    store.move("foo", "bar");
    REQUIRE_FALSE(store.exists("foo"));
    REQUIRE(store.exists("bar"));
    REQUIRE(store.read("bar") == std::vector<std::uint8_t>{1, 2, 3});

    store.remove("bar");
    REQUIRE_FALSE(store.exists("bar"));

    // move de origem ausente e no-op (contrato do port, save_backup_test.cpp espelha
    // o mesmo comportamento no InMemorySaveStore).
    store.move("nao_existe", "tambem_nao");
    REQUIRE_FALSE(store.exists("tambem_nao"));

    std::filesystem::remove_all(dir);
}

TEST_CASE("FsSaveStore: read de nome ausente lanca std::out_of_range (mesmo "
          "contrato do InMemorySaveStore)",
          "[platform][fs][save]") {
    const auto dir = make_temp_dir("store_ausente");
    std::filesystem::create_directories(dir);
    FsSaveStore store(dir.string());
    REQUIRE_THROWS_AS(store.read("nao_existe"), std::out_of_range);
    std::filesystem::remove_all(dir);
}
