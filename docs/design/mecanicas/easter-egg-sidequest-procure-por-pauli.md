<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Easter egg: a sidequest "Procure por Pauli"

**Status:** CANON FECHADO. Decisão do líder (12/09/2026, verbatim em §1) fixa a estrutura da piada;
três decisões seguintes, do mesmo dia, fecham local, recompensa e gatilho, todas em área cifrada.
Prosa final (a fala do Gus, se houver, e qualquer redação além das linhas ilustrativas abaixo) cabe a
`narrative-writer`, por L-76 global: este documento é arquitetura, não prosa.

**Ponteiros (L-30):** cena em `docs/narrative/comic-reliefs/easter-eggs-educacionais.md` (EE-25);
NPC em `CHARS.md` §6 (Joaquim Bartolomeu), mora na Zona do Silêncio; taxonomia e naming de carta em
`docs/design/mecanicas/cartas-technomagik.md` §2.2 e `docs/design/mecanicas/cartas/pulso_eletrico.md`
(padrão de carta comum de família Elétrico). As 2 áreas secretas já fechadas da Zona do Silêncio
(`docs/design/dungeons/areas-secretas/07-zona-silencio-mista.md`) e o orçamento de 34 áreas secretas
(`mundo-topologia.md` §4.1) não são tocados por este documento.

**Local, recompensa, ambientação e a referência real são área cifrada:**
`docs/_secret/mecanicas/sidequest-procure-por-pauli-solucao.md`.

## 1. Decisão do líder (12/09/2026), verbatim

*"opcao um, será sidequest dada por npc. Ele vai dizer: 'para achar o item [...], você deve procurar
por Pauli!'. O jogador vai imaginar que Pauli é alguém que vai dar o item para ele, e nunca vai achar
nenhum npc com esse nome. Mas, ao abrir essa porta num dungeon, vai encontrar [...] lá."*

Um NPC manda o jogador procurar uma pessoa chamada Pauli. Essa pessoa não existe no elenco do jogo,
em lugar nenhum. "Pauli" é, na verdade, uma porta escondida, em local e com recompensa fechados na
área cifrada. Quem a abre encontra o item ali.

## 2. Quem dá a missão: Joaquim Bartolomeu, e por quê

**Joaquim Bartolomeu (32 anos)** já é NPC canônico (`CHARS.md` §6; ficha completa em
`docs/narrative/environments/07-zona-do-silencio.md` §6): técnico Underground, filho de luthiers,
descobriu por acidente a Câmara do Equinócio Acústico em -3, e já é caracterizado como guardião de
segredo por natureza: *"quando alguém pede pra entrar, ele pergunta primeiro: 'por quê?' Resposta
errada e ele finge não saber do que está se falando"*, e *"se você sair daqui e contar, eu não te
conheci."* Ele é exatamente o tipo de NPC que teria topado, nos próprios reparos de cabo e nos
diagramas velhos do distrito, com um nome riscado que ele mesmo não sabe explicar, sem qualquer
pista de para onde ele apontava, e que repassaria essa curiosidade ao Gus sem fingir saber mais do
que sabe (a distância entre onde Joaquim achou o nome e onde a porta realmente fica é o ponto, e faz
parte da área cifrada).

Nenhum outro NPC medido no corpus (CHARS.md completo, `docs/narrative/characters/`) combina, ao
mesmo tempo, acesso físico a registros antigos do distrito e a voz de quem guarda segredo por ofício
sem nunca mentir sobre o que sabe. Por isso a escolha recai sobre ele, e não sobre um NPC novo.

### A fala (linhas ilustrativas; prosa final cabe a `narrative-writer`, L-76 global)

**Ao dar a missão:**

> JOAQUIM: "Você quer achar isso, tá bem. Não vou te dar o caminho de bandeja, mas vou te dar um
> nome: procura o Pauli. Não me pergunta quem é, nem onde, porque eu não sei nada disso. Só sei que
> achei o nome riscado num diagrama velho, de um jeito que não parecia rabisco de brincadeira. Quem
> chegou até ele antes de mim jurou que valeu o trabalho, e nem esse eu sei dizer onde foi parar.
> Acha o Pauli, e o resto se resolve sozinho."

**Se o jogador voltar e perguntar de novo, sem nunca entregar a resposta:**

> JOAQUIM: "Já te disse o que sei: Pauli. Não vou repetir de outro jeito só porque você quer mais
> fácil. E se você achar antes de mim, me faz um favor: não me conta como foi. Eu não perguntei."

A fala nunca afirma que Pauli é gente, e nunca mente: Joaquim genuinamente não sabe quem ou o que é
Pauli, só repassa um nome que encontrou. Isso cumpre a exigência do líder (§1): o jogador imagina uma
pessoa, sem que o jogo tenha dito uma mentira em nenhum momento.

### Gatilho de disponibilidade (FECHADO, decisão do líder 12/09/2026)

A fala fica disponível assim que a Zona do Silêncio dá acesso ao NPC, sem nenhum amarre a um ponto do
arco principal. Não há Knowledge, item ou progresso narrativo adicional para destravar: no instante em
que o jogador pode conversar com Joaquim, a missão já existe e pode ser aceita.

## 3. A trava: a piada nunca se explica dentro do jogo

Nenhum personagem, em tempo algum, diz:

- que Pauli é ou não uma pessoa;
- qualquer termo de física, biografia real ou explicação da referência;
- onde a porta fica, antes de o jogador topar com ela por conta própria.

O jogo nunca zomba do jogador por ter procurado um NPC chamado Pauli, e a porta é achável por
exploração normal, por quem nunca ouviu a fala de Joaquim e nunca soube de nenhuma referência: quem
topa com ela sem contexto entra do mesmo jeito e recebe a mesma recompensa. Quem conhece a
referência real ganha só a piada, nunca um atalho, item extra ou vantagem mecânica que quem não
conhece não tenha.

**Frase-guarda:** qualquer redação futura que explique a referência, confirme ou negue Pauli como
pessoa, ou dê vantagem mecânica a quem entende a piada, contradiz esta decisão.

## 4. O que ainda fica para produção

- Posição exata da porta dentro do grafo interno do local onde ela fica (fica para produção, junto
  do layout fino da área, `mundo-topologia.md` §10 item 1).
- Efeito numérico exato da carta que a porta guarda: `economy-designer`.
- Redação final de qualquer linha além das ilustrativas acima: `narrative-writer`, L-76 global.

Local, recompensa e gatilho (as três frentes que o líder decidiu em 12/09/2026) estão fechados na
área cifrada e na seção "Gatilho de disponibilidade" acima.

## 5. Busca de gêmeo (L-17)

Varredura contra o corpus público, refeita nesta rodada (12/09/2026):

- **"Pauli" (qualquer grafia, case-insensitive):** achado só neste documento e em
  `easter-eggs-educacionais.md` (a cena EE-25, que cita o NPC e a fala, sem local nem número).
- **"137" como número isolado (`\b137\b`):** 0 ocorrências no lado público deste easter egg;
  ocorrências alheias em outros arquivos do corpus (linhas de arquivo citadas por número, código de
  saída de processo) não têm relação com esta piada.
- **"constante de estrutura fina" / "efeito Pauli" (case-insensitive):** 0 ocorrências no lado
  público.

**Total: nenhuma ocorrência pública liga "Pauli" a "137" fora deste ponteiro para a área cifrada.**
Nenhum NPC, lugar ou item chamado Pauli existe no corpus.
