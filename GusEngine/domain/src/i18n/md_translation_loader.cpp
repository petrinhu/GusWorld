// gus/domain/i18n/md_translation_loader.cpp
//
// Implementacao do parser de catalogo .md. Ver header para o contrato e as
// regras preservadas do C# (MdTranslationLoader.Parse).

#include "gus/domain/i18n/md_translation_loader.hpp"

#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace gus::domain::i18n {

namespace {

// Whitespace ASCII, como String.Trim()/IsNullOrWhiteSpace do .NET para o
// conjunto que aparece nos catalogos (espaco, tab, CR, LF, FF, VT).
constexpr std::string_view kWhitespace = " \t\r\n\f\v";

// Remove whitespace das duas pontas. Devolve "" se for tudo whitespace.
std::string trim(std::string_view s) {
    const auto begin = s.find_first_not_of(kWhitespace);
    if (begin == std::string_view::npos) {
        return std::string{};
    }
    const auto end = s.find_last_not_of(kWhitespace);
    return std::string{s.substr(begin, end - begin + 1)};
}

// Junta as linhas de valor com '\n' e apara as pontas. Vazio se nao ha linhas.
std::string join_and_trim(const std::vector<std::string_view>& lines) {
    if (lines.empty()) {
        return std::string{};
    }
    std::string joined;
    for (std::size_t i = 0; i < lines.size(); ++i) {
        if (i != 0) {
            joined.push_back('\n');
        }
        joined.append(lines[i]);
    }
    return trim(joined);
}

}  // namespace

bool is_translation_key(std::string_view s) noexcept {
    if (s.empty()) {
        return false;
    }
    const auto is_upper = [](char c) { return c >= 'A' && c <= 'Z'; };
    const auto is_digit = [](char c) { return c >= '0' && c <= '9'; };
    if (!is_upper(s.front())) {
        return false;  // deve comecar com letra maiuscula ASCII
    }
    for (const char c : s) {
        if (!is_upper(c) && !is_digit(c) && c != '_') {
            return false;
        }
    }
    return true;
}

TranslationCatalog parse(std::string_view content) {
    TranslationCatalog result;
    std::string current_key;
    std::vector<std::string_view> current_value_lines;
    bool in_entry = false;

    std::size_t pos = 0;
    while (pos <= content.size()) {
        // Fatia uma linha [pos, nl) e avanca pos para depois do '\n'.
        const std::size_t nl = content.find('\n', pos);
        std::string_view line;
        if (nl == std::string_view::npos) {
            line = content.substr(pos);
            pos = content.size() + 1;  // marca fim apos a ultima linha
        } else {
            line = content.substr(pos, nl - pos);
            pos = nl + 1;
        }
        // Catalogo salvo com CRLF: apara o '\r' que sobra no fim da linha.
        if (!line.empty() && line.back() == '\r') {
            line.remove_suffix(1);
        }

        // Candidato a chave: "## X". So vira chave de fato se X for UPPER_SNAKE.
        // Headers de secao ("## §1. Menu") falham aqui e nao viram chave.
        if (line.size() >= 3 && line.substr(0, 3) == "## " &&
            is_translation_key(trim(line.substr(3)))) {
            // Fecha a entry anterior (last-wins: operator[] sobrescreve).
            if (in_entry) {
                result[current_key] = join_and_trim(current_value_lines);
            }
            current_key = trim(line.substr(3));
            current_value_lines.clear();
            in_entry = true;
        } else if (in_entry) {
            // Acumula linhas de valor. Filtra qualquer linha header MD ("# X",
            // "### X", e "## §N" que nao passou no teste de chave acima).
            if (!line.empty() && line.front() == '#') {
                continue;
            }
            current_value_lines.push_back(line);
        }
    }

    // Nao esquecer a ultima entry.
    if (in_entry) {
        result[current_key] = join_and_trim(current_value_lines);
    }

    return result;
}

}  // namespace gus::domain::i18n
