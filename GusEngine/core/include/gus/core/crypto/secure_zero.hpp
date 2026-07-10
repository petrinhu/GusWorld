// gus/core/crypto/secure_zero.hpp
//
// Zeragem segura anti-otimizador (ADR-015 decisao 5, item 4 do wipe do Hardcore).
// Um std::fill/memset comum PODE ser eliminado pelo otimizador se a memoria zerada
// nao for lida depois (dead store elimination) - sem gerar nenhum aviso. Isso
// deixaria segredos (chave derivada, SaveData decifrado) vivos na RAM apos o
// permadeath. Wrapper proprio sobre crypto_wipe do Monocypher vendorizado (usa
// volatile internamente, garantido pela biblioteca a nao ser otimizado fora).
//
// Cross-ref: docs/tech/adr/ADR-015-save-security-v2-offline.md secao 5,
//            GusEngine/third_party/monocypher/monocypher.h (crypto_wipe).

#ifndef GUS_CORE_CRYPTO_SECURE_ZERO_HPP
#define GUS_CORE_CRYPTO_SECURE_ZERO_HPP

#include <cstddef>

namespace gus::core::crypto {

// Zera `size` bytes a partir de `data`, garantido a NAO ser eliminado pelo
// otimizador (equivalente a explicit_bzero/SecureZeroMemory). Usar em qualquer
// buffer que tenha guardado chave/segredo/payload decifrado antes de sair de
// escopo (ou explicitamente no wipe do permadeath Hardcore).
void secure_zero(void* data, std::size_t size) noexcept;

}  // namespace gus::core::crypto

#endif  // GUS_CORE_CRYPTO_SECURE_ZERO_HPP
