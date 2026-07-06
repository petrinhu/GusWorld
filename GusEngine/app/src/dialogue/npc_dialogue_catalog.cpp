// gus/app/src/dialogue/npc_dialogue_catalog.cpp
//
// Implementacao do I/O do grafo de dialogo do Bertoldo. Ver header.

#include "gus/app/dialogue/npc_dialogue_catalog.hpp"

#include <cstdlib>  // std::getenv
#include <fstream>
#include <ios>
#include <sstream>

#include "gus/core/asset_paths.hpp"
#include "gus/domain/dialogue/dialogue_text.hpp"

// Pasta dos grafos de dialogo (game/dialogues/) DENTRO do repo (raiz = pai de
// GusEngine/), embutida pelo CMake - mesma receita de GUSWORLD_TRANSLATIONS_DIR.
// Override em runtime via env GUSWORLD_DIALOGUES.
#ifndef GUSWORLD_DIALOGUES_DIR
#define GUSWORLD_DIALOGUES_DIR ""
#endif

namespace gus::app::dialogue {

namespace {
std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}
}  // namespace

std::string resolve_npc_intro_bertoldo_dialogue_path() {
    if (const char* env = std::getenv("GUSWORLD_DIALOGUES")) {
        if (env[0] != '\0') {
            return env;  // caminho COMPLETO do .dlg.txt (o lider aponta direto)
        }
    }
    const std::string file(gus::core::assets::kNpcIntroBertoldoDlgFile);
    const std::string compiled = GUSWORLD_DIALOGUES_DIR;
    if (!compiled.empty()) {
        return join(compiled, file);
    }
    return join(std::string(gus::core::assets::kDialoguesDir), file);
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
