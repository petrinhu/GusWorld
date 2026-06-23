# Bibliotecas C++ candidatas a incorporacao (vendoring) - GusWorld

**Status:** pesquisa 2026-06-22. **Fonte:** cppreference External Libraries (~876 libs em ~40 categorias), filtradas pelas categorias uteis a um RPG 2D single-player C++20/Qt6 (equipe de agentes por categoria).

**Filosofia ZERO-DEP:** incorporar o CODIGO no repo (header-only ou poucos arquivos), NAO como dependencia externa de build. Criterio de aprovacao: (1) util ao jogo, (2) vendorizavel (header-only/poucos arquivos), (3) licenca permissiva compativel com GPLv3.

**STATUS 2026-06-22:** as candidatas + 5 da categoria games (entt, anax, entityplus, entityx, box2d) foram VENDORIZADAS em `GusEngine/third_party/` (32 libs, 26 MB; build segue verde, NAO integradas no CMake ainda, include path sob demanda). Anura/Frogatto (engines completas, licenca custom) NAO incorporadas. Catalogo em `THIRD-PARTY-LICENSES.md`. As libs nao usadas no codigo serao deletadas na higiene do M9.

`★` = recomendada pro primeiro set (necessidade real ou iminente).

## Tabela mestre (candidatas aprovadas)

### Audio
| lib | o que faz | licenca | arquivos | url |
| :--- | :--- | :--- | :--- | :--- |
| **miniaudio** `★` | engine de audio completa (playback, decode WAV/MP3/FLAC, mixing, resampling) | public-domain ou MIT-0 | 2 | github.com/mackron/miniaudio |
| dr_libs | decoders single-header WAV/FLAC/MP3 | Unlicense/PD | 3 headers | github.com/mackron/dr_libs |
| AudioFile | leitura/escrita WAV/AIFF | MIT | 1 | github.com/adamstark/AudioFile |

### RNG e Math
| lib | o que faz | licenca | arquivos | url |
| :--- | :--- | :--- | :--- | :--- |
| **PCG (pcg-cpp)** `★` | PRNG seedavel excelente (streams/jump) | Apache-2.0 ou MIT | header-only | github.com/imneme/pcg-cpp |
| **fpm** | math de ponto-fixo (determinismo cross-platform Linux+Windows) | MIT | header-only | github.com/MikeLankamp/fpm |
| GLM | vetores/matrizes/quaternions GLSL-like | MIT | header-only | github.com/g-truc/glm |
| hlslpp | math SIMD sintaxe HLSL (alternativa a GLM, escolher 1) | MIT | header-only | github.com/redorav/hlslpp |
| Wykobi | geometria computacional 2D (intersecoes, distancias, hull) | MIT | header-only | wykobi.com |
| ExprTk | parser/avaliador de expressoes math em runtime (formulas data-driven) | MIT | single-header | partow.net/programming/exprtk |

### Imagem
| lib | o que faz | licenca | arquivos | url |
| :--- | :--- | :--- | :--- | :--- |
| **stb_image** `★` | decode PNG/JPG/TGA/BMP/GIF de arquivo ou memoria | public-domain/MIT | 1 header | github.com/nothings/stb |
| stb_image_write | grava PNG/TGA/BMP | public-domain/MIT | 1 header | idem |
| stb_rect_pack | bin-packing 2D (montar atlas de sprites) | public-domain/MIT | 1 header | idem |
| stb_truetype | parse/rasteriza fontes TrueType (bitmap font / atlas de glifos) | public-domain/MIT | 1 header | idem |

### Containers
| lib | o que faz | licenca | arquivos | url |
| :--- | :--- | :--- | :--- | :--- |
| **plf::colony** `★` | container com ponteiro/iterador estavel (pool de entidades: NPCs/projeteis/particulas) | zlib | 1 | github.com/mattreecebentley/plf_colony |
| plf::queue / stack / list | FIFO/LIFO/list otimizados | zlib | 1 cada | mattreecebentley |
| **Frozen** | maps/sets constexpr + busca de string compile-time (tabelas ID->item/skill/glyph) | Apache-2.0 | header-only | github.com/serge-sans-paille/frozen |

### Utilitarios (Generic)
| lib | o que faz | licenca | arquivos | url |
| :--- | :--- | :--- | :--- | :--- |
| **expected-lite** | valor-ou-erro sem excecao (std::expected-like; save/load/parsing) | Boost | single-header | github.com/martinmoene/expected-lite |
| **match(it)** | pattern-matching sem heap (FSMs de combate/dialogo legiveis) | Apache-2.0 | single-header | github.com/BowenFu/matchit.cpp |
| gsl-lite | span/not_null/narrow/finally | MIT/Boost | single-header | github.com/martinmoene/gsl-lite |
| scope-lite | scope_exit/fail/success (RAII guard) | Boost | single-header | martinmoene |
| ring-span-lite | view de ring buffer (replay, ring de audio, log de frames) | Boost | single-header | martinmoene |
| Better Enums | enum reflexivo (enum<->string, iteracao) | BSD-2 | single-header | github.com/aantron/better-enums |

### Text
| lib | o que faz | licenca | arquivos | url |
| :--- | :--- | :--- | :--- | :--- |
| **fmt** `★` | formatacao de string (std::format-like, placeholders nomeados) | MIT | 3 headers (header-only mode) | github.com/fmtlib/fmt |
| **CTRE** | regex compile-time (tags em texto rico, parsing de CSV de dialogo/i18n) | Apache-2.0 | single-header | github.com/hanickadot/ctre |
| lexy | parser combinator constexpr (scripts de dialogo, puzzle Vetor Gambito) | Boost | header-only | github.com/foonathan/lexy |
| PEGTL | parser PEG zero-dep (DSL "magia=software", tabelas de loot) | Boost | header-only | github.com/taocpp/PEGTL |
| inja | template engine Jinja2-like (ATENCAO: arrasta nlohmann/json) | MIT | header-only +1 dep | github.com/pantor/inja |

### Serialization
| lib | o que faz | licenca | arquivos | url |
| :--- | :--- | :--- | :--- | :--- |
| cppcodec | base64/base32/hex encode-decode (blobs binarios em save JSON) | MIT | header-only | github.com/tplgy/cppcodec |
| visit_struct | reflexao estatica de membros de struct (serializar save sem boilerplate) | Boost | 1 header | github.com/garbageslam/visit_struct |

### Physics 2D e Concurrency
Avaliadas, sem candidata vendorizavel prioritaria pro nosso caso: fisica 2D seria (Box2D/Chipmunk) NAO e header-only (build proprio); concorrencia em single-player ja e coberta por Qt/std::thread/std::jthread. Revisitar sob demanda (geometria 2D leve = Wykobi acima ja cobre colisao/raycast/line-of-sight).

## Avisos de licenca (importantes)
- **Apache-2.0** (PCG, Frozen, match(it), CTRE) e compativel com **GPLv3** (NAO com GPLv2). GusWorld e GPLv3, entao OK.
- **public-domain** (miniaudio, dr_libs, stb): zona cinza juridica em alguns paises. Vendorizar escolhendo o FALLBACK de licenca explicito quando oferecido: miniaudio -> MIT-0; dr_libs -> Unlicense; stb -> MIT. Preservar o cabecalho de licenca do autor intacto.
- **Better Enums** = BSD-2-Clause (nao Boost; nome certo no third-party).
- **GLM vs hlslpp**: escolher UM (redundantes).
- **inja** arrasta `nlohmann/json.hpp` (dep header extra) - so se realmente precisar de template engine.
- **span-lite / optional-lite / variant-lite**: C++20 ja tem `std::span`/`std::optional`/`std::variant`. So vendorizar se o toolchain das maquinas-alvo nao entregar.

## Processo de incorporacao (quando o lider escolher cada lib)
1. Copiar o(s) header(s)/source pra uma pasta tipo `GusEngine/third_party/<lib>/`, com o arquivo `LICENSE` da lib preservado ao lado.
2. Citar a lib em `THIRD-PARTY-LICENSES.md` + nos Creditos do README (nome, licenca, url).
3. NAO virar dependencia de build externa (sem find_package/vcpkg): so `#include` do header local.
4. Anti-over-engineering (YAGNI): incorporar so o que resolve necessidade real atual ou iminente; o resto fica nesta lista pra sob-demanda.
