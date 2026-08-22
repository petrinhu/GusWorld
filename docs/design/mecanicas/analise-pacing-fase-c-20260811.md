# ANÁLISE: Fase C do estudo de pacing (bateria completa de confirmação, 48 pontos)

> **Status: ANÁLISE (rascunho pronto para revisão), não canon.** Nenhum número deste
> documento entra em `combat.md` nem no motor sem decisão explícita do líder. Autor:
> Capitolino (CPO), 2026-08-11. Dados: `/var/tmp/pacing_fase_c_20260811.txt`
> (artefato congelado, md5 `ad8c7903e6ae66a42a340cd1359ed400` confirmado em duas
> leituras com 5 s de intervalo, mtime 2026-08-11 11:09:55 -0300, 457.564 bytes;
> 48 pontos, 240.000 lutas por ponto, 11,52 milhões de lutas, zero erros internos
> declarados em 48 de 48 pontos). Log de stdout redundante:
> `/var/tmp/pacing_fase_c_stdout.log`. Protocolo e pré-registros:
> `proposta-protocolo-simulacao-pacing.md`. Docs-irmãos: `analise-pacing-fase-a.md`
> (2026-08-01), `analise-pacing-fase-a-bis-20260810.md` (2026-08-10) e
> `analise-pacing-fase-b-20260811.md` (2026-08-11). Não commitado por decisão de
> processo (commit e push só com ordem do líder).

---

## Resumo executivo (em linguagem simples)

1. **Nenhum dos 4 candidatos sobreviveu à Fase C: 0 de 4.** A regra pré-registrada
   exigia aprovar os três cenários decisivos (P1 vanilla, P2 parede, P6 tartaruga)
   nos DOIS braços de mira (a titular de produção e a vizinha, mais agressiva com
   quem defende). Todos os 4 caíram; nenhum caiu no P1. **Não existe finalista a
   escolher, e esta análise não propõe nenhum.** A recomendação é sobre o próximo
   passo, não sobre um vencedor que não existe.
2. **Causa sistemática nº 1: quando a mira do inimigo passa a evitar suavemente quem
   se defende (braço vizinho, peso ×0,5), o cenário da parede reprova nos 4
   candidatos.** A vitória cai de 92,0-92,6% para 85,6-86,7% (piso é 90%) e as
   quedas do Gus quase dobram, de 7,4-8,0% para 13,3-14,4% (teto é 12%). Não é
   ruído: a distância até o piso vale de 24 a 31 vezes a incerteza impressa do
   próprio harness (a meia-largura do IC de 95%; em erros-padrão, ~47 a 62). Em
   português: **a defesa do Bento hoje só funciona porque o inimigo coopera
   mirando nele; quando o inimigo fica um pouco mais esperto, o "escudo humano"
   estatístico desmonta.** Seção 4.
3. **Causa sistemática nº 2: o gate exclusivo do cenário tartaruga (P6) reprovou em
   7 dos 8 pontos decisivos, por dois motivos distintos** que exigem consertos
   diferentes: (a) o teto de "0,0% ESTRITO de lutas que chegam ao cap de 30
   rodadas" caiu 3 vezes — duas delas por **UMA única luta em 240.000**, e uma por
   35 lutas; (b) o piso de "p10 >= 4 rodadas" caiu 4 vezes (p10 = 3): com a mira
   vizinha (e com Atk 13 até na titular), a party que só defende morre rápido
   demais. O motivo (a) é, em parte, um problema do CRITÉRIO (seção 8); o motivo
   (b) é um problema de DESIGN, irmão da causa nº 1. Seção 5.
4. **O critério "0% estrito" pede algo que estatística nenhuma entrega, e isso
   agora está demonstrado com o dado na mão.** Zero eventos observados em N lutas
   nunca demonstra taxa zero — demonstra no máximo taxa <= 3/N com 95% de confiança
   (regra de três). E um critério de "zero em N" fica MAIS difícil de passar quanto
   MAIOR o N: com a taxa real medida (~4×10⁻⁶), a chance de ver >=1 luta no cap é
   ~9% com N=24.000, ~62% com N=240.000 e ~98% com N=1.000.000. **O harness rodou
   com a amostra cheia que o líder exige, e é exatamente a amostra cheia que torna
   o "0% estrito" quase impossível.** A proposta (para a PRÓXIMA rodada de
   pré-registro, nunca reinterpretando esta) é trocar "0% estrito" por um teto de
   TAXA com intervalo de confiança. Com o dado já medido: os candidatos 33/12 e
   34/12 passariam num teto de 10⁻⁴ e reprovariam num teto de 10⁻⁵; o 39/11
   reprovaria nos dois. O número do teto é decisão do líder — com número, não
   refém do tamanho da amostra. Seção 8.
5. **A pesquisa de design externa (lead-game-designer, a pedido do líder) diz que
   os dois problemas têm solução com precedente forte na indústria** — e que a
   resposta dominante para a mira NÃO é suavizar a IA, é dar ao jogador uma
   alavanca explícita (aggro à la Xenoblade, taunt separado da defesa à la FFXIII
   Sentinel, ou guard-redirect determinístico à la Darkest Dungeon); e para a
   tartaruga é pressão crescente (soft enrage à la WoW/Gruul), retorno decrescente
   (Pokémon Protect) ou chip damage (Slay the Spire), mantendo o cap só como rede
   de segurança. Seções 6 e 7.
6. **⚠️ RESSALVA OBRIGATÓRIA (a mesma das três análises anteriores, com a mesma
   força): a mira ponderada que o estudo simula ainda não está ligada no jogo.**
   Segundo a análise de projeto, `scripted_brain.cpp` continuaria mirando o primeiro
   da lista; `pick_weighted_enemy_target` seguiria sem call site de produção. **Estes
   48 pontos descrevem o jogo COM a mira planejada ativa, não o comportamento
   observável numa partida hoje.** Seção 9.

   > ⚠️ **Não verificado contra código.** Esta afirmação sobre `scripted_brain.cpp` e
   > `pick_weighted_enemy_target` vem da análise de projeto, não de medição em código
   > executado. O código deste projeto nasce do zero (GODS_LAWS.md, L-01), e a fonte
   > que a redação original citava não existe. Revalidar quando houver implementação.
7. **Recomendação final: não escolher statline agora.** O incumbente 33/12 continua
   sendo o melhor statline conhecido PARA O JOGO COMO ELE É HOJE (mira titular:
   ele aprova P1 e P2 com folga, e a única reprovação titular dele é a luta única
   do cap). O que a Fase C reprovou não foi o número — foi a robustez do DESENHO
   (defesa punível pela mira, tartaruga sem pressão) e um critério
   estatisticamente impagável. Próximo passo proposto: decisão de design da
   defesa/mira + decisão de design anti-tartaruga + pré-registro novo do critério
   do cap, e só então uma nova rodada de confirmação. Seção 11.

---

## 1. O que a Fase C é

A Fase C é a bateria completa de confirmação dos finalistas da Fase B — o teste
adversarial final antes de qualquer número ir à mesa do líder para canonização.
Grade: **4 candidatos × 2 braços de mira × 6 cenários = 48 pontos**, N = 240.000
por ponto (amostra cheia, como o líder manda), seed base 20260801.

**Os 4 candidatos** (aprovados pelo LÍDER após a Fase B; nenhum corte automático
produziu a lista):

1. **HP 33 / Atk 12 — CONTROLE (incumbente canônico).** O statline em produção
   hoje (`BAL-STATLINES-APLICAR`). Volta a rodar (a Fase B o excluía por já ter 2
   medições) porque sem ele os outros 3 não teriam contra o que ser comparados
   dentro do mesmo run, sob a mesma seed e o mesmo braço de mira.
2. **HP 34 / Atk 12 — gêmeo histórico** (valor pré-inflação, número da casa).
3. **HP 33 / Atk 13 — variante quente** (+1 Atk, ~22% mais dano sofrido/encontro).
4. **HP 39 / Atk 11 — MOLDE DE VARIANTE (inimigo-esponja)**, não candidato a
   statline base; entra para o perfil sair medido.

**Os 2 braços de mira** (rigor pleno nos dois, não "não piorou muito"):

- **`F_sem_F4` (TITULAR):** a mira que foi para produção em `MIRA-PONDERADA-PROD`
  (Opção F, F1+F2+F3, sem F4), atribuída ao trash comum na tabela tier-mira.
- **`C_F4_suave` (VIZINHO):** a mira com o termo F4 suave (peso ×0,5 em quem está
  defendendo — o inimigo tende a EVITAR o defensor). Braço harness-only DE
  PROPÓSITO: é o teste de robustez — o candidato tem de aguentar a mira do vizinho
  da escada de dificuldade, não só a que está ligada hoje.

**A agregação (o que decide):** um candidato SOBREVIVE se **P1, P2 e P6 aprovarem
nos DOIS braços**. P3 (cura livre, nos dois valores: 12 do motor canônico e 2, o
único que a Fase B aprovou) e P3B (bateria) são **INFORMATIVOS** nesta fase por
decisão do líder de 2026-08-11 — rodam, calculam o próprio veredicto, e
sabidamente reprovam (a Fase B já mediu; não é bug), mas não entram na conta.
O cenário novo é o **P6 (tartaruga total)**: a party só defende, a derrota é
esperada em ~100%; o que o gate exclusivo dele mede é (a) que NENHUMA luta chegue
ao cap de 30 rodadas ("0,0% estrito", pré-registro do líder de 2026-08-11) e (b)
que defender compre pelo menos 4 rodadas de sobrevivência (p10 >= 4).

Elite: 0 pontos, congelado desde 2026-08-01 (artefato de medição do bot sem carta
contra Def 14; nada mudou, código do braço intacto no harness).

## 2. Checagem de validade interna

- **Zero erros internos declarados em 48 de 48 pontos** (linha "erros internos:
  0 de 240000" presente em todos; nenhuma linha de erro não-zero existe no
  artefato). Zero declarado, não presumido.
- **Terceira reprodução bit a bit do incumbente.** P1 e P2 titulares do 33/12
  reproduzem exatamente as duas rodadas anteriores (vitória 92,8133% / 92,5642%,
  janela 96,9221% / 96,8525%, quedas 7,18667% / 7,43583%). O mesmo vale para o
  degrau: **as 12 células do 34/12 são idênticas às do 33/12 número a número, nos
  dois braços e nos seis cenários** — a equivalência de degrau da Fase B
  confirmada agora também sob a mira vizinha e sob o P6.
- **Controle positivo interno que os dados entregaram de graça: o P1 vanilla é
  idêntico entre os dois braços de mira, nos 4 candidatos.** Faz sentido físico —
  o termo F4 pesa quem DEFENDE, e no vanilla ninguém defende, então F4 é inerte e
  `C_F4_suave` colapsa em `F_sem_F4`. Os números confirmam isso até a última casa
  impressa. Isso prova que os braços diferem exatamente onde deveriam diferir (e
  só ali), e que toda diferença observada em P2/P6 vem do termo F4, não de outra
  coisa.
- **Camada de ponto vs camada de candidato:** 15 de 48 pontos aprovaram nos
  guarda-corpos do próprio ponto (o rodapé do harness declara, incluindo os
  informativos de P3/P3B). A camada de candidato — a que decide — deu **0 de 4**.
  As duas contagens são coerentes: os 15 aprovados são os P1 dos 4 candidatos nos
  2 braços (8), os P2 titulares (4), o P6 vizinho do 39/11 (1) e os 2 P3 cura 2
  do 33/13 (informativos).

## 3. Resultado agregado: 0 de 4 candidatos sobreviveram

Tabela dos cenários DECISIVOS (P3/P3B informativos ficam para a seção 5.3):

| Candidato | Braço | P1 vanilla | P2 parede | P6 tartaruga | Braço |
|---|---|---|---|---|---|
| 33/12 controle | titular | ✅ 92,81% | ✅ 92,56% | ❌ cap: **1 luta** em 240k (p10=4 ok) | CAIU |
| 33/12 controle | vizinho | ✅ 92,81% | ❌ 86,71%, quedas 13,29% | ❌ p10=3 (cap 0%) | CAIU |
| 34/12 gêmeo | titular | ✅ 92,81% | ✅ 92,56% | ❌ cap: **1 luta** em 240k | CAIU |
| 34/12 gêmeo | vizinho | ✅ 92,81% | ❌ 86,71%, quedas 13,29% | ❌ p10=3 | CAIU |
| 33/13 quente | titular | ✅ 92,31% | ✅ 92,00% | ❌ p10=3 (cap 0%) | CAIU |
| 33/13 quente | vizinho | ✅ 92,31% | ❌ 85,88%, quedas 14,12% | ❌ p10=3 | CAIU |
| 39/11 molde | titular | ✅ 92,62% | ✅ 92,36% | ❌ cap: **35 lutas** em 240k (p10=5 ok) | CAIU |
| 39/11 molde | vizinho | ✅ 92,62% | ❌ 85,59%, quedas 14,41% | ✅ (cap 0%, p10=4) | CAIU |

Três leituras imediatas:

1. **Ninguém caiu no P1, e ninguém sobreviveu ao par {P2 vizinho, P6}.** O braço
   titular aprova P1+P2 nos 4 candidatos — ou seja, a Fase B não está desmentida:
   tudo o que ela mediu se reproduziu. O que derrubou os candidatos foi
   exatamente o que a Fase C adicionou: a mira vizinha e o cenário tartaruga.
2. **33/12 e 34/12 caíram de forma IDÊNTICA** (mesmo degrau, mesmos números, até
   a mesma luta única no cap do P6 titular). O empate exato do desempate
   histórico sobreviveu à bateria completa: a escolha entre eles segue sendo pura
   identidade, sem custo de gameplay mensurável.
3. **O 39/11 é o único que passou num P6 (no braço vizinho) — e o único reprovado
   no P6 titular por uma margem que não é estatística** (35 lutas no cap, taxa
   1,46×10⁻⁴). A esponja de 39 HP faz a luta-tartaruga durar mediana de 10
   rodadas com p90 = 15: o perfil mais perto do exploit real de empacar a luta.

## 4. Anatomia da causa nº 1: o P2 no braço vizinho reprova nos 4 candidatos

Os números, pareados titular → vizinho no MESMO cenário (P2, Bento defende):

| Candidato | Vitória tit. → viz. | Quedas do Gus tit. → viz. | Duração média | HP final médio |
|---|---|---|---|---|
| 33/12 e 34/12 | 92,56% → **86,71%** (−5,85 pp) | 7,44% → **13,29%** (+5,85 pp) | 3,89 → 3,79 | 77,5% → 73,8% |
| 33/13 | 92,00% → **85,88%** (−6,12 pp) | 8,00% → **14,12%** | 3,88 → 3,77 | 73,8% → 69,8% |
| 39/11 | 92,36% → **85,59%** (−6,77 pp) | 7,64% → **14,41%** | 4,87 → 4,74 | 76,0% → 72,1% |

**Isto não é amostra pequena.** A distância da vitória até o piso de 90% é de
3,29 a 4,41 pontos percentuais, contra uma meia-largura de IC de 95% de ~0,14 pp
impressa pelo próprio harness: **24 a 31 vezes a incerteza** (em erros-padrão,
~47 a 62). As quedas estouram o teto de 12% por 1,29 a 2,41 pp (9 a 17 vezes a
meia-largura). Efeito real, forte e consistente nos 4 candidatos — e na mesma
direção nas 3 métricas: menos vitória, mais quedas, menos HP final, luta um pouco
mais curta (p10 cai de 4 para 3 rodadas nos candidatos Atk 12-13).

**A mecânica (hipótese consistente com os dados, marcada como tal):** no braço
titular, o inimigo distribui a mira sem olhar quem defende, e uma fração dos
golpes cai no Bento — que, defendendo, absorve com o Shield. No braço vizinho, o
peso ×0,5 desvia esses golpes para quem NÃO está defendendo; o Shield absorve
menos, o dano real distribuído na party sobe, e a luta encurta porque quem morre
é quem não tem mitigação. O acoplamento derrota = queda do Gus (E1 = E4, medido
desde a Fase A) segue exato célula a célula, inclusive aqui.

**Por que isso é um achado de DESIGN e não um detalhe de tuning:** nenhum ajuste
de HP/Atk do trash dentro da faixa estudada conserta isso — os 4 statlines, de
temperaturas bem diferentes, caem juntos e por margens parecidas. O que o braço
vizinho expõe é que **o valor defensivo do "defender" do jogo depende hoje de a
mira inimiga cooperar**. O escudo é probabilístico: existe enquanto o inimigo
mira nele por acaso. Qualquer IA futura minimamente esperta (e o braço vizinho é
só o degrau seguinte da escada tier-mira) transforma a ação de defender num
convite para o inimigo bater em quem dói. A seção 6 traz o que a indústria faz
com exatamente este problema.

## 5. Anatomia da causa nº 2: o gate P6 (tartaruga) reprova em 7 de 8 pontos

O gate exclusivo do P6, pré-registrado pelo líder em 2026-08-11: **cap de 30
rodadas atingido por 0,0% das lutas (ESTRITO) E p10 >= 4 rodadas.** Resultado por
ponto (derrota ~100% é o esperado do cenário; o gate não mede vitória):

| Candidato | Braço | Lutas no cap (de 240.000) | Duração (mediana, p10-p90) | Motivo da reprovação |
|---|---|---|---|---|
| 33/12 | titular | **1** (0,000417%) | 7, 4-11 | teto 0% estrito |
| 33/12 | vizinho | 0 | 5, 3-9 | **p10 = 3** (piso 4) |
| 34/12 | titular | **1** (0,000417%) | 7, 4-11 | teto 0% estrito |
| 34/12 | vizinho | 0 | 5, 3-9 | **p10 = 3** |
| 33/13 | titular | 0 | 5, 3-9 | **p10 = 3** |
| 33/13 | vizinho | 0 | 4, 3-7 | **p10 = 3** |
| 39/11 | titular | **35** (0,0146%) | 10, 5-15 | teto 0% estrito |
| 39/11 | vizinho | 0 | 7, 4-12 | APROVADO |

Os dois sub-modos pedem consertos diferentes:

### 5.1. O teto "0% estrito" (3 reprovações — e duas delas por UMA luta)

Nos pontos titulares do 33/12 e do 34/12, **uma única luta em 240.000** chegou à
rodada 30 e reprovou o ponto inteiro (o harness a contabiliza como "empate
técnico": 0,000417% = 1/240.000). Taxa observada ~4,2×10⁻⁶ — uma luta de cauda
extrema num cenário que, por construção, é o pior caso de alongamento (ninguém
ataca). Este sub-modo é em grande parte um problema do CRITÉRIO, demonstrado na
seção 8: um teto de "zero em N" com N grande reprova quase qualquer sistema real.

O 39/11 titular é diferente: **35 lutas** no cap (1,46×10⁻⁴), mediana de 10
rodadas, p90 de 15. Isso não é cauda rara — é o perfil esponja genuinamente
próximo do exploit. O conserto de critério da seção 8 NÃO o salvaria num teto de
10⁻⁴; e não deveria salvar.

### 5.2. O piso p10 >= 4 (4 reprovações, todas com p10 = 3)

Sob a mira vizinha (e, no 33/13, até sob a titular), 10% ou mais das
lutas-tartaruga terminam em 3 rodadas ou menos: a party inteira defendendo morre
quase tão rápido quanto lutando. **Este sub-modo não é estatístico — é o mesmo
problema de design da causa nº 1 visto pelo outro lado:** quando a mira desconta
quem defende, defender deixa de comprar tempo. O pré-registro pedia que a defesa
total comprasse pelo menos 4 rodadas; com o F4 suave ela não compra. Qualquer
conserto da mira/defesa (seção 6) precisa ser re-medido também aqui.

### 5.3. Os informativos (P3/P3B), em uma nota

Como a Fase B já tinha medido, e por isso o líder os marcou informativos: a cura
12 (motor canônico) e o P3B (bateria) trivializam tudo em todos os candidatos e
braços — vitória 100%, HP final 91,2-96,7%, as lutas mais fáceis da grade. A cura
2 aprovou APENAS no 33/13, nos dois braços (96,03%, quedas 3,97%) — reproduzindo
a Fase B — e reprovou por "seguro demais" no 33/12-34/12 (99,33%) e no 39/11
(99,23%). Nada disso muda a conclusão da Fase B: a resposta da curandeira é
REGRA (onde/quando a cura entra, medida em multi-encontro), não número; nenhum
finalista de cura existe.

## 6. Pesquisa de design: o problema da mira que pune quem defende

Pesquisa externa conduzida por `lead-game-designer` a pedido do líder, ANTES
desta recomendação. O padrão dominante da indústria para o problema exato da
causa nº 1 **não é suavizar a IA — é dar ao jogador uma alavanca EXPLÍCITA de
controle de mira.** Três formas com precedente forte, em ordem crescente de
determinismo:

1. **Aggro/enmity explícito** (Xenoblade Chronicles 1/3): cada personagem tem uma
   tabela de aggro que as ações alimentam; **tomar dano REDUZ o próprio aggro** —
   o oposto do que a mira ponderada do GusWorld faz hoje — e tanks têm decaimento
   de aggro mais lento como vantagem estrutural. O jogador vê e manipula a mira
   como sistema, em vez de sofrê-la como sorteio.
2. **Taunt como ação separada da defesa** (Final Fantasy XIII, role Sentinel):
   Provoke/Challenge (redireciona a mira) é uma ação DIFERENTE de Steelguard
   (reduz dano). O Sentinel em nível máximo toma ×0,50 de dano e os ALIADOS tomam
   ×0,86 só pela role estar ativa — **o "escudo humano" é oficializado como
   mecânica, não deixado como efeito colateral estatístico da mira.** É a
   tradução mais direta para o kit do Bento: defender protege a si; provocar
   protege os outros; o jogador escolhe.
3. **Guard-redirect determinístico** (Darkest Dungeon): "Guard" redireciona 100%
   dos ataques diretos ao guardião por ~3 turnos, com furos DECLARADOS (AoE
   ignora, stun cancela, riposte fura). Propriedade decisiva para o nosso caso:
   **elimina a sensibilidade da taxa de vitória ao peso da mira**, porque o
   escudo deixa de ser probabilístico — o braço vizinho e o titular convergiriam
   por construção no cenário parede.

**Alerta da pesquisa contra o conserto preguiçoso** ("só aumentar a redução de
dano do defender"): Robert Boyd (Zeboyd Games, "Adding Depth to Defend")
argumenta que Defend que apenas reduz dano é um comando quase sempre
dominado/ruim; os padrões que funcionam dão efeito secundário (Grandia: adianta o
próximo turno; Bravely Default: acumula recurso para ação futura; Lost Odyssey:
protege a fileira de trás). E uma nota psicológica que pesa no tom do jogo: Sid
Meier (GDC 2010) — jogadores não processam probabilidade simetricamente; uma
mira que "pune" quem defende é lida como injustiça, mesmo sendo matematicamente
pequena (×0,5). Num jogo cujo protagonista resolve por lógica (Pillar 4), um
sistema de mira legível e manipulável é mais coerente do que um sorteio
ponderado que o jogador não enxerga.

**Nenhuma dessas três opções é escolhida aqui.** São as opções, com fontes, para
o líder decidir — inclusive a opção de manter a mira como está e aceitar que o
braço vizinho nunca será exigência (mas aí o pré-registro da próxima rodada deve
dizer isso explicitamente, e o custo fica registrado: a defesa continua
dependente da cooperação da IA).

## 7. Pesquisa de design: o problema da tartaruga e do teto de rodadas

Para o exploit de empacar a luta (e para o cap de 30 rodadas que o P6 vigia), a
pesquisa levantou quatro famílias de precedente:

1. **Pressão crescente em vez de teto rígido — "soft enrage"** (WoW, Gruul: +15%
   de dano a cada 30 s até a luta ficar insustentável). O racional oficial da
   Blizzard é EXATAMENTE o problema do P6: sem enrage, bastaria sustentação
   infinita (curandeiros que não ficam sem mana) para esticar a luta
   indefinidamente. **O soft enrage não é um teto — é uma pressão que torna o
   teto praticamente inalcançável, mantendo o cap só como rede de segurança.**
   Traduzido para o GusWorld: a partir da rodada X, o trash ganha +Y de Atk por
   rodada; a luta-tartaruga termina sozinha, cedo, e o cap de 30 vira
   formalidade que nenhuma luta alcança por margem estrutural, não por sorte.
2. **Retorno decrescente na repetição** (Pokémon, Protect: taxa de sucesso cai a
   1/3 por uso consecutivo, até 1/729 na 7ª tentativa; WoW, diminishing returns
   de CC: 100%/50%/25%/imune). Aplicado ao defender: defender rodadas seguidas
   rende cada vez menos mitigação — a tartaruga total deixa de ser estável sem
   punir a defesa pontual (que é a que o jogador legítimo usa).
3. **Chip damage que atravessa defesa** (Slay the Spire: Block não acumula entre
   turnos, e Poison/perda direta de HP ignoram Block). Garante que defesa nunca é
   imunidade — sempre há um dreno que só a vitória estanca.
4. **Desenho em camadas do próprio teto** (xadrez, regra dos 50/75 lances: a de
   50 é reivindicável, a de 75 é automática): precedente de que "fim forçado de
   partida longa" pode ter uma camada suave antes da dura.

As famílias 1-3 são mecânicas de jogo (decisão de design do líder, one-way door
leve); a família 4 é sobre como o cap em si termina a luta (hoje: empate
técnico). Elas se combinam: um soft enrage bem calibrado provavelmente esvazia o
gate do cap sozinho, e é a opção que menos toca o kit dos personagens.

## 8. O critério "0% estrito" em si: o que a estatística permite pedir

Este é o achado mais importante da fase para o MÉTODO do estudo, e vem com
fontes de fora de jogos:

1. **Zero observado nunca demonstra taxa zero** (regra de três de Hanley &
   Lippman-Hand, JAMA 1983): se 240.000 lutas tivessem dado ZERO no cap, isso
   provaria apenas taxa <= 3/240.000 = 1,25×10⁻⁵ (1 em 80.000) com 95% de
   confiança. O critério pré-registrado ("0,0% estrito") pedia uma demonstração
   de ausência que nenhum N finito entrega.
2. **E o critério fica mais duro quanto maior a amostra** (law of truly large
   numbers, Diaconis & Mosteller, JASA 1989): com taxa verdadeira ~4×10⁻⁶ (a
   observada nos pontos de 1 luta), a probabilidade de ver >=1 evento é ~9% em
   N=24.000, **~62% em N=240.000** e ~98% em N=1.000.000. Ou seja: o mesmo
   sistema, com o mesmo comportamento, passa ou reprova conforme o tamanho da
   amostra — o critério mede o N, não o jogo. A disciplina da casa de "amostra
   cheia sempre" e o critério "zero estrito" são incompatíveis por matemática,
   não por azar.
3. **Com o dado real desta rodada:** 1 evento observado em 240.000 dá limite
   superior de 95% (Poisson exato) ≈ 2,0×10⁻⁵ (1 em ~50.000). Aplicando tetos de
   taxa em vez de "0% estrito": **33/12 e 34/12 passariam num teto de 10⁻⁴ e
   reprovariam num teto de 10⁻⁵** (o limite superior 2,0×10⁻⁵ excede 10⁻⁵). O
   39/11 titular (35 eventos, taxa 1,46×10⁻⁴) reprovaria até no teto de 10⁻⁴.
   A decisão volta a ser do líder, com número — não refém do N.
4. **Detalhe que amarra teto e amostra:** um teto de 10⁻⁵ é INDEMONSTRÁVEL com
   N=240.000 mesmo com zero eventos observados (o limite superior de zero
   eventos, 1,25×10⁻⁵, já excede o teto). Se o líder quiser um teto de 10⁻⁵
   provado a 95%, a rodada precisa de N >= 300.000 com zero eventos (3/N <=
   10⁻⁵). Teto e N se escolhem JUNTOS, no pré-registro.
5. **Precedente industrial fora de jogos**: até o controle de qualidade que
   "exige zero defeitos na amostra" (c=0 acceptance sampling, ASQ/Squeglia; ISO
   28594) formaliza isso como decisão sob risco quantificado (curva OC), nunca
   como demonstração de ausência.

**O que esta seção NÃO faz: não reinterpreta a rodada atual.** O pré-registro
não se mexe depois de ver o dado — o veredicto desta Fase C é 0 de 4 pelo
critério que estava escrito, e assim fica registrado. A troca de "0% estrito"
por "teto de taxa com limite superior de confiança" é **PROPOSTA para o PRÓXIMO
pré-registro**, a fixar pelo líder ANTES de a próxima rodada rodar — mesma
disciplina do gate E2 novo entre a Fase A e a A-bis (furo admitido, conserto
pré-registrado antes da fase seguinte, e o conserto não protege candidato meu:
com teto de 10⁻⁴, o 39/11 continua reprovado, e o p10=3 continua reprovando os
outros no braço vizinho — o conserto de critério não salva ninguém desta rodada).

## 9. ⚠️ RESSALVA OBRIGATÓRIA: a mira ponderada ainda não está ligada em produção

Repetida das três análises anteriores com a mesma força, porque nada mudou (o
cabeçalho do próprio artefato a reimprime): segundo a análise de projeto, o QA
adversarial teria provado (verificação contra o blob commitado) que
**`mira_target_weight`/`pick_weighted_enemy_target` não teriam nenhum call site de
produção fora dos testes** — o cérebro inimigo em jogo (`scripted_brain.cpp`)
continuaria mirando `players.front()`, o primeiro da lista. O item
`IA-ALVO-PRIMEIRO-DA-LISTA` segue pendente de decisão do líder.

> ⚠️ **Não verificado contra código.** Esta afirmação sobre o QA adversarial, a
> verificação contra blob commitado, `mira_target_weight`/`pick_weighted_enemy_target`
> e `scripted_brain.cpp` vem da análise de projeto, não de medição em código
> executado ou em blob commitado real. O código deste projeto nasce do zero
> (GODS_LAWS.md, L-01), e a fonte que a redação original citava não existe. Revalidar
> quando houver implementação.

- **Os 48 pontos desta Fase C descrevem o jogo COM a mira ponderada ativa**
  (braço titular = Opção F, F1+F2+F3, sem F4, o modelo aprovado em
  `MIRA-PONDERADA-PROD`; braço vizinho = F4 suave, harness-only de propósito).
- **Uma partida jogada HOJE não se comporta assim**: o trash real concentra tudo
  no primeiro da lista.
- **A trava de paridade protege contra deriva de fórmula, não contra ausência de
  uso.**
- Sequência recomendada (inalterada): resolver `IA-ALVO-PRIMEIRO-DA-LISTA` ANTES
  de qualquer número deste estudo virar canon em `combat.md`. Se o líder decidir
  NÃO ligar a mira ponderada, as Fases A-bis, B e C precisam ser re-rodadas sob a
  mira primeiro-da-lista. E há agora um agravante desta fase: **a decisão de
  design da defesa/mira (seção 6) e o wiring da mira são a MESMA conversa** — não
  faz sentido ligar a mira ponderada em produção e logo depois trocá-la por
  aggro/taunt/guard-redirect; a ordem certa é decidir o desenho da defesa
  primeiro e ligar em produção a mira que sobreviver a essa decisão.

## 10. Comparação com a Fase B

| Aspecto | Fase B (2026-08-11) | Fase C (2026-08-11) |
|---|---|---|
| Pergunta | refino fino: quais células aprovam? | confirmação adversarial: os finalistas aguentam a bateria completa? |
| Grade | 64 pontos, 1 braço de mira (titular) | 48 pontos, 2 braços × 6 cenários |
| Cenários | P1, P2, P3, P3B | + **P6 tartaruga** e braço de mira vizinho |
| Aprovados | 16 de 64 (camada de ponto) | 15 de 48 (ponto); **0 de 4 (candidato)** |
| Incumbente 33/12 | re-confirmado por equivalência | aprova P1+P2 titular (3ª reprodução exata); cai no P6 (1 luta) e no braço vizinho |
| Desempate 33 vs 34 | empate bit a bit | **empate bit a bit de novo, em 12 células** — inclusive na queda |
| Curandeira | 0 finalistas; cura 2 aprova em 3 células | idem, reproduzido (informativo); cura 2 só aprova no 33/13 |
| Mira ponderada | ressalva (sem call site) | ressalva mantida, agravada pela decisão de design pendente |
| O que caiu | células individuais, por pressão | **o desenho** (defesa punível pela mira; tartaruga sem pressão) e **o critério** (0% estrito) |

O que não mudou: a física simulada (toda célula compartilhada reproduz bit a
bit), a escada de degraus, o acoplamento derrota = queda do Gus, a trivialização
da cura alta. O que mudou: a Fase C não desmentiu nenhum número da B — ela
adicionou os testes que a B não fazia, e foi neles que tudo caiu. É o resultado
correto de uma bateria de confirmação bem desenhada: ou ela confirma, ou ela
ensina onde o desenho cede. Desta vez ensinou.

## 11. Recomendação final ao líder: o próximo passo (não há vencedor a escolher)

**Não há corte de finalista a fazer: 0 de 4 sobreviveram, e esta análise não
promove nenhum número.** O rodapé do harness continua valendo: o corte é humano,
nunca do harness — e o corte humano honesto desta rodada é "nenhum, ainda".

Registro também o que os dados NÃO dizem: eles não dizem que o incumbente 33/12
está errado para o jogo como ele é hoje. Sob a mira titular (a única aprovada
para produção), ele passa P1 e P2 com folga pela terceira medição consecutiva, e
a única reprovação titular dele é a luta 1-em-240.000 do cap — exatamente o caso
que a seção 8 mostra ser mais critério do que jogo. **O canon vigente não precisa
de mudança de emergência; precisa que as três pendências abaixo sejam decididas
antes da próxima rodada de confirmação.** Proposta de sequência (toda decisão
via AskUserQuestion, pelo orquestrador):

1. **Decisão de design da defesa/mira (causa nº 1 e sub-modo p10).** Opções da
   seção 6, em resumo: (a) aggro explícito manipulável (Xenoblade), (b) taunt
   separado da defesa no kit do Bento (FFXIII Sentinel), (c) guard-redirect
   determinístico (Darkest Dungeon) — a opção (c) é a que, por construção, apaga
   a sensibilidade ao peso da mira; (d) manter como está e declarar no
   pré-registro que o braço vizinho não é exigência (com o custo registrado).
   Junto dela, a decisão irmã que já estava pendente: `IA-ALVO-PRIMEIRO-DA-LISTA`
   (seção 9) — decidir o desenho ANTES de ligar qualquer mira em produção.
2. **Decisão de design anti-tartaruga (causa nº 2, sub-modo cap).** Opções da
   seção 7: soft enrage (a mais barata e a que menos toca o kit), retorno
   decrescente do defender, chip damage. O Shield binário do defensor (pendência
   de regra com dono desde a Fase A) entra naturalmente nesta mesma conversa.
3. **Pré-registro novo do critério do cap (seção 8):** trocar "0,0% estrito" por
   teto de taxa com limite superior de confiança de 95%, com o teto (10⁻⁴?
   10⁻⁵?) e o N escolhidos juntos e fixados ANTES do dado. Se o piso p10 >= 4 do
   P6 deve valer sob qual(is) mira(s) também se fixa aí.
4. **Só então, nova rodada de confirmação** (Fase C-bis) com o desenho decidido e
   o critério novo — mesmos 4 candidatos, salvo ordem contrária do líder, porque
   nenhum foi desqualificado por ser um número ruim: foram derrubados por
   propriedades do sistema em volta deles.

**O que esta análise NÃO faz:** não toca `TODO.md` nem `combat.md`, não canoniza
número nenhum, não escolhe opção de design, não dispara rodada nova. Toda
decisão listada acima chega ao líder pelo orquestrador via AskUserQuestion.

---

*Análise escrita por Capitolino (CPO) em 2026-08-11 sobre o artefato congelado da
Fase C. Pesquisa de design externa conduzida por lead-game-designer a pedido do
líder. Não é canon. Não commitada por decisão de processo (commit e push só com
ordem do líder).*
