<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Overclock: medidor de risco comprado, não concedido

**Tipo Diátaxis:** Explanation (design). **Audiência:** design/engenharia deste projeto. **Status:** PROPOSTA de design, não canon fechado; aguarda ratificação do líder. **Autoria da ideia:** Gus Dragon (colaborador humano, filho do líder), aprovada pelo líder em 06/09/2026 (L-07, L-16, L-34 de `GODS_LAWS.md`). **Owner:** `lead-game-designer`.

Cross-ref: `docs/design/pillars.md` (Pillar 1, Pillar 3), `docs/design/mecanicas/modos-morte.md` (os quatro modos de dificuldade), `docs/design/mecanicas/cartas-hardware-pirataria-energia.md` §3 e §5 (originalidade da carta, bateria, tensão de pico), `docs/design/mecanicas/cartas/_vocabulario.md` §5 e §8 (`Shield`, `trigger`), `docs/design/mecanicas/cartas/urandom.md`, `docs/design/mecanicas/combat.md` §11 (canal FALHA/CRIT/COMUM), `docs/design/mecanicas/puzzle-gambito.md` (mecanismo irmão de mitigação de RNG, para não confundir os dois).

---

## 1. A ideia, na origem

Gus Dragon propôs uma função para as dificuldades altas que ajude levemente o jogador, com um custo: ele se arrisca mais (por exemplo, jogando cartas de efeito sorteado em vez de garantido) e em troca ganha uma vantagem temporária. A inspiração dele é um medidor de fúria de outro jogo, que enche conforme o jogador se expõe.

## 2. Por que isto não fere o corte C-06 (`GODS_LAWS.md`, L-29 do projeto)

O corte proíbe **dificuldade dinâmica adaptativa**: o jogo nunca lê o desempenho do jogador e ajusta a dificuldade em resposta a ele (nada de o jogo ficar mais fácil sozinho porque o jogador está morrendo muito, nem mais difícil porque está indo bem demais).

O Overclock não olha para o desempenho do jogador em nenhum momento. Ele lê uma única coisa: **se o jogador escolheu, com informação completa e antes do resultado, jogar uma carta cujo efeito é sorteado em vez da garantida**. Essa escolha é do jogador, feita à vontade, e o medidor responde à escolha, não ao resultado dela: carregar não depende de a jogada sair bem ou mal (item 4). A dificuldade do save continua fixa, escolhida uma vez, nunca reescrita pelo sistema (`modos-morte.md` §2.2). O Overclock é uma ferramenta tática **dentro** de um combate de uma dificuldade já fixa, não um mecanismo que muda essa dificuldade.

**Este parágrafo é o primeiro alvo de qualquer auditoria futura, e deve ser reconferido junto com qualquer alteração desta spec.**

## 3. Levantamento: existe carta de efeito sorteado em número suficiente?

A pista do Gus Dragon é boa, mas a resposta exata importa para o desenho.

**Por design fixo, existe exatamente 1 carta cujo efeito em si (não só a magnitude) é sempre sorteado entre resultados distintos: `urandom`** (`docs/design/mecanicas/cartas/urandom.md`), que sorteia por peso uma faixa (fraco/médio/forte/especial, e na versão pirata também um resultado de falha) e redireciona a execução para uma carta já existente no catálogo. Ela é `tier: especial`, uma cópia no jogo inteiro (`_vocabulario.md` §9): sozinha, não é gatilho suficiente para uma mecânica que precisa disparar com regularidade num combate inteiro.

**Mas a varredura achou algo maior que uma carta isolada: o próprio sistema de pirataria de cartas já universaliza a dualidade "efeito garantido (original) contra efeito sorteado (pirata)" a qualquer carta do catálogo.** O princípio canônico de `cartas-hardware-pirataria-energia.md` §3 fixa que toda carta cruza duas dimensões, originalidade e bateria, e que a variação nasce de dado, nunca de comportamento fixo para um efeito só. O exemplo já especificado ali (a cura da curandeira) mostra a forma exata que o Gus Dragon descreveu: **original cura em magnitude fixa e drena fração fixa da carga; a cópia pirata cura em faixa RNG e drena fração RNG maior.** O mesmo princípio se estende, por desenho, à COMUM, à ESPECIAL e à SUPER (`cartas-hardware-pirataria-energia.md` §2 a §15): qualquer carta do jogo, na sua cópia pirata, tem o efeito puxado para uma faixa de sorte mais larga que a original, mais o risco de falha por entrega insuficiente (fumaça e faísca, drena a bateria inteira, `cartas-hardware-pirataria-energia.md` §5 item 10).

**Conclusão do levantamento:** o gatilho natural não é "jogar `urandom`" (só ela), é **jogar qualquer carta na variante pirata**, contra a mesma carta na variante original. Isso está disponível hoje, em canon, para potencialmente as ~25 cartas do catálogo extraído (`docs/design/mecanicas/cartas/_INDEX.md`), sempre que o jogador optar por craftar, comprar ou equipar a cópia em vez do original. `urandom` continua como o caso mais puro da categoria (ela é assim mesmo na sua versão original), e serve de exemplo pedagógico do sistema, mas não é a base numérica dele.

## 4. Como o medidor enche

**Gatilho:** o medidor de Overclock de um combatente sobe um degrau sempre que **esse combatente joga uma carta na variante pirata** (`cartas-hardware-pirataria-energia.md` §3, coluna "Pirata (clone)"), disparado no momento `OnCast` (`_vocabulario.md` §8, o mesmo gatilho que os `@effect` das cartas já usam).

**Independente do resultado.** O medidor sobe pela aposta em si, não pelo desfecho dela: sobe tanto se a bateria entrega o pico pedido (efeito sai inteiro, dentro da faixa RNG mais larga da pirata) quanto se falha por entrega insuficiente (a carta não funciona direito e a bateria drena por inteiro, `cartas-hardware-pirataria-energia.md` §5 item 10). É a exposição ao risco que enche o medidor, não a sorte de ter dado certo; se só o sucesso enchesse, o medidor recompensaria sorte prévia em vez de coragem de escolha, e deixaria de ser uma aposta genuína.

**Por combatente, não por party.** Cada personagem carrega o próprio medidor, no mesmo espírito de "cada carta gasta a própria bateria" e do Knowledge por ator: quem se arrisca é quem acumula, e a party não herda o risco de ninguém.

**Não conta jogar a versão original**, mesmo que ela role o canal FALHA/CRIT/COMUM normal do combate (`combat.md` §11). Aquele sorteio é universal a todo ataque e não é uma escolha adicional do jogador; o que enche o Overclock é especificamente a decisão de trocar o efeito garantido pelo sorteado, contra a alternativa que o jogador tinha disponível na mão.

## 5. O que ele dá

**Uma face só, para não inchar a mecânica** (o pedido do Gus Dragon foi "ajude levemente"; três vantagens simultâneas, ou um menu de escolha entre elas, transformaria uma aposta simples numa segunda camada de otimização, o oposto de leve).

**Efeito escolhido: redução de dano recebido, via o status `Shield` que o vocabulário de cartas já define** (`_vocabulario.md` §5: "absorve dano até um teto"). Ao descarregar o medidor cheio (ação dedicada, ver §6), o combatente recebe `Shield` por tempo limitado, aplicado como `ApplyStatus` (`_vocabulario.md` §4) igual a qualquer carta faria.

**Por que esta face, e não as outras duas que o Gus Dragon também sugeriu:**

- **Contra "aumentar dano causado":** dano extra é bom em qualquer situação, o tempo todo; uma mecânica assim tende a virar rotina de otimização de DPS ("sempre vale a pena"), o oposto de uma aposta situacional. Fere também, ainda que de leve, o Pillar 1 ("nunca por dano bruto").
- **Contra "gastar menos energia":** a economia de bateria já é o eixo central do sistema de pirataria (§3 acima); reduzir custo justamente na mesma moeda que a aposta arrisca cria um ciclo fechado que se autoalimenta (arrisco bateria para gastar menos bateria), difícil de calibrar sem virar bola de neve.
- **A favor de `Shield`:** ele responde diretamente ao que a punição das dificuldades altas ataca. O Difícil pune HP=0 com respawn a stats quase-zero; o Hardcore pune com o kernel-panic puzzle como última chance e permadeath na falha (`modos-morte.md` §1, §2.3a). Nos dois modos, o recurso mais escasso do jogador é sobreviver ao próximo golpe, não causar mais dano nem economizar carga. `Shield` mitiga exatamente esse ponto de ruptura, e só ajuda quando o jogador de fato leva dano, o que o mantém situacional em vez de sempre-bom.
- **Reaproveitamento em vez de mecanismo novo (DRY, regra dos 3 de `docs/arquitetura-principios.md`):** `Shield` já existe no vocabulário, com regra de empilhamento própria (`_vocabulario.md` §6) e uso conhecido por outras cartas; não nasce um status inédito só para esta mecânica.

## 6. Quanto dura, e como esvazia

Um medidor que só enche vira recurso guardado; a régua do Gus Dragon pede leveza e risco de verdade, e os dois falham se o jogador puder acumular Overclock indefinidamente até o momento perfeito.

- **Teto do medidor:** um número fixo de degraus (proposta de forma, não de valor: NÚMERO PENDENTE DE MEDIÇÃO, `economy-designer` mais playtest). Ao atingir o teto, o medidor não sobe mais até ser descarregado.
- **Descarregar é ação dedicada, não automática.** Uma ação nova, `DischargeOverclock`, custando AP (NÚMERO PENDENTE DE MEDIÇÃO), disponível só com o medidor no teto, consome o medidor por inteiro e aplica `Shield` no próprio combatente por um número fixo de turnos (NÚMERO PENDENTE DE MEDIÇÃO) com um teto de absorção proporcional ao investimento em risco (NÚMERO PENDENTE DE MEDIÇÃO). Gastar AP nisso compete com gastar AP em atacar ou defender no mesmo turno, o que mantém a decisão tática, não gratuita.
- **Drenagem passiva pelo mesmo princípio que já rege bateria no jogo.** Se o medidor não é descarregado, ele perde carga sozinho ao longo dos turnos ("toda bateria perde carga com o tempo", decisão do líder de 02/09/2026, `cartas-hardware-pirataria-energia.md` §5 item 12): um degrau a menos por turno em que o combatente não jogou nenhuma carta pirata (taxa de decaimento, NÚMERO PENDENTE DE MEDIÇÃO). Isso empurra o jogador a usar o que acumulou, em vez de guardar para sempre, e reaproveita uma regra que o próprio universo do jogo já explica ao jogador (nenhum conceito novo de fricção a ensinar).
- **O `Shield` recebido, por sua vez, dura por tempo limitado e não empilha com ele mesmo além de `Refresh`** (`_vocabulario.md` §6): descarregar de novo enquanto já existe um `Shield` de pé no combatente renova a duração dele, não soma o teto de absorção, para não permitir empilhamento sem limite por spam de cartas piratas.

## 7. Em quais dificuldades existe

**Proposta: Difícil e Hardcore, não Fácil nem Médio.** Decisão de design, sujeita a confirmação do líder (o Gus Dragon sugeriu "a mais alta, ou a seguinte, dependendo do balanceamento").

Razão: em Fácil, morrer custa um reload puro; em Médio, custa uma dívida ou um safe mode de HP grátis (`modos-morte.md` §1). Nos dois, a rede de segurança já é generosa, e oferecer ali uma ferramenta a mais para não morrer dilui o que diferencia esses modos dos dois seguintes, sem servir a nenhuma dor real do jogador nesses níveis. Em Difícil, a punição (respawn a stats quase-zero, recuperação por marcos) e em Hardcore (permadeath na falha do puzzle) são severas o bastante para que uma ferramenta ativa de mitigação, comprada com risco real, tenha peso tático genuíno: é exatamente onde "ajude levemente, com um custo" faz diferença sentida. Restringir só a Hardcore, como o Gus Dragon também cogitou, é alternativa válida e mais conservadora; **decisão final entre as duas cabe ao líder, com o playtest da onda de balanceamento decidindo se o Difícil também precisa dela.**

## 8. Informação visível (Pillar 1)

Antes de confirmar `DischargeOverclock`, a interface mostra o teto de absorção e a duração que o `Shield` resultante vai ter (quando a camada `present/` existir, `L-27`). Antes de jogar uma carta pirata, a interface já mostra a faixa RNG mais larga da pirata contra a garantia da original (herdado do sistema de pirataria, não desta spec). Nada aqui é opaco: bate com "RNG calibrado, com % visível" e a anti-mecânica explícita do Pillar 1, "aleatoriedade que pune skill sem mitigação". O Overclock é, por desenho, a mitigação: quem se arrisca de olhos abertos ganha uma rede de segurança comprada, quem não se arrisca não perde nada por isso.

## 9. Risco de degeneração e como a régua se protege

- **Se usar carta pirata fosse igualmente bom com e sem o Overclock, não haveria aposta.** A pirataria já carrega risco de verdade por si só (falha por entrega insuficiente drena a bateria inteira, `cartas-hardware-pirataria-energia.md` §5 item 10); o Overclock não cria esse risco, ele dá uma razão a mais para aceitá-lo. O risco de origem já existe e é independente desta spec.
- **Se `Shield` for forte demais, "jogar pirata sempre" vira a jogada dominante nas dificuldades altas**, o que fecha exatamente o buraco que a régua do Gus Dragon veda ("levemente"). O teto de absorção e a duração precisam nascer pequenos na primeira calibração e crescer só se o playtest mostrar que ninguém usa a mecânica (sintoma do oposto: fraca demais para valer o risco).
- **Se a drenagem passiva for lenta demais, o medidor vira poupança**, guardado até o momento ideal, o que apaga a pressão de decisão a cada turno. A taxa de decaimento é o parâmetro mais sensível desta spec e o primeiro candidato a errar na calibração inicial.

## 10. Pontos abertos

1. **Difícil também recebe a mecânica, ou só Hardcore?** (§7) Decisão do líder.
2. **Teto de degraus do medidor, custo de AP de `DischargeOverclock`, duração e teto de absorção do `Shield` resultante, e taxa de decaimento passivo** (§6): todos NÚMERO PENDENTE DE MEDIÇÃO, para a onda de balanceamento (`economy-designer` mais playtest), no mesmo espírito de todo outro número desta fase do projeto (`cartas-numeros-proposta.md`, `modos-morte.md` §5).
3. **Nome de exibição ao jogador.** "Overclock" é o termo de engenharia usado neste documento (regra de nomenclatura de TI em inglês, `cartas-hardware-pirataria-energia.md` §4); o nome que aparece na UI, e a copy que explica o sistema sem parede de texto (corte `C-16`), são trabalho de `ux-writer`/`narrative-writer`, fora do escopo desta spec de design.
4. **Se o medidor de um companion sobe também com cartas piratas equipadas por ele** (§4, "por combatente"): a leitura natural desta spec é sim, mas nenhuma decisão do líder cobre esse caso especificamente ainda.
