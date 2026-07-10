# Monocypher (vendorizado)

Vendorizado em 2026-07-10 (ADR-015 Onda 1, `SAVE-CRYPTO-V2-VENDOR`).

- **Fonte:** <https://github.com/LoupVaillant/Monocypher>
- **Tag:** `4.0.3`
- **Commit:** `ab2b16dd619ad5f6979a4fbe69cfa324a6fcc35f` (2026-06-15)
- **Licença:** dupla, CC0-1.0 (domínio público) ou BSD-2-Clause (fallback), ver `LICENCE.md` neste diretório. Registrada em `THIRD-PARTY-LICENSES.md` da raiz do repo.
- **Arquivos:** `monocypher.c` + `monocypher.h`, cópia byte-a-byte do upstream (conferida por `md5sum` no vendoring; NÃO modificados).

## Por que Monocypher (não DIY, não OpenSSL)

Ver `docs/tech/adr/ADR-015-save-security-v2-offline.md` decisão 1: primitivas AEAD com nonce (XChaCha20-Poly1305) e KDF memory-hard (Argon2id) são uma classe de risco onde "rolar a cifra a mão" pode passar num roundtrip feliz e falhar silenciosamente sob análise adversarial (reuso de nonce, indexação sutil do Argon2). Monocypher é o mesmo padrão já usado para o SHA-256/HMAC próprio do projeto (ADR-006: arquivo vendorizado, domínio público, revisável linha a linha), mas para primitivas que a comunidade cripto já escrutinou publicamente por anos.

## O que este projeto usa daqui

- `crypto_aead_lock` / `crypto_aead_unlock` (XChaCha20-Poly1305, nonce de 24 bytes) — wrapper em `GusEngine/core/include/gus/core/crypto/aead_xchacha20poly1305.hpp`.
- `crypto_argon2` (Argon2id) — wrapper em `GusEngine/core/include/gus/core/crypto/argon2id.hpp`.
- `crypto_wipe` — wrapper em `GusEngine/core/include/gus/core/crypto/secure_zero.hpp`.

Nenhum código de `core/`/`domain/` inclui `monocypher.h` diretamente fora desses 3 wrappers (encapsula a dependência vendorizada atrás de uma API própria, mesmo padrão do `sha256.hpp`/`hmac_sha256.hpp`).
