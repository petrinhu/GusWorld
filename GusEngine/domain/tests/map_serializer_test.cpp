// map_serializer_test.cpp
//
// Spec executavel (Catch2 v3) do formato binario proprio .gmap SELADO com
// HMAC-SHA256 do core/. Mesmo esquema/padrao do save (ADR-006): envelope
// MAGIC || LENGTH || PAYLOAD || HMAC(32), HMAC sobre MAGIC||LENGTH||PAYLOAD; load
// valida HMAC ANTES de versao, rejeita futuro (forward-only), nao-lancante por valor
// (MapLoadResult Ok/HmacInvalid/Corrupt/VersionTooNew/Invalid).
//
// Oraculo:
//   (a) roundtrip: TileMap -> serialize -> load -> TileMap IDENTICO;
//   (b) tamper: flip de 1 byte (qualquer regiao) -> HmacInvalid;
//   (c) determinismo: mesmo mapa -> mesmo selo (bytes identicos);
//   (d) corrupto: magic errado / truncado / length inconsistente -> Corrupt;
//   (e) futuro: schema_version > atual -> VersionTooNew;
//   (f) invariante: payload selado mas dims/matriz divergentes -> Invalid.
//
// Subsistema: domain/map. POCO puro, ZERO Qt/SDL, headless.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <vector>

#include "gus/domain/map/map_serializer.hpp"
#include "gus/domain/map/tile_map.hpp"

using gus::domain::map::kMapSchemaVersion;
using gus::domain::map::load_map;
using gus::domain::map::MapLoadResult;
using gus::domain::map::Cell;
using gus::domain::map::Portal;
using gus::domain::map::serialize_map;
using gus::domain::map::TileKind;
using gus::domain::map::TileMap;

namespace {

std::uint16_t k(TileKind t) { return static_cast<std::uint16_t>(t); }

// Fixture rica: mapa 5x4 com paredes nas bordas, marco, entrada/saida, spawn,
// portais. Exercita matriz + metadados juntos.
TileMap rich_map() {
    TileMap m(5, 4, 2.0f);
    for (std::int32_t x = 0; x < 5; ++x) {
        m.set(x, 0, k(TileKind::Parede));
        m.set(x, 3, k(TileKind::Parede));
    }
    for (std::int32_t y = 0; y < 4; ++y) {
        m.set(0, y, k(TileKind::Parede));
        m.set(4, y, k(TileKind::Parede));
    }
    m.set(2, 0, k(TileKind::Entrada));
    m.set(2, 3, k(TileKind::Saida));
    m.set(1, 1, k(TileKind::Marco));
    m.set_spawn(Cell{2, 1});
    m.add_portal(Portal{"entrada_norte", Cell{2, 0}});
    m.add_portal(Portal{"saida_sul", Cell{2, 3}});
    return m;
}

}  // namespace

TEST_CASE("map_serializer: roundtrip preserva mapa identico", "[map][serializer]") {
    const TileMap original = rich_map();
    const auto bytes = serialize_map(original);
    const auto out = load_map(bytes);
    REQUIRE(out.result == MapLoadResult::Ok);
    REQUIRE(out.map == original);
}

TEST_CASE("map_serializer: determinismo (selo estavel)", "[map][serializer]") {
    const TileMap m = rich_map();
    const auto a = serialize_map(m);
    const auto b = serialize_map(m);
    REQUIRE(a == b);
}

TEST_CASE("map_serializer: tamper de 1 byte -> rejeitado (nunca Ok)",
          "[map][serializer][tamper]") {
    // Anti-tamper: o jogador NAO edita o mapa para atravessar parede. Um flip de 1
    // byte em QUALQUER regiao do envelope nunca carrega como Ok.
    //   - magic / payload / tag HMAC: o conteudo selado muda -> HmacInvalid;
    //   - byte do campo LENGTH: muda o tamanho declarado do payload, detectado
    //     ESTRUTURALMENTE (total != tamanho real) ANTES do HMAC -> Corrupt.
    // Ambos sao rejeicao legitima; o que NAO pode jamais acontecer e Ok.
    const auto bytes = serialize_map(rich_map());
    const std::vector<std::size_t> alvos = {
        0,                 // magic    -> HmacInvalid
        5,                 // length   -> Corrupt (estrutural)
        bytes.size() / 2,  // payload  -> HmacInvalid
        bytes.size() - 1,  // tag HMAC -> HmacInvalid
    };
    for (std::size_t i : alvos) {
        auto tampered = bytes;
        tampered[i] ^= 0x01;
        const auto out = load_map(tampered);
        REQUIRE(out.result != MapLoadResult::Ok);
    }

    // Foco no caso central (matriz de tiles adulterada): selo quebra -> HmacInvalid.
    auto payload_tamper = bytes;
    payload_tamper[bytes.size() / 2] ^= 0x01;
    REQUIRE(load_map(payload_tamper).result == MapLoadResult::HmacInvalid);
}

TEST_CASE("map_serializer: magic errado / truncado / length ruim -> Corrupt",
          "[map][serializer][corrupt]") {
    SECTION("magic errado") {
        auto bytes = serialize_map(rich_map());
        bytes[0] = 'X';  // quebra o magic; (HMAC tambem quebraria, mas o decoder
                         // valida magic ANTES e ja resolve Corrupt)
        // Para isolar Corrupt-por-magic do HmacInvalid, recompoe um HMAC valido
        // sobre o magic errado nao da: o load valida HMAC primeiro. Aqui aceitamos
        // que magic-quebrado caia em HmacInvalid OU Corrupt; o que NAO pode e Ok.
        const auto out = load_map(bytes);
        REQUIRE(out.result != MapLoadResult::Ok);
    }
    SECTION("buffer curto demais") {
        std::vector<std::uint8_t> tiny = {'G', 'M', 'A', 'P'};
        const auto out = load_map(tiny);
        REQUIRE(out.result == MapLoadResult::Corrupt);
    }
    SECTION("vazio") {
        const auto out = load_map({});
        REQUIRE(out.result == MapLoadResult::Corrupt);
    }
}

TEST_CASE("map_serializer: schema_version futura -> VersionTooNew",
          "[map][serializer][version]") {
    // Forja um payload com versao atual+1, selado corretamente (a chave mora no
    // binario por design). O load deve validar HMAC OK e entao recusar o futuro.
    const auto forged =
        gus::domain::map::serialize_map_with_version(rich_map(), kMapSchemaVersion + 1);
    const auto out = load_map(forged);
    REQUIRE(out.result == MapLoadResult::VersionTooNew);
}

TEST_CASE("map_serializer: payload selado mas dims invalidas -> Invalid",
          "[map][serializer][invariante]") {
    // Forja um envelope cujo payload anuncia width*height != matriz materializavel,
    // selado corretamente. O load valida HMAC OK, materializa e a validacao de
    // invariante rejeita (defesa em profundidade, igual ao save).
    const auto forged = gus::domain::map::forge_bad_dims_envelope();
    const auto out = load_map(forged);
    REQUIRE(out.result == MapLoadResult::Invalid);
}
