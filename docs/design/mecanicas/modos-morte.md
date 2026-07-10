# Modos de Morte (MODOS-MORTE) — sistema de fail-state escalonado por dificuldade

**Status:** PROPOSTA — spec fechada pelo `lead-game-designer`, aguardando canonização do criador supremo (líder). Nada aqui está implementado; é o documento que orienta a implementação incremental (ver §7).

**Escopo:** expande o Pillar 4 (`pillars.md`, seção "Game over") de 2 níveis (Normal/Hard) pra **4 níveis** (Fácil/Médio/Difícil/Hardcore), cada um com consequência mecânica própria pra morte de Gus. NÃO é escopo do M7-COSTURA (canon já registrado — o M7 mantém o placeholder simples "volta pra cidade"; ver §6).

**Cross-refs:** `docs/design/pillars.md` Pillar 4; `docs/design/mecanicas/economia.md` §3 (Hospital/derrota — base do Médio); memória `project_morte_dificuldade_canon` (decisão original do líder, 2026-07-03); memória `project_dragon_victory_canon` (o clímax que nenhum modo pode furar); item `MODOS-MORTE` no `TODO.md` (hoje só referenciado no roadmap PI10 + no item COMBATE-TEORIA-JOGOS #6, sem linha de tabela própria ainda — sugiro abrir a linha ao canonizar esta spec).

---

## §1. Os 4 modos (recap do canon fechado)

| Modo | Trigger de morte | Consequência mecânica | Depende de |
|---|---|---|---|
| **Fácil** | Gus HP=0 | Volta o **último save** (game over clássico JRPG) | Save I/O em disco (✅ já existe, M2) |
| **Médio** *(default, §2.1)* | Gus HP=0 | **Whiteout estilo Pokémon**: acorda no Hospital com perdas (safe mode 13% HP-máx grátis OU dívida plafonada ~72cr — `economia.md` §3.3) | Motor de Hospital/dívida implementado (canon no doc, código ainda não construído) |
| **Difícil** | Gus HP=0 | **Respawn deslocado + stats quase-zero**, local depende de quem deu o golpe final: criatura → Selve Sombria; humano → casa destruída. Recuperação por marcos (§2.4) | Sistema de dificuldade (§2.2) + detecção criatura/humano (§3.1) + locais reais (level design) |
| **Hardcore/Hell** | Gus HP=0 (só destravado pós-vitória Difícil) | **Kernel-panic puzzle como última chance** (RESOLVIDO 2026-07-10, §2.3a): resolve = sobrevive; falha = **permadeath**. Save isolado, anti-rollback offline (âncora out-of-band + machine-binding) + wipe por trechos (§2.3) | Vitória final no Difícil (conteúdo de fim-de-jogo, bem à frente no roadmap) |

---

## §2. Decisões do líder (2026-07-10) — congeladas, incorporadas

### 2.1 Dificuldade default = Médio

Jogo novo nasce em Médio. Fácil e Difícil são escolha explícita; Hardcore não aparece nessa lista (é unlock separado, §2.3).

### 2.2 Dificuldade fixa por save (não trocável) + UX de aviso

A dificuldade é escolhida **uma vez**, na criação do save, e **nunca muda** depois. Onde/quando: ver §3.2 (encaixe na tela de título / SAVE-LOAD-UI).

**Aviso #1 — texto inline, sempre visível enquanto o jogador navega as opções** (legenda fixa abaixo da lista de dificuldades, mesmo padrão visual de legenda da tela de título):

> *"Essa escolha vale pra sempre nesse jogo salvo. Depois de começar, não dá pra trocar de dificuldade aqui — só criando um save novo."*

**Aviso #2 — splash de confirmar/cancelar**, disparado ao selecionar uma dificuldade (ANTES de criar o save de fato). Reusa o componente `.confirm-pill`/`.confirm-title` já estabelecido (mesma família visual do "comecar novo jogo?" do `title_menu.hpp` e do "sobrescrever save?" do `save_load_menu.hpp`) — variante **informativa**, não `danger` (não é uma ação destrutiva, é uma decisão permanente mas positiva):

> **Título:** "Confirmar dificuldade: [Médio]"
> **Corpo:** "Você não vai poder trocar de dificuldade nesse jogo salvo depois de começar. Se quiser jogar de outro jeito mais tarde, é só começar um save novo."
> **Botões:** `[Confirmar]` `[Cancelar]`

`Cancelar` volta pra tela de seleção (não pra tela de título). `Confirmar` grava a dificuldade escolhida no `SaveData` novo (§3.2) e prossegue pro jogo.

**Descrições curtas por opção** (texto de apoio na própria lista, uma linha por modo — proposta de copy, `narrative-designer`/líder pode polir depois):

- **Fácil** — "Se você perder, volta pro seu último save."
- **Médio** *(recomendado)* — "Se você perder, acorda no Hospital. Custa um pouco de crédito."
- **Difícil** — "Se você perder, acorda longe e bem fraco. Precisa se recuperar aos poucos."

### 2.3 Hardcore isolado + puzzle de última chance + anti-rollback offline + wipe por trechos

**Atualização do líder (2026-07-10, após pesquisa de referência):** o objetivo passou a ser o **máximo de anti-rollback que dê pra fazer OFFLINE**. Servidor/always-online foi **rejeitado deliberadamente** — furaria a soberania de dado do projeto (regra cross-project de execução local, memória `feedback_execucao_local_cross_platform`: o mecânico pode ser 100% offline, só a inferência de LLM é dependência externa irredutível — não é o caso aqui). A ameaça real que esta mecânica precisa derrotar é o **cheater CASUAL** (copiar a pasta de save antes de arriscar a run, restaurar se morrer) — não um adversário forense. 3 camadas complementares, defesa-em-profundidade; **nenhuma delas, nem as 3 juntas, é forense-proof offline** (ver caveat no fim).

**Isolamento:** Hardcore vive num **slot dedicado próprio**, fora da lista de 6 manuais + Auto do SAVE-LOAD-UI — os saves normais nunca são tocados por essa mecânica. Proponho **1 único slot Hardcore** (nome lógico `"hardcore"`, mesmo padrão de `path_for` do `FsSaveStore`: `hardcore.sav` + cadeia `hardcore.backup1..backup3`), não uma lista — coerente com a moldura narrativa ("a run canônica já terminou", é um epílogo de uma vida só por vez). Se o líder quiser múltiplas runs Hardcore simultâneas, é trivial virar `hardcore_1..hardcore_N`; deixo como proposta mínima.

#### 2.3a Kernel-panic puzzle como última chance — RESOLVIDO 2026-07-10

Contradição do §4 (pillars.md original vs canon 2026-07-03) **resolvida a favor da opção (a)** que eu havia recomendado: o Hardcore **NÃO** é permadeath incondicional no instante em que HP=0. Ao chegar HP=0 em Hardcore, dispara-se o **kernel-panic puzzle** (o mini-jogo de recompilação já previsto no `pillars.md` original — sequência puzzle pra "reboot"):

- **Resolve o puzzle → sobrevive.** Proposta de detalhe (não fechada, ver §5): Gus revive no lugar, sem wipe, com HP/stats no mesmo piso "quase-zero" do respawn do Difícil (§2.4, 5% — reusa o número, não duplica conceito).
- **Falha o puzzle → permadeath de verdade**, dispara as 3 camadas abaixo (anti-rollback + wipe por trechos).
- **A chance é ÚNICA por run (decisão do líder 2026-07-10):** o puzzle só aparece na PRIMEIRA vez que HP=0 no Hardcore. Sobreviveu ao puzzle uma vez → se morrer DE NOVO depois, NÃO há puzzle: **permadeath direto**. Impl (Fase 4): flag `puzzle_used` na **ÂNCORA selada** (4º campo, junto do `run_dead` — decisão do líder 2026-07-10; ADR-015 §4), setada ao resolver — a partir daí HP=0 = fim. É o que a copy do aviso comunica ("última chance de retorno", chance única).

**Copy do aviso HARDCORE_WARN_* — APROVADA pelo líder 2026-07-10 (ux-writer; entra no catálogo i18n na Fase 4, quando o Hardcore for construído):** splash denso, 4 bullets (1 por risco), variante `.danger`, card MAIOR que o `.confirm-pill` padrão, **foco default = Cancelar**. Título "Jogar em Hardcore?" / intro "Antes de continuar, veja o que muda:". Bullets: (1) **permadeath-chance-única** = *"Ao chegar a 0 de HP, você tem UMA última chance de retorno: resolver um puzzle. Conseguir = sobrevive. Mas é única — falhar, ou morrer de novo depois, = fim definitivo, sem volta."* (ajuste do líder ao rascunho do ux-writer, incorporando a unicidade); (2) save apagado irrecuperável na morte; (3) unlock preso à máquina (reinstalar/trocar PC perde); (4) save isolado + dificuldade fixa. Botões "Sim, jogar Hardcore" / "Cancelar".

O puzzle **É** a "morte condicional" que o Pillar 4 original já previa — não é elemento novo, é o gancho antigo reencaixado no Hardcore em vez de ficar solto num "Hard mode" genérico que os 4 níveis substituem. Preserva a agência mecânica coerente com Pillar 1 (lógica vence força) em vez de a run acabar só por um dado de HP.

**Camada 1 — âncora monotônica out-of-band (a que de fato rejeita a cópia restaurada; as outras 2 são reforço):**

- Um contador/hash de sequência é guardado **FORA do arquivo `.sav`**, num local diferente da pasta de saves (ex.: keychain do SO, ou um arquivo escondido em outra pasta — a escolha exata do mecanismo por SO é detalhe de implementação, ver §7).
- O `.sav` embute a MESMA sequência + um nonce, selados junto com o payload (mesma família criptográfica do envelope GDS2/HMAC já selado, ADR-006 — cross-ref também `ASSETS-SELO-F2`, que reusa a mesma base de selo/HMAC pra outro fim).
- **Ao carregar:** a sequência dentro do save precisa **bater exatamente** com a âncora externa. Restaurar só o `.sav` (backup antigo, cópia manual da pasta) sem a âncora correspondente — ou com sequência mais antiga que a âncora viva — causa descasamento e o load é **REJEITADO** (mesma família de motivo que o `LoadOutcome` já expõe hoje pra `HmacInvalid`/`WrongSlot`, um caso novo).
- **No permadeath (falha do puzzle §2.3a):** a âncora é atualizada pra marcar aquela run como **MORTA** (sequência final). Qualquer cópia do `.sav` restaurada depois — mesmo intacta byte-a-byte — não bate mais com a âncora viva e é recusada.
- **A MESMA âncora guarda o unlock do Hardcore** (ver §2.3b — sem arquivo soft separado).

**Camada 2 — machine-binding:** o HMAC do save (extensão do que o ADR-006 já faz pro envelope selado) passa a incorporar uma chave derivada **da máquina**, não só do conteúdo. Efeito colateral desejado: mover o `.sav` pra outro PC também quebra o load — reforça a Camada 1 sem depender de rede.

**Camada 3 — wipe por trechos (otimização aprovada pelo líder, substitui o wipe-de-arquivo-inteiro da versão anterior desta spec):**

- Não é preciso zerar o arquivo inteiro. Basta sobrescrever **o selo/HMAC + o nonce anti-rollback + o cabeçalho** do envelope — isso já torna o arquivo **incarregável** (o `load_save`/`load_game` já rejeita qualquer payload sem selo válido, comportamento existente hoje). Instantâneo mesmo em mídia lenta (HDD antigo) — o objetivo é "incarregável", não "sem resquício de dado em disco" (saves são poucos KB; zerar tudo não protegeria mais que zerar o essencial).
- Cobre os 4 arquivos da cadeia (primário + `backup1..backup3` — senão a morte "final" seria revertível via `load_game_from_backup`, que já existe no motor).
- Zerar o corpo inteiro do arquivo segue permitido como reforço barato opcional, **N=3 passadas** (mesma profundidade canônica já usada pra cadeia de backup, `kBackupChainDepth = 3` — consistência de convenção): zero / random / zero. Não é o mecanismo principal (a Camada 3 essencial é o overwrite do selo/cabeçalho); **N=3 é proposta minha, a fixar na onda de balanceamento junto com os percentuais do §2.4** (não canon fechado).
- Depois do wipe: **unlink** dos 4 arquivos + **limpa o `SaveData` da RAM** da sessão ativa + devolve o jogador pra tela de título (não há "continuar" possível pra aquele run).

**Caveat honesto (documentar sempre que essa feature for citada/exibida):**

- **Nenhuma das 3 camadas, nem juntas, é forense-proof offline.** Contra quem **snapshota a máquina inteira** (clone de disco, snapshot de VM, backup do SO antes de morrer — âncora out-of-band incluída no snapshot), nada impede restaurar tudo junto de volta a um estado anterior. **Só um servidor autoritativo resolveria isso de verdade** — é por isso que o Diablo hardcore usa servidor — e essa rota foi **rejeitada deliberadamente** pela soberania de dado do projeto: trade-off consciente, não descuido.
- Em **SSD/CoW/journaling**, o bloco físico antigo do trecho sobrescrito pode não ser fisicamente destruído (mesma ressalva de sempre — vale menos aqui, porque o alvo agora é "invalidar o selo", não "destruir o dado").
- **O que isso DE FATO derrota:** o cheater casual — copiar a pasta de saves antes de arriscar a run, restaurar se morrer. Cobre a esmagadora maioria dos jogadores; não cobre quem ativamente clona a máquina/VM pra burlar.
- Nunca vender essa feature como "impossível recuperar" em texto pro jogador — a copy do "Game Over Hardcore" (fora do escopo desta spec, narrativo) deve refletir "esse jogo salvo acabou" sem prometer indestrutibilidade técnica.

#### 2.3b Unlock do Hardcore — SEM profile.json (rework 2026-07-10)

A proposta anterior desta spec (arquivo `profile.json` novo, JSON legível, fora do save) foi **rejeitada pelo líder**: um flag "zerou o Difícil" num arquivo soft/editável a mão destruiria a integridade do Hardcore — destravar viraria trivial (edita o JSON, pronto). **Rework:** o unlock ("zerou o Difícil → Hardcore disponível") mora **DENTRO da mesma âncora selada+machine-bound do save-crypto** (Camada 1 acima) — mesma proteção que já protege o próprio save, sem soft-spot novo. Alternativa considerada e descartada como proposta principal (mas registrada): o unlock "morre" junto com o save do Difícil-zerado (temático — "a prova é o próprio save vencido" — mas frágil: se o jogador apagar aquele save depois, perde o unlock sem querer). A âncora selada é a opção recomendada.

**Nível de detalhe desta spec:** aqui fica só o DESIGN (o quê + por quê + o trade-off). Os detalhes de CRYPTO/implementação (derivação da chave de máquina, formato exato da âncora out-of-band incluindo o bit de unlock, mecanismo por SO, quais bytes exatos do envelope contam como "selo/cabeçalho" pro wipe por trechos) ficam pro **`security-engineer`**, já em produção como **ADR-014** (save-crypto estado-da-arte offline: AEAD XChaCha20-Poly1305 via Monocypher vendorizado — ver memória `reference_libs_vendorizadas` — + cifra + machine-bind + âncora anti-rollback + wipe por trechos), na hora de **construir** o Hardcore — que só é buildável pós-Difícil jogável (Fase 4, §6), não agora.

### 2.4 Difícil: stats quase-zero + recuperação por marcos (não por tempo)

**Magnitude no respawn (PROPOSTA — a fixar na onda de balanceamento, `economy-designer`/playtest, sequência numérica recorrente — mesmo idioma de `economia.md`):** **5%** de HP-máx e de cada stat de combate (Atk/Def/Spd) de Gus, arredondado pra baixo, **piso 1**. É deliberadamente mais severo que o "safe mode" do Médio (13% — `economia.md` §3.3.1): a escolha de jogar em Difícil deve doer mais que um wipe acidental em Médio, senão os dois modos convergem.

**Recuperação por 3 marcos** (nunca por tempo passivo/passos andados — canon já fechado pelo líder), curva Fibonacci-flavored consistente com o resto da economia. **Os 4 percentuais abaixo são PROPOSTA, a validar/ajustar na onda de balanceamento** (não canon fechado):

| Marco | Gatilho | Stats sobem para |
|---|---|---|
| (respawn) | morte no Difícil | **5%** |
| 1 — Voltar à cidade | cruzar o hub GusWorld City | **34%** |
| 2 — Dormir | evento de descanso/pernoite (gatilho NOVO — ver §7) | **89%** |
| 3 — Cura completa | Hospital pago OU beat narrativo de cura plena (`economia.md` §3.2) | **100%** |

Cada marco é permanente até o próximo (não decai de volta). Se o jogador morrer de novo ANTES de atingir 100%, o respawn reresseta pra 5% (não "acumula punição" além disso — mesma lógica anti-bola-de-neve da dívida do Hospital, `economia.md` §3.3.3).

### 2.5 Difícil: framing narrativo do despertar (5 amarras — proteção do Dragon Victory)

O momento do respawn NÃO é uma cutscene — é um **log glitchado mínimo**, formato log de sistema, não prosa emotiva. Copy-exemplo:

> `SINAL PERDIDO... RECONECTANDO... LOCALIZAÇÃO: DESCONHECIDA`

**5 amarras** (constraints do `narrative-designer`, protegem a exclusividade do clímax Dragon Victory — memória `project_dragon_victory_canon`):

1. **Sem testemunha** — ninguém da party vê o momento do respawn.
2. **Sem diálogo de reconhecimento** — Gus não comenta/processa emocionalmente na hora (nada de "eu quase morri...").
3. **Framing técnico de fail-safe corrompido** — é um mecanismo de sistema falhando (coerente com Pillar 2, magia=sistema formal), **NÃO** onírico/milagroso.
4. **Tom confuso/inconveniente**, não dramático/trágico — o jogador sente "que droga, preciso me reerguer", não um momento de peso narrativo.
5. **ZERO elemento visual do Dragon** — sem aura, sem gradient cyan→vermelho, sem leitmotiv musical. Esses elementos são **exclusivos** do clímax (Dragon Victory) — reusá-los aqui esvaziaria o payoff.

**Ponto de continuidade registrado (checar quando os beats forem escritos):** a iconografia Selve Sombria / casa destruída usada neste respawn pode ser reaproveitada em outros beats futuros (ex.: cenas de exploração, side-content) — vale conferir consistência visual/tonal quando esses beats forem de fato roteirizados, pra não criar uma leitura "todo respawn feio = presságio do Dragon".

---

## §3. Detalhes resolvidos

### 3.1 Detecção criatura-vs-humano

**Onde vive o dado:** novo enum + campo no template de inimigo, `GusEngine/domain/include/gus/domain/templates/enemy_template.hpp` (POCO puro, mesmo arquivo de `BrainKind`/`is_boss`):

```cpp
// Natureza diegética do inimigo (Difícil §2.4): decide o LOCAL de respawn na
// morte de Gus. Ordinal EXPLICITO (contrato binário do serializer, mesmo padrão
// de BrainKind). Default = Creature (a maioria dos encontros do VS é Selve).
enum class EnemyKind : std::uint32_t {
    Creature = 0,
    Human = 1,
};

// dentro de EnemyTemplate:
EnemyKind kind = EnemyKind::Creature;
```

Campo aditivo, `validate()` não precisa mudar (todo ordinal do enum é válido por construção). Cada `EnemyTemplate` canônico em `canonical_templates.cpp` recebe `kind` explícito na hora de canonizar (fauna Selve = `Creature`; NPCs/inimigos humanos de facção = `Human`).

**Como o combate expõe "quem deu o golpe final":** NÃO precisa de campo novo em `combat_records.hpp`. `CombatResult::log` (vetor de `CombatLogEntry`) já registra `actor_id` + `target_id` + `value` (dano) por ação. Resolução: ao Gus chegar a HP=0 (`CombatOutcome::Defeat`), o código de app (fora do domain — o domain não sabe de Selve/casa destruída) percorre `log` **de trás pra frente** procurando a última entrada com `target_id == gus_id` e `value > 0` (dano real); pega o `actor_id` dela, resolve o `EnemyTemplate` correspondente via `TemplateSource`, lê `.kind`.

**Fallback defensivo:** se por algum motivo nenhuma entrada bater (log vazio/corrompido — não deveria acontecer em fluxo normal), assume `EnemyKind::Creature` (mesmo valor do default do enum — consistente, e narrativamente o caso mais genérico/seguro).

### 3.2 Seleção de dificuldade — onde/quando/armazenamento

**Onde:** tela nova, inserida no fluxo de **Novo Jogo** da tela de título (`title_menu.hpp`, já existe — SAVE-LOAD-UI etapa 4). Hoje: `Novo Jogo` → (se há save existente) mini-diálogo Sim/Não → `StartNewGame`. Proposta: entre a confirmação de "Novo Jogo" e o `StartNewGame` de fato, entra uma **tela de seleção de dificuldade** (3 opções: Fácil / Médio / Difícil — Hardcore não aparece aqui, §2.3), foco inicial em **Médio** (§2.1), navegação idêntica ao padrão já estabelecido (setas/WASD + Enter, clique = foco+confirma). Selecionar dispara o Aviso #2 (§2.2); `Confirmar` grava a escolha e só ENTÃO o jogo começa do zero.

Novo POCO análogo a `TitleMenuState`: `DifficultyMenuState` (arquivo sugerido `gus/app/screens/difficulty_menu.hpp`, mesmo espírito de `title_menu.hpp`/`save_load_menu.hpp` — 100% testável sem SDL/disco), RML/RCSS dedicado (`difficulty_menu_rml.hpp/.cpp`) e o loop GL (`difficulty_menu_loop.hpp/.cpp`).

**Armazenamento (`SaveData`, `gus/domain/save/save_data.hpp`):** campo novo, aditivo, schema **V5** (mesmo padrão dos bumps V2→V3→V4 já documentados no header do arquivo):

```cpp
enum class DifficultyLevel : std::uint32_t {
    Facil = 0,
    Medio = 1,
    Dificil = 2,
    Hardcore = 3,
};

// dentro de SaveData:
// Dificuldade FIXA deste save (§2.2) — setada 1x na criação, NUNCA reescrita
// depois. Migrator V4->V5: saves antigos (pré-dificuldade) sobem como Medio
// (default canônico §2.1 — não havia escolha explícita antes, Medio é o
// "meio-termo" mais coerente pra não punir nem trivializar saves legados).
DifficultyLevel difficulty = DifficultyLevel::Medio;

// Estágio de recuperação pós-morte no Difícil (§2.4). 0 = sem penalidade ativa
// (ou modo != Dificil); 1 = acabou de respawnar (5%); 2 = cruzou Marco 1 (34%);
// 3 = cruzou Marco 2 (89%); 4 = Marco 3 completo, 100%, volta a 0 (sem penalidade).
// Irrelevante fora do modo Difícil.
int difficult_recovery_stage = 0;
```

`difficulty` é **fixo** por contrato (nenhuma função de domínio deve expor um "setter" pós-criação — a única escrita legítima é no momento de `StartNewGame`, antes do primeiro `save_game`). `difficult_recovery_stage` é o único campo mutável desta spec, avança conforme os marcos do §2.4.

### 3.3 Interação com o resto

- **O motor de morte (dispatcher central)** lê `SaveData.difficulty` no momento em que `CombatOutcome::Defeat` chega com o alvo = Gus, e roteia:
  - `Facil` → `load_game` do slot ativo (reload puro).
  - `Medio` → fluxo Hospital (`economia.md` §3.3 — safe mode 13% / dívida).
  - `Dificil` → respawn deslocado (§3.1 pra local + §2.4 pra magnitude/recuperação; framing narrativo §2.5), grava `difficult_recovery_stage = 1`.
  - `Hardcore` → dispara o kernel-panic puzzle (§2.3a); resolve = sobrevive (revive no lugar, sem wipe); falha = wipe por trechos + anti-rollback (§2.3) + volta pra título.
- **O placeholder atual do M7** ("morte comum = volta pra cidade no ponto") **continua intocado durante o M7** — canon já registrado ("NÃO é escopo do M7-COSTURA"). Ele é substituído pelo dispatcher acima **quando MODOS-MORTE for implementado como feature própria**, POSTERIOR ao M7 (ver roadmap PI10, já lista `MODOS-MORTE` em paralelo ao M7, não como pré-requisito dele). Até lá, TODOS os saves (sem `difficulty` setado, ou com o placeholder ativo) se comportam como hoje.
- **Companions** não mudam: HP=0 de companion continua "incapacitado, não morto" em **todos os 4 níveis** (regra separada do Pillar 4, não afetada por esta spec — só a morte de GUS é escalonada).

---

## §4. Proposta de atualização do Pillar 4 (`pillars.md`) — NÃO aplicada, aguarda o líder

O `pillars.md` é canônico e imutável sem aprovação explícita (regra do projeto). Trecho atual (`docs/design/pillars.md`, seção "Game over"):

> - **Normal default**: game over puro (HP=0 → reload save).
> - **Hard mode (unlock pós-zerar)**: permadeath + **kernel panic puzzle** ao chegar HP=0 (sequência puzzle pra reboot; falhar = game over real).

**Proposta de substituição** (a aplicar no `pillars.md` só quando o líder canonizar):

> - **4 níveis de dificuldade, fixos por save, escolhidos na criação** (ver `docs/design/mecanicas/modos-morte.md`):
>   - **Fácil**: reload do último save.
>   - **Médio** *(default)*: whiteout — acorda no Hospital, custo de créditos (`economia.md` §3).
>   - **Difícil**: respawn deslocado (Selve Sombria se morto por criatura, casa destruída se morto por humano), stats quase-zero, recuperação por marcos (voltar à cidade / dormir / curar).
>   - **Hardcore/Hell** *(unlock só pós-vitória no Difícil)*: HP=0 dispara o **kernel-panic puzzle** (última chance) — resolve = sobrevive; falha = **permadeath**, save isolado protegido por anti-rollback offline (âncora out-of-band + machine-binding) e apagado por wipe por trechos.

E o bullet espelhado em "Sistemas-âncora > Dificuldade Normal / Hard" (linhas 249-252) vira um ponteiro pro doc novo em vez de repetir o texto.

**RESOLVIDO 2026-07-10 (contradição entre 2 canons):** o Pillar 4 original (2026-05-15) descrevia o Hard mode com um **kernel panic puzzle** como última chance antes do permadeath ("falhar = game over real" — morte condicional, não incondicional). O canon do Hardcore de 2026-07-03 (memória `project_morte_dificuldade_canon`) dizia que a morte era incondicionalmente final, sem mencionar o puzzle. Eu havia recomendado a opção **(a)** — manter o puzzle como última chance — e o **líder aprovou (a)**: o kernel-panic puzzle sobrevive no Hardcore (§2.3a), o wipe só dispara se o jogador falhar o puzzle. Preserva o gancho mecânico original e dá agência coerente com o Pillar 1 (lógica vence força) em vez de um dado puro de sorte-ou-fim.

---

## §5. Sinalizações novas pro líder (não decidido sozinho)

1. ~~Unlock do Hardcore via `profile.json`~~ — **REJEITADO pelo líder** (flag soft/editável furaria a integridade do Hardcore). Rework: unlock mora dentro da âncora selada+machine-bound (§2.3b). A opção da âncora é a que recomendo, mas **ainda é um ponto que volta pro líder confirmar** (não é 100% fechado — só a alternativa "morre com o profile.json" que foi descartada com certeza).
2. ~~A contradição do kernel-panic-puzzle~~ — **RESOLVIDA 2026-07-10**: líder aprovou a opção (a), puzzle mantido como última chance (§2.3a, §4).
3. **Locais reais de respawn do Difícil** ("dentro da Selve Sombria" / "casa destruída") não existem ainda como cena/coordenada — dependem do `level-designer`. Até lá, a implementação do dispatcher pode usar um respawn-point placeholder por `EnemyKind` (mesma filosofia do placeholder do M7), e o líder decide quando essa dívida entra na fila do level-designer.
4. **O gatilho "Dormir" (Marco 2, §2.4)** não existe hoje como mecânica (não há cama/pernoite no jogo). Proponho, como ponte, ele cair no MESMO "beat narrativo" genérico que já destrava cura grátis (`economia.md` §3.2) até uma mecânica de sono dedicada existir — mas aí os Marcos 1 e 2 ficariam mecanicamente idênticos (2 beats narrativos em sequência), o que esvazia um pouco a intenção de 3 marcos distintos. Vale o líder decidir se aceita essa simplificação temporária ou se quer segurar o Difícil inteiro até "Dormir" ter mecânica própria.
5. **Magnitude dos percentuais do Difícil (5% / 34% / 89% / 100%, §2.4) e N=3 passadas do reforço opcional do wipe (§2.3a Camada 3)** são PROPOSTA minha — **marcadas explicitamente pra fixar na onda de balanceamento** (`economy-designer` + playtest), não canon fechado.
6. **Mecanismo exato da âncora out-of-band, do bit de unlock dentro dela, e da chave de machine-binding (§2.3)** — o QUÊ e o PORQUÊ estão fixados nesta spec (decisão do líder), o COMO já está em produção como **ADR-014** pelo `security-engineer` (AEAD XChaCha20-Poly1305/Monocypher — memória `reference_libs_vendorizadas`) — não é uma decisão que eu (lead-game-designer) deva ou possa fechar sozinho.
7. **Detalhe do "sobrevive" no kernel-panic puzzle (§2.3a)**: proponho HP/stats no mesmo piso de 5% do respawn do Difícil (reusa o número, §2.4) em vez de inventar uma 2ª magnitude — mas é proposta, não fechada; também cabe na onda de balanceamento do item 5.

---

## §6. Plano de implementação incremental (anti-OE)

**Fase 0 — pode entrar já, zero dependência nova de conteúdo:**
- `DifficultyLevel` enum + campo `SaveData.difficulty` (+ `difficult_recovery_stage`), schema V5 aditivo + migrator V4→V5 (default `Medio`).
- Tela de seleção de dificuldade (`DifficultyMenuState` + RML + loop) encaixada no fluxo de Novo Jogo — Aviso #1 (legenda) + Aviso #2 (splash confirmar/cancelar).
- `EnemyKind` enum + campo `EnemyTemplate.kind` (aditivo, default `Creature`) — dado puro, sem consumidor ainda.
- **Fácil** funcional ponta-a-ponta: reload do último save no `CombatOutcome::Defeat` — reusa 100% o save I/O já existente (M2 ✅). É literalmente a substituição mais barata do placeholder do M7 pra saves marcados Fácil.

**Fase 1 — dispatcher central (depende só da Fase 0):**
- No `CombatOutcome::Defeat` com alvo Gus, ramifica por `SaveData.difficulty`. `Facil` já fica 100% operacional aqui (reusa Fase 0). `Medio`/`Dificil`/`Hardcore` ficam com stub/TODO explícito até as fases seguintes.

**Fase 2 — Médio (depende do motor de Hospital/dívida ser codado):**
- Hoje `economia.md` §3.1/§3.3 é canon de DESIGN, não há código do Hospital ainda. Médio só liga de verdade quando essa implementação existir (fora do escopo desta spec — é o pré-requisito natural do Médio).

**Fase 3 — Difícil (depende da Fase 1 + level design):**
- `difficult_recovery_stage` + os 3 marcos (§2.4) — Marco "voltar à cidade" e "cura completa" já têm gatilhos análogos no motor (beats narrativos, §3.2 economia.md); "Dormir" precisa da decisão do §5 item 4.
- Locais reais de respawn (§5 item 3) — bloqueia o polish final, mas o dispatcher pode nascer com placeholder.

**Fase 4 — Hardcore, mecanismo real + unlock (depende do fim-de-jogo existir):**
- Só é exercitável de ponta-a-ponta quando o jogo tiver um final jogável (bem à frente no roadmap) — o unlock (bit dentro da âncora selada, §2.3b) dispara na vitória do Difícil. Escopo desta fase, TODO explícito pro `security-engineer`, já em produção como **ADR-014** (não pro `lead-game-designer`, não decido crypto): formato da âncora out-of-band + o bit de unlock + mecanismo por SO, derivação da chave de machine-binding, cifra AEAD (XChaCha20-Poly1305/Monocypher), quais bytes do envelope contam como "selo/cabeçalho" pro wipe por trechos, e a implementação do kernel-panic puzzle em si (§2.3a, mini-jogo de UI — trabalho de `gameplay_engineer`/`frontend`, não crypto). Até lá, o design (§2.3) já está fechado o bastante pra esses sub-specs começarem assim que a Fase 4 entrar em pauta — não precisa esperar o resto da implementação.

---

**Resumo executivo p/ o líder:** os 4 modos ficam formalmente especificados; Fácil é codável JÁ (zero dependência de conteúdo); Médio espera o Hospital ser implementado; Difícil espera level design pros locais de respawn + uma decisão pequena sobre o gatilho "Dormir" — e ganhou o framing narrativo do despertar (log glitchado + 5 amarras, protege o Dragon Victory, §2.5); Hardcore MANTÉM o kernel-panic puzzle como última chance (opção (a) aprovada, resolve a contradição com o pillars.md original) e ganhou 3 camadas de defesa-em-profundidade 100% offline pro caso de falha no puzzle (âncora monotônica out-of-band = a que rejeita a cópia restaurada, e agora também guarda o UNLOCK do Hardcore sem precisar de arquivo soft separado; machine-binding via HMAC estendido do ADR-006; wipe por trechos do selo/cabeçalho, não do arquivo inteiro) — decisão consciente do líder de trocar o trade-off "forense-proof via servidor" (rejeitado, fura soberania de dado) por "derrota o cheater casual offline" (aceito), mesmo caveat que o Diablo hardcore enfrentaria sem servidor. O COMO técnico já está em produção como **ADR-014** pelo `security-engineer` (AEAD XChaCha20-Poly1305/Monocypher). 7 pontos ficam explicitamente pendentes de decisão do líder (§5): confirmação final de que o unlock vive na âncora (não no save-morto), os locais reais do Difícil, o gatilho "Dormir", a validação na onda de balanceamento dos percentuais do Difícil (5%/34%/89%/100%) e do N=3 do wipe, a confirmação de que o COMO da crypto fica com o `security-engineer` (ADR-014), e a magnitude do "sobrevive" no kernel-panic puzzle.
