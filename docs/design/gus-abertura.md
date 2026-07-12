# GUS-ABERTURA: Esqueleto da Abertura de GusWorld

> **Status:** PROPOSTA (decisões do criador 2026-07-12, esqueleto; detalhes finos a brainstormar).
>
> **Tipo de documento:** blueprint estrutural (beat sheet + ludonarrative analysis + foreshadowing tracker). NÃO contém redação final de diálogo, cutscene ou texto do recado. Redação final é responsabilidade do `narrative-writer`, em handoff futuro, após aprovação deste esqueleto e resolução dos pontos marcados "a brainstormar depois".
>
> **Item TODO.md:** `GUS-ABERTURA` (linha 434). Depende de `LORE-ORIGEM-MULTIVERSO` (parcialmente canonizada, seed #1 do brainstorm-backlog).
>
> **Cross-refs:** `sinopse.md` §3 (Protagonista), §4 (Família Vance); `docs/design/gdd.md` §6 (Mecânicas-âncora), §7.1 (Topologia de acesso ao mundo); `docs/narrative/arco-principal.md` §Ato 1 Diagnóstico (ver §10 deste doc, reconciliação); `docs/narrative/characters/brunus-vetorial.md` + `brunus-vetorial-conto.md`; `docs/design/brainstorm-backlog.md` #8; memórias `project_nome_gus_canon`, `project_familia_vance_canonica`.

---

## 0. Escopo e princípio de leitura

Este documento fixa **cinco beats obrigatórios**, decididos pelo criador em 2026-07-12, na ordem dada. Ele NÃO decide:

- o conteúdo exato do recado do Brunus;
- a identidade da "força antiga" (fica velada, é lore de endgame);
- o primeiro companion e a primeira área (isso é o item `MUNDO-TOPOLOGIA`, documento à parte);
- a cena e o scripting fino do tutorial (blocking exato, timing, câmera).

Cada um desses pontos aparece marcado como **[A BRAINSTORMAR]** nas seções correspondentes, e consolidado na lista final (§11).

---

## 1. Beat sheet macro

| # | Beat | Função narrativa | Localização | Duração alvo | Ponto de framework |
|---|---|---|---|---|---|
| 1 | Tutorial (dia comum) | Estabelece caráter + Pillar 1/2 (magia = software) desde o minuto 1 | Apartamento Vance, Edifício Vance, 6º andar leste, Núcleo Metropolitano | 4-7 min | Story Circle: zona de conforto |
| 2 | A interrupção (sumiço) | Quebra o dia comum; catalisador do arco | Apartamento Vance -> botica-laboratório do Brunus | 2-3 min | Story Circle: necessidade / Catalyst (Save the Cat) |
| 3 | A investigação | Gus resolve por lógica, não por susto (Pillar 4) | Botica-laboratório do Brunus | 3-5 min | Debate / busca de pista |
| 4 | O gancho velado | Planta ameaça de endgame sem confirmar nada | Botica-laboratório do Brunus (leitura do recado) | 1-2 min | Foreshadow plant |
| 5 | O fecho (partida) | Cruza o limiar; abre o mundo | Botica -> rua -> saída do Núcleo Metropolitano | 1-2 min | Story Circle: cruzar o limiar / Break into Two |

**Duração total estimada da abertura:** 12-19 minutos de gameplay real, dentro do orçamento de "Setup (1-1.5h gameplay)" já previsto em `arco-principal.md` §Ato 1 (a abertura é a fração inicial desse bloco maior, não o bloco inteiro).

---

## 2. Beat 1: Tutorial (dia comum)

### Função

Estabelecer o Gus como tinkerer analítico (Pillar 4) e o pilar "magia = sistema formal computável" (Pillar 2, TechnoMagik) antes de qualquer conflito. O jogador aprende scan e controles fazendo, não lendo texto de tutorial.

### Conteúdo estrutural

- **Ação central:** Gus está em casa, no próprio quarto/bancada, **refinando o software do próprio Tavus-Drive**. Não é uma cutscene passiva; é gameplay mínimo desde o segundo 1 (o jogador já controla o Gus mexendo em algo).
- **Ensino orgânico sugerido** (ligar aos verbos de `gdd.md` §6, sem fixar sequência exata):
  - Scan (§6.1, Sintonização Ortodôntica) ensinado ao Gus escanear os próprios objetos do quarto (peças de reposição, deck de cartas guardado, foto da família) em vez de um inimigo. Reforça o pilar antes do combate existir na sessão.
  - Compilação (§6.2) pode aparecer em miniatura: Gus testa uma carta-token sozinho, no ar do próprio quarto, sem alvo (efeito cosmético/diagnóstico, não combate).
  - Controles de movimento e câmera ensinados andando pelo apartamento, não por prompt de texto.
- **Environmental storytelling (mostrar, não contar):** o quarto/bancada carrega a caracterização sem monólogo:
  - Peças de Tavus-Drive espalhadas, ferramentas de solda (ecoa o "Opening Image" preexistente em `arco-principal.md`, ver §10).
  - Jogo de xadrez visível (tio Yakov, sinopse §4).
  - Retrato de família separada mas afetuosa (Pyotor + Gargi + Gus, sem clima de ruptura, cf. `project_familia_vance_canonica`).
  - Um objeto de origem do Brunus (frasco, reagente, ou bilhete antigo de visita) plantado no cenário **antes** do sumiço, como Chekhov's gun barata: o jogador vê e esquece; beat 2 reativa.
- **Presença da mãe (Gargi):** cf. sinopse §4, Gargi é "presença cotidiana primária do filho"; cena doméstica com ela (silenciosa ou curta) reforça família funcional descentralizada sem dramatizar a separação dos pais. Pyotor pode estar ausente por rotina normal (itinerante, cf. sinopse §4), não por conflito.

### Ludonarrative harmony

Ensinar scan/compilação num momento de manutenção caseira, não de combate, casa mecânica e tema desde o primeiro contato: o jogador aprende que a "magia" do Gus é, literalmente, o ofício dele. Isso é harmony deliberado (mecânica = tema), não dissonância.

### [A BRAINSTORMAR]

Blocking exato, ordem exata dos passos de tutorial, se há ou não um mini-combate de teste (havia um no rascunho antigo, ver §10), diálogo final da cena.

---

## 3. Beat 2: A interrupção (sumiço)

### Função

Quebrar o dia comum. Catalyst (Save the Cat) / chamado à aventura que ainda não é aventura, é preocupação concreta com uma pessoa específica.

### Conteúdo estrutural

O mentor **Brunus Vetorial** (boticário itinerante, ~700 anos, cf. `brunus-vetorial.md`) **desaparece**. O sumiço precisa ser sentido, não anunciado por HUD. Três mecanismos candidatos de descoberta, estruturalmente equivalentes, não decididos:

| Opção | Mecanismo | Prós | Contras |
|---|---|---|---|
| A | Brunus tinha visita agendada/rotineira ao apartamento Vance e não aparece; Gargi ou o próprio Gus estranha o atraso | Reforça vínculo cotidiano já estabelecido em `brunus-vetorial.md` (§ "Ponte com o pai ausente", relação de confiança) | Exige estabelecer a rotina de visitas no próprio beat 1, sem tempo de tutorial sobrando |
| B | Gus tenta contato de rotina (rádio, mensagem, qualquer canal diegético já estabelecido) e não recebe resposta | Econômico, cabe dentro do próprio quarto, sem mudar de cena | Depende de existir um canal de comunicação diegético já definido (a confirmar em design técnico) |
| C | Gus vai à botica-laboratório por outro motivo (buscar reagente, entregar recado do pai) e encontra o lugar vazio/trancado/em desordem | Motiva a transição espacial pro beat 3 organicamente; permite environmental storytelling forte (loja bagunçada, sinais de saída apressada) | Precisa de um motivo prévio plausível pro Gus ir até lá logo cedo |

Recomendação estrutural (não vinculante, é do narrative-designer, decisão final é do criador): **C**, porque resolve dois problemas de uma vez, motiva a transição espacial (beat 2 -> beat 3 no mesmo local) e entrega environmental storytelling sem exigir infraestrutura nova de comunicação diegética.

### Tom

Curto. Não é jump scare, é vazio errado: a ausência de alguém confiável, presente na rotina, registrada por um detalhe fora do lugar (porta destrancada quando devia estar trancada, luz apagada na hora errada, poeira em cima de algo que devia estar em uso).

### [A BRAINSTORMAR]

Mecanismo exato de descoberta (escolher entre A/B/C ou variante), blocking, se há diálogo de terceiros (Gargi comentando) nesse beat ou se é solitário desde já.

---

## 4. Beat 3: A investigação

### Função

Resolver por lógica, não por pânico (Pillar 4 intacto mesmo em cena de tensão). Gus lê o ambiente como leria um sistema, não corre gritando.

### Conteúdo estrutural

- **Espaço:** botica-laboratório do Brunus (cf. `brunus-vetorial.md` § "Ofício e axiologia": "tem uma botica-laboratório como base"). **Nota de continuidade:** este local ainda não está canonizado em `PLACES.md`; ao aprovar este esqueleto, adicionar entrada correspondente.
- **Gameplay sugerido:** reutilizar o verbo scan (§6.1) recém-ensinado no beat 1, agora aplicado a um cenário real com stakes. Diegeticamente elegante: o jogador já sabe scanear, e o jogo pede que ele use a ferramenta pra achar o recado escondido ou parcialmente oculto, em vez de simplesmente andar até um objeto brilhante.
- **O que a investigação revela:** sinais de partida apressada mas não de violência explícita (evitar dark gratuito sem propósito, cf. anti-pattern do canon), e o **recado pessoal** deixado pelo Brunus para o Gus especificamente (não um bilhete genérico; endereçado).
- **Reforço de caracterização:** a cena pode reaproveitar o objeto plantado no beat 1 (o frasco/reagente/bilhete antigo do Brunus visto no quarto do Gus) como o gatilho que faz o Gus pensar em ir à botica primeiro, antes de qualquer outra hipótese. Amarra os três primeiros beats num fio de causa e efeito único, sem exposição.

### Especificação estrutural do recado (formato, não conteúdo)

Segue o padrão de "documento descobrível" já usado em `docs/narrative/in-world-docs.md` (formato, não conteúdo):

| Campo | Valor estrutural |
|---|---|
| Tipo de documento | Recado pessoal, endereçado ao Gus nominalmente |
| Suporte | A definir: papel físico (coerente com o "químico analógico" do Brunus) ou gravação em suporte não-digital (coerente com "não é testemunha-enciclopédia digital") |
| Tom de voz | Precisa e econômica, cf. voice sample de `brunus-vetorial.md` (frases médias, cadência lenta, tic de medir o tempo) |
| Função de plot | Confirma o sumiço como deliberado/consciente (não acidente, não sequestro banal), sem entregar quem ou o quê |
| Função emocional | Ecoa a voz do Brunus já estabelecida (o jogador que já leu falas dele, se houver, reconhece o tom; quem não leu, sente a economia de palavras como característica, não como falha de escrita) |

### [A BRAINSTORMAR]

Conteúdo exato do recado (texto final é redação, cabe ao `narrative-writer` em handoff futuro). Suporte físico exato. Se há puzzle leve pra encontrar o recado ou se ele está à vista.

---

## 5. Beat 4: O gancho velado

### Função

Plantar a intriga de endgame sem confirmar nada. Foreshadowing pago (mandato do narrative-designer: toda seta plantada cobra resolução, mas nem toda seta é puxada na abertura).

### Conteúdo estrutural

O recado insinua que:

1. Existe uma **força antiga** que persegue quem "sabe demais".
2. **O Brunus sabia de algo perigoso** e, implicitamente, foi atrás disso ou fugiu disso.

### Disciplina de velado (regra dura, não negociável neste documento)

- **NÃO nomear nem confirmar** o Dragon Victory, a linhagem Pyotor I Draco Vance, Vyrdragon, ou qualquer fratura de origem (Estilhaçamento / `LORE-ORIGEM-MULTIVERSO`) nesta cena.
- O recado pode usar linguagem ambígua compatível com o que o Brunus já é, canonicamente, capaz de dizer sem quebrar o próprio personagem: ele "esqueceu o técnico e os fatos antigos" (cf. `brunus-vetorial.md` § "O esquecimento") mas "retém vínculo, amizade, culpa". Ou seja: o recado pode soar como aviso pessoal e afetivo ("cuide-se", "não procure isso sozinho", "há coisas que voltam") sem virar aula de lore.
- Tom: sombrio, ameaçador, mas nunca didático. O jogador deve sentir que uma peça maior existe, sem receber o nome da peça.

### Foreshadowing tracker (entrada desta abertura)

| # | Setup (beat/cena) | Payoff alvo | Tipo | Status |
|---|---|---|---|---|
| ABERTURA-1 | Beat 4, recado do Brunus, menção velada à "força antiga" | Endgame (reveal ligado a Dragon Victory / lore de origem, beat exato TBD em `arco-principal.md`) | Diálogo/texto | plantado, payoff a determinar |
| ABERTURA-2 | Beat 4, "Brunus sabia de algo perigoso" | Reveal futuro do que o Brunus de fato sabia/fez (não necessariamente o Dragon Victory; pode ser lore intermediária a definir) | Diálogo/texto | plantado, payoff a determinar |
| ABERTURA-3 (opcional) | Objeto plantado no beat 1 (item do Brunus no quarto do Gus) | Reencontro ou reconhecimento do mesmo objeto em cena futura com o Brunus (se/quando reaparecer) | Visual/mecânico | plantado, payoff a determinar |

### [A BRAINSTORMAR]

Identidade da força antiga (fica velada por design; não é pra esta sessão nem pra este documento). Exato ponto de payoff no arco principal.

---

## 6. Beat 5: O fecho (enxuto)

### Função

Cruzar o limiar (Story Circle) / Break into Two (Save the Cat), mas **sem** recrutar companion e **sem** abrir a primeira área de setting. Este beat termina a abertura propriamente dita; o que vem depois é escopo do item `MUNDO-TOPOLOGIA`.

### Conteúdo estrutural

- Gus, sozinho, com o recado (fisicamente carregado ou registrado no Tavus-Drive) e o peso da ameaça velada, decide sair para investigar.
- **Sem gating artificial hard** (cf. `gdd.md` §7.1): a cena termina abrindo o mundo, não uma porta trancada com aviso de "área liberada". O jogador sai da botica/apartamento para a rua, e a partir daí o mundo aberto responde por dificuldade e não por trava.
- **Enxuto por design:** este beat não introduz o primeiro companion nem a primeira área de setting. Esses dois elementos são responsabilidade do documento `MUNDO-TOPOLOGIA` (que decide onde o Gus vai primeiro e quem encontra), consistente com a estrutura já existente em `arco-principal.md` §Ato 1 ("Recrutamento Companion #1: ... dependendo de onde Gus vai primeiro").
- **Opcional, a considerar em brainstorm futuro, não decidido aqui:** uma despedida curta com a Gargi, ecoando o "Mãe pede pra Gus não ir" já presente em `arco-principal.md` (ver §10), mas recontextualizada em torno da ameaça ao Brunus, não da anomalia genérica antiga.

### [A BRAINSTORMAR]

Se há ou não beat de despedida com a mãe, blocking da saída, se o recado fica visível na UI/inventário desde já ou só reaparece em momento narrativo específico.

---

## 7. Diagrama de pacing emocional

```
Tensão
alto   |                    *
       |                   / \
médio  |        *         /   \
       |       / \       /     \
baixo  | *----*   *-----*       *---->
       |
       Beat1  Beat2 Beat3  Beat4   Beat5
       calmo  susto vale   pico    vale
       setup  curto invest dread   resolução
                    -igação         (decisão,
                                     não alívio)
```

- **Beat 1 (baixo):** calma doméstica, estabelece linha de base emocional.
- **Beat 2 (pico curto):** susto controlado, não jump scare gráfico, é vazio errado.
- **Beat 3 (vale, mas com tensão residual):** investigação analítica, ritmo mais lento, jogador processa pistas.
- **Beat 4 (pico de dread):** ápice emocional da abertura, mas contido (é leitura de um recado, não uma revelação visual grande).
- **Beat 5 (vale de resolução determinada):** não é alívio, é decisão. O jogador sai da cena com peso, não com paz.

Técnica de horror psicológico aplicável ao beat 2-4 (cf. mandato do narrative-designer): **antecipação e mundano subvertido** (o cotidiano da botica, corrompido pela ausência) rendem mais do que qualquer revelação explícita. Evitar imagem de violência gráfica; a ameaça é sugerida, nunca mostrada.

---

## 8. Ludonarrative: diagnóstico

| Elemento | Harmony ou dissonância | Nota |
|---|---|---|
| Ensinar scan/compilação em cena doméstica (beat 1) | Harmony | Mecânica ensinada = ofício do personagem = tema do jogo (Pillar 1/2), sem hiato entre o que se aprende e o que o personagem é |
| Reusar o scan pra achar o recado (beat 3) | Harmony | O jogador resolve como o Gus resolveria: por instrumento, não por sorte ou by-the-numbers de UI |
| Resolução por lógica, sem combate forçado na abertura | Harmony (Pillar 4) | Gus de 11 anos não vira herói de ação na cena que define seu caráter; a tensão é cognitiva/emocional, não de combate |
| Eventual mini-combate de tutorial herdado do rascunho antigo (ver §10) | A verificar | Se mantido, precisa ficar claramente separado da cena de tensão do sumiço, pra não misturar "combate-tutorial mecânico" com "cena de ameaça emocional real"; risco de diluir o peso do beat 2-4 se elas colidirem no mesmo espaço de jogo |

---

## 9. Continuidade com a família Vance

Regra explícita deste esqueleto, reforçando a instrução do criador: **o sumiço é do mentor Brunus, não da família.** A família Vance permanece canonicamente "separada mas funcional" (cf. `project_familia_vance_canonica`, `sinopse.md` §4):

- Gargi (mãe) pode aparecer no beat 1 e, opcionalmente, no beat 5, sem dramatizar a separação dos pais.
- Pyotor (pai) pode estar ausente por rotina normal de médico-cyber itinerante (não é abandono, é ofício, cf. sinopse §4), sem precisar aparecer fisicamente na abertura.
- Nenhum dos dois pais é fridgeado, ferido ou ameaçado nesta abertura. A ameaça recai inteiramente sobre uma figura fora do núcleo familiar (o mentor), preservando a família como base segura (safe base canônica, cf. sinopse §4, ficha da Gargi) à qual o Gus pode, narrativamente, ainda voltar.

---

## 10. Reconciliação com `docs/narrative/arco-principal.md` (Ato 1, Setup)

O documento `arco-principal.md` (canônico, Revisão 1) já contém um rascunho de abertura pré-existente, no bloco "Ato 1: Diagnóstico (setup)", sub-beats "Opening Image", "Tutorial diegético", "Catalyst", "Mãe", "Recrutamento Companion #1", "Foreshadow Sterling", "Foreshadow Dante", "Break into Two". Esse rascunho **precede** a decisão do criador de 2026-07-12 registrada neste documento, e diverge em pontos estruturais importantes:

| Ponto | Rascunho antigo (`arco-principal.md`) | Esqueleto novo (este documento) |
|---|---|---|
| Cena de abertura | Gus soldando upgrade na Matriz Ortodôntica; mãe traz chá, sem palavras | Gus refinando o software do próprio Tavus-Drive (foco em software, não hardware físico) |
| Tutorial | Combate-tutorial contra drone Sterling Corp com "comportamento emergente inesperado" | Tutorial doméstico via scan/compilação nos próprios objetos, sem combate necessariamente |
| Catalyst | Anomalia massiva detectada via Matriz Ortodôntica, sinal da Orla Recursiva, ruído incoerente (pré-anuncia Patch-Zero) | Sumiço do mentor Brunus Vetorial |
| Recrutamento | Companion #1 recrutado ainda dentro do Ato 1 Setup, antes do "Break into Two" | Nenhum companion recrutado na abertura; recrutamento e primeira área ficam no item `MUNDO-TOPOLOGIA`, à parte |
| Papel da mãe | "Mãe pede pra Gus não ir. Não é briga; é cuidado contido." | Preservável, mas a reconectar à ameaça ao Brunus em vez da anomalia genérica |

**Recomendação estrutural (não decisão):** após aprovação deste esqueleto pelo criador, `arco-principal.md` §Ato 1 Setup precisa de uma passada de sincronização (fora do escopo deste documento) para alinhar os sub-beats "Catalyst" e "Recrutamento Companion #1" à nova decisão. Até lá, este documento (`gus-abertura.md`) é a fonte mais recente para a abertura; `arco-principal.md` §Ato 1 Setup fica marcado como defasado nesse trecho específico, sem prejuízo do resto do documento (que segue canônico).

---

## 11. Itens marcados "a brainstormar depois" (lista consolidada)

1. Conteúdo exato do recado do Brunus (redação final cabe ao `narrative-writer`).
2. Identidade da "força antiga" (fica velada; lore de endgame, ligada a `LORE-ORIGEM-MULTIVERSO`).
3. Primeiro companion e primeira área (item `MUNDO-TOPOLOGIA`, documento à parte).
4. Cena e scripting fino do tutorial (blocking exato, timing, câmera, diálogo final).
5. Mecanismo exato de descoberta do sumiço (escolher entre as opções A/B/C do §3, ou variante).
6. Se há ou não beat de despedida com a Gargi no beat 5, e seu conteúdo exato.
7. Suporte físico do recado (papel vs outro meio) e se há puzzle leve para encontrá-lo.
8. Local canônico da botica-laboratório do Brunus em `PLACES.md` (ainda não cadastrado).
9. Ponto exato de payoff das entradas ABERTURA-1/2/3 do foreshadowing tracker (§5) em `arco-principal.md`.
10. Sincronização formal de `arco-principal.md` §Ato 1 Setup com este esqueleto (§10).

---

## 12. Próximo passo sugerido

Após aprovação deste esqueleto pelo criador (incluindo resolução dos pontos do §11 que forem necessários antes de redigir), o pacote de handoff ao `narrative-writer` deve incluir: os cinco beats fixados, o tom (sombrio contido, sem gore, Pillar 4 intacto), a voz do Brunus já estabelecida em `brunus-vetorial.md`, a disciplina de velado do §5, e a nota de continuidade familiar do §9. Este documento não inclui os parâmetros AEN de handoff porque a redação final ainda depende de decisões pendentes (§11); preencher o pacote de handoff é passo seguinte, não deste documento.
