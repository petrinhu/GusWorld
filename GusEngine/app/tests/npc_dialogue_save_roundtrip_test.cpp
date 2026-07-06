// GusEngine/app/tests/npc_dialogue_save_roundtrip_test.cpp
//
// Catch2 (headless, I/O REAL em disco - diretorio TEMPORARIO proprio, mesmo padrao
// hermetico de platform/tests/save_file_store_test.cpp) do CRITERIO DE SAIDA do
// M7-DIALOGO/NPC-MVP: "falar com 1 NPC, escolha muda flag, flag sobrevive a save/
// load round-trip". Junta as 3 pontas:
//   1) o ARQUIVO REAL do Bertoldo (game/dialogues/npc_intro_bertoldo.dlg.txt),
//      carregado via gus::app::dialogue::load_dialogue_graph_from_file;
//   2) o runtime POCO (gus::domain::dialogue::DialogueRuntime) escrevendo em
//      SaveData::flags por referencia (ADR-014: domain/dialogue NAO depende de
//      domain/save - a integracao acontece aqui, no app/, como no Maestro real);
//   3) o I/O REAL de save/load (gus::platform::fs::save_game/load_game, FsSaveStore,
//      M2-SAVE-IO) - prova que a flag setada pela escolha do jogador SOBREVIVE a um
//      round-trip de verdade em disco, nao so em memoria.

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <string>

#include "gus/app/dialogue/npc_dialogue_catalog.hpp"
#include "gus/domain/dialogue/dialogue_runtime.hpp"
#include "gus/domain/save/save_data.hpp"
#include "gus/domain/save/save_serializer.hpp"
#include "gus/platform/fs/save_file_store.hpp"

using gus::app::dialogue::load_dialogue_graph_from_file;
using gus::app::dialogue::resolve_npc_intro_bertoldo_dialogue_path;
using gus::domain::dialogue::DialogueRuntime;
using gus::domain::save::LoadResult;
using gus::domain::save::SaveData;
using gus::platform::fs::load_game;
using gus::platform::fs::save_game;

namespace {

std::filesystem::path make_temp_dir(const char* suffix) {
    auto dir = std::filesystem::temp_directory_path() /
               (std::string("gusworld_npc_dialogue_roundtrip_") + suffix);
    std::filesystem::remove_all(dir);
    return dir;
}

// SaveData minimo mas valido (validate() nao lanca), slot_id coerente com o slot
// fisico (contrato de save_game - MESMO padrao de save_file_store_test.cpp).
SaveData make_valid_save(int slot) {
    SaveData data;
    data.timestamp_ms = 1000;
    data.playtime_seconds = 12.0;
    data.current_scene_path = "distritos_inferiores";
    data.party_roster = {"gus"};
    data.party_active = {"gus"};
    data.slot_id = slot;
    return data;
}

}  // namespace

TEST_CASE("M7-DIALOGO NPC-MVP: falar com o Bertoldo, escolher um ramo, e a flag "
          "SOBREVIVE a um save/load round-trip REAL em disco (criterio de saida)",
          "[npc_dialogue][save][roundtrip][integracao]") {
    // 1) Carrega o grafo REAL (nao um fixture inline) do disco.
    const auto graph = load_dialogue_graph_from_file(
        resolve_npc_intro_bertoldo_dialogue_path());
    REQUIRE(graph.has_value());

    // 2) Roda a conversa: entra (seta npc_intro.met), avanca ate a escolha, escolhe
    // o ramo CURIOSO, converge, sai.
    SaveData data = make_valid_save(1);
    REQUIRE_FALSE(data.flags.count("npc_intro.met"));

    {
        DialogueRuntime rt(*graph, data.flags);
        rt.enter();  // n0_greet: on_enter seta npc_intro.met=true
        CHECK(data.flags.at("npc_intro.met") == true);
        REQUIRE(rt.current().id == "n0_greet");

        rt.advance();  // -> n1_hook (no de escolha)
        REQUIRE(rt.current().id == "n1_hook");
        REQUIRE(rt.current().options.size() == 3u);

        rt.choose(0);  // ramo CURIOSO (option 0) -> npc_intro.choice_curioso=true
        CHECK(data.flags.at("npc_intro.choice_curioso") == true);
        CHECK_FALSE(data.flags.count("npc_intro.choice_pragmatico"));
        CHECK_FALSE(data.flags.count("npc_intro.choice_seco"));

        rt.advance();  // n2a_curioso -> n3_reconverge
        REQUIRE(rt.current().id == "n3_reconverge");
        rt.advance();  // -> @exit
        REQUIRE(rt.finished());
    }

    // 3) Persiste em disco (I/O REAL, FsSaveStore) e RECARREGA - prova o round-trip.
    const auto dir = make_temp_dir("m7_dialogo_bertoldo");
    REQUIRE(save_game(data, 1, dir.string()));

    const auto outcome = load_game(1, dir.string());
    REQUIRE(outcome.has_value());
    REQUIRE(outcome->result == LoadResult::Ok);

    // A FLAG sobrevive - este e o criterio de saida do M7-DIALOGO (NPC-MVP).
    const auto& loaded_flags = outcome->data.flags;
    REQUIRE(loaded_flags.count("npc_intro.met") == 1u);
    CHECK(loaded_flags.at("npc_intro.met") == true);
    REQUIRE(loaded_flags.count("npc_intro.choice_curioso") == 1u);
    CHECK(loaded_flags.at("npc_intro.choice_curioso") == true);
    CHECK_FALSE(loaded_flags.count("npc_intro.choice_pragmatico"));
    CHECK_FALSE(loaded_flags.count("npc_intro.choice_seco"));

    std::filesystem::remove_all(dir);
}

TEST_CASE("M7-DIALOGO NPC-MVP: os 3 ramos de escolha sao MUTUAMENTE EXCLUSIVOS na "
          "flag persistida (escolher pragmatico NAO seta os outros 2)",
          "[npc_dialogue][save][roundtrip]") {
    const auto graph = load_dialogue_graph_from_file(
        resolve_npc_intro_bertoldo_dialogue_path());
    REQUIRE(graph.has_value());

    SaveData data = make_valid_save(2);
    {
        DialogueRuntime rt(*graph, data.flags);
        rt.enter();
        rt.advance();
        rt.choose(1);  // ramo PRAGMATICO (option 1)
        rt.advance();
        rt.advance();
        REQUIRE(rt.finished());
    }

    const auto dir = make_temp_dir("m7_dialogo_pragmatico");
    REQUIRE(save_game(data, 2, dir.string()));
    const auto outcome = load_game(2, dir.string());
    REQUIRE(outcome.has_value());
    REQUIRE(outcome->result == LoadResult::Ok);

    const auto& loaded_flags = outcome->data.flags;
    CHECK(loaded_flags.at("npc_intro.choice_pragmatico") == true);
    CHECK_FALSE(loaded_flags.count("npc_intro.choice_curioso"));
    CHECK_FALSE(loaded_flags.count("npc_intro.choice_seco"));

    std::filesystem::remove_all(dir);
}
