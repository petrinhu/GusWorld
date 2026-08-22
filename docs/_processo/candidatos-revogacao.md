# Candidatos a revogação - GusWorld

**Autor:** technical-writer (agente operacional, levantamento somente leitura)
**Data:** 2026-08-21
**Base legal:** `GODS_LAWS.md` (25 leis, lido por inteiro antes de começar)
**Regras seguidas nesta tarefa:** L-14 (não declaro nada morto, só levanto candidato com evidência; a decisão é do líder), L-24 (revogado se apaga, não se arquiva como histórico, com a exceção de efeito jurídico), L-02, L-19, L-22, L-25 citadas com atenção especial pelo coordenador.

**Aviso de método, obrigatório por L-14:** este documento **não decide nada**. Cada item abaixo é um **candidato**. "Ação proposta" é a MINHA recomendação, não uma execução, e nenhuma delas foi executada. Nenhum arquivo do projeto foi tocado. Somente leitura, do início ao fim.

**Separação FATO/INFERÊNCIA:** toda citação com `arquivo:linha` é FATO (texto lido diretamente do arquivo). Quando escrevo por que considero algo contraditório, ou quando arrisco uma leitura do que a lei alcança, marco **[INFERÊNCIA]**.

Este relatório é escrito seção por seção, incrementalmente, começando pelo Grupo A.

---

## Grupo A - Revogado por decisão explícita do líder já registrada em lei

Ordenado do mais crítico (o que um agente leria e obedeceria por engano, com maior dano) para o menos crítico.

### A.1 - CONTRACT.md §6.1: convenção de nomes inteira (pt-br, `m_`, ALL_CAPS)

**Já apontado pelo coordenador.** Texto exato, `CONTRACT.md:362-369`:

```
### 6.1 Naming

- Names MUST reveal intent. No abbreviations unless universally known (`id`, `url`, `http`).
- Functions: verb + noun (`buscarItem`, `salvarCache`, `renderizarLista`).
- Booleans: `is`, `has`, `can`, `should` prefix (`isValido`, `hasCached`, `canRetry`).
- Constants: ALL_CAPS with underscores (`MAX_TENTATIVAS`, `URL_BASE`).
- Private members: `m_` prefix (`m_cliente_http`, `m_cache`).
- MUST NOT use single-letter names except loop counters (`i`, `j`) and lambda args.
```

**Por que é o mais crítico:** é a regra que um implementador aplica **a cada identificador escrito**, em todo arquivo, sem exceção. Se seguida como está, **100% do código nasceria errado** e precisaria de rename em massa.

**Lei que revoga:** `L-22` (GODS_LAWS.md:314-322) - "Identificador e comentário de código em inglês, no estilo da biblioteca padrão de C++: `snake_case`, sem prefixo `m_`... Isto **substitui** o `CONTRACT.md` §6.1". A própria lei já se autodeclara substituta explícita deste parágrafo.

**Nuance [INFERÊNCIA]:** `ALL_CAPS` para constantes não é mencionado nominalmente por L-22 (que fala de `snake_case` e ausência de `m_`), mas a biblioteca padrão de C++ (`std::numeric_limits`, etc.) não usa `ALL_CAPS` para constantes internas - então a linha de constantes também é candidata, com confiança um pouco menor que as outras duas (pt-br e `m_`).

**Ação proposta:** apagar o parágrafo inteiro de `CONTRACT.md:362-369` e substituir por uma remissão a `L-22` (ou por texto novo equivalente, decisão do líder).

---

### A.2 - CONTRACT.md §12.1: seção inteira "C++ / Qt (Mandatory)"

Texto exato, `CONTRACT.md:767-793` (trecho representativo; a seção é maior):

```
## 12. Language-Specific Rules

### 12.1 C++ / Qt *(Mandatory)*

**Version:** C++20 minimum. Qt 6.x.
...
**Qt Specifics:**
- MUST use Qt parent-child ownership for widgets (parent deletes children).
- MUST use signals/slots for cross-layer communication (Observer pattern built-in).
- MUST NOT call network, file I/O, or database from the UI thread.
- MUST use `QThread` + `moveToThread()` or `QtConcurrent` for background work.
- MUST use `Qt::QueuedConnection` when emitting across threads.
- MUST use `QSqlQuery` with `bindValue()` - NEVER string concatenation for SQL.
- MUST check `QNetworkReply::error()` before reading response data.
- MUST handle all `QFile::open()` failures.
- Theme/style: MUST use QSS via central theme system, NEVER `setStyleSheet()`...
```

**Por que é crítico:** o rótulo **"Mandatory"** está escrito no próprio cabeçalho. Um agente implementador que não tenha lido `GODS_LAWS.md` (ou que leia CONTRACT.md depois e "misture tudo", exatamente o risco que motivou a L-24) pode concluir que Qt é obrigatório no GusWorld. Isso violaria simultaneamente:
- **Lei Zero** (GODS_LAWS.md:1): "O GusWorld liga em exatamente duas coisas: o GlintFx e o sistema operacional" - Qt é uma terceira coisa, proibida por construção.
- **L-03**: "Version: C++20 minimum" contradiz C++23.
- **L-05**: sinais/slots do Qt seriam um framework paralelo de comunicação entre camadas, o oposto de "ou liga direto no GlintFx, ou não existe".
- **L-17**: a arquitetura de 5 camadas (`present/app/domain/content/core`) não tem lugar para `QThread`, `QSqlQuery` ou `QNetworkReply` - o jogo é single-player offline sem rede nem banco de dados (confirmado por `docs/design/pillars.md:33-34`, anti-pillar "Não é IAP", "Não é always-online").

**Ação proposta:** apagar a seção `12.1` inteira e, se o líder quiser uma seção C++/GlintFx no lugar, escrevê-la do zero alinhada a L-17.

---

### A.3 - CONTRACT.md §5.1: "Architecture Layers" de 4 camadas genéricas

Texto exato, `CONTRACT.md:317-340` (trecho):

```
## 5. Architecture Layers

### 5.1 Layer Rules
┌─────────────────────────────────────────┐
│  FRONTEND / PRESENTATION                │  UI components, widgets, views
│  MIDDLEWARE / APPLICATION / SERVICE     │  Use cases, orchestration
│  BACKEND / DOMAIN                       │  Entities, interfaces, rules
│  INFRASTRUCTURE / DATA                  │  HTTP, SQL, file system, APIs
└─────────────────────────────────────────┘
...
[ ] No domain entity imports Qt network/SQL/widget modules?
```

**Lei que revoga:** `L-17` (GODS_LAWS.md:209-244) define **cinco** camadas nomeadas e ordenadas diferentemente - `present/ → app/ → domain/ → content/ → core/` - com gate de CI específico (`#include <glintfx/` só em `present/`), e nenhuma camada "Infrastructure/Data" genérica de HTTP/SQL (o jogo não tem rede nem banco). O checklist final (`CONTRACT.md:352`) ainda cita "Qt network/SQL/widget modules" nominalmente, reforçando a incompatibilidade.

**Ação proposta:** apagar `CONTRACT.md:317-359` (seção 5 inteira, "Architecture Layers" + checklist) e substituir por remissão a L-17, ou por uma versão reescrita das 5 camadas.

---

### A.4 - TESTES.md T1: ferramenta QtTest + meta de 70% de cobertura

**A meta de cobertura já foi apontada pelo coordenador.** Texto exato, `TESTES.md:47-49`:

```
## T1 - Testes Unitários

**Objetivo:** verificar que cada módulo se comporta conforme especificado de forma isolada.

**Ferramenta:** QtTest (embutido no Qt6) ou Google Test.

**Critério de aprovação:** 0 falhas. Cobertura mínima de 70% nos módulos críticos.
```

**Duas revogações no mesmo parágrafo:**
- "Ferramenta: QtTest (embutido no Qt6)" - revogado pela **Lei Zero** + **L-05** (Qt não liga no projeto; framework de teste teria que ser o que o GlintFx já usa ou Google Test puro, sem o Qt).
- "Cobertura mínima de 70% nos módulos críticos" - revogado por **L-19** (GODS_LAWS.md:281): "**Sem meta numérica de cobertura.** Com TDD estrito, todo código de comportamento nasce de um teste que falhou, então cobertura é consequência, não alvo."

**Ação proposta:** reescrever T1 removendo a menção a QtTest e a meta de 70%, alinhando ao ciclo vermelho/verde/refatorar de L-19 e aos quatro portões (zero warning, ASan/UBSan, clang-tidy/cppcheck, gitleaks).

---

### A.5 - `docs/design/producao/ci-build-plan.md`: gate de release em 1 distro só, Windows "deferido pós-v1"

Texto exato, `docs/design/producao/ci-build-plan.md:12`:

> "**Decisões canônicas (2026-06-03):** artefatos do gate = `.tar.gz` + `.rpm` (Fedora-first); **AppImage adiado** pós-VS. **Smoke em 1 distro (Fedora) como gate, Ubuntu não-bloqueante.**"

E `docs/design/producao/ci-build-plan.md:70,76`:

> "Windows export (`build_windows.sh`, F2-CI.1) | **Deferido pós-v1**... Windows: export template preparado mas FORA do gate v1... ship pós-v1."

**Lei que revoga:** `L-20` (GODS_LAWS.md:287-293): "**A matriz das cinco plataformas existe desde o primeiro commit.** Fedora 44 pinado, Ubuntu, Arch, CachyOS e Windows, cada um com entrada própria." e `L-09` (GODS_LAWS.md:130-136): cinco alvos, cinco entradas distintas na matriz de CI, nenhuma como "não-bloqueante" ou "pós-v1".

**Por que é crítico:** um agente que monte o CI usando este documento como fonte configuraria **1 plataforma** (Fedora) como gate único e adiaria Windows/Arch/CachyOS - o oposto direto de L-20, e bem no início do projeto, quando a fundação de CI é a primeira fatia (L-20: "a partir do primeiro módulo de verdade, não há mais essa saída").

**Nota de sobreposição com Grupo B:** este documento já se autodeclara "escrito sobre o stack Godot/C# aposentado" (`ci-build-plan.md:6`, nota de stack), mas a decisão de gate-em-1-distro **não está dentro dessa ressalva** - é apresentada como "Decisões canônicas" agnósticas de stack, então trato como Grupo A pleno, não como bloco histórico já neutralizado.

**Ação proposta:** reescrever a seção de "Decisões canônicas" e a tabela de plataformas para refletir a matriz de cinco alvos desde o primeiro commit (L-20), com decisão do líder sobre se o "gate" muda de conceito ou se as cinco entradas viram gate conjunto.

---

### A.6 - `ASSETS-LICENSE.md` e `NOTICE`: "Código = Apache License 2.0"

Texto exato, `ASSETS-LICENSE.md:13,21,37`:

> "| Código-fonte | Apache License 2.0 | ver [LICENSE](LICENSE) |"
> "## Código = Apache License 2.0"
> "A licença completa está em [LICENSE](LICENSE) (texto Apache License 2.0 verbatim, imutável)."

Texto exato, `NOTICE:4-5`:

> "This product includes software developed by petrinhu, licensed under the Apache License, Version 2.0."

**Lei que revoga:** `L-08` (GODS_LAWS.md:120-128): "**Código: AGPL-3.0-or-later**... Motivo técnico... o GlintFx é AGPL-3.0-or-later. O executável do GusWorld linkado a ele é obra combinada, e distribuir esse binário obriga a obra inteira a sair sob AGPL-3.0."

**Verificação de risco jurídico feita antes de classificar como A e não D:** confirmei que **não existe arquivo `LICENSE`** no repositório (`ls LICENSE` = "Arquivo ou diretório inexistente") e que **não há nenhum commit no repositório** (fato dado na tarefa original). Ou seja: nenhum binário sob Apache foi de fato distribuído **a partir deste repositório**. O forward-statement "código = Apache" é seguro de revisar (Grupo A). **Mas** o próprio texto do arquivo (`ASSETS-LICENSE.md:29-35,166-168`) alega que "releases já publicadas sob GPLv3/AGPL-3.0 permanecem naquela licença para quem já as recebeu" - isso descreve uma distribuição histórica **fora deste repositório** (era anterior do projeto) que eu não tenho como confirmar nem descartar sozinho. **Essa cláusula específica NÃO entra no Grupo A - vai para o Grupo D**, junto com a Zona 1 CC-BY-SA.

**Ação proposta:** em `ASSETS-LICENSE.md` e `NOTICE`, trocar "Apache License 2.0" por "AGPL-3.0-or-later" no que descreve o regime **vigente/forward** do código, preservando (Grupo D) as cláusulas sobre o que já foi distribuído no passado até o líder confirmar. Redigir a ressalva exigida por L-08 (nota explicando que o jogo é offline, sem servidor) no README e em nota própria, nunca dentro do texto da licença.

---

### A.7 - `docs/design/pillars.md:27`: "Cel-shaded 3D low-poly"

**Já apontado pelo coordenador e por mim no relatório anterior.** Texto exato:

> `docs/design/pillars.md:27`: "Não é fotorrealista. Cel-shaded 3D low-poly, paleta restrita (style guide define)."

**Lei que revoga:** `L-02` (GODS_LAWS.md:58-64): "O jogo é **2D pixel-art em runtime**." Também listado nominalmente em `L-13` (GODS_LAWS.md:180) como contradição já identificada.

**Ação proposta:** reescrever a linha do anti-pillar de estética para "Não é fotorrealista. 2D pixel-art, paleta restrita (style guide define)." - decisão de redação final cabe ao líder.

---

### A.8 - `docs/design/mecanicas/core-loop-exploracao.md:95`: a palavra "orbital" (não a perspectiva 3/4 inteira)

Texto exato:

> `docs/design/mecanicas/core-loop-exploracao.md:95`: "**DA-1** | Câmera e navegação no overworld | **3/4 orbital + controle direto** - WASD/stick move Gus; câmera orbita por botão separado. Coerente com "3D real" e combat.md."

**Lei que revoga:** `L-13` (GODS_LAWS.md:181) já julga explicitamente a palavra: "**Perspectiva 3/4 sobrevive em 2D; orbital não.**" Ou seja, a própria lei já separou o que está morto (`orbital`, `câmera orbita por botão`, `"3D real"`) do que ainda depende de decisão (perspectiva final de câmera).

**Por que só o fragmento "orbital"/"3D real", não a linha inteira:** a perspectiva "3/4" e o esqueleto WASD/controle direto **não** foram revogados por L-13 - só a rotação de câmera livre (orbital) e a alegação de que é "3D real". A decisão final de câmera (3/4 fixa, top-down, ou outra) está **pendente** (ver Grupo E, item E.1) - por isso este item aparece nos DOIS grupos, com recortes diferentes: a palavra "orbital" e a frase "Coerente com '3D real'" já morreram (Grupo A); o resto da decisão DA-1 aguarda o líder (Grupo E).

**Ação proposta:** apagar apenas "orbital" e "Coerente com '3D real' e combat.md" desta linha; deixar "3/4... WASD/stick move Gus" como está até o líder decidir a câmera final (Grupo E).

---

### A.9 - 17 ocorrências de "C++20 + `__DEP_REMOVIDA__`" como pilha vigente (forward, não em bloco histórico)

**Achado consolidado.** Contei 17 ocorrências exatas da frase "C++20 + `__DEP_REMOVIDA__`" no corpus. Duas delas (`docs/art/style-guide.md:227,229`) **já estão dentro do bloco `<details><summary>Histórico</summary>` do próprio arquivo** e vão para o Grupo B. As outras **15 são afirmação corrente/vigente**, não histórica, e por isso entram no Grupo A:

| Arquivo:linha | Contexto |
|---|---|
| `docs/design/gdd.md:3` | "Escopo solo G1, engine própria C++20 + `__DEP_REMOVIDA__` (ADR-008), PC Linux v1 (Windows pós-v1)" |
| `docs/art/style-guide.md:5,7` | "hoje superado pelo pivô C++20+`__DEP_REMOVIDA__`" / "engine própria C++20 + `__DEP_REMOVIDA__`" |
| `docs/art/characters/gus.md:70` | "Test render na engine (C++20 + `__DEP_REMOVIDA__`)" |
| `docs/design/gus-abertura.md:274` | "stack C++20+`__DEP_REMOVIDA__`/2D" |
| `docs/design/narrativa/dialogue-tree-npc-intro.md:115` | "re-pivot para a engine própria C++20 + `__DEP_REMOVIDA__` (ADR-008)" |
| `docs/design/producao/plano_vs.md:5` | "engine própria C++20 + `__DEP_REMOVIDA__`" |
| `docs/design/producao/ci-build-plan.md:6` | idem |
| `docs/design/producao/art-spike-protocol.md:6` | idem |
| `docs/design/mecanicas/combat.md:727` | "No motor **C++20 + `__DEP_REMOVIDA__`** o equivalente é..." |
| `docs/design/mecanicas/battle-screen.md:3` | "Implementacao no M5 (BattleScreen) da engine C++20 + `__DEP_REMOVIDA__`" |
| `docs/narrative/deep/_INDEX.md:151` | "engine própria C++20 + `__DEP_REMOVIDA__`, em andamento" |
| `docs/narrative/diary/knowledge-gates.md:376` | "engine própria C++20 + `__DEP_REMOVIDA__`" |
| `docs/narrative/deep/ontologia/leitmotivs-musicais-detalhados.md:142` | "engine própria (C++20 + `__DEP_REMOVIDA__`, áudio via `__DEP_REMOVIDA__`)" |
| `docs/narrative/diary/ui-spec.md:411` | "UI da engine própria (glintfx sobre C++20 + `__DEP_REMOVIDA__`)" |

**Lei que revoga:** dupla revogação em cada ocorrência - **`L-03`** (C++23, não C++20) e **Lei Zero** (GusWorld liga só em GlintFx + SO; `__DEP_REMOVIDA__` descreve uma dependência que, por tudo que o corpus mostra, não é o GlintFx). `gdd.md:3` é revogado também por **`L-20`** ("PC Linux v1, Windows pós-v1" contradiz a matriz de 5 plataformas desde o primeiro commit).

**Ação proposta:** trocar "C++20" por "C++23" em todas as 15 ocorrências; decidir com o líder se `__DEP_REMOVIDA__` deve virar simplesmente "GlintFx" (se o termo redigido era mesmo o nome antigo do dependency) ou ser removido por completo do texto, já que a arquitetura L-17 não descreve mais uma dependência genérica e sim GlintFx nominalmente.

---

### A.10 - `docs/narrative/diary/knowledge-gates.md:380-426`: schema de save em JSON (texto)

Texto exato, `docs/narrative/diary/knowledge-gates.md:380`:

> "### 10.1 Save schema (JSON versionado `save_version: 1`)"

seguido de um exemplo de objeto JSON legível (`:384` em diante).

**Lei que revoga:** `L-18` (GODS_LAWS.md:247-259): "**Nenhum dado do jogo é armazenado em formato de texto.** A proibição alcança mapa, save, configuração do jogador e item." JSON é formato de texto por definição. Reforçado por `L-25` (GODS_LAWS.md:355-383): "envelope binário único... serialização binária versionada dos POCOs de domínio."

**Ação proposta:** apagar a seção 10.1 ou reescrevê-la como referência ao envelope binário de L-25 (o "conteúdo" descrito - `save_version`, IDs de trigger, `cross_ref_pairs` - pode sobreviver como **campo de dado**, só o formato de arquivo precisa deixar de ser JSON).

---

### A.11 - `resources/dialogues/*.dlg.txt`: formato de texto lido em runtime pelo jogo

Texto exato, `resources/dialogues/npc_intro_bertoldo.dlg.txt:8-10`:

> "o `SUCESSOR` consumido pelo runtime C++ (`DialogueRuntime`/`parse_text_to_dialogue_graph`) [...] Este arquivo (`.dlg.txt`) e a fonte editavel de verdade."

**Lei que revoga (com ressalva):** `L-25` (GODS_LAWS.md:371) permite texto estruturado **dentro do repositório**, desde que **lido apenas pelo gerador em tempo de build** - "**Nada em texto na distribuição**". O cabeçalho deste arquivo descreve o oposto: consumo pelo **runtime** (o jogo rodando), não por um gerador de build. Se essa descrição for literal, contradiz L-18/L-25.

**Confiança menor que os itens anteriores [INFERÊNCIA]:** não tenho como confirmar, só pela prosa do `.dlg.txt`, se "runtime" aqui significa "o executável final do jogador" ou "o motor de teste/build que compila o grafo de diálogo antes do empacotamento" - a diferença decide se isto viola a lei ou não. Por isso concentro a ação proposta em levar o caso ao líder, não em apagar.

**Ação proposta:** não apagar sozinho; perguntar ao líder se `.dlg.txt` é fonte de build (compatível com L-25) ou formato de runtime (incompatível), e ajustar a descrição do cabeçalho para dizer explicitamente qual dos dois é.

---

### A.12 - `resources/translations/pt_br.md`: formato-texto com loader C# fantasma

Texto exato, `resources/translations/pt_br.md:5`:

> "Formato: `## CHAVE_UPPER_SNAKE` + valor abaixo. Ver engine/foundation/localization/`MdTranslationLoader.cs`."

**Duas camadas de problema:** (1) o loader citado é C# e não existe no repositório (Grupo C, ver C.4); (2) se as traduções forem carregadas como texto em runtime, é o mesmo tipo de tensão com L-18/L-25 do item A.11.

**Ação proposta:** mesma recomendação do item A.11 - não apagar sozinho, perguntar ao líder se strings de tradução contam como "item"/"configuração" para efeito de L-18, e corrigir a referência ao loader C# fantasma de qualquer forma (isso não depende da resposta sobre formato).

---

## Grupo B - Blocos explicitamente marcados como histórico, superado ou de era anterior

Por `L-24`, tudo neste grupo é candidato a apagamento total (não arquivamento). Por `L-14`, a decisão de apagar cada um é do líder - aqui só listo e meço.

### B.1 - `docs/art/style-guide.md:220-344` (125 linhas) - bloco `<details>` completo

```
docs/art/style-guide.md:220: <details>
docs/art/style-guide.md:221: <summary>Histórico — spec 3D (era Godot, superada)</summary>
docs/art/style-guide.md:344: </details>
```

**Conteúdo:** a spec 3D inteira (proporção de mesh, budget de tris, Lighting, Shader Strategy, Texture Pipeline, anti-referências formuladas em termos 3D) preservada dentro do colapsável. **Maior bloco histórico do corpus.**

**Ação proposta:** apagar as 125 linhas (220-344) inteiras, inclusive as tags `<details>`/`<summary>`/`</details>`.

### B.2 - Os 8 blocos `<details>` de `docs/specs/character-spec-*.md` (306 linhas no total)

| Arquivo | Linhas do bloco | Tamanho |
|---|---|---|
| `docs/specs/character-spec-bento-requiem.md` | 25-59 | 35 linhas |
| `docs/specs/character-spec-caua-volt.md` | 23-57 | 35 linhas |
| `docs/specs/character-spec-dante-grid.md` | 26-61 | 36 linhas |
| `docs/specs/character-spec-gus.md` | 28-82 | 55 linhas |
| `docs/specs/character-spec-iara-lumen.md` | 25-59 | 35 linhas |
| `docs/specs/character-spec-jaci-proxy.md` | 26-60 | 35 linhas |
| `docs/specs/character-spec-linda-siren.md` | 25-58 | 34 linhas |
| `docs/specs/character-spec-sterling-locke.md` | 29-69 | 41 linhas |

**Conteúdo de cada bloco:** o mesmo padrão - "Histórico - spec 3D (era Godot, superada)" com prompt de geração de imagem 3D (Midjourney/Stable Diffusion), proporção de mesh, budget de tris.

**Nuance [INFERÊNCIA]:** diferente do bloco B.1 (puramente descritivo de pipeline técnico), estes blocos contêm o **prompt textual de cada personagem** (silhueta, traço de identidade, cor-marca), que pode ter valor de referência para reconstruir o prompt PixelLab 2D equivalente (a própria `docs/specs/_INDEX.md:9` diz que a spec 2D detalhada ainda não foi escrita). Apagar sem antes extrair os traços de identidade (não-3D) para algum lugar pode custar retrabalho. Registro isso como consideração para o líder, não como recomendação de manter.

**Ação proposta:** apagar os 8 blocos por completo, e se o líder quiser preservar os traços de identidade (silhueta, cor-marca, acessório-assinatura) para uso futuro no prompt 2D, extraí-los ANTES do apagamento para uma seção nova "Traços de identidade" fora do bloco histórico, no mesmo arquivo.

### B.3 - `docs/design/levels/blockout-distritos-inferiores.md:335-340` (6 linhas)

Texto exato:

> "`<details>`
> `<summary>Traçado anterior de 30x20 (superado em 2026-08-04, **mantido por registro**)</summary>`
> [conteúdo do traçado antigo]
> `</details>`"

**Achado de maior risco neste subgrupo:** o próprio título do bloco usa literalmente a expressão **"mantido por registro"** - exatamente o padrão que `L-24` proíbe pelo nome ("Nada de bloco 'Histórico', 'Superado', 'era antiga', 'mantido para registro'"). Cito porque é o caso mais claramente nomeado pela lei no corpus inteiro.

**Ação proposta:** apagar as 6 linhas.

### B.4 - "NOTA DE STACK" em `docs/design/producao/*` (3 arquivos, parágrafo único cada, ~10-14 linhas cada)

Diferente de B.1-B.3 (blocos colapsáveis com tag HTML), estes são parágrafos de aviso no início do documento, não escondidos, mas explicitamente rotulados como nota sobre stack aposentado:

- `docs/design/producao/plano_vs.md:5` - "**NOTA DE STACK (2026-06-23, pós-ADR-008).** Este plano foi escrito sobre o stack Godot 4 + C# .NET 8 AOT, depois aposentado [...]"
- `docs/design/producao/ci-build-plan.md:6` - mesmo padrão, sobre export templates Godot e PoC AOT.
- `docs/design/producao/art-spike-protocol.md:6` - mesmo padrão, sobre import Godot + material toon.

**Nuance:** diferente do Grupo A (onde as "Decisões canônicas" dentro destes mesmos arquivos seguem valendo e contradizem lei nova), estas notas específicas **já se autodeclaram históricas** - a diferença de A.5 é que A.5 mira a tabela de plataformas (que NÃO está dentro da nota, e contradiz lei nova), enquanto aqui miro só o parágrafo da nota em si (que já é auto-descrito como morto).

**Ação proposta:** apagar os 3 parágrafos de nota (não apagar o resto de cada documento, que segue canônico segundo os próprios arquivos).

### B.5 - `docs/design/mecanicas/dialogue-tree-npc-intro.md:115`: linha "DA-2 | ... OBSOLETO (pós-ADR-008)"

Texto exato (célula de tabela):

> "**DA-2** | Lib/sistema de diálogo | **OBSOLETO (pós-ADR-008).** A escolha original era DialogueManager (addon Godot 4, MIT) [...] O blueprint de diálogo deste doc segue válido [...] Ver ROADMAP.md."

**Ação proposta:** apagar a célula "OBSOLETO" e sua justificativa, mantendo a linha da tabela vazia ou com nota curta "decisão de lib de diálogo: N/A na engine atual", já que o blueprint (regra em si) segue válido segundo o próprio texto.

### B.6 - `docs/design/mecanicas/combat.md:727` (parágrafo "Nota de stack")

Texto exato (início):

> "> **Nota de stack (2026-06-23, pós-ADR-008).** Esta seção descreve a integração no vocabulário do protótipo **C# + Godot** (signals, `CombatManager` como 'ponte Node', `game/tools/TestCombatIntegration.cs`), que morre no M8. No motor **C++20 + `__DEP_REMOVIDA__`** o equivalente é [...]"

**Ação proposta:** apagar o parágrafo. **Ressalva:** a segunda metade do parágrafo (o que o motor C++ atual faz) tem conteúdo técnico não-histórico (o contrato de acumular `StatusEffectChange` numa lista drenável) que pode valer a pena preservar reescrito, fora do enquadramento "nota de stack" - decisão do líder.

### B.7 - `ASSETS-LICENSE.md:140-154`: "Regra de fronteira (histórica): arquivos de cena Godot (.tscn / .tres)" (15 linhas)

Texto exato do título e da nota:

> "## Regra de fronteira (histórica): arquivos de cena Godot (.tscn / .tres)
> > **Nota (2026-07-22):** o stack Godot/C# foi decommissionado no marco M8; arquivos `.tscn` e `.tres` não existem mais no repositório [...]; não se aplica a nenhum arquivo do repositório atual."

**Ação proposta:** apagar as 15 linhas (140-154) inteiras.

### B.8 - Menções pontuais de 1 linha "SUPERADO"/"OBSOLETO" espalhadas (contagem, não itemizadas uma a uma)

Além dos blocos grandes acima, há **menções de uma linha** marcando um número ou decisão específica como superada, sem formar bloco próprio. Localizadas por `grep`, listo por arquivo (o texto completo de cada uma está disponível se o líder quiser, omito aqui para não inflar o relatório com dezenas de linhas repetitivas):

- `docs/design/mecanicas/cartas-statlines-rascunho.md:3` - aviso de que o documento inteiro está "SUPERADO LINHA-A-LINHA por `_EFEITOS-ESCOLHIDOS.md`", mas ainda serve como "referência de formato de tabela". **[INFERÊNCIA]** este é um caso limítrofe: o documento não é 100% morto (mantém utilidade de formato), então talvez não deva ser apagado por inteiro - registro para o líder decidir entre apagar tudo, apagar só o conteúdo divergente, ou manter como está.
- `docs/design/mecanicas/deck-mao-sistema.md:137` - 1 linha, "**Supersessão:** o canon antigo 'deck de 15 em campo' [...] fica SUPERADO pela estrutura bolsa→mão."
- `docs/narrative/characters/prelore_vilao.md`, `docs/narrative/characters/sterling-locke.md`, `docs/narrative/deep/antagonists/sterling-locke-deep.md`, `docs/narrative/deep/factions/ordem-recursiva.md`, `docs/narrative/deep/factions/sterling-corp.md`, `docs/narrative/environments/05-setor-mirage.md`, `CHARS.md`: ocorrências da palavra "SUPERADO"/"OBSOLETO" em contexto de **retcon narrativo** (versão antiga de vilão/facção substituída por versão nova de lore), não de stack técnico. **[INFERÊNCIA] Estas provavelmente NÃO são candidatas a esta tarefa** - `L-24` fala de "regra, seção ou documento que o líder revogou" no contexto desta rodada de leis técnicas (21/08/2026); retcon de lore é um tipo de decisão diferente, já coberto por processo editorial próprio do projeto. Sinalizo a existência delas por completude, mas não recomendo tratá-las como parte deste levantamento sem confirmação do líder de que L-24 alcança também revisão de lore.

**Ação proposta geral deste subitem:** líder decide, item a item, se quer a lista completa e granular de todas as ocorrências (posso levantar) ou se os 2 primeiros bullets (técnicos) bastam para esta rodada.

---

## Grupo C - Referências a coisas que não existem

Agrupado por alvo inexistente. Confirmei a não-existência de cada alvo com `find`/`ls` no repositório real, em 2026-08-21.

### C.1 - `GusEngine/` (o código-fonte C++/C# antigo)

**Confirmado inexistente:** `find . -iname "GusEngine*"` não retorna nada.

**Ocorrências:** 14, entre elas `docs/design/mecanicas/cartas-spec-dados.md:12-13` (`GusEngine/domain/include/gus/domain/combat/combat_records.hpp`, `GusEngine/domain/deck/deck_records.hpp`), `docs/design/mecanicas/combat.md:752` (`game/tools/TestCombatIntegration.cs`), e outras referências de caminho de arquivo espalhadas em `docs/design/mecanicas/*`.

**Nota de precedência:** `L-01` (GODS_LAWS.md:48-56) já resolve o alcance disto: "o `GusEngine/` citado no corpus [...] não são base, não são referência, não são canon [...] trate como descrição de intenção, não como arquivo consultável." Ou seja, este grupo **já tem uma lei que rege como tratá-lo** (não como código a apagar do corpus, mas como intenção de design a reinterpretar) - por isso não proponho apagar as referências, só sinalizo onde estão para quem for extrair os campos das entidades da prosa.

**Ação proposta:** não apagar. Cada implementador que for derivar uma entidade de dado (carta, save, diálogo) desses documentos deve saber que o caminho de arquivo citado é fantasma, e derivar da prosa, como a própria L-01 já manda.

### C.2 - `ADR-001` a `ADR-021` (15 números distintos citados, nenhum arquivo existe)

**Confirmado inexistente:** `find . -iname "ADR-*.md"` não retorna nada em lugar nenhum do repositório; `docs/tech/` (caminho citado nos links) também não existe.

**Contagem de citações por número** (`grep -rohE "ADR-0[0-9]{2}" docs *.md | sort | uniq -c`):

| ADR | Ocorrências |
|---|---|
| ADR-016 | 38 |
| ADR-008 | 33 |
| ADR-010 | 12 |
| ADR-006 | 9 |
| ADR-001 | 9 |
| ADR-021 | 8 |
| ADR-020 | 8 |
| ADR-015 | 6 |
| ADR-014 | 6 |
| ADR-002 | 5 |
| ADR-019 | 3 |
| ADR-005 | 3 |
| ADR-017 | 1 |
| ADR-004 | 1 |
| ADR-003 | 1 |

**Nota de precedência:** `GODS_LAWS.md:396` (pendência de lei) já registra: "**Numeração de ADR recomeça em ADR-001** neste projeto; o destino das citações órfãs do corpus é decisão do líder, caso a caso (L-14)." Ou seja, o líder **já sabe** que a numeração vai recomeçar, e já decidiu (por antecipação, na própria lei) que o destino de cada citação órfã é dele, caso a caso - reforça que eu não decido nem apago, só listo.

**Ação proposta:** não apagar nenhuma citação de ADR sem o líder decidir, caso a caso, qual das 15 vira um ADR real novo (renumerado a partir de ADR-001) e qual é órfã de fato (decisão que documentava algo já revogado por outra via).

### C.3 - `ROADMAP.md`

**Confirmado inexistente:** `find . -iname "ROADMAP.md"` vazio.

**Ocorrências:** citado como onde ficam os detalhes de arquitetura de engine atual em pelo menos `docs/design/mecanicas/dialogue-tree-npc-intro.md:115`, `docs/narrative/diary/ui-spec.md:411`, `docs/design/mecanicas/battle-screen.md` (implícito via "ver ROADMAP"), `docs/art/style-guide.md:227`, entre outros.

**Ação proposta:** não apagar as citações (elas fazem sentido assim que o `ROADMAP.md` for criado); levar ao líder a pergunta de saber se este documento deve ser criado agora, dado que `L-17` já fixa a arquitetura de 5 camadas em lei - o `ROADMAP.md` poderia nascer como a materialização dessa lei em cronograma.

### C.4 - `MdTranslationLoader.cs`

**Confirmado inexistente:** nenhum arquivo `.cs` existe no repositório (busca de `find . -iname "*.cs"` não retorna nada; consistente com L-01, código antigo em C# não é base).

**Ocorrência:** `resources/translations/pt_br.md:5` - "Ver engine/foundation/localization/`MdTranslationLoader.cs`."

**Ação proposta:** apagar ou reescrever a citação do loader (já que é C# e o projeto agora é C++23/GlintFx), mantendo o formato de arquivo (`## CHAVE_UPPER_SNAKE`) como está até o líder decidir se strings de tradução também vão para o envelope binário de L-25 (ver item A.12).

### C.5 - `.tscn` / `.tres` (arquivos de cena Godot)

**Confirmado inexistente:** `find . -iname "*.tscn" -o -iname "*.tres"` vazio.

**Ocorrência principal:** o bloco inteiro `ASSETS-LICENSE.md:140-154` (já listado como B.7).

**Ação proposta:** coberto pela ação de B.7 (apagar o bloco).

### C.6 - `THIRD-PARTY-LICENSES.md`

**Confirmado inexistente:** `ls THIRD-PARTY-LICENSES.md` = "Arquivo ou diretório inexistente".

**Ocorrência:** `ASSETS-LICENSE.md:159` - "Bibliotecas e fontes de terceiros têm licenças próprias, listadas em [THIRD-PARTY-LICENSES.md](THIRD-PARTY-LICENSES.md)."

**Ação proposta:** não apagar o link (ele é um lembrete legítimo de que o arquivo precisa ser criado, especialmente agora que GlintFx e possivelmente outras libs entram via FetchContent); levar ao líder como item de criação, não de revogação.

### C.7 - `docs/tech/` (diretório inteiro, inclusive `docs/tech/ai-assets-provenance.md` e `docs/tech/adr/`)

**Confirmado inexistente:** `ls docs/tech` = "Arquivo ou diretório inexistente".

**Ocorrências:** `ASSETS-LICENSE.md:63` (`docs/tech/ai-assets-provenance.md`), mais todos os links `docs/tech/adr/ADR-XXX-....md` já contados em C.2.

**Ação proposta:** mesma recomendação de C.2/C.6 - depende de decisão do líder sobre se o diretório `docs/tech/` nasce (com ADRs renumerados) ou se as citações são reescritas sem o caminho.

### C.8 - `docs/mockups/`

**Confirmado inexistente:** `ls docs/mockups` = "Arquivo ou diretório inexistente".

**Ocorrências:** `docs/design/card-frame-spec.md:7-8` (`mockups/16-moldura-cartas.html`, `17-moldura-cores-dominio.html`) e `docs/design/mecanicas/terminal-estetica.md:30` (`docs/design/mockups/11-terminal-glitch-glyphs.html`).

**Nota [INFERÊNCIA]:** estas citações a arquivos HTML de mockup são diferentes das outras deste grupo - **não** são resíduo do stack Godot antigo, parecem ser referência a um mockup de UI que foi feito em algum momento (provavelmente numa sessão de chat, não versionado) e nunca chegou a virar arquivo no repositório. Não necessariamente "fantasma da era anterior", pode ser "nunca commitado".

**Ação proposta:** não apagar; perguntar ao líder se os mockups existem em algum lugar (histórico de chat, máquina local) e podem ser recuperados/versionados, ou se a citação deve ser removida por não haver mais o artefato.

---

## Grupo D - Texto com efeito jurídico ou de direito adquirido (NÃO apagar sem análise)

Esta é a exceção explícita de `L-24` (GODS_LAWS.md:351): "texto cuja remoção teria efeito jurídico ou de direito adquirido [...] é irrevogável por natureza. Nesse caso o agente **não apaga e leva o caso ao líder**, explicando por quê." Nenhum item abaixo deve ser tocado.

### D.1 - `ASSETS-LICENSE.md:41-64` - Zona 1, CC-BY-SA 4.0, irrevogável

Texto exato (`:41-49`):

> "## Zona 1: assets publicados até 2026-07-31 (CC-BY-SA 4.0, irrevogável)
>
> Todo asset que já estava publicado no repositório em ou antes de 2026-07-31 permanece sob **Creative Commons Atribuição-CompartilhaIgual 4.0 Internacional (CC-BY-SA 4.0)**, para sempre, para quem já recebeu. Isto vale com todas as letras, sem eufemismo: quem clonou, baixou ou recebeu esse conteúdo enquanto ele estava sob CC-BY-SA continua com a licença que recebeu. Uma decisão do titular sobre assets futuros não desfaz uma licença já concedida sobre o passado."

**Já apontado pelo coordenador como exemplo.** **Por que é arriscado apagar:** se algum terceiro efetivamente clonou/baixou/recebeu esses assets sob CC-BY-SA enquanto essa era a licença vigente (a era anterior do projeto, antes deste repo zerado), apagar o texto que reconhece essa concessão não desfaria o direito adquirido de quem recebeu, mas destruiria o **registro** de que a concessão existiu - o tipo exato de dano que a exceção de L-24 existe para prevenir. **Confirmação parcial de que o cenário é real, não hipotético:** o `AI-DISCLOSURE.md` e o histórico do próprio projeto (fato dado na tarefa: git zerado, mas os assets do corpus continuam em `resources/`/`assets/` com datas de modificação anteriores a 2026-07-31) tornam plausível que houve, sim, distribuição anterior via outro canal (ex.: repositório antigo, per memória global sobre migração Codeberg→GitHub em outros projetos do líder).

**Ação proposta:** não tocar. Se o líder quiser reorganizar este documento, a Zona 1 precisa ser preservada literalmente ou substituída por texto jurídico equivalente redigido por ele/advogado, nunca apagada.

### D.2 - `ASSETS-LICENSE.md:29-35` - histórico de licença de código (GPLv3/AGPL-3.0 retroativos)

Texto exato:

> "**Histórico:** o código foi GPLv3 até 2026-07-31, e AGPL-3.0 antes disso. Fontes C# (`.cs`) e scripts GDScript (`.gd`) da era Godot não existem mais no repositório atual [...] mas **continuavam cobertos pela GPLv3 enquanto existiram**; esta seção não remove nem enfraquece aquela cobertura retroativa. A rotação para Apache License 2.0 (ADR-021) não retroage: releases já publicadas sob GPLv3/AGPL-3.0 permanecem naquela licença para quem já as recebeu."

**Por que é arriscado apagar:** mesmo raciocínio de D.1 - se código sob GPLv3/AGPL-3.0 já foi distribuído (era anterior), essa distribuição gerou direito para quem recebeu, independente de qualquer mudança de licença posterior (inclusive a mudança que o Grupo A.6 propõe, de Apache para AGPL-3.0-or-later). **Nuance:** a PARTE que diz "A rotação para Apache License 2.0" ficará desatualizada assim que A.6 for aplicado (não vai mais existir rotação para Apache, e sim para AGPL direto) - mas a cláusula de fundo (releases antigas mantêm a licença que tinham) precisa sobreviver reescrita, não apagada.

**Ação proposta:** não apagar; reescrever para refletir a nova cadeia de licenças (GPLv3/AGPL-3.0 antiga → não mais "Apache" → AGPL-3.0-or-later nova) preservando a garantia retroativa para quem já recebeu.

### D.3 - `ASSETS-LICENSE.md:93-100` - Zona 3, livros-companheiros, direitos reservados

Texto exato:

> "## Zona 3: livros-companheiros = direitos reservados (inalterado)
>
> Os **dois livros-companheiros** NÃO entram no CC-BY-SA, em nenhuma zona. São obra literária à parte, com **todos os direitos reservados** ao autor (petrinhu, 2026), desde antes desta reorganização."

**Por que é arriscado apagar:** esta é a base jurídica do próprio plano do líder citado em `inicial.md` ("o lore deve ter outra licença e se tornará livro para uso comercial") - apagar enfraqueceria a reserva de direitos sobre a obra que ele pretende comercializar. Mesmo não sendo uma concessão a terceiro (é reserva, não licença), é texto que sustenta um direito comercial futuro do próprio líder, e removê-lo sem necessidade é dano evitável.

**Ação proposta:** não tocar.

### D.4 - `NOTICE:7-10` - carve-out de marca

Texto exato:

> "Trademark notice: this license covers the source code only. It does not grant any right to the GusWorld name, its logo, or its character names as identifiers of origin. Forks and redistributions must present themselves under a different name."

**Por que é arriscado apagar:** independentemente de qual licença de código vigora (Apache hoje, AGPL-3.0-or-later amanhã, ver A.6), este parágrafo é o que impede um fork de se apresentar como o produto oficial usando o nome "GusWorld" - apagá-lo sem reescrever equivalente destrava esse risco imediatamente, mesmo durante a janela entre apagar o texto antigo e escrever o novo.

**Ação proposta:** não apagar sem já ter a frase de substituição pronta (troca simultânea, nunca lacuna).

### D.5 - `AI-DISCLOSURE.md` (arquivo inteiro, não inspecionado linha a linha nesta rodada)

**Nota de escopo [INFERÊNCIA]:** não encontrei, nas buscas feitas, nenhuma menção a licença/Apache/AGPL dentro deste arquivo - ou seja, não localizei um candidato específico de Grupo A dentro dele. Mas, dado que o nome do arquivo sinaliza divulgação de uso de IA (relevante para proveniência de direito autoral de asset gerado por IA, mencionado em `ASSETS-LICENSE.md:63`), **classifico o arquivo inteiro como zona de cautela**: qualquer edição futura nele deveria passar pela mesma pergunta "isto afeta direito adquirido de terceiro que já recebeu asset com esta proveniência declarada?" antes de qualquer corte. Não é um candidato concreto agora, é um aviso de fronteira.

**Ação proposta:** nenhuma ação agora; nenhuma edição neste arquivo sem re-checar proveniência de asset primeiro.

---

## Grupo E - Candidatos que dependem de decisão do líder ainda não tomada

### E.1 - `docs/design/mecanicas/core-loop-exploracao.md:95` - decisão DA-1, perspectiva de câmera

Já citado em A.8 (a palavra "orbital" já morreu). O que falta decidir, listado em `GODS_LAWS.md:392` (pendências de lei): "**Perspectiva e câmera do overworld**: 3/4 fixa, top-down, ou outra." Enquanto isso, a linha DA-1 fica com um buraco (perspectiva 3/4 fixa? livre-mas-não-orbital? top-down?) que nenhum agente deve preencher sozinho.

**Decisão que falta:** o líder escolher entre (pelo menos) 3/4 fixa, top-down puro, ou outra variante compatível com 2D pixel-art.

**Ação proposta:** não implementar nada de câmera/mapa/exploração até esta decisão sair (a própria `L-13` já bloqueia isso explicitamente).

### E.2 - Statline de carta: código (AGPL) ou asset (direitos reservados)?

`GODS_LAWS.md:382,391` registra a pendência: "o Cláudio (CLO) responde se statline de carta é código sob AGPL ou asset sob direitos reservados, porque compilar conteúdo dentro do executável mistura os dois regimes." Isso afeta diretamente como `docs/design/mecanicas/cartas-comuns-statlines.md`, `cartas-numeros-proposta.md` e o catálogo de cartas em geral devem ser licenciados quando compilados no binário (L-25, fase 2).

**Decisão que falta:** parecer jurídico do CLO + decisão do líder.

**Ação proposta:** nenhuma edição de licença nos documentos de statline até o parecer sair.

### E.3 - Mecanismo de proteção anti-adulteração (save/config/mapa)

`GODS_LAWS.md:261,393` registra: "as opções de mecanismo estão sendo levantadas pelo Narciso (CISO) [...] A decisão é dele [do líder]." Isso significa que qualquer detalhe técnico específico de criptografia/formato de envelope que apareça em documentos de design **além** do que já está fixado em L-25 (que já é lei) é, por definição, ainda não decidido.

**Ação proposta:** nenhuma ação sobre isto além do que A.10/A.11/A.12 já cobrem (formato de texto identificado como incompatível); o desenho fino do envelope binário aguarda o CISO.

### E.4 - UI em HTML/RML/RCSS: o que se escreve agora

`GODS_LAWS.md:394` registra a pendência textualmente: "**UI em HTML, RML e RCSS** (item 21): o que se escreve agora, sabendo que nenhum arquivo desses existe hoje no repositório." Confirma o achado do meu relatório anterior (Seção 6): zero arquivos `.rml`/`.rcss`/`.css` no corpus.

**Decisão que falta:** o líder decidir o que (se algo) começa a ser escrito nesta camada agora, dado que `L-06` já manda que "a camada que desenha só nasce quando o GlintFx tiver janela, contexto gráfico, entrada e texto."

**Ação proposta:** nenhuma criação de arquivo de UI até esta decisão.

### E.5 - Reparo das 28 ocorrências de `__DEP_REMOVIDA__`

`GODS_LAWS.md:395` já registra esta pendência nominalmente: "agente propõe caso a caso, o líder aprova antes de qualquer edição." Isto é **mais amplo** que o item A.9 deste relatório (que cobre as 17 ocorrências da frase específica "C++20 + `__DEP_REMOVIDA__`"): existem outras 9 ocorrências da string sozinha em contextos variados (`__DEP_REMOVIDA__Renderer`, `__DEP_REMOVIDA__VIDEODRIVER=dummy`, e as 3 corrupções de texto não-técnicas já identificadas no meu relatório anterior - "Sto`__DEP_REMOVIDA__`ight Archive", "www.`__DEP_REMOVIDA__`amer.com", "core::`__DEP_REMOVIDA__`").

**Ação proposta:** não proponho reparo aqui (a lei já pede que o agente que for reparar proponha caso a caso); registro que as 3 corrupções não-técnicas (Sanderson, pcgamer, Rust `core::fmt`) precisam de reparo textual simples (restaurar a palavra original), diferente das ocorrências técnicas (que dependem de decisão sobre nome de dependência).

### E.6 - Numeração de ADR: qual citação órfã vira ADR novo

Já coberto em C.2. `GODS_LAWS.md:396`: "Numeração de ADR recomeça em ADR-001 neste projeto; o destino das citações órfãs do corpus é decisão do líder, caso a caso."

**Ação proposta:** nenhuma, além de ter fornecido a lista completa com contagem em C.2 para facilitar a decisão caso a caso.

---

## Fim do relatório

Nenhuma edição, remoção ou criação de arquivo foi feita no projeto GusWorld durante este levantamento. Todas as ações descritas acima são **propostas**, não execuções.

**Arquivo do relatório (caminho absoluto):**
`/var/tmp/builds/claude-1000/-home-petrus-IDrive-Documentos-projetos-claudebrain-Projects-GusWorld/ead63a0b-09c1-4057-9d86-648ceaf458c8/scratchpad/candidatos-revogacao.md`