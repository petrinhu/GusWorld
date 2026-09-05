<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Conquistas (GusWorld) — especificação `.gw.achv`

> **Origem:** item `D18` do `TODO.md`. Formaliza, em blueprint estrutural (não em prosa final — este documento é arquitetura, a redação de banner/copy cabe a `narrative-writer` quando `present/` nascer), as **sete conquistas cross-ato** listadas na tabela "Distribuição expandida pelos atos" de `docs/narrative/comic-reliefs.md`: EE-4, EE-9, EE-10, EE-12, EE-16, EE-18, EE-20.
>
> **Ponteiros (L-30):** `docs/narrative/comic-reliefs.md` (linhas 918-1093, seção "Homenagens diégeticas", e linha 2088, tabela "Distribuição expandida pelos atos") · `docs/tech/convencao-formatos-gw.md` (definição de `.gw.achv` e das duas naturezas da conquista, linhas 89-114).
>
> **Fora de escopo aqui, por lei:** a apresentação do banner de conquista (texto na tela, animação, som) é `present/`, bloqueada pelo GlintFx e ainda inexistente (L-06, L-27). Este documento especifica só o **gatilho de domínio** — a condição verificável na camada de regra — e o **estado mínimo** que o save precisa registrar. Nomear, redigir dica final e desenhar a UI do banner são etapas posteriores, de outros agentes.

---

## 0. Regras estruturais válidas para as sete (derivadas do `.gw.achv`, não inventadas aqui)

Antes de especificar cada conquista, três invariantes já fixados em `convencao-formatos-gw.md` valem para as sete sem exceção, e evitam repetir o mesmo raciocínio sete vezes:

1. **Duas naturezas que não moram juntas.** O que a conquista É (nome, dica, condição de destravar, oculta ou não) vive no catálogo `.gw.achv`, igual para todo save. O que aconteceu com ela (destravou ou não) vive no envelope binário selado do jogador (L-25). Este documento especifica só o primeiro; o segundo já está decidido — ver item 2.

2. **O save guarda um CONJUNTO de `id`, não um contador de destravamentos.** Por construção, um conjunto não distingue "destravei uma vez" de "destravei três vezes": um `id` está dentro ou fora dele. **Logo, nenhuma das sete conquistas é destravável mais de uma vez** — não por escolha de design de cada uma, mas porque a estrutura de dado que as guarda não tem onde registrar uma segunda vez. Isto NÃO impede que o **contador que alimenta a condição** (bestiário documentado, mortes na mesma cena, sequência sem acerto) continue vivo e mudando depois do destravamento — só a **conquista em si** não retravamento nem re-destrava.

3. **Reocorrência do gatilho após já destravada = no-op idempotente.** Se a condição volta a ser satisfeita depois que o `id` já está no conjunto do save (ex.: o jogador documenta o 101º inimigo, ou morre pela 101ª vez na mesma cena), nada acontece: inserir um `id` já presente num conjunto não muda o conjunto. É a mesma garantia que o `D17` já exige para o baú (abrir duas vezes não gera duas cartas) — aqui aplicada ao mesmo formato de dado.

Essas três regras respondem, de uma vez, à pergunta "e se o gatilho ocorrer com o jogo em estado inválido (destravamento duplicado)?" para as sete conquistas. Onde uma conquista tiver um estado inválido **próprio**, além deste, ele está descrito na seção dela.

**Nomenclatura (L-22):** identificador em inglês, `snake_case`, prefixo `achv_`. Nome de exibição e dica são prosa/asset (direitos reservados, L-08); o `id` e a condição de destravar são regra (código, AGPL). Os nomes de exibição abaixo são os que já existem em `comic-reliefs.md` — citados, não inventados; onde não existem, isso está marcado.

---

## 1. Tabela-resumo

| `id` | Origem | Gatilho coberto no corpus? | Oculta |
|---|---|---|---|
| `achv_byte_collector` | EE-9 | Sim, com uma tensão numérica a levar ao líder (§2.9) | Sim |
| `achv_hospital_plaque` | EE-10 | **Não — lacuna** (§2.10) | — |
| `achv_persistent_geometry` | EE-12 | Sim, com uma dependência de infraestrutura a levar ao líder (§2.12) | Sim |
| `achv_blue_shell` | EE-16 | **Não — lacuna** (§2.16) | — |
| `achv_encourager` | EE-18 | Parcial: gatilho descrito, mas nunca enquadrado como "conquista oculta" (§2.18) | A confirmar |
| `achv_emergency_meeting` | EE-20 | Sim | Sim |
| `achv_recovery_mushroom` | EE-4 | **Não — lacuna** (§2.4) | — |

Três das sete (EE-4, EE-10, EE-16) não têm condição de destravar em lugar nenhum do corpus — são item, placa e cena de diálogo, não conquistas com gatilho declarado. Ficam na tabela porque `comic-reliefs.md` as lista na linha "Cross-ato (conquistas)", mas a especificação delas é uma pergunta ao líder (§3), não uma invenção minha.

---

## 2. Especificação por conquista

### 2.4 — `achv_recovery_mushroom` (EE-4, "O Cogumelo-Recuperador") — LACUNA

**O que o corpus diz** (`comic-reliefs.md`, EE-4): item raro, dropável em zonas de bioma misto no final do Ato 2. Usar = +1 vida extra ao Gus (revive automático se HP=0 na próxima batalha). Isto é especificação de **item** (`.gw.item` ou `.gw.card`, a decidir por quem especificar o item em si), não de conquista: **não há, em lugar nenhum do texto, um nome de conquista, uma dica de conquista, nem uma condição de destravar** ligada a este item.

**Por que aparece na lista mesmo assim:** `comic-reliefs.md` linha 2088 cita "EE-4 (cogumelo)" na linha "Cross-ato (conquistas)" da tabela de distribuição. A cena EE-4 em si nunca usa a palavra "conquista" — ela está na seção "Homenagens diégeticas" ao lado de EE-3 (Encanador), que é puro flavor sem qualquer gatilho de sistema.

**O que eu NÃO faço:** inventar que existe uma conquista "achou o cogumelo" ou "usou o cogumelo pra reviver". Nenhuma das duas está no corpus. Levo a decisão ao líder em §3.

---

### 2.9 — `achv_byte_collector` (EE-9, "Pegue Todos os Bytes")

**Nome (asset, citado do corpus):** "Coletor de Bytes".
**Dica (asset, citada do corpus):** *"Você documentou 100 inimigos. Os Anciões da Pilha Sobrecarregada te reverenciam."*
**Oculta:** sim ("conquista oculta", texto explícito do corpus).

**Gatilho de domínio:** um contador cumulativo de eventos de documentação de bestiário atinge **100**. Em termos de evento de domínio (vocabulário já em uso no `combat.md`, `CombatBus`): cada vez que uma entrada do Bestiário do Diário é criada ou completada, o sistema de bestiário emite um evento (ex.: `BestiaryEntryDocumented(foe_id, stage)`); um avaliador de conquistas ouve esse evento e incrementa o contador.

**Estado mínimo a contar:** um único inteiro cumulativo (não um conjunto de quais inimigos, não posicional — só a contagem). Comparar com o invariante do `.gw.box` do `D17`: aqui não há duplicação a prevenir por item, só a soma de eventos.

**Destravável mais de uma vez:** não (regra 2 do §0).

**Estado inválido:** coberto pela regra 3 do §0 (reocorrência após destravado = no-op). Um estado inválido próprio: se o save for carregado com o contador **maior** que 100 mas o `id` **ainda fora** do conjunto de destravadas (ex.: save migrado de uma versão do jogo anterior à existência desta conquista), a avaliação relê o contador armazenado e destrava no primeiro carregamento seguinte — não é preciso "voltar no tempo" e recontar os eventos, só reavaliar o valor já registrado contra o limiar.

⚠️ **Tensão numérica a levar ao líder, não resolvida aqui:** `docs/narrative/diary/entries-fichas-bestiary.md` (§"Sumário das fichas catalogadas") fixa o total catalogável do Bestiary em **"~22 inimigos turn-based"**. A meta de 100 do EE-9 é quase 5× esse número. Duas leituras possíveis, e as duas são compatíveis com o texto de EE-9 ("documentou 100 inimigos"), mas dão contadores diferentes:

- **(A) Contagem cumulativa de eventos**, contando toda vez que um inimigo é documentado (incluindo reencontros com o mesmo inimigo já catalogado antes — cada abate ou cada "1º combate" novo soma, mesmo repetindo espécie). Compatível com "100" mesmo havendo só ~22 espécies.
- **(B) Contagem de entradas ÚNICAS completas do catálogo.** Neste caso 100 é **matematicamente impossível** com ~22 espécies catalogáveis, e o número do corpus está errado ou o catálogo precisa crescer.

Não decido entre as duas (L-14). Pergunta formal em §3. **✅ RESOLVIDO em 30/08/2026 (`G12` do `TODO.md`, Eixo 2 de `docs/_secret/proposta-balanceamento-easter-eggs.md`):** nenhuma das duas leituras — o líder escolheu criar o conceito de **documentação multi-estágio por espécie** (avistamento, primeiro combate, análise completa, fraqueza descoberta, item de drop identificado), mantendo o limiar em **100**. Com ~22 espécies × 5 estágios ≈ 110, o limiar fica alcançável pela leitura de entradas únicas, sem baixar o número nem reescrever o asset. **Trabalho novo derivado, ainda não feito:** aplicar o conceito de estágio em `docs/narrative/diary/entries-fichas-bestiary.md` — item `D32` do `TODO.md`, pré-requisito para este gatilho funcionar de fato.

---

### 2.10 — `achv_hospital_plaque` (EE-10, "A Placa do Hospital") — LACUNA

**O que o corpus diz** (`comic-reliefs.md`, EE-10): placa de flavor na entrada do hospital, com a letra miúda de piada sobre doação acelerar a cura. É texto ambiental estático (o mesmo tipo de conteúdo de EE-3, EE-5, EE-13, EE-14, EE-17, EE-21, EE-22 — nenhuma delas é conquista). **Não há gatilho, nem contador, nem nome ou dica de conquista** associados a ela em nenhum lugar do texto.

**Por que aparece na lista mesmo assim:** mesma situação do EE-4 — citada na linha "Cross-ato (conquistas)" de `comic-reliefs.md` sem que a cena original a trate como sistema.

**O que eu NÃO faço:** inventar "leu a placa" ou "visitou o hospital pela primeira vez" como gatilho — nenhum dos dois está no corpus, e a maioria dos EE-N regionais (que este se pareceria) não vira conquista. Pergunta ao líder em §3.

---

### 2.12 — `achv_persistent_geometry` (EE-12, "Persistência Geométrica")

**Nome (asset, citado do corpus):** "Persistência Geométrica".
**Dica (asset, citada do corpus):** *"Você morreu 100 vezes na mesma cena. Nós respeitamos isso."*
**Oculta:** sim ("conquista oculta", texto explícito do corpus).

**Gatilho de domínio:** o contador de derrotas de Gus dentro de uma mesma identidade de cena atinge o limiar de dificuldade do save (tabela abaixo, não mais um valor único). Em vocabulário de evento já estabelecido em `combat.md` §16 (`CombatBus`): `CombatEnded(outcome=Defeat, payload)` seguido de `ActorDefeated(gus)` — o avaliador de conquistas incrementa, por identidade de cena, um contador de derrotas.

**✅ RESOLVIDO em 30/08/2026** (`G12` do `TODO.md`, Eixo 5 de `docs/_secret/proposta-balanceamento-easter-eggs.md`, `AskUserQuestion`): o valor único de 100 fica revogado (L-24) e substituído por um limiar escalado por dificuldade:

| Dificuldade | Limiar de derrotas na mesma cena |
|---|---|
| Fácil | 34 |
| Médio | 55 |
| Difícil | 89 |
| Hardcore | 144 |

Segue **sem efeito mecânico**, só o banner de piada, sem economia a desequilibrar. **Pendente, fora de escopo desta atualização:** a linha "Dica" acima ainda cita o valor único "100"; reescrevê-la é trabalho de `narrative-writer`, tal qual `D33` fez para o Eixo 1.

**Estado mínimo a contar:** um mapa `id_cena → contagem de derrotas`, não um único inteiro global, a condição é "N vezes **na mesma** cena", com N dado pela tabela acima, então derrotas em cenas diferentes não se somam entre si.

**Destravável mais de uma vez:** não (regra 2 do §0). Uma vez que QUALQUER cena atinja o limiar da dificuldade do save, a conquista destrava e não retrava; uma segunda cena atingir o limiar depois é no-op (regra 3 do §0).

**Estado inválido:** coberto pela regra 3 do §0. Um caso próprio: se o mesmo evento de derrota for processado duas vezes por um bug de replay/rollback do save (L-25, fase 2, verificação por re-execução), o incremento tem que ser amarrado à identidade do evento de domínio, não à leitura do save, senão o contador infla sem uma derrota real correspondente. Isto não é peculiar desta conquista; é a mesma exigência de determinismo que `D13` já coloca para todo o resto do estado ("mesma semente mais mesma lista de comandos reproduz o estado final byte a byte").

⚠️ **Dependência de infraestrutura, não decidida aqui:** "mesma cena" pressupõe uma identidade estável de cena/encontro que **sobreviva a tentativas repetidas**. O `combat.md` já emite `CombatStarted(encounter)`, o que prova que existe um conceito de `encounter` no domínio, mas não está documentado em lugar nenhum se esse identificador é **da instância de combate** (um novo `encounter` a cada tentativa, o que tornaria o limiar de derrotas na mesma cena incontável, porque cada tentativa teria um id diferente) ou **do local/configuração fixa** que gera a luta (o mesmo `encounter` toda vez que o jogador entra ali, o que é o que esta conquista precisa para funcionar). Busca em `docs/` não encontrou nenhum documento que resolva essa distinção (`scene_id`/`encounter_id`/`room_id`, zero ocorrências fora deste rascunho). Pergunta formal em §3.

---

### 2.16 — `achv_blue_shell` (EE-16, "Casca Azul") — LACUNA

**O que o corpus diz** (`comic-reliefs.md`, EE-16): diálogo casual entre Cauã e Linda durante descanso, sobre uma corrida perdida no minigame de EE-15 por causa de uma casca azul. É cena de humor (mesma família das Cenas 1-14, 16-19 do documento), não um sistema com condição de destravar. **Não há nome de conquista, dica, nem gatilho** em nenhum lugar do texto — nem mesmo a palavra "conquista".

**Por que aparece na lista mesmo assim:** citada na linha "Cross-ato (conquistas)" de `comic-reliefs.md`, junto das outras seis, sem que o texto da cena a trate como tal.

**O que eu NÃO faço:** inventar que "assistir a esta cena" ou "perder uma corrida do EE-15 com casca azul" é gatilho de conquista — nenhum dos dois está descrito como condição de destravar, e as outras cenas de diálogo cômico (Cenas 1-19) não têm equivalente de conquista. Pergunta ao líder em §3.

---

### 2.18 — `achv_encourager` (EE-18, "O Encorajador") — parcial

**O que o corpus diz** (`comic-reliefs.md`, EE-18): "Sistema oculto: se Gus morre 10 vezes na mesma cena específica, NPC aparece no save ou hospital" com uma fala de encorajamento, oferece **Token-de-Coragem** (regenera 1 vida ao iniciar combate) e grava uma entrada de Diário. Há também uma mensagem opcional de carregamento.

**Isto TEM gatilho explícito** (diferente de EE-4/EE-10/EE-16), mas **nunca é enquadrado como "conquista oculta"** com nome e dica de banner, ao contrário de EE-9/EE-12/EE-20 — é descrito como sistema/evento narrativo. Por isso a especificação abaixo cobre o que É derivável, e a seção de perguntas cobre o que não é.

**Gatilho de domínio (derivado, não inventado):** o contador de derrotas de Gus dentro de uma mesma identidade de cena atinge **10** — mesmo mecanismo de `achv_persistent_geometry` (§2.12), limiar diferente e fixo em toda dificuldade (o de §2.12 passou a escalar por dificuldade desde 30/08/2026, este continua em 10).

**Estado mínimo a contar:** o mesmo mapa `id_cena → contagem de derrotas` de §2.12 — não é um contador separado; é o MESMO contador lido num limiar mais baixo, o que sugere as duas conquistas compartilham a mesma fonte de estado em vez de duplicá-la.

**Destravável mais de uma vez:** conforme a regra 2 do §0, a **conquista** (se vier a existir uma) não retrava. Mas o **efeito narrativo** (a fala do NPC, o Token-de-Coragem) — que É explicitamente descrito no corpus — pode ser projetado para repetir a cada cena distinta que atingir 10 derrotas, já que o texto fala em "cena específica" no singular mas o propósito de design (apoio ao jogador que está travado) sugere um sistema geral de suporte, não um evento único de campanha. Não decido isso; ver §3. **✅ RESOLVIDO em 30/08/2026** (`G12` do `TODO.md`, Eixo 3 de `docs/_secret/proposta-balanceamento-easter-eggs.md`): repete, sim — o Token é concedido **uma vez por identidade de cena**, não uma vez por campanha; magnitude e escopo no bloco de §3, pergunta 7.

**Estado inválido:** mesma regra 3 do §0 para o aspecto conquista; para o aspecto sistema de suporte (entrega de item), a idempotência também deveria valer por cena — entrar na cena pela 11ª vez sem sair não deveria entregar um segundo Token-de-Coragem.

⚠️ **Dependência de infraestrutura:** mesma lacuna de identidade de cena do §2.12.
⚠️ **Dependência de catálogo:** "Token-de-Coragem" não existe em nenhum arquivo `.gw.item`/`.gw.card` hoje (busca no repositório inteiro retorna só esta única menção, em `comic-reliefs.md`). A entrega deste efeito depende de o item ser especificado em outro lugar antes de existir de fato.

---

### 2.20 — `achv_emergency_meeting` (EE-20, "Reunião de Emergência")

**Nome (asset, citado do corpus):** "Reunião de Emergência".
**Dica (asset, citada do corpus):** *"Você chamou todo mundo pra sala e não resolveu nada. De novo. Aqui, confiança é opcional."*
**Oculta:** sim ("conquista oculta", texto explícito do corpus).

**Gatilho de domínio:** dentro do minigame do EE-19 ("O Time de Tarefas"), Gus aperta o botão vermelho e **erra** a adivinhação de qual grade a figura sumida usou, **três vezes seguidas**, sem nenhum acerto no meio.

**Estado mínimo a contar:** um único contador de **sequência** (streak) de tentativas malsucedidas consecutivas no minigame do botão vermelho. Precisa de duas transições: incrementa a cada erro; **zera a zero a cada acerto** — a palavra "seguidas" no corpus é estrita, então um acerto no meio de duas tentativas erradas invalida a sequência anterior por completo, não só "não conta para o total".

**Destravável mais de uma vez:** não (regra 2 do §0).

**Estado inválido:** coberto pela regra 3 do §0. Caso próprio: se o jogador atinge a sequência de 3 erros e SÓ DEPOIS o jogo verifica a condição (ex.: verificação em lote em vez de por evento), o resultado tem que ser o mesmo que verificar erro a erro — destravar no instante do terceiro erro consecutivo, não em algum momento posterior arbitrário.

⚠️ **Dependência de infraestrutura, não plena mas registrada:** o gatilho em si está bem descrito, mas o EE-19 (o minigame do botão vermelho) ainda não tem uma especificação própria de estados (o que conta como "aperta o botão", como o "acerto"/"erro" é resolvido, se há um limite de tentativas por sessão de jogo). Esta seção assume que tal especificação virá a existir e só descreve a fatia que toca a conquista; não é uma segunda pergunta ao líder, porque EE-19 não está entre as sete pedidas neste item, fica registrado para quem especificar o minigame em si.

**✅ GUARDRAIL FIXADO em 30/08/2026** (`G12` do `TODO.md`, Eixo 4 de `docs/_secret/proposta-balanceamento-easter-eggs.md`, `AskUserQuestion`), para quem especificar EE-19: o número de grades/opções por rodada escala por dificuldade.

| Dificuldade | Grades/opções por rodada |
|---|---|
| Fácil | 3 |
| Médio | 5 |
| Difícil | 8 |
| Hardcore | sem valor decidido na fonte, não inventado aqui |

O limiar de "3 erros seguidos" do corpus **não muda**. Nota de argumento, corrigida na própria decisão: mais grades tornam a conquista **mais acessível**, não mais rara, o piso protege a alcançabilidade orgânica (destravar sem grinding deliberado), não a raridade.

---

## 3. Perguntas ao líder (lacunas e ambiguidades que este documento NÃO resolve)

Nenhuma das sete abaixo foi decidida por mim (L-14). Cada uma referencia a seção correspondente.

1. **EE-4 (§2.4) — existe conquista aqui, ou é só o item?** O corpus só descreve o Cogumelo-Recuperador como item de revive. Se o líder quiser uma conquista ligada a ele, preciso saber qual condição: obter o item pela primeira vez, ou usá-lo (reviver) pela primeira vez — as duas são leituras plausíveis e nenhuma está escrita.
2. **EE-10 (§2.10) — existe conquista aqui, ou é só a placa?** Nenhuma condição de destravar está descrita. Se o líder quiser uma, preciso de qual evento a dispara (ler a placa? entrar no hospital pela primeira vez? algo ligado à piada da doação?).
3. ~~**EE-9 (§2.9) — "100 inimigos" é contagem cumulativa de eventos ou de entradas únicas?**~~ **✅ RESOLVIDO em 30/08/2026** pelo líder (`G12` do `TODO.md`, `AskUserQuestion`): nem cumulativa nem entrada única simples — cria-se **documentação multi-estágio por espécie** (5 estágios), limiar mantido em 100. Detalhe em §2.9 acima e em `docs/_secret/proposta-balanceamento-easter-eggs.md` Eixo 2.
4. **EE-12 e EE-18 (§2.12, §2.18) — "mesma cena" é identidade de local fixo ou de instância de tentativa?** Preciso saber se o domínio já tem (ou vai ganhar) um identificador de cena/encontro que persista através de retries — sem isso, nem "100 vezes na mesma cena" nem "10 vezes na mesma cena específica" são implementáveis como descritas.
5. **EE-16 (§2.16) — existe conquista aqui, ou é só a cena de diálogo?** Nenhuma condição de destravar está descrita, e nenhuma das outras 18 cenas de humor tem conquista equivalente. Se o líder quiser uma, preciso de qual evento a dispara.
6. **EE-18 (§2.18) — é uma conquista de catálogo (banner com nome e dica) ou um sistema de suporte sem entrada em `.gw.achv`?** Ao contrário de EE-9/EE-12/EE-20, o corpus nunca chama isto de "conquista oculta" — é "sistema oculto". Preciso saber se ele entra no catálogo `.gw.achv` (e aí precisa de nome e dica próprios, que também não existem) ou se fica de fora, e a linha da tabela de `comic-reliefs.md` está usando "conquista" num sentido mais largo que o `.gw.achv` formaliza.
7. ~~**EE-18 (§2.18) — o efeito (Token-de-Coragem + NPC) repete a cada cena que atingir 10 derrotas, ou é um evento único de campanha?**~~ **✅ RESOLVIDO em 30/08/2026** pelo líder (`G12` do `TODO.md`, `AskUserQuestion`): repete por cena distinta (Opção A de `docs/_secret/proposta-balanceamento-easter-eggs.md` Eixo 3), com escala do líder — Token concedido uma vez por identidade de cena, cura por dificuldade (Fácil 3% / Médio 5% / Difícil 8% do HP-máx, `Regen` no primeiro turno seguinte), Hardcore sem Token. Detalhe em §2.18 acima.

---

## 4. Cross-refs

- `docs/narrative/comic-reliefs.md` — fonte das sete cenas/itens e da tabela de distribuição.
- `docs/tech/convencao-formatos-gw.md` — definição do `.gw.achv`, as duas naturezas da conquista, o que o save guarda.
- `docs/narrative/diary/entries-fichas-bestiary.md` — sistema de Bestiário citado no gatilho de EE-9 e na tensão numérica da pergunta 3.
- `docs/design/mecanicas/combat.md` §16 — vocabulário de evento (`CombatBus`, `CombatStarted(encounter)`, `ActorDefeated`, `CombatEnded`) usado para derivar os gatilhos de EE-12 e EE-18.
- `docs/design/mecanicas/modos-morte.md` — dispatcher de derrota de Gus, ponto de origem provável do evento que alimenta EE-12/EE-18.
- `GODS_LAWS.md` L-04, L-14, L-17, L-18, L-22, L-25, L-33 — atomicidade, não-decisão de agente, regra como comando/evento, envelope selado, nomenclatura, proteção de dado, revisão em cinco perguntas.
