# Áreas secretas dentro das dungeons — índice

> **Status:** PROPOSTA de design (nível conceito, não layout fino). Item `D34` da `TODO.md`.
>
> **Escopo desta pasta, e o que ela NÃO faz:** `docs/design/mundo-topologia.md` §4.1 fixou a
> QUANTIDADE (32 áreas secretas, escalada por tipo de dungeon) e disse explicitamente que
> "posição, conteúdo e recompensa de cada área secreta ficam para a fase de PRODUÇÃO, junto do
> layout fino de cada dungeon, com o `level-designer` e a engine de mapa" (§10, item 1). O `D34`
> da `TODO.md` repete a mesma reserva: "este item só fixa a quantidade". Esta pasta portanto
> **não fecha posição em grade, coordenada de sala nem item exato de recompensa** — isso segue
> deferido, porque a engine de mapa não existe. O que ela entrega é o mesmo nível de detalhe que
> `mundo-topologia.md` §4 já usa para as próprias 13 dungeons ("só conceito": tema + gimmick +
> forma) — aqui, aplicado a cada área secreta: a **pista** que a torna encontrável, o **verbo**
> que abre ela, a **categoria** de recompensa (não o item exato) e o **beat** de storytelling
> ambiental que ela carrega. Quando a engine de mapa e o `level-designer` de produção chegarem à
> fase de layout fino, este documento é o ponto de partida, não o substituto dele.
>
> **Cross-refs:** `docs/design/mundo-topologia.md` §4 (conceito das 13 dungeons), §4.1 (a
> aritmética que gera os 32), §10 (o que fica para produção); `docs/design/mecanicas/save-por-local.md`
> (regra de save por dungeon, relevante para o risco de cada desvio); `CHARS.md`, `PLACES.md`
> (personagens e sub-locais citados); TODO.md item `D34`.

## Aritmética conferida (05/09/2026, contra o `mundo-topologia.md` de hoje)

A contagem de tipos de dungeon em `mundo-topologia.md` §4 não mudou com a seção nova §4.2
(cenas de clímax de área). §4.2 separa **cena de clímax** de **dungeon de exploração** — ela
esclarece que o ritual do Núcleo Mandelbrot não é dungeon e não compete pelo orçamento de 13, mas
não retira nem acrescenta nenhuma das 13 dungeons existentes, nem muda o tipo de nenhuma delas.
A tabela de tipos (`mundo-topologia.md` §4, "Tipos de dungeon") segue: tutorial 1 · puzzle puro 4
· só batalhas 3 · labirinto 2 · mista 3 — treze dungeons, o mesmo total de sempre. A aritmética do
`D34`/§4.1 continua válida sem ajuste:

| Tipo | Dungeons | Áreas secretas cada | Subtotal |
|---|---|---|---|
| labirinto | 2 | 5 | 10 |
| puzzle puro | 4 | 3 | 12 |
| mista | 3 | 2 | 6 |
| só batalhas | 3 | 1 | 3 |
| tutorial | 1 | 1 | 1 |
| **Total** | **13** | — | **32** |

## A régua aplicada (por que cada área é área secreta, não sala extra)

Cada uma das 32 áreas, nos 13 arquivos abaixo, responde a mesma pergunta antes de existir: **que
pista o espaço dá antes de a área existir para o jogador?** Nenhuma pede "encoste em toda
parede" — cada uma nomeia um sinal (visual, sonoro, tátil ou de padrão) que já está ali antes de
o jogador saber que há algo a achar. E o número por tipo tem uma razão que cada arquivo honra:
labirinto ganha 5 porque o verbo dele é orientação espacial, e as áreas secretas de um labirinto
pedem leitura de padrão/repetição; dungeon só de batalha ganha 1, e essa única área nunca pede
orientação espacial — ela é gate de maestria de combate ou risco extra, nunca puzzle de
exploração.

## Os 13 arquivos, um por dungeon

| # | Arquivo | Dungeon | Tipo | Áreas | Status |
|---|---|---|---|---|---|
| 1 | `01-dutos-aparato-abertura.md` | Dutos Infernais — aparato (abertura) | tutorial | 1 | fechado |
| 2 | `02-dutos-laboratorio-fir.md` | Dutos Infernais — laboratório FIR | só batalhas | 1 | fechado |
| 3 | `03-mirage-festival.md` | Setor Mirage — o Festival | puzzle puro | 3 | fechado |
| 4 | `04-periferia-vielas.md` | Periferia (residencial) — vielas | labirinto | 5 | fechado |
| 5 | `05-ferrovelhos-reduto-dante.md` | Ferrovelhos — reduto do Dante | só batalhas | 1 | fechado |
| 6 | `06-zona-silencio-acustico.md` | Zona do Silêncio #1 — puzzle acústico | puzzle puro | 3 | fechado |
| 7 | `07-zona-silencio-mista.md` | Zona do Silêncio #2 — mista | mista | 2 | fechado |
| 8 | `08-orla-recursiva.md` | Orla Recursiva — labirinto fractal | labirinto | 5 | fechado |
| 9 | `09-selve-sombria.md` | Selve Sombria — mista | mista | 2 | fechado |
| 10 | `10-catedrais-liturgico.md` | Catedrais #1 — puzzle litúrgico | puzzle puro | 3 | fechado |
| 11 | `11-catedrais-batalhas.md` | Catedrais #2 — só batalhas | só batalhas | 1 | fechado |
| 12 | `12-selve-profunda-kola.md` | Selve Profunda — Kola-SG3-12262 (final) | mista (clímax) | 2 | ⚠️ RETIDO |
| 13 | `13-area-faraday.md` | Área faraday especial | puzzle puro (EM) | 3 | fechado |
| | | | **Total** | **32** | |

## Estado de aprovação (05/09/2026, decisão do líder por `AskUserQuestion`)

**12 arquivos fechados, 1 retido.** Os arquivos 1 a 11 e 13 (12 dungeons) tiveram a granularidade
de design confirmada pelo líder e seguem como estão.

**O arquivo 12 (`12-selve-profunda-kola.md`) está RETIDO, separado dos outros doze: nada nele é
canon até o líder revisar.** É o único que toca uma dungeon já canonizada em profundidade
(`docs/design/dungeons/kola-sg3-12262-espelho-obsidiana.md`), com beat sheet fixado pelo líder e
metade do conteúdo em área cifrada, que acabou de passar por um recuo de material para a cifra —
um erro ali custa mais que nos outros doze. As duas áreas secretas propostas ali são desvios
opcionais ancorados nos beats já públicos (1 e 3), reconferidas em 05/09/2026 contra o texto
público pós-recuo sem divergência encontrada, sem tocar nenhum beat cifrado (4 em diante) nem
alterar a cadeia de missão existente.
