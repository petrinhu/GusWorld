// gus/platform/src/fs/settings_file_store.cpp
//
// Implementacao do I/O real de settings.json (MENU-PAUSA-CONFIG-SOM). Ver header.
// Travado por platform/tests/settings_file_store_test.cpp (TEST-FIRST: roundtrip,
// ausente/corrompido degrada, permissoes 0700/0600, sobrescrita).

#include "gus/platform/fs/settings_file_store.hpp"

#include <cstdlib>  // std::getenv
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <system_error>

#include "gus/domain/settings/system_settings_json.hpp"

namespace gus::platform::fs {

namespace {
std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}
}  // namespace

std::string resolve_settings_dir() {
    if (const char* env = std::getenv("GUSWORLD_HOME")) {
        if (env[0] != '\0') {
            return join(env, ".gusworld");
        }
    }
    if (const char* home = std::getenv("HOME")) {
        if (home[0] != '\0') {
            return join(home, ".gusworld");
        }
    }
    // Fallback defensivo (HOME ausente - ambiente atipico): diretorio relativo ao
    // CWD. Nunca deveria acontecer num host real, mas nao lanca.
    return ".gusworld";
}

std::string settings_file_path(const std::string& dir) {
    return join(dir, "settings.json");
}

gus::domain::settings::SystemSettings load_system_settings(const std::string& dir) {
    namespace stdfs = std::filesystem;
    const std::string path = settings_file_path(dir);

    std::error_code ec;
    if (!stdfs::exists(path, ec) || ec) {
        return {};  // 1a execucao do jogo (ou dir ausente): defaults, sem log de erro
    }

    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "settings_file_store: falha ao abrir '" << path
                  << "' para leitura - usando defaults (degradacao graciosa)\n";
        return {};
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();

    const auto result = gus::domain::settings::parse_system_settings(buffer.str());
    if (result.error != gus::domain::settings::SystemSettingsParseError::None) {
        std::cerr << "settings_file_store: '" << path
                  << "' corrompido/malformado - usando defaults (degradacao "
                     "graciosa)\n";
    }
    // Mesmo em erro, result.settings ja preserva os defaults nos campos que nao
    // resolveram (contrato de parse_system_settings) - devolve sempre.
    return result.settings;
}

bool save_system_settings(const gus::domain::settings::SystemSettings& settings,
                           const std::string& dir) {
    namespace stdfs = std::filesystem;
    std::error_code ec;

    if (!stdfs::exists(dir, ec)) {
        stdfs::create_directories(dir, ec);
        if (ec) {
            std::cerr << "settings_file_store: falha ao criar diretorio '" << dir
                      << "': " << ec.message() << "\n";
            return false;
        }
    }
    // 0700 (rwx so o dono, nada pro grupo/outros) - LGPD, dado do usuario.
    stdfs::permissions(dir, stdfs::perms::owner_all, stdfs::perm_options::replace, ec);
    if (ec) {
        std::cerr << "settings_file_store: falha ao setar permissao 0700 em '" << dir
                  << "': " << ec.message() << " (segue mesmo assim)\n";
        ec.clear();
    }

    const std::string path = settings_file_path(dir);
    const std::string json = gus::domain::settings::serialize_system_settings_pretty(settings);

    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        std::cerr << "settings_file_store: falha ao abrir '" << path
                  << "' para escrita\n";
        return false;
    }
    out << json;
    out.close();
    if (out.fail()) {
        std::cerr << "settings_file_store: falha ao escrever em '" << path << "'\n";
        return false;
    }

    // 0600 (rw so o dono, sem exec, nada pro grupo/outros).
    stdfs::permissions(path, stdfs::perms::owner_read | stdfs::perms::owner_write,
                        stdfs::perm_options::replace, ec);
    if (ec) {
        std::cerr << "settings_file_store: falha ao setar permissao 0600 em '" << path
                  << "': " << ec.message() << " (arquivo gravado mesmo assim)\n";
    }

    return true;
}

}  // namespace gus::platform::fs
