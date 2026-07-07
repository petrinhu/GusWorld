// gus/platform/src/fs/controls_file_store.cpp
//
// Implementacao do I/O real de controles (tela Controles, M2). Ver header.
// Travado por platform/tests/controls_file_store_test.cpp (TEST-FIRST: roundtrip,
// ausente/corrompido degrada, permissoes 0700/0600, multi-perfil, sobrescrita).

#include "gus/platform/fs/controls_file_store.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <system_error>

#include "gus/domain/input/controls_json.hpp"
#include "gus/domain/input/controls_name.hpp"
#include "gus/domain/input/controls_restore.hpp"

namespace gus::platform::fs {

namespace {
std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}
}  // namespace

std::string controls_file_path(const std::string& dir, const std::string& profile) {
    return join(dir, gus::domain::input::controls_file_name(profile));
}

gus::domain::input::InputRemapConfig load_controls(const std::string& dir,
                                                     const std::string& profile) {
    namespace stdfs = std::filesystem;
    const std::string path = controls_file_path(dir, profile);

    std::error_code ec;
    if (!stdfs::exists(path, ec) || ec) {
        return gus::domain::input::default_controls();  // 1a vez / perfil novo
    }

    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "controls_file_store: falha ao abrir '" << path
                  << "' para leitura - usando default_controls() (degradacao "
                     "graciosa)\n";
        return gus::domain::input::default_controls();
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();

    const auto result = gus::domain::input::parse_controls(buffer.str());
    if (result.error != gus::domain::input::ControlsParseError::None) {
        std::cerr << "controls_file_store: '" << path
                  << "' corrompido/malformado - usando default_controls() "
                     "(degradacao graciosa)\n";
        return gus::domain::input::default_controls();
    }
    return result.config;
}

bool save_controls(const gus::domain::input::InputRemapConfig& config, const std::string& dir,
                    const std::string& profile) {
    namespace stdfs = std::filesystem;
    std::error_code ec;

    if (!stdfs::exists(dir, ec)) {
        stdfs::create_directories(dir, ec);
        if (ec) {
            std::cerr << "controls_file_store: falha ao criar diretorio '" << dir
                      << "': " << ec.message() << "\n";
            return false;
        }
    }
    // 0700 (rwx so o dono) - LGPD, dado do usuario.
    stdfs::permissions(dir, stdfs::perms::owner_all, stdfs::perm_options::replace, ec);
    if (ec) {
        std::cerr << "controls_file_store: falha ao setar permissao 0700 em '" << dir
                  << "': " << ec.message() << " (segue mesmo assim)\n";
        ec.clear();
    }

    const std::string path = controls_file_path(dir, profile);
    const std::string json = gus::domain::input::serialize_controls_pretty(config);

    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        std::cerr << "controls_file_store: falha ao abrir '" << path << "' para escrita\n";
        return false;
    }
    out << json;
    out.close();
    if (out.fail()) {
        std::cerr << "controls_file_store: falha ao escrever em '" << path << "'\n";
        return false;
    }

    // 0600 (rw so o dono, sem exec) - LGPD.
    stdfs::permissions(path, stdfs::perms::owner_read | stdfs::perms::owner_write,
                        stdfs::perm_options::replace, ec);
    if (ec) {
        std::cerr << "controls_file_store: falha ao setar permissao 0600 em '" << path
                  << "': " << ec.message() << " (arquivo gravado mesmo assim)\n";
    }

    return true;
}

}  // namespace gus::platform::fs
