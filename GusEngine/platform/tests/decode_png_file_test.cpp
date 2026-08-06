// SPDX-License-Identifier: Apache-2.0
// GusEngine/platform/tests/decode_png_file_test.cpp
//
// PNG-DECODE-ADOPT (2026-08-06): prova EXECUTAVEL de que a troca de
// glintfx::decode_image_file por glintfx::decode_png_file nos 3 call sites de decode
// (app/src/app_icon.cpp, platform/src/render2d/render2d_gl3.cpp,
// platform/src/render2d/render2d_sdl.cpp) RESOLVE o defeito - nao so de que compila.
//
// O DEFEITO (medido pela auditoria independente da W1, reconferido aqui em disco):
// `decode_image_file` faz dispatch PNG/JPG/TGA pelo sniffing do stb_image. A perna TGA e
// FROUXA por construcao - o formato TGA nao tem numero magico, entao o sniff se contenta
// com um byte `image_type` plausivel mais um `bits_per_pixel` plausivel, e NAO exige que o
// dado de pixel prometido pelo header de 18 bytes exista de fato (o stb_image preenche o
// que falta com zero em vez de falhar). Resultado: 18 bytes de header TGA declarando
// 4096x4096 a 32 bpp + 64 bytes de ruido - 82 bytes no total, com nome ".png" - viram um
// handle VALIDO (`ok == true`, nao uma rejeicao) de uma RGBA8 4096x4096 toda alpha-zero:
// 67.108.864 bytes alocados a partir de 82 bytes de entrada, amplificacao de ~818.000x. Um
// chamador que so testa "o handle e valido" (como os nossos 3 testavam) nao percebe nada -
// o asset vira em silencio um quadrado grande transparente em vez de falhar alto.
//
// O REMEDIO ja existia no pin que usamos (v0.30.0): `decode_png_file` (image.hpp:691) checa
// os 8 bytes da assinatura de arquivo PNG (0x89 'P' 'N' 'G' \r \n 0x1A \n, da propria
// especificacao PNG - RFC 2083/ISO 15948) ANTES de o buffer alcancar o stb_image. Adotar
// API que a lib ja expoe exatamente pra este fim NAO e contornar lacuna dela (o que a lei
// da casa proibe, CLAUDE.md 2026-07-29) - e o oposto.
//
// FIXTURES EM BYTES, NAO EM ASSET DO REPO, de proposito: o teste tem de rodar sem depender
// de layout de disco, de LFS ou de qualquer PNG especifico continuar existindo - e o PNG
// 1x1 abaixo e curto o bastante pra ser auditavel a olho (assinatura + IHDR + IDAT + IEND,
// CRCs conferidos por zlib.crc32 na geracao).
//
// ⚠️ O 3o TEST_CASE assere o comportamento da DEPENDENCIA (decode_image_file ACEITANDO o
// forjado). Ele existe porque e o unico jeito de provar, no mesmo input, que a troca muda o
// resultado - sem ele os outros dois so provariam que decode_png_file funciona, nunca que
// precisavamos dele. Se um dia um bump do glintfx deixar ESSE teste vermelho, a noticia e
// BOA (eles fecharam o dispatch geral): o caminho e atualizar/aposentar aquele TEST_CASE e
// reavaliar o GATE(decode-image-file-zero) - NUNCA reverter a troca dos 3 call sites.

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <glintfx/image.hpp>

#include "tmp_dir_test_support.hpp"

namespace {

// PNG 1x1 RGBA8 valido (70 bytes): assinatura + IHDR (1x1, bit depth 8, color type 6 =
// RGBA) + IDAT (deflate de {filter=0, R=255, G=0, B=0, A=255}) + IEND. CRC de cada chunk
// calculado na geracao (zlib.crc32), nao chutado.
const std::vector<std::uint8_t> kPng1x1Red = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48,
    0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x06, 0x00, 0x00,
    0x00, 0x1F, 0x15, 0xC4, 0x89, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x44, 0x41, 0x54, 0x78,
    0xDA, 0x63, 0xF8, 0xCF, 0xC0, 0xF0, 0x1F, 0x00, 0x05, 0x00, 0x01, 0xFF, 0x56, 0xC7,
    0x2F, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82};

// O ARQUIVO FORJADO, byte a byte: 18 bytes de header TGA + 64 bytes de ruido = 82.
// Layout do header TGA (little-endian, campos na ordem da especificacao):
//   [0]    id_length          = 0
//   [1]    color_map_type     = 0  (sem palette)
//   [2]    image_type         = 2  (truecolor sem RLE - PLAUSIVEL, e so isso que o
//                                   sniff do stb_image exige nesta posicao)
//   [3..7] color_map_spec     = 0  (5 bytes zerados)
//   [8..9] x_origin           = 0
//   [10..11] y_origin         = 0
//   [12..13] width            = 4096 (0x1000)
//   [14..15] height           = 4096 (0x1000)
//   [16]   bits_per_pixel     = 32  (PLAUSIVEL -> RGBA)
//   [17]   image_descriptor   = 8   (8 bits de alpha)
// Nada garante que os 4096*4096*4 = 67.108.864 bytes de pixel prometidos existam - e
// justamente essa ausencia de garantia que o stb_image nao cobra.
std::vector<std::uint8_t> forged_tga_82_bytes() {
    std::vector<std::uint8_t> bytes(18, 0);
    bytes[2] = 2;      // image_type: truecolor
    bytes[12] = 0x00;  // width lo
    bytes[13] = 0x10;  // width hi  -> 4096
    bytes[14] = 0x00;  // height lo
    bytes[15] = 0x10;  // height hi -> 4096
    bytes[16] = 32;    // bits per pixel
    bytes[17] = 8;     // alpha bits
    // 64 bytes de "dado de pixel" sem relacao nenhuma com os 64 MB prometidos.
    for (std::uint8_t i = 0; i < 64; ++i) {
        bytes.push_back(static_cast<std::uint8_t>(0xA5 ^ i));
    }
    return bytes;
}

// Escreve os bytes em disco, em modo BINARIO (sem traducao de \n no Windows - o CI MSVC
// e alvo real, e um PNG "consertado" por traducao de fim de linha deixaria de decodificar
// por um motivo que nao tem nada a ver com o que este teste mede).
void write_bytes(const std::filesystem::path& p, const std::vector<std::uint8_t>& bytes) {
    std::ofstream out(p, std::ios::binary | std::ios::trunc);
    REQUIRE(out.is_open());
    out.write(reinterpret_cast<const char*>(bytes.data()),
              static_cast<std::streamsize>(bytes.size()));
    out.close();
    REQUIRE(std::filesystem::file_size(p) == bytes.size());
}

}  // namespace

TEST_CASE("PNG-DECODE-ADOPT: decode_png_file RECUSA o TGA forjado de 82 bytes com nome .png",
          "[platform][image][security]") {
    gus::test_support::ScopedTempDir tmp("decode_png_forged");

    // O nome do arquivo diz .png DE PROPOSITO: a defesa nao pode depender da extensao
    // (que e texto controlado por quem forneceu o arquivo), e sim do CONTEUDO. Este e o
    // cenario realista - asset malicioso entregue com o nome que o jogo espera.
    const std::filesystem::path forged = tmp.path() / "asset_malicioso.png";
    const std::vector<std::uint8_t> bytes = forged_tga_82_bytes();
    REQUIRE(bytes.size() == 82);
    write_bytes(forged, bytes);

    const glintfx::DecodedImagePixels decoded =
        glintfx::decode_png_file(forged.string().c_str());

    // Contrato fail-high do glintfx: `ok == false` e o UNICO sinal de falha.
    REQUIRE_FALSE(decoded.ok);
    // E a rejeicao acontece ANTES de qualquer alocacao: nada de 64 MB de zeros.
    CHECK(decoded.pixels.empty());
    CHECK(decoded.width == 0);
    CHECK(decoded.height == 0);
}

TEST_CASE("PNG-DECODE-ADOPT: decode_png_file ACEITA um PNG real de 1x1 e devolve RGBA straight",
          "[platform][image]") {
    gus::test_support::ScopedTempDir tmp("decode_png_valid");
    const std::filesystem::path png = tmp.path() / "pixel.png";
    write_bytes(png, kPng1x1Red);

    const glintfx::DecodedImagePixels decoded =
        glintfx::decode_png_file(png.string().c_str());

    REQUIRE(decoded.ok);
    CHECK(decoded.width == 1);
    CHECK(decoded.height == 1);
    REQUIRE(decoded.pixels.size() == 4u);  // 1 * 1 * RGBA8
    // Alpha STRAIGHT (nao-premultiplicado): os 3 call sites contam com isso -
    // render2d_gl3 premultiplica DEPOIS (blend GL premultiplied), render2d_sdl e
    // app_icon consomem straight direto (SDL_BLENDMODE_BLEND / SDL_SetWindowIcon).
    CHECK(decoded.pixels[0] == 255);  // R
    CHECK(decoded.pixels[1] == 0);    // G
    CHECK(decoded.pixels[2] == 0);    // B
    CHECK(decoded.pixels[3] == 255);  // A
}

TEST_CASE("PNG-DECODE-ADOPT: decode_image_file ACEITA o mesmo forjado (o motivo da troca)",
          "[platform][image][security]") {
    // Ver o aviso no topo do arquivo: este TEST_CASE mede a DEPENDENCIA, e e o unico
    // jeito de provar - no MESMO input - que trocar a funcao muda o resultado. Vermelho
    // aqui apos um bump = boa noticia, e nunca motivo pra reverter os call sites.
    gus::test_support::ScopedTempDir tmp("decode_image_file_forged");
    const std::filesystem::path forged = tmp.path() / "asset_malicioso.png";
    write_bytes(forged, forged_tga_82_bytes());

    const glintfx::DecodedImagePixels decoded =
        glintfx::decode_image_file(forged.string().c_str());

    // O handle e VALIDO - nao ha crash, nao ha ok==false: e exatamente por isso que a
    // degradacao dos call sites antigos (`if (!decoded.ok) return;`) nao pegava nada.
    REQUIRE(decoded.ok);
    CHECK(decoded.width == 4096);
    CHECK(decoded.height == 4096);
    // 82 bytes de arquivo -> 67.108.864 bytes em memoria: a amplificacao de ~818.000x,
    // medida e nao presumida.
    CHECK(decoded.pixels.size() == static_cast<std::size_t>(4096) * 4096 * 4);
    const std::size_t amplificacao = decoded.pixels.size() / 82u;
    CHECK(amplificacao > 800000u);
}

TEST_CASE("PNG-DECODE-ADOPT: decode_png_file degrada em entrada invalida sem excecao",
          "[platform][image]") {
    gus::test_support::ScopedTempDir tmp("decode_png_invalid");

    SECTION("path nulo") {
        CHECK_FALSE(glintfx::decode_png_file(nullptr).ok);
    }
    SECTION("arquivo inexistente") {
        const std::filesystem::path missing = tmp.path() / "nao_existe.png";
        CHECK_FALSE(glintfx::decode_png_file(missing.string().c_str()).ok);
    }
    SECTION("arquivo vazio") {
        const std::filesystem::path empty = tmp.path() / "vazio.png";
        write_bytes(empty, {});
        CHECK_FALSE(glintfx::decode_png_file(empty.string().c_str()).ok);
    }
    SECTION("assinatura PNG valida mas corpo truncado") {
        // A checagem de assinatura e a PRIMEIRA guarda, nao um substituto do decode:
        // um arquivo que PASSA nos 8 bytes magicos ainda tem de decodificar de verdade.
        std::vector<std::uint8_t> truncado(kPng1x1Red.begin(), kPng1x1Red.begin() + 24);
        const std::filesystem::path p = tmp.path() / "truncado.png";
        write_bytes(p, truncado);
        CHECK_FALSE(glintfx::decode_png_file(p.string().c_str()).ok);
    }
}
