# PROPOSTA — critério e distribuição de intensidade do campo PEM nas 13 dungeons

> **Status: PROPOSTA apreciada e superada.** O líder julgou esta proposta em 30/08/2026, por `AskUserQuestion`, e decidiu diretamente: **12 dungeons em intensidade total e 1 em intensidade fraca** — a de abertura, "Dutos — aparato (abertura)". Isto **substitui** a distribuição de 5 fraca / 8 total que este documento propunha (Seção 5); o canon vigente é `docs/design/mecanicas/save-por-local.md` §1.2, não a tabela abaixo. O critério de quatro regras (Seções 1-4) não foi adotado como o critério de decisão — só a Regra 1 (abertura → fraca) sobrevive na prática, e por coincidência de resultado, não porque o líder tenha ratificado a hierarquia inteira. Este documento **não foi apagado**: registra o raciocínio apresentado à decisão, mas nenhuma linha da Seção 5 abaixo é canon.
>
> **Item da tabela:** `G8`.
>
> **Base de leitura (só leitura, não editada por este documento):** `docs/design/mecanicas/save-por-local.md` §1.2 e §6 item 2; `docs/design/mundo-topologia.md` §2 e §4.

---

## 0. O que já está fechado (não é decidido aqui)

- Todas as 13 dungeons têm o campo do PEM. A intensidade é **fraca** ou **total**, nunca ausente (`save-por-local.md` §1.2).
- **Total:** salva-se só na porta (entrada/saída); carregar reinicia na porta; save manual e autosave desligados fora da porta.
- **Fraca:** o campo bloqueia só o autosave dentro da dungeon; o save manual continua disponível pelo menos na porta.
- A percepção de qual intensidade está em vigor é por **sinal diegético no menu de save**, sem tutorial em texto (decisão do líder, 28/08/2026, `save-por-local.md` §3).
- **A intensidade é dado do mapa** — atribuída por dungeon, estática por design, não calculada em tempo de jogo a partir do desempenho do jogador. Isto casa direto com o corte `C-06` da L-29 (proibido dificuldade dinâmica adaptativa): a proposta abaixo **não introduz nenhuma adaptação por jogador**, é uma tabela fixa, decidida na mesa de design.

O que falta, e é objeto deste documento: **qual das 13 fica em qual intensidade, e por quê.**

## 1. Por que um critério antes de uma tabela

Uma distribuição de 13 linhas sem regra por trás é palpite disfarçado de decisão. O critério abaixo usa só sinais que o corpus já declara — tipo de dungeon (`mundo-topologia.md` §4, dado explícito e completo para as 13), posição na campanha (abertura / meio / clímax), e, quando o texto o diz, comprimento da incursão. Onde o corpus não diz, este documento **declara a lacuna** em vez de imaginar (Seção 4).

Dois sinais pedidos na tarefa **não existem no corpus hoje**, e é preciso dizer isso com todas as letras:

- **Densidade de encontro por dungeon:** `mundo-topologia.md` §3 mede densidade de **população/quests por área** (cheia × enxuta), não densidade de encontro **dentro** de cada dungeon. Nenhuma das 13 dungeons tem contagem de encontros. Não usado.
- **Ponto de não-retorno:** o único ponto do corpus marcado como fronteira dramática inequívoca é o **clímax** (Selve Profunda, a dungeon final — `save-por-local.md` §4 descreve o clímax do enredo acontecendo nesta janela da campanha). Nenhuma outra dungeon tem marcação textual de ponto de não-retorno. Usado só para a regra 2 (Seção 2).
- **Comprimento da incursão:** declarado explicitamente em só 3 das 13 (Seção 4, tabela de lacunas). Usado como critério primário só nesses 3; nas demais, o **tipo** de dungeon faz esse papel (ele já embute uma expectativa de forma — labirinto pressupõe percurso que se estende até o jogador se situar; "só batalhas" pressupõe atrito por ondas; puzzle puro pressupõe uma resolução, não percurso).

## 2. O critério proposto (hierarquia de regras, a primeira que casar decide)

**Regra 1 — Abertura do jogo → FRACA.**
A primeira dungeon que o jogador visita é também o primeiro contato dele com a própria restrição de save. Isto é uma mecânica nova sendo ensinada, e o corte `C-16` da L-29 exige onboarding orgânico, não punitivo, no primeiro contato com qualquer sistema novo. Impor a versão mais dura logo na primeira dungeon puniria um jogador que ainda não sabe que a regra existe. Só há uma dungeon nesta posição: **Dutos — aparato (abertura)**.

**Regra 2 — Clímax da campanha → TOTAL.**
A dungeon final é o único ponto do corpus com marcação textual de fronteira dramática inequívoca (Seção 1). Neste ponto, o jogador já dominou a mecânica de save há dezenas de dungeons; a convenção do gênero citado como referência (Zelda de SNES, Chrono Trigger — `save-por-local.md` §1.2) reserva o bloqueio mais duro justamente para o trecho final. Só há uma dungeon nesta posição: **Selve Profunda (final)**.

**Regra 3 — Área faraday especial → TOTAL, por ressonância temática, não por tipo.**
A área faraday é tipada como "puzzle puro" na tabela de `mundo-topologia.md` §4 — o que, pela Regra 4 abaixo, apontaria para fraca. Esta é a **exceção deliberada**: é a própria dungeon-tema da Gaiola de Faraday, já descrita como um lugar onde "o scan/HUD cega (sinal morto)" — o jogador já entra num espaço que suprime feedback. Redobrar esse isolamento com a intensidade mais dura do próprio campo que dá nome à dungeon é coerência temática, não capricho; e o custo de refazer é baixo, porque o corpus a descreve como "câmara blindada" (uma câmara, não um percurso longo). Só há uma dungeon nesta posição: **Área faraday**.

**Regra 4 — Para as 10 restantes, decidir por TIPO** (dado explícito e completo em `mundo-topologia.md` §4):

| Tipo | Intensidade | Por quê |
|---|---|---|
| **só batalhas** | TOTAL | A tensão do tipo é atrito por ondas; reiniciar na porta depois de uma derrota é a mecânica clássica do gênero de referência, e reforça o risco em vez de anulá-lo. |
| **labirinto** | TOTAL | A tensão do tipo É se perder; permitir save no meio do labirinto anula a mecânica central dele. |
| **puzzle puro** | FRACA | Puzzle resolvido não ganha tensão ao ser refeito do zero — forçar isso é "backtrack obrigatório sem razão" (anti-padrão de level design), não desafio. A restrição fraca ainda impede autosave num estado de puzzle a meio caminho de uma solução ruim, mas não pune a resolução com repetição sem propósito. |
| **mista** | Sem regra fixa — resolver pelo sinal textual disponível em cada caso (Seção 3), porque o tipo mistura risco e cognição e não empurra sozinho para nenhum lado. |

## 3. As 3 dungeons "mista": resolução caso a caso

Nenhuma das três tem comprimento declarado no corpus (Seção 4). A regra de tipo (Seção 2) não resolve sozinha porque "mista" combina, por definição, o argumento de "labirinto/batalha → total" com o de "puzzle → fraca". Uso dois sinais que **o próprio texto de `mundo-topologia.md` já fornece** para cada uma, em vez de um desempate inventado:

- **Zona do Silêncio #2 → FRACA.** O texto a descreve como "respiro de variedade após a #1" — é o próprio corpus, não uma inferência minha, quem a enquadra como momento de menor pressão dentro da Zona do Silêncio. Uma restrição fraca é consistente com "respiro"; total contradiria a própria intenção declarada do lugar.
- **Selve Sombria → TOTAL.** Aqui o corpus não oferece um sinal de tom equivalente (não é chamada de "respiro" nem de nada parecido). Na ausência desse sinal, uso o **tier de dificuldade** já fixado em `mundo-topologia.md` §2 como critério de desempate declarado: Selve Sombria é tier 4 (difícil), o segundo mais alto das 13 dungeons regulares. Tier mais alto = jogador mais avançado, mais preparado para a versão dura. Esta é uma inferência assumida como tal, não um dado explícito — ver Seção 4.
- **Selve Profunda (final)** já está decidida pela Regra 2 (clímax), não por esta seção.

## 4. Lacunas explícitas do corpus (não preenchidas por invenção)

| O que falta | Onde o texto para | Como esta proposta lidou com isso |
|---|---|---|
| Comprimento de 10 das 13 dungeons | `mundo-topologia.md` §4 dá comprimento só para **Dutos — aparato** ("linear-curta"), **Setor Mirage — Festival** ("labirinto-de-ilusões curto") e **Selve Profunda** ("culminante longa"). As outras 10 não têm adjetivo de comprimento. | Usado o **tipo** de dungeon como proxy declarado (Seção 2), nunca um comprimento inventado. Marcado aqui como indeterminado, não como "curta" ou "longa" presumidas. |
| Densidade de encontro dentro de cada dungeon | Não existe no corpus; `mundo-topologia.md` §3 só mede densidade de população/quests por **área**, não por dungeon. | Não usado como critério (Seção 1). |
| Ponto de não-retorno em qualquer dungeon além do clímax | Não existe marcação textual disso em nenhuma das outras 12. | Não usado além da Regra 2 (Seção 1, Seção 2). |
| Sinal de tom (tipo "respiro") em Selve Sombria ou em qualquer outra das 10 da Regra 4 | Só existe para Zona do Silêncio #2 ("respiro de variedade após a #1"). | Onde existe (Zona do Silêncio #2), usado. Onde não existe (Selve Sombria), a proposta recorre ao tier de dificuldade como desempate assumido — ver Seção 3, e isto está sinalizado como inferência, não fato do corpus. |

Nenhuma linha da tabela da Seção 5 se apoia em conteúdo de dungeon que o corpus não declara (layout sala-a-sala, encontros específicos, itens guardados) — isso é fase de produção, fora do escopo desta proposta e do próprio `mundo-topologia.md` §10.

## 5. Distribuição resultante (13 dungeons)

| # | Dungeon | Área | Tipo (§4 mundo-topologia) | Comprimento no corpus | Regra aplicada | Intensidade |
|---|---|---|---|---|---|---|
| 1 | Dutos — aparato (abertura) | Dutos Infernais | tutorial (+puzzle leve) | curta (explícito) | Regra 1 (abertura) | **FRACA** |
| 2 | Dutos — laboratório FIR (Ato 2) | Dutos Infernais | só batalhas | indeterminado | Regra 4 (tipo) | **TOTAL** |
| 3 | Setor Mirage — Festival | Setor Mirage | puzzle puro | curto (explícito) | Regra 4 (tipo) | **FRACA** |
| 4 | Periferia — vielas | Periferia | labirinto | indeterminado | Regra 4 (tipo) | **TOTAL** |
| 5 | Ferrovelhos — reduto do Dante | Ferrovelhos | só batalhas | indeterminado | Regra 4 (tipo) | **TOTAL** |
| 6 | Zona do Silêncio #1 (acústico) | Zona do Silêncio | puzzle puro | indeterminado | Regra 4 (tipo) | **FRACA** |
| 7 | Zona do Silêncio #2 | Zona do Silêncio | mista | indeterminado | Regra 3 caso a caso ("respiro", texto) | **FRACA** |
| 8 | Orla Recursiva | Orla Recursiva | labirinto | indeterminado | Regra 4 (tipo) | **TOTAL** |
| 9 | Selve Sombria | Selve Sombria | mista | indeterminado | Regra 3 caso a caso (tier 4, desempate assumido) | **TOTAL** |
| 10 | Catedrais #1 (litúrgico) | Catedrais Neo-Sylvania | puzzle puro | indeterminado | Regra 4 (tipo) | **FRACA** |
| 11 | Catedrais #2 | Catedrais Neo-Sylvania | só batalhas | indeterminado | Regra 4 (tipo) | **TOTAL** |
| 12 | Selve Profunda (final) | Selve Profunda | mista (clímax) | longa (explícito) | Regra 2 (clímax) | **TOTAL** |
| 13 | Área faraday | Área faraday especial | puzzle puro (EM/Faraday) | indeterminado (descrita como "câmara", sugere curta) | Regra 3 (ressonância temática, exceção ao tipo) | **TOTAL** |

**Contagem: 5 fraca, 8 total.** Não houve meta de equilíbrio numérico — o canon só pede que a intensidade varie por dungeon (`save-por-local.md` §1.2), não uma proporção. A distribuição resultante tende a endurecer com o avanço da campanha (tier 2: 2 fraca / 3 total; tier 3: 2 fraca / 1 total; tier 4: 1 fraca / 2 total; tier 5 e faraday: total) — uma curva compatível com dificuldade crescente já fixada em `mundo-topologia.md` §2, mas isto é observação posterior à aplicação do critério, não um objetivo que guiou a distribuição.

## 6. Conformidade com o que já foi fixado

- **Não acrescenta campo ao save.** A intensidade proposta aqui é um atributo por dungeon, fixo por design, e — como o líder já registrou — pertence ao **dado do mapa**, não ao envelope de save. O save consulta essa informação (via o requisito já descrito em `save-por-local.md` §5 item 1: "a área carregada precisa expor... se aquele lugar é cidade ou dungeon, e, sendo dungeon, se o PEM local está ativo, desativado ou nunca existiu ali"); esta proposta só preenche **qual valor** (fraca/total) cada uma das 13 dungeons carrega nesse campo do mapa, não muda a forma do save nem do formato binário selado (L-18, L-25). O item `D11` continua livre para nascer com o formato já previsto.
- **Não introduz dificuldade dinâmica.** A tabela da Seção 5 é estática, decidida na mesa de design, sem leitura de desempenho do jogador — respeita o corte `C-06` da L-29.
- **Não decide a forma do sinal diegético.** Isto já está fechado (menu de save, `save-por-local.md` §3) e fora do escopo deste documento; a proposta só diz qual intensidade cada dungeon carrega, não como o menu a comunica.
- **Não toca layout sala-a-sala nem conteúdo específico de dungeon** (fora do escopo, `mundo-topologia.md` §10 item 1).

## 7. O que segue sendo decisão do líder

1. **Aceitar, ajustar ou rejeitar o critério** (Seções 1-4) — inclusive a hierarquia de prioridade das 4 regras, e a exceção temática da Regra 3 para a área faraday.
2. **Aceitar, ajustar ou rejeitar a distribuição resultante** (Seção 5), linha a linha.
3. **Resolver as duas lacunas assumidas por desempate** (Seção 3): Selve Sombria por tier, e a suposição de que "câmara blindada" implica incursão curta na área faraday — se o líder tiver informação de comprimento não registrada no corpus, ela sobrepõe este critério.
4. Qualquer decisão de conteúdo específico por dungeon (o que cada lugar secreto/carta-chave guarda) segue em aberto conforme `mundo-topologia.md` §10, fora do escopo desta proposta.
