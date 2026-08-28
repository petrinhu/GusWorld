<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# docs/tech/adr

Os ADRs do `gusworld_legacy`, trazidos verbatim. O código que eles descrevem não existe mais
(L-01: código nasce do zero). O que cada bullet registra é **o que sobrevive** — decisão, padrão ou
lição — e não a stack morta em que cada um foi escrito.

- `ADR-001` — pausa do deep-lore como gating de engenharia. Morto: a instalação da engine antiga
  que o ADR desbloqueava. Não trazido como princípio à parte: "lore não bloqueia código, puxe
  sob demanda" é cadência de projeto, não decisão técnica; fica registrado aqui, decisão de trazer
  para outro lugar é do líder.
- `ADR-004` — contrato do `EnvironmentModifier` (ambientes de combate: terreno×clima×período,
  `multAmbiente`). A implementação antiga morreu; o conteúdo de fórmula já vive, atualizado, em
  `docs/design/mecanicas/combat.md` — redundante, não contraditório. Ver achado em `../_INDEX.md`.
- `ADR-005` — GPLv3 + CC-BY-SA + livros reservados. Morto por inteiro; o próprio documento já se
  marca **SUPERADO** (pela rotação de licença de código que, por sua vez, foi superada pela L-08).
- `ADR-006` — editado em `G21` (28/08/2026): saiu a primitiva HMAC-SHA256 escrita em casa (a
  cripto vem do GlintFx, LEI ZERO/L-25); ficaram o formato de serialização binário do `domain/`, o
  carimbo de data/hora por save e a semente de RNG por data/hora/milissegundo, mais a nota de que
  migrators forward-only operam sobre structs versionadas.
- `ADR-007` — editado em `G21` (28/08/2026): saiu o desenho inteiro em torno do arquivo de
  controles em JSON legível (formato texto proibido pela L-18); ficaram as três camadas
  anti-tamper do save que não dependiam do JSON (detectar sem crashar, slot-id selado dentro do
  payload, chave do selo derivada por KDF em vez de embutida crua).
- `ADR-016` — motor `techMagic` de efeito data-driven (lista de instruções, não VM/bytecode). Morto
  como código; o princípio (dado antes de máquina virtual) é o mesmo que a L-04/L-17 de hoje
  consagram. Já tem edição de terminologia aplicada por outra passada (troca pontual de
  "easter-egg" por "subtexto") — não é achado, já está tratado.
- `ADR-017` — combate unificado por Action-Clock (pedido do Gus Dragon + velocidade por carta).
  Decisão de **feel de combate**, não de arquitetura de engine; não contradiz nenhuma lei. Fica como
  registro de design histórico; se o combate atual precisar dessa decisão, é o `product-manager` ou
  o `lead-game-designer` quem confirma se já está em `docs/design/mecanicas/`.
- `ADR-019` — arquitetura de conteúdo atômica/data-driven (canonização de um padrão já emergente).
  Precursor direto da L-04 ("proibido monolito; cada elemento do jogo é átomo com POCO próprio").
  Sem contradição; decisão viva, já refletida em lei.
- `ADR-020` — peças de dado componíveis em módulos estreitos + regra do mundo data-driven.
  Precursor direto da L-04/L-17. Sem contradição; decisão viva, já refletida em lei.

**Não presentes** — números aposentados, não reutilizar: `ADR-002`, `ADR-003`, `ADR-008`, `ADR-009`,
`ADR-010`, `ADR-011`, `ADR-012`, `ADR-013`, `ADR-014`, `ADR-015`, `ADR-018` e `ADR-021`.

## ADRs de fundação do GusWorld (código do zero, 25/08/2026)

Os quatro abaixo são decisões novas do líder, tomadas antes da primeira fatia de código, cada
uma com razão de mudar própria (L-33): sistema de build, consumo de dependência, estratégia de
CI e harness de teste são preocupações distintas, ainda que as quatro nasçam da mesma ordem de
serviço.

- [`ADR-022`](ADR-022-cmake-versao-minima-fixada.md) — CMake como sistema de build, com
  `cmake_minimum_required(VERSION 3.28)` medido (não suposto) contra as cinco plataformas
  obrigatórias (L-09, L-20); piso real é o Ubuntu 24.04 (3.28.3), e não conflita com o que o
  GlintFx já exige. Ninja como gerador **nas cinco plataformas, Windows incluído** — divergência
  consciente do próprio job Windows do GlintFx (que usa o gerador padrão do Visual Studio),
  declarada como consequência, não escondida. Sem piso de versão fixado para o Ninja.
- [`ADR-023`](ADR-023-glintfx-submodulo-git-pinado.md) — o GlintFx entra no build como submódulo
  git, pinado num commit específico, em **`framework/GlintFx`** (nome escolhido pelo líder: nem
  `external/`, nem `vendor/` — `vendor` sugeriria fornecedor de terceiro, e o GlintFx é projeto
  irmão dele; `framework` diz o que a pasta é e mantém legível a fronteira da LEI ZERO); atualizar
  o ponteiro é ato deliberado, nunca automático. Vínculo **compartilhado** (`.so`), espelhando o
  próprio GlintFx — decisão consciente do líder, contra a recomendação técnica (estático);
  consequência de ABI entre compiladores declarada explicitamente, com o que o submódulo pinado
  protege e o que não protege fora do nosso CI.
- [`ADR-024`](ADR-024-ci-cinco-plataformas-cachyos-container.md) — CachyOS entra na matriz de CI
  como container da imagem oficial `cachyos/cachyos:latest` (confirmada por precedente já em
  produção no próprio GlintFx), nunca como Arch reconfigurado. Linux compila com GCC **e** Clang,
  os dois, em cada uma das quatro plataformas, ambos bloqueantes (L-20); Windows compila com MSVC.
  Matriz de **NOVE** entradas, confirmado pelo líder (a leitura literal e uniforme, corrigindo um
  "sete" que era erro aritmético do relay, não do trabalho já feito) — razão dele: cobertura
  desigual esconde bug.
- [`ADR-025`](ADR-025-harness-teste-proprio-minimo.md) — harness de teste próprio e mínimo
  (asserção, registro, `main`), e a correção registrada de que a LEI ZERO não fala sobre
  ferramenta de build nem de teste; a cultura de dependência zero vem da L-07 do `GODS_LAWS.md`
  do GlintFx, não de uma lei do GusWorld.
