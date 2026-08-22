# ANÁLISE: Fase C-bis do estudo de pacing (Provocar + soft enrage + Bento real, 56 pontos)

> **Status: ANÁLISE (rascunho pronto para revisão), não canon.** Nenhum número deste
> documento entra em `combat.md` nem no motor sem decisão explícita do líder. Autor:
> Capitolino (CPO), 2026-08-12. Dados: artefato congelado
> `.../scratchpad/cbis_oficial/pacing_sim_phase_c_bis_report.txt`
> (md5 `74f739173572af5c1a7564b7209a7a69`, 584.533 bytes, conferido nas DUAS pontas da
> análise — antes da primeira leitura, 02:19:52 de 2026-08-12, e de novo ao término;
> mtime 2026-08-12 02:17:55 -0300). Run oficial: 56 pontos (4 candidatos × 2 braços de
> mira × 7 slots de cenário), N = 300.000 por ponto = **16,8 milhões de lutas**, seed
> base 20260801 (a mesma do estudo inteiro), disparo por
> `GUSWORLD_PACING_SIM_PHASE_C_BIS_FULL`, executado pelo orquestrador (instrução do
> próprio código de teste). **Zero erros internos declarados em 56 de 56 pontos.**
> Pré-registro: `proposta-protocolo-simulacao-pacing.md` §9, ASSINADO pelo líder em
> 2026-08-11. Reconferência de Def 13 pré-run: `spec-provocar-soft-enrage-criterio-cap.md`
> §8 (commit `f17acd4c`). Docs-irmãos: `analise-pacing-fase-a-bis-20260810.md`,
> `analise-pacing-fase-b-20260811.md`, `analise-pacing-fase-c-20260811.md`.

---

## Resumo executivo (em linguagem simples)

1. **Os dois problemas sistêmicos que derrubaram a Fase C estão CONSERTADOS, e agora
   medidos com N pleno.** (a) A sensibilidade ao braço de mira — a diferença de vitória
   entre a mira titular e a vizinha no cenário-parede — caiu de **5,85-6,77 pontos
   percentuais na Fase C para 0,1pp** com Provocar ligado: o gate G5, que é a pergunta
   que a Fase C fez, **passou nos 4 candidatos**. (b) A tartaruga terminou: com soft
   enrage, **nenhuma das 2,4 milhões de lutas P6 da grade chegou ao cap de 30 rodadas**
   (k = 0 nos 8 pontos, zero declarado), p99 de 10-13 rodadas, máximo absoluto da grade
   inteira 16 rodadas. Os gates H1-H5 passaram TODOS, nos 4 candidatos — inclusive H2,
   que prova com delta pareado por seed que a luta saudável não sente o enrage existir
   (deltas exatamente 0,0).
2. **Mesmo assim, 0 de 4 candidatos sobreviveram à bateria — pelo critério do harness E
   pelo critério corrigido.** O harness (que ainda trata G2 e G4 como decisivos) reprova
   os 4 por G1+G2+G3+G4. Aplicando a leitura correta — G2 informativo por decisão do
   líder de 2026-08-11, G4 informativo pelo plano B pré-declarado (o empate 0%×0% se
   confirmou EXATO no run oficial, seção 4) — sobram **duas reprovações decisivas, G1 e
   G3, e elas reprovam os 4 candidatos igualmente**. Não há finalista, e esta análise
   não promove nenhum.
3. **A natureza da reprovação mudou — e isso é a notícia.** A Fase C caiu por
   propriedades do DESENHO (defesa punível pela mira, tartaruga sem término) que exigiam
   decisão de design nova. A C-bis cai por **calibragem da alavanca nova**, e as duas
   reprovações têm **correção pré-declarada no pré-registro §9.3, fixada antes do
   dado**: G1 (o provocador atrai golpes DEMAIS: 85,1-85,7% no braço titular, teto 85%)
   se corrige mexendo em `kMiraProvokeMultiplier`, e só nele; G3 (Provocar+Defender
   trivializa: vitória 100,0%/99,9%, teto 97%) se corrige tornando as duas ações
   mutuamente exclusivas, sem mexer no multiplicador.
4. **Ressalva honesta sobre a correção do G3, visível no dado do P7b:** o cenário
   informativo P7b (provocar SEM defender) também venceu ~100% das lutas (99,996% no
   ponto impresso), com o provocador caindo em **0% delas**. Ou seja, a trivialização do
   cenário-parede-com-alavanca **não vem da combinação com Defender** — vem do próprio
   redirecionamento para um tanque que não cai. A exclusividade pré-declarada
   provavelmente não desce a vitória abaixo de 97 sozinha; baixar o multiplicador (a
   correção do G1) é a alavanca que mexe nas três reprovações ao mesmo tempo (G1, G3 e o
   empate do G4), porque as três são a mesma física. Seções 5, 6 e 8.
5. **G2 (informativo) reprovou de novo, como esperado e por isso mesmo é informativo:**
   no braço vizinho, o P2 clássico (defender sem provocar) segue em 85,8-86,9% de
   vitória com 13,1-14,2% de quedas do Gus — números quase idênticos aos da Fase C. É a
   pendência real do design da defesa-sem-alavanca, registrada, não resolvida por
   Provocar (nem deveria: G2 mede exatamente o cenário em que a alavanca NÃO é usada).
   No P7, com a alavanca, o mesmo braço vizinho dá 99,9% de vitória e 0,1% de quedas.
6. **Validade interna forte:** o empate bit a bit 33/12 × 34/12 se reproduziu pela
   **quarta medição consecutiva**, agora também nos cenários novos (P7/P7b, célula a
   célula); os pontos-controle sem enrage são bit-idênticos aos com enrage (coerente com
   H1 = 0,0%: nenhuma luta saudável alcança a rodada 8, então o enrage é literalmente
   inerte nelas — e o pareamento por seed prova isso com delta 0,0 exato, não
   aproximado); zero erros internos declarados em 56 de 56 pontos.
7. **⚠️ RESSALVA OBRIGATÓRIA (a mesma das quatro análises anteriores, com a mesma
   força):** segundo a análise de projeto, a mira ponderada seguiria **sem call site de
   produção** (`scripted_brain.cpp` miraria `players.front()`). Estes 56 pontos
   descrevem o jogo COM a mira planejada ativa. Nenhum número daqui vira canon antes de
   `IA-ALVO-PRIMEIRO-DA-LISTA` ser decidido. Seção 10.

   > ⚠️ **Não verificado contra código.** Esta afirmação sobre `scripted_brain.cpp` e
   > sobre o call site de produção vem da análise de projeto, não de medição em código
   > executado. O código deste projeto nasce do zero (GODS_LAWS.md, L-01), e a fonte
   > que a redação original citava não existe. Revalidar quando houver implementação.

---

## 1. O que a C-bis é

Re-rodada da bateria de confirmação da Fase C com o sistema em volta consertado: a ação
**Provocar** no motor real (PROV-1/PROV-2), o **soft enrage** na FSM real (ENRAGE-1/2), o
**critério do cap trocado de "0% estrito" por teto de taxa com UB95** (SIM-RARE, tripla
10⁻⁴ / 95% / N=300.000 decidida pelo líder), e o **Bento real** (HP 55/Atk 10/Def 13/SPD
5, `combat.md` §17) no lugar da Jaci-proxy nos cenários de parede/provocação
(`BENTO-STATLINE-COMBATE`; P1/P6 seguem com a Jaci por desenho declarado). Grade:

> ⚠️ **Não verificado contra código.** As expressões "motor real" e "FSM real" acima
> descrevem a premissa da análise de projeto, não medição em código executado. O
> código deste projeto nasce do zero (GODS_LAWS.md, L-01), e a fonte que a redação
> original citava não existe. Os parâmetros de grade seguem válidos como
> especificação de protocolo; revalidar a execução quando houver implementação.

- **4 candidatos** (os mesmos da Fase C, por decisão do líder — nenhum foi derrubado lá
  por ser número ruim): 33/12 controle, 34/12 gêmeo, 33/13 quente, 39/11 molde-esponja.
- **2 braços de mira**: `F_sem_F4` (titular de produção) e `C_F4_suave` (vizinho,
  robustez, harness-only de propósito).
- **7 slots de cenário**: P1 e P2 com enrage LIGADO (canônicos) e DESLIGADO (controles
  pareados por seed, existem SÓ para o delta de H2), P6 tartaruga com enrage, P7 (parede
  com provocação: provoca E defende — onde G1, G3, G4, G5, G6 são medidos) e P7b
  (provocar sem defender, informativo, nenhum gate reprova por ele).
- Fora da grade, declarado em vez de silenciado: elite (0 pontos, congelado desde
  2026-08-01), P3/P3B (o pré-registro §9.1 não os lista), P7c/P7d (cobertura de teste
  das exceções E5/E6 da spec, não pontos oficiais).

**Quem decide:** os gates G1-G6 (§9.3) e H1-H5 (§9.4) por candidato, com a regra
fail-closed do harness ("gate NÃO AVALIADO não aprova"). Os guarda-corpos por PONTO
(E1/E2/E4/E8/E9) seguem impressos como leitura auxiliar: 28 de 56 pontos aprovaram neles
(as reprovações de ponto são dominadas por P7/P7b vencendo acima do teto E1 de 97% e pelo
P6, que perde ~100% por construção — nenhuma delas decide a fase).

## 2. Checagem de validade interna

- **Hash nas duas pontas:** md5 `74f739173572af5c1a7564b7209a7a69` conferido antes da
  primeira leitura e re-conferido ao término da análise, idêntico ao congelado pelo
  orquestrador. Artefato estável durante toda a leitura.
- **Zero erros internos declarados em 56 de 56 pontos** (a linha
  `erros internos: 0 de 300000` aparece exatamente 56 vezes; nenhuma linha não-zero
  existe). Zero declarado, não presumido.
- **Quarta reprodução do empate de degrau 33/12 × 34/12.** As células compartilhadas dos
  dois são idênticas número a número TAMBÉM nos cenários novos: G1 85,3%/77,8% nos dois,
  P2 vizinho 86,9%/13,1063% nos dois, P6 célula a célula (até o E6 de absorção,
  80,5735% ± 0,0156 idêntico), P7b com as mesmas quedas 0,009%. O empate do desempate
  histórico sobrevive à C-bis inteira.
- **Controle pareado por seed provado bit a bit:** os pontos P1/P2 com enrage DESLIGADO
  reproduzem os LIGADOS exatamente (ex.: quedas do Gus 7,21733% nos dois lados do par).
  Não é coincidência, é física: H1 mediu 0,0% de lutas alcançando a rodada 8, então o
  enrage nunca disparou em luta saudável — e o pareamento por seed transforma isso em
  identidade bit a bit, que é o delta 0,0 que H2 exige. O harness mediu o controle DENTRO
  do run (não herdou da Fase C), como o pré-registro §9.5 item 6 manda.
- **A party impressa em todo ponto** (linha PARTY): parede/provocação com
  `bento HP55/Atk10/Def13/SPD5`, P1/P6 com a Jaci — exatamente o desenho declarado em
  `BENTO-STATLINE-COMBATE`, e o rodapé de quedas rotula o ocupante real do slot.
- **Condição de validade do enrage conferida na data do run** (cabeçalho do artefato):
  mitigação da party limitada por construção (Shield pool = Def, cura por bateria), como
  o §9.4 exige para a âncora analítica de H3/H5 valer.

## 3. Camada 1: os gates como o harness os imprimiu (G2 e G4 ainda decisivos)

Vereditos por candidato, transcritos do bloco `GATES DO PRE-REGISTRO` do artefato
(linhas 7485-7626). O harness ainda trata os 11 gates como decisivos — a leitura
corrigida é a seção 4.

| Gate | Critério | 33/12 controle | 34/12 gêmeo | 33/13 quente | 39/11 molde |
|---|---|---|---|---|---|
| **G1** fatia de golpes no provocador (P7, 2 braços) | 70-85% | **FAIL**: tit **85,3 [FORA]**, viz 77,8 ok | **FAIL**: tit **85,3**, viz 77,8 | **FAIL**: tit **85,7**, viz 78,2 | **FAIL**: tit **85,1**, viz 74,0 |
| **G2** vitória/quedas Gus em P2 e P7, braço vizinho | ≥90% e ≤12% | **FAIL**: P2 **86,9/13,1**; P7 99,9/0,1 ok | **FAIL**: P2 **86,9/13,1** | **FAIL**: P2 **86,1/13,9** | **FAIL**: P2 **85,8/14,2** |
| **G3** vitória no P7 (provoca+defende) | ≤97% | **FAIL**: tit **100,0**, viz **99,9** | **FAIL**: 100,0/99,9 | **FAIL**: 100,0/99,9 | **FAIL**: 100,0/99,9 |
| **G4** quedas do provocador: P7 > P2 estrito | P7 > P2 | **FAIL**: 0,0 vs 0,0 nos 2 braços | **FAIL**: 0,0 vs 0,0 | **FAIL**: 0,0 vs 0,0 | **FAIL**: 0,0 vs 0,0 |
| **G5** diferença de vitória no P7 entre braços | ≤5pp | **PASS**: **0,1pp** | **PASS**: 0,1pp | **PASS**: 0,1pp | **PASS**: 0,1pp |
| **G6** duração no P7 (régua E2 herdada, AMB-02) | janela 3-5 | **PASS**: mediana 4, janela 100% | **PASS**: idem | **PASS**: mediana 4, janela 100/99,9% | **PASS**: mediana 5, janela 100% |
| **H1** % de lutas P1/P2 que alcançam R0=8 | ≤1,0% | **PASS**: 0,0% nos 4 pontos | **PASS** | **PASS** | **PASS** |
| **H2** delta com × sem enrage em P1/P2 | ≤5pp | **PASS**: todos os deltas 0,0 | **PASS** | **PASS** | **PASS** |
| **H3** p99 de duração no P6 | ≤20 rodadas | **PASS**: tit 11, viz 11 | **PASS**: 11/11 | **PASS**: 11/10 | **PASS**: 13/12 |
| **H4** cap de 30 no P6 (tripla do evento raro) | UB95/N ≤ 10⁻⁴ | **PASS**: k=0, UB95=2,996 → 9,986×10⁻⁶, nos 2 braços | **PASS**: idem | **PASS**: idem | **PASS**: idem |
| **Veredito do harness** | todos verdes | **REPROVOU** | **REPROVOU** | **REPROVOU** | **REPROVOU** |

**H5 (global da grade, não por candidato): PASS** — máximo observado na grade INTEIRA =
**16 rodadas** (ponto: P6 do 39/11, braço titular, enrage ligado), contra o teto estrito
de 20. Linha final do harness: *"Fase C-bis concluida: 0 de 4 candidatos aprovaram em
TODOS os gates G1-G6/H1-H4, com H5 (global) verde"*.

Detalhe de impressão que o pré-registro exigia e o harness cumpriu: o H4 sai com
`k=0 de N=300000 | taxa=0.000e+00 | UB95=2.996 eventos = 9.986e-06/tentativa | teto=1.000e-04`
mesmo com k = 0 — zero declarado, nunca zero presumido. E nos pontos sem provocador
(P1/P2/P6) o rodapé de mira declara *"G1/G4 nao se aplicam a este ponto (zero declarado,
nao ausencia de medida)"*.

## 4. Camada 2: o critério aplicado, com G2 e G4 informativos

**Por que G2 não decide nesta rodada:** decisão do líder de 2026-08-11 (registrada no
item `PACING-FASE-C-BIS` do TODO.md) — G2 mede exatamente o que a Fase C já mediu, sem
nenhuma ferramenta nova (Provocar/enrage não tocam o P2: H2 provou delta 0,0). Segue
impresso e registrado como pendência real do design da defesa (seção 8.1).

**Por que G4 não decide nesta rodada — e a verificação da condição exata do plano B:** o
protocolo §9.3 pré-declara: *"Se G4 EMPATAR em 0%×0% ... a correção pré-declarada é `G4
vira informativo`"*. A condição é o empate EXATO, não uma isenção geral. Conferido no
dado oficial, candidato a candidato: as quedas do provocador (Bento) são **0% no P7 E no
P2, nos DOIS braços, nos QUATRO candidatos** — o E4 cru confirma `bento=0%` em todos os
16 pontos relevantes (8 de P2, 8 de P7), sem arredondamento escondendo queda residual
(o campo sai zerado, ± 0). **O empate 0%×0% se confirmou; o plano B se aplica; G4 vira
informativo nesta leitura** — continua medido e impresso (tabela da seção 3), deixa de
decidir. Se qualquer célula tivesse mostrado o provocador caindo, G4 teria permanecido
decisivo; não foi o caso.

**Gates decisivos restantes: G1, G3, G5, G6, H1, H2, H3, H4 (por candidato) + H5
(global).** Recalculando:

| Candidato | G1 | G3 | G5 | G6 | H1 | H2 | H3 | H4 | H5 global | **Veredito corrigido** |
|---|---|---|---|---|---|---|---|---|---|---|
| 33/12 controle | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | **REPROVADO (G1+G3)** |
| 34/12 gêmeo | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | **REPROVADO (G1+G3)** |
| 33/13 quente | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | **REPROVADO (G1+G3)** |
| 39/11 molde | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | **REPROVADO (G1+G3)** |

**Sobreviventes à bateria completa com o critério corrigido: 0 de 4.** A correção de
leitura (G2/G4 informativos) não salvou ninguém — o que reprova agora é o par G1+G3, e
ele reprova os 4 candidatos de forma quase idêntica, o que aponta para a CALIBRAGEM da
alavanca (multiplicador, combinação de ações), não para os statlines. Nenhum candidato
foi desqualificado por número próprio em NENHUMA das duas fases de confirmação.

## 5. Anatomia da reprovação nº 1: G1 estourou o teto por cima, só no braço titular

Os números: titular 85,3% (33/12 e 34/12), 85,7% (33/13), 85,1% (39/11) — teto 85,0%.
Margens de 0,1 a 0,7pp na precisão impressa (uma casa decimal). O braço vizinho passou
com folga nos 4 (74,0-78,2%).

- **Não é ruído nos candidatos Atk 12-13:** a fatia é medida sobre ~2,1 milhões de
  golpes por ponto; mesmo tratando a luta (300.000 por ponto) como unidade de
  independência, o erro-padrão fica na casa de centésimos de pp. 85,3 vs 85,0 é real.
  **No 39/11 (85,1%) a margem é de 0,1pp na precisão impressa** — a reprovação mais
  fina da grade; registro isso com honestidade, e registro também que a régua
  pré-registrada não tem cláusula de tolerância: FORA é FORA.
- **A folga analítica de 0,7pp do Caetano não se materializou — inverteu.** A
  reconferência pré-run (spec §8.1) previa 83,8% no P7 com a party sã e topo de 84,3% no
  P7b; o medido foi 85,3% no P7 e 86,9-87,2% no P7b. **Hipótese consistente com os
  dados, marcada como tal:** a tabela analítica é estática, e a dinâmica realimenta — o
  provocador concentra os golpes, se fere, e o F1 dele (dano recebido) multiplica por 12
  junto com o taunt: a linha "provocador ferido pela metade, aliados sãos" da mesma
  fórmula dá 1848/(1848+242) ≈ 88,4%, e a mistura medida (85,3%) fica entre a estática e
  essa. Os aliados, protegidos, nunca sobem o próprio peso. O erro da previsão foi de
  modelo (estática × dinâmica), não de aritmética.
- **Correção pré-declarada (§9.3, fixada antes do dado):** mexer em
  `kMiraProvokeMultiplier` (hoje 12,0; faixa útil declarada 6,0-20,0), **e só nele**.
  A direção é para baixo. Nenhum valor novo é proposto aqui — o número é decisão do
  líder com o `lead-game-designer`, e a rodada seguinte o mede.

## 6. Anatomia da reprovação nº 2: G3 (trivialização), e o que o P7b revela sobre a correção

G3 mede a vitória no P7 com o provocador também defendendo: teto 97%, medido
**100,0% (titular) e 99,9% (vizinho) nos 4 candidatos**. Quedas do Gus no P7:
0,01-0,14%. A parede com a alavanca ligada virou a luta mais segura da grade.

O pré-registro fixa a correção: **tornar Provocar e Defender mutuamente exclusivos**, e
NÃO mexer no multiplicador por causa de G3. Mas o dado informativo do P7b precisa estar
na mesa quando o líder decidir:

- **P7b (provocar SEM defender) também venceu ~100%** (99,996% no ponto do 39/11
  vizinho, impresso com o guarda-corpo E1 do ponto reprovando por "seguro demais";
  mesma ordem nos demais), com **quedas do provocador = 0% nos 8 pontos P7b** e quedas
  do Gus de 0,004-0,14%.
- Ou seja: **a trivialização não vem da combinação com Defender — vem do
  redirecionamento em si**, para um tanque (Def 13, HP 55) que os inimigos de trash não
  derrubam em 4-5 rodadas nem sem Shield. A exclusividade pré-declarada, sozinha,
  provavelmente não desce G3 abaixo de 97 — o cenário-irmão sem Defender já está em
  ~100%.
- A previsão da spec §8.2 de que o P7b custaria queda ao provocador (~14 de dano
  líquido/rodada → cai em ~3,9 rodadas) **não se materializou**: os inimigos morrem
  durante a luta e o foco decai junto (a própria spec declarava essa reserva). No trash
  medido, provocar é efetivamente grátis em pele — que é exatamente o que o empate do
  G4 diz pelo outro lado.

**Leitura que amarra as três reprovações (G1, G3 e o empate do G4): são a mesma física.**
O multiplicador 12 manda golpes demais ao Bento (G1 acima do teto), o Bento absorve tudo
sem cair (G4 empata em zero), e a party atrás dele vence sempre (G3 acima do teto).
Baixar o multiplicador é a única alavanca pré-declarada que mexe nas três na direção
certa ao mesmo tempo — menos golpes no tanque, mais golpes nos outros, vitória menos
automática. Se ela basta, só a próxima rodada diz. **Esta análise não escolhe valor nem
combinação de correções; as opções vão ao líder na seção final.**

## 7. O que passou — e responde a pergunta que a onda inteira fez

1. **G5 = 0,1pp nos 4 candidatos (teto 5pp).** A Fase C mediu 5,85-6,77pp de
   sensibilidade ao braço de mira e ISSO derrubou todo mundo; o pré-registro §9.3 diz:
   *"G5 é o gate que responde a pergunta que a Fase C fez"*. Respondida: **com uma
   alavanca explícita no jogo, o resultado deixa de depender da esperteza da IA.** O
   escudo do tanque deixou de ser probabilístico.
2. **A tartaruga morreu de morte estrutural.** H3: p99 de 10-13 rodadas (Fase C: lutas
   no cap de 30). H4: **k = 0 em 300.000 lutas, nos 8 pontos P6** — e com a tripla nova,
   até k = 20 aprovaria: o gate não passou por sorte de cauda, passou com UB95 uma ordem
   de grandeza abaixo do teto (9,986×10⁻⁶ contra 10⁻⁴). H5: máximo da grade inteira de
   16,8 milhões de lutas = 16 rodadas. A dor de origem (1 luta em 240.000 reprovando
   candidato) não se repetiria nem por azar — e não precisou: não houve luta capada.
3. **O enrage é invisível para a luta saudável, provado da forma mais forte possível:**
   H1 = 0,0% (nenhuma luta P1/P2 alcança a rodada 8) e H2 com deltas exatamente 0,0
   contra controles pareados por seed no mesmo run — identidade bit a bit, não
   "diferença pequena".
4. **G6 verde nos 4:** o P7 continua na janela 3-5 (mediana 4-5, ≥99,9% das lutas na
   janela). A alavanca nova não alongou nem encurtou o pacing para fora da régua.
5. **O critério novo do cap funcionou como método:** k impresso mesmo em zero, UB95
   visível, veredito por tripla pré-registrada. A troca "0% estrito → teto de taxa" fez
   exatamente o que a seção 8 da análise da Fase C prometeu, sem absolver o que merecia
   reprovar (o teste do conserto interesseiro do §9.2 nem chegou a ser exercitado:
   nenhum candidato dependeu da folga).

## 8. Os informativos, sempre impressos, nunca escondidos

### 8.1. G2 (informativo por decisão do líder)

P2 no braço vizinho: 86,9% de vitória / 13,1% de quedas do Gus (33/12 e 34/12), 86,1% /
13,9% (33/13), 85,8% / 14,2% (39/11) — contra piso 90% e teto 12%. Levemente melhor que
a Fase C (86,71/13,29 etc.; ~0,2pp, direção consistente com o Bento Def 13 absorvendo um
pouco mais que a Jaci-proxy Def 10), mas a mesma reprovação. **A pendência de design da
defesa-sem-alavanca sob mira esperta continua real e registrada.** O contraste é o
argumento de produto: no MESMO braço vizinho, o P7 (com alavanca) dá 99,9% / 0,1%. A
resposta do jogo à mira esperta existe — chama-se Provocar — mas o G2 mede o jogador que
não a usa. Se o P2-sob-vizinho deve aprovar POR SI, é decisão de design futura (as
opções da Fase C §6 seguem na mesa); não é este gate nesta rodada.

### 8.2. G4 (informativo pelo plano B pré-declarado)

Empate 0%×0% exato, nos 2 braços, nos 4 candidatos (seção 4). O que o pré-registro queria
comprar com G4 — "provocar não pode ser grátis" — não está comprado, e o P7b (0% de
quedas do provocador nos 8 pontos) mostra que não é artefato do Shield: é o trash de 4
rodadas que não tem tempo de derrubar um tanque de Def 13/HP 55 nem com 85% dos golpes
nele. O preço em pele do Provocar, no trash, hoje é zero medido. Três leituras possíveis,
todas do líder, nenhuma desta análise: (a) aceitar que o preço do Provocar em trash é o
custo de AP/turno, não pele, e deixar G4 informativo em definitivo para trash (re-decidir
para elite/boss, onde as lutas são longas); (b) baixar o multiplicador (correção do G1) e
re-medir se o empate quebra; (c) mexer em `kProvokeDefFactor` — mas o QA-SIM já
argumentou por que isso não resolve com Shield pool = Def, e o pré-registro só o indica
para reprovação NUMÉRICA, que não é o caso.

### 8.3. P7b e a métrica de foco

O longest focus run do P7b confirma a mecânica: mediana de 4 rodadas seguidas no mesmo
alvo, e em 99,4% das lutas a maior sequência foi no provocador. A alavanca redireciona
como desenhado; o que está em discussão é o quanto (G1) e a que preço (G4).

## 9. Achados de método

1. **A previsão analítica pré-run subestimou o efeito dinâmico do F1 (seção 5).** Onde a
   spec §8.1 previa folga de 0,7pp DENTRO da faixa, o dado ficou 0,1-0,7pp FORA, por
   cima. Lição para a próxima reconferência analítica: fórmula estática de mira com
   termo de dano-recebido precisa de pelo menos o caso "provocador ferido, aliados sãos"
   na tabela — que é o estado estacionário que o próprio taunt produz.
2. **A prévia do QA-SIM (n=30k/60k, com a Jaci-proxy) acertou a direção de todos os
   gates, mas errou o lado da fronteira em G1** ("G1/G3 passam perto da fronteira" →
   oficial: G1 reprovou por 0,1-0,7pp). Parte é proxy → Bento real (o Bento se fere mais
   que a Jaci e realimenta o F1 mais rápido), parte é N. Prévia serve para dimensionar,
   não para antecipar veredito de gate apertado — mais um ponto para a disciplina de só
   ler veredito no run oficial.
3. **Reprovação por margem fina em gate de banda:** o 39/11 reprovou G1 por 0,1pp na
   precisão impressa. O harness imprime a fatia com UMA casa decimal; para uma régua com
   borda em 85,0%, a próxima iteração do harness deveria imprimir duas casas e a
   meia-largura do IC da fatia (como já faz com E1/E4), para que "FORA por 0,1pp" seja
   distinguível de "FORA por 0,14pp" sem reabrir o artefato cru. Nota de ferramenta, não
   de critério — não muda nenhum veredito desta rodada.
4. **H5 global = 16 rodadas > teto analítico de 15 da spec §3.4.** Não é violação: a
   prova analítica cobria a party de referência contra o statline de referência, e o
   ponto de 16 rodadas é o P6 do 39/11 (esponja, 118% do HP de referência), que a prova
   nunca cobriu — o §8.3 da spec já tinha registrado hipótese análoga (Bento em
   tartaruga → ~18). O gate (20) segurou com folga 4. Vigiar a premissa se a grade
   mudar, como a spec pede.
5. **O empate bit a bit 33/12 × 34/12 na quarta medição consecutiva** — agora incluindo
   cenários que não existiam nas medições anteriores — é o controle de regressão mais
   barato do estudo: qualquer divergência futura entre os dois num cenário compartilhado
   é sinal de bug de harness, não de física nova.
6. **Disciplina de congelamento cumprida:** md5 conferido nas duas pontas por leitura
   independente, sem regeneração no meio (as duas leituras batem com o hash congelado
   pelo orquestrador na hora do run).

## 10. ⚠️ RESSALVA OBRIGATÓRIA: a mira ponderada ainda não está ligada em produção

Repetida das quatro análises anteriores, porque nada mudou e o cabeçalho do próprio
artefato a reimprime: segundo a análise de projeto, `mira_target_weight`/
`pick_weighted_enemy_target` não teriam call site de produção; `scripted_brain.cpp`
miraria o primeiro da lista. **Os 56 pontos descrevem o jogo COM a mira ponderada
ativa; uma partida jogada hoje não se comporta assim.** `IA-ALVO-PRIMEIRO-DA-LISTA`
segue pendente de decisão do líder e continua bloqueando a canonização de qualquer
número deste estudo em `combat.md`. O agravante da Fase C se dissolveu parcialmente,
segundo a análise: a decisão de design da defesa JÁ foi tomada (Provocar + soft enrage,
descritos como implementados no motor real) — o que falta é ligar a mira que
sobreviveu a essa decisão, na ordem que o protocolo §9.1 já fixa.

> ⚠️ **Não verificado contra código.** As afirmações acima sobre
> `mira_target_weight`/`pick_weighted_enemy_target`, `scripted_brain.cpp` e sobre
> Provocar/soft enrage estarem "implementados no motor real" vêm da análise de
> projeto, não de medição em código executado. O código deste projeto nasce do zero
> (GODS_LAWS.md, L-01), e a fonte que a redação original citava não existe. Revalidar
> quando houver implementação.

## 11. Comparação com a Fase C

| Aspecto | Fase C (2026-08-11) | C-bis (2026-08-12) |
|---|---|---|
| Grade | 48 pontos, N=240.000 (11,5M lutas) | 56 pontos, N=300.000 (16,8M lutas) |
| Sistema | sem alavanca de mira, sem enrage, cap "0% estrito", Jaci-proxy | Provocar + soft enrage no motor REAL, tripla do evento raro, Bento real |
| Sensibilidade à mira (a causa nº 1) | 5,85-6,77pp, reprova os 4 | **0,1pp, G5 PASS nos 4** |
| Tartaruga (a causa nº 2) | cap batido em 3 pontos (até 35 lutas), p10=3 em 4 pontos | **k=0 em 8 de 8 pontos, p99 10-13, máx 16** |
| Critério do cap | 1 luta em 240k reprovava candidato | k=20 ainda aprovaria; k medido = 0 |
| Veredito | 0 de 4 (por desenho + critério) | **0 de 4 (por calibragem: G1+G3, ambos com correção pré-declarada)** |
| Empate 33 × 34 | bit a bit (3ª vez) | bit a bit (4ª vez, incluindo P7/P7b) |
| O que falta | 3 decisões de design + critério novo | 2 correções pré-declaradas + re-rodada |

A C-bis não desmentiu nenhum número da Fase C (o G2 informativo reproduz o P2-vizinho
quase idêntico); ela mediu o que a C não podia medir e mostrou que o conserto sistêmico
funcionou. O que sobrou é sintonia da alavanca nova — um problema menor, com remédio já
escrito e assinado antes do dado.

---

## PARA O LÍDER DECIDIR

**Quantos sobreviveram:** **0 de 4**, tanto pelo critério do harness (G1+G2+G3+G4)
quanto pelo critério corrigido com G2 informativo (sua decisão de 2026-08-11) e G4
informativo (plano B pré-declarado, acionado porque o empate 0%×0% se confirmou exato
nos dois braços e nos quatro candidatos). Com o critério corrigido, as reprovações são
**G1 e G3, idênticas nos 4 candidatos** — nenhum statline foi desqualificado por número
próprio.

**A resposta da pergunta da onda** ("com Provocar + soft enrage + Bento real, os
candidatos sobrevivem à bateria completa?"): **ainda não — mas os dois problemas
sistêmicos que a Fase C expôs estão resolvidos e medidos** (sensibilidade à mira: de
~6pp para 0,1pp, G5 verde; tartaruga: zero lutas no cap em 2,4M lutas P6, p99 ≤ 13,
máximo global 16, H1-H5 todos verdes). O que reprova agora é a calibragem da alavanca
nova, e **as duas reprovações caem em correções que o senhor já pré-declarou e assinou**:

1. **G1 reprovado por cima no braço titular (85,1-85,7% vs teto 85%):** correção
   pré-declarada = baixar `kMiraProvokeMultiplier` (hoje 12,0; faixa útil 6,0-20,0), e
   só ele. Precisa de um valor novo (decisão sua, com o `lead-game-designer`) e de
   re-rodada.
2. **G3 reprovado (100,0%/99,9% vs teto 97%):** correção pré-declarada = Provocar e
   Defender mutuamente exclusivos. **Ressalva medida:** o P7b (provocar sem defender) já
   vence ~100% — a exclusividade sozinha provavelmente não desce G3 abaixo do teto. As
   três reprovações (G1, G3, empate do G4) são a mesma física do multiplicador forte
   demais sobre um tanque que não cai; baixar o multiplicador é a alavanca comum. Cabe
   ao senhor decidir a combinação: (a) aplicar as DUAS correções e re-rodar; (b) só o
   multiplicador, mantendo a combinação de ações, e re-rodar; (c) re-pré-registrar o
   teto de G3 se o senhor decidir que "a parede com alavanca vence sempre" é
   comportamento desejado, não defeito (registro que isso reabriria a régua E1 de
   trivialização — contra-argumento meu, decisão sua).
3. **G4 ficou informativo nesta rodada pelo plano B.** Fica a decisão de para onde ele
   vai em definitivo: informativo permanente no trash (o preço do Provocar é AP, não
   pele) ou re-avaliado junto com a correção do multiplicador. O P7b mostrou que
   provocar hoje não custa pele nem sem Defender.
4. **G2 (informativo) segue reprovando** — a defesa-sem-alavanca sob mira esperta
   continua a pendência de design registrada desde a Fase C. Nada nesta rodada a
   resolve; as opções da análise da Fase C §6 continuam na mesa quando o senhor quiser.
5. **Pendências de processo:** (a) confirmar retroativamente a reconferência autônoma do
   Caetano (spec §8, commit `f17acd4c` — ela CONFIRMOU as faixas, não mudou nenhuma);
   (b) `IA-ALVO-PRIMEIRO-DA-LISTA` segue bloqueando canonização de qualquer número em
   `combat.md`; (c) se houver nova rodada (C-ter), o pré-registro dela se fixa ANTES do
   dado, como sempre — incluindo a decisão sobre G3/G4 acima.

**O que esta análise NÃO faz:** não escolhe multiplicador, não decide exclusividade de
ações, não canoniza número, não dispara rodada nova. O incumbente 33/12 continua sem
necessidade de mudança de emergência (o jogo de hoje nem usa a mira ponderada), e o
empate 33/34 segue perfeito pela quarta medição. Toda decisão acima chega ao senhor pelo
orquestrador via AskUserQuestion.

---

*Análise escrita por Capitolino (CPO) em 2026-08-12 sobre o artefato congelado da Fase
C-bis (md5 conferido nas duas pontas). Camada 1 = gates como impressos pelo harness;
Camada 2 = critério aplicado com G2 informativo (decisão do líder 2026-08-11) e G4
informativo (plano B pré-declarado do protocolo §9.3, condição de empate 0%×0%
verificada no dado oficial). Não é canon. Commitada localmente por ordem desta fatia;
sem push.*
