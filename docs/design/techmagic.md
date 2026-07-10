# techMagic — conceito (record de brainstorm, v1)

> **Status:** REGISTRO de brainstorm colaborativo (líder + orquestrador, 2026-07-10). **NÃO é canon final** — é o registro das decisões do criador nesta sessão. A canonização polida (reescrever Pillar/terminologia/lore-bible, gerar o nome in-world, desenhar mecânica) e o código são **feats pendentes** (ver `TODO.md`: `TECHMAGIC-CANON`, `TECHMAGIC-EXECUTOR`), a serem executados por agentes especialistas com aprovação do líder. Nada aqui foi escrito no canon nem no código ainda.

## O que é

**techMagic** é o **nome-guarda-chuva** de TODO o sistema de magia do GusWorld. Consolida, num único termo, o framing canônico "magia = **sistema formal computável**" (que já substituiu o antigo "magia = software" pra acomodar a relojoaria de latão do Bento "Requiem" — ver Pillar de magia em `pillars.md`).

Decisão de nível: techMagic é o **guarda-chuva**, não uma vertente. Cobre runa compilada, silício e engrenagem de latão — qualquer substrato que rode um sistema formal.

## Camada dupla (híbrido meta + in-world)

- **Meta (nosso):** "techMagic" é a etiqueta de design/comunicação que usamos (docs, GDD, talvez marketing).
- **In-world (Sylvarin):** existe, em paralelo, um **nome canônico em Sylvarin** (A DEFINIR — feat `TECHMAGIC-CANON`, via `narrative-designer` com o léxico Sylvarin em `docs/narrative/lingua/`) que os personagens de fato usam no mundo. Dois rótulos, um conceito.

## Princípio

Toda magia é um **sistema formal computável** rodando num substrato físico. Corolário forte: **nada é verdadeiramente aleatório** dentro do mundo (o "caos" é complexidade determinística — Mandelbrot, ruído Perlin — ou corrupção do sistema; ver 3 camadas).

## Metafísica de 3 camadas (decisão do líder)

1. **A regra — tudo é techMagic (computável).** O universo é, na base, computação sobre substrato.
2. **A corrupção — o terror (de dentro do sistema).** O "caos genuíno" / **Patch-Zero** = techMagic **quebrado**: bug, overflow, undefined behavior, o sistema se voltando contra si. **RECLASSIFICAÇÃO CANÔNICA:** Patch-Zero deixa de ser "caos irredutível / limite do conhecimento" (framing antigo do Pillar) e passa a ser **corrupção computável** — muito complexo, mas não metafisicamente-fora. É mais assustador: a chamada vem de dentro da casa.
3. **O acima — o sublime, raríssimo.** Um punhado de coisas-limiar ficam **genuinamente acima do formalizável**: a **origem multiversal** (o "acidente"/o programador — ver `docs/narrative/deep/eras/cosmologia-origem-deep.md`), o **divino**, o **Helíaco Vyr**. Não é terror, é o sublime — o "source fora do programa em execução", o horizonte tipo halting-problem. Reservado pros beats-chave da lore.

> A qualidade "irredutível / limite do conhecimento" **migra** do Patch-Zero (camada 2) pra camada 3.

## Camada de código (o easter-egg vira literal)

No jogo cujo tema É "magia = software", o próprio código encarna a tese: **`techMagic` será o nome de um símbolo de código MUITO importante** — especificamente **o executor de conjuros**: o runtime que pega um cartão COMPILADO do Codex e EXECUTA o efeito no combate.

Encaixe canônico que fecha o triângulo **conceito ↔ hardware do personagem ↔ código real**: o `techMagic` executor **modela o Tavus-Drive de pulso do Gus** — o "executor de cartões" do Pillar 3 (óculos táticos ↔ matriz ortodôntica ↔ Tavus-Drive executor).

### Fork de arquitetura (decisão PARA A IMPLEMENTAÇÃO, não agora)

Alerta anti-over-engineering registrado no brainstorm: o `techMagic` executor pode ser
- **(a) interpretador/VM de conjuro de verdade** — cartas *compilam* pra bytecode/AST que o `techMagic` executa. Lindo tematicamente, mas cheira OE pra G1; OU
- **(b) o resolvedor dos cartões data-driven que já existem** (`resolve_use_card` em `combat_state_machine.cpp`, renomeado/refatorado pra `TechMagic`). Entrega o mesmo easter-egg com risco zero.

Decisão de `software-architect` + `gameplay_engineer` **na hora de implementar** (feat `TECHMAGIC-EXECUTOR`), não no brainstorm.

## Fios ainda abertos (viram sub-decisões das feats pendentes)

- **Nome in-world em Sylvarin** (candidatos via `narrative-designer` → líder escolhe).
- **Manifestação em mecânica de jogo:** o jogador VÊ a corrupção/overflow (camada 2)? O Tavus-Drive tem estados visíveis? A camada 3 vira um beat de gameplay? (`lead-game-designer`).
- **Quem PRATICA techMagic e como se aprende** (relação com Knowledge Progression / scan).
- **Relação com a terminologia existente:** Glyph / Token / Conjuro / Codex / C-Arcane — techMagic é o meta-frame; esses são seus componentes/vocabulário. Formalizar na canonização.

## Cross-refs

`docs/design/pillars.md` (Pillar de magia — alvo da reformulação) · `docs/narrative/characters/patch-zero.md` (reclassificação) · `docs/narrative/lingua/` (léxico Sylvarin pro nome in-world) · `docs/narrative/deep/eras/cosmologia-origem-deep.md` (camada 3, origem) · `docs/design/mecanicas/combat.md` + `combat_state_machine.cpp` (o executor) · Pillar 3 / Tavus-Drive · `docs/design/brainstorm-backlog.md`.
