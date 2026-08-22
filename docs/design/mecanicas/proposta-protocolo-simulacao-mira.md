# PROPOSTA: Protocolo de simulação estatística da mira inimiga

> **Status: PROPOSTA, não canon.** Nada aqui vira regra do jogo. Este documento é o protocolo
> do estudo estatístico pedido pelo líder em 2026-08-01, para aprovação ANTES de rodar
> qualquer simulação. Autor: Capitolino (CPO). Nenhum número deste doc entra no motor como
> valor de produção; todos os valores de simulação são marcados `//SIM`.
>
> Origem: playtest ao vivo (2026-07-31), observação do líder: "quando fiquei apenas
> defendendo, o inimigo se concentrou em atacar apenas um (Cauã). Não sei se é o certo."
> Pergunta de design, verbatim do líder: "qual seria a estatística sobre 'é melhor derrubar
> a parede logo enquanto tenho força, ou deixar a parede para depois comigo estando já
> enfraquecido?' não quero opinião, quero dados internos da nossa engine de luta."
>
> Cross-ref: `combat.md` (motor), `proposta-mira-inimiga.md` (fatores F1-F8),
> `pillars.md`, `docs/design/mecanicas/cartas-technomagik.md`.

---

## 0. Princípio do estudo (a régua que decide "melhor")

Uma coisa precisa ficar declarada antes de qualquer número: **o objetivo NÃO é achar a
política de mira que faz o inimigo vencer mais.** Um inimigo maximamente letal pode ser um
produto pior. "Melhor método" aqui significa **melhor experiência para o nosso público**
(criança 11+ e adulto nostálgico de SNES), medida por proxies que a simulação alcança:

1. Luta de trash dentro da janela canônica de **3 a 5 rodadas** (§15.1 do combat.md).
2. Fogo **espalhado** o bastante para ninguém virar saco de pancadas (a queixa de origem),
   mas **concentrado** o bastante para o inimigo parecer inteligente, não aleatório.
3. O **Gus ameaçado, nunca massacrado** (contrato de fragilidade, §2.1; Pillar 4).
4. Comportamento **legível**: o jogador precisa conseguir formar uma teoria de "por que ele
   atacou fulano" (Pillar 1: o jogo se vence lendo o sistema).

A taxa de vitória serve só de guarda-corpo de balance: uma política que muda muito a taxa
de vitória mexeu na dificuldade, não só na sensação, e isso é decisão separada.

**Validade interna:** a simulação roda o **motor real** (`CombatStateMachine` headless,
POCO puro, o mesmo caminho dos testes e do auto-resolve §19.6), nunca uma cópia das regras
em script. É exatamente o que o líder pediu: "dados internos da nossa engine de luta".
Fórmula de dano §11, Shield real do Defend (`resolve_defend`: Shield com Magnitude = Def,
Duration 1, Replace), fila CTB real, RNG injetado e seedável (`IRandomSource`, ADR-006).
Cada luta é reproduzível pela seed: qualquer resultado estranho pode ser re-rodado
golpe a golpe.

---

## 1. Opinião prévia declarada (para os dados poderem me contrariar)

O líder pediu opinião ANTES dos dados, por escrito, para o experimento ser honesto. Aqui
vai a minha, com o raciocínio e com o que me faria mudar de ideia.

### 1.1 O fato do motor que estrutura tudo

No nosso motor, a "parede" do Defender **não é um muro permanente: é um muro que renasce**.
Defender aplica Shield com pool = Def e **dura 1 rodada**; se o personagem defender de novo
no turno seguinte, o muro volta inteiro, de graça (custa só a ação dele). E o dano acima do
pool **passa** (overflow: `Hp -= dano - absorvido`).

Isso muda a pergunta. "Derrubar a parede logo enquanto tenho força" faz sentido contra
parede de tijolo (quebrou, acabou). Contra parede que renasce a cada rodada, o dano
investido nela **evapora**: na rodada seguinte o pool está cheio de novo. Já o dano no
personagem sem defesa é **permanente** (ninguém regenera HP de graça no nosso combate).

### 1.2 Minhas três hipóteses

- **H1 (mecânica):** contra parede renovável, "deixar a parede para depois" (evitar o
  defensor, F4 negativo) é a jogada mecanicamente mais forte para o inimigo. Bater na
  parede é pagar dano por um recurso que o adversário repõe de graça.
- **H2 (produto, a ressalva):** F4 forte demais **recria a queixa original com o sinal
  trocado**. Se o inimigo quase nunca bate em quem defende, o fogo inteiro se concentra nos
  não-defendidos (de novo "bateu só no Cauã"), e defender vira botão de invulnerabilidade:
  o jogador manda o Gus defender para sempre e o inimigo o ignora para sempre. Minha aposta
  é em **F4 moderado**: reduz a atração, não a zera.
- **H3 (a exceção):** "quebrar a parede cedo" só deve vencer quando o golpe médio do
  inimigo **ultrapassa com folga o pool** do Shield (Atk alto contra Def baixa): aí o
  overflow passa e a parede quase não desconta. Ou seja, se houver um cenário onde quebrar
  cedo ganha, aposto que é o de inimigo elite, não o de trash.

### 1.3 O que me faria mudar de ideia

1. Se o braço "quebra a parede cedo" (E, abaixo) vencer mais E mantiver o espalhamento de
   dano saudável E a duração dentro da janela 3-5: eu estava errado e a intuição "inimigo
   esperto não bate em parede" cai.
2. Se o braço de F4 forte (D) NÃO produzir concentração de fogo nem o exploit de defesa
   eterna nos lotes 2 e 3: a minha ressalva H2 era medo infundado e F4 pode ser agressivo.
3. Se a parede quase não mover nenhuma métrica em nenhum braço (overflow domina com os
   statlines canônicos): F4 vira detalhe cosmético e não vale complexidade de peso.

---

## 2. Desenho do experimento: braços e lotes

Analogia direta com ensaio clínico: cada **braço** é uma política de mira (o "tratamento"),
cada **lote** é um cenário de luta (a "população"), e rodamos **as mesmas 200 lutas
semeadas** de cada lote sob cada braço. Comparar braços dentro do mesmo lote isola o efeito
da política; comparar lotes sob o mesmo braço mostra onde cada política quebra.

### 2.1 Os 6 braços (políticas de mira inimiga)

| Braço | Política | O que representa |
|---|---|---|
| **A** | `players.front()` (status quo) | Controle 1: o comportamento de hoje, que gerou a queixa |
| **B** | Sorteio uniforme entre vivos | Controle 2: ruído puro, inimigo "burro aleatório" |
| **C** | Ponderada F1+F2+F3, **F4 suave** (peso do defensor ×0.5) | "Evita a parede, mas não sempre" (minha aposta H2) |
| **D** | Ponderada F1+F2+F3, **F4 forte** (peso ×0.1) | "Quase nunca bate na parede" |
| **E** | Ponderada F1+F2+F3, **F4 invertido** (peso ×2.0) | **"Quebra a parede cedo, enquanto tenho força"** (a hipótese rival do líder) |
| **F** | Ponderada F1+F2+F3, **sem F4** (×1.0) | Isola o que F4 acrescenta: se F ≈ C ≈ D, F4 é irrelevante |

A pergunta-mãe do líder é respondida pelo contraste **C/D (deixar para depois) × E (quebrar
cedo) × F (indiferente)**, com A e B como réguas de referência.

### 2.2 Os 7 lotes (cenários)

Lote 1 é vanilla por ordem do líder. Os demais cruzam vantagem/desvantagem entre a party e
os inimigos, como ele pediu. Para cada lote: o que varia, o que fica fixo, e a pergunta que
SÓ ele responde.

**Fixo em todos os lotes, salvo indicação:** party canônica de referência (Gus HP 34 / Atk 8
/ Def 5 / SPD 9; Cauã 55/14/8/13; Jaci 55/9/10/7), inimigos Sentinela-Bit (55/10/8, trash,
1 AP), fórmulas e FSM reais, `KnowledgeKills = 0` (1º encontro), cap de segurança de 30
rodadas (luta que chega lá registra como "empate técnico", não trava o estudo).

| # | Nome | O que varia | Pergunta que só ele responde |
|---|---|---|---|
| **L1** | **Vanilla** | Nada: 3 heróis (bot sub-ótimo §19.6, só ataque básico) vs 3 Sentinela-Bit. Ninguém defende, nenhuma carta. | Linha de base: sem parede nenhuma, os fatores F1+F2+F3 sozinhos já produzem espalhamento e duração melhores que o status quo (A) e que a moeda (B)? Como F4 nunca dispara aqui, C=D=E=F por construção: é o teste de sanidade do harness (se divergirem, tem bug). |
| **L2** | **Parede dedicada** | Jaci (maior Def, 10) **defende todo turno**; Gus e Cauã atacam. | **A pergunta-mãe, na forma pura:** com UMA parede renovável e DOIS alvos moles, quebrar a parede cedo (E) rende mais que deixá-la para depois (C/D)? E o custo UX: sob D, a Jaci vira intocável e o fogo esmaga os outros dois? |
| **L3** | **Turtle total** | Os 3 heróis **só defendem**, a batalha inteira (reproduz o playtest do líder). | O caso degenerado: quando TODOS são parede, o que cada braço faz? A luta estoura o cap de rodadas (empate infinito)? Precisa de regra de fallback ("se todos defendem, os pesos se re-normalizam")? Este lote também mede se turtling é exploit viável de sobrevivência. |
| **L4** | **Suporte ativo** | Jaci **cura o mais ferido** todo turno (cura simulada `//SIM`); Gus e Cauã atacam. | Fator F3: o inimigo que aprende a focar a curandeira devolve a luta para a janela 3-5? E o inimigo que a ignora (A, B) produz luta arrastada (cura anula dano)? É o único lote onde a duração pune diretamente a política burra. |
| **L5** | **Cartas de agro (vantagem dos heróis)** | Cauã joga **honeypot** na rodada 1 (peso ×3.0, +10% Def, 2 rodadas, teto canônico do líder); Gus recebe **Firewall DROP** (peso ×0, 2 rodadas). Jaci ataca. | Fatores F5/F6: as cartas cumprem a promessa sob cada braço? O honeypot com só +10% de Def é tática (concentra fogo aguentável) ou suicídio (o portador derrete)? O Firewall protege o Gus sem virar imortalidade (o fallback "todos protegidos → default" funciona)? |
| **L6** | **Vantagem dos inimigos** | Inimigos viram 1 Daemon-Guard elite (Atk alto `//SIM`) + 2 Sentinela-Bit; heróis entram com 70% do HP. Jaci defende todo turno (parede sob pressão). | O estresse: com overflow alto (Atk elite > pool do Shield), o veredicto da parede inverte (minha H3)? E sob pressão, a ponderada vira "bullying do ferido" (F2 empurra o fogo para quem já está caindo)? O Gus cai demais (Pillar 4)? |
| **L7** | **Atacante único** | 1 Daemon-Guard elite vs party 3; Jaci defende todo turno. | Com UM inimigo, cada escolha de alvo é 100% do fogo da rodada: é o cenário onde a política de mira mais aparece na tela do jogador, e o mais plausível para "quebrar a parede cedo" (um atacante só não pode dividir atenção). Nenhum lote com 3+ inimigos isola isso, porque lá o fogo médio se dilui. |

**Por que 7 e não 5:** L1 é exigência do líder; L2 e L3 são as duas formas da pergunta dele
(parede única e parede total); L4 é o único que exercita F3; L5 o único que exercita F5/F6;
L6 o único com desvantagem dos heróis + vantagem dos inimigos (pedido explícito dele); L7 o
único com atacante único. Nenhum responde a pergunta de outro. Se o líder quiser enxugar,
**L7 é o primeiro a cair** (a pergunta dele é a mais refinada e a menos urgente); L2 e L3
são inegociáveis, porque são a pergunta de origem.

**Fora do estudo, de propósito:** F7 (vendeta declarada) fica FORA da simulação: é gatilho
narrativo, o peso dele se decide por dramaturgia e leitura de cena, não por estatística.
F8 (explorar fraqueza elemental) fica fora porque o líder já decidiu que é só chefe/tier
alto, e chefe é puzzle desenhado à mão, não estatística de trash. Boss em geral está fora:
este estudo é sobre a mira de encontro comum.

---

## 3. Quantas lutas (o N)

> ### ✅ DECIDIDO PELO LÍDER EM 2026-08-01 (sobrepõe a proposta desta seção)
>
> | Parâmetro | Valor aprovado |
> |---|---|
> | **N por política (braço)** | **240.000** |
> | **N por cenário (lote)** | 240.000 × 6 braços = **1.440.000** |
> | **Total do estudo** | 7 lotes × 1.440.000 = **~10.080.000 lutas** |
> | **Braços** | os 6 propostos (§2.1) |
> | **Lotes** | os 7 propostos (§2.2) |
> | **Bot da party** | construir as variantes necessárias (defender, curar, jogar carta) |
> | **Janela do harness** | corrigir de 4-8 para **3-5** (§15.1) antes de rodar |
>
> **Custo de máquina MEDIDO (não estimado), 2026-08-01:** o harness atual resolve
> **25.000 lutas em 0,47 s** (~53.000 lutas/s, ~19 µs por luta) no binário `linux-release`.
> Pelo N aprovado, 10 milhões de lutas dão ~3 min no perfil atual; com lutas mais ricas
> (defesa, cura, cartas, sorteio ponderado) o tempo cresce, mas segue na casa dos minutos.
> Justificativa do líder: *"Se o custo de máquina é irrelevante e estatística é ciência de
> medir repetições, não há problema."*
>
> **Por que o N grande passou a fazer sentido AGORA:** a saída atual do harness mostra
> `mean = median = p95` (variância ZERO), porque o ataque básico é aritmética determinística
> e o bot não consome RNG. No instante em que a mira vira **sorteio ponderado**, entra acaso
> de verdade no resultado, e o tamanho de amostra deixa de ser cerimônia.
>
> ### ⚠️ Piso de relevância (MCID), decidido pelo líder: **5 pontos percentuais**
>
> Com 240.000 lutas por braço, a margem de erro de 95% de uma proporção cai para cerca de
> **±0,2 ponto percentual**. Nesse tamanho **quase toda diferença dá "estatisticamente
> significativa"**, inclusive as que ninguém sente jogando (o análogo clínico: p < 0,001 com
> redução absoluta de risco de 0,3%). Por isso o líder fixou, **antes de ver qualquer dado**,
> a menor diferença que conta como diferença real:
>
> - **Diferença < 5 pontos percentuais entre dois braços = EMPATE**, mesmo com significância
>   estatística. Não entra na decisão de design, não vira argumento, não é reportada como
>   "vantagem de X sobre Y".
> - Vale para as métricas em percentual (E1 taxa de vitória, E3 concentração, E4 quedas,
>   E5 previsibilidade, E6 eficiência da parede, E7 exploit).
> - Para métricas em rodadas (E2, E8), o análogo é **0,5 rodada**.
> - **Regra de escalonamento por empate (§3 original) fica SEM EFEITO:** com este N não há
>   empate por falta de amostra; empate aqui significa "a diferença é pequena de verdade".
>
> ### Progresso no terminal (ajuste físico, não de conteúdo)
>
> O formato pedido é `lote [x] de [7], simulação [n] de [1440000]`. Uma linha por luta seriam
> 10 milhões de linhas, ilegíveis e mais lentas que a própria simulação. **A linha é a mesma,
> impressa a cada 1% do lote** (144 atualizações por lote, ~1.000 no estudo inteiro). Mesma
> informação, com frequência que cabe na tela.

### Análise original da proposta (mantida por registro)

O líder pediu 200 por lote. Resposta honesta sobre poder estatístico, em linguagem de
ensaio clínico:

- **200 lutas por braço-lote detectam com segurança diferenças GRANDES**: em taxa de
  vitória, uma diferença de uns 10 a 15 pontos percentuais entre dois braços aparece de
  forma confiável (com 200 por braço, a margem de erro de uma proporção fica em torno de
  ±7 pontos no pior caso). Para médias (duração em rodadas, espalhamento de dano), 200 é
  mais que confortável: a média estabiliza com margem pequena.
- **200 NÃO detecta diferença fina** (uns 5 pontos percentuais) nem mede bem **evento
  raro** (ex.: party wipe a ~5% de frequência): nessas zonas dois braços podem parecer
  iguais sendo diferentes.
- **Custo de rodar mais é desprezível:** a FSM headless resolve uma luta em menos de 1 ms
  (§19.6). O estudo inteiro (7 lotes × 6 braços × 200 = 8.400 lutas) roda em segundos.

**Proposta: 200 é o padrão, como pedido, com uma regra de escalonamento pré-declarada**
(para não virar pesca de resultado): se, na métrica decisória de um lote, dois braços
terminarem estatisticamente empatados mas com diferença que MUDARIA a decisão de design,
esse par re-roda com 1.000 lutas, e só ele. Não inflo por segurança: 200 basta para a
maioria das leituras que queremos.

Nota de transparência sobre a contagem: cada lote tem 6 braços, então "lote de 200" vira
1.200 lutas por lote (200 por braço). A linha de progresso no terminal segue o formato do
líder: `lote [x] de [7], simulação [n] de [1200]`.

---

## 4. Estatísticas a medir (nome, cálculo, decisão que informa)

Uma taxa de vitória sozinha não diz nada sobre diversão. A bateria abaixo mede a
experiência, não só o resultado.

| # | Métrica | Como se calcula | Que decisão informa |
|---|---|---|---|
| **E1** | **Taxa de vitória da party** (+ margem de erro 95%) | vitórias / 200 | Guarda-corpo de balance: braço que muda muito a taxa de vitória alterou a dificuldade, não só a sensação. Não é o critério de escolha, é o alarme. |
| **E2** | **Duração em rodadas** | média, mediana, e a faixa onde caem 80% das lutas (percentis 10 e 90); % de lutas dentro da janela canônica 3-5 | O canon manda trash durar 3-5 rodadas. Braço que estica (ex.: ignorar a curandeira em L4) ou encurta demais reprova, mesmo vencendo bonito nas outras métricas. |
| **E3** | **Concentração de dano** | % do dano total da party que caiu no membro MAIS atingido; e % de lutas em que um único membro levou mais de 70% do dano ("luta do saco de pancadas") | A queixa de origem em número. Régua de leitura: ~33% = fogo perfeitamente dividido em party de 3; 100% = o playtest do dia 31. Informa a MAGNITUDE dos pesos (F4 inclusive): queremos o meio do caminho, com viés a definir pelo líder. |
| **E4** | **Quedas** | % de lutas com ao menos 1 herói a 0 HP; QUEM cai (por personagem); quedas do Gus e disparos da Análise Preditiva (§2.1) contados à parte | Pillar 4: o Gus é o mais frágil por contrato e NÃO pode ser o que mais cai em encontro trash. Braço que derruba o Gus com frequência exige contrapeso (ex.: firewall barato cedo no jogo) ou é vetado. |
| **E5** | **Previsibilidade da mira** | % de turnos inimigos em que o alvo REPETIU o da rodada anterior (taxa de repetição) | Legibilidade (Pillar 1): 100% de repetição = decoreba (o status quo A); repetição no nível da moeda (~33% em party de 3) = ruído ilegível (B). Queremos um meio: o inimigo persegue uma lógica, mas reage. Informa se o sorteio ponderado precisa de "inércia de alvo" (bônus por manter o alvo) ou de mais temperatura. |
| **E6** | **Eficiência da parede** | % do dano bruto inimigo absorvido por Shield; dano inimigo "desperdiçado" em pool que expirou cheio | **A pergunta-mãe em termos do motor.** Se sob o braço E (quebrar cedo) a maior parte do dano morre em pool renovável, quebrar cedo é objetivamente queimar força à toa (H1 confirmada). Se o overflow passa quase tudo (L6), a parede é fina e F4 perde importância (H3). |
| **E7** | **Exploit de defesa** (L2/L3) | Taxa de vitória e HP final médio da party quando 1 ou 3 membros só defendem; % de lutas em L3 que batem no cap de 30 rodadas | Se defender para sempre VENCE com folga sob F4 forte (D), criamos estratégia dominante degenerada (anti-pillar §15). Informa o piso do peso F4 (nunca zerar) e se precisa regra de fallback quando todos defendem. |
| **E8** | **Rodada da primeira queda** | média e mediana da rodada em que o primeiro herói cai (nas lutas em que alguém cai) | Pacing de tensão: queda na rodada 1-2 = paulada injusta (público 11+); queda só no fim = tensão bem construída. Complementa E2. |

Leitura cruzada pré-declarada (para não escolher a métrica depois de ver os dados): a
decisão principal usa **E3 + E5 + E6** (espalhamento, legibilidade, física da parede), com
**E2** como restrição dura (janela 3-5) e **E1/E4** como vetos (balance e Pillar 4).

---

## 5. Valores inventados (a maior fonte de erro, toda ela visível)

Tudo abaixo NÃO tem POCO nem definição canônica e entra na simulação com valor razoável
`//SIM`. Nenhum vira canon por ter sido usado aqui; a onda de implementação real re-decide
cada um com o líder.

| # | O que falta | Valor `//SIM` proposto | Justificativa |
|---|---|---|---|
| V1 | **Peso base de mira** | 100 por herói vivo | Escala arbitrária de referência; só as RAZÕES entre pesos importam no sorteio. |
| V2 | **F1 (dano causado atrai)** | +1 de peso por ponto de dano causado nas últimas 2 rodadas | Janela de 2 rodadas = memória curta, legível ("ele revida quem bateu"); linear para simplicidade. Um Cauã batendo forte (~30 de dano em 2 rodadas) dobra a própria atração. |
| V3 | **F2 (ferido atrai)** | +100 × (1 − HP/HPmax) | Herói a 50% ganha +50; a 20% ganha +80. Instinto de finalização sem virar execução certeira. |
| V4 | **F3 (suporte atrai)** | +60 de peso se curou/buffou aliado nas últimas 2 rodadas | Ordem de grandeza entre F1 típico e F2 grave: "matar o curandeiro" é prioridade clássica, mas não obsessão. |
| V5 | **F4 (defendendo repele)** | multiplicador por braço: ×0.5 (C), ×0.1 (D), ×2.0 (E), ×1.0 (F) | É a variável experimental. As quatro magnitudes cobrem o espectro de "suave" a "invertido". |
| V6 | **Honeypot (F5)** | peso ×3.0 por 2 rodadas; +10% Def enquanto dura | O teto de +10% de Def é decisão do líder (canon da proposta de mira). O ×3.0 e as 2 rodadas são chute a validar: L5 existe para isso. |
| V7 | **Firewall DROP (F6)** | peso ×0 por 2 rodadas; se TODOS os vivos estiverem com peso 0, re-normaliza para uniforme | O fallback é obrigatório (sem ele, party toda protegida = inimigo sem ação válida). |
| V8 | **Atk do Daemon-Guard** | 18 | TBD no canon (§17). Dá dano 10 no Cauã (Def 8) e 13 no Gus (Def 5): elite ameaça de verdade sem one-shot. |
| V9 | **SPD dos inimigos** | Sentinela-Bit 8, Daemon-Guard 10 | TBD no canon. 8 encaixa o trash entre Jaci (7) e Gus (9); elite 10 abre a rodada às vezes, pressiona sem dominar. |
| V10 | **Cura da Jaci (L4)** | 12 HP no aliado mais ferido, 1 AP + 2 mana | Não existe carta de cura canônica fechada com número; 12 HP anula ~2 golpes de trash, o suficiente para o fator F3 ter consequência mensurável. |
| V11 | **Bot da party** | `AutoResolveBrain` (§19.6: ataque básico + no máx. 1 carta single-target; sem Scan/Gambito/combo) + as variantes por lote (defensor dedicado, turtle, healer) | Já é a IA de party canonizada para rodar a FSM sem humano. Viés declarado: o bot é pior e mais previsível que um jogador real. |
| V12 | **Cap de rodadas** | 30; luta que chega lá = "empate técnico" registrado | Trava de segurança para L3 (turtle total) não rodar para sempre; 30 = teto do boss final no canon, folga absurda para trash. |
| V13 | **KnowledgeKills** | 0 (1º encontro: variância ±30%, falha 5%) | Pior caso de imprevisibilidade do lado do jogador; se a política de mira funciona no encontro mais ruidoso, funciona nos calmos. |

---

## 6. O que a simulação NÃO responde (e só o playtest humano responde)

Honestidade de escopo. A simulação mede **equilíbrio e comportamento emergente do motor**.
Ela não mede:

1. **Diversão.** Nenhuma das 8 métricas é "foi legal". São proxies (duração, espalhamento,
   tensão); a validação final é o líder e o Gus jogando.
2. **Legibilidade percebida.** E5 mede repetição de alvo; não mede se o jogador ENTENDE por
   que foi alvo. Isso depende do telegraph na tela (ícone de intenção, canon não
   construído) e da frase de vendeta: pura apresentação, invisível ao headless.
3. **Sensação de justiça de uma criança de 11 anos.** "O inimigo veio em cima de mim de
   novo" pode ser tensão deliciosa ou choro na mesa: a diferença é apresentação, ritmo e
   volume, não estatística.
4. **O drama de ser focado.** A vendeta (F7) e o momento narrativo ficaram fora por
   desenho: o peso dramático se decide por dramaturgia.
5. **O jogador real.** O bot sub-ótimo não usa Scan, Gambito nem combos: um humano bom
   muda de estratégia NO MEIO da luta reagindo à mira, e essa dança
   (mira-reage-jogador-reage-mira) é exatamente o que queremos criar e exatamente o que o
   headless não simula.
6. **Chefes.** Boss é puzzle desenhado à mão (canon); a mira estatística é assunto de
   encontro comum.

O estudo decide **a física da mira** (os pesos, o sinal do F4). A **sensação** da mira se
valida depois, no playtest com o telegraph desenhado na tela.

---

## 7. Protocolo de execução (as cláusulas do líder, na ordem)

1. **Aprovação prévia:** este documento vai ao líder; lotes, braços, N e valores `//SIM`
   só rodam depois do aval dele (item a item se ele quiser).
2. **Harness:** ferramenta de simulação ao lado do
   `balance_harness.hpp` existente, rodando a `CombatStateMachine` real headless.
   Implementação por agente de engenharia (backend-engineer/gameplay_engineer), não inline.
3. **Seeds determinísticas:** seed base fixa + índice da luta. As MESMAS 200 seeds de um
   lote rodam sob os 6 braços (comparação pareada: cada braço enfrenta exatamente as
   mesmas lutas). Qualquer luta é re-rodável para inspeção golpe a golpe.
4. **Execução em background**, mostrando no terminal APENAS:
   `lote [x] de [7], simulação [n] de [1200]`.
5. **Entrega em duas camadas**, como pedido: primeiro os dados matematicamente (tabelas
   por lote × braço, com margens de erro), depois explicação simples de cada resultado.
6. **Depois dos dados:** raciocínio de C-level com opções e explicações simples mas
   extensas de vantagem/desvantagem de cada escolha, no escopo definido pelo líder: como
   o público-alvo se sentiria, o que cada escolha faz com os pillars e com a impressão
   que queremos causar. A decisão final é do líder.
7. **Escalonamento pré-declarado:** par de braços empatado em métrica decisória re-roda
   com 1.000 lutas (só o par, só o lote).

---

*Proposta escrita por Capitolino (CPO) em 2026-08-01. Não é canon. Não foi commitada por
decisão de processo (commit e push só com ordem do líder).*
