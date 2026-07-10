// gus/domain/src/map/map_serializer.cpp
//
// Serializer binario proprio + HMAC-SHA256 (core/) do TileMap (.gmap). Ver header
// para envelope e layout. POCO puro, ZERO Qt/SDL/I/O. HMAC de gus::core::crypto
// (proprio, validado contra FIPS/RFC), chave DERIVADA com contexto "map-hmac"
// (separacao de dominio: save != mapa). Mesma disciplina do save (ADR-006).

#include "gus/domain/map/map_serializer.hpp"

#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include "gus/core/crypto/hmac_sha256.hpp"
#include "gus/core/crypto/key_derivation.hpp"

namespace gus::domain::map {

namespace {

namespace crypto = gus::core::crypto;

// MAGIC "GMAP". Distingue de save("GDS3", ADR-015) e templates("GDT1"). O sufixo
// do magic NAO e a versao (que vive no payload, u32 inicial).
constexpr std::array<std::uint8_t, 4> kMagic = {'G', 'M', 'A', 'P'};
constexpr std::size_t kMagicLen = 4;
constexpr std::size_t kLengthFieldLen = 4;
constexpr std::size_t kHmacLen = crypto::kSha256DigestSize;      // 32
constexpr std::size_t kHeaderLen = kMagicLen + kLengthFieldLen;  // 8

// Chave de integridade DERIVADA, PROPRIA do mapa (contexto "map-hmac"). NAO e
// sigilo (o segredo-base vive no binario, extraivel por decompile): e separacao de
// dominio + soberania da origem da chave (nao aparece literal). Mesmo padrao do save.
const std::vector<std::uint8_t>& embedded_key() {
    static const std::vector<std::uint8_t> k = [] {
        static const std::string base = "gusengine-cpp-2026-integrity-base-secret";
        static const std::string ctx = "map-hmac";
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

void put_i32(std::vector<std::uint8_t>& out, std::int32_t v) {
    put_u32(out, static_cast<std::uint32_t>(v));
}

void put_u16(std::vector<std::uint8_t>& out, std::uint16_t v) {
    out.push_back(static_cast<std::uint8_t>(v & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((v >> 8) & 0xFFu));
}

void put_f32(std::vector<std::uint8_t>& out, float v) {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &v, sizeof(bits));
    put_u32(out, bits);
}

void put_string(std::vector<std::uint8_t>& out, const std::string& s) {
    put_u32(out, static_cast<std::uint32_t>(s.size()));
    out.insert(out.end(), s.begin(), s.end());
}

// ---- erros internos do decoder (mapeados para MapLoadResult) ---------------

// Mapa adulterado (HMAC nao bate).
struct MapIntegrityError {};
// Mapa malformado (magic/length/truncado/payload invalido).
struct MapCorruptError {
    std::string what;
};
// Mapa de versao futura (forward-only).
struct MapVersionTooNewError {
    int save_version;
    int current_version;
};

// ---- reader binario (little-endian), com bounds-check ----------------------

class Reader {
public:
    explicit Reader(const std::vector<std::uint8_t>& buf) : buf_(buf) {}

    std::uint16_t read_u16() {
        require(2, "u16");
        const std::uint16_t v =
            static_cast<std::uint16_t>(buf_[pos_]) |
            static_cast<std::uint16_t>(static_cast<std::uint16_t>(buf_[pos_ + 1])
                                       << 8);
        pos_ += 2;
        return v;
    }

    std::uint32_t read_u32() {
        require(4, "u32");
        const std::uint32_t v =
            static_cast<std::uint32_t>(buf_[pos_]) |
            (static_cast<std::uint32_t>(buf_[pos_ + 1]) << 8) |
            (static_cast<std::uint32_t>(buf_[pos_ + 2]) << 16) |
            (static_cast<std::uint32_t>(buf_[pos_ + 3]) << 24);
        pos_ += 4;
        return v;
    }

    std::int32_t read_i32() { return static_cast<std::int32_t>(read_u32()); }

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

    void expect_end() const {
        if (pos_ != buf_.size())
            throw MapCorruptError{
                "Payload de mapa tem bytes extras apos os campos esperados."};
    }

    // Maximo de elementos que CABEM no resto do buffer, dado o tamanho minimo de
    // cada um (CWE-789/CWE-130): rejeita count externo absurdo ANTES de reserve.
    std::uint32_t bounded_count(std::uint32_t count, std::size_t min_elem_bytes,
                                const char* what) const {
        if (min_elem_bytes != 0 &&
            static_cast<std::uint64_t>(count) * min_elem_bytes > remaining())
            throw MapCorruptError{std::string("Contagem implausivel em ") + what +
                                  " (excede bytes restantes do payload)."};
        return count;
    }

private:
    std::size_t remaining() const { return buf_.size() - pos_; }

    void require(std::size_t n, const char* what) const {
        if (pos_ + n > buf_.size())
            throw MapCorruptError{std::string("Payload de mapa truncado ao ler ") +
                                  what + "."};
    }

    const std::vector<std::uint8_t>& buf_;
    std::size_t pos_ = 0;
};

// ---- payload (layout v1) ---------------------------------------------------

std::vector<std::uint8_t> build_payload(const TileMap& map, int version) {
    std::vector<std::uint8_t> p;
    put_u32(p, static_cast<std::uint32_t>(version));
    // v2+: map_id (UUID textual) DENTRO do HMAC, logo apos a versao. v1 nao tinha.
    if (version >= 2) put_string(p, map.map_id());
    put_i32(p, map.width());
    put_i32(p, map.height());
    put_f32(p, map.tile_size());

    const auto& tiles = map.tiles();
    put_u32(p, static_cast<std::uint32_t>(tiles.size()));
    for (std::uint16_t t : tiles) put_u16(p, t);

    put_i32(p, map.spawn().x);
    put_i32(p, map.spawn().y);

    put_u32(p, static_cast<std::uint32_t>(map.portals().size()));
    for (const Portal& portal : map.portals()) {
        put_string(p, portal.id);
        put_i32(p, portal.cell.x);
        put_i32(p, portal.cell.y);
    }
    return p;
}

// ---- envelope (pack/unpack) ------------------------------------------------

std::vector<std::uint8_t> pack(const std::vector<std::uint8_t>& payload) {
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

// Valida o envelope e extrai o PAYLOAD. Lanca MapCorruptError (formato) ou
// MapIntegrityError (HMAC mismatch).
std::vector<std::uint8_t> unpack(const std::vector<std::uint8_t>& data) {
    if (data.size() < kHeaderLen + kHmacLen)
        throw MapCorruptError{"Dados de mapa curtos demais (header + hmac)."};

    for (std::size_t i = 0; i < kMagicLen; ++i) {
        if (data[i] != kMagic[i])
            throw MapCorruptError{"Magic bytes invalidos: nao e um mapa 'GMAP'."};
    }

    const std::uint32_t payload_len =
        static_cast<std::uint32_t>(data[kMagicLen]) |
        (static_cast<std::uint32_t>(data[kMagicLen + 1]) << 8) |
        (static_cast<std::uint32_t>(data[kMagicLen + 2]) << 16) |
        (static_cast<std::uint32_t>(data[kMagicLen + 3]) << 24);

    const std::uint64_t expected_total =
        static_cast<std::uint64_t>(kHeaderLen) + payload_len + kHmacLen;
    if (expected_total != data.size())
        throw MapCorruptError{
            "Length inconsistente: total declarado != tamanho real."};

    const std::size_t payload_end = kHeaderLen + payload_len;

    const auto expected = crypto::hmac_sha256(embedded_key().data(),
                                              embedded_key().size(), data.data(),
                                              payload_end);
    std::array<std::uint8_t, kHmacLen> actual{};
    std::memcpy(actual.data(), data.data() + payload_end, kHmacLen);
    if (!crypto::fixed_time_equals(expected, actual))
        throw MapIntegrityError{};

    return std::vector<std::uint8_t>(
        data.begin() + static_cast<std::ptrdiff_t>(kHeaderLen),
        data.begin() + static_cast<std::ptrdiff_t>(payload_end));
}

// Forward-only v1->v2: o v1 nao tinha map_id; o decode ja deixa o campo VAZIO para
// um payload v1, entao a migracao apenas confirma (sem binding). Gancho para campos
// aditivos futuros; mantem a forma "migrate(decoded, from_version)" do save.
TileMap migrate_to_current(TileMap decoded, int from_version) {
    if (from_version < 2) {
        // v1 -> v2: sem map_id na origem; carrega como mapa SEM binding (id vazio).
        // (decode_payload ja nao escreveu map_id para v1, entao nada a fazer; este
        //  bloco documenta a intencao e e o ponto de costura para futuras migracoes.)
        decoded.set_map_id("");
    }
    return decoded;
}

// Materializa o TileMap do payload (na versao lida). Pode lancar MapCorruptError
// (truncado/contagem absurda) ou std::invalid_argument (dims do construtor).
TileMap decode_payload(Reader& r, int version) {
    // v2+: map_id selado vem logo apos a versao (ja consumida pelo caller). v1 nao tem.
    std::string map_id;
    if (version >= 2) map_id = r.read_string();

    const std::int32_t width = r.read_i32();
    const std::int32_t height = r.read_i32();
    const float tile_size = r.read_f32();

    // O construtor valida dims/tile_size (fail-fast). Dims absurdas (selado mas
    // divergente) lancam std::invalid_argument -> mapeado para Invalid no caller.
    TileMap map(width, height, tile_size);

    const std::uint32_t tile_count = r.bounded_count(r.read_u32(), 2, "tiles");
    std::vector<std::uint16_t> tiles;
    tiles.reserve(tile_count);
    for (std::uint32_t i = 0; i < tile_count; ++i) tiles.push_back(r.read_u16());
    map.assign_tiles_raw(std::move(tiles));

    map.set_spawn(Cell{r.read_i32(), r.read_i32()});

    const std::uint32_t portal_count =
        r.bounded_count(r.read_u32(), 4 + 4 + 4, "portais");
    for (std::uint32_t i = 0; i < portal_count; ++i) {
        Portal portal;
        portal.id = r.read_string();
        portal.cell = Cell{r.read_i32(), r.read_i32()};
        map.add_portal(std::move(portal));
    }
    r.expect_end();

    map.set_map_id(std::move(map_id));  // vazio para v1; UUID selado para v2+
    return map;
}

}  // namespace

// ---- API publica -----------------------------------------------------------

std::vector<std::uint8_t> serialize_map(const TileMap& map) {
    map.validate();  // fail-fast antes de empacotar (std::invalid_argument)
    // v2 EXIGE identidade: serializar um mapa sem map_id e bug do chamador (o
    // compilador deve ter lido #map_id da fonte). Fail-fast, igual ao resto.
    if (map.map_id().empty())
        throw std::invalid_argument(
            "serialize_map: map_id vazio (v2 exige UUID de identidade; defina "
            "#map_id na fonte CSV).");
    return pack(build_payload(map, kMapSchemaVersion));
}

std::vector<std::uint8_t> serialize_map_with_version(const TileMap& map,
                                                     int version) {
    return pack(build_payload(map, version));
}

std::vector<std::uint8_t> forge_bad_dims_envelope() {
    // Payload v2 com width*height != tile_count: anuncia 2x2 mas grava 3 tiles.
    std::vector<std::uint8_t> p;
    put_u32(p, static_cast<std::uint32_t>(kMapSchemaVersion));
    put_string(p, "forged-bad-dims");  // v2: map_id apos a versao
    put_i32(p, 2);  // width
    put_i32(p, 2);  // height
    put_f32(p, 1.0f);
    put_u32(p, 3);  // tile_count divergente (esperado 4)
    put_u16(p, 0);
    put_u16(p, 1);
    put_u16(p, 0);
    put_i32(p, 0);  // spawn x
    put_i32(p, 0);  // spawn y
    put_u32(p, 0);  // sem portais
    return pack(p);  // selado corretamente
}

std::vector<std::uint8_t> forge_legacy_v1_envelope(const TileMap& map) {
    map.validate();  // o mapa-fonte tem de ser coerente (so o id e omitido)
    // build_payload com version=1 NAO escreve map_id (formato antigo), selado correto.
    return pack(build_payload(map, 1));
}

MapLoadOutcome load_map(const std::vector<std::uint8_t>& data,
                        const std::string& expected_map_id) {
    MapLoadOutcome out;
    std::vector<std::uint8_t> payload;
    try {
        // 1. Integridade primeiro: HMAC mismatch ANTES de ler versao/migrar.
        payload = unpack(data);
    } catch (const MapIntegrityError&) {
        out.result = MapLoadResult::HmacInvalid;
        return out;
    } catch (const MapCorruptError&) {
        out.result = MapLoadResult::Corrupt;
        return out;
    }

    try {
        Reader r(payload);
        const int version = static_cast<int>(r.read_u32());

        // 3. Versao futura: forward-only nao le o futuro.
        if (version > kMapSchemaVersion)
            throw MapVersionTooNewError{version, kMapSchemaVersion};
        if (version < 1)
            throw MapCorruptError{"schema_version invalido (< 1): " +
                                  std::to_string(version)};

        // 4-5. Materializa (na versao lida), migra ate atual, valida invariantes.
        TileMap decoded = decode_payload(r, version);
        TileMap migrated = migrate_to_current(std::move(decoded), version);
        migrated.validate();  // defesa em profundidade pos-HMAC

        // BINDING DE IDENTIDADE: se o chamador informou um expected_map_id, o map_id
        // SELADO tem de bater. Divergencia (com selo OK) = map-swap -> IdentityMismatch.
        // expected vazio = sem binding (so HMAC). O map_id vive dentro do HMAC, entao
        // adulterar o id ja teria caido em HmacInvalid antes de chegar aqui.
        if (!expected_map_id.empty() && migrated.map_id() != expected_map_id) {
            out.result = MapLoadResult::IdentityMismatch;
            return out;
        }

        out.map = std::move(migrated);
        out.result = MapLoadResult::Ok;
        return out;
    } catch (const MapVersionTooNewError&) {
        out.result = MapLoadResult::VersionTooNew;
        return out;
    } catch (const MapCorruptError&) {
        out.result = MapLoadResult::Corrupt;
        return out;
    } catch (const std::invalid_argument&) {
        // Payload bem-formado e selado, mas schema-divergente (dims/matriz/meta).
        out.result = MapLoadResult::Invalid;
        return out;
    } catch (const std::out_of_range&) {
        out.result = MapLoadResult::Corrupt;
        return out;
    }
}

}  // namespace gus::domain::map
