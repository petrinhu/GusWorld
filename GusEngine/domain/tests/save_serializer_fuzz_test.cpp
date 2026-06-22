// save_serializer_fuzz_test.cpp
//
// REFORCO DE QA (nao-bloqueante) do DECODER de save GDS2. O deserialize binario
// recebe bytes NAO-CONFIAVEIS (arquivo no disco do user, possivelmente truncado,
// corrompido por FS, ou adulterado de proposito). Este teste alimenta entradas
// MALFORMADAS e exige REJEICAO com ERRO TIPADO (SaveCorruptError / SaveIntegrityError
// / SaveVersionTooNewError / std::invalid_argument), NUNCA crash, UB, leitura
// fora-de-limite ou aceitacao-de-lixo.
//
// Determinismo: o gerador aleatorio usa SEED FIXA (kSeed). Mesmo input em toda run.
//
// Tecnica: boundary values (truncamento em cada offset), equivalence partitioning
// (magic / length / version), e fuzzing dirigido (bytes aleatorios deterministicos +
// payloads HMAC-validos com campos invalidos / counts gigantes).
//
// NAO altera codigo de producao. Se um caso AQUI provar crash/UB/aceitacao-de-lixo,
// e um achado para a thread principal levar ao lider (ver
// docs/auditoria/AUDIT-M3-2026-06-22/auditoria_qa_fuzzing.md).
//
// Subsistema: domain/save (marco M3). POCO puro, ZERO Qt, headless.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

#include "gus/domain/save/save_serializer.hpp"

using gus::domain::save::deserialize_save;
using gus::domain::save::pack_save;
using gus::domain::save::SaveCorruptError;
using gus::domain::save::SaveData;
using gus::domain::save::SaveIntegrityError;
using gus::domain::save::SaveVersionTooNewError;
using gus::domain::save::serialize_save;

namespace {

constexpr std::uint32_t kSeed = 0x6755D2A6u;  // seed FIXA: determinismo total

// Save valido minimo so com o necessario para serializar e roundtrippar.
SaveData seed_fixture() {
    SaveData s;
    s.schema_version = 3;
    s.timestamp_ms = 1718900000123LL;
    s.current_scene_path = "res://world/gusworld_city.tscn";
    s.party_roster = {"gus", "caua"};
    s.inventory = {{"credito", 89}};
    return s;
}

// Helper: escreve um u32 LE no fim de um buffer.
void put_u32_le(std::vector<std::uint8_t>& out, std::uint32_t v) {
    out.push_back(static_cast<std::uint8_t>(v & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((v >> 8) & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((v >> 16) & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((v >> 24) & 0xFFu));
}

// "QUALQUER excecao std::, mas NUNCA crash/UB". O contrato IDEAL e erro tipado do
// dominio; aceitamos tambem std:: (length_error/bad_alloc/invalid_argument) como
// "rejeitou sem crashar". O importante deste predicado: a funcao SEMPRE termina
// lancando algo capturavel, nunca retorna lixo nem derruba o processo (o que o
// AddressSanitizer/UBSan do CI pegaria se houvesse OOB).
template <typename Fn>
bool rejects_with_any_exception(Fn&& fn) {
    try {
        fn();
        return false;  // ACEITOU: nao lancou -> potencial aceitacao-de-lixo
    } catch (const std::exception&) {
        return true;
    } catch (...) {
        return true;
    }
}

}  // namespace

// ---- buffer vazio / curto demais (boundary inferior) -----------------------

TEST_CASE("save/fuzz: buffer vazio rejeita tipado", "[domain][save][fuzz]") {
    REQUIRE_THROWS_AS(deserialize_save({}), SaveCorruptError);
}

TEST_CASE("save/fuzz: buffers menores que header+hmac rejeitam tipado",
          "[domain][save][fuzz]") {
    // header(8) + hmac(32) = 40 bytes minimos. Tudo abaixo e corrupcao estrutural.
    for (std::size_t n = 1; n < 40; ++n) {
        std::vector<std::uint8_t> buf(n, 0xABu);
        REQUIRE_THROWS_AS(deserialize_save(buf), SaveCorruptError);
    }
}

// ---- truncamento em CADA offset de um save valido (boundary) ---------------

TEST_CASE("save/fuzz: truncar em cada offset rejeita, nunca crash",
          "[domain][save][fuzz]") {
    const auto good = serialize_save(seed_fixture());
    // Para cada prefixo de comprimento 0..size-1, o decoder deve rejeitar com
    // alguma excecao (corrupcao/integridade), jamais crashar ou aceitar.
    for (std::size_t len = 0; len < good.size(); ++len) {
        std::vector<std::uint8_t> truncated(good.begin(),
                                            good.begin() + static_cast<std::ptrdiff_t>(len));
        INFO("truncado em len=" << len);
        REQUIRE(rejects_with_any_exception(
            [&] { (void)deserialize_save(truncated); }));
    }
}

// ---- magic errado (equivalence partitioning) -------------------------------

TEST_CASE("save/fuzz: cada byte de magic corrompido rejeita como corrupcao",
          "[domain][save][fuzz]") {
    for (std::size_t i = 0; i < 4; ++i) {
        auto bytes = serialize_save(seed_fixture());
        bytes[i] ^= 0xFFu;
        INFO("magic byte " << i << " corrompido");
        REQUIRE_THROWS_AS(deserialize_save(bytes), SaveCorruptError);
    }
}

TEST_CASE("save/fuzz: magic de template (GDT1) num save rejeita",
          "[domain][save][fuzz]") {
    auto bytes = serialize_save(seed_fixture());
    bytes[0] = 'G';
    bytes[1] = 'D';
    bytes[2] = 'T';
    bytes[3] = '1';  // magic de TEMPLATE, dominio cruzado
    REQUIRE_THROWS_AS(deserialize_save(bytes), SaveCorruptError);
}

// ---- length: gigante / zero / overflow (equivalence + boundary) ------------

TEST_CASE("save/fuzz: length declarado gigante (0xFFFFFFFF) rejeita sem alocar",
          "[domain][save][fuzz]") {
    // Constroi um buffer pequeno cujo campo LENGTH afirma ~4 GiB de payload. O
    // unpack deve detectar a inconsistencia (expected_total != data.size()) ANTES
    // de qualquer leitura/alocacao guiada por esse length.
    std::vector<std::uint8_t> buf = {'G', 'D', 'S', '2'};
    put_u32_le(buf, 0xFFFFFFFFu);          // LENGTH gigante
    buf.insert(buf.end(), 32, 0x00u);       // hmac placeholder (tamanho minimo)
    REQUIRE_THROWS_AS(deserialize_save(buf), SaveCorruptError);
}

TEST_CASE("save/fuzz: length declarado zero num buffer maior rejeita",
          "[domain][save][fuzz]") {
    std::vector<std::uint8_t> buf = {'G', 'D', 'S', '2'};
    put_u32_le(buf, 0u);                     // LENGTH = 0 mas ha payload real
    buf.insert(buf.end(), 16, 0x11u);        // bytes a mais
    buf.insert(buf.end(), 32, 0x00u);        // hmac placeholder
    REQUIRE_THROWS_AS(deserialize_save(buf), SaveCorruptError);
}

TEST_CASE("save/fuzz: length off-by-one (real-1 e real+1) rejeita",
          "[domain][save][fuzz]") {
    const auto good = serialize_save(seed_fixture());
    const std::uint32_t real_len = static_cast<std::uint32_t>(good.size() - 8u - 32u);
    for (std::uint32_t delta : {static_cast<std::uint32_t>(real_len - 1u),
                                static_cast<std::uint32_t>(real_len + 1u)}) {
        auto bytes = good;
        bytes[4] = static_cast<std::uint8_t>(delta & 0xFFu);
        bytes[5] = static_cast<std::uint8_t>((delta >> 8) & 0xFFu);
        bytes[6] = static_cast<std::uint8_t>((delta >> 16) & 0xFFu);
        bytes[7] = static_cast<std::uint8_t>((delta >> 24) & 0xFFu);
        INFO("length declarado=" << delta << " real=" << real_len);
        REQUIRE_THROWS_AS(deserialize_save(bytes), SaveCorruptError);
    }
}

// ---- HMAC valido mas payload com COUNT/LEN interno gigante ------------------
//
// O caso mais perigoso: o envelope e CONSISTENTE (length casa, HMAC bate sobre o
// payload mentiroso) mas um count interno (ex.: tamanho de uma string ou de uma
// lista) afirma bilhoes de elementos. O reader DEVE rejeitar via require()
// (truncamento) e nao tentar reservar/alocar memoria absurda guiada pelo count.

TEST_CASE("save/fuzz: payload HMAC-valido com string-len gigante rejeita",
          "[domain][save][fuzz]") {
    // Payload: u32 version(1) | i64 timestamp | f64 playtime | u32 scene_len GIGANTE
    // ... o reader le scene_len e o require() deve falhar (faltam bytes), lancando
    // SaveCorruptError, sem alocar a string gigante.
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 1u);                 // schema_version = 1 (layout minimo)
    payload.insert(payload.end(), 8, 0x00u); // timestamp_ms (i64)
    payload.insert(payload.end(), 8, 0x00u); // playtime_seconds (f64)
    put_u32_le(payload, 0xFFFFFFF0u);        // current_scene_path len GIGANTE
    // (sem os bytes da string: require() deve barrar)

    const auto packed = pack_save(payload);  // HMAC valido sobre o payload mentiroso
    REQUIRE(rejects_with_any_exception(
        [&] { (void)deserialize_save(packed); }));
}

TEST_CASE("save/fuzz: payload HMAC-valido com list-count gigante rejeita",
          "[domain][save][fuzz]") {
    // Apos os campos escalares, party_roster e um list<str> cujo COUNT u32 afirma
    // ~4 bilhoes de strings. O reader nao deve travar/alocar; deve rejeitar quando
    // faltarem bytes para a 1a string.
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 1u);                 // schema_version = 1
    payload.insert(payload.end(), 8, 0x00u); // timestamp_ms
    payload.insert(payload.end(), 8, 0x00u); // playtime_seconds
    put_u32_le(payload, 0u);                 // current_scene_path len = 0 (string vazia)
    // player_position(3 f64) + player_rotation(3 f64) = 6 doubles = 48 bytes zerados.
    payload.insert(payload.end(), 48, 0x00u);
    put_u32_le(payload, 0xFFFFFFFFu);        // party_roster count GIGANTE

    const auto packed = pack_save(payload);
    REQUIRE(rejects_with_any_exception(
        [&] { (void)deserialize_save(packed); }));
}

// ---- version: futura / zero / negativa-bit-pattern -------------------------

TEST_CASE("save/fuzz: version futura rejeita tipado (forward-only)",
          "[domain][save][fuzz]") {
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 9999u);              // version muito futura
    const auto packed = pack_save(payload);  // HMAC valido; version ilegal
    REQUIRE_THROWS_AS(deserialize_save(packed), SaveVersionTooNewError);
}

TEST_CASE("save/fuzz: version zero rejeita como corrupcao",
          "[domain][save][fuzz]") {
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 0u);                 // version < 1: invalida
    const auto packed = pack_save(payload);
    REQUIRE_THROWS_AS(deserialize_save(packed), SaveCorruptError);
}

TEST_CASE("save/fuzz: version 0xFFFFFFFF (negativa como int) rejeita tipado",
          "[domain][save][fuzz]") {
    // read_u32 -> static_cast<int> = -1. O codigo trata version<1 como corrupcao;
    // (e nunca como > current, pois -1 nao e > atual). Deve rejeitar, nao crashar.
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 0xFFFFFFFFu);
    const auto packed = pack_save(payload);
    REQUIRE(rejects_with_any_exception(
        [&] { (void)deserialize_save(packed); }));
}

// ---- HMAC valido + version OK mas payload truncado no meio dos campos -------

TEST_CASE("save/fuzz: payload HMAC-valido truncado apos version rejeita",
          "[domain][save][fuzz]") {
    // So a version, sem nenhum campo comum. require() para o 1o read deve barrar.
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 1u);
    const auto packed = pack_save(payload);
    REQUIRE_THROWS_AS(deserialize_save(packed), SaveCorruptError);
}

// ---- bytes 100% aleatorios deterministicos (seed FIXA) ---------------------

TEST_CASE("save/fuzz: 2000 buffers aleatorios (seed fixa) nunca crasham",
          "[domain][save][fuzz]") {
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
        // Pode lancar (esperado) ou nao (input acidentalmente sem nenhum campo a
        // ler depois de validar). O contrato e: NUNCA crashar / OOB. ASan/UBSan no
        // CI capturam violacao de memoria; aqui garantimos termino limpo.
        try {
            (void)deserialize_save(buf);
        } catch (const std::exception&) {
            // ok: rejeitado de forma tipada/std.
        }
    }
    SUCCEED("2000 iteracoes aleatorias sem crash/UB");
}

// ---- bytes aleatorios DENTRO de um envelope HMAC-valido --------------------

TEST_CASE("save/fuzz: payloads aleatorios HMAC-validos nunca crasham",
          "[domain][save][fuzz]") {
    // Aqui o envelope SEMPRE passa no HMAC (pack_save sela um payload aleatorio),
    // forcando o DECODER DO PAYLOAD a lidar com lixo bem-selado. E o caminho que
    // mais exercita o Reader (version/strings/maps/counts arbitrarios).
    std::mt19937 rng(kSeed ^ 0x1234u);
    std::uniform_int_distribution<int> byte_dist(0, 255);
    std::uniform_int_distribution<int> len_dist(4, 200);

    int accepted = 0;
    for (int iter = 0; iter < 2000; ++iter) {
        const int n = len_dist(rng);
        std::vector<std::uint8_t> payload;
        payload.reserve(static_cast<std::size_t>(n));
        for (int i = 0; i < n; ++i)
            payload.push_back(static_cast<std::uint8_t>(byte_dist(rng)));
        const auto packed = pack_save(payload);
        INFO("iter=" << iter << " payload_len=" << n);
        try {
            (void)deserialize_save(packed);
            ++accepted;  // raríssimo, mas se um payload aleatorio for valido, ok
        } catch (const std::exception&) {
            // ok
        }
    }
    // Documenta quantos lixos passaram (esperado ~0). Nao falha o teste: provar
    // ausencia de CRASH e o objetivo. accepted>0 seria material p/ analise manual.
    INFO("payloads aleatorios aceitos como save valido: " << accepted);
    SUCCEED("2000 payloads aleatorios HMAC-validos sem crash/UB");
}
