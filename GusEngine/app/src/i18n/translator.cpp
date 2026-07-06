// gus/app/src/i18n/translator.cpp
//
// Implementacao do Translator de UI (ver header). Parser = POCO do dominio; I/O de
// arquivo na fronteira app/. verb_label_key mapeia verbo -> chave i18n.

#include "gus/app/i18n/translator.hpp"

#include "gus/core/asset_paths.hpp"               // caminhos de asset centralizados
#include "gus/platform/assets/asset_source.hpp"   // ASSETS-VFS-F1 (ADR-013): porteiro

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
    // ASSETS-VFS-F1 (ADR-013): delega pro primitivo compartilhado read_raw_file (o MESMO
    // usado por FilesystemAssetSource::read() apos resolver um id) - aceita qualquer path
    // (literal ou ja resolvido), preservando o contrato public de load_from_file.
    const auto bytes = gus::platform::assets::read_raw_file(path);
    if (!bytes.has_value() || bytes->empty()) {
        return false;  // ausente/ilegivel/vazio: fallback (mostra a chave)
    }
    std::string content(bytes->size(), '\0');
    for (std::size_t i = 0; i < bytes->size(); ++i) {
        content[i] = static_cast<char>(bytes.value()[i]);
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
    // ASSETS-VFS-F1 (ADR-013): a cadeia `env GUSWORLD_TRANSLATIONS = override LITERAL >
    // macro GUSWORLD_TRANSLATIONS_DIR > CWD (kTranslationsDir)` foi CONSOLIDADA em
    // FilesystemAssetSource::resolve_path (familia I18N, dispatch pelo prefixo
    // "game/translations/" do id). Assinatura/contrato INTOCADOS - paridade provada em
    // platform/tests/asset_source_test.cpp.
    const std::string id =
        join(std::string(gus::core::assets::kTranslationsDir),
             std::string(gus::core::assets::kTranslationPtBrFile));
    return gus::platform::assets::FilesystemAssetSource().resolve_path(id);
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
