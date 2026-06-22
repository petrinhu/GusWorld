// gus/domain/src/input/controls_json.cpp
//
// Serializer + parser JSON proprio MINIMO do schema de controles (ADR-007). POCO
// puro, ZERO Qt, ZERO I/O, ZERO dep externa. Ver header para o contrato. Travado por
// tests/controls_json_test.cpp (roundtrip canonico/pretty, ordem do ActionRegistry,
// corpus de malformados sem crash, forward-compat).
//
// O parser e um recursive-descent SO do necessario para este schema. Robusto: NUNCA
// lanca; toda falha vira ControlsParseError. Limite de profundidade (kMaxDepth) como
// defesa contra recursao patologica (entrada de usuario).

#include "gus/domain/input/controls_json.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include "gus/domain/input/action_registry.hpp"

namespace gus::domain::input {

namespace {

// Profundidade maxima de aninhamento. O schema real tem ~3 niveis (obj/actions[]/
// binding-obj); um teto folgado protege contra entrada patologica sem custo real.
constexpr int kMaxDepth = 32;

// ---- serializer ------------------------------------------------------------

// Escape JSON basico do schema (\" \\ \n \t). Os action_name canonicos sao
// [a-z0-9_], mas escapamos defensivamente qualquer string do schema.
void append_escaped(std::string& out, const std::string& s) {
    out.push_back('"');
    for (const char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            default: out.push_back(c); break;
        }
    }
    out.push_back('"');
}

void append_bool(std::string& out, bool b) { out += b ? "true" : "false"; }

void append_int(std::string& out, long long v) { out += std::to_string(v); }

// Float deterministico e roundtrippavel. O schema usa deadzone (0..1) e axis_value
// (-1..1); valores "redondos" comuns. Usamos uma representacao curta e estavel.
void append_float(std::string& out, float v) {
    // std::to_string(float) da 6 casas fixas: deterministico e suficiente para os
    // valores do schema (multiplos de 0.05 etc.). Evita notacao cientifica (fora do
    // escopo do parser).
    std::string s = std::to_string(static_cast<double>(v));
    // Apara zeros a direita para uma forma canonica estavel (mas mantem ".0").
    if (s.find('.') != std::string::npos) {
        std::size_t last = s.find_last_not_of('0');
        if (last != std::string::npos && s[last] == '.') last += 1;  // mantem 1 zero
        s.erase(last + 1);
    }
    out += s;
}

// Indentacao da forma pretty.
void indent(std::string& out, int level, bool pretty) {
    if (!pretty) return;
    for (int i = 0; i < level; ++i) out += "  ";
}

void newline(std::string& out, bool pretty) {
    if (pretty) out.push_back('\n');
}

void colon(std::string& out, bool pretty) { out += pretty ? ": " : ":"; }

// Emite as actions na ORDEM do ActionRegistry (determinismo do canonico). Cada
// action do config e indexada por nome; emitimos na ordem do registry as que
// existem no config. Actions do config cujo nome nao esta no registry sao
// preservadas no fim (defensivo; o parse ja descarta nomes desconhecidos, mas a
// serializacao nao deve perder dado se chamada com um config "exotico").
const ActionBindings* find_in_config(const InputRemapConfig& cfg,
                                     const std::string& name) {
    for (const auto& a : cfg.actions)
        if (a.action_name == name) return &a;
    return nullptr;
}

void serialize_binding_key(std::string& out, const KeyBinding& k, int level,
                           bool pretty) {
    out.push_back('{');
    newline(out, pretty);
    indent(out, level + 1, pretty);
    out += "\"keycode\"";
    colon(out, pretty);
    append_int(out, k.keycode);
    out.push_back(',');
    newline(out, pretty);
    indent(out, level + 1, pretty);
    out += "\"ctrl\"";
    colon(out, pretty);
    append_bool(out, k.ctrl_pressed);
    out.push_back(',');
    newline(out, pretty);
    indent(out, level + 1, pretty);
    out += "\"shift\"";
    colon(out, pretty);
    append_bool(out, k.shift_pressed);
    out.push_back(',');
    newline(out, pretty);
    indent(out, level + 1, pretty);
    out += "\"alt\"";
    colon(out, pretty);
    append_bool(out, k.alt_pressed);
    newline(out, pretty);
    indent(out, level, pretty);
    out.push_back('}');
}

void serialize_index_obj(std::string& out, const char* key, int value, int level,
                         bool pretty) {
    out.push_back('{');
    newline(out, pretty);
    indent(out, level + 1, pretty);
    out.push_back('"');
    out += key;
    out.push_back('"');
    colon(out, pretty);
    append_int(out, value);
    newline(out, pretty);
    indent(out, level, pretty);
    out.push_back('}');
}

void serialize_axis(std::string& out, const GamepadAxisBinding& ax, int level,
                    bool pretty) {
    out.push_back('{');
    newline(out, pretty);
    indent(out, level + 1, pretty);
    out += "\"axis\"";
    colon(out, pretty);
    append_int(out, ax.axis);
    out.push_back(',');
    newline(out, pretty);
    indent(out, level + 1, pretty);
    out += "\"axis_value\"";
    colon(out, pretty);
    append_float(out, ax.axis_value);
    newline(out, pretty);
    indent(out, level, pretty);
    out.push_back('}');
}

// Emite um array de itens com um callback de item. Mantem a ordem do vetor.
template <typename Vec, typename EmitItem>
void serialize_array(std::string& out, const Vec& items, int level, bool pretty,
                     EmitItem emit_item) {
    if (items.empty()) {
        out += "[]";
        return;
    }
    out.push_back('[');
    newline(out, pretty);
    for (std::size_t i = 0; i < items.size(); ++i) {
        indent(out, level + 1, pretty);
        emit_item(items[i], level + 1);
        if (i + 1 < items.size()) out.push_back(',');
        newline(out, pretty);
    }
    indent(out, level, pretty);
    out.push_back(']');
}

void serialize_action(std::string& out, const ActionBindings& a, int level,
                      bool pretty) {
    out.push_back('{');
    newline(out, pretty);

    indent(out, level + 1, pretty);
    out += "\"action_name\"";
    colon(out, pretty);
    append_escaped(out, a.action_name);
    out.push_back(',');
    newline(out, pretty);

    indent(out, level + 1, pretty);
    out += "\"deadzone\"";
    colon(out, pretty);
    append_float(out, a.deadzone);
    out.push_back(',');
    newline(out, pretty);

    indent(out, level + 1, pretty);
    out += "\"keys\"";
    colon(out, pretty);
    serialize_array(out, a.keys, level + 1, pretty,
                    [&](const KeyBinding& k, int lv) {
                        serialize_binding_key(out, k, lv, pretty);
                    });
    out.push_back(',');
    newline(out, pretty);

    indent(out, level + 1, pretty);
    out += "\"gamepad_buttons\"";
    colon(out, pretty);
    serialize_array(out, a.gamepad_buttons, level + 1, pretty,
                    [&](const GamepadButtonBinding& b, int lv) {
                        serialize_index_obj(out, "button_index", b.button_index, lv,
                                            pretty);
                    });
    out.push_back(',');
    newline(out, pretty);

    indent(out, level + 1, pretty);
    out += "\"mouse_buttons\"";
    colon(out, pretty);
    serialize_array(out, a.mouse_buttons, level + 1, pretty,
                    [&](const MouseButtonBinding& b, int lv) {
                        serialize_index_obj(out, "button_index", b.button_index, lv,
                                            pretty);
                    });
    out.push_back(',');
    newline(out, pretty);

    indent(out, level + 1, pretty);
    out += "\"gamepad_axes\"";
    colon(out, pretty);
    serialize_array(out, a.gamepad_axes, level + 1, pretty,
                    [&](const GamepadAxisBinding& ax, int lv) {
                        serialize_axis(out, ax, lv, pretty);
                    });
    newline(out, pretty);

    indent(out, level, pretty);
    out.push_back('}');
}

std::string serialize_impl(const InputRemapConfig& cfg, bool pretty) {
    // Ordena as actions a emitir pela ordem do ActionRegistry (canonico). Inclui no
    // fim, defensivamente, qualquer action do config fora do registry (sem perder
    // dado), embora o parse normal nunca produza essas.
    std::vector<const ActionBindings*> ordered;
    ordered.reserve(cfg.actions.size());
    for (const auto& def : ActionRegistry::actions()) {
        if (const auto* a = find_in_config(cfg, def.action_name)) ordered.push_back(a);
    }
    for (const auto& a : cfg.actions) {
        if (ActionRegistry::get_by_name(a.action_name) == nullptr)
            ordered.push_back(&a);
    }

    std::string out;
    out.push_back('{');
    newline(out, pretty);

    indent(out, 1, pretty);
    out += "\"config_version\"";
    colon(out, pretty);
    append_int(out, cfg.config_version);
    out.push_back(',');
    newline(out, pretty);

    indent(out, 1, pretty);
    out += "\"actions\"";
    colon(out, pretty);
    serialize_array(out, ordered, 1, pretty,
                    [&](const ActionBindings* a, int lv) {
                        serialize_action(out, *a, lv, pretty);
                    });
    newline(out, pretty);

    out.push_back('}');
    return out;
}

// ---- parser (recursive descent, NUNCA lanca) -------------------------------

// Valor JSON parseado em uma arvore minima (so o necessario do schema). Mantemos
// uma representacao leve para nao acoplar o parser ao struct alvo (separa "ler
// JSON" de "mapear no InputRemapConfig").
struct JsonValue;
using JsonArray = std::vector<JsonValue>;
struct JsonMember;  // par chave->valor de objeto

struct JsonValue {
    enum class Type { Null, Bool, Int, Double, String, Array, Object } type =
        Type::Null;
    bool b = false;
    long long i = 0;
    double d = 0.0;
    std::string s;
    JsonArray arr;
    std::vector<JsonMember> obj;  // ordem de insercao preservada
};

struct JsonMember {
    std::string key;
    JsonValue value;
};

// Parser de tokens com erro por valor. Cada metodo de parse devolve false em erro e
// preenche err_.
class JsonParser {
   public:
    explicit JsonParser(const std::string& src) : src_(src) {}

    bool parse(JsonValue& out) {
        skip_ws();
        if (pos_ >= src_.size()) {
            set_err(ControlsParseError::Empty);
            return false;
        }
        if (!parse_value(out, 0)) return false;
        skip_ws();
        if (pos_ != src_.size()) {
            // Lixo apos o valor raiz.
            set_err(ControlsParseError::UnexpectedChar);
            return false;
        }
        return true;
    }

    ControlsParseError error() const { return err_; }
    std::size_t error_offset() const { return err_pos_; }

   private:
    void set_err(ControlsParseError e) {
        if (err_ == ControlsParseError::None) {  // preserva o primeiro erro
            err_ = e;
            err_pos_ = pos_;
        }
    }

    void skip_ws() {
        while (pos_ < src_.size()) {
            const char c = src_[pos_];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
                ++pos_;
            else
                break;
        }
    }

    char peek() const { return pos_ < src_.size() ? src_[pos_] : '\0'; }

    bool parse_value(JsonValue& out, int depth) {
        if (depth > kMaxDepth) {
            set_err(ControlsParseError::TooDeep);
            return false;
        }
        skip_ws();
        if (pos_ >= src_.size()) {
            set_err(ControlsParseError::UnexpectedEnd);
            return false;
        }
        const char c = peek();
        switch (c) {
            case '{': return parse_object(out, depth);
            case '[': return parse_array(out, depth);
            case '"': return parse_string_value(out);
            case 't':
            case 'f': return parse_bool(out);
            case 'n': return parse_null(out);
            default:
                if (c == '-' || (c >= '0' && c <= '9')) return parse_number(out);
                set_err(ControlsParseError::UnexpectedChar);
                return false;
        }
    }

    bool parse_object(JsonValue& out, int depth) {
        out.type = JsonValue::Type::Object;
        ++pos_;  // consume '{'
        skip_ws();
        if (peek() == '}') {
            ++pos_;
            return true;
        }
        while (true) {
            skip_ws();
            if (peek() != '"') {
                set_err(ControlsParseError::UnexpectedChar);
                return false;
            }
            JsonValue key_val;
            if (!parse_string_value(key_val)) return false;
            skip_ws();
            if (peek() != ':') {
                set_err(ControlsParseError::UnexpectedChar);
                return false;
            }
            ++pos_;  // consume ':'
            JsonMember member;
            member.key = key_val.s;
            if (!parse_value(member.value, depth + 1)) return false;
            out.obj.push_back(std::move(member));
            skip_ws();
            const char n = peek();
            if (n == ',') {
                ++pos_;
                skip_ws();
                if (peek() == '}') {  // virgula sobrando antes do fim
                    set_err(ControlsParseError::UnexpectedChar);
                    return false;
                }
                continue;
            }
            if (n == '}') {
                ++pos_;
                return true;
            }
            set_err(n == '\0' ? ControlsParseError::UnexpectedEnd
                              : ControlsParseError::UnexpectedChar);
            return false;
        }
    }

    bool parse_array(JsonValue& out, int depth) {
        out.type = JsonValue::Type::Array;
        ++pos_;  // consume '['
        skip_ws();
        if (peek() == ']') {
            ++pos_;
            return true;
        }
        while (true) {
            JsonValue item;
            if (!parse_value(item, depth + 1)) return false;
            out.arr.push_back(std::move(item));
            skip_ws();
            const char n = peek();
            if (n == ',') {
                ++pos_;
                skip_ws();
                if (peek() == ']') {
                    set_err(ControlsParseError::UnexpectedChar);
                    return false;
                }
                continue;
            }
            if (n == ']') {
                ++pos_;
                return true;
            }
            set_err(n == '\0' ? ControlsParseError::UnexpectedEnd
                              : ControlsParseError::UnexpectedChar);
            return false;
        }
    }

    bool parse_string_value(JsonValue& out) {
        out.type = JsonValue::Type::String;
        if (peek() != '"') {
            set_err(ControlsParseError::UnexpectedChar);
            return false;
        }
        ++pos_;  // consume opening quote
        std::string s;
        while (pos_ < src_.size()) {
            const char c = src_[pos_++];
            if (c == '"') {
                out.s = std::move(s);
                return true;
            }
            if (c == '\\') {
                if (pos_ >= src_.size()) {
                    set_err(ControlsParseError::UnexpectedEnd);
                    return false;
                }
                const char e = src_[pos_++];
                switch (e) {
                    case '"': s.push_back('"'); break;
                    case '\\': s.push_back('\\'); break;
                    case '/': s.push_back('/'); break;
                    case 'n': s.push_back('\n'); break;
                    case 't': s.push_back('\t'); break;
                    case 'r': s.push_back('\r'); break;
                    case 'b': s.push_back('\b'); break;
                    case 'f': s.push_back('\f'); break;
                    default:
                        // \uXXXX e outros escapes fora do escopo: erro gracioso.
                        set_err(ControlsParseError::UnexpectedChar);
                        return false;
                }
            } else {
                s.push_back(c);
            }
        }
        set_err(ControlsParseError::UnexpectedEnd);  // string nao fechada
        return false;
    }

    bool parse_bool(JsonValue& out) {
        if (src_.compare(pos_, 4, "true") == 0) {
            pos_ += 4;
            out.type = JsonValue::Type::Bool;
            out.b = true;
            return true;
        }
        if (src_.compare(pos_, 5, "false") == 0) {
            pos_ += 5;
            out.type = JsonValue::Type::Bool;
            out.b = false;
            return true;
        }
        set_err(ControlsParseError::BadLiteral);
        return false;
    }

    bool parse_null(JsonValue& out) {
        if (src_.compare(pos_, 4, "null") == 0) {
            pos_ += 4;
            out.type = JsonValue::Type::Null;
            return true;
        }
        set_err(ControlsParseError::BadLiteral);
        return false;
    }

    bool parse_number(JsonValue& out) {
        const std::size_t start = pos_;
        bool is_double = false;
        if (peek() == '-') ++pos_;
        bool any_digit = false;
        while (pos_ < src_.size() && src_[pos_] >= '0' && src_[pos_] <= '9') {
            ++pos_;
            any_digit = true;
        }
        if (pos_ < src_.size() && src_[pos_] == '.') {
            is_double = true;
            ++pos_;
            while (pos_ < src_.size() && src_[pos_] >= '0' && src_[pos_] <= '9') {
                ++pos_;
                any_digit = true;
            }
        }
        // Notacao cientifica (e/E) esta FORA do escopo: rejeita graciosamente.
        if (pos_ < src_.size() && (src_[pos_] == 'e' || src_[pos_] == 'E')) {
            set_err(ControlsParseError::BadNumber);
            return false;
        }
        if (!any_digit) {
            set_err(ControlsParseError::BadNumber);
            return false;
        }
        const std::string tok = src_.substr(start, pos_ - start);
        if (is_double) {
            try {
                out.type = JsonValue::Type::Double;
                out.d = std::stod(tok);
            } catch (...) {
                set_err(ControlsParseError::BadNumber);
                return false;
            }
        } else {
            try {
                out.type = JsonValue::Type::Int;
                out.i = std::stoll(tok);  // 64-bit (keycode pode exceder 32 bits)
            } catch (...) {
                // overflow (numero gigante) = erro gracioso, nunca crash.
                set_err(ControlsParseError::BadNumber);
                return false;
            }
        }
        return true;
    }

    const std::string& src_;
    std::size_t pos_ = 0;
    ControlsParseError err_ = ControlsParseError::None;
    std::size_t err_pos_ = 0;
};

// ---- mapeamento JsonValue -> InputRemapConfig (forward-compat) -------------

const JsonValue* member(const JsonValue& obj, const char* key) {
    if (obj.type != JsonValue::Type::Object) return nullptr;
    for (const auto& m : obj.obj)
        if (m.key == key) return &m.value;
    return nullptr;
}

bool as_int(const JsonValue* v, long long& out) {
    if (v == nullptr) return false;
    if (v->type == JsonValue::Type::Int) {
        out = v->i;
        return true;
    }
    if (v->type == JsonValue::Type::Double) {  // tolera "1.0" onde se espera int
        out = static_cast<long long>(v->d);
        return true;
    }
    return false;
}

bool as_float(const JsonValue* v, float& out) {
    if (v == nullptr) return false;
    if (v->type == JsonValue::Type::Double) {
        out = static_cast<float>(v->d);
        return true;
    }
    if (v->type == JsonValue::Type::Int) {
        out = static_cast<float>(v->i);
        return true;
    }
    return false;
}

bool as_bool(const JsonValue* v, bool& out) {
    if (v == nullptr || v->type != JsonValue::Type::Bool) return false;
    out = v->b;
    return true;
}

void map_keys(const JsonValue& arr, ActionBindings& ab) {
    if (arr.type != JsonValue::Type::Array) return;
    for (const auto& it : arr.arr) {
        if (it.type != JsonValue::Type::Object) continue;
        KeyBinding kb;
        long long code = 0;
        if (as_int(member(it, "keycode"), code)) kb.keycode = code;
        bool flag = false;
        if (as_bool(member(it, "ctrl"), flag)) kb.ctrl_pressed = flag;
        if (as_bool(member(it, "shift"), flag)) kb.shift_pressed = flag;
        if (as_bool(member(it, "alt"), flag)) kb.alt_pressed = flag;
        ab.keys.push_back(kb);
    }
}

void map_index_list(const JsonValue& arr, std::vector<int>& out) {
    if (arr.type != JsonValue::Type::Array) return;
    for (const auto& it : arr.arr) {
        if (it.type != JsonValue::Type::Object) continue;
        long long idx = 0;
        if (as_int(member(it, "button_index"), idx))
            out.push_back(static_cast<int>(idx));
    }
}

void map_axes(const JsonValue& arr, ActionBindings& ab) {
    if (arr.type != JsonValue::Type::Array) return;
    for (const auto& it : arr.arr) {
        if (it.type != JsonValue::Type::Object) continue;
        GamepadAxisBinding ax;
        long long axis = 0;
        if (as_int(member(it, "axis"), axis)) ax.axis = static_cast<int>(axis);
        float val = 0.0f;
        if (as_float(member(it, "axis_value"), val)) ax.axis_value = val;
        ab.gamepad_axes.push_back(ax);
    }
}

}  // namespace

std::string serialize_controls_canonical(const InputRemapConfig& cfg) {
    return serialize_impl(cfg, /*pretty=*/false);
}

std::string serialize_controls_pretty(const InputRemapConfig& cfg) {
    return serialize_impl(cfg, /*pretty=*/true);
}

ControlsParseResult parse_controls(const std::string& json) {
    ControlsParseResult result;

    JsonParser parser(json);
    JsonValue root;
    if (!parser.parse(root)) {
        result.error = parser.error();
        result.error_offset = parser.error_offset();
        return result;
    }
    if (root.type != JsonValue::Type::Object) {
        result.error = ControlsParseError::NotAnObject;
        return result;
    }

    // config_version (default 1 se ausente).
    if (const JsonValue* cv = member(root, "config_version")) {
        long long v = 1;
        if (as_int(cv, v))
            result.config.config_version = static_cast<int>(v);
        else {
            result.error = ControlsParseError::TypeMismatch;
            return result;
        }
    }

    // actions (opcional; se presente DEVE ser array).
    if (const JsonValue* acts = member(root, "actions")) {
        if (acts->type != JsonValue::Type::Array) {
            result.error = ControlsParseError::TypeMismatch;
            return result;
        }
        for (const auto& a : acts->arr) {
            if (a.type != JsonValue::Type::Object) continue;  // forward-compat
            const JsonValue* name_v = member(a, "action_name");
            if (name_v == nullptr || name_v->type != JsonValue::Type::String)
                continue;  // sem nome utilizavel: ignora
            // Forward-compat: action_name desconhecido = ignora (nao quebra).
            if (ActionRegistry::get_by_name(name_v->s) == nullptr) continue;

            ActionBindings ab;
            ab.action_name = name_v->s;
            float dz = 0.5f;
            if (as_float(member(a, "deadzone"), dz)) ab.deadzone = dz;  // ausente=0.5

            if (const JsonValue* keys = member(a, "keys")) map_keys(*keys, ab);

            std::vector<int> btns;
            if (const JsonValue* gb = member(a, "gamepad_buttons"))
                map_index_list(*gb, btns);
            for (const int idx : btns)
                ab.gamepad_buttons.push_back(GamepadButtonBinding{idx});

            std::vector<int> mbtns;
            if (const JsonValue* mb = member(a, "mouse_buttons"))
                map_index_list(*mb, mbtns);
            for (const int idx : mbtns)
                ab.mouse_buttons.push_back(MouseButtonBinding{idx});

            if (const JsonValue* ax = member(a, "gamepad_axes")) map_axes(*ax, ab);

            result.config.actions.push_back(std::move(ab));
        }
    }

    result.error = ControlsParseError::None;
    return result;
}

}  // namespace gus::domain::input
