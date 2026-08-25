<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# docs/tech/adr

Os ADRs do `gusworld_legacy`, trazidos verbatim. O código que eles descrevem não existe mais
(L-01: código nasce do zero). O que cada bullet registra é **o que sobrevive** — decisão, padrão ou
lição — e não a stack morta (Godot, C#, Qt, SDL, RmlUi) em que cada um foi escrito. `ADR-010`
foi apagado por ordem do líder (L-24); quatro ADRs abaixo ainda citam o número dele como ponteiro
órfão (ver a seção "pendências de lei" do `GODS_LAWS.md`) — já registrado, não é achado novo.

- `ADR-001` — pausa do deep-lore como gating de engenharia (era Godot). Morto: a instalação do
  Godot que o ADR desbloqueava. Não trazido como princípio à parte: "lore não bloqueia código, puxe
  sob demanda" é cadência de projeto, não decisão técnica; fica registrado aqui, decisão de trazer
  para outro lugar é do líder.
- `ADR-002` — C# .NET AOT em vez de GDScript. Morto por inteiro; o próprio documento já se marca
  **SUPERADO**.
- `ADR-003` — DialogueManager (plugin de diálogo do Godot). Morto por inteiro; o próprio documento
  já se marca **Superseded by ADR-014**.
- `ADR-004` — contrato do `EnvironmentModifier` (ambientes de combate: terreno×clima×período,
  `multAmbiente`). A implementação C# morreu; o conteúdo de fórmula já vive, atualizado, em
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
- `ADR-011` — plano da 1ª onda de áudio (kit mínimo real, alicerce→SFX→música, API pequena sem
  event bus prematuro). Morto: a stack de áudio específica e os caminhos de arquivo. O princípio
  anti-over-engineering (não construir infraestrutura que a onda atual não precisa) é genérico o
  bastante para não precisar de resgate à parte — já é prática corrente do projeto.
- `ADR-012` — plano do M7 (costura de save+combate+diálogo). Morto por inteiro: plano de integração
  de um binário que não existe mais. Cita `ADR-010` (ponteiro órfão já rastreado).
- `ADR-013` — `AssetSource`, porteiro único de acesso a asset sobre filesystem. Morto como código;
  o padrão "fonte editável no repo → build empacota e sela → runtime só lê o artefato" é o mesmo que
  a L-25 já cravou para o catálogo de conteúdo — não precisa de resgate à parte, a lei já o contém.
  Cita `ADR-010` (ponteiro órfão já rastreado).
- `ADR-014` — runtime de diálogo POCO C++20, data-driven. Morto como código (base C++/SDL
  descontinuada); o próprio documento já se marca supersedendo o ADR-003. O padrão (runtime headless
  POCO, sem lib externa) é consistente com a L-17 de hoje, sem contradição.
- `ADR-015` — segurança de save v2 offline: AEAD, machine-binding só no Hardcore, âncora selada
  anti-rollback, wipe por crypto-shred. É o ADR mais próximo da L-25 vigente — quase um rascunho
  dela. Único ponto revogado: a fonte da cifra (vendoriza Monocypher; a lei manda vir do GlintFx).
  Ver achado em `../_INDEX.md` — sem marcador de superação no documento.
- `ADR-016` — motor `techMagic` de efeito data-driven (lista de instruções, não VM/bytecode). Morto
  como código; o princípio (dado antes de máquina virtual) é o mesmo que a L-04/L-17 de hoje
  consagram. Já tem edição de terminologia aplicada por outra passada (troca pontual de
  "easter-egg" por "subtexto") — não é achado, já está tratado.
- `ADR-017` — combate unificado por Action-Clock (pedido do Gus Dragon + velocidade por carta).
  Decisão de **feel de combate**, não de arquitetura de engine; não contradiz nenhuma lei. Fica como
  registro de design histórico; se o combate atual precisar dessa decisão, é o `product-manager` ou
  o `lead-game-designer` quem confirma se já está em `docs/design/mecanicas/`.
- `ADR-018` — correção de flash visual por contexto GL duplicado (bug fix pontual num renderer que
  não existe mais). Morto por inteiro, sem princípio a resgatar. Cita `ADR-010` (ponteiro órfão já
  rastreado).
- `ADR-019` — arquitetura de conteúdo atômica/data-driven (canonização de um padrão já emergente).
  Precursor direto da L-04 ("proibido monolito; cada elemento do jogo é átomo com POCO próprio").
  Sem contradição; decisão viva, já refletida em lei.
- `ADR-020` — peças de dado componíveis em módulos estreitos + regra do mundo data-driven.
  Precursor direto da L-04/L-17. Sem contradição; decisão viva, já refletida em lei.
- `ADR-021` — rotação de licença de código para Apache 2.0 + assets em duas zonas por data de
  corte. Superada pela L-08 (AGPL-3.0-or-later + todos os direitos reservados, regime único). Ver
  achado em `../_INDEX.md` — sem marcador de superação no documento.

**Não presentes** (deletados por ordem do líder sob a L-24, não recriar): `ADR-008`
(repivot Qt→SDL3) e `ADR-009` (motor de UI/RmlUi) foram apagados junto com `ADR-010` (adoção do
GlintFx em modo embed) — o modelo de arquitetura que os três descreviam foi revogado pela LEI ZERO.
Duas variantes de nome (`ADR-008-repivot-qt-to-sdl3.md`, `ADR-009-rmlui.md`) existiam no
`gusworld_legacy` só como histórico de rename (o nome da dependência foi raspado do arquivo antes
da apagagem) — confirmado por `git log --follow`, mesmo conteúdo, nada de novo a trazer.
