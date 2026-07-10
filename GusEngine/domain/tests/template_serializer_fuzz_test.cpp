// template_serializer_fuzz_test.cpp
//
// REFORCO DE QA (nao-bloqueante) do DECODER de templates GDT1. Os arquivos .gdt sao
// dado externo (assets no disco; em teoria mod-friendly no futuro), entao o
// deserialize recebe bytes NAO-CONFIAVEIS. Este teste alimenta entradas MALFORMADAS
// e exige REJEICAO com ERRO TIPADO (TemplateCorruptError / TemplateIntegrityError /
// std::invalid_argument), NUNCA crash, UB, leitura fora-de-limite ou aceitacao-de-lixo.
//
// Determinismo: gerador aleatorio com SEED FIXA (kSeed).
//
// NAO altera codigo de producao. Achados (ex.: enum fora de range aceito) estao
// documentados em docs/auditoria/AUDIT-M3-2026-06-22/auditoria_qa_fuzzing.md.
//
// Subsistema: domain/templates (marco M3). POCO puro, ZERO Qt, headless.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

#include "gus/domain/templates/character_template.hpp"
#include "gus/domain/templates/enemy_template.hpp"
#include "gus/domain/templates/template_serializer.hpp"

using gus::domain::templates::BrainKind;
using gus::domain::templates::CardFamily;
using gus::domain::templates::CharacterTemplate;
using gus::domain::templates::deserialize_character;
using gus::domain::templates::deserialize_enemy;
using gus::domain::templates::EnemyTemplate;
using gus::domain::templates::pack;
using gus::domain::templates::serialize_character;
using gus::domain::templates::serialize_enemy;
using gus::domain::templates::TemplateCorruptError;

namespace {

constexpr std::uint32_t kSeed = 0x1B0CA17Eu;  // seed FIXA: determinismo total

CharacterTemplate char_fixture() {
    return CharacterTemplate{"gus",  34, 8, 5, 9, CardFamily::Eletrico,
                             true,   {"pulso_eletrico", "scan_basico"}};
}

EnemyTemplate enemy_fixture() {
    return EnemyTemplate{"daemon_guard",      144,   11, 14, 6, CardFamily::Cinetico,
                         BrainKind::Scripted, false, {"pulso_cinetico"}};
}

void put_u32_le(std::vector<std::uint8_t>& out, std::uint32_t v) {
    out.push_back(static_cast<std::uint8_t>(v & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((v >> 8) & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((v >> 16) & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((v >> 24) & 0xFFu));
}

template <typename Fn>
bool rejects_with_any_exception(Fn&& fn) {
    try {
        fn();
        return false;
    } catch (const std::exception&) {
        return true;
    } catch (...) {
        return true;
    }
}

}  // namespace

// ---- curto demais / vazio (boundary inferior) ------------------------------

TEST_CASE("tpl/fuzz: buffer vazio rejeita tipado", "[domain][templates][fuzz]") {
    REQUIRE_THROWS_AS(deserialize_character({}), TemplateCorruptError);
    REQUIRE_THROWS_AS(deserialize_enemy({}), TemplateCorruptError);
}

TEST_CASE("tpl/fuzz: menores que header+hmac rejeitam tipado",
          "[domain][templates][fuzz]") {
    for (std::size_t n = 1; n < 40; ++n) {
        std::vector<std::uint8_t> buf(n, 0xCDu);
        INFO("len=" << n);
        REQUIRE_THROWS_AS(deserialize_character(buf), TemplateCorruptError);
        REQUIRE_THROWS_AS(deserialize_enemy(buf), TemplateCorruptError);
    }
}

// ---- truncamento em CADA offset (boundary) ---------------------------------

TEST_CASE("tpl/fuzz: truncar character em cada offset rejeita, nunca crash",
          "[domain][templates][fuzz]") {
    const auto good = serialize_character(char_fixture());
    for (std::size_t len = 0; len < good.size(); ++len) {
        std::vector<std::uint8_t> t(good.begin(),
                                    good.begin() + static_cast<std::ptrdiff_t>(len));
        INFO("char truncado em len=" << len);
        REQUIRE(rejects_with_any_exception(
            [&] { (void)deserialize_character(t); }));
    }
}

TEST_CASE("tpl/fuzz: truncar enemy em cada offset rejeita, nunca crash",
          "[domain][templates][fuzz]") {
    const auto good = serialize_enemy(enemy_fixture());
    for (std::size_t len = 0; len < good.size(); ++len) {
        std::vector<std::uint8_t> t(good.begin(),
                                    good.begin() + static_cast<std::ptrdiff_t>(len));
        INFO("enemy truncado em len=" << len);
        REQUIRE(rejects_with_any_exception(
            [&] { (void)deserialize_enemy(t); }));
    }
}

// ---- magic errado (equivalence) --------------------------------------------

TEST_CASE("tpl/fuzz: cada byte de magic corrompido rejeita como corrupcao",
          "[domain][templates][fuzz]") {
    for (std::size_t i = 0; i < 4; ++i) {
        auto bytes = serialize_character(char_fixture());
        bytes[i] ^= 0xFFu;
        INFO("magic byte " << i);
        REQUIRE_THROWS_AS(deserialize_character(bytes), TemplateCorruptError);
    }
}

TEST_CASE("tpl/fuzz: magic de save (GDS3, ADR-015) num template rejeita "
          "(dominio cruzado)",
          "[domain][templates][fuzz]") {
    auto bytes = serialize_character(char_fixture());
    bytes[0] = 'G';
    bytes[1] = 'D';
    bytes[2] = 'S';
    bytes[3] = '3';
    REQUIRE_THROWS_AS(deserialize_character(bytes), TemplateCorruptError);
}

// ---- length: gigante / zero / off-by-one (equivalence + boundary) ----------

TEST_CASE("tpl/fuzz: length gigante (0xFFFFFFFF) rejeita sem alocar",
          "[domain][templates][fuzz]") {
    std::vector<std::uint8_t> buf = {'G', 'D', 'T', '1'};
    put_u32_le(buf, 0xFFFFFFFFu);
    buf.insert(buf.end(), 32, 0x00u);
    REQUIRE_THROWS_AS(deserialize_character(buf), TemplateCorruptError);
}

TEST_CASE("tpl/fuzz: length off-by-one rejeita",
          "[domain][templates][fuzz]") {
    const auto good = serialize_character(char_fixture());
    const std::uint32_t real_len = static_cast<std::uint32_t>(good.size() - 8u - 32u);
    for (std::uint32_t delta : {static_cast<std::uint32_t>(real_len - 1u),
                                static_cast<std::uint32_t>(real_len + 1u)}) {
        auto bytes = good;
        bytes[4] = static_cast<std::uint8_t>(delta & 0xFFu);
        bytes[5] = static_cast<std::uint8_t>((delta >> 8) & 0xFFu);
        bytes[6] = static_cast<std::uint8_t>((delta >> 16) & 0xFFu);
        bytes[7] = static_cast<std::uint8_t>((delta >> 24) & 0xFFu);
        INFO("length declarado=" << delta);
        REQUIRE_THROWS_AS(deserialize_character(bytes), TemplateCorruptError);
    }
}

// ---- HMAC valido mas string-len / deck-count internos gigantes -------------

TEST_CASE("tpl/fuzz: payload HMAC-valido com id-len gigante rejeita",
          "[domain][templates][fuzz]") {
    // 1o campo do payload e o id (u32 len | bytes). len gigante sem os bytes deve
    // barrar em require(), sem alocar a string.
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 0xFFFFFFF0u);        // id_len GIGANTE
    const auto packed = pack(payload);       // HMAC valido sobre payload mentiroso
    REQUIRE(rejects_with_any_exception(
        [&] { (void)deserialize_character(packed); }));
}

TEST_CASE("tpl/fuzz: payload HMAC-valido com deck-count gigante rejeita",
          "[domain][templates][fuzz]") {
    // id valido + stats + family + flag, depois deck_count gigante sem cartas.
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 3u);                 // id_len = 3
    payload.push_back('g'); payload.push_back('u'); payload.push_back('s');
    put_u32_le(payload, 34u);                // max_hp
    put_u32_le(payload, 8u);                 // atk
    put_u32_le(payload, 5u);                 // def
    put_u32_le(payload, 9u);                 // spd
    put_u32_le(payload, 0u);                 // family = Eletrico
    payload.push_back(0u);                   // is_universal_compiler = false
    put_u32_le(payload, 0xFFFFFFFFu);        // deck_count GIGANTE

    const auto packed = pack(payload);
    REQUIRE(rejects_with_any_exception(
        [&] { (void)deserialize_character(packed); }));
}

// ---- IMP-01: deck-count gigante rejeita TIPADO ANTES de alocar -------------
//
// Defesa em profundidade (auditoria_seguranca_crypto.md IMP-01, CWE-789): um .gdt
// SELADO (HMAC VALIDO, atacante tem a chave embarcada por design) cujo deck_count
// afirma ~4 bilhoes de cartas. Antes do fix, read_deck chamava reserve(count) ANTES
// de checar bytes restantes -> bad_alloc/length_error (crash). Apos o fix,
// bounded_count rejeita com TemplateCorruptError ANTES de qualquer alocacao grande
// (cada carta custa >= 4 bytes de length, entao count > remaining()/4 e implausivel).

TEST_CASE("tpl/IMP-01: deck-count gigante rejeita TemplateCorruptError antes de alocar",
          "[domain][templates][fuzz][imp01]") {
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 3u);                 // id_len = 3
    payload.push_back('g'); payload.push_back('u'); payload.push_back('s');
    put_u32_le(payload, 34u);                // max_hp
    put_u32_le(payload, 8u);                 // atk
    put_u32_le(payload, 5u);                 // def
    put_u32_le(payload, 9u);                 // spd
    put_u32_le(payload, 0u);                 // family = Eletrico
    payload.push_back(0u);                   // is_universal_compiler = false
    put_u32_le(payload, 0xFFFFFFFFu);        // deck_count GIGANTE

    const auto packed = pack(payload);
    REQUIRE_THROWS_AS(deserialize_character(packed), TemplateCorruptError);
}

// ---- HMAC valido + bem-formado mas invariante violado (validate no load) ---

TEST_CASE("tpl/fuzz: character com max_hp=0 (HMAC valido) rejeita por validate",
          "[domain][templates][fuzz]") {
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 3u);
    payload.push_back('g'); payload.push_back('u'); payload.push_back('s');
    put_u32_le(payload, 0u);                 // max_hp = 0 -> invariante max_hp>0 viola
    put_u32_le(payload, 0u);                 // atk
    put_u32_le(payload, 0u);                 // def
    put_u32_le(payload, 0u);                 // spd
    put_u32_le(payload, 0u);                 // family
    payload.push_back(0u);                   // flag
    put_u32_le(payload, 0u);                 // deck_count = 0
    const auto packed = pack(payload);
    REQUIRE_THROWS_AS(deserialize_character(packed), std::invalid_argument);
}

TEST_CASE("tpl/fuzz: character com atk negativo (bit-pattern) rejeita por validate",
          "[domain][templates][fuzz]") {
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 3u);
    payload.push_back('g'); payload.push_back('u'); payload.push_back('s');
    put_u32_le(payload, 10u);                // max_hp ok
    put_u32_le(payload, 0xFFFFFFFFu);        // atk = -1 (i32) -> viola atk>=0
    put_u32_le(payload, 0u);                 // def
    put_u32_le(payload, 0u);                 // spd
    put_u32_le(payload, 0u);                 // family
    payload.push_back(0u);
    put_u32_le(payload, 0u);
    const auto packed = pack(payload);
    REQUIRE_THROWS_AS(deserialize_character(packed), std::invalid_argument);
}

TEST_CASE("tpl/fuzz: character com id vazio (HMAC valido) rejeita por validate",
          "[domain][templates][fuzz]") {
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 0u);                 // id_len = 0 -> id vazio viola invariante
    put_u32_le(payload, 10u);                // max_hp
    put_u32_le(payload, 0u);                 // atk
    put_u32_le(payload, 0u);                 // def
    put_u32_le(payload, 0u);                 // spd
    put_u32_le(payload, 0u);                 // family
    payload.push_back(0u);
    put_u32_le(payload, 0u);
    const auto packed = pack(payload);
    REQUIRE_THROWS_AS(deserialize_character(packed), std::invalid_argument);
}

TEST_CASE("tpl/fuzz: cauda extra apos os campos (HMAC valido) rejeita",
          "[domain][templates][fuzz]") {
    // payload bem-formado de char + 1 byte de lixo na cauda; expect_end() barra.
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 3u);
    payload.push_back('g'); payload.push_back('u'); payload.push_back('s');
    put_u32_le(payload, 10u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    payload.push_back(0u);
    put_u32_le(payload, 0u);                 // deck_count = 0
    payload.push_back(0x99u);                // CAUDA EXTRA
    const auto packed = pack(payload);
    REQUIRE_THROWS_AS(deserialize_character(packed), TemplateCorruptError);
}

// ---- A1 (auditoria M3): enum family/brain fora de range agora REJEITA no load ----------
//
// FECHADO no chunk 4 do M5 (A1): o validate() dos templates passou a checar o ORDINAL de
// family/brain (alem de id/stats/deck), e templates::CardFamily foi religado a fonte
// canonica do combate. Um payload HMAC-valido com family=9999 desserializa, chama
// validate() no fim, e e REJEITADO com std::invalid_argument (defesa contra .gdt selado
// mas schema-divergente). Antes do A1 isto era ACEITO silenciosamente (achado documentado
// na auditoria QA). A virada de REQUIRE_NOTHROW para REQUIRE_THROWS_AS aqui e a melhoria
// prometida, NAO uma regressao.

TEST_CASE("tpl/A1: family fora de range REJEITA no load (validate de ordinal)",
          "[domain][templates][fuzz][a1]") {
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 3u);
    payload.push_back('g'); payload.push_back('u'); payload.push_back('s');
    put_u32_le(payload, 10u);                // max_hp ok
    put_u32_le(payload, 0u);                 // atk
    put_u32_le(payload, 0u);                 // def
    put_u32_le(payload, 0u);                 // spd
    put_u32_le(payload, 9999u);              // family FORA do range {0..4}
    payload.push_back(0u);
    put_u32_le(payload, 0u);                 // deck_count = 0
    const auto packed = pack(payload);

    // A1: validate() no load rejeita o ordinal fora do dominio. Nao crasha; erro TIPADO.
    REQUIRE_THROWS_AS(deserialize_character(packed), std::invalid_argument);
}

// ---- bytes aleatorios deterministicos (seed FIXA) --------------------------

TEST_CASE("tpl/fuzz: 2000 buffers aleatorios (seed fixa) nunca crasham",
          "[domain][templates][fuzz]") {
    std::mt19937 rng(kSeed);
    std::uniform_int_distribution<int> byte_dist(0, 255);
    std::uniform_int_distribution<int> len_dist(0, 256);

    for (int iter = 0; iter < 2000; ++iter) {
        const int n = len_dist(rng);
        std::vector<std::uint8_t> buf;
        buf.reserve(static_cast<std::size_t>(n));
        for (int i = 0; i < n; ++i)
            buf.push_back(static_cast<std::uint8_t>(byte_dist(rng)));
        INFO("iter=" << iter << " len=" << n);
        try { (void)deserialize_character(buf); } catch (const std::exception&) {}
        try { (void)deserialize_enemy(buf); } catch (const std::exception&) {}
    }
    SUCCEED("4000 desserializacoes aleatorias (char+enemy) sem crash/UB");
}

// ---- bytes aleatorios DENTRO de envelope HMAC-valido -----------------------

TEST_CASE("tpl/fuzz: payloads aleatorios HMAC-validos nunca crasham",
          "[domain][templates][fuzz]") {
    std::mt19937 rng(kSeed ^ 0xABCDu);
    std::uniform_int_distribution<int> byte_dist(0, 255);
    std::uniform_int_distribution<int> len_dist(0, 200);

    int accepted = 0;
    for (int iter = 0; iter < 2000; ++iter) {
        const int n = len_dist(rng);
        std::vector<std::uint8_t> payload;
        payload.reserve(static_cast<std::size_t>(n));
        for (int i = 0; i < n; ++i)
            payload.push_back(static_cast<std::uint8_t>(byte_dist(rng)));
        const auto packed = pack(payload);
        INFO("iter=" << iter << " payload_len=" << n);
        try { (void)deserialize_character(packed); ++accepted; }
        catch (const std::exception&) {}
    }
    INFO("payloads aleatorios aceitos como character valido: " << accepted);
    SUCCEED("2000 payloads aleatorios HMAC-validos sem crash/UB");
}
