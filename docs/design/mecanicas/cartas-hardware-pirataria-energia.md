# Cartas: hardware, energia e pirataria

> **Atualização 2026-08-25 (líder, revogação de C-03/C-13 na L-29 de `GODS_LAWS.md`):** o craft de cópia pirata deixa de ser exclusivo da COMUM e passa a alcançar **ESPECIAL e SUPER**, sempre em qualidade inferior ao original — ver §15, seção nova. A SUPER também deixa de ser descrita como "forjada" em `cartas-technomagik.md` (ela é carta original; reflexo já aplicado lá, §2.4).
>
> **Atualização 2026-08-25 (líder, canon pessoal do Gus Dragon, L-16 de `GODS_LAWS.md`):** o líder dita, no mesmo dia, a expansão de quatro para **doze comidas** favoritas reais dele como itens de cura do jogo, mais **oito ingredientes de craft** (catálogo completo em `comidas-ingredientes-craft.md` §5.5 (catálogo movido de `economia.md` em 25/08/2026, L-33)). Duas das doze, suco de limão e água com gás, continuam alimentando craft de **bateria de baixa qualidade**, via refino em suco puro de limão / água destilada: ver §5, subseção "Bateria de baixa qualidade (craftada)", que nesta rodada também fecha se a bateria craftada herda os riscos da pirata comprada (herda, sim).
>
> **Status:** ✅ **CANON FECHADO (líder, 2026-07-28).** Os quatro pontos que este cabeçalho listava como abertos foram entregues: os **números** viraram `cartas-numeros-proposta.md` (com as últimas duas ambiguidades fechadas em 2026-07-28), a **spec de implementação** virou `cartas-spec-dados.md` (aprovada em 2026-07-28) e `cartas-spec-logica.md`, e os **efeitos do vírus** e o **mercado negro** estão fixados aqui e nos docs de números. Reflexo pendente em `cartas-technomagik.md` e na terminologia, que é passo de escrita, não decisão. _(Status original, por registro: BRAINSTORM EM ANDAMENTO, líder, 2026-07-18.)_ Decisões fechadas via AskUserQuestion capturadas fielmente. Ainda ABERTO: efeitos exatos do vírus, o mercado negro (lugar/vendedor), números (delegar ao `economy-designer`), spec de implementação (delegar a `gameplay_engineer`/`backend-engineer`). NÃO é canon fechado até o líder revisar o doc consolidado.
>
> **Origem:** derivou do "vírus do Dante" (guarda-chuva Faraday, ver `docs/design/mundo-topologia.md` / brainstorm-backlog). O líder escalou o evento pontual (Dante injeta vírus na carta Faraday à noite) para um SISTEMA geral de cartas modificadas / piratas / infectadas / com bateria.
>
> **Cross-ref:** Pillar 1 (magia = software, "scripts rúnicos compilados"), Pillar 3 (triângulo hardware / Tavus-Drive), memórias `project_gus_eixo_compilado_interpretado` (compilado=rápido=BOM / interpretado=lento=RUIM), `reference_deck_mao_sistema` (especiais únicas × comuns bounded), `project_axiologia_canonica` (austríaco, anti-compadrio), `project_terminologia`, roster de análogos (Volta, Bastiat) em `docs/design/roster-analogos/`.

---

## 1. Enquadramento moral (one-way-door, decisão do líder)

**Mercado livre legítimo × a FRAUDE é que é o mal.** Comprar/modificar carta no mercado cinza NÃO é imoral — pode até ser resistência ao monopólio-compadrio do Sterling/DRE. O mal é:

- a **fraude** (vírus escondido, como o Dante; vender pirata como original);
- o **risco de fonte podre não avaliado** — o custo que **não se vê** (o **Bastiat** do roster é o professor diegético disso).

Lição diegética: **caveat emptor / avalie a fonte**, não "pirataria = pecado". Alinha com o austríaco anti-compadrio e com "faz-se dinheiro com trabalho".

## 2. Três tipos de carta fora do canal oficial

1. **Modificada (mod / homebrew):** carta base tunada (buff, efeito extra, custo menor). Legítima em si (você mexe no que é seu), mas **instável se mal "compilada"** — o mod porco roda mal e drena bateria mais rápido (custo do eixo compilado/interpretado).
2. **Clone pirata:** cópia não-autorizada de uma carta; degradada + o vetor mais comum de vírus.
3. **Infectada (vírus):** carrega payload oculto malicioso. É a **fraude** — o mal do enquadramento.

O **mercado negro** é o *lugar* onde os três circulam, misturados com usados legítimos honestos.

### Clone-falso de especial
As cartas especiais são **únicas** (uma no jogo). Mas existe **imitação pirata** de especial: parece o poder raro, é falsificação degradada/instável/possível-vírus, e **NÃO é a única real** (não conta como a especial). Não quebra a unicidade e ensina "a cópia não é o original". O Sterling pode usar falsos; um vendedor pode te enganar com um "Faraday" pirata.

### Clone-falso de super (decisão do líder, 25/08/2026)
O mesmo raciocínio vale, com um grau a mais de dificuldade, para a **SUPER**: existe uma original no jogo inteiro (`cartas-technomagik.md` §2.4), e ela **nunca é forjada nem craftada por ninguém** — é a peça original de Tusk, escrita em asmódico puro, num compilador que se perdeu (§15.2 abaixo). Qualquer coisa que saia de uma bancada tentando imitá-la é, por definição, uma cópia pirata, nunca a original. Tratamento completo em §15.

## 3. Física da carta (o que prova original × pirata × homebrew)

> **PRINCÍPIO CANÔNICO (líder, 2026-08-08, AMPLIADO no mesmo dia): TODAS as cartas
> devem passar pelo mecanismo de originalidade da carta (§3, esta seção) E degradação
> da bateria (§5, "Energia: baterias CR2032") — as duas variáveis JUNTAS, não uma ou
> outra. Isso é o DEFAULT; exceções existem, mas cada uma se discute e se aprova caso
> a caso com o líder, nunca por omissão.** Redação original (mesmo dia, mais estreita):
> "qualquer efeito que CONSOME energia" — o líder ampliou para "todas as cartas" horas
> depois, ao ver a implementação da curandeira fechar. Ao planejar uma carta nova,
> o ponto de partida é "ela passa pelo mecanismo"; só vira exceção se alguém levantar
> o caso e o líder concordar (a única candidata cogitada até aqui, a passiva mana-0,
> foi fechada em sentido contrário em 02/09/2026: carta passiva nunca tem custo zero,
> tem dois gastos, ver §5, "Passiva tem dois gastos: standby e disparo"). Nasceu da
> carta de cura da curandeira (`CURANDEIRA-LIMITE-USOS`), primeiro caso implementado:
> ao planejar CADA carta nova, responder às duas perguntas antes de existir — "o que
> muda se a carta for original × pirata × homebrew?" e "o que muda se a bateria
> estiver nova × degradada?". É o mesmo espírito da lei do átomo (ADR-019/020)
> aplicado à energia da carta: a variação vem de DADO cruzando as duas dimensões,
> nunca de comportamento hardcoded para um efeito só. Exemplo já especificado (cura):
> original cura magnitude fixa e drena fração fixa da carga; pirata cura em faixa RNG
> e drena fração RNG maior; bateria degradada (SoH baixo) ganha chance de falha total
> no uso, proporcional à degradação — ver `EnemyDifficultyConstants`-style tabela em
> código, não `if` espalhado.

| | **Original (oficial)** | **Vazia (homebrew)** | **Pirata (clone)** |
|---|---|---|---|
| Memória | ROM pura | **EPROM** de baixa qualidade | ROM |
| Chip | excelente | baixíssima qualidade | — |
| Capacidade | vários **MR** | poucos **kR** (só conjuro pequeno/comum) | — |
| Upload | de fábrica | **muito lento** | 1x definitivo |
| Conector | **nenhum** | **RSB** externo, visível | **RSB interno escondido = prova cabal de pirataria** |
| Mexer no software | **queima na hora** (anti-tamper físico) | regravável, mas **desgasta até queimar** | travado após gravar |
| Sinal externo | — | — | diferenças visuais sutis (poucos notam) |

**Amarras:** ROM de fábrica = compilado/rápido = BOM; EPROM lenta = lado interpretado/lento. O "queima se mexer" é anti-tamper físico, irmão do save-crypto. O conector interno é a evidência forense da fraude (a pirataria é cinza; a *ocultação* denuncia).

**Esta física é a mesma para pirata de COMUM, de ESPECIAL e de SUPER (decisão do líder, 25/08/2026, §15).** O que muda entre os tiers não é o hardware — é o **teto de fidelidade** alcançável, porque só a COMUM tem um original em circulação disponível pra copiar de perto; ESPECIAL e SUPER nunca circularam, e é isso que barra a engenharia reversa (§15.2).

**Homebrew COPIA, não cria (líder, 2026-07-18 — preserva o anti-pillar):** gravar uma carta vazia via RSB é **duplicar** um conjuro que já existe (pirataria = cópia ilegal), **não** inventar um efeito novo numa bancada. Isso mantém intacto o anti-pillar do `gdd.md`/`cartas-technomagik.md` ("cartas são obtidas, não craftadas; o jogador nunca monta carta nova numa bancada") e casa com **von Neumann = replicação**. O jogador nunca cria conjuro inédito — ele copia um existente para um meio pirata barato.

**Isto vale também para ESPECIAL e SUPER (decisão do líder, 25/08/2026, §15):** copiar não é criar, mesmo quando o alvo copiado é único no jogo inteiro. O mecanismo é sempre o mesmo — gravar cópia numa carta vazia via RSB, na bancada. O que muda por tier é se existe **conhecimento disponível** pra copiar bem: o efeito de uma COMUM roda num sistema aberto e conhecido (a mesma compilação-no-cast que o jogo inteiro usa); o efeito de uma ESPECIAL ou da SUPER nunca foi exposto a ninguém pra examinar, e é essa falta de exame que trava a cópia no teto de 20% de fidelidade (§15.2).

## 4. Unidade de memória (terminologia canônica)

- **runa (r)** = bit · **Runa (R)** = Byte · **1 R = 8 r** (base 8, igual ao real).
- Prefixos decimais **kR / MR / GR / TR** (×1000) e binários **kiR / MiR / GiR / TiR** (×1024) — o par kB/kiB do mundo real.
- Ancorada no Pillar 1 ("feitiços são scripts **rúnicos** compilados").

**Conector = RSB (Runic Serial Bus):** o análogo diegético do USB (Universal Serial Bus), trocando "Universal" por "Runic". A carta vazia expõe um RSB externo; a pirata esconde um RSB interno (§3).

**Regra de nomenclatura (líder, 2026-07-18):** **termos de TI são universais em inglês** (RSB, ROM, EPROM, bit, Byte, CPU…). O giro diegético entra no CONCEITO/adjetivo quando cabe (Universal→Runic; a unidade própria runa/Runa), nunca em traduzir a sigla pro pt-br.

**Gravador = terminal de bancada FIXO:** o conjuro é escrito/compilado na carta vazia num "computador do jogo" de bancada (interface de **terminal**, casa com o terminal-glitch canon e os logs de combate), conectado via **RSB**. Fixo (não em campo) — combina com o upload lento da EPROM ruim. Ficam **SÓ nas oficinas do mercado negro / ferro-velho** (§14): gravar homebrew é atividade de submundo maker; o Gus **não** tem bancada em casa. Reforça que homebrew é legítimo, mas vive no cinza.

## 5. Energia: baterias CR2032

- A carta tem **X usos** porque tem **bateria**; esgotada, a carta fica inerte até trocar. Diegese para o limite de usos.
- Um **recurso Y** da carta drena mais rápido (o efeito premium consome mais fundo).
- **CR2032** = a moeda de lítio, literalmente a bateria de BIOS/placa-mãe ("o coração que guarda o estado") — casa com carta = hardware.
- **Degradação:** a bateria degrada com uso e ciclos de carga.
- **De fábrica (padrão, a CR2032 acima):** capacidade real igual à capacidade de face, sempre; a leitura de carga que o jogador vê é a verdade (decisão do líder, 03/09/2026, ver "Capacidade de face × capacidade real" mais abaixo neste §5).
- **Especiais:** bateria **selada / de maior capacidade** — mais confiável (condiz com "especiais protegidas"), mas ainda exige cuidado no endgame (amarra com o clímax sem-save/PEM). Capacidade real igual à de face, como a de fábrica; o que a distingue não é desvio de capacidade, é degradar mais devagar (decisão do líder, 03/09/2026, verbatim: *"Real igual à face, mas ela degrada mais devagar"*); a curva de degradação dela é número do `economy-designer`, pendente.

### Mana e bateria são o mesmo recurso (decisão do líder, 31/08/2026)

> **Decisão do líder, por `AskUserQuestion`, verbatim:** *"como magia é tecnologia, então mana e carga
> de bateria se confundem e são a mesma coisa. Pode ser citado mana ou bateria... O jogador no início
> vai se confundir, mas é proposital e depois vai passar a entender quando alguém disser por exemplo
> que a bateria está com pouco mana."*

**Mana e carga de bateria são o MESMO recurso, com dois nomes — não dois custos, não dois medidores,
não duas economias.** A origem não é conveniência de sistema, é consequência direta do **Pillar 2**
(`pillars.md`): *"magia é sistema formal computável"* — se magia é tecnologia, o combustível dela É a
carga elétrica, e "mana" é só o nome que a tradição mágica dá ao que o hardware chama de "carga de
bateria". As duas palavras descrevem a mesma substância, nunca dois recursos separados.

**Os dois termos são intercambiáveis no texto do jogo.** Uma carta pode dizer "mana", um personagem
pode dizer "bateria", e as duas falas descrevem a mesma coisa. Nenhuma delas é a forma "certa" e a
outra a forma "de brincadeira" — são sinônimos canônicos.

**A confusão inicial do jogador é DELIBERADA, não descuido de tutorial.** Ela nasce sem explicação e
se desfaz por **fala de personagem** — o exemplo do próprio líder é alguém dizer *"a bateria está com
pouco mana"* — nunca por tela de tutorial, pop-up ou texto explicativo. **Registrado aqui como
intenção de desenho para que ninguém "conserte" essa ambiguidade depois achando que foi erro:** ela é
o efeito pretendido, ecoando o onboarding orgânico já canônico (Pillar 1, anti-mecânica "tutorial
wall-of-text"; corte `C-16`). Fala que ensina a equivalência ainda não foi escrita — é trabalho de
`ux-writer`/`narrative-writer`, fora do escopo desta fatia; fica registrada aqui como necessidade
pendente, não como texto pronto.

**Relação com o que já está especificado, sem alterar nada disso:** `cartas-spec-logica.md` §3 já liga
os dois numericamente — a decisão de hoje confirma que esse acoplamento não é coincidência de número:
é identidade de substância. **A ambiguidade que esta seção levantava originalmente — se a rampa do
ator em `combat.md` §5 (`manaMax = 2 + turnos`) e a bateria física da carta eram dois medidores
separados ou o mesmo — foi respondida pelo líder no mesmo dia.** A resposta está na seção "Estoque e
vazão", logo abaixo.

### Múltiplas baterias e a barra da tela (acréscimo do líder, 31/08/2026)

> **Verbatim do líder:** *"A barra de mana que mostra na tela abaixo do quadro do personagem é
> representa a carga/mana da bateria mais cheia, se o jogador não tiver selecionado outra."*

Este acréscimo completa a decisão acima, no mesmo dia e sobre o mesmo assunto: não é só uma
equivalência de nomes, é um sistema de recurso.

1. **O jogador carrega VÁRIAS baterias**, não uma — rastreadas individualmente, cada uma com a
   própria carga.
2. **Existe uma bateria ATIVA**, e a barra na tela mostra a carga dela.
3. **O padrão é a bateria mais cheia.** Sem escolha do jogador, o jogo elege automaticamente a de
   maior carga.
4. **O jogador pode selecionar outra bateria**, e a barra passa a mostrar a selecionada — a escolha
   de qual bateria usar é decisão dele, não do sistema.
5. **A barra fica abaixo do quadro do personagem.** Isto é posição de interface, registrada aqui como
   decisão de layout já tomada pelo líder — **não é desenho de interface feito nesta fatia** (L-27
   deste projeto: nenhuma interface se escreve antes de o GlintFx traduzir marcação). Nenhuma
   marcação, folha de estilo ou mockup nasce deste parágrafo; ele só fixa o fato, para quando a
   camada `present/` existir.

**Leitura que isto abre, sem criar regra nova:** a bateria deixa de ser custo passivo e vira recurso
que o jogador **gerencia** — escolher qual bateria gastar é decisão tática. Isso explica, sem nenhuma
regra adicional, por que a carta do Volta pode ser usada de novo tendo outra bateria disponível: ela
consome a bateria ativa por inteiro, e trocar de bateria devolve o recurso (`docs/design/mecanicas/cartas/volta.md`).

**Consequência marcada, NÃO resolvida aqui:** se cada bateria tem carga própria e uma carta pode
consumir a ativa inteira, o custo fixo de "mana 6" que algumas cartas registram (ex.: Volta, ver
`cartas-technomagik.md` linha 70) precisa ser lido como "6 da bateria ativa" — coerente, mas não
escrito em lugar nenhum. A mesma dúvida já está marcada, de forma independente, em
`docs/design/mecanicas/cartas/volta.md` ("Pontas soltas"): não decidida ali, não decidida aqui.

**Duas contradições candidatas achadas na varredura de 31/08/2026, AMBAS respondidas pelo líder no
mesmo dia (ver "Estoque e vazão", logo abaixo):**

1. ✅ **Respondida.** `docs/design/mecanicas/combat.md` §5 especificava um pool de mana do ATOR,
   separado da bateria física da carta. O líder respondeu: não são dois medidores, são duas
   propriedades da MESMA bateria (estoque e vazão) — ver "Estoque e vazão" abaixo, e a correção já
   aplicada em `combat.md` §5 e `cartas-spec-logica.md` §3.1.
2. ✅ **Respondida.** `docs/design/mecanicas/battle-screen.md` linha 50 descreve, dentro do painel
   lateral ("cockpit"), "retrato GRANDE (64px) + nome + barra de HP (com número) + pips de AP (latão)
   e Mana (cyan)". **Correção de fato, para não repetir:** esta seção havia dito que essa spec estava
   "embarcada como constantes em `battle_scene.cpp`" — esse arquivo **não existe**; este repositório
   não tem código nenhum (`find` por `battle_scene*` devolve vazio, `CLAUDE.md` confirma ausência de
   `CMakeLists.txt`/`src/`/`include/`). O documento cita nomes de um consumidor planejado, não de
   código que roda aqui — não é spec "já implementada". Quanto à pergunta em si: os pips de Mana não
   eram forma errada nem tela desatualizada, era só metade do que agora existe — ver "Estoque e
   vazão" abaixo. Não editado: é spec de tela, e a L-27 deste projeto proíbe desenhar interface nesta
   fatia de qualquer forma.

### Estoque, vazão e saúde: as propriedades físicas da bateria (decisões do líder, 31/08/2026, 02/09/2026 e 03/09/2026)

**Em 31/08/2026 o líder respondeu a ambiguidade combat.md × bateria levantada acima: não são dois
medidores, é UM sistema, com propriedades físicas da mesma bateria. Em 02/09/2026 ele deu a essas
propriedades o vocabulário elétrico definitivo, acrescentou a terceira, a saúde, fundiu a vazão do
turno com a corrente que a bateria consegue entregar, e fixou que toda bateria perde carga com o
tempo. Em 03/09/2026 ele desdobrou a capacidade num par face e real, com faixa de desvio própria
por tipo de bateria, e fechou que as duas leituras de tensão (OCV e sob carga) são calculadas a
partir da carga e da saúde, nunca campo guardado à parte.**

> **Decisão do líder, 02/09/2026, verbatim:** *"Sobre a bateria: ela deve ter no POCO as
> caracteristicas de Tensão de repouso (tensão em circuito aberto / OCV) e Tensão sob carga (tensão
> de partida / cranking voltage). Na mesa de manutencao/craft e item de medicao da bateria, informa:
> estado de saúde (SoH) e capacidade de entregar corrente e informa: estado de carga (SoC). O
> parâmetro de fábrica correspondente é o CCA (Cold Cranking Amps / corrente de partida a frio):
> quantos amperes a bateria consegue fornecer. A tensão de repouso sozinha engana. Uma bateria velha
> pode estar cheia (12,6 V) e mesmo assim não acionar o efeito da carga, porque não aguenta a
> corrente de pico. A tensão de partida sozinha também engana se a bateria estiver só descarregada:
> uma bateria boa com 12,1 V em repouso também cai demais sob carga. Por isso o protocolo correto é:
> Medir tensão de repouso (SoC); Se estiver abaixo de ~12,4 V, recarregar primeiro; Só então fazer o
> teste sob carga (SoH / CCA). Resumo:*
> *voltagem parada = carga*
> *voltagem na partida = capacidade de entregar corrente (saúde / resistência interna / CCA)"*
>
> E, em seguida, verbatim: *"esses parametros podem ser testados na mesa de craft ou com um item
> chamado multimetro"*
>
> Respostas do líder no mesmo dia, por `AskUserQuestion`, verbatim: escala de tensão, *"Escala
> própria, ancorada nos 3 V da CR2032"*; vazão contra CCA, *"São o mesmo limite, e o teto 8 passa a
> depender da saúde"*; pico por efeito, *"Cada carta declara o próprio pico, independente do gasto"*;
> visibilidade da saúde, *"Só depois de medir; até lá fica desconhecida"*; instrumento, *"multimetro
> substitui voltimetro sim"*; entrega insuficiente, *"Uma coisa só: ela não funciona direito e drena
> a bateria interia, enquanto fica soltando fumacinhas e faíscas [sinalizacao visual ao jogador]"*;
> perda de carga, *"Toda bateria perde carga com o tempo (2a lei da termodinamica). Baterias
> originais de qualidade tem isso insensivel e faz parecer na prática que não perde, mas baterias
> velhas tem queda de carga em pouco tempo, mesmo sem uso."*; classes, *"cada carta tem sua classe.
> cada bateria tem sua classe, sao coisas diferentes"*.

**"Bateria de internet" é SINAL, não canal de compra duvidoso (decisão do líder, 03/09/2026, por
`AskUserQuestion`, ratificando a resposta do Gus Dragon na issue 3 do bus, 02/09/2026).** A leitura
que o líder deu em 02/09/2026 sobre a expressão "bateria de internet", de que ela descrevia
o canal de compra pela rede, junto a vendedores de origem duvidosa, foi superada nesta data.
Verbatim do Gus Dragon, ao escolher entre as duas opções que lhe foram devolvidas: *"Jeito 1, o 2
sinto que já foi coberto com as baterias privadas"*. O "Jeito 1" era uma bateria que precisa de
sinal para funcionar e só recarrega onde há cobertura dentro da dungeon; o "Jeito 2", que ele
recusou, era o canal de compra duvidoso, por já estar coberto pelas baterias "pirata / genérica" e
"de baixa qualidade (craftada)" que este §5 já descreve, acima. O líder ratificou a escolha do Gus
no mesmo dia. Regra de recarga por sinal, completa: ver "Bateria de internet: recarga por
cobertura de sinal", mais abaixo.

**Escala de tensão do jogo (decisão do líder, 02/09/2026):** os volts do jogo são uma escala própria,
ancorada no nominal de **3 V da CR2032** (§5, cabeçalho). ⚠️ **Os valores 12,6 V, 12,4 V e 12,1 V que
aparecem no verbatim acima são o exemplo do líder sobre bateria de carro real, e NÃO são valores do
jogo.** Ninguém os transcreve para tabela, catálogo, texto de carta ou fala de personagem. O que
sobrevive deles é a forma: três patamares (cheia, limiar de recarga, descarregada) como proporções
do nominal de 3 V, cujos números são do `economy-designer` e ratificação do líder. O vocabulário em
volts que o §5 já usava permanece.

Os termos SoC, SoH, CCA e OCV ficam em inglês, pela regra de nomenclatura do §4 (termos de TI e
engenharia são universais em inglês; o giro diegético entra no conceito, nunca na sigla).

1. **Capacidade (estoque), em vocabulário elétrico o estado de carga, SoC:** a carga da bateria
   ativa. Persiste entre batalhas, degrada, exige recarga real. É o que a barra na tela mostra, e
   **a carga aparece sempre** (decisão do líder, 02/09/2026). A leitura que revela o estoque é a
   **tensão de repouso (OCV)**: bateria parada, sem nada puxando corrente. Resumo do líder:
   *"voltagem parada = carga"*. Desde 03/09/2026, a capacidade se desdobra em face e real: ver
   "Capacidade de face × capacidade real", logo abaixo, nesta mesma seção.
2. **Vazão (taxa máxima de descarga por turno), que é a capacidade da bateria de entregar corrente
   (decisão do líder, 02/09/2026: "são o mesmo limite").** Por turno, o ator pode sacar até `2 +
   contagemPropriaDeTurnos`, a rampa de `combat.md` §5 relida como taxa, não como orçamento próprio
   (decisão de 31/08/2026, forma preservada). **O teto dessa rampa é o CCA efetivo da bateria em
   uso**: o CCA de fábrica do modelo, descontado pela saúde (SoH) daquele exemplar. Bateria nova de
   modelo bom entrega mais por turno; bateria velha, ou de modelo fraco, entrega menos, mesmo cheia.
   Vazão do turno e capacidade de entregar corrente não são dois limites: são um só, visto de dois
   lados, o do ator e o da bateria. **Consequência de trabalho, não de número:** o ritmo do combate
   (`combat.md` §5, `cartas-technomagik.md` §2.3, a rodada em que o custo premium das ESPECIAIS
   destrava) foi calibrado com um teto fixo de vazão e **precisa ser recalibrado**, porque o teto
   agora depende do hardware que o jogador estiver usando; é trabalho do `economy-designer`, com o
   líder ratificando, e nenhum número novo é proposto aqui.
3. **O limite real de cada turno é o MENOR entre a vazão do turno (a rampa, limitada pelo CCA efetivo
   da bateria) e o que ainda resta na bateria ativa.** Nunca se saca mais do que a bateria tem.
4. **Deixa de existir subtração dupla.** `cartas-spec-logica.md` §3.1 dizia que o `ManaCost` sai da
   bateria e "o mesmo valor que já sai da mana do ator"; isso descrevia dois medidores em paralelo.
   Agora é **uma subtração só**, da bateria, limitada pela vazão do turno. Redação corrigida no
   próprio `cartas-spec-logica.md` §3.1 (L-24 deste projeto: o que virou passado se apaga, não se
   guarda como histórico).
5. **Os dois elementos da tela existem, e cada um mostra uma coisa** (decisão do líder, 31/08/2026,
   completando o acréscimo de "Múltiplas baterias e a barra da tela"): **barra contínua = o ESTOQUE**
   (a carga da bateria ativa); **pips discretos = a VAZÃO** (quanto ainda se pode sacar neste turno;
   com a decisão de 02/09/2026, esse quanto depende da saúde da bateria em uso). Razão que o líder
   acolheu: as duas propriedades separadas passam a ser visíveis e distintas, e o jogador enxerga de
   relance se está limitado pela carga que resta ou pelo teto do turno; o sistema se ensina sozinho,
   sem tutorial, o que casa com o corte `C-16` (onboarding orgânico, sem parede de texto). **Isto
   resolve a contradição candidata 2 acima:** a spec do cockpit em `battle-screen.md` linha 50 não
   estava errada nem desatualizada, só descrevia um elemento (pips) onde agora cabem dois (barra de
   carga da bateria + pips de vazão); os pips de AP, que já existiam, não mudam, porque AP é outro
   recurso, com propósito distinto (`combat.md` linha 199). **Posição confirmada, sem conflito:**
   "abaixo do quadro do personagem" (verbatim do acréscimo anterior) bate com o cockpit da spec, onde
   o medidor de Mana já fica sob o retrato e o nome; a dúvida anterior era de FORMA (pips × barra),
   nunca de posição; registrado aqui para ninguém reabrir achando que há divergência de layout.
   **Requisito novo para a tela, registrado como fato e não desenhado aqui (L-27):** como a saúde só
   se conhece depois de medir (item 8), a interface vai precisar distinguir saúde DESCONHECIDA de
   saúde RUIM; a forma disso, assim como cor, tamanho, posição fina, ordem de empilhamento e
   marcação dos dois elementos, nasce com a camada `present/` e espera o GlintFx.
6. **Saúde (SoH) e capacidade de entregar corrente (decisão do líder, 02/09/2026):** a terceira
   propriedade, independente do estoque. Toda bateria nasce com um **parâmetro de fábrica, o CCA**
   (corrente de partida a frio): quantos amperes ela consegue fornecer no pico. O uso e os ciclos de
   recarga degradam a saúde (a resistência interna sobe), e com ela cai a corrente que a bateria
   ainda consegue entregar, que é o teto da vazão do item 2. A leitura que revela a saúde é a
   **tensão sob carga (tensão de partida)**: o quanto a tensão cai quando o efeito puxa a corrente
   de pico. Resumo do líder: *"voltagem na partida = capacidade de entregar corrente (saúde /
   resistência interna / CCA)"*. O CCA de cada modelo e a curva de queda por SoH são balanceamento
   (`economy-designer`), não decididos aqui.
7. **Por que cada medida sozinha engana, com os dois exemplos do líder** (volts de bateria de carro,
   exemplo dele, nunca valor do jogo): uma bateria **velha pode estar cheia** (12,6 V em repouso, no
   exemplo) **e mesmo assim não acionar o efeito da carta**, porque não aguenta a corrente de pico: o
   estoque está lá, a saúde não. E uma bateria **boa, só descarregada** (12,1 V em repouso, no
   exemplo), **também cai demais sob carga**: a saúde está lá, o estoque não. Estoque e saúde são
   eixos separados; ler um e concluir sobre o outro é o erro que o protocolo abaixo existe para
   impedir.
8. **A saúde é desconhecida até ser medida (decisão do líder, 02/09/2026).** A carga aparece sempre;
   a saúde só depois do teste sob carga, na mesa ou com o multímetro (item 9). O exemplar guarda se
   já foi medido (`is_health_measured`), e a regra do que pode ser mostrado é do domínio, obedecida
   pela tela (L-17, armadilha 5). **O protocolo de medição, na ordem do líder:** (1) medir a tensão
   de repouso, que dá o SoC; (2) se estiver abaixo do limiar de recarga (na escala do jogo, o
   patamar correspondente ao "~12,4 V" do exemplo dele), **recarregar primeiro**; (3) só então fazer
   o **teste sob carga**, que dá o SoH e a corrente disponível (CCA efetivo). Testar sob carga uma
   bateria só descarregada dá diagnóstico falso de bateria ruim.
9. **Onde se mede (decisão do líder, 02/09/2026):** na **mesa de manutenção/craft** (o ramo de reparo
   do `.gw.table`, a mesma bancada do §4/§14/§15.5 que já recarrega, troca bateria e limpa vírus) ou
   com o **multímetro**, item de catálogo que substitui o antigo medidor (ver "Itens de energia",
   abaixo): mesmo preço de 13 cr e mesma concessão gratuita pela missão do Volta. Os dois informam as
   mesmas três coisas: SoC, SoH e capacidade de entregar corrente. A mesa, além de medir, executa o
   passo 2 do protocolo (recarregar); o multímetro só mede. Nenhum dos dois revela nada de bateria
   que ainda não é do jogador: a regra do §14 ("as ferramentas NÃO revelam antes de comprar")
   continua de pé, senão o risco da bateria pirata morreria.
10. **O encontro entre os dois POCOs é o mecanismo central, e tem dois resultados (decisões do
    líder, 01/09/2026 e 02/09/2026).** A carta declara o que EXIGE, a bateria declara o que ENTREGA.
    Ao acionar, o jogo confere se a bateria instalada segura, sob a carga que a carta puxa, a tensão
    de pico que a carta pede. **(1) Entrega plena:** segura; o efeito sai inteiro e a carga cai o
    gasto normal. **(2) Falha por entrega insuficiente:** não segura; **a carta não funciona direito
    E a bateria drena por inteiro**, vira `DEPLETED` (`cartas-spec-logica.md` §3.2), **e enquanto
    isso acontece saem fumacinhas e faíscas, que são o SINAL VISUAL desse processo ao jogador**, não
    um estado mais brando anterior a ele. Verbatim do líder, 01/09/2026: *"Depende: se um poder
    gasta x de bateria e o jogador liga uma bateria que o máximo dela pela degradação ou tipo é x-10
    (por exemplo), além da carta não funcionar direito, ela drena a bateria inteira."* E, 02/09/2026,
    sobre o sinal: *"Uma coisa só: ela não funciona direito e drena a bateria interia, enquanto fica
    soltando fumacinhas e faíscas"*. "O máximo dela pela degradação ou tipo" é o CCA efetivo: o tipo
    dá o CCA de fábrica, a degradação o desconta. É a bateria velha cheia do exemplo 7, vista pelo
    lado da carta. O que "não funcionar direito" significa em cada carta é dado dela, no espírito do
    princípio canônico do §3 (a variação vem de dado cruzando originalidade × bateria). A carta do
    Volta já carrega o mesmo perfil, por decisão do líder de 02/09/2026: um pico de consumo para dar
    o início, depois consumo contínuo até acabar (`docs/design/mecanicas/cartas/volta.md`).
    **Fronteira (LEI ZERO, L-06, L-27):** o que é do GusWorld é o resultado (plena ou falha) e o
    evento que a falha emite; fumaça e faísca são desenho, nascem em `present/` ligada direto ao
    GlintFx, e nada disso se especifica aqui.
11. **Os dois lados da conta moram em átomos diferentes, e cada lado tem a própria classe (decisões
    do líder, 02/09/2026).** Verbatim: *"E cada carta passa a ter no POCO a tensao de pico necessária
    para acionar o efeito e o gasto de carga/mana, além da carga em repouso"*, e *"cada carta tem sua
    classe. cada bateria tem sua classe, sao coisas diferentes"*. A CARTA carrega a DEMANDA, em **três
    números soltos, nenhum derivado do outro** ("cada carta declara o próprio pico, independente do
    gasto"): a tensão de pico necessária para acionar o efeito, o gasto de carga/mana por
    acionamento (que já era o `ManaCost`) e a carga em repouso (o consumo por ficar equipada e ligada
    sem disparar, o "gasto de standby" da passiva, agora campo de toda carta). É essa independência
    que arma a armadilha desenhada pelo líder: uma carta de gasto baixo pode exigir pico alto, e a
    bateria velha e cheia não a aciona. A BATERIA carrega a OFERTA: capacidade (de face e real, ver
    "Capacidade de face × capacidade real" abaixo), CCA de fábrica, perda de carga com o tempo,
    carga atual, ciclos, se a saúde já foi medida. As duas leituras, tensão de repouso (OCV) e
    tensão sob carga, não são campos guardados à parte (decisão do líder, 03/09/2026): são
    CALCULADAS a partir da carga (SoC) e da saúde (SoH) já armazenadas, porque existe uma fonte de
    verdade só; é impossível um save ter carga cheia e tensão de bateria vazia ao mesmo tempo, e o
    validador pode recusar arquivo incoerente com essa regra. **A classe da carta (homebrew, pirata comum, pirata
    especial falso, comum original, especial) e a classe da bateria (os tipos do §5: de fábrica,
    selada das ESPECIAIS, pirata/genérica, de baixa qualidade craftada) são dois eixos
    independentes; capacidade é propriedade da BATERIA, nunca da classe da carta.** Nenhum campo de
    demanda entra na bateria e nenhum campo de oferta entra na carta (L-04, L-33). Cada TIPO de
    bateria é átomo com POCO próprio (verbatim: *"lembre que cada bateria é um átomo e cada um tipo
    tem seu poco"*). Campo existir não obriga valor diferente de zero. A especificação campo a campo
    dos POCOs é documento próprio (ver "Pontos ABERTOS").
12. **Toda bateria perde carga com o tempo (decisão do líder, 02/09/2026, segunda lei da
    termodinâmica).** Vale para todos os tipos, inclusive a de fábrica: na original de qualidade a
    perda é imperceptível, a ponto de o jogador jurar que ela não perde; na bateria velha a queda é
    rápida, **mesmo sem uso**. A perda é função da saúde (SoH) do exemplar, não só do tipo: a mesma
    bateria perde mais à medida que envelhece. Vale também para bateria guardada na mochila ou no
    slot de usadas, não só para a instalada numa carta. **"Com o tempo" é tempo DE JOGO contado,
    nunca o relógio da máquina** (`docs/tech/convencao-formatos-gw.md`, "A mochila registra, não
    age": degrada por uso e ciclo, nunca por tempo de parede, senão o replay byte a byte da L-17 não
    fecha). A curva da perda por SoH é número do `economy-designer`; a unidade de tempo de jogo em
    que ela se aplica é proposta de agente, não decisão do líder (ver "Pontos ABERTOS").

**Leitura que explica o conjunto:** a rampa deixa de ser um orçamento que aparece do nada a cada
turno e vira o quanto a bateria consegue entregar por vez, e esse quanto tem dono: a saúde da
bateria em uso. Bateria real tem capacidade, tem corrente máxima, tem saúde e perde carga parada; o
modelo agora tem as quatro, e a segunda e a quarta são função da terceira. A bateria pirata que
"mente sobre a carga" (§5, "Bateria pirata / genérica") cabe neste modelo sem regra extra: a carga
dela aparece boa, sempre visível, e a saúde, que é o que está ruim, fica desconhecida até alguém
medir (leitura de agente sobre as decisões de 01/09 e 02/09, não frase do líder; sinalizada para
confirmação).

**Duas consequências da resposta de 31/08, JÁ RESPONDIDAS no canon vizinho, não são pendência nova.**
Corrigido em 31/08/2026, depois de o orquestrador conferir o corpus inteiro antes de marcar qualquer
coisa como aberta:

- **(a) Ficar sem bateria no meio da batalha passa a ser possível, e o mecanismo JÁ ESTÁ ESPECIFICADO.**
  Antes não era possível: a rampa recarregava ao máximo todo turno. Agora o estoque pode acabar, e a
  carta vira `DEPLETED` (inerte, gate sempre `REJECTED` até troca, `cartas-spec-logica.md` §3.2). O
  jogador troca por uma bateria carregada do inventário em combate via ação de emergência dedicada,
  `SwapBattery`, custando **2 AP fixo, sem escalar por dificuldade** (`cartas-spec-logica.md` §3.3-3.4,
  `cartas-numeros-proposta.md` §1c). A bateria velha `DEPLETED` retirada vai para o inventário como
  item, não desaparece (ver "Descarte", abaixo). A carta do Volta já dependia exatamente disso
  (`docs/design/mecanicas/cartas/volta.md`). **Precedente do líder no mesmo padrão** (`TODO.md` item
  `F1`, decisão de 24/08/2026): a carta `glitch` não recebe exceção, "bateria em batalha: consome,
  sem exceção", porque "TODAS as cartas passam por originalidade E degradação da bateria; exceção
  nunca por omissão" (decisão do líder, 08/08/2026). Fora de combate, a mesma carta já fixa o que
  acontece quando a carga acaba no meio de um uso (dentro da parede: o personagem é cuspido de volta
  pelo lado por onde entrou, com preço em dano), precedente de forma, não de conteúdo, para o caso
  de combate. **O que continua genuinamente aberto, e só isto:** se ficar sem carga da bateria ativa
  no meio do combate tem alguma consequência ALÉM de não poder jogar a carta até trocar; o canon não
  cobre isso. A decisão de 02/09 acrescenta um segundo jeito de chegar a `DEPLETED`, a falha por
  entrega insuficiente do item 10, sem mudar o que acontece depois.
- **(b) A degradação da bateria ganha efeito mecânico direto, e a curva JÁ TEM NÚMERO.** Cada recarga
  tira **13 pontos percentuais de SoH** (começa em 100%); abaixo de **21% SoH** a bateria é
  considerada morta, só serve pra vender/reciclar no ferro-velho, dando **~6 recargas de vida útil**
  por bateria física (`cartas-numeros-proposta.md` §1b). O preço de recarga na estação também escala
  com o SoH entregue (base 3 cr, multiplicador 1,2×-2,0×, mesma fonte). Com a decisão de 02/09, o
  SoH deixa de ser só preço de recarga e passa a ser a grandeza que decide o teto de vazão de cada
  turno (item 2), se a bateria aguenta o pico de cada efeito (itens 6 e 10) e quanto ela perde
  parada (item 12); as curvas SoH → CCA efetivo e SoH → perda de carga são números novos, pendentes.

### Capacidade de face × capacidade real (decisão do líder, 03/09/2026)

> **Decisão do líder, verbatim:** *"Capacidade nominal de face é a mesma, e nas originais a de
> face é igual a real. Nas outras, isso muda. A de face é idêntica, mas a real muda já de inicio,
> varia entre 99%-70% (rng) do valor de face. Isso tem de ser discutido para cada tipo, de acordo
> com a característica de cada bateria."*

O campo único de capacidade que o item 1 acima descreve (na abertura desta seção) vira DOIS
campos, para todo tipo de bateria:

1. **Capacidade de face:** o valor anunciado, impresso na bateria. Idêntica em todos os tipos: de
   fábrica, selada das ESPECIAIS, pirata/genérica e craftada.
2. **Capacidade real:** o que a bateria de fato entrega. Nasce igual à de face ou abaixo dela,
   conforme o tipo, sempre desde a origem (fabricação ou craft), nunca por evento posterior. O
   comportamento de cada tipo está descrito na subseção correspondente dele, mais abaixo neste §5.

**Isto dá causa física à mentira da bateria pirata que este §5 já descrevia, na subseção "Bateria
pirata / genérica" ("mente sobre a carga").** A pirata não mente só na leitura exibida ao jogador,
ela É menor do que anuncia, desde a origem. Cuidado, para não reintroduzir campo redundante: isto
não é uma tensão ou capacidade guardada mentirosa à parte da capacidade real; é a capacidade real
sendo, ela mesma, menor que a de face, com a leitura exibida ao jogador mostrando a de face. Um
único campo de capacidade real basta; nenhum campo extra de "capacidade exibida" é necessário.

**Único número autorizado nesta decisão: a faixa de 70% a 99% do valor de face, sorteada (RNG), e
ela vale especificamente para a pirata/genérica (subseção abaixo), não é faixa padrão dos quatro
tipos.** Nenhum outro número (curva de degradação da selada, faixa craftada, distribuição exata do
sorteio) foi dado pelo líder nesta rodada; ficam para o `economy-designer`, com ratificação dele.

### Cada carta gasta a própria bateria: o fecho do sistema (decisão do líder, 31/08/2026)

**O líder respondeu a última ambiguidade do sistema — a que ficava marcada em `combat.md` §5 e
`cartas-spec-logica.md` §3.1: cada carta gasta a PRÓPRIA bateria; as avulsas do inventário são
reserva de troca.**

1. **A carta consome a bateria dela.** O modelo por-exemplar que `cartas-spec-dados.md` já descreve
   (carga, degradação e integridade por exemplar, não por catálogo) **continua valendo, sem mudança
   nenhuma.** Duas cópias da mesma carta seguem podendo ter cargas diferentes.
2. **Bateria avulsa é peça de troca.** O item de inventário que `cartas-spec-dados.md` linha 29 já
   registra (bateria comprável, "a pirata mente sobre a carga") é o que o jogador **encaixa numa
   carta descarregada** para voltar a usá-la. Não é um pool do qual as cartas puxam.
3. **A barra mostra a bateria da carta ATIVA**, e a mais cheia é o padrão quando o jogador não
   escolheu outra. "Selecionar outra bateria", na frase do líder que abriu esta seção ("Múltiplas
   baterias e a barra da tela"), é **escolher outra carta como ativa** — não escolher de um cinto
   comum.
4. **O Volta funciona sem regra nova:** ele esvazia a própria bateria por inteiro; o jogador encaixa
   uma reserva e usa de novo. Era exatamente o que a frase dele ("se tiver outra bateria, pode usar
   novamente") descrevia (`docs/design/mecanicas/cartas/volta.md`).

**A alternativa do cinto comum foi considerada e DESCARTADA pelo líder** — registrado aqui para a
hipótese não voltar: um pool de baterias compartilhado por todas as cartas do ator apagaria a
diferença entre duas cópias da mesma carta (uma pode estar com bateria pirata degradada, outra com
original cheia) e enfraqueceria o sistema de pirataria inteiro (§1-§3, §15), que depende da bateria
ser propriedade física de UMA carta específica.

**Isto fecha, sem ambiguidade restante, a pergunta `card_instance.battery`** que `G26`/`G27`
(`TODO.md`) vinham marcando: é a bateria própria da carta. Refletido em `combat.md` §5 e
`cartas-spec-logica.md` §3.1.

### Passiva tem dois gastos: standby e disparo (decisão do líder, 02/09/2026)

> **Decisão do líder, verbatim:** *"Neste jogo, mana e carga de bateria é a mesma coisa! Logo, mana 6
> é a mesma coisa de dizer que gastou seis de carga da bateria. Mude o canon, carta passiva tem dois
> tipos de gasto de mana/carga: gasto menor para ficar ativa em standby e carga maior quando o efeito
> for exercido/usado."*

Decisões anteriores do líder, no mesmo assunto, verbatim:

- 31/08/2026: *"opcao 1. mana = bateria. Leis da termodinamica: nao existe trabalho sem energia."*
- 01/09/2026: *"Nascem dois tipos de consumo."*
- 01/09/2026: *"A carta passiva tem uma mecanica de uso de bateria diferente: enquanto ligada em
  standby, usa o mínimo para ficar apenas ativa. Mas quano o poder dela é ativado, o gasto de energia
  sai pelo poder."*

**A carta `passiva` (`cartas/_vocabulario.md` §9) nunca tem custo zero de mana/bateria.** O canon
anterior descrevia a passiva como "ManaCost 0, equip-only" (`cartas-technomagik.md` §2.3), como se
ela não consumisse nada além do slot ocupado — essa leitura está revogada por completo; o texto que a
sustentava foi corrigido nos documentos que a repetiam, sem guardar a redação antiga como histórico
(L-24 deste projeto). No lugar dela, toda passiva tem **dois gastos distintos**, na mesma moeda
(mana = bateria, seção acima):

1. **Gasto de standby:** contínuo, menor, cobra enquanto a carta está equipada e ligada, mesmo sem o
   efeito ter sido exercido.
2. **Gasto de disparo:** maior, cobra no momento em que o efeito da passiva é de fato exercido/usado.

**Os dois números, quanto é o gasto de standby e quanto é o gasto de disparo, para cada uma das
passivas hoje catalogadas (Ada, Hayek, Mises, Planck, Pythagoras), não foram decididos pelo líder
nesta rodada e não são inventados aqui.** Ficam para a fatia seguinte: proposta do `economy-designer`
e ratificação do líder (item de INBOX aberto em `TODO.md`).

**Decisão do líder, 03/09/2026, por `AskUserQuestion`:** a mesma regra alcança a **face passiva**
das cartas Híbridas (Faraday, Maxwell, Newton, von Neumann, John Dee, `cartas-technomagik.md` §2.3),
mas não as cartas **Fora de combate** (Euler, Menger, Turing). A razão que o líder acolheu: a face
passiva da Híbrida fica LIGADA no personagem enquanto a carta está equipada, exatamente como as
cinco passivas puras (Ada, Hayek, Mises, Planck, Pythagoras); as três Fora de combate são
posse-only e não ficam ligadas em nada, então continuam em `mana 0`. Os números do gasto de standby
e do gasto de disparo de cada Híbrida não foram decididos nesta rodada e não são inventados aqui;
ficam para o `economy-designer`, item de INBOX em `TODO.md`.

### Troca e recarga
- **Cidade:** grátis — abrir inventário, pôr carregada no lugar da usada.
- **Estação de recarga:** te dá uma bateria **NOVA carregada** e cobra pela tua velha; o preço (**1,2x–2x** o de uma recarga) varia com a **degradação** da que você entrega.
- **Trade-off:** recarregar você mesmo custa *tempo de espera*; comprar já-carregada custa mais crédito.
- **In-battle (arena):** trocar (se tiver carregada) custa **1 turno + AP** (número → `economy-designer`).

### Descarte
- Bateria descarregada **ocupa espaço** no inventário e **não pode ser jogada fora** — **crime ambiental**, tratado com **sátira leve do excesso regulatório** (registro LucasArts; a megacidade te obriga a carregar teu próprio lixo tóxico). Só vender ou trocar. **Regra dura, não branda:** a interface não oferece a opção de descartar — não é uma escolha errada com punição, o jogo simplesmente não deixa (`cartas-spec-logica.md` §3.2: a bateria "não pode ser jogada fora", sem ramo de exceção).
- **Slot dedicado a baterias usadas** (acréscimo do líder, 31/08/2026, verbatim: *"Bateria usada vai pra o slot de baterias usadas da mochila do jogador, ou do bolso. Para levar pro lixo de baterias ou recarregar. Nunca deve ser jogada fora."*): a bateria `DEPLETED` que sai da carta (troca em cidade, estação ou emergência in-battle — `cartas-spec-logica.md` §3.2-3.4) tem um **compartimento próprio** na mochila/bolso, não uma vaga qualquer do inventário geral. Não é mecânica nova — é o mesmo `BatteryItem{charge:0}` que o canon já descreve, agora com endereço fixo: dali ela vai pro **lixo de baterias** (ferro-velho, reciclagem) ou pra **recarga** (estação, carregador). **Leitura, porque é o ponto:** o jogo usa pilha CR2032 real, e lítio é resíduo perigoso que não vai no lixo comum — a regra ensina descarte correto pela mecânica, sem discurso, mesma pedagogia por sistema da equivalência mana-bateria (onboarding orgânico, corte `C-16`). **Não decidido, marcado:** se o slot tem capacidade limitada, e o que acontece se ele encher.

### Itens de energia (novos)
- **Carregador solar** — recarrega baterias passivamente (evita depender de estação/compra).
- **Powerbank** com visor LED de carga própria restante.
- **Carregador rápido** de bateria.
- **Multímetro** (item de medição): informa estado de carga (SoC), estado de saúde (SoH) e capacidade de entregar corrente, na escala de volts do jogo ancorada nos 3 V da CR2032; a carga aparece sempre, a saúde só depois deste teste ou do teste na mesa (decisões do líder, 02/09/2026). Comprável por 13 cr OU adquirido na **missão do Volta**. Verbatim do líder: *"esses parametros podem ser testados na mesa de craft ou com um item chamado multimetro"*. É a peça que a ficha do Gus já lista no cinto dele (`docs/narrative/deep/characters/gus-dragon.md`).
- **Ghost do Volta** = o **tutor** de todo o sistema de energia (baterias, ciclos, degradação, recarga). Casa com o roster (cada análogo ensina sua área; Volta = a pilha).

### Bateria pirata / genérica
Existe: mais barata, mas **mente sobre a carga**, morre cedo, pode **danificar a carta** ou ser vetor de vírus. A fraude do enquadramento no nível do hardware — caveat emptor também nas pilhas.

**Capacidade real sorteada, o jogador não vê (decisão do líder, 03/09/2026, verbatim: "Desvio na
faixa que o senhor deu, e o jogador não vê").** A capacidade real nasce sorteada entre 70% e 99%
da capacidade de face, e a bateria continua exibindo a de face como se fosse a real; o jogador só
descobre medindo (mesa de craft ou multímetro, acima neste §5), o que dá razão de existir ao
multímetro. O sorteio é por exemplar: duas piratas do mesmo lote podem sair diferentes.

### Bateria de baixa qualidade (craftada, canon do Gus Dragon, decisão do líder 2026-08-25)

> **Canon pessoal (L-16 de `GODS_LAWS.md`), amarrado ao mesmo eixo do §12 (Bastiat: "o que se vê e
> o que não se vê") e à Doutrina do Comedimento (`economia.md` §0).** O líder, verbatim, sobre o
> suco de limão: *"Com o tempo, o personagem aprende a filtrar o suco de limão (craft) e obtem
> 'suco puro de limão', item obrigatório para craft de baterias de baixa qualidade mas que são
> melhor que nada pra quem tá sem bateria nenhuma no inventario. O suco puro de limao tambem pode
> ser obtido em lojas e negociacoes com personagens. Trades: se gastar para bateria, fica sem item
> para limpar debuff e vice-versa. Entra como mais uma lição do jogo: saber economizar e destinar
> corretamente recursos escassos."* E sobre a água com gás, no mesmo dia: *"[restaura fôlego ou
> energia de ação]. E pode ser usada em craft de bateria, após refinada para 'agua destilada'.
> Mesma logica do suco de limao."*

**Desvio depende da qualidade do refino (decisão do líder, 03/09/2026, verbatim: "O desvio dela
depende da qualidade do refino").** Quanto melhor o refino do suco de limão e da água com gás (os
dois insumos desta subseção, descritos logo abaixo), mais perto da capacidade de face nasce a
capacidade real. Isto abre um sistema que ainda não existe: não há, hoje, nenhum eixo de qualidade
de refino no canon, nem grau, nem número, nem mecanismo de como o refino varia. Ele depende de
duas decisões do líder que continuam abertas, já registradas mais abaixo nesta subseção ("Não
decidido, e não inventado aqui"): se o craft exige os dois insumos refinados juntos ou se cada um
sozinho já basta, e o momento narrativo em que o personagem aprende a filtrar. Nenhum sistema de
qualidade de refino é desenhado aqui; a decisão só registra que ele vai precisar existir.

Duas das doze comidas (`comidas-ingredientes-craft.md` §5.5.1, catálogo movido de `economia.md`
§5.5 em 2026-08-25, L-33) alimentam este sistema por DOIS insumos refinados:

- **Suco de limão** → refinado em **suco puro de limão**.
- **Água com gás** → refinada em **água destilada**.

**O refino é craft narrativo-gated, não uma bancada aberta desde o início.** "Com o tempo"
(verbatim do líder) é progressão: o MOMENTO exato em que o personagem aprende a filtrar (marco de
história, requisito de nível, ou entrega de um mestre do roster) não foi decidido e fica como
lacuna, ver "Em aberto" abaixo.

**Onde o refino e o craft da bateria acontecem: a MESMA bancada já canônica.** É o terminal RSB de
bancada fixa das oficinas do mercado negro/ferro-velho (§4, §14), a mesma que já cobre reparo
(recarregar, trocar bateria, limpar vírus) e cópia pirata de carta (§15.5): **não é sistema
paralelo**. Refinar suco/água e craftar a bateria de baixa qualidade a partir do insumo refinado
entram como mais duas operações dessa bancada, no mesmo `.gw.table`.

**A bateria de baixa qualidade é rede de segurança, não sucata. Regra dura, para ninguém
"consertar" isto depois:** o próprio líder chamou de "melhor que nada pra quem tá sem bateria
nenhuma no inventario". Ela é PIOR que a bateria de fábrica (reusa a física da EPROM da §3: upload
lento, degrada mais rápido), mas SEMPRE funciona o suficiente para tirar o jogador do zero-bateria.
Nenhum ajuste futuro pode reduzi-la a "sem efeito nenhum" ou "efetivamente inútil": isso contradiz a
razão de ela existir.

**Terceira rota, para o refino não virar gargalo de farm:** suco puro de limão TAMBÉM se compra em
lojas e se negocia com personagens; o líder foi explícito nisso, especificamente sobre o limão.
Pela "mesma lógica" que ele deu para a água com gás, o mesmo provavelmente vale para a água
destilada, mas isto é EXTENSÃO POR SIMETRIA (mesmo padrão de inferência já usado em §15.2 para as
ESPECIAIS), não uma frase dele sobre água destilada especificamente: sinalizado para confirmação,
não decidido aqui.

**O trade-off É a mecânica, não efeito colateral (verbatim do líder: "se gastar para bateria, fica
sem item para limpar debuff e vice-versa").** Suco de limão e água com gás são, cada um, UM item que
serve a DOIS fins mutuamente exclusivos: consumir agora (limpa debuff / restaura fôlego-AP, ver
`comidas-ingredientes-craft.md` §5.5.1) OU guardar e refinar (insumo de bateria de baixa
qualidade). Gastar um lado fecha o outro para aquela unidade do item. É a Doutrina do Comedimento (`economia.md` §0) em
miniatura: quem administra o estoque dos dois lanches com cuidado tem, ao mesmo tempo, cura
disponível E bateria de reserva; quem esbanja um lado fica sem o outro bem na hora que precisar.

**Coerência física, registrada como observação de acerto, não como invenção nova:** suco de limão e
água com sal/gás dissolvido são, no mundo real, a base de uma pilha caseira improvisada (eletrodos
de metais diferentes numa solução ácida ou condutora geram corrente). A escolha do líder é
fisicamente coerente com o Pillar 1 (a magia do mundo é sistema formal computável, a natureza é
matemática), mas isto é leitura do que ele já decidiu, não uma extensão de química nova sobre o que
ele disse.

**Herda os riscos da bateria pirata comprada (decisão do líder, 2026-08-25 — fecha a lacuna que
esta seção deixava aberta sobre se a craftada era categoria própria, sem riscos).** A bateria de
baixa qualidade craftada NÃO é uma alternativa mais fraca porém limpa: ela herda os TRÊS riscos já
atribuídos à bateria pirata comprada (subseção "Bateria pirata / genérica" acima) — pode **mentir
sobre a carga**, **morrer cedo**, e **danificar a carta ou ser vetor de vírus**. Faz sentido pela
mesma física da §3 (upload lento, EPROM, sem controle de qualidade de fábrica) e pela mesma lição
Bastiat do §12: craft caseiro não é imune ao "custo que não se vê" só por o jogador ter feito com as
próprias mãos.

**Isto CONVIVE com a rede de segurança, não a substitui.** O risco herdado NUNCA reduz a bateria
craftada a "sem efeito nenhum" — a regra dura logo acima ("melhor que nada pra quem tá sem bateria
nenhuma no inventário", verbatim do líder) continua de pé, intocada. O que muda é que o
`economy-designer`, ao numerar a chance de cada risco, tem que respeitar as DUAS pontas ao mesmo
tempo: a bateria craftada dá energia REAL (nunca zero, nunca falha garantida) **e** carrega risco de
verdade (nunca risco zero — senão ela viraria estritamente melhor que a de fábrica, o que quebraria
a Doutrina do Comedimento, `economia.md` §0: quem esbanja a bateria de fábrica não pode sair
ganhando ao trocar pela alternativa caseira sem custo nenhum). O ponto de equilíbrio exato
(probabilidade de cada risco, se é igual ou menor que o da pirata comprada) é número, não decidido
pelo líder nesta rodada.

**Não decidido, e não inventado aqui:**
- Se o craft exige suco puro de limão E água destilada JUNTOS, ou se cada um sozinho já basta para
  uma bateria de baixa qualidade (duas receitas paralelas).
- Capacidade, número de usos, preço de compra do suco puro/água destilada, taxa de sucesso do
  refino, o momento narrativo em que o personagem "aprende a filtrar", e a probabilidade numérica de
  cada risco herdado (mentir sobre a carga / morrer cedo / danificar a carta ou vírus): tudo é
  número de balanceamento (`economy-designer`), nada decidido pelo líder nesta rodada.

### Bateria de internet: recarga por cobertura de sinal (decisão do Gus Dragon, ratificada pelo líder em 03/09/2026)

A bateria que equipa a carta `glitch` (§13) é "de internet": ela só recarrega em área com
cobertura de sinal dentro da dungeon. Fora dessas áreas ela não recarrega, mesmo pelos canais já
descritos acima em "Troca e recarga" (cidade, estação, in-battle); a cobertura de sinal é
pré-condição da recarga desta bateria, não um canal alternativo a eles.

**Não é um quinto tipo de bateria no eixo da compra.** O líder já havia dito, em 02/09/2026, que
"bateria de internet" não nomeava um tipo à parte na tabela de baterias (§5, "Bateria pirata /
genérica" e "Bateria de baixa qualidade (craftada)"); a leitura de "internet" como canal de compra
duvidoso foi superada acima. O que nasce aqui é uma **regra de recarga por zona**, própria da
bateria desta carta, não um tipo novo na classificação de bateria.

**Fronteira da LEI ZERO (L-06, L-27 deste projeto):** cobertura de sinal como dado de mapa é
formato de mapa, e formato de mapa é do GlintFx. O que é do GusWorld aqui é a regra de jogo: onde a
bateria recarrega e o que muda quando não há sinal. Nenhuma etiqueta de bloco, campo de arquivo de
mapa ou marcação nasce deste parágrafo; se faltar função no GlintFx para isto, o pedido segue pelo
bus quando o trabalho esbarrar de verdade nela (L-07 deste projeto).

**Não decidido, não inventado aqui:** raio ou desenho da cobertura, taxa de recarga dentro da zona,
e quantas zonas de sinal existem em cada dungeon. Números e desenho de nível ficam para
`economy-designer`/`level-designer`.

## 6. Costuras entre software e hardware
- **Mod porco** → drena bateria mais rápido (custo do "compilado ruim").
- **Vírus** → pode sugar a bateria escondido (malware que consome energia).
- **Bateria pirata** → a fraude no nível do hardware.

## 7. Tutores diegéticos (análogos do roster, não NPCs novos)

Três análogos do roster ensinam o sistema, cada um na sua especialidade real (nenhum NPC criado só pra isso):

- **von Neumann** — o hardware da carta: **memória** (ROM/EPROM, capacidade em Runa, arquitetura stored-program) e **clonagem/pirataria** (a auto-replicação é a carta "Construtor Universal" dele; quem entende de cópia explica a pirataria). Reforça a amarra canon **von Neumann ↔ linhagem Neumann/Óxido** (Linda "Siren" Neumann) e a Óxido = "segurança opaca" (anti-tamper, o falso a detectar).
- **Turing** — a parte **forense/cripto**: detectar o vírus, ler o **conector interno escondido**, quebrar a cifra do falso (Enigma ↔ cripto-glifos).
- **Volta** — a **energia**: baterias, ciclos, degradação, recarga (ghost-tutor; ver §5).

## 8. Vírus de carta: payloads (efeitos)

Todo vírus é **oculto** — o jogador não sabe que pegou até se manifestar (ou até o Turing detectar). Tipos (podem coexistir, raridades/perigos diferentes):

- **Sabotador de combate (logic bomb):** a carta falha ou vira contra você no pior momento — o golpe do Dante generalizado. Dispara numa condição (turno crítico, chefe, HP baixo).
- **Backdoor / spyware:** a carta REPORTA ao inimigo; o Sterling "vê" tua mão/deck/posição e te antecipa. Explica diegéticamente como o vilão sempre sabe teus movimentos.
- **Worm de deck:** espalha pras outras cartas do deck em cadeia E **deixa a carta LENTA mesmo se ela for compilada/ROM** (nem a original escapa da lentidão — degrada performance por baixo). Cria pressão de contenção (isolar antes que contamine tudo).
- **Falso-benigno (isca Bastiat):** parece dar um BUFF ("carta turbinada!"), mas cobra um custo oculto retardado (dreno de bateria, HP, crédito ou debuff). O ensino do Bastiat puro.
- **Adware Sterling:** propaganda **indesejada e não solicitada** das indústrias Sterling. Fluxo ao acionar o feitiço: (1) a propaganda toma a tela (ex.: *"Os dispositivos Sterling são melhores pois têm mais qualidade. Conte conosco!"*); (2) espera alguns segundos; (3) o **X de fechar** aparece no topo direito, pequeno e quase invisível; (4) o personagem fecha; (5) só então o efeito da carta ocorre. Adware clássico — e sátira do corporativo-compadrio Sterling.
- **Zip-bomb (bomba de descompressão):** a carta parece pequena e inofensiva (poucos **kR**), mas ao soltar o feitiço ela **incha** e estoura a memória — trava/corrompe o deck, entope a memória (impede usar outras cartas no turno) ou consome a bateria de uma vez. Casa direto com o sistema de memória em Runa (§4). *(Ideia do Gus Dragon, playtester, 2026-07-18.)*

## 9. Como se pega vírus (vetores de contaminação)

- **Ao soltar o feitiço de uma carta contaminada:** probabilidade **X%** de contágio em TRÊS direções — (a) o **deck do inimigo no encontro** (o vírus vira **arma de duplo-gume**: pode sabotar o oponente, mas espalha nos dois sentidos); (b) o **ecossistema do mundo** (cartas em circulação — mercado negro, NPCs, trocas — vão ficando contaminadas; herda-se o vírus ao comprar/trocar de fonte suja); (c) o **próprio deck** (worm interno, §8). Single-player, mas com ecossistema de contágio.
- **Feitiço inimigo "sem efeito aparente":** certos ataques inimigos parecem não fazer nada — o personagem acha que nada aconteceu — mas infectaram (silencioso; casa com backdoor).
- **Databank pirata adulterado:** comprar databank pirata contaminado infecta.
- **Cartas grátis com adware** (§10): opt-in consciente.

**Risco de contaminação por tipo de carta** (`[calcular]` → `economy-designer`):

| Tipo | Risco |
|---|---|
| Especial (original) | **0** (não contamina) |
| **Super (original)** | **0** (não contamina — mesma ROM de qualidade suprema da especial; linha acrescentada 25/08/2026, §15) |
| Comum (original) | `[calcular]` — a ROM **não é reescrita**, mas o vírus age como **rootkit** (residente em runtime) |
| Pirata especial | `[calcular]` |
| **Pirata super** | `[calcular]` — mesma lógica do pirata especial, com o teto de 20% de fidelidade (§15.2) restringindo ainda mais o quanto a cópia se aproxima do original (linha acrescentada 25/08/2026, §15) |
| Pirata comum | `[calcular]` |
| **Homebrew (EPROM)** | `[calcular]` — **a mais vulnerável** (mais fácil pegar, mais difícil não ser afetada) |

Ordem de vulnerabilidade: **especial/super (0) < comum < pirata especial/pirata super < pirata comum ≪ homebrew.**

**Especiais e Super são IMUNES a vírus comum** (risco 0 — ROM de qualidade suprema). **O vírus do Dante no clímax é a exceção que confirma a regra:** não é infecção acidental, é uma **arma ESPECIAL fabricada pelas indústrias Sterling sob medida** para furar e neutralizar a carta **Gaiola de Faraday** especificamente; o Dante (acesso logístico) só a instala à noite. Uma especial só cai por uma arma dedicada, nunca por descuido — o que reforça o peso da traição e do poder industrial da Sterling.

**Apelo dramático (líder, 2026-07-18):** a Gaiola de Faraday é provavelmente a **primeira carta especial** do jogador (obtida cedo, no interior-faraday) e o acompanha o **jogo inteiro** — a especial em que ele mais confia e a que mais se apega. É exatamente ELA que o vírus-arma da Sterling mata no clímax. A carta-companheira "morre" na hora mais crítica: a traição do Dante dói mais porque tira do jogador o que ele tinha de mais familiar. Reforça a decisão de obtê-la cedo (§ guarda-chuva Faraday).

**Destino da carta no clímax (líder, 2026-07-18 — resolve AMB-DADOS-02):** a arma da Sterling **QUEIMA** a Faraday (ela "morre" de verdade — `is_burned_out`, não uma infecção curável comum). O **único** jeito de recuperá-la é o branch de **redenção do Dante** (ele, por ação, remove o vírus que plantou e restaura a carta). Dá peso enorme à escolha de redimir: salva a carta-companheira E o traidor de uma vez. Reconcilia com o "a redenção = remover o vírus" do guarda-chuva Faraday.

## 10. Cartas grátis (bônus de compra) e adware

- **Cartas grátis de bônus de compra** vêm **SEMPRE com adware** (§8), **avisado** ao personagem — ele concorda ou não (opt-in consciente).
- Pra se livrar da carta: **só no ferro-velho**. Não pode no lixo normal — descartar eletrônico em qualquer lugar é **crime ambiental** (mesmo raciocínio das baterias, §5).

## 11. Cura / antivírus

- Cartas contaminadas **não têm antivírus** de prateleira.
- O **Turing** faz o **diagnóstico** (avisa que a carta está infectada / que o falso é falso, antes de você usar) e oferece uma **cura PARCIAL ARRISCADA**: tenta remover o vírus com **X% de sucesso** e **X% de queimar** o chipset na tentativa (`[calcular]` → balance). É uma aposta consciente do jogador.
- Fora dessa aposta, a carta infectada é praticamente irrecuperável — **conviver, arriscar a remoção (e talvez queimar), ou descartar no ferro-velho**.

## 12. Arco econômico = o Bastiat do jogo inteiro ("o que se vê e o que não se vê")

- **Começo do jogo** (pouco dinheiro): é mais fácil adquirir **pirata** que original — tentador, barato, acessível.
- **Ao longo do jogo** (mais dinheiro, ganho por trabalho): passa a poder comprar **produtos de mais qualidade** (original confiável).
- É a lição do **Bastiat** encarnada na progressão inteira: o barato sai caro (vírus, instabilidade, adware); conforme prospera pelo trabalho honesto, o jogador migra do pirata pro confiável. Ensino diegético central, alinhado à axiologia canônica.

## 13. Cartas e features específicas (desta onda)

### Carta `urandom` (pirata × original) — ideia do Gus Dragon
Carta-caos baseada no `/dev/random` do Linux:
- **Efeito:** ao usar, sorteia o efeito de **qualquer carta do teu deck (incluindo especiais)**, podendo ser bom ou ruim, no **caster ou no inimigo** — totalmente imprevisível.
- **Ponderação inversa:** quanto mais forte o efeito, **menor a probabilidade** de sair (a maioria é fraca; os fortes são jackpots raros; puxar o efeito de uma especial é o jackpot mais raro de todos).
- **Duas versões (pirata × original):**
  - **Pirata** — achada no **mercado negro** (caótica, instável, muito azar — mais chance de cair no próprio caster / efeito ruim). A instabilidade é temática (fonte duvidosa, "nem sei o que faz"). Casa com o sistema de pirataria (§1–3).
  - **Original** — carta extra-especial **ÚNICA**, o **prêmio de completar a RunaDex** (§13): mais poderosa e com **ponderação generosa** (a "de verdade"). O tema pirata×original que criamos hoje, aplicado à própria carta do Gus.
- **Nome:** `urandom` (variante non-blocking do `/dev/random`; nome próprio arcano, velado o bastante pra não gritar "Linux"). Pesos/probabilidades das duas versões → `economy-designer`/balance.

### Carta `glitch` (aprovada pelo líder em 2026-08-21) — ideia do Gus Dragon (`Dragon-Drv`)
Atravessar parede é o bug clássico de jogo (o *clip* de speedrun); num mundo onde magia é programa, uma carta chamada `glitch` é um bug rodando de propósito.
- **O que faz:** fora de batalha, permite ao Gus atravessar blocos, para seguir em frente em masmorra ou lugar específico.
- **Alcance:** atravessa **somente blocos marcados como atravessáveis pelo designer**, um a um, no desenho da fase. Parede rachada sim; porta trancada não; borda do mapa nunca. Ferramenta de puzzle, não furo de progressão.
- **Duração:** **não é fixa.** Sai do tipo da carta (original, homebrew ou pirata), do tipo da bateria e da carga dela — exatamente como o princípio canônico da §3 acima já exige de toda carta.
- **Preso na parede e uso em batalha, FECHADOS (líder, 24/08/2026 e 03/09/2026; autoria das opções do Gus Dragon, issue 3 do bus, 21/08/2026):** ficar sem carga, ou sem sinal (ver §5, "Bateria de internet"), dentro da parede cospe o personagem de volta por onde entrou; por dificuldade, Fácil/Médio/Difícil voltam ao começo da sala e só o Hardcore reseta a dungeon inteira. Em batalha a carta não ataca (ou quase nada de dano) e tem 50/50 de bloquear o golpe do oponente. Detalhe completo, incluindo o preço em dano e o teste de aceite do ponto de retorno: `TODO.md` item `F1`.
- **Sem número, custo, raridade, nome de família ou efeito de batalha atribuídos** — nada disso foi decidido além do que está acima.

### RunaDex (índice de coleção, estilo Pokédex) — ideia do líder
Álbum/índice inspirado no Pokédex (o público-alvo ama; casa com "Pokémon na apresentação de batalha"). Nome amarra com a terminologia **runa** (§4): RunaDex = índice de Runas/cartas. **Três estados de entrada:**
- **Vista** (só viu, ex.: um inimigo usou): **silhueta / "???"** — dá senso de caça e mostra o que falta descobrir.
- **Tida** (já possuiu): **face revelada**.
- **No deck ativo:** face revelada + **moldura de neon azul**.

**Acesso:** pelos **óculos táticos** do Gus (diegético — HUD/scan, hardware-âncora do Pillar 3) **E** atalho no **menu de pausa** (conveniência).
**Completude:** mostra contagem de coleção (X/Y descobertas) e **completar tudo dá a `urandom` ORIGINAL** (carta extra-especial única, suprema — ver §13; ideia do Gus Dragon). A pirata você acha no mercado negro; a original é o troféu da coleção completa.

## 14. Mercado negro

### Estrutura (três camadas, todas)
- **Coração nos Ferrovelhos** (Periferia Industrial, setting #4, à sombra da **FIR**): o ecossistema de sucata onde convive a **tríade** — FIR oficial (monopólio-compadrio, caro) × **Sucata Honesta** (comércio livre honesto) × mercado negro (barato/livre/arriscado). Amarra Periferia + FIR + Dante + o descarte legal (§5/§10).
- **Rede oculta espalhada:** vendedores clandestinos em várias áreas do mundo, acessados por senha/reputação/contato — o mercado negro não é um lugar só.
- **Bazar noturno dedicado:** um hub concentrado e memorável.

### Vendedores
- **Vários, com REPUTAÇÕES diferentes** (confiáveis, golpistas, caóticos): a lição do **Bastiat** virada mecânica — escolher de quem comprar É o jogo. Cabe um vendedor-âncora recorrente entre eles pra dar personalidade/humor.
- **Um contato ligado ao Dante / FIR:** fonte perigosa com peso narrativo (amarra o traidor); pode destravar mercadoria específica ou ser uma armadilha.

### Enquadramento (recap §1)
O mercado negro **não** é o vilão — é resistência ao monopólio-compadrio da FIR/Sterling. O mal é a **fraude** (vírus, vender pirata como original) e a fonte podre não avaliada. O ferro-velho é também o **único descarte legal** de eletrônico/bateria (§5/§10).

### Avaliar a fonte (a lição Bastiat jogável)
Duas camadas que se combinam:
- **Reputação (macro, o vendedor):** a *taxa de confiabilidade* dele, **não binária** — um pode ser ~90% honesto, outro ~40%, outro golpista assumido. **Descoberta por:** (a) **experiência própria** (comprou ruim → o jogo lembra que aquele vendedor te passou a perna); (b) **fofoca de NPCs** (comentam quem é de confiança / quem é golpista → dá pra evitar golpe ANTES de cair). Orgânico e social, sem tela de estrelas.
- **Sinais/preço (micro, o item):** mesmo num vendedor de boa fama, *aquele item* pode ter preço bom demais / sinal sutil de pirata. Caveat emptor — "o que não se vê" mora no bom-demais-pra-ser-verdade.
- As **ferramentas (multímetro/Turing) NÃO revelam antes de comprar** (senão matariam o risco): são diagnóstico/cura **depois** (§11).

---

## 15. Pirataria alcança ESPECIAL e SUPER (decisão do líder, 25/08/2026)

> **Revoga o que bloqueava isto:** os cortes **C-03** e **C-13** da L-29 (`GODS_LAWS.md`) foram revogados nesta data. O GusWorld TEM crafting, e ele alcança as cartas — inclusive ESPECIAL e SUPER, sob a forma de **cópia pirata**. As ORIGINAIS continuam fora de alcance de qualquer bancada: só progresso narrativo (`cartas-technomagik.md` §2.3-2.4).

### 15.1 O que pode ser craftado, e o que nunca pode

| | COMUM | ESPECIAL / SUPER original | ESPECIAL / SUPER pirata |
|---|---|---|---|
| Craftável | sim, já era canon (`cartas-technomagik.md` §2.2) | **não, nunca** | **sim — decisão de hoje** |
| Mecanismo | compilação-no-cast (runtime) | progresso narrativo (Tavus-Eco do mestre; para a SUPER, missão-capstone — mecanismo exato de entrega em aberto, §15.6) | homebrew via RSB, gravado numa carta vazia, na bancada das oficinas do mercado negro/ferro-velho (§4 e §14 acima) |
| Fidelidade | não se aplica — não há "original" separado da carta pra imitar, é sistema aberto por natureza | é a régua, 100% por definição | **teto de 20%** (§15.2) |

O jogador nunca monta a original ESPECIAL ou SUPER numa bancada — ele monta, no máximo, uma **imitação**. É a mesma distinção que o anti-pillar do `gdd.md` já preservava para a COMUM (compilar ≠ inventar), estendida aqui: gravar uma cópia pirata de uma ESPECIAL ou da SUPER é **duplicar-tentando**, nunca criar (§3.6 acima).

### 15.2 Por que a fidelidade tem teto: engenharia reversa impossível

**Decisão do líder, 25/08/2026, sobre a SUPER especificamente, verbatim:** *"A original foi criada em asmódico puro, com compilador próprio que se perdeu ao longo dos tempos. As melhores cartas piratas nao alcancam nem 20% de fidelidade, pois RE é impossivel, já que a carta não está disponível para ninguém tentar fazer RE e entender os mecanismos."*

A causa não é dificuldade técnica arbitrária — é que **ninguém jamais teve a carta original em mãos para examinar**. Engenharia reversa exige uma amostra pra desmontar; a SUPER nunca circulou, então não existe amostra. O mesmo vale, por construção, para as 20 ESPECIAIS: cada uma é única, entregue direto pelo Tavus-Eco do mestre, e nunca esteve em circulação para alguém copiar por exame direto. _(Esta extensão às ESPECIAIS é inferência por simetria com a razão dada pelo líder para a SUPER — ele agrupou as duas no mesmo tratamento na decisão 1, mas a frase sobre RE impossível foi dita sobre a SUPER. Sinalizado no relatório para confirmação.)_

**Teto: as melhores cópias piratas não passam de 20% de fidelidade.** É o único número que o líder deu; nenhum outro valor (percentual de sucesso de craft, custo em crédito ou material, taxa por grau de falha) está decidido — fica para a onda de balanceamento, com `economy-designer`, junto do resto da tabela de risco de contaminação (§9) e dos preços já sinalizados como `[calcular]` neste documento.

### 15.3 Três graus de falha (decisão do líder, 25/08/2026)

Craftar uma cópia pirata de ESPECIAL ou SUPER nunca produz o original. O resultado cai num destes três graus, todos **abaixo** do original:

1. **Qualidade inferior:** a cópia funciona, faz o que a original faz, mas pior — mais fraca, menos confiável (reusa a física §3: EPROM em vez de ROM, upload lento, degrada mais rápido).
2. **Efeitos trocados:** a cópia funciona, mas faz algo **diferente** do que a original faz — a tentativa de reconstrução saiu errada e o comportamento resultante não é o pretendido.
3. **Só o nome, sem função nenhuma:** a cópia se apresenta como a carta (nome, aparência), mas **não faz nada** — a imitação falhou por completo.

Qual dos três graus sai de uma tentativa de craft é número de balanceamento (probabilidade), **não decidido**. O que está fechado é que os três graus existem e nenhum alcança o original.

### 15.4 A escolha econômica que isto abre

O tradeoff, verbatim do líder: *"tradeoff de comprar coisas ou craftar coisas tentando copiar original"*. Duas rotas para a MESMA cópia pirata de ESPECIAL/SUPER, com custo de natureza diferente:

- **Comprar pronta:** um clone-falso já feito, no mercado negro (§14, vendedores com reputação variável — a lição Bastiat já canônica). Custa crédito, e a qualidade da cópia comprada é aposta na reputação do vendedor.
- **Craftar você mesmo:** gravar a cópia na bancada (§4, RSB, oficinas do mercado negro/ferro-velho). Custa o material de craft e o risco do próprio resultado (§15.3), sem intermediário cobrando margem.

**As ORIGINAIS não entram nesse tradeoff** — elas continuam fora de qualquer canal comercial, só por progresso narrativo (`cartas-technomagik.md` §2.3-2.4: "nunca é loot de baú, nunca é compra"). O tradeoff é sempre entre duas cópias piratas, nunca entre pirata e original.

### 15.5 Onde isto mora tecnicamente

A bancada onde a cópia é gravada é a mesma do §4 (terminal de bancada fixo, RSB, oficinas do mercado negro/ferro-velho, §14) — **não é um sistema paralelo**. No formato de arquivo do projeto, essa bancada é o `.gw.table` (ver `docs/tech/convencao-formatos-gw.md`), que agora cobre tanto reparo (recarregar, trocar bateria, limpar vírus) quanto criação de cópia pirata, com um campo dizendo qual operação é qual.

### 15.6 Em aberto

O mecanismo exato de como a party recebe a carta SUPER original ao fim da missão-capstone — já que "forjar" deixou de ser a resposta — não foi decidido pelo líder nesta rodada. `cartas-technomagik.md` §2.4 sinaliza o mesmo ponto em aberto. Documentos fora do escopo desta rodada de edição ainda descrevem um "ritual de forja da Carta 21" (`docs/design/roster-analogos/21-helion-tusk.md`, `docs/narrative/deep/characters/mestre-cap-21-helion-tusk.md`) e citam "forjada" em outros dois lugares (`docs/design/mecanicas/cartas/_vocabulario.md`, `docs/design/mecanicas/deck-mao-sistema.md`) — contradições reabertas pela decisão de hoje, reportadas, não corrigidas aqui.

---

## Pontos ABERTOS (retomar aqui)
- ✅ **Números** (ENTREGUE: `cartas-numeros-proposta.md`) → `economy-designer` (preço de recarga, AP da troca in-battle, drain rate, capacidade kR/MR, vida de ciclos, **X% e a tabela de risco de contaminação da §9**, preço pirata vs original ao longo do arco). Sem item correspondente no `TODO.md` (L-30).
- ✅ **Spec de implementação** (ENTREGUE e APROVADA 2026-07-28: `cartas-spec-dados.md` + `cartas-spec-logica.md`) → `gameplay_engineer` (usos/bateria, estados de carta, vírus/adware) + `backend-engineer` (modelo de dados: tipo, memória, bateria, integridade, flag de infecção). Sem item correspondente no `TODO.md` (L-30).
- ✅ **Canonizado pelo líder em 2026-07-28.** Falta só refletir em `cartas-technomagik.md` e na terminologia (passo de escrita). Era: canonizar no doc de mecânicas + refletir em `cartas-technomagik.md` / terminologia quando o líder aprovar. Ponteiro (L-30): `TODO.md` item `G27`.
- **Camada pirata de ESPECIAL/SUPER (§15, decisão do líder 25/08/2026):** canon fechado, números em aberto → `economy-designer` (probabilidade de cada um dos 3 graus de falha, custo em crédito/material do craft, preço da cópia comprada pronta no mercado negro). Mecanismo de entrega da SUPER original ao fim da missão-capstone também em aberto (§15.6) → líder. Registrado no INBOX do `TODO.md` em 03/09/2026; ainda não é item numerado da tabela, e vira item quando a reordenação acontecer.
- **Bateria de baixa qualidade craftada a partir de lanches refinados (§5, decisão do líder 25/08/2026; herança de risco FECHADA no mesmo dia):** canon fechado no mecanismo (refino gated, bancada compartilhada com reparo/pirataria, trade-off consumir×refinar, rede de segurança, terceira rota de compra, herança dos três riscos da bateria pirata comprada) → `economy-designer` (capacidade/usos, preço de compra do suco puro/água destilada, taxa de sucesso do refino, probabilidade numérica de cada risco herdado). Duas lacunas de desenho, não numéricas, ainda em aberto → líder: se o craft exige os dois insumos refinados juntos ou cada um sozinho já basta; e o momento narrativo em que o personagem "aprende a filtrar". Registrado no INBOX do `TODO.md` em 03/09/2026; ainda não é item numerado da tabela, e vira item quando a reordenação acontecer. O item `G12` cobre só a origem dos insumos (suco de limão e água com gás), não estes números de craft.
