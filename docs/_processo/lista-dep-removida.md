# Lista completa de ocorrências de `__DEP_REMOVIDA__` — item `B1a`

**Autor:** technical-writer (agente operacional, levantamento somente leitura)
**Data:** 2026-08-28
**Base legal:** `GODS_LAWS.md` (leis L-11, L-14, L-24, L-25, L-30, L-32)
**Regras seguidas:** L-11 (nada decidido aqui, só listado — a decisão de correção é do líder em `G1`), L-14 (nenhuma ocorrência é declarada morta/lixo por mim; ofereço recomendação, não conclusão), L-24 (toda recomendação abaixo separa a CITAÇÃO órfã da AFIRMAÇÃO por trás dela), L-25 (`docs/book/`, `docs/_secret/`, `docs/narrative/deep/stinger/` são cifrados por git-crypt; contados pelo caminho, conteúdo não citado), L-30 (este documento não é item de tabela, é o anexo que `B1a` referencia).

**Aviso de método, como no relatório-irmão `candidatos-revogacao.md`:** este documento **não decide nada** e **não edita nada**. Nenhum arquivo do projeto foi tocado além deste. Somente leitura, do início ao fim. Escrito seção por seção, incrementalmente.

---

## 1. Contagem total (medida, não estimada)

⚠️ A tabela de pendências estimava **28 ocorrências em 45 arquivos**. A estimativa acertou o número de **arquivos** (45) e errou o de **ocorrências** por uma ordem de grandeza — a estimativa provavelmente contava "casos a julgar" (agrupamentos), não ocorrências literais da string. Meça sempre; não repita o "28" como fato.

**Comando-base:**
```bash
git grep -l "__DEP_REMOVIDA__" | wc -l                              # arquivos
git grep -o "__DEP_REMOVIDA__" | wc -l                               # ocorrências (contagem de string, não de linha)
git grep -c "__DEP_REMOVIDA__" | awk -F: '{s+=$2} END{print s}'      # linhas com pelo menos 1 match, somadas por arquivo
```

| Métrica | Valor medido | Método |
|---|---|---|
| Arquivos afetados | **45** | `git grep -l` |
| Ocorrências da string (uma linha pode ter 2+) | **173** | `git grep -o \| wc -l`, conferido por soma linha a linha (`grep -o` por linha) — dois caminhos independentes bateram: 171 (legíveis) + 2 (cifrados) = 173 |
| Linhas com pelo menos 1 ocorrência | **116** | `git grep -c`, somado por arquivo; conferido por `wc -l` da lista de `git grep -n` (114 legíveis + 2 cifradas) |
| Arquivos cifrados (git-crypt) na lista | **2** | `docs/book/BIBLE-V1-APENDICE-G.md`, `docs/book/BIBLE-V1-PREFACIO.md` — nenhum em `docs/_secret/` nem `docs/narrative/deep/stinger/` |

Os dois caminhos de contagem (soma de `-o` por arquivo vs. soma de `-c` por arquivo, mais a contagem manual linha a linha) convergiram nas três tentativas — sem divergência a investigar.

**Escopo do `git grep`:** só a árvore de trabalho atual (estado do repo hoje), não o histórico. Não há dado de commit anterior a considerar aqui — o marcador foi posto por uma ferramenta de sanitização antes do primeiro commit deste ciclo do projeto (ver §3.5, achado de dano colateral).

---

## 2. Agrupamento por categoria (o pedido central)

Cinco categorias emergiram do material — pelo que a frase **afirma**, não pela contagem:

| Categoria | O que afirma | Linhas | Marcadores | Ação nesta fase |
|---|---|---|---|---|
| **CAT-A** | "a engine própria é C++2x + [dependência]" — afirmação de **stack vigente** | 11 | 12 | Recomendação de correção |
| **CAT-B** | identificador técnico de **API/build/arquivo** da dependência, dentro de ADR ou spec de arquitetura | 65 | 105 | Recomendação de correção |
| **CAT-C** | **crédito de produção / pessoa real**, prosa corrida (não é ADR nem stack-line) | 6 | 6 | Recomendação de correção |
| **CAT-D** | **dano colateral** — corrompeu palavra sem nenhuma relação com a dependência técnica | 2 (1 legível + 1 cifrada) | 2 | Recomendação de correção direta |
| **CAT-E** | **citação do marcador como tópico**, dentro de relatório de auditoria/processo ou da própria lei/tabela — não é dano no corpo do texto | 31 | 47 | Recomendo **não editar** |
| **NC** (não classificado) | conteúdo cifrado, não inspecionado | 1 (cifrada) | 1 | Sem categoria até autorização de abrir o arquivo cifrado |

**Soma de checagem:** 11+65+6+2+31 = 115 linhas classificadas + 1 NC = 116 linhas. Marcadores: 12+105+6+2+47 = 172 + 1 NC = 173. Bate com a contagem total do §1.

---

## 3. CAT-A — Afirmação de stack vigente ("C++2x + `__DEP_REMOVIDA__`")

**O que a frase afirma, em geral:** que a engine do jogo, HOJE, é escrita em C++20 ou C++23 mais a dependência removida. Isso colide de frente com a **LEI ZERO** ("o GusWorld assenta só em GlintFx e no sistema operacional") se lida como descrição do presente. Pode também ser leitura válida de uma decisão **histórica** (o motor que precedeu o GlintFx) — a categoria não decide qual das duas, só sinaliza a colisão.

**Sobreposição com achado anterior:** `docs/_processo/candidatos-revogacao.md` (item A.9) já cobriu 17 ocorrências dessa mesma frase, das quais 2 já estavam em bloco `<details><summary>Histórico</summary>` (fora de escopo de correção) e 15 eram "vigentes". A contagem que meço aqui hoje (11 linhas, algumas com C++20 e outras já com C++23) é **menor** que aquela — decorre de: (a) o item A.9 contava por ocorrência de frase, incluindo `docs/design/mecanicas/combat.md:727` e `docs/design/producao/plano_vs.md:5` e `docs/design/mecanicas/dialogue-tree-npc-intro.md:115`, que **hoje não têm mais o marcador** nesta minha varredura (já corrigidos, ou o caminho mudou — não investiguei qual); (b) as citações que aparecem SÓ dentro de `docs/_processo/candidatos-revogacao.md` (que quota a frase como evidência) vão para CAT-E, não aqui.

| Caminho:linha | Contexto |
|---|---|
| `docs/art/characters/gus.md:70` | "Test render na engine (**C++23 + `__DEP_REMOVIDA__`**) com câmera 3/4 rotacional → screenshot apresentado ao user" |
| `docs/art/style-guide.md:5` | "...hoje superado pelo pivô **C++23+`__DEP_REMOVIDA__`** com arte 2D pixel-art via PixelLab (ver ADR-008...)" |
| `docs/art/style-guide.md:7` | "Solo G1 indie, engine própria **C++23 + `__DEP_REMOVIDA__`**, 2D pixel-art estilizado via pipeline PixelLab..." |
| `docs/design/gdd.md:3` | "Escopo solo G1, engine própria **C++23 + `__DEP_REMOVIDA__`** (ADR-008), PC Linux v1 (Windows pós-v1), single-player." |
| `docs/design/gus-abertura.md:274` | "...Toda peça abaixo foi escolhida por ser BARATA de implementar na stack **C++23+`__DEP_REMOVIDA__`**/2D..." |
| `docs/design/mecanicas/animation-plan.md:41` | "**GUS completo** (piloto) -> integrar na engine **`__DEP_REMOVIDA__`** (preview/viewer) -> validar com o lider." |
| `docs/design/mecanicas/battle-screen.md:3` | "...Implementacao no M5 (BattleScreen) da engine **C++23 + `__DEP_REMOVIDA__`**. ..." |
| `docs/narrative/deep/_INDEX.md:151` | "Cronograma \| Deep-lore prioritário (paralelo orgânico à Fase 2: vertical slice na engine própria **C++23 + `__DEP_REMOVIDA__`**, em andamento)" |
| `docs/narrative/deep/ontologia/leitmotivs-musicais-detalhados.md:142` | "...regida pela infraestrutura de áudio da engine própria (**C++23 + `__DEP_REMOVIDA__`**, áudio via **`__DEP_REMOVIDA__`**; ver ADR-008)..." (2 marcadores) |
| `docs/narrative/diary/knowledge-gates.md:376` | "...depois superado pela engine própria **C++23 + `__DEP_REMOVIDA__`** (ver ADR-008). ..." |
| `docs/narrative/diary/ui-spec.md:411` | "Conteúdo (layer 2): text rendering on-demand via a UI da engine própria (glintfx sobre **C++23 + `__DEP_REMOVIDA__`**; ver ADR-008)." |

**Recomendação (proposta, não aplicada):** tratar como DUAS perguntas separadas por lote, não misturadas:

1. **Se a frase descreve a arquitetura ATUAL** (a maioria destas parece descrever, porque fala no presente e sem marcação de "histórico"): trocar `__DEP_REMOVIDA__` por `GlintFx`, já que hoje é isso que é verdade sob a LEI ZERO. Onde ainda diz "C++20", subir para "C++23" no mesmo passe (L-03, achado A.9 do relatório-irmão já cobre isso).
2. **Se alguma dessas frases pretende registrar uma decisão HISTÓRICA** (o motor que antecedeu o GlintFx, hoje descartado) — nenhuma das 11 linhas acima parece estar marcada como histórica no arquivo em que vive (ao contrário de `docs/art/style-guide.md:227,229`, já dentro de bloco `<details><summary>Histórico</summary>` e por isso fora desta lista) — então a citação órfã (L-24) deve ser **restaurada com o nome real** dentro de um bloco explicitamente histórico, não apagada silenciosamente.

Decisão de qual frase cai em (1) ou (2) é do líder — eu não tenho como saber, lendo cada arquivo isolado, se a intenção editorial era "isto é o presente" ou "isto ficou datado e ninguém atualizou".

---

## 4. CAT-B — Identificador técnico de API/build/arquivo, dentro de ADR ou spec de arquitetura

**O que a frase afirma, em geral:** um fato técnico sobre COMO algo foi construído ou diagnosticado — nome de classe/função/macro/variável de ambiente/arquivo de código da dependência removida, quase sempre dentro de um Architecture Decision Record. Concentração pesada em dois arquivos: **ADR-008** (a decisão original de adoção, 16 linhas) e **ADR-018** (o diagnóstico de um bug real de flash visual, 6 linhas mas densas — até 8 marcadores numa única linha).

**Achado lateral, fora da contagem principal (nome de arquivo, não string `__DEP_REMOVIDA__`):** o próprio arquivo `docs/tech/adr/ADR-008-repivot-qt-to-_.md` tem o nome da dependência raspado do **nome do arquivo** (colapsado para `_`) — mesmo fenômeno da sanitização, mas não capturado por `git grep` porque o nome do arquivo não contém a string literal `__DEP_REMOVIDA__`. Achado semelhante em `docs/design/mecanicas/modos-morte.md:166`, que cita um arquivo hipotético `difficulty_menu_____.hpp/.cpp` (5 underscores seguidos) — provável colapso do mesmo tipo. Não contei estes dois como ocorrências (não batem a string exata pedida), mas registro para o líder decidir se entram no escopo de `B1b`.

**Achado lateral com implicação de compliance:** `docs/tech/adr/ADR-005-license-gpl3-assets-ccbysa.md:117` registra que o **header vendorizado da dependência removida ficou FORA de `third_party/`** e por isso não recebeu o SPDX do projeto — sem o nome real, `THIRD-PARTY-LICENSES.md` não consegue documentar a atribuição correta dessa dependência. Recomendo que esta linha específica tenha prioridade alta na fase de correção (`B1b`), por ter efeito prático fora da prosa.

**Achado lateral de status:** `ADR-008` (a decisão original) tem `Status: Accepted`, sem marca de superação — diferente de `ADR-009` ("Superseded by ADR-010") e de `ADR-002`/`ADR-003` (explicitamente "SUPERADO"). Se a dependência de fato saiu do projeto, o Status do ADR-008 pode precisar de atualização própria — decisão do líder, fora do escopo desta lista (é sobre o campo `Status`, não sobre o marcador).

Tabela por arquivo (contexto resumido; linha completa disponível em `git grep -n "__DEP_REMOVIDA__"` para quem quiser o texto integral):

| Caminho | Linhas | Marcadores | Contexto representativo |
|---|---|---|---|
| `docs/tech/adr/ADR-008-repivot-qt-to-_.md` | 1,10,12,13,14,16,21,23,24,25,26,30,32,36,40,41 | 29 | Título: "Re-pivot da camada de plataforma anterior para a seguinte, mais motor de UI declarativo". Corpo inteiro do ADR descreve a dependência (render 2D, gamepad, áudio, ABI portável, ganho sobre a camada anterior) |
| `docs/tech/adr/ADR-009-motor-ui-hud-embutido.md` | 1,6,14,30,41,42,45,47 | 9 | Título e corpo: "motor de UI/HUD embutido... sobre `__DEP_REMOVIDA__`, dentro das 4 camadas"; invariante de camada "zero `__DEP_REMOVIDA__`" em `core/`/`domain/` |
| `docs/tech/adr/ADR-011-m6-audio-onda1-plano.md` | 6,10,24,49,60,65 | 8 | "O ADR-008 já decidiu a biblioteca de áudio (`__DEP_REMOVIDA__`, vendorizada em `third_party/`)"; "inicializar device `__DEP_REMOVIDA__` + mixer/buses mínimo" |
| `docs/tech/adr/ADR-013-asset-source-vfs-fase1.md` | 6,21,22,25,37,95 | 7 | Caminhos de arquivo hipotéticos: `render2d___DEP_REMOVIDA__.cpp`, `app/src/__DEP_REMOVIDA__window.cpp` (nenhum existe ainda no repo — confirmado, não há `src/`) |
| `docs/tech/adr/ADR-018-contexto-gl-unico-flash-ctx.md` | 11,15,17,21,23,27 | 26 | Diagnóstico de bug real: "`__DEP_REMOVIDA__Renderer`", "`__DEP_REMOVIDA__GL_CreateContext`", "`__DEP_REMOVIDA__ReconfigureWindow`" — linha 11 sozinha tem 8 marcadores |
| `docs/tech/adr/ADR-014-dialogue-runtime-poco.md` | 11,56,151 | 3 | "morreu no pivot para C++20 + `__DEP_REMOVIDA__`"; invariante "ZERO dependência de terceiro, ZERO I/O" |
| `docs/tech/adr/ADR-019-arquitetura-conteudo-atomica-data-driven.md` | 46,52 | 2 | "warning de `switch` não exaustivo só existia no `__DEP_REMOVIDA__` vendorizado"; "layout `__DEP_REMOVIDA__` + input + loop de render" |
| `docs/tech/adr/ADR-021-licenciamento-apache-assets-reservados.md` | 161,234 | 2 | "Caça a irmãos do caso `__DEP_REMOVIDA__`"; "THIRD-PARTY-LICENSES.md (...inclusive o caso `__DEP_REMOVIDA__`)" — referência ao CASO, não a uma API específica |
| o arquivo do `ADR-002` | 5 | 2 | Dentro do campo Status ("SUPERADO"), cronologia: "...depois de C++20 + `__DEP_REMOVIDA__` (ADR-008...)" — já marcado como registro histórico no próprio doc |
| `docs/tech/adr/ADR-003-dialogue-library.md` | 5 | 1 | Status: "Superseded by ADR-014 (...no pivô da engine anterior para a seguinte, ADR-008...)" |
| `docs/tech/adr/ADR-005-license-gpl3-assets-ccbysa.md` | 117 | 1 | Ver achado de compliance acima — "o header do `__DEP_REMOVIDA__` vendorizado FORA de `third_party/`" |
| `docs/tech/adr/ADR-012-m7-paridade-jogavel-plano.md` | 6 | 2 | Cross-ref: "[ADR-008](...) (`__DEP_REMOVIDA__`)" |
| `docs/tech/adr/ADR-017-action-clock-combat-unificado.md` | 29 | 1 | "Host-drawn, não glintfx/`__DEP_REMOVIDA__`." |
| `docs/tech/mapas-formato-legado.md` | 62 | 1 | "logica de transformacao e POCO puro do `domain/` (sem `__DEP_REMOVIDA__`, sem fstream)." |
| `docs/art/vfx-combate-familias.md` | 43 | 1 | "sprites desenhados pelo `__DEP_REMOVIDA__Renderer` em runtime" |
| `docs/design/card-frame-spec.md` | 47 | 1 | "montar um render minimo glintfx (`__DEP_REMOVIDA__`+RCSS) da carta" |
| `docs/design/mecanicas/cartas-spec-dados.md` | 36 | 1 | "zero `__DEP_REMOVIDA__`/GL/glintfx/IO, mesma regra de `combat_records.hpp`" |
| `docs/design/mecanicas/modos-morte.md` | 166,255 | 3 | "100% testável sem `__DEP_REMOVIDA__`/disco", "`__DEP_REMOVIDA__`/RCSS dedicado", "Tela de seleção de dificuldade (...+ `__DEP_REMOVIDA__` + loop)" |
| `docs/design/mecanicas/terminal-estetica.md` | 43 | 1 | "faria o `__DEP_REMOVIDA__` auto-usar no faltante mas o motor próprio não" |
| `docs/design/propostas/copy-stubs-combate.md` | 50 | 1 | "o caminho do glintfx desenha por `__DEP_REMOVIDA__` com a métrica real da fonte" |
| `docs/design/ui-kit/PORTING-RCSS.md` | 23,30 | 2 | "elemento real no `__DEP_REMOVIDA__`"; "elemento `<img>` `__DEP_REMOVIDA__`" |
| `docs/narrative/diary/ui-spec.md` | 452 | 1 | "Xbox/PS4/PS5 nativo via `__DEP_REMOVIDA__`" |

**Recomendação (proposta, não aplicada):** estas são citações técnicas em registros de decisão (ADR), não texto de marketing — esconder o nome real prejudica quem precisa entender o histórico de arquitetura no futuro, inclusive para compliance de licença (ADR-005). Pela leitura da L-24, a **afirmação técnica** por trás de cada uma continua válida (o diagnóstico do ADR-018, por exemplo, descreve um bug real já corrigido; a arquitetura de 4 camadas do ADR-009/ADR-014 continua valendo). É a **citação do nome** que ficou órfã. Recomendo restaurar o nome real da dependência nestes documentos — são registros históricos por natureza (ADR = decisão tomada num momento, não afirmação de presente), e cabe ao líder decidir só se algum ADR específico também precisa de nota de "superado" adicional (fora do escopo do marcador em si).

---

## 5. CAT-C — Crédito de produção / pessoa real, em prosa corrida

**O que a frase afirma, em geral:** quem orientou o projeto tecnicamente, ou por que uma escolha de formato de áudio foi feita — não é ADR, é texto de divulgação (`AI-DISCLOSURE.md`) ou nota de produção de asset (`AUDIO_KIT_PROVISORIO.md`).

| Caminho:linha | Contexto | Observação |
|---|---|---|
| `AI-DISCLOSURE.md:14` (EN) | "...he gave me plenty of architecture and stack advice, and guided me on using **`__DEP_REMOVIDA__`** and spritesheets for movement." — vers. mais curta na l.14: "...whose training shaped how I think about tech...guided me on architecture, **`__DEP_REMOVIDA__`** and spritesheets" | Crédito ao irmão do líder ("El Iagows") |
| `AI-DISCLOSURE.md:33` (EN) | igual, versão de agradecimentos | idem |
| `AI-DISCLOSURE.md:73` (PT) | "...que me orientou em arquitetura, **`__DEP_REMOVIDA__`** e spritesheets" | tradução da l.14 |
| `AI-DISCLOSURE.md:92` (PT) | "...me orientou no uso de **`__DEP_REMOVIDA__`** e de spritesheets para o movimento." | tradução da l.33 |
| `assets/AUDIO_KIT_PROVISORIO.md:74` | "...atende à recomendação de evitar Ogg/Vorbis no **`__DEP_REMOVIDA__`** sem decoder extra." | nota técnica de decisão de formato |
| `assets/AUDIO_KIT_PROVISORIO.md:75` | "Música já veio em MP3 (decodificação nativa do **`__DEP_REMOVIDA__`**), só precisou de redução de bitrate..." | idem |

**Recomendação (proposta, não aplicada):**
- **`AI-DISCLOSURE.md`** credita uma **pessoa real** (L-16 se aplica por analogia de cuidado, mesmo não sendo o filho do líder) — restaurar o nome técnico aqui tem duas leituras possíveis e a decisão é do líder: (a) restaurar o nome real da dependência, mantendo o crédito preciso; (b) generalizar para algo como "arquitetura, escolha de motor/renderer e spritesheets", já que a dependência específica não faz mais parte do projeto e o crédito à PESSOA (a orientação dada) continua válido independente do nome da ferramenta.
- **`AUDIO_KIT_PROVISORIO.md`** é puramente técnico, sem pessoa envolvida na frase — recomendo restaurar o nome real diretamente, é a leitura mais simples desta categoria inteira.

---

## 6. CAT-D — Dano colateral: corrupção de palavra sem relação com a dependência

**O que a frase afirma:** nada sobre a dependência técnica — o marcador aterrissou no meio de uma palavra qualquer, evidência de que a ferramenta de sanitização rodou um `sed`/regex amplo demais sobre o corpus, sem revisão de falso-positivo (achado já registrado por `docs/_processo/mapa-corpus-gusworld.md:152`, que encontrou 3 casos desse tipo — 5 deles já foram corrigidos em 25/08 por decisão do líder, incluindo `core::fmt` e 3 links `pcgamer.com`, conforme `TODO.md:125`).

| Caminho:linha | Contexto | Confiança da restauração |
|---|---|---|
| `resources/prompts_images/feitos/CASSIEL_FERRAZ_IMAGEPROMPT.md:13` | "...his stubby hands rest **fi`__DEP_REMOVIDA__`y** on the head of a heavy iron SMITH'S HAMMER planted upright on the ground in front of him, like a king on a sword." | **Alta.** "fi" + "y" com o miolo faltando bate com a palavra inglesa "**firmly**" ("rest firmly on the head of..."), mesmo padrão do achado anterior (miolo de palavra comum apagado, início e fim sobrevivem) |
| `docs/book/BIBLE-V1-APENDICE-G.md:152` (**cifrado**) | Conteúdo não citado aqui (L-25). O relatório não-cifrado `docs/_processo/mapa-corpus-gusworld.md:152` já registra e cita esta MESMA ocorrência anteriormente (achado prévio, já público naquele arquivo) — aponto para lá em vez de recitar o conteúdo de novo. | A correção real, se autorizada, precisa ser feita dentro do arquivo cifrado e exige a chave do git-crypt |

**Recomendação (proposta, não aplicada):** restauração direta nos dois casos — não há julgamento de "qual nome de dependência" envolvido, é reconstrução de palavra comum corrompida, mesma classe do lote já corrigido em 25/08. `CASSIEL_FERRAZ_IMAGEPROMPT.md` pode ser corrigido sem tocar em nenhum outro arquivo. `BIBLE-V1-APENDICE-G.md` precisa da chave do git-crypt para a edição (fase `B1b`, fora do escopo desta compilação).

---

## 7. CAT-E — Citação do marcador como tópico (recomendo NÃO editar)

**O que a frase afirma:** estes documentos **discutem o marcador em si** como achado de auditoria ou item de tabela — não são texto corrompido, são relatórios/leis/pendências que citam `__DEP_REMOVIDA__` deliberadamente, entre crases, como evidência do que precisa ser investigado. Editar aqui apagaria o rastro da própria auditoria que descobriu o problema (o oposto do que a L-24 pede).

| Arquivo | Linhas | Marcadores | Natureza |
|---|---|---|---|
| `GODS_LAWS.md` | 562 | 1 | A própria lei que registra o item de reparo ("Reparo das 28 ocorrências de `__DEP_REMOVIDA__`: agente propõe caso a caso...") |
| `TODO.md` | 125, 130 | 2 | Os itens `B1a`/`G1` da tabela de pendências, citando o marcador como assunto do item — **fora de escopo de edição nesta tarefa por instrução explícita, além de ser CAT-E por natureza** |
| `docs/_processo/candidatos-revogacao.md` | 190,192,196,197,198,199,200,201,204,205,206,207,208,209,211,213,329,536,538 | 26 | Relatório de auditoria anterior (achado A.9 e E.5), citando a frase-alvo e a string sozinha como evidência tabulada |
| `docs/_processo/lente-esforco.md` | 35, 153 | 2 | Estimativa de esforço do item B1, citando "28 ocorrências" como referência de escopo |
| `docs/_processo/lente-fluxo.md` | 40 | 1 | Mapa de dependência de fluxo entre trilhas (B1 vs. G1) |
| `docs/_processo/lente-produto.md` | 37 | 1 | Priorização WSJF do item B1 |
| `docs/_processo/mapa-corpus-gusworld.md` | 148, 152 | 11 | Inventário do corpus — a linha 152 é a que **descobriu** o padrão de dano colateral (cita 3 exemplos, incluindo o caso cifrado do §6) |
| `docs/_processo/mineracao-roadmap-todo-antigos.md` | 50, 53 | 2 | Mineração de roadmap antigo, nomeando o marcador como "nomeada `__DEP_REMOVIDA__` no backup por redação prévia" |
| `docs/_processo/parecer-licenca-catalogo.md` | 14 | 1 | Parecer de licença, nota lateral: "`AI-DISCLOSURE.md` (...) contém o placeholder `__DEP_REMOVIDA__`" |

**Recomendação (proposta, não aplicada):** **não editar.** Se, depois de `B1b`, as contagens citadas nesses relatórios (ex.: "28 ocorrências", "17 ocorrências") ficarem desatualizadas, isso é uma atualização de NÚMERO nesses relatórios — tarefa distinta, fora do escopo do marcador em si, e também fora do escopo de `B1a`/`B1b`.

---

## 8. Não classificado

| Caminho:linha | Motivo |
|---|---|
| `docs/book/BIBLE-V1-PREFACIO.md:26` (**cifrado**) | Conteúdo não inspecionado por instrução da tarefa (L-25) — sem nenhum relatório não-cifrado pré-existente que descreva esta ocorrência especificamente (diferente do caso do §6, que já tinha cross-referência pública), não há base para propor categoria sem abrir o arquivo. |

**Não classificado: 1 de 116 linhas (0,9%), 1 de 173 marcadores.**

---

## 9. Resumo para a decisão do líder em `G1`

- **173 ocorrências, 116 linhas, 45 arquivos** — medido, não estimado.
- **5 categorias + 1 não classificado**, cada uma com recomendação PRÓPRIA (não é "aprovar tudo de uma vez"): CAT-A (stack vigente, decidir presente-vs-histórico por linha), CAT-B (identificador técnico em ADR, restaurar nome real recomendado, com prioridade alta na linha de compliance do ADR-005), CAT-C (crédito de pessoa real + nota técnica, dois tratamentos distintos), CAT-D (dano colateral, restauração direta e de alta confiança), CAT-E (não editar — são o próprio rastro de auditoria).
- **2 achados laterais fora da contagem principal:** nome de arquivo corrompido em `ADR-008-repivot-qt-to-_.md` e em `difficulty_menu_____.hpp/.cpp` (citado dentro de `modos-morte.md:166`) — não capturados pela busca de string porque o nome de arquivo colapsou para `_`, não para o marcador completo.
- **1 achado de compliance:** `ADR-005:117` — sem o nome real, `THIRD-PARTY-LICENSES.md` não consegue atribuir corretamente a licença dessa dependência.
- **1 achado de status:** `ADR-008` segue "Accepted" (não "Superseded"), inconsistente com o resto do corpus que já trata essa stack como passada.

Aguardando aprovação em bloco (`G1`) para destravar a fase de edição (`B1b`).
