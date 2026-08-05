// SPDX-License-Identifier: Apache-2.0
// gus/app/src/screens/rml_escape.cpp
//
// Implementacao de rml_escape. Ver header para o contrato, o achado que originou a
// fatia (I18N-ESCAPE-MARKUP) e as 3 armadilhas de uso.

#include "gus/app/screens/rml_escape.hpp"

namespace gus::app::screens {

std::string rml_escape(std::string_view text) {
    // UMA passada, byte a byte, decidindo por caractere. Isto e o que torna a
    // ARMADILHA DA ORDEM impossivel de reintroduzir por construcao: a alternativa
    // ingenua (3 find/replace em sequencia sobre a string inteira) so esta correta
    // se o '&' for o PRIMEIRO, senao o '&' que o escape do '<' introduz ("&lt;")
    // e reescapado na passada seguinte e vira "&amp;lt;". Aqui cada byte da ENTRADA
    // e classificado uma vez e o que se escreve na SAIDA nunca e reexaminado - nao
    // existe "segunda passada" onde o bug possa nascer.
    std::string out;
    out.reserve(text.size());  // caso comum: nada a escapar, zero realoc
    for (const char c : text) {
        switch (c) {
            case '&':
                out += "&amp;";
                break;
            case '<':
                out += "&lt;";
                break;
            case '>':
                out += "&gt;";
                break;
            default:
                out += c;  // demais bytes (UTF-8 multibyte inclusive) passam intactos
                break;
        }
    }
    return out;
}

}  // namespace gus::app::screens
