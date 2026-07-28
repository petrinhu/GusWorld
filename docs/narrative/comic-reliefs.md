# Alívios Cômicos (GusWorld)

Cenas de humor LucasArts-like (Monkey Island, Amazon Queen, Day of the Tentacle, Sam & Max): timing, gravidade absurda aplicada a banalidades, paródia, meta-humor leve, **sem palavrão e sem fan-service** (Pillar 4). Cada cena ancorada em meme/cultura de programação reconhecível mas diegética ao mundo.

**Princípios:**
- Beats curtos. Pausas medidas. Reações silenciosas valem mais que falas.
- Personagem sério dizendo absurdo OU personagem ridículo dizendo verdade.
- Sem quebrar 4ª parede explícito; sempre ancorado no mundo (linguagens fictícias canônicas: C-Arcane, Asmódico, Óxido, Pythia).
- Distribuir ~8-12 cenas ao longo do jogo, **uma por capítulo** (ritmo).
- Algumas plantam foreshadow (Dante traidor) ou reforçam theme (Sterling = bug declarado feature).

---

## Cena 1: "Sexta-feira, cinco da tarde" (deploy de produção)

**Contexto:** Ato 1, oficina central. Cauã acabou de testar um conjuro novo.

```
[INT. BANCADA DE COMPILAÇÃO, CIDADE, 17h00]

CAUÃ se aproxima da bancada de produção carregando um Token novo, animado.

CAUÃ
Terminei de testar! Vou subir pro Codex de produção
agora pra gastar no próximo encontro!

GUS, sem levantar a cabeça do esquema.

GUS
Que dia é hoje, Cauã?

CAUÃ
Sexta. Por quê?

GUS
Que horas são?

CAUÃ
[olhando o relógio]
Cinco. (pausa) Cinco em ponto.

[GUS larga o estilete devagar. Levanta-se. Olha CAUÃ nos
 olhos por três beats. Fundo: trovão distante.]

GUS
[sussurro]
Cauã.

CAUÃ
...sim?

GUS
Sexta-feira. Cinco da tarde.

CAUÃ
Sim. E?

GUS
[gravidade total]
Nunca. NUNCA subiremos código pra produção
numa sexta-feira às cinco da tarde.

CAUÃ
Mas eu testei tu...

GUS
Você testou em ambiente local. Produção... produção é
outro bicho.

[Trovão mais forte.]

CAUÃ
Não tinha trovão antes.

GUS
Existe sempre, Cauã. Só não escutamos.

[BENTO entra carregando o escudo-catedral. Para. Avalia
 a cena com gravidade.]

BENTO
Sexta-feira, cinco da tarde, e alguém vai subir código?

GUS
Eu o detive a tempo.

BENTO
[fazendo o sinal do compilador no peito]
Que os Anciões do Asmódico te abençoem, garoto.

[CAUÃ olha do escudo de BENTO pro estilete de GUS pro Codex
 ainda na própria mão.]

CAUÃ
...vou subir segunda?

GUS
Subiremos terça. Segunda também morre coisa.
```

**Payoff:** call-back ao meme dev "deploy só na terça". Gravidade religiosa de Bento eleva tom.

---

## Cena 2: "Tabulações vs Espaços" (a guerra santa)

**Contexto:** Catedrais de Neo-Sylvania, sessão de treinamento. Bento ensina Gus a escrever Asmódico.

```
[CATEDRAL, sala de estudos. Cronômetro de latão tique-taqueia.]

BENTO
Cada instrução do Asmódico é separada por... TABULAÇÕES.

GUS
Hmm. Posso usar espaços?

[Silêncio mortal. Pássaros mecânicos voam dos vitrais.]

BENTO
[lento, perigoso]
Repete.

GUS
Espaços. Em vez de tab. Quatro espaços por nível.

[BENTO ergue o escudo. Encara GUS por longos segundos.]

BENTO
Se eu te dissesse, garoto, que **três gerações** da Ordem
do Asmódico morreram defendendo a TABULAÇÃO contra os
heréticos do ESPAÇO...

GUS
[recuando meio passo]
Eu... não sabia que era pessoal.

BENTO
Cada espaço que você digita é um soluço na alma do
compilador-mestre.

GUS
Mas o Pythia da Jaci aceita os do...

BENTO
PYTHIA SEQUER COMPILA. Pythia é POESIA, não engenharia.

[JACI passa por trás carregando ampolas. Pausa.]

JACI
[sussurro pra Gus]
Eu ouvi. Use o que você quiser. Pythia te entende.

[BENTO finge não ter ouvido.]
```

**Payoff:** estabelece linguagem-conflito como recorrente. Jaci aliada do Gus na flame war.

**Notas de leitura (decisões do criador, 2026-07-26):**

- **"PYTHIA SEQUER COMPILA" é ERRO DELIBERADO do Bento. NÃO CORRIGIR.** Ao pé da letra a frase é falsa (Pythia compila para bytecode). Ela fica **porque o Bento é um garoto de 14 anos em surto retórico, defendendo diante do grupo a linguagem que é a identidade dele**, e não um manual. A frase anterior dele ("cada espaço que você digita é um soluço na alma do compilador-mestre") já marca o registro: quem fala assim não faz ressalva técnica. **O certo é ensinado na própria cena, por contraponto:** a Jaci responde na hora ("use o que você quiser, Pythia te entende") e ele finge não ter ouvido. Uma versão tecnicamente correta desta fala seria uma frase que o Bento não diria. _(Registro honesto: em 2026-07-26 esta fala chegou a ser trocada por "PYTHIA NEM VIRA MÁQUINA" e o criador **reverteu no mesmo dia**, por este motivo. Se você acha que achou um bug aqui, leia a regra dos pesos 5 e 2 em "Regras de tom" antes de mexer.)_ A [Cena 15](#cena-15-pergunta-amanhã-o-cauã-descobre) depende da **substância** desta fala, não da redação, e sobreviveu ao ajuste e à reversão sem retrabalho.
- **A tabulação do Asmódico é liberdade deste mundo, não erro.** Assembly real não é sensível a tabulação (isso é Python e Makefile), mas **Asmódico não é assembly**: é o análogo dele aqui, e tem regra própria. Registrado para ninguém "corrigir" esta cena no futuro achando que achou um bug.

---

## Cena 3: "Funciona no meu Drive"

**Contexto:** Dutos Infernais, puzzle elétrico. Conjuro do Cauã falha repetidas vezes.

```
[DUTOS, porta selada por código. Cauã tenta executar 3 vezes. Falha.]

CAUÃ
O conjuro funciona no MEU Tavus-Drive! Eu juro!

GUS
[calmamente, anotando no Diário]
Acredito em você.

CAUÃ
Então por que a porta não abre?!

GUS
A porta exige interpretação consistente.
Seu Drive interpreta diferente do dela.

CAUÃ
Mas FUNCIONA NO MEU!

[CAUÃ chuta a porta. Porta abre.]

CAUÃ
[radiante]
TÁ VENDO?

GUS
[anotando no Diário]
"Solução: chute. Reprodutível?: incerto."
```

**Payoff:** wisdom dev clássica. Gus mantém rigor científico até no humor.

---

## Cena 4: "Force Push" (foreshadow Dante traidor)

**Contexto:** Periferia Industrial (Ferrovelhos). Dante faz manutenção no aparelho ortodôntico do Gus. **Plantar suspeita**; dispara só após 3 recrutamentos de Ato 2 completos (~60%; conta só recrutamentos, não o retorno do Cauã) (D.1).

```
[OFICINA DE DANTE, bancada coberta de sucata. Gus deitado, boca
 aberta. Dante ajusta com dedo-ferramenta modular.]

DANTE
Tô só sobrescrevendo a configuração antiga.
Push forçado, mais limpo.

GUS
[alerta, sentando-se]
Espera. Force push?

DANTE
Sim, é mais rápido. Histórico fica limpo.

GUS
Dante, na Ordem do Asmódico, force push é considerado...

DANTE
...heresia, eu sei. Mas a gente não é Asmódico aqui, é?

[Beats. Dante não olha pra Gus.]

DANTE
Bento se preocupa demais com tradição.
Histórico é peso morto.

GUS
[sem se mover]
Histórico é como a gente sabe pra onde a coisa vai.

DANTE
[continua o ajuste, em silêncio]

[GUS engole. A cena segue normal. Mas o jogador percebe.]
```

**Payoff:** foreshadow planted. Dante despreza histórico = despreza tradição/consequência = futuro traidor. Cena cômica na superfície, ominosa no subtexto.

---

## Cena 5: "Expressão Regular" (Iara apaixonada por opacidade)

**Contexto:** Setor Mirage. Iara decifrou um conjuro do inimigo escrito em Óxido. Quer mostrar pro Gus.

```
[BECO HOLOGRÁFICO. Iara projeta o conjuro decodificado.]

IARA
[orgulhosa]
Olha que beleza. Quatro caracteres fazem o trabalho
de oitenta linhas em C-Arcane.

  ^[\x{22D3}-\x{A78B}]+(?<lumen>[0-9]+)$

GUS
[cabeça inclinada 30 graus]
Iara, isso é... legível?

IARA
Pra mim, é poesia.
Pra você, é múmia falando.

GUS
Posso escrever três conjuros aninhados que fazem
a mesma coisa?

IARA
Três conjuros que fazem o mesmo que UMA expressão regular?

GUS
Sim, mas eu vou conseguir LER em seis meses.

IARA
[sussurro provocante, andando devagar]
Aprenda. Óxido recompensa a paciência.

[GUS encara o conjuro mais um segundo. Suspira.]

GUS
Vou ficar com meus três conjuros.
```

**Payoff:** clássico "regex is write-only". Iara representa estética de elegância opaca; Gus, legibilidade pragmática.

---

## Cena 6: "Bug declarado Feature" (Sterling propaganda)

**Contexto:** Cidade GusWorld, atravessando praça. Holograma corporativo gigante exibe propaganda da Sterling Corp.

```
[HOLOGRAMA gigante mostra Sterling Locke estilizado, sorrindo frio.]

VOZ STERLING (gravação)
O Global Runtime Environment não tem bugs.
Tem... comportamentos emergentes inesperados.

GUS
[parado na praça, irritado]
Isso. É. Bug.

LINDA
[passando junto, fones nas orelhas]
Mas se ele declarar que é feature, vira feature?

GUS
Esse é exatamente o tipo de raciocínio que destruiu
a Apex-Data Systems.

LINDA
[cantarolando, fones]
Bug é feature, débito é capital,
Chapter 11 é renovação institucional...

GUS
Para. Você está pensando como Sterling.

LINDA
[congela, assustada]
Que horror.

[Linda corre pra longe do holograma. Gus segue caminhando.]
```

**Payoff:** reforça theme (B.2) "inteligência serve à vida": Sterling = inteligência serve a si própria, distorce realidade via linguagem. Linda como personagem que captou rápido.

---

## Cena 7: "Tomo da Pilha Sobrecarregada"

**Contexto:** Setor Mirage ou cidade. Gus consulta um tomo antigo no Diário.

```
[BANCADA, Gus folheia tomo grosso. Cauã se aproxima curioso.]

CAUÃ
Que livro é esse?

GUS
"Tomo da Pilha Sobrecarregada: Respostas
dos Anciões a Perguntas Pequenas."

CAUÃ
Sério?

GUS
Centenas de Anciões enfrentaram problemas como o nosso.
Documentaram. Tudo. Aqui.

CAUÃ
[abre página aleatória]
"Pergunta de 2089: como inverter uma lista?
 Resposta: duplicata. Já respondida em 2079."

GUS
Os Anciões são severos.

CAUÃ
[outra página]
"Pergunta: como começar?
 Resposta: encerrada por ser vaga demais."

GUS
A Pilha Sobrecarregada não é gentil. Mas é precisa.

CAUÃ
Aqui tem uma pergunta de 2025:
"Por que minha rotina não compila?"
Resposta de 2025: "RTFM."
Resposta de 2026: "Marcado como inseguro."
Resposta de 2089: "Use Pythia."

GUS
[sorri pouco]
Os Anciões evoluem.
```

**Payoff:** Stack Overflow personificado. Cauã engatilha cada vez melhor que o anterior; payoff "Use Pythia" como vitória da Jaci na flame war. **Nota de leitura: a vitória é de ergonomia (é a resposta mais fácil de escrever e de ensinar), nunca de execução. O eixo compilado/interpretado não se move aqui.**

---

## Cena 8: "Comentários em Latim"

**Contexto:** Catedrais. Bento revisa um conjuro do Gus.

```
[ATELIÊ DE CONJUROS, Bento examina pergaminho do Gus.]

BENTO
Onde estão os comentários?

GUS
Está auto-documentado. Os nomes das variáveis explicam tudo.

BENTO
[escandalizado]
Auto-documentado? AUTO?

GUS
Sim. `frequência_ressonante_da_matriz` significa
frequência ressonante da matriz.

BENTO
Garoto. Garoto. Se eu morrer amanhã, quem vai
entender este código?

GUS
...eu?

BENTO
E se VOCÊ também morrer?

GUS
[longa pausa]
Eu... vou comentar.

BENTO
[severo]
Em latim. Como manda a tradição.

GUS
Em PORTUGUÊS, Bento.

BENTO
[suspira, derrotado pela modernidade]
Em português. Pela última vez.

[Bento sai murmurando "Lasciate ogne tradizione" em
 latim baixinho.]
```

**Payoff:** Bento como guardião de costumes mortos. Tradução defensiva linha-a-linha.

---

## Cena 9: "Off-by-one" (a tragédia menor)

**Contexto:** Selve Sombria. Jaci conta ampolas pra fazer antídoto.

```
[CLAREIRA, Jaci abre o jaleco, conta ampolas no slot.]

JACI
Onze. Preciso de dez. Sobrou uma.

GUS
Você não tinha doze faltando duas?

JACI
[pausa, conta de novo]
...dez. Preciso de onze. Falta uma.

GUS
Você não disse onze faltando uma?

JACI
[conta terceira vez]
Falta uma.

GUS
Falta uma do quê?

JACI
[encarando o slot]
Eu não sei mais.

GUS
[abre o Diário, anota]
"Erro de contagem em loop biológico.
 Hipótese: off-by-one da Jaci."

JACI
NÃO É OFF-BY-ONE.

GUS
Faltou uma?

JACI
[silêncio]
Faltou uma.
```

**Payoff:** off-by-one é o bug mais clássico. Personagem brilhante caindo no mais bobo = humor universal dev.

---

## Cena 10: "Não é magia, é cache"

**Contexto:** Cauã testa um conjuro rápido. Funciona instantâneo. Tenta de novo, falha.

```
[DUTOS INFERNAIS, Cauã se exibe.]

CAUÃ
Olha esse novo conjuro: BAM!
[ataque dispara em zero tempo]

GUS
Impressionante velocidade.

CAUÃ
De novo: BAM!
[ataque demora 3 segundos]

GUS
...

CAUÃ
Por que demorou agora?

GUS
Não é magia, Cauã. É cache.

CAUÃ
Cash? Você quer dinheiro?

GUS
Não. Cache. Memória rápida.

CAUÃ
Tá. E?

GUS
A primeira vez foi compilado e guardado.
Segunda vez, o cache invalidou. Recompilou.

CAUÃ
Então como faço pra ser sempre BAM?

GUS
Você não invalida o cache.

CAUÃ
Como invalido o cache?

GUS
Cauã, dos dois problemas mais difíceis em ciência da
compilação, um é nomear coisas; o outro é invalidar cache.

CAUÃ
Qual é o terceiro?

GUS
Off-by-one.

[JACI passa, ouve. Murmura "EU NÃO TIVE OFF-BY-ONE" e segue.]
```

**Payoff:** quote clássico "two hard problems in CS"; call-back à cena 9 (Jaci ofendida).

---

## Distribuição sugerida pelos atos

| Ato | Cenas |
|---|---|
| Ato 1 (introdução cidade) | 1 (sexta-feira), 6 (Sterling propaganda) |
| Ato 2 (incursões companions, distribuídas conforme recrutamento) | 2 (tabs vs espaços, após recrutar Bento), 3 (funciona no meu, Cauã), 5 (regex, Iara), 7 (Pilha Sobrecarregada), 9 (off-by-one, Jaci), 10 (cache, Cauã) |
| Ato 2 (após 3 recrutamentos, ~60%) | 4 (force push, foreshadow Dante) |
| Ato 2/3 transição | 8 (comentários latim, Bento) |

Adicionar conforme o arco progride. Limite: máximo 1 cena cômica por capítulo (não saturar).

---

## Regras de tom

- **Sem cinismo adulto.** Humor de Gus é seco mas inocente. Companions reagem com surpresa real.
- **Sem ridicularização gratuita.** Cada companion tem dignidade; humor sai da SITUAÇÃO, não de DEFEITO de personagem.
- **Bento NUNCA é piada do "velho fora de moda".** Ele tem razão tanto quanto Gus em metade dos casos. Tensão é genuína.
- **Sterling NUNCA é cômico.** Mantém a frieza ameaçadora. Comédia em torno dele é sempre satirizando a propaganda dele.
- **Patch-Zero NUNCA é cômico.** Terror puro.
- **Linguagem profana zero** (Pillar 4.3). "Que horror" é o palavrão máximo.

### Conceito correto x personagem defendendo o que pensa (regra dos pesos, criador 2026-07-26)

**A regra é ensinar o certo.** Sempre que possível, e é o que o jogo entrega na maior parte das vezes.

**A exceção:** quando aparecem **pontas soltas, falhas de sincronia ou de congruência** com conceitos sólidos, entra o argumento das **crianças disputando espaço**. Elas erram. São humanas, não são computadores, e criança tem mais margem para dissonância cognitiva: defende o que pensa mesmo quando no fundo sabe que está errada, porque precisa de engajamento social. Essa exceção **traz alma aos personagens** e costuma ser o que costura pontas soltas do canon.

**Os pesos, para decidir no caso concreto:**

| O que está em jogo | Peso |
|---|---|
| Conceito correto | **5** |
| Tentativa social de manter o argumento (personagem defendendo o que pensa) | **2** |

Ou seja: o certo ganha por mais de dois para um, e **não se escreve fala errada por preguiça ou por efeito fácil**. Mas quando o erro do personagem serve para dar alma, marcar quem ele é diante do grupo, ou costurar uma incongruência que já existia, ele **fica** e vira material.

**PROTOCOLO OBRIGATÓRIO (criador, 2026-07-26): erro técnico em fala de personagem NÃO se corrige e NÃO se decide sozinho. Leva-se ao criador, e a apresentação DEVE incluir a opção de justificativa social** (manter a fala porque o personagem está defendendo o que pensa diante do grupo), ao lado da opção de corrigir. Quem revisa **levanta** o achado; quem decide é o criador. Vale para qualquer agente e para o orquestrador.

**Corolário prático, para quem for revisar canon:** achar um erro técnico na boca de um personagem **não é autorização para corrigi-lo**. Verifique primeiro se o certo é ensinado ali por contraponto (outro personagem responde, o resultado desmente, a cena seguinte mostra o custo). Se é, a fala fica. **Uma frase tecnicamente impecável na boca de um garoto de 14 anos em surto retórico é uma frase que ele não diria.** Este é o gate que faltou uma vez, em 2026-07-26, e que fez uma fala canônica ser "consertada" e revertida no mesmo dia.

**A régua "fato: proibido mentir" continua exclusiva dos apartes do Gus** (`gus-apartes-c-arcane.md`), e é exclusiva de propósito: ali ele é o veículo didático, e o rigor é o ponto. Os outros personagens são humanos e erram.

---

## Cenas de guerra de plataforma

Convenções diegéticas pra evitar marca registrada:

| Referência real | Nome diégetico | Origem |
|---|---|---|
| Windows | **Janelarum** | "janela" + sufixo latim arcaico |
| Linux | **Pinguium** | latim de "pinguim" (mascote Tux) |
| Nintendo Switch | **Comutador** | literal switch + simbolismo de transição |
| PlayStation | **Tocador-Trono** | Play+Station = Tocar+Trono, soa pomposo |

**Regra:** Pinguium sempre vence Janelarum. Comutador sempre vence Tocador-Trono. Vitória por argumento, não por bullying.

---

### Cena 11: "Janelarum travou de novo"

**Contexto:** Periferia Industrial. Dante consertando terminal de um aliado mecânico.

```
[OFICINA DE DANTE, terminal antigo de aliado piscando azul.]

ALIADO MECÂNICO
Travou de novo. Janelarum trava toda semana.

DANTE
Eu te falei pra trocar.

ALIADO
Eu sei. Mas era de graça.

DANTE
Era de graça porque eles cobram depois.

GUS
[se aproxima, olha o terminal]
Mensagem de erro?

ALIADO
"O Janelarum precisa reiniciar pra terminar de iniciar."

GUS
[longa pausa]
Isso é uma frase válida pra eles?

DANTE
É, pelo visto.

GUS
[abre o Diário, anota]
"Sistema-operacional vendido como produto que precisa
 ser reiniciado pra começar a ser produto."

ALIADO
E o que você sugere?

GUS
Pinguium.

ALIADO
Mas é difícil de instalar.

GUS
Instale uma vez. Não reinicie nunca mais.

[Aliado encara o terminal. Encara Gus. Encara Dante.
 Suspira longo.]

ALIADO
Como se chama o pinguim mascote?

GUS
Não tem mascote oficial. Tem mil. Cada distribuição
escolheu o próprio pinguim.

ALIADO
Eu queria só um.

GUS
Liberdade tem custos.
```

**Payoff:** classic "Windows needs restart" + "Linux distro fragmentation" mas Gus elogia liberdade. Sem hostilidade.

---

### Cena 12: "Atualização obrigatória"

**Contexto:** Setor Mirage. Iara tenta executar conjuro num terminal público Janelarum.

```
[BECO MIRAGE, terminal de pesquisa publicitária piscando.]

IARA
[digitando rápido]
Só preciso 10 segundos pra consultar a database.

[TELA: "Atualização do Janelarum em progresso. Não
 desligue. 17% concluído."]

IARA
Era pra fazer isso às 3 da manhã.

[TELA: "32% concluído. Reiniciando."]

IARA
Reiniciando no MEIO da atualização?

[TELA: "Atualização falhou. Restaurando versão anterior."]

IARA
Ótimo.

[TELA: "Restauração falhou. Atualizando."]

GUS
[aparece atrás dela]
Quanto tempo?

IARA
Quarenta minutos atrás eu queria 10 segundos.

GUS
Posso te emprestar meu terminal pessoal.
Roda Pinguium.

IARA
Demora pra atualizar?

GUS
Atualiza enquanto eu trabalho. Em segundo plano.
Sem reiniciar.

IARA
[encara o terminal Janelarum]
[encara Gus]
Empresta.
```

**Payoff:** Windows Update meme universal. Conversion sutil de Iara (não frontal) pra Pinguium.

---

### Cena 13: "Linha de comando" (peer pressure)

**Contexto:** Cauã vê Gus operando terminal Pinguium puro.

```
[BANCADA, Gus digitando comandos rápidos no terminal.]

CAUÃ
Como você sabe TUDO ISSO de cabeça?

GUS
Eu não sei. Eu pergunto.

CAUÃ
Pra quem?

GUS
[digita: `homem conjuro`]
Pro manual.

[Tela enche de texto explicativo.]

CAUÃ
[boquiaberto]
O Janelarum também tem manual?

GUS
Tem. Eles colocam num PDF de quinhentas páginas
num site separado.

CAUÃ
[longa pausa]
Por quê?

GUS
[encolhe os ombros]
Pra vender certificação.

CAUÃ
[sussurro horrorizado]
Vender o MANUAL?

GUS
Não o manual. A prova de que você leu o manual.

CAUÃ
[escondendo o Tavus-Drive Janelarum-compatível atrás das costas]
Quero ver mais comandos.
```

**Payoff:** `man pages` classic + certífication grift Microsoft. Cauã convertido por horror.

---

### Cena 14: "Comutador vs Tocador-Trono"

**Contexto:** Cauã e Linda no quartel-general, debatendo durante intervalo.

```
[SALA COMUM, Cauã e Linda passando o tempo. Iara ouve.]

CAUÃ
O Comutador tem aquele jogo do encanador.

LINDA
Tocador-Trono tem gráficos melhores.

CAUÃ
Tem. Mas não tem o encanador.

LINDA
Tem o encanador de mascote ELES.

CAUÃ
Não, o ENCANADOR encanador. O original.

LINDA
[pausa]
É verdade.

CAUÃ
E o jogo da fazenda. E o jogo da garota da montanha.
E o jogo dos bichinhos coletadores.

LINDA
Eu admito, a biblioteca é boa.

CAUÃ
E você pode JOGAR DEITADA.

LINDA
[para tudo]
Espera. Como assim deitada?

CAUÃ
Você solta da TV e leva.
Joga deitada no portátil.

LINDA
[encara Cauã]
Você tá me dizendo que o Comutador é console
E portátil ao mesmo tempo?

CAUÃ
Eu não disse desde o começo?

LINDA
[se levanta, decidida]
Onde compra um Comutador?

IARA
[do canto]
Vai pagar resgate. Eles vendem o jogo
do encanador pelo mesmo preço de cinco
anos atrás.

LINDA
Eu pago.

IARA
Eu sei que você paga. Eu pagaria também.

GUS
[passando]
É monopólio simpático, mas é monopólio.

LINDA
Mas é SIMPÁTICO.

GUS
[suspira]
É. Eu sei. Eu também comprei.
```

**Payoff:** hybrid console hook + Nintendo evergreen pricing meme. Até Gus admite.

---

## Homenagens diégeticas

Referências reconhecíveis sem violação de marca. Cada uma redirige pra elemento diégetico do mundo GusWorld. **Aparecem como flavor opcional, não críticas ao caminho principal.**

### Hollow Knight (2)

**EE-1: O Cavaleiro Silencioso**

Em algum corredor dos Dutos Infernais, Gus encontra estatueta de inseto antropomórfico de armadura escura carregando agulha-espada. Placa: *"Em memória de um reino enterrado sob a cidade que veio antes. O cavaleiro silencioso ainda sonha."*

Diary entry registra: *"Existe um reino abaixo da cidade. Não consigo descer. Quem sabe um dia."*

**EE-2: O Cartógrafo Esquecido**

NPC inseto com lentes redondas vende mapas dos Dutos Infernais por moedas. Fala:
> "Compre o mapa primeiro. Explorar sem mapa é desperdício de juventude."

Se Gus tenta atravessar a região sem comprar, o NPC suspira: *"Você vai voltar."*

---

### Super Mario (2)

**EE-3: O Encanador do Cano Verde**

Cano vertical verde-musgo aparente na Periferia Industrial. NPC com macacão azul + camiça vermelha + bigode farto fala:
> "Princesa? Não, não tem princesa aqui. Mas tem RATO. E tem ENCANAMENTO. É quase a mesma coisa."

Gus pode pular em cima de um rato pequeno tipo cogumelo-com-pernas. Rato vira pancake. NPC dá thumbs-up.

**EE-4: O Cogumelo-Recuperador**

Item raro dropável em zonas de bioma misto (final ato 2): cogumelo verde brilhante. Usar = +1 vida extra ao Gus (revive automático se HP=0 na próxima batalha). Descrição no Diário:
> *"O Cogumelo-Recuperador. Lendário. Item raro. Existe um meme antigo que diz: 'A vida é sua. Mas o cogumelo te dá outra.'"*

---

### Zelda (2)

**EE-5: O Velho do Porão**

Numa caverna escondida da Selve Sombria, NPC de capa verde + chapéu pontudo + barba branca oferece Token raro:
> "É perigoso ir só. Leve isto."

Token: **Espada-Vetor** (causa dano em onda em linha reta). Visualmente, lembra uma espada pixelada de 8-bit. NPC desaparece quando Gus volta.

**EE-6: O Pedestal Triangular**

Em ruína profunda na Selve, há pedestal vazio com três marcas triangulares (sabedoria, poder, coragem). Gus interage:
> *"Vazio. Os três pedestais foram preenchidos por alguém antes."*

Diary recebe entry oculto: *"Três heranças. Três portadores. Um deles ainda dorme."* Som de corônica curta toca (tchá-na-na-na-an) quando lido.

---

### Terraria (2)

**EE-7: O Guia**

NPC residente fixo na Periferia Industrial chamado simplesmente **O Guia**. Responde qualquer pergunta com:
> "Pra começar a derrotar o Olho Esculpido, você precisa de armadura de cobre. Pra fazer armadura, você precisa minerar. Pra minerar, você precisa de picareta. Pra fazer picareta..."

Loop infinito. Gus eventualmente sai. NPC continua falando.

**EE-8: O Olho Esculpido**

Mini-boss opcional na fronteira-Selve. Olho gigante voador, vermelho. Drop: **Token-da-Vísion-Antiga** (revela toda telegrafia inimiga por 3 turnos). Diary:
> *"Algumas culturas honram olhos como demônios. Aqui ele é caaçável."*

---

### Pokemon (2)

**EE-9: "Pegue Todos os Bytes"**

Conquista oculta destrava quando Gus tem 100 entradas completas no Bestiário do Diário:
> **"Coletor de Bytes: Você documentou 100 inimigos. Os Anciões da Pilha Sobrecarregada te reverenciam."**

Som curto de captura: *"plink-plink-plink-clic."*

**EE-10: A Placa do Hospital**

Placa na entrada do hospital onde companions são curados:
> "Bem-vindo ao Centro de Recuperação. Seus aliados serão totalmente restaurados.
>
> Aguardamos seu retorno."

Em letras pequenas embaixo: *"P.S.: A cura gratuita demora. Doações aceleram o processo. (Diretora Joana)"*

---

### Geometry Dash (2)

**EE-11: Cubo Neon do Ritmo**

Mini-game opcional no Setor Mirage: Iara desafia Gus a um puzzle de avoidance ritmado. Avatar = cubo neon ciano. Spike pontiagudo magenta no caminho. Trilha eletrônica acelera. Vencer 3 leveis seguidos dá **Token-de-Sincronia** (efeito timing input bonus extendido).

**EE-12: "Você morreu 100 vezes na mesma tela"**

Conquista oculta:
> **"Persistência Geométrica: Você morreu 100 vezes na mesma cena.
>  Nós respeitamos isso."**

Achievement banner aparece com fundo de quadrados neon piscando.

---

### Stardew Valley (2)

**EE-13: A Fazenda da Avó**

No vilarejo da Jaci (fronteira-Selve), terreno abandonado com casa rústica. NPC mais velha:
> "Era da sua avó. Você pode plantar nela, se quiser."

Mini-sistema opcional: Gus pode plantar sementes Token-comum durante uma noite no vilarejo, colher de manhã (in-game time skip). Drop: **Cristal-de-Colheita** (Token raro de cura). NPC entrega carta:
> *"Sua avó deixou isto. Ela dizia que toda lógica começa numa horta."*

**EE-14: O Pelicano**

Placa na entrada do vilarejo:
> **VILAREJO DO PELICANO BRANCO**
>
> *População: 47. Estações: 4 por ano. Bondade: ilimitada.*

NPC saudando: *"Olá! Sou o prefeito. Antes era difícil. Agora é só plantar e esperar."*

---

### Mario Kart (2)

**EE-15: A Corrida do Cogumelo**

Mini-game opcional liberável no Setor Mirage: corrida de pista usando exo-botas do Cauã. Itens coletáveis no chão:
- **Casca-Verde** (atinge linha reta à frente)
- **Casca-Vermelha** (segue alvo)
- **Casca-Azul** (atinge o líder, devastador)
- **Banana-Sintética** (escorregão)

Win condition: top 3 em 8 corredores. Drop: **Token-Trovão** (acelera própria iniciativa em combate por 2 turnos).

**EE-16: "Casca Azul"**

Diálogo casual entre Cauã e Linda durante descanso:

```
CAUÃ
Você sabe daquela corrida no Mirage que a gente perdeu
pelo Comutador-Antigo do Tao?

LINDA
Não fale.

CAUÃ
Eu estava em primeiro a 50 metros do final.

LINDA
Não. FALE.

CAUÃ
Casca azul.

LINDA
[silêncio longo]

CAUÃ
A casca azul é a inveja institucionalizada.
```

---

### Celeste (2)

**EE-17: A Garota da Torre**

Na Catíral de Neo-Sylvania mais alta, Gus encontra NPC garota cabelo vermelho usando moletom roxo, escalando degrau a degrau. Suspirando. Olha pra Gus:
> "É difícil. Mas você consegue. Eu consigo. A gente consegue."

Não vende, não recompensa. Só sorri e continua subindo. Som de respiração profunda toca.

**EE-18: O Encorajador**

Sistema oculto: se Gus morre 10 vezes na mesma cena específica, NPC aparece no save ou hospital:
> "Respira. Você consegue.
>  Não é fraqueza pedir ajuda."

Oferece **Token-de-Coragem** (regenera 1 vida ao iniciar combate). Diary:
> *"Algumas montanhas se escalam aos poucos. Algumas batalhas se vencem ao admitir derrota."*

Mensagem opcional discreta no canto da tela durante carregamento:
> *"Você é mais forte do que pensa. (Equipe de desenvolvimento.)"*

---

## Cena 15: "Pergunta amanhã" (o Cauã descobre)

**Origem:** decisão do criador em 2026-07-26. Nasce da tensão X-01 do inventário de disputas de linguagem: o bordão do Cauã é "compila e roda" (`vozes-party.md:145-152`) e o Bento diz na Cena 2 que Pythia **nem vira máquina**. Duas peças canônicas que se chocam de frente, e o criador decidiu que o Cauã **descobre**. _(A fala do Bento foi ajustada em 2026-07-26, decisão X-06: era "PYTHIA SEQUER COMPILA". Esta cena depende da substância, não da redação, e por isso sobreviveu ao ajuste sem retrabalho.)_

**Contexto:** Ato 2, dentro da mini-quest posterior do Cauã (`characters/caua-volt.md:65-67`), depois que a facção radical foi destruída e ele volta aos Dutos pra reconstruir a comunidade desmoralizada. É o momento em que ele volta a **ensinar**, e é por isso que a pergunta chega.

**Estrutura:** três movimentos separados no tempo de campanha, não uma cena contínua.

**Regra de ferro da cena inteira:** **ninguém comenta.** Nem a ausência do bordão, nem a volta do tic. Nenhum personagem aponta, explica ou repara em voz alta.

### Movimento 1: a pergunta

```
[DUTOS INFERNAIS, o bloco em reconstrução. Andaime improvisado,
 cabo novo passado por cima do cabo queimado. CAUÃ ensina cinco
 moleques sentados em caixotes. O menor deles não desgruda.]

CAUÃ
Escreve o passo. Fala o que você quer. Ela entende.

MOLEQUE
E se eu escrever errado?

CAUÃ
Ela perdoa. Você conserta depois.

MOLEQUE
[já escrevendo]
Pronto.

CAUÃ
[lê por cima do ombro]
Você escreveu "liga tudo".

MOLEQUE
É o que eu quero.

CAUÃ
Tudo o quê?

MOLEQUE
Tudo.

CAUÃ
[pausa]
Apaga o "tudo".

[Os outros moleques riem. O menor não acha graça nenhuma e
 apaga com muita seriedade.]

MOLEQUE
E se eu não souber consertar?

CAUÃ
Aí você me chama.

MOLEQUE
E se você não estiver?

[CAUÃ para. Olha pro moleque. Olha pro bloco em volta, que até
 duas semanas atrás tinha mais gente sentada nele.]

CAUÃ
Aí você chama outro. Sempre tem outro.

[O moleque aceita a resposta na hora, do jeito que só quem tem
 oito anos aceita. Volta a escrever. CAUÃ solta o ar.]

MOLEQUE
Cauã.

CAUÃ
Oi.

MOLEQUE
O que é compilar?

[Silêncio. Um dos moleques maiores levanta a cabeça, curioso,
 porque também nunca perguntou.]

CAUÃ
[abre a boca]
É quando...

[pausa]

CAUÃ
Pergunta amanhã.

[Ele guarda o estilete. Vai embora devagar.]

[Ele não estala os dedos.]

[Nenhum dos moleques acha isso estranho. O Cauã sempre volta.]
```

### Movimento 2: o intervalo

```
[CATEDRAL DE NEO-SYLVANIA, fim de tarde. O eco devolve cada
 passo com meio segundo de atraso. CAUÃ entra e odeia o lugar
 imediatamente.]

[BENTO está sentado num degrau, limpando uma engrenagem do
 escudo com um pano. Não levanta a cabeça.]

BENTO
Você está pisando no mosaico.

CAUÃ
Tem mosaico no chão inteiro.

BENTO
Tem.

[pausa]

BENTO
E você está pisando nele.

[CAUÃ olha pros próprios pés. Dá um passo pro lado. Não
 melhora nada. Desiste.]

CAUÃ
Preciso perguntar uma parada.

BENTO
Pergunta.

[CAUÃ não pergunta. BENTO continua limpando a engrenagem. Não
 ajuda. Passam uns bons segundos, e o eco não devolve nada,
 porque ninguém falou.]

CAUÃ
Um moleque me perguntou uma coisa ontem.

BENTO
E?

CAUÃ
E eu falei pra ele perguntar amanhã.

[pausa]

CAUÃ
Amanhã é hoje.

[BENTO limpa a engrenagem.]

CAUÃ
Você falou que a minha nem compila.

BENTO
Falei.

CAUÃ
Você tava zoando?

[BENTO para de limpar. Põe o pano no colo.]

BENTO
[pausa longa]
Não.

[O eco devolve o "não" meio segundo depois. CAUÃ escuta duas
 vezes.]

[Silêncio. BENTO não acrescenta nada, e a pausa passa do ponto
 em que seria confortável acrescentar.]

BENTO
Mas você diz a palavra todo dia.

[pausa]

BENTO
Isso não é mentira, garoto. É promessa.

[CAUÃ senta no degrau, dois abaixo do BENTO. Não pede licença.
 BENTO não reclama.]

CAUÃ
Promessa de quê?

BENTO
De que um dia a palavra vai ser verdade na tua boca.

[pausa]

BENTO
Não sou eu que decido quando.

[CAUÃ fica com isso um tempo. Depois levanta e vai embora. No
 meio da nave, para.]

CAUÃ
Bento.

BENTO
Garoto.

CAUÃ
Obrigado.

[BENTO não responde. Espera o eco devolver a palavra e só
 então volta a limpar a engrenagem.]
```

**Depois desta cena, e até o Movimento 3, o Cauã não diz "compila e roda".** Não é uma cena, é uma ausência: o bordão sai do pool de barks e o tic de estalar os dedos por empolgação sai junto (requisito de implementação `CAUA-BORDAO-3-ESTADOS` no `TODO.md`). **Ninguém comenta a falta.**

Durante esse intervalo, um beat mudo da Linda (ela lê hesitação pelo tom e não articula publicamente, `vozes-party.md:1089`):

```
[Combate qualquer, durante o intervalo. CAUÃ avança sem dizer
 nada antes de avançar.]

[LINDA ajusta o fone. Olhar lateral nele por um segundo a mais
 do que precisaria.]

[Ela não fala.]
```

### Movimento 3: a palavra devolvida

```
[COMBATE. A party está apertada. CAUÃ saca a carta nova.]

[Ele abre a boca. Não diz nada. Ninguém repara, porque ninguém
 tem tempo de reparar em nada.]

[A carta compila. Dispara no mesmo instante em que a mão dele
 termina o gesto.]

CAUÃ
Compila e roda.

[pausa]

CAUÃ
Essa aqui compilou mesmo.

[Estala os dedos.]

[Ninguém comenta.]
```

```
[DUTOS INFERNAIS, depois. O moleque está sentado no mesmo
 caixote, com o mesmo estilete.]

MOLEQUE
Você falou pra eu perguntar amanhã.

CAUÃ
Falei.

MOLEQUE
É amanhã.

[CAUÃ senta ao lado dele no caixote.]

CAUÃ
Então senta direito, que é longo.
```

**Payoff:** o Cauã não perde nada. Ele termina com a palavra que sempre disse, e desta vez ela é verdade, ganha e não emprestada. O beat inteiro roda em cima de mecânica que já era canônica dele (o bordão some quando a hora pesa, `vozes-party.md:181-183`), então nenhuma reação nova precisou ser inventada. A resposta à pergunta do moleque **nunca é mostrada**, porque a cena não é sobre a resposta. É sobre um garoto de treze anos descobrir que ensinou uma palavra que ele não tinha, e ir buscá-la.

**O Bento não é a piada e não é o carrasco.** Ele diz "Não." seco, sozinho, e deixa a pausa passar do ponto confortável antes de oferecer qualquer coisa. Quando oferece, não é consolo: é uma tarefa ("não sou eu que decido quando"). No fim ele não diz "de nada". A dignidade dele é a mesma da Cena 2, vista do outro lado.

**Notas de execução:**

- **Zero fala do Gus.** Ele pode estar em cena no Movimento 3 (é a party em combate), mas não tem nenhuma linha e não reage. Nos Movimentos 1 e 2 ele não está.
- **O Bento não faz o sinal do compilador em nenhum momento**, de propósito. Ele não está abençoando nem enlutando; está entregando uma verdade e uma tarefa (`vozes-party.md:586`).
- **O mosaico do chão** aparece como incômodo prático do Cauã, nunca nomeado nem explicado.
- **O eco de meio segundo** faz dois trabalhos: a Catedral sendo hostil ao moleque dos Dutos (desconforto no começo) e o que obriga o Cauã a ouvir o "Não." duas vezes (peso no meio).
- **O moleque não tem nome.** Se for nomeado, entra em `CHARS.md` por decisão do criador.
- **Gatilho do Movimento 3 (decisão do criador 2026-07-26):** o payoff dispara no primeiro uso de uma carta Pythia que compila de verdade (`@jit`), e essa carta é **entregue pela própria mini-quest do Cauã**, para o arco nunca ficar pendurado em quem não explorar.

---

## Cena 16: "Ela está pensando" (Cauã x Iara, a briga sem o Gus)

**Origem:** frente `LINGUAGENS-COMICAS-DISPUTAS`, aprovada pelo criador em 2026-07-27. Preenche o par **Cauã x Iara**, que o inventário achou VAZIO (lacuna L-03: nem diálogo escrito, nem linha na matriz de dinâmicas).

**Contexto:** Ato 2, qualquer respiro depois de recrutar a Iara. **O Gus NÃO está na cena, e isso é o ponto:** os dois passam a briga inteira sem árbitro. Requisito de elenco: só dispara com o Gus fora do grupo ativo.

```
[SALA COMUM. IARA monta uma carta na bancada, sem pressa. CAUÃ
 já usou a dele três vezes e está entediado.]

CAUÃ
Você ainda tá montando essa?

IARA
Ainda.

CAUÃ
Eu usei a minha três vezes no tempo que você levou até agora.

IARA
[sem levantar os olhos]
Duas você errou.

CAUÃ
Uma eu acertei. Eu só precisava de uma.

[IARA continua montando. CAUÃ senta na beirada da bancada.]

CAUÃ
Usa a minha. Vai. Empresto.

IARA
Usa você.

CAUÃ
[levanta a carta, declama]
Descarregando... AGORA.

[Nada acontece. Um zumbido baixo. A carta pisca.]

IARA
Ela está pensando.

CAUÃ
Ela está CARREGANDO.

IARA
Do lado de fora é a mesma coisa.

[A carta finalmente dispara. Uma lata do outro lado da sala
 tomba, com atraso constrangedor.]

CAUÃ
Funcionou.

IARA
Funcionou. Eventualmente.

[CAUÃ olha pra carta. Olha pra IARA. Tira outra carta do bolso.
 Esta ele manuseia com um cuidado diferente.]

CAUÃ
Essa aqui não.

IARA
Essa aqui o quê?

CAUÃ
Essa aqui compila antes de rodar. De verdade. Não é jeito de
falar.

[CAUÃ estala os dedos. A carta dispara no mesmo instante. A
 segunda lata some da bancada antes de qualquer um piscar.]

[Silêncio.]

IARA
[baixo]
...essa é da minha família.

CAUÃ
Essa é da minha família. Só passou a andar mais rápido.

IARA
Ela passou a andar mais rápido porque compilou.

CAUÃ
[pausa]
É.

IARA
Você acabou de dar razão ao Gus.

CAUÃ
Eu não dei razão a ninguém.

IARA
Você fez questão de dizer "de verdade". Ninguém diz "de
verdade" à toa.

[CAUÃ abre a boca. Fecha.]

CAUÃ
Pelo menos o meu não briga comigo.

IARA
O meu briga antes. O dele não briga nunca.

[Pausa. As duas frases ficam no ar tempo demais.]

CAUÃ
[devagar]
A gente acabou de elogiar o Gus.

IARA
A gente acabou de elogiar o Gus.

CAUÃ
Ninguém precisa saber disso.

IARA
Ninguém vai saber disso.

[Do canto da sala, LINDA levanta a cabeça. Um dos fones já
 estava fora.]

LINDA
Eu ouvi as duas vezes.

[IARA fecha os olhos.]

CAUÃ
Quanto você quer pra esquecer?

LINDA
[recoloca o fone]
Não é matéria pra troca.
```

**Payoff:** a lei do eixo enunciada sem ninguém enunciar a lei. A carta de Pythia fica rápida no exato instante em que deixa de ser interpretada, e é o **próprio Cauã** quem faz questão de sublinhar o "de verdade", entregando o argumento do Gus com as próprias mãos. Ninguém apanha: o Cauã ganha o começo (iteração barata), a Iara ganha o fim (ela lê o que ele disse), e a piada final é dos dois contra o ausente.

**Honestidade técnica:** o `@jit` compila mesmo, para código de máquina, antes de executar. A carta é rápida **porque compilou**, o que prova a lei em vez de furá-la. Nenhuma fala da cena afirma que Pythia interpretada é rápida.

**Notas de voz:** o Cauã estala os dedos (tic canônico) e não usa bordão em momento pesado, porque não há momento pesado aqui. A Iara sussurra pouco, mantém frase curta e **nenhuma fala dela é ornamentada**, o que a mantém no eixo dela sem invadir o da Linda. A Linda entra com uma linha seca e a palavra "matéria", que é dela por canon.

**Relação com as farpas (decisão do criador, 2026-07-27):** as farpas **F11, F12 e F13** de [`farpas-linguagens.md`](farpas-linguagens.md) vivem dentro desta cena quase palavra por palavra. Elas continuam valendo, mas **só disparam DEPOIS de a cena ter acontecido**: viram lembrança curta do que o jogador viu, nunca repetição cega de algo que ele ainda não presenciou.

---

## Cena 17: "Pra frente e pra trás" (Jaci x Linda)

**Origem:** frente `LINGUAGENS-COMICAS-DISPUTAS`, aprovada pelo criador em 2026-07-27. Preenche o par **Jaci x Linda**, que o inventário achou VAZIO (lacuna L-04).

**Contexto:** Ato 2, imediatamente depois de um combate. Primeira vez que as duas trabalham lado a lado sem ninguém no meio. As duas contam o tempo inteiro e nunca perceberam que a outra também é uma contadora. Requisito de elenco: só dispara com o Gus fora do grupo ativo.

```
[CORREDOR, pós-combate. LINDA marca compasso com dois dedos no
 cano da parede. JACI, ajoelhada, conta ampolas.]

LINDA
Três, dois, um.

JACI
Espera.

LINDA
[sem parar]
Três, dois, um.

JACI
Espera. Deixa eu contar de novo.

[LINDA para. Ajusta o fone. Olha.]

LINDA
Eu já contei.

JACI
Você contou pra frente.

LINDA
Contei.

JACI
Eu conto pra trás. Pra trás é conferir. Pra frente é só
combinar.

[LINDA processa isso por dois segundos, e dois segundos é
 bastante tempo pra ela.]

LINDA
Então conta comigo. No meu compasso.

JACI
No seu compasso eu perco a conta.

LINDA
Ninguém perde a conta no compasso. O compasso existe pra isso.

JACI
Tá.

[LINDA marca. JACI conta junto.]

LINDA
Três, dois, um.

JACI
Nove.

[Pausa.]

LINDA
Nove o quê?

JACI
Nove ampolas. Eram dez.

LINDA
Eu marquei três tempos exatos.

JACI
Eu sei. No primeiro tempo tinha dez. No terceiro tinha nove.

LINDA
[longo]
Você usou uma no meio da contagem.

JACI
[olha pra própria mão]
...usei.

[JACI guarda a ampola vazia no bolso do jaleco. Com cuidado,
 como se ainda servisse pra alguma coisa.]

JACI
Ele respirou duas vezes no meio do seu compasso, Linda.

LINDA
[ajusta o fone]
Meio compasso a mais.

JACI
Meio compasso a mais.

LINDA
O meu não monta com peça faltando. Ele para antes de mim.

JACI
O meu monta. E se faltar, ele me diz qual faltou. Com o nome e
o número.

LINDA
Depois.

JACI
Depois eu ainda estou aqui pra ler.

[LINDA não responde na hora. Volta a marcar o cano com dois
 dedos, mais devagar do que antes.]

LINDA
Decide agora quem fica com a última.

JACI
Eu não sei quem vai precisar antes de abrir a caixa.

LINDA
O meu exige que você saiba. Antes.

JACI
[fecha o jaleco]
Por isso ele é mais rápido que eu.

[LINDA para de marcar.]

LINDA
Você admitiu.

JACI
Eu conto, Linda. Eu não minto.

[LINDA quase sorri. Volta a marcar, e desta vez marca no ritmo
 da contagem da JACI, não no dela.]

LINDA
Nove.

JACI
Nove.
```

**Payoff:** a Jaci **concede a lentidão sem discutir** e vence noutro eixo, que é a forma canônica de a Pythia perder com dignidade. A Linda ganha o argumento técnico inteiro (a decisão que se toma antes, a montagem que não sai com peça faltando) e ainda assim é ela quem cede o compasso no fim. A aproximação acontece em cima de uma contagem de ampola, que é a coisa mais banal que as duas tinham em comum.

**Notas de voz:**

- **A Linda não tira os fones em nenhum momento**, e ninguém comenta o ritmo dela: o guia reserva o ritmo parando para dor real e proíbe outro personagem de comentar quando isso acontece. Aqui ela **desacelera** e passa a marcar no ritmo da outra, e o "quase sorri" é o mesmo micro-gesto que ela já tem com o Cauã.
- **A Jaci não grita** e o tic de contar aparece, mas **o off-by-one não é a punchline**: a conta não fecha porque ela mesma usou uma ampola salvando alguém, e ela percebe sozinha. A Cena 9 já gastou a piada do off-by-one puro.
- Nenhuma fala da Linda é ornamentada; nenhuma fala da Jaci é apressada.

**Relação com as farpas (decisão do criador, 2026-07-27):** as farpas **F17, F18, F19 e F21** de [`farpas-linguagens.md`](farpas-linguagens.md) vivem dentro desta cena. Elas continuam valendo, mas **só disparam DEPOIS de a cena ter acontecido**. Ficam livres desde o começo apenas a F20 e a F22.

---

## Cena 18: "A peça que ainda gira" (Gus, Bento e Dante)

**Origem:** frente `LINGUAGENS-COMICAS-DISPUTAS`, aprovada pelo criador em 2026-07-27. Fecha a lacuna L-05: o desenho das três fases do deslize do Dante estava pronto, mas **não existia nenhuma cena da fase tardia**. A [Cena 4](#cena-4-force-push-foreshadow-dante-traidor) é o vazamento fundador e acontece muito antes.

**Contexto:** Ato 2 tardio, **depois da Cena 4** e a partir de ~75% de campanha (fase (c) do Dante). Oficina da Periferia Industrial. Uma ventoinha industrial antiga, presa por gambiarra, ventila metade do quarteirão.

```
[OFICINA DE DANTE. Uma ventoinha industrial gira no teto, presa
 por uma braçadeira, um pedaço de correia e fé. Faz "tec" a
 cada volta.]

DANTE
Eu arranco isso hoje e ponho uma nova em vinte minutos.

BENTO
Não.

DANTE
Bento. Ela faz "tec".

BENTO
Ela faz "tec" desde antes de você nascer.

[BENTO se aproxima. Olha pra ventoinha por um tempo
 desconfortável. Faz o sinal do compilador sobre o cronômetro.]

GUS
[baixinho, pro Dante]
Ele fez o sinal. Pra uma ventoinha.

DANTE
Ele faz pra tudo que é velho.

BENTO
[sem se virar]
Eu faço pra tudo que aguentou.

[Pausa. A ventoinha faz "tec".]

GUS
Eu consigo melhorar ela.

BENTO
Como.

GUS
[já anotando no Diário]
Ela gasta metade do que puxa brigando com o próprio eixo. Eu
reescrevo o passo, alinho a fase, e ela puxa menos e sopra
mais. Dá pra fazer hoje.

BENTO
E se ela puxar menos do que precisa?

GUS
Não vai. Eu vou medir.

BENTO
Você vai medir o que você sabe medir.

[GUS abre a boca. Não tem resposta pronta, e isso o incomoda
 mais que o argumento.]

BENTO
Guarda a peça velha.

DANTE
Peça velha quebrou.

BENTO
Guarda a peça velha, Dante.

DANTE
Rápido resolve. Recusar não resolve nada, só atrasa.

[DANTE olha pro lado. Um segundo a mais do que o normal.]

[GUS levanta os olhos do Diário.]

GUS
...nisso ele tem razão.

[BENTO não responde. Olha pro DANTE por mais tempo do que se
 olha pra alguém que só falou de ventoinha. Depois olha pro
 GUS. Depois pro chão.]

[BENTO atravessa a oficina, pega da caixa de descarte a peça
 que DANTE trocou na semana passada, e guarda no bolso.]

DANTE
Você guardou aquilo.

BENTO
Guardei.

DANTE
Não serve pra nada.

BENTO
A peça velha te ensina onde a nova vai quebrar de novo.

GUS
Isso não é argumento técnico, Bento.

BENTO
Não é. É conta.

[Silêncio. A ventoinha faz "tec".]

GUS
[anotando]
"Peça velha: guardada. Motivo: o Bento olhou pra mim."

BENTO
Escreve o resto.

GUS
Que resto?

BENTO
"Ainda não medi tudo."

[GUS encara o Diário. Escreve. Não gosta de ter escrito.]

[A ventoinha faz "tec".]
```

**Payoff:** três coisas ao mesmo tempo, e o jogador só vê a primeira.

1. **Superfície:** gravidade litúrgica aplicada a uma ventoinha caindo aos pedaços, com sinal do compilador e tudo. **O Bento não é a piada:** ele está certo do início ao fim, fecha os dois argumentos que importam, e o "tec" final é a ventoinha dando razão a ele.
2. **Ensaio do pecado do Gus.** É o **ensaio**, não o evento: aqui ele quer otimizar infra que serve a terceiros, ouve o aviso ("você vai medir o que você sabe medir"), e a única coisa que o segura é um olhar. A falha canônica dele acontece **depois e em outro lugar**, e nenhum designer deve tratar esta cena como se fosse ela.
3. **O deslize do Dante, não comentado:** ele recita o **axioma inteiro do C-Arcane** ("rápido resolve, recusar não resolve nada") professando Asmódico, e olha pro lado um segundo a mais. Ninguém nomeia nada, e o Gus ainda **dá razão a ele em voz alta**. Na primeira jogada é o mecânico prático discutindo com o tradicionalista; na releitura, é a confissão.

**Encaixe na progressão canônica de fases**, sem calibragem inventada: a fala do axioma **já é canon** no guia do Dante e entra sem alteração; o Bento reage só com o olhar, como o guia manda; o glance lateral é o indicador de temperatura que sobe pelas três fases; e ninguém comenta o deslize, que é a condição da calibragem aprovada pelo criador.

**Corte deliberado (decisão do criador, 2026-07-27):** a versão original desta cena trazia um **segundo** deslize, lexical ("assim otimiza" corrigido para "assim assenta"). Ele foi **removido**: o verbo "assenta" não existe no canon, foi inventado na redação e ninguém o decifra, nem quem entende de linguagens. O deslize lexical da fase tardia vive na farpa **F09** (`ponteiro` → `endereço`), no contexto dela, que é conserto sob pressão. **Um deslize por cena, e o que ficou é o mais forte dos dois.**

---

## Cena 19: "Pra tudo que aguentou" (Cauã x Bento)

**Origem:** frente `LINGUAGENS-COMICAS-DISPUTAS`, aprovada pelo criador em 2026-07-28. Preenche o par **Cauã x Bento** no registro deles, sem o Gus como eixo (as Cenas 2 e 8 são Gus x Bento com o Cauã de passagem).

⚠ **CRONOLOGIA: esta cena acontece ANTES da [Cena 15](#cena-15-pergunta-amanhã-o-cauã-descobre)**, embora o número seja maior (a numeração deste arquivo é ordem de criação, não da linha do tempo). Ela é a **fundação** da Cena 15: sem ela, o jogador tem que aceitar de graça que o Cauã atravessaria a cidade para fazer uma pergunta dolorosa justo ao Bento. Aqui se estabelece o motivo: o Bento tratou o trabalho dele com seriedade uma vez, então ele sabe que vai receber resposta reta e não piada.

**Contexto:** Ato 2, Fase 2 do retorno de aprofundamento do Cauã, depois de conter a ameaça de inundação dos túneis. A party seguiu; **o Bento ficou para conferir a estrutura, porque é o que ele faz, e ninguém pediu**. Geografia invertida em relação à Cena 15: aqui é o Bento quem está fora do próprio território. **O Gus não está na cena.**

```
[DUTOS INFERNAIS, galeria do bloco. Um tronco de cabos sobe
 pela parede e some no teto. No meio dele, a emenda: fita,
 braçadeira, um pedaço de mangueira cortada e um gancho que já
 foi outra coisa.]

[BENTO está parado na frente da emenda há um tempo
 considerável. CAUÃ chega e para ao lado dele.]

CAUÃ
Que foi?

BENTO
Estou olhando.

CAUÃ
Tá olhando desde quando?

BENTO
Desde que eu vi.

[Pausa. O tronco de cabos zumbe baixo.]

BENTO
Isso é um gancho de cortina.

CAUÃ
É.

BENTO
Por quê.

CAUÃ
Porque era o que tinha.

[BENTO fecha os olhos por um segundo. Abre.]

BENTO
Quantas emendas.

CAUÃ
Nessa aqui? Oito.

BENTO
Oito.

CAUÃ
Oito boas.

[BENTO se agacha. Segue o cabo com o dedo, sem encostar,
 emenda por emenda, até onde ele some no teto.]

BENTO
Há quanto tempo isso está de pé?

CAUÃ
Cinco anos.

[BENTO olha pra ele.]

CAUÃ
Cinco anos, Bento. Se tivesse errado, já tinha caído.

BENTO
Ou ainda não caiu.

CAUÃ
Mas não caiu.

[O tronco estala. Uma vez, seco. Os dois olham pra cima. Não
 acontece mais nada.]

CAUÃ
Ele faz isso.

BENTO
Eu sei que ele faz isso. Eu ouvi.

[Pausa longa. CAUÃ enfia as mãos no bolso.]

BENTO
O que tem em cima disso?

CAUÃ
O bloco.

BENTO
Quantas casas.

CAUÃ
Todas.

[BENTO não responde na hora. Levanta devagar.]

CAUÃ
Pode falar.

BENTO
Falar o quê.

CAUÃ
O que você tá pensando. Que é feio, que é errado, que na
Catedral de vocês isso não passaria de...

BENTO
Não passaria.

CAUÃ
Pois é.

BENTO
Na Ordem eu aprendi que o que é mal feito cai, e que cai
cedo, porque o mundo não tem paciência nenhuma com o que foi
feito errado.

[pausa]

BENTO
Isso está de pé há cinco anos segurando todas as casas de um
bloco.

[pausa]

BENTO
As duas coisas são verdade, garoto. É isso que me incomoda.

[BENTO leva a mão ao cronômetro no peito. Para no meio do
 gesto.]

CAUÃ
Que que você tá fazendo?

BENTO
Nada.

CAUÃ
Você ia fazer aquele negócio da cruz.

BENTO
Eu ia.

CAUÃ
Por quê?

[BENTO leva mais tempo do que uma pergunta daquele tamanho
 merece.]

BENTO
Porque eu faço pra tudo que aguentou.

[Silêncio. O tronco zumbe.]

CAUÃ
Então faz.

BENTO
Não é de graça pra mim.

CAUÃ
Eu sei que não é.

[pausa]

CAUÃ
Faz mesmo assim.

[BENTO faz o sinal do compilador sobre o cronômetro, na frente
 de uma emenda de fita, mangueira e gancho de cortina.]

[Ele não parece feliz. Também não parece arrependido.]

CAUÃ
[baixo, sem zoar]
Valeu.

BENTO
Não agradece. Não foi elogio.

CAUÃ
Foi o quê então?

BENTO
Foi contagem.

[CAUÃ aceita isso, do jeito que se aceita uma moeda de um
 país que a gente não conhece.]

BENTO
Me ensina.

CAUÃ
Ensinar o quê?

BENTO
A emenda. Como se faz.

[CAUÃ olha pra ele pra conferir se é pegadinha. Não é.]

CAUÃ
Você quer aprender gambiarra.

BENTO
Eu quero saber o que segura o bloco onde meus aliados dormem.

[CAUÃ pega o rolo de fita. Hesita. Estende a ponta do cabo
 pro BENTO.]

CAUÃ
Segura aqui. Não solta.

BENTO
[segurando]
Isso eu sei fazer.

[Os dois ficam ali, um segurando e o outro emendando, sem
 concordar em nada.]
```

**Payoff:** a régua do Bento pega ele mesmo. Ele passou a vida defendendo pureza e recusa antes do erro, mas o critério que ele **usa de verdade**, e que ele mesmo enunciou na Cena 18, é outro: aguentou, então merece o sinal. A gambiarra dos Dutos passa nesse teste com folga, e ele não tem como fugir sem se contradizer. **Ninguém cede identidade:** o Bento continua achando a emenda errada e diz isso na cara, o Cauã continua achando que de pé é melhor que bonito e não recua um passo. O que muda é que os dois descobrem que fazem a mesma coisa da vida, que é **aguentar**, e a cena entrega isso por ação em vez de fala: a única parte da gambiarra que o Bento já sabe fazer é segurar.

**O que o sinal custa a ele** (o sinal é reservado e não pode virar decoração): ele **para no meio do gesto**, precisa ser **autorizado pelo dono** da coisa que vai benzer, e diz em voz alta que **não é de graça** para ele. Depois nega que tenha sido elogio e chama de "contagem", o mesmo vocabulário com que fechou a Cena 18 ("Não é. É conta."). O sinal sai do bolso dele, não da boca.

**Regra dos pesos aplicada e declarada (decisão do criador, 2026-07-28):** a fala do Cauã "se tivesse errado, já tinha caído" é **falácia do sobrevivente**, e fica assim de propósito. É ele defendendo cinco anos do próprio trabalho diante do mais velho da party, no território dele: recuar ali seria admitir que o bloco esteve em risco esse tempo todo por culpa dele, e um garoto de treze anos não faz essa conta em voz alta na frente de quem veio inspecionar. **O certo é ensinado duas vezes na própria cena:** pelo "ou ainda não caiu" do Bento, que é a formulação correta em quatro palavras, e pelo **tronco estalando sozinho** duas falas depois, enquanto o dono explica que é normal. O Cauã não concede, e é isso que faz o beat funcionar.

**Easter egg velado:** cinco anos, oito emendas. Nada é nomeado.

---

## Distribuição expandida pelos atos

| Ato | Cenas adicionadas |
|---|---|
| Ato 1 (introdução cidade) | 11 (Janelarum trava), 12 (Atualização obrigatória), EE-3 (Encanador), EE-7 (Guia) |
| Ato 2 (incursões) | 13 (Linha de comando, após recrutar Cauã), 14 (Comutador vs Tocador-Trono) |
| Ato 2 (mini-quest do Cauã, pós-crise) | 15 (Pergunta amanhã, três movimentos separados no tempo; o Movimento 3 dispara na carta `@jit` da própria mini-quest) |
| Ato 2 (pares sem o Gus, requer o Gus fora do grupo ativo) | 16 (Ela está pensando, Cauã x Iara), 17 (Pra frente e pra trás, Jaci x Linda) |
| Ato 2 tardio (pós-Cena 4, a partir de ~75%) | 18 (A peça que ainda gira, fase tardia do deslize do Dante) |
| Ato 2 (fase 2 do retorno do Cauã, ANTES da Cena 15 na cronologia) | 19 (Pra tudo que aguentou, Cauã x Bento, fundação da Cena 15) |
| Ato 2 (regional) | EE-1, EE-2 (Dutos), EE-5, EE-6 (Selve), EE-8 (fronteira), EE-11, EE-15 (Mirage), EE-13, EE-14 (vilarejo Jaci), EE-17 (Catedrais) |
| Cross-ato (conquistas) | EE-4 (cogumelo), EE-9 (100 inimigos), EE-10 (placa hospital), EE-12 (100 mortes mesma cena), EE-16 (diálogo casca azul), EE-18 (encorajador) |

---

## Como estas cenas chegam ao jogador (decisão do criador, 2026-07-28)

**As cenas têm DUAS formas, e as duas são canônicas.**

**1. No jogo: diálogo linear com narração.** Cada cena vira um arquivo `.dlg.txt` consumido pelo runtime de diálogo (ADR-014), no mesmo formato do `npc_intro_bertoldo.dlg.txt`. As falas entram como estão. **As direções cênicas viram linhas de um speaker de narração**, no registro terminal já aprovado para logs e telas de sistema.

Isto é o que o motor faz **hoje**, sem feature nova: o formato tem `speaker`, `text` (chave i18n), escolhas e flags. O que ele **não** tem é movimento de sprite scriptado, e por isso o gesto interrompido no meio, o dedo seguindo o cabo e o tronco estalando são **narrados**, não encenados. Cutscene com sprites móveis seria feature de engine e não está na fila.

**Consequência prática para quem converter:** toda fala e toda linha de narração precisa de **chave i18n**, e a narração precisa ser escrita como texto que se sustenta lido, não como rubrica de roteiro. "[BENTO se agacha]" é rubrica; "Bento se agacha e segue o cabo com o dedo, sem encostar" é narração.

**2. No livro: a prosa, com a direção cênica preservada.** As cenas foram escritas com direção cênica, e é em prosa que elas rendem inteiras. Elas entram nos volumes (F5-BK) na forma em que estão neste arquivo, que continua sendo a fonte.

**As 22 farpas de [`farpas-linguagens.md`](farpas-linguagens.md) não têm esse problema:** são barks de duas a quatro linhas e cabem no sistema atual sem nenhuma adaptação.

---

## Templates pra novas cenas

Quem quiser adicionar (designer, narrative-designer, escrita expandida em produção):

1. **Setup:** Banal/dev real comum (deploy, regex, comentário, push, cache, type coercion, race condition, infinite recursion, deadlock, RTFM, premature optimization).
2. **Personagem A** trata com gravidade absurda.
3. **Personagem B** reage perdido.
4. **Beat de tensão** (silêncio, trovão, ação física).
5. **Payoff** = call-back ou setup pra próximo gag.
6. **Não conclui**: vida segue. Próxima cena continua.
