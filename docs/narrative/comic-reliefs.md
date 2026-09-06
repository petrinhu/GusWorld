# Alívios Cômicos (GusWorld)

Cenas de humor LucasArts-like (Monkey Island, Amazon Queen, Day of the Tentacle, Sam & Max): timing, gravidade absurda aplicada a banalidades, paródia, meta-humor leve, **sem palavrão e sem fan-service** (Pillar 4). Cada cena ancorada em meme/cultura de programação reconhecível mas diegética ao mundo.

**Princípios:**
- Beats curtos. Pausas medidas. Reações silenciosas valem mais que falas.
- Personagem sério dizendo absurdo OU personagem ridículo dizendo verdade.
- Sem quebrar 4ª parede explícito; sempre ancorado no mundo (linguagens fictícias canônicas: C-Arcane, Asmódico, Óxido, Pythia).
- Distribuir ~8-12 cenas ao longo do jogo, **uma por capítulo** (ritmo).
- Algumas plantam foreshadow (Dante traidor) ou reforçam theme (Sterling = bug declarado feature).

---

## Índice das cenas

**Este documento foi fatiado em 21 arquivos em `docs/narrative/comic-reliefs/`, um por cena/bloco, em 05/09/2026 (item `D37` do `TODO.md`). Nenhuma palavra de cena ou de fala mudou no processo — só a divisão em arquivos.** O que vale para o conjunto (princípios, distribuição por atos, regras de tom, como as cenas chegam ao jogador, templates para cenas novas) continua neste arquivo, abaixo.

| Cena | Arquivo |
|---|---|
| Cena 1: "Sexta-feira, cinco da tarde" (deploy de produção) | [`comic-reliefs/cena-01-sexta-feira-cinco-da-tarde.md`](comic-reliefs/cena-01-sexta-feira-cinco-da-tarde.md) |
| Cena 2: "Tabulações vs Espaços" (a guerra santa) | [`comic-reliefs/cena-02-tabulacoes-vs-espacos.md`](comic-reliefs/cena-02-tabulacoes-vs-espacos.md) |
| Cena 3: "Funciona no meu Drive" | [`comic-reliefs/cena-03-funciona-no-meu-drive.md`](comic-reliefs/cena-03-funciona-no-meu-drive.md) |
| Cena 4: "Force Push" (foreshadow Dante traidor) | [`comic-reliefs/cena-04-force-push.md`](comic-reliefs/cena-04-force-push.md) |
| Cena 5: "Expressão Regular" (Iara apaixonada por opacidade) | [`comic-reliefs/cena-05-expressao-regular.md`](comic-reliefs/cena-05-expressao-regular.md) |
| Cena 6: "Bug declarado Feature" (Sterling propaganda) | [`comic-reliefs/cena-06-bug-declarado-feature.md`](comic-reliefs/cena-06-bug-declarado-feature.md) |
| Cena 7: "Tomo da Pilha Sobrecarregada" | [`comic-reliefs/cena-07-tomo-da-pilha-sobrecarregada.md`](comic-reliefs/cena-07-tomo-da-pilha-sobrecarregada.md) |
| Cena 8: "Comentários em Latim" | [`comic-reliefs/cena-08-comentarios-em-latim.md`](comic-reliefs/cena-08-comentarios-em-latim.md) |
| Cena 9: "Off-by-one" (a tragédia menor) | [`comic-reliefs/cena-09-off-by-one.md`](comic-reliefs/cena-09-off-by-one.md) |
| Cena 10: "Não é magia, é cache" | [`comic-reliefs/cena-10-nao-e-magia-e-cache.md`](comic-reliefs/cena-10-nao-e-magia-e-cache.md) |
| Convenções diegéticas de plataforma (Janelarum, Pinguium, Comutador, Tocador-Trono) | [`comic-reliefs/convencoes-diegeticas-guerra-de-plataforma.md`](comic-reliefs/convencoes-diegeticas-guerra-de-plataforma.md) |
| Cena 11: "Janelarum travou de novo" | [`comic-reliefs/cena-11-janelarum-travou-de-novo.md`](comic-reliefs/cena-11-janelarum-travou-de-novo.md) |
| Cena 12: "Atualização obrigatória" | [`comic-reliefs/cena-12-atualizacao-obrigatoria.md`](comic-reliefs/cena-12-atualizacao-obrigatoria.md) |
| Cena 13: "Linha de comando" (peer pressure) | [`comic-reliefs/cena-13-linha-de-comando.md`](comic-reliefs/cena-13-linha-de-comando.md) |
| Cena 14: "Comutador vs Tocador-Trono" | [`comic-reliefs/cena-14-comutador-vs-tocador-trono.md`](comic-reliefs/cena-14-comutador-vs-tocador-trono.md) |
| Homenagens diégeticas (catálogo com as 11 obras, EE-1 a EE-22) | [`comic-reliefs/homenagens-diegeticas.md`](comic-reliefs/homenagens-diegeticas.md) |
| Cena 15: "Pergunta amanhã" (o Cauã descobre; 3 movimentos) | [`comic-reliefs/cena-15-pergunta-amanha.md`](comic-reliefs/cena-15-pergunta-amanha.md) |
| Cena 16: "Ela está pensando" (Cauã x Iara, a briga sem o Gus) | [`comic-reliefs/cena-16-ela-esta-pensando.md`](comic-reliefs/cena-16-ela-esta-pensando.md) |
| Cena 17: "Pra frente e pra trás" (Jaci x Linda) | [`comic-reliefs/cena-17-pra-frente-e-pra-tras.md`](comic-reliefs/cena-17-pra-frente-e-pra-tras.md) |
| Cena 18: "A peça que ainda gira" (Gus, Bento e Dante) | [`comic-reliefs/cena-18-a-peca-que-ainda-gira.md`](comic-reliefs/cena-18-a-peca-que-ainda-gira.md) |
| Cena 19: "Pra tudo que aguentou" (Cauã x Bento) | [`comic-reliefs/cena-19-pra-tudo-que-aguentou.md`](comic-reliefs/cena-19-pra-tudo-que-aguentou.md) |

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

## Distribuição expandida pelos atos

| Ato | Cenas adicionadas |
|---|---|
| Ato 1 (introdução cidade) | 11 (Janelarum trava), 12 (Atualização obrigatória), EE-3 (Encanador), EE-7 (Guia) |
| Ato 2 (incursões) | 13 (Linha de comando, após recrutar Cauã), 14 (Comutador vs Tocador-Trono) |
| Ato 2 (mini-quest do Cauã, pós-crise) | 15 (Pergunta amanhã, três movimentos separados no tempo; o Movimento 3 dispara na carta `@jit` da própria mini-quest) |
| Ato 2 (pares sem o Gus, requer o Gus fora do grupo ativo) | 16 (Ela está pensando, Cauã x Iara), 17 (Pra frente e pra trás, Jaci x Linda) |
| Ato 2 tardio (pós-Cena 4, a partir de ~75%) | 18 (A peça que ainda gira, fase tardia do deslize do Dante) |
| Ato 2 (fase 2 do retorno do Cauã, ANTES da Cena 15 na cronologia) | 19 (Pra tudo que aguentou, Cauã x Bento, fundação da Cena 15) |
| Ato 2 (regional) | EE-1, EE-2 (Dutos), EE-5, EE-6 (Selve), EE-8 (fronteira), EE-11, EE-15, EE-19 (Mirage), EE-13, EE-14 (vilarejo Jaci), EE-17 (Catedrais), EE-21, EE-22 (Orla Recursiva) |
| Cross-ato (conquistas) | EE-4 (cogumelo), EE-9 (144 inimigos), EE-10 (placa hospital), EE-12 (mortes na mesma cena, limiar por dificuldade), EE-16 (diálogo casca azul), EE-18 (encorajador), EE-20 (reunião de emergência) |

---

## Como estas cenas chegam ao jogador (decisão do criador, 2026-07-28)

**As cenas têm DUAS formas, e as duas são canônicas.**

**1. No jogo: diálogo linear com narração.** Cada cena vira um arquivo `.dlg.txt` consumido pelo runtime de diálogo, no mesmo formato do `npc_intro_bertoldo.dlg.txt`. As falas entram como estão. **As direções cênicas viram linhas de um speaker de narração**, no registro terminal já aprovado para logs e telas de sistema.

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
