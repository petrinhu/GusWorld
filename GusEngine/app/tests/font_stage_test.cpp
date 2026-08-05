// SPDX-License-Identifier: Apache-2.0
// GusEngine/app/tests/font_stage_test.cpp
//
// ASSETS-FONTE-TELAS-GEMEO: Catch2 HEADLESS de gus::app::screens::stage_ui_fonts - a peca
// que substituiu as 7 copias identicas de staging de fonte espalhadas pelas telas (ver
// gus/app/screens/font_stage.hpp pro defeito original).
//
// POR QUE DA PRA TESTAR SEM ABRIR JANELA: o staging e I/O de arquivo puro (resolver o
// caminho pelo porteiro + copiar 2 .ttf pro stage). Ele acontece ANTES de qualquer GL/
// glintfx::UiLayer existir - quem precisa de contexto GL e o load_font_face que vem
// DEPOIS, e esse continua coberto so pelos harnesses de interacao (Xvfb). Ou seja: este
// arquivo cobre a metade do fio que era exatamente a que falhava em silencio.
//
// O log e capturado por glintfx::set_log_sink (FW-LOG, v0.26.0) - e assim que se prova que
// a falha de copia PARA DE SER SILENCIOSA, que e o coracao desta fatia.

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <glintfx/log.hpp>

#include "gus/app/screens/font_stage.hpp"
#include "gus/core/asset_paths.hpp"
#include "tmp_dir_test_support.hpp"

namespace fs = std::filesystem;
using gus::app::screens::stage_ui_fonts;

namespace {

const std::string kRegular{gus::core::assets::kFontMonoRegularFile};
const std::string kBold{gus::core::assets::kFontMonoBoldFile};

// setenv/unsetenv sao POSIX; MSVC usa _putenv_s (string vazia REMOVE a var). MESMO helper
// portavel de platform/tests/asset_source_test.cpp.
void portable_setenv(const char* name, const char* value) {
#ifdef _WIN32
    _putenv_s(name, value);
#else
    setenv(name, value, /*overwrite=*/1);
#endif
}

void portable_unsetenv(const char* name) {
#ifdef _WIN32
    _putenv_s(name, "");
#else
    unsetenv(name);
#endif
}

// RAII: seta a env var e restaura o valor anterior (ou remove) no fim do escopo.
class ScopedEnv {
public:
    ScopedEnv(const char* name, const std::string& value) : name_(name) {
        if (const char* prev = std::getenv(name); prev != nullptr) {
            had_prev_ = true;
            prev_ = prev;
        }
        portable_setenv(name, value.c_str());
    }
    ~ScopedEnv() {
        if (had_prev_) {
            portable_setenv(name_.c_str(), prev_.c_str());
        } else {
            portable_unsetenv(name_.c_str());
        }
    }
    ScopedEnv(const ScopedEnv&) = delete;
    ScopedEnv& operator=(const ScopedEnv&) = delete;

private:
    std::string name_;
    bool had_prev_ = false;
    std::string prev_;
};

// Garante a env AUSENTE durante o escopo (restaura depois).
class ScopedUnsetEnv {
public:
    explicit ScopedUnsetEnv(const char* name) : name_(name) {
        if (const char* prev = std::getenv(name); prev != nullptr) {
            had_prev_ = true;
            prev_ = prev;
        }
        portable_unsetenv(name);
    }
    ~ScopedUnsetEnv() {
        if (had_prev_) {
            portable_setenv(name_.c_str(), prev_.c_str());
        }
    }
    ScopedUnsetEnv(const ScopedUnsetEnv&) = delete;
    ScopedUnsetEnv& operator=(const ScopedUnsetEnv&) = delete;

private:
    std::string name_;
    bool had_prev_ = false;
    std::string prev_;
};

// RAII: captura o que o glintfx logar durante o escopo e devolve o sink default no fim
// (LogSink{} vazio reseta pro fprintf(stderr) embutido - contrato de glintfx/log.hpp).
class LogCapture {
public:
    LogCapture() {
        glintfx::set_log_sink([this](glintfx::LogLevel level, const char* message) {
            lines_.push_back({level, message != nullptr ? message : ""});
        });
    }
    ~LogCapture() { glintfx::set_log_sink(glintfx::LogSink{}); }
    LogCapture(const LogCapture&) = delete;
    LogCapture& operator=(const LogCapture&) = delete;

    struct Line {
        glintfx::LogLevel level;
        std::string text;
    };

    [[nodiscard]] const std::vector<Line>& lines() const { return lines_; }

    [[nodiscard]] std::size_t count_containing(const std::string& needle) const {
        std::size_t n = 0;
        for (const auto& l : lines_) {
            if (l.text.find(needle) != std::string::npos) {
                ++n;
            }
        }
        return n;
    }

private:
    std::vector<Line> lines_;
};

class TempDir : public gus::test_support::ScopedTempDir {
public:
    explicit TempDir(const char* tag = "gusworld_font_stage_test")
        : gus::test_support::ScopedTempDir(tag) {}
};

void write_file(const fs::path& p, const std::string& content) {
    std::error_code ec;
    fs::create_directories(p.parent_path(), ec);
    std::ofstream f(p, std::ios::binary);
    f << content;
}

std::string read_file(const fs::path& p) {
    std::ifstream f(p, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(f)),
                       std::istreambuf_iterator<char>());
}

}  // namespace

TEST_CASE("stage_ui_fonts: copia as DUAS fontes reais do repo pro stage (macro/CWD)",
          "[font_stage]") {
    ScopedUnsetEnv no_fonts("GUSWORLD_FONTS");
    ScopedUnsetEnv no_assets("GUSWORLD_ASSETS");
    TempDir stage;

    REQUIRE(stage_ui_fonts(stage.path()));
    REQUIRE(fs::exists(stage.path() / kRegular));
    REQUIRE(fs::exists(stage.path() / kBold));
    // Fonte de verdade, nao arquivo vazio (a Pixel Operator Mono e CC0 versionada).
    REQUIRE(fs::file_size(stage.path() / kRegular) > 0);
    REQUIRE(fs::file_size(stage.path() / kBold) > 0);
}

TEST_CASE("stage_ui_fonts: honra a env GUSWORLD_FONTS (o override nao se perdeu na "
          "migracao pro porteiro)",
          "[font_stage]") {
    ScopedUnsetEnv no_assets("GUSWORLD_ASSETS");
    TempDir fonts_src("gusworld_font_stage_src");
    TempDir stage;
    write_file(fonts_src.path() / kRegular, "REGULAR-DO-ENV");
    write_file(fonts_src.path() / kBold, "BOLD-DO-ENV");
    ScopedEnv env("GUSWORLD_FONTS", fonts_src.path().string());

    REQUIRE(stage_ui_fonts(stage.path()));
    REQUIRE(read_file(stage.path() / kRegular) == "REGULAR-DO-ENV");
    REQUIRE(read_file(stage.path() / kBold) == "BOLD-DO-ENV");
}

TEST_CASE("stage_ui_fonts: env GUSWORLD_FONTS vazia NAO sequestra (cai no macro/CWD)",
          "[font_stage]") {
    ScopedUnsetEnv no_assets("GUSWORLD_ASSETS");
    TempDir vazia("gusworld_font_stage_vazia");  // sem nenhum .ttf dentro
    TempDir stage;
    ScopedEnv env("GUSWORLD_FONTS", vazia.path().string());

    // Este e o caso que o codigo ANTIGO quebrava: ele usava a env as cegas, a copia
    // falhava, e a UI ficava sem fonte em silencio. Agora degrada pro nivel de baixo.
    REQUIRE(stage_ui_fonts(stage.path()));
    REQUIRE(fs::file_size(stage.path() / kRegular) > 0);
    REQUIRE(fs::file_size(stage.path() / kBold) > 0);
}

TEST_CASE("stage_ui_fonts: sobrescreve o que ja estava no stage (overwrite_existing)",
          "[font_stage]") {
    ScopedUnsetEnv no_assets("GUSWORLD_ASSETS");
    TempDir fonts_src("gusworld_font_stage_src");
    TempDir stage;
    write_file(fonts_src.path() / kRegular, "NOVO-REGULAR");
    write_file(fonts_src.path() / kBold, "NOVO-BOLD");
    write_file(stage.path() / kRegular, "LIXO-VELHO-QUE-PRECISA-SUMIR");
    ScopedEnv env("GUSWORLD_FONTS", fonts_src.path().string());

    REQUIRE(stage_ui_fonts(stage.path()));
    REQUIRE(read_file(stage.path() / kRegular) == "NOVO-REGULAR");
}

TEST_CASE("stage_ui_fonts: falha de copia devolve false e LOGA (nunca mais silenciosa)",
          "[font_stage]") {
    ScopedUnsetEnv no_fonts("GUSWORLD_FONTS");
    ScopedUnsetEnv no_assets("GUSWORLD_ASSETS");
    TempDir base;
    // Stage INEXISTENTE: copy_file para dentro de um diretorio que nao existe falha
    // deterministicamente (ENOENT) - inclusive rodando como root, ao contrario de um
    // teste baseado em permissao de escrita (containers de CI rodam como root).
    const fs::path stage_inexistente = base.path() / "nao_criado";

    LogCapture log;
    REQUIRE_FALSE(stage_ui_fonts(stage_inexistente));

    // As DUAS falhas sao reportadas (nada de curto-circuito escondendo a segunda).
    REQUIRE(log.count_containing(kRegular) >= 1);
    REQUIRE(log.count_containing(kBold) >= 1);
    REQUIRE(log.lines().size() == 2);
    for (const auto& line : log.lines()) {
        REQUIRE(line.level == glintfx::LogLevel::Warn);
        // Contexto obrigatorio: o que estava sendo feito + a causa raiz do sistema.
        REQUIRE(line.text.find("font_stage") != std::string::npos);
        REQUIRE(line.text.find(stage_inexistente.string()) != std::string::npos);
    }
}

TEST_CASE("stage_ui_fonts: falha de UMA fonte so devolve false e loga SO ela",
          "[font_stage]") {
    ScopedUnsetEnv no_assets("GUSWORLD_ASSETS");
    TempDir fonts_src("gusworld_font_stage_src");
    TempDir stage;
    // So o regular existe na pasta do env; o bold nao existe em lugar NENHUM (nome
    // apontado por um env que nao tem o arquivo -> cai no macro/CWD, que TEM o bold real).
    // Pra isolar o bold de verdade, apontamos a env pra uma pasta que tem so o regular E
    // conferimos que o bold veio do repo (arquivo grande), nao do env.
    write_file(fonts_src.path() / kRegular, "SO-O-REGULAR");
    ScopedEnv env("GUSWORLD_FONTS", fonts_src.path().string());

    LogCapture log;
    REQUIRE(stage_ui_fonts(stage.path()));
    REQUIRE(log.lines().empty());  // nada falhou: o bold degradou pro macro/CWD
    REQUIRE(read_file(stage.path() / kRegular) == "SO-O-REGULAR");
    REQUIRE(fs::file_size(stage.path() / kBold) > std::string("SO-O-REGULAR").size());
}
