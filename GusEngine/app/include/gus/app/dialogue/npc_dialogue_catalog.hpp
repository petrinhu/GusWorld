// gus/app/dialogue/npc_dialogue_catalog.hpp
//
// I/O REAL de arquivo do grafo de dialogo do Bertoldo (M7-DIALOGO, NPC-MVP): le o
// .dlg.txt do disco na FRONTEIRA app/ (mesma disciplina de Translator::load_from_
// file/resolve_translations_path - domain/dialogue permanece ZERO I/O) e entrega ao
// parser POCO de dominio (gus::domain::dialogue::parse_text_to_dialogue_graph).
//
// Cross-ref: gus/domain/dialogue/dialogue_text.hpp (parser, formato .dlg.txt),
//            game/dialogues/npc_intro_bertoldo.dlg.txt (fonte editavel),
//            gus/app/i18n/translator.hpp (MESMA receita de resolucao de path).

#ifndef GUS_APP_DIALOGUE_NPC_DIALOGUE_CATALOG_HPP
#define GUS_APP_DIALOGUE_NPC_DIALOGUE_CATALOG_HPP

#include <optional>
#include <string>

#include "gus/domain/dialogue/dialogue_graph.hpp"

namespace gus::app::dialogue {

// Resolve o caminho do .dlg.txt do Bertoldo: env GUSWORLD_DIALOGUES (caminho
// COMPLETO do arquivo, o lider aponta direto - mesmo contrato de GUSWORLD_
// TRANSLATIONS) > macro embutida em compilacao (GUSWORLD_DIALOGUES_DIR) > relativo
// ao CWD (game/dialogues/npc_intro_bertoldo.dlg.txt). So MONTA a string; nao le
// nada do disco.
[[nodiscard]] std::string resolve_npc_intro_bertoldo_dialogue_path();

// Le `path` (I/O real) e delega o parse a
// gus::domain::dialogue::parse_text_to_dialogue_graph. Devolve std::nullopt se o
// arquivo nao existir/estiver vazio - degradacao segura, MESMO padrao de
// Translator::load_from_file (o chamador decide o fallback: sem NPC nesta sessao,
// nunca crasha). DialogueTextError (formato malformado) / std::invalid_argument
// (grafo estruturalmente incoerente) PROPAGAM se o conteudo existir mas estiver
// errado - fail-fast, e erro de AUTORIA a corrigir no .dlg.txt, nao um caso de I/O.
[[nodiscard]] std::optional<gus::domain::dialogue::DialogueGraph>
load_dialogue_graph_from_file(const std::string& path);

}  // namespace gus::app::dialogue

#endif  // GUS_APP_DIALOGUE_NPC_DIALOGUE_CATALOG_HPP
