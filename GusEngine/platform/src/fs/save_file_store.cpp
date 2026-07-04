// gus/platform/src/fs/save_file_store.cpp
//
// Implementacao do I/O real do save (M2-SAVE-IO). Ver header. Travado por
// platform/tests/save_file_store_test.cpp (TEST-FIRST: roundtrip, slot vazio,
// corrompido/adulterado != ausente, backup chain em disco, T1.2 WrongSlot,
// permissoes 0700/0600, fail-fast de slot invalido).

#include "gus/platform/fs/save_file_store.hpp"

#include <cstdlib>  // std::getenv
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <system_error>

#include "gus/domain/save/save_backup.hpp"
#include "gus/domain/save/save_slots.hpp"

namespace gus::platform::fs {

namespace {

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

// Cria `dir` (0700) sob demanda. false = falha de I/O (nunca lanca).
bool ensure_saves_dir(const std::string& dir) {
    namespace stdfs = std::filesystem;
    std::error_code ec;

    if (!stdfs::exists(dir, ec)) {
        stdfs::create_directories(dir, ec);
        if (ec) {
            std::cerr << "save_file_store: falha ao criar diretorio '" << dir
                      << "': " << ec.message() << "\n";
            return false;
        }
    }
    stdfs::permissions(dir, stdfs::perms::owner_all, stdfs::perm_options::replace, ec);
    if (ec) {
        std::cerr << "save_file_store: falha ao setar permissao 0700 em '" << dir
                  << "': " << ec.message() << " (segue mesmo assim)\n";
    }
    return true;
}

}  // namespace

// ---- FsSaveStore (port SaveStore sobre arquivos reais) ---------------------

FsSaveStore::FsSaveStore(std::string dir) : dir_(std::move(dir)) {}

std::string FsSaveStore::path_for(const std::string& name) const {
    return join(dir_, name + ".sav");
}

bool FsSaveStore::exists(const std::string& name) const {
    std::error_code ec;
    const bool present = std::filesystem::exists(path_for(name), ec);
    return present && !ec;
}

std::vector<std::uint8_t> FsSaveStore::read(const std::string& name) const {
    const std::string path = path_for(name);
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open())
        throw std::out_of_range("FsSaveStore: nome ausente ou ilegivel: " + name);
    return std::vector<std::uint8_t>((std::istreambuf_iterator<char>(in)),
                                     std::istreambuf_iterator<char>());
}

void FsSaveStore::write(const std::string& name, const std::vector<std::uint8_t>& bytes) {
    const std::string path = path_for(name);
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out.is_open())
        throw std::runtime_error("FsSaveStore: falha ao abrir para escrita: " + name);
    out.write(reinterpret_cast<const char*>(bytes.data()),
              static_cast<std::streamsize>(bytes.size()));
    out.close();
    if (out.fail())
        throw std::runtime_error("FsSaveStore: falha ao escrever bytes: " + name);

    // 0600 (rw so o dono, sem exec) - LGPD, dado do jogador.
    std::error_code ec;
    std::filesystem::permissions(path,
                                 std::filesystem::perms::owner_read |
                                     std::filesystem::perms::owner_write,
                                 std::filesystem::perm_options::replace, ec);
    if (ec) {
        std::cerr << "save_file_store: falha ao setar permissao 0600 em '" << path
                  << "': " << ec.message() << " (arquivo gravado mesmo assim)\n";
    }
}

void FsSaveStore::move(const std::string& from, const std::string& to) {
    std::error_code ec;
    const std::string from_path = path_for(from);
    if (!std::filesystem::exists(from_path, ec)) return;  // no-op (contrato do port)

    std::filesystem::rename(from_path, path_for(to), ec);
    if (ec) {
        throw std::runtime_error("FsSaveStore: falha ao mover '" + from + "' -> '" +
                                 to + "': " + ec.message());
    }
}

void FsSaveStore::remove(const std::string& name) {
    std::error_code ec;
    std::filesystem::remove(path_for(name), ec);
    // Ausente ja e no-op pelo contrato do port; falha real de remove() aqui e rara
    // e a interface do port nao devolve status - nao ha o que o caller faca com ela.
}

// ---- resolve_saves_dir -------------------------------------------------------

std::string resolve_saves_dir() {
    if (const char* env = std::getenv("GUSWORLD_HOME")) {
        if (env[0] != '\0') {
            return join(join(env, ".gusworld"), "saves");
        }
    }
    if (const char* home = std::getenv("HOME")) {
        if (home[0] != '\0') {
            return join(join(home, ".gusworld"), "saves");
        }
    }
    // Fallback defensivo (HOME ausente - ambiente atipico): diretorio relativo ao
    // CWD. Nunca deveria acontecer num host real, mas nao lanca.
    return join(".gusworld", "saves");
}

// ---- has_save / save_game / load_game ---------------------------------------

bool has_save(int slot, const std::string& dir) {
    // Fail-fast: slot invalido e erro de programacao (out_of_range propaga).
    const std::string name = gus::domain::save::primary_logical_name(slot);
    FsSaveStore store(dir);
    return store.exists(name);
}

bool save_game(const gus::domain::save::SaveData& data, int slot, const std::string& dir) {
    // Fail-fast (nao e I/O): slot invalido / SaveData invariante-violada propagam
    // (std::out_of_range / std::invalid_argument via serialize_save -> validate()).
    gus::domain::save::primary_logical_name(slot);
    const std::vector<std::uint8_t> bytes = gus::domain::save::serialize_save(data);

    try {
        if (!ensure_saves_dir(dir)) return false;
        FsSaveStore store(dir);
        gus::domain::save::write_with_backup_rotation(store, slot, bytes);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "save_file_store: falha de I/O ao gravar slot " << slot
                  << " em '" << dir << "': " << e.what()
                  << " (degradacao segura: estado em memoria da sessao continua "
                     "valido, so a persistencia falhou)\n";
        return false;
    }
}

std::optional<gus::domain::save::LoadOutcome> load_game(int slot, const std::string& dir) {
    // Fail-fast (nao e I/O): slot invalido propaga std::out_of_range.
    const std::string name = gus::domain::save::primary_logical_name(slot);

    try {
        FsSaveStore store(dir);
        if (!store.exists(name)) return std::nullopt;  // 1a execucao / slot vazio
        const auto bytes = store.read(name);
        return gus::domain::save::load_save(bytes, slot);
    } catch (const std::exception& e) {
        std::cerr << "save_file_store: falha de I/O ao carregar slot " << slot
                  << " ('" << name << "'): " << e.what()
                  << " - tratando como slot vazio (degradacao segura)\n";
        return std::nullopt;
    }
}

}  // namespace gus::platform::fs
