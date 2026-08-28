# Proposta de copy: os 2 stubs de programador do log de combate

**ID:** `COPY-STUBS-COMBATE`
**Status:** **PROPOSTA. NADA AQUI É CANON.** Nenhum catálogo de tradução e nenhum arquivo de código foi tocado.
**Autor:** narrative-designer (proposta). Aprovação item a item do líder.
**Data:** 2026-08-06.

> **Como aprovar:** cada opção tem um ID (1A, 1B, 1C, 2A, 2B, 2C). Escolha **uma por frase**, ou peça variante. Só depois da sua escolha alguém edita `resources/translations/pt_br.md`. O texto que o jogador lê é canon, então a decisão é sua.

---

## 1. O problema

Duas ações de combate ainda escrevem fala de programador na caixa de log que o jogador lê:

| Chave | Valor de hoje | Onde dispara |
|---|---|---|
| `COMBAT_LOG_AUTORESOLVE_UNAVAILABLE` | `[auto-resolve: a implementar]` | tela de abertura da batalha, quando o jogador aperta `[Q] Resolver sem encarar` num encontro de trash |
| `COMBAT_LOG_GAMBIT_UNAVAILABLE` | `GAMBITO: requer brain do alvo (telegraph entra no incr 5)` | quando o jogador escolhe o verbo GAMBITO e o motor ainda não tem o dado de comportamento do alvo |

A fiação já está pronta (fatia `I18N-UILOG`, commit `b902e751`): as duas chaves existem nos dois catálogos, o translator já resolve, e a troca é **uma linha por idioma**, sem tocar código.

**O que cada frase precisa comunicar:** a função não está disponível agora. Nada mais.

E o que ela **não pode** fazer é afirmar um estado do mundo que pode ser falso. Essa regra já custou uma correção no gêmeo do COMPILAR: a primeira redação dizia "nenhum Conjuro pronto no buffer", o que faria um jogador com cartas na mão concluir que o sistema de cartas está quebrado. **Frase de stub fala da FUNÇÃO, nunca do estado do mundo.**

---

## 2. Medição de largura (o número que limita a frase)

**Orçamento: 45 caracteres de corte duro. Recomendo escrever com teto de 42.**

### Como cheguei nesse número

O log tem **dois** consumidores. Medi os dois e mando o menor.

**Consumidor A, o HUD de produção (via glintfx). É este que manda.**

| Fato | Onde |
|---|---|
| `#log { font-size: 10dp; }` | `battle_cockpit_____.cpp:243` |
| `#log .ln { white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }` | idem, `:247-248` (a linha **não quebra**, ela é **cortada com reticências**) |
| `#cockpit { width: 252dp; padding: 10dp 12dp 0dp 12dp; }` | idem, `:74-78`. O comentário do próprio arquivo (`:236-237`) declara a largura útil como **~228dp** |
| avanço de glifo `= kMonoAdvanceRatio * px_size`, com `kMonoAdvanceRatio = 0.5f` (PixelOperatorMono é monoespaçada) | `text_metrics.hpp:32` |

Conta: `228dp / (0.5 * 10dp) = 228 / 5 = 45,6`, ou seja **45 caracteres** antes das reticências.

Se a caixa for content-box em vez de border-box (o arquivo não declara `box-sizing`, e o default do RCSS é content-box, o que daria 252dp úteis), o teto sobe para 50. **Fico com o número menor, 228dp, porque é o que o próprio arquivo declara**, e porque errar para menos só custa concisão.

**Ressalva honesta:** eu **não executei a fonte** para conferir o avanço real. O `0.5` é a constante que o repositório declara e usa no próprio `text_width`; o caminho do glintfx desenha por __DEP_REMOVIDA__ com a métrica real da fonte, que numa face monoespaçada deve bater com `0.5em`, mas isso não foi medido aqui. Se o avanço real for `0.55em`, o teto cai para cerca de 41. **Por isso recomendo escrever com teto de 42:** o texto sobrevive às duas leituras.

**Consumidor B, o desenho à mão (fallback, só quando `hud_external_` é falso).**

| Fato | Onde |
|---|---|
| painel do log: `x=188`, `w = 960 - 188 = 772` px lógicos | `battle_layout.cpp:168-173` mais `battle_layout.hpp:117-119` |
| `kLogTextPx = 11.0f` | `battle_scene_render.cpp:56` |
| texto começa em `x + pad(2) + 5` | `battle_scene_render.cpp:792` |

Conta: `(772 - 7) / (0.5 * 11) = 765 / 5,5 = 139` caracteres. **Não limita nada.**

### Achado da medição (não pedido, mas importante)

- A frase do GAMBITO de hoje tem **57 caracteres**. Ela **já estoura** os 45 e **já aparece cortada com reticências** no HUD de produção. O jogador de hoje provavelmente lê algo como `GAMBITO: requer brain do alvo (telegraph e...`. Ou seja: além de falar como programador, ela fala pela metade.
- A frase do auto-resolve tem **29 caracteres** e cabe.
- O HUD de produção mostra só as **2 últimas** entradas do log (`battle_preview.cpp:1453`, `scene_->log_lines(2)`), mais a now-line. Frase de stub some rápido: ela tem uma passagem só para ser entendida.

### Restrições de caractere (fonte)

A fonte do jogo é a **PixelOperatorMono, sem fallback**. O catálogo já registra um caso em que um símbolo virou tofu na tela (`pt_br.md:223-227`: o `⚠` não existe na fonte, virou caixa vazia, e a solução foi um `!` ASCII). Logo, nas propostas abaixo:

- **acentos sim** (vogais acentuadas, cedilha e til estão cobertos e já aparecem no catálogo);
- **nada de** `…` (reticência de um caractere só), `→`, `⚠`, aspas tipográficas ou travessão. Se precisar de reticência, são **três pontos** ASCII;
- **sem em-dash** (regra da casa) e **sem travessão em fala**.

---

## 3. ITEM 1: `COMBAT_LOG_AUTORESOLVE_UNAVAILABLE`

Contexto: dispara **na abertura da batalha**, quando o jogador aceita a oferta `[Q] Resolver sem encarar` (chave `COMBAT_INTRO_AUTORESOLVE`), que só aparece em encontro sem chefe. A frase precisa dizer que essa saída não está ligada ainda, sem prometer nem descrever como ela vai funcionar. O design do auto-resolve **não está canonizado**, então a copy não pode antecipar custo, penalidade nem regra.

### Opção 1A, voz do sistema (gêmea da linha do COMPILAR que você já aprovou)

- **pt-br:** `RESOLVER SEM ENCARAR: rotina offline.` **(37 caracteres)**
- **en-intl:** `RESOLVE WITHOUT FACING: routine offline.` (40)
- **Ângulo:** o sistema do mundo falando em registro de terminal, no mesmo molde `VERBO: <razão>` da linha aprovada do COMPILAR. Usa de propósito o **mesmo rótulo que o jogador acabou de ler no botão**, então ele liga uma coisa na outra sem esforço.
- **Evita:** `auto-resolve` (nome interno da função, não do mundo), `a implementar` (prazo de desenvolvedor), e os colchetes de stub.
- **Risco:** é a mais seca das três. Não tem sabor, tem clareza.

### Opção 1B, voz do hardware do Gus (Pillar 3)

- **pt-br:** `Tavus-Drive: sem rotina para isso ainda.` **(40 caracteres)**
- **en-intl:** `Tavus-Drive: no routine for that yet.` (37)
- **Ângulo:** quem responde é o **executor do triângulo de hardware** (Pillar 3: Óculos Táticos, Matriz Ortodôntica, Tavus-Drive). O aparelho é que não tem a rotina, e isso ancora a recusa num objeto que o jogador já conhece, em vez de numa caixa de erro abstrata.
- **Evita:** o mesmo da 1A, e ainda tira o texto do registro impessoal, que é onde jargão de dev costuma voltar a se esconder.
- **Risco:** `Tavus-Drive` é canon (pillars.md, Pillar 3), mas hoje **não aparece em nenhuma string de UI**. Esta opção seria a primeira vez que o nome do aparelho fala com o jogador pelo log. Isso é uma decisão de voz, sua.

### Opção 1C, voz do Gus

- **pt-br:** `Gus: sem atalho. Vai ter que ser na mão.` **(40 caracteres)**
- **en-intl:** `Gus: no shortcut. We do this by hand.` (37)
- **Ângulo:** o próprio Gus notando, em duas frases curtas (canon AHSD: a fala encurta enquanto a cabeça acelera). Há precedente de fala de personagem no combate: `COMBAT_DEFEAT_BARK` = `{0}: Opa. Reboot em 3, 2...`. Transforma uma limitação técnica em caracterização: não tem atalho, então a gente joga.
- **Evita:** todo o vocabulário de desenvolvimento, inclusive a palavra "rotina".
- **Risco:** **esta opção põe palavra na boca do Gus.** Pela regra da casa, tudo que define o Gus original passa por autorização explícita sua. Se você gostar do ângulo mas não da frase, o certo é você ditar a frase.

---

## 4. ITEM 2: `COMBAT_LOG_GAMBIT_UNAVAILABLE`

Contexto: o jogador escolheu GAMBITO e o motor ainda não tem o dado de comportamento do alvo. O verbo canônico é **prever** (Gambito-Prever), e a manifestação canônica são os **Óculos Táticos** projetando um **tabuleiro** com as peças e seus vetores previstos (`docs/design/mecanicas/puzzle-gambito.md` seção 1, e `core-loop-exploracao.md`).

**Armadilha desta frase especificamente:** existe uma mecânica planejada de **inimigo com intent caótico**, que é imprevisível **de propósito** (GDD 6.1). Então uma frase que diga "esse alvo não dá leitura" seria lida pelo jogador como **"este inimigo é do tipo imprevisível"**, que é uma afirmação falsa sobre o mundo, e pior: ensina errado uma mecânica real. As opções 2A e 2B falam da **função**; a 2C flerta com o estado do mundo e vai com o risco marcado.

### Opção 2A, voz do sistema (gêmea da linha do COMPILAR)

- **pt-br:** `GAMBITO: previsão offline.` **(26 caracteres)**
- **en-intl:** `GAMBIT: prediction offline.` (27)
- **Ângulo:** o sistema, no mesmo molde `VERBO: <razão>` do COMPILAR. Curtíssima, cabe com folga enorme, e sobrevive inteira mesmo se meu número de largura estiver errado por 30%.
- **Evita:** `brain` (nome de uma interface de código, `IEnemyBrain`), `telegraph` (jargão de design), `incr 5` (número de incremento interno). Os três eram vazamento direto da oficina para a tela.
- **Risco:** nenhum que eu veja. É a mais segura das seis propostas deste documento.

### Opção 2B, voz do hardware (Pillar 3)

- **pt-br:** `Óculos Táticos: o tabuleiro não sobe.` **(37 caracteres)**
- **en-intl:** `Tactical Goggles: the board will not load.` (42, exatamente no teto que recomendei)
- **Ângulo:** o vértice de **input** do triângulo respondendo. Usa **tabuleiro**, que já é a palavra canônica para o que o Gambito projeta (o mini-board holográfico), então a frase ensina o vocabulário do mundo em vez de gastar espaço explicando.
- **Evita:** o mesmo da 2A, e ainda amarra a ação ao aparelho que a executa (Pillar 3), reforçando a leitura de que gambito é recurso de hardware, não poder mágico difuso.
- **Risco:** dois. (a) Pode ser lida como "meus óculos estão com defeito", que é um estado do mundo, ainda que bem mais brando que "esse inimigo é ilegível". (b) O nome **em inglês** dos Óculos Táticos **não está canonizado** em lugar nenhum; `Tactical Goggles` é proposta minha, e a decisão do nome em inglês é sua, na passada de tradução.

### Opção 2C, voz do Gus

- **pt-br:** `Gus: previsão fora do ar. Jogo às cegas.` **(40 caracteres)**
- **en-intl:** `Gus: prediction is down. Playing blind.` (39)
- **Ângulo:** o Gus notando e já decidindo, em duas frases curtas. A primeira metade fala da **função** (previsão fora do ar), não do inimigo, o que desarma a armadilha do intent caótico. A segunda metade é caracterização pura: **jogar às cegas** é expressão de xadrez, que é exatamente como o canon descreve a cabeça dele (Pillar 4: resolve por xadrez e otimização, não por força).
- **Evita:** `brain`, `telegraph`, `incr 5`, e também a impessoalidade de caixa de erro.
- **Risco:** **põe palavra na boca do Gus** (mesma ressalva da 1C: precisa da sua autorização explícita, e se o ângulo agradar mas a frase não, a frase é sua).

---

## 5. O que eu recomendaria

**Recomendo o par 1A + 2A**, e explico por quê, inclusive contra o meu próprio gosto:

- **Consistência acima de sabor.** Já existe uma terceira frase da mesma família, já aprovada por você: `COMBAT_LOG_COMPILE_UNAVAILABLE` = `COMPILAR: módulo do compilador offline nesta build.`. As três são a mesma situação (verbo escolhido, função ainda não ligada). Se as três falarem no mesmo molde `VERBO: <razão>`, o jogador aprende o padrão numa vez e reconhece nas outras duas. Se cada uma falar de um jeito, ele lê três bugs diferentes.
- **Sobrevivem à incerteza da medida.** 26 e 37 caracteres passam folgados em qualquer leitura da largura.
- **Não gastam autorização sua com uma frase descartável.** Essas duas linhas **existem para morrer**: quando o auto-resolve e o Gambito-Prever forem implementados, elas somem do jogo. Põe-se a voz do Gus (1C ou 2C) em frase que fica, não em placa de "ainda não".

**Se você quiser sabor**, minha preferida pessoal é a **2C**, e ela é defensável: "jogo às cegas" é xadrez, e xadrez é o Gus. Nesse caso eu manteria **1A + 2C**, e não misturaria três vozes em duas linhas.

---

## 6. Item extra que eu NÃO fui pedido para mexer (decisão sua)

Achei o gêmeo das duas frases enquanto media. Registro e paro aqui.

`COMBAT_LOG_COMPILE_UNAVAILABLE` = `COMPILAR: módulo do compilador offline nesta build.` tem **51 caracteres**, ou seja, **também estoura os 45 e também está sendo cortado com reticências** no HUD de produção. E ele carrega **`build`**, que é palavra de oficina, não do mundo: o jogador não sabe o que é uma build.

Se você quiser, a poda mínima preserva a sua redação aprovada e só tira o rabo:

- **pt-br:** `COMPILAR: módulo do compilador offline.` (39)

**Não apliquei nada.** Essa linha já passou por você uma vez; mexer nela sem ordem seria eu desfazer aprovação sua.

---

## 7. Nota sobre a coluna en-intl

O catálogo `en_intl.md` estava com **valores vazios de propósito** até pós-v1.0.0 quando esta nota foi escrita. **Em 25/08/2026 o líder alterou o corte `C-09`** (`GODS_LAWS.md` L-29): o inglês passa a entrar **na própria 1.0**, e o cabeçalho de `en_intl.md` já reflete isso. A paridade de **chave** continua obrigatória; o que muda é o prazo do **valor**.

Logo, aprovar a versão em inglês aqui **não é mais** reservar a frase para depois: quem executar a troca preenche `en_intl.md` junto com `pt_br.md` (escopo `D19a`/`D19b` do `TODO.md`), e não deixa mais a chave em inglês vazia.

---

## 8. Termos do mundo que eu quase inventei e decidi não inventar

Registro porque termo novo é porta de mão única e passa por você.

1. **Nome próprio para o módulo de previsão** (do tipo "Preditor", "Núcleo Preditivo", "Módulo de Prognose"). Batizar o subsistema resolveria a frase com elegância, e criaria um substantivo canônico novo para sempre por causa de uma mensagem descartável. **Rejeitado.** Usei o verbo que já existe (GAMBITO) mais o substantivo comum "previsão".
2. **`Antena de Longo Alcance`.** Este **existe** (implante I2 da Matriz Ortodôntica, `economia.md`, dá +1 turno de Gambito-Prever). Era tentador escrever "fora do alcance da antena". **Rejeitado por mentir sobre o estado do jogo:** implicaria que a previsão depende de um implante que o jogador talvez nem tenha comprado, e faria a compra parecer obrigatória.
3. **`telemetria`.** Aparece no repositório, mas só em documento de QA, como termo de game-dev nosso. Não é palavra do mundo. **Rejeitado.**
4. **`buffer`, `daemon`, `handshake`, `payload`.** Todos soam plausíveis no mundo (magia é software), e todos seriam jargão de programador entrando pela porta dos fundos, que é exatamente o defeito que esta tarefa existe para corrigir. **Rejeitados.** Vale notar que "buffer" já tinha sido barrado uma vez, na correção anterior da linha do COMPILAR.
5. **A penalidade do auto-resolve.** O comentário no código menciona "penalidade por selo" como design futuro. Eu quase escrevi uma frase que insinuava esse custo. **Rejeitado:** o design do auto-resolve não está canonizado, e copy que insinua regra **cria** regra.
6. **`Tactical Goggles`** como nome em inglês dos Óculos Táticos (opção 2B). Não inventei em português, mas **em inglês não existe canon nenhum**, então qualquer coisa que eu escreva ali é invenção. Marquei na própria opção, em vez de deixar passar como se fosse tradução óbvia.
