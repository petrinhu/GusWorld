// gus/app/src/i18n/translator.cpp
//
// Implementacao do Translator de UI (ver header). Parser = POCO do dominio; I/O de
// arquivo na fronteira app/. verb_label_key mapeia verbo -> chave i18n.

#include "gus/app/i18n/translator.hpp"

#include <cstdlib>  // std::getenv
#include <fstream>
#include <ios>
#include <sstream>

// Raiz do repo (pai de GusEngine/) embutida pelo CMake, pra achar game/translations/
// rodando do build dir. Override em runtime via env GUSWORLD_TRANSLATIONS.
#ifndef GUSWORLD_TRANSLATIONS_DIR
#define GUSWORLD_TRANSLATIONS_DIR ""
#endif

namespace gus::app::i18n {

namespace {
using gus::app::screens::BattleVerb;

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) {
        return b;
    }
    if (a.back() == '/') {
        return a + b;
    }
    return a + "/" + b;
}
}  // namespace

void Translator::load_from_content(std::string_view md_content) {
    catalog_ = gus::domain::i18n::parse(md_content);
}

bool Translator::load_from_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return false;
    }
    std::ostringstream buf;
    buf << in.rdbuf();
    const std::string content = buf.str();
    if (content.empty()) {
        return false;
    }
    load_from_content(content);
    return true;
}

std::string Translator::tr(const std::string& key) const {
    const auto it = catalog_.find(key);
    if (it == catalog_.end() || it->second.empty()) {
        return key;  // fallback: a propria chave (sinaliza traducao faltando)
    }
    return it->second;
}

std::string resolve_translations_path() {
    if (const char* env = std::getenv("GUSWORLD_TRANSLATIONS")) {
        if (env[0] != '\0') {
            return env;  // caminho COMPLETO do .md (o lider aponta direto)
        }
    }
    const std::string compiled = GUSWORLD_TRANSLATIONS_DIR;
    if (!compiled.empty()) {
        return join(compiled, "pt_br.md");
    }
    return "game/translations/pt_br.md";
}

std::string_view verb_label_key(BattleVerb verb) noexcept {
    switch (verb) {
        case BattleVerb::Scan:     return "COMBAT_ACTION_SCAN";
        case BattleVerb::Gambito:  return "COMBAT_VERB_GAMBITO";
        case BattleVerb::Atacar:   return "COMBAT_ACTION_ATTACK";
        case BattleVerb::Defender: return "COMBAT_ACTION_DEFEND";
        case BattleVerb::Compilar: return "COMBAT_VERB_COMPILAR";
        case BattleVerb::Flee:     return "COMBAT_FLEE";
    }
    return "COMBAT_ACTION_ATTACK";
}

}  // namespace gus::app::i18n
