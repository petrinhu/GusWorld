// gus/domain/src/input/controls_name.cpp
//
// Implementacao do sanitize de nome de perfil + nome de arquivo de controles
// (ADR-007). POCO puro, ZERO Qt, ZERO I/O. Ver header para o contrato. Travado por
// tests/controls_name_sanitize_test.cpp.

#include "gus/domain/input/controls_name.hpp"

#include <string>

namespace gus::domain::input {

namespace {

// ASCII a-z/A-Z.
bool is_ascii_alpha(unsigned char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool is_ascii_digit(unsigned char c) { return c >= '0' && c <= '9'; }

// Caractere ja seguro para nome de perfil: alfanumerico ASCII ou underscore.
// (O underscore e seguro mas NAO conta como "conteudo" para o fallback.)
bool is_safe_char(unsigned char c) {
    return is_ascii_alpha(c) || is_ascii_digit(c) || c == '_';
}

}  // namespace

std::string sanitize_profile_name(std::string_view raw_name) {
    std::string out;
    out.reserve(raw_name.size());
    bool has_content = false;  // ao menos um alfanumerico ASCII

    for (const char ch : raw_name) {
        const unsigned char c = static_cast<unsigned char>(ch);
        if (is_ascii_alpha(c)) {
            const char lower = static_cast<char>(c | 0x20u);  // to-lower ASCII
            out.push_back(lower);
            has_content = true;
        } else if (is_ascii_digit(c)) {
            out.push_back(static_cast<char>(c));
            has_content = true;
        } else if (c == '_') {
            // underscore ja vem seguro; preserva 1:1 (nao conta como conteudo).
            out.push_back('_');
        } else {
            // espaco, / \ : * ? " < > |, controle, nao-ASCII: tudo vira '_'.
            out.push_back('_');
        }
    }

    // Fallback: um perfil precisa de um identificador com conteudo real. Nome vazio
    // ou composto so de '_' (ex.: "///" -> "___") cai no perfil "default".
    if (!has_content) return std::string(kDefaultProfile);
    return out;
}

std::string controls_file_name(std::string_view raw_name) {
    return sanitize_profile_name(raw_name) + "_controls.json";
}

}  // namespace gus::domain::input
