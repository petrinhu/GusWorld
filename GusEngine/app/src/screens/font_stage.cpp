// SPDX-License-Identifier: Apache-2.0
// gus/app/src/screens/font_stage.cpp
//
// Ver header (gus/app/screens/font_stage.hpp) pro motivo desta peca existir
// (ASSETS-FONTE-TELAS-GEMEO) e pro contrato de degradacao.

#include "gus/app/screens/font_stage.hpp"

#include <string>

#include <glintfx/log.hpp>  // FW-LOG: o log da casa vem do glintfx, nunca do SDL

#include "gus/core/asset_paths.hpp"  // kFontsDir/kFontMonoRegularFile/kFontMonoBoldFile
#include "gus/platform/assets/asset_source.hpp"  // ASSETS-VFS-F1 (ADR-013): o porteiro

namespace gus::app::screens {

namespace {

namespace fs = std::filesystem;

std::string join_id(std::string_view dir, std::string_view file) {
    std::string out(dir);
    if (!out.empty() && out.back() != '/') {
        out += '/';
    }
    out += file;
    return out;
}

// Copia UMA fonte pro stage. `filename` e o nome do .ttf (o mesmo dos dois lados: e o
// stage ACHATADO que o glintfx enxerga).
bool copy_one_font(std::string_view filename, const fs::path& stage_dir) {
    // O id logico ("assets/fonts/<arquivo>") cai na familia FONTES do porteiro, que faz a
    // cascata CHECADA (ver asset_source.cpp::resolve_font). Antes desta fatia, aqui havia
    // a macro GUSWORLD_FONTS_DIR crua + getenv("GUSWORLD_FONTS") a mao, sem checar disco.
    const std::string id = join_id(gus::core::assets::kFontsDir, filename);
    const std::string src =
        gus::platform::assets::FilesystemAssetSource().resolve_path(id);
    const fs::path dst = stage_dir / std::string(filename);

    std::error_code ec;
    fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
    if (ec) {
        // Via PLANA log(level, texto) - NUNCA a variadica log_warn(fmt, ...): a mensagem
        // embute texto EXTERNO (caminhos de disco vindos de env/macro + a string de erro
        // do sistema), e um '%' legitimo num nome de arquivo viraria especificador de
        // formato (CWE-134; ver o paragrafo "DOIS PONTOS DE ENTRADA DISTINTOS" de
        // glintfx/log.hpp). Mesma disciplina do FATIA1-LOG-CLOCK.
        const std::string msg = "font_stage: falha ao copiar a fonte '" + src +
                                "' -> '" + dst.string() + "': " + ec.message() +
                                " (a UI cai no fallback de fonte do RmlUi)";
        glintfx::log(glintfx::LogLevel::Warn, msg.c_str());
        return false;
    }
    return true;
}

}  // namespace

bool stage_ui_fonts(const fs::path& stage_dir) {
    // As duas copias sao SEMPRE tentadas (nada de curto-circuito por &&): se so o bold
    // falhasse, um && esconderia o regular - e vice-versa. Sem flood: quem chama reescreve
    // o .rml a cada MUDANCA DE ESTADO da tela (navegacao/confirmacao), nao por frame.
    const bool regular = copy_one_font(gus::core::assets::kFontMonoRegularFile, stage_dir);
    const bool bold = copy_one_font(gus::core::assets::kFontMonoBoldFile, stage_dir);
    return regular && bold;
}

}  // namespace gus::app::screens
