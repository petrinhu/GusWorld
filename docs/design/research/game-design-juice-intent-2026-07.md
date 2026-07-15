# Intent/telegraph + Juice em camadas — mergulho implementação-level (2026-07)

> Pesquisa web (WebSearch + WebFetch), não decisão. Aprofunda os 2 achados de maior alavancagem BARATA da varredura ampla (`docs/design/research/game-design-refs-2026-07.md`, itens #1 e #2 do resumo executivo). Aqui o foco é **COMO construir**, não "o que é" — dado técnico, não só conceito. Trechos marcados **[opinião]** são interpretação minha aplicando ao GusWorld; o resto tem URL aberta nesta sessão.
>
> Contexto verificado no código antes de escrever: o motor **já tem** o contrato de telegraph (`GusEngine/domain/include/gus/domain/combat/enemy_brain.hpp` + `combat_records.hpp`) e **já usa** RmlUi via glintfx (`GusEngine/app/`). As recomendações abaixo casam com o que existe, não propõem reescrever nada.

---

## TEMA 1 — Enemy intent / telegraph: como estruturar (dado + UI)

### (A) O padrão técnico

**Slay the Spire — intent como número exato, recalculado ao vivo, com estado "desconhecido" explícito.**

- O sistema não nasceu como número exato: a versão inicial mostrava só faixas de dano por tipo de arma. Em playtesting, "os testadores acharam mais engajador ter os números disponíveis, não precisar decorar o que cada símbolo significa, e conseguiam criar novas estratégias" — por isso o jogo final mostra dano exato, agrupado em 7 faixas visuais (0-4 até 30+) só para a ARTE do ícone (o número embaixo é exato). Fonte: [Intent — Slay the Spire Wiki](https://slaythespire.wiki.gg/wiki/Intent).
- **O intent é uma projeção recalculada a cada leitura, não um valor congelado no início do turno inimigo.** Vulnerable no jogador e Weak no inimigo **mudam o número exibido em tempo real**: "o dano mostrado no intent é ajustado por debuffs como Vulnerable no jogador e Weak no inimigo" — se o jogador aplica Vulnerable DEPOIS que o inimigo já "decidiu" atacar, o número na tela sobe imediatamente. Fonte: [Intent — Slay the Spire Wiki](https://slaythespire.wiki.gg/wiki/Intent).
- Existe um ícone dedicado de **incerteza declarada**: "Unknown" cobre ações que não podem ser previstas (ataque dividido, explosão, invocação, renascimento) — e há um relic (Runic Dome) que força TODOS os intents pra "Unknown" em troca de vantagem, ou seja, esconder intent é tratado como trade-off de jogo, não bug. Fonte: [Intent — Slay the Spire Wiki](https://slaythespire.wiki.gg/wiki/Intent).

**Into the Breach — "perfect information" via sistema de 2 turnos + alvo geométrico, não só número.**

- O inimigo (Vek) tem um turno de **posicionamento** (se move pro tile de onde vai atacar e mostra a trilha/área que será atingida) separado do turno de **execução** (ataca de fato no turno seguinte). O que é telegrafado não é só "quanto dano" — é **QUAL TILE/ÁREA será atingida**, com trilha de ataque + caixa vermelha sobre o alvo. Fonte: [Attacks — Into the Breach Wiki](https://intothebreach.fandom.com/wiki/Attacks), [Vek — Into the Breach Wiki](https://intothebreach.fandom.com/wiki/Vek).
- Consequência direta pro dado: como o alvo é geométrico (tile), **empurrar o inimigo, matá-lo antes do turno dele, ou mover a unidade ameaçada pra fora do tile invalida o ataque telegrafado** — ou seja, o telegraph não é "isso vai acontecer", é "isso vai acontecer SE nada mudar", e a interação do jogador nesse intervalo é o próprio jogo. GDC postmortem confirma a intenção de design: "quando todo ataque inimigo é telegrafado e não há chance aleatória nas opções de ataque, o jogo começa a parecer um puzzle". Fonte: [GDC Vault — Into the Breach Design Postmortem](https://www.gdcvault.com/play/1025772/-Into-the-Breach-Design), [Road to the IGF: Subset Games' Into the Breach — Game Developer](https://www.gamedeveloper.com/game-platforms/road-to-the-igf-subset-games-i-into-the-breach-i-).

**Griftlands — o mesmo vocabulário visual reaproveitado em 2 modos de jogo (combate físico + negociação verbal).**

- O "anel interno" do HUD de oponente lista os intents e permite pré-visualizar a ação futura: "o anel interno menor lista todos os intents e permite prever ações futuras", mostrando qual argumento/alvo será atacado, quanto dano, e quando o inimigo vai "implantar" um argumento (delay de N turnos, não só "próximo turno"). Fonte: [Negotiation — Griftlands Wiki](https://griftlands.fandom.com/wiki/Negotiation).
- Intent pode ser escondido por habilidade especial do oponente (paralelo direto ao Runic Dome de Slay the Spire) — confirma que "esconder intent" é um padrão recorrente de trade-off, não uma exceção de um jogo só. Fonte: mesma acima.

**Padrão geral de UI de fila de turno (tactics genérico) — relevante pro Gambito reordenar iniciativa.**

- A forma mais comum é uma fileira/pilha de retratos mostrando os PRÓXIMOS turnos de todos os combatentes (Fell Seal: linha de retratos no topo; Alchemist Code: 5 próximos turnos no canto superior esquerdo). Fonte: [Visual Initiative Queue — TV Tropes](https://tvtropes.org/pmwiki/pmwiki.php/Main/VisualInitiativeQueue).
- **Achado mais relevante pro Gambito:** "alguns jogos deixam o jogador PRÉ-VISUALIZAR como a ordem de turno vai mudar — se você seleciona uma ação mas ainda não confirmou, a fila muda de ordem pra mostrar o que vai acontecer" antes de comprometer a jogada. Fonte: [Visual Initiative Queue — TV Tropes](https://tvtropes.org/pmwiki/pmwiki.php/Main/VisualInitiativeQueue).

### (B) → GusWorld (como construir barato)

O motor **já modela isso**. `GusEngine/domain/include/gus/domain/combat/enemy_brain.hpp` declara:

```cpp
// Telegraph obrigatorio: o que o inimigo pretende fazer no proximo turno.
virtual IntentPreview preview_intent(const CombatState& state, const CombatActor& self) = 0;
```

E `combat_records.hpp` já tem o dado:

```cpp
struct IntentPreview {
    std::string actor_id;
    std::string predicted_action_id;
    int predicted_damage = 0;
    TargetShape predicted_shape = TargetShape::Single;   // Self|Single|Linha|Area3x3|Grupo
    std::string predicted_target_id;
    bool is_chaotic = false;  // true => Gambito-Prever retorna ruido
};
```

Isso já é **exatamente** o padrão Slay the Spire (número exato) + Into the Breach (`predicted_shape`/`predicted_target_id` = geometria do alvo, não só dano) + o ícone "Unknown" (`is_chaotic`). O comentário no header ("TODA AI é obrigada a expor IntentPreview... um inimigo sem intent legível quebraria o pillar de informação") já é a mesma filosofia de "perfect information" do Into the Breach. **Passos concretos:**

1. **Confirmar que `preview_intent` é chamado a cada leitura da tela, não cacheado.** A assinatura já recebe `const CombatState&` por parâmetro — isso é o padrão certo (recálculo ao vivo, como Vulnerable/Weak mudando o número do Slay the Spire). Se algum ponto do `battle_scene.cpp` guardar o `IntentPreview` num campo persistente sem invalidar quando o estado muda (ex.: jogador aplica debuff no inimigo ANTES de agir), isso quebraria a paridade com o padrão — vale um teste específico: "aplicar debuff que reduz dano do inimigo → o número do intent na tela cai no mesmo frame, sem precisar passar de turno". **[opinião]** isso é o teste automatizado mais valioso pra fechar o TEMA 1.
2. **Ícone + número, RmlUi-nativo.** Um `<span>`/elemento por intent, com classe RCSS por `predicted_action_id`/categoria (ataque/buff/debuff/passar) — cor consistente por categoria é tema comum aos dois temas desta pesquisa (ver TEMA 2-B). O número (`predicted_damage`) via data-binding (`bind_number`, já documentado na API do glintfx).
3. **`is_chaotic = true` → ícone "?"**, replicando 1:1 o "Unknown" do Slay the Spire — zero trabalho de design novo, só decisão de asset (1 glifo extra, reaproveitável do alfabeto cifrado do jogo).
4. **`predicted_shape` → destacar os alvos, não só mostrar 1 ícone.** Into the Breach mostra o TILE/área atingida, não só "X de dano". Pro GusWorld (grid de combate por cartas, não tile-based livre): `Linha`/`Area3x3`/`Grupo` devem highlightar visualmente os personagens/posições afetados (borda RCSS nos slots-alvo), não só aparecer como texto — isso é o que torna o intent "perfect information" de verdade (o jogador vê ONDE vai doer, não só QUANTO).
5. **Gambito = a mecânica de "pré-visualizar reordenação de turno" que já é padrão de tactics genérico** (TV Tropes acima) — o campo `reorder_delta` já existe em `CombatAction` (`combat_records.hpp:136`). Sugiro (não decisão, só leitura de padrão): ao jogador armar uma ação de Gambito (antes de confirmar), a UI de fila de turno mostra a ordem PROJETADA pós-reorder ao lado da ordem atual, do jeito que "alguns jogos deixam pré-visualizar a fila mudando antes de confirmar" — reaproveita o mesmo layout de fila-de-retratos que qualquer UI de turn-order tactics já teria, só adiciona o estado "prévia".
6. **Griftlands valida um ponto de reuso:** o mesmo componente de intent (ícone+número+alvo) pode servir tanto pro combate quanto pra qualquer futura mecânica social/negociação do GusWorld sem reinventar layout — não é decisão pra agora, é nota pra arquitetura de componente reutilizável.

### (C) Veredito de custo — TEMA 1

**BARATO.** O dado já existe no domínio (`IntentPreview`), o contrato já obriga toda IA a expô-lo, e o `battle_scene.hpp` já tem `intent_for(...)` retornando `std::optional<IntentPreview>` (comentário: "a cena mostra o ícone sobre o inimigo"). O trabalho que falta é: (a) confirmar recálculo ao vivo (teste, não código novo), (b) bind RmlUi de ícone+número+cor por categoria (RCSS + data model, técnica padrão já em uso no projeto para outras telas), (c) highlight de alvo por `TargetShape` (RCSS de borda em slot, sem asset 3D), (d) prévia de reorder de fila pro Gambito (reaproveita `reorder_delta` já existente). Nenhum item exige asset novo, shader novo, ou pedido ao dev do glintfx.

---

## TEMA 2 — "Juice" em camadas sem VFX 3D: a receita decomposta

### (A) O padrão técnico

**Screen shake — fórmula real, não "chacoalhar aleatório".**

- Squirrel Eiserloh (GDC 2016, "Juicing Your Cameras With Math") formaliza um sistema de **trauma**: um valor em `[0,1]` que sobe em eventos ("+= 0.2 ou 0.5" por evento) e decai linearmente com o tempo; a intensidade de shake exibida é `trauma²` ou `trauma³` (não linear) — "trauma 0.30/0.60/0.90 vira 3%/22%/73% de shake" com o expoente cúbico, o que faz eventos pequenos quase não tremerem e eventos grandes tremerem desproporcionalmente. A implementação usa **ruído Perlin/simplex por eixo**, não `random()` puro: "ruído suavizado é bem melhor que aleatório pra screen shake" porque funciona automaticamente com pause/slow-motion e é reproduzível em replay. Fonte: [GDC 2016: Squirrel Eiserloh — Juicing Your Cameras With Math (transcrição)](https://archive.org/stream/GDC2016Eiserloh/GDC2016-Eiserloh_djvu.txt).
- Jan Willem Nijman (Vlambeer, "The Art of Screenshake", INDIGO Classes 2013) demonstrou ao vivo que uma sequência de mudanças PEQUENAS e independentes (som, animação de hit, knockback, screen shake, partícula) empilhadas transforma um clone genérico de shooter em algo "satisfatório" — a apresentação é citada como referência canônica do conceito de "juice" na indústria. Fonte: [Jan Willem Nijman — Vlambeer — "The art of screenshake" (YouTube)](https://www.youtube.com/watch?v=AJdEqssNZ-U).

**Hit-pause / freeze-frame — janela de tempo, não efeito visual.**

- O termo genérico é **hitstop**/**hitlag**: "no frame exato do impacto, o jogo para o tempo por algumas dezenas de milissegundos, depois retoma" — jogos de luta (Street Fighter) e Smash Bros usam isso pra "vender" a colisão como algo que custou energia. Faixa efetiva citada: **60–90ms** pra feedback de impacto perceptível sem quebrar o ritmo. Implementação típica: variável global de "gamespeed"/timescale que TODAS as instâncias móveis/timers checam antes de avançar — a pausa de impacto vira "zerar ou quase-zerar essa variável por N ms". Fonte: [Game feel on the web: squash, shake, and the art of juice](https://valdemird.com/blog/game-feel-on-the-web/) via busca (ver nota de cautela abaixo), [Hit Freeze Frames — GameMaker Community](https://forum.gamemaker.io/index.php?threads/hit-freeze-frames.57894/).

**Squash-and-stretch / anticipation — dos 12 princípios de animação aplicados a jogos.**

- Squash-and-stretch "injeta personalidade e energia em cada ação, fazendo todo movimento parecer intencional"; anticipation (recuo antes da ação) tem uma armadilha específica em jogos: "colocar anticipation no personagem JOGÁVEL arrisca criar input lag" (o jogador aperta o botão e o personagem "recua" antes de agir de verdade) — por isso anticipation costuma ser aplicado a elementos NÃO controlados por input direto (inimigos, UI, resultado de uma ação já confirmada), não à resposta imediata ao clique do jogador. Fonte: [12 Principles for Game Animation — Chris Totten (Medium)](https://totter87.medium.com/12-principles-for-game-animation-a9137ef44345), [The 12 Principles Of Animation (In Video Games) — Game Anim](https://www.gameanim.com/2019/05/15/the-12-principles-of-animation-in-video-games/).

**Balatro — a estrutura de "camadas empilhadas", com nota de cautela sobre números exatos.**

- O princípio central, citável com confiança (é uma tese de design, não medição de código): "remova UMA camada e o jogo ainda funciona; remova TODAS e você tem uma planilha" — a "juice" empilha MULTIPLICATIVAMENTE (cada canal reforça os outros: animação de carta, contador de número rolando, shake escalado por magnitude, partícula, áudio com pitch crescente). A ordem sequencial de ativação de Jokers (um de cada vez, da esquerda pra direita, com callout visual) é o mecanismo que "substitui páginas de documentação por 300ms de animação sequencial" — o jogador entende a CADEIA causal (A ativou, que ativou B, que ativou C) por delay visual, não por texto. Fonte: [Balatro: Juicy Feedback in a Poker Roguelike — Blake Crosley](https://blakecrosley.com/guides/design/balatro).
  - **Nota de cautela [opinião]:** a mesma fonte, quando perguntada por valores EXATOS de implementação (ms de cada fase, hex de cor, curva cubic-bezier), devolveu números com precisão que Balatro (jogo de código fechado) não teria como confirmar publicamente — trato esses números específicos como **ilustrativos do padrão** (a ordem dos canais, a existência de cores fixas por categoria, a existência de delay entre gatilhos), não como fato verificado linha a linha do código-fonte do jogo. Não uso nenhum desses números específicos nas recomendações abaixo.

**Contagem numérica animada ("tweening" de número) — 2 caminhos técnicos, e nenhum é nativo do RmlUi.**

- Caminho CSS puro (web): usa `@property` pra declarar uma custom property numérica interpolável, combinada com CSS `counter` — funciona só em navegadores Chromium recentes (Firefox/Safari têm suporte parcial/nenhum). Fonte: [Animating Number Counters — CSS-Tricks](https://css-tricks.com/animating-number-counters/).
- Caminho JavaScript (universal): um loop (`requestAnimationFrame` ou equivalente) recalcula o valor exibido a cada frame entre início e fim, com easing, e escreve o texto do elemento diretamente. Fonte: [Number Countup Animation With Vanilla JavaScript — CSS Script](https://www.cssscript.com/number-countup-animation-counter/).
- **RmlUi não tem `@property`/CSS counters** (é um subconjunto de CSS, não um browser) — então o caminho aplicável ao GusWorld é o equivalente ao caminho JS: o HOST (C++) escreve o número interpolado a cada frame via data-binding, não a UI sozinha.

**Screen shake em CSS/UI (não câmera 3D) — GPU-only, sem reflow.**

- Técnica padrão: `transform: translate()` dentro de `@keyframes`, alternando deslocamentos pequenos em % do tempo da animação (10%, 20%, 30%...); usar `transform` em vez de mudar `top`/`left` evita reflow/repaint de layout porque é uma propriedade acelerada por GPU/composição, não uma propriedade de fluxo de documento. Fonte: [CSS Shake Animation — UnusedCSS](https://unused-css.com/blog/css-shake-animation/), ["Shake" CSS Keyframe Animation — CSS-Tricks](https://css-tricks.com/snippets/css/shake-css-keyframe-animation/).

**RmlUi/RCSS — o que já é suportado nativamente (fonte primária, doc oficial).**

Extração de [Animations, transitions, and transforms — RmlUi Documentation](https://mikke89.github.io/RmlUiDoc/pages/rcss/animations_transitions_transforms.html):

- **`@keyframes`** com sintaxe idêntica a CSS (`0%`/`from`/`to`).
- **`animation: <duração>s <delay>s? <easing>? <iterações>? alternate? paused? <nome>`** — roda continuamente, pode ter `infinite`.
- **`transition: [<propriedades>|all] <duração>s <delay>s? <easing>?`** — dispara só quando uma CLASSE ou pseudo-classe (`:hover`, `:focus`) é adicionada/removida do elemento (não em toda mudança de propriedade solta).
- **`transform`** com `translate`/`translate3d`, `scale`/`scale3d`, `rotate`/`rotate3d`, `skew`, `matrix`, `perspective` — com `transform-origin` configurável. RmlUi declara **"suporte total de interpolação para transforms"**, ou seja, animar `transform` é um caso de primeira classe, não workaround.
- **11 famílias de easing nativas**, cada uma com variante `-in`/`-out`/`-in-out`: `back, bounce, circular, cubic, elastic, exponential, linear, quadratic, quartic, quintic, sine`. `back-out` e `elastic-out` cobrem exatamente o "pop" de squash-and-stretch sem precisar de curva custom.
- **O que É animável:** números, comprimentos, porcentagens, ângulos, cores, keywords, transforms, **decorators**, e **filters**.
- **O que NÃO é animável:** `box-shadow` — a doc é explícita: "Box shadows do not currently support being animated." Isso é uma limitação real a desviar (ver B abaixo).
- Gotcha já registrado em memória do projeto (`reference_glintfx_api.md`): **não existe `text-shadow`** — glow só funciona em CAIXAS (via `filter: drop-shadow()`, que segue o alpha shape renderizado, não a bounding box), nunca em texto puro sem wrapper.

### (B) → GusWorld (como construir barato)

Mapeamento explícito **RmlUi-nativo vs. pedido-glintfx**, por componente de juice:

| Componente de juice | Como construir | RmlUi-nativo ou pedido-glintfx? |
|---|---|---|
| Cor consistente por categoria (dano/cura/buff/debuff/crítico) | Classe RCSS por categoria, cor fixa no stylesheet | **RmlUi-nativo** (é CSS puro, já suportado) |
| Contagem numérica animada (HP/dano subindo ou descendo) | Host C++ interpola o valor por N frames e escreve via `bind_number`/`set_number` (mesmo padrão já documentado no `reference_glintfx_api.md` para outros data models) | **RmlUi-nativo** — é lógica do HOST, não feature nova da lib; a API de data-binding já existe |
| Screen shake (do painel/log, não câmera 3D) | `@keyframes` com `transform: translate()` alternando offsets pequenos, disparado por `add_class("shake")` via `set_property`/`add_class` (API v0.9.0 já existe) e removido após a duração | **RmlUi-nativo** — `transform` tem "suporte total de interpolação" pela doc oficial |
| Hit-pause / freeze-frame (60-90ms) | **NÃO é UI** — é pausar a SIMULAÇÃO de combate (o game loop C++) por N ms no frame de impacto, mantendo o RENDER da UI rodando (senão a tela trava de vez, o que é diferente de hitstop) | **Fora do RmlUi inteiramente** — é lógica de `combat_state_machine`/loop de app, zero dependência de glintfx |
| Pop/squash-stretch em elemento (crítico, combo) | `transform: scale()` com easing `back-out` ou `elastic-out` (nativos!) disparado por toggle de classe | **RmlUi-nativo** — as curvas de easing certas já existem prontas, não precisa aproximar |
| Sequência causal A→B→C (combo de cartas, Joker-style) | `animation-delay` crescente por elemento (0ms, staggered), ou `transition-delay` por classe aplicada em sequência pelo host | **RmlUi-nativo** — `@keyframes` + `delay` cobre isso sem JS/lógica extra |
| Glow/destaque em número crítico | `filter: drop-shadow(cor x y blur)` **em um elemento-caixa** (span com padding), NUNCA em texto solto | **RmlUi-nativo, COM RESSALVA:** lembrar o gotcha "sem text-shadow" — precisa envolver o número num elemento-caixa, não é um bug a reportar, é um padrão de markup a seguir |
| Glow/destaque animado (pulsar) | Mesma técnica acima + `animation` no `filter`, já que "filters" está na lista de propriedades animáveis | **RmlUi-nativo** |
| Sombra "encaixotada" de elemento (não glow) | `box-shadow` **estático** funciona; mas **não pode animar** `box-shadow` (limitação documentada) — se precisar animar sombra, trocar por `filter: drop-shadow()` (que É animável) | **RmlUi-nativo com workaround conhecido** (trocar de propriedade, não pedido a dev) |
| Entrada/saída de linha do log-terminal | `transition` de `opacity`+`transform:translateY()` disparado por classe (`.entering`/`.entered`) | **RmlUi-nativo** |

**Conclusão do mapeamento: nenhum componente de juice do TEMA 2 exige pedido ao dev do glintfx pra este slice.** Toda a superfície necessária (`transform`, `filter`, `transition`, `animation`, `add_class`/`remove_class`, `set_number`) já está na v0.9.2 pinada (`GusEngine/CMakeLists.txt`), confirmada em `reference_glintfx_api.md`. **[opinião]** O único cenário que justificaria um pedido futuro é performance em máquina fraca com MUITAS animações simultâneas (ex.: 6 companions + inimigos todos "pulsando" ao mesmo tempo) — isso é pergunta de profiling empírico pós-implementação, não uma lacuna de feature conhecida hoje.

**Como isso casa com o "terminal" diegético:**

- Cada linha do log = 1 elemento com: cor por categoria (RCSS class), número tweenado (host interpola), entrada com transition translateY+opacity, e — se for resultado de combo — delay em cascata por linha (igual ao Joker sequencial do Balatro, mas aplicado a texto de terminal em vez de card 3D).
- Hit-pause fica FORA da UI: é o loop de combate pausando brevemente antes de a próxima linha do log aparecer — dá o "peso" do impacto sem precisar de asset.
- Screen shake, se usado, deve ser escopado ao painel/container de combate (não a tela inteira) pra não quebrar a legibilidade do texto do terminal — usar a lógica de trauma/decay do Eiserloh (mesmo em 2D) evita que o shake pareça "batida seca" e sim tenha ataques fracos quase imperceptíveis e ataques fortes visivelmente mais violentos (curva `trauma²`/`trauma³`, não linear).

### (C) Veredito de custo — TEMA 2

**BARATO, quase inteiramente.** É a conclusão mais forte desta pesquisa: **cada item do checklist de juice mapeia 1:1 pra uma feature já documentada e já pinada do RmlUi/glintfx**, exceto o hit-pause (que não é UI, é lógica de loop C++, também barata). Não há necessidade de pedido de feature nova ao dev do glintfx para completar este slice — só disciplina de RCSS + um pouco de lógica de tween/hit-pause no lado app (`GusEngine/app/`).

---

## Checklist de implementação priorizado

Ordem por valor × custo × dependência (o que destrava o quê):

1. **Cor consistente por categoria** (RCSS class por tipo de ação/dano) — zero dependência, base de tudo que segue. BARATO.
2. **Ícone+número de intent + highlight de alvo por `TargetShape`** (TEMA 1) — dado já existe no domínio (`IntentPreview`), só falta bind RmlUi. BARATO. Depende de #1 pra cor por categoria do ícone.
3. **Confirmar/testar recálculo ao vivo de `preview_intent`** quando o estado muda no meio do turno do jogador (debuff aplicado antes de agir) — é teste, não feature nova, mas fecha uma lacuna de paridade com o padrão Slay the Spire/Into the Breach. BARATO.
4. **Contagem numérica animada** (HP/dano) via host tween + `set_number` — depende de #1/#2 já estarem no ar pra não duplicar trabalho de bind. BARATO.
5. **Entrada/saída de linha do log-terminal** (`transition` translateY+opacity) — independente, pode entrar em paralelo com #4. BARATO.
6. **Hit-pause (60-90ms)** no loop de combate no frame de impacto — mexe no `combat_state_machine`/app loop, não na UI; fazer depois que o log já reage visualmente (#5), senão a pausa não tem o que "vender". BARATO/MÉDIO (é lógica de timing no loop, precisa não travar input).
7. **Pop/squash-stretch em crítico/combo** (`scale` + `back-out`/`elastic-out`) — depende de #4/#5 pra não competir com outra animação no mesmo elemento no mesmo frame. BARATO.
8. **Sequência causal A→B→C com `animation-delay`** (combo de cartas) — natural extensão de #7, mesmo mecanismo de trigger. BARATO.
9. **Screen shake escopado ao painel de combate**, com curva trauma²/trauma³ (não linear) — deixar por último entre os itens de juice porque é o mais fácil de exagerar e o mais fácil de testar contra os outros já no ar. BARATO, mas exige calibração de "quanto é demais" (só se descobre testando com os itens 1-8 já funcionando).
10. **Gambito: prévia de reordenação de fila de turno** antes de confirmar a ação — reaproveita `reorder_delta` já existente em `CombatAction`; fazer por último porque depende da UI de fila-de-turno já existir com os intents (#2) visíveis, senão a prévia de reorder não tem contexto visual pra comparar antes/depois. MÉDIO (é a peça que mais precisa de decisão de design/UX, não só bind).

---

## Notas finais

- Todas as fontes com URL foram abertas via WebSearch/WebFetch nesta sessão (2026-07). A doc oficial do RmlUi (`mikke89.github.io/RmlUiDoc`) foi lida diretamente — é a fonte de maior confiança deste documento por ser a própria referência técnica da lib que o GusWorld usa.
- Uma fonte (Balatro/blakecrosley.com) devolveu números de implementação (ms exatos, hex de cor, cubic-bezier) que não são verificáveis contra código-fonte fechado — marcados explicitamente como **[opinião]**/ilustrativos, não usados nas recomendações concretas.
- Uma URL (Griftlands Fandom, tentativa de aprofundar o "anel interno") retornou erro HTTP 402 no WebFetch; a informação usada sobre Griftlands vem só do snippet de busca (WebSearch), não de leitura completa da página — sinalizado aqui por transparência.
- Este documento é **insumo de pesquisa**, não decisão de design/arquitetura. Toda aplicação concreta (o checklist final, qualquer mudança em `enemy_brain.hpp`/`combat_records.hpp`/RCSS novo) deve passar por `AskUserQuestion` com o criador antes de virar trabalho de implementação, conforme a regra canônica do projeto (`CLAUDE.md` — "Modo de trabalho com agentes").
