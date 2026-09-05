# Dungeon final da Selve Profunda: Kola-SG3-12262, o Espelho de Obsidiana

> **Status:** CANÔNICO (com lacunas nomeadas, ver secão 6). Promove a captura do líder de
> 29/08/2026 de matéria-prima cifrada para registro de design público. **Registro de design
> (estrutura, beats, gates, estados), não prosa final** — redação em prosa cabe ao
> `narrative-writer`, seguindo o mesmo protocolo já usado em
> `docs/design/roster-analogos/21-helion-tusk.md`.
>
> **Documento partido em 05/09/2026 (ordem do líder via `AskUserQuestion`, executado pelo
> `narrative-designer`): esta metade é PÚBLICA — estrutura, cadeia de missão até a party entrar na
> sala do espelho (beats 1-8), lore de setup e handoff. O nome da Era Lendária é público desde
> hoje (decisão do líder); o que ficou cifrado é a revelação central (o que aconteceu no fim dela,
> o sistema de datação, e o que o espelho é) e o confronto final (beats 9-12), movidos para
> `docs/_secret/dungeons/kola-sg3-12262-desfecho.md`, área cifrada, por revelarem o desfecho da
> dungeon a quem ainda não jogou. Este documento aponta pra lá pelo caminho, sem citar conteúdo.**
>
> **Fonte:** `docs/_secret/dungeons/CAPTURA-BRUTA-kola-espelho-obsidiana.md` (texto do líder,
> verbatim, 29/08/2026, e as rodadas de decisão subsequentes na mesma data) e
> `docs/_secret/dungeons/conferencia-canon-espelho-obsidiana.md` (apuração de canon, mesma data).
> Esses dois arquivos são área cifrada (`docs/_secret/`, L-25) e continuam sendo o registro
> processual bruto; este documento é a canonização pública derivada deles, no mesmo papel que
> `docs/design/roster-analogos/21-helion-tusk.md` ocupa em relação ao brainstorm do capstone do
> Tusk. Pesquisa factual de apoio, também cifrada por estar no mesmo diretório:
> `docs/_secret/dungeons/pesquisa-espelho-obsidiana.md` (John Dee, o espelho histórico, RFID) e
> `docs/_secret/dungeons/pesquisa-profundidade-temperatura.md` (Kola real, gradiente geotérmico,
> limite humano).
>
> **Cross-refs:** `docs/design/mundo-topologia.md` (orçamento de 13 dungeons),
> `docs/design/roster-analogos/15-john-dee.md` (a carta Espelho Negro, já pública),
> `docs/design/mecanicas/cartas/dee.md` (implementação da carta), `docs/design/technomagik.md`
> (metafísica de 3 camadas, alvo da propagação do `D30`), `docs/design/mecanicas/save-por-local.md`
> (mecânica PEM/Gaiola de Faraday já canônica), `docs/design/roster-analogos/03-tesla.md` (a
> válvula de Tesla de 6 alças, ecoada aqui), `docs/design/roster-analogos/21-helion-tusk.md`
> (Helion Tusk, figura compartilhada), `docs/narrative/characters/brunus-vetorial.md` (Brunus
> Vetorial, restrições de uso), `PLACES.md` (Subsolo do Edifício Vance, Biblioteca Cintilante, já
> atualizados em 29-30/08/2026), `CHARS.md`, `docs/narrative/deep/settings/08-selve-profunda.md`
> (ver secão 7, sequência de clímax resolvida em 05/09/2026), `docs/_secret/dungeons/kola-sg3-12262-desfecho.md`
> (metade cifrada deste mesmo documento).

---

## 0. O que este documento cobre, e o que não cobre

Este é o documento **hub** da dungeon final. Ele fixa a identidade dela, o orçamento, a cadeia de
missão de ponta a ponta e o conjunto de decisões de lore que a sustentam. Ele **não** duplica o
que outros quatro itens da tabela de pendências já possuem como escopo próprio (L-33, L-34: cada
um é seu próprio átomo) — apontar, não copiar:

| O que fica de fora daqui | Onde vive |
|---|---|
| Especificação técnica do traje térmico com tubos de oxigênio | `D27`? não — **`D25`** (item da tabela) |
| Especificação da dungeon de 8 níveis sob o subsolo do Edifício Vance | `D27` |
| Especificação do artefato-espelho (pareamento por RF, ganho de 30%, camada de tradução) | `D28` |
| Propagação das consequências ao `CHARS.md`, `sinopse.md`, `docs/design/technomagik.md` e
> `docs/narrative/arco-principal.md` | `D30` |
| Entrada da Era Lendária e do sistema de dois calendários em `docs/narrative/timeline.md` | `D22` |
| Expansão mínima do Sylvarin e o texto da inscrição da porta | `D23` (depende de `D22`) |
| Dicionário de Sylvarin em PDF | `D24` (depende de `D23`) |

Este documento referencia o conteúdo de cada um desses seis itens no ponto da cadeia de missão em
que ele entra, mas o texto final de cada peça é responsabilidade do item correspondente.

---

## 1. Identidade e orçamento

**A Kola-SG3-12262 É a única dungeon orçada para a Selve Profunda.** `docs/design/mundo-topologia.md:58`
já reservava, na tabela de distribuição de dungeons (13 no total, padrão numérico velado), **exatamente
uma** dungeon para a Selve Profunda, rotulada "a dungeon final". Esta decisão (líder, 29/08/2026,
registrada em `docs/_secret/dungeons/CAPTURA-BRUTA-kola-espelho-obsidiana.md`, secão "Decisões do
líder, 29/08/2026, após a conferência de canon", item 1) preenche esse slot com a Kola-SG3-12262: não
se cria dungeon nova, não se altera o total de 13, e a cerca do `mundo-topologia.md` fica intacta.

O conceito já registrado em `docs/design/mundo-topologia.md:95` para essa dungeon única ("mista
(clímax): culminante longa; gimmick = tudo junto (puzzle + labirinto + batalha)... rumo ao confronto
final. O ápice") descreve exatamente a estrutura da Kola-SG3-12262 (secão 3 abaixo): dois retornos à
caverna, dois puzzles de porta/fechadura, combate opcional no retorno, e desembocadura direta no
confronto final. **Seguimento sugerido, não executado aqui:** o `mundo-topologia.md:95` continua
descrevendo o conceito de forma genérica, sem nomear a Kola-SG3-12262; nomeá-la ali e apontar para
este documento é edição pontual de outro arquivo, fora do escopo desta fatia (`D21`).

**O que este orçamento NÃO cobre — sinalizado, não resolvido aqui:** a dungeon de 8 níveis sob o
subsolo do Edifício Vance (`D27`) é uma estrutura **separada**, geograficamente no Núcleo
Metropolitano, área para a qual `docs/design/mundo-topologia.md:47` orça **zero** dungeons. A
conferência de canon (`docs/_secret/dungeons/conferencia-canon-espelho-obsidiana.md`, secão 6)
identificou que essa segunda estrutura fica **fora do orçamento fechado de 13** sem que o líder
tenha decidido se ela conta contra o total, se é uma categoria à parte, ou se o
`mundo-topologia.md` precisa de uma revisão numérica. **Correção de fato (05/09/2026): a "Área
faraday especial" NÃO é exemplo de categoria à parte.** Ela está dentro do total de 13, é uma das
áreas orçadas na própria tabela (`docs/design/mundo-topologia.md:17,26,59-60`). O único precedente
real de algo fora do cômputo das 13 é o repositório leve do mini-mapa que a Montadora Confluência
ganha (`docs/design/mundo-topologia.md:57`), explicitamente não-dungeon. **Isto nunca foi resolvido
nas rodadas de decisão seguintes** (verificado: nenhuma das secões 10 a 17 da captura-bruta toca o
assunto). Fica como lacuna nomeada (secão 6.1).

---

## 2. Cadeia causal fechada: por que ali, por que ninguém sabia

Antes da cadeia de missão, três causalidades já fechadas pelo líder sustentam a plausibilidade da
dungeon e evitam "por que ninguém achou isso antes":

- **A geografia extrema é real, não inventada para o jogo.** O poço real de Kola (SG-3) parou em
  12.262 m porque a rocha, àquela profundidade e temperatura (~180 °C), deixa de se comportar como
  sólido e passa a fluir como plástico, fechando qualquer perfuração assim que ela para
  (`docs/_secret/dungeons/pesquisa-profundidade-temperatura.md`, secão 1). O fundo jogável da
  Kola-SG3-12262 fica **logo acima** dessa zona de rocha pastosa, que fecha sozinha atrás da party:
  isso é o motivo diegético do limite de profundidade, não uma parede arbitrária (líder, verbatim,
  item 12 da captura-bruta: "eles chegam pouco antes da rocha pastosa que fecha sozinha").
- **A impossibilidade física de descer a pé até ali (calor de ~180 °C, ar irrespirável) é
  resolvida por um item de hardware, não por magia.** Uma roupa térmica com tubos de oxigênio,
  encomendada a Helion Tusk e Brunus Vetorial (especificação completa em `D25`), resolve as duas
  impossibilidades térmica/respiratória; a terceira (túnel estável) é resolvida pela geografia do
  parágrafo anterior. Nenhuma das três impossibilidades é contornada por decreto de lore.
- **O disco de ouro (a chave da porta) chegou sob o prédio do Gus por acidente
  histórico, não por destino.** A sociedade secreta que preservou o disco perdeu o próprio registro
  de onde ele estava guardado; o Edifício Vance foi erguido sobre a entrada esquecida porque o
  terreno parecia vazio; os operários da construção deixaram um alçapão e não voltaram, porque não
  foram pagos para isso (líder, verbatim, item 14 da captura-bruta). A casa do Gus é consequência do
  esquecimento em três camadas (entrada aparentemente desabada, memória apagada pelo tempo, alçapão
  coberto por objetos), não uma coincidência narrativa. Localização já canonizada: **Subsolo do
  Edifício Vance**, sob o carpete, `PLACES.md:84`.

---

## 3. Cadeia de missão (beat sheet estrutural)

A party já possui a carta **Espelho Negro** antes de qualquer beat abaixo começar: ela vem da missão
de descoberta de John Dee (`docs/design/roster-analogos/15-john-dee.md:47-51`, já pública, mecânica
já implementada em `docs/design/mecanicas/cartas/dee.md`), sem que a party saiba, naquele momento, que
a carta tem qualquer relação com esta dungeon. O espelho é a **origem histórica** da carta, não o
lugar onde ela se obtém (líder, decisão 5, captura-bruta): isto desfaz qualquer aparência de laço
causal circular entre carta e espelho.

1. **Primeira descida (a pé), até a rocha pastosa.** A party desce a Kola-SG3-12262 e resolve, na
   camada mais funda de todas, o **puzzle de achar e desligar o dispositivo EMP/PEM mais forte do
   jogo** (aplicação em escala máxima da mecânica já canônica de PEM-por-dungeon,
   `docs/design/mecanicas/save-por-local.md:36-42`, item-chave "Emissor do Tesla" e carta passiva
   "Gaiola de Faraday" já existentes; nenhuma mecânica nova aqui, só a instância mais forte dela).
   Chegando perto do fundo, a temperatura torna a descida impossível sem equipamento; a party volta
   à superfície de mãos vazias quanto ao fundo, mas já tendo desligado o PEM mais forte do jogo.
2. **Pedido do traje.** A party pede a Helion Tusk e a Brunus Vetorial, em conjunto, um item capaz de
   suportar a descida. Nenhum dos dois consegue sozinho ("isso é muito complexo, não consigo fazer
   sozinho"); Gus conclui que os dois juntos formam a peça. Especificação completa da cena, do item e
   da divisão de trabalho entre o alquimista/químico (Brunus) e o tecnicista (Tusk): `D25`.
   ⚠️ **Ponta nomeada, não resolvida aqui:** o corpus não registra que Brunus e Helion Tusk já se
   conheçam ou já tenham colaborado (`docs/_secret/dungeons/conferencia-canon-espelho-obsidiana.md`,
   secão 5, "Brunus e Helion já se conhecem?"); esta cena é a primeira vez que os dois trabalham
   juntos, e isso precisa ser escrito como início de relação, não como continuação de uma já
   estabelecida.
3. **Segunda descida, equipada.** Com o traje térmico (`D25`), a party volta ao fundo da
   Kola-SG3-12262. Os inimigos já enfrentados podem reaparecer, mas **o combate deixa de ser
   obrigatório** neste retorno (líder, verbatim, item da captura-bruta). A party encontra uma porta
   com fechadura em forma de disco, do raio de um disco de vinil (LP), com uma inscrição em Sylvarin.
4. **Tradução da inscrição (bloqueada até `D23`/`D24` existirem).** A inscrição, quando puder ser
   lida, diz que a chave da porta foi enviada ao espaço numa data do calendário antigo e caiu de
   volta décadas depois, contendo resumos da humanidade da época. O **conteúdo semântico resumido
   aqui** basta para desenhar a estrutura do puzzle; as datas exatas e o destino do envio, que o
   texto em Sylvarin precisa nomear, estão fixados em área cifrada
   (`docs/_secret/dungeons/kola-sg3-12262-desfecho.md` secão 2), por revelarem a mesma descoberta
   central do desfecho da dungeon. O **texto em Sylvarin** propriamente dito é trabalho separado
   (`D23`, que depende de `D22` fixar o deslocamento entre calendários — também cifrado, mesma
   seção). Ver lacuna nomeada na secão 6 sobre o que isso exige de quem executar `D23`/`D24`.
5. **Interlúdio: a segunda dungeon.** A party precisa sair de novo e buscar outra dungeon,
   inteiramente distinta, escondida sob o **Subsolo do Edifício Vance** (`PLACES.md:84`,
   `CHARS.md` §4), atrás de um alçapão sob o carpete que o próprio Gus nunca notara ao usar o
   espaço todos os dias. Especificação completa dos 8 níveis: `D27`. No fim dela, a party encontra o
   **disco de ouro**.
6. **Passagem obrigatória pela Biblioteca Cintilante.** Antes de voltar à Kola-SG3-12262, a party leva
   o disco de ouro à Biblioteca Cintilante (ambiguidade resolvida pelo líder: é o disco de **ouro**,
   não o de obsidiana, que vai à biblioteca — captura-bruta, item 11). A biblioteca já está
   canonizada como reaberta e em uso pela população antes desta missão (`PLACES.md:127`); este passo
   não reabre nada, só usa um lugar já vivo.
7. **Terceira descida: a porta se abre.** De volta à Kola-SG3-12262, a party usa o disco de ouro para
   destrancar a porta em forma de LP e entra na sala do espelho.
8. **A sala do Espelho de Obsidiana.** O espelho (22 × 18,4 cm, item 13 da captura-bruta) brilha
   quando a party se aproxima. Gus usa um aplicativo de escuta de frequências no computador de braço
   e deduz que carta e espelho se "percebem" por indução/RF, e que a carta precisa estar com
   bateria carregada para o espelho reagir; a pista visual é a face idêntica entre a arte da carta e
   o espelho físico. Base técnica de plausibilidade (RFID passivo por indução, alcance limitado a
   poucos centímetros/um metro, ver `docs/_secret/dungeons/pesquisa-espelho-obsidiana.md` secão 5):
   o desenho apoiado em indução de curto alcance é fisicamente consistente; alcance maior exigiria
   backscatter UHF ou bateria própria no espelho, o que a cena não descreve precisar. Especificação
   mecânica completa do artefato (o ganho de 30%, a camada de tradução/kernel, o pareamento):
   `D28`.
9-12. **O desfecho da dungeon (revelação central e confronto final) é área cifrada.** Continuação
   direta do beat 8, incluindo o que o espelho revela sobre a natureza do mundo, o efeito de equipar
   o espelho e o confronto que fecha a cadeia: `docs/_secret/dungeons/kola-sg3-12262-desfecho.md`
   secão 1. Não reproduzido aqui por revelar o desfecho a quem ainda não jogou (critério do líder,
   05/09/2026).

---

## 4. O calendário da Era Lendária, e o Espelho como kernel: área cifrada

O nome da Era Lendária é público (seção 0 e handoff apontam para ela por nome). O que fica cifrado
é o conteúdo desta seção e da seção 5: a notação do calendário antigo, o deslocamento entre as duas
contagens, o ano em que a era termina, e a explicação de o que o espelho é e faz. Esse conteúdo foi
movido em 05/09/2026 para
`docs/_secret/dungeons/kola-sg3-12262-desfecho.md` secões 2 e 3, por revelar a mesma descoberta
central que os beats 9-12 (secão 3 acima). Não reproduzido aqui pelo mesmo critério: quem ainda não
jogou não pode saber o que se descobre no fim. `D22` (propagação a `docs/narrative/timeline.md`) e
`D30` (propagação a `docs/design/technomagik.md`) continuam apontando pra lá, não pra este
documento.

---

## 6. Lacunas nomeadas (não decidido, não preenchido por invenção deste documento)

Estas sete pontas seguem em aberto depois de toda a apuração e todas as rodadas de decisão do
líder já registradas; nenhuma foi resolvida por este documento, por dever da L-13/L-14:

1. **Orçamento da segunda dungeon (8 níveis, `D27`).** Se ela conta contra o total fechado de 13
   dungeons, se é categoria à parte (o precedente real de algo fora do cômputo é o repositório
   leve da Montadora Confluência, `docs/design/mundo-topologia.md:57`, explicitamente não-dungeon;
   a Área faraday especial NÃO é exemplo disso, ela está dentro do total de 13), ou se o
   `mundo-topologia.md` precisa de revisão numérica. Ver secão 1.
2. **Relação entre esta missão e a missão-capstone de Helion Tusk.** O corpus não diz se a missão
   do espelho acontece antes, depois ou em paralelo à missão-capstone dele
   (`docs/design/roster-analogos/21-helion-tusk.md`), que já tem a própria cadeia de acesso (as 20
   cartas) e o próprio esconderijo deliberadamente em aberto. As duas missões competem pela mesma
   figura rara e pela mesma região geográfica ampla (Selve Profunda / Selve Sombria).
3. **Relação prévia entre Brunus Vetorial e Helion Tusk.** Não há registro de que já se conheçam;
   o corpus só os liga por arquétipo temático (herói de inteligência que abre mão de crédito por
   ética), nunca por biografia ou cronologia compartilhada. Ver secão 3, beat 2.
4. **Nome in-world em Sylvarin do conceito TechnoMagik.** Segue "A DEFINIR"
   (`docs/design/technomagik.md:18,73`); só relevante se a inscrição da porta (`D23`) precisar do
   termo em Sylvarin, não em português/meta.
5. **Calibração/simulação da redução de dificuldade do confronto final.** Detalhe completo em área
   cifrada (`docs/_secret/dungeons/kola-sg3-12262-desfecho.md` secão 4), por nomear o alvo do
   confronto final e o mecanismo exato da revelação. Resumo sem spoiler: nenhum número de
   calibração existe ainda; a disciplina de simulação já praticada no projeto (critério
   pré-registrado antes do dado existir) se aplica quando esse trabalho for aberto.
6. **Motivo de Helion Tusk esconder a própria carta-capstone**, distinto do motivo (já fechado) de
   ele ter apagado o compilador. Ponta própria de `21-helion-tusk.md`, citada aqui só porque a
   secão 3, beat 2, deste documento aproxima os dois personagens pela primeira vez.
7. **Execução de `D23` (inscrição em Sylvarin) e `D24` (dicionário) precisa de acesso à área
   cifrada.** Desde 05/09/2026, as datas exatas e o destino do envio que o texto em Sylvarin
   precisa nomear (beat 4, secão 3) vivem só em `docs/_secret/dungeons/kola-sg3-12262-desfecho.md`
   secão 2, por recuo de revelação decidido pelo líder. O conteúdo semântico resumido no beat 4
   basta para desenhar a estrutura do puzzle, mas não basta para escrever a inscrição em si: quem
   executar `D23`/`D24` sem acesso à cifra não consegue fechar essa peça.

---

## 7. Sequência com o clímax do Núcleo Mandelbrot (resolvido pelo líder, 05/09/2026)

**Achado desta fatia (29/08/2026):** `docs/narrative/deep/settings/08-selve-profunda.md`, revisado
em 19/05/2026 e marcado "não modificar sem aprovação do criador supremo", já descreve o clímax do
Ato 3 da Selve Profunda como a penetração ritual do **Núcleo Mandelbrot** por Mariana, Jaci e Gus,
com a decisão de ending tomada em uma de três rotas (Bronze/Prata/Ouro; ver também `PLACES.md:96`,
"Núcleo Mandelbrot Interno"). Essa cena não menciona a Kola-SG3-12262, o espelho de obsidiana, nem
qualquer elemento desta dungeon. Nenhum dos dois documentos de apuração de 29/08/2026
(captura-bruta ou conferência de canon) cita `08-selve-profunda.md` ou o Núcleo Mandelbrot em
nenhum momento — a conferência de canon checou orçamento de dungeons, geografia, personagens e
cortes, mas não cruzou com o clímax já canonizado desta mesma área. `mundo-topologia.md:58` orça
exatamente **uma** dungeon para a Selve Profunda, e este documento a preenche com a Kola-SG3-12262;
`08-selve-profunda.md` registra, como canon fechado desde maio, um clímax diferente, na mesma área,
com outro elenco e outro mecanismo.

**Decisão do líder, 05/09/2026, por `AskUserQuestion`: os dois climaxes são sequência, e a
Kola-SG3-12262 vem ANTES do ritual do Núcleo Mandelbrot.** Nenhum dos dois substitui o outro; são
dois eventos jogáveis distintos, na mesma área, em ordem fixa. Esta decisão está registrada como
canon nos dois documentos: aqui, e em `docs/narrative/deep/settings/08-selve-profunda.md` (nota de
sequência ao final daquele documento, que não altera mais nada do texto já aprovado em 19/05/2026).

**Consequência resolvida pelo líder em 05/09/2026, por `AskUserQuestion`:** o ritual do Núcleo
Mandelbrot **não conta contra o total de 13 dungeons**. Ele é cena de clímax de área, categoria que
`docs/design/mundo-topologia.md` já reservava para os interiores de missão (secão 6 daquele
documento) e que agora tem critério explícito e um segundo membro (secão 4.2 daquele documento): o
jogador vive um evento central (a escolha de ending entre três rotas), não navega uma sequência de
salas com desafios em série, como uma dungeon exige. O orçamento de 1 dungeon para a Selve Profunda
(a Kola-SG3-12262) fica intacto, e o total de 13 não muda.

**Isto é distinto e não resolve a mesma pergunta ainda em aberto para a dungeon de 8 níveis do
Vance (`D27`, secão 1/6.1 acima):** aquela é uma dungeon de exploração de verdade (level design
sala-a-sala), não uma cena de clímax, e por isso não cabe na mesma saída; segue como lacuna
nomeada, decisão do líder pendente.

---

## 8. Cortes da L-29 atravessados

Nenhum corte é contradito por esta dungeon; um tensiona de leve, sem violar:

- **C-02 (sem mundo aberto, sem mundo persistente):** a cadeia de idas e vindas (caverna → traje →
  caverna → subsolo do Vance → biblioteca → caverna) usa áreas já existentes e alcançáveis desde o
  início, consistente com `mundo-topologia.md:13`. **Cabe.**
- **C-15 (escopo fechado, sem duração fixa em horas):** a cadeia é grande (duas dungeons, uma
  missão de fabricação de item, uma parada obrigatória de biblioteca), mas não é vedada pelo corte
  reescrito em 28/08/2026; o princípio que sobrevive nele (mecânica nova não entra só por ser boa,
  precisa caber num jogo com fim) merece atenção quando o escopo total desta cadeia for medido
  contra o resto do Ato 3. **Tensiona, não contradiz.**
- Nenhum outro corte é tocado.

---

## 9. Handoff

Este documento fecha o `D21`. Trabalho seguinte, cada um seu próprio átomo:

- `D22` (calendário/Era Lendária em `timeline.md`) → `D23` (inscrição Sylvarin) → `D24` (dicionário
  PDF), nesta ordem de dependência.
- `D25` (traje térmico), `D27` (dungeon de 8 níveis), `D28` (artefato-espelho): três especificações
  técnicas paralelas, sem dependência entre si, todas com `D21` como pré-requisito.
- `D30` (propagação a `CHARS.md`, `sinopse.md`, `docs/design/technomagik.md`,
  `docs/narrative/arco-principal.md`).
- Redação em prosa final de qualquer beat desta cadeia: `narrative-writer`, só depois de as
  especificações técnicas acima existirem; a sequência com o clímax do Núcleo Mandelbrot já está
  resolvida (secão 7), mas o veredito sobre o orçamento de dungeons da Selve Profunda (secão 7,
  consequência sinalizada) segue pendente de decisão do líder e bloqueia qualquer redação que
  dependa dele.
- Redação da metade cifrada (revelação central, confronto final): mesma regra, além de exigir
  acesso à área cifrada `docs/_secret/dungeons/kola-sg3-12262-desfecho.md`.
