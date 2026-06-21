// gus/domain/i18n/md_translation_loader.hpp
//
// Parser de catalogo de traducoes em formato Markdown proprio. Portado de
// engine/foundation/localization/MdTranslationLoader.cs (so a parte Parse; o
// LoadFromFile dependia de Godot.FileAccess e fica para platform/fs).
//
// POCO puro, ZERO Qt (invariante de domain/, engine-design.md secao 2). Recebe
// o CONTEUDO do catalogo como string; NAO faz I/O.
//
// Formato:
//     ## NOME_DA_CHAVE            (UPPER_SNAKE_CASE: [A-Z][A-Z0-9_]*)
//     Valor da chave. Pode ter
//     varias linhas ate a proxima "## ".
//
// Regras preservadas do C#:
//   - so "## X" com X em UPPER_SNAKE vira chave; headers de secao
//     ("## §1. Menu") sao ignorados (nao viram chave-fantasma);
//   - linhas iniciadas por '#' dentro de um valor sao filtradas;
//   - valor multi-linha juntado por '\n', com trim das pontas;
//   - chave duplicada = last-wins;
//   - chave sem corpo mapeia para string vazia (nao some do mapa).

#ifndef GUS_DOMAIN_I18N_MD_TRANSLATION_LOADER_HPP
#define GUS_DOMAIN_I18N_MD_TRANSLATION_LOADER_HPP

#include <map>
#include <string>
#include <string_view>

namespace gus::domain::i18n {

// Mapa chave -> valor de um catalogo. std::map (ordenado) da iteracao
// deterministica de graca, util para diff/log; a busca e O(log n), irrelevante
// para o tamanho de um catalogo de UI.
using TranslationCatalog = std::map<std::string, std::string>;

// True se 's' e uma chave valida: UPPER_SNAKE_CASE, isto e, comeca com letra
// maiuscula ASCII e segue so com maiusculas/digitos/underscore. Equivalente ao
// regex ^[A-Z][A-Z0-9_]*$ do C#, sem arrastar <regex>. String vazia = false.
[[nodiscard]] bool is_translation_key(std::string_view s) noexcept;

// Parseia o conteudo de um catalogo .md e devolve o mapa chave -> valor.
// Parser puro: NAO interpola placeholders {0}/{1} (isso e responsabilidade de
// quem exibe o texto); eles sobrevivem literais no valor.
[[nodiscard]] TranslationCatalog parse(std::string_view content);

}  // namespace gus::domain::i18n

#endif  // GUS_DOMAIN_I18N_MD_TRANSLATION_LOADER_HPP
