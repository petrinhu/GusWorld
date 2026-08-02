# PROPOSTA: comedimento × periculosidade, mapeamento de mira por tier e auditoria de coerência

> **Status: PROPOSTA, não canon.** A doutrina em si (economia.md §0) já foi aprovada pelo líder em
> 2026-08-01. Tudo neste documento (tabela de tiers, achados de auditoria, seeds narrativos) é
> PROPOSTA do `economy-designer`, pendente de aprovação item a item, como manda a regra da casa.
>
> **Autor:** economy-designer, 2026-08-01.
> **Cross-ref:** `economia.md` §0 (a doutrina), `proposta-mira-inimiga.md` (fatores F1-F8, cartas
> honeypot/firewall), `analise-mira-resultados.md` (o estudo MIRA-SIM, 10.080.000 lutas, autor
> Capitolino/CPO), `cartas-hardware-pirataria-energia.md` e `cartas-numeros-proposta.md` (bateria,
> pirataria, arco Bastiat), `combat.md` §12-§13 (tiers de inimigo: Trash / Elite / Mini-boss /
> Boss / Boss final), `pillars.md` Pillar 1, Pillar 2 e Pillar 4.

---

> **Atualização 2026-08-01 (segunda rodada):** o líder decidiu as quatro questões em aberto desta
> proposta. (1) confirmou a leitura da Opção F para "minion burro" (seção 1 abaixo vira registro
> histórico do contra-argumento, não pergunta aberta); (2) escolheu vantagem NÃO financeira por
> poupança, sem juros a favor, e recusou explicitamente criar um banco dentro do jogo (seção 6);
> (3) quer memória financeira acumulada ao longo da campanha inteira, pesando mais no fim (seção
> 7); (4) quer teto no Firewall, na mesma lógica do teto de 10% do honeypot (seção 8). As seções 6,
> 7 e 8 são PROPOSTA de mecanismo (o líder decidiu O QUÊ, o economy-designer propõe COMO); nada
> delas vira canon sem aprovação item a item.
>
> **Atualização 2026-08-01 (terceira rodada):** três novas decisões, mais uma correção de leitura
> que veio do team-lead, não de mim (registrada por transparência, porque o mecanismo dela já
> estava certo desde a primeira versão, só o resumo verbal é que achatou). (a) o gatilho do
> contador de reputação é e sempre foi **contrair dívida**, nunca **precisar de cura**: seção 6.1
> ganhou um parágrafo explícito sobre essa diferença, porque é o coração da doutrina e é fácil de
> achatar num resumo. (b) os três patamares têm nome definitivo do líder: **prudente**,
> **ponderado**, **imprudente** (localizado **VidaLoka** em pt-br e **YOLO** em en-intl), não
> Bronze/Prata/Ouro (seção 6.2 atualizada). (c) o reconhecimento da memória financeira (seção 7)
> vira uma escada de vários degraus ao longo da campanha inteira, não dois momentos isolados
> (seção 7 redesenhada). (d) o teto do Firewall, seção 8, está **APROVADO** na forma proposta.

## 1. Contra-argumento necessário antes da tabela: "burro" não pode significar "quebrado"

> **RESOLVIDO pelo líder em 2026-08-01: confirmada a leitura da Opção F.** "Minion burro" não é a
> mira de hoje (Opção A). O contra-argumento abaixo fica como registro do raciocínio, não como
> pergunta em aberto.

O líder pediu que os minions comuns usem "opções mais burras", inclusive para o jogador ganhar
mais e ficar feliz, sem perder o desafio. Antes de traduzir isso numa tabela, preciso registrar um
ponto de atrito real entre a leitura mais literal desse pedido e os dados que já existem.

Das seis políticas medidas no estudo MIRA-SIM, a mais "burra" na superfície é a **Opção A** (mira
sempre o primeiro da lista, o comportamento de hoje). Só que os dados do próprio estudo reprovam
a Opção A sem apelação, e por dois motivos que não têm nada a ver com dificuldade:

1. **Ela trava o jogo, não só facilita.** Um time que só defende produz **100% de empates
   técnicos** (30 rodadas, zero queda, zero dano em qualquer direção) contra a Opção A. Isso não é
   "desafio baixo", é o jogo não terminar. Foi literalmente a queixa que abriu este estudo inteiro
   (o líder ficou 30 rodadas "só defendendo" e nada acontecia).
2. **Ela é decoreba, não burrice simpática.** 92% de repetição de alvo é reconhecida pela criança
   de 11 anos já na segunda luta (Pillar 1: "vencer lendo o sistema" não tem o que ler ali, é
   sempre a mesma leitura) e pelo adulto nostálgico como "o inimigo burro de RPG de 1988", no mau
   sentido.

Ou seja: **a Opção A não é "o inimigo mais fácil", é o inimigo QUEBRADO.** Usá-la como o degrau
mais baixo da escada de periculosidade recriaria exatamente o bug que motivou o estudo, só que
agora com aval de "isso é doutrina". Isso fura o Pillar 1 (não há sistema pra ler) e, pela lei do
próprio estudo, cria a trava-empate que nenhuma quantidade de "o jogador ganha mais e fica feliz"
compensa (o jogador não ganha: ele trava).

**Minha leitura do pedido do líder, que seguirá adiante nesta proposta:** "burro" para o minion
comum deve mapear para a **Opção F** (ponderada, mas sem o fator explícito de evitar quem
defende): o inimigo já reage a quem causa dano e a quem está ferido (F1/F2), então tem UM
algoritmo legível e não trava contra o turtle, mas ainda ignora completamente a nuance "quem está
de escudo levantado", o que dá a leitura de "burro" sem reintroduzir o bug. Nos números do
estudo, a Opção F entrega vitória de 82,6% (alta, "jogador feliz"), Gus cai em 17,4% (ainda
arriscado, "sem perder desafio"), e o turtle morre (só 2,9% bate no teto de rodadas, contra 100%
da Opção A).

Se o líder quis dizer literalmente a Opção A (mira fixa) para os minions mais fracos, preciso
sinalizar que os dados dizem que isso reintroduz o bug de origem. Meu contra-argumento fica
registrado aqui; a escolha final é do líder via aprovação da tabela abaixo.

---

## 2. Tabela de escalonamento por periculosidade (proposta, aprovar linha a linha)

Tiers conforme já canônico em `combat.md` §12-§13 (`ScriptedBrain` para Trash; `UtilityBrain`
para Elite, Mini-boss e Boss, com ruído Perlin exclusivo do Patch-Zero).

| Tier | Política de mira proposta | F4 (evitar/atacar defensor) | F7 (vendeta declarada) | F8 (fraqueza elemental) | Por que essa sensação serve à doutrina |
|---|---|---|---|---|---|
| **Trash comum** (primeiras zonas) | **Opção F** (ponderada por F1/F2/F3, sem F4 explícito) | implícito só via F1 (quem defende não causa dano, perde peso de revide) | não | não | primeiro contato com o sistema: o inimigo já reage ao jogo do jogador (Pillar 1), sem punir quem ainda está aprendendo a guarda. Alta taxa de vitória, zero decoreba, zero travamento. |
| **Trash avançado** (mid-game, quando honeypot/firewall já existem na mão) | **Opção C** (F4 suave) | suave (metade do peso normal) | ocasional, se a missão pedir | não | o inimigo passa a "respeitar a guarda sem temê-la". É o primeiro degrau em que defender vira ferramenta tática, não parede eterna: começa a treinar o jogador pro tipo de decisão que o mestre final vai cobrar de verdade. |
| **Elite** | **Opção C**, com `UtilityBrain` pontuando por utilidade (não roteiro fixo) | suave | ocasional | **não** (decidido pelo líder, 2026-08-01; ver pergunta 4 na seção 5) | ainda legível (Pillar 1), mas reage mais rápido que o Trash porque escolhe por heurística, não por script. Primeiro sabor de "adversário que pensa", sem ainda mirar fraqueza: esse degrau fica reservado pro Mini-boss, pra escada de esperteza durar até o confronto final. |
| **Mini-boss** | **Opção D** (F4 forte), SEMPRE com os contrapesos do estudo (firewall acessível e barato, statlines revisados) | forte | se a cutscene pedir | sim | aqui é onde o "predador cirúrgico" que o CPO vetou como padrão de trash vira apropriado: luta rara, telegrafada, com o jogador já tendo acesso à ferramenta que neutraliza o pior do F4 forte (o firewall). É o arquétipo "Caçador" que o próprio CPO recomendou guardar para um inimigo especial, não descartar. |
| **Boss (não-final)** | **Opção D** + F8 sempre ativo | forte | conforme roteiro da cena | sim | o chefe mira a fraqueza elemental do jogador e pune preparo ruim (build descuidado, sem componente certo, sem carta boa). Telegraph de intenção obrigatório na tela (já canon em `battle-screen.md`), porque um chefe cruel sem aviso é injusto, não desafiador. |
| **Boss final** (Sterling / Patch-Zero) | **Opção D** + F7 (vendeta declarada, M1 já aprovado em `proposta-mira-inimiga.md` §3) + F8; Patch-Zero mantém `is_chaotic` exclusivo, fora desta escada | forte | sim (M1: Sterling declara Gus) | sim | é o ponto que o líder descreveu de propósito: "vai depender de uma economia inteligente do jogador pra chegar lá e ter uma vitória menos dolorosa". O jogador que guardou crédito, cuidou de bateria, montou deck com cartas boas e não chegou zerado no craft sofre menos aqui. O que esbanjou ao longo do jogo paga o preço exatamente nesta luta, que é o clímax da doutrina inteira. |

**O que esta tabela NÃO decide sozinha** (fica com playtest, per `analise-mira-resultados.md` §4):
os pesos exatos de F1-F8, a duração das cartas honeypot/firewall, e se a transição
Trash-comum→Trash-avançado deve ser por zona, por capítulo ou por posse de carta. Tudo isso entra
como `//PLAYTEST`, igual ao resto do balanceamento do motor.

**Dependência dura, não opcional:** a linha do Mini-boss e das duas linhas de Boss só é aprovável
se o pacote de contrapesos do estudo (`analise-mira-resultados.md` §3, Opção C) estiver de pé:
firewall acessível e barato cedo, statlines revisados, telegraph na tela. Sem isso, subir F4 para
"forte" nesses tiers fura o Pillar 4 ("ameaçado, nunca massacrado") do mesmo jeito que a Opção D
crua furou no estudo (Gus cai em 46,3% das lutas de trash com parede).

---

## 3. Auditoria de coerência: onde a economia de hoje tensiona ou contradiz a doutrina

Varredura de `economia.md` e `cartas-hardware-pirataria-energia.md` / `cartas-numeros-proposta.md`
contra a régua "o comedido sai melhor no final". A maior parte do sistema já está alinhada (ver
§3.4); os dois pontos abaixo são onde a resposta não é um SIM limpo.

### 3.1 Achado principal: não existe retorno POSITIVO por poupar, só ausência de punição

O exemplo do próprio líder (o velho que anda despreocupado porque "minha poupança garantiu isso")
descreve poupança como algo que dá PROTEÇÃO/PODER ativo no fim de jogo, não só a ausência de
dívida. Hoje, em `economia.md`, "juros" só aparece num lugar: §3.3.2, e ali eles SOBEM o que o
esbanjador deve (punição). Não existe o espelho: nenhum crédito guardado rende nada, nenhuma
poupança cresce. Um jogador que guarda 500 créditos por 10 zonas chega no craft com exatamente
500 créditos, nem mais nem menos; o único "prêmio" por ter guardado é PODER PAGAR algo caro
depois (T3 upgrade ~400cr, componente raro, firewall), o que já é real, mas é um prêmio passivo
("você não gastou, então ainda tem"), não um prêmio ativo ("guardar rendeu algo a mais").

Isso não é necessariamente uma contradição (é comum em RPG que dinheiro parado não renda), mas é
uma lacuna real frente à ambição do exemplo que o próprio líder escreveu. Duas leituras possíveis,
e a escolha entre elas é dele (pergunta 1 na seção 4):

- **Leitura A (sem sistema novo):** os sinks grandes de fim de jogo (T3, firewall, componente
  raro, rede grátis do Tusk) JÁ são o prêmio da poupança, e o que falta é só reforçar isso na
  narrativa (seção 4 abaixo), não no código. Mais barato, doutrina cumprida por composição do que
  já existe.
- **Leitura B (sistema novo, pequeno):** um "juros a favor" simples, capado e não-composto
  (espelho exato da engenharia anti-bola-de-neve de §3.3.2, só com o sinal trocado), que rende
  pouco mas rende, sobre crédito parado por zona cruzada. Mais fiel à imagem literal do velho
  ("minha poupança garantiu isso" soa a algo que cresce), mas é escopo novo.

### 3.2 Achado: o cap anti-bola-de-neve desacopla episódios; não há memória financeira ao longo da campanha

A dívida do Hospital (§3.3.2/§3.3.3) é, com razão, capada e anti-composta: nenhuma dívida singular
pode virar espiral (~72cr no pior caso, quitável em ~17 encontros). Isso é correto do ponto de
vista ético (anti-dark-pattern, protege a criança de 11 anos de um loop de punição sem saída) e eu
não recomendo tocar nisso. Mas tem uma consequência que tensiona a doutrina: **um jogador que
esbanja e entra em dívida repetidas vezes ao longo do jogo inteiro, mas sempre quita cada episódio
antes do próximo, chega no boss final financeiramente parecido com quem nunca esbanjou.** Não há
nenhum contador cumulativo ("quantas vezes você já entrou em dívida na campanha inteira") que
torne o boss final estruturalmente mais difícil para o esbanjador crônico especificamente por
CAUSA do padrão repetido, além do que ele já perde no momento (crédito, XP, loot daquele
encontro).

Isso é exatamente o ponto que a fala do líder aponta ("isso se inverte proporcionalmente até
chegar o mestre final"): hoje, cada episódio de esbanjamento é isolado e esquecido assim que
quitado; não existe cumulativo. Não estou recomendando revogar a trava anti-bola-de-neve (ela
protege contra dark pattern real), só registrando que, tal como está, ela também apaga a memória
financeira de longo prazo que a doutrina pede. Pergunta 2 na seção 4 é sobre isso.

### 3.3 Guardrail preventivo: os números do Firewall (ainda não fechados) podem anular a doutrina antes de nascer

`proposta-mira-inimiga.md` recomenda (via CPO) que o Firewall seja "acessível e barato cedo" como
contrapeso ao F4 forte. Isso está certo para resolver o problema imediato (queda do Gus), mas se
os números de bateria/mana/duração/turnos do Firewall saírem generosos demais, ele vira a mesma
armadilha que o honeypot já tem teto de 10% pra evitar: um botão "sempre ligado, sem custo real",
que mata a tensão "poupo a carta pra depois ou uso agora" que é o coração da doutrina aplicada a
combate.

> **RESOLVIDO pelo líder em 2026-08-01: o Firewall recebe teto, na mesma lógica do honeypot.** A
> forma proposta do teto está desenvolvida na seção 8.

### 3.4 Pontos onde a doutrina JÁ é cumprida hoje (para registro, não é tudo problema)

- **Bateria degrada com uso** (`cartas-hardware-pirataria-energia.md` §5): quem usa mais paga mais
  em recarga e reposição; quem cuida (carregador solar, recarga na cidade) gasta menos ao longo do
  jogo. Alinhado.
- **Arco Bastiat pirata × original** (mesmo doc §12, números fechados em `cartas-numeros-proposta.md`
  §7.3): comprar barato agora custa caro depois (vírus, instabilidade), e o desconto pirata é
  justamente MAIOR no modo Difícil, onde a tentação de esbanjar (comprar errado por pressa) dói
  mais. Alinhado, e já é o precedente mais forte no jogo pra doutrina de hoje.
- **Pedágio de bancada de terceiro vs. rede grátis do Tusk** (economia.md §7.9): quem tem paciência
  de completar a missão-capstone nunca mais paga pedágio; quem não tem, paga ~130-195cr ao longo
  do jogo inteiro. Alinhado, e é literalmente "poupar esforço agora paga menos depois".
  - **Nota de leitura reversa (registro honesto):** dependendo de como a campanha se desenrola,
    esse exemplo também pode ser lido como "quem se apressa em completar a missão principal ganha
    o desconto mais cedo", o que é o oposto de "quem espera/poupa". Não vejo isso como
    contradição real (completar a capstone é conquista, não impulsividade), só registro que o
    enquadramento narrativo (seção 4) deve deixar claro que o prêmio é por MÉRITO/paciência
    daquela missão, não por pressa.
- **`urandom` pirata × original** (mesmo doc §13): a pirata (achado rápido, de fonte suja, no
  mercado negro) é literalmente pior em expectativa (33,3% de chance de sair pela culatra); a
  original (prêmio de completar a RunaDex inteira, exige tempo e disciplina de colecionar) nunca
  tem esse risco. Alinhado.
- **Hospital, dívida com juros** (economia.md §3.3.2): mesmo capada, a dívida SEMPRE custa mais que
  pagar à vista; o esbanjador que nunca guarda reserva de emergência paga juros que o comedido
  nunca paga. Alinhado (e é a base do achado 3.2 acima, que só aponta o limite desse alinhamento,
  não a ausência dele).

---

## 4. Seeds de entrega narrativa (o QUE dizer e ONDE, não a prosa final)

A prosa final é do `narrative-writer`, depois que o líder aprovar. Aqui só o gancho (quem fala,
onde, o que a cena precisa comunicar) e uma nota de risco (se a ideia arrisca virar sermão).

Os dois exemplos abaixo são do próprio líder e ficam registrados como seed 1 e seed 2, âncora de
tom para todas as outras:

1. **Ferro-velho, Periferia Industrial (setting 4).** Um velho trabalhando na sucata porque não
   poupou para a idade avançada. Contraste implícito com a seed 2, sem precisar dizer isso em voz
   alta. Risco: baixo, se a cena não parar pra explicar o porquê (mostra, não conta).
2. **Estrada ou cidade, encontro casual.** Um velho andando sem medo, que credita a tranquilidade
   à poupança de recursos, cartas e cuidado com bateria ao longo da vida. Frase-modelo já escrita
   pelo líder (ver mensagem original); tom "quase desinteressado", nunca didático.

Seeds adicionais propostos:

3. **Hospital (a própria tela de dívida, §3.3.2).** Um paciente idoso internado, em dívida, que
   comenta (sem se dirigir ao jogador) que "devia ter guardado mais antes de ficar velho". Amarra
   DIRETO com a mecânica que o jogador está usando naquele instante, sem quebrar a quarta parede.
   Risco: médio, precisa ser um comentário de ambiente (overheard), não uma fala dirigida a Gus,
   senão vira sermão.
4. **Bazar noturno / mercado negro (setting Periferia, `cartas-hardware-pirataria-energia.md`
   §14).** Um vendedor veterano com reputação impecável, construída ao longo de décadas de
   negócio honesto, contrastado com um vendedor jovem desesperado (reputação queimada, precisa
   vender rápido e barato porque ninguém mais confia nele). Nenhum dos dois precisa mencionar
   poupança; a CONDIÇÃO deles já ensina. Risco: baixo, é mostrar-não-contar puro.
5. **Volta, ghost-tutor de energia (§5 do mesmo doc).** Como ele já é o tutor diegético de
   bateria/degradação/recarga, uma fala dele sobre cuidar de energia ao longo do tempo cai natural
   dentro do que ele já ensina mecanicamente. Risco: baixo, mas cuidado para não duplicar o
   tutorial de bateria com um sermão de poupança por cima; a lição deve vir JUNTO da explicação
   mecânica, não depois dela.
6. **Bancada de craft (Forja de Firmware / Bancada de Compilação, §7.1).** Um artesão mestre que
   guardou componentes raros por anos e agora forja upgrades de tier 3 sem esforço, contrastado
   com um aprendiz que "queimou" componentes em crafts vistosos de tier 1 e não tem nada guardado
   para o upgrade grande. Risco: baixo/médio, funciona melhor como observação de um NPC sobre o
   OUTRO NPC (o aprendiz), não como conselho direto ao jogador.
7. **RunaDex (índice de coleção, `cartas-hardware-pirataria-energia.md` §13).** Um colecionador
   veterano que levou o jogo inteiro pra completar o índice e ganhou a `urandom` original;
   contrastado com um NPC que vende duplicatas por crédito rápido e nunca completa nada. Risco:
   baixo, é sistêmico (o próprio jogo já entrega essa lição via loot), a fala só reforça.
8. **Zona do Silêncio (setting 6, Linda "Siren").** Um eremita isolado sobrevivendo sozinho há
   anos com bateria racionada com cuidado. Encaixa bem no tom desolado do setting sem precisar de
   diálogo longo; pode ser até ambientação silenciosa (baterias empilhadas, organizadas,
   etiquetadas) em vez de fala. Risco: baixo.
9. **NÃO recomendado: reflexão pós-luta de um boss ou mini-boss derrotado.** Cogitei e descartei:
   um inimigo comentando "você venceu porque não desperdiçou cartas" soa a tutorial disfarçado
   logo depois do combate, no pior momento possível (adrenalina da vitória, não o momento certo
   pra reflexão). Risco: alto, registra aqui só para não ser reproposto sem essa ressalva.
10. **Companion específico (ex.: Bento "Requiem", relojoaria mecânica de latão, peças
    conservadas por gerações).** Encaixe temático óbvio (hardware dele já É sobre conservação
    mecânica), mas caracterização de companion é território sensível (`feedback_personagens_erram`
    / `feedback_nada_canoniza_sem_aprovacao`): não proponho texto, só sinalizo que a família dele
    seria um lugar coerente SE o líder e o `narrative-writer` quiserem desenvolver, e que isso é
    decisão narrativa, não econômica.

---

## 5. Perguntas (histórico: as sete já foram decididas pelo líder em 2026-08-01)

1. ~~Leitura de "minions mais burros"~~ **DECIDIDO: Opção F, não Opção A** (seção 1).
2. ~~Retorno sobre poupança~~ **DECIDIDO: vantagem não financeira (acesso, preço, confiança),
   SEM juros a favor, sem banco no jogo** (desenho na seção 6).
3. ~~Memória financeira cumulativa~~ **DECIDIDO: sim, a campanha lembra, pesando mais no fim**
   (desenho na seção 7).
4. ~~Onde começa "tier alto" para F8~~ **DECIDIDO: só do Mini-boss para cima; o Elite fica de
   fora.** Racional do líder: o Elite já assusta por reagir melhor (F1/F2/F3 + F4 suave); guardar
   a fraqueza elemental para o Mini-boss cria mais um degrau na escada de esperteza, fazendo a
   progressão durar mais até o confronto final. Se o Elite já explorasse fraqueza, o jogo queimaria
   esse degrau cedo demais e sobraria menos espaço para o Mini-boss surpreender. Tabela da seção 2
   atualizada.
5. ~~Rótulo dos patamares~~ **DECIDIDO: `prudente` / `ponderado` / `imprudente` (VidaLoka em
   pt-br, YOLO em en-intl), rótulo próprio, não reaproveita Bronze/Prata/Ouro** (seção 6.2).
6. ~~Quantos momentos de reconhecimento~~ **DECIDIDO: mais gradual que dois momentos, escada de
   seis degraus ao longo da campanha inteira** (seção 7.2).
7. ~~Teto do Firewall~~ **APROVADO na forma proposta (custo de bateria real mais duração curta)**
   (seção 8).

---

## 6. Vantagem não financeira por poupança (mecanismo, decisão do líder já tomada: "o quê";
proposta abaixo é "o como")

**Regra de ouro do próprio pedido:** não pode ser saldo instantâneo (senão o jogador junta crédito
na véspera do gatilho e aprende o oposto do que queremos ensinar). Precisa medir **constância**.

### 6.1 Três desenhos possíveis

**Desenho 1, recomendado: contador de "zonas limpas", cumulativo, nunca reseta.**

> **Parágrafo obrigatório, para que ninguém mais resuma isso errado (aconteceu uma vez):** o
> gatilho **NÃO é "foi ao Hospital"**. Ir ao Hospital é rotina normal do jogo, todo jogador vai. O
> gatilho é especificamente **contrair dívida (§3.3.2) por não ter crédito guardado na hora**.
> Um jogador que apanha muito, precisa de cura completa, e PAGA à vista porque guardou crédito é
> exatamente o comportamento prudente que a doutrina quer premiar, mesmo tendo ido ao Hospital
> tantas vezes quanto qualquer outro. Só quem chega **sem nada** e precisa da dívida a crédito é
> que conta contra o contador. A distinção é entre "apanhou muito" (não é imprudência, é jogo
> difícil) e "não guardou o suficiente pra se cuidar quando precisou" (é a imprudência que a
> doutrina mede). O sistema abaixo já foi desenhado assim desde a primeira versão; este parágrafo
> só deixa isso impossível de achatar num resumo.

A cada zona cruzada (o MESMO beat narrativo que já dispara a cura grátis, `economia.md` §3.2: zero
infraestrutura nova de gatilho), o jogo verifica um único fato binário e já rastreado hoje: **o
jogador entrou no fluxo de dívida do Hospital (§3.3.2) durante aquela zona?** Se não (mesmo que
tenha se curado várias vezes, desde que sempre à vista), a zona conta como "zona prudente" e um
contador cumulativo sobe +1, pra sempre (nunca desce, nunca reseta). Se sim (contraiu dívida ao
menos uma vez naquela zona), a zona não soma, mas também não subtrai nada da conquista anterior.

- **Por que não pode ser gamed por saldo de véspera:** o gatilho não é "quanto você tem agora", é
  "você precisou de dívida em algum momento desta zona inteira". Impossível fingir isso guardando
  dinheiro no minuto antes da fronteira; ou você precisou de dívida durante a zona, ou não
  precisou, e isso já aconteceu antes do jogador saber que está sendo medido.
- **Por que é barato:** reaproveita um evento que o motor já registra (entrada em dívida, §3.3.2) e
  um beat que o motor já dispara (fronteira de zona, §3.2). Zero sistema de amostragem de saldo,
  zero UI nova além de uma linha no Diário do Gus (§6.3).
- **Prós:** não pune quem começou esbanjando (monotônico: um erro cedo não apaga o progresso
  depois); barato; não pode ser gamed; legível ("não precisei de dívida" é um fato que o próprio
  jogador sabe se conquistou ou não, sem precisar consultar HUD nenhuma).
- **Contras:** é grosso (zona inteira conta como limpa ou não, sem gradação dentro da zona); um
  jogador que teve azar de combate e caiu em safe mode (§3.3.1, grátis) sem nunca ter usado a
  dívida a crédito NÃO deveria ser penalizado por isso (safe mode já é net-negativo por si só, não
  é esbanjo); por isso o gatilho negativo do contador é **estritamente** "entrou no fluxo de dívida
  a crédito (§3.3.2)", nunca "sofreu wipe" isolado (que pode ser azar puro de combate, não
  administração financeira).

**Desenho 2: saldo-mínimo sustentado por zona (rolling minimum).**

Em vez de um fato binário, o motor guardaria o MENOR saldo que o jogador teve em qualquer instante
dentro da zona (não o saldo no fim), comparado a um piso proporcional ao custo típico daquela zona
(ex.: o custo de uma cura completa da party). Se o mínimo nunca caiu abaixo do piso, zona conta.

- **Prós:** mede "manteve uma reserva de verdade" de forma mais fiel à imagem literal de poupança.
- **Contras:** exige rastrear um mínimo contínuo (mais caro de implementar que um evento discreto
  já existente); mais difícil de comunicar ("por que essa zona não contou?" não tem uma resposta
  óbvia de uma frase, o que fere a exigência de legibilidade); risco de opacidade (Pillar 2).

**Desenho 3: trilha contínua tipo Knowledge (sobe/desce suave por decisão).**

Um placar contínuo que sobe um pouco a cada vez que o jogador recusa a saída fácil disponível
(paga cura à vista mesmo doendo, quita dívida no plano agressivo) e não sobe (não desce) quando
aceita a saída fácil.

- **Prós:** mais granular, sensação de progresso constante.
- **Contras:** mais caro de balancear, mais difícil de comunicar em patamares claros (o próprio
  risco que o líder apontou: "consequência invisível não ensina nada"); escopo maior que o
  necessário pra um estúdio solo.

**Recomendação: Desenho 1.** É o mais barato, o mais difícil de explorar por timing, e o único que
não corre risco de punir azar de combate como se fosse imprudência financeira.

### 6.2 Onde a vantagem aparece (forma, não número)

Reaproveita o sistema de **reputação de vendedor** já canônico
(`cartas-hardware-pirataria-energia.md` §14: "vários vendedores, com reputações diferentes,
descoberta por experiência própria ou fofoca de NPC"). A mesma engrenagem, espelhada: o mercado
também forma uma opinião sobre O JOGADOR.

**Patamares, nome definitivo do líder (2026-08-01): `prudente` / `ponderado` / `imprudente`.**
O terceiro patamar recebe rótulo localizado próprio, não tradução literal: **`VidaLoka`** em
pt-br e **`YOLO`** em en-intl (equivalência cultural deliberada, ecoa a frase que abriu esta
doutrina no mesmo dia: "a dúvida da humanidade: prudência × yolo"). Rótulo **separado** de
Bronze/Prata/Ouro (esse trio fica reservado para Knowledge Progression e o gate do auto-resolve;
a reputação de prudência tem vocabulário próprio, por decisão do líder).

> **Nota para o `i18n-l10n-specialist`:** as chaves `economia.reputacao.imprudente` (nome
> provisório) entram no catálogo com valores **divergentes de propósito** entre pt-br
> (`VidaLoka`) e en-intl (`YOLO`). Isso não é inconsistência de tradução a corrigir; é
> equivalência cultural deliberada, decidida pelo líder. Os outros dois patamares (`prudente` /
> `ponderado`) traduzem normalmente.

| Patamar | Onde aparece | Tipo de vantagem |
|---|---|---|
| **Prudente** | Sucata Honesta (Periferia, comércio livre honesto) + vendedores de reputação (§14) | acesso a estoque limitado mais cedo, preço melhor (desconto de reputação, não de saldo) |
| **Ponderado** | Igual acima, sem o extra | preço e acesso normais; é o patamar neutro, sem bônus nem prejuízo |
| **Imprudente (VidaLoka / YOLO)** | Igual acima | preço e acesso normais também (nunca pior que o padrão, ver §7.2 sobre por que), só sem os extras do patamar Prudente |

Nenhum desses é "dinheiro rendendo dinheiro": são portas que se abrem ou preços que baixam porque
o mundo aprendeu a confiar no jogador, exatamente como o líder pediu (acesso, preço, confiança).

### 6.3 Como o jogador percebe (obrigatório, senão a lição não existe)

1. **Uma linha de diálogo pontual, na primeira vez que o jogador cruza um patamar**, do vendedor
   relevante ("ouvi dizer que você é gente de responder pelo que gasta") antes do desconto/acesso
   passar a valer. Nunca silencioso.
2. **Nova sub-página "Reputação" no Diário do Gus** (mesmo padrão de Bestiário/Cartas/Lore/
   Companions, `pillars.md` §Sistemas-âncora), mostrando o patamar atual em RÓTULO diegético
   (nunca o número cru), com uma frase curta explicando o que sobe o patamar. Consistente com
   Pillar 2 (nunca opaco).

---

## 7. Memória financeira acumulada, pesando mais no fim (mecanismo)

**O que é lembrado:** o MESMO fato do desenho 1 da seção 6, pelo lado negativo: cada vez que o
jogador entra no fluxo de dívida a crédito do Hospital (§3.3.2), ao longo da campanha inteira, não
só daquele episódio. **Não é um contador novo separado**: é a mesma métrica de reputação, olhada
pelo lado "quantas vezes precisei" em vez de "quantas zonas fiquei limpo". Um único sistema, duas
leituras.

### 7.1 Peso por ato (resolve a exigência do líder: "as primeiras horas contam menos")

Cada episódio de dívida ganha um peso conforme o ato da história em que aconteceu, reaproveitando
a estrutura de 3 atos já canônica (`pillars.md` §Estrutura de capítulos):

| Ato | Peso do episódio de dívida | Racional |
|---|---|---|
| **Ato 1** | **×0,5** | é a fase de aprendizado do sistema inteiro; todo mundo é pobre no começo, e precisar de dívida aqui é esperado, não imprudência |
| **Ato 2** | **×1,0** | já teve tempo de aprender a economia; peso normal |
| **Ato 3** | **×2,0** | já devia saber administrar; um esbanjo aqui, perto do clímax, é o que a fala do líder descreve como "a velhice pobre" |

A soma ponderada desses episódios (menos as zonas limpas da seção 6, que seguem contando
positivo, sem peso por ato) é o placar que decide o bônus de clímax (§7.2).

### 7.2 A escada de reconhecimento (vários degraus, não dois momentos)

O líder pediu algo mais gradual do que "um eco no fim do ato 2 mais a cena final". Proposta: uma
escada de **seis degraus**, reaproveitando beats narrativos que já existem na estrutura de
capítulos (`pillars.md` §Estrutura de capítulos: recrutamento de companions no ato 2, transição de
atos), pra não pedir infraestrutura nova além de linhas de diálogo condicionais.

| Degrau | Quando | Intensidade | O que acontece |
|---|---|---|---|
| **0 (contínuo, pano de fundo)** | Sempre disponível, atualizado a cada zona cruzada | passiva | Diário do Gus, sub-página Reputação (§6.3): mostra o patamar ATUAL sempre que o jogador quiser conferir. Resolve sozinho "nunca ser pego de surpresa", porque está disponível o tempo todo pra quem checar. |
| **1** | Fim do Ato 1 (após o 1º ou 2º recrutamento) | leve | Primeira fala explícita de um NPC/vendedor reconhecendo o patamar atual. Ainda é cedo (peso ×0,5), então o tom é de apresentação ("isso existe, e já estamos de olho"), nunca de veredito. |
| **2 a 4** | A cada novo recrutamento de companion no Ato 2 (reaproveita a cena de chegada em cada cidade nova, já prevista) | crescente, mas ainda leve | Um NPC ou vendedor já familiar comenta, em 1 linha, se o patamar mudou desde o último degrau. Variação de texto a cada vez (não repete a mesma frase), pra não virar tique. Os bônus da seção 6.2 (acesso, preço) já valem a partir do momento em que o patamar sobe, não ficam guardados pra depois: o jogador sente o efeito PRÁTICO em tempo real, o reconhecimento NARRATIVO é que vem em degraus. |
| **5** | Transição para o Ato 3 (quando o peso ×2,0 começa a valer) | aviso explícito, obrigatório | Um NPC comenta de passagem que "aqui perto do fim, cada escolha pesa mais" (tom "quase desinteressado", nunca pop-up de regra). É o aviso ANTECIPADO que a régua do líder exige: o jogador sabe, antes de as escolhas pesarem o dobro, que a régua mudou. |
| **6 (final)** | Última preparação, imediatamente antes de entrar na Cúpula Sterling (Pillar 5, "todos reconvergem no ato 3") | máxima, é o pagamento grande | Um vendedor/aliado já estabelecido ao longo do jogo (nunca alguém novo de última hora) reconhece o histórico completo em diálogo, e SÓ ENTÃO concede o bônus de clímax proporcional ao placar ponderado da seção 7.1: acesso a estoque melhor, preço melhor, ou uma ajuda pontual (ex.: um adiantamento sem juros, distinto da dívida do Hospital) para a preparação final. |

**Por que seis degraus e não dois:** cada degrau intermediário (1 a 4) já entrega uma fatia de
sentimento de causa-e-efeito enquanto ainda há tempo de sobra pra mudar de rumo (estamos só no
Ato 1/2), em vez de guardar toda a informação pro fim, quando já não dá mais pra agir. O degrau 5
é o aviso explícito que a régua do líder pede ("nunca ser pego de surpresa"). O degrau 6 é o único
que de fato paga a conta grande, porque é o único ponto em que o peso ×2,0 do Ato 3 já terminou de
se acumular.

**Decisão de design que proponho e explico (não é neutra, por isso registro aqui):** em TODOS os
degraus, o bônus é sempre um ACRÉSCIMO sobre o preço/acesso normal, nunca uma penalidade ABAIXO do
normal. Mesmo o jogador no patamar Imprudente/VidaLoka consegue comprar tudo que precisa, em
qualquer degrau, ao preço de sempre; ele só não ganha o extra que o patamar Prudente ganha. **Isso
ainda cumpre "o esbanjador sofre mais" em termos relativos** (chega pior preparado que quem foi
comedido, na mesma luta), mas nunca vira punição absoluta de quem já está no pior momento do jogo.
Escolhi isso pela régua do próprio líder ("punição tem que ser legível, antecipável e evitável");
não vejo leitura segura de "piorar o preço de quem já está afundado, bem na hora da luta mais
dura" que não vire dark pattern. Se o líder quiser uma leitura mais dura em algum degrau
específico (ex.: o patamar VidaLoka paga ACIMA do normal só no degrau 6), é decisão dele; eu
recomendaria não ir por aí, pelos motivos acima.

### 7.3 Como o jogador descobre que isso existe

A escada da seção 7.2 já É a resposta: o degrau 0 (Diário sempre visível) garante que a informação
nunca fica escondida; os degraus 1 a 4 dão retorno de causa-e-efeito repetido ao longo de todo o
jogo, não só no fim; o degrau 5 avisa explicitamente ANTES do peso dobrar; o degrau 6 entrega o
resultado sempre em diálogo, nunca em número cru ("você sempre soube guardar o que importava" ou o
oposto, silêncio educado, nunca um insulto, antes de aplicar o efeito mecânico). Nenhum jogador que
prestou atenção em qualquer um dos seis degraus chega ao fim sem saber onde está.

### 7.4 Risco que eu mesmo sinalizo (régua anti-dark-pattern)

Um jogador pode cair em dívida por **necessidade genuína** (fase difícil, azar de combate, decisão
cara mas correta) e não por imprudência. O peso ×0,5 do Ato 1 mitiga parte disso (a fase mais
pobre do jogo conta menos), mas não elimina o problema no Ato 2/3: não tenho como o sistema
distinguir mecanicamente "precisei mesmo" de "fui descuidado" sem inflar o escopo bem além do que
cabe num estúdio solo. **Registro isso como limite conhecido, não escondo.** Mitigação prática: o
bônus de clímax nunca é grande o bastante pra decidir a luta sozinho (ele ajuda a preparação, não
substitui jogar bem), então mesmo o jogador que caiu em dívida por necessidade genuína ainda vence
o jogo normalmente, só sem o extra. Se o playtest mostrar que a diferença dói mais do que isso,
o primeiro ajuste é achatar o peso do Ato 3, não veto do sistema inteiro.

---

## 8. Teto do Firewall (forma do teto, não o número)

> **APROVADO pelo líder em 2026-08-01, na forma proposta abaixo** (custo real de bateria mais
> duração curta por uso). O número exato sai do balanceamento com dados, como o resto do motor de
> cartas.

Mesma lógica do honeypot (`proposta-mira-inimiga.md` §2.A): o teto de 10% de defesa ali existe pra
manter viva a dúvida "uso agora ou guardo pra depois". O Firewall precisa do mesmo tipo de teto,
só que aplicado ao que ele realmente controla (não é defesa, é "sumir da lista de mira").

**Recomendo compor o teto em DUAS frentes, não uma só, porque uma frente sozinha ainda deixa um
jeito de o Firewall virar "sempre ligado, de graça":**

1. **Custo de bateria real e proporcional ao poder do efeito** (o mesmo eixo de energia que já é
   canon em `cartas-hardware-pirataria-energia.md` §5/§6: "o efeito premium consome mais fundo").
   Sumir da mira do inimigo por um turno inteiro é um efeito forte; o dreno de bateria por uso
   precisa refletir isso, não ser um custo simbólico. Isso é o que cria, na prática, a MESMA
   pergunta que o líder quer preservar: "gasto a carga da carta nesta luta boba, ou guardo pra
   luta que vai doer de verdade" (a frase dele, aplicada ao Firewall em vez do honeypot).
2. **Duração curta por ativação** (não "liga e esquece pro resto da luta"): o efeito dura N turnos
   baixo, obrigando reativar (e pagar bateria de novo) se a luta continuar perigosa. Duração curta
   e custo real, juntos, impedem o Firewall de virar imunidade permanente barata.

**O que NÃO deve compor o teto:** nenhum "cooldown de metagame" (ex.: só pode usar 1x por dia fora
de combate), porque isso seria opaco e desligado da diegese (cartas-hardware §5 já estabeleceu que
o limite de uso vem de ENERGIA, não de um relógio arbitrário). O teto certo é físico (bateria) e
temporal-dentro-da-luta (duração), igual ao resto do sistema de energia já canônico.

Os números exatos (quanto de bateria, quantos turnos) ficam para o balanceamento com dados,
igual ao resto do motor de cartas (`//PLAYTEST`).

---

**Nada neste documento é canon.** A doutrina (economia.md §0) já é. As sete decisões do líder
citadas ao longo do documento (leitura da Opção F, vantagem não financeira, memória acumulada,
nome dos patamares, escada de reconhecimento em vários degraus, teto do Firewall aprovado, e o
alcance de F8 a partir do Mini-boss) também já são decisões tomadas.

**A escada de dificuldade por periculosidade (seção 2) está fechada em nível de design**: todas as
sete perguntas em aberto desta proposta foram respondidas pelo líder, e a tabela de tiers não tem
mais nenhuma linha pendente. Os MECANISMOS propostos para implementar cada decisão (seções 6, 7 e
8) seguem PROPOSTA de forma, pendente de aprovação item a item; os números exatos de cada um
(pesos de F1-F8, duração e bateria do Firewall, limiares de patamar) ficam `//PLAYTEST`, a fechar
com dados de simulação, igual ao resto do balanceamento do motor.
