// gus/domain/src/map/map_csv.cpp
//
// Compilador de mapa (lado puro): CSV -> TileMap. Ver map_csv.hpp para o formato e
// a politica de erro. POCO puro, ZERO Qt/SDL/I/O (opera sobre string em memoria).

#include "gus/domain/map/map_csv.hpp"

#include <cctype>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace gus::domain::map {

namespace {

// Tira espacos/tab das pontas. string_view -> string_view (sem alocar).
std::string_view trim(std::string_view s) {
    std::size_t b = 0;
    std::size_t e = s.size();
    while (b < e && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r')) ++b;
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r')) --e;
    return s.substr(b, e - b);
}

// Quebra por delimitador, sem trim (o chamador decide). Mantem campos vazios.
std::vector<std::string_view> split(std::string_view s, char delim) {
    std::vector<std::string_view> out;
    std::size_t start = 0;
    for (std::size_t i = 0; i <= s.size(); ++i) {
        if (i == s.size() || s[i] == delim) {
            out.push_back(s.substr(start, i - start));
            start = i + 1;
        }
    }
    return out;
}

// Tokeniza por espacos (1+), trim implicito. Para diretivas "#spawn 1 1".
std::vector<std::string_view> tokens(std::string_view s) {
    std::vector<std::string_view> out;
    std::size_t i = 0;
    while (i < s.size()) {
        while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
        std::size_t start = i;
        while (i < s.size() && s[i] != ' ' && s[i] != '\t') ++i;
        if (i > start) out.push_back(s.substr(start, i - start));
    }
    return out;
}

// Parseia um tile-id: deve ser inteiro nao-negativo que cabe em uint16. Qualquer
// desvio (vazio, nao-digito, negativo, > 65535) lanca MapCsvError com a linha.
std::uint16_t parse_tile_id(std::string_view tok, std::size_t line_1based) {
    const std::string_view t = trim(tok);
    if (t.empty())
        throw MapCsvError("CSV linha " + std::to_string(line_1based) +
                          ": celula vazia.");
    for (char c : t) {
        if (std::isdigit(static_cast<unsigned char>(c)) == 0)
            throw MapCsvError("CSV linha " + std::to_string(line_1based) +
                              ": tile-id invalido '" + std::string(t) +
                              "' (esperado inteiro 0..65535).");
    }
    // So digitos: converte com guarda de range.
    unsigned long long v = 0;
    for (char c : t) {
        v = v * 10ull + static_cast<unsigned long long>(c - '0');
        if (v > 65535ull)
            throw MapCsvError("CSV linha " + std::to_string(line_1based) +
                              ": tile-id '" + std::string(t) +
                              "' excede 65535.");
    }
    return static_cast<std::uint16_t>(v);
}

// Parseia um inteiro (coordenada/contagem) com sinal opcional. Lanca MapCsvError.
std::int32_t parse_int(std::string_view tok, std::size_t line_1based,
                       const char* what) {
    const std::string_view t = trim(tok);
    try {
        std::size_t consumed = 0;
        const int v = std::stoi(std::string(t), &consumed);
        if (consumed != t.size()) throw std::invalid_argument("lixo");
        return v;
    } catch (...) {
        throw MapCsvError("CSV linha " + std::to_string(line_1based) + ": " +
                          what + " invalido '" + std::string(t) + "'.");
    }
}

float parse_float(std::string_view tok, std::size_t line_1based,
                  const char* what) {
    const std::string_view t = trim(tok);
    try {
        std::size_t consumed = 0;
        const float v = std::stof(std::string(t), &consumed);
        if (consumed != t.size()) throw std::invalid_argument("lixo");
        return v;
    } catch (...) {
        throw MapCsvError("CSV linha " + std::to_string(line_1based) + ": " +
                          what + " invalido '" + std::string(t) + "'.");
    }
}

}  // namespace

TileMap parse_csv_to_tilemap(std::string_view csv) {
    // Metadados (default) e a grade crua, coletados em uma passada.
    float tile_size = 1.0f;
    bool has_spawn = false;
    Cell spawn{0, 0};
    std::vector<Portal> portals;
    std::string map_id;  // UUID de identidade (#map_id); vazio se ausente.

    std::vector<std::vector<std::uint16_t>> rows;
    int width = -1;  // largura da primeira linha de grade

    std::size_t line_no = 0;
    std::size_t start = 0;
    for (std::size_t i = 0; i <= csv.size(); ++i) {
        if (i != csv.size() && csv[i] != '\n') continue;
        const std::string_view raw = csv.substr(start, i - start);
        start = i + 1;
        ++line_no;

        const std::string_view line = trim(raw);
        if (line.empty()) continue;
        if (line.size() >= 2 && line[0] == '/' && line[1] == '/') continue;

        if (line[0] == '#') {
            // Diretiva de metadado.
            const auto tk = tokens(line.substr(1));  // sem o '#'
            if (tk.empty()) continue;
            const std::string_view dir = tk[0];
            if (dir == "map_id") {
                if (tk.size() != 2)
                    throw MapCsvError("CSV linha " + std::to_string(line_no) +
                                      ": #map_id espera 1 valor (uuid).");
                map_id = std::string(tk[1]);
            } else if (dir == "tile_size") {
                if (tk.size() != 2)
                    throw MapCsvError("CSV linha " + std::to_string(line_no) +
                                      ": #tile_size espera 1 valor.");
                tile_size = parse_float(tk[1], line_no, "tile_size");
            } else if (dir == "spawn") {
                if (tk.size() != 3)
                    throw MapCsvError("CSV linha " + std::to_string(line_no) +
                                      ": #spawn espera <x> <y>.");
                spawn.x = parse_int(tk[1], line_no, "spawn x");
                spawn.y = parse_int(tk[2], line_no, "spawn y");
                has_spawn = true;
            } else if (dir == "portal") {
                if (tk.size() != 4)
                    throw MapCsvError("CSV linha " + std::to_string(line_no) +
                                      ": #portal espera <id> <x> <y>.");
                Portal p;
                p.id = std::string(tk[1]);
                p.cell.x = parse_int(tk[2], line_no, "portal x");
                p.cell.y = parse_int(tk[3], line_no, "portal y");
                portals.push_back(std::move(p));
            } else {
                throw MapCsvError("CSV linha " + std::to_string(line_no) +
                                  ": diretiva desconhecida '#" +
                                  std::string(dir) + "'.");
            }
            continue;
        }

        // Linha de grade: numeros separados por virgula.
        const auto cells = split(line, ',');
        std::vector<std::uint16_t> row;
        row.reserve(cells.size());
        for (std::string_view c : cells) row.push_back(parse_tile_id(c, line_no));

        if (width < 0) {
            width = static_cast<int>(row.size());
        } else if (static_cast<int>(row.size()) != width) {
            throw MapCsvError("CSV linha " + std::to_string(line_no) +
                              ": largura " + std::to_string(row.size()) +
                              " != largura esperada " + std::to_string(width) +
                              ".");
        }
        rows.push_back(std::move(row));
    }

    if (rows.empty() || width <= 0)
        throw MapCsvError("CSV sem nenhuma linha de grade.");

    TileMap map(width, static_cast<int>(rows.size()), tile_size);
    for (std::int32_t y = 0; y < static_cast<std::int32_t>(rows.size()); ++y)
        for (std::int32_t x = 0; x < width; ++x)
            map.set(x, y, rows[static_cast<std::size_t>(y)]
                              [static_cast<std::size_t>(x)]);

    if (has_spawn) map.set_spawn(spawn);
    for (auto& p : portals) map.add_portal(std::move(p));
    map.set_map_id(std::move(map_id));  // identidade (vazio se sem #map_id)

    // Invariantes (spawn/portal dentro dos limites): std::invalid_argument se nao.
    map.validate();
    return map;
}

}  // namespace gus::domain::map
