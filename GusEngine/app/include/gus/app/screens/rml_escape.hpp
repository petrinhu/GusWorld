// SPDX-License-Identifier: Apache-2.0
// gus/app/screens/rml_escape.hpp
//
// I18N-ESCAPE-MARKUP: escape de METACARACTERES de markup para o texto EXTERNO que
// as 6 telas injetam no RML que elas mesmas montam (catalogo de traducao
// resources/translations/*.md e arquivos de dialogo resources/dialogues/*.dlg.txt).
//
// POR QUE EXISTE (o achado, 2026-08-04): o parser de RML proprio que o glintfx esta
// construindo recebeu 15 fixtures com o markup final destas telas e REJEITOU uma - o
// '&' literal do rotulo "Menu & Diálogo" (CONTROLS_GROUP_MENU_DIALOGUE, tela de
// Controles), com "malformed entity reference: '&' not closed by a terminating ';'".
// O RmlUi de hoje TOLERA (quando nenhuma entidade conhecida casa, copia o '&' e
// segue), entao o jogo nunca quebrou - e o glintfx ja fez o parser dele tolerar
// tambem. Portanto isto NAO e conserto de bug ativo NEM contorno de lacuna do
// glintfx (a lei da casa proibiria contornar): e HIGIENE do NOSSO gerador de markup.
// O risco que fecha e FUTURO e tem nome: a traducao en-intl esta planejada, e um
// tradutor escrevendo "Save & Exit" ou "A < B" produz markup que um parser ESTRITO
// recusa.
//
// CAMADA (app/): escapar e manipulacao de string PURA (sem SDL/GL/glintfx/I-O), entao
// caberia em qualquer camada pelo criterio de dependencia. Fica em app/screens/ pelo
// criterio de DONO: o markup RML so nasce aqui (app/src/screens/*_rml.cpp); core/ e
// primitiva de engine (tempo/espaco/cripto/anim) e domain/ e regra de jogo - markup
// nao e nem um nem outro. Mesmo racional que ja pos o escape de JSON em
// domain/src/input/controls_json.cpp (append_escaped): o escape mora onde mora o
// formato que ele serve.
//
// CONTRATO DE USO (as 3 armadilhas):
//   1. ORDEM: '&' e substituido PRIMEIRO. Se '<' viesse antes, o '&' que o proprio
//      escape introduz ("&lt;") seria reescapado depois e viraria "&amp;lt;".
//   2. PONTO UNICO: aplicar SO na string de texto externo, no instante em que ela
//      entra no ostringstream - NUNCA na saida montada (escaparia as nossas proprias
//      tags) e NUNCA duas vezes na mesma string (viraria "&amp;amp;"). Nao ha
//      idempotencia por design: e o ponto unico de aplicacao que impede o duplo
//      escape, nao a funcao.
//   3. NAO passar por aqui o markup DECORATIVO que nos escrevemos a mao - o "&gt; "
//      do marcador de opcao selecionada e os "&nbsp;" de npc_dialogue_rml.cpp sao
//      entidades DELIBERADAS; escapa-las mostraria "&gt;" literal na tela.
//
// ESCOPO: os 3 metacaracteres de TEXTO ('&', '<', '>'). Aspas ('"'/'\'') NAO entram:
// o uso e conteudo de elemento, nao valor de atributo. O unico ponto de ATRIBUTO que
// recebe dado externo (o nome do arquivo de retrato em npc_dialogue_rml.cpp) esta
// documentado la.

#ifndef GUS_APP_SCREENS_RML_ESCAPE_HPP
#define GUS_APP_SCREENS_RML_ESCAPE_HPP

#include <string>
#include <string_view>

namespace gus::app::screens {

// Devolve `text` com os 3 metacaracteres de markup trocados pelas entidades
// correspondentes: '&' -> "&amp;", '<' -> "&lt;", '>' -> "&gt;". O '&' SEMPRE
// primeiro (ver armadilha 1 no topo). Demais bytes (UTF-8 inclusive) passam
// intactos. NUNCA lanca.
[[nodiscard]] std::string rml_escape(std::string_view text);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_RML_ESCAPE_HPP
