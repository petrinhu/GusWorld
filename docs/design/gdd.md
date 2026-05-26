# GusWorld — Game Design Document (1-page)

Versão: 0.1 (pré-produção) — escopo solo G1, Godot 4, PC Linux+Windows, single-player.

---

## 1. Tagline

RPG turn-based de prodígio-hacker de 11 anos que compila feitiços como código e prevê inimigos como xadrez, entre uma metrópole ciber-gótica e uma floresta tecnorgânica regida por matemática.

---

## 2. Pillars

Detalhe completo em [[pillars]]. Resumo decisório:

1. **Lógica vence força** — SIM turn-based + predição; NÃO QTE/twitch/HP-sponge.
2. **Magia é software, natureza é matemática** — SIM cartas com sintaxe + fractais visíveis; NÃO RNG opaco / lore místico.
3. **Triângulo de hardware é a interface** — SIM upgrades em Óculos / Matriz / Tavus-Drive; NÃO habilidade sem origem técnica.
4. **Prodígio de 11 anos, não herói adulto** — SIM tom analítico / vulnerável / curioso; NÃO power-fantasy adulta / edgy.
5. **Contraste bipartido cidade × floresta** — SIM paleta/áudio/mecânica distintos por lado; NÃO bioma híbrido genérico.

---

## 3. Aesthetic alvo (MDA — LeBlanc)

Ordem de prioridade:

1. **Challenge** — puzzle/combat exigem leitura de sistema, não reflexo.
2. **Discovery** — Selve Sombria como espaço de leis matemáticas a decifrar; cidade como rede de segredos a hackear.
3. **Expression** — montagem de deck rúnico e build de Gus é assinatura do jogador.

Secundárias toleradas (não decisórias): Narrative, Fantasy. Ignoradas: Sensation, Fellowship, Submission.

---

## 4. Core loop (30s-2min)

```
SCAN (óculos) → IDENTIFICAR padrão/inimigo/puzzle
   ↓
PLANEJAR (matriz amplifica info: range, fraqueza, próximo movimento)
   ↓
EXECUTAR (Tavus-Drive: jogar carta, prever vetor, resolver puzzle)
   ↓
FEEDBACK visível (<150ms): grid holográfico atualiza, inimigo telegrafa, recurso muda
   ↓
PROGRESS (XP de análise, carta nova, fragmento de lore, upgrade de hardware)
   ↺ repeat
```

Verb dominante: **decifrar**. Sub-verbos: scanear, compilar, prever.

---

## 5. Meta loop

- **Sessão (30-60min):** atravessar 1-2 nós do bioma atual, resolver 1 puzzle-âncora + 2-3 encontros de combate, ganhar 1-2 cartas novas ou 1 fragmento de upgrade de hardware.
- **Sessão-a-sessão:** desbloquear nova carta-tipo, novo módulo de hardware (upgrade em 1 dos 3 vértices), nova área (capítulo).
- **Long-term (4-8h total):** completar 6 capítulos, montar 2-3 decks especializados, decifrar a verdade-âncora da Selve Sombria, enfrentar boss final que exige uso coordenado dos três vértices do hardware.

Sem grind: progressão é linear-narrativa, não farmável. Replay opcional via builds alternativas de deck.

---

## 6. Mecânicas-âncora (3, integradas a combat + puzzle + aventura)

### 6.1 Sintonização Ortodôntica (scan/range)

**Verb:** scanear. **Vértice:** Matriz Ortodôntica amplifica os Óculos.

Em **combate turn-based**, gastar 1 ponto de ação ativa scan: revela HP, fraqueza elemental, próximo movimento telegrafado, e — com upgrades — buffs/debuffs ativos do inimigo. Sem scan, o jogador joga às cegas; com scan, jogo vira xadrez de info completa parcial. Trade-off: ação gasta em scan é ação não gasta em ataque.

Em **puzzle/aventura**, scan revela padrão matemático do bioma (Fibonacci nas árvores, função recursiva no pulso bioluminescente, harmônica no canto de criatura). Decifrar o padrão abre passagem, identifica caminho seguro, ou expõe "bug" (inimigo-vírus) escondido.

Upgrade-tree no vértice Matriz: ganho de range (escuta ecolocalização de longe), filtros (ignora ruído eletromagnético da cidade), e modo "sonar maxilar" passivo que destaca anomalias no minimapa. Pillar servido: 1, 2, 3.

### 6.2 Compilação de Deck Rúnico (TCG-style combat)

**Verb:** compilar/jogar. **Vértice:** Tavus-Drive executa.

Em **combate turn-based**, Gus tem deck de 40-60 cartas totais, 15 em campo, rúnicas. Cada carta = `tipo + modificador + alvo` (sintaxe diegética legível: ex. `RAIZ.elétrica → área-3x3`). Por turno: saca 3, joga 1-2 dependendo de mana (recurso de compilação). Cartas combinam: `Código de Raiz` + `Pulso Elétrico` = armadilha elétrica em área; combos são receitas determinísticas, não RNG.

Deck é construído fora de combate. Cartas obtidas por progresso narrativo (não loot drop aleatório). Trade-off: deck especializado (4 cópias da mesma carta) vs versátil (variedade).

Em **puzzle/aventura**, cartas têm uso ambiental: `Código de Raiz` abre caminho em parede orgânica, `Pulso Elétrico` ativa terminal corrompido. Mesma carta resolve combate e puzzle — recompensa duplo-uso.

Anti-degeneração: sem carta "win-button"; mana cresce no turno limitando combos premium ao mid-late. Pillar servido: 1, 2, 3.

### 6.3 Vetores do Gambito (predição xadrez tática)

**Verb:** prever. **Vértice:** Óculos Táticos projetam.

Em **combate turn-based**, após scan, o jogador pode gastar 1 ponto de ação para sobrepor um tabuleiro holográfico no campo: vê trajetória prevista do inimigo no próximo turno (alvo, dano, área). Com upgrade, vê 2-3 turnos à frente para um inimigo escolhido. Alternativa: gastar 1 ponto para **forçar recuo** ou **redirecionar vetor** de um inimigo, sacrificando dano no turno atual.

Em **puzzle/aventura**, mesmo tabuleiro decifra movimento de patrulhas (stealth opcional na cidade) e prevê trajetória de projéteis ambientais na Selve (pólen-vírus, esporos balísticos).

Sinergia com 6.1 e 6.2: scan dá input, gambito dá output, deck é a ferramenta. Os três vértices do hardware se manifestam aqui em ciclo. Anti-degeneração: prever 100% destrói desafio — predição custa AP, falha contra inimigos com "intent caótico" (mini-bosses específicos), e party de 3 personagens limita micro-controle. Pillar servido: 1, 3.

---

## 7. Estrutura geral

- **Duração total:** 4-8 horas (campanha principal), +2h se completar todos os puzzles opcionais.
- **Capítulos:** 6 (alternando cidade ↔ Selve ↔ cidade ↔ Selve ↔ híbrido-set-piece ↔ boss final).
- **Biomas:** 2 macro (Megacidade ciber-gótica, Selve Sombria tecnorgânica) com 3 sub-zonas cada.
- **Party:** 3 personagens em combate (Gus + 2 companions desbloqueados ao longo da campanha; companions ainda a desenhar — placeholder para narrative-designer).
- **Progressão qualitativa:**
  - Cap 1-2: Gus solo, 1 vértice ativo (Tavus-Drive básico, ~10 cartas). Tutorial diegético.
  - Cap 3-4: 2 companions, 2 vértices, ~20 cartas, primeiros upgrades de hardware.
  - Cap 5-6: triângulo completo, deck-building real, boss exige coordenação dos três vértices + party.

Sem level numérico estilo "Gus Lv 47". Progressão por **módulos de hardware** + **cartas desbloqueadas** + **fragmentos de lore decifrados**. Curva de poder: sigmoid (lenta-rápida-platô) para evitar power-creep no late.

---

## 8. Métrica de sucesso

**Time-to-fun ≤ 5 minutos.** Em playtest N≥10, jogador novo precisa ter executado o core loop completo (scan → compilar → prever → resolver 1 encontro) em até 5 minutos de gameplay real. Se falhar, onboarding é refeito.

Secundárias (não bloqueantes): "would play again" ≥70%, taxa de conclusão da campanha ≥40% em primeira run.

---

## 9. Cortes explícitos (NÃO está no escopo G1)

- Multiplayer / co-op / PvP.
- Open-world / mundo persistente.
- Crafting profundo / sistema econômico de cidade.
- Romance / morality system / múltiplos finais (1 final + variantes mínimas).
- Voice acting (apenas texto).
- Mocap / cutscenes pré-renderizadas (usar in-engine).
- Dynamic difficulty adjustment (oferecer 2 modos fixos: Padrão + Analítico-hard).
- Sistema de companions com afinidade / dating-sim mechanics.
- Mais de 2 macro-biomas.
- Crafting de cartas (cartas são obtidas, não craftadas).
- DLC / season pass / live-service.
- Console port no MVP (Linux+Windows apenas).
- Tradução além de pt-br + en-us.
- Tutorial wall-of-text / cinemática de abertura > 90s.
- Mais de 8h de campanha (escopo G1 mata).
