// gus/app/src/screens/npc_dialogue_overlay.cpp
//
// Implementacao da logica de interacao do overlay de dialogo do NPC. Ver header.

#include "gus/app/screens/npc_dialogue_overlay.hpp"

#include <cctype>

namespace gus::app::screens {

namespace {

// "bertoldo" -> "ACTOR_BERTOLDO" (MESMA convencao de ACTOR_GUS/ACTOR_CAUA, ver
// pt_br.md §8: "ACTOR_<id_upper>; fallback = id lowercase se ausente").
std::string actor_key_for(const std::string& speaker_id) {
    std::string key = "ACTOR_";
    key.reserve(6 + speaker_id.size());
    for (const char c : speaker_id) {
        key += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return key;
}

}  // namespace

std::string npc_dialogue_actor_display_name(
    const std::string& speaker_id, const gus::app::i18n::Translator& translator) {
    const std::string actor_key = actor_key_for(speaker_id);
    std::string speaker_label = translator.tr(actor_key);
    if (speaker_label == actor_key) {
        speaker_label = speaker_id;  // fallback: id em minusculo (chave ausente)
    }
    return speaker_label;
}

std::vector<std::string> npc_dialogue_overlay_lines(
    const gus::domain::dialogue::DialogueNode& node,
    const gus::app::i18n::Translator& translator, int selected_option) {
    std::vector<std::string> lines;

    const std::string speaker_label =
        npc_dialogue_actor_display_name(node.speaker_id, translator);
    lines.push_back(speaker_label + ":");
    lines.push_back(translator.tr(node.text_key));

    if (node.options.empty()) {
        lines.push_back(translator.tr("DIALOGUE_CONTINUE"));
    } else {
        for (std::size_t i = 0; i < node.options.size(); ++i) {
            const bool is_selected = static_cast<int>(i) == selected_option;
            lines.push_back((is_selected ? "> " : "  ") +
                             translator.tr(node.options[i].label_key));
        }
    }
    return lines;
}

int apply_npc_dialogue_input(gus::domain::dialogue::DialogueRuntime& runtime,
                              NpcDialogueInputAction action, int selected_option) {
    if (runtime.finished()) {
        return selected_option;  // conversa ja encerrada - o chamador fecha o loop
    }
    const gus::domain::dialogue::DialogueNode& node = runtime.current();
    const bool is_choice_node = !node.options.empty();

    switch (action) {
        case NpcDialogueInputAction::None:
            return selected_option;
        case NpcDialogueInputAction::MoveUp:
            if (!is_choice_node) return selected_option;
            return npc_dialogue_move_selection(selected_option, -1,
                                                static_cast<int>(node.options.size()));
        case NpcDialogueInputAction::MoveDown:
            if (!is_choice_node) return selected_option;
            return npc_dialogue_move_selection(selected_option, +1,
                                                static_cast<int>(node.options.size()));
        case NpcDialogueInputAction::Confirm:
            if (is_choice_node) {
                runtime.choose(static_cast<std::size_t>(selected_option));
            } else {
                runtime.advance();
            }
            return 0;  // selecao SEMPRE reseta ao entrar num no novo
    }
    return selected_option;
}

}  // namespace gus::app::screens
