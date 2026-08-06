// SPDX-License-Identifier: Apache-2.0
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

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <system_error>

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

// TEMPFILE-PROD-UNICO (2026-08-06): com o stage UNICO POR EXECUCAO, cada processo de
// teste passa a deixar o SEU stage em disco (~372 KB: 2 .ttf + 5 .png + 2 .rml). O stage
// de PRODUCAO nao se auto-remove de proposito (o humano depurando precisa achar o .rml
// depois que o jogo fechou), entao a limpeza e responsabilidade de quem cria lixo em
// TESTE. Medido sem isto: 84 diretorios (~31 MB) sobrando depois de 3 rodadas de 12 pares
// concorrentes. RAII: remove mesmo se o REQUIRE lancar no meio.
struct TempPathCleanup {
    std::filesystem::path path;
    ~TempPathCleanup() {
        std::error_code ec;
        std::filesystem::remove_all(path, ec);
    }
};

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

// TEMPFILE-PROD-UNICO (2026-08-06): o stage e FIXO DENTRO da execucao (as 3 variantes +
// o base_url do chamador precisam do MESMO dir) e UNICO ENTRE execucoes (dois processos
// que exercitem o cockpit nao podem disputar o mesmo dir). Este caso trava as DUAS metades
// da propriedade; a metade "entre execucoes" e provada de verdade, com 2 processos, no
// caso [concorrencia] mais abaixo.
TEST_CASE("glintfx_cockpit_stage_dir: unico por EXECUCAO (nao e o nome fixo eterno), "
          "estavel dentro do processo",
          "[battle_cockpit_rml]") {
    namespace fs = std::filesystem;
    const std::string stage = glintfx_cockpit_stage_dir();
    const TempPathCleanup limpa{stage};
    // Nome fixo eterno = dois processos no mesmo dir (a corrida de TEMPFILE-PROD-UNICO).
    const fs::path legado = fs::temp_directory_path() / "gusworld_glintfx_cockpit";
    REQUIRE(stage != legado.string());
    // O prefixo continua DESCOBRIVEL (o humano acha por glob durante depuracao), mas o
    // nome carrega um sufixo de execucao depois dele.
    const std::string leaf = fs::path(stage).filename().string();
    REQUIRE(leaf.rfind("gusworld_glintfx_cockpit", 0) == 0);
    REQUIRE(leaf.size() > std::string("gusworld_glintfx_cockpit").size());
    // Fixo DENTRO da execucao: battle_preview.cpp pega o dir por fora (base_url) DEPOIS de
    // write_baked/write_live copiarem os assets - se mudasse por chamada, o base_url
    // apontaria pra um dir vazio.
    REQUIRE(glintfx_cockpit_stage_dir() == stage);
    REQUIRE(fs::path(write_baked_cockpit_rml(false)).parent_path().string() == stage);
    REQUIRE(fs::path(write_live_cockpit_rml()).parent_path().string() == stage);
}

// TEMPFILE-PROD-UNICO: o smoke .rml e um arquivo SOLTO no tmp (nao vive no stage - ele
// existe justamente para NAO depender de base-url/assets). Mesmo assim precisa de nome
// unico por execucao: dois processos gravando o mesmo path e a mesma corrida.
TEST_CASE("write_smoke_glintfx_rml: nome unico por EXECUCAO (nao e o .rml fixo eterno)",
          "[battle_cockpit_rml]") {
    namespace fs = std::filesystem;
    const std::string path = write_smoke_glintfx_rml();
    const TempPathCleanup limpa{path};
    const fs::path legado = fs::temp_directory_path() / "gusworld_glintfx_smoke.rml";
    REQUIRE(path != legado.string());
    const std::string leaf = fs::path(path).filename().string();
    REQUIRE(leaf.rfind("gusworld_glintfx_smoke", 0) == 0);
    REQUIRE(fs::path(path).extension() == ".rml");
    // Estavel dentro do processo (o chamador pode reescrever o mesmo arquivo por frame de
    // depuracao sem espalhar lixo).
    REQUIRE(write_smoke_glintfx_rml() == path);
}

// TEMPFILE-PROD-UNICO: a CORRIDA de verdade, com DOIS PROCESSOS.
//
// Por que 2 processos e nao 2 threads: o defeito e caminho previsivel COMPARTILHADO entre
// execucoes independentes (catch_discover_tests roda 1 processo por TEST_CASE, e o cenario
// real e 2 sessoes/agentes rodando a suite ao mesmo tempo). Thread nao reproduz: dentro do
// mesmo processo o path SEMPRE foi o mesmo de proposito.
//
// O filho roda ESTE MESMO binario (/proc/self/exe) com o filtro do caso ESCONDIDO logo
// abaixo, grava a variante intro=true (bloco de ABERTURA) e imprime o path. Depois o pai
// grava a variante MUTUAMENTE EXCLUSIVA (intro=false, bloco de COMBATE). Com nome fixo o
// pai SOBRESCREVE o arquivo do filho e o conteudo do filho vira o do pai - que e
// exatamente a falha medida (0/3/2 falhas em 12 pares concorrentes). Deterministico: nao
// depende de timing, so de os dois caminhos serem iguais ou nao.
//
// Linux-only por causa de /proc/self/exe + popen. Em outra plataforma o caso e PULADO com
// motivo explicito (SKIP) - cobertura declarada, nao fingida.
TEST_CASE("write_baked_cockpit_rml: dois PROCESSOS nao se atropelam (stage unico por "
          "execucao)",
          "[battle_cockpit_rml][concorrencia]") {
#ifndef __linux__
    SKIP("dois-processos depende de /proc/self/exe + popen (POSIX/Linux); nesta "
         "plataforma a corrida entre execucoes fica sem cobertura automatizada");
#else
    namespace fs = std::filesystem;

    // ATENCAO: o /proc/self/exe precisa ser resolvido AQUI, no processo de teste. Passar a
    // string literal "/proc/self/exe" pro popen NAO funciona - o popen roda `sh -c`, e la
    // dentro o /proc/self/exe ja e o do SHELL (o erro que isso da e enganoso: "Arquivo ou
    // diretorio inexistente", que parece binario ausente e nao filtro errado).
    std::error_code ec_exe;
    const fs::path exe = fs::read_symlink("/proc/self/exe", ec_exe);
    INFO("executavel do teste: " << exe.string() << " (ec=" << ec_exe.message() << ")");
    REQUIRE_FALSE(ec_exe);
    REQUIRE(fs::exists(exe));

    std::string child_out;
    {
        const std::string cmd = "\"" + exe.string() + "\" \"[.cockpit_stage_child]\" 2>&1";
        FILE* pipe = ::popen(cmd.c_str(), "r");
        REQUIRE(pipe != nullptr);
        char buf[4096];
        while (std::fgets(buf, sizeof(buf), pipe) != nullptr) child_out += buf;
        const int rc = ::pclose(pipe);
        INFO("saida do processo filho:\n" << child_out);
        REQUIRE(rc == 0);
    }

    const std::string kMarker = "COCKPIT_CHILD_PATH=";
    const std::size_t pos = child_out.find(kMarker);
    INFO("saida do processo filho:\n" << child_out);
    REQUIRE(pos != std::string::npos);
    const std::size_t begin = pos + kMarker.size();
    const std::size_t end = child_out.find('\n', begin);
    REQUIRE(end != std::string::npos);
    const std::string child_path = child_out.substr(begin, end - begin);
    REQUIRE_FALSE(child_path.empty());
    REQUIRE(fs::exists(child_path));

    // O pai grava a variante OPOSTA (combate).
    const std::string parent_path = write_baked_cockpit_rml(/*intro=*/false);
    const TempPathCleanup limpa_pai{fs::path(parent_path).parent_path()};

    // (a) caminhos distintos entre execucoes.
    REQUIRE(parent_path != child_path);
    REQUIRE(fs::path(parent_path).parent_path() != fs::path(child_path).parent_path());

    // (b) o que importa de verdade: o arquivo do FILHO continua com o conteudo do filho.
    const std::string child_rml = slurp(child_path);
    REQUIRE(child_rml.find("id=\"opening\"") != std::string::npos);
    REQUIRE(child_rml.find("id=\"combat\"") == std::string::npos);

    // (c) e o do pai com o do pai.
    const std::string parent_rml = slurp(parent_path);
    REQUIRE(parent_rml.find("id=\"combat\"") != std::string::npos);
    REQUIRE(parent_rml.find("id=\"opening\"") == std::string::npos);

    // Limpeza do stage do FILHO: o stage de producao NAO se auto-remove (de proposito -
    // o humano precisa achar o .rml depois da execucao pra depurar), entao quem cria lixo
    // em teste limpa o proprio lixo.
    std::error_code ec;
    fs::remove_all(fs::path(child_path).parent_path(), ec);
#endif
}

// Caso ESCONDIDO (tag com '.'): nao entra na suite normal, so e chamado pelo caso
// [concorrencia] acima, no processo FILHO. Grava a variante intro=true e imprime o path.
TEST_CASE("cockpit stage child: grava o BAKED intro=true e imprime o path",
          "[.cockpit_stage_child]") {
    std::cout << "COCKPIT_CHILD_PATH=" << write_baked_cockpit_rml(/*intro=*/true)
              << std::endl;
}

TEST_CASE("write_smoke_glintfx_rml: DIV com gradiente/glow em px, sem @font-face nem "
          "data-model (ADR-010 F1, degrau mais simples)",
          "[battle_cockpit_rml]") {
    const std::string path = write_smoke_glintfx_rml();
    const TempPathCleanup limpa{path};  // TEMPFILE-PROD-UNICO: 1 arquivo por processo
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
          "data-model removido, bloco de ABERTURA ausente, bloco de COMBATE presente, "
          "SEM @font-face injetado (FONT-EXTEND-GLITCH: fonte registrada via "
          "load_font_face, nao mais por string)",
          "[battle_cockpit_rml]") {
    const std::string path = write_baked_cockpit_rml(/*intro=*/false);
    const TempPathCleanup limpa{glintfx_cockpit_stage_dir()};  // TEMPFILE-PROD-UNICO
    const std::string rml = slurp(path);

    // FONT-EXTEND-GLITCH (2026-07-29): o @font-face por string foi MATADO - a familia
    // agora e registrada 1x pelo chamador via glintfx::UiLayer::load_font_face (API
    // v0.24.0), fora deste .rml. Trava o comportamento NOVO (mesmo espirito do teste:
    // characterization, nao "ideal").
    REQUIRE(rml.find("@font-face") == std::string::npos);
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
    const TempPathCleanup limpa{glintfx_cockpit_stage_dir()};  // TEMPFILE-PROD-UNICO
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
          "por pill, data-class-sel por indice, log data-for, SEM @font-face injetado "
          "(FONT-EXTEND-GLITCH: fonte registrada via load_font_face, nao mais por string)",
          "[battle_cockpit_rml]") {
    const std::string path = write_live_cockpit_rml();
    const TempPathCleanup limpa{glintfx_cockpit_stage_dir()};  // TEMPFILE-PROD-UNICO
    const std::string rml = slurp(path);

    // FONT-EXTEND-GLITCH (2026-07-29): ver o comentario equivalente no TEST_CASE do BAKED
    // acima - mesma migracao, mesmo racional.
    REQUIRE(rml.find("@font-face") == std::string::npos);
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
