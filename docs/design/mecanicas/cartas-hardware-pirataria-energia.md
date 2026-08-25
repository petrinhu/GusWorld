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
> o caso e o líder concordar (ex.: uma passiva mana-0 sem custo de bateria pode ser
> candidata a exceção, mas isso ainda não foi decidido — não presumir). Nasceu da
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
- **Especiais:** bateria **selada / de maior capacidade** — mais confiável (condiz com "especiais protegidas"), mas ainda exige cuidado no endgame (amarra com o clímax sem-save/PEM).

### Troca e recarga
- **Cidade:** grátis — abrir inventário, pôr carregada no lugar da usada.
- **Estação de recarga:** te dá uma bateria **NOVA carregada** e cobra pela tua velha; o preço (**1,2x–2x** o de uma recarga) varia com a **degradação** da que você entrega.
- **Trade-off:** recarregar você mesmo custa *tempo de espera*; comprar já-carregada custa mais crédito.
- **In-battle (arena):** trocar (se tiver carregada) custa **1 turno + AP** (número → `economy-designer`).

### Descarte
- Bateria descarregada **ocupa espaço** no inventário e **não pode ser jogada fora** — **crime ambiental**, tratado com **sátira leve do excesso regulatório** (registro LucasArts; a megacidade te obriga a carregar teu próprio lixo tóxico). Só vender ou trocar.

### Itens de energia (novos)
- **Carregador solar** — recarrega baterias passivamente (evita depender de estação/compra).
- **Powerbank** com visor LED de carga própria restante.
- **Carregador rápido** de bateria.
- **Medidor (voltímetro)** — mostra carga atual e degradação **em volts**; comprável OU adquirido na **missão do Volta**.
- **Ghost do Volta** = o **tutor** de todo o sistema de energia (baterias, ciclos, degradação, recarga). Casa com o roster (cada análogo ensina sua área; Volta = a pilha).

### Bateria pirata / genérica
Existe: mais barata, mas **mente sobre a carga**, morre cedo, pode **danificar a carta** ou ser vetor de vírus. A fraude do enquadramento no nível do hardware — caveat emptor também nas pilhas.

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
- **Em aberto (não decidido, não inventar):** o que acontece se a bateria acabar com o Gus dentro da parede, e o uso da carta em batalha. As duas perguntas foram devolvidas ao Gus na issue 3 do bus e aguardam a resposta dele.
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
- As **ferramentas (voltímetro/Turing) NÃO revelam antes de comprar** (senão matariam o risco) — são diagnóstico/cura **depois** (§11).

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
- [x] ✅ **Números** (ENTREGUE: `cartas-numeros-proposta.md`) → `economy-designer` (preço de recarga, AP da troca in-battle, drain rate, capacidade kR/MR, vida de ciclos, **X% e a tabela de risco de contaminação da §9**, preço pirata vs original ao longo do arco).
- [x] ✅ **Spec de implementação** (ENTREGUE e APROVADA 2026-07-28: `cartas-spec-dados.md` + `cartas-spec-logica.md`) → `gameplay_engineer` (usos/bateria, estados de carta, vírus/adware) + `backend-engineer` (modelo de dados: tipo, memória, bateria, integridade, flag de infecção).
- [x] ✅ **Canonizado pelo líder em 2026-07-28.** Falta só refletir em `cartas-technomagik.md` e na terminologia (passo de escrita). Era: canonizar no doc de mecânicas + refletir em `cartas-technomagik.md` / terminologia quando o líder aprovar.
- [ ] **Camada pirata de ESPECIAL/SUPER (§15, decisão do líder 25/08/2026):** canon fechado, números em aberto → `economy-designer` (probabilidade de cada um dos 3 graus de falha, custo em crédito/material do craft, preço da cópia comprada pronta no mercado negro). Mecanismo de entrega da SUPER original ao fim da missão-capstone também em aberto (§15.6) → líder.
- [ ] **Bateria de baixa qualidade craftada a partir de lanches refinados (§5, decisão do líder 25/08/2026; herança de risco FECHADA no mesmo dia):** canon fechado no mecanismo (refino gated, bancada compartilhada com reparo/pirataria, trade-off consumir×refinar, rede de segurança, terceira rota de compra, herança dos três riscos da bateria pirata comprada) → `economy-designer` (capacidade/usos, preço de compra do suco puro/água destilada, taxa de sucesso do refino, probabilidade numérica de cada risco herdado). Duas lacunas de desenho, não numéricas, ainda em aberto → líder: se o craft exige os dois insumos refinados juntos ou cada um sozinho já basta; e o momento narrativo em que o personagem "aprende a filtrar".
