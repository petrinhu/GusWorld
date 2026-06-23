// gus/app/src/screens/anim_catalog.cpp
//
// Ver header. Varredura de resources/sprites/gus/ via std::filesystem. Camada app/
// (toca disco); nenhuma chamada SDL/GPU aqui - so monta caminhos.

#include "gus/app/screens/anim_catalog.hpp"

#include <algorithm>
#include <cstdlib>  // std::getenv
#include <filesystem>
#include <optional>
#include <utility>

#ifndef GUSWORLD_ASSETS_DIR
#define GUSWORLD_ASSETS_DIR ""
#endif

namespace fs = std::filesystem;

namespace gus::app::screens {

namespace {

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) {
        return b;
    }
    if (a.back() == '/') {
        return a + b;
    }
    return a + "/" + b;
}

// Extrai o indice de um arquivo "f<N>.png" (ex.: "f3.png" -> 3). Para rotations,
// o arquivo e "<N>_<nome>.png" (ex.: "5_north-east.png" -> 5). Devolve nullopt se
// o nome nao casar com nenhum dos dois padroes. Robusto a N de varios digitos.
std::optional<int> frame_index(const std::string& filename, bool rotation) {
    if (rotation) {
        // Prefixo numerico ate o primeiro '_' (ou '.').
        std::size_t i = 0;
        std::string digits;
        while (i < filename.size() &&
               (filename[i] >= '0' && filename[i] <= '9')) {
            digits.push_back(filename[i]);
            ++i;
        }
        if (digits.empty()) {
            return std::nullopt;
        }
        // Tem que ser seguido por '_' ou '.' (separador), nao mais digitos colados a letra.
        if (i < filename.size() && filename[i] != '_' && filename[i] != '.') {
            // ainda aceita: ja paramos nos digitos; qualquer separador serve
        }
        try {
            return std::stoi(digits);
        } catch (...) {
            return std::nullopt;
        }
    }
    // Padrao f<N>.png
    if (filename.size() < 2 || filename[0] != 'f') {
        return std::nullopt;
    }
    std::string digits;
    for (std::size_t i = 1; i < filename.size() && filename[i] != '.'; ++i) {
        if (filename[i] < '0' || filename[i] > '9') {
            return std::nullopt;
        }
        digits.push_back(filename[i]);
    }
    if (digits.empty()) {
        return std::nullopt;
    }
    try {
        return std::stoi(digits);
    } catch (...) {
        return std::nullopt;
    }
}

bool is_png(const fs::path& p) {
    auto ext = p.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext == ".png";
}

// Coleta os PNGs de um diretorio que casam o padrao de frame (f<N> ou rotation),
// ordenados pelo indice numerico. Devolve {indice, caminho-absoluto}.
std::vector<std::string> collect_frames(const fs::path& dir, bool rotation) {
    std::vector<std::pair<int, std::string>> indexed;
    std::error_code ec;
    if (!fs::is_directory(dir, ec)) {
        return {};
    }
    for (const auto& de : fs::directory_iterator(dir, ec)) {
        if (ec) {
            break;
        }
        if (!de.is_regular_file()) {
            continue;
        }
        const fs::path& p = de.path();
        if (!is_png(p)) {
            continue;
        }
        const std::optional<int> idx = frame_index(p.filename().string(), rotation);
        if (!idx.has_value()) {
            continue;
        }
        indexed.emplace_back(*idx, fs::absolute(p).string());
    }
    std::sort(indexed.begin(), indexed.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });
    std::vector<std::string> frames;
    frames.reserve(indexed.size());
    for (auto& [idx, path] : indexed) {
        frames.push_back(std::move(path));
    }
    return frames;
}

}  // namespace

std::string resolve_gus_sprites_dir() {
    // 1) Override por ambiente (o lider aponta pra qualquer raiz de assets).
    if (const char* env = std::getenv("GUSWORLD_ASSETS")) {
        if (env[0] != '\0') {
            return join(env, "sprites/gus");
        }
    }
    // 2) Caminho do repo embutido em compilacao (raiz resources/ do repo).
    const std::string compiled = GUSWORLD_ASSETS_DIR;
    if (!compiled.empty()) {
        return join(compiled, "sprites/gus");
    }
    // 3) Relativo ao CWD (rodando da raiz do repo).
    return "resources/sprites/gus";
}

std::vector<AnimEntry> build_gus_anim_catalog(const std::string& gus_dir) {
    std::vector<AnimEntry> out;
    std::error_code ec;
    const fs::path root(gus_dir);
    if (!fs::is_directory(root, ec)) {
        return out;  // pasta ausente: lista vazia, viewer reporta e sai limpo
    }

    // (1) anims/<NOME>/f*.png  ->  uma anim por subpasta.
    const fs::path anims_dir = root / "anims";
    if (fs::is_directory(anims_dir, ec)) {
        std::vector<fs::path> subdirs;
        for (const auto& de : fs::directory_iterator(anims_dir, ec)) {
            if (ec) {
                break;
            }
            if (de.is_directory()) {
                subdirs.push_back(de.path());
            }
        }
        std::sort(subdirs.begin(), subdirs.end());
        for (const auto& sd : subdirs) {
            std::vector<std::string> frames = collect_frames(sd, /*rotation=*/false);
            if (!frames.empty()) {
                out.push_back({sd.filename().string(), std::move(frames)});
            }
        }
    }

    // (2) walk/<dir>/f*.png  ->  uma anim "walk_<dir>" por direcao.
    const fs::path walk_dir = root / "walk";
    if (fs::is_directory(walk_dir, ec)) {
        std::vector<fs::path> dirs;
        for (const auto& de : fs::directory_iterator(walk_dir, ec)) {
            if (ec) {
                break;
            }
            if (de.is_directory()) {
                dirs.push_back(de.path());
            }
        }
        std::sort(dirs.begin(), dirs.end());
        for (const auto& dd : dirs) {
            std::vector<std::string> frames = collect_frames(dd, /*rotation=*/false);
            if (!frames.empty()) {
                out.push_back(
                    {"walk_" + dd.filename().string(), std::move(frames)});
            }
        }
    }

    // (3) rotations/<i>_<nome>.png  ->  uma anim sintetica "turntable" (8 estaticos
    //     ciclando em loop, ordenados pelo indice).
    const fs::path rot_dir = root / "rotations";
    {
        std::vector<std::string> frames = collect_frames(rot_dir, /*rotation=*/true);
        if (!frames.empty()) {
            out.push_back({"turntable", std::move(frames)});
        }
    }

    // Ordena por rotulo pra navegacao estavel (Tab/setas).
    std::sort(out.begin(), out.end(),
              [](const AnimEntry& a, const AnimEntry& b) { return a.label < b.label; });
    return out;
}

}  // namespace gus::app::screens
