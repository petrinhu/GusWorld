// GusEngine/app/tests/battle_cockpit_rml_test.cpp
//
// AC-E11 A2 (ADR-019): teste de CARACTERIZACAO da montagem RML/RCSS do cockpit,
// extraida de battle_preview.cpp pra battle_cockpit_rml.cpp. Cada TEST_CASE captura o
// que write_smoke_glintfx_rml/write_baked_cockpit_rml/write_live_cockpit_rml escrevem
// HOJE e trava esse comportamento (nao o "ideal") - o objetivo e detectar REGRESSAO na
// extracao (e em qualquer refator futuro), nao validar design. Headless, sem SDL_Init
// nem janela: as 3 funcoes escrevem um .rml num tempfile (fs::temp_directory_path()) e
// devolvem o path; aqui so lemos o arquivo de volta e checamos marcadores estruturais
// (MESMO espirito de title_menu_rml_test.cpp/system_menu_rml_test.cpp - string, nao
// pixel/renderizacao real).
//
// cockpit_retrato_flat_for/glintfx_cockpit_stage_dir/kVerbLabels sao POCO puro
// (sem I/O) - testados direto, sem tempfile.
//
// Cross-ref: gus/app/screens/battle_cockpit_rml.hpp (declaracoes);
//            docs/tech/adr/ADR-019.

#include <fstream>
#include <sstream>

#include <catch2/catch_test_macros.hpp>

#include "gus/app/screens/battle_cockpit_rml.hpp"
#include "gus/domain/combat/combat_actor.hpp"

using namespace gus::app::screens;
using gus::domain::combat::CardFamily;
using gus::domain::combat::CombatActor;

namespace {

// Le o arquivo inteiro num string - helper generico pra ler de volta o .rml que as
// funcoes escreveram no tempfile.
std::string slurp(const std::string& path) {
    std::ifstream f(path);
    REQUIRE(f.good());
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

CombatActor make_gus() {
    return CombatActor("gus", "Gus", /*max_hp=*/55, /*atk=*/10, /*def=*/5, /*spd=*/8,
                        CardFamily::Universal, /*is_player_side=*/true,
                        /*is_boss=*/false, /*knowledge_kills=*/0,
                        /*is_universal_compiler=*/true);
}

CombatActor make_caua() {
    return CombatActor("caua", "Cauã", /*max_hp=*/40, /*atk=*/12, /*def=*/4, /*spd=*/9,
                        CardFamily::Eletrico, /*is_player_side=*/true);
}

CombatActor make_enemy() {
    return CombatActor("inimigo1", "Daemon-Corrompido", /*max_hp=*/30, /*atk=*/8,
                        /*def=*/3, /*spd=*/6, CardFamily::Bioquimico,
                        /*is_player_side=*/false);
}

}  // namespace

TEST_CASE("kVerbLabels: 6 rotulos, ordem = BattleVerb (Scan/Gambito/Atacar/Defender/"
          "Compilar/Fugir)",
          "[battle_cockpit_rml]") {
    REQUIRE(kVerbLabels.size() == 6);
    REQUIRE(kVerbLabels[0] == "SCAN");
    REQUIRE(kVerbLabels[1] == "GAMBITO");
    REQUIRE(kVerbLabels[2] == "ATACAR");
    REQUIRE(kVerbLabels[3] == "DEFENDER");
    REQUIRE(kVerbLabels[4] == "COMPILAR");
    REQUIRE(kVerbLabels[5] == "FUGIR");
}

TEST_CASE("cockpit_retrato_flat_for: inimigo usa o retrato generico, Gus usa a versao "
          "no-bg, resto da party usa retrato_file_for",
          "[battle_cockpit_rml]") {
    const CombatActor gus = make_gus();
    const CombatActor caua = make_caua();
    const CombatActor enemy = make_enemy();

    REQUIRE(cockpit_retrato_flat_for(enemy) == "retrato_inimigo.png");
    REQUIRE(cockpit_retrato_flat_for(gus) == "retrato_gus_combate_nobg.png");
    REQUIRE(cockpit_retrato_flat_for(caua) == "retrato_caua.png");
}

TEST_CASE("glintfx_cockpit_stage_dir: caminho nao-vazio na pasta de tempfile do stage",
          "[battle_cockpit_rml]") {
    const std::string stage = glintfx_cockpit_stage_dir();
    REQUIRE_FALSE(stage.empty());
    REQUIRE(stage.find("gusworld_glintfx_cockpit") != std::string::npos);
    // Estavel entre chamadas (mesma pasta - o stage dir e reusado pelas 3 variantes).
    REQUIRE(glintfx_cockpit_stage_dir() == stage);
}

TEST_CASE("write_smoke_glintfx_rml: DIV com gradiente/glow em px, sem @font-face nem "
          "data-model (ADR-010 F1, degrau mais simples)",
          "[battle_cockpit_rml]") {
    const std::string path = write_smoke_glintfx_rml();
    const std::string rml = slurp(path);

    REQUIRE(rml.find("id=\"smoke\"") != std::string::npos);
    REQUIRE(rml.find("vertical-gradient") != std::string::npos);
    REQUIRE(rml.find("box-shadow") != std::string::npos);
    // Unidades em px (nao dp) - o smoke e deterministico sem dp_ratio.
    REQUIRE(rml.find("80px") != std::string::npos);
    REQUIRE(rml.find("@font-face") == std::string::npos);
    REQUIRE(rml.find("data-model") == std::string::npos);
}

TEST_CASE("write_baked_cockpit_rml(intro=false): bindings viram literais (Gus 55/55), "
          "data-model removido, bloco de ABERTURA ausente, bloco de COMBATE presente",
          "[battle_cockpit_rml]") {
    const std::string path = write_baked_cockpit_rml(/*intro=*/false);
    const std::string rml = slurp(path);

    REQUIRE(rml.find("@font-face") != std::string::npos);
    REQUIRE(rml.find("data-model") == std::string::npos);
    REQUIRE(rml.find("{{nome}}") == std::string::npos);
    REQUIRE(rml.find(">Gus<") != std::string::npos);
    REQUIRE(rml.find("VETOR DO GAMBITO") != std::string::npos);
    REQUIRE(rml.find(">55<") != std::string::npos);
    REQUIRE(rml.find("id=\"combat\"") != std::string::npos);
    REQUIRE(rml.find("id=\"opening\"") == std::string::npos);
    // data-if e' removido do ATRIBUTO (nao ha data-model pra resolver): o bloco que
    // fica NAO carrega mais o atributo. "data-if" como PALAVRA ainda sobrevive nos
    // comentarios HTML do doc autorado ("<!-- ... (data-if !intro). -->") - por isso o
    // teste checa a forma do ATRIBUTO (com aspas), nao a substring nua.
    REQUIRE(rml.find("data-if=\"intro\"") == std::string::npos);
    REQUIRE(rml.find("data-if=\"!intro\"") == std::string::npos);
    // moldura/retrato achatados no stage (nome de arquivo flat, sem subpasta).
    REQUIRE(rml.find("retratos/retrato_gus_combate_nobg.png") == std::string::npos);
    REQUIRE(rml.find("retrato_gus_combate_nobg.png") != std::string::npos);
}

TEST_CASE("write_baked_cockpit_rml(intro=true): bloco de ABERTURA presente, bloco de "
          "COMBATE ausente",
          "[battle_cockpit_rml]") {
    const std::string path = write_baked_cockpit_rml(/*intro=*/true);
    const std::string rml = slurp(path);

    REQUIRE(rml.find("id=\"opening\"") != std::string::npos);
    REQUIRE(rml.find("GUSWORLD") != std::string::npos);
    REQUIRE(rml.find("id=\"combat\"") == std::string::npos);
    // Mesma ressalva do teste intro=false: checa o ATRIBUTO (com aspas), nao a
    // substring nua ("data-if" sobrevive em comentario HTML do doc autorado).
    REQUIRE(rml.find("data-if=\"intro\"") == std::string::npos);
    REQUIRE(rml.find("data-if=\"!intro\"") == std::string::npos);
}

TEST_CASE("write_live_cockpit_rml: mantem data-model + bindings vivos, foco navegavel "
          "por pill, data-class-sel por indice, log data-for",
          "[battle_cockpit_rml]") {
    const std::string path = write_live_cockpit_rml();
    const std::string rml = slurp(path);

    REQUIRE(rml.find("@font-face") != std::string::npos);
    REQUIRE(rml.find("data-model=\"hud\"") != std::string::npos);
    // Bindings PRESERVADOS (nao viram literal, diferente do BAKED).
    REQUIRE(rml.find("{{nome}}") != std::string::npos);
    REQUIRE(rml.find("{{hp}}") != std::string::npos);
    // Foco navegavel injetado nos pills.
    REQUIRE(rml.find("tab-index: auto; nav: auto;") != std::string::npos);
    // data-class-sel por indice (ordem = BattleVerb: 0=Scan..5=Fugir).
    REQUIRE(rml.find("data-class-sel=\"sel == 0\"") != std::string::npos);
    REQUIRE(rml.find("data-class-sel=\"sel == 2\"") != std::string::npos);
    REQUIRE(rml.find("data-class-sel=\"sel == 5\"") != std::string::npos);
    // Retrato vivo: decorator dirigido por binding, nao mais estatico so.
    REQUIRE(rml.find("data-style-decorator=") != std::string::npos);
    REQUIRE(rml.find("retrato_src") != std::string::npos);
    // Log vivo: data-for sobre a lista + now-line com verbo/alvo.
    REQUIRE(rml.find("data-for=\"line : log\"") != std::string::npos);
    REQUIRE(rml.find("{{verb}}") != std::string::npos);
    REQUIRE(rml.find("{{alvo}}") != std::string::npos);
    // Os 3 retratos que o ator ATIVO pode assumir (Gus/inimigo generico/Cauã/Jaci) sao
    // referenciados no RCSS estatico OU copiados pro stage - a fonte (Gus) segue no doc.
    REQUIRE(rml.find("retrato_gus_combate_nobg.png") != std::string::npos);
}
