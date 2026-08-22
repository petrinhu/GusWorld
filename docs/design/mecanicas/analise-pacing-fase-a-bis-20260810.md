# ANÁLISE: Fase A-bis do estudo de pacing (re-rodada, PACING-SIM, grade grossa, só trash)

> **Status: ANÁLISE (rascunho pronto para revisão), não canon.** Nenhum número deste
> documento entra em `combat.md` nem no motor sem decisão explícita do líder. Autor:
> Capitolino (CPO), 2026-08-10. Dados: `/var/tmp/pacing_fase_a_bis_20260810.txt`
> (artefato congelado, md5 `f55ae19cbd3bf1f9efd50e2e9c824e46` confirmado em duas
> leituras com 5 s de intervalo, mtime 2026-08-10 22:36:03 -0300; 80 pontos,
> 240.000 lutas por ponto, 19,2 milhões de lutas, zero erros internos declarados em
> 80 de 80 pontos). Protocolo e pré-registros: `proposta-protocolo-simulacao-pacing.md`.
> Doc-irmão (Fase A original, 2026-08-01): `analise-pacing-fase-a.md`. Não commitado
> por decisão de processo (commit e push só com ordem do líder).

---

## Resumo executivo (em linguagem simples)

1. **O jogo de hoje está sentado exatamente no alvo.** Depois que o líder aplicou os
   statlines novos (`BAL-STATLINES-APLICAR`: vida do trash 55 para 33, ataque 10 para
   12), o statline canônico virou o centro da grade, e é ELE que aprova: os únicos 2
   pontos verdes em 80 são "vida 33, ataque 12" nos dois cenários base (vanilla e
   parede). A re-rodada não pede mudança nenhuma de número no trash base: **ela
   confirma que o conserto já feito é o vencedor.**
2. **Os 2 aprovados são os mesmos vencedores da Fase A original, reproduzidos número
   a número** (vitória 92,81% e 92,56%, janela 3-5 em 96,9%, quedas do Gus 7,2-7,4%,
   idênticos ao relatório de 2026-08-01 até a precisão impressa). O que mudou não foi
   a luta, foi a leitura: em agosto eles eram "vida cortada a 60% do canon"; hoje são
   "o jogo exatamente como está". A reprodução exata é, ela mesma, uma checagem de
   validade (seeds pareadas + mesma grade absoluta + trava de paridade da mira).
3. **O gate novo do líder funcionou como desenhado.** O falso aprovado da Fase A
   original (o antigo ponto 78: mediana 7 rodadas, só 4,39% das lutas na janela, mas
   verde na cauda) reapareceu com os MESMOS números e agora **reprova** pelo critério
   pré-registrado em 2026-08-03 (mediana em 3-5 E janela >= 60%). Isso é correção de
   metodologia confirmada nos dados, não um resultado pior do jogo.
4. **Por que 78 pontos reprovaram: a grade herdada erra quase toda por excesso.** Os
   valores absolutos de vida testados (22/33/44/55) vinham do mundo antigo (fatias de
   55); com a referência nova em 33, tudo acima dela vira esponja perigosa e tudo o
   que tem ataque baixo vira passeio. 64 pontos caíram por "seguro demais" (vitória
   98-100%), 12 por "letal demais" (vitória de 77% até 28%), 40 pela janela de
   duração, 49 por trivialidade de HP final. Detalhe na seção 4.
5. **Curandeira: nada mudou.** Cura 6, 9 e 12 seguem tornando a luta imperdível na
   vizinhança do vencedor (100% de vitória, HP final 92-95%), como na Fase A original:
   o eixo da cura continua precisando descer abaixo de 6 na Fase B. O braço P3B (motor
   real de bateria) **não consta desta rodada por ESCOPO CORRETO, não por lacuna**: a
   análise concluiu que `Scenario::P3B_TrashHealerBattery` pertenceria a
   `build_phase_b_healer_battery_grid` (linha 1628), não a `build_phase_a_grid` (linhas
   1333-1340) — P3B sempre foi um braço da Fase B/C, nunca da Fase A. Ver correção na
   seção 7.

   > ⚠️ **Não verificado contra código.** Este valor vem da análise de projeto, não de
   > medição em código executado. O código deste projeto nasce do zero (GODS_LAWS.md,
   > L-01), e a fonte que a redação original citava não existe. Revalidar quando
   > houver implementação.
6. **⚠️ RESSALVA DO QA (leia antes de usar qualquer número): a mira ponderada que o
   estudo simula ainda não está ligada no jogo.** O harness usa a mira ponderada
   (Opção F, F1+F2+F3) e há trava de paridade com a fórmula de produção, mas a análise
   assume que `pick_weighted_enemy_target` não tem nenhum call site de
   produção fora dos testes: o cérebro inimigo em jogo (`scripted_brain.cpp`) ainda
   miraria o primeiro da lista. **Estes números descrevem o jogo COM a mira planejada
   ativa, não o comportamento observável numa partida hoje.** Seção 8.

   > ⚠️ **Não verificado contra código.** Esta afirmação sobre o call site de
   > `pick_weighted_enemy_target` e sobre `scripted_brain.cpp` vem da análise de
   > projeto, não de medição em código executado. O código deste projeto nasce do
   > zero (GODS_LAWS.md, L-01), e a fonte que a redação original citava não existe.
   > Revalidar quando houver implementação.
7. **A Fase B não pode rodar como está escrita.** Ela foi ancorada na leitura antiga
   (multiplicadores da referência 55). Precisa ser re-ancorada nos vencedores desta
   rodada (fatia futura F7) antes de rodar: grade re-centrada em 33, cura estendida
   para baixo, P3B medido de verdade. Recomendação na seção 9.

---

## 1. O que esta re-rodada é, e o que mudou desde a Fase A original

A Fase A original (2026-08-01) mediu o jogo ANTES de duas mudanças grandes chegarem
ao canon/motor. O líder decidiu re-rodar a mesma grade com o estado atual antes de
prosseguir ao refino fino (Fase B). Quatro mudanças entraram entre as duas rodadas
(descritas pela análise de projeto como tendo passado por `check.sh` verde, e por
verificação e auditoria adversarial independente do harness):

> ⚠️ **Não verificado contra código.** A afirmação de que estas mudanças passaram por
> `check.sh`, foram verificadas pelo orquestrador e auditadas por QA adversarial no
> harness vem da análise de projeto, não de medição em código executado. O código
> deste projeto nasce do zero (GODS_LAWS.md, L-01), e a fonte que a redação original
> citava não existe. Revalidar quando houver implementação.

1. **Gate E2 novo, pré-registrado pelo líder em 2026-08-03:** um ponto só é candidato
   se a mediana de rodadas estiver em [3,5] E pelo menos 60% das lutas caírem na
   janela. É o conserto do furo admitido na seção 6 da análise original (o gate de
   cauda sozinho deixou passar um ponto com mediana 7). Pré-registrado antes do dado,
   como manda a disciplina; o efeito medido está na seção 5.
2. **Trava de paridade da mira:** o harness agora garante que simula a MESMA fórmula
   de mira ponderada que existe no motor (braço F_NoF4, peso F1+F2+F3, sem F4). Ver,
   porém, a ressalva da seção 8 sobre o call site de produção.
3. **Braço P3B da curandeira** (motor real de bateria: carta original, 12 HP fixos,
   dreno de 50% da carga atual, 6 usos por bateria) implementado no harness ao lado
   do P3 antigo (cura ilimitada, âncora histórica). **Nota de escopo: nenhuma célula
   P3B aparece neste artefato** (seção 7).
4. **A referência de "100% de HP" nos rótulos passou de 55 para 33** (statline novo).
   Os 4 valores ABSOLUTOS de vida testados continuam os mesmos (22/33/44/55); a
   leitura percentual virou 66,7% / 100,0% / 133,3% / 166,7% da referência nova.

**Escopo decidido pelo líder:** só o braço trash (80 pontos). O braço elite segue
congelado desde 2026-08-01 (artefato de medição: bot sem carta contra Def 14 causa
sempre 1 de dano; nada disso mudou, e o cabeçalho do run registra que o código do
braço elite continua inteiro no harness, só desligado). A Fase B não rodou nesta
rodada, por decisão dele, porque está ancorada num vencedor que esta rodada
re-etiquetou (seção 9).

## 2. Checagem de validade interna

- 80 pontos, N = 240.000 por ponto (amostra cheia, como o líder mandou), seed base
  20260801 (a mesma da rodada original), **zero erros internos declarados em 80 de
  80 pontos** (linha "erros internos: 0 de 240000" presente em todos; nenhuma linha
  de erro não-zero existe no artefato). Zero declarado, não presumido.
- **A reprodução número a número é a checagem que os dados entregaram de graça.**
  Todas as células conferidas em detalhe batem com a rodada original até a precisão
  impressa: o vencedor P1 (92,8133% / 3,88936 rodadas / janela 96,9221% / quedas
  7,18667%), o vencedor P2 (92,5642% / 96,8525% / 7,43583%), o antigo ponto 78
  (92,6317% / mediana 7 / janela 4,39375%) e até a célula mais letal (ponto 76,
  derrota 68,55125%, o mesmo 68,55% do relatório original). Com seeds pareadas e a
  mesma grade absoluta, é o comportamento esperado; se a trava de paridade da mira
  tivesse mudado a fórmula SIMULADA, esses números teriam se movido. Não se moveram:
  a luta simulada é a mesma, **o que mudou entre as rodadas é o critério de
  julgamento e a leitura, não a física do combate.**

## 3. Resultado agregado: 2 aprovados em 80

| Ponto | Cenário | HP (leitura nova) | Atk | Vitória | Mediana rodadas | Janela 3-5 | Quedas Gus | 1ª queda (med.) | HP final | E10 dano/enc. |
|---|---|---|---|---|---|---|---|---|---|---|
| 31 | P1 vanilla | 33 (**100% do canon**) | 12 | 92,81% | 4 | 96,92% | 7,19% | rodada 3 | 75,3% | 30,8 HP |
| 32 | P2 parede | 33 (**100% do canon**) | 12 | 92,56% | 4 | 96,85% | 7,44% | rodada 3 | 77,5% | 27,0 HP |

Os dois passam em TODOS os 7 guarda-corpos, incluindo o gate E2 novo (janela ~97%,
folga enorme sobre o piso de 60%). São as mesmas células vencedoras da Fase A
original; a diferença é de identidade, não de número: **na rodada original elas eram
a proposta (HP×0,60 do canon inflado); nesta rodada elas são o incumbente (100% do
canon vigente).** O estudo saiu de "o que consertar" para "o conserto está certo".

Observações que continuam valendo dos dados originais e se reproduziram aqui:

- **E1 e E4 seguem sendo a mesma régua no trash** (derrota = queda do Gus, célula por
  célula; ex.: ponto 31, derrota 7,18667% e queda do Gus 7,18667%). A faixa E1 de
  90-97% continua implicando quedas de 3-10%, e o teto E4 de 12% continua nunca sendo
  o gate que morde sozinho (as 12 células que estouram E4 também estouram E1 por
  baixo; ver seção 4).
- **O Shield do defensor segue binário** (E6 = 100% em toda a coluna P2, inclusive no
  vencedor). Pendência de REGRA com dono (líder), não deste estudo; medida, não
  perseguida.
- **E11 no vencedor: mediana de 11 golpes até o primeiro inimigo cair** (mesmo valor
  da rodada original). O alvo histórico de junho ("3-5 golpes") continua inalcançável
  por HP, como a análise original já tinha provado; as duas réguas "3 a 5" (rodadas
  vs golpes) seguem sendo perguntas diferentes.

## 4. Anatomia das 78 reprovações (o dado útil para o design)

Cada ponto reprova por um ou mais guarda-corpos (as contagens abaixo somam mais que
78 porque os gates não são exclusivos). Enumeração completa do artefato, não amostra:

| Guarda-corpo | Reprovações | Direção |
|---|---|---|
| E1 vitória fora de 90-97% | **76** | 64 por CIMA (98-100%, seguro demais) + 12 por BAIXO (77% a 28%, letal) |
| E9 HP final fora de 55-90% | **49** | 48 por cima (>90%, luta que não custa nada) + 1 por baixo (ponto 76: 50,1%) |
| E2 janela (gate novo) | **40** | mediana fora de [3,5] e/ou janela < 60% |
| E4 quedas do Gus > 12% | **12** | as mesmas 12 células que reprovam E1 por baixo |
| E8 1ª queda nas rodadas 1-2 | **11** | duas famílias, ver abaixo |
| E2 cauda (p10/p90) | **2** | pontos 76 e 77 (p10 = 1: derrota na primeira rodada) |
| E9 trivialidade (sem-dano > 40%) | **0** | nenhum ponto reprovou por aqui |

Três leituras de design saem dessa tabela:

1. **A grade herdada está descalibrada para o mundo novo, e isso era esperado.** Os
   absolutos 22/33/44/55 eram fatias de 55; com a referência em 33, a grade virou
   66%-167% do canon, quase toda fora do envelope útil. O gate dominante é E1 por
   cima (64 células): ataque 8-10 e/ou qualquer cura tornam a luta um passeio. Isso
   não é um defeito do jogo; é a grade fazendo a pergunta antiga para um jogo que já
   respondeu. A Fase B precisa re-centrar a grade na referência 33 (seção 9).
2. **A geometria do perigo confirmou a lição da rodada original, agora com nome
   novo: o perigo mora ACIMA da referência.** As 12 células letais são exatamente os
   pares P1/P2 acima do canon com ataque de médio para cima: HP 33 com Atk 14
   (quedas 22-23%), HP 44 com Atk 12 (20-21%) e Atk 14 (40-42%), HP 55 com Atk 10
   (17%), Atk 12 (51-53%) e Atk 14 (69-72%). Na rodada original isso se descrevia
   como "o meio da grade"; re-lido contra a referência nova, o padrão é mais simples:
   **esponja + ataque alto = luta longa o bastante para o dano acumular.** Variante
   de inimigo com mais vida DEVE vir com ataque contido, ou o teto de quedas estoura.
3. **E8 tem duas famílias, e uma delas é um aviso sutil sobre cura.** Das 11 células
   com primeira queda precoce, 4 são as letais óbvias (Atk 14, muitas quedas, cedo).
   As outras 7 são células P3 quase imperdíveis (quedas de 0,05% a 3,15%) onde a
   raríssima derrota que existe acontece NA LARGADA (mediana rodada 1-2): quando a
   cura torna a luta imperdível no regime normal, o único jeito de perder é ser
   atropelado antes de a cura entrar. É um perfil de morte ruim (parece arbitrário ao
   jogador) e mais um argumento contra cura alta, além da trivialização.

## 5. O gate E2 novo em ação: o antigo falso aprovado agora reprova

O ponto 78 (P3 healer, HP 55, Atk 14, cura 6) reproduziu exatamente os números da
rodada original: vitória 92,63% (verde em E1), quedas 7,37% (verde em E4), cauda
p10=7/p90=7 (verde no gate de cauda)... e mediana 7 rodadas com só 4,39% das lutas na
janela. Na Fase A original, isso ganhou o carimbo [APROVADO] porque o único gate de
duração era a cauda; foi o furo que o team-lead apontou e que esta análise admitiu na
época, com conserto proposto ANTES da Fase B. O líder pré-registrou o conserto em
2026-08-03, e nesta rodada o gate novo fez o que prometia: **[REPROVADO] E2 janela,
mediana 7 fora de [3,5], janela 4,39% abaixo de 60%.**

Registro com as mesmas três razões que me permitiram propor isso de cabeça erguida:
o furo foi apontado de fora, o conserto foi pré-registrado antes da fase seguinte, e
ele encolheu a lista de candidatos (de 3 para 2), não a protegeu. A lista de 2
aprovados desta rodada é a lista que a Fase A original SEMPRE deveria ter produzido.
O ponto 79 (cura 9, vitória 96,85%) cai pelo mesmo gate: sem o critério novo, ele
teria sido um SEGUNDO falso aprovado nesta rodada (passa em E1, E4 e cauda); o gate
novo pegou os dois.

## 6. Comparação explícita com a Fase A original

| Aspecto | Fase A (2026-08-01) | Fase A-bis (2026-08-10) |
|---|---|---|
| Grade | 116 pontos (80 trash + 36 elite) | 80 pontos (só trash; elite congelado) |
| Aprovados | 3 (sendo 1 falso aprovado admitido) | **2** (o falso aprovado agora reprova) |
| Vencedores | P1/P2 com HP 33, Atk 12 (lidos como HP×0,60) | **os mesmos**, lidos como 100% do canon |
| Números dos vencedores | 92,81% / 92,56% de vitória | idênticos até a precisão impressa |
| Gate de duração | só cauda (p10/p90); furo admitido | cauda + mediana em [3,5] + janela >= 60% |
| Referência de 100% | 55 (canon inflado de junho) | 33 (canon vigente pós-BAL-STATLINES) |
| Status do vencedor | proposta de mudança | **confirmação do canon vigente** |
| Curandeira | cura 6+ trivializa perto do vencedor | idem, reproduzido; P3B ainda não medido |
| Elite | artefato de medição, 0 aprovados | fora de escopo (congelado, código intacto) |

O que NÃO mudou: a física simulada da luta (reprodução exata), a alavanca HP como
dial de duração, o acoplamento E1=E4, o Shield binário, a conclusão sobre cura alta.
O que mudou: o critério de duração (mais honesto), a leitura da grade (o vencedor
virou o incumbente) e o significado da rodada (de "propor conserto" para "validar o
conserto aplicado").

## 7. Curandeira: resultado reproduzido, P3B corretamente fora de escopo nesta rodada

**O que o artefato mostra:** nenhuma das 48 células P3 aprova. Na vizinhança do
vencedor (HP 33, Atk 12), cura 6 dá 100% de vitória com HP final médio em 92,0%
(reprova E1 por cima e E9), e cura 9 e 12 só pioram. A conclusão da rodada original
segue de pé, agora contra o canon vigente: **dentro da grade testada, não existe trio
(HP, Atk, cura) que aprove nos três cenários ao mesmo tempo; o eixo da cura precisa
ser estendido para baixo de 6 na Fase B** (2, 3, 4, 5), e se nem isso sair da
trivialização, a resposta honesta é regra (cadência/custo), não número.

**Correção pós-entrega (orquestrador):** o rascunho
original desta seção declarava a ausência de células P3B como pendência não
resolvida. Segundo a análise de projeto, não é: `pacing_sim_harness.hpp` teria
`build_phase_a_grid` (linhas 1333-1340) só combinando `build_phase_a_trash_grid()` e,
se o escopo pedir, `build_phase_a_elite_grid()` — nenhuma menção a P3B. O braço
`Scenario::P3B_TrashHealerBattery` só seria emitido por `build_phase_b_healer_battery_grid`
(linha 1628 em diante), com comentário atribuído a "Braco P3B da Fase B
(decisao do lider 2026-08-10)". A fatia que implementou o motor de bateria (F3) sempre
declarou isso: "Fase B 52 → 61 pontos (+9 do braço P3B)" — nunca falou em adicionar
P3B à Fase A. **Não há pendência de wiring nem decisão a confirmar com o engenheiro:
a comparação P3 (cura ilimitada) versus P3B (cura com 6 usos e dreno) está corretamente
reservada para a Fase B, que ainda não rodou** (aguardando a re-ancoragem da F7). Isso
importa porque o limite de usos é exatamente o tipo de "regra em vez de número" que a
análise original previu como possível desfecho da curandeira: pode ser que a cura 12
do motor real, LIMITADA a 6 usos por bateria, não trivialize nada — mas essa resposta
só vem quando a F7 rodar a Fase B re-ancorada com o braço P3B incluído.

> ⚠️ **Não verificado contra código.** Este valor vem da análise de projeto, não de
> medição em código executado. O código deste projeto nasce do zero (GODS_LAWS.md,
> L-01), e a fonte que a redação original citava (`pacing_sim_harness.hpp`,
> `build_phase_a_grid`, `build_phase_b_healer_battery_grid`, linhas específicas e
> comentário no código) não existe. Revalidar quando houver implementação.

## 8. ⚠️ RESSALVA DO QA: a mira ponderada ainda não está ligada em produção

Segundo a análise de projeto, **`mira_target_weight`/`pick_weighted_enemy_target` não
teriam nenhum call site de produção fora dos testes**: o cérebro do inimigo em jogo
(`scripted_brain.cpp`) continuaria mirando `players.front()`, o primeiro da lista. O
item `IA-ALVO-PRIMEIRO-DA-LISTA`, que faria esse wiring, segue pendente de decisão do
líder.

> ⚠️ **Não verificado contra código.** Esta afirmação sobre a ausência de call site de
> `mira_target_weight`/`pick_weighted_enemy_target` e sobre `scripted_brain.cpp` vem
> da análise de projeto, não de medição em código executado ou em blob commitado. O
> código deste projeto nasce do zero (GODS_LAWS.md, L-01), e a fonte que a redação
> original citava não existe. Revalidar quando houver implementação.

Consequência direta para ESTA análise, sem enterrar:

- **Os números deste documento descrevem o jogo com a mira ponderada ativa** (braço
  F_NoF4, o modelo de mira que o líder aprovou em `MIRA-PONDERADA-PROD`), porque é
  ela que o harness simula, com trava de paridade contra a fórmula do motor.
- **Uma partida jogada HOJE não se comporta assim**: o trash real ainda concentra
  tudo no primeiro da lista. O estudo da mira já mediu o que isso significa (mais
  concentração, perfil de queda diferente); até o wiring acontecer, os aprovados
  desta rodada são "aprovados sob a mira planejada", não "aprovados sob o jogo
  observável".
- **A trava de paridade protege contra deriva de fórmula, não contra ausência de
  uso.** Ela garante que estudo e motor calculam o MESMO peso; não garante que
  alguém chama esse cálculo em produção. São duas verificações diferentes, e só a
  primeira existe hoje.
- Recomendação de sequência (decisão do líder, via orquestrador): resolver o
  `IA-ALVO-PRIMEIRO-DA-LISTA` ANTES de qualquer número desta rodada virar canon em
  `combat.md`; canonizar statline validado sob uma mira que o jogo não usa cria
  exatamente o tipo de divergência silenciosa que a trava de paridade quis matar.

## 9. Recomendação ao líder

**Sobre o trash base: nenhuma mudança.** O statline canônico vigente (HP 33, Atk 12)
é o vencedor medido nos dois cenários base, com folga em todos os guarda-corpos.
A re-rodada valida `BAL-STATLINES-APLICAR` tal como aplicado.

**Sobre a Fase B: re-ancorar antes de rodar (fatia F7), em cinco movimentos.**

1. **Re-centrar a grade do braço 1 na referência 33.** A recomendação original (HP
   30/33/34/36/39, Atk 11/12/13, bissecção direcional das bordas dos degraus) segue
   válida em valores absolutos; a F7 deve re-emitir a grade com os rótulos da
   referência nova (91%-118%) e manter a célula HP=34 exata (o valor pré-inflação e
   número da casa) para o desempate sair medido, não presumido.
2. **Braço curandeira com o eixo estendido para baixo** (cura 2/3/4/5, âncora 6),
   sobre HP {33, 34, 36} × Atk {12, 13}, como já proposto na análise original.
3. **Incluir o braço P3B, que já está pronto no código, esperando a Fase B rodar**
   (seção 7): não é pendência a resolver, é escopo correto desde a F3 — a F7 só
   precisa efetivamente rodar `build_phase_b_healer_battery_grid` junto do resto. A
   pergunta "o limite de 6 usos + dreno resolve o que o número da cura sozinho não
   resolve?" é possivelmente a resposta inteira do braço curandeira.
4. **Condicionar a canonização ao wiring da mira** (seção 8): decisão do
   `IA-ALVO-PRIMEIRO-DA-LISTA` antes de qualquer número entrar em `combat.md`; se o
   líder decidir NÃO ligar a mira ponderada, a Fase A-bis precisa ser re-rodada com
   a mira primeiro-da-lista, porque os aprovados foram medidos sob a outra.
5. **Elite continua fora** até o bot com carta existir (pré-requisitos inalterados
   da análise original: decisão de design "elite cai por carta", bot novo no
   harness, QA adversarial, e só então re-screening).

**O que esta análise NÃO faz:** não re-ancora a Fase B (isso é a F7), não toca
`TODO.md` nem `combat.md`, não recomenda número novo nenhum. Toda decisão listada
acima chega ao líder pelo orquestrador via AskUserQuestion.

---

*Análise escrita por Capitolino (CPO) em 2026-08-10 sobre o artefato congelado da
Fase A-bis. Não é canon. Não commitada por decisão de processo (commit e push só com
ordem do líder).*
