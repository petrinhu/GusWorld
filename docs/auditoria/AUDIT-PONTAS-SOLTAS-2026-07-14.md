# AUDITORIA DE PONTAS SOLTAS — 2026-07-14

> **Escopo:** caçar assuntos tratados como RESOLVIDOS que têm ponta solta, contradição cross-doc, ou inconsistência de dependência/tempo (classe-exemplo do criador: "implante do mapa só no endgame, mas o mapa serve o jogo inteiro"). Principalmente arquitetura/design/lore; alguma técnica.
> **Método:** orquestrador (thread principal, método CTO) + 4 subagents sonnet read-only, uma lente cada: (A) mundo/mini-mapa/loot `lead-game-designer`; (B) cartas/combate/economia `economy-designer`; (C) lore/narrativa/continuidade `narrative-designer`; (D) arquitetura/código/design↔código `software-architect`. **Ninguém modificou nada.**
> **Classificação:** 🔴 VERMELHO (resolver já) · 🟡 AMARELO (resolver logo) · 🟢 VERDE (não interfere, sem risco de propagar).
> **Item na tabela:** `AUD-PONTAS-SOLTAS-2026-07-14` (INBOX do TODO.md).

## Resumo

| Onda | Qtd | Natureza |
|---|---|---|
| 🔴 Vermelho | 8 | Contradições ativas entre docs canônicos + dependências que travam gameplay/implementação |
| 🟡 Amarelo | 13 | Docs desatualizados pós-decisão, primitivas de engine não rastreadas, atrito de UX/timing |
| 🟢 Verde | 7 | Cosmético / doc incompleto sem risco de propagação |
| (anteriores) | 6 | Resíduos de auditorias passadas ainda abertos (já rastreados no board) |

## Passada de sync (2026-07-14, commit `1d0acb4`)

**14 achados mecânicos RESOLVIDOS** (doc-fix puro, sem decisão): PS-R6, PS-R2, PS-Y3, PS-Y4, PS-Y5, PS-Y6, PS-Y7 (virou item `MINIMAPA-SAVE-SCHEMA`), PS-Y8, PS-Y12, PS-G1, PS-G3, PS-G4, PS-G5, PS-G7.

**RESTAM (precisam de DECISÃO do criador):** 🔴 PS-R1, PS-R3, PS-R4, PS-R5, PS-R7, PS-R8 · 🟡 PS-Y1, PS-Y2, PS-Y9, PS-Y10, PS-Y11, PS-Y13 · 🟢 PS-G2 (commitar app/tools?), PS-G6 (Brunus botica, junto da canonização da abertura). Levados ao criador via AskUserQuestion.

---

## 🔴 ONDA 1 — VERMELHO (resolver já)

**PS-R1 · CardFamily não comporta ~13 das 20 cartas especiais** — cartas · `combat.md` §17 (`enum CardFamily {Eletrico,Bioquimico,Sonico,Cinetico,Criptografico}`, 5 valores) × `_EFEITOS-ESCOLHIDOS.md` / `cartas-statlines-rascunho.md` ("Utilitário"). `Card.Family` é campo obrigatório; só ELM mapeia limpo. MAT/CMP/OCU/ECO (13/20) não têm família válida; "Utilitário" NÃO é membro do enum. **Correção:** decisão do criador — (a) adicionar `Utilitario`/`Universal` (multFraqueza=1.0, fora da roda), ou (b) tornar `Family` nullable pras fora-da-roda + regra de multFraqueza. Sem isso, 13 cartas não instanciam.

**PS-R2 · Contradição de slot: fora-de-combate ocupam o deck ou não?** — cartas · `cartas-technomagik.md` §2.3 (Faraday/Euler/Menger "NÃO ocupam slot de deck") vs §7.2 (mesma revisão: "Carta ESPECIAL ocupa 1 dos 15 slots, já fechado em §2.3"). **Correção:** ressalva no §7.2 excetuando as fora-de-combate.

**PS-R3 · "Só 3 itens usáveis" contradiz o craft canônico** — economia/sistemas · `mini-mapa.md` §0.1 ("loot nunca é usável exceto os 3 do mini-mapa; tudo só-pra-vender") vs `economia.md` (CANÔNICO 2026-05-30) §2 ("Baú/terminal") + §7.8 (18 ingredientes drop→craft de 5 poções/5 implantes/3 cartuchos, todos USÁVEIS). Nenhum doc cita o outro. **Correção:** definir 2 pools (loot-de-repositório sell-only vs drops-de-craft) e nomear nos dois docs; revisar a frase "só 3 usáveis" e o "Baú" da economia.

**PS-R4 · Zona do Silêncio: 3 conteúdos disputam 2 slots de dungeon** — mundo/quest · `mundo-topologia.md` §4 (2 dungeons: Linda #1 + mista #2) vs `mini-mapa.md` §0.2 (M2 marcadores) vs `21-helion-tusk.md` (dungeon do arco Tusk / Câmara Última Frequência). 3 payloads, 2 espaços, sem alocação. **Correção:** decisão — fundir 2 dos 3, abrir 3º slot, ou M2 vira "lugar secreto" (a área já tem 3 orçados).

**PS-R5 · Montadora Confluência: 13→14 dungeons + categoria ambígua (3 docs divergem AGORA)** — mundo/topologia · `mundo-topologia.md` §4 (Montadora 0 dungeons, área própria Tier 4, total 13) vs `mini-mapa.md` §0 (repositório da M3) vs `PLACES.md`:119 (Sub-local DENTRO da Selve Profunda, Tier 5). É nó próprio do mapa-múndi ou cômodo da Selve Profunda? **Correção:** fixar categoria (nó Tier-4 independente → ajustar PLACES; ou sub-área Selve Profunda → remover linha própria e recontar 13→12) + resolver o total de dungeons. **Resolver junto de PS-R4.**

**PS-R6 · CONTRACT.md §7 descreve o save format desatualizado** — tech/contrato · `CONTRACT.md`:255-286 (diz "schema v4, cripto HMAC-SHA256, ADR-006/007") vs código real (`kSaveSchemaVersion=5`, envelope `GDS3` AEAD XChaCha20/Monocypher, ADR-015). O doc processual #2 na hierarquia ficou pra trás. **Por quê urgente:** a próxima feat de código prevista (`mini-mapa.md` §7.5, estender o save) enganaria quem seguir o CONTRACT (planejaria V4→V5/HMAC em vez de V5→V6/AEAD). **Correção:** atualizar §7 pra GDS3/AEAD/`v5` citando ADR-015.

**PS-R7 · Arco de falibilidade do Gus depende do Bento "no comando" de missão OBRIGATÓRIA, mas Bento é ordem-livre** — lore/design (timing) · `vozes-party.md` (beat 5: reversão "sob comando do tanker Bento") vs `arco-principal.md` (os 5 companions restantes em ordem livre) + gdd §7.1 (sem gate-hard). Se a reversão dispara em % fixo e o jogador não tem Bento → trava, ou fallback quebra a lição, ou força Bento cedo (one-way novo). **Correção (recomendada):** condicionar o beat a Bento já recrutado (dispara na 1ª oportunidade após ele juntar), decidir no brainstorm que `vozes-party.md` já marca pendente, antes de tocar `arco-principal.md`.

**PS-R8 · Helion Tusk marcado ✅ canônico em CHARS.md, mas o pacote 21 "aguarda aprovação" + cascata incompleta** — personagens/canon · `CHARS.md`:279 (Tusk `✅ canônico`, único dos 21) vs `21-helion-tusk.md`:2 ("PROPOSTA, aguarda aprovação") + `TODO.md` (roster ainda "em andamento") + `_RECONCILIACOES.md` item 7 não cumprido (sem entrada de facção do consórcio, sem "Forja do Vértice"/"Montadora" em PLACES, **Vitório Cardoso sem linha própria em CHARS §3**). Status de canonização se autocontradiz. **Correção:** reverter Tusk pra `🟡 design` OU confirmar aprovação explícita + propagar a cascata (factions/PLACES/Vitório Cardoso).

---

## 🟡 ONDA 2 — AMARELO (resolver logo)

**PS-Y1 · 4 cartas híbridas não cabem nas "3 sub-categorias fechadas"** — cartas · Maxwell/Newton/von Neumann/John Dee têm passiva+ativa no mesmo card; §2.3 assume 1 carta = 1 categoria. **Correção:** 4ª linha "HÍBRIDA" (passiva sempre-ligada + ativa mana~6 1×/batalha, 1 slot; passivo não sofre o 1×/batalha).

**PS-Y2 · Efeitos exigem primitivas de engine inexistentes, não listadas no §9** — cartas/combate · Newton "reflete dano" (não há status Reflect em `StatusId`); Pythagoras "√(a²+b²) quando 2 aliados batem no mesmo alvo" (combo CROSS-ATOR; só existe combo 1-ator §10); von Neumann/Bruno clone como 4º ator (vs "Party=3 fixo" §2). Não estão no §9 dados-faltantes. **Correção:** adicionar ao §9; decidir clone = ator-na-fila (fura Party=3) vs entidade-Object (mais barato, reusa modificador).

**PS-Y3 · `cartas-statlines-rascunho.md` desatualizado vs 20/20 efeitos, sem marcador** — cartas · linhas materialmente erradas (Pythagoras/Maxwell/Euler do rascunho ≠ efeito escolhido). **Correção:** cabeçalho "SUPERADO linha-a-linha por `_EFEITOS-ESCOLHIDOS.md`; usar só como formato de tabela".

**PS-Y4 · Volta no §9 (exemplo "mana→Shield") ≠ efeito escolhido (leech termodinâmico)** — cartas · `cartas-technomagik.md` §9 vs `_EFEITOS-ESCOLHIDOS.md`. **Correção:** atualizar o exemplo do Volta no §9.

**PS-Y5 · ADR-003 (DialogueManager/Godot) sem marcador Superseded** — tech/ADR · morto no pivot; ADR-014 não declara "Supersedes ADR-003" e ADR-003 segue "Accepted". **Correção:** `Status: Superseded by ADR-014` + "Supersedes: ADR-003".

**PS-Y6 · Índice de ADRs do ROADMAP incompleto** — tech/docs · falta ADR-009/011/012/013/014 e **015 (save security v2)**. **Correção:** completar a tabela (reforça PS-R6).

**PS-Y7 · mini-mapa §7.5 (novo schema de save) sem item no TODO** — design↔código · conjunto grande de campos novos (fog/implante/VRAM/skill/SSD/config); `CONTRACT.md §3` marca "toque em save" = feature grande com cooling-off; invisível no board. **Correção:** criar `MINIMAPA-SAVE-SCHEMA` (backend-engineer, bump v5→v6+migrator+teste tamper/roundtrip).

**PS-Y8 · `engine-design.md` em zona cinzenta pós-pivot** — tech/docs · CONTRACT cita como autoridade técnica #4, mas o corpo descreve Qt RHI/QML/Qt Test como stack (só 1 nota no topo avisa). **Correção:** reescrever/aposentar (como architecture/engine-modules/build já foram).

**PS-Y9 · Fog/OOM só resolve na Montadora (Tier 4, quase-fim) → Médio+ passa o jogo com névoa** — design/timing (a CLASSE DO EXEMPLO DO CRIADOR) · a missão da VRAM está na área mais distante; no Médio+ o jogador navega o mundo aberto quase todo com ruído corrompido, justo quando mais precisa planejar rota. **Correção:** (a) mover a VRAM pra distância média, ou (b) reduzir o RAIO de fog progressivamente por marco (não binário). Decisão do criador.

**PS-Y10 · M1 (puzzle de reparo) numa dungeon tipada "só batalhas"** — mundo/mecânica · a única dungeon do Ferrovelhos (§4) é "reduto do Dante, só batalhas"; M1 é "1 puzzle + diálogo". **Correção:** M1 vira "lugar secreto" do Ferrovelhos; ajustar a frase de `mini-mapa.md` §0 ("3 repositórios: dungeon OU lugar secreto").

**PS-Y11 · Zero mapa (nem TAB) até achar M1 → janela sem navegação** — onboarding/UX · o mundo é aberto desde o Beat 5 mas nenhum mapa (nem da área atual) existe antes de M1 (Ferrovelhos, Tier 2). **Correção:** liberar o mapa-TAB da ÁREA ATUAL desde o início (só tilemap, sem marcador); a conquista diegética fica pro mapa-múndi + marcadores + fog-fix. Decisão do criador.

**PS-Y12 · `characters/party.md` ainda diz "ordem livre" sem a exceção do Cauã** — narrativa/continuidade · resíduo da reconciliação da abertura que escapou. **Correção:** 1 frase (Cauã fixo #1, resto livre).

**PS-Y13 · 3 camadas de nome não-reconciliadas para as 20 cartas** — mecânica/narrativa · nome-título PT ("Dilatação Temporal", na prosa canônica dos mestres) × alcunha EN ("Time-Dilate", `_EFEITOS-ESCOLHIDOS.md`) × ID `cardExec-[figura]`. Sem regra de convivência (ao contrário de Tavus-Eco/Morlhin). Risco: 20 diálogos divergentes. **Correção:** adendo (10º item em `_RECONCILIACOES.md` ou nota no `_EFEITOS`) definindo o papel de cada camada.

---

## 🟢 ONDA 3 — VERDE (não interfere; sem risco de propagar)

- **PS-G1 · TODO linha 440** ainda lista `ENGINE-MAPA-ONDA (loader .gmap)` como pendência de MUNDO-TOPOLOGIA, já autocorrigido 5 linhas antes (engine de mapa pronta desde M4). → editar a célula.
- **PS-G2 · `GusEngine/app/tools/`** (8 probes citados como "prova" em itens ✅) nunca commitado (~6 dias). → commitar como dev-tooling ou mover pra scratch e não citar como prova.
- **PS-G3 · Lista de exemplos "PASSIVA" em §2.3** desatualizada (falta Planck/Turing/Volta). → refrescar na próxima passada.
- **PS-G4 · Ferrovelhos sem linha própria em `PLACES.md`** (carrega dungeon+2 secretos+Dante+4 mestres+M1). → adicionar linha.
- **PS-G5 · Mercado da Sucata Honesta:** PLACES diz charneira Núcleo↔Periferia; `mundo-topologia.md` §6 agrupa no "campus Ferrovelhos". → 1 frase clarificando.
- **PS-G6 · Botica-laboratório do Brunus** sem entrada em `PLACES.md` (já auto-flagged em `gus-abertura.md` §11). → adicionar quando canonizar a abertura.
- **PS-G7 · `_EFEITOS-ESCOLHIDOS.md` linha 56** cita "Einstein = Grav-Wave" como exemplo de estilo, mas o efeito escolhido é "Time-Dilate". → atualizar (resíduo de brainstorm).

---

## Resíduos de AUDITORIAS ANTERIORES ainda abertos (já rastreados no board)

- **AUD-MINIAUDIO-ALSA-LEAK** (PI10) — leak pré-existente em `ma_context_open_pcm__alsa`; bloqueia AC-E2 (incluir `platform/` no job ASan). Aberto, precisa item próprio.
- **AUD-MINIAUDIO-UAF** — patch entregue (`317a2e9`), 🔍 aguarda verificação final.
- **Build Windows nunca validado** — gate antes do M8 (dívida crescente; CUT.11 = Linux-only no ship v1).
- **F5-BK.AUDIT** — verificação textual profunda do canon (pré-tradução), pendente e grande (~88 sessões; "fadiga era-2", tique degradado 156×).
- **TODO-PARSER-BUG** — infra do harness (`~/.claude/githooks/`), fora do produto.
- **en_intl 7/142** — tradução (CUT.9, pós-1.0). Itens `F2-*` legados (era Godot, dormentes até M8).

---

## Nota de padrão (do orquestrador)

A MAIORIA dos vermelhos/amarelos é da MESMA raiz: **docs escritos em rodadas anteriores ao fechamento de uma decisão recente não foram re-sincronizados** (CONTRACT vs save real; §7.2 vs §2.3; rascunho de statlines vs efeitos; party.md vs reconciliação; ROADMAP vs ADRs; CardFamily vs roster de 20). Não são erros de design — são **débito de sincronização de docs** acumulado numa sessão de brainstorm muito rápida. Recomendação de processo: uma passada curta de "sync de docs" fecha ~metade destes de uma vez.
