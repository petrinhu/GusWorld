# techMagic — conceito (record de brainstorm, v1)

> **Status:** REGISTRO de brainstorm colaborativo (líder + orquestrador, 2026-07-10). **NÃO é canon final** — é o registro das decisões do criador nesta sessão. A canonização polida (reescrever Pillar/terminologia/lore-bible, gerar o nome in-world, desenhar mecânica) e o código são **feats pendentes** (ver `TODO.md`: `TECHMAGIC-CANON`, `TECHMAGIC-EXECUTOR`), a serem executados por agentes especialistas com aprovação do líder. Nada aqui foi escrito no canon nem no código ainda.
>
> **CORREÇÃO DE TERMO (criador, 2026-07-12):** o termo canônico do meta-conceito é **"TechnoMagik"** (não "techMagic"), e o adjetivo/relativo de tudo que se liga a ele é **"technologiko"**. Este doc ainda usa "techMagic" no corpo (grafia antiga); a reconciliação completa da grafia (corpo do doc + a seção Tavus-Eco abaixo + eventual nome de símbolo de código) fica no item `TECHMAGIC-CANON`. Daqui pra frente, em prosa/lore, usar **TechnoMagik**.

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

## Tavus-Eco (canon aprovado 2026-07-12)

Aplicação mais literal de "magia = software". O **Tavus-Eco** é um self compilado: diante da morte, o mestre executa sobre si mesmo o mesmo gesto que qualquer conjuro exige, compila um snapshot finito da própria voz, do próprio método e da lição que escolheu preservar, e o sela no artefato que carrega a carta do Codex. O eco dorme, inerte, indistinguível de registro morto, até que alguém resolva a prova que o mestre deixou como condição de execução (o puzzle da missão). Resolvida a prova, o Tavus-Eco roda: o mestre aparece, projeção compilada, entrega a carta pela própria mão e conversa, presença limitada ao que foi compilado, não fantasma nem sobrevivente. O mestre continua morto na sua era; o que a party encontra na Era 3 é o que ele executou de si mesmo antes do fim. É a razão canônica pela qual a party ENCONTRA cada mestre pra receber a carta, sem que nenhum deles esteja vivo.

- **Etimologia:** raiz Sylvarin `tavus-` ("o pulso que executa"), híbrido moderno no padrão de Tavus-Drive. Nome in-world puro em Sylvarin fica pendente (candidato com `lhin-`, canto/fala), junto do batismo do techMagic.
- **Criação (Era 2, 17 mestres):** o mestre compila-se diante da morte, cada um à sua cara (Faraday trança o eco na malha, von Neumann aplica a si o teorema da auto-cópia, etc.). **Variante Era 1 (arcano-cerimonial, os 3 mestres OCU):** não "compila"; grava o próprio eco em canto, na pedra, na roda de memória, no instrumento ritual, do jeito que a era sabia guardar o que não podia perder. Mesma lógica formal (Pillar 1 é universal), só o substrato muda; a taxonomia da Era 3 classifica os dois sob o mesmo nome, porque a raiz `tavus-` é anterior a todos eles.
- **Ativação:** a prova/puzzle da missão é a condição de execução. Resolveu, roda.
- **Comportamento:** entrega a carta e conversa, limitado ao snapshot compilado (a descoberta, a voz, a lição). Não aprende, não é onisciente.
- **Limite duro (guarda do segredo):** o Tavus-Eco é **camada 1** (techMagic comum, computável). A **camada 3** (origem, Estilhaçamento, convergência) é incompilável por definição, logo o eco estruturalmente NÃO pode conter nem revelar cosmologia de origem. O momento "O eco" (§5 de cada doc mestre, o déjà-vu, a cicatriz do Einstein) é justamente a parte da vida que a compilação não captura; o eco pode, no máximo, hesitar onde o original hesitou, sem saber por quê. O Tavus-Eco e o "eco" do §5 são fenômenos DISTINTOS e nunca são ligados em texto.
- **Nome Sylvarin puro (criador, 2026-07-12): `Morlhin`** ("voz-sombra"), de *mor-* (sombra) + *lhin-* (voz/canto). Registrado no léxico (`docs/narrative/lingua/02-lexico-semente.md`). Convivência de termos: **Tavus-Eco** = o nome híbrido moderno da Era 3 (padrão de Tavus-Drive); **Morlhin** = o nome puro/antigo em Sylvarin. Mesmo fenômeno, dois registros.

### Cena de encontro (Morlhin / Tavus-Eco) — design canon (criador, 2026-07-12)

**Formato: TEMPLATE UNIFORME de 5 beats + sabor por mestre** (barato: mesma estrutura, só variam diálogo e a cor do glow; dev solo). A "projeção" é retrato/sprite do mestre + glow simples, nunca 3D/animação pesada.

1. **Ativação:** o puzzle da missão é resolvido; o artefato/console (Era 2) ou o instrumento ritual (Era 1, os 3 OCU) "roda" e acende, com o glow do campo do mestre (a cor do campus dele, ver `mundo-topologia.md` §6).
2. **Aparição:** o Morlhin se ergue, projeção compilada. Não é fantasma nem sobrevivente: é o que o mestre executou de si antes do fim.
3. **A lição (estrutura fixa, insight do criador 2026-07-12):** a fala do mestre segue **dois movimentos**: (a) ele conta os **feitos reais dele para a humanidade em linguagem ACESSÍVEL a leigo** (sem jargão, o jogador aprende de verdade o que a figura fez); (b) então **deriva a carta disso**: "portanto, o efeito desta carta é [resumo do efeito em linguagem simples]". Assim o efeito nunca parece arbitrário, sai do feito real da figura. É o coração do ensino diegético. Limitada ao snapshot: não aprende, não é onisciente; pode hesitar onde o original hesitou, sem saber por quê (guarda do §5, camada 3 incompilável). Handoff de redação = `narrative-writer`, com este molde de 2 movimentos + o efeito escolhido da carta (ROSTER-EFEITOS-CARTAS).
4. **A entrega:** entrega a carta do Codex pela própria mão (UI da carta).
5. **Dissipa:** o Morlhin se apaga (one-shot, momento único e melancólico com um morto); o artefato volta inerte. **A lição fica arquivada no Diário/Codex** para reler (preserva o objetivo de ENSINO sem quebrar o "snapshot finito").

- **Sabor por mestre:** diálogo (voz + descoberta + lição) + cor do glow (por campo) + substrato (Era 2 = console compilado; Era 1 OCU = canto/pedra/roda-de-memória/instrumento ritual, cf. canon acima).
- **Custo:** BARATO por encontro (retrato + glow + diálogo + UI de carta + 1 entrada de Diário). Escala com o roster já desenhado.
- **Handoff pendente:** a REDAÇÃO do diálogo de cada um dos 20 mestres (voz + lição) é feat do `narrative-writer` (colaborativo, aprovação do criador); aqui está só o template estrutural. Ver `docs/design/roster-analogos/OBRA-DE-FICCAO-E-METODOLOGIA.md` AMB-24.

## Fios ainda abertos (viram sub-decisões das feats pendentes)

- **Nome in-world em Sylvarin** (candidatos via `narrative-designer` → líder escolhe).
- **Manifestação em mecânica de jogo:** o jogador VÊ a corrupção/overflow (camada 2)? O Tavus-Drive tem estados visíveis? A camada 3 vira um beat de gameplay? (`lead-game-designer`).
- **Quem PRATICA techMagic e como se aprende** (relação com Knowledge Progression / scan).
- **Relação com a terminologia existente:** Glyph / Token / Conjuro / Codex / C-Arcane — techMagic é o meta-frame; esses são seus componentes/vocabulário. Formalizar na canonização.

## Cross-refs

`docs/design/pillars.md` (Pillar de magia — alvo da reformulação) · `docs/narrative/characters/patch-zero.md` (reclassificação) · `docs/narrative/lingua/` (léxico Sylvarin pro nome in-world) · `docs/narrative/deep/eras/cosmologia-origem-deep.md` (camada 3, origem) · `docs/design/mecanicas/combat.md` + `combat_state_machine.cpp` (o executor) · Pillar 3 / Tavus-Drive · `docs/design/brainstorm-backlog.md`.
