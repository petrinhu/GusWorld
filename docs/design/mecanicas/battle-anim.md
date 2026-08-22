# Animacao de Combate (BattleScreen): Game-Feel e Timing (GusWorld)

**Status:** Canonico. Especificacao de game-feel da animacao de ataque ratificada pelo criador supremo em 2026-06-25 (3 tipos de ataque + hit-react + encaixe no pacing de 2 beats + tabela de sons + briefing de sprites de batalha). Spec de APRESENTACAO: o motor (`domain/combat/`) ja resolve dano/status (combat.md, fechado); este doc descreve COMO a animacao toca sobre o estado que o motor entrega, sem mexer no motor.

**Convencao de escrita:** pt-br. Termos de game-dev/animacao no original (idle, cast, melee, hit-react, knockback, anticipation, follow-through, hit-pause, hit-stop, screen-shake, windup, recovery, placeholder, floater, beat, sprite-sheet). Sem em-dash; usa hifen, virgula, parenteses, dois-pontos.

**Cross-ref:**
- [`battle-screen.md`](battle-screen.md) (layout side-view, §5.2 pacing de 2 beats por turno de inimigo, §3.4 sprites PixelLab, camera estatica D7).
- [`combat.md`](combat.md) (motor: FSM §3, fila CTB §4, 5 familias §6, formula de dano §11, canais FALHA/CRIT/COMUM).
- [`../art/vfx-combate-familias.md`](../art/vfx-combate-familias.md) (VFX 2D das 5 familias: o projetil/cast real da magia sai daqui depois do placeholder circular).
- [`locomotion.md`](locomotion.md) (sabor reusado: animacao data-driven, timing por evento, placeholder antes do sprite, sem flip Pillar 3).
- [`animation-plan.md`](animation-plan.md) (matriz de anims por papel + pipeline PixelLab; ESTE doc detalha o timing/game-feel das anims de combate que aquele plano lista).

---

## 1. O ator na arena = sprite animado, NAO o retrato

Decisao do criador (2026-06-25). Na arena side-view, cada combatente (party e inimigos) e um **sprite de corpo inteiro animado**, em pe, virado na direcao dos oponentes (idle de combate / battle-idle). O **retrato** vive SO no cockpit lateral (painel do ator ativo + celulas da fila CTB, battle-screen §2): retrato e HUD, sprite e ator de arena. Nao confundir os dois.

- **Orientacao sem flip (Pillar 3):** party a esquerda usa a pose de batalha virada pra DIREITA; inimigos a direita usam a pose virada pra ESQUERDA. Desenhos distintos, nunca espelhados (locomotion.md §1: o hardware assimetrico do personagem nao pode trocar de lado).
- **Tamanho fixo, pixel-perfect:** o sprite de arena respeita o slot de altura fixa `kActorSlotH` (battle-screen §5, D2/D3): sem escala dinamica por contagem de inimigos. Mini-boss ocupa mais por sprite-base maior, nao por escala.

### 1.1 Placeholder por ora: RETRATO como stand-in (transicao)

Enquanto os sprites de corpo animados nao existem, a arena testa com **RETRATOS como placeholder** (o asset que ja temos). E so andaime de teste, coerente com o sabor "placeholder antes do sprite" da locomocao (locomotion.md §6, retangulo piscante valida cadencia antes da arte). Quando os sprites de batalha entrarem (briefing §5 deste doc, pipeline PixelLab battle-screen §3.4), trocam o retrato sem mexer na logica de animacao: os GANCHOS de timing (beat de windup, instante de contato, hit-react) ja estarao validados com o placeholder.

---

## 2. Animacao de ataque por TIPO

Dois arquetipos de entrega, decididos pelo criador. O TIPO da carta/acao decide qual toca.

### 2.1 Magia (cast no lugar + projetil que viaja)

O atacante **conjura no PROPRIO lugar** (nao se desloca): toca a animacao de **cast** (windup de conjuracao) e a magia **VIAJA como projetil** ate o alvo. Feel de mago estacionario lancando feitico (Final Fantasy clássico).

- **Placeholder por ora:** o projetil e uma **bolinha circular** (placeholder) viajando do caster ao alvo. E suficiente pra validar o timing (cast -> viagem -> impacto).
- **VFX real depois:** o projetil-placeholder e substituido pelo **VFX da familia** correspondente (vfx-combate-familias.md): o raio ramificado do Eletrico, a nuvem que assenta do Bioquimico, os aneis do Sonico, o impacto-fenda do Cinetico, a grade que varre do Criptografico. Cada familia tem forma/cor/timing proprios; o placeholder circular e neutro de proposito (so testa o esqueleto de timing). A troca nao mexe na logica: o gancho "spawna projetil do caster, viaja, impacta no alvo" ja existe.
- **Cinetico (caso de borda):** o Cinetico e impacto fisico pesado (combat.md §6); mesmo sendo "magia=software", visualmente pode ler como golpe a distancia (anel de choque no alvo) e roda pelo caminho de magia (cast no lugar + efeito viaja/projeta). Decisao fina de qual familia usa melee real fica pro polimento de arte; o DEFAULT canonico e: **carta = caminho magia (cast + projetil); ataque basico fisico = caminho melee**.

### 2.2 Melee (desloca, golpeia, volta)

O atacante **SE DESLOCA** ate o oponente, executa o golpe (attack-melee), e **VOLTA** a posicao de repouso (battle-idle). Feel Chrono Trigger / Final Fantasy: o ator corre ate o alvo, bate, recua. E o caminho do **ataque basico fisico** (combat.md §5, 0 mana) e de cartas explicitamente corpo-a-corpo.

- O deslocamento e ida-e-volta: posicao de repouso -> aproxima do alvo -> golpe -> retorna a posicao de repouso. A camera e estatica (battle-screen D7): nao acompanha o ator; o movimento acontece dentro do frame fixo.
- Sem flip no deslocamento: o ator party mantem a pose virada pra direita ao avancar pra direita; o inimigo, virada pra esquerda (Pillar 3).

### 2.3 Hit-react do alvo (vale pros dois tipos)

O oponente atingido, **em magia E em melee**, executa a reacao de dano:

1. **Expressao rapida de SOFRIMENTO** (hurt): pose/quadro de dor breve.
2. **Movimentacao pra TRAS (knockback visual):** o sprite recua um tranco.
3. **VOLTA a posicao de pe em repouso** (battle-idle).

- **Knockback visual != knockback mecanico.** O recuo de hit-react e SEMPRE cosmetico (todo golpe que conecta empurra um tranco e volta). O status **Knockback** do Cinetico (combat.md §9, `ReorderActor(alvo, +1)`) e MECANICO e seu feedback principal e o retrato deslizando pra tras na FILA visivel (vfx-combate-familias.md, Cinetico). Os dois podem coincidir num golpe Cinetico, mas nao sao a mesma coisa: o tranco visual existe em qualquer hit; o reposicionamento na fila so quando o status Knockback e aplicado.
- **Multimodalidade (Pillar 4):** a dor le por POSE + recuo + floater de dano + (quando ha) overlay de status na silhueta (vfx-combate-familias.md §5). Nunca so cor.
- **FALHA (combat.md §11):** quando o golpe cai no canal FALHA (dano 0), NAO ha hit-react de sofrimento: o efeito se dissolve antes de tocar o alvo (vfx-combate-familias.md: assinatura corrompe em `#F43F5E`, `FALHA DE COMPILACAO`). O alvo permanece em battle-idle. Coerente com o anti-gore (Pillar 4): "compila erro e dissolve", nao acerta.

---

## 3. Encaixe no pacing (2 beats) e timing fino

A apresentacao ja roda um `PacingDirector` (battle-screen §5.2) que drena os eventos do motor UM a UM, com timing. O turno de inimigo tem **2 beats** (battle-screen §5.2, refinamento D8): **Beat 1 ANUNCIO** (`AnnouncingEnemy`, ~0.7s, nada resolveu, HP intacto) e **Beat 2 RESOLUCAO** (aplica dano + floater + log). A animacao de ataque encaixa exatamente nesses beats.

### 3.1 Onde cada parte da animacao mora

| Fase da animacao | Beat / estado do PacingDirector | O que toca |
|---|---|---|
| **Anticipation / windup** (cast carregando OU passos de aproximacao do melee) | **Beat 1 ANUNCIO** (`AnnouncingEnemy`, gancho explicito ja previsto em `advance_pacing`) | a animacao de preparacao toca aqui, ANTES de o golpe resolver. O motor ainda nao aplicou nada: HP intacto, sem floater. E o telegraph (Pillar 1: o jogador VE o golpe vir). |
| **Contato / impacto** | inicio do **Beat 2 RESOLUCAO** (`begin_enemy_step` -> `WaitingDelay`) | o golpe CONECTA: aqui o motor aplica dano/status, e a apresentacao dispara floater + queda de HP + log de consequencia + hit-react do alvo (sofrimento + knockback) + (depois) VFX da familia descompactando no alvo. |
| **Recovery / follow-through** | dentro do **Beat 2** (`WaitingDelay`, ~0.8s) | o atacante volta a posicao de repouso (melee retorna; cast dissipa o residual); o alvo termina o recuo e volta a battle-idle; floater sobe e some (~700ms, battle-screen D7). |

- **Turno do JOGADOR:** ja pausa no menu (`WaitingPlayerInput`), entao NAO usa o beat de anuncio separado. A animacao de ataque do membro da party toca quando ele confirma a acao (a aproximacao melee / o cast servem de proprio "anuncio"); o contato dispara o floater/hit-react igual. O ritmo de leitura pos-golpe ainda respeita o delay (~0.8s) antes de devolver o controle.
- **Regra de ouro (game-feel < 100ms):** o INPUT do jogador (confirmar [Atacar]/[COMPILAR]) tem resposta visivel em menos de 100ms (o sprite ja inicia o windup), mesmo que a resolucao completa do golpe leve os beats. Responsividade sentida no input; peso sentido nos beats. (Mesmo principio do snap <100ms do tap-to-place, battle-screen D6.)

### 3.2 Sincronia placeholder (retratos) -> sprite real

O `skip()` do pacing NUNCA colapsa o Beat 1 ANUNCIO (battle-screen §5.2, fix "ataque colado/duplo"): o anuncio sempre toca seu tempo proprio justamente porque **e o beat do windup da animacao**. Isso ja esta travado em regressao (`battle_scene_test`). Consequencia: quando o sprite real entrar, ele tem ~0.7s garantidos de windup pra ler (sem risco de o jogador "comer" o anuncio apertando rapido). O placeholder de retrato hoje ocupa esse beat parado; o sprite amanha o preenche com a animacao.

### 3.3 Hit-pause e screen-feedback (game-feel, quando couber)

Tecnicas de juice a aplicar no CONTATO (Beat 2), por intensidade do golpe. Ja antecipadas no vocabulario de VFX (vfx-combate-familias.md §3, canais):

| Tecnica | Quando | Parametro alvo (ajustar em playtest) |
|---|---|---|
| **Hit-pause / hit-stop** | em todo contato que conecta; mais longo no Cinetico (peso) e no CRIT | freeze curto ~50-150ms no instante do impacto; CRIT dobra; vende peso sem atrasar o pacing |
| **Screen-shake** | golpes pesados (Cinetico) e CRIT | amplitude proporcional ao dano; curto (0.1-0.3s); leve no Sonico (shockwave), forte no Cinetico CRIT (racha o chao, vfx §Cinetico) |
| **Flash de impacto** | todo contato | 1 frame de flash na cor da familia no alvo (vfx §Eletrico: flash branco; cada familia ja define o seu) |
| **CRIT amplificado** | canal CRIT | efeito ao maximo (mais ramos/aneis/densidade), `[CRITICO]` na cor da familia, hit-stop dobrado (vfx §3) |
| **FALHA dissolve** | canal FALHA | sem hit-react, sem shake; assinatura corrompe em `#F43F5E` e dissolve (vfx §3); o "tropeço" e visual, nao impacto |

Camera ESTATICA (battle-screen D7): screen-shake e um deslocamento curto do frame inteiro, nao um pan que segue o ator (pan enjoa no CTB e esconde fila/intents). Hit-pause e screen-shake sao de POLIMENTO: a base funcional (cast/melee/hit-react + floater) entra primeiro; o juice entra por cima sem refazer.

---

## 4. Tabela de sons (3.x) - PENDENTE, FAZER SO APOS O VISUAL

Decisao do criador (2026-06-25): o audio entra **depois** do visual de combate estar de pe. Registrado aqui como tabela pendente; a producao e do `audio-designer-composer` (cross-ref CLAUDE.md, squad Fase 2). Cada som lista o **gatilho** (evento do combate que dispara) e o **status** (todos "a definir, pos-visual").

| # | Som | Gatilho (evento do combate) | Status |
|---|---|---|---|
| **3.1** | **Magia sendo solta** (UM som por magia/familia: 5 sons-base, um por familia da roda, combat.md §6) | no CAST: ao iniciar o windup de conjuracao (Beat 1) e/ou ao spawnar o projetil. Casa com a assinatura sonora de cada familia (timing rapido Eletrico vs lento Bioquimico, vfx-combate-familias.md) | a definir, pos-visual |
| **3.2a** | **Passos correndo** (melee desloca) | durante o deslocamento de aproximacao do melee (§2.2); cadencia dos passos casa com a distancia percorrida, sabor da locomocao (locomotion.md §4: passo por distancia, nao por timer) | a definir, pos-visual |
| **3.2b** | **Golpe sendo dado** (impacto do melee) | no CONTATO do melee (Beat 2); pode variar leve por peso (Cinetico mais grave/seco) | a definir, pos-visual |
| **3.3** | **Golpe/magia RECEBIDA = vocal de dor** ("ai" / "ui" / "uuhh") | no hit-react do alvo (§2.3), no instante do contato (Beat 2). NAO toca em FALHA (dano 0, sem hit-react). Voz coerente com a idade/personagem (Gus 11 anos, Pillar 4) | a definir, pos-visual |

Notas pra o `audio-designer-composer` (quando entrar):
- O hit-pause (§3.3) e oportunidade natural de acento sonoro (o silencio do freeze vende o impacto).
- O canal CRIT pede variacao sonora (mais forte/agudo); a FALHA pede um som de "erro de compilacao" (coerente com o VFX `FALHA DE COMPILACAO`, vfx §3), nao um vocal de dor.
- UM som por familia em 3.1 (5 sons), nao um por carta (anti-explosao de assets pra solo G1); variacao por modificador/combo e polimento posterior.

---

## 5. Briefing de sprites de batalha necessarios (pra a arte / PixelLab)

Lista do que o sistema de animacao de combate PRECISA, por papel. Vira o briefing de arte futuro (pipeline PixelLab, battle-screen §3.4; matriz geral em animation-plan.md). **Sem flip (Pillar 3):** party usa a pose virada pra direita, inimigos a virada pra esquerda; desenhos distintos. Cada geracao reforca os tracos-assinatura canonicos do personagem (memoria `feedback_reforcar_caracteristicas_canonicas_geracao`: Gus = aparelho ortodontico + antena + computador de braço; Caua = glow cyan; etc).

### 5.1 Poses/anims por papel

| Pose / anim | Papel | Party (Gus + 6) | Inimigos | Para que serve |
|---|:--|:--:|:--:|---|
| **battle-idle** (em pe, virado pro oponente) | repouso de combate | sim | sim | estado-base na arena; idle congelado agora, respiracao additive depois (sabor locomotion.md §2) |
| **cast** (conjurar no lugar) | ataque magia (§2.1) | sim | sim | windup de conjuracao no Beat 1; antes do projetil viajar |
| **attack-melee** (golpe fisico) | ataque melee (§2.2) | sim | sim | o golpe no fim do deslocamento |
| **(deslocamento)** (correr ate o alvo e voltar) | locomocao de melee (§2.2) | sim | sim | REUSA o walk/run de batalha; pode partir do walk-leste/oeste ja planejado (locomotion.md). Nao e desenho novo se o run de batalha cobrir |
| **hit-react** (sofrimento + recuo + volta) | reacao de dano (§2.3) | sim | sim | dor no contato; volta a battle-idle |
| **victory** (comemoracao) | fim de combate (vitoria) | sim | nao | toca no BUILD SUCCEEDED (battle-screen §3.1); so party |
| **KO / down** (derrotado) | incapacitacao / wipe | sim | sim | companion incapacitado (Pillar 4) / inimigo removido / party wipe -> hospital |

### 5.2 Faseamento (minimo agora -> rico depois)

Espelha a filosofia data-driven da locomocao (locomotion.md §5): comeca no MINIMO, suaviza por DADOS sem reescrever logica.

- **Minimo pra o slice jogavel:** battle-idle + cast + attack-melee + hit-react, por party-membro e por inimigo do encontro. (`victory`, `KO` e a respiracao do idle entram logo depois.)
- **Adiciona por dados:** mais quadros de cast/golpe, respiracao additive no battle-idle, signature/ultimate por membro (animation-plan.md; ex. Dragon Victory do Gus, aura cyan->vermelho, memoria `project_dragon_victory_canon`). Nenhuma logica nova: so dados/sprite-sheet a mais.
- **Hit-react: 1 quadro de sofrimento basta no minimo** (a leitura vem de pose + recuo + floater); suavizar com mais quadros e direcao (light/heavy) e polimento.
- **Inimigos:** so as anims de combate (battle-idle/cast/attack-melee/hit-react/KO); inimigo NAO precisa de walk/run de overworld nem victory (animation-plan.md ja decide isso).

### 5.3 Direcao de batalha

Uma direcao de batalha por ator (frente/3-4, virada pro oponente), conforme animation-plan.md (combate = 1 direcao de batalha, nao as 8 do splash). A arena side-view so precisa do par leste (party) / oeste (inimigo) sem flip; o turntable de 8 direcoes e do splash de apresentacao, fora da arena.

---

## 6. Validacao (checklist de game-feel)

- [ ] Input de ataque responde em < 100ms (windup inicia antes da resolucao completa).
- [ ] Magia: caster conjura no lugar, projetil (bolinha placeholder) viaja ate o alvo e impacta.
- [ ] Melee: ator desloca ate o alvo, golpeia, e VOLTA a posicao de repouso (sem ficar deslocado).
- [ ] Hit-react: sofrimento + recuo pra tras + volta a battle-idle, nos dois tipos de ataque.
- [ ] Windup mora no Beat 1 ANUNCIO; o contato/floater/hit-react no Beat 2 RESOLUCAO (nada resolve no anuncio).
- [ ] `skip()` nao colapsa o Beat 1 (o windup sempre toca seu tempo proprio; sem "ataque colado/duplo").
- [ ] FALHA (dano 0): sem hit-react, efeito dissolve em `#F43F5E`; alvo fica em battle-idle.
- [ ] Sem flip: party virada pra direita, inimigos pra esquerda; aparato no lado canonico (Pillar 3).
- [ ] Camera estatica: screen-shake e deslocamento curto do frame, nao pan que segue o ator.
- [ ] Troca retrato-placeholder -> sprite real nao quebra os ganchos de timing (mesmo beat de windup/contato).
- [ ] (Audio, pos-visual) som de magia por familia, passos + golpe do melee, vocal de dor no hit-react, gatilhos certos.

---

## 7. Pipeline de implementacao (agentes)

1. **game-animator** (este doc): fecha o game-feel e o timing das anims de combate; briefa as poses (§5).
2. **engine-graphics-programmer**: pluga as anims na BattleScene (`app/`), enganchando nos beats do `PacingDirector` (battle-screen §5.2): windup no Beat 1, contato/floater/hit-react/VFX no Beat 2; spawna o projetil de magia (placeholder circular -> VFX de familia depois); hit-pause/screen-shake como passe de polimento. Le o estado do motor `domain/combat/`, NAO o altera.
3. **3d-artist-rigger / pipeline PixelLab** (battle-screen §3.4, animation-plan.md): produz as poses de batalha (§5), reforçando tracos-assinatura canonicos por personagem.
4. **audio-designer-composer**: a tabela de sons (§4), SO APOS o visual.
5. **qa-engineer**: valida o checklist (§6) no encontro turn-based jogavel (3-segundo readability test, separacao das 5 familias, responsividade < 100ms).

A regra canonica (battle-screen §6, combat.md): o motor (`domain/`, `core/`) permanece POCO puro; a animacao vive em `app/` e so LE o estado + drena os eventos com timing. Nenhuma anim toca a fórmula de dano nem a FSM.
