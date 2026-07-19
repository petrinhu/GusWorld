// gus/domain/src/save/save_serializer.cpp
//
// Serializer binario proprio + AEAD XChaCha20-Poly1305 (core/, sobre o Monocypher
// vendorizado) do estado de save. Ver header para envelope e layout. POCO puro,
// ZERO Qt. ADR-015 (SAVE-CRYPTO-V2-ENVELOPE): envelope GDS3 substitui o GDS2/HMAC
// do ADR-006 SO no save (templates continuam GDS2/HMAC, fora de escopo).
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

#include "gus/core/crypto/aead_xchacha20poly1305.hpp"
#include "gus/core/crypto/argon2id.hpp"
#include "gus/domain/deck/deck_records.hpp"  // CardInstance (V6 card_collection)
#include "gus/domain/save/save_migrators.hpp"

namespace gus::domain::save {

namespace {

namespace crypto = gus::core::crypto;

// MAGIC "GDS3" (GusDragon Save v3, ADR-015). O sufixo do magic NAO e a versao do
// schema (a versao vive no payload, u32 inicial); distingue de templates ("GDT1",
// que segue INTOCADO no formato GDS2/HMAC).
constexpr std::array<std::uint8_t, 4> kMagic = {'G', 'D', 'S', '3'};
constexpr std::uint16_t kEnvelopeVersion = 1;  // versao do FORMATO do envelope

constexpr std::size_t kMagicLen = 4;
constexpr std::size_t kEnvelopeVerLen = 2;
constexpr std::size_t kSlotIdLen = 4;
constexpr std::size_t kRollbackCtrLen = 8;
// AAD = magic || envelope_ver || slot_id || rollback_ctr (ADR-015 decisao 2).
constexpr std::size_t kAadLen =
    kMagicLen + kEnvelopeVerLen + kSlotIdLen + kRollbackCtrLen;  // 18
constexpr std::size_t kNonceLen = crypto::kAeadNonceSize;        // 24
constexpr std::size_t kCiphertextLenFieldLen = 4;
constexpr std::size_t kTagLen = crypto::kAeadTagSize;  // 16
// AAD || nonce || ciphertext_len, ANTES do ciphertext em si.
constexpr std::size_t kHeaderLen = kAadLen + kNonceLen + kCiphertextLenFieldLen;  // 46

// Chave AEAD derivada via Argon2id (ADR-015 decisao 3, piso OWASP default do
// wrapper: m=19456 KiB/t=2/p=1) do MESMO segredo-base embutido do ADR-006/T2.2 +
// sal fixo "gusworld-save-v2" (o contexto que o ADR chama de "sal"). SEM
// fingerprint de maquina nesta onda (saves normais portaveis cross-PC; o slot
// Hardcore machine-bound e a Onda 3). Cache estatico: a derivacao (custo de
// dezenas de ms por design, memory-hard) roda uma vez por processo, nao por save.
const std::array<std::uint8_t, crypto::kAeadKeySize>& aead_key() {
    static const std::array<std::uint8_t, crypto::kAeadKeySize> k = [] {
        static const std::string base = "gusengine-cpp-2026-integrity-base-secret";
        static const std::string salt = "gusworld-save-v2";
        return crypto::derive_key_argon2id(
            reinterpret_cast<const std::uint8_t*>(base.data()), base.size(),
            reinterpret_cast<const std::uint8_t*>(salt.data()), salt.size());
    }();
    return k;
}

// ---- writer binario (little-endian) ---------------------------------------

void put_u16(std::vector<std::uint8_t>& out, std::uint16_t v) {
    out.push_back(static_cast<std::uint8_t>(v & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((v >> 8) & 0xFFu));
}

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

    std::uint16_t read_u16() {
        require(2, "u16");
        const std::uint16_t v = static_cast<std::uint16_t>(buf_[pos_]) |
                                (static_cast<std::uint16_t>(buf_[pos_ + 1]) << 8);
        pos_ += 2;
        return v;
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

// Layout LEGADO (V2..V5): current_hp | xp | list<str> deck. Usado SO pelos
// payloads antigos (build_payload_v2..v5, fixtures de migracao) - o layout
// CORRENTE (V6+) usa write_character_states_v6/read_character_states_v6 abaixo
// (DECK-4: card_collection substitui deck).
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

// Escreve uma lista de CardInstance (instance_id u64 | card_id str) - shape
// compartilhado entre card_collection.active e .dead (V6).
void put_card_instance_list(
    std::vector<std::uint8_t>& out,
    const std::vector<gus::domain::deck::CardInstance>& list) {
    put_u32(out, static_cast<std::uint32_t>(list.size()));
    for (const auto& inst : list) {
        put_u64(out, inst.instance_id);
        put_string(out, inst.card_id);
    }
}

// Le uma lista de CardInstance. IMP-01 (CWE-789): cada instancia custa no minimo
// 12 bytes (u64 instance_id + u32 do length da string) - bounded_count barra um
// count implausivel ANTES do reserve, mesmo padrao de read_string_list.
std::vector<gus::domain::deck::CardInstance> read_card_instance_list(Reader& r,
                                                                      const char* what) {
    const std::uint32_t n = r.bounded_count(r.read_u32(), 12, what);
    std::vector<gus::domain::deck::CardInstance> list;
    list.reserve(n);
    for (std::uint32_t i = 0; i < n; ++i) {
        gus::domain::deck::CardInstance inst;
        inst.instance_id = r.read_u64();
        inst.card_id = r.read_string();
        list.push_back(std::move(inst));
    }
    return list;
}

// Layout FIXTURE V6 (DECK-4, congelado em CARDS-HW-1): current_hp | xp |
// card_collection{ list<CardInstance{u64 instance_id, str card_id}> active |
// list<CardInstance> dead | u64 next_instance_id } | list<u64> hand_selection.
// SEM CardPhysicalState (layout ANTERIOR ao bump V7) - usado SO por
// serialize_save_v6 (fixture de migracao) e pelo read path de version==6. O
// layout CORRENTE (V7+) e write_character_states_v7/read_character_states_v7
// abaixo (CADA CardInstance ganhou physical).
void write_character_states_v6(std::vector<std::uint8_t>& payload, const SaveData& s) {
    put_map(payload, s.character_states,
            [](std::vector<std::uint8_t>& o, const CharacterSaveState& st) {
                put_i32(o, st.current_hp);
                put_i32(o, st.xp);
                put_card_instance_list(o, st.card_collection.active);
                put_card_instance_list(o, st.card_collection.dead);
                put_u64(o, st.card_collection.next_instance_id);
                put_u32(o, static_cast<std::uint32_t>(st.hand_selection.size()));
                for (const auto instance_id : st.hand_selection) put_u64(o, instance_id);
            });
}

void read_character_states_v6(Reader& r, SaveData& s) {
    const std::uint32_t n = r.read_u32();
    for (std::uint32_t i = 0; i < n; ++i) {
        const std::string id = r.read_string();
        CharacterSaveState st;
        st.current_hp = r.read_i32();
        st.xp = r.read_i32();
        st.card_collection.active = read_card_instance_list(r, "v6-card-active");
        st.card_collection.dead = read_card_instance_list(r, "v6-card-dead");
        st.card_collection.next_instance_id = r.read_u64();
        // hand_selection: cada id custa >= 8 bytes (u64).
        const std::uint32_t hand_n = r.bounded_count(r.read_u32(), 8, "v6-hand");
        st.hand_selection.reserve(hand_n);
        for (std::uint32_t j = 0; j < hand_n; ++j)
            st.hand_selection.push_back(r.read_u64());
        s.character_states.emplace(id, std::move(st));
    }
}

// ---- CardPhysicalState (V7, CARDS-HW-1) -------------------------------------
//
// u32 origin | u16 battery_recharge_cycles | u32 battery_charge_deficit |
// u8 is_infected | u8 is_diagnosed | u32 virus_kind | u8 is_burned_out.
// Enums gravados como u32 cru (mesmo padrao de difficulty/CardTier neste codec);
// o hardening de ordinal fora do dominio fica em CardPhysicalState::validate()
// (chamado por CardCollectionState::validate(), defesa em profundidade no fim de
// deserialize_save - mesmo padrao ja usado por DifficultyLevel).
void put_card_physical_state(std::vector<std::uint8_t>& out,
                             const gus::domain::deck::CardPhysicalState& p) {
    put_u32(out, static_cast<std::uint32_t>(p.origin));
    put_u16(out, p.battery_recharge_cycles);
    put_u32(out, p.battery_charge_deficit);
    out.push_back(p.is_infected ? 1u : 0u);
    out.push_back(p.is_diagnosed ? 1u : 0u);
    put_u32(out, static_cast<std::uint32_t>(p.virus_kind));
    out.push_back(p.is_burned_out ? 1u : 0u);
}

gus::domain::deck::CardPhysicalState read_card_physical_state(Reader& r) {
    gus::domain::deck::CardPhysicalState p;
    p.origin = static_cast<gus::domain::deck::CardOrigin>(r.read_u32());
    p.battery_recharge_cycles = r.read_u16();
    p.battery_charge_deficit = r.read_u32();
    p.is_infected = (r.read_u8() != 0);
    p.is_diagnosed = (r.read_u8() != 0);
    p.virus_kind = static_cast<gus::domain::deck::VirusKind>(r.read_u32());
    p.is_burned_out = (r.read_u8() != 0);
    return p;
}

// Escreve uma lista de CardInstance NO LAYOUT V7 (instance_id u64 | card_id str |
// CardPhysicalState physical) - shape compartilhado entre card_collection.active
// e .dead (V7, CARDS-HW-1).
void put_card_instance_list_v7(
    std::vector<std::uint8_t>& out,
    const std::vector<gus::domain::deck::CardInstance>& list) {
    put_u32(out, static_cast<std::uint32_t>(list.size()));
    for (const auto& inst : list) {
        put_u64(out, inst.instance_id);
        put_string(out, inst.card_id);
        put_card_physical_state(out, inst.physical);
    }
}

// Le uma lista de CardInstance no layout V7. IMP-01 (CWE-789): cada instancia
// custa no minimo 29 bytes - 12 do shape V6 (8 do instance_id u64 + 4 do length
// da string card_id) + 17 de CardPhysicalState (origin u32=4, cycles u16=2,
// deficit u32=4, is_infected u8=1, is_diagnosed u8=1, virus_kind u32=4,
// is_burned_out u8=1) - bounded_count barra um count implausivel ANTES do
// reserve, mesmo padrao de read_card_instance_list (V6).
std::vector<gus::domain::deck::CardInstance> read_card_instance_list_v7(Reader& r,
                                                                         const char* what) {
    const std::uint32_t n = r.bounded_count(r.read_u32(), 12 + 17, what);
    std::vector<gus::domain::deck::CardInstance> list;
    list.reserve(n);
    for (std::uint32_t i = 0; i < n; ++i) {
        gus::domain::deck::CardInstance inst;
        inst.instance_id = r.read_u64();
        inst.card_id = r.read_string();
        inst.physical = read_card_physical_state(r);
        list.push_back(std::move(inst));
    }
    return list;
}

// Layout CORRENTE (V7+, CARDS-HW-1): current_hp | xp | card_collection{
// list<CardInstance{u64 instance_id, str card_id, CardPhysicalState physical}>
// active | list<CardInstance> dead | u64 next_instance_id } | list<u64>
// hand_selection. SUBSTITUI o layout V6 (write_character_states_v6 acima, agora
// congelado como fixture de migracao) - CADA CardInstance ganhou physical.
// credits NAO mora aqui - carteira UNICA da party (write_credits_wallet abaixo).
void write_character_states_v7(std::vector<std::uint8_t>& payload, const SaveData& s) {
    put_map(payload, s.character_states,
            [](std::vector<std::uint8_t>& o, const CharacterSaveState& st) {
                put_i32(o, st.current_hp);
                put_i32(o, st.xp);
                put_card_instance_list_v7(o, st.card_collection.active);
                put_card_instance_list_v7(o, st.card_collection.dead);
                put_u64(o, st.card_collection.next_instance_id);
                put_u32(o, static_cast<std::uint32_t>(st.hand_selection.size()));
                for (const auto instance_id : st.hand_selection) put_u64(o, instance_id);
            });
}

void read_character_states_v7(Reader& r, SaveData& s) {
    const std::uint32_t n = r.read_u32();
    for (std::uint32_t i = 0; i < n; ++i) {
        const std::string id = r.read_string();
        CharacterSaveState st;
        st.current_hp = r.read_i32();
        st.xp = r.read_i32();
        st.card_collection.active = read_card_instance_list_v7(r, "v7-card-active");
        st.card_collection.dead = read_card_instance_list_v7(r, "v7-card-dead");
        st.card_collection.next_instance_id = r.read_u64();
        // hand_selection: cada id custa >= 8 bytes (u64).
        const std::uint32_t hand_n = r.bounded_count(r.read_u32(), 8, "v7-hand");
        st.hand_selection.reserve(hand_n);
        for (std::uint32_t j = 0; j < hand_n; ++j)
            st.hand_selection.push_back(r.read_u64());
        s.character_states.emplace(id, std::move(st));
    }
}

// V6 (DECK-4): carteira UNICA da party (i64 credits), gravada 1x - NAO
// per-character (docs/design/mecanicas/economia.md, single-currency).
void write_credits_wallet(std::vector<std::uint8_t>& payload, const SaveData& s) {
    put_i64(payload, s.credits);
}

void read_credits_wallet(Reader& r, SaveData& s) { s.credits = r.read_i64(); }

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

// Bloco COMUM aos layouts V4 e V5 (idêntico até slot_id inclusive) - fatorado pra
// nao duplicar entre build_payload_v4 (fixture de migracao) e build_payload_current
// (V5, que so acrescenta o bloco de dificuldade por cima).
void write_v4_tail(std::vector<std::uint8_t>& payload, const SaveData& data) {
    write_common_payload(payload, data);
    write_character_states(payload, data);
    write_enemy_knowledge(payload, data);
    write_input_remap_backup(payload, data);
    write_controls_hash(payload, data);
    put_i32(payload, data.slot_id);
}

// V5 (MODOS-MORTE Fase 0): u32 difficulty | i32 difficult_recovery_stage.
void write_difficulty_fields(std::vector<std::uint8_t>& payload, const SaveData& s) {
    put_u32(payload, static_cast<std::uint32_t>(s.difficulty));
    put_i32(payload, s.difficult_recovery_stage);
}

void read_difficulty_fields(Reader& r, SaveData& s) {
    s.difficulty = static_cast<DifficultyLevel>(r.read_u32());
    s.difficult_recovery_stage = r.read_i32();
}

// Monta o payload no layout corrente (V7, CARDS-HW-1): u32 schema_version ||
// comuns || character_states_v7 (card_collection COM CardInstance::physical +
// hand_selection, SEM o deck legado) || enemy_knowledge || input_remap_backup ||
// controls_hash128 || i32 slot_id || u32 difficulty || i32
// difficult_recovery_stage || i64 credits (carteira UNICA da party, gravada 1x no
// FIM do payload - nao per-character).
std::vector<std::uint8_t> build_payload_current(const SaveData& data) {
    std::vector<std::uint8_t> payload;
    put_u32(payload, static_cast<std::uint32_t>(current_schema_version()));
    write_common_payload(payload, data);
    write_character_states_v7(payload, data);
    write_enemy_knowledge(payload, data);
    write_input_remap_backup(payload, data);
    write_controls_hash(payload, data);
    put_i32(payload, data.slot_id);
    write_difficulty_fields(payload, data);
    write_credits_wallet(payload, data);
    return payload;
}

// Monta o payload no layout V6 (comuns || character_states_v6 COM
// card_collection/hand_selection MAS SEM CardInstance::physical || enemy_knowledge
// || input_remap_backup || controls_hash128 || slot_id || difficulty ||
// difficult_recovery_stage || credits, SEM o campo V7: physical dentro de cada
// CardInstance), para a fixture de migracao V6->V7 (CARDS-HW-1). E o que
// build_payload_current fazia ANTES deste bump (DECK-4 congelado como "geracao V6").
std::vector<std::uint8_t> build_payload_v6(const SaveData& data) {
    std::vector<std::uint8_t> payload;
    put_u32(payload, 6u);
    write_common_payload(payload, data);
    write_character_states_v6(payload, data);
    write_enemy_knowledge(payload, data);
    write_input_remap_backup(payload, data);
    write_controls_hash(payload, data);
    put_i32(payload, data.slot_id);
    write_difficulty_fields(payload, data);
    write_credits_wallet(payload, data);
    return payload;
}

// Monta o payload no layout V4 (comuns || character_states || enemy_knowledge ||
// input_remap_backup || controls_hash128 || slot_id, SEM os campos V5:
// difficulty/difficult_recovery_stage), para a fixture de migracao V4->V5.
std::vector<std::uint8_t> build_payload_v4(const SaveData& data) {
    std::vector<std::uint8_t> payload;
    put_u32(payload, 4u);
    write_v4_tail(payload, data);
    return payload;
}

// Monta o payload no layout V5 (comuns || character_states COM O DECK LEGADO (list
// <str>, write_character_states acima) || enemy_knowledge || input_remap_backup ||
// controls_hash128 || slot_id || difficulty || difficult_recovery_stage, SEM os
// campos V6: card_collection/credits/hand_selection), para a fixture de migracao
// V5->V6 (DECK-4). E o que build_payload_current fazia ANTES deste bump (MODOS-
// MORTE Fase 0 congelado como "geracao V5").
std::vector<std::uint8_t> build_payload_v5(const SaveData& data) {
    std::vector<std::uint8_t> payload;
    put_u32(payload, 5u);
    write_v4_tail(payload, data);
    write_difficulty_fields(payload, data);
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

// ---- envelope GDS3 (pack/unpack), ADR-015 decisao 2 -------------------------

std::vector<std::uint8_t> pack_save(const std::vector<std::uint8_t>& payload,
                                    std::int32_t slot_id,
                                    std::uint64_t rollback_ctr) {
    // AAD = magic || envelope_ver || slot_id || rollback_ctr (os primeiros
    // kAadLen bytes do envelope final - amarra esses campos NA TAG mesmo sem
    // cifra-los, ADR-015 decisao 2).
    std::vector<std::uint8_t> aad;
    aad.reserve(kAadLen);
    aad.insert(aad.end(), kMagic.begin(), kMagic.end());
    put_u16(aad, kEnvelopeVersion);
    put_i32(aad, slot_id);
    put_u64(aad, rollback_ctr);

    const auto nonce = crypto::generate_nonce();  // CSPRNG, nunca reusado
    const auto locked = crypto::aead_lock(aead_key(), nonce, aad.data(), aad.size(),
                                          payload.data(), payload.size());

    std::vector<std::uint8_t> buffer;
    buffer.reserve(kHeaderLen + locked.cipher_text.size() + kTagLen);
    buffer.insert(buffer.end(), aad.begin(), aad.end());
    buffer.insert(buffer.end(), nonce.begin(), nonce.end());
    put_u32(buffer, static_cast<std::uint32_t>(locked.cipher_text.size()));
    buffer.insert(buffer.end(), locked.cipher_text.begin(), locked.cipher_text.end());
    buffer.insert(buffer.end(), locked.tag.begin(), locked.tag.end());
    return buffer;
}

std::vector<std::uint8_t> unpack_save(const std::vector<std::uint8_t>& data) {
    // Minimo: AAD(18) + nonce(24) + ciphertext_len(4) + ciphertext(>=0) + tag(16).
    if (data.size() < kHeaderLen + kTagLen)
        throw SaveCorruptError("Dados de save curtos demais (header + nonce + tag).");

    for (std::size_t i = 0; i < kMagicLen; ++i) {
        if (data[i] != kMagic[i])
            throw SaveCorruptError("Magic bytes invalidos: nao e um save 'GDS3'.");
    }

    const std::uint16_t envelope_ver =
        static_cast<std::uint16_t>(data[kMagicLen]) |
        (static_cast<std::uint16_t>(data[kMagicLen + 1]) << 8);
    if (envelope_ver != kEnvelopeVersion)
        throw SaveCorruptError("Versao de envelope desconhecida: " +
                               std::to_string(envelope_ver));

    // slot_id (i32) e rollback_ctr (u64) NAO precisam de leitura separada aqui:
    // ja fazem parte dos primeiros kAadLen bytes usados como AAD abaixo. Qualquer
    // adulteracao desses campos quebra a verificacao AEAD (nao um memcmp a parte,
    // ADR-015 decisao 2 - simplifica o antigo T1.2 do ADR-007).

    const std::size_t ciphertext_len_off = kAadLen + kNonceLen;
    const std::uint32_t ciphertext_len =
        static_cast<std::uint32_t>(data[ciphertext_len_off]) |
        (static_cast<std::uint32_t>(data[ciphertext_len_off + 1]) << 8) |
        (static_cast<std::uint32_t>(data[ciphertext_len_off + 2]) << 16) |
        (static_cast<std::uint32_t>(data[ciphertext_len_off + 3]) << 24);

    const std::uint64_t expected_total =
        static_cast<std::uint64_t>(kHeaderLen) + ciphertext_len + kTagLen;
    if (expected_total != data.size())
        throw SaveCorruptError(
            "Length inconsistente: total declarado != tamanho real.");

    std::array<std::uint8_t, kNonceLen> nonce{};
    std::memcpy(nonce.data(), data.data() + kAadLen, kNonceLen);

    const std::size_t ciphertext_off = kHeaderLen;
    const std::size_t tag_off = ciphertext_off + ciphertext_len;
    std::array<std::uint8_t, kTagLen> tag{};
    std::memcpy(tag.data(), data.data() + tag_off, kTagLen);

    std::vector<std::uint8_t> plain;
    const bool ok = crypto::aead_unlock(plain, aead_key(), nonce, data.data(),
                                        kAadLen, data.data() + ciphertext_off, tag,
                                        ciphertext_len);
    if (!ok)
        throw SaveIntegrityError(
            "Tag AEAD nao bate: save adulterado ou corrompido.");

    return plain;
}

// ---- serialize -------------------------------------------------------------

std::vector<std::uint8_t> serialize_save(const SaveData& data) {
    data.validate();  // fail-fast antes de empacotar (std::invalid_argument)
    // slot_id do envelope (AAD) = data.slot_id (mesmo valor selado dentro do
    // payload); rollback_ctr = 0 (saves normais, ADR-015 Onda 2).
    return pack_save(build_payload_current(data), data.slot_id);
}

std::vector<std::uint8_t> serialize_save_unchecked(const SaveData& data) {
    // SO testes (sem validate).
    return pack_save(build_payload_current(data), data.slot_id);
}

// ---- deserialize (version-aware, forward-only) -----------------------------

SaveData deserialize_save(const std::vector<std::uint8_t>& data) {
    // 1. Integridade primeiro: tag AEAD invalida lanca SaveIntegrityError aqui,
    //    ANTES de ler versao ou migrar (nunca migra bytes adulterados, CONTRACT §7).
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
    // V2..V5: layout LEGADO (character_states com o `deck` de string). V6: layout
    // card_collection/hand_selection SEM CardInstance::physical (DECK-4). V7+:
    // MESMO layout, CADA CardInstance ganhou physical (CARDS-HW-1). Os 3 grupos
    // sao mutuamente exclusivos - a versao lida decide qual formato esta nos bytes.
    if (version >= 2 && version <= 5) read_character_states(r, decoded);
    if (version == 6) read_character_states_v6(r, decoded);
    if (version >= 7) read_character_states_v7(r, decoded);
    if (version >= 3) read_enemy_knowledge(r, decoded);
    if (version >= 4) {
        read_input_remap_backup(r, decoded);
        read_controls_hash(r, decoded);
        decoded.slot_id = r.read_i32();
    }
    if (version >= 5) read_difficulty_fields(r, decoded);
    // V6 (DECK-4): carteira UNICA da party, gravada 1x no FIM do payload.
    if (version >= 6) read_credits_wallet(r, decoded);
    r.expect_end();

    // 5. Sobe pela chain ate a versao atual (no-op se ja == atual).
    SaveData migrated = migrate_to_current(std::move(decoded), version);

    // 6. Defesa em profundidade: valida invariantes mesmo apos tag AEAD valida
    //    (schema drift e possivel; payload forjado com chave vazada).
    migrated.validate();
    return migrated;
}

// ---- fixtures de migracao (declaradas em save_migrators.hpp) ----------------
//
// Moram aqui (junto do codec) para reaproveitar o writer V1 sem expor o codec
// interno. A logica da chain fica em save_migrators.cpp.

std::vector<std::uint8_t> serialize_save_v1(const SaveData& data) {
    return pack_save(build_payload_v1(data, 1), data.slot_id);
}

std::vector<std::uint8_t> serialize_save_v2(const SaveData& data) {
    return pack_save(build_payload_v2(data), data.slot_id);
}

std::vector<std::uint8_t> serialize_save_v3(const SaveData& data) {
    return pack_save(build_payload_v3(data), data.slot_id);
}

std::vector<std::uint8_t> serialize_save_v4(const SaveData& data) {
    return pack_save(build_payload_v4(data), data.slot_id);
}

std::vector<std::uint8_t> serialize_save_v5(const SaveData& data) {
    return pack_save(build_payload_v5(data), data.slot_id);
}

std::vector<std::uint8_t> serialize_save_v6(const SaveData& data) {
    return pack_save(build_payload_v6(data), data.slot_id);
}

std::vector<std::uint8_t> make_v1_payload(int version) {
    SaveData minimal;
    minimal.current_scene_path = "res://forged.tscn";
    return pack_save(build_payload_v1(minimal, version), minimal.slot_id);
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
