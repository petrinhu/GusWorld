// gus/domain/src/settings/system_settings_json.cpp
//
// Serializer + parser JSON proprio MINIMO do schema de SystemSettings
// (MENU-PAUSA-CONFIG-SOM). Ver header para o contrato. Travado por
// tests/system_settings_json_test.cpp (TEST-FIRST: roundtrip, defaults, corpus de
// malformados sem crash, forward-compat, clamp de volume).
//
// Parser recursive-descent minimo (o schema e um objeto flat de 3 campos - bem
// mais simples que o de controls_json.cpp, que tem arrays aninhados). Mesmo
// espirito: NUNCA lanca, toda falha vira SystemSettingsParseError.

#include "gus/domain/settings/system_settings_json.hpp"

#include <algorithm>  // std::clamp
#include <string>
#include <vector>

namespace gus::domain::settings {

namespace {

// ---- serializer -------------------------------------------------------------

void append_float(std::string& out, float v) {
    // Mesma receita de controls_json.cpp: std::to_string(float) da 6 casas fixas
    // (deterministico), aparadas de zeros a direita mantendo pelo menos ".0".
    std::string s = std::to_string(static_cast<double>(v));
    if (s.find('.') != std::string::npos) {
        std::size_t last = s.find_last_not_of('0');
        if (last != std::string::npos && s[last] == '.') last += 1;
        s.erase(last + 1);
    }
    out += s;
}

// ---- parser (recursive descent, NUNCA lanca) --------------------------------

// Valor JSON parseado numa arvore minima (so o necessario deste schema flat).
struct JsonValue;
struct JsonMember {
    std::string key;
    JsonValue* value = nullptr;
};

struct JsonValue {
    enum class Type { Null, Bool, Number, String, Object } type = Type::Null;
    bool b = false;
    double num = 0.0;
    std::string s;
    std::vector<std::pair<std::string, JsonValue>> obj;  // ordem de insercao
};

class JsonParser {
   public:
    explicit JsonParser(const std::string& src) : src_(src) {}

    bool parse(JsonValue& out) {
        skip_ws();
        if (pos_ >= src_.size()) {
            set_err(SystemSettingsParseError::Empty);
            return false;
        }
        if (!parse_value(out)) return false;
        skip_ws();
        if (pos_ != src_.size()) {
            set_err(SystemSettingsParseError::UnexpectedChar);
            return false;
        }
        return true;
    }

    SystemSettingsParseError error() const { return err_; }

   private:
    void set_err(SystemSettingsParseError e) {
        if (err_ == SystemSettingsParseError::None) err_ = e;
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

    bool parse_value(JsonValue& out) {
        skip_ws();
        if (pos_ >= src_.size()) {
            set_err(SystemSettingsParseError::UnexpectedEnd);
            return false;
        }
        const char c = peek();
        switch (c) {
            case '{': return parse_object(out);
            case '"': return parse_string(out);
            case 't':
            case 'f': return parse_bool(out);
            case 'n': return parse_null(out);
            default:
                if (c == '-' || (c >= '0' && c <= '9')) return parse_number(out);
                set_err(SystemSettingsParseError::UnexpectedChar);
                return false;
        }
    }

    bool parse_object(JsonValue& out) {
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
                set_err(SystemSettingsParseError::UnexpectedChar);
                return false;
            }
            JsonValue key_val;
            if (!parse_string(key_val)) return false;
            skip_ws();
            if (peek() != ':') {
                set_err(SystemSettingsParseError::UnexpectedChar);
                return false;
            }
            ++pos_;  // consume ':'
            JsonValue member_val;
            if (!parse_value(member_val)) return false;
            out.obj.emplace_back(key_val.s, std::move(member_val));
            skip_ws();
            const char n = peek();
            if (n == ',') {
                ++pos_;
                skip_ws();
                if (peek() == '}') {
                    set_err(SystemSettingsParseError::UnexpectedChar);
                    return false;
                }
                continue;
            }
            if (n == '}') {
                ++pos_;
                return true;
            }
            set_err(n == '\0' ? SystemSettingsParseError::UnexpectedEnd
                               : SystemSettingsParseError::UnexpectedChar);
            return false;
        }
    }

    bool parse_string(JsonValue& out) {
        out.type = JsonValue::Type::String;
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
                    set_err(SystemSettingsParseError::UnexpectedEnd);
                    return false;
                }
                const char e = src_[pos_++];
                switch (e) {
                    case '"': s.push_back('"'); break;
                    case '\\': s.push_back('\\'); break;
                    case 'n': s.push_back('\n'); break;
                    case 't': s.push_back('\t'); break;
                    default:
                        set_err(SystemSettingsParseError::UnexpectedChar);
                        return false;
                }
            } else {
                s.push_back(c);
            }
        }
        set_err(SystemSettingsParseError::UnexpectedEnd);
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
        set_err(SystemSettingsParseError::BadLiteral);
        return false;
    }

    bool parse_null(JsonValue& out) {
        if (src_.compare(pos_, 4, "null") == 0) {
            pos_ += 4;
            out.type = JsonValue::Type::Null;
            return true;
        }
        set_err(SystemSettingsParseError::BadLiteral);
        return false;
    }

    bool parse_number(JsonValue& out) {
        const std::size_t start = pos_;
        if (peek() == '-') ++pos_;
        bool any_digit = false;
        while (pos_ < src_.size() && src_[pos_] >= '0' && src_[pos_] <= '9') {
            ++pos_;
            any_digit = true;
        }
        if (pos_ < src_.size() && src_[pos_] == '.') {
            ++pos_;
            while (pos_ < src_.size() && src_[pos_] >= '0' && src_[pos_] <= '9') {
                ++pos_;
                any_digit = true;
            }
        }
        // Notacao cientifica (e/E) fora do escopo: rejeita graciosamente.
        if (pos_ < src_.size() && (src_[pos_] == 'e' || src_[pos_] == 'E')) {
            set_err(SystemSettingsParseError::BadNumber);
            return false;
        }
        if (!any_digit) {
            set_err(SystemSettingsParseError::BadNumber);
            return false;
        }
        try {
            out.type = JsonValue::Type::Number;
            out.num = std::stod(src_.substr(start, pos_ - start));
        } catch (...) {
            set_err(SystemSettingsParseError::BadNumber);
            return false;
        }
        return true;
    }

    const std::string& src_;
    std::size_t pos_ = 0;
    SystemSettingsParseError err_ = SystemSettingsParseError::None;
};

const JsonValue* member(const JsonValue& obj, const char* key) {
    if (obj.type != JsonValue::Type::Object) return nullptr;
    for (const auto& [k, v] : obj.obj)
        if (k == key) return &v;
    return nullptr;
}

}  // namespace

std::string serialize_system_settings_pretty(const SystemSettings& s) {
    std::string out;
    out += "{\n";
    out += "  \"schema_version\": ";
    out += std::to_string(s.schema_version);
    out += ",\n";
    out += "  \"music_volume\": ";
    append_float(out, s.music_volume);
    out += ",\n";
    out += "  \"sfx_volume\": ";
    append_float(out, s.sfx_volume);
    out += "\n";
    out += "}";
    return out;
}

SystemSettingsParseResult parse_system_settings(const std::string& json) {
    SystemSettingsParseResult result;

    JsonParser parser(json);
    JsonValue root;
    if (!parser.parse(root)) {
        result.error = parser.error();
        return result;
    }
    if (root.type != JsonValue::Type::Object) {
        result.error = SystemSettingsParseError::NotAnObject;
        return result;
    }

    if (const JsonValue* v = member(root, "schema_version")) {
        if (v->type != JsonValue::Type::Number) {
            result.error = SystemSettingsParseError::TypeMismatch;
            return result;
        }
        result.settings.schema_version = static_cast<int>(v->num);
    }

    if (const JsonValue* v = member(root, "music_volume")) {
        if (v->type != JsonValue::Type::Number) {
            result.error = SystemSettingsParseError::TypeMismatch;
            return result;
        }
        result.settings.music_volume =
            std::clamp(static_cast<float>(v->num), 0.0f, 1.0f);
    }

    if (const JsonValue* v = member(root, "sfx_volume")) {
        if (v->type != JsonValue::Type::Number) {
            result.error = SystemSettingsParseError::TypeMismatch;
            return result;
        }
        result.settings.sfx_volume =
            std::clamp(static_cast<float>(v->num), 0.0f, 1.0f);
    }

    result.error = SystemSettingsParseError::None;
    return result;
}

}  // namespace gus::domain::settings
