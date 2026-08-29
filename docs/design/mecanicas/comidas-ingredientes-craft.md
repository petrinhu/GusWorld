# Comidas, ingredientes de craft e a cozinha do apartamento Vance (GusWorld G1/F3-Alpha)

**Status:** canônico. Extraído de `economia.md` §5.5 em 2026-08-25 (decisão do líder, verbatim:
"opcao um e deixe o ponteiro no economia.md para o novo arquivo"). Razão da extração: L-33 de
`GODS_LAWS.md`: economia de crédito muda quando preço, fonte e sumidouro mudam; comida muda
quando o catálogo e o craft mudam. As duas viviam juntas em `economia.md` só porque foi ali que a
conversa começou, não porque compartilham razão de mudar.
**Escopo:** canon pessoal do Gus Dragon (L-16): doze comidas, oito ingredientes de craft, a
cozinha do apartamento Vance como local de craft (caderneta de Gargi Vance, `in-world-docs.md`
Documento 19), e as duas decisões novas de 25/08/2026 sobre como cozinhar se amarra a essa
caderneta.
**Ponteiro de volta:** o documento-mãe é [`economia.md`](economia.md) §5.5 (que mantém o número da
seção, vazio de conteúdo próprio, só para não quebrar citação externa nenhuma). Craft de bateria a
partir de duas destas comidas continua em
[`cartas-hardware-pirataria-energia.md`](cartas-hardware-pirataria-energia.md) §5, sem mudança.

---

## §5.5. Lanches e ingredientes de craft (canon do Gus Dragon)

> **Canon pessoal (L-16 de `GODS_LAWS.md`).** Os itens abaixo são as comidas e os ingredientes de
> craft favoritos reais do Gus Dragon, filho do líder e colaborador humano do projeto,
> transformados em item de cura e em insumo de craft do jogo com aceite dele. Decisão original em
> 2026-07-17 (chocolate, suco de uva; suco de limão e água com gás ficaram com efeito aberto);
> fechada pelo líder em 2026-08-25 (efeito de suco de limão e água com gás) e **expandida pelo
> líder no mesmo dia, por ditado direto**, de quatro para **doze comidas** mais **oito ingredientes
> de craft** (ditado preservado em `DITADO-COMIDAS.md`, scratchpad de trabalho). **Não confundir
> com o PERSONAGEM Gus** (Gustaf VII Tavus Vance), canon público e distinto. Fora do apelido "Gus
> Dragon" (ou `Dragon-Drv`), o nome de batismo dele não é citado, aqui nem em nenhum outro lugar do
> projeto.

### 5.5.1 Comidas (12)

Doze consumíveis. Todos os doze têm efeito fechado pelo líder ao fim desta rodada (2026-08-25:
pizza de atum e macarrão ao molho pesto, as duas últimas, fecharam agora, ver §5.5.1e e §5.5.1f).
A única nuance que resta é o item 6 (tirinhas de frango): a CATEGORIA do buff ("buff ofensivo
temporário") está fechada, só a estatística exata (Ataque ou Força) não foi escolhida; por isso
não carrega marcador de pendência, é uma escolha binária dentro de efeito já fechado, não um
efeito em aberto (L-11: quem decide é o líder, não o agente).

| # | Item | Efeito | Craft / refino | Decisão |
|---|---|---|---|---|
| 1 | **Chocolate** | cura de HP, a MAIOR do catálogo (é o favorito real do Gus Dragon) | nenhum; consumível simples | líder, 2026-07-17 |
| 2 | **Suco de uva** | cura de HP, média | nenhum; consumível simples | líder, 2026-07-17 |
| 3 | **Suco de limão** | limpa efeito negativo (debuff) da party | refina em **suco puro de limão** (insumo de bateria de baixa qualidade, `cartas-hardware-pirataria-energia.md` §5) | líder, 2026-08-25 |
| 4 | **Água com gás** | restaura **fôlego do corpo** (`core::player::WindedTimer`, `stamina.md`) — **não** é energia de ação/AP; lacuna anterior FECHADA nesta rodada, ver §5.5.3 | refina em **água destilada** (mesmo papel de insumo de bateria) | líder, 2026-08-25 |
| 5 | **Sanduíche de queijo, sem queijo derretido** | fôlego do corpo **e** HP juntos, pouco de cada (mesmo recurso "fôlego" da linha 4, agora fechado — ver §5.5.3) | nenhum decidido | líder, 2026-08-25 |
| 6 | **Tirinhas de frango** | **buff temporário de ataque ou força** — a categoria "buff ofensivo temporário" está fechada; qual das duas estatísticas exatas (se forem distintas no jogo) não foi escolhida pelo líder | nenhum decidido | líder, 2026-08-25 |
| 7 | **Churrasco** | **cura leve em TODA a party de uma vez** (área, não single-target; ver amarra §5.5.1a) | nenhuma ligação óbvia a ingrediente do §5.5.2 | líder, 2026-08-25 |
| 8 | **Pastilha de menta** | **consumo sem custo de turno, só no overworld** (não vale em combate; ver amarra §5.5.1b) | nenhuma ligação óbvia | líder, 2026-08-25 |
| 9 | **Macarrão ao alho e óleo** | **imunidade temporária a UM tipo de efeito negativo** (ver amarra §5.5.1c) | nome cita **Alho** (§5.5.2) | líder, 2026-08-25 |
| 10 | **Pão de alho** | **escudo temporário**: absorve dano antes de o HP cair (ver amarra §5.5.1d) | nome e composição óbvia citam **Alho** e **Manteiga** (§5.5.2) | líder, 2026-08-25 |
| 11 | Pizza de atum | **buff pequeno em toda a party, dura até o próximo descanso** (ver amarra §5.5.1e) | nome e composição óbvia citam **Atum em lata** e **Orégano** (§5.5.2) | líder, 2026-08-25 |
| 12 | Macarrão ao molho pesto | **limpa veneno e corrosão, especificamente** (ver amarra §5.5.1f; par com macarrão ao alho e óleo) | nenhuma ligação óbvia (pesto tradicional leva parmesão, mas o NOME do item não cita o ingrediente, não presumido aqui) | líder, 2026-08-25 |

#### 5.5.1a Churrasco: amarra de preço (`economy-designer`)

**Cura de área tem de ter preço que ESCALA com o tamanho da party.** Sem essa amarra, cura
de área é matematicamente mais barata por ponto de vida entregue do que cura individual (o
mesmo consumo cura N atores em vez de 1), e vira sempre a melhor compra, o que fere §0.1
("comedido sai melhor que esbanjador, nunca empate por acidente de fórmula"). O NÚMERO da
escala (linear por membro, degrau, teto) é do `economy-designer`, na onda de
balanceamento; esta seção só fixa que a escala existe.

**Forma do item, registrada para não se reabrir sozinha:** churrasco é **item comum**, sem
restrição de transporte. O relatório anterior levantava que ele não caberia numa mochila
(sugerindo virar bônus de descanso em vez de consumível). O líder corrigiu isso, verbatim:
"Churrasco é apenas um pedaço de carne, não precisa estar no espeto para transporte."
Churrasco entra no inventário como qualquer um dos outros onze itens desta lista, sem
nenhuma mecânica de porte ou de preparo prévio.

#### 5.5.1b Pastilha de menta: amarra de escopo (`economy-designer`)

**A restrição "só overworld, não vale em combate" É o efeito, não um detalhe de
implementação.** Consumo sem custo de turno dentro de combate quebraria a Doutrina do
Comedimento (§0 de `economia.md`): viraria "come antes de qualquer ação", turno de graça
repetível sempre que houvesse pastilha em estoque, o oposto de decisão comedida. Fora de
combate, sem custo de turno para gastar, o risco de grind desaparece porque não há "turno" a
economizar no overworld. Qualquer implementação futura que porte este item para dentro do
combate sem repensar o preço fura esta amarra.

#### 5.5.1c Macarrão ao alho e óleo: amarra de precedente (`economy-designer`)

**Não é mecânica nova.** A poção `P2`, Ampola de Antídoto (`economia.md` §7.2), já entrega
"Dispel Poison/Corrode + imune a DoT por 2 turnos", o mesmo formato de imunidade
temporária a uma família estreita de efeito negativo. O macarrão ao alho e óleo usa este
precedente como PORTE DE REFERÊNCIA: escopo estreito (**um** tipo de efeito negativo, não
uma categoria inteira), duração curta. Se a imunidade sair larga (vários tipos, ou uma
categoria completa de debuff), ela neutraliza uma família inteira de inimigo que ataca por
aquele eixo; o `economy-designer` calibra o número dentro deste teto de escopo, não fora
dele.

#### 5.5.1d Pão de alho: amarra de vocabulário (`economy-designer`)

**Reusa o `Shield` já canônico** (`combat.md` §9: pool de absorção que protege o HP de
QUALQUER fonte de dano, removido ao zerar o pool ou expirar por Duration, não é status
novo). O único cuidado: o Shield do pão de alho tem de ficar **mais fraco** que o Shield da
própria ação de Defender (`combat.md` §9: "`Defend` aplica Shield com Magnitude = Def"),
senão o item comum compete com (ou substitui) a ação tática de defender. O número exato
(Magnitude, Duration) é do `economy-designer`.

#### 5.5.1e Pizza de atum: amarra de duração e o risco de virar ritual (`economy-designer`)

**A duração "até o próximo descanso" não é um número solto: ela existe porque dormir virou
mecânica própria nesta mesma rodada** (`modos-morte.md` §5, sinalização 4). Antes disso, "até o
próximo descanso" não tinha onde ancorar (dormir ainda era um "beat narrativo" genérico sem
identidade própria). Agora tem.

**Risco de degeneração nomeado, não numerado:** buff pequeno e barato, aplicado à party inteira,
é o padrão clássico de comida-buff que vira ritual obrigatório antes de toda incursão (comer
sempre, porque não comer é estritamente pior). A duração escolhida pelo líder já trabalha CONTRA
esse risco, mas não o resolve sozinha: como o efeito dura até dormir de novo, e não até a próxima
incursão nem por um número fixo de turnos, reaplicar a pizza só é necessário depois de um
descanso, não antes de cada masmorra. Isso desloca a cadência de "sempre antes de lutar" pra "só
depois de dormir", e dormir, pelo mesmo §5 do `modos-morte.md` (mesma sinalização 4), **custa
tempo** (amarra direta com o item `F4` do `TODO.md`, missão cronometrada, ideia do Gus Dragon). Se
dormir for raro e caro, comer pizza também fica raro; o ritual de pré-incursão não chega a se
formar.

**A ressalva que fica escrita para não se perder:** essa mitigação é CONDICIONAL ao resto da
mecânica de dormir, que está bloqueada aguardando a resposta do Gus Dragon (issue 8 do bus
`gusworld_ia_autocomm`, `modos-morte.md` §5 sinalização 4; cobre onde se dorme, se pode ser
interrompido, quanto tempo custa, e se o jogador usaria). Se a resposta dele tornar dormir barato,
frequente ou disponível sem custo real, o argumento acima enfraquece e a pizza volta a correr
risco de virar hábito automático. O `economy-designer` revisita esta amarra quando a mecânica de
dormir fechar de vez, não antes.

#### 5.5.1f Macarrão ao molho pesto: amarra de par e de precedente (`economy-designer`)

**Forma um par com o macarrão ao alho e óleo (#9, §5.5.1c), não uma variação dele.** Alho e óleo
já fechou como imunidade temporária a UM tipo de efeito negativo: previne, olhando pra frente.
Pesto limpa veneno e corrosão que a party JÁ tem: cura, olhando pra trás. É essa oposição de
função (um previne, o outro limpa) que dá identidade aos dois pratos de massa, não o nome
parecido ("macarrão ao X"). Registrado aqui pra ninguém, numa rodada futura, fundir os dois por
parecerem semelhantes.

**Amarra de precedente, no mesmo formato do §5.5.1c:** a Ampola de Antídoto (P2, `economia.md`
§7.2) já entrega "Dispel Poison/Corrode + imune a DoT por 2 turnos", o MESMO par de efeitos
negativos que o pesto ataca (veneno e corrosão), só que a poção crafta soma uma imunidade
temporária por cima do dispel, e o pesto (pelo que o líder decidiu: "limpa veneno e corrosão,
especificamente") só dispensa o que já está aplicado, sem cauda de imunidade. **Essa diferença de
escopo é o teto:** se uma implementação futura der ao pesto a mesma imunidade temporária que a P2
tem, o item comum passa a igualar (ou substituir) o item craftado, esvaziando o investimento em
ingrediente e craft daquela poção, o mesmo risco de dominância que o §5.5.1c já vetou pro alho e
óleo, agora incidindo sobre o par pesto/P2 em vez de alho e óleo/P2. O `economy-designer` calibra
o número (magnitude, se dispensa 1 ou os 2 efeitos de uma vez) dentro deste teto, não fora dele.

### 5.5.2 Ingredientes de craft (8)

Catálogo à parte, ditado no mesmo dia. São insumos de craft, não consumíveis de cura (não
confundir com os 18 ingredientes já canônicos do sistema de Poções/Implantes, `economia.md` §7.8,
que são de outra família temática: hardware/Selve/cripto). **Origem (§5.5.2a) e papel (§5.5.2b)
fechados nesta rodada (2026-08-25, ditado direto do líder).** Quantidade por receita, se pertencem
a uma bancada nova ou já existente, e todo número (preço, taxa de sucesso) continuam **não
decididos**: ver §5.5.2c e "Não decidido" ao fim de cada subseção.

| Ingrediente | Papel | Origem | Comida onde a ligação é óbvia pelo nome | Decisão |
|---|---|---|---|---|
| Queijo parmesão ralado | **EM ABERTO** | loja/conhecimento/exploração (§5.5.2a) | nenhuma ligação óbvia | papel não decidido; origem líder, 2026-08-25 |
| Orégano | **Temperar** | loja/conhecimento/exploração (§5.5.2a) | Pizza de atum | líder, 2026-08-25 |
| Alho | **Cozinhar** | loja/conhecimento/exploração (§5.5.2a) | Macarrão ao alho e óleo; Pão de alho | líder, 2026-08-25 (papel cozinhar); ver nota abaixo sobre 2º papel |
| Manteiga | **Cozinhar** | loja/conhecimento/exploração (§5.5.2a) | Pão de alho | líder, 2026-08-25 |
| Páprica | **Temperar** | loja/conhecimento/exploração (§5.5.2a) | nenhuma ligação óbvia | líder, 2026-08-25 |
| Atum em lata | **Cozinhar** | loja/conhecimento/exploração (§5.5.2a) | Pizza de atum | líder, 2026-08-25 |
| Tempero composto seco (cebola, alho, salsa) | **Temperar** | loja/conhecimento/exploração (§5.5.2a) | nenhuma das doze comidas cita o tempero composto pelo nome (evidência vem do próprio nome do ingrediente) | líder, 2026-08-25 |
| Camarões sem casca | **EM ABERTO** | loja/conhecimento/exploração (§5.5.2a) | nenhuma das doze comidas cita camarão | papel não decidido; origem líder, 2026-08-25 |

#### 5.5.2a Origem dos oito (decisão do líder, 2026-08-25)

Perguntado entre "só loja", "loja mais drop travado por conhecimento" e "exploração e mundo", o
líder respondeu **"2+3"**: as duas opções combinadas, não uma escolha entre elas.

1. **Loja e negociação com personagens**, mesmo canal já usado pro suco puro de limão
   (`cartas-hardware-pirataria-energia.md` §5: *"O suco puro de limão também pode ser obtido em
   lojas e negociações com personagens"*), estendido aqui à família dos ingredientes de cozinha.
   Mais **drop travado por conhecimento** para o que for raro, precedente já fixado pros outros 18
   ingredientes do sistema de Poções/Implantes (`economia.md` §7.8).
2. **Exploração do mundo**: armário, cozinha, baú, horta. Achado físico no cenário, não comprado
   nem dropado de inimigo.

**Exclusão explícita, escrita para não se reabrir sozinha:** nenhum dos oito vem de **drop de
inimigo comum (trash mob) repetido**. O corpus já fecha essa porta pros 18 ingredientes irmãos,
verbatim: *"não dá pra farmar trash mob pra conseguir componente-boss"* (`economia.md` §7.8, nota
anti-grind). A mesma frase vale aqui palavra por palavra, porque a mesma porta furaria o mesmo
corte, **C-15** (campanha de escopo fechado: farm de trash mob é tempo de jogo que a campanha não
tem), e o **Pilar 1** ("resolve por análise, nunca por grind"). Farm de inimigo comum não é fonte do
craft de cozinha; isto não é omissão a ler como brecha, é vetado pelo mesmo racional que já veta
pro resto do jogo.

**Não decidido:** qual ingrediente específico vem de qual canal específico (esta loja, aquele NPC,
aquele baú, aquela horta) é posicionamento de conteúdo/level design, não princípio; fica para
`economy-designer` e para quem desenhar os espaços de exploração. A tabela acima registra a mesma
política de origem pros oito porque foi isso que o líder decidiu: uma política uniforme pra família
inteira, não uma distribuição individual item a item.

#### 5.5.2b Papel de cada um (decisão do líder, 2026-08-25)

Perguntado se os oito craftam as doze comidas, se temperam comida já existente, se são insumo de
bateria, ou os três, o líder respondeu: **os três papéis, conforme o ingrediente.** Ele não disse
qual é qual; o mapeamento abaixo vem só de onde o próprio nome do ingrediente ou o nome do prato já
falam. Onde não é evidente, o papel fica **EM ABERTO**, não distribuído por simetria.

- **Cozinhar** (vira comida, base/fundação do prato, não aditivo de prato pronto): **Alho**
  (Macarrão ao alho e óleo: o prato É alho refogado no óleo. Pão de alho: alho+manteiga viram
  estrutura do pão assado), **Manteiga** (Pão de alho, funde e vira estrutura do pão), **Atum em
  lata** (Pizza de atum, é a proteína que dá identidade ao prato, não algo polvilhado por cima).
- **Temperar** (não cria prato, melhora um que já existe): **Orégano** e **Páprica**. Nenhuma das
  doze comidas cita a páprica pelo nome, e o orégano só aparece ligado à Pizza de atum; a evidência
  aqui não é nome de prato, é a natureza do próprio ingrediente: as duas são especiarias clássicas,
  sempre usadas em pequena quantidade pra realçar sabor de um prato já pronto, nunca como
  base/corpo dele (diferente do Alho, que É a base do prato nas duas citações que tem). **Tempero
  composto seco (cebola, alho, salsa)**: mesma lógica de evidência-pelo-nome-do-ingrediente, mais
  direta ainda. Nenhuma das doze comidas o cita pelo nome, mas o PRÓPRIO NOME do ingrediente já
  declara a natureza ("Tempero composto").
- **Insumo técnico** (vai para craft de bateria): **nenhum dos oito.** Essa família já está
  representada no catálogo por **suco puro de limão** e **água destilada**, refinados das comidas
  #3 e #4 (`cartas-hardware-pirataria-energia.md` §5), que não fazem parte deste catálogo de oito.
  Não forçado aqui por não haver evidência.
- **EM ABERTO** (nome não diz): **Queijo parmesão ralado**. Nenhuma das doze comidas o cita pelo
  nome (nem a #12, Macarrão ao molho pesto: pesto tradicional leva parmesão, mas o NOME do prato
  não cita o ingrediente, e o §5.5.1 já registrou "não presumido aqui"; a mesma cautela se aplica
  de novo, agora ao papel). Natureza (queijo ralado) não decide sozinha entre temperar e cozinhar.
  **Camarões sem casca**: nenhuma das doze comidas cita camarão pelo nome; natureza (proteína) não
  decide sozinha entre cozinhar e insumo técnico.

**Alho: papel duplo sinalizado, não confirmado.** Alho está confirmado como **Cozinhar** por
citação de nome em dois pratos, nos dois como base do prato, nunca como tempero de prato pronto. A
convenção culinária brasileira comum trata alho como base de "tempero" (refogado de alho e cebola),
mas essa é convenção cultural geral, não evidência textual deste corpus: nenhuma das doze comidas
usa alho no sentido de "tempera um prato já pronto". Fica sinalizado, não decidido: se o líder
quiser um segundo papel pro alho (temperar, além de cozinhar), é decisão dele, não inferência deste
documento.

**Não decidido:** quantidade por receita, se um ingrediente EM ABERTO acaba tendo papel único ou
duplo quando o líder decidir, e todo número de balanceamento (`economy-designer`, quando a onda
chegar lá).

#### 5.5.2c Bancada de cozinha: achado de investigação, metodologia movida para `docs/_processo/`

**Conclusão (fica aqui):** antes desta rodada não existia, em nenhum lugar do canon deste projeto,
uma "bancada de cozinha" nem sistema equivalente; o termo não é do líder, nasceu de uma opção de
pergunta de um orquestrador que ele não escolheu. A resolução final (onde se cozinha de fato) está
em §5.5.4, logo abaixo.

**A metodologia da busca** (padrões de grep testados, arquivos varridos, e a constatação de que
`DITADO-COMIDAS.md` não existe neste checkout do repositório) **foi movida para**
[`docs/_processo/investigacao-bancada-cozinha.md`](../../_processo/investigacao-bancada-cozinha.md),
**por ser processo de investigação, não spec de design (L-33).**

### 5.5.3 Fôlego do corpo × energia de ação (LACUNA FECHADA, 2026-08-25)

A versão anterior deste documento registrava como lacuna aberta se "água com gás" mirava o
**fôlego do corpo** (`core::player::WindedTimer`, `stamina.md`) ou a **energia de ação (AP)** de
combate. O líder fechou isso nesta rodada, por ditado direto: água com gás restaura **fôlego do
corpo**, não AP. A mesma leitura vale para a linha 5 (sanduíche de queijo sem queijo derretido),
que soma fôlego a HP.

Os dois recursos seguem distintos, como já eram:

1. **Fôlego do corpo** (`core::player::WindedTimer`, `stamina.md`) — a inércia da respiração
   ofegante do Gus no overworld, agora com fonte de recuperação por item (água com gás, sanduíche
   de queijo).
2. **Energia de ação (AP)** de combate — usada pela Bio-Ampola (`economia.md` §4) e pelo custo de
   troca de bateria em batalha (`cartas-hardware-pirataria-energia.md` §5, "In-battle (arena)").
   **Nenhuma** das doze comidas restaura AP; se o líder quiser um item de cura que reponha AP, é
   decisão nova, não coberta por este ditado.

Números finais (magnitude de cura, quantidade de debuff limpo, valor de fôlego restaurado, preço,
fonte de drop/compra) continuam do `economy-designer`, na MESMA curva numérica canônica já usada
para Bio-Ampola (`economia.md` §4) e Life Ampola (`economia.md` §5): nenhum decidido aqui.

**O refino e o destino do que sai das comidas 3 e 4 são canon de carta, não de crédito, e vivem em
`cartas-hardware-pirataria-energia.md` §5** ("Bateria de baixa qualidade craftada"), porque o
consumo final é energia de carta, o mesmo domínio que já governa bateria, pirataria e RSB. Este
documento é o dono do CATÁLOGO das doze comidas, dos oito ingredientes de craft e da curva de
crédito deles; o outro é o dono do que acontece quando suco de limão e água com gás viram insumo
de hardware, inclusive o trade-off entre consumir agora e guardar para refinar, e agora também da
herança de risco da bateria craftada (mesma subseção).

### 5.5.4 Onde se cozinha: cozinha do apartamento Vance (decisão do líder, 2026-08-25)

**Decisão, escolhida pelo líder entre três opções apresentadas:** cozinhar acontece na cozinha do
apartamento Vance, usando a caderneta de receitas que já é canon (`in-world-docs.md`, Documento
19, "Caderneta de saudações de Gargi Vance"). Não é sistema novo, não é "bancada de cozinha" (a
pergunta que a §5.5.2c deixou aberta está resolvida acima, apontando pra cá): é o mesmo objeto
que já existia no lore, agora com função mecânica.

**Reusado verbatim da caderneta** (`in-world-docs.md`, Documento 19, linha ~793-800): "caderneta
de cozinha barata, capa de oleado florido, ~50 páginas; receitas escritas à mão", de autoria de
Gargi Vance (mãe do Gus), encontrada na "gaveta da cozinha do apartamento Vance", "acessível desde
o início" do jogo. Este documento de economia usa esse vocabulário como âncora do local; não
redefine nem acrescenta a ele: a caderneta segue sendo canon narrativo, não canon mecânico, e
quem é dono dela continua sendo `in-world-docs.md`.

**O que NÃO é estendido aqui:** a caderneta, à parte, já tem uma trava própria de INTERAÇÃO
(Knowledge baixa, e Gus precisa ter voltado pra casa 3 vezes usando a mecânica de save base pra
destravar a interação com o objeto). Isto é fato já canônico do documento narrativo, não uma
decisão nova. Se a mecânica de COZINHAR herda esse mesmo gate ou tem o próprio é pergunta que
ainda não foi feita ao líder: fica registrada como pendência de amarração futura, não decidida
por presunção de simetria aqui. **Resolvido nesta mesma rodada, ver §5.5.5, logo abaixo.**

**Por que o líder escolheu isto contra as outras duas opções (razão dele, verbatim resumida na
ordem de serviço):** cozinhar fica preso a um lugar, como dormir, e por isso não vira hábito
repetido antes de cada incursão, o mesmo mecanismo de contenção que a amarra do §5.5.1e descreve
pra pizza, agora aplicado à AÇÃO de cozinhar, não só ao efeito do prato pronto. E amarra a comida
à casa, que é onde comida de criança mora (a própria caderneta já é lore da mãe cozinhando pro
Gus).

**Implicação de custo, sem número novo:** cozinhar exige voltar pra casa, e voltar custa tempo. O
jogador numa incursão, ou numa missão cronometrada (`missoes-cronometradas.md`, item `F4`), que
quer comer pizza fresca ou pesto recém-feito decide entre passar pelo apartamento Vance e seguir
direto: a mesma tensão que já existe pra dormir (§5.5.1e) e que o item `F4` já formaliza pro
tempo fora de batalha: "fora da batalha o relógio corre em todos os quatro modos de dificuldade"
(decisão `G5`, 24/08/2026, `missoes-cronometradas.md`). Nenhuma duração, nenhum número de minutos
é fixado aqui; isso é do `level-designer` e do `economy-designer`, juntos, na onda de
balanceamento.

**Correção de origem que fica registrada:** o termo "bancada de cozinha", que apareceu em
documento de trabalho antes desta rodada, não é do líder: nasceu de um orquestrador, numa opção
de pergunta que ele não escolheu (busca no corpus inteiro: zero ocorrências fora do §5.5.2c, que
já registrava isso como achado, não decisão). Onde esse termo aparecer em qualquer lugar do
projeto, o lugar correto é "a cozinha do apartamento Vance".

### 5.5.5 Cozinhar herda a trava de três retornos da caderneta (decisão do líder, 2026-08-25)

**Decisão:** cozinhar destrava JUNTO com a caderneta, não antes, não com portão próprio. A trava
já é canon narrativo (`in-world-docs.md`, Documento 19: "Gus precisa ter usado a mecânica de save
base canônica pelo menos 3 vezes (voltar pra casa 3x) para o jogo destravar a interação com a
caderneta", trigger "mecânica de save base + 3 retornos", "Gate: Knowledge baixa"). Esta seção
não cria trava nova: a pergunta que o §5.5.4 deixava em aberto ("se a mecânica de COZINHAR herda
esse mesmo gate ou tem o próprio") está fechada, e a resposta é herda.

**Razão do líder:** a trava já existe e já tem razão narrativa (a cumplicidade com a mãe vira
hábito antes de a descoberta acontecer, exatamente o subtexto que o Documento 19 já registra:
"voltar pra casa 3x... subtexto: a cumplicidade com a mãe vira hábito; hábito leva à descoberta").
Amarrar cozinhar a essa mesma trava evita inventar portão novo onde já existe um funcional.

**O que isto implica, sem número novo:** antes dos 3 retornos, a interação com a caderneta (e,
por consequência, o ato de cozinhar) não está disponível, nem como leitura de receita, nem como
craft de prato. Depois dos 3 retornos, os dois destravam no mesmo instante, pelo mesmo evento de
save. Não há uma segunda contagem, um segundo Knowledge gate, nem uma segunda condição narrativa
específica para o craft de comida em si.

**Não decidido:** se, depois da trava geral destravar, cada COMIDA individual (das doze do §5.5.1)
tem seu próprio requisito de desbloqueio (ver §5.5.6, que nomeia isso como lacuna própria, porque
depende de como a caderneta distribui as receitas, não de quando a interação abre).

### 5.5.6 A caderneta na mecânica: cada receita nova é uma página escrita à mão pela mãe (decisão do líder, 2026-08-25)

**A decisão:** aprender a cozinhar um prato É ler mais uma página escrita à mão por Gargi Vance.
O craft de comida não é uma bancada abstrata que "desbloqueia receita": é, mecanicamente, a
mesma caderneta do Documento 19 sendo lida uma página a mais de cada vez. O craft mais mundano do
jogo carrega, por construção, a ausência dos pais (a mãe que escreveu a receita; os bilhetes do
pai itinerante dobrados entre as páginas da mesma caderneta), **sem nenhuma linha de diálogo
explicando isso.** O jogador não é informado "esta receita é especial porque sua mãe a escreveu";
ele simplesmente lê a letra dela toda vez que aprende a cozinhar algo novo, e o jogo confia nisso
para carregar o peso, não numa cutscene.

**Por que isto é mecânica e não nota de rodapé:** o Documento 19 já faz esse mesmo trabalho
silencioso para o Pyotor: a "Função" do documento registra que os bilhetes "humanizam Pyotor
(presente em ausência via cartas raras)" sem precisar de uma fala explicando o vazio que ele
deixa. A decisão de hoje estende a MESMA técnica (presença por artefato, não por exposição) ao
lado da Gargi: cada página de receita nova é, para ela, o que cada bilhete já era para o Pyotor.
Repetir a ação de cozinhar, a cada prato novo do catálogo de doze, é repetir o contato com a
letra da mãe, de novo e de novo, ao longo do jogo inteiro, não uma única vez num flashback.

**O que este documento NÃO inventa (L-11: decisão é do líder, não do agente):**

- **Nenhum conteúdo de receita.** O Documento 19 mostra três receitas de exemplo (Pão-de-bit com
  canela, Café-de-neurônio, Caldo-de-folha-fractal) e nenhuma delas é uma das doze comidas deste
  catálogo (§5.5.1). Não presumido aqui que as doze comidas SÃO essas três, nem que substituem
  elas: é uma lacuna, ver abaixo.
- **Nenhum bilhete novo do Pyotor.** Os quatro bilhetes já são canon fechado do Documento 19,
  datados (-7, -4, -2, -0.5) e com conteúdo próprio; este documento não acrescenta um quinto nem
  reescreve os quatro existentes.
- **Nenhuma contagem de páginas por comida, nem ordem de aprendizado.** A caderneta tem ~50
  páginas no total (Documento 19); quantas dessas páginas mapeiam para as doze comidas, se é uma
  página por comida ou mais, e em que ordem elas se tornam legíveis (pela ordem da tabela §5.5.1,
  por narrativa, por Knowledge, livre) não está decidido em canon nenhum.

**Lacunas nomeadas, para o líder decidir quando quiser (não preenchidas aqui):**

1. Qual das doze comidas corresponde a qual página/receita da caderneta, se a correspondência é
   1-para-1, ou se algumas comidas não têm página própria (aprendidas de outro jeito) e outras
   páginas da caderneta não correspondem a nenhuma das doze (só lore, sem craft).
2. Quantas páginas por receita, e se isso varia por prato.
3. A ordem em que as doze são desbloqueadas: linear pela ordem da tabela §5.5.1, por escolha livre
   do jogador, ou amarrada a outro marco narrativo.
4. Se os quatro bilhetes do Pyotor interferem no ritmo de aprendizado: o Documento 19 já descreve
   um bilhete encartado "no meio do caderno, receita interrompida pela metade": se isso significa
   que uma receita específica fica mecanicamente bloqueada até certo ponto da história por causa
   do bilhete, ou se é só ambientação de texto sem efeito de craft, não foi perguntado ao líder.
5. Se, além da trava geral de três retornos (§5.5.5), cada receita individual tem um segundo gate
   de Knowledge próprio, ou se todas as doze ficam disponíveis de uma vez assim que a trava geral
   abre.

**Quem cruza o quê:** o mapeamento página↔receita, se e quando o líder decidir, cruza
`in-world-docs.md` Documento 19 (lore, propriedade do time de narrativa; este documento só lê,
nunca edita) com este arquivo (mecânica de craft). Nenhum dos dois lados decide sozinho: narrativa
não inventa efeito mecânico de prato, mecânica não inventa conteúdo de página.

---

**Última revisão:** 2026-08-25, extração de `economia.md` §5.5 (L-33: comida e economia de
crédito mudam por razão diferente), mais as duas decisões novas da mesma rodada: cozinhar herda a
trava de três retornos da caderneta (§5.5.5) e a caderneta entra na mecânica, cada receita nova
sendo uma página escrita à mão pela mãe (§5.5.6). Nenhum número novo introduzido nesta revisão.
Histórico anterior a esta extração (doze comidas, oito ingredientes, fôlego × AP, onde se cozinha)
preservado inteiro acima, sem edição de conteúdo; só a §5.5.2c foi encurtada, com a metodologia
de busca movida para `docs/_processo/investigacao-bancada-cozinha.md`.
