# Licenças de terceiros (atribuição)

> Componentes de terceiros empacotados ou usados pelo GusWorld, com a respectiva licença e fonte. **Incluir este arquivo no pacote de release.**

O código próprio do GusWorld está sob GPLv3 (ver [LICENSE](LICENSE)); os assets próprios sob CC-BY-SA 4.0 (ver [ASSETS-LICENSE.md](ASSETS-LICENSE.md)). Os itens abaixo são de terceiros e mantêm a licença original de cada um.

---

## Tabela de atribuição

| Componente | Licença | Titular / fonte | Nota |
|---|---|---|---|
| Qt 6 | LGPLv3 (núcleo) + GPLv3 (alguns módulos) | The Qt Company / Qt Project | Camada de plataforma da Fase 1 do pivot; SUBSTITUIDA por SDL3 no re-pivot (ADR-008). Sai do projeto conforme as fases SDL fecharem. |
| SDL3 | zlib | SDL / Sam Lantinga e contribuidores | Camada de plataforma atual (ADR-008). Via FetchContent (release-3.4.10), link estatico. |
| RmlUi | MIT | mikke89 e contribuidores | UI do jogador (Fase 3 do re-pivot). Via FetchContent (6.2); ainda nao usada. |
| Dear ImGui | MIT | Omar Cornut (ocornut) e contribuidores | UI de debug/dev (futuro). |
| Godot Engine | MIT | Juan Linietsky, Ariel Manzur e contribuidores | Engine da fase de transição (pre-pivot). |
| DialogueManager (addon) | MIT | Copyright (c) 2022-present Nathan Hoad | Addon Godot de diálogo. |
| Fontes Noto | SIL OFL 1.1 | Google e contribuidores | <https://openfontlicense.org> |
| Fonte Inter | SIL OFL 1.1 | The Inter Project Authors | <https://openfontlicense.org> |
| Fonte JetBrains Mono | SIL OFL 1.1 | JetBrains s.r.o. | <https://openfontlicense.org> |
| OpenSSL | Apache-2.0 | OpenSSL Project | Apenas se empacotado via Qt. |

---

## Bibliotecas C++ vendorizadas em `GusEngine/third_party/` (2026-06-22)

Codigo-fonte incorporado no repo (filosofia zero-dep), cada lib com seu arquivo `LICENSE` preservado na pasta. Catalogo: [`docs/tech/libs-vendoring-candidatas.md`](docs/tech/libs-vendoring-candidatas.md). As libs nao usadas no codigo serao removidas na higiene do M9.

| lib | licenca | fonte |
|---|---|---|
| miniaudio | Public Domain (Unlicense) ou MIT-0 | github.com/mackron/miniaudio |
| dr_libs | Public Domain (Unlicense) ou MIT-0 | github.com/mackron/dr_libs |
| AudioFile | MIT | github.com/adamstark/AudioFile |
| pcg-cpp | MIT ou Apache-2.0 | github.com/imneme/pcg-cpp |
| fpm | MIT | github.com/MikeLankamp/fpm |
| glm | MIT (ou Happy Bunny) | github.com/g-truc/glm |
| hlslpp | MIT | github.com/redorav/hlslpp |
| exprtk | MIT | github.com/ArashPartow/exprtk |
| stb | MIT ou Public Domain | github.com/nothings/stb |
| wykobi | MIT | github.com/ArashPartow/wykobi |
| plf_colony | zlib | github.com/mattreecebentley/plf_colony |
| plf_queue | zlib | github.com/mattreecebentley/plf_queue |
| plf_stack | zlib | github.com/mattreecebentley/plf_stack |
| plf_list | zlib | github.com/mattreecebentley/plf_list |
| frozen | Apache-2.0 | github.com/serge-sans-paille/frozen |
| expected-lite | BSL-1.0 (Boost) | github.com/martinmoene/expected-lite |
| gsl-lite | MIT ou BSL-1.0 | github.com/martinmoene/gsl-lite |
| scope-lite | BSL-1.0 (Boost) | github.com/martinmoene/scope-lite |
| ring-span-lite | BSL-1.0 (Boost) | github.com/martinmoene/ring-span-lite |
| matchit | Apache-2.0 | github.com/BowenFu/matchit.cpp |
| better-enums | BSD-2-Clause | github.com/aantron/better-enums |
| fmt | MIT | github.com/fmtlib/fmt |
| ctre | Apache-2.0 | github.com/hanickadot/compile-time-regular-expressions |
| lexy | BSL-1.0 (Boost) | github.com/foonathan/lexy |
| pegtl | BSL-1.0 (Boost) | github.com/taocpp/PEGTL |
| cppcodec | MIT | github.com/tplgy/cppcodec |
| visit_struct | BSL-1.0 (Boost) | github.com/cbeck88/visit_struct |
| entt | MIT | github.com/skypjack/entt |
| anax | MIT | github.com/miguelishawt/anax |
| entityplus | BSL-1.0 (Boost) | github.com/Yelnats321/EntityPlus |
| entityx | MIT | github.com/alecthomas/entityx |
| box2d | MIT | github.com/erincatto/box2d |

Nota de compatibilidade: todas permissivas e compativeis com o GPLv3 do jogo (Apache-2.0 e compativel com GPLv3, nao com GPLv2; public-domain usar o fallback explicito MIT/Unlicense).

---

## Links das licenças (texto oficial)

Referência por nome + URL; o texto legal completo de cada licença vive na fonte oficial e/ou é embarcado junto do respectivo componente no pacote de release.

- **MIT**: <https://opensource.org/license/mit>
- **LGPLv3**: <https://www.gnu.org/licenses/lgpl-3.0.html>
- **GPLv3**: <https://www.gnu.org/licenses/gpl-3.0.html>
- **SIL OFL 1.1**: <https://openfontlicense.org>
- **Apache-2.0**: <https://www.apache.org/licenses/LICENSE-2.0>
- **BSL-1.0 (Boost)**: <https://www.boost.org/LICENSE_1_0.txt>
- **zlib**: <https://opensource.org/license/zlib>
- **BSD-2-Clause**: <https://opensource.org/license/bsd-2-clause>
- **Unlicense / Public Domain**: <https://unlicense.org>

---

## Qt: oferta de source (LGPL/GPL)

Qt entra no projeto via pivot C++. O GusWorld é GPLv3, então o link (inclusive estático) com Qt LGPL/GPL é permitido sem licença comercial. Obrigações a cumprir no release:

- Identificar a versão exata do Qt empacotada.
- Disponibilizar o **source correspondente** do Qt (link para o tarball oficial da versão usada satisfaz a obrigação).
- Manter os avisos de copyright do Qt no pacote.

Fonte do Qt: <https://download.qt.io/official_releases/qt/>

---

## Manutenção

Atualizar esta tabela sempre que adicionar, remover ou trocar a versão de qualquer dependência de terceiro (addon Godot, fonte, biblioteca C++/NuGet, lib do sistema empacotada). Auditoria de licença = item RF-9 do pivot.

*Recomendação técnica de atribuição; validação jurídica formal cabe ao titular.*
