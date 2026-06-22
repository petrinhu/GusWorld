// gus/domain/src/templates/template_serializer.cpp
//
// Implementacao do serializer binario proprio + HMAC-SHA256 (core/). Ver header
// para o envelope e o layout do payload. POCO puro, ZERO Qt. O HMAC vem de
// gus::core::crypto (proprio, validado contra FIPS/RFC). ADR-006.

#include "gus/domain/templates/template_serializer.hpp"

#include <array>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "gus/core/crypto/hmac_sha256.hpp"

namespace gus::domain::templates {

namespace {

namespace crypto = gus::core::crypto;

// MAGIC "GDT1" (GusDragon Template v1).
constexpr std::array<std::uint8_t, 4> kMagic = {'G', 'D', 'T', '1'};
constexpr std::size_t kMagicLen = 4;
constexpr std::size_t kLengthFieldLen = 4;
constexpr std::size_t kHmacLen = crypto::kSha256DigestSize;  // 32
constexpr std::size_t kHeaderLen = kMagicLen + kLengthFieldLen;  // 8

// Chave de integridade FIXA embutida (ADR-006: integridade casual local, nao
// sigilo). Chave PROPRIA dos templates do GusEngine (distinta da futura chave de
// save), para que um payload de save nao passe como template valido e vice-versa.
const std::vector<std::uint8_t>& embedded_key() {
    static const std::string s =
        "gusengine-cpp-2026-template-v1-hmac-sha256-key";
    static const std::vector<std::uint8_t> k(s.begin(), s.end());
    return k;
}

// ---- writer binario (little-endian) ---------------------------------------

void put_u32(std::vector<std::uint8_t>& out, std::uint32_t v) {
    out.push_back(static_cast<std::uint8_t>(v & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((v >> 8) & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((v >> 16) & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((v >> 24) & 0xFFu));
}

// int32 gravado como bit-pattern LE (fiel ao tipo; stats sao >= 0 por invariante).
void put_i32(std::vector<std::uint8_t>& out, int v) {
    put_u32(out, static_cast<std::uint32_t>(v));
}

void put_string(std::vector<std::uint8_t>& out, const std::string& s) {
    put_u32(out, static_cast<std::uint32_t>(s.size()));
    out.insert(out.end(), s.begin(), s.end());
}

void put_deck(std::vector<std::uint8_t>& out,
              const std::vector<std::string>& deck) {
    put_u32(out, static_cast<std::uint32_t>(deck.size()));
    for (const auto& card : deck) {
        put_string(out, card);
    }
}

// ---- reader binario (little-endian), com bounds-check ----------------------

// Cursor sobre o payload; cada leitura valida limites e lanca TemplateCorruptError
// se o buffer acabar (payload truncado/forjado).
class Reader {
   public:
    explicit Reader(const std::vector<std::uint8_t>& buf) : buf_(buf) {}

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

    std::uint8_t read_u8() {
        require(1, "u8");
        return buf_[pos_++];
    }

    std::string read_string() {
        const std::uint32_t len = read_u32();
        require(len, "string");
        std::string s(buf_.begin() + static_cast<std::ptrdiff_t>(pos_),
                      buf_.begin() + static_cast<std::ptrdiff_t>(pos_ + len));
        pos_ += len;
        return s;
    }

    std::vector<std::string> read_deck() {
        const std::uint32_t count = read_u32();
        std::vector<std::string> deck;
        deck.reserve(count);
        for (std::uint32_t i = 0; i < count; ++i) {
            deck.push_back(read_string());
        }
        return deck;
    }

    // Garante que NAO sobraram bytes (payload com cauda extra = corrupcao).
    void expect_end() const {
        if (pos_ != buf_.size()) {
            throw TemplateCorruptError(
                "Payload .gdt tem bytes extras apos os campos esperados.");
        }
    }

   private:
    void require(std::size_t n, const char* what) const {
        if (pos_ + n > buf_.size()) {
            throw TemplateCorruptError(
                std::string("Payload .gdt truncado ao ler ") + what + ".");
        }
    }

    const std::vector<std::uint8_t>& buf_;
    std::size_t pos_ = 0;
};

}  // namespace

// ---- envelope (pack/unpack) ------------------------------------------------

std::vector<std::uint8_t> pack(const std::vector<std::uint8_t>& payload) {
    std::vector<std::uint8_t> buffer;
    buffer.reserve(kHeaderLen + payload.size() + kHmacLen);

    // MAGIC
    buffer.insert(buffer.end(), kMagic.begin(), kMagic.end());
    // LENGTH (uint32 LE)
    put_u32(buffer, static_cast<std::uint32_t>(payload.size()));
    // PAYLOAD
    buffer.insert(buffer.end(), payload.begin(), payload.end());
    // HMAC sobre header + payload (tudo que veio antes).
    const auto tag = crypto::hmac_sha256(embedded_key().data(),
                                         embedded_key().size(), buffer.data(),
                                         buffer.size());
    buffer.insert(buffer.end(), tag.begin(), tag.end());
    return buffer;
}

std::vector<std::uint8_t> unpack(const std::vector<std::uint8_t>& data) {
    if (data.size() < kHeaderLen + kHmacLen) {
        throw TemplateCorruptError(
            "Dados .gdt curtos demais (menos que header + hmac).");
    }

    // MAGIC
    for (std::size_t i = 0; i < kMagicLen; ++i) {
        if (data[i] != kMagic[i]) {
            throw TemplateCorruptError(
                "Magic bytes invalidos: nao e um arquivo .gdt 'GDT1'.");
        }
    }

    // LENGTH
    const std::uint32_t payload_len =
        static_cast<std::uint32_t>(data[kMagicLen]) |
        (static_cast<std::uint32_t>(data[kMagicLen + 1]) << 8) |
        (static_cast<std::uint32_t>(data[kMagicLen + 2]) << 16) |
        (static_cast<std::uint32_t>(data[kMagicLen + 3]) << 24);

    const std::uint64_t expected_total =
        static_cast<std::uint64_t>(kHeaderLen) + payload_len + kHmacLen;
    if (expected_total != data.size()) {
        throw TemplateCorruptError(
            "Length inconsistente: total declarado != tamanho real do arquivo.");
    }

    const std::size_t payload_end = kHeaderLen + payload_len;

    // HMAC recompute sobre header + payload, compara em tempo constante.
    const auto expected = crypto::hmac_sha256(embedded_key().data(),
                                              embedded_key().size(), data.data(),
                                              payload_end);
    std::array<std::uint8_t, kHmacLen> actual{};
    std::memcpy(actual.data(), data.data() + payload_end, kHmacLen);
    if (!crypto::fixed_time_equals(expected, actual)) {
        throw TemplateIntegrityError(
            "HMAC mismatch: arquivo .gdt adulterado ou corrompido.");
    }

    return std::vector<std::uint8_t>(
        data.begin() + static_cast<std::ptrdiff_t>(kHeaderLen),
        data.begin() + static_cast<std::ptrdiff_t>(payload_end));
}

// ---- CharacterTemplate -----------------------------------------------------

std::vector<std::uint8_t> serialize_character(const CharacterTemplate& tpl) {
    tpl.validate();  // fail-fast antes de empacotar (lanca std::invalid_argument)

    std::vector<std::uint8_t> payload;
    put_string(payload, tpl.id);
    put_i32(payload, tpl.max_hp);
    put_i32(payload, tpl.atk);
    put_i32(payload, tpl.def);
    put_i32(payload, tpl.spd);
    put_u32(payload, static_cast<std::uint32_t>(tpl.family));
    payload.push_back(tpl.is_universal_compiler ? 1u : 0u);
    put_deck(payload, tpl.base_deck);

    return pack(payload);
}

CharacterTemplate deserialize_character(const std::vector<std::uint8_t>& data) {
    const auto payload = unpack(data);  // valida HMAC + envelope

    Reader r(payload);
    CharacterTemplate tpl;
    tpl.id = r.read_string();
    tpl.max_hp = r.read_i32();
    tpl.atk = r.read_i32();
    tpl.def = r.read_i32();
    tpl.spd = r.read_i32();
    tpl.family = static_cast<CardFamily>(r.read_u32());
    tpl.is_universal_compiler = (r.read_u8() != 0);
    tpl.base_deck = r.read_deck();
    r.expect_end();

    tpl.validate();  // defesa contra payload bem-formado mas schema-divergente
    return tpl;
}

// ---- EnemyTemplate ---------------------------------------------------------

std::vector<std::uint8_t> serialize_enemy(const EnemyTemplate& tpl) {
    tpl.validate();

    std::vector<std::uint8_t> payload;
    put_string(payload, tpl.id);
    put_i32(payload, tpl.max_hp);
    put_i32(payload, tpl.atk);
    put_i32(payload, tpl.def);
    put_i32(payload, tpl.spd);
    put_u32(payload, static_cast<std::uint32_t>(tpl.family));
    put_u32(payload, static_cast<std::uint32_t>(tpl.brain));
    payload.push_back(tpl.is_boss ? 1u : 0u);
    put_deck(payload, tpl.base_deck);

    return pack(payload);
}

EnemyTemplate deserialize_enemy(const std::vector<std::uint8_t>& data) {
    const auto payload = unpack(data);

    Reader r(payload);
    EnemyTemplate tpl;
    tpl.id = r.read_string();
    tpl.max_hp = r.read_i32();
    tpl.atk = r.read_i32();
    tpl.def = r.read_i32();
    tpl.spd = r.read_i32();
    tpl.family = static_cast<CardFamily>(r.read_u32());
    tpl.brain = static_cast<BrainKind>(r.read_u32());
    tpl.is_boss = (r.read_u8() != 0);
    tpl.base_deck = r.read_deck();
    r.expect_end();

    tpl.validate();
    return tpl;
}

}  // namespace gus::domain::templates
