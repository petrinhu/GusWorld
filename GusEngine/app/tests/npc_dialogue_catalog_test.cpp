// GusEngine/app/tests/npc_dialogue_catalog_test.cpp
//
// Catch2 (headless) do I/O do grafo de dialogo do Bertoldo (M7-DIALOGO, NPC-MVP):
// resolve_npc_intro_bertoldo_dialogue_path (resolucao de caminho) +
// load_dialogue_graph_from_file (le + delega ao parser POCO de dominio). Prova,
// entre outras coisas, que o ARQUIVO REAL SHIPADO (resources/dialogues/
// npc_intro_bertoldo.dlg.txt, M8 decommission moveu de game/dialogues/ via git mv)
// parseia sem erro fim-a-fim (nao so um fixture inline).

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>

#include "gus/app/dialogue/npc_dialogue_catalog.hpp"
#include "gus/domain/dialogue/dialogue_text.hpp"  // DialogueTextError

using gus::app::dialogue::load_dialogue_graph_from_file;
using gus::app::dialogue::resolve_npc_intro_bertoldo_dialogue_path;
using gus::domain::dialogue::DialogueRegister;

TEST_CASE("resolve_npc_intro_bertoldo_dialogue_path: contem o nome do arquivo",
          "[npc_dialogue][catalog]") {
    const std::string path = resolve_npc_intro_bertoldo_dialogue_path();
    CHECK(path.find("npc_intro_bertoldo.dlg.txt") != std::string::npos);
}

TEST_CASE("load_dialogue_graph_from_file: arquivo ausente devolve nullopt "
          "(degradacao segura)",
          "[npc_dialogue][catalog]") {
    const auto result =
        load_dialogue_graph_from_file("/caminho/que/nao/existe/x.dlg.txt");
    CHECK_FALSE(result.has_value());
}

TEST_CASE("load_dialogue_graph_from_file: conteudo vazio devolve nullopt",
          "[npc_dialogue][catalog]") {
    const auto dir = std::filesystem::temp_directory_path() /
                      "gusworld_npc_dialogue_catalog_test_vazio";
    std::filesystem::create_directories(dir);
    const auto path = dir / "vazio.dlg.txt";
    {
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
    }
    const auto result = load_dialogue_graph_from_file(path.string());
    CHECK_FALSE(result.has_value());
    std::filesystem::remove_all(dir);
}

TEST_CASE("load_dialogue_graph_from_file: conteudo malformado PROPAGA "
          "DialogueTextError (fail-fast de autoria, nao um caso de I/O)",
          "[npc_dialogue][catalog]") {
    const auto dir = std::filesystem::temp_directory_path() /
                      "gusworld_npc_dialogue_catalog_test_malformado";
    std::filesystem::create_directories(dir);
    const auto path = dir / "malformado.dlg.txt";
    {
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        out << "isso nao segue a sintaxe do formato-texto\n";
    }
    CHECK_THROWS_AS(load_dialogue_graph_from_file(path.string()),
                    gus::domain::dialogue::DialogueTextError);
    std::filesystem::remove_all(dir);
}

TEST_CASE("load_dialogue_graph_from_file: o ARQUIVO REAL shipado "
          "(resources/dialogues/npc_intro_bertoldo.dlg.txt) parseia OK fim-a-fim",
          "[npc_dialogue][catalog][integracao]") {
    const std::string path = resolve_npc_intro_bertoldo_dialogue_path();
    const auto graph = load_dialogue_graph_from_file(path);
    REQUIRE(graph.has_value());
    CHECK(graph->dialogue_id == "npc_intro_bertoldo");
    CHECK(graph->default_register == DialogueRegister::Warm);
    CHECK(graph->entry_node_id == "n0_greet");
    CHECK(graph->nodes.size() == 6u);
    REQUIRE(graph->nodes.count("n1_hook") == 1u);
    CHECK(graph->nodes.at("n1_hook").options.size() == 3u);
}
