<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Vocabulário compartilhado das cartas

**Tipo Diátaxis:** Reference. **Audiência:** quem lê ou escreve qualquer `.gw.card`, ou o `.md` de qualquer carta individual. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extraído do código do projeto anterior (`GusEngine/domain/include/gus/domain/cards/card_enums.hpp`, 344 linhas) sob L-01 — **conhecimento, não código**: nenhuma linha de C++ aparece abaixo, só o significado de cada termo.

> Este documento **não pertence a nenhuma carta**: é o átomo de vocabulário que todas as 25 compartilham (L-33, "documento é átomo de assunto"). Cada carta aponta para cá em vez de repetir a definição de cada termo (L-30).

## 1. Por que este vocabulário existe

O projeto anterior modelava cada carta como um **programa**: uma lista ordenada de instruções (`EffectSpec`) que o motor de combate executa em momentos específicos do turno. Este documento descreve o vocabulário desse programa — os nomes que aparecem nos blocos `@effect` de cada `.gw.card` (`FORMATO.md` §5) — sem descrever o motor que os interpreta, porque o motor não existe neste projeto (L-01).

## 2. Família (`family`)

A identidade elemental de uma carta, usada pela roda de fraqueza do sistema de combate (fora do escopo desta extração — ver `combat.md` §6 quando existir neste projeto). Seis valores:

- **`eletrico`**, **`bioquimico`**, **`sonico`**, **`cinetico`**, **`criptografico`** — as 5 famílias que competem na roda de fraqueza (fraco/resistente/imune contra as outras).
- **`universal`** — família que **não** compete na roda: o multiplicador de fraqueza é sempre neutro, sem fraco/resistente/imune. No legado, cobre as cartas dos mestres que não têm origem eletromagnética clara (a maioria dos matemáticos, cientistas da computação e economistas) mais os efeitos utilitários que já eram "de qualquer família" (Shield, Regen, Haste, Slow). As ~7 cartas de domínio eletromagnético (Faraday, Maxwell, Tesla, Volta) ficam em `eletrico`; as ~13 restantes, em `universal`.

## 3. Tipo-base diegético (`base_type`)

A gramática narrativa "tipo.família" de uma carta (a forma física do conjuro, não o efeito). Cinco valores: `pulso`, `raiz`, `eco`, `fenda`, `glifo`. Nas 5 comuns, cada valor descreve a natureza do efeito (`pulso` = impacto direto, `raiz` = crescimento/cura, `eco` = ressonância, `fenda` = abertura/exploit). Nas 20 especiais, todas usam `glifo` (carta-programa não-elemental) — o próprio código-legado marca isso como `//PLAYTEST`, por não existir uma convenção fechada de tipo-base para cartas especiais.

## 4. Tipo de efeito (`kind`, dentro de `@effect`)

Os 14 tipos de efeito que uma carta especial pode declarar. Cada um é um **marcador de vocabulário**: o significado exato de `magnitude`/`percent`/`duration` muda por tipo, e o `.md` de cada carta que usa um tipo específico explica a leitura para aquele caso.

| `kind` | O que significa, em prosa |
|---|---|
| `ApplyStatus` | aplica um `status` (§5) a um alvo, com a duração e a regra de empilhamento (§6) declaradas |
| `Leech` | drena uma fração do dano causado e devolve ao conjurador como recurso |
| `Reflect` | devolve uma fração do dano recebido a quem o causou |
| `HypotenuseCombo` | quando dois aliados acertam o mesmo alvo na mesma rodada, soma os dois golpes pela raiz da soma dos quadrados (nunca a soma simples, sempre maior que cada golpe isolado e menor que a soma) |
| `CloneAlly` | aplica ao alvo um status que, ao fim de cada turno próprio do portador, reaplica uma fração do último golpe de dano dele |
| `RepeatLastAction` | ecoa o dano da última ação de dano de qualquer aliado nesta rodada, a uma fração e com uma chance declaradas |
| `ChainDamage` | depois do dano no alvo primário, salta para os próximos inimigos vivos, retendo uma fração do dano a cada salto |
| `DelayAction` | reordena a fila de iniciativa: empurra o alvo para o fim da fila da rodada (ou o adianta, no modo-benefício em aliado) |
| `DamageQuantize` | substitui a variação contínua de dano por degraus fixos, com chances declaradas para cada degrau |
| `RevealIntent` | aplica um status de vidência no conjurador e revela, sem consumir turno nem sorteio, a intenção prevista de cada inimigo vivo |
| `DiversityBonus` | concede um bônus escalonado de dano (e reduz a chance de falha) ao lado inteiro quando os aliados agem de formas diferentes na mesma rodada |
| `ApEfficiency` | efeito duplo: concede pontos de ação extras ao portador, e aplica um atraso/desconto a inimigos com uma marcação específica |
| `TokenRefund` | a primeira carta especial ativa/híbrida jogada na batalha não consome o uso único dela |
| `RandomRedirect` | sorteia, por peso, um efeito entre vários já existentes no catálogo e redireciona a carta para ele |

## 5. Identidade de status (`status`)

Um status é um efeito temporário aplicável a qualquer combatente. Os primeiros 13 (`Stun` a `Slow`) são o conjunto genérico já usado pelas cartas comuns; do 13 em diante, cada um nasceu para servir a uma carta especial específica, mas nada impede reuso futuro.

| `status` | Efeito, em prosa |
|---|---|
| `Stun` | pula o turno |
| `Poison` | dano contínuo por veneno |
| `Corrode` | reduz uma defesa ao longo do tempo |
| `Disrupt` | reduz o poder da próxima ação do alvo |
| `Silence` | bloqueia o uso de cartas |
| `Knockback` | empurra o alvo, alterando a posição na fila |
| `Break` | reduz uma defesa de forma direta |
| `Expose` | aumenta o dano de carta que o alvo recebe |
| `Decrypt` | remove buffs do alvo |
| `Shield` | absorve dano até um teto |
| `Regen` | cura por turno |
| `Haste` | aumenta a velocidade de ação |
| `Slow` | reduz a velocidade de ação |
| `SobrecargaTermica` | dano contínuo com decaimento (mais forte no início) somado a uma redução de velocidade enquanto dura |
| `Resfriamento` | aumenta a velocidade e reduz o custo da próxima carta jogada |
| `Reflect` | marca o portador para devolver uma fração do dano recebido |
| `BlindagemEM` | imunidade a debuffs de origem elétrica; ao ser aplicado, também limpa os debuffs elétricos já ativos |
| `NullProof` | guarda, no portador, a capacidade de forçar o próximo golpe seu contra um alvo resistente ou imune a acertar como se fosse neutro — e se consome nesse golpe |
| `Scrying` | enquanto ativo em qualquer aliado vivo, revela a intenção prevista de cada inimigo a cada rodada |
| `Eco` | ao fim de cada turno próprio do portador, reaplica uma fração do último dano que ele causou |
| `Provocado` | reduz a defesa efetiva de quem o porta (fora do escopo desta extração de cartas: nasce de uma ação básica, não de uma carta) |

## 6. Regra de empilhamento (`stack_rule`)

O que acontece quando o mesmo status é aplicado de novo a um alvo que já o porta:

| `stack_rule` | Comportamento |
|---|---|
| `Replace` | a nova aplicação substitui a anterior por completo |
| `Refresh` | renova a duração, sem alterar nem somar a magnitude |
| `StackMagnitude` | soma a magnitude da nova aplicação à existente |
| `StackDuration` | soma a duração da nova aplicação à existente |

## 7. Filtro de lado (`side_filter`)

Restringe de que lado o alvo de um efeito precisa estar para o efeito valer. Três valores: `Any` (sem restrição — comportamento de toda carta que não declara filtro), `EnemyOnly` (só vale contra inimigo do conjurador), `AllyOnly` (só vale a favor de um aliado do conjurador, incluindo o próprio conjurador). Quando o alvo resolvido está do lado errado, o efeito **dissipa** — não é erro, não é substituído por outra coisa, simplesmente não acontece, e o motivo fica registrado (no legado, como uma linha de log).

## 8. Gatilho (`trigger`)

Quando, dentro do ciclo de combate, um `@effect` dispara: `OnCast` (no momento em que a carta é jogada), `OnDamageDealt` (quando o portador causa dano), `OnDamageReceived` (quando o portador recebe dano), `OnAllyTurnEnd` (no fim do turno de qualquer aliado), `OnRoundEnd` (no fim da rodada inteira), `Always` (nenhuma das 20 cartas extraídas usa este gatilho; existe no vocabulário para uso futuro).

## 9. Tier e categoria (`tier`, `category`)

`tier` distingue as três trilhas de raridade: `comum` (obtida por loja/craft/loot, sem exclusividade), `especial` (uma cópia no jogo inteiro, entregue por progresso narrativo — nunca comprada, craftada ou dropada), `super` (a carta-capstone, forjada a partir das 20 especiais). `category` só faz sentido quando `tier` não é `comum`, e distingue **quatro** formas de uso, todas descritas em `cartas-technomagik.md` §2.3 deste projeto (não repetido aqui, L-30):

- **`ativa`** — jogada uma vez por batalha, com custo de mana.
- **`passiva`** — sempre ligada enquanto equipada, sem custo de mana, sem consumir o uso único da batalha.
- **`hibrida`** — duas faces no mesmo objeto: uma sempre ligada e uma jogável.
- **`fora_de_combate`** — não ocupa o deck de batalha; age fora do combate (exploração, economia).

## 10. Fontes

Extraído de `card_enums.hpp` (344 linhas) e `card_records.hpp` (142 linhas) do projeto anterior, lidos via `git show` do clone read-only, sob a autorização de leitura da L-01 (o código não é base, mas o conhecimento que ele carrega é insumo válido). Nenhuma linha de C++ foi copiada; todo nome de campo e de valor foi traduzido para prosa.
