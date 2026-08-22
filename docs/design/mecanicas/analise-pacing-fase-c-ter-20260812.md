# ANÁLISE: Fase C-ter do estudo de pacing (re-rodada com `kMiraProvokeMultiplier` = 10,5)

> **Status: ANÁLISE (rascunho pronto para revisão), não canon.** Nenhum número deste
> documento entra em `combat.md` nem no motor sem decisão explícita do líder. Autor:
> Capitolino (CPO), 2026-08-12. Dados: artefato congelado
> `.../scratchpad/cter_oficial/pacing_sim_phase_c_ter_report.txt`
> (md5 `9a97a48f3124713714cc765ef77d35a8`, 584.660 bytes; estabilidade provada por
> DUAS âncoras: o congelamento do orquestrador na hora do run, mtime 2026-08-12
> 06:41:11 -0300, e a medição independente de 08:04:29, posterior a TODA leitura que
> esta análise fez do artefato, idêntica ao congelado. ⚠️ O arquivo foi APAGADO do
> scratchpad às ~08:09, DEPOIS do fechamento da janela de leitura; ver seção 2 para o
> que isso afeta e o que não afeta). Run oficial:
> 56 pontos (4 candidatos × 2 braços de mira × 7 slots de cenário), N = 300.000 por
> ponto = **16,8 milhões de lutas**, seed base **20260812** (NOVA, decisão do líder:
> não confirmar in-sample o dado da 20260801 que calibrou o multiplicador; ver a
> ressalva de método na seção 9.1), disparo por `GUSWORLD_PACING_SIM_PHASE_C_TER_FULL`.
> **Zero erros internos declarados em 56 de 56 pontos.** Única mudança de física em
> relação à C-bis: `kMiraProvokeMultiplier` 12,0 → **10,5** (decisão do líder
> 2026-08-12, item `PROV-MULT-AJUSTE`; proposta e previsão pré-registrada do
> `lead-game-designer` em `spec-provocar-soft-enrage-criterio-cap.md` §9, escrita ANTES
> do dado). Docs-irmãos: `analise-pacing-fase-c-bis-20260812.md` (linha de base),
> `analise-pacing-fase-c-20260811.md`, `analise-pacing-fase-b-20260811.md`.

---

## Resumo executivo (em linguagem simples)

1. **O G1 está CONSERTADO: a correção do multiplicador funcionou, nos 8 pontos.** A
   fatia de golpes no provocador saiu da zona proibida (85,1-85,7% na C-bis, teto 85%)
   para **83,3-83,9% no braço titular e 71,1-75,6% no vizinho**, tudo dentro da banda
   70-85, com folga mínima de 1,1pp nas duas bordas. O G1 passou nos 4 candidatos,
   inclusive na leitura crua do harness.
2. **A previsão pré-registrada do `lead-game-designer` BATEU, com erro máximo de
   0,3pp.** O §9 da spec previu, antes do dado, cada um dos 8 pontos do G1 (erro de
   0,0 a 0,3pp, todos na direção do viés que o próprio §9.1 declarou), previu o P7b em
   ~85,3% (medido: 85,3%, exato na precisão impressa) e previu que **G3 NÃO desceria
   com nenhum multiplicador da faixa útil**. G3 mediu 100,0%/99,8% contra teto 97%:
   reprovou de novo, como previsto. A previsão falseável não foi falseada. Seção 5.
3. **Veredito com o critério corrigido: 0 de 4, e agora por UMA reprovação só, o G3.**
   Com G2 informativo (decisão do líder 2026-08-11) e G4 informativo permanente no
   trash (decisão do líder 2026-08-12), a bateria decisiva é G1+G3+G5+G6+H1-H5, e o
   único vermelho, idêntico nos 4 candidatos, é o G3. Isso **fecha o ciclo da abordagem
   "só o multiplicador"**: a mesma causa resolveu G1 e não resolveu G3, exatamente o
   que o líder mandou testar numa mudança única. A decisão sobre o destino do G3, que o
   líder deliberadamente adiou para depois do dado, agora tem o dado. Seção final.
4. **Por que G3 não cai, dito sem estatística:** a parede com alavanca vence sempre
   porque o tanque que atrai os golpes não morre em luta de 4 rodadas, e os golpes que
   sobram para o Gus são poucos demais para derrubá-lo. Mexer no multiplicador muda
   ONDE os golpes caem, não o fato de que caem em quem aguenta. O P7b confirma pelo
   outro lado: sem Defender a party sofre o dobro do dano (28,7% do pool contra 13,5%)
   e ainda assim vence 99,98% das lutas. Seção 6.
5. **Achado investigado (o "G2 mudou?" do escopo desta análise): o G2 NÃO mudou, e a
   explicação é estrutural, não ruído.** Na precisão impressa o G2 é idêntico ao da
   C-bis (86,9/13,1 etc.); em precisão alta mudou 0,0014pp (~4 lutas em 300 mil). A
   causa: o harness deriva a seed de cada luta como `base_seed + i`, então as seeds
   20260801 e 20260812 compartilham **299.989 das 300.000 lutas por ponto (99,996%)**.
   Nada além do multiplicador se moveu (bom), mas a "seed nova" foi atendida só
   nominalmente: a amostra do C-ter é quase a mesma da C-bis. Com folga de 1,1pp e
   ruído de centésimos, nenhuma conclusão muda; a nota de método fica registrada e a
   correção proposta (hash na derivação da seed) vai à seção 9.1.
6. **Validade interna forte:** empate bit a bit 33/12 × 34/12 pela QUINTA medição
   consecutiva (mesmas fatias, mesmas contagens de golpes: 2.099.972 no P7 titular dos
   dois); H1 = 0,0%, H2 com deltas exatamente 0,0 (controles pareados por seed
   bit-idênticos); k = 0 lutas capadas em 2,4M lutas P6; máximo global da grade = 16
   rodadas (mesmo ponto e mesmo valor da C-bis); zero erros internos declarados 56/56.
7. **⚠️ RESSALVA OBRIGATÓRIA (a mesma das cinco análises anteriores):** segundo a
   análise de projeto, a mira ponderada seguiria **sem call site de produção**
   (`scripted_brain.cpp` miraria `players.front()`). Estes 56 pontos descrevem o
   jogo COM a mira planejada ativa. Nenhum número daqui vira canon antes de
   `IA-ALVO-PRIMEIRO-DA-LISTA` ser decidido. Seção 10.

   > ⚠️ **Não verificado contra código.** Esta afirmação sobre `scripted_brain.cpp` e
   > sobre o call site de produção vem da análise de projeto, não de medição em código
   > executado. O código deste projeto nasce do zero (GODS_LAWS.md, L-01), e a fonte
   > que a redação original citava não existe. Revalidar quando houver implementação.

---

## 1. O que a C-ter é

Re-rodada da C-bis com **uma única mudança de física**: `kMiraProvokeMultiplier`
12,0 → 10,5 (`combat_constants.hpp`, espelho `kProvokeWeightMultiplier` do harness
sincronizado, paridade travada por `static_assert` + `REQUIRE`; item
`PROV-MULT-AJUSTE`). Tudo o mais é idêntico à C-bis: mesmos 4 candidatos (33/12
controle, 34/12 gêmeo, 33/13 quente, 39/11 molde-esponja), mesmos 2 braços de mira
(`F_sem_F4` titular, `C_F4_suave` vizinho harness-only), mesmos 7 slots de cenário
(P1/P2 com e sem enrage, P6, P7, P7b), mesmos gates G1-G6/H1-H5 fail-closed, mesmo
N = 300.000. Diferenças de procedimento: seed base **20260812** (nova, pela disciplina
de pré-registro; ver seção 9.1 para o que "nova" comprou de fato) e gate de disparo
`GUSWORLD_PACING_SIM_PHASE_C_TER_FULL`.

O que esta rodada foi desenhada para responder, nas palavras da decisão do líder
(2026-08-12): baixar SÓ o multiplicador testa, numa mudança única, se a mesma causa
física resolve G1 e G3 juntos. A previsão pré-registrada do `lead-game-designer`
(spec §9.5, falseável, assinada antes do run): resolve G1, NÃO resolve G3.

Nota de leitura do artefato: o cabeçalho e os rótulos internos reimprimem "Fase
C-bis" porque o C-ter reusa `run_phase_c_bis` parametrizada por `base_seed` (decisão
da fatia de implementação: nenhuma lógica de grade duplicada). A identidade do run é
dada pela linha `seed base: 20260812` e pelo gate de disparo. O artefato NÃO imprime o
valor do multiplicador vigente; ver a nota de ferramenta na seção 9.4.

## 2. Checagem de validade interna

- **Hash nas duas pontas, e a perda POSTERIOR do artefato, na ordem exata dos
  fatos:** (1) o orquestrador congelou o artefato na hora do run: md5
  `9a97a48f3124713714cc765ef77d35a8`, 584.660 bytes, mtime 06:41:11 de 2026-08-12
  (âncora inicial); (2) todas as leituras que esta análise fez do artefato ocorreram
  antes de 08:04:29; (3) às 08:04:29, medição independente devolveu md5 e tamanho
  IDÊNTICOS ao congelado (âncora final, posterior à última leitura): **a janela de
  leitura inteira está coberta, o artefato não mudou durante a análise**; (4) às
  ~08:09, o scratchpad da sessão do run foi esvaziado e o artefato foi apagado
  (constatado 08:12:31 na verificação pré-commit; suspeito óbvio: limpeza automática
  de sessão, scratchpads são efêmeros por desenho). **O que a perda afeta:** a
  re-auditabilidade direta do disco; ninguém mais confere este artefato por leitura.
  **O que ela NÃO afeta:** a validade desta análise (provada pelas âncoras 1 e 3) e a
  reprodutibilidade: o run é determinístico por seed, então re-executar com
  `GUSWORLD_PACING_SIM_PHASE_C_TER_FULL` e seed 20260812 sobre o mesmo código deve
  regenerar o artefato, e o md5 aqui registrado é o alvo da conferência. Lição de
  processo, registrada: artefato congelado de run oficial deve ser copiado para
  armazenamento durável (fora de scratchpad de sessão) ANTES da análise, não depois.
  Registro de transparência adicional: a primeira citação do hash nesta sessão de
  análise foi feita antes de a medição independente inicial retornar; a análise só
  prosseguiu ao veredito com medição real em mãos, e todas as medições batem.
- **Zero erros internos declarados em 56 de 56 pontos** (a linha
  `erros internos: 0 de 300000` aparece exatamente 56 vezes; nenhuma linha não-zero
  existe). Zero declarado, não presumido.
- **Quinta reprodução do empate de degrau 33/12 × 34/12.** Todas as células
  compartilhadas dos dois são idênticas número a número: G1 83,4%/75,1% nos dois
  (inclusive a CONTAGEM de golpes, 2.099.972 no P7 titular e 2.099.739 no vizinho,
  idênticas), P2 vizinho 86,8923%/13,1077% nos dois, P7 titular vitória 99,9767% nos
  dois, H3 p99 11/11 nos dois. Qualquer divergência futura entre os gêmeos num cenário
  compartilhado segue sendo sinal de bug de harness, não de física nova.
- **Controles pareados por seed provados bit a bit:** os pontos P1/P2 com enrage
  DESLIGADO reproduzem os LIGADOS exatamente (H2 com deltas 0,0 exatos), coerente com
  H1 = 0,0%: nenhuma luta saudável alcança a rodada 8, o enrage é inerte nelas.
- **A party impressa em todo ponto** (linha PARTY): parede/provocação com
  `bento HP55/Atk10/Def13/SPD5`, P1/P6 com a Jaci, como o desenho de
  `BENTO-STATLINE-COMBATE` declara.
- **O multiplicador vigente foi descrito pela análise de projeto como tendo o valor**
  `kMiraProvokeMultiplier = 10.5` em `combat_constants.hpp` e
  `kProvokeWeightMultiplier = 10.5` no espelho `mira_sim_harness.hpp`, com o
  comentário da correção `PROV-MULT-AJUSTE`. O próprio dado é consistente com isso por
  dentro: a fatia do G1 caiu ~2pp em relação à C-bis, exatamente o deslocamento que a
  inversão do §9 previa para 12,0 → 10,5 (seção 5).

  > ⚠️ **Não verificado contra código.** A citação de `combat_constants.hpp` e
  > `mira_sim_harness.hpp` com o valor `10.5` vem da análise de projeto, não de
  > medição em código executado. O código deste projeto nasce do zero (GODS_LAWS.md,
  > L-01), e a fonte que a redação original citava não existe. A consistência interna
  > do dado (queda de ~2pp) continua válida como leitura da simulação; revalidar o
  > valor do multiplicador quando houver implementação.

## 3. Camada 1: os gates como o harness os imprimiu (G2 e G4 ainda decisivos)

Vereditos por candidato, transcritos do bloco `GATES DO PRE-REGISTRO` do artefato
(linhas 7485-7626). O harness ainda trata os 11 gates como decisivos; a leitura
corrigida é a seção 4.

| Gate | Critério | 33/12 controle | 34/12 gêmeo | 33/13 quente | 39/11 molde |
|---|---|---|---|---|---|
| **G1** fatia de golpes no provocador (P7, 2 braços) | 70-85% | **PASS**: tit 83,4, viz 75,1 | **PASS**: tit 83,4, viz 75,1 | **PASS**: tit 83,9, viz 75,6 | **PASS**: tit 83,3, viz 71,1 |
| **G2** vitória/quedas Gus em P2 e P7, braço vizinho | ≥90% e ≤12% | **FAIL**: P2 **86,9/13,1**; P7 99,8/0,2 ok | **FAIL**: P2 **86,9/13,1** | **FAIL**: P2 **86,1/13,9** | **FAIL**: P2 **85,8/14,2** |
| **G3** vitória no P7 (provoca+defende) | ≤97% | **FAIL**: tit **100,0**, viz **99,8** | **FAIL**: 100,0/99,8 | **FAIL**: 100,0/99,8 | **FAIL**: 100,0/99,8 |
| **G4** quedas do provocador: P7 > P2 estrito | P7 > P2 | **FAIL**: 0,0 vs 0,0 nos 2 braços | **FAIL**: 0,0 vs 0,0 | **FAIL**: 0,0 vs 0,0 | **FAIL**: 0,0 vs 0,0 |
| **G5** diferença de vitória no P7 entre braços | ≤5pp | **PASS**: **0,2pp** | **PASS**: 0,2pp | **PASS**: 0,2pp | **PASS**: 0,2pp |
| **G6** duração no P7 (régua E2 herdada, AMB-02) | janela 3-5 | **PASS**: mediana 4, janela 100/99,9% | **PASS**: idem | **PASS**: idem | **PASS**: mediana 5, janela 100% |
| **H1** % de lutas P1/P2 que alcançam R0=8 | ≤1,0% | **PASS**: 0,0% nos 4 pontos | **PASS** | **PASS** | **PASS** |
| **H2** delta com × sem enrage em P1/P2 | ≤5pp | **PASS**: todos os deltas 0,0 | **PASS** | **PASS** | **PASS** |
| **H3** p99 de duração no P6 | ≤20 rodadas | **PASS**: tit 11, viz 11 | **PASS**: 11/11 | **PASS**: 11/10 | **PASS**: 13/12 |
| **H4** cap de 30 no P6 (tripla do evento raro) | UB95/N ≤ 10⁻⁴ | **PASS**: k=0, UB95=2,996 → 9,986×10⁻⁶, nos 2 braços | **PASS**: idem | **PASS**: idem | **PASS**: idem |
| **Veredito do harness** | todos verdes | **REPROVOU** | **REPROVOU** | **REPROVOU** | **REPROVOU** |

**H5 (global da grade): PASS.** Máximo observado na grade INTEIRA = **16 rodadas**
(ponto: P6 do 39/11, braço titular, enrage ligado), contra teto estrito de 20. O mesmo
ponto e o mesmo valor da C-bis. Linha final do harness: *"0 de 4 candidatos aprovaram
em TODOS os gates G1-G6/H1-H4, com H5 (global) verde"*.

Mudança contra a C-bis já visível na camada crua: o harness reprovava por
**G1+G2+G3+G4**; agora reprova por **G2+G3+G4**. O G1 passou até na régua fail-closed
do harness. Nota de precisão: o gate imprime vitória com UMA casa decimal, então o
"100,0%" do G3 titular é arredondamento; o valor cru do ponto é 99,9767% (derrota
0,0233%). Nenhuma luta terminou "sem derrota possível"; a régua não é enganada por
isso (o teto é 97), mas o registro fica para leitura honesta.

## 4. Camada 2: o critério aplicado, com G2 e G4 informativos

**Por que G2 não decide:** decisão do líder de 2026-08-11 (item `PACING-FASE-C-BIS`):
G2 mede o cenário parede SEM a alavanca, que nenhuma ferramenta desta onda toca. Segue
impresso e registrado como pendência real do design da defesa (seção 8.1).

**Por que G4 não decide:** decisão do líder de 2026-08-12, **definitiva** (não mais o
plano B condicional da C-bis): G4 é **informativo PERMANENTE no trash**. O preço de
Provocar no trash é o AP gasto, não a pele do provocador; desenho aceito, não
pendência. O empate 0%×0% se reproduziu exato nesta rodada (quedas do Bento = 0% nos
16 pontos relevantes de P2/P7, dois braços, quatro candidatos, e também nos 8 de P7b),
coerente com a previsão do §9.6 de que com M menor o Bento levaria ainda menos golpes.

**Gates decisivos: G1, G3, G5, G6, H1, H2, H3, H4 (por candidato) + H5 (global).**

| Candidato | G1 | G3 | G5 | G6 | H1 | H2 | H3 | H4 | H5 global | **Veredito corrigido** |
|---|---|---|---|---|---|---|---|---|---|---|
| 33/12 controle | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | **REPROVADO (G3)** |
| 34/12 gêmeo | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | **REPROVADO (G3)** |
| 33/13 quente | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | **REPROVADO (G3)** |
| 39/11 molde | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | **REPROVADO (G3)** |

**Sobreviventes à bateria completa com o critério corrigido: 0 de 4.** Mas a natureza
do 0/4 mudou de novo, e na direção certa: na Fase C eram duas causas sistêmicas de
desenho; na C-bis, duas de calibragem (G1+G3); agora é **uma só (G3)**, idêntica nos 4
candidatos, e ela é exatamente a que o pré-registro previu que nenhum multiplicador
resolveria. Nenhum candidato foi desqualificado por número próprio em nenhuma das três
rodadas de confirmação. Se o líder decidir que G3 não decide no trash-com-alavanca
(opção na seção final), **os 4 candidatos aprovariam a bateria inteira com este mesmo
dado**, sem rodada nova.

## 5. Previsão vs Medido: o teste de calibração do método do §9

Esta é a seção que o run realmente pagou. O §9 da spec fixou, ANTES do dado, previsões
numéricas por inversão do dado medido da C-bis (`r = fatia/(1-fatia)/M`), com viés
declarado ("r constante superestima levemente a fatia em M menor; ≤0,2pp").

### 5.1 G1, os 8 pontos da grade

| Ponto | Previsto (§9.4.1) | Medido (C-ter) | Erro | Dentro da banda? |
|---|---|---|---|---|
| 33/12 titular | 83,6% | **83,4%** | -0,2pp | sim (folga 1,6 p/ teto) |
| 34/12 titular | 83,6% | **83,4%** | -0,2pp | sim |
| 33/13 titular | 84,0% | **83,9%** | -0,1pp | sim (folga 1,1, a menor do teto) |
| 39/11 titular | 83,3% | **83,3%** | 0,0pp | sim |
| 33/12 vizinho | 75,4% | **75,1%** | -0,3pp | sim |
| 34/12 vizinho | 75,4% | **75,1%** | -0,3pp | sim |
| 33/13 vizinho | 75,8% | **75,6%** | -0,2pp | sim |
| 39/11 vizinho | 71,4% | **71,1%** | -0,3pp | sim (folga 1,1 p/ piso, a menor do piso) |

- **Erro máximo 0,3pp, médio ~0,2pp, e TODOS os desvios na direção prevista** (medido
  abaixo ou igual ao estimado, que é o viés do `r` constante que o §9.1 declarou com a
  direção e a escala). A magnitude real do viés no pior ponto (0,3pp) passou de leve a
  estimativa "≤0,2pp", sem consequência: a assimetria de folga que o §9.3 comprou de
  propósito (mais folga no piso, o lado com viés contra) absorveu exatamente isso.
- **A previsão de veredito bateu: G1 passa nos 8 pontos.** Folga real mínima: 1,1pp no
  teto (33/13 titular, 83,9 vs 85) e 1,1pp no piso (39/11 vizinho, 71,1 vs 70). O §9
  previa 1,0 e 1,4; o viés comeu piso e devolveu teto, como declarado.
- **Contra a C-bis:** a fatia caiu 1,8-1,9pp no titular e 2,6-2,9pp no vizinho. O §9.6
  estimou "tira ~2pp da fatia". Confirmado.

### 5.2 P7b (informativo): o acerto mais fino da rodada

O §9.4 registrou, "para ninguém se surpreender no relatório do C-ter": P7b em M=10,5
ficaria em ~**85,3%** pela inversão. Medido: **85,3%** (33/12, precisão impressa; 85,6
no 33/13, 85,4 no 39/11). O estimador cravou o cenário que ele nem estava sendo
escolhido para otimizar.

### 5.3 G3: a previsão central, e ela se confirmou

O §9.5 pré-registrou como **previsão falseável**: "G3 NÃO desce com nenhum M da faixa
útil; G3 ≈ 99,9% em M=10,5; se medir G3 ≤ 97%, o modelo desta seção está errado".

- **Medido: 100,0% titular (cru 99,9767%) e 99,8% vizinho (cru 99,8093%), nos 4
  candidatos. FORA do teto de 97. A previsão se confirmou.**
- No detalhe que o modelo Poisson conseguia alcançar: ele previa quedas do Gus ≈0,06%
  no P7 em M=10,5, calibrado num único ponto e sem distinguir braços. Medido: 0,023%
  no titular e 0,19% no vizinho. A previsão pontual fica entre os dois braços e acerta
  a ordem de grandeza nos dois; para descer G3 a 97% seriam necessárias quedas ~3%,
  fator de 15-130× acima do medido. A conclusão analítica (G1 e G3, como
  pré-registrados, são incompatíveis no P7 de trash com o Bento real) sai do run
  **medida**, não mais só modelada.

### 5.4 O que isso diz do MÉTODO (a resposta à pergunta desta rodada)

O §8 da spec (tabela estática não calibrada) errou o G1 em ~1,5pp e errou o LADO da
fronteira: previu folga e houve reprovação. O §9 (inversão do dado dinâmico, viés
declarado, restrição de dois lados) errou no máximo 0,3pp em 8 pontos, acertou a
direção de todos os desvios, acertou os dois vereditos (G1 passa, G3 reprova) e cravou
o informativo P7b. **O estimador por inversão está validado como método de calibração
desta alavanca**: para a próxima sintonia de parâmetro de mira, a rota é a do §9.1
(inverter o dado medido, declarar o viés e a direção, exigir folga maior que o erro
residual), não a tabela estática. O run também mostra o limite honesto do método: o
viés declarado como "≤0,2pp" mediu 0,3pp no pior ponto; quem for reusar o estimador
deve orçar a folga contra ~0,3pp, não contra 0,2pp.

## 6. Anatomia da única reprovação decisiva: G3, agora com o mecanismo medido duas vezes

G3 mede a vitória no P7 (provocador também defende): teto 97%, medido 100,0%/99,8% nos
4 candidatos (quedas do Gus 0,023-0,19%). O dado novo desta rodada fecha o diagnóstico
que a C-bis só podia indiciar:

- **Não é o multiplicador.** M caiu de 12,0 para 10,5, a fatia do provocador caiu ~2pp,
  a fatia do Gus subiu na proporção prevista, e a vitória se moveu 0,0-0,1pp na
  precisão impressa (C-bis: 100,0/99,9; C-ter: 100,0/99,8). A alavanca inteira da faixa
  útil não alcança o teto: era a previsão, agora é medição.
- **Não é a combinação com Defender.** P7b (provoca SEM defender): vitória 99,98%
  titular, quedas do Gus 0,015%, quedas do provocador 0%, com a party sofrendo
  **28,7% do pool de HP** contra 13,5% no P7. Tirar o Defender dobra o dano sofrido e
  não move a vitória. A exclusividade Provocar+Defender, sozinha, não compraria o teto
  de 97 (o cenário-irmão sem Defender já vence ~100%).
- **É a física do cenário:** trash de 4 rodadas contra um tanque Def 13/HP 55 que não
  cai (G4 empatado em 0% pela segunda rodada consecutiva) com um teto de vitória (97%)
  escrito quando a parede não tinha alavanca. Os três fatos são o mesmo fato.

Consequência lógica, que não é desta análise decidir: ou o teto do G3 muda (ou deixa
de decidir no trash-com-alavanca), ou a física do cenário muda (design novo fora do
pré-registro atual: taunt com duração menor, trash que derruba tanque, teto de rodadas
de Provocado). As opções estão na seção final, com os contra-argumentos devidos.

## 7. O que passou, e o que isso confirma da onda inteira

1. **G1 verde nos 8 pontos: a alavanca agora está calibrada.** 83,3-83,9% titular
   (1 em 6 golpes ainda escapa do taunt) e 71,1-75,6% vizinho. A janela de taunt
   coincide com a fatia global em todos os pontos (o provocador nunca caiu no meio da
   rodada), e o longest focus run mostra a mecânica legível: mediana de 3 rodadas
   seguidas no provocador no titular, 97,9% das lutas com a maior sequência nele.
2. **G5 = 0,2pp (teto 5pp).** A resposta da Fase C continua de pé com o M novo: com
   alavanca explícita, o resultado não depende da esperteza da IA. (C-bis: 0,1pp; a
   diferença de 0,1 para 0,2 é o P7-vizinho vazando um pouco mais de golpes ao Gus com
   M menor, visível também nas quedas 0,1% → 0,2%.)
3. **G6 verde nos 4** (mediana 4-5, janela ≥99,9%): baixar o multiplicador não mexeu na
   duração, como o §9.6 previa (a duração é dirigida pelo DPS da party).
4. **A tartaruga segue morta e o enrage segue invisível:** H1 = 0,0%, H2 = deltas 0,0
   exatos, H3 p99 10-13, H4 k=0 com UB95 uma ordem de grandeza abaixo do teto, H5
   máximo global 16. Nenhum efeito colateral do M novo em nenhum gate H, como previsto.
5. **G4 (informativo permanente) reproduziu o empate 0%×0% exato**, coerente com a
   decisão do líder de que o preço de Provocar no trash é AP, não pele.

## 8. Os informativos, sempre impressos, nunca escondidos

### 8.1. G2 (informativo por decisão do líder)

P2 no braço vizinho: 86,9/13,1 (33/12 e 34/12), 86,1/13,9 (33/13), 85,8/14,2 (39/11),
contra piso 90 e teto 12. **Idêntico à C-bis na precisão impressa**, e a seção 9.1
explica por quê (as duas rodadas compartilham 99,996% das lutas). A pendência de
design da defesa-sem-alavanca sob mira esperta continua real, registrada e intocada
por esta rodada, que não a mediu com amostra nova em nenhum sentido prático. O
contraste de produto segue: no mesmo braço vizinho, o P7 com alavanca dá 99,8/0,2.

### 8.2. G4 (informativo permanente, decisão do líder 2026-08-12)

Empate 0,0%×0,0% exato nos 2 braços, nos 4 candidatos, e 0% também nos 8 pontos P7b.
Não há mais plano B a acionar: o estatuto é permanente para o trash, e a re-discussão
só volta se um dia existir P7 de elite/boss (lutas longas, onde pele pode voltar a ser
preço). Registro mantido porque o pré-registro manda imprimir o que se mede.

### 8.3. P7b, e um fato estrutural que esta rodada expôs

O P7b saiu **bit-idêntico entre os dois braços de mira** (85,3% de exatamente
2.099.978 golpes nos dois, no 33/12). Não é bug: os braços diferem apenas pelo
multiplicador F4, que segundo a análise de projeto só se aplica a alvo com Shield
ativo (`mira_sim_harness.hpp:512`; C = ×0,5, F = ×1,0, referência não verificada, ver
ressalva abaixo), e no P7b ninguém defende, então as duas fórmulas colapsam na mesma.
O mesmo fato explica por que P7 SIM diverge entre braços (o Bento defende lá, e o
braço C desconta a mira sobre ele: 83,4 vs 75,1). Vale registrar porque é o tipo de
identidade que, vista sem a explicação, parece defeito de harness; vista com ela, é
mais um controle interno gratuito.

> ⚠️ **Não verificado contra código.** A citação de `mira_sim_harness.hpp:512` vem da
> análise de projeto, não de medição em código executado. O código deste projeto
> nasce do zero (GODS_LAWS.md, L-01), e a fonte que a redação original citava não
> existe. A leitura do dado (P7b bit-idêntico entre braços) continua válida; revalidar
> o mecanismo quando houver implementação.

## 9. Achados de método

### 9.1. A "seed nova" foi nominal: `base_seed + i` compartilha 99,996% da amostra

O achado que esta análise foi mandada verificar ("o G2 mudou?") tem resposta medida e
causa estrutural. Medido: G2 idêntico à C-bis na precisão impressa; em precisão alta,
13,1077% contra 13,1063% (Δ = 0,0014pp ≈ 4 lutas líquidas em 300 mil). Isso é pequeno
DEMAIS para duas amostras independentes (o ruído esperado entre seeds independentes
seria ~0,09pp), e a causa estaria no harness, segundo a análise de projeto:
`run_point_battles` derivaria a seed de cada luta como `base_seed + i`
(`pacing_sim_harness.hpp:1736`, referência não verificada, ver ressalva na seção
9.4). As bases 20260801 e
20260812 distam 11, então **cada ponto da C-ter re-simulou 299.989 das 300.000 lutas
da C-bis** (as mesmas seeds, deslocadas de índice), trocando só 11 lutas em cada
borda. Consequências, separadas com cuidado:

- **Para a pergunta "algo além do multiplicador se moveu?": não.** Nos cenários cuja
  física não mudou (P1/P2/P6), os números são quase-idênticos exatamente como a
  sobreposição prevê (o delta cabe nas ±22 lutas trocadas). Não há efeito espúrio.
- **Para a disciplina que motivou a seed nova: o objetivo não foi alcançado de fato.**
  A intenção do líder era o veredito não confirmar in-sample o dado que calibrou o
  10,5; com 99,996% de sobreposição, a amostra é praticamente a mesma e o ruído
  amostral dos dois runs é quase totalmente correlacionado. Atenua o problema o fato
  de o ruído em jogo (centésimos de pp) ser ~30× menor que a folga do veredito
  (1,1pp): mesmo uma amostra genuinamente independente não teria como virar nenhum
  gate. As conclusões numéricas desta rodada ficam de pé; a alegação "out-of-sample"
  não deve ser repetida sem esta nota.
- **Correção proposta para o harness (nota de ferramenta, decisão do Caetano):**
  derivar a seed por luta com um hash misturador (ex.: splitmix64 sobre
  `base_seed ⊕ i`) em vez de soma, para que seed base nova signifique amostra nova. O
  pareamento por seed que o H2 exige continua funcionando igual (ele é interno ao run,
  mesma base). Enquanto isso não muda, "seed nova" nos pré-registros deve ser lida
  como "janela deslocada da mesma sequência".

### 9.2. O estimador por inversão é o novo padrão de calibração (e seu erro é ~0,3pp)

Seção 5.4: duas rodadas compararam os dois métodos nos mesmos 8 pontos. Tabela
estática (§8): erro 1,5pp, lado errado da fronteira. Inversão calibrada (§9): erro
0,0-0,3pp, direção declarada de antemão, dois vereditos e um informativo acertados.
Orçar folga futura contra 0,3pp de viés, não 0,2pp.

### 9.3. Gate de banda com borda em número redondo continua imprimindo UMA casa

Reitero a nota da análise da C-bis (achado 3), que segue não atendida e nesta rodada
ficou dos dois lados da mesma moeda: o G1 do 33/13 titular passou por 1,1pp e o G3
imprime "100,0%" para 99,9767%. Duas casas decimais e a meia-largura do IC nas linhas
de gate poupariam reabrir o artefato cru. Não muda veredito nenhum desta rodada.

### 9.4. O artefato não declara o multiplicador vigente

A física mudou entre C-bis e C-ter, e o artefato não imprime `kMiraProvokeMultiplier`
em lugar nenhum. A redação original afirmava que "a verificação foi feita por fora, no
código, e por dentro, pelo deslocamento medido da fatia", citando o cabeçalho como
tendo os parâmetros do enrage lidos de `combat_constants.hpp`. A mesma linha deveria
passar a imprimir o multiplicador do Provocar (e o `kProvokeDefFactor`), pela mesma
razão: se o dado de produção mudar, o artefato deve se autodatar. Nota de ferramenta,
decisão do Caetano.

> ⚠️ **Não verificado contra código.** A afirmação "a verificação foi feita por fora,
> no código" e a citação de `combat_constants.hpp` vêm da análise de projeto, não de
> medição em código executado. O código deste projeto nasce do zero (GODS_LAWS.md,
> L-01), e a fonte que a redação original citava não existe. A consistência interna do
> dado (deslocamento medido da fatia) continua válida como leitura da simulação;
> revalidar o valor do multiplicador quando houver implementação.

### 9.5. O título interno do artefato diz "C-bis"

Reuso deliberado de `run_phase_c_bis` parametrizada (nenhuma grade duplicada, decisão
correta), mas o cabeçalho impresso herda o nome. Quem achar este artefato daqui a
meses deve identificá-lo pela seed 20260812 e pelo gate de disparo. Se houver C-quater,
vale parametrizar também o rótulo impresso.

## 10. ⚠️ RESSALVA OBRIGATÓRIA: a mira ponderada ainda não está ligada em produção

Repetida das cinco análises anteriores, porque nada mudou e o cabeçalho do próprio
artefato a reimprime: segundo a análise de projeto, `mira_target_weight`/
`pick_weighted_enemy_target` não teriam call site de produção; `scripted_brain.cpp`
miraria o primeiro da lista. **Os 56 pontos descrevem o jogo COM a mira ponderada
ativa; uma partida jogada hoje não se comporta assim.** `IA-ALVO-PRIMEIRO-DA-LISTA`
segue pendente de decisão do líder e continua bloqueando a canonização de qualquer
número deste estudo em `combat.md`.

> ⚠️ **Não verificado contra código.** Esta afirmação sobre
> `mira_target_weight`/`pick_weighted_enemy_target` e `scripted_brain.cpp` vem da
> análise de projeto, não de medição em código executado. O código deste projeto
> nasce do zero (GODS_LAWS.md, L-01), e a fonte que a redação original citava não
> existe. Revalidar quando houver implementação.

## 11. Comparação com a C-bis

| Aspecto | C-bis (M=12,0, seed 20260801) | C-ter (M=10,5, seed 20260812) |
|---|---|---|
| G1 titular | 85,1-85,7% **FORA** (teto 85) | **83,3-83,9% ok** (folga ≥1,1pp) |
| G1 vizinho | 74,0-78,2% ok | 71,1-75,6% ok (folga ≥1,1pp) |
| G3 | 100,0/99,9% FORA | **100,0/99,8% FORA (como pré-registrado)** |
| G4 | 0%×0% (plano B acionado) | 0%×0% (informativo permanente, decisão do líder) |
| G5 | 0,1pp | 0,2pp |
| G2 (informativo) | 85,8-86,9 / 13,1-14,2 | idêntico na precisão impressa (seção 9.1) |
| H1-H5 | todos verdes, máx global 16 | todos verdes, máx global 16 (mesmo ponto) |
| Reprovações decisivas | G1+G3 | **só G3** |
| Empate 33 × 34 | bit a bit (4ª vez) | bit a bit (**5ª vez**) |
| Previsão pré-run | §8 errou 1,5pp, lado errado | **§9 errou ≤0,3pp, tudo confirmado** |
| O que falta | escolher M novo + re-rodar | **decisão do líder sobre o G3** |

---

## PARA O LÍDER DECIDIR

**Quantos sobreviveram:** **0 de 4** pelo critério corrigido vigente (G2 informativo,
G4 informativo permanente), com **uma única reprovação, o G3, idêntica nos 4
candidatos**. O harness cru reprova por G2+G3+G4; o G1 passou até nele.

**O ciclo que o senhor desenhou fechou, e com a resposta limpa:** baixar só o
multiplicador **resolveu o G1** (8/8 pontos na banda, folga 1,1pp, previsão do
`lead-game-designer` confirmada com erro ≤0,3pp) e **não resolveu o G3**, exatamente
como o §9.5 pré-registrou. A informação está paga: a causa da trivialização do P7 não
é o tamanho do multiplicador nem a combinação com Defender (o P7b vence 99,98% sem
Defender, sofrendo o dobro do dano); é o cenário parede-com-alavanca contra um teto de
vitória escrito antes de a alavanca existir. O senhor adiou o destino do G3 para
depois do dado; o dado chegou. As opções, com meus contra-argumentos onde devo:

1. **G3 vira informativo no trash-com-alavanca** (mesma família da sua decisão do G4:
   "a parede com alavanca vence mesmo; o preço é AP"). Consequência imediata: com este
   mesmo dado do C-ter, **os 4 candidatos aprovam a bateria completa** sem rodada
   nova, e o estudo de pacing entra na fase de escolha do statline (que sempre foi
   sua). Contra-argumento meu, registrado: isso deixa o cenário-parede-com-alavanca
   oficialmente fora de qualquer régua de trivialização; mitiga o fato de G6 (duração
   3-5) e E9 (hp-final, sem-dano) continuarem medindo o P7, então "vence sempre" não
   vira "vence sem jogar". É a opção coerente com a decisão que o senhor já tomou pro
   G4, e é a que eu recomendo apresentar primeiro.
2. **Re-pré-registrar o teto do G3** (por exemplo, régua sobre quedas do Gus em vez de
   vitória, ou teto de vitória realista para o P7). Contra-argumento meu: qualquer
   teto novo escolhido AGORA seria calibrado sobre o dado que acabou de medir
   99,8-100,0%, ou seja, régua feita para o número passar; só faz sentido se nascer de
   um argumento de design independente do dado, e teria de se fixar antes de qualquer
   rodada futura que dependa dele.
3. **Aplicar a exclusividade Provocar+Defender** (a correção pré-declarada original do
   G3). Contra-argumento meu, agora com medição: o P7b diz que ela quase certamente
   não desce G3 abaixo de 97, então só entra se o senhor a quiser por OUTRA razão
   (custo de decisão tática, identidade do verbo), não como conserto do G3. Custaria
   spec + implementação + rodada nova para, na previsão, mudar o veredito de nada.
4. **Mudar a física do cenário** (taunt com duração/decay, trash que ameaça tanque,
   etc.). É decisão de design nova, fora do pré-registro atual, com custo cheio de
   spec + pré-registro + rodada. Só vale se o senhor considerar que "parede com
   alavanca vence sempre" é defeito de jogo sentível, e o G6 verde (lutas de 4-5
   rodadas, não intermináveis) sugere que não é.

**Decisões menores que também são suas:**

5. **`kMiraProvokeMultiplier` = 10,5 fica?** Recomendo que sim: medido verde nos 8
   pontos com folga equilibrada nas duas bordas. Canonizar o número em `combat.md`
   continua bloqueado por `IA-ALVO-PRIMEIRO-DA-LISTA`, como tudo deste estudo.
6. **Nota de método da seed (seção 9.1):** a "seed nova" compartilhou 99,996% da
   amostra com a C-bis por causa do `base_seed + i`. Nenhuma conclusão desta rodada
   muda, mas se o senhor quiser que "seed nova" signifique amostra nova nos próximos
   pré-registros, autorize o Caetano a trocar a derivação por hash (nota de
   ferramenta, teste de identidade por seed continua possível dentro do mesmo run).
7. **Pendências que seguem de pé:** G2 (design da defesa-sem-alavanca) continua
   pendência registrada intocada; `IA-ALVO-PRIMEIRO-DA-LISTA` continua bloqueando
   canonização; as notas de ferramenta 9.3/9.4/9.5 vão ao Caetano quando o senhor
   autorizar a próxima iteração do harness.

**O que esta análise NÃO faz:** não decide o destino do G3, não canoniza o 10,5, não
promove candidato, não dispara rodada nova. O incumbente 33/12 segue sem necessidade
de mudança de emergência (o jogo de hoje nem usa a mira ponderada), e o empate 33/34
segue perfeito pela quinta medição. Toda decisão acima chega ao senhor pelo
orquestrador via AskUserQuestion.

---

*Análise escrita por Capitolino (CPO) em 2026-08-12 sobre o artefato congelado da Fase
C-ter (md5 conferido nas duas pontas). Camada 1 = gates como impressos pelo harness;
Camada 2 = critério aplicado com G2 informativo (decisão do líder 2026-08-11) e G4
informativo permanente (decisão do líder 2026-08-12). Seção 5 = previsão pré-registrada
do `lead-game-designer` (spec §9) contra o medido, ponto a ponto. Não é canon.
Commitada localmente por ordem desta fatia; sem push.*
