// gus/domain/i18n/translation_parity_validator.hpp
//
// Validador de paridade estrutural entre catalogos de traducao. Portado de
// engine/foundation/localization/TranslationParityValidator.cs (F2-S.12a).
//
// POCO puro, ZERO Qt (invariante de domain/). Garante que os locales NAO
// cresçam dessincronizados conforme o numero de chaves aumenta. Tres
// invariantes (preservadas do C#):
//   1. Paridade      - conjunto de chaves de source (pt_br) == target (en_intl);
//   2. Sem duplicata - nenhuma chave repetida no mesmo locale (o last-wins do
//                      parser e silencioso, so o conteudo cru revela);
//   3. Source nao-vazia - pt_br (lingua-fonte) NAO pode ter valor vazio; en_intl
//                      (fallback pos-v1.0.0) PODE.

#ifndef GUS_DOMAIN_I18N_TRANSLATION_PARITY_VALIDATOR_HPP
#define GUS_DOMAIN_I18N_TRANSLATION_PARITY_VALIDATOR_HPP

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace gus::domain::i18n {

// Locale ja parseado: chave -> valor. Igual ao TranslationCatalog do loader,
// repetido aqui para o validador nao depender do header do parser.
using Locale = std::map<std::string, std::string>;

// Relatorio imutavel de uma validacao. Os vetores vem ordenados (Ordinal),
// para diff de CI estavel e leitura humana previsivel. is_valid() so e true
// quando os tres estao vazios.
struct TranslationParityReport {
    // Chaves no source mas ausentes no target (esquecidas na traducao).
    std::vector<std::string> missing_in_target;
    // Chaves no target mas ausentes no source (orfas, sem fallback possivel).
    std::vector<std::string> missing_in_source;
    // Chaves com valor vazio/so-espacos no source (proibido na lingua-fonte).
    std::vector<std::string> empty_in_source;

    [[nodiscard]] bool is_valid() const noexcept {
        return missing_in_target.empty() && missing_in_source.empty() &&
               empty_in_source.empty();
    }

    // Sumario humano-legivel. "Paridade estrutural OK." quando valido; lista as
    // chaves problematicas por categoria quando invalido.
    [[nodiscard]] std::string to_summary() const;
};

// Compara dois locales ja parseados e devolve o relatorio. 'source' e a
// lingua-fonte (pt_br); 'target' e o fallback (en_intl), onde valores vazios
// sao aceitos.
[[nodiscard]] TranslationParityReport validate(const Locale& source, const Locale& target);

// Varre o conteudo .md CRU e devolve as chaves que aparecem mais de uma vez.
// Necessario porque parse() faz last-wins silencioso (o mapa resultante nunca
// revela a duplicata). So conta linhas "## X" com X em UPPER_SNAKE (headers de
// secao sao ignorados). Resultado ordenado, cada chave duplicada listada uma vez.
[[nodiscard]] std::vector<std::string> find_duplicate_keys(std::string_view content);

}  // namespace gus::domain::i18n

#endif  // GUS_DOMAIN_I18N_TRANSLATION_PARITY_VALIDATOR_HPP
