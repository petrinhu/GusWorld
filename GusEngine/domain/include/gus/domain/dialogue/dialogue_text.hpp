// gus/domain/dialogue/dialogue_text.hpp
//
// Parser POCO do FORMATO-TEXTO proprio de dialogo (ADR-014 decisao 1): string em
// memoria -> DialogueGraph validado. Fonte editavel no repo (mesmo espirito do CSV
// de mapa gus/domain/map/map_csv.hpp e do catalogo i18n .md), onde o
// narrative-designer/narrative-writer trabalha. POCO puro, ZERO Qt/SDL, ZERO I/O
// real (o platform/ le o arquivo-fonte do disco -- ex. "npc_intro.dlg.txt" -- e
// passa o conteudo aqui, mesmo padrao de translator.cpp/font_atlas.cpp).
//
// FORMATO (extensao convencional sugerida: .dlg.txt):
//
//   Linhas em branco e comentarios de linha inteira "//" sao ignorados.
//
//   #meta dialogue_id <id>                  identidade do dialogo (obrigatorio, 1x)
//   #meta default_register terminal|warm    registro default do grafo (obrigatorio)
//   #meta entry <node_id>                   no de entrada (obrigatorio)
//
//   @node <id>                              abre um bloco de no (id unico no arquivo)
//     speaker: <speaker_id>                 quem fala
//     text: <CHAVE_I18N>                    chave i18n da fala (resolvida por
//                                            Translator::tr() no app/, nao aqui)
//     register: terminal|warm               override de registro (opcional)
//     on_enter: <flag_key>=<true|false>      efeito ao ENTRAR neste no (opcional)
//     -> <next_id>                          transicao LINEAR (no SEM opcoes;
//                                            next_id = outro @node ou "@exit")
//     - [<LABEL_KEY>] -> <next_id> [flag:<flag_key>=<true|false>]
//                                            UMA linha de OPCAO (>=2 por no de
//                                            escolha); LABEL_KEY = chave i18n do
//                                            texto do botao; "flag:..." e o efeito
//                                            opcional aplicado ao ESCOLHER essa opcao.
//
//   Um no e LINEAR (usa exatamente uma linha "->") OU de ESCOLHA (usa 2+ linhas
//   "- [...] -> ..."); nao mistura os dois no mesmo bloco. O bloco do no termina no
//   proximo "@node" ou no fim do arquivo.
//
//   Convergencia (2+ escolhas apontando pro MESMO next_id, ex. o Bertoldo) e so
//   repetir o mesmo next_id em linhas de opcao diferentes -- nao ha sintaxe
//   especial de "merge".
//
// VALIDACAO: diretiva/token malformado (falta "#meta" obrigatorio, "@node" sem id,
// "->"/"- [...]" sem destino, "chave: valor" sem ":", flag sem "=true|false",
// registro fora de {terminal,warm}, diretiva fora de um bloco @node, id de @node
// duplicado) -> DialogueTextError (mensagem com a linha 1-based). Grafo
// estruturalmente incoerente (entry inexistente, next_id/opcao apontando para no
// inexistente, no orfao, no de escolha com <2 opcoes) -> DialogueGraph::validate()
// -> std::invalid_argument (mesma disciplina de map_csv.cpp).
//
// Cross-ref: ADR-014, dialogue_graph.hpp, gus/domain/map/map_csv.hpp (parser irmao
// no mesmo espirito), docs/design/narrativa/dialogue-tree-npc-intro.md (blueprint
// que este formato precisa conseguir renderizar).

#ifndef GUS_DOMAIN_DIALOGUE_DIALOGUE_TEXT_HPP
#define GUS_DOMAIN_DIALOGUE_DIALOGUE_TEXT_HPP

#include <stdexcept>
#include <string>
#include <string_view>

#include "gus/domain/dialogue/dialogue_graph.hpp"

namespace gus::domain::dialogue {

// Formato-texto malformado (diretiva desconhecida, sintaxe quebrada, #meta
// ausente). Carrega a mensagem com a linha 1-based quando aplicavel. NAO cobre
// incoerencia ESTRUTURAL do grafo (entry/next_id inexistente, no orfao, no de
// escolha com <2 opcoes): isso e invariante do DialogueGraph
// (std::invalid_argument via validate(), chamado no fim do parse).
class DialogueTextError : public std::runtime_error {
public:
    explicit DialogueTextError(const std::string& message) : std::runtime_error(message) {}
};

// Parser POCO: formato-texto (string) -> DialogueGraph validado. Lanca
// DialogueTextError (formato) ou std::invalid_argument (estrutura do grafo, via
// DialogueGraph::validate() chamado ao final).
[[nodiscard]] DialogueGraph parse_text_to_dialogue_graph(std::string_view text);

}  // namespace gus::domain::dialogue

#endif  // GUS_DOMAIN_DIALOGUE_DIALOGUE_TEXT_HPP
