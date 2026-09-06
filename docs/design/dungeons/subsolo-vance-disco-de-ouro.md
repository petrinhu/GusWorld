# Dungeon do Subsolo do Edifício Vance: conceito e o disco de ouro (`D27`)

> **Status:** CONCEITO (mesmo grão que as 13 dungeons já registradas em `docs/design/mundo-topologia.md`
> §4, decisão do líder de 12/07/2026: só tema + gimmick + forma; **layout fino fica para produção**,
> com a engine de mapa existindo e o `level-designer` da vez — nenhuma das duas condições vale hoje
> (o projeto não tem código, e a camada que desenha nasce bloqueada pelo GlintFx). Nada aqui é
> planta, sala a sala, ou disposição de encontro.
>
> **Orçamento e segunda entrada decididos pelo líder em 06/09/2026** (seção 4): a dungeon é orçada
> no Núcleo Metropolitano, com uma segunda entrada pelos Dutos Infernais por uma porta de três
> estados.
>
> **Cross-refs:** `TODO.md` (item `D27`), `docs/design/mundo-topologia.md` §4 e §4.2 (orçamento de
> 13 dungeons e o critério que separa dungeon de exploração de cena de clímax), `docs/design/dungeons/kola-sg3-12262-espelho-obsidiana.md`
> (`D21`, beats 5-6, seção 2 — cadeia causal do disco de ouro), `PLACES.md:83-84,127` (Edifício
> Vance, Subsolo do Edifício Vance e Biblioteca Cintilante), `sinopse.md:80` e `CHARS.md:47` (Gargi
> Vance), `docs/narrative/environments/01-cidade-cyber-gotica.md:49-51` (as duas bancadas).

---

## 1. Confirmação da fonte: acesso e as duas bancadas

A leitura da tabela de pendências foi conferida contra o canon, e ele confirma o aviso, sem
divergência. Há **duas bancadas** no Edifício Vance: a da **Gargi Vance**, ativa, na **sala do
apartamento, 6º andar**, visível da rua (`sinopse.md:80`; `CHARS.md:47`) — **não** é o acesso; e a
de **Pyotor Vance, herdada pelo Gus**, no **Subsolo do Edifício Vance**, sub-local distinto do
apartamento, na base do prédio (`PLACES.md:84`), sob um alçapão coberto por carpete que o próprio
Gus nunca notou (`docs/narrative/environments/01-cidade-cyber-gotica.md:50-51`). **É este** o
acesso à dungeon.

Gêmeos verificados por busca literal (`D27`, "disco de ouro", "Subsolo do Edifício Vance",
"alçapão"): **encontrados 8 arquivos, analisados 8, falharam 0**, sem contradição entre eles.

## 2. Conceito

**Tema:** uma instalação de guarda, esquecida sob a fundação moderna do Edifício Vance — erguida
por quem escondeu o disco de ouro, séculos antes de o prédio existir em cima dela sem saber.

**Gimmick:** cada câmara é um **selo formal** (vocabulário TechnoMagik, camada 1 — sistema
computável, não decreto de lore) que só cede a um pequeno teste de lógica/sequência; a party aprende
ali a mesma leitura de padrão que a porta em forma de LP da Kola-SG3-12262 vai cobrar depois
(`D21` beat 3). Combate é raro e concentrado perto do fim, não distribuído em toda a extensão.

**Forma:** trilha curta e linear, com dois desvios opcionais e retorno garantido (áreas secretas,
não caminho alternativo), mais uma **segunda entrada**, pelos Dutos Infernais (seção 4).

**Parada obrigatória na Biblioteca Cintilante** (`PLACES.md:127`, acervo do auge cooperativo da Era
1): é o único acervo do mundo antigo o bastante para **autenticar** uma peça da idade do disco de
ouro — sem essa confirmação a party não saberia ter achado a chave certa, só um disco qualquer.
Entrega **prova de autenticidade**, não decifração: o conteúdo da inscrição da porta segue
dependendo de `D23`/`D24`, como já fixado no `D21`.

**O objeto do fundo:** o disco de ouro, já canônico em `D21` (seção 2, beat 5) como a chave da
porta da Kola-SG3-12262. O que está gravado nele — datas, calendário — é conteúdo cifrado em
`docs/_secret/dungeons/kola-sg3-12262-desfecho.md` seção 2, por revelar o desfecho de outra
dungeon; não repetido nem resumido aqui.

## 3. O que é um "nível" aqui

Nesta perspectiva fixa e sem eixo de altura (L-26), "nível" é **um espaço distinto conectado à
grade**, mapa próprio como qualquer área do mundo — nunca andar visto de lado nem câmera subindo.
Avançar de nível troca de mapa; a sensação de profundidade vem só de composição (paleta, som), nunca
de geometria empilhada.

## 4. Orçamento e a segunda entrada (decisão do líder, 06/09/2026)

**Decisão do líder, verbatim:** *"E os dois se interligam, mas a porta é invisível do lado do edf
Vance. Só será vista quando aberta do lado dos dutos infernais, se tornando um caminho
alternativo. Quando aberta a porta da primeira vez, ela será vista pelos dois lados normalmente."*

O slot sai dos Dutos Infernais (que passam de 2 para 1 dungeon orçada) e vai para o Núcleo
Metropolitano (`mundo-topologia.md` §4, tabela atualizada) — o total de 13 fica intacto. A razão não
é só aritmética: **o subsolo do Edifício Vance e os Dutos Infernais são o mesmo subterrâneo**, e a
`D27` ganha uma **segunda entrada**, pelos Dutos.

**A porta tem três estados, nesta ordem:**

1. **Fechada e invisível do lado do Edifício Vance.** Quem desce pelo alçapão do subsolo do Vance
   não a vê nem interage com ela — não é uma porta trancada, é uma porta que não está lá para esse
   lado.
2. **Visível e destrancável do lado dos Dutos Infernais**, desde o início. É de lá que a party a
   abre pela primeira vez.
3. **Depois de aberta uma vez, visível e utilizável dos dois lados, para sempre** — vira caminho
   alternativo entre as duas áreas, no mesmo padrão de atalho já canônico do mundo
   (`mundo-topologia.md` §9: encurta entre dois pontos já alcançáveis, nunca é o único acesso).

**Isto cabe no que o jogo tem, conferido:** é transição de mapa 2D entre duas áreas do mundo já
existentes, sem eixo de altura (seção 6) — o mesmo mecanismo de qualquer atalho do §9. O estado da
porta (aberta/fechada, visível/invisível por lado) é uma flag de progresso persistida no save, do
mesmo tipo que qualquer outro estado de mundo (PEM desligado, item obtido) — nenhum mecanismo novo,
nenhuma exigência que o jogo não possua.

**Onde a porta encaixa no grafo da seção 3:** perto do início da trilha (entre N1 e N2), não perto
do fim — assim o atalho poupa a travessia de entrada para quem já abriu a porta, sem deixar a party
pular o corpo da dungeon (N3-N8) e ir direto ao disco de ouro. Quem entra pelos Dutos cai em N2; N1
(o vestíbulo-tutorial sob o alçapão) só é visto por quem entra pelo Edifício Vance.

**Decisão do líder, verbatim: "opcao 1. Quando Cauã acabar esse arco, ele vai achar a porta para a
outra dungeon."** O laboratório FIR continua existindo, inteiro, na metade que se entra pelos Dutos
Infernais — ondas de guardas, terminais hackeáveis, o payoff da Subestação 7 (`PLACES.md:88`), onde
sempre esteve. **Canon novo:** é o próprio Cauã quem acha a porta, ao terminar aquele arco — a
descoberta da passagem é o prêmio narrativo de fechar o arco dele, não um achado solto de
exploração. Isto amarra os três estados da porta a um evento de história, não só a uma regra de
mecânica: ela fica invisível do lado do Edifício Vance e só existe do lado dos Dutos até esse
momento específico — **quem** a abre e **quando** já estão fixados.

**Efeito sobre a ordem de descoberta, medido:** como o arco do Cauã (Ato 2) fecha bem antes da
cadeia de missão da Kola-SG3-12262 (Ato 3, `D21`) começar a apontar para o alçapão do Vance, a porta
já estará aberta e visível dos dois lados na primeira vez que a party desce pelo Edifício Vance —
ela não precisa ser "achada" de novo, só está lá. **Isto não quebra o conceito desta dungeon:**
`N1` (o vestíbulo-tutorial) continua sendo visto por quem segue a missão principal e entra pelo
alçapão, porque é a rota que a missão da Kola aponta. **Único caso em que a ordem muda:** um jogador
que explore os Dutos e feche o arco do Cauã bem cedo pode, em tese, atravessar para `N2` e visitar o
corpo da dungeon (`N3`-`N8`, inclusive o disco de ouro) antes de a missão do Espelho pedir por ele —
o mesmo padrão já aceito no resto do jogo para mundo aberto sem trava dura (C-02, `mundo-topologia.md`
§1) e já precedente no próprio `D21` (a carta Espelho Negro é obtida antes de a party saber da
relação dela com esta cadeia). Nesse caso, quem entra por `N2` só perde a introdução suave de `N1`
ao gimmick dos selos — perda pequena, porque `N3` já ensina por si (reset sem penalidade, seção 2) e
o padrão geral de "scan revela passagem" já foi ensinado na missão de abertura dos Dutos. **A posição
da porta (entre `N1` e `N2`) não muda com este canon.**

## 5. Cortes da L-29 atravessados

Nenhum corte é contradito. **C-02** (gating nunca é trava dura): a porta é atalho, nunca único
acesso — a entrada pelo Edifício Vance sempre funciona sozinha. **Cabe.**

## 6. Handoff

- **Propagado:** `docs/design/mundo-topologia.md` §4, §4.1, §4.2 e a lista de conceitos já refletem
  esta dungeon, a fusão com o slot dos Dutos Infernais e a porta de três estados.
- **Produção (fora deste documento, só quando a engine de mapa existir):** layout sala-a-sala, com
  `level-designer` (`mundo-topologia.md` §10 item 1); conteúdo das duas áreas secretas (item 3 do
  mesmo §10); redação em prosa da entrega do disco e da cena na Biblioteca Cintilante, por
  `narrative-writer`, só depois de aprovação deste conceito.
