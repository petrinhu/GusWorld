// gus/platform/src/fs/save_file_store.cpp
//
// Implementacao do I/O real do save (M2-SAVE-IO). Ver header. Travado por
// platform/tests/save_file_store_test.cpp (TEST-FIRST: roundtrip, slot vazio,
// corrompido/adulterado != ausente, backup chain em disco, T1.2 WrongSlot,
// permissoes 0700/0600, fail-fast de slot invalido).

#include "gus/platform/fs/save_file_store.hpp"

#include <algorithm>  // std::fill (secure_wipe_save)
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

// MODOS-MORTE Fase 0 (secure_wipe_save): escrito ANTES do envelope virar GDS3
// (ADR-015 Onda 2). Estes offsets (8/32) eram MAGIC(4)+LENGTH(4) no INICIO e
// HMAC(32, SHA-256) no FIM do GDS2 - NAO correspondem mais ao layout real do GDS3
// (magic+envelope_ver+slot_id+rollback_ctr = 18 bytes no inicio; tag AEAD = 16
// bytes no fim). NAO E UM REGRESSAO SILENCIOSA: corromper os primeiros 8 bytes de
// QUALQUER envelope GDS3 ainda corrompe o MAGIC "GDS3" inteiro (bytes[0..4)),
// e o unpack_save rejeita magic invalido como primeiro passo (SaveCorruptError)
// antes mesmo de tentar decifrar - entao o wipe AINDA funciona (o arquivo fica
// inutilizavel), so que por corromper o magic em vez de mirar precisamente a tag
// AEAD. O crypto-shred APROPRIADO (sobrescrever SO nonce+tag, ADR-015 decisao 5)
// e escopo da Onda 5 (SAVE-CRYPTO-V2-WIPE); ate la este wipe funciona mas nao usa
// a vantagem "matematicamente irrecuperavel na hora" que o AEAD oferece.
constexpr std::size_t kEnvelopeHeaderLen = 8;  // corrompe o MAGIC GDS3 (suficiente p/ rejeitar no load; nao mira o AAD/nonce inteiro)
constexpr std::size_t kEnvelopeHmacLen = 32;   // corrompe a TAG AEAD (16) + parte do fim do ciphertext (16)

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

bool delete_save(int slot, const std::string& dir) {
    // Fail-fast (nao e I/O): slot invalido propaga std::out_of_range.
    const std::string primary = gus::domain::save::primary_logical_name(slot);
    FsSaveStore store(dir);
    store.remove(primary);
    for (int k = 1; k <= gus::domain::save::kBackupChainDepth; ++k) {
        store.remove(gus::domain::save::backup_logical_name(slot, k));
    }
    // Confirma que o primario de fato sumiu (degradacao segura: permissao negada
    // deixaria o arquivo vivo - o CHAMADOR decide o que avisar, MESMO espirito de
    // save_game/load_game).
    return !store.exists(primary);
}

namespace {

// Sobrescreve o selo (inicio + fim do envelope, ver kEnvelopeHeaderLen/
// kEnvelopeHmacLen no topo do arquivo) de UM arquivo (nome logico) e unlinka.
// Devolve true se, ao final, o arquivo NAO existe mais (ausente de inicio conta
// como sucesso - no-op idempotente, mesmo contrato de delete_save/
// FsSaveStore::remove).
bool wipe_one(FsSaveStore& store, const std::string& name) {
    if (!store.exists(name)) return true;  // nada a apagar (idempotente)

    std::vector<std::uint8_t> bytes;
    try {
        bytes = store.read(name);
    } catch (const std::exception& e) {
        std::cerr << "save_file_store: [secure_wipe] falha ao ler '" << name
                  << "' antes do wipe: " << e.what()
                  << " (degradacao segura: nao progride sobre arquivo ilegivel)\n";
        return false;
    }

    if (bytes.size() >= kEnvelopeHeaderLen + kEnvelopeHmacLen) {
        // Corrompe o MAGIC (inicio) e a regiao da tag AEAD (fim) - o resto do
        // ciphertext fica intacto em disco, mas SEM selo valido o load ja rejeita
        // (SaveCorruptError/SaveIntegrityError, comportamento EXISTENTE hoje).
        std::fill(bytes.begin(),
                  bytes.begin() + static_cast<std::ptrdiff_t>(kEnvelopeHeaderLen),
                  std::uint8_t{0xFF});
        std::fill(bytes.end() - static_cast<std::ptrdiff_t>(kEnvelopeHmacLen),
                  bytes.end(), std::uint8_t{0x00});
        try {
            store.write(name, bytes);
        } catch (const std::exception& e) {
            std::cerr << "save_file_store: [secure_wipe] falha ao sobrescrever o "
                         "selo de '"
                      << name << "': " << e.what()
                      << " (segue pro unlink mesmo assim)\n";
        }
    }
    // Zera o buffer em RAM (nao deixa o payload antigo residente na memoria do
    // processo) ANTES do unlink.
    std::fill(bytes.begin(), bytes.end(), std::uint8_t{0});

    store.remove(name);
    return !store.exists(name);
}

}  // namespace

bool secure_wipe_save(int slot, const std::string& dir) {
    // Fail-fast (nao e I/O): slot invalido propaga std::out_of_range (mesmo
    // contrato de delete_save/has_save/save_game/load_game).
    const std::string primary = gus::domain::save::primary_logical_name(slot);
    FsSaveStore store(dir);

    bool all_gone = wipe_one(store, primary);
    for (int k = 1; k <= gus::domain::save::kBackupChainDepth; ++k) {
        const bool gone = wipe_one(store, gus::domain::save::backup_logical_name(slot, k));
        all_gone = all_gone && gone;
    }
    return all_gone;
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

std::optional<gus::domain::save::LoadOutcome> load_game_from_backup(int slot,
                                                                     const std::string& dir) {
    // Fail-fast (nao e I/O): slot invalido propaga std::out_of_range (mesmo
    // contrato de load_game - primary_logical_name ja valida).
    (void)gus::domain::save::primary_logical_name(slot);
    FsSaveStore store(dir);

    for (int k = 1; k <= gus::domain::save::kBackupChainDepth; ++k) {
        const std::string name = gus::domain::save::backup_logical_name(slot, k);
        try {
            if (!store.exists(name)) continue;  // esta geracao nunca existiu
            const auto bytes = store.read(name);
            const auto outcome = gus::domain::save::load_save(bytes, slot);
            if (outcome.result == gus::domain::save::LoadResult::Ok) return outcome;
            // Geracao presente mas tambem ilegivel (corrompida/versao/etc.) -
            // segue tentando a proxima (mais antiga), nao desiste na 1a falha.
        } catch (const std::exception& e) {
            std::cerr << "save_file_store: falha de I/O ao ler backup '" << name
                      << "' do slot " << slot << ": " << e.what()
                      << " - tentando a proxima geracao (degradacao segura)\n";
        }
    }
    return std::nullopt;  // nenhuma geracao de backup carregou Ok
}

}  // namespace gus::platform::fs
