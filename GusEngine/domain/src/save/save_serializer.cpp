// gus/domain/src/save/save_serializer.cpp
//
// Serializer binario proprio + HMAC-SHA256 (core/) do estado de save. Ver header
// para envelope e layout. POCO puro, ZERO Qt. HMAC de gus::core::crypto (proprio,
// validado contra FIPS/RFC). ADR-006.
//
// Este .cpp tambem define os helpers de FIXTURE de migracao declarados em
// save_migrators.hpp (serialize_save_v1 / make_v1_payload): eles dependem do MESMO
// codec binario (layout V1 = layout V2 sem character_states), entao moram junto do
// codec para nao duplica-lo (DRY). A LOGICA de migracao (chain de structs) fica em
// save_migrators.cpp.

#include "gus/domain/save/save_serializer.hpp"

#include <array>
#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "gus/core/crypto/hmac_sha256.hpp"
#include "gus/core/crypto/key_derivation.hpp"
#include "gus/domain/save/save_migrators.hpp"

namespace gus::domain::save {

namespace {

namespace crypto = gus::core::crypto;

// MAGIC "GDS2" (GusDragon Save). O sufixo do magic NAO e a versao do schema (a
// versao vive no payload, u32 inicial); distingue de templates ("GDT1").
constexpr std::array<std::uint8_t, 4> kMagic = {'G', 'D', 'S', '2'};
constexpr std::size_t kMagicLen = 4;
constexpr std::size_t kLengthFieldLen = 4;
constexpr std::size_t kHmacLen = crypto::kSha256DigestSize;       // 32
constexpr std::size_t kHeaderLen = kMagicLen + kLengthFieldLen;   // 8

// Chave de integridade DERIVADA (ADR-006 T2.2): em vez de embutida CRUA, a chave do
// HMAC do save e derivada de um segredo-base + um contexto ("save-hmac"). Transparente
// ao layout (so muda o VALOR do HMAC). NAO e sigilo (o segredo-base vive no binario,
// extraivel por decompile): e soberania da ORIGEM da chave (nao aparece literal). O
// contexto da separacao de dominio (save != templates). PROPRIA do SAVE.
const std::vector<std::uint8_t>& embedded_key() {
    static const std::vector<std::uint8_t> k = [] {
        static const std::string base = "gusengine-cpp-2026-integrity-base-secret";
        static const std::string ctx = "save-hmac";
        return crypto::derive_key(
            reinterpret_cast<const std::uint8_t*>(base.data()), base.size(),
            reinterpret_cast<const std::uint8_t*>(ctx.data()), ctx.size());
    }();
    return k;
}

// ---- writer binario (little-endian) ---------------------------------------

void put_u32(std::vector<std::uint8_t>& out, std::uint32_t v) {
    out.push_back(static_cast<std::uint8_t>(v & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((v >> 8) & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((v >> 16) & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((v >> 24) & 0xFFu));
}

void put_i32(std::vector<std::uint8_t>& out, int v) {
    put_u32(out, static_cast<std::uint32_t>(v));
}

void put_u64(std::vector<std::uint8_t>& out, std::uint64_t v) {
    for (int i = 0; i < 8; ++i)
        out.push_back(static_cast<std::uint8_t>((v >> (8 * i)) & 0xFFu));
}

void put_i64(std::vector<std::uint8_t>& out, std::int64_t v) {
    put_u64(out, static_cast<std::uint64_t>(v));
}

void put_f64(std::vector<std::uint8_t>& out, double v) {
    std::uint64_t bits = 0;
    std::memcpy(&bits, &v, sizeof(bits));
    put_u64(out, bits);
}

// f32 (V4: deadzone / axis_value). Grava o bit-pattern IEEE-754 de 32 bits LE
// (roundtrip fiel, mesmo criterio do f64).
void put_f32(std::vector<std::uint8_t>& out, float v) {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &v, sizeof(bits));
    put_u32(out, bits);
}

void put_string(std::vector<std::uint8_t>& out, const std::string& s) {
    put_u32(out, static_cast<std::uint32_t>(s.size()));
    out.insert(out.end(), s.begin(), s.end());
}

void put_string_list(std::vector<std::uint8_t>& out,
                     const std::vector<std::string>& list) {
    put_u32(out, static_cast<std::uint32_t>(list.size()));
    for (const auto& s : list) put_string(out, s);
}

void put_vec3(std::vector<std::uint8_t>& out, const Vec3& v) {
    put_f64(out, v.x);
    put_f64(out, v.y);
    put_f64(out, v.z);
}

template <typename Map, typename PutValue>
void put_map(std::vector<std::uint8_t>& out, const Map& m, PutValue put_value) {
    // std::map itera em ordem de chave: serializacao deterministica (selo estavel).
    put_u32(out, static_cast<std::uint32_t>(m.size()));
    for (const auto& [k, v] : m) {
        put_string(out, k);
        put_value(out, v);
    }
}

// ---- reader binario (little-endian), com bounds-check ----------------------

class Reader {
   public:
    explicit Reader(const std::vector<std::uint8_t>& buf) : buf_(buf) {}

    std::uint8_t read_u8() {
        require(1, "u8");
        return buf_[pos_++];
    }

    std::uint32_t read_u32() {
        require(4, "u32");
        const std::uint32_t v = static_cast<std::uint32_t>(buf_[pos_]) |
                                (static_cast<std::uint32_t>(buf_[pos_ + 1]) << 8) |
                                (static_cast<std::uint32_t>(buf_[pos_ + 2]) << 16) |
                                (static_cast<std::uint32_t>(buf_[pos_ + 3]) << 24);
        pos_ += 4;
        return v;
    }

    int read_i32() { return static_cast<int>(read_u32()); }

    std::uint64_t read_u64() {
        require(8, "u64");
        std::uint64_t v = 0;
        for (int i = 0; i < 8; ++i)
            v |= static_cast<std::uint64_t>(
                     buf_[pos_ + static_cast<std::size_t>(i)])
                 << (8 * i);
        pos_ += 8;
        return v;
    }

    std::int64_t read_i64() { return static_cast<std::int64_t>(read_u64()); }

    double read_f64() {
        const std::uint64_t bits = read_u64();
        double v = 0.0;
        std::memcpy(&v, &bits, sizeof(v));
        return v;
    }

    float read_f32() {
        const std::uint32_t bits = read_u32();
        float v = 0.0f;
        std::memcpy(&v, &bits, sizeof(v));
        return v;
    }

    std::string read_string() {
        const std::uint32_t len = read_u32();
        require(len, "string");
        std::string s(buf_.begin() + static_cast<std::ptrdiff_t>(pos_),
                      buf_.begin() + static_cast<std::ptrdiff_t>(pos_ + len));
        pos_ += len;
        return s;
    }

    std::vector<std::string> read_string_list() {
        // IMP-01 (CWE-789): cada string custa no minimo 4 bytes (o u32 de length).
        // Rejeita count implausivel ANTES de reserve, para um payload selado mas
        // mentiroso nao pedir dezenas de GiB e crashar com bad_alloc/length_error.
        const std::uint32_t count = bounded_count(read_u32(), 4, "string-list");
        std::vector<std::string> list;
        list.reserve(count);
        for (std::uint32_t i = 0; i < count; ++i) list.push_back(read_string());
        return list;
    }

    Vec3 read_vec3() {
        Vec3 v;
        v.x = read_f64();
        v.y = read_f64();
        v.z = read_f64();
        return v;
    }

    void expect_end() const {
        if (pos_ != buf_.size())
            throw SaveCorruptError(
                "Payload de save tem bytes extras apos os campos esperados.");
    }

    // IMP-01 (CWE-789/CWE-130): maximo de elementos que CABEM no resto do buffer,
    // dado o tamanho minimo de cada um. Rejeita count externo absurdo ANTES de
    // reserve/loop, para um payload selado-mas-mentiroso (atacante tem a chave por
    // design) nao alocar dezenas de GiB. Erro TIPADO do decoder, nao bad_alloc.
    std::uint32_t bounded_count(std::uint32_t count, std::size_t min_elem_bytes,
                                const char* what) const {
        if (min_elem_bytes != 0 &&
            static_cast<std::uint64_t>(count) * min_elem_bytes > remaining())
            throw SaveCorruptError(std::string("Contagem implausivel em ") + what +
                                   " (excede bytes restantes do payload).");
        return count;
    }

   private:
    std::size_t remaining() const { return buf_.size() - pos_; }

    void require(std::size_t n, const char* what) const {
        if (pos_ + n > buf_.size())
            throw SaveCorruptError(
                std::string("Payload de save truncado ao ler ") + what + ".");
    }

    const std::vector<std::uint8_t>& buf_;
    std::size_t pos_ = 0;
};

// ---- campos COMUNS a V1 e V2 (tudo menos character_states) -----------------

void write_common_payload(std::vector<std::uint8_t>& payload, const SaveData& s) {
    put_i64(payload, s.timestamp_ms);
    put_f64(payload, s.playtime_seconds);
    put_string(payload, s.current_scene_path);
    put_vec3(payload, s.player_position);
    put_vec3(payload, s.player_rotation);
    put_string_list(payload, s.party_roster);
    put_string_list(payload, s.party_active);
    put_map(payload, s.flags, [](std::vector<std::uint8_t>& o, bool v) {
        o.push_back(v ? 1u : 0u);
    });
    put_map(payload, s.inventory,
            [](std::vector<std::uint8_t>& o, int v) { put_i32(o, v); });
    put_map(payload, s.quest_progress,
            [](std::vector<std::uint8_t>& o, int v) { put_i32(o, v); });
    put_map(payload, s.relations,
            [](std::vector<std::uint8_t>& o, int v) { put_i32(o, v); });
}

void read_common_payload(Reader& r, SaveData& s) {
    s.timestamp_ms = r.read_i64();
    s.playtime_seconds = r.read_f64();
    s.current_scene_path = r.read_string();
    s.player_position = r.read_vec3();
    s.player_rotation = r.read_vec3();
    s.party_roster = r.read_string_list();
    s.party_active = r.read_string_list();

    {  // flags: bool = 1 byte
        const std::uint32_t n = r.read_u32();
        for (std::uint32_t i = 0; i < n; ++i) {
            const std::string k = r.read_string();
            s.flags[k] = (r.read_u8() != 0);
        }
    }
    auto read_int_map = [&](std::map<std::string, int>& m) {
        const std::uint32_t n = r.read_u32();
        for (std::uint32_t i = 0; i < n; ++i) {
            const std::string k = r.read_string();
            m[k] = r.read_i32();
        }
    };
    read_int_map(s.inventory);
    read_int_map(s.quest_progress);
    read_int_map(s.relations);
}

void write_character_states(std::vector<std::uint8_t>& payload, const SaveData& s) {
    put_map(payload, s.character_states,
            [](std::vector<std::uint8_t>& o, const CharacterSaveState& st) {
                put_i32(o, st.current_hp);
                put_i32(o, st.xp);
                put_string_list(o, st.deck);
            });
}

void read_character_states(Reader& r, SaveData& s) {
    const std::uint32_t n = r.read_u32();
    for (std::uint32_t i = 0; i < n; ++i) {
        const std::string id = r.read_string();
        CharacterSaveState st;
        st.current_hp = r.read_i32();
        st.xp = r.read_i32();
        st.deck = r.read_string_list();
        s.character_states.emplace(id, std::move(st));
    }
}

// V3: enemy_knowledge = map<enemy_type_id, kills>. Mesmo codec dos demais int-maps;
// std::map garante ordem de chave (selo deterministico).
void write_enemy_knowledge(std::vector<std::uint8_t>& payload, const SaveData& s) {
    put_map(payload, s.enemy_knowledge,
            [](std::vector<std::uint8_t>& o, int v) { put_i32(o, v); });
}

void read_enemy_knowledge(Reader& r, SaveData& s) {
    const std::uint32_t n = r.read_u32();
    for (std::uint32_t i = 0; i < n; ++i) {
        const std::string enemy_type_id = r.read_string();
        s.enemy_knowledge[enemy_type_id] = r.read_i32();
    }
}

// V4 (ADR-007): input_remap_backup. As actions sao gravadas na ORDEM do vetor (que o
// caller monta na ordem do ActionRegistry: determinismo do selo). Layout fiel ao
// ADR-007 item 3.
void write_input_remap_backup(std::vector<std::uint8_t>& payload, const SaveData& s) {
    const auto& cfg = s.input_remap_backup;
    put_u32(payload, static_cast<std::uint32_t>(cfg.config_version));
    put_u32(payload, static_cast<std::uint32_t>(cfg.actions.size()));
    for (const auto& a : cfg.actions) {
        put_string(payload, a.action_name);
        put_f32(payload, a.deadzone);
        put_u32(payload, static_cast<std::uint32_t>(a.keys.size()));
        for (const auto& k : a.keys) {
            put_i64(payload, k.keycode);
            payload.push_back(k.ctrl_pressed ? 1u : 0u);
            payload.push_back(k.shift_pressed ? 1u : 0u);
            payload.push_back(k.alt_pressed ? 1u : 0u);
        }
        put_u32(payload, static_cast<std::uint32_t>(a.gamepad_buttons.size()));
        for (const auto& b : a.gamepad_buttons) put_i32(payload, b.button_index);
        put_u32(payload, static_cast<std::uint32_t>(a.mouse_buttons.size()));
        for (const auto& b : a.mouse_buttons) put_i32(payload, b.button_index);
        put_u32(payload, static_cast<std::uint32_t>(a.gamepad_axes.size()));
        for (const auto& ax : a.gamepad_axes) {
            put_i32(payload, ax.axis);
            put_f32(payload, ax.axis_value);
        }
    }
}

void read_input_remap_backup(Reader& r, SaveData& s) {
    gus::domain::input::InputRemapConfig cfg;
    cfg.config_version = static_cast<int>(r.read_u32());
    const std::uint32_t actions_count = r.bounded_count(r.read_u32(), 4, "v4-actions");
    cfg.actions.reserve(actions_count);
    for (std::uint32_t i = 0; i < actions_count; ++i) {
        gus::domain::input::ActionBindings a;
        a.action_name = r.read_string();
        a.deadzone = r.read_f32();
        const std::uint32_t keys_n = r.bounded_count(r.read_u32(), 11, "v4-keys");
        for (std::uint32_t k = 0; k < keys_n; ++k) {
            gus::domain::input::KeyBinding kb;
            kb.keycode = r.read_i64();
            kb.ctrl_pressed = (r.read_u8() != 0);
            kb.shift_pressed = (r.read_u8() != 0);
            kb.alt_pressed = (r.read_u8() != 0);
            a.keys.push_back(kb);
        }
        const std::uint32_t gb_n = r.bounded_count(r.read_u32(), 4, "v4-gpbtn");
        for (std::uint32_t j = 0; j < gb_n; ++j)
            a.gamepad_buttons.push_back(
                gus::domain::input::GamepadButtonBinding{r.read_i32()});
        const std::uint32_t mb_n = r.bounded_count(r.read_u32(), 4, "v4-mousebtn");
        for (std::uint32_t j = 0; j < mb_n; ++j)
            a.mouse_buttons.push_back(
                gus::domain::input::MouseButtonBinding{r.read_i32()});
        const std::uint32_t ax_n = r.bounded_count(r.read_u32(), 8, "v4-axes");
        for (std::uint32_t j = 0; j < ax_n; ++j) {
            gus::domain::input::GamepadAxisBinding ax;
            ax.axis = r.read_i32();
            ax.axis_value = r.read_f32();
            a.gamepad_axes.push_back(ax);
        }
        cfg.actions.push_back(std::move(a));
    }
    s.input_remap_backup = std::move(cfg);
}

// V4: controls_hash128 (16 bytes crus).
void write_controls_hash(std::vector<std::uint8_t>& payload, const SaveData& s) {
    payload.insert(payload.end(), s.controls_hash128.begin(),
                   s.controls_hash128.end());
}

void read_controls_hash(Reader& r, SaveData& s) {
    for (std::size_t i = 0; i < s.controls_hash128.size(); ++i)
        s.controls_hash128[i] = r.read_u8();
}

// Monta o payload no layout corrente (V4): u32 schema_version || comuns ||
// character_states || enemy_knowledge || input_remap_backup || controls_hash128 ||
// i32 slot_id.
std::vector<std::uint8_t> build_payload_current(const SaveData& data) {
    std::vector<std::uint8_t> payload;
    put_u32(payload, static_cast<std::uint32_t>(current_schema_version()));
    write_common_payload(payload, data);
    write_character_states(payload, data);
    write_enemy_knowledge(payload, data);
    write_input_remap_backup(payload, data);
    write_controls_hash(payload, data);
    put_i32(payload, data.slot_id);
    return payload;
}

// Monta o payload no layout V3 (comuns + character_states + enemy_knowledge, SEM os
// campos V4), para a fixture de migracao V3->V4.
std::vector<std::uint8_t> build_payload_v3(const SaveData& data) {
    std::vector<std::uint8_t> payload;
    put_u32(payload, 3u);
    write_common_payload(payload, data);
    write_character_states(payload, data);
    write_enemy_knowledge(payload, data);
    return payload;
}

// Monta o payload no layout V1 (u32 version || comuns; SEM character_states/
// enemy_knowledge), com a versao declarada (1 no save_v1 real; arbitrario em
// make_v1_payload).
std::vector<std::uint8_t> build_payload_v1(const SaveData& data, int declared_version) {
    std::vector<std::uint8_t> payload;
    put_u32(payload, static_cast<std::uint32_t>(declared_version));
    write_common_payload(payload, data);
    return payload;
}

// Monta o payload no layout V2 (u32 version=2 || comuns || character_states; SEM
// enemy_knowledge), para a fixture de migracao V2->V3.
std::vector<std::uint8_t> build_payload_v2(const SaveData& data) {
    std::vector<std::uint8_t> payload;
    put_u32(payload, 2u);
    write_common_payload(payload, data);
    write_character_states(payload, data);
    return payload;
}

}  // namespace

// ---- envelope (pack/unpack) ------------------------------------------------

std::vector<std::uint8_t> pack_save(const std::vector<std::uint8_t>& payload) {
    std::vector<std::uint8_t> buffer;
    buffer.reserve(kHeaderLen + payload.size() + kHmacLen);

    buffer.insert(buffer.end(), kMagic.begin(), kMagic.end());
    put_u32(buffer, static_cast<std::uint32_t>(payload.size()));
    buffer.insert(buffer.end(), payload.begin(), payload.end());

    const auto tag = crypto::hmac_sha256(embedded_key().data(),
                                         embedded_key().size(), buffer.data(),
                                         buffer.size());
    buffer.insert(buffer.end(), tag.begin(), tag.end());
    return buffer;
}

std::vector<std::uint8_t> unpack_save(const std::vector<std::uint8_t>& data) {
    if (data.size() < kHeaderLen + kHmacLen)
        throw SaveCorruptError("Dados de save curtos demais (header + hmac).");

    for (std::size_t i = 0; i < kMagicLen; ++i) {
        if (data[i] != kMagic[i])
            throw SaveCorruptError("Magic bytes invalidos: nao e um save 'GDS2'.");
    }

    const std::uint32_t payload_len =
        static_cast<std::uint32_t>(data[kMagicLen]) |
        (static_cast<std::uint32_t>(data[kMagicLen + 1]) << 8) |
        (static_cast<std::uint32_t>(data[kMagicLen + 2]) << 16) |
        (static_cast<std::uint32_t>(data[kMagicLen + 3]) << 24);

    const std::uint64_t expected_total =
        static_cast<std::uint64_t>(kHeaderLen) + payload_len + kHmacLen;
    if (expected_total != data.size())
        throw SaveCorruptError(
            "Length inconsistente: total declarado != tamanho real.");

    const std::size_t payload_end = kHeaderLen + payload_len;

    const auto expected = crypto::hmac_sha256(embedded_key().data(),
                                              embedded_key().size(), data.data(),
                                              payload_end);
    std::array<std::uint8_t, kHmacLen> actual{};
    std::memcpy(actual.data(), data.data() + payload_end, kHmacLen);
    if (!crypto::fixed_time_equals(expected, actual))
        throw SaveIntegrityError("HMAC mismatch: save adulterado ou corrompido.");

    return std::vector<std::uint8_t>(
        data.begin() + static_cast<std::ptrdiff_t>(kHeaderLen),
        data.begin() + static_cast<std::ptrdiff_t>(payload_end));
}

// ---- serialize -------------------------------------------------------------

std::vector<std::uint8_t> serialize_save(const SaveData& data) {
    data.validate();  // fail-fast antes de empacotar (std::invalid_argument)
    return pack_save(build_payload_current(data));
}

std::vector<std::uint8_t> serialize_save_unchecked(const SaveData& data) {
    return pack_save(build_payload_current(data));  // SO testes (sem validate)
}

// ---- deserialize (version-aware, forward-only) -----------------------------

SaveData deserialize_save(const std::vector<std::uint8_t>& data) {
    // 1. Integridade primeiro: HMAC mismatch lanca SaveIntegrityError aqui, ANTES
    //    de ler versao ou migrar (nunca migra bytes adulterados, CONTRACT §7).
    const auto payload = unpack_save(data);

    // 2. Le a versao do payload validado (fonte canonica).
    Reader r(payload);
    const int version = static_cast<int>(r.read_u32());

    // 3. Versao futura: forward-only nao le o futuro.
    if (version > current_schema_version())
        throw SaveVersionTooNewError(version, current_schema_version());

    if (version < 1)
        throw SaveCorruptError("schema_version invalido (< 1): " +
                               std::to_string(version));

    // 4. Materializa o SaveData no layout DA VERSAO LIDA (V1 nao tem
    //    character_states). Os campos comuns sao identicos entre V1 e V2.
    SaveData decoded;
    decoded.schema_version = version;
    read_common_payload(r, decoded);
    if (version >= 2) read_character_states(r, decoded);
    if (version >= 3) read_enemy_knowledge(r, decoded);
    if (version >= 4) {
        read_input_remap_backup(r, decoded);
        read_controls_hash(r, decoded);
        decoded.slot_id = r.read_i32();
    }
    r.expect_end();

    // 5. Sobe pela chain ate a versao atual (no-op se ja == atual).
    SaveData migrated = migrate_to_current(std::move(decoded), version);

    // 6. Defesa em profundidade: valida invariantes mesmo apos HMAC valido
    //    (schema drift e possivel; payload forjado com chave vazada).
    migrated.validate();
    return migrated;
}

// ---- fixtures de migracao (declaradas em save_migrators.hpp) ----------------
//
// Moram aqui (junto do codec) para reaproveitar o writer V1 sem expor o codec
// interno. A logica da chain fica em save_migrators.cpp.

std::vector<std::uint8_t> serialize_save_v1(const SaveData& data) {
    return pack_save(build_payload_v1(data, 1));
}

std::vector<std::uint8_t> serialize_save_v2(const SaveData& data) {
    return pack_save(build_payload_v2(data));
}

std::vector<std::uint8_t> serialize_save_v3(const SaveData& data) {
    return pack_save(build_payload_v3(data));
}

std::vector<std::uint8_t> make_v1_payload(int version) {
    SaveData minimal;
    minimal.current_scene_path = "res://forged.tscn";
    return pack_save(build_payload_v1(minimal, version));
}

// ---- load_save (T1.1 detect-and-respond, NAO-lancante) ---------------------

LoadOutcome load_save(const std::vector<std::uint8_t>& data, int expected_slot) {
    LoadOutcome out;
    try {
        out.data = deserialize_save(data);
        out.result = LoadResult::Ok;
    } catch (const SaveIntegrityError&) {
        out.result = LoadResult::HmacInvalid;
        return out;
    } catch (const SaveVersionTooNewError&) {
        out.result = LoadResult::VersionTooNew;
        return out;
    } catch (const SaveCorruptError&) {
        out.result = LoadResult::Corrupt;
        return out;
    } catch (const std::invalid_argument&) {
        // Payload bem-formado e selado, mas schema-divergente (invariante violada).
        out.result = LoadResult::Invalid;
        return out;
    }

    // T1.2: o save carregou integro. Se veio de um save PRE-V4 (sem slot_id selado,
    // slot_id == -1 default que o migrator nao altera), adota o slot informado pelo
    // load como origem (e o slot de onde a camada de I/O leu). Saves V4 ja trazem o
    // slot_id selado: ai comparamos.
    if (expected_slot >= 0) {
        if (out.data.slot_id < 0) {
            // Origem nao selada (save migrado): assume o slot lido como origem.
            out.data.slot_id = expected_slot;
        } else if (out.data.slot_id != expected_slot) {
            // Slot selado diverge do slot lido: arquivo trocado de posicao. Dados
            // ficam disponiveis; a app decide avisar/aceitar.
            out.result = LoadResult::WrongSlot;
        }
    }
    return out;
}

}  // namespace gus::domain::save
