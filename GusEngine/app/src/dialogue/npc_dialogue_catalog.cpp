// gus/app/src/dialogue/npc_dialogue_catalog.cpp
//
// Implementacao do I/O do grafo de dialogo do Bertoldo. Ver header.

#include "gus/app/dialogue/npc_dialogue_catalog.hpp"

#include <fstream>
#include <ios>
#include <sstream>

#include "gus/core/asset_paths.hpp"
#include "gus/domain/dialogue/dialogue_text.hpp"
#include "gus/platform/assets/asset_source.hpp"  // ASSETS-VFS-F1b (ADR-013): porteiro

namespace gus::app::dialogue {

namespace {
std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}
}  // namespace

std::string resolve_npc_intro_bertoldo_dialogue_path() {
    // ASSETS-VFS-F1b (ADR-013): a cadeia `env GUSWORLD_DIALOGUES = override LITERAL >
    // macro GUSWORLD_DIALOGUES_DIR > CWD (kDialoguesDir)` foi CONSOLIDADA em
    // FilesystemAssetSource::resolve_path (familia DIALOGUES, dispatch pelo prefixo
    // "resources/dialogues/" do id, M8 decommission moveu de "game/dialogues/" via
    // git mv; mesmo padrao da familia I18N). Assinatura/contrato
    // INTOCADOS - paridade provada em platform/tests/asset_source_test.cpp e reforcada
    // pelos testes de integracao ja existentes deste arquivo (carregam o grafo real
    // via o path resolvido).
    const std::string id =
        join(std::string(gus::core::assets::kDialoguesDir),
             std::string(gus::core::assets::kNpcIntroBertoldoDlgFile));
    return gus::platform::assets::FilesystemAssetSource().resolve_path(id);
}

std::optional<gus::domain::dialogue::DialogueGraph> load_dialogue_graph_from_file(
    const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return std::nullopt;
    }
    std::ostringstream buf;
    buf << in.rdbuf();
    const std::string content = buf.str();
    if (content.empty()) {
        return std::nullopt;
    }
    // Propaga DialogueTextError/std::invalid_argument (fail-fast de autoria) -
    // NAO capturado aqui, mesmo contrato do header.
    return gus::domain::dialogue::parse_text_to_dialogue_graph(content);
}

}  // namespace gus::app::dialogue
