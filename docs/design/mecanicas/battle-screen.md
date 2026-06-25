# Tela de Batalha (BattleScreen): Design de Apresentacao

**Status:** Decisoes macro ratificadas pelo criador supremo em 2026-06-23 (brainstorm colaborativo, 5 perguntas via AskUserQuestion). Spec de APRESENTACAO do combate; o motor e as regras vivem em [`combat.md`](combat.md) (canonico, fechado, nao reaberto aqui). Implementacao no M5 (BattleScreen) da engine C++20 + SDL3.

**Convencao de escrita:** pt-br. Termos de game-dev no original. Sem em-dash; usa ponto, virgula, parenteses, dois-pontos.

**Cross-ref:** [`combat.md`](combat.md) (motor turn_combat ja portado/auditado), [`pillars.md`](../pillars.md), [`locomotion.md`](locomotion.md) (poses 4 direcoes, Pillar 3), [`../art/vfx-combate-familias.md`](../art/vfx-combate-familias.md) (vocabulario visual de ataque das 5 familias).

---

## 1. As 5 decisoes macro (criador, 2026-06-23)

| # | Decisao | Escolha | Por que | Implicacao |
|---|---|---|---|---|
| 1 | **Modelo de cena** | **Tela de batalha separada** (mundo some -> arena dedicada -> volta ao mapa). Feel Pokemon/Final Fantasy. | Mais barata e MUITO mais legivel: o sistema de cartas/pipeline/fila precisa de espaco de tela limpo, sem o mundo 3D competindo. | A arena e uma cena propria. Evolui depois pro **hibrido** (fundo da arena = pintura do bioma onde esbarrou) SEM refazer: so trocar o fundo plano por uma arte de bioma. |
| 2 | **Angulo dos atores** | **Side-view de perfil** (Chrono Trigger / FF). Party a esquerda olhando pra direita; inimigos a direita olhando pra esquerda. | Ve todos de corpo inteiro: ideal pra COMANDAR 3 personagens com fila CTB. A referencia-mae (Chrono Trigger) e exatamente assim. | **Zero arte nova:** usa as poses leste (party) e oeste (inimigos) que ja existem. Sem flip (Pillar 3 respeitado). Nao depende das diagonais (ARTE-DIAGONAL-8DIR fica fora do caminho critico). |
| 3 | **Layout** | **Comando-first** (menu de verbos do ator ativo; cartas + pipeline abrem so ao COMPILAR). | Tela limpa e legivel; casa com Chrono Trigger/Sea of Stars/Pokemon (todos comando-first). Nao polui com 3-4 inimigos. As cartas tomam o palco na hora certa. | A mao de 15 cartas NAO fica sempre na tela; surge num leque + pipeline de 3 slots quando o jogador escolhe COMPILAR. |
| 4 | **Feedback / juice** | **Numeros flutuantes sobre o alvo + caixa de log.** | O numero da impacto imediato; o log carrega as mensagens de sistema longas que o combat.md EXIGE (ERRO DE COMPILACAO, COMPILADO, ANALISE PREDITIVA). | Dois canais de feedback: popup de dano (curto, sobre o alvo) + log rolando (mensagens de sistema + historico). |
| 5 | **Telegraph de intent** | **Icone de intencao flutuando sobre o inimigo** (estilo Slay the Spire). | Padrao-ouro de legibilidade; casa com Scan (revela) e Gambito-Prever (aprofunda alvo/area). Pillar 1: informacao habilita acao. | Cada inimigo mostra um simbolo do plano (atacar quem + dano previsto / defender / aplicar status). **Patch-Zero** (intent caotico, one-way door ja canon) mostra um icone "ruido" embaralhado. |

---

## 2. Layout consolidado (mockup)

```
+------------------------------------------------------+
| FILA CTB:  > Gus   > [i]   > Caua   > [i]   > Jaci .. |   topo: ordem de turno (Gambito mexe aqui)
+------------------------------------------------------+
|                                                      |
|  [Gus >]                      (espada 12 ->Gus) [i<] |   arena side-view:
|                                                      |   party a ESQ (pose leste)
|  [Caua >]             (escudo)                  [i<] |   inimigos a DIR (pose oeste)
|                                                      |   icone de intent sobre cada inimigo
|  [Jaci >]                     (gota veneno)     [i<] |
|                         -45 [CRITICO]                |   numero de dano flutua sobre o alvo
+------------------------------------------------------+
| > TURNO: Gus     HP||||||   AP:3   Mana:5            |   painel do ator ativo
|   [Scan] [Gambito] [Atacar] [Defender] [COMPILAR] [Flee]
|------------------------------------------------------|
| LOG:  > COMPILADO: Descarga Tripla                   |   caixa de log (mensagens de sistema)
|       > inimigo sofre 45  [CRITICO]                  |
+------------------------------------------------------+

ao escolher [COMPILAR], abre POR CIMA (overlay):
   mao (leque):  [carta] [carta] [carta] [carta] [carta]
   pipeline:     [ slot 1 ] [ slot 2 ] [ slot 3 ]   -> COMPILADO: <combo>
```

### Zonas da tela

- **Topo, faixa horizontal:** fila de iniciativa CTB (proximos N atores, ordem esquerda->direita). E mecanica, nao cosmetico (o Gambito-Reordenar opera nela). Mostra retratos pequenos + marca de "proximo".
- **Centro, arena:** atores em side-view. Party (ate 3) a esquerda; inimigos (1 a 4) a direita. Sobre cada inimigo, o icone de intent. Sobre o alvo de um golpe, o numero de dano flutuante.
- **Base, painel do ator ativo:** so do ator cujo turno e (CTB por-ator). HP, AP (3), Mana (ramp), status effects, e o menu de verbos.
- **Base, caixa de log:** mensagens de sistema (COMPILADO, ERRO DE COMPILACAO, ANALISE PREDITIVA, FALHA/CRIT) + historico curto.
- **Overlay de compilacao:** so visivel ao escolher COMPILAR. Leque da mao + pipeline de 3 slots.

---

## 3. O que a BattleScreen apresenta (mapeado do motor)

O motor (`domain/combat/`) ja entrega o estado; a BattleScreen e a CAMADA DE APRESENTACAO (em `app/`). Mapeamento:

| Estado do motor (combat.md) | Onde aparece na tela |
|---|---|
| Fila de iniciativa por SPD (CTB) | faixa do topo |
| Ator ativo + AP (3) + Mana (ramp) | painel da base |
| HP de cada ator + Shield (pool) | barra sob cada ator + no painel |
| Status effects (Stun, Poison, Expose...) | icones sob o ator afetado |
| IntentPreview do inimigo (telegraph) | icone de intent sobre o inimigo |
| Mao de 15 cartas + pipeline de 3 slots | overlay de COMPILAR |
| Roda de fraqueza (revelada por Scan) | indicado ao mirar (fraco/neutro/resistente/imune) |
| Dano por canal (FALHA / CRIT / COMUM) | numero flutuante + sufixo no log ([CRITICO], FALHA DE COMPILACAO) |
| Combo casado | COMPILADO: <nome> no log + flash na pipeline |
| Erros de pre-condicao (mana/AP/Null sem Scan) | ERRO DE COMPILACAO: <motivo> no log |
| Analise Preditiva (Pillar 4, 1x/batalha) | ANALISE PREDITIVA: golpe fatal absorvido no log |
| Ambiente (terreno/clima/periodo) | marca discreta na arena (fundo/HUD); Scan-ambiente revela tier |

---

## 3.1 Tela de resultado (fim de combate)

Decidido no brainstorm 2026-06-23. O fim do combate e um LOG DE BUILD que imprime ao vivo (terminal; coerente com "software fala em terminal", ver [`combat-flavor.md`](combat-flavor.md) paragrafo 5). Acervo de frases (vitoria/derrota/rotulos) em combat-flavor.md.

- **Formato:** o terminal imprime as etapas e os ganhos linha a linha (build rodando), nao um painel estatico. A satisfacao esta no scroll de sucesso.
- **Vitoria:** imprime `BUILD SUCCEEDED` / `exit 0` + ganhos: XP, loot, Knowledge (o selo do bestiario sobe), mestria de carta (+1 por uso, Pillar 1). **Metrica de build:** os turnos viram "compile time"; build rapido ganha rotulo de elogio (blazing fast / clean build) + BONUS de loot/XP por eficiencia; build lento recebe rotulo NEUTRO (nunca xinga). Liga com Knowledge (dominar = build mais rapido) e com o bonus de "lutar de verdade" do auto-kill.
- **Derrota (party wipe):** imprime `BUILD FAILED` / `core dumped`, corta pro HOSPITAL (canon Pillar 4 + economia: `ActorIncapacitated` -> hospital). NAO e game-over, NAO perde progresso: a derrota e um CUSTO (cura proporcional ao dano, ~1cr/3HP). Hospital tematizado como "restaurando do snapshot".
- **Sem creditos pro hospital (anti-softlock; parametros do economy-designer + criador 2026-06-23):** o jogador ESCOLHE, nunca trava:
  - **Safe mode (gratis):** revive a party a **13% do HP max** (Fibonacci; piso 1 HP), "no optimizations". So ofertado em wipe REAL (nunca voluntario). Capenga sem death-loop (a Analise Preditiva cobre o golpe fatal seguinte). Anti-abuso: o HP ganho vale menos que o XP/loot/credito perdido ao morrer (perder e sempre net-negativo).
  - **OU cura completa a credito (divida = "taxa de recompilacao do snapshot"):** sai inteiro, devendo. Termos TRANSPARENTES no terminal antes de aceitar (Pillar 2, anti-Sterling: sem letra miuda):
    - **Principal:** 1 cura completa da party (~48cr no VS, escala sozinho).
    - **Juros:** 5% SIMPLES (nunca composto) sobre o principal, 1x por NOVA ZONA cruzada, teto 21% do principal (~11cr). Param de crescer no teto.
    - **Multa:** so por REINCIDENCIA (wipe enquanto ainda devendo). Escada curta: 8cr na 1a, 13cr da 2a em diante, TRAVA em 13cr. Evento pontual, nunca relogio.
    - **Cap total fixo ~72cr** (principal + encargos com tetos proprios). Encargos nunca compram mais cura nem movem o cap.
    - **Quitacao automatica:** o JOGADOR escolhe o PLANO ao sair do hospital (pode reajustar): agressivo (62% do credito recebido pra divida) / medio (50%) / suave (38%); o resto fica livre. Ordem de abate: multa -> juros -> principal. Pior caso quita em ~17 encontros, sempre jogando normal.
    - Bloqueia SO compras/craft, NUNCA o hospital (curar sempre disponivel = anti-softlock).
  - Tematizacao terminal: `insufficient funds` -> `[1] safe mode (13% HP, gratis)` / `[2] recompilar a credito (ver termos)`.
- **Build NAO-otimizado (combate RESOLVIDO sem encarar; canon 2026-06-25, combat.md §19):** quando o jogador escolhe `Resolver sem encarar` (auto-resolve opt-in), o motor roda headless com a IA sub-otima (`AutoResolveBrain`) e o terminal de resultado imprime um build `-O0`, distinto do build a mao:
  - **Pulou e sobreviveu:** `building (no optimizations, -O0)...` -> `warning: combat resolved unattended` -> `BUILD SUCCEEDED` com loot REDUZIDO (sem rotulo de elogio de eficiencia; o `-O0` ja comunica "lento/sujo"). Penalidade de loot/dano por selo (x/y) = economia.md (economy-designer).
  - **Pulou e deu wipe:** `building (no optimizations, -O0)...` -> `BUILD FAILED` / `core dumped` -> HOSPITAL (mesmo fluxo acima: safe mode gratis OU credito).
  - Contraste `-O0` (pulou, sem otimizar) vs `-O2` (encarou e otimizou, blazing fast + bonus) = feedback diegetico da escolha (Pillar 2: magia=software; encarar = compilar a mao otimizando).

A economia do hospital (estes parametros) deve ser integrada formalmente em [`economia.md`](economia.md) via economy-designer.

---

## 3.2 Overlay de compilacao (mao + pipeline)

Decidido no brainstorm 2026-06-23. O CORACAO da jogabilidade: aparece ao escolher COMPILAR no menu de verbos (par.1, decisao 3, comando-first). Materializa a pipeline de 3 slots do [`combat.md`](combat.md) par.10.

- **Montagem (tap-to-place):** clica uma carta da mao e ela vai pro proximo slot livre da pipeline; clica no slot pra remover. Funciona igual em mouse, teclado e controller (sem drag-drop).
- **Mao (leque), carta COMPACTA:** cada carta mostra cor da familia + nome + custo de mana + marcador RAPIDA (compilada) ou LENTA (interpretada). O detalhe (power, fraqueza vs alvo, status aplicado, mestria, alvo) aparece num painel lateral ao FOCAR a carta. Leque limpo, decisao agil.
- **Pipeline de 3 slots:** preenchida em ordem. Cada slot = carta OU modificador (canon par.10). Conforme monta, mostra o custo acumulado (mana/AP) e, se a assinatura casar uma receita:
  - combo JA descoberto (no codex de receitas): preview ao vivo `COMPILANDO: <nome>` + efeito + se e rapido/lento.
  - combo NAO catalogado: `??? combo nao catalogado` (dica de que vale disparar pra descobrir; preserva o aesthetic Discovery).
- **Modificadores (combat.md par.8):** ocupam 1 slot e afetam a carta do slot anterior, somando mana.
  - **Object (+1)** e **Stream (+2):** sempre disponiveis (2 botoes fixos).
  - **Null (+1):** ESPECIAL, so via carta/desbloqueio (o mais forte, anti-buff); exige Scan previo no alvo (botao desabilitado sem Scan, com a mensagem de erro do par.10).
- **Erros (combat.md par.10):** tentativa invalida mostra a mensagem no log: `ERRO DE COMPILACAO: mana insuficiente (custa X, tem Y)` / `AP insuficiente` / `alvo invalido` / `Null requer Scan previo` / `pipeline ja contem 3 slots`.
- **Disparar:** confirma a pipeline -> resolve sequencial (slot 1, 2, 3) -> se casou receita, `COMPILADO: <combo>` no log + flash na pipeline. A velocidade (rapida/lenta, cast-time) do resultado segue [`combat-flavor.md`](combat-flavor.md) par.1.

Decisoes finas: posicao do overlay = D5 (inferior parcial, arena com dim); animacao de encaixe = D6 (snap + slot acende + pulse de receita); ver paragrafo 5. Vocabulario visual do marcador rapida/lenta e dos icones de modificador fica pro ux-ui-designer no polimento (os icones de modificador ja existem em resources/sprites/icons-m5/modificador/).

---

## 3.3 Transicao de entrada e saida

Decidido 2026-06-24. **Entrada:** ao esbarrar no inimigo, BOOT/COMPILACAO tematico (~0.5s, rapido, nao cansa ao repetir): flash + scanline + micro-log de terminal (`encounter.init()` / `arena: linking atores...` / `READY`) e a arena MONTA. Simetria com a tela de resultado (abre "compilando", fecha "BUILD SUCCEEDED"). **NAO usa o glitch / RGB-split** (reservado ao anomalo / Patch-Zero, ver vfx-combate-familias.md): a linguagem aqui e scanline + build-log, nao glitch. **Saida:** o inverso (a arena "descompila" de volta pro overworld). Coerente com magia=software e o registro terminal de todo o combate.

---

## 3.4 Pipeline dos atores na arena (decisao do criador 2026-06-24)

Os atores na arena side-view sao **sprites 2D puros**, gerados direto no **PixelLab** (mesmo pipeline dos 37 icones + 11 retratos ja produzidos). NAO ha bake 3D: o jogo e 2D-only (style-guide, ADR-008); o 3D era so ferramenta de bake e foi aposentado no pivot (os .glb do art-spike foram deletados). Consequencias:
- Poses de batalha (idle / cast / ataque / hit / vitoria) por personagem = PixelLab. A foto `gus_baby/gus_meiafrente.png` serve de referencia pra pose lateral do Gus; demais personagens partem dos retratos/character-specs ja canonicos.
- O PixelLab gera personagem com 4/8 direcoes nativas (`create_character`), util pra a pose leste (party) / oeste (inimigo) sem flip (Pillar 3).
- Fecha o gate que estava adiado: as poses de corpo inteiro deixam de ser bloqueadas e entram no pipeline normal de arte.

---

## 4. Escopo do M5 (vertical slice) vs depois

### M5 entrega (a tela jogavel do combate)
- A cena separada (transicao de entrada/saida, forma simples no M5; polimento depois).
- Side-view com party (placeholders/poses leste-oeste) vs inimigos.
- Faixa de fila CTB lendo o estado do motor.
- Painel do ator ativo (HP/AP/Mana/status) + menu de verbos.
- Overlay de COMPILAR (leque da mao + pipeline de 3 slots).
- Numero flutuante + caixa de log com as mensagens de sistema do combat.md.
- Icone de intent sobre o inimigo (ScriptedBrain ja expoe IntentPreview).
- Loop completo: entra do overworld -> resolve o combate -> volta ao overworld (engancha no CombatEnded/PlayerBus).

### Fora do M5 (interface ja preparada)
- Fundo de bioma (evolucao pro hibrido).
- Animacoes de ataque ricas por familia (VFX de Pulso/Raiz/Eco...): polimento de arte posterior.
- Transicao "assinatura" elaborada (a do M5 e funcional).
- COMBATE-AUTOKILL + Resolver sem encarar (instant-win no overworld + auto-resolve opt-in): **mecanica CANONIZADA em combat.md §19** (eixo de dominio auto-kill/auto-resolve/encarar). A BattleScreen ja incorpora o HOLD de abertura + verbo (§5.2 D13) e o terminal `-O0` (§3.1); o auto-kill silencioso no overworld (micro-animacao por arquetipo + falas-balao) e apresentacao de OVERWORLD, fora da BattleScreen, plugada DEPOIS.
- CARTAS-CAST-TIME (cartas lenta/rapida; ver [`combat-flavor.md`](combat-flavor.md) + INBOX do TODO): a tela ja deve PREVER o marcador de cast na fila (ex: "Gus: interpretando...") e o spinner de fases; a mecanica de motor (cast-time real) entra como extensao DEPOIS da BattleScreen base, sem refazer.

---

## 5. Decisoes finas de layout (FECHADAS pelo criador 2026-06-24)

Briefing 100% fechado pro engine-graphics-programmer (proposta do lead-game-designer + decisao do criador via AskUserQuestion).

- **D1 Resolucao base:** **640x360** (16:9), pixel-perfect, escala inteira (x2 720p / x3 1080p). E a tela mais densa do jogo (7 atores + 4 zonas de HUD + overlay); 640x360 da folego sem perder o pixel.
- **D2/D3 Arena (disposicao):** **coluna unica de cada lado, espacamento fixo.** Party empilha a esquerda (pose leste), inimigos a direita (pose oeste), sempre centralizados no eixo vertical (1 a 4 inimigos, SEM escala dinamica = pixel-perfect, mira deterministica). **Gus levemente recuado** (1 regra, serve Pillar 4: o fragil). Profundidade real (V/frente-tras) = polimento pos-M5. Mini-boss ocupa mais por sprite-base maior, nao por escala.
- **D4 Fila CTB:** mostra os **5 proximos**, celula = **retrato 48px** (asset nativo, sem downscale) + marca de "proximo" no 1o. SEM nome, SEM mini-barra (a barra de HP vive sob o ator na arena). Preparada pra o marcador "interpretando..." (cast LENTO, CARTAS-CAST-TIME) ocupar uma celula como ator-fantasma; se resolver alem de 5 casas, a 5a celula marca "+N".
- **D5 Overlay de COMPILAR:** **inferior parcial** (~40% da tela, sobe de baixo sobre painel+log), a **arena fica visivel atras com leve dim** (nao apaga). Pillar 1: ver intent/fraqueza do inimigo enquanto monta o combo. (Acoplado a D1=640x360 pra caber.)
- **D6 Encaixe de carta (tap-to-place):** **snap instantaneo** (<100ms, sem tween bloqueante) + o slot "acende" na cor/icone da familia + quando a assinatura casa receita, a **pipeline pulsa** (preview COMPILANDO). Ao remover, fade-out rapido. O juice esta no "ding" de combo fechado (Discovery). Slide animado = polimento opcional depois.
- **D7 Camera + dano:** **camera estatica** (sem zoom/pan a cada turno; CTB troca muito de turno, pan enjoa e esconde fila/intents), so um **highlight/seta no ator ativo**. Dano = **numero flutuante** sobre o alvo (sobe + fade ~700ms), **cor por canal** (combat.md par.11): COMUM claro, CRIT ciano + bold, FALHA dano-0 vermelho-erro `#F43F5E`, CURA verde com `+N`. Cinematografia de golpe especial = pos-M5.
  - **REVISAO D7-LOG (criador, 2026-06-25, apos teste no display):** o log **NARRA o combate** (mostra TODA acao e dano em texto, nao so eventos notaveis). Motivo: a regra original "log so notavel + dano = numero flutuante" deixou o dano comum **invisivel** enquanto os numeros flutuantes nao existiam (eram do incr 5); ao implementar os dois juntos, o criador decidiu que o log deve narrar (ex.: "Caua ataca o Drone: 25", "Drone usa Pulso em Gus: 12", "Gus defende"). Mantem **cor por categoria** (dano/cura/status/sistema), **bold** nas mensagens-codigo (COMPILADO / ERRO DE COMPILACAO) e nos golpes notaveis, e **rola cortando no tamanho da caixa** (mostra as ultimas N linhas). O numero flutuante cobre o IMPACTO imediato; o log cobre o HISTORICO legivel. (Implementado no incr 5: `battle_floaters` + `battle_log_model::build_log_lines` sem filtro de notavel.)

---

## 5.1 Sistema de fonte (decisao do criador 2026-06-24)

A engine NAO tem sistema de fonte (o IRenderer so faz rect/textura). Decisao: **manter font-free por enquanto** (HP=barra, AP/Mana=pips, status=icones PNG; tudo legivel sem texto). Decidir a fonte quando o combate EXIGIR texto de verdade. Reversivel: numeros se sobrepoem as barras sem mudar o layout POCO.

**Tensao registrada (dever de contra-argumentar, NAO bloqueia):** o design da BattleScreen e CENTRADO em texto de terminal (o LOG com COMPILADO/ERRO DE COMPILACAO/as 100 frases de falha, os numeros de dano flutuante, a tela de resultado BUILD SUCCEEDED, as falas-terminal). Font-free cobre o PAINEL (incremento 2), mas o LOG e os numeros (incrementos 3-5) sao o coracao do "magia=software" e SAO texto. Consequencia pratica: ate a fonte entrar, o log/numeros ficam como PLACEHOLDER (estrutura da caixa sem conteudo legivel). Candidato pronto: `stb_truetype` ja vendorizado em `third_party/stb`. O criador decide o momento; provavel gatilho = quando o log/terminal precisar ser lido de fato.

---

## 5.2 Ritmo / pacing do combate (decisoes do criador 2026-06-25, do playtest no display)

O playtest pegou que o combate sem RITMO atropela tudo (inimigos agem antes do jogador ver, floaters/log somem, confuso de quem e a vez). Decisoes:

- **D8 Modelo de ritmo: HIBRIDO.** Cada turno mostra acao + numero flutuante + log, pausa ~0.8s, segue sozinho; o jogador pode APERTAR uma tecla pra acelerar/avancar pro proximo. Fluido por padrao + controle quando quer ler. A apresentacao processa UM evento de cada vez (fila de eventos drenada do motor com timing), NAO resolve tudo instantaneo.
- **D9 Indicador de turno: banner + highlight + 'sua vez'.** Texto claro de quem joga ("TURNO DE GUS"); ator ativo com destaque forte (brilho/seta); na vez do jogador, "SUA VEZ: escolha uma acao"; na vez do inimigo, "vez do inimigo". Leitura imediata.
- **D10 Abertura: comeca PARADA e ESPERA INPUT.** Ao abrir: arena monta, "BATALHA!" breve + a fila, NINGUEM agiu. **A abertura agora PARA e ESPERA o jogador iniciar** (decisao do criador 2026-06-25, casa com o verbo Resolver sem encarar, D13): a luta so comeca quando o jogador manda [Encarar] (Enter). Ao iniciar, os turnos animam um a um com o ritmo; se o inimigo for o 1o (maior SPD), o jogador VE ele atacar com numero+log no tempo certo. (Resolve "inimigos ja atacaram antes de eu ver": agora o 1o turno so dispara com input do jogador.)
- **D11 Floater no ataque do INIMIGO:** com o pacing, cada ataque inimigo aparece no seu tempo com seu numero flutuante (antes batiam todos juntos e somiam).
- **D12 Log narra a CONSEQUENCIA:** nao so "X atacou", mas "X atacou Y: Y recebeu N de dano" e o status ("Y ficou com Stun"). A apresentacao monta a linha completa a partir do dano + status que o motor ja entrega. E com o pacing, o log nao rola rapido demais (uma linha por evento no ritmo).
- **D13 Estado de HOLD na abertura + verbo "Resolver sem encarar" (canon 2026-06-25, combat.md §19; SO TRASH).** O estado parado do D10 e um HOLD que espera o input. Opcoes apresentadas:
  - **[Encarar] (Enter):** inicia o combate normal (caminho ja descrito em D10). Custo ZERO de atrito (default; 99% das lutas).
  - **[Resolver sem encarar] (tecla dedicada):** SO aparece para TRASH com selo de dominio Bronze+ (gate de onboarding; combat.md §19.2). Ao apertar, abre o **aviso de consequencias** com o **rotulo de risco** (LOW / MEDIUM / HIGH, lido do Knowledge; HIGH adverte explicitamente que pode mandar a party pro Hospital). Confirmar -> roda o auto-resolve headless (`AutoResolveBrain`, combat.md §19.6) -> terminal de resultado `-O0` (§3.1). Voltar -> encara.
  - Em luta NAO-trash ou sem selo, o verbo NAO aparece; o HOLD ainda espera [Encarar] (Enter) para iniciar (o hold de abertura e universal; o verbo de pular e condicional ao tier Trash + selo Bronze+).
  - O atalho global para pular em massa e o **toggle "Auto-resolver" do HUD de 3 estados** (Encarar tudo / Auto so dominado [default] / Auto maximo; combat.md §19.5). O verbo per-luta e o override pontual no estado default.
  - Consequencia arquitetural: o `PacingDirector` ganha o estado de HOLD inicial esperando input (ja existe `WaitingPlayerInput`; aqui e um hold de abertura ANTES do 1o turno, com o menu [Encarar]/[Resolver sem encarar]). O auto-resolve em si nao usa a BattleScene (roda a FSM headless); so o resultado volta pro terminal.

Consequencia arquitetural: a BattleScene ganha um "diretor de pacing" (POCO testavel) que consome os eventos do motor um a um, com timing e estados (animando / esperando-input-do-jogador / esperando-delay). O motor (domain/combat) NAO muda - so a apresentacao deixa de drenar tudo de uma vez.

**Status de implementacao (incremento 6, entregue 2026-06-25):** D8-D12 implementados. `PacingDirector` POCO (`gus/app/screens/battle_pacing.{hpp,cpp}`): estados Intro / WaitingDelay / WaitingPlayerInput, avanco por `tick(dt)`, `skip()` pra acelerar. A BattleScene comeca PARADA (D10), anima 1 turno por vez com delay ~0.8s (D8/D11), mostra o banner de turno (D9) e narra a consequencia (dano + status via `consequence_suffix`, D12). Cada inimigo age UMA vez por turno (1 ataque = 1 floater no seu tempo), em vez de gastar os 3 AP de uma vez. Intro `kPacingIntroSeconds` = 0.9s; delay `kPacingStepDelaySeconds` = 0.8s.

**Refinamento D8: 2 BEATS no turno de inimigo (2026-06-25, do playtest):** o lider no display: "a tela aparece com o ataque ja feito" e "quando colocar a animacao isso vai ser perdido". Causa: o motor resolve o turno instantaneo e a apresentacao mostrava o ANUNCIO junto do resultado ja aplicado. Fix: o turno de inimigo agora tem 2 beats. **Beat 1 ANUNCIO** (`PacingState::AnnouncingEnemy`, `kPacingAnnounceSeconds` = 0.7s): mostra "Vez de <nome>" + highlight, NADA resolveu (HP intacto, sem floater, sem log de acao). **E o beat onde a animacao de ataque (windup) vai morar** (gancho explicito em `advance_pacing`). **Beat 2 RESOLUCAO** (`begin_enemy_step` -> `WaitingDelay`): aplica o golpe (dano + floater + queda de HP + log de consequencia), pausa ~0.8s, proximo. Vale tambem pro 1o turno apos a intro (BATALHA! -> anuncio -> resolve). O turno do JOGADOR ja pausa no menu (`WaitingPlayerInput`), entao nao precisa do beat de anuncio. Regressao travada em `battle_scene_test` (no anuncio o alvo esta intacto; so o resolve bate).

**Status D13 - HOLD de abertura aguardando input (2026-06-25, do playtest):** o lider no display: "abriu com pequena pausa e atacou" + "nao vi nenhum BATALHA!". O Intro de 0.9s temporizado ainda passava antes dele olhar. Fix (decisao do lider): o `PacingState::Intro` deixou de ter timer - vira um HOLD que ESPERA INPUT (igual ao turno do jogador): `tick`/`skip` nao avancam o Intro; so `begin_combat()` (ENCARAR) sai dele e libera o 1o turno. A abertura mostra "BATALHA!" + prompt: **"[Enter] Encarar"** (sempre) e **"[Q] Resolver sem encarar"** (so quando `offers_auto_resolve()` = todos os inimigos sao TRASH; no demo sim - boss/elite escondem). O verbo de auto-resolve esta como PLACEHOLDER NAO-DESTRUTIVO neste incremento (`request_auto_resolve()` so loga "[auto-resolve: a implementar]"); o calculo real (`AutoResolveBrain` headless + penalidade por selo, §19) vira num incremento separado pos-canonizacao. Implementado: `PacingDirector::begin_combat`/`waiting_intro`; `BattleScene::start_combat`/`request_auto_resolve`/`offers_auto_resolve`; render do prompt; viewer (Enter=Encarar, Q=placeholder). Regressao travada em `battle_scene_test` (abertura nao auto-avanca por tempo; skip nao comeca; Q nao e destrutivo e nao sai do hold).

**Fix "ataque colado/duplo" (2026-06-25, do playtest):** o lider no display: "quando o ataque SEGUINTE e de um inimigo, sai COLADO com o anterior (impressao de ataque duplo)". Causa-raiz (confirmada por instrumentacao da fila real): apertar a tecla durante o BEAT 1 (anuncio) chamava `skip()`, que ZERAVA o timer do anuncio -> o anuncio durava 1 frame e o golpe resolvia colado. Fix: `skip()` agora so acelera a pausa de LEITURA pos-resolucao (`WaitingDelay`); NUNCA o anuncio (`AnnouncingEnemy`) - o anuncio "Vez de <nome>" sempre toca seu tempo proprio (e o beat do windup da animacao). Verificado: com skip a cada frame, o anuncio dura ~0.7s (42 frames), nao 1. Regressao em `battle_scene_test` (skip nao colapsa o anuncio; na fila REAL todo inimigo resolvido teve seu proprio anuncio).

**Fix sobreposicao arena x painel (2026-06-25, do playtest):** o lider no display: a coluna de 4 inimigos (slots 56x64 empilhados) transbordava pra dentro do menu/log (4o inimigo atras do verbo "Fugir"). Causa: 4 slots de 64px + gaps (~274px) nao cabiam na banda da arena (kArenaTop=64..kArenaBottom=250 = 186px); o empilhamento clampava no topo e invadia o painel (kActivePanelTop=252). Fix: a ALTURA do slot virou ADAPTATIVA por contagem (`arena_slot_height(count)`): teto kActorSlotH=64 com 1-2 atores, encolhe pra a coluna inteira caber na banda com folga (3 atores -> 55px; 4 -> 40px; piso kActorSlotMinH=36). A banda termina ACIMA do painel e o painel/log tem fundo OPACO desenhado por cima dos atores (ordem: fundo arena -> atores -> painel/menu/log). Verificado pra 1..4 de cada lado: todo slot termina com y+altura <= kActivePanelTop. Regressao travada em `battle_layout_test` (nenhum slot invade o painel em nenhuma contagem).
  - **TENSAO COM D3 (reportar ao criador):** o D3 dizia "1 a 4 inimigos, SEM escala dinamica = pixel-perfect". O fix introduz escala da ALTURA do quadro do slot por contagem (mesma altura pra todos do lado, deterministica - nao e por-sprite arbitrario), pra nao transbordar. Alternativas que preservariam "sem escala" (decisao do criador, NAO implementadas): (a) manter 64px fixo e dispor 4 inimigos em 2 sub-colunas/zigue-zague a direita; (b) banda de arena mais alta (subir a fila CTB / encolher o painel); (c) limitar a 3 inimigos visiveis no slice. O fix atual (altura adaptativa) e o caminho de menor atrito e cabe pixel-perfect; se o criador preferir manter D3 estrito, troca-se a estrategia sem mexer no resto.

---

## 6. Pipeline de implementacao (agentes)

1. **lead-game-designer**: fecha as pendencias finas do paragrafo 5 (propostas -> AskUserQuestion ao criador).
2. **engine-graphics-programmer**: implementa a BattleScreen em `app/` (cena, render side-view, overlay de cartas, numeros flutuantes, log, icones de intent), lendo o estado do motor `domain/combat/` e enganchando no barramento de eventos (`core/events/`) por CombatEnded/PlayerBus.
3. **qa-engineer**: plano de teste da tela (loop overworld->batalha->volta; legibilidade dos feedbacks).

A regra canonica: o motor (`domain/`, `core/`) permanece POCO puro; a BattleScreen vive em `app/` e so LE o estado + drena a lista de mudancas, sem framework no dominio.
