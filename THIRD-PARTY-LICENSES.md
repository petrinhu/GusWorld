# Licenças de terceiros (atribuição)

> Componentes de terceiros empacotados ou usados pelo GusWorld, com a respectiva licença e fonte. **Incluir este arquivo no pacote de release.**

O código próprio do GusWorld está sob Apache License 2.0 (ver [LICENSE](LICENSE)); os assets próprios sob CC-BY-SA 4.0 (ver [ASSETS-LICENSE.md](ASSETS-LICENSE.md)). Os itens abaixo são de terceiros e mantêm a licença original de cada um.

---

## Tabela de atribuição (dependências ATIVAS no binário C++/SDL3 atual)

| Componente | Licença | Titular / fonte | Nota |
|---|---|---|---|
| SDL3 | zlib | SDL / Sam Lantinga e contribuidores | Camada de plataforma atual (ADR-008). Via FetchContent (release-3.4.10), link estatico. |
| glintfx | Apache-2.0 | petrinhu (<https://github.com/petrinhu/glintfx>) | Motor de UI/HUD em embed mode (`glintfx::UiLayer`), ADR-010. Via FetchContent, pin `v0.30.0` em `GusEngine/CMakeLists.txt` (bump BUMP-GLINTFX-V030, 2026-08-05; conferido contra a árvore baixada e contra o `config.hpp` gerado) (`GLINTFX_BACKEND_GLFW=OFF`, embed-only), linkado SO em `app/` (GATE de 4 camadas: nunca `core/`/`domain/`). **A tag `v0.29.0` é o corte de rotação de licença do glintfx: até a `v0.28.0` inclusive é MPL-2.0 para sempre, da `v0.29.0` em diante é Apache-2.0** (conferido no `LICENSE` da tag, não presumido pela palavra deles). Apache-2.0 é a mesma licença do código próprio do GusWorld — sem questão de compatibilidade a resolver, a nota de compatibilidade MPL acima deixou de se aplicar a este pino. |
| RmlUi | MIT | mikke89 e contribuidores | UI do jogador, **ATIVA** desde o M2 (glintfx embed mode, ADR-010); corrige nota anterior desatualizada ("ainda não usada"). Via FetchContent, pin fixado por SHA `2cd28864ae25ed345b70598751703a5433b12356`, correspondente à **v6.3** (alinhado ao pin que o glintfx exige). `RMLUI_FONT_ENGINE=freetype`. |
| FreeType | FTL (FreeType License) | The FreeType Project | Dependência do RmlUi (`find_package(Freetype)`, biblioteca de sistema, Linux: `libfreetype-dev`, NÃO via FetchContent). Rasteriza a fonte do HUD (Pixel Operator Mono) e os font-effects (glow/outline). **A FTL exige aviso de atribuição na documentação de distribuição; texto obrigatório na seção dedicada abaixo.** |
| miniaudio | Unlicense (domínio público) **ou** MIT-0, à escolha de quem usa (dupla, declarada no rodapé do próprio `miniaudio.h`) | David Reid / mackron (<https://github.com/mackron/miniaudio>) | **Dependência TRANSITIVA: não é vendorizada por nós, mas ESTÁ no binário distribuído.** Entra pelo glintfx, que vendoriza e compila o próprio `miniaudio.h` (`glintfx/third_party/miniaudio/`, pin `0.11.25`, commit `9634bedb5b5a2ca38c1ee7108a9358a4e233f14d`) quando `GLINTFX_MODULE_AUDIO=ON` (`GusEngine/CMakeLists.txt:188`). A fachada `AudioEngine` do GusWorld fala com `glintfx::Audio` e não inclui `miniaudio.h`; a cópia direta que existia em `GusEngine/third_party/miniaudio/` foi removida na higienização do M9 (2026-07-22), quando o áudio migrou para o módulo do glintfx. Nenhuma das duas licenças exige aviso de atribuição no binário, mas o componente é distribuído, então é listado aqui. |
| SDL_GameControllerDB (subconjunto Linux) | zlib | Sam Lantinga e a comunidade do SDL; base mantida em <https://github.com/mdqinc/SDL_GameControllerDB> | **Dependência TRANSITIVA, específica de Linux, e ESTÁ no binário distribuído.** Entra pelo glintfx, que vendoriza um subconjunto filtrado do banco de mapeamentos **como DADO** (`glintfx/third_party/gamecontrollerdb/`) e o embute via `#include "gamecontrollerdb_linux.inc"` em `glintfx/src/gamepad.cpp:105`, consumido pelo parser clean-room do próprio glintfx (nenhum código do SDL é compilado ou linkado por essa via). Ativo sempre que `GLINTFX_MODULE_GAMEPAD=ON`, o que aqui vale para `UNIX AND NOT APPLE` (`GusEngine/CMakeLists.txt:194-198`); em Windows e macOS o módulo é OFF e o dado não entra. A zlib pede que o aviso não seja removido nem alterado de distribuição de código-fonte, e não impõe atribuição em distribuição binária; listado aqui porque é obra de terceiro com titular próprio embarcada no release Linux, que é a plataforma da v1.0.0. |
| Headers do Khronos (OpenGL) | MIT (`GL/glcorearb.h`, `KHR/khrplatform.h`) e Apache-2.0 (`gl.xml`, o registry) | The Khronos Group Inc. | **Dependência TRANSITIVA de BUILD, não de distribuição.** O glintfx vendoriza os headers de declaração da API OpenGL (`glintfx/third_party/khronos/`) para gerar e compilar o próprio loader GL. São `typedef`, `#define` e protótipo: **nenhum código de terceiro do Khronos entra no binário** por essa via, ao contrário do miniaudio e do gamecontrollerdb, que entram. Listado pelo mesmo motivo do Catch2, porque esta tabela se pretende exaustiva. |
| glad (loader OpenGL 3.3 core, gerado) | MIT (gerador e headers gerados; ver `platform/rmlui/RmlUi_Include_GL3.h`) | David Herberth (<https://github.com/Dav1dde/glad>) | **NO BINÁRIO.** Loader de funções GL gerado pelo glad 2.0.0-beta em 2022 e **vendorizado FORA de `third_party/`**, em `GusEngine/platform/rmlui/RmlUi_Include_GL3.h` (3.582 linhas). ⚠ **O nome do arquivo engana:** o prefixo `RmlUi_` é resquício do backend aposentado, e o `README.md` daquela pasta o descreve como header vendorizado do glad. Achado em 2026-07-28, quando a passada de SPDX parou nele: **nenhuma varredura de `third_party/` o encontraria**, e ele não constava nem aqui nem no `ACKNOWLEDGMENTS.md`. O vizinho `gl3_loader.cpp` é código nosso e recebeu SPDX normalmente. |
| Catch2 | BSL-1.0 (Boost) | catchorg / Phil Nash, Martin Hořeňovský e contribuidores (<https://github.com/catchorg/Catch2>) | Framework da suíte de testes (`ctest` + Catch2 nas 4 camadas). Via FetchContent, pin `v3.7.1` em `GusEngine/CMakeLists.txt` (linha 349). **Não entra no binário distribuído** (linka só nos alvos de teste, que não são empacotados no release), então a obrigação de atribuição da BSL-1.0 não recai sobre o pacote do jogo; fica listado aqui porque esta tabela se pretende exaustiva e o Catch2 é dependência de build real. |
| Fontes Noto | SIL OFL 1.1 | Google e contribuidores | <https://openfontlicense.org> |
| Fonte Inter | SIL OFL 1.1 | The Inter Project Authors | <https://openfontlicense.org> |
| Fonte JetBrains Mono | SIL OFL 1.1 | JetBrains s.r.o. | <https://openfontlicense.org> |

---

## Atribuição obrigatória (FreeType, FTL)

A FreeType License exige que a documentação de distribuição do produto inclua o seguinte aviso, verbatim:

> This software is based in part on the work of the FreeType Team.

Este arquivo cumpre essa obrigação para o GusWorld. O texto acima também deve acompanhar qualquer pacote de release (mesmo requisito de "incluir este arquivo no pacote de release" do topo deste documento).

---

## Legado dormente (não linkado no binário C++/SDL3 atual)

> Componentes da era Godot/C# (`engine/`/`game/`) e da fase intermediária Qt do pivot. **Removidos do disco no M8 (decommission, 2026-07-22)** — não existem mais em `main`; sobrevivem só na tag `pre-m8-godot-legacy` (histórico/arqueologia, recuperável se um dia for preciso). NÃO entram no binário `gusengine_app` produzido hoje. Reativar exigiria decisão explícita de reverter o pivot C++20+SDL3 (ADR-008) e resgatar o código da tag.

| Componente | Licença | Titular / fonte | Nota |
|---|---|---|---|
| Qt 6 | LGPLv3 (núcleo) + GPLv3 (alguns módulos) | The Qt Company / Qt Project | Camada de plataforma da Fase 1 do pivot original; SUBSTITUÍDA por SDL3 no re-pivot (ADR-008). Não empacotada em nenhum release atual. |
| Godot Engine | MIT | Juan Linietsky, Ariel Manzur e contribuidores | Engine da fase de transição pré-pivot (`engine/`/`game/`); removida do disco no M8, preservada na tag `pre-m8-godot-legacy`. |
| DialogueManager (addon) | MIT | Copyright (c) 2022-present Nathan Hoad | Addon Godot de diálogo; removido do disco no M8 junto do Godot legado, preservado na tag `pre-m8-godot-legacy`. |
| Dear ImGui | MIT | Omar Cornut (ocornut) e contribuidores | UI de debug/dev cogitada na fase Qt; não linkada no binário C++/SDL3 atual. |

*Nota (achado COS-2): a linha antiga de OpenSSL ("Apache-2.0, apenas se empacotado via Qt") foi removida. Qt é legado dormente e o binário SDL3 atual não empacota OpenSSL. Se uma dependência empacotar OpenSSL no futuro, reintroduzir a linha na tabela de dependências ATIVAS.*

---

## Bibliotecas C++ vendorizadas em `GusEngine/third_party/` (atualizado no M9, 2026-07-22)

Codigo-fonte incorporado no repo (filosofia zero-dep), cada lib com seu arquivo `LICENSE` preservado na pasta. Catalogo historico das 33 candidatas avaliadas: [`docs/tech/libs-vendoring-candidatas.md`](docs/tech/libs-vendoring-candidatas.md).

**Higiene do M9 (2026-07-22):** das 33 libs vendorizadas em 2026-06-22, 31 nunca tiveram `#include` real fora de `third_party/` nem `add_subdirectory`/FetchContent no CMake do projeto (miniaudio foi a ultima a cair: desde o commit `d79d880`/F2 do GLINTFX-INTEGRACAO o audio passou a usar `glintfx::Audio`, restando so comentario). As 31 foram removidas do disco (`git rm`); so as 2 efetivamente linkadas ficam:

| lib | licenca | fonte |
|---|---|---|
| stb | MIT ou Public Domain | github.com/nothings/stb |
| monocypher | CC0-1.0 ou BSD-2-Clause (dupla) | github.com/LoupVaillant/Monocypher | ADR-015 (SAVE-CRYPTO-V2). Tag 4.0.3, commit `ab2b16dd619ad5f6979a4fbe69cfa324a6fcc35f`. UNICO vendor com build proprio (2 arquivos C, compilado via `third_party/monocypher/CMakeLists.txt`); `stb` e header-only. AEAD XChaCha20-Poly1305 + Argon2id do envelope de save v2. |

Nota de compatibilidade: ambas permissivas e compativeis com a Apache License 2.0 do jogo.

**Precisão de inventário sobre o `stb` (2026-07-28; atualizado no bump v0.27.0 e na purga `PURGA-STB-NOSSO`, ambos em 2026-07-30): o binário carrega DUAS cópias dele, e desde a purga elas não têm mais o mesmo conteúdo.** A linha acima descreve a nossa, em `GusEngine/third_party/stb/`, hoje com **dois** single-headers: `stb_image_write.h` (encode de PNG para screenshot/probe) e `stb_truetype.h` (rasterização de fonte, `platform/src/render2d/font_atlas.cpp`). O **`stb_image.h` (decode) saiu da nossa cópia em 2026-07-30**: a decodificação migrou para `glintfx::decode_image_file` (commits `46a12e3` e `9387155`), o header ficou sem nenhum consumidor (zero `#include` real no repo inteiro: produção, testes e `app/tools/`) e foi removido por `git rm`. O decode continua dentro do binário distribuído, mas pela cópia DELES, não pela nossa; nenhuma obrigação de atribuição muda com isso, porque a lib creditada é a mesma. _(Correção de omissão, não mudança de estado: o `stb_truetype.h` já estava em uso real e não constava desta nota até esta revisão: a nota falava de "dois single-headers" e listava dois, mas o vendor tinha três. Registrado aqui porque um inventário de atribuição que esquece um arquivo compilado é exatamente o defeito que esta seção existe para evitar.)_

O glintfx vendoriza as SUAS PRÓPRIAS cópias de `stb_image.h` e `stb_image_write.h` e compila as duas implementações — `stb_image_impl.cpp` (desde sempre, ver `glintfx/src/render_gl3.cpp:25-26`) e **`stb_image_write_impl.cpp` (novo na v0.27.0, IMG-ENCODE)** —, ambos dentro da OBJECT library `glintfx_image`, que **não tem flag de desligar**: as duas entram no binário distribuído sempre. Somando os dois lados, o único arquivo hoje vendorizado **em dobro** é o `stb_image_write.h`: o decode existe só do lado deles, o TrueType só do nosso. Mesma biblioteca, mesmo autor, mesma licença e — conferido byte-a-byte no bump v0.27.0 — **exatamente a mesma versão** (`stb_image_write` v1.16 nas duas cópias), logo não há obrigação adicional nem risco de divergência de comportamento; fica registrado para o inventário bater com o que o binário realmente contém. Nota de link, medida por `nm` no executável final: só **uma** definição de `stbi_write_png` sobra no binário (o objeto do lado que o linker resolve primeiro satisfaz o símbolo e o membro de arquivo do outro lado nunca é puxado), então a dupla vendorização não gera símbolo duplicado nem inflação do binário pelo dobro do stb.

---

## Links das licenças (texto oficial)

Referência por nome + URL; o texto legal completo de cada licença vive na fonte oficial e/ou é embarcado junto do respectivo componente no pacote de release.

- **MIT**: <https://opensource.org/license/mit>
- **LGPLv3**: <https://www.gnu.org/licenses/lgpl-3.0.html>
- **GPLv3**: <https://www.gnu.org/licenses/gpl-3.0.html>
- **MPL-2.0**: <https://www.mozilla.org/en-US/MPL/2.0/>
- **FTL (FreeType License)**: <https://freetype.org/license.html>
- **SIL OFL 1.1**: <https://openfontlicense.org>
- **Apache-2.0**: <https://www.apache.org/licenses/LICENSE-2.0>
- **BSL-1.0 (Boost)**: <https://www.boost.org/LICENSE_1_0.txt>
- **zlib**: <https://opensource.org/license/zlib>
- **BSD-2-Clause**: <https://opensource.org/license/bsd-2-clause>
- **Unlicense / Public Domain**: <https://unlicense.org>

---

## Qt: oferta de source (LGPL/GPL), LEGADO, não aplicável ao release atual

Qt entrou no projeto na Fase 1 do pivot C++ e hoje é **legado dormente** (ver tabela acima); o binário C++20+SDL3 atual não empacota Qt. Esta seção fica registrada como referência caso o legado `engine/`/`game/` volte a ser empacotado algum dia (decisão explícita de reverter o pivot). Se isso acontecer: o GusWorld é Apache License 2.0 (permissiva, sem copyleft). O link com os módulos **LGPLv3** do Qt seguiria as obrigações padrão da LGPL (fonte disponível para modificação, ou objetos relinkáveis se estático), independentes da nossa licença. Já o link com módulos sob **GPLv3** do Qt exigiria distribuir o binário combinado sob termos compatíveis com a GPLv3 (copyleft forte); a licença comercial da Qt Company é a via para evitar essa obrigação com link estático dos módulos GPL, ou usar só os módulos LGPL com link dinâmico. Obrigações a cumprir no release, nesse cenário:

- Identificar a versão exata do Qt empacotada.
- Disponibilizar o **source correspondente** do Qt (link para o tarball oficial da versão usada satisfaz a obrigação).
- Manter os avisos de copyright do Qt no pacote.

Fonte do Qt: <https://download.qt.io/official_releases/qt/>

---

## Manutenção

Atualizar esta tabela sempre que adicionar, remover ou trocar a versão de qualquer dependência de terceiro: **biblioteca C++ via FetchContent** (pin no `GusEngine/CMakeLists.txt`), **lib vendorizada** em `GusEngine/third_party/`, **fonte**, **lib do sistema empacotada** no release, ou **vendor transitivo** que entre junto de uma dependência nossa (ver a regra do bump do glintfx abaixo). Auditoria de licença = item RF-9 do pivot.

_(Esta frase citava "addon Godot" e "biblioteca C++/NuGet" até 2026-07-28. Godot, C# e NuGet saíram do projeto no decommission do M8, em 2026-07-22: não há mais addon nem pacote NuGet a rastrear. O legado Godot continua coberto na seção de legado dormente acima, que é registro histórico e não instrução de manutenção.)_

**Dependência transitiva conta, e o bump do glintfx é gatilho obrigatório (regra acrescentada em 2026-07-28).** A formulação anterior mandava atualizar quando "uma dependência de terceiro" mudasse, e foi exatamente por ela que um componente escapou: o glintfx é dependência nossa, mas os vendors DELE não são "nossos" na leitura literal, e a tabela só rastreava o dono antigo. Foi o que aconteceu com o miniaudio, que **não saiu do binário, mudou de dono** (deixou de ser vendorizado por nós e passou a chegar embutido no glintfx), e ficou fora deste documento por isso.

Portanto: **sempre que o pin do glintfx mudar (`GIT_TAG` em `GusEngine/CMakeLists.txt`), reconferir os vendors dele contra esta tabela**, mesmo que nenhuma dependência nossa tenha mudado. O conjunto de vendors do glintfx não é estável entre releases. Conferência mecânica, depois de configurar o build:

```bash
find GusEngine/build/linux-release/_deps/glintfx-src/glintfx/third_party -type f | sort
```

⚠️ **A receita acima era `ls` do diretório até 2026-07-30, e ela FALHOU em silêncio nesse dia.** Um `ls` responde "quais vendors existem", que **não** é a pergunta que este documento precisa fazer — a pergunta é "qual código de terceiro entrou no binário". A v0.27.0 do glintfx não criou vendor nenhum (os mesmos quatro diretórios), mas **acrescentou um arquivo novo dentro de um vendor que já existia** (`stb/stb_image_write.h`), e o `ls` devolvia os mesmos quatro nomes de sempre: teria dito "nada novo" com um single-header de terceiro novo, compilado, dentro do binário distribuído. O `find -type f` é o mínimo que responde à pergunta certa. Vale a regra geral: **listar o contêiner nunca prova o conteúdo.**

No pin **`v0.28.0`** (bump BUMP-GLINTFX-V028, 2026-07-30; reconferido pós-configure com o `find -type f` acima, na árvore realmente baixada) os quatro DIRETÓRIOS continuam os mesmos de `v0.20.0` em diante — `gamecontrollerdb`, `khronos`, `miniaudio` e `stb` —, mas o conteúdo **mudou**: `stb/` passou a trazer **`stb_image_write.h` (71.221 bytes, v1.16)** além do `stb_image.h` que já vinha, e ele é compilado de verdade (`src/stb_image_write_impl.cpp` entrou na OBJECT library `glintfx_image`, que não tem gate desligável), logo **está no binário distribuído**. Isso vem junto do IMG-ENCODE da v0.27.0 (`glintfx::encode_image_memory`/`encode_image_file`, `glintfx/include/glintfx/image.hpp`), que nós ainda **não** consumimos — nosso encode de screenshot continua indo pela nossa própria cópia do `stb_image_write.h`. Sem obrigação nova de atribuição (mesma lib, autor, licença e versão do que já estava creditado), inventário atualizado no parágrafo de precisão do `stb` acima. Além disso a v0.27.0 é o pagamento dos 4 defeitos que o nosso review adversarial provocou (DEC-NOTHROW e TEX-NOTHROW: `decode_image_file`/`decode_image_memory`/`Draw2d::load_texture`/`Draw2d::create_texture` agora `noexcept`), sem flag `GLINTFX_MODULE_*` nova. A v0.26.0 trouxe FW-LOG (`glintfx::set_log_sink`/`log`/`log_info`/`log_warn`/`log_error`, `glintfx/include/glintfx/log.hpp`) e FW-CLOCK (`glintfx::monotonic_now_ns()`, `glintfx/include/glintfx/clock.hpp`), ambos implementados dentro do `glintfx_core` já existente (OBJECT library sempre-ON, junto de `system_clock.cpp`/`version.cpp`) — sem flag `GLINTFX_MODULE_*` nova e sem vendor novo. **Nota à parte, NÃO um vendor novo em `third_party/`:** a partir da v0.10.0 (default de runtime desde então, build-flag adicionada depois) o glintfx compila opcionalmente o motor de fonte PRÓPRIO clean-room (`glintfx/vendor/core/`, sob `GLINTFX_OWN_FONT_ENGINE`, que fixamos OFF desde sempre) — mas `glintfx/vendor/core/README.md` declara explicitamente que é código PRÓPRIO deles (mesmo autor, MPL-2.0, "not third-party code... no entry is added to NOTICE"), então não entra nesta tabela mesmo se algum dia ligarmos a flag. Entrada nova em `third_party/` precisa de linha nova aqui, classificando **se entra no binário distribuído ou se é só build**, porque a obrigação de atribuição depende disso. Vale o mesmo raciocínio para qualquer outra dependência que vendorize código de terceiro (hoje, além do glintfx, o RmlUi é o caso a vigiar).

No pin **`v0.29.0`** (bump BUMP-GLINTFX-V029, 2026-07-31; conferido pela árvore git recursiva das duas tags, arquivo a arquivo, não só por diretório) os quatro DIRETÓRIOS de `third_party/` deles continuam os mesmos (`gamecontrollerdb`, `khronos`, `miniaudio`, `stb`), **zero arquivo novo, zero arquivo removido** — só o SPDX interno de dois arquivos do `gamecontrollerdb` (`README.md` e `gamecontrollerdb_linux.inc`) mudou de `MPL-2.0` pra `Apache-2.0`, o mesmo texto de rotação de licença aplicado ao resto do repositório deles. Esta release **é** a rotação de licença do glintfx (ver linha do glintfx na tabela acima): `SPDX-License-Identifier` mudou em todos os arquivos de código-fonte e no `CMakeLists.txt` deles, a versão do `project()` foi pra `0.29.0`, e o `CMakeLists.txt` ganhou um `install(FILES LICENSE NOTICE ...)` novo — mas esse bloco vive inteiro dentro de `if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)`, que só é verdade quando o glintfx é o projeto top-level, nunca quando consumido via `FetchContent`/`add_subdirectory` como aqui; **este repositório nunca chama `cmake --install` para o glintfx** (o único `cmake --install` do repo é do FreeType, em `tools/winbuild_container.sh:71`, prefixo próprio), então o bloco novo é inerte para nós. Diff dos 20 headers públicos de `glintfx/include/glintfx/` entre as duas tags: **byte a byte idêntico exceto a linha 1 do SPDX**, em todos os 20 arquivos, confirmando por enumeração (não pela palavra deles) que não há mudança de assinatura nem de comportamento. Sem obrigação nova de atribuição.

No pin **`v0.30.0`** (bump BUMP-GLINTFX-V030, 2026-08-05) os quatro DIRETÓRIOS de `third_party/` deles continuam os mesmos (`gamecontrollerdb`, `khronos`, `miniaudio`, `stb`), com **zero arquivo novo, zero removido e zero byte alterado** — nem o SPDX interno, que foi o único delta do bump anterior. **Nenhuma entrada nova para classificar entre "entra no binário" e "só build", logo nenhuma linha nova nesta tabela**; só o número do pin na linha do glintfx acima mudou. ⚠️ **Correção de método, na mesma linhagem do `ls` → `find` da v0.27.0 e da comparação arquivo-a-arquivo da v0.28.0: comparar CAMINHO não prova CONTEÚDO.** A primeira passada desta conferência comparou apenas os `path` da árvore git recursiva das duas tags, viu o diff vazio e teria assinado "idêntico" **sem ter olhado um único byte de conteúdo** — exatamente o mesmo erro de forma que o `ls` cometia (responder uma pergunta mais fraca que a necessária), só que um nível acima. A conferência foi refeita comparando **SHA de blob + path** (`gh api .../git/trees/v0.{29,30}.0?recursive=true`), que é o mínimo que prova conteúdo idêntico nos 19 arquivos. Vale a regra geral, agora na segunda formulação: **listar o contêiner não prova o conteúdo, e listar o nome do conteúdo também não — só o hash prova.** Nota à parte sobre a release em si, relevante para quem auditar o inventário no futuro: a v0.30.0 foi anunciada como "zero mudança na superfície pública", o que é verdade sobre a onda `RMLX-0` mas **falso sobre a tag** (5 headers públicos mudaram e 1 novo entrou, `scroll_types.hpp`); nada disso toca atribuição de terceiro, mas registra-se aqui porque a frase do anúncio, aceita pelo valor de face, teria dispensado a conferência inteira.

*Recomendação técnica de atribuição; validação jurídica formal cabe ao titular.*
