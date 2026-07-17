# Guia prático: como escrever diálogos melhores

> **Status:** guia de referência para o time narrativo do GusWorld (narrative-writer, narrative-designer, revisor-textual). Não é regra canônica de lore, é ferramenta de ofício. Aplica-se a toda prosa de diálogo do projeto: deep-lore, antologia, in-world-docs com fala, e futura implementação de diálogo interativo em jogo.
>
> **Formato:** checklist acionável, não ensaio. Pensado pra consulta rápida durante escrita e revisão.

---

## 0. Antes de escrever uma linha: pra que serve esta fala?

Toda fala precisa passar em pelo menos **um** destes três testes. Se não passa em nenhum, corte.

1. **Avança o enredo** (revela um fato que muda a decisão de alguém, cria uma complicação, resolve uma).
2. **Revela personagem** (mostra quem ele é através de como ele fala, não do que ele afirma sobre si mesmo).
3. **Cria ou libera tensão** (aumenta o conflito da cena, ou dá o respiro certo depois de um pico).

Corte cumprimentos vazios ("oi, tudo bem?", "e aí, como foi seu dia?"), despedidas de cortesia e qualquer troca que só existe porque "é o que pessoas reais diriam". Prosa não é transcrição: diálogo bom é a essência de uma conversa real, não a gravação dela.

**Aplicação GusWorld:** no elenco de 11-14 anos, isso importa em dobro. Crianças/pré-adolescentes falam por atalhos, interrompem, e vão direto ao ponto que importa emocionalmente pra elas, mesmo que seja tangencial ao "assunto oficial" da cena. Isso é characterização, não erro.

---

## 1. Subtexto: personagens não dizem tudo

- **Regra de ouro:** o que o personagem diz raramente é 100% o que ele quer, sente ou sabe. A distância entre a fala e a intenção real é o subtexto, e é ela que dá densidade à cena.
- Prefira a linha oblíqua à linha direta. Em vez de "Estou com medo de perder você" (on-the-nose), o personagem pergunta algo lateral, muda de assunto, ou ataca com sarcasmo, e a cena (ação + contexto) entrega o resto.
- Contraste palavra x ação é a ferramenta mais barata de subtexto: personagem diz "tô bem" com os punhos fechados. O leitor faz a conta sozinho, e confiar nessa conta é respeito pela inteligência de quem lê.
- Diálogo de opostos: dois personagens conversando sobre assuntos ligeiramente diferentes na mesma troca (um fala do presente, o outro por trás está processando o passado) cria profundidade sem exposição.
- Ironia e insinuação valem mais que afirmação. Se a cena permite dupla leitura, geralmente está funcionando.

**Aplicação GusWorld:** Dante "Grid" (traidor disfarçado) é o caso-limite do projeto inteiro: cada fala dele precisa ter uma leitura inocente e uma leitura culpada simultâneas até o reveal do clímax. Auditar retroativamente: nenhuma fala do Dante pode "entregar" a traição on-the-nose antes da hora.

---

## 2. Voz distinta por personagem (crucial no GusWorld)

Cada personagem deve ser identificável só pela fala, sem etiqueta de nome. Teste prático: tape os nomes de quem fala num trecho de diálogo, dá pra saber quem é quem?

Elementos que constroem voz distinta:
- **Léxico:** palavras que ele usa e que os outros não usam.
- **Ritmo/sintaxe:** frases curtas e cortantes vs. elaboradas e cheias de subordinada; quem interrompe, quem elabora demais, quem responde com pergunta.
- **O que ele NÃO diz:** um tique de omissão é tão identificador quanto um tique de presença (ex.: personagem que nunca pede desculpas diretamente, sempre por rodeio).
- **Idade e formação:** vocabulário e complexidade sintática devem refletir 11-14 anos, nem "criança de propaganda de TV" nem "adulto disfarçado de criança". Ver Pillar 4 (prodígio analítico, não power-fantasy adulta): a inteligência de Gus aparece na lógica da fala, não em vocabulário adulto.
- **Gírias e marcas geracionais com moderação:** uma ou duas por personagem, usadas com consistência, não uma lista extensa (senão vira caricatura).

**Aplicação GusWorld: a "linguagem-âncora" já é a ferramenta de voz pronta.** Cada companion defende uma linguagem de programação in-world (C-Arcane/Asmódico/Óxido/Pythia, ver `docs/narrative/characters/party.md`) como parte da personalidade. Isso não é só lore de sistema de magia, é uma fonte primária de voz de diálogo:

  - **Bento (Asmódico, tradição/latim litúrgico):** fala mais formal, cadenciada, referências a "pureza" e "regra".
  - **Cauã e Jaci (Pythia, "perdão de erros", rápida de escrever):** fala mais solta, tolerante a gafe própria, calorosa.
  - **Iara e Linda (Óxido, elegância opaca, "três bits decidem"):** fala mais econômica, cortada, deliberadamente ambígua.
  - **Gus (C-Arcane, baixo-nível mas legível):** fala que busca clareza mesmo quando o assunto é complexo; ele traduz em vez de mistificar.
  - **Dante (Asmódico fake, C-Arcane tardio):** a fala dele deve soar como um Bento levemente errado (o "sotaque" está quase certo, mas não é autêntico), isso é uma pista de personagem plantada em superfície de voz, não só em plot.

Antes de escrever fala de qualquer companion, reler a seção "linguagem-âncora" dele em `party.md` e no arquivo individual (`characters/<nome>.md`).

---

## 3. Dialogue tags: quase invisíveis

- **"Disse" e "perguntou" são os verbos corretos na maior parte do tempo.** Eles são transparentes: o leitor pula por cima e vai direto pro conteúdo da fala.
- **Evite advérbios em tags** ("disse raivosamente", "perguntou tristemente"). Se a fala precisa de advérbio pra comunicar o tom, a fala está fraca: reescreva a fala, não a tag. Exceção rara: quando o tom real é ambíguo por natureza (ironia difícil de perceber só pelo texto).
- **Prefira beat de ação** a tag verbal quando possível: uma ação física curta antes/depois da fala identifica o falante e comunica emoção sem precisar de tag nenhuma.
  - Fraco: "Não vou fazer isso," disse Cauã, irritado.
  - Melhor: Cauã cruzou os braços. "Não vou fazer isso."
- **Verbos "exóticos" cansam rápido** ("ele retrucou", "ela vociferou", "ele indagou"). Um ou dois na cena inteira, no máximo.
- Nem toda fala precisa de tag. Se já ficou claro pelo contexto ou pela alternância de dois falantes, deixe a fala sozinha.

---

## 4. Mostrar em vez de contar via fala

- Emoção entra pela ação, pelo corpo, pelo ambiente, não pela declaração. Personagem não anuncia "estou furioso", ele bate a porta, ou baixa a voz de um jeito que assusta mais que gritar.
- Beats de ação intercalados fazem dois trabalhos ao mesmo tempo: dão ritmo visual à cena (evita "cabeças flutuantes", só falas empilhadas sem corpo) e comunicam subtexto.
- Regra prática: se uma linha de diálogo só existe para anunciar um estado emocional que a cena já deveria estar mostrando, ela é redundante. Corte a linha ou corte a mostra, nunca as duas.

---

## 5. Não fazer info-dump ("as you know, Bob")

- **O erro:** dois personagens contam um ao outro fatos que ambos já sabem, só para o leitor ficar sabendo. Soa artificial porque ninguém fala assim na vida real.
- **Correções práticas:**
  - Mova o fato pra narração/ação em vez de fala, quando possível.
  - Deixe um personagem ignorante de propósito (um recém-chegado, um novato, alguém enganado) para que a explicação seja genuinamente necessária dentro da cena.
  - Entregue a informação através de conflito: uma discussão sobre o que fazer revela a regra/fato por tabela, sem que ninguém precise "explicar a política pra quem já sabe".
  - Distribua a informação aos poucos (drip-feed), nunca tudo de uma vez.
  - Teste: essa fala é dita porque o personagem precisa dizer isso pro outro personagem, ou porque o leitor precisa saber? Se é a segunda, reescreva.

**Aplicação GusWorld:** deep-lore e in-world-docs têm muita exposição de sistema (linguagens de programação-magia, facções, hardware). Regra: exposição de sistema entra por ação/observação/documento in-world (já é convenção do projeto, ver `in-world-docs.md`), nunca por dois personagens "recapitulando o manual" um pro outro em diálogo de cena.

---

## 6. Cada fala tem um objetivo (conflito na conversa)

- Toda cena de diálogo funciona melhor quando cada personagem quer algo diferente da troca, mesmo que pequeno (quero que ele pare de falar, quero mudar de assunto, quero uma desculpa, quero provar que estou certo).
- Se os dois personagens querem exatamente a mesma coisa e concordam o tempo todo, a cena não tem atrito, e sem atrito não há motivo pra ela existir em prosa (mesma regra do item 0, função do diálogo).
- Personagens discordando, mesmo em detalhe pequeno, empurra a cena adiante. Contraposição vale mais que consenso.
- "Diálogo é a expressão verbal da ação": pensar em cada fala como uma tentativa de fazer algo ao outro personagem (convencer, provocar, proteger, testar), não como uma frase que só "soa bem".

---

## 7. Ritmo: frases curtas na tensão, silêncio como recurso

- Em momentos de alta tensão, encurte. Frases longas e elaboradas soam calmas, o que é errado pro momento de pico.
- Máximo prático: três frases por fala em cena de tensão; se o personagem "discursa", quebre com uma interrupção, uma reação do ouvinte, ou um beat de ação.
- **Silêncio é um recurso ativo, não um vazio.** Uma pausa não preenchida (marcada por reticências, ação, ou simplesmente a próxima fala não respondendo à pergunta) comunica mais do que enchimento verbal.
- Interrupções soam naturais e ativas: pessoas de verdade cortam a fala umas das outras. Usar com moderação em prosa (excesso vira ruído visual), mas não ter medo de interromper quando a emoção da cena pede.
- Varie o comprimento de fala entre personagens na mesma cena. Se todos falam no mesmo metrônomo, a cena soa monótona mesmo com vozes lexicalmente diferentes.

---

## 8. Formatação: aspas, não travessão (convenção GusWorld)

Fontes de mercado brasileiro (Wagner RMS, Editora Flyve) recomendam o traço longo de abertura de fala (o "travessão", caractere de pontuação típico do padrão editorial brasileiro para romance impresso) como padrão de mercado. **Esta é a prática de mercado, não a convenção deste projeto.**

**O GusWorld usa aspas para fala, com zero em-dash em toda a prosa narrativa** (regra de projeto já consolidada, ver memória `feedback_dialogo_travessao_vol2`, que registra que a prática real em Vol. 2/contos já é só aspas, sem exceção de travessão). Isso está alinhado à regra anti-em-dash mais ampla do projeto, aplicada a toda prosa, com única exceção documentada em `docs/narrative/in-world-docs.md` para prosa autoral in-character (que também não usa travessão, usa outros recursos).

**Prática recomendada para o GusWorld:**
- Fala entre aspas duplas: "Não vou fazer isso," disse Cauã.
- Beat de ação como frase própria, sem precisar de vírgula/travessão de interrupção: Cauã cruzou os braços. "Não vou fazer isso."
- Interrupção de fala dentro da própria fala: usar reticências resolve sem quebrar a regra anti-em-dash; cortar a frase e retomar com ação também funciona.
- **Nunca introduzir o traço de abertura de fala como "correção de estilo"** ao revisar diálogo existente, mesmo que a fonte de mercado recomende. Se um revisor externo (ou fonte de craft) sugerir esse traço, a resposta é: convenção GusWorld é aspas, mantido deliberadamente.

---

## 9. Erros comuns (o que faz diálogo soar ruim)

| Erro | Sintoma | Correção |
|---|---|---|
| On-the-nose | Personagem diz exatamente o que sente/pensa | Trocar por ação, ironia, ou fala oblíqua |
| Voz intercambiável | Duas falas trocam de personagem sem soar errado | Reforçar léxico/ritmo únicos por personagem (teste do item 2) |
| Info-dump / "as you know, Bob" | Personagens recontam fatos que já sabem | Mover pra narração, criar personagem ignorante, ou revelar por conflito |
| Advérbio de tag | "disse raivosamente" | Reescrever a fala ou trocar por beat de ação |
| Monólogo sem interrupção | Um personagem fala parágrafos sem resposta | Quebrar com reação do ouvinte, pergunta, ou beat físico |
| Realismo tedioso | Cumprimentos, "oi tudo bem", hesitações reais completas | Cortar; prosa não é transcrição |
| Sem conflito de objetivo | Os dois personagens concordam o tempo todo | Dar a cada um algo diferente que ele quer da troca |
| Sotaque fonético pesado | Escrever "cê" "num" "spera" excessivamente | Sinalizar registro/classe/região com léxico e ritmo, não ortografia fonética |

### Antes / depois rápido

**Antes (on-the-nose, tag com advérbio, voz genérica):**
> "Estou com muito medo de que o Sterling descubra o plano," disse Jaci, nervosamente.
> "Não se preocupe, vai dar tudo certo," respondeu Cauã, calmamente.

**Depois (subtexto, beat de ação, vozes diferenciadas via linguagem-âncora Pythia):**
> Jaci girou o bio-scanner entre os dedos, sem olhar pra ninguém. "E se ele já sabe?"
> "Então a gente reescreve o script." Cauã deu de ombros, mas os olhos não saíram da porta. "Pythia perdoa erro. A gente é que não pode."

---

## 10. Como aplicar este guia às falas já existentes do GusWorld

Passo a passo pra revisão retroativa (uso pelo `revisor-textual` ou `narrative-writer` em auditoria de diálogo):

1. **Isolar todas as falas de um personagem** num doc/capítulo e ler só elas em sequência (sem o resto da prosa). Pergunta: soa consistente com a linguagem-âncora e a idade dele? Alguma fala poderia pertencer a outro companion sem soar errado?
2. **Rodar o teste do item 0** em cada troca de diálogo: essa fala avança enredo, revela personagem, ou cria/libera tensão? Se não, marcar para corte.
3. **Caçar advérbios de tag e verbos dicendi exóticos** (busca de texto por "-mente" perto de aspas de fechamento, e por verbos como "vociferou", "esbravejou", "indagou").
4. **Caçar info-dump:** qualquer troca onde os dois personagens já deveriam saber o que estão dizendo um ao outro.
5. **Conferir zero em-dash** em blocos de diálogo (regra de formatação do projeto).
6. **Casos-sensíveis:** falas do Dante "Grid" (checar duplo sentido inocente/culpado), falas de exposição de sistema de magia/facção (checar se estão em diálogo, deveriam estar em documento in-world ou ação, não em fala recapitulativa).
7. Registrar achados no formato já usado pelo `revisor-textual` (severidade CRÍTICO/MÉDIO/LEVE) se a auditoria for formal.

---

## Fontes consultadas

**As 7 fontes indicadas:**
1. Writing Dialogue: How to Master Dialogue in Fiction, Jericho Writers: https://jerichowriters.com/writing-dialogue/
2. Como escrever diálogos, Vil Toreis: https://viltoreis.com/escrever-dialogos/
3. A arte do diálogo: criação e formatação de diálogos impactantes, Wagner RMS: https://wagnerrms.com/a-arte-do-dialogo-criacao-e-formatacao-de-dialogos-impactantes/
4. Como escrever diálogos, Editora Flyve: https://www.editoraflyve.com/post/como-escrever-diálogos
5. Como criar diálogos realistas para seu livro, Blog Clube de Autores: https://blog.clubedeautores.com.br/2020/09/como-criar-dialogos-realistas-para-seu-livro.html
6. 9 dicas para escrever diálogos melhores, Roni Zealine: https://www.ronizealine.com/9-dicas-para-escrever-dialogos-melhores/
7. Dicas para o seu primeiro livro, 6º passo: Diálogo, Wattpad: https://www.wattpad.com/307982825-dicas-para-o-seu-primeiro-livro-6º-passo-diálogo

**Fontes adicionais (busca web):**
8. Subtext in Writing: The Hidden Depths, Atmosphere Press: https://atmospherepress.com/subtext-in-writing/
9. Adverbs for Dialogue: Do You Need Them in Dialogue?, ProWritingAid: https://prowritingaid.com/adverbs-for-dialogue
10. Everything Authors Need to Know About Dialogue Tags, Nathan Bransford: https://nathanbransford.com/blog/2020/05/everything-authors-need-to-know-about-dialogue-tags
11. Top 14 Tips and Tools for Creating Unique Character Voices, Helping Writers Become Authors: https://www.helpingwritersbecomeauthors.com/character-voices/
12. Good branching dialogue: the basics, Video Games Writing (rharwick.com): https://www.rharwick.com/blog/good-branching-dialogue-the-basics
13. How to Avoid the "As You Know, Bob" Trope in Dialogue, Helping Writers Become Authors: https://www.helpingwritersbecomeauthors.com/as-you-know-bob/

**Cross-ref interno:** `docs/narrative/characters/party.md` (matriz de linguagens-âncora), `docs/design/pillars.md` (Pillar 4), `docs/narrative/in-world-docs.md` (exceção de em-dash), memória de projeto `feedback_dialogo_travessao_vol2.md`.

---

**Autoria:** compilado por agente de pesquisa a pedido do time narrativo GusWorld, 2026-07-08. Guia de ofício, não documento canônico de lore, atualizável sem processo de aprovação de criador supremo (mas mudanças de convenção de formatação, se propostas, seguem a regra normal do projeto).
