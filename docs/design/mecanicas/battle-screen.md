# Tela de Batalha (BattleScreen): Design de Apresentacao

**Status:** Decisoes macro ratificadas pelo criador supremo em 2026-06-23 (brainstorm colaborativo, 5 perguntas via AskUserQuestion). Spec de APRESENTACAO do combate; o motor e as regras vivem em [`combat.md`](combat.md) (canonico, fechado, nao reaberto aqui). Implementacao no M5 (BattleScreen) da engine C++20 + SDL3.

**Convencao de escrita:** pt-br. Termos de game-dev no original. Sem em-dash; usa ponto, virgula, parenteses, dois-pontos.

**Cross-ref:** [`combat.md`](combat.md) (motor turn_combat ja portado/auditado), [`pillars.md`](../pillars.md), [`locomotion.md`](locomotion.md) (poses 4 direcoes, Pillar 3).

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
- **Sem creditos pro hospital (sintese safe-mode + divida, decisao 2026-06-23):** o jogador ESCOLHE, nunca trava (anti-softlock):
  - **Safe mode (gratis):** revive a party com HP minimo (~20%, parametro a afinar), "no optimizations". Sai capenga (a fraqueza E o custo; nao da pra abusar, pois curar pagando e sempre melhor).
  - **OU cura completa a credito (divida):** sai inteiro, mas com saldo NEGATIVO; precisa quitar antes de novas compras (responsabilidade economica, axiologia canon). Limite de divida + juros (se houver) = parametro do economy-designer.
  - Tematizacao terminal: `insufficient funds` -> `[1] safe mode (20% HP)` / `[2] full repair on credit (debt)`.

A economia do hospital (safe-mode + divida, parametros) deve ser canonizada em [`economia.md`](economia.md) via economy-designer.

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
- COMBATE-AUTOKILL (instant-win no overworld; ver INBOX do TODO): canonizar no combat.md e plugar aqui DEPOIS.
- CARTAS-CAST-TIME (cartas lenta/rapida; ver [`combat-flavor.md`](combat-flavor.md) + INBOX do TODO): a tela ja deve PREVER o marcador de cast na fila (ex: "Gus: interpretando...") e o spinner de fases; a mecanica de motor (cast-time real) entra como extensao DEPOIS da BattleScreen base, sem refazer.

---

## 5. Pendencias de detalhe (a propor pelos agentes, validar com o criador)

Decisoes FINAS ainda nao tomadas (vao por AskUserQuestion quando os agentes chegarem nelas):
- **Transicao exata** de entrada/saida (flash, wipe estilo Pokemon, zoom): o lead-game-designer/engine-graphics propoe 2-3, o criador escolhe.
- **Forma fina da fila** (quantos atores a frente mostrar; tamanho dos retratos).
- **Estilo dos icones** de intent e de status (vocabulario visual coeso com o HUD do overworld).
- **Posicionamento** com 1 vs 4 inimigos (formacao que nao quebra a leitura).
- **Cores/tokens** da UI de batalha (herdar do HUD do overworld).

---

## 6. Pipeline de implementacao (agentes)

1. **lead-game-designer**: fecha as pendencias finas do paragrafo 5 (propostas -> AskUserQuestion ao criador).
2. **engine-graphics-programmer**: implementa a BattleScreen em `app/` (cena, render side-view, overlay de cartas, numeros flutuantes, log, icones de intent), lendo o estado do motor `domain/combat/` e enganchando no barramento de eventos (`core/events/`) por CombatEnded/PlayerBus.
3. **qa-engineer**: plano de teste da tela (loop overworld->batalha->volta; legibilidade dos feedbacks).

A regra canonica: o motor (`domain/`, `core/`) permanece POCO puro; a BattleScreen vive em `app/` e so LE o estado + drena a lista de mudancas, sem framework no dominio.
