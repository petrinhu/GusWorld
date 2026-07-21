// save_serializer_fuzz_test.cpp
//
// REFORCO DE QA (nao-bloqueante) do DECODER de save GDS3 (ADR-015). O deserialize binario
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
// payloads AEAD-validos com campos invalidos / counts gigantes).
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

// Helper: escreve um u64 LE no fim de um buffer (instance_id/next_instance_id
// dos payloads V7 forjados a mao abaixo - o codec de producao usa put_u64
// interno, nao exportado no header publico; este e so o espelho local do teste).
void put_u64_le(std::vector<std::uint8_t>& out, std::uint64_t v) {
    for (int i = 0; i < 8; ++i)
        out.push_back(static_cast<std::uint8_t>((v >> (8 * i)) & 0xFFu));
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

TEST_CASE("save/fuzz: buffers menores que header+nonce+tag rejeitam tipado",
          "[domain][save][fuzz]") {
    // ADR-015 GDS3: aad(18) + nonce(24) + ciphertext_len(4) + tag(16) = 62 bytes
    // minimos (ciphertext vazio). Tudo abaixo e corrupcao estrutural.
    for (std::size_t n = 1; n < 62; ++n) {
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

// Offsets fixos do envelope GDS3 (ADR-015): aad(18) = magic(4)+envelope_ver(2)+
// slot_id(4)+rollback_ctr(8); nonce(24) logo apos; ciphertext_len(u32) no offset
// 18+24=42; ciphertext depois; tag(16) no fim. Espelha save_serializer.cpp (o
// teste NAO inclui o header real de producao: monta os bytes crus pra provar a
// rejeicao ANTES de qualquer AEAD).
constexpr std::size_t kAadLen = 18;
constexpr std::size_t kNonceLenFuzz = 24;
constexpr std::size_t kCiphertextLenOff = kAadLen + kNonceLenFuzz;  // 42
constexpr std::size_t kHeaderLenFuzz = kCiphertextLenOff + 4;       // 46
constexpr std::size_t kTagLenFuzz = 16;

TEST_CASE("save/fuzz: ciphertext_len declarado gigante (0xFFFFFFFF) rejeita sem "
          "alocar",
          "[domain][save][fuzz]") {
    // Constroi um envelope GDS3 pequeno cujo campo CIPHERTEXT_LEN afirma ~4 GiB.
    // O unpack deve detectar a inconsistencia (expected_total != data.size())
    // ANTES de qualquer leitura/alocacao guiada por esse length.
    std::vector<std::uint8_t> buf = {'G', 'D', 'S', '3'};
    buf.resize(kAadLen, 0x00u);                  // envelope_ver+slot_id+rollback_ctr
    buf.resize(kAadLen + kNonceLenFuzz, 0x00u);  // nonce placeholder
    put_u32_le(buf, 0xFFFFFFFFu);                 // CIPHERTEXT_LEN gigante
    buf.insert(buf.end(), kTagLenFuzz, 0x00u);    // tag placeholder (tamanho minimo)
    REQUIRE_THROWS_AS(deserialize_save(buf), SaveCorruptError);
}

// GCC 16.1 (Fedora 44) emite -Wstringop-overflow FALSO-POSITIVO neste TEST_CASE
// (resize+insert em vector<uint8_t> sob Catch2, ref FEDORA-GCC16). Supressao
// targeted, so aqui, so no GCC (Clang/MSVC nao emitem o FP).
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif
TEST_CASE("save/fuzz: ciphertext_len declarado zero num buffer maior rejeita",
          "[domain][save][fuzz]") {
    std::vector<std::uint8_t> buf = {'G', 'D', 'S', '3'};
    buf.resize(kAadLen, 0x00u);
    buf.resize(kAadLen + kNonceLenFuzz, 0x00u);
    put_u32_le(buf, 0u);                      // CIPHERTEXT_LEN = 0 mas ha payload real
    buf.insert(buf.end(), 16, 0x11u);          // bytes a mais
    buf.insert(buf.end(), kTagLenFuzz, 0x00u); // tag placeholder
    REQUIRE_THROWS_AS(deserialize_save(buf), SaveCorruptError);
}
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

TEST_CASE("save/fuzz: ciphertext_len off-by-one (real-1 e real+1) rejeita",
          "[domain][save][fuzz]") {
    const auto good = serialize_save(seed_fixture());
    const std::uint32_t real_len = static_cast<std::uint32_t>(
        good.size() - kHeaderLenFuzz - kTagLenFuzz);
    for (std::uint32_t delta : {static_cast<std::uint32_t>(real_len - 1u),
                                static_cast<std::uint32_t>(real_len + 1u)}) {
        auto bytes = good;
        bytes[kCiphertextLenOff] = static_cast<std::uint8_t>(delta & 0xFFu);
        bytes[kCiphertextLenOff + 1] = static_cast<std::uint8_t>((delta >> 8) & 0xFFu);
        bytes[kCiphertextLenOff + 2] =
            static_cast<std::uint8_t>((delta >> 16) & 0xFFu);
        bytes[kCiphertextLenOff + 3] =
            static_cast<std::uint8_t>((delta >> 24) & 0xFFu);
        INFO("ciphertext_len declarado=" << delta << " real=" << real_len);
        REQUIRE_THROWS_AS(deserialize_save(bytes), SaveCorruptError);
    }
}

// ---- tag AEAD valida mas payload com COUNT/LEN interno gigante ------------------
//
// O caso mais perigoso: o envelope e CONSISTENTE (length casa, tag AEAD bate sobre o
// payload mentiroso) mas um count interno (ex.: tamanho de uma string ou de uma
// lista) afirma bilhoes de elementos. O reader DEVE rejeitar via require()
// (truncamento) e nao tentar reservar/alocar memoria absurda guiada pelo count.

TEST_CASE("save/fuzz: payload AEAD-valido com string-len gigante rejeita",
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

    const auto packed = pack_save(payload);  // tag AEAD valida sobre o payload mentiroso
    REQUIRE(rejects_with_any_exception(
        [&] { (void)deserialize_save(packed); }));
}

TEST_CASE("save/fuzz: payload AEAD-valido com list-count gigante rejeita",
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

// ---- IMP-01: count interno gigante rejeita TIPADO ANTES de alocar ----------
//
// Defesa em profundidade (auditoria_seguranca_crypto.md IMP-01, CWE-789): um save
// SELADO (TAG AEAD VALIDA, o atacante tem a chave embarcada por design) cujo COUNT
// interno de um list<str> afirma ~4 bilhoes de elementos. Antes do fix, o decoder
// chamava reserve(count) ANTES de checar bytes restantes -> bad_alloc/length_error
// (crash em vez de "save corrompido"). Apos o fix, bounded_count rejeita com
// SaveCorruptError ANTES de qualquer alocacao grande (cada elemento custa >= 4 bytes
// de length, entao count > remaining()/4 e implausivel).

TEST_CASE("save/IMP-01: list-count gigante rejeita SaveCorruptError antes de alocar",
          "[domain][save][fuzz][imp01]") {
    // party_roster.count = 0xFFFFFFFF num payload curto. bounded_count deve barrar
    // com o erro TIPADO do decoder, NAO bad_alloc/length_error do reserve.
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 1u);                 // schema_version = 1
    payload.insert(payload.end(), 8, 0x00u); // timestamp_ms
    payload.insert(payload.end(), 8, 0x00u); // playtime_seconds
    put_u32_le(payload, 0u);                 // current_scene_path len = 0
    payload.insert(payload.end(), 48, 0x00u);// player_position + player_rotation
    put_u32_le(payload, 0xFFFFFFFFu);        // party_roster count GIGANTE

    const auto packed = pack_save(payload);
    REQUIRE_THROWS_AS(deserialize_save(packed), SaveCorruptError);
}

// ---- version: futura / zero / negativa-bit-pattern -------------------------

TEST_CASE("save/fuzz: version futura rejeita tipado (forward-only)",
          "[domain][save][fuzz]") {
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 9999u);              // version muito futura
    const auto packed = pack_save(payload);  // tag AEAD valida; version ilegal
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

// ---- tag AEAD valida + version OK mas payload truncado no meio dos campos -------

TEST_CASE("save/fuzz: payload AEAD-valido truncado apos version rejeita",
          "[domain][save][fuzz]") {
    // So a version, sem nenhum campo comum. require() para o 1o read deve barrar.
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 1u);
    const auto packed = pack_save(payload);
    REQUIRE_THROWS_AS(deserialize_save(packed), SaveCorruptError);
}

// ---- DECK-4 (V6): count gigante nas listas NOVAS de card_collection --------
//
// Mesmo padrao IMP-01 acima, agora exercitando o layout V6 CORRENTE
// (write_character_states_v6/read_character_states_v6, save_serializer.cpp):
// card_collection.active e um list<CardInstance{u64 instance_id, str card_id}>
// cujo COUNT (u32) e atacante-controlado (o payload esta DENTRO de um envelope
// selado com tag AEAD valida - bytes forjados, chave nao vazou aqui, mas o
// decoder nao pode assumir boa-fe do conteudo). Cada elemento custa no minimo 12
// bytes (8 do instance_id + 4 do length da string) - bounded_count deve rejeitar
// ANTES do reserve(count), mesmo com o payload por si so bem pequeno.

TEST_CASE("save/v6/fuzz: card_collection.active com count gigante rejeita "
          "SaveCorruptError antes de alocar",
          "[domain][save][fuzz][v6][imp01]") {
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 6u);                  // schema_version = 6 (layout corrente)
    payload.insert(payload.end(), 8, 0x00u);  // timestamp_ms (i64)
    payload.insert(payload.end(), 8, 0x00u);  // playtime_seconds (f64)
    put_u32_le(payload, 0u);                  // current_scene_path len = 0
    payload.insert(payload.end(), 48, 0x00u); // player_position + player_rotation
    put_u32_le(payload, 0u);                  // party_roster count = 0
    put_u32_le(payload, 0u);                  // party_active count = 0
    put_u32_le(payload, 0u);                  // flags count = 0
    put_u32_le(payload, 0u);                  // inventory count = 0
    put_u32_le(payload, 0u);                  // quest_progress count = 0
    put_u32_le(payload, 0u);                  // relations count = 0
    // character_states_v6: 1 entrada ("gus"), depois o count de active MENTE.
    put_u32_le(payload, 1u);                  // character_states count = 1
    put_u32_le(payload, 3u);                  // "gus" (string key, len=3)
    payload.push_back('g');
    payload.push_back('u');
    payload.push_back('s');
    payload.insert(payload.end(), 4, 0x00u);  // current_hp (i32) = 0
    payload.insert(payload.end(), 4, 0x00u);  // xp (i32) = 0
    put_u32_le(payload, 0xFFFFFFFFu);         // card_collection.active count GIGANTE
    // (sem os bytes das instancias: bounded_count deve barrar ANTES do reserve)

    const auto packed = pack_save(payload);   // tag AEAD valida sobre o payload mentiroso
    REQUIRE_THROWS_AS(deserialize_save(packed), SaveCorruptError);
}

TEST_CASE("save/v6/fuzz: hand_selection com count gigante rejeita SaveCorruptError "
          "antes de alocar",
          "[domain][save][fuzz][v6][imp01]") {
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 6u);
    payload.insert(payload.end(), 8, 0x00u);
    payload.insert(payload.end(), 8, 0x00u);
    put_u32_le(payload, 0u);
    payload.insert(payload.end(), 48, 0x00u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 1u);  // character_states count = 1
    put_u32_le(payload, 3u);
    payload.push_back('g');
    payload.push_back('u');
    payload.push_back('s');
    payload.insert(payload.end(), 4, 0x00u);  // current_hp
    payload.insert(payload.end(), 4, 0x00u);  // xp
    put_u32_le(payload, 0u);                  // card_collection.active count = 0
    put_u32_le(payload, 0u);                  // card_collection.dead count = 0
    payload.insert(payload.end(), 8, 0x00u);  // next_instance_id (u64) = 0
    // credits NAO mora aqui (V6 pos-correcao do lider: carteira UNICA da party,
    // gravada 1x no FIM do payload em SaveData - ver write_credits_wallet). Logo
    // apos next_instance_id vem DIRETO o count de hand_selection.
    put_u32_le(payload, 0xFFFFFFFFu);         // hand_selection count GIGANTE
    // (sem os u64 da selecao: bounded_count deve barrar ANTES do reserve)

    const auto packed = pack_save(payload);
    REQUIRE_THROWS_AS(deserialize_save(packed), SaveCorruptError);
}

// ---- V7 (CARDS-HW-1): count gigante nas listas NOVAS de card_collection ----
//
// GAP encontrado na revisao adversarial (qa-engineer, verificacao independente do
// incremento CARDS-HW-1, a09ebd6): os 2 casos IMP-01 acima cobrem SO o layout V6
// (read_card_instance_list, min 12 bytes/elemento). O layout CORRENTE V7+
// (read_card_instance_list_v7, min 12+17=29 bytes/elemento - CADA CardInstance
// ganhou CardPhysicalState) NAO tinha nenhum caso hostil aqui; os testes aleatorios
// (2000 iteracoes abaixo) tambem nunca exercitam o payload V7 de proposito (a
// chance de um schema_version==7 sair de bytes aleatorios e ~1/2^32). Os 4 casos
// abaixo fecham esse buraco, espelhando o padrao IMP-01 ja usado para V6.

TEST_CASE("save/v7/fuzz: card_collection.active com count gigante rejeita "
          "SaveCorruptError antes de alocar (bound V7 = 29 bytes/elemento, nao "
          "12 do V6)",
          "[domain][save][fuzz][v7][imp01]") {
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 7u);                  // schema_version = 7 (layout corrente)
    payload.insert(payload.end(), 8, 0x00u);  // timestamp_ms (i64)
    payload.insert(payload.end(), 8, 0x00u);  // playtime_seconds (f64)
    put_u32_le(payload, 0u);                  // current_scene_path len = 0
    payload.insert(payload.end(), 48, 0x00u); // player_position + player_rotation
    put_u32_le(payload, 0u);                  // party_roster count = 0
    put_u32_le(payload, 0u);                  // party_active count = 0
    put_u32_le(payload, 0u);                  // flags count = 0
    put_u32_le(payload, 0u);                  // inventory count = 0
    put_u32_le(payload, 0u);                  // quest_progress count = 0
    put_u32_le(payload, 0u);                  // relations count = 0
    // character_states_v7: 1 entrada ("gus"), depois o count de active MENTE.
    put_u32_le(payload, 1u);                  // character_states count = 1
    put_u32_le(payload, 3u);                  // "gus" (string key, len=3)
    payload.push_back('g');
    payload.push_back('u');
    payload.push_back('s');
    payload.insert(payload.end(), 4, 0x00u);  // current_hp (i32) = 0
    payload.insert(payload.end(), 4, 0x00u);  // xp (i32) = 0
    put_u32_le(payload, 0xFFFFFFFFu);         // card_collection.active count GIGANTE
    // (sem os bytes das instancias V7: bounded_count deve barrar ANTES do reserve)

    const auto packed = pack_save(payload);   // tag AEAD valida sobre o payload mentiroso
    REQUIRE_THROWS_AS(deserialize_save(packed), SaveCorruptError);
}

TEST_CASE("save/v7/fuzz: card_collection.dead com count gigante rejeita "
          "SaveCorruptError antes de alocar",
          "[domain][save][fuzz][v7][imp01]") {
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 7u);
    payload.insert(payload.end(), 8, 0x00u);
    payload.insert(payload.end(), 8, 0x00u);
    put_u32_le(payload, 0u);
    payload.insert(payload.end(), 48, 0x00u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 1u);  // character_states count = 1
    put_u32_le(payload, 3u);
    payload.push_back('g');
    payload.push_back('u');
    payload.push_back('s');
    payload.insert(payload.end(), 4, 0x00u);  // current_hp
    payload.insert(payload.end(), 4, 0x00u);  // xp
    put_u32_le(payload, 0u);                  // card_collection.active count = 0
    put_u32_le(payload, 0xFFFFFFFFu);         // card_collection.dead count GIGANTE
    // (sem as instancias V7: bounded_count deve barrar ANTES do reserve)

    const auto packed = pack_save(payload);
    REQUIRE_THROWS_AS(deserialize_save(packed), SaveCorruptError);
}

TEST_CASE("save/v7/fuzz: hand_selection do layout V7 (apos card_collection COM "
          "physical) com count gigante rejeita SaveCorruptError antes de alocar",
          "[domain][save][fuzz][v7][imp01]") {
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 7u);
    payload.insert(payload.end(), 8, 0x00u);
    payload.insert(payload.end(), 8, 0x00u);
    put_u32_le(payload, 0u);
    payload.insert(payload.end(), 48, 0x00u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 1u);  // character_states count = 1
    put_u32_le(payload, 3u);
    payload.push_back('g');
    payload.push_back('u');
    payload.push_back('s');
    payload.insert(payload.end(), 4, 0x00u);  // current_hp
    payload.insert(payload.end(), 4, 0x00u);  // xp
    put_u32_le(payload, 0u);                  // card_collection.active count = 0
    put_u32_le(payload, 0u);                  // card_collection.dead count = 0
    payload.insert(payload.end(), 8, 0x00u);  // next_instance_id (u64) = 0
    put_u32_le(payload, 0xFFFFFFFFu);         // hand_selection count GIGANTE
    // (sem os u64 da selecao: bounded_count deve barrar ANTES do reserve)

    const auto packed = pack_save(payload);
    REQUIRE_THROWS_AS(deserialize_save(packed), SaveCorruptError);
}

TEST_CASE("save/v7/fuzz: CardInstance V7 truncado NO MEIO do bloco novo de "
          "CardPhysicalState (apos origin+cycles, faltam deficit/flags/virus/"
          "burned_out) rejeita, nunca le lixo/UB",
          "[domain][save][fuzz][v7][imp01]") {
    // count=1 (plausivel), instance_id+card_id validos, so o bloco physical vem
    // cortado bem no meio (4 bytes de origin + 2 de cycles = 6 dos 17 esperados) -
    // ataca especificamente o offset novo que o incremento CARDS-HW-1 introduziu
    // (read_card_physical_state), nao coberto pelos truncamentos genericos de
    // "truncar em cada offset" (que usam um save V7 JA valido/completo, nunca um
    // payload V7 forjado a mao como este).
    std::vector<std::uint8_t> payload;
    put_u32_le(payload, 7u);
    payload.insert(payload.end(), 8, 0x00u);
    payload.insert(payload.end(), 8, 0x00u);
    put_u32_le(payload, 0u);
    payload.insert(payload.end(), 48, 0x00u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 0u);
    put_u32_le(payload, 1u);  // character_states count = 1
    put_u32_le(payload, 3u);
    payload.push_back('g');
    payload.push_back('u');
    payload.push_back('s');
    payload.insert(payload.end(), 4, 0x00u);  // current_hp
    payload.insert(payload.end(), 4, 0x00u);  // xp
    put_u32_le(payload, 1u);                  // card_collection.active count = 1 (plausivel)
    put_u64_le(payload, 1u);                  // instance_id
    put_u32_le(payload, 4u);                  // card_id len=4
    payload.push_back('t');
    payload.push_back('e');
    payload.push_back('s');
    payload.push_back('t');
    put_u32_le(payload, 0u);                  // physical.origin = OriginalRom (4 bytes)
    payload.push_back(0x00u);                 // battery_recharge_cycles LO (so 2 dos 17
    payload.push_back(0x00u);                 // esperados - corta aqui de proposito)

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

// ---- bytes aleatorios DENTRO de um envelope AEAD-valido --------------------

TEST_CASE("save/fuzz: payloads aleatorios AEAD-validos nunca crasham",
          "[domain][save][fuzz]") {
    // Aqui o envelope SEMPRE passa na verificacao AEAD (pack_save sela um payload aleatorio),
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
    SUCCEED("2000 payloads aleatorios AEAD-validos sem crash/UB");
}
