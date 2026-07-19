# Cartas: hardware, energia e pirataria — PROPOSTA DE NÚMEROS

**Status:** os 4 pontos estruturais (§8) foram **FECHADOS PELO LÍDER em 2026-07-18** (bateria escala por dificuldade; vírus homebrew 55% fixo; `urandom` pirata backfire = 1/3 exato, crédito Gus Dragon; desconto pirata×original escala por dificuldade). O restante do documento (números de fundo/derivação) segue **PROPOSTA `//PLAYTEST`** do `economy-designer`, preenchendo os `[calcular]` de [`cartas-hardware-pirataria-energia.md`](cartas-hardware-pirataria-energia.md). Todo número não marcado como FECHADO é **FRAMEWORK**: a PROPORÇÃO/lógica de derivação é o que se fixa agora; o valor exato pode mexer ±10-20% no playtest N=3, sem quebrar a estrutura.

**Método:** nenhum número foi inventado solto — cada um deriva de um número canônico já fechado (ver "Deriva de" em cada tabela). Onde não havia âncora direta, usei a escada de Fibonacci já empregada no projeto (`8, 13, 21, 34, 55, 89, 144`) como grade de valores — o easter egg #1 continua **velado**, nunca nomeado em texto de jogo.

**Cross-ref:** [`cartas-hardware-pirataria-energia.md`](cartas-hardware-pirataria-energia.md) (o sistema), [`economia.md`](economia.md) (crédito canônico), [`cartas-technomagik.md`](cartas-technomagik.md) §2.2-2.3 (ManaCost/Power), [`deck-mao-sistema.md`](deck-mao-sistema.md) (deck/mão), [`capacitor-item.md`](capacitor-item.md) (item de energia elétrica já desenhado, mesma família conceitual).

---

## 0. Resumo executivo (tabela única de consulta)

| # | Número | Valor proposto | Deriva de |
|---|---|---|---|
| 1a | **Capacidade de bateria — FECHADO PELO LÍDER: escala por dificuldade** (ver §1a e §8.1). **5ª classe `PirataEspecialFalso` fechada pelo `economy-designer` em 2026-07-19 (AMB-DADOS-01).** | Fácil 16/42/68/110/288 · Médio 8/21/34/55/144 · Difícil 4/10/17/27/72 · Hardcore 4/10/17/27/72 (Homebrew/Pirata comum/**Pirata especial falso**/Comum original/Especial) | escada Fibonacci (Médio = original recomendado; Fácil = ×2; Difícil/Hardcore = ÷2). PirataEspecialFalso = 34 no Médio, o degrau Fibonacci que já faltava entre 21 e 55 |
| 1a | Drain por uso ("recurso Y") | **= ManaCost da carta** (1/2/3/6), não escala por dificuldade | ManaCost canônico (cartas-technomagik §2.2-2.3) |
| 1b | Degradação por ciclo de recarga | **−13 p.p. de SoH por recarga** | escada Fibonacci |
| 1b | Piso de descarte (SoH mínimo) | **21% SoH** | escada Fibonacci |
| 1b | Preço de recarga (base) | **3 cr** | venda NPC de comum (economia.md §8c, 3-5cr) |
| 1b | Multiplicador de recarga (degradação) | **1,2×–2,0×** linear com SoH entregue | já fixado no doc-fonte |
| 1c | Troca de bateria IN-BATTLE | **2 AP** | Gambito-Reordenar (combat.md, 2 AP) |
| 2 | Memória — Comum original | **5 MR** | 1 MR por Token-fonte (3 Tokens) arredondado |
| 2 | Memória — Especial original | **21 MR** | escada Fibonacci |
| 2 | Memória — Homebrew (EPROM) | **13 kR** (teto) | tamanho de script = 5kR × ManaCost |
| 2 | Ciclos de regravação homebrew antes de queimar | **8 regravações** | escada Fibonacci |
| 2 | Tempo de upload homebrew | **1 beat narrativo** (gate, não timer) | precedente Hospital (economia.md §3.2) |
| 3 | Contaminação — Especial (original) | **0%** | canon fixo |
| 3 | Contaminação — Comum (original) | **1%** | escada Fibonacci |
| 3 | Contaminação — Pirata especial | **8%** | escada Fibonacci |
| 3 | Contaminação — Pirata comum | **21%** | escada Fibonacci |
| 3 | Contaminação — Homebrew | **55% FIXO — FECHADO PELO LÍDER, não escala por dificuldade** (§8.2) | escada Fibonacci |
| 3 | Propagação por cast (carta já infectada) | **13%** | escada Fibonacci |
| 4 | `urandom` original — pesos (fraco/médio/forte/especial) | **21 / 34 / 21 / 8** (inalterado) | escada Fibonacci |
| 4 | **`urandom` pirata — pesos — FECHADO PELO LÍDER: backfire = 1/3 exato** (§8.3, crédito **Gus Dragon**) | **7 / 2 / 1 / 0 / 5** (fraco/médio/forte/jackpot/backfire, base 15) = 46,7% / 13,3% / 6,7% / 0% / **33,3%** | reponderação exata pra backfire = 1/3 |
| 5 | Adware Sterling — tempo até o X | **5 segundos** | escada Fibonacci |
| 6 | Cura do Turing — sucesso / queima | **62% / 38%** | split φ já usado em economia.md §2.1/§3.3.2 |
| 7 | Carregador solar | **55 cr** | escada Fibonacci |
| 7 | Powerbank c/ visor LED | **34 cr** | escada Fibonacci |
| 7 | Carregador rápido | **21 cr** | escada Fibonacci |
| 7 | Voltímetro | **13 cr** (ou grátis, missão do Volta) | escada Fibonacci |
| 7 | Pirata comum (preço) | escala com o desconto por dificuldade (linha abaixo) | comum original 12-18cr (economia.md §8c) |
| 7 | **Desconto mercado negro — FECHADO PELO LÍDER: escala por dificuldade** (§8.4) | Fácil 30% · Médio 50% · Difícil 70% · Hardcore 70% | ver §7.3 — quanto mais difícil, maior o desconto (mais tentador) |

---

## 1. Bateria CR2032

### 1a. Capacidade e "recurso Y" (drain premium) — FECHADO PELO LÍDER (2026-07-18): ESCALA POR DIFICULDADE

**Modelo proposto (mantido):** a bateria não guarda "N usos" genéricos — guarda **unidades de carga**, e cada cast consome carga **igual ao `ManaCost` da carta** (1/2/3 para COMUM, ~6 para ESPECIAL, já fixados em `cartas-technomagik.md` §2.2-2.3). Isso implementa "recurso Y drena mais rápido" **sem inventar um eixo novo**: o "efeito premium" já É o ManaCost mais alto. **`ManaCost` não escala por dificuldade** (mantido intacto do canon combat); só a CAPACIDADE da bateria escala. Para as **Híbridas** (Faraday/Maxwell/Newton/von Neumann/John Dee, cartas-technomagik §2.3): a face **passiva** (ManaCost 0, sempre ligada) **não drena carga** — só a face **ativa**, quando disparada, consome as ~6 unidades.

**Decisão do líder (§8.1): a bateria NÃO é um valor único — escala por dificuldade**, usando as 3 réguas já mapeadas nas opções A/B/C originais (§8.1):

| Dificuldade | Perfil | Homebrew | Pirata comum | **Pirata especial (clone-falso)** | Comum original | Especial (selada) |
|---|---|---|---|---|---|---|
| **Fácil** | RELAXADA (opção B, ×2) | 16 | 42 | **68** | 110 | 288 |
| **Médio** | TENSA (opção A, baseline) | 8 | 21 | **34** | 55 | 144 |
| **Difícil** | APERTADA (opção C, ÷2) | 4 | 10 | **17** | 27 | 72 |
| **Hardcore** | APERTADA (igual ao Difícil) | 4 | 10 | **17** | 27 | 72 |

**Racional específico do `PirataEspecialFalso` (AMB-DADOS-01, fechado pelo `economy-designer` em 2026-07-19):** a hipótese de partida da spec era "mesma capacidade do Pirata comum" (só o disfarge muda, o hardware por baixo é igual). Contra-argumento aceito: a própria tabela de contaminação (§3, já fechada) já trata o clone-falso como hardware de **qualidade intermediária**, não igual ao pirata genérico — 8% de risco, entre o Comum original (1%) e o Pirata comum (21%), porque o vendedor que fabrica um clone convincente o bastante pra enganar o display/scanner do jogador precisa de componentes/blindagem melhores que o pirata de fundo de quintal. Aplicando a MESMA lógica à bateria: o clone-falso fica um degrau ACIMA do Pirata comum, não igual a ele. Isso cai de graça na escada Fibonacci já em uso — o Médio das outras 4 classes é 8/21/55/144, saltando o **34** que fica exatamente entre 21 e 55; nenhum número novo foi inventado, só o degrau que já estava faltando na sequência 8,13,21,**34**,55,89,144. Fácil/Difícil/Hardcore seguem a mesma regra ×2/÷2 (floor) das outras 4 linhas — 34 é par, então Difícil/Hardcore fecha exato em 17, sem arredondamento. Ordem monotônica preservada em toda dificuldade: Homebrew < Pirata comum < Pirata especial falso < Comum original < Especial.

**Racional:** a mesma escada Fibonacci (8/21/55/144) vira o eixo **Médio**; Fácil dobra (relaxa o gerenciamento pro público casual/kids), Difícil corta pela metade (aperta pro público que já busca desafio). **Hardcore usa o mesmo valor do Difícil** (não aperta ainda mais) — o Hardcore já pune por outros vetores (permadeath, save isolado, machine-bind); duplicar o aperto de bateria em cima disso seria punição composta desnecessária.

**Sink de crédito por dificuldade (recalculado):** mesma metodologia do sanity check original — ~180 casts por carta comum ativa ao longo da campanha inteira (ManaCost médio ~2), ~5 comuns ativos na mão, preço médio de recarga **~4,8cr** (meio da faixa 3,6-6cr, §1b, que NÃO escala por dificuldade).

| Dificuldade | Capacidade comum | Casts/carga (÷ManaCost médio 2) | Trocas por carta (÷180 casts) | Trocas totais (×5 comuns) | **Sink de crédito total (× ~4,8cr)** |
|---|---|---|---|---|---|
| Fácil | 110 | 55 | ~3,3 | ~16 | **~77 cr** |
| Médio | 55 | 27,5 | ~6,5 | ~33 | **~158 cr** |
| Difícil | 27 | 13,5 | ~13,3 | ~67 | **~322 cr** |
| Hardcore | 27 | 13,5 | ~13,3 | ~67 | **~322 cr** (mas correndo o risco do permadeath — o crédito gasto pode nunca "compensar" se a run acaba antes) |

O sink de bateria no Difícil/Hardcore é **~4,2× maior** que no Fácil — quase o mesmo fator (110÷27≈4,1) da própria razão de capacidades, como esperado (a matemática é linear por construção). Isso **soma-se** ao efeito já canônico de "a economia geral é mais magra no Difícil/Hardcore" (economia.md §2.1, Tusk 610→144cr, razão ~4,2× também) — os dois eixos reforçam a mesma mensagem: **dificuldade alta = cada crédito pesa mais, e a bateria cobra mais dele.** Nenhum dos dois eixo domina o outro (mesma ordem de grandeza), evitando que o Difícil vire punitivo-sozinho.

**Nota sobre a ESPECIAL em dificuldades altas:** capacidade/`ManaCost`(~6) = batalhas até esgotar: Fácil 48 batalhas · Médio 24 · Difícil/Hardcore **12 batalhas**. Numa campanha com ~30-40 encontros elite/boss, 12 batalhas é o primeiro perfil em que uma especial MUITO usada (ex.: a Gaiola de Faraday, primeira especial obtida, usada o jogo inteiro — canon §9 do doc-fonte) pode plausivelmente precisar de **1-2 trocas de bateria no Difícil/Hardcore**, reforçando organicamente a nota já canônica ("[a bateria selada] ainda exige cuidado no endgame") exatamente nos modos onde isso deveria doer mais.

### 1b. Degradação por ciclo de recarga + preço de recarga

**Não escala por dificuldade** (só a capacidade da §1a escala — decisão do líder foi específica sobre capacidade, degradação/preço de recarga seguem o mesmo valor em todo modo, mantendo o cálculo do sink acima consistente/comparável entre dificuldades).

- **Degradação:** cada recarga tira **13 pontos percentuais de State-of-Health (SoH)** da bateria (começa em 100%). Bateria abaixo de **21% SoH** é considerada morta — não seve mais pra recarregar (só vender/reciclar no ferro-velho, mesma regra de descarte-obrigatório do canon §5). Isso dá **~6 recargas de vida útil** por bateria física antes de trocar de vez (100 → 87 → 74 → 61 → 48 → 35 → 22 → abaixo do piso).
- **Preço de recarga na estação:** base **3 cr** (ancorado no preço de venda NPC de uma comum, economia.md §8c: "3-5 cr"), multiplicado por **1,2× a 2,0×** conforme o SoH da bateria que você entrega na troca (já fixado no doc-fonte, aqui só a fórmula):
  - `preço = 3 × (1,2 + 0,8 × (1 − SoH_normalizado))`, onde `SoH_normalizado = (SoH_entregue − 21%) / (100% − 21%)`, clamp [0,1].
  - Bateria quase nova entregue (SoH ~95%) → ~3,6cr. Bateria quase morta (SoH ~25%) → ~5,9cr.
- **Recarregar você mesmo** (fora da estação, com item Carregador — §7): grátis, mas custa **tempo de espera diegético** (já fixado no doc-fonte como trade-off; proponho: 1 beat narrativo de espera passiva, mesmo padrão da cura grátis do Hospital §3.2 — sem inventar um relógio novo).

### 1c. Troca IN-BATTLE

**2 AP, não escala por dificuldade** (`AP = 3 fixo/turno` também não escala, combat.md §5 — manter a troca em 2 AP em todo modo preserva a mesma "sensação tática" da ação, o aperto vem da CAPACIDADE §1a, não do custo da troca em si). Ancorado no `AP=3 fixo/turno` (combat.md §5) e na régua de custo já canônica: ações leves (Scan, Gambito-Prever, gadgets, usar Ampola) custam **1 AP**; reposicionamento tático pesado (Gambito-Reordenar) custa **2 AP**. Trocar bateria em combate é uma ação de emergência do mesmo peso que reordenar a fila — cara o bastante pra ser uma decisão real (deixa só 1 AP de sobra no turno), mas não consome o turno inteiro (3 AP), preservando a "válvula" que o doc-fonte pede.

---

## 2. Memória (Runa)

| Tipo | Capacidade | Deriva de |
|---|---|---|
| **Comum original** | **5 MR** | arredondamento de "1 MR por Token" (carta = 3 Tokens compilados, cartas-technomagik §2.2) + folga de firmware |
| **Especial original** | **21 MR** | ~4× a comum — script legendário pré-compilado, mais complexo |
| **Homebrew (EPROM vazia)** | **13 kR** (teto de capacidade) | ver fórmula abaixo |

**Por que o teto de homebrew é MUITO menor (kR, não MR) mesmo pra copiar a MESMA função:** a ROM original de fábrica é "bloatada" de propósito (diagnóstico, anti-tamper, certificação, redundância — o excesso burocrático-corporativo, sátira do Sterling/DRE já canônica em `project_axiologia_canonica`). Uma cópia homebrew magra tem só o payload funcional puro, sem nenhuma dessas camadas — daí caber em muito menos espaço, ao custo de ser instável (ligação direta com §3, risco de vírus).

- **Fórmula do tamanho do script comum (aplica a homebrew e a original por igual, unidade kR):** `tamanho ≈ 5 kR × ManaCost`. Mana 1 → 5kR · Mana 2 → 10kR · Mana 3 → 15kR.
- **Com teto de homebrew = 13kR:** cabe copiar Jab (5kR) e Golpe+status (10kR), **NÃO cabe** copiar Assinatura tier-3 (15kR > 13kR). **Isso é um gate de design deliberado, não um bug:** pirataria/homebrew serve pro básico; o efeito mais forte de cada família continua sendo prestígio de canal legítimo (loja/loot/craft). Reforça a lição Bastiat — "o barato serve pro simples, não pro que importa de verdade".
- **Ciclos de regravação antes de queimar (canon §3: "regravável, mas desgasta até queimar"):** proponho **8 regravações** por carta homebrew antes do EPROM morrer de vez (vira sucata, só ferro-velho). Eixo INDEPENDENTE da bateria (§1) — um é energia, outro é desgaste de escrita física.
- **Tempo de upload homebrew ("muito lento"):** proponho **gate narrativo, não timer de espera real** — reusa o padrão já canônico da cura grátis do Hospital (economia.md §3.2: "1 beat narrativo"). Você deixa a carta na bancada do mercado negro (§14 do doc-fonte) e ela fica pronta ao cruzar o próximo beat/nó de mapa. **Zero UI nova** (sem barra de progresso, sem contagem de segundos) — barato pra dev solo e mecanicamente honesto com "muito lento" sem irritar o jogador com espera real. *(Ver AMB-01 abaixo — alternativa de UX existe, é decisão leve.)*

---

## 3. Contaminação por vírus (%) — FECHADO PELO LÍDER (2026-07-18): 55% FIXO, NÃO ESCALA POR DIFICULDADE

**Decisão do líder (§8.2, opção A):** o risco de contaminação da homebrew fica em **55% fixo em todos os modos de dificuldade** (Fácil/Médio/Difícil/Hardcore) — ao contrário da bateria (§1a) e do desconto (§7.3), este número **não** ganhou um eixo de escala. O vírus morde igual em qualquer modo; é o desconto (§7.3) e a economia geral (mais magra no Difícil) que fazem a MESMA mordida doer mais forte quando o modo é mais difícil — reforça exatamente o racional do líder para o §8.4 ("como o vírus morde igual, a lição bate mais forte no hard").

Tabela respeitando a ordem canônica **especial(0) < comum < pirata especial < pirata comum ≪ homebrew** (doc-fonte §9):

| Tipo | Risco de contaminação (na aquisição/exposição) |
|---|---|
| Especial (original) | **0%** (canon fixo — só a arma dedicada da Sterling no clímax, nunca RNG) |
| Comum (original) | **1%** |
| Pirata especial (clone-falso) | **8%** |
| Pirata comum | **21%** |
| **Homebrew (EPROM)** | **55%** |

**Quando rola:** 1 vez, no momento de AQUISIÇÃO da carta (loot/compra/upload homebrew) — fica marcada como `IsInfected` oculto (canon: vírus é sempre oculto até o Turing diagnosticar ou o payload disparar). Escada 1/8/21/55 usa a mesma razão de vizinhos (~2,5-3×) do resto do documento.

**Propagação secundária (carta JÁ infectada, ao ser conjurada — doc-fonte §9, vetor "worm de deck"):** proponho **13%** de chance de contágio por cast de uma carta já sabidamente infectada, nas 3 direções descritas no doc-fonte (deck inimigo / ecossistema / próprio deck). Número secundário, menos crítico que a tabela principal — ajustável livremente no playtest sem afetar a ordem estrutural acima.

---

## 4. Carta `urandom` (pirata × original) — backfire da PIRATA FECHADO PELO LÍDER (2026-07-18): 1/3 EXATO

**Crédito:** a carta `urandom` em si é **ideia do Gus Dragon** (playtester, filho do líder — já registrado no doc-fonte §13). **O balanceamento do backfire pirata em exatamente 1/3 (33,3%) também é decisão do Gus Dragon**, registrada aqui pelo mesmo motivo que o doc-fonte credita as ideias dele: é conteúdo do playtester, não do economy-designer.

**Ponderação por peso relativo** (converte pra probabilidade dividindo pelo total). Estrutura de 4 faixas (fraco/médio/forte/jackpot-especial) para a ORIGINAL; a PIRATA troca o jackpot por um bucket de **backfire** (efeito ruim/no próprio caster):

### Original (prêmio da RunaDex — ponderação generosa, inalterada)

| Faixa | Peso | Probabilidade |
|---|---|---|
| Fraco (Jab-tier) | 21 | 25,0% |
| Médio (Golpe+status) | 34 | 40,5% |
| Forte (Assinatura/Control) | 21 | 25,0% |
| **Jackpot (efeito de uma ESPECIAL)** | 8 | **9,5%** |

### Pirata (mercado negro — caótica, azarada) — REPONDERADA pro backfire = 1/3 exato

| Faixa | Peso | Probabilidade |
|---|---|---|
| Fraco | 7 | 46,7% |
| Médio | 2 | 13,3% |
| Forte | 1 | 6,7% |
| Jackpot (efeito de ESPECIAL) | **0** | **0% — nunca puxa uma especial de verdade** |
| **Backfire** (efeito ruim, aplicado no próprio caster) | 5 | **33,3% (= 1/3 exato, 5/15)** |

**Como cheguei em 7/2/1/0/5 (total 15):** o pedido do líder é backfire **exatamente** `1/3`, não um arredondamento (diferente do 50% da minha proposta original, ou de um 25% mais suave). Um total divisível por 3 com backfire = total/3 resolve isso de forma exata: escolhi **total = 15, backfire = 5** (`5/15 = 1/3` cravado), e distribuí os 10 pesos restantes entre fraco/médio/forte preservando a MESMA forma da proposta original (fraco dominante, forte raro) com números pequenos e limpos (7/2/1). Resultado: a pirata fica **menos brutal que a minha proposta original** (33,3% de backfire em vez de 50%) mas segue **claramente mais arriscada que a original de RunaDex** (que tem 0% de backfire) — o meio-termo exato que o líder pediu, não a opção B "suave ~25%" nem a A "brutal 50%" das minhas alternativas.

**Racional (mantido):** a pirata NUNCA acerta o jackpot de especial (reforça "a cópia não é o original" — canon §2.3, clone-falso não conta como a especial de verdade) e tem **1 em cada 3 vezes** um resultado ruim pro próprio jogador. A original inverte completamente: nunca tem bucket de backfire, e o jackpot vira realista (quase 1 em 10) — condizente com ser o troféu de completar a RunaDex inteira.

---

## 5. Adware Sterling: timing

**5 segundos** de propaganda intransponível, então o **X** aparece (pequeno, canto superior direito, quase invisível — já descrito no doc-fonte), clica, efeito da carta dispara. 5 é Fibonacci e curto o bastante pra não roubar o tempo real do jogador (dev solo, respeito ao público 11+) mas longo o bastante pra ser genuinamente irritante — a piada É a irritação, sátira do adware real. **Refinamento opcional (não obrigatório agora):** depois da 3ª exposição na mesma sessão, o X já aparece desde o início (skip imediato) — reduz fadiga de repetição sem remover o incômodo na primeira vez. Fica registrado como ideia, não como escopo fechado.

---

## 6. Cura do Turing

**62% de sucesso / 38% de queima do chipset** (a carta é perdida/destruída). Escolhi reusar literalmente o split **φ (62%/38%)** que já é canon em `economia.md` §2.1 (crédito Tusk, razão ~62% entre dificuldades) e §3.3.2 (planos de quitação 62/50/38%) — não é número novo, é o MESMO padrão numérico do projeto reaproveitado num contexto novo (o binário sucesso/queima soma 100%, sem 3º resultado neutro: **é aposta real**, sem espaço pra "tentar de novo de graça" — evita grind degenerado de retry infinito).

---

## 7. Preços (itens de energia + curva pirata×original)

### 7.1 Itens de energia (novos)

| Item | Preço | Racional |
|---|---|---|
| **Carregador solar** (recarga passiva) | **55 cr** | maior investimento, payback em ~14 recargas evitadas (55 ÷ ~4cr médio) |
| **Powerbank c/ visor LED** | **34 cr** | conveniência+informação, sem economia direta de crédito |
| **Carregador rápido** | **21 cr** | só reduz o tempo de espera do autorrecarregamento |
| **Voltímetro** | **13 cr** (ou grátis via missão do Volta, já canônico) | ferramenta de diagnóstico simples, preço-de-entrada baixo |

Escada 13/21/34/55 = Fibonacci, todos dentro da faixa "pequeno investimento" já usada em `economia.md` §8c (comum 12-18cr) e §2 (baú 13-34cr) — nenhum desses itens compete de preço com craft de tier 2/3 (~150-400cr).

### 7.2 Pirata × original (preço direto) — desconto FECHADO PELO LÍDER (2026-07-18): ESCALA POR DIFICULDADE

**Decisão do líder (§8.4):** ao contrário da minha proposta original (50% flat em todo modo), o desconto **escala por dificuldade, na direção "mais difícil = MAIOR desconto"** — o oposto do que se faria "pra facilitar o modo fácil": aqui é o **modo difícil que fica mais tentador**, de propósito.

| Dificuldade | Desconto | Comum original | **Comum pirata** |
|---|---|---|---|
| Fácil | **30%** | 12-18 cr (canon, não escala) | **~8-13 cr** (70% do original) |
| Médio | **50%** | 12-18 cr | **6-9 cr** (50% do original) |
| Difícil | **70%** | 12-18 cr | **~4-5 cr** (30% do original) |
| Hardcore | **70%** (igual ao Difícil) | 12-18 cr | **~4-5 cr** |

### 7.3 A curva ao longo do arco (Bastiat) — DOIS EIXOS agora, ambos sem sistema novo de preço dinâmico

Com a decisão do líder, o arco Bastiat passa a ter **dois eixos independentes**, nenhum deles exigindo um sistema de preço dinâmico caro de manter (dev solo, escopo barato):

1. **Eixo INTER-dificuldade (novo, fixo por modo, decisão do líder §8.4):** o desconto-base do modo escolhido no início da campanha (30/50/70%, tabela acima) — reflete "em qual dificuldade o jogador está jogando" e fica constante a campanha inteira naquele modo.
2. **Eixo INTRA-campanha (mantido da proposta original, emergente, sem código novo):** dentro de QUALQUER modo, a renda do jogador sobe com o multiplicador de zona já canônico (`economia.md` §2: `8cr × mult(zona)`), então o mesmo desconto ABSOLUTO em crédito (ex.: ~4-5cr no Difícil) fica proporcionalmente cada vez menor frente à renda — a tentação se dissolve sozinha ao longo da campanha, em qualquer modo, de graça.

**Por que "mais difícil = desconto maior" reforça o Bastiat em vez de quebrá-lo (racional do líder, registrado):** no Difícil/Hardcore, três coisas acontecem ao MESMO TEMPO, todas apontando pra mesma lição:

- A economia geral já é mais magra (Tusk 610→144cr, economia.md §2.1) — o jogador tem menos crédito sobrando.
- O desconto pirata é o MAIOR de todos os modos (70%) — a tentação de comprar pirata é a mais forte justamente quando o jogador está mais apertado.
- O vírus da homebrew **morde igual em todo modo** (55% fixo, §3/§8.2) — **não afrouxa** pra compensar o aperto.

A combinação é deliberadamente dura: é precisamente no modo onde o jogador MAIS precisa economizar que a pirataria fica MAIS tentadora e o risco continua o MESMO — reforça o ensino Bastiat com mais força justamente pro público que já busca desafio (Difícil/Hardcore), sem punir o público casual do modo Fácil (desconto menor, 30%, junto com bateria relaxada §1a — o modo Fácil é coerentemente mais gentil em TODOS os eixos deste sistema, não só num).

---

## 8. PONTOS-CHAVE — DECISÃO DO LÍDER: **FECHADOS EM 2026-07-18**

Os 4 pontos abaixo mudam a SENSAÇÃO do sistema inteiro, não só um número. Abaixo: a decisão FINAL do líder primeiro, depois a tabela de opções original (mantida como registro do raciocínio/contraste).

### 8.1 Quão punitiva é a gestão de bateria — **DECISÃO FINAL: ESCALA POR DIFICULDADE** (nenhuma das 3 opções sozinha — as 3 combinadas, uma por modo)

O líder não escolheu uma opção única: escolheu as **3** ao mesmo tempo, uma por dificuldade — Fácil=B(relaxada), Médio=A(tensa, minha recomendação original), Difícil=C(apertada), Hardcore=C(igual ao Difícil). Números finais e sink recalculado por modo: **ver §1a**.

| Opção (registro) | Descrição | Efeito | Onde caiu |
|---|---|---|---|
| A — Tensa | capacidades 8/21/55/144, drain = ManaCost | ~30-35 trocas na campanha, ~150-175cr de sink total (era minha recomendação single-valor) | **virou o perfil Médio** |
| B — Relaxada | dobrar (16/42/110/288) | bateria quase nunca é gargalo | **virou o perfil Fácil** |
| C — Apertada | metade (4/10/27/72) | trocas ~2× mais frequentes | **virou o perfil Difícil e Hardcore** |

### 8.2 Quão perigoso é o vírus (contaminação da homebrew) — **DECISÃO FINAL: opção A, 55%, FIXO EM TODO MODO**

Diferente da bateria e do desconto, o líder confirmou a opção A **sem escalar por dificuldade** — o vírus morde igual em todo modo (ver racional cruzado em §7.3: é o desconto+economia magra que fazem a MESMA mordida doer mais no Difícil, não o vírus ficando mais perigoso).

| Opção (registro) | Descrição | Efeito | Decisão |
|---|---|---|---|
| **A — Alta, 55% — ESCOLHIDA** | homebrew é genuinamente arriscada; ~1 em cada 2 cartas piratas caseiras vem contaminada | pirataria = escolha de risco real, não farol de conveniência disfarçado | fixo, todo modo |
| B — Moderada, ~35% | mais permissivo | não escolhida | — |
| C — Baixa, ~20% | esvazia a lição Bastiat | não escolhida | — |

### 8.3 O custo da `urandom` pirata (quão azarada) — **DECISÃO FINAL: backfire = 1/3 exato (33,3%), nenhuma das 3 opções listadas — decisão própria do Gus Dragon**

O líder (repassando a decisão do **Gus Dragon**, playtester) não escolheu A (50%) nem B (~25%) nem C (sem backfire) — pediu um valor **exato e específico, 1 em 3**, entre a minha opção A e B. Pesos recalculados exatos em **§4**: fraco 7 / médio 2 / forte 1 / jackpot 0 / backfire 5 (base 15).

| Opção (registro) | Descrição | Decisão |
|---|---|---|
| A — Brutal, 50% backfire | minha recomendação original | não escolhida (perto, mas não é) |
| B — Suave, ~25% backfire | mais jogável | não escolhida |
| C — Sem backfire dedicado | só zera jackpot | não escolhida |
| **Escolhida: 1/3 exato (33,3%)** | meio-termo exato pedido pelo Gus Dragon | **ver §4 pra reponderação completa** |

### 8.4 A relação pirata×original de preço (quão grande é o desconto) — **DECISÃO FINAL: ESCALA POR DIFICULDADE, mais difícil = MAIOR desconto**

O líder não escolheu A/B/C isolado — escolheu as 3 réguas como uma escada por dificuldade: Fácil=B(30%), Médio=A(50%, minha recomendação original), Difícil=C(70%), Hardcore=C(igual ao Difícil). Racional do líder, registrado: "como o vírus morde igual, a lição bate mais forte no hard" (ver desenvolvimento completo em **§7.3**).

| Opção (registro) | Descrição | Efeito | Onde caiu |
|---|---|---|---|
| A — 50% flat | era minha recomendação single-valor | mais barato de implementar | **virou o perfil Médio** |
| B — 30% (modesto) | pirata quase não compensa desde o início | **virou o perfil Fácil** |
| C — 70% (agressivo) | pirata MUITO tentadora | **virou o perfil Difícil e Hardcore** |

---

## Ambiguidades registradas

- **AMB-01 (UX do upload homebrew):** proposto gate narrativo (1 beat, zero timer real) como default barato. Alternativa: barra de progresso em tempo real (~34 segundos, Fibonacci) na tela da bancada, mais "sentida" mas exige UI nova. Recomendo o gate narrativo; líder pode preferir o timer se quiser mais peso sensorial na cena de pirataria.
- **AMB-02 (drain de "recurso Y"):** interpretei como = ManaCost da carta (reuso total do número já canônico). Alternativa não escolhida: um multiplicador extra fixo só pra cartas de Controle/DoT (achei redundante — o ManaCost já capta isso, ManaCost 2-3 já é o Control/DoT).

---

**Última revisão:** 2026-07-18 — **rodada 2: os 4 pontos estruturais do §8 fechados pelo líder** (bateria e desconto pirata×original agora escalam por dificuldade; vírus homebrew confirmado 55% fixo; `urandom` pirata reponderada pro backfire = 1/3 exato, decisão/crédito do Gus Dragon). Revisão anterior (rodada 1, mesma data): proposta inicial completa do `economy-designer`, preenchendo todos os `[calcular]` de `cartas-hardware-pirataria-energia.md`. **Próximo passo:** canonização formal — refletir estes números de volta no doc-fonte (`cartas-hardware-pirataria-energia.md`) e em `cartas-technomagik.md` quando o líder confirmar que não há mais ajuste pendente.
