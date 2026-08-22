# ANÁLISE: Fase B do estudo de pacing (refino fino, PACING-SIM, só trash, 64 pontos)

> **Status: ANÁLISE (rascunho pronto para revisão), não canon.** Nenhum número deste
> documento entra em `combat.md` nem no motor sem decisão explícita do líder. Autor:
> Capitolino (CPO), 2026-08-11. Dados: `/var/tmp/pacing_fase_b_20260811.txt`
> (artefato congelado, md5 `0c384d8844c80686791e999ff6da9be7` confirmado em duas
> leituras com 5 s de intervalo, mtime 2026-08-11 07:47:14 -0300, 570.065 bytes;
> 64 pontos, 240.000 lutas por ponto, 15,36 milhões de lutas, zero erros internos
> declarados em 64 de 64 pontos). Log de stdout redundante:
> `/var/tmp/pacing_fase_b_stdout.log`. Protocolo e pré-registros:
> `proposta-protocolo-simulacao-pacing.md`. Docs-irmãos: `analise-pacing-fase-a.md`
> (2026-08-01) e `analise-pacing-fase-a-bis-20260810.md` (2026-08-10). Não commitado
> por decisão de processo (commit e push só com ordem do líder).

---

## Resumo executivo (em linguagem simples)

1. **16 dos 64 pontos passaram em todos os guarda-corpos**: 13 no braço de refino do
   trash base (P1/P2), 3 no braço de cura livre (P3, todos com a cura mínima testada,
   2 por uso), e **zero no braço da bateria (P3B)**. A lista completa está na seção 3;
   por regra do líder, ela vai INTEIRA à mesa dele, sem corte automático (o próprio
   harness imprime isso no rodapé).
2. **A resposta da pergunta-manchete é NÃO: o limite de 6 usos + dreno da bateria não
   resolve o que o número da cura sozinho não resolve — dentro de uma luta avulsa,
   ele PIORA.** O motor canônico da carta de cura (original, bateria nova, 12 HP por
   uso) entregou as 6 lutas mais triviais da grade inteira: vitória 100% nas 6
   células, zero quedas, HP final de 91,2% a 95,2%. O motivo é aritmético, não de
   opinião: a luta mediana dura 4 rodadas, a curandeira cura no máximo 1 vez por
   rodada, então o teto de 6 usos é **inatingível por construção** numa luta avulsa —
   e cada uso cura 12 HP, o dobro da âncora 6 e seis vezes o único valor que aprova
   (2). A pergunta que a bateria responde de verdade vive ENTRE lutas (dungeon com
   bateria persistente), e esse cenário este harness de luta única não mede. Seção 4.
3. **O braço de refino confirmou e cercou o incumbente.** O statline canônico
   (HP 33, Atk 12) não foi re-medido diretamente (excluído da grade por já ter sido
   medido 2× com reprodução exata), mas foi re-medido POR EQUIVALÊNCIA: as células
   HP 34 e 36 com Atk 12 no cenário vanilla produzem números bit a bit idênticos aos
   do vencedor da A-bis (92,81% de vitória, 96,92% na janela, 7,19% de quedas) —
   é o degrau da escada: dentro do mesmo degrau, a luta é a mesma. **O desempate
   33 vs 34, que a Fase A pediu para sair medido, saiu: empate exato.** Seção 6.
4. **A geometria da aprovação no trash ficou nítida**: aprova {4 rodadas + Atk 12-13}
   ou {5 rodadas + Atk 11}. Fora disso: 4 rodadas + Atk 11 = seguro demais (vitória
   98,6%); 5 rodadas + Atk 12-13 = letal demais (quedas 19,9-21,8%); 3 rodadas =
   barrado pelo gate de queda precoce. As bordas dos degraus (produto que a Fase A
   encomendou) estão na seção 6, com a folga de variação de inimigo que elas compram.
5. **A cura livre só aprova no valor mínimo testado (2 por uso)** — 3 células
   aprovadas, todas heal=2 — e mesmo esse número contradiz o motor canônico de
   12 HP. A conclusão da Fase A ganhou força: a resposta honesta para a curandeira
   é REGRA (onde e quando a cura entra), não número; e a regra canônica da bateria
   precisa ser medida no cenário onde ela morde (multi-encontro), não na luta avulsa.
6. **⚠️ RESSALVA OBRIGATÓRIA (a mesma das duas análises anteriores): a mira ponderada
   que o estudo simula ainda não está ligada no jogo.** Segundo a análise de projeto,
   `scripted_brain.cpp` continuaria mirando o primeiro da lista; `pick_weighted_enemy_target`
   seguiria sem call site de produção. **Estes 64 pontos descrevem o jogo COM a mira
   planejada ativa, não o comportamento observável numa partida hoje.** Seção 7.

   > ⚠️ **Não verificado contra código.** Esta afirmação sobre `scripted_brain.cpp` e
   > `pick_weighted_enemy_target` vem da análise de projeto, não de medição em código
   > executado. O código deste projeto nasce do zero (GODS_LAWS.md, L-01), e a fonte
   > que a redação original citava não existe. Revalidar quando houver implementação.
7. **Recomendação de finalistas para a Fase C** (corte proposto por gente, como o
   harness exige; decisão final do líder): o incumbente 33/12 como controle, o gêmeo
   histórico 34/12, a variante quente 33/13, e o molde de variante lenta 39/11.
   Nenhum finalista de curandeira ainda. Seção 9.

---

## 1. O que esta fase é (a re-ancoragem F7.1, em uma página)

A Fase B é o refino fino em torno do vencedor — mas o vencedor mudou de identidade
entre o desenho original (2026-08-01) e a execução. A fatia F7.1 (commit `bffb01ca`,
decisão do líder 2026-08-10) re-ancorou a grade nos vencedores da Fase A-bis: o
statline canônico vigente (HP 33, Atk 12), e não mais os multiplicadores da
referência antiga de 55. O antigo "vencedor" de cura (HP 55, Atk 14, cura 6) saiu de
âncora porque a A-bis o reprovou pelo gate E2 novo (mediana 7 rodadas, 4,39% na
janela). A grade re-ancorada, descrita pela análise de projeto como verificada por QA
adversarial independente na F7.2 (28+30+6 = 64 pontos conferidos, exclusão do centro
validada, pareamento P3⇄P3B validado, mutation testing 16/17 mutantes mortos + 1
provado equivalente, veredicto PRONTO), ficou assim:

> ⚠️ **Não verificado contra código.** A descrição de QA adversarial e mutation
> testing acima (16/17 mutantes mortos, 1 equivalente) vem da análise de projeto, não
> de medição em código executado. O código deste projeto nasce do zero
> (GODS_LAWS.md, L-01), e a fonte que a redação original citava não existe. A grade
> de pontos resultante (28+30+6 = 64) continua válida como especificação de
> protocolo; revalidar a execução quando houver implementação.

- **Braço P1/P2 (28 pontos):** HP {30, 33, 34, 36, 39} × Atk {11, 12, 13} × os dois
  cenários base (vanilla e parede), MENOS o centro 33/12 — já medido duas vezes com
  reprodução bit a bit (Fase A e A-bis), não se gasta amostra remedindo.
- **Braço P3 free-heal (30 pontos):** HP {33, 34, 36} × Atk {12, 13} × cura
  {2, 3, 4, 5, 6}. A cura 6 (a antiga âncora) entra DENTRO do mesmo run, para a
  comparação cura-contra-cura não atravessar rodadas do estudo.
- **Braço P3B bateria (6 pontos):** as MESMAS 6 células de HP/Atk do braço de cura,
  com o motor real da carta (original, bateria nova, Médio: 12 HP por uso, dreno de
  50% da carga atual, 6 usos por bateria). Sem eixo de cura livre: a cura não é mais
  parâmetro, é consequência do hardware.

Entre a F7.2 e o run houve uma única correção (A-1): um comentário de estimativa de
tempo, sem efeito nos números. N = 240.000 por ponto (amostra cheia, como o líder
manda), seed base 20260801, elite segue congelado (pré-requisitos inalterados desde
2026-08-01).

## 2. Checagem de validade interna

- **Zero erros internos declarados em 64 de 64 pontos**: a linha "erros internos:
  0 de 240000" está presente nos 64 blocos e nenhuma linha de erro não-zero existe
  no artefato (verificado por enumeração, não por amostra). Zero declarado, não
  presumido.
- **Os gates E2 (cauda e janela) reprovaram ZERO pontos em 64.** Nas rodadas
  anteriores o E2 reprovava 40 células; a re-ancoragem posicionou a grade inteira
  dentro da janela de duração (mediana 3-5 em todos os 64 pontos). A grade agora
  pergunta a pergunta certa: quem separa aprovado de reprovado é pressão
  (vitória/quedas/custo), não duração.
- **A escada + seeds pareadas produziram a checagem de graça desta rodada:** células
  de HP diferente dentro do mesmo degrau entregam números IDÊNTICOS até a última
  casa impressa. Exemplos: P1 33/13 ≡ 34/13 ≡ 36/13 (vitória 92,3083% nas três);
  P2 30/12 ≡ 34/12 (92,5642%); P2 36/11 ≡ 39/11 (92,3571%); P3B 33/12 ≡ 34/12
  (HP final 95,2105% nos dois). É o comportamento esperado quando o dano por rodada
  é determinístico: se o teto-inteiro de HP/dano não muda, a trajetória da luta não
  muda, e as seeds pareadas reproduzem tudo. Nada disso é duplicação de bug; é a
  física medida na Fase A aparecendo onde deveria.
- **A equivalência re-mediu o incumbente sem gastar célula com ele:** P1 34/12
  reproduz exatamente os números do vencedor da A-bis (92,8133% / 3,88936 rodadas /
  janela 96,9221% / quedas 7,18667% / HP final 75,3092% / E10 30,7521 HP) — a
  exclusão do centro não perdeu informação, porque 34 está no mesmo degrau de 33.

## 3. Resultado agregado: 16 aprovados em 64 (13 P1/P2 + 3 P3 + 0 P3B)

### Braço P1/P2 (13 de 28)

| Ponto | Cenário | HP | Atk | Vitória | Mediana | Janela 3-5 | Quedas Gus | 1ª queda (med.) | HP final | E10 dano/enc. |
|---|---|---|---|---|---|---|---|---|---|---|
| 4 | P2 parede | 30 | 12 | 92,56% | 4 | 96,85% | 7,44% | rodada 3 | 77,5% | 27,0 HP |
| 6 | P2 parede | 30 | 13 | 92,00% | 4 | 96,55% | 8,00% | rodada 3 | 73,8% | 32,1 HP |
| 9 | P1 vanilla | 33 | 13 | 92,31% | 4 | 96,63% | 7,69% | rodada 3 | 70,6% | 37,7 HP |
| 10 | P2 parede | 33 | 13 | 92,00% | 4 | 96,55% | 8,00% | rodada 3 | 73,8% | 32,1 HP |
| 13 | P1 vanilla | 34 | 12 | 92,81% | 4 | 96,92% | 7,19% | rodada 3 | 75,3% | 30,8 HP |
| 14 | P2 parede | 34 | 12 | 92,56% | 4 | 96,85% | 7,44% | rodada 3 | 77,5% | 27,0 HP |
| 15 | P1 vanilla | 34 | 13 | 92,31% | 4 | 96,63% | 7,69% | rodada 3 | 70,6% | 37,7 HP |
| 16 | P2 parede | 34 | 13 | 92,00% | 4 | 96,55% | 8,00% | rodada 3 | 73,8% | 32,1 HP |
| 18 | P2 parede | 36 | 11 | 92,36% | 5 | 98,56% | 7,64% | rodada 3 | 76,0% | 28,5 HP |
| 19 | P1 vanilla | 36 | 12 | 92,81% | 4 | 96,92% | 7,19% | rodada 3 | 75,3% | 30,8 HP |
| 21 | P1 vanilla | 36 | 13 | 92,31% | 4 | 96,63% | 7,69% | rodada 3 | 70,6% | 37,7 HP |
| 23 | P1 vanilla | 39 | 11 | 92,62% | 5 | 98,59% | 7,38% | rodada 3 | 74,6% | 30,8 HP |
| 24 | P2 parede | 39 | 11 | 92,36% | 5 | 98,56% | 7,64% | rodada 3 | 76,0% | 28,5 HP |

**Células aprovadas nos DOIS cenários ao mesmo tempo** (o critério que o vencedor da
A-bis cumpre): **33/13, 34/12, 34/13 e 39/11** — mais o incumbente 33/12, aprovado
nos dois na A-bis e representado aqui por equivalência de degrau. As aprovações
de um lado só contam a geometria: 30/12 e 30/13 aprovam só na parede (no vanilla a
luta de 3 rodadas cai no gate E8); 36/11 aprova só na parede (no vanilla é seguro
demais, 98,6%); 36/12 e 36/13 aprovam só no vanilla (na parede a luta vira 5 rodadas
e o teto de quedas estoura, 20,7-21,8%).

### Braço P3 free-heal (3 de 30 — todos com cura 2)

| Ponto | Célula | Cura | Vitória | Mediana | Janela | Quedas Gus | 1ª queda (med.) | HP final | E10 |
|---|---|---|---|---|---|---|---|---|---|
| 34 | 33/13 | 2 | 96,03% | 4 | 98,32% | 3,97% | rodada 3 | 77,7% | 35,2 HP |
| 44 | 34/13 | 2 | 96,03% | 4 | 98,32% | 3,97% | rodada 3 | 77,7% | 35,2 HP |
| 49 | 36/12 | 2 | 96,30% | 5 | 99,32% | 3,70% | rodada 4 | 78,1% | 36,4 HP |

Nota honesta sobre esses 3: eles aprovam com um número de cura (2) que **não é o do
motor canônico** (12 HP por uso). São informação sobre onde a cura livre deixa de
trivializar, não candidatos a canon da curandeira (seção 4 e 9).

### Braço P3B bateria (0 de 6)

Nenhuma célula aprova. Todas as 6 reprovam pelo mesmo par de gates (E1 vitória 100%
e E9 HP final acima de 90%). Detalhe na seção 4, porque este zero É a resposta da
pergunta-manchete.

## 4. A pergunta-manchete: P3 vs P3B, pareado célula a célula

**Pergunta pré-declarada (F7.1): o limite de 6 usos + dreno da bateria resolve o que
o número de cura sozinho não resolve?** As mesmas 6 células de HP/Atk existem nos
dois braços; a comparação é direta:

| Célula | P3 cura 2 (melhor caso do braço) | P3 cura 6 (âncora) | P3B motor real (12 HP, 6 usos, dreno 50%) |
|---|---|---|---|
| 33/12 | 99,33% — REPROVA (seguro demais) | 100%, HP final 92,0% — REPROVA | **100%, HP final 95,2% — REPROVA** |
| 33/13 | 96,03% — **APROVA** | 99,63%, HP final 87,3% — REPROVA | **100%, HP final 93,5% — REPROVA** |
| 34/12 | 99,33% — REPROVA | 100%, HP final 92,0% — REPROVA | **100%, HP final 95,2% — REPROVA** |
| 34/13 | 96,03% — **APROVA** | 99,63%, HP final 87,3% — REPROVA | **100%, HP final 93,5% — REPROVA** |
| 36/12 | 96,30% — **APROVA** | 99,95%, HP final 90,5% — REPROVA | **100%, HP final 93,7% — REPROVA** |
| 36/13 | 92,00% — REPROVA (só E8) | 99,21%, HP final 84,4% — REPROVA | **100%, HP final 91,2% — REPROVA** |

**Placar pareado: P3 aprova 3 de 6 (sempre com cura 2); P3B aprova 0 de 6.** E não é
empate técnico: em TODAS as 6 células, o P3B é estritamente MAIS trivial que a
própria âncora de cura 6 — vitória 100% contra 99,2-100%, HP final sempre mais alto
(95,2 vs 92,0; 93,5 vs 87,3; 91,2 vs 84,4), zero quedas contra quedas raras. As 6
lutas do P3B são as mais triviais da grade inteira de 64 pontos.

**Veredicto: NÃO — dentro de uma luta avulsa, o limite de usos + dreno não resolve
nada, porque ele nunca chega a morder.** O argumento é aritmético e sai dos próprios
dados:

1. A luta mediana do P3B dura exatamente 4 rodadas (p10 = p90 = 4 nas 6 células). A
   curandeira usa a carta no máximo 1 vez por rodada. Logo, o máximo de usos numa
   luta é 4-5 — **o teto de 6 usos é inatingível por construção no cenário de luta
   única.** O limitador que o braço existia para testar não participa da medição.
2. Enquanto o limite não morde, o que sobra é o número: 12 HP por uso, o dobro da
   âncora 6 que já trivializava, e seis vezes o único valor que aprova (2). Com
   bateria NOVA (o setup declarado do braço), a cura entra quase garantida.
3. **A pergunta que a bateria responde de verdade é de OUTRA escala de tempo**: 6
   usos e dreno de 50% por uso são recursos de dungeon — mordem na terceira, quarta
   luta da mesma bateria, no SoH que degrada, na carta pirata que falha. Um harness
   de luta avulsa com bateria nova a cada luta mede o teto superior de trivialização
   do motor, e ele mediu: é alto. O mecanismo limitador precisa de um cenário
   multi-encontro (o gancho existe no protocolo: o E7/P6 de gauntlet) para ser
   medido de verdade.

**Observação de mecânica registrada para o engenheiro confirmar** (não muda o
veredicto, mas precisa de resposta antes da Fase C da curandeira): nas células de
HP 36, a luta do P3 dura 5 rodadas (qualquer cura), mas a do P3B dura 4 — o motor
real ENCURTOU a luta em relação à cura livre. A hipótese consistente com os dados é
que no P3 a curandeira gasta o turno curando (menos dano da party por rodada,
TTK-out mais longo), enquanto no P3B ela nem sempre usa a carta (e quando não usa,
ataca). É hipótese, não dado; está marcada como tal.

**Consequência de design (proposta, decisão do líder):** para a luta avulsa de
trash, o resultado das três rodadas converge — cura por número só sai da
trivialização em 2 por uso, e o motor canônico de 12 HP não pode estar disponível
sem freio DENTRO da luta. As saídas honestas são regra, não número: (a) medir o
freio canônico (usos/dreno/SoH/pirata) no cenário multi-encontro onde ele vive;
(b) se a luta avulsa precisar de freio próprio, ele será de cadência ou custo
(cooldown, AP, 1 uso por luta), o desfecho que a Fase A já tinha previsto como
possível. Nenhuma das duas entra em canon por esta análise.

## 5. Anatomia das 48 reprovações

Contagens por guarda-corpo (não exclusivos; um ponto pode reprovar em vários).
Enumeração completa do artefato, não amostra:

| Guarda-corpo | Reprovações | Onde |
|---|---|---|
| E1 vitória > 97% (seguro demais) | **38** | 7 no braço P1/P2 (todas as células Atk 11 de 4 rodadas), 25 no P3 (toda cura >= 3, e cura 2 nas células frias), 6 no P3B (100% nas seis) |
| E8 1ª queda precoce (mediana 1-2) | **20** | 3 no P1/P2 (as lutas de 3 rodadas do vanilla + o ponto 1), 17 no P3 |
| E9 HP final > 90% (luta que não custa nada) | **9** | cura 6 em 33/12 e 34/12 (91,95%), cura 6 em 36/12 (90,48%), e as 6 células P3B (91,2-95,2%) |
| E1 vitória < 90% (letal demais) | **6** | os pares 36/12, 36/13 (parede) e 39/12, 39/13 (ambos cenários): vitória 78,2-80,1% |
| E4 quedas do Gus > 12% | **6** | as mesmas 6 células letais (19,9-21,8%) |
| E2 cauda (p10/p90) | **0** | — |
| E2 janela (gate novo) | **0** | — |
| E9 trivialidade (sem-dano > 40%) | **0** | — |

Quatro leituras de design:

1. **O E2 zerou e isso é o sucesso da re-ancoragem** (na A-bis ele reprovava 40 de
   80). Toda a grade agora vive dentro da janela de duração; a seleção passou a ser
   feita por pressão. É o comportamento que se espera de um refino bem centrado.
2. **O gate que passou a morder é o E8** (20 reprovações), e ele tem as mesmas duas
   famílias diagnosticadas na A-bis, agora mais nítidas: as células de cura
   quase-imperdíveis onde a raríssima derrota acontece na largada (nos pontos 37/38
   e 47/48, cura 5-6 com Atk 13, a mediana da 1ª queda é a RODADA 1 com quedas de
   0,37% — o perfil de morte que parece arbitrário ao jogador), e as lutas curtas.
   **Registro de desenho de gate, sem propor mudança no meio do estudo:** numa luta
   de 3 rodadas, qualquer queda tem mediana quase forçada em 1-2, então o E8
   funciona na prática como veto estrutural de lutas de 3 rodadas (é ele, sozinho,
   que reprova P1 30/12 e 30/13, com vitória verde). Se o líder um dia quiser lutas
   de 3 rodadas, o E8 precisa de reavaliação pré-registrada; até lá, o veto vale.
3. **Quatro pontos reprovaram SÓ no E8**: P1 30/12 e 30/13 (as lutas de 3 rodadas) e
   P3 36/13 com cura 2 e 3 (vitória 92,0% e 94,9%, tudo verde menos a 1ª queda na
   rodada 2). São os "quase" da rodada — em particular o P3 36/13 cura 2-3, que é a
   vizinhança onde a cura baixa mais chegou perto de aprovar com pressão de verdade.
4. **A geometria do perigo re-confirmou a lição das duas rodadas anteriores, agora
   com a borda exata**: 5 rodadas + Atk >= 12 = 19,9-21,8% de quedas (acima do teto);
   5 rodadas + Atk 11 = 7,4-7,6% (aprovado). O acúmulo de dano por rodada extra é
   grande porque a luta inteira é curta: uma rodada a mais são +25% de exposição.

## 6. As bordas dos degraus e o desempate 33 vs 34 (o produto que a Fase A encomendou)

A Fase A pediu que o refino reportasse as bordas dos degraus da escada, "tolerância
de design para variantes". Medidas:

- **P1 vanilla:** 3 rodadas em HP 30; 4 rodadas em HP {33, 34, 36}; 5 rodadas em
  HP 39. Bordas: 3→4 entre 30 e 33; 4→5 entre 36 e 39.
- **P2 parede:** 4 rodadas em HP {30, 33, 34}; 5 rodadas em HP {36, 39}. Borda 4→5
  entre 34 e 36 — **um degrau antes do vanilla**, porque o turno absorvido pelo
  Shield do defensor desloca a conta.

O que isso compra de design, lido junto com a seção 5:

- **A folga do statline canônico é +1 HP com segurança total** (34 aprova nos dois
  cenários com Atk 12 e 13) **e +3 HP só no vanilla** (36 aprova no vanilla e
  estoura na parede com Atk >= 12). Variante de inimigo acima de 34 HP precisa ou de
  ataque contido (Atk 11) ou de não aparecer em encontros com defensor.
- **Variante lenta (5 rodadas) existe e é viável, mas só com Atk 11**: 39/11 e 36/11
  (parede) aprovam com quedas ~7,4-7,6%. É o molde para o inimigo-esponja deliberado.
- **O desempate 33 vs 34 saiu medido, como a Fase A mandou: empate EXATO.** P1 34/12
  reproduz o vencedor da A-bis número a número (mesmo degrau), e P2 34/12 idem. A
  escolha entre manter 33 (canon vigente, já aplicado) e voltar ao 34 histórico
  (valor pré-inflação e número da casa) é agora uma decisão de identidade sem custo
  de gameplay mensurável NOS CENÁRIOS MEDIDOS — com a ressalva de que outro cenário,
  com outro dano por rodada, pode separá-los (é um dos motivos de levar 34 à Fase C,
  seção 9). A decisão é do líder, pelo desempate pré-registrado que ele fixou.
- Insumo para o economy-designer (E10, doutrina do comedimento): nos aprovados, o
  dano sofrido por encontro fica entre 27,0 e 37,7 HP; a variante Atk 13 custa ~22%
  a mais de HP por encontro que a Atk 12 (37,7 vs 30,8 no vanilla) — se o líder
  escolher a variante quente, o custo de hospital/cura por encontro sobe junto.

## 7. ⚠️ RESSALVA OBRIGATÓRIA: a mira ponderada ainda não está ligada em produção

Repetida das duas análises anteriores com a mesma força, porque nada mudou: segundo a
análise de projeto, o QA adversarial da fatia F5 teria provado (verificação contra o
blob commitado) que **`mira_target_weight`/`pick_weighted_enemy_target` não teriam
nenhum call site de produção fora dos testes** — o cérebro inimigo em jogo
(`scripted_brain.cpp`) continuaria mirando `players.front()`, o primeiro da lista. O
item `IA-ALVO-PRIMEIRO-DA-LISTA` segue pendente de decisão do líder. A A-bis revalidou
o achado; esta análise o repete sem re-verificar o código.

> ⚠️ **Não verificado contra código.** Esta afirmação sobre o QA adversarial, a
> verificação contra blob commitado, `mira_target_weight`/`pick_weighted_enemy_target`
> e `scripted_brain.cpp` vem da análise de projeto, não de medição em código
> executado ou em blob commitado real. O código deste projeto nasce do zero
> (GODS_LAWS.md, L-01), e a fonte que a redação original citava não existe. Revalidar
> quando houver implementação.

- **Os 64 pontos desta Fase B descrevem o jogo COM a mira ponderada ativa** (Opção
  F, F1+F2+F3, sem F4 — o modelo aprovado em `MIRA-PONDERADA-PROD`), porque é ela
  que o harness simula, com trava de paridade contra a fórmula do motor.
- **Uma partida jogada HOJE não se comporta assim**: o trash real concentra tudo no
  primeiro da lista. Os 16 aprovados são "aprovados sob a mira planejada", não
  "aprovados sob o jogo observável".
- **A trava de paridade protege contra deriva de fórmula, não contra ausência de
  uso.** Garante que estudo e motor calculam o mesmo peso; não garante que alguém o
  chama em produção.
- Sequência recomendada (inalterada): resolver `IA-ALVO-PRIMEIRO-DA-LISTA` ANTES de
  qualquer número desta rodada virar canon em `combat.md`. Se o líder decidir NÃO
  ligar a mira ponderada, as Fases A-bis e B precisam ser re-rodadas sob a mira
  primeiro-da-lista antes de qualquer canonização.

## 8. Comparação com a Fase A-bis

| Aspecto | Fase A-bis (2026-08-10) | Fase B (2026-08-11) |
|---|---|---|
| Grade | 80 pontos, herdada (fatias de 55) | 64 pontos, re-ancorada no vencedor (F7.1) |
| Aprovados | 2 de 80 (2,5%) | **16 de 64 (25%)** — 13 P1/P2, 3 P3, 0 P3B |
| Gate dominante | E1 por cima (64) e E2 (40) | E1 por cima (38); **E2 zerou** |
| Braço de cura | cura 6/9/12: tudo trivializa | cura 2 aprova em 3 células; 3+ trivializa |
| Braço P3B | fora de escopo (é braço da Fase B) | **medido: 0 de 6, hiper-trivial** |
| Incumbente 33/12 | vencedor único (2 cenários) | re-confirmado por equivalência de degrau |
| Desempate 33 vs 34 | pendente ("deve sair medido") | **medido: empate bit a bit** |
| Bordas dos degraus | não era o escopo | P1: 30\|33 e 36\|39; P2: 34\|36 |
| Mira ponderada | ressalva (sem call site) | ressalva mantida, inalterada |

O que não mudou: a física simulada (as células compartilhadas entre rodadas
reproduzem), a alavanca HP como dial de duração, o acoplamento derrota = queda do
Gus (visível de novo: E1 e E4 reprovam juntos nas 6 células letais), o Shield
binário do defensor (E6 = 100% em toda a coluna P2, pendência de regra com dono), e
a conclusão de que cura alta trivializa. O que mudou: a grade finalmente pergunta no
lugar certo, e a pergunta nova (bateria) foi respondida — contra a expectativa
otimista da A-bis, que especulava que "pode ser que a cura 12 limitada a 6 usos não
trivialize nada". Trivializa, na luta avulsa; a esperança do limite ficou para o
cenário multi-encontro, onde ele de fato opera.

## 9. Recomendação ao líder: quem vai à Fase C

O harness imprime no rodapé, e eu repito como regra de processo: **"Fase C NAO RODA
AUTOMATICAMENTE (decisao do lider): todos os sobreviventes da Fase B entram na
bateria completa, e o corte para os 2-4 finalistas e feito por gente, na hora de
levar ao lider - nunca por corte automatico deste harness."** A tabela dos 16 está
inteira na seção 3; o corte abaixo é a minha proposta de gente, com motivo, para o
líder aprovar, ajustar ou rejeitar via AskUserQuestion (pelo orquestrador).

**Proposta: 4 células para a bateria completa de cenários da Fase C.**

1. **HP 33 / Atk 12 (incumbente canônico) — entra como CONTROLE obrigatório.**
   Não está entre os 16 (foi excluído da grade por já ter 2 medições com reprodução
   exata), mas é o baseline contra o qual a Fase C compara tudo; rodá-lo na bateria
   completa é o que valida o canon vigente nos cenários que ainda não o viram.
2. **HP 34 / Atk 12 — o gêmeo histórico.** Empate bit a bit com o incumbente nos
   cenários medidos; vai à C porque cenários novos (outro dano por rodada) podem
   separar 33 de 34, e porque é a única forma de o desempate pré-registrado
   (valor pré-inflação, número da casa) ser decidido com dado completo em vez de
   presumido. Custo marginal ~zero: se a C também der empate, a decisão vira pura
   identidade, documentada.
3. **HP 33 / Atk 13 — a variante quente.** Aprovada nos dois cenários (92,3% /
   92,0%), quedas 7,7-8,0%, mesmo comprimento de luta e ~22% mais dano sofrido por
   encontro (37,7 HP no vanilla). É a opção de temperatura: se o líder achar o
   incumbente manso demais no playtest, esta é a alternativa já validada. (34/13 é
   a mesma luta, medido; não gastar célula com os dois.)
4. **HP 39 / Atk 11 — molde de VARIANTE de inimigo, não candidato a statline base.**
   A única receita de luta de 5 rodadas aprovada nos dois cenários (7,4-7,6% de
   quedas). Levar à C como perfil de inimigo-esponja deliberado, para o design de
   variantes ter uma segunda textura de trash com números já cercados.

**Curandeira: NENHUM finalista nesta rodada, e a recomendação é explícita.** As 3
células de cura 2 aprovadas ficam na tabela como informação, não como candidatas:
2 HP por uso contradiz o motor canônico (12 HP) e canonizá-las criaria uma segunda
fonte de verdade da cura. O caminho que proponho, em ordem: (a) decisão do líder
sobre onde o freio da cura deve viver (regra dentro da luta vs bateria entre lutas);
(b) se bateria: estudo-irmão curto multi-encontro (gauntlet estilo P6, bateria
persistente, incluindo os eixos que este run não varreu — SoH degradado, carta
pirata/homebrew com falha = 100-SoH%), com pré-registro próprio ANTES de rodar;
(c) a resposta do engenheiro à observação de mecânica da seção 4 (por que o P3B
encurta a luta) antes de qualquer re-medição da curandeira. Até lá, a cura da Jaci
segue sem número canônico validado por simulação — dizer isso com todas as letras é
melhor do que promover um número que os dados reprovaram.

**O que esta análise NÃO faz:** não toca `TODO.md` nem `combat.md`, não canoniza
número nenhum, não dispara a Fase C. Toda decisão listada acima chega ao líder pelo
orquestrador via AskUserQuestion.

---

*Análise escrita por Capitolino (CPO) em 2026-08-11 sobre o artefato congelado da
Fase B. Não é canon. Não commitada por decisão de processo (commit e push só com
ordem do líder).*
