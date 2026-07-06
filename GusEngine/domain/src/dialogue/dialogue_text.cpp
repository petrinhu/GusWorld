// gus/domain/src/dialogue/dialogue_text.cpp
//
// Implementacao do parser POCO do formato-texto de dialogo. Ver dialogue_text.hpp
// para a sintaxe. POCO puro, ZERO Qt/SDL/I-O (opera sobre string em memoria).

#include "gus/domain/dialogue/dialogue_text.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace gus::domain::dialogue {

namespace {

// Tira espacos/tab/CR das pontas. string_view -> string_view (sem alocar).
std::string_view trim(std::string_view s) {
    std::size_t b = 0;
    std::size_t e = s.size();
    while (b < e && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r')) ++b;
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r')) --e;
    return s.substr(b, e - b);
}

bool starts_with(std::string_view s, std::string_view prefix) noexcept {
    return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
}

// "chave: valor" -> {chave (trim), valor (trim)}. Erro se nao ha ':'.
std::pair<std::string_view, std::string_view> split_kv(std::string_view line,
                                                        std::size_t line_no) {
    const auto pos = line.find(':');
    if (pos == std::string_view::npos)
        throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                                ": diretiva de no espera 'chave: valor': '" +
                                std::string(line) + "'.");
    return {trim(line.substr(0, pos)), trim(line.substr(pos + 1))};
}

// "chave=true|false" -> FlagEffect.
FlagEffect parse_flag_effect(std::string_view expr, std::size_t line_no) {
    const auto pos = expr.find('=');
    if (pos == std::string_view::npos)
        throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                                ": efeito de flag malformado (esperado"
                                " 'chave=true|false'): '" + std::string(expr) + "'.");
    const std::string_view key = trim(expr.substr(0, pos));
    const std::string_view val = trim(expr.substr(pos + 1));
    if (key.empty())
        throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                                ": efeito de flag sem chave.");
    FlagEffect fx;
    fx.flag_key = std::string(key);
    if (val == "true") {
        fx.value = true;
    } else if (val == "false") {
        fx.value = false;
    } else {
        throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                                ": valor de flag invalido (esperado true|false): '" +
                                std::string(val) + "'.");
    }
    return fx;
}

DialogueRegister parse_register(std::string_view tok, std::size_t line_no) {
    if (tok == "terminal") return DialogueRegister::Terminal;
    if (tok == "warm") return DialogueRegister::Warm;
    throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                            ": registro invalido (esperado terminal|warm): '" +
                            std::string(tok) + "'.");
}

}  // namespace

DialogueGraph parse_text_to_dialogue_graph(std::string_view text) {
    DialogueGraph graph;
    bool has_dialogue_id = false;
    bool has_default_register = false;
    bool has_entry = false;

    std::map<std::string, DialogueNode> nodes;
    std::optional<DialogueNode> current;  // no em construcao (entre "@node"s)

    const auto finalize_current = [&]() {
        if (!current.has_value()) return;
        if (nodes.find(current->id) != nodes.end())
            throw DialogueTextError("dialogo: no '" + current->id + "' duplicado.");
        nodes.emplace(current->id, std::move(*current));
        current.reset();
    };

    std::size_t line_no = 0;
    std::size_t start = 0;
    for (std::size_t i = 0; i <= text.size(); ++i) {
        if (i != text.size() && text[i] != '\n') continue;
        const std::string_view raw = text.substr(start, i - start);
        start = i + 1;
        ++line_no;

        const std::string_view line = trim(raw);
        if (line.empty()) continue;
        if (line.size() >= 2 && line[0] == '/' && line[1] == '/') continue;

        if (starts_with(line, "#meta ")) {
            const std::string_view rest = trim(line.substr(6));
            const auto sp = rest.find(' ');
            if (sp == std::string_view::npos)
                throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                                        ": #meta espera '<chave> <valor>'.");
            const std::string_view key = trim(rest.substr(0, sp));
            const std::string_view val = trim(rest.substr(sp + 1));
            if (val.empty())
                throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                                        ": #meta '" + std::string(key) +
                                        "' sem valor.");
            if (key == "dialogue_id") {
                graph.dialogue_id = std::string(val);
                has_dialogue_id = true;
            } else if (key == "default_register") {
                graph.default_register = parse_register(val, line_no);
                has_default_register = true;
            } else if (key == "entry") {
                graph.entry_node_id = std::string(val);
                has_entry = true;
            } else {
                throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                                        ": #meta desconhecido '" + std::string(key) +
                                        "'.");
            }
            continue;
        }

        if (starts_with(line, "@node ")) {
            finalize_current();
            const std::string_view id = trim(line.substr(6));
            if (id.empty())
                throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                                        ": @node sem id.");
            DialogueNode node;
            node.id = std::string(id);
            current = std::move(node);
            continue;
        }

        // Demais diretivas exigem um @node aberto.
        if (!current.has_value())
            throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                                    ": diretiva fora de um bloco @node: '" +
                                    std::string(line) + "'.");

        if (starts_with(line, "->")) {
            const std::string_view next = trim(line.substr(2));
            if (next.empty())
                throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                                        ": '->' sem destino.");
            if (!current->next_node_id.empty())
                throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                                        ": no '" + current->id +
                                        "' ja tem '->' definido (1 por no linear).");
            current->next_node_id = std::string(next);
            continue;
        }

        if (starts_with(line, "- ")) {
            // "- [LABEL] -> next [flag:key=bool]"
            std::string_view rest = trim(line.substr(2));
            if (rest.empty() || rest.front() != '[')
                throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                                        ": opcao malformada (esperado"
                                        " '- [LABEL] -> next').");
            const auto close = rest.find(']');
            if (close == std::string_view::npos)
                throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                                        ": opcao sem ']' de fechamento.");
            const std::string_view label = trim(rest.substr(1, close - 1));
            if (label.empty())
                throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                                        ": label da opcao vazio.");
            std::string_view tail = trim(rest.substr(close + 1));
            if (!starts_with(tail, "->"))
                throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                                        ": opcao sem '->' apos o label.");
            tail = trim(tail.substr(2));
            if (tail.empty())
                throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                                        ": opcao sem destino.");

            const auto sp = tail.find(' ');
            const std::string_view next_id =
                sp == std::string_view::npos ? tail : trim(tail.substr(0, sp));
            if (next_id.empty())
                throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                                        ": opcao sem destino.");

            DialogueOption opt;
            opt.label_key = std::string(label);
            opt.next_node_id = std::string(next_id);
            if (sp != std::string_view::npos) {
                const std::string_view suffix = trim(tail.substr(sp + 1));
                if (!suffix.empty()) {
                    if (!starts_with(suffix, "flag:"))
                        throw DialogueTextError(
                            "dialogo linha " + std::to_string(line_no) +
                            ": sufixo de opcao invalido (esperado"
                            " 'flag:chave=valor'): '" + std::string(suffix) + "'.");
                    opt.effect = parse_flag_effect(suffix.substr(5), line_no);
                }
            }
            current->options.push_back(std::move(opt));
            continue;
        }

        // Demais linhas: "chave: valor" (speaker/text/register/on_enter).
        const auto [key, val] = split_kv(line, line_no);
        if (val.empty())
            throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                                    ": '" + std::string(key) + "' sem valor.");
        if (key == "speaker") {
            current->speaker_id = std::string(val);
        } else if (key == "text") {
            current->text_key = std::string(val);
        } else if (key == "register") {
            current->register_override = parse_register(val, line_no);
        } else if (key == "on_enter") {
            current->on_enter = parse_flag_effect(val, line_no);
        } else {
            throw DialogueTextError("dialogo linha " + std::to_string(line_no) +
                                    ": diretiva de no desconhecida '" +
                                    std::string(key) + "'.");
        }
    }

    finalize_current();

    if (!has_dialogue_id)
        throw DialogueTextError("dialogo: #meta dialogue_id ausente.");
    if (!has_default_register)
        throw DialogueTextError("dialogo: #meta default_register ausente.");
    if (!has_entry)
        throw DialogueTextError("dialogo: #meta entry ausente.");

    graph.nodes = std::move(nodes);
    graph.validate();  // std::invalid_argument se estrutura incoerente
    return graph;
}

}  // namespace gus::domain::dialogue
