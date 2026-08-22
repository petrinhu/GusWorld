# Animação de Locomoção (GusWorld)

**Status:** Canônico - decisão do líder 2026-06-22. Spec enxuto de produção. NÃO reabrir: registra a decisão tomada, não oferece alternativas.

**Cross-ref:** [combat.md](combat.md) (FSM de combate, timing por turno; locomoção é o overworld que precede a batalha). Pillar 3 (Triângulo de hardware é a interface), em `docs/design/pillars.md` §"Pillar 3".

**Convenção de escrita:** pt-br. Termos de game-dev no original (idle, walk, run, ping-pong, additive, data-driven, sprite, tile). Sem em-dash; usa hífen, vírgula, parênteses, dois-pontos.

---

## 1. Decisão

Locomoção top-down em **4 direções puras**: Sul, Norte, Leste, Oeste. Sem diagonais animadas. Estilo de cadência minimalista (poucos frames), suavizável depois sem retrabalho de código.

As 4 direções são **desenhadas à mão, uma a uma, em TODOS os personagens** (party, antagonistas E NPCs). **Proibido espelhamento / flip horizontal** para gerar Leste a partir de Oeste (ou vice-versa).

> **Motivo canônico (Pillar 3):** o hardware do personagem é assimétrico. Os óculos táticos, a Matriz Ortodôntica (antena UHF/VHF do Gus) e os aparatos laterais ficam de UM lado. Um flip horizontal trocaria o lado do aparato (antena no lado errado, olho do óculos invertido), quebrando a leitura diegética do Triângulo de hardware. Por isso Leste e Oeste são desenhos distintos, nunca o mesmo desenho refletido.

---

## 2. Estados, poses, direções e custo de desenho

Dois estados nesta spec: **idle** (parado) e **walk** (andando). Cada direção tem o seu conjunto próprio de poses.

A pose **neutra** (pernas juntas) é **UMA SÓ** por direção: serve de idle E entra duas vezes no ciclo de walk (ping-pong). Ela é desenhada uma vez e **reusada**.

| Estado | Poses por direção | Quais | Desenhos NOVOS por direção | Status |
|---|---|---|---|---|
| **idle** | 1 | neutro (pernas juntas) = frame congelado | 0 (é a mesma pose neutra do walk) | **MÍNIMO AGORA** |
| **walk** | 3 distintas (4 no ciclo tocado) | neutro, perna-direita-à-frente, perna-esquerda-à-frente | **2** (perna-dir, perna-esq); o neutro vem do idle | **MÍNIMO AGORA** |
| idle (respiração) | 2 | neutro + variação leve de respiração | +1 por direção | FUTURO FASEÁVEL (additive, sem retrabalho) |

**Custo de desenho por personagem (MÍNIMO AGORA):** 2 poses novas de walk × 4 direções = **8 desenhos de walk**, mais a pose neutra de cada direção (4) que cobre idle e walk ao mesmo tempo. Total prático: **~8 a 12 desenhos por personagem** (8 walk + 4 neutros, sendo os 4 neutros compartilhados idle/walk).

**Respiração do idle (FUTURO):** vira idle de 2 frames (neutro + variação), tocado como camada additive. É só somar 1 desenho por direção e 1 entrada de dado; **zero código novo** (ver §5).

---

## 3. Naming de arquivos e montagem do ping-pong

### Naming sugerido (placeholder; tamanho/tile = pendência §7)

Padrão: `{personagem}_{estado}_{direcao}_{pose}`.

| Arquivo | Papel |
|---|---|
| `gus_idle_sul` | idle Sul (= pose neutra Sul, reusada no walk) |
| `gus_idle_norte` | idle Norte |
| `gus_idle_leste` | idle Leste |
| `gus_idle_oeste` | idle Oeste |
| `gus_walk_sul_dir` | Sul, perna direita à frente |
| `gus_walk_sul_esq` | Sul, perna esquerda à frente |
| `gus_walk_norte_dir` / `gus_walk_norte_esq` | Norte |
| `gus_walk_leste_dir` / `gus_walk_leste_esq` | Leste |
| `gus_walk_oeste_dir` / `gus_walk_oeste_esq` | Oeste |

O neutro do walk NÃO é um arquivo novo: a sequência aponta para `gus_idle_{direcao}` quando precisa do neutro.

### Como o ping-pong monta a sequência tocada

Para cada direção, a sequência de walk é montada a partir das 3 poses (neutro vem do idle), tocando em ping-pong (vai-e-volta), com o neutro entre os extremos:

```
neutro -> perna-direita -> neutro -> perna-esquerda -> (repete)
```

Exemplo Sul:

```
gus_idle_sul -> gus_walk_sul_dir -> gus_idle_sul -> gus_walk_sul_esq -> (loop)
```

São 4 quadros TOCADOS por ciclo a partir de 3 desenhos (2 novos + o neutro reusado). O parado é o mesmo ciclo congelado no quadro `gus_idle_{direcao}`.

---

## 4. Timing por distância percorrida (não por tempo)

A troca de pose é disparada pela **distância andada**, não por um timer de relógio. Isso elimina o pé patinando ("foot slide") no movimento livre.

| Parâmetro | Valor (tile 16 px) | Regra |
|---|---|---|
| Passo de troca (walk) | trocar de pose a cada **~8 px** percorridos | metade do tile; cada ciclo completo cobre 1 tile |
| Passo de troca (run) | **~10-12 px** por troca | passada MAIS LONGA, não mais rápida |
| Escala | proporcional ao tile | se o tile mudar de 16 px, escalar os 8/10-12 px na mesma proporção |

**Run** (correr, Shift) NUNCA troca de frame mais rápido que o walk: a passada é mais comprida (avança ~10-12 px entre trocas em vez de ~8 px). Isso mantém o pé "colado" no chão na velocidade alta e evita o efeito de personagem deslizando.

Personagem parado: distância percorrida = 0, logo nenhuma troca acontece e o sprite fica no quadro neutro (idle congelado).

---

## 5. Requisito de arquitetura: animação data-driven

**Requisito canônico do futuro módulo de animação (locomoção):** a definição de "quais frames cada estado/direção toca, em que ordem, e a cada quantos px troca" é **DADO (config), não código**.

| Princípio | O que exige |
|---|---|
| Frames por estado/direção = dados | a lista de poses de cada (estado, direção) é uma tabela/config carregada, não um `switch` no código |
| Ordem de play = dados | o padrão ping-pong (e sua sequência) é descrito nos dados; o código só lê e toca |
| Passo de troca = dado | os ~8 px (walk) e ~10-12 px (run) são valores de config, escaláveis com o tile |
| Faseamento sem quebrar | aumentar walk de 3 -> 4 -> 6 poses, ou idle de 1 -> 2 (respiração), é **só editar/adicionar dados**: nenhum código reescrito, nada quebra |

**Consequência:** começamos no MÍNIMO de frames (idle 1, walk 3) e a suavização futura (mais poses, respiração additive, run dedicado) entra como **dados novos**, sem tocar a lógica do módulo. O módulo de animação deve ser desenhado para ler essa config desde o primeiro commit, mesmo enquanto só existe o placeholder.

---

## 6. Placeholder piscante (validador de cadência no M1/locomoção)

Antes de qualquer sprite existir, o personagem é um **retângulo placeholder** (já entregue no M1). Esse retângulo DEVE **piscar entre 2 cores a cada ~8 px percorridos**, exatamente no passo de troca do walk.

- Objetivo: validar a **cadência** (o "feeling" do passo por distância) ANTES de desenhar sprites.
- O piscar usa o MESMO disparo por distância da §4 (não um timer): andou ~8 px, alterna a cor.
- Parado: não pisca (distância 0).
- Quando os sprites chegarem, o mesmo disparo por distância passa a trocar as poses no lugar de alternar cores: a cadência já estará validada e idêntica.

---

## 7. Pendências futuras (explícitas, fora do escopo deste doc)

| Pendência | Quando / com quem |
|---|---|
| **Tamanho de sprite e do tile em px** | definir com `art-director` ao produzir o 1º sprite. **Assumido 16 px** nesta spec; se mudar, reescalar os números de timing (§4) na mesma proporção |
| **Run dedicado** (poses próprias de corrida) | fase posterior. Hoje run reusa as poses de walk com passada mais longa (§4). Poses próprias de run viram dados adicionais (§5) |
| **Respiração do idle** (idle 2-frame additive) | fase posterior. Soma 1 desenho por direção + 1 entrada de dado; zero código novo (§2, §5) |
| **Outros estados** (ex.: interação/usar, pulo, carregar, reação) | FORA deste doc. Esta spec cobre SÓ idle e walk. Estados novos serão specados à parte quando entrarem no roadmap |
