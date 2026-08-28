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
  marca **SUPERADO** (por ADR-021, que por sua vez está superado pela L-08 — ver achado).
- `ADR-006` — HMAC-SHA256 próprio + envelope binário `GDS2` do save. A fonte da primitiva (escrita
  em casa) está revogada pela L-25 (crypto vem do GlintFx); o desenho do envelope
  (magic/length/payload/hmac) é o ancestral direto do envelope que a L-25 exige hoje. Ver achado em
  `../_INDEX.md` — sem marcador de superação no documento.
- `ADR-007` — `controls.json` legível com hash-128 de detecção e restauração via backup no save.
  O formato texto está revogado pela L-18; o mecanismo de segurança (detectar sem prevenir, diff ao
  jogador, backup embutido em todo save, hash sobre forma canônica) é o desenho mais detalhado do
  corpus e antecede a L-25 quase ponto a ponto. Ver achado em `../_INDEX.md` — sem marcador de
  superação no documento.
- `ADR-015` — segurança de save v2 offline: AEAD, machine-binding só no Hardcore, âncora selada
  anti-rollback, wipe por crypto-shred. É o ADR mais próximo da L-25 vigente — quase um rascunho
  dela. Único ponto revogado: a fonte da cifra vendorizada; a lei manda vir do GlintFx. **Onda 1
  do plano de execução (vendorizar a cifra) precisa de revisão do líder antes de qualquer
  implementação — ver achado em `../_INDEX.md`.**
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
- `ADR-021` — rotação de licença de código para Apache 2.0 + assets em duas zonas por data de
  corte. Superada pela L-08 (AGPL-3.0-or-later + todos os direitos reservados, regime único). Ver
  achado em `../_INDEX.md` — sem marcador de superação no documento.

**Não presentes** (deletados por ordem do líder sob a L-24, não recriar): `ADR-002`, `ADR-003`,
`ADR-008`, `ADR-009`, `ADR-010`, `ADR-011`, `ADR-012`, `ADR-013`, `ADR-014` e `ADR-018` foram
apagados — cada um descrevia, por inteiro ou quase, um modelo de arquitetura ou uma biblioteca de
plataforma revogados pela LEI ZERO. Nenhum princípio de fundo desses nove sobrevivia à parte do
código morto que descreviam; onde algum trecho tinha vida própria (ex.: anti-over-engineering do
plano de áudio), já é prática corrente do projeto, sem precisar de resgate textual.

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
