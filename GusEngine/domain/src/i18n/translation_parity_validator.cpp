// gus/domain/i18n/translation_parity_validator.cpp
//
// Implementacao do validador de paridade estrutural i18n. Ver header para o
// contrato e as invariantes preservadas do C# (TranslationParityValidator).

#include "gus/domain/i18n/translation_parity_validator.hpp"

#include <algorithm>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "gus/domain/i18n/md_translation_loader.hpp"  // is_translation_key (regra compartilhada)

namespace gus::domain::i18n {

namespace {

constexpr std::string_view kWhitespace = " \t\r\n\f\v";

// True se a string e vazia ou so whitespace (equivalente a
// string.IsNullOrWhiteSpace do .NET para o conjunto ASCII relevante).
bool is_blank(std::string_view s) {
    return s.find_first_not_of(kWhitespace) == std::string_view::npos;
}

// Apara o '\r' de fim de linha (catalogo salvo com CRLF) e nada mais: aqui so
// se compara o conteudo cru das linhas "## X" para contar duplicatas.
std::string_view strip_cr(std::string_view line) {
    if (!line.empty() && line.back() == '\r') {
        line.remove_suffix(1);
    }
    return line;
}

// Trim das pontas, para extrair a chave de "## <chave>".
std::string_view trim_view(std::string_view s) {
    const auto begin = s.find_first_not_of(kWhitespace);
    if (begin == std::string_view::npos) {
        return std::string_view{};
    }
    const auto end = s.find_last_not_of(kWhitespace);
    return s.substr(begin, end - begin + 1);
}

void append_category(std::string& sb, std::string_view label,
                     const std::vector<std::string>& keys) {
    if (keys.empty()) {
        return;
    }
    sb += "\n  - ";
    sb += label;
    sb += ": ";
    for (std::size_t i = 0; i < keys.size(); ++i) {
        if (i != 0) {
            sb += ", ";
        }
        sb += keys[i];
    }
}

}  // namespace

std::string TranslationParityReport::to_summary() const {
    if (is_valid()) {
        return "Paridade estrutural OK.";
    }
    std::string sb = "Falha de paridade estrutural i18n:";
    append_category(sb, "missing_in_target (no source, falta no target)", missing_in_target);
    append_category(sb, "missing_in_source (no target, falta no source / orfa)",
                    missing_in_source);
    append_category(sb, "empty_in_source (valor vazio na lingua-fonte)", empty_in_source);
    return sb;
}

TranslationParityReport validate(const Locale& source, const Locale& target) {
    TranslationParityReport report;

    for (const auto& [key, value] : source) {
        if (!target.contains(key)) {
            report.missing_in_target.push_back(key);
        }
        if (is_blank(value)) {
            report.empty_in_source.push_back(key);
        }
    }

    for (const auto& [key, value] : target) {
        if (!source.contains(key)) {
            report.missing_in_source.push_back(key);
        }
    }

    // std::map ja itera em ordem de chave, mas ordena-se explicitamente para
    // nao depender do tipo do container e manter a garantia mesmo se a entrada
    // mudar (espelha o Sort(Ordinal) do C#).
    std::sort(report.missing_in_target.begin(), report.missing_in_target.end());
    std::sort(report.missing_in_source.begin(), report.missing_in_source.end());
    std::sort(report.empty_in_source.begin(), report.empty_in_source.end());

    return report;
}

std::vector<std::string> find_duplicate_keys(std::string_view content) {
    std::unordered_map<std::string, int> counts;

    std::size_t pos = 0;
    while (pos <= content.size()) {
        const std::size_t nl = content.find('\n', pos);
        std::string_view line;
        if (nl == std::string_view::npos) {
            line = content.substr(pos);
            pos = content.size() + 1;
        } else {
            line = content.substr(pos, nl - pos);
            pos = nl + 1;
        }
        line = strip_cr(line);

        if (line.size() < 3 || line.substr(0, 3) != "## ") {
            continue;
        }
        const std::string_view candidate = trim_view(line.substr(3));
        if (!is_translation_key(candidate)) {
            continue;  // headers de secao nao sao chaves
        }
        ++counts[std::string{candidate}];
    }

    std::vector<std::string> dups;
    for (const auto& [key, n] : counts) {
        if (n > 1) {
            dups.push_back(key);
        }
    }
    std::sort(dups.begin(), dups.end());
    return dups;
}

}  // namespace gus::domain::i18n
