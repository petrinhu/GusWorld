# Reconciliações do Roster de Análogos (LORE-ORIGEM-MULTIVERSO)

> **STATUS: PROPOSTA.** Não é canon. Nenhum doc canônico foi alterado para produzir este documento (auditoria read-only). Mapeia toda ponta solta detectada nos 21 pacotes de `docs/design/roster-analogos/01..21*.md` que toca canon já fechado (`CHARS.md`, `PLACES.md`, `docs/narrative/lore-bible.md`, `docs/narrative/factions.md`, `docs/narrative/deep/*`) e precisa de decisão explícita do criador antes de qualquer canonização. Formato por item: (a) conflito exato com citação, (b) 1-2 resoluções propostas com prós/contras, (c) docs que precisariam de ajuste se aprovada, (d) severidade. Nenhuma prosa canônica é escrita aqui; isso cabe ao `narrative-writer` só depois de decisão.

## Sumário executivo

**9 pontas mapeadas.** 2 de severidade média (tocam integridade institucional/econômica de canon já fechado, mas resolvíveis só com ajuste textual, sem mudar fato nenhum), 1 que bloqueia a canonização de um único pacote (13, não do roster inteiro) até decisão de execução, 1 cosmética já com decisão de sabor pendente registrada no próprio doc-fonte, 3 de baixo risco/opcionais (já sinalizadas como tal pelos próprios autores dos pacotes), 1 puramente administrativa (registro futuro em `factions.md`), 1 nota mecânica sem impacto de lore. Nenhuma ponta exige reescrever fato, data ou nome já fixado no canon central.

| # | Ponta | Severidade | Bloqueia canonização de |
|---|---|---|---|
| 1 | von Neumann ↔ linhagem Neumann/Óxido | Média-alta (decisão de execução) | só o pacote 13 |
| 2 | Núcleo Mandelbrot: origem do nome | Cosmética | nada (já reconhecida como aberta no próprio doc) |
| 3 | Menger ↔ origem do Crédito vs. "crédito gerido por Sterling Corp" (lore-bible §11.4) | Média | nada, se resolvida com 1 frase adicional |
| 4 | Giordano Bruno: fogueira vs. ostracismo (tom do final) | Baixa (decisão de sabor já sinalizada no próprio doc) | nada, doc já assume leitura default |
| 5 | John Dee: espelho ouve ecos do Estilhaçamento (flourish opcional) | Baixa (explicitamente opcional) | nada |
| 6 | Turing: "enclave pré-Ordem Recursiva" precisa de identidade institucional clara | Média | nada, mas gera ambiguidade se não esclarecido |
| 7 | Helion Tusk / Forja do Vértice: novo consórcio sem registro em `factions.md` | Baixa/administrativa | nada agora; ação futura na canonização |
| 8 | "Forja do Vértice" (Era 3) ecoa "Forja Ferraz" (Era 1, `PLACES.md` §5b) | Cosmética | nada |
| 9 | Newton/Einstein: gates de puzzle sobrepostos (meio-jogo ambos) | Informativa, não é lore | nada |

---

## 1. von Neumann ↔ linhagem Neumann/Óxido

### (a) Conflito exato

O doc `13-von-neumann.md` (linhas 22-30) contém, na sua própria voz, uma recomendação que **contradiz uma decisão já registrada do criador**:

> `13-von-neumann.md:25`: "Esta proposta **NÃO liga** John von Neumann a essa linhagem, de propósito, pra não reescrever a ancestralidade fechada de Linda. Trata a coincidência de sobrenome como dois ecos convergentes distintos do multiverso que carregam nomes parecidos sem parentesco algum entre si (...) Recomendação de segurança editorial: manter sempre 'von Neumann' completo (nunca só 'Neumann' sozinho)."

Mas o backlog fonte já registra a decisão oposta, tomada pelo criador:

> `docs/design/brainstorm-backlog.md` (bloco Fase B, cluster computação): "**DECISAO CANON (criador): LIGAR von Neumann a linhagem NEUMANN/OXIDO** (ancestral da familia da Linda Siren = Linda Neumann; heranca tecnica da Oxido traca ao analogo, estilo sangue-dragon Vance). CUIDADO de timeline com a Tamara Neumann canonica (Era 2 criadora do Oxido) - reconciliar via narrative-designer; **atualizar 13-von-neumann.md**."

Ou seja: o pacote 13, como está redigido hoje, ainda não implementa a própria decisão que o criador já tomou. Isso NÃO é uma escolha em aberto no sentido de "qual das duas" — é uma tarefa de execução pendente ("atualizar 13-von-neumann.md" é literal no backlog) que toca canon estabelecido em dois pontos:

- `CHARS.md` (§5, linha ~71): "**Tamara Neumann** — Engenheira-mãe Era 2 (~-110), criadora canônica de Óxido (...) Ancestral direta linhagem Neumann (Brígida, Linda)."
- `docs/narrative/deep/magic/4-linguagens-deep.md` (§5): "Óxido é a linguagem da paranoia produtiva (...) projetada por Tamara Neumann (...) no ano de -110 (...) Tamara é mãe-fundadora canon de Óxido, autoria sem disputa em toda historiografia séria." E mais adiante: "A Ordem Recursiva, casa Vance, casa Berenger, casa Vanderbist, **casa Neumann**, todas as quatro linhagens fundadoras das quatro linguagens vivas (...)."

Isto é: "casa Neumann" não é só uma família qualquer, é uma das **quatro casas fundadoras das quatro linguagens vivas** do jogo (paralela a casa Vance/C-Arcane). Ligar von Neumann como ancestral dessa casa é uma amarração de alto peso estrutural, não um detalhe cosmético. É por isso que merece confirmação explícita de execução antes de canonizar, mesmo a decisão já estando tomada em princípio.

### (b) Resoluções propostas

**Opção A (recomendada; implementa a decisão já tomada pelo criador).** Reescrever `13-von-neumann.md` §2 substituindo o bloco "ALERTA DE COLISÃO DE NOME" por uma amarração explícita, com 3 salvaguardas que preservam tudo que já é canon:
1. **Não-autoria preservada:** von Neumann continua SEM criar nenhuma linguagem (Tamara Neumann segue única e indiscutível autora canônica de Óxido, "autoria sem disputa" no `4-linguagens-deep.md` permanece intocado). Ele é ancestral de sangue/nome, não coautor técnico. Mesmo padrão já usado para Pyotor I Draco Vance ("sangue-dragon Vance", `CHARS.md` §8b): o ancestral empresta legado e nome, o descendente faz o trabalho real.
2. **Posicionamento cronológico:** von Neumann já está proposto "antes ou em paralelo a -150" (antes da Primeira Compilação). Tamara Neumann atua em -110. Gap de ~40 anos comporta 1-2 gerações (ex.: von Neumann → filho ou neto → Tamara), suficiente para "ancestral direta" sem forçar a timeline.
3. **Flourish opcional de sabor:** o von Neumann real nasceu "Neumann János Lajos" (ordem húngara, sobrenome primeiro) e só ganhou a partícula "von" em 1913 por título comprado pelo pai. Isso permite, se o criador gostar, uma imagem elegante: a linhagem GusWorld perde a partícula "von" ao longo das gerações (o mesmo "von" que ele carregava por vaidade nobiliárquica não sobrevive à linhagem operária-técnica que virou "casa Neumann"), ficando só "Neumann" a partir de Tamara. Fecha bem com o tema "ordem espontânea > título concedido" que o roster já carrega (cluster economia austríaca).

Prós: cumpre a decisão do criador já registrada; reforça o padrão "sangue-dragon Vance" já aprovado alhures; dá origem histórica rica à casa fundadora de Óxido sem tocar a autoria de Tamara. Contras: é a amarração de MAIOR peso estrutural do roster inteiro (toca uma das 4 casas fundadoras de linguagem), então merece um re-check explícito do criador nesta rodada, mesmo já "decidida" em princípio, porque a implementação concreta (quantas gerações, o que sobra do "von") ainda não foi escrita em lugar nenhum.

**Opção B (manter o doc como está hoje, não linkar).** Preserva o texto atual do pacote 13 (tratamento como coincidência estrutural do multiverso, sempre grafado "von Neumann" completo). Prós: zero risco, zero cascata, nenhum doc canônico precisa mudar. Contras: contradiz decisão já registrada do criador no backlog; desperdiça a amarração temática mais forte do cluster computação (o autor real de "programa é dado, dado é programa" tornando-se literalmente ancestral da casa que fundou a linguagem "paranoica" do jogo).

### (c) Docs a ajustar se aprovada (Opção A)

- `docs/design/roster-analogos/13-von-neumann.md` §2 (reescrever o bloco "ALERTA DE COLISÃO DE NOME").
- `CHARS.md` §5 (entrada Tamara Neumann): acrescentar cross-ref "ancestral remoto: von Neumann (análogo histórico, roster LORE-ORIGEM-MULTIVERSO)" quando canonizado.
- Nenhuma mudança em `docs/narrative/deep/magic/4-linguagens-deep.md` (a autoria de Tamara permanece intocada; só um adendo de linhagem anterior, se o criador quiser, no mesmo estilo dos ancestrais Era 1 já documentados em `CHARS.md` §8b/§8c).

### (d) Severidade

Bloqueia a canonização do **pacote 13 especificamente** até a reescrita (a decisão em si já foi tomada; falta só a redação consistente). Não bloqueia os outros 20 pacotes do roster.

---

## 2. Núcleo Mandelbrot: origem do nome

### (a) Conflito exato

`08-mandelbrot.md` já se autodeclara "RECONCILIAÇÃO SINALIZADA" na própria nota de escopo (linha 5): o Núcleo Mandelbrot já é lugar canônico fechado (`PLACES.md` linha 94: "Núcleo Mandelbrot Interno | Sub-local (Selve Profunda) | Cross-eras (3 GRE) | Coração Selve Profunda | ✅ canônico (climax ato 3, 3 rotas Bronze/Prata/Ouro)"; `environments/08-selve-profunda.md` linhas 44-48 e 158-162 descrevem a estrutura e as 3 rotas sem nunca explicar a origem do NOME). Busquei em todo `docs/narrative/deep/` por qualquer menção de data ou ato de nomeação do Núcleo em homenagem a alguém: **não existe nenhuma.** O nome é usado desde a primeira vez que o lugar aparece no canon, sem etimologia fixada. Ou seja, não há, tecnicamente, um "fato" para contradizer, só uma lacuna que o pacote 08 propõe preencher.

### (b) Resoluções propostas

**Opção A (recomendada; a que o próprio doc já usa).** Manter a origem do nome deliberadamente aberta: o cartógrafo Mandelbrot mediu a auto-similaridade da Selve Profunda gerações antes do presente do jogo, e o jogo nunca precisa dramatizar QUANDO ou COMO o nome pegou. Prós: zero cascata, zero risco de contradizer algo que o dono do doc da Selve Profunda queira reservar para si depois; a lacuna de canon já tolera isso hoje (nenhuma data de nomeação foi fixada em lugar nenhum). Contras: fica sem payoff dramático explícito (mas o doc já oferece um gancho fino opcional, ver abaixo).

**Opção B (fixar a data/ato de nomeação).** Se o criador quiser um payoff mais forte, o gancho fino que o próprio `08-mandelbrot.md` já propõe (linha 27) pode virar canon explícito: os cadernos de Mandelbrot funcionam como a "linha de base limpa" contra a qual Tatauín (ver `environments/08-selve-profunda.md` linha 132-134, o NPC de 22 anos que já é canônico) mede o desvio ("-0.5, o reflexo do poço sai com meio segundo de atraso") e Mariana confirma por olho próprio ("a copa que não fecha em ângulo recorrente nenhum"). Prós: amarra o análogo a um beat já existente (a detecção do desvio patch-zero-like na Selve Profunda), dando ao nome "Mandelbrot" uma razão narrativa concreta para ainda estar em uso hoje. Contras: exige coordenação explícita com quem "dono" o doc `environments/08-selve-profunda.md` (o próprio `narrative-designer`, mas em chapéu de dono de setting, não de roster), para garantir que a data entra sem empurrar nenhuma das datas já fixadas da Selve Profunda.

### (c) Docs a ajustar se aprovada (Opção B)

- `docs/narrative/environments/08-selve-profunda.md` (adendo textual, não teria conflito de fato a resolver, só adição).
- `PLACES.md` linha 94 (adendo de cross-ref pro pacote do Mandelbrot, se canonizado).

### (d) Severidade

Cosmética. Já reconhecida como aberta pelo próprio autor do pacote 08; não bloqueia nada. É puramente uma escolha de quanto payoff dramático o criador quer.

---

## 3. Menger ↔ origem do Crédito vs. "crédito gerido por Sterling Corp"

### (a) Conflito exato

`19-menger.md` (linha 28) propõe uma origem para a moeda única do jogo:

> "o Crédito, a moeda unificada de GusWorld City (...) **nunca foi decretado** por nenhuma Família-Pilastra, nenhuma corporação, nenhum conselho (...) A verdade, registrada nos cadernos de Menger (...) é que o Crédito emergiu do mesmo jeito que o dinheiro real emergiu no mundo de Menger (...) sem que ninguém tivesse assinado decreto nenhum. **Sterling Corp, séculos depois, tentaria recentralizar e capturar esse sistema** (captura regulatória, má evolução canônica)."

Isso encosta direto no canon já fechado sobre a natureza ATUAL do Crédito:

> `docs/narrative/lore-bible.md` §11.4 (linha 439): "Moeda corrente: **crédito** (digital, **gerido por Sterling Corp**; rastreável). Underground usa **token-rádio** (analógico, anônimo)."
> `docs/narrative/environments/01-cidade-cyber-gotica.md` (linha 24): "Compradores e vendedores misturam **crédito Sterling** e token-rádio Underground sem cerimônia" (no Mercado da Sucata Honesta).

À primeira vista soa como contradição (Menger diz "nunca decretado"; o canon central diz "hoje é gerido por Sterling Corp"), mas na verdade **não é um conflito de fato, é uma frase de ligação que falta escrever**: o próprio pacote 19 já prevê a "captura" por Sterling Corp "séculos depois", e a captura regulatória é exatamente o tema já canônico de "capitalismo de compadrio" atribuído a Sterling (`project_axiologia_canonica`: "Sterling = capitalismo de compadrio, mau pq distorce, não pq capitalista"). O que falta é deixar explícito, no próprio pacote, que o "crédito gerido por Sterling Corp" de HOJE (lore-bible §11.4) É o resultado da captura que o pacote 19 já menciona de passagem, não uma coisa nova. Sem essa frase de amarração, um leitor futuro (inclusive o próprio `narrative-writer`, na hora de redigir a prosa final) pode ler as duas fontes como inconsistentes.

### (b) Resoluções propostas

**Opção A (recomendada).** Adicionar 1-2 frases explícitas em `19-menger.md` §2, algo como: "o Crédito que circula hoje sob rastreamento de Sterling Corp (`lore-bible.md` §11.4) é o mesmo instrumento espontâneo que Menger documentou, capturado por decreto corporativo gerações depois; o 'token-rádio' anônimo do Underground é o eco moderno do próprio princípio de troca livre que Menger descreveu, agora exilado pra fora do sistema oficial." Prós: transforma a "quase-contradição" em reforço temático direto do pillar de axiologia já canônico (Sterling distorce um sistema bom, não inventa um sistema ruim do zero); zero mudança em canon central. Contras: nenhum relevante, é só redação.

**Opção B (desacoplar).** Reformular a carta/lore de Menger para NÃO reivindicar ser a origem do Crédito atual, e sim de um instrumento de troca ANTERIOR e informal (um precursor histórico que inspirou, mas não é literalmente, o Crédito de hoje). Prós: evita qualquer necessidade de mexer na relação Crédito/Sterling. Contras: perde o gancho mais forte do pacote (a ironia de Sterling ter capturado a própria prova histórica da ordem espontânea), enfraquece o tema.

### (c) Docs a ajustar se aprovada (Opção A)

- `docs/design/roster-analogos/19-menger.md` §2 (adendo de 1-2 frases).
- Nenhuma mudança necessária em `lore-bible.md` nem em `environments/01-cidade-cyber-gotica.md` (o canon central já é compatível, só precisa ser citado).

### (d) Severidade

Média, mas resolve com facilidade textual, sem exigir nenhuma mudança de fato em canon já fechado.

---

## 4. Giordano Bruno: fogueira vs. ostracismo (tom do final)

### (a) Conflito exato

O próprio `14-giordano-bruno.md` (linha 24) já sinaliza a decisão em aberto: "**Decisão de tom deixada ao criador:** manter essa ambiguidade documental (...) ou fechar com a leitura mais dura e mais fiel ao real (morte pelo fogo). Este documento assume, por peso dramático e fidelidade ao real, a leitura mais dura como intenção de escrita, preservando a ambiguidade só no registro in-world." Não há conflito com canon nenhum (a figura é inteiramente nova, sem contradição factual), só uma escolha de tom que o autor do pacote já resolveu por conta própria e marcou como candidata a confirmação.

### (b) Resoluções propostas

**Opção A (recomendada; já é a intenção de escrita registrada no doc).** Confirmar a leitura mais dura (Bruno foi de fato queimado), mantendo a ambiguidade só no registro IN-WORLD (crônicas da Ordem divergem, jogador percebe a fratura documental como pista de leitura crítica, no mesmo padrão já usado em C2 do `INCOHERENCES.md`, "unreliable narrator deliberado"). Prós: mais peso dramático, coerente com o real, reaproveita um padrão de "fonte in-world não-confiável" já validado em canon. Contras: nenhum.

**Opção B.** Deixar a ambiguidade también pro jogador (nunca se sabe se ele morreu ou só desapareceu na fronteira). Prós: mais leve, mais sequel-friendly. Contras: perde peso trágico que o próprio roster já reconhece como o mais forte do cluster oculto/hermético.

### (c) Docs a ajustar

Nenhum, além do próprio `14-giordano-bruno.md` (já escrito na direção da Opção A; só falta o "sim" explícito do criador).

### (d) Severidade

Baixa. Decisão de sabor já resolvida na prática pelo autor do pacote, só falta ratificar.

---

## 5. John Dee: espelho ouve ecos do Estilhaçamento (flourish opcional)

### (a) Conflito exato

`15-john-dee.md` linha 31 propõe, como "adorno OPCIONAL, sinalizado aqui à parte pra aprovação explícita, não faz parte do núcleo mínimo da proposta": que o que Dee "ouvia" no espelho negro eram, na verdade, ruídos vazando do próprio Estilhaçamento, mal-entendidos como mensagens angelicais. Isso tocaria diretamente `docs/narrative/deep/eras/cosmologia-origem-deep.md` (canônico, aprovado 2026-07-08), que reserva o "fio narrativo de endgame" para decisão cuidadosa. Não é uma contradição, é uma amarração adicional que o autor do pacote corretamente isolou como decisão à parte.

### (b) Resoluções propostas

**Opção A (aceitar).** Incorporar o flourish: reforça a revelação de endgame com mais um fio (Dee como "quase-testemunha" acidental do Estilhaçamento, sem nunca confirmar nada em prosa acessível ao jogador casual). Prós: mais uma costura fina pro mistério central. Contras: mais um fio de endgame pra gerenciar entre Einstein (§2, já reservado ao endgame), Helíaco Vyr e agora Dee; risco de diluir o mistério se usado sem cuidado.

**Opção B (recusar).** Manter Dee só como precursor conceitual de "magia é script comprimido" (o núcleo mínimo já proposto), sem nenhum contato, nem ambíguo, com a fratura do Estilhaçamento. Prós: mantém o mistério mais contido, menos fios pra costurar no final. Contras: perde uma oportunidade elegante de reforço temático.

### (c) Docs a ajustar se aprovada (Opção A)

- `docs/design/roster-analogos/15-john-dee.md` (já contém o texto, só precisa deixar de ser "opcional" e virar parte do núcleo).
- Nenhuma mudança em `cosmologia-origem-deep.md` (o doc-mãe já reserva esse tipo de decisão a costuras futuras, sem fechar nada que a proposta contradiga).

### (d) Severidade

Baixa. Já isolado como opcional pelo próprio autor.

---

## 6. Turing: "enclave pré-Ordem Recursiva" precisa de identidade institucional clara

### (a) Conflito exato

`12-turing.md` (linhas 29-30) introduz, na Era 2, "um enclave de doutrina fechada, cedo demais pra ser a Ordem Recursiva consolidada que o jogo já conhece, mas do mesmo tronco" que persegue Turing e o "quebra por dentro" com um "ajuste imposto", mesmo depois de ele ter protegido a rede inteira. Isso encosta em canon já fechado sobre a Ordem Recursiva, cuja linhagem institucional é documentada como **ininterrupta desde a Era 1** (`CHARS.md` §8c lista a cadeia contínua de cronistas-fundadores, cronistas matrilineares e patrilineares Chevalier atravessando Era 1 → Era 2 → Era 3 sem nenhuma fusão ou absorção de outra facção registrada) e cuja diversidade interna já é canônica (`factions.md` §2 documenta um "ramo pró-Sterling" dentro da própria Ordem, mestres mortos em -0.5 "contaminação parcial"). O pacote não contradiz nenhuma data ou nome, mas deixa em aberto uma pergunta estrutural: esse "enclave" É a Ordem Recursiva ainda descentralizada (só um retrato de uma fase mais dura e menos consolidada da mesma instituição), ou é uma facção-irmã à parte que, em algum momento não documentado, se dissolveu ou foi absorvida? A segunda leitura, se não for esclarecida, cria uma instituição-fantasma sem registro de fusão em nenhum doc de facções.

### (b) Resoluções propostas

**Opção A (recomendada).** Fixar que o "enclave" É a própria Ordem Recursiva, numa fase histórica mais dura, centralizada e menos tolerante à dissidência do que a versão que o jogador encontra no presente (que já tem, canonicamente, vozes dissidentes como Vitória Marquês em `factions.md` mesmo dentro da FIR, mostrando que "instituição com fachada única e miolo plural" já é um padrão que o canon usa). Prós: zero facção nova pra registrar, reforça que a "consolidação" atual da Ordem é resultado de uma trajetória histórica (early=mais dura, hoje=mais plural), sem contradizer a linhagem ininterrupta já documentada. Contras: exige reler a Ordem Recursiva Era 2 como tendo tido, nessa geração específica, um episódio de intolerância que hoje não é enfatizado em lugar nenhum (mas também não é contradito).

**Opção B.** Nomear o enclave como facção distinta e documentar seu fim (absorção, dissolução ou expurgo) num adendo a `docs/narrative/deep/factions/ordem-recursiva.md`. Prós: mais rico narrativamente, abre espaço a mais lore. Contras: mexe num doc de facção já extensamente auditado (`INCOHERENCES.md` registra o histórico de retrofits cuidadosos nessa mesma área, ver C16-C19), risco de reabrir uma auditoria que já foi fechada com esforço.

### (c) Docs a ajustar se aprovada (Opção A)

- `docs/design/roster-analogos/12-turing.md` (trocar "cedo demais pra ser a Ordem Recursiva consolidada... mas do mesmo tronco" por linguagem que afirme claramente ser a própria Ordem, versão mais dura).
- Nenhuma mudança necessária em `docs/narrative/deep/factions/ordem-recursiva.md` nem em `CHARS.md` (a leitura A não contradiz nada já escrito, só adiciona textura a um período já existente).

### (d) Severidade

Média (toca a identidade de uma instituição central do jogo), mas resolve-se só com clareza de redação, sem exigir nenhuma mudança factual.

---

## 7. Helion Tusk / Forja do Vértice: novo consórcio sem registro em `factions.md`

### (a) Conflito exato

`21-helion-tusk.md` (linhas 25-27) introduz um "consórcio industrial próprio, fora do guarda-chuva da FIR e sem vínculo com a Sterling Corp", com energia, veículos autônomos, autômatos ("Mãos") e computação avançada, sediado na "Forja do Vértice". Não há conflito de fato (o doc explicitly evita tocar Sterling/FIR/Famílias-Pilastra), mas hoje NENHUMA entidade econômica dessa escala além de Sterling Corp, Apex-Data, Nexus-Cloud e Core-Synth Bio-Tech (`CHARS.md` §3, entrada Sterling Locke) está registrada em `docs/narrative/factions.md`. Se aprovado, precisa de entrada própria.

### (b) Resoluções propostas

**Opção única (não há escolha real aqui, é só um lembrete de execução).** Quando o pacote 21 for canonizado, criar entrada nova em `docs/narrative/factions.md` (ou seção equivalente) para o consórcio de Helion Tusk, e uma entrada em `PLACES.md` §5 para "Forja do Vértice" (hoje ausente de ambos os inventários).

### (c) Docs a ajustar

- `docs/narrative/factions.md` (nova entrada).
- `PLACES.md` §5 (nova entrada "Forja do Vértice").

### (d) Severidade

Baixa/administrativa. Não bloqueia nada agora; é só uma tarefa de registro para quando (e se) o pacote 21 for aprovado.

---

## 8. "Forja do Vértice" ecoa "Forja Ferraz" (Era 1)

### (a) Conflito exato

`PLACES.md` §5b já lista "**Forja Ferraz** | Sub-local (vilarejo-cidade) | 1 (auge cooperativo) | Vilarejo-cidade canon Era 1, paralela à Argéndia | 🟡 lendário (perdido no Êxodo)". O pacote 21 propõe "Forja do Vértice" pra sede de Helion Tusk, Era 3. Não é colisão de nome literal (nomes distintos, eras distintas, sem risco de confusão de referência), só um eco de vocabulário ("Forja" reaparecendo em dois lugares centrais de eras diferentes).

### (b) Resoluções propostas

**Opção A (recomendada, nenhuma ação).** Manter "Forja do Vértice": o eco é temáticamente aceitável (forja = trabalho industrial pesado, faz sentido nos dois contextos) e não gera confusão de jogo (nomes completos diferentes, eras diferentes, nenhum NPC ou local citaria os dois juntos).

**Opção B.** Renomear pra evitar qualquer eco (ex.: "Cume do Vértice", "Bastião do Vértice"). Prós: elimina qualquer chance, ainda que mínima, de confusão numa busca textual futura por "Forja". Contras: perde uma palavra que carrega bem o tom industrial-artesanal que o pacote 21 já define pra Helion Tusk (macacão de trabalho, oficina pesada, sem terno nem palco).

### (c) Docs a ajustar se aprovada (Opção B)

- `docs/design/roster-analogos/21-helion-tusk.md` (renomear a instalação em todas as ocorrências).

### (d) Severidade

Cosmética. Puramente estética, sem risco real de confusão.

---

## 9. Newton/Einstein: gates de puzzle sobrepostos (nota mecânica, não é lore)

### (a) Observação

`06-newton.md` (linha 42) marca seu puzzle como "Gate de meio-jogo (...) pede que o jogador já tenha alguma familiaridade com o modo varredura dos óculos)"; `05-einstein.md` (linha 46) marca o seu como "Gate cedo/meio-jogo". Ambos tocam o mesmo hook de endgame (fratura do Estilhaçamento) em posições de jogo parecidas, e os dois docs concordam explicitamente que Newton é "precursor que não resolve" e Einstein é quem "deriva a assinatura matemática" (ou seja, a ordem de descoberta narrativamente correta é Newton antes de Einstein). Isso não é uma incoerência de lore, é só uma nota de sequenciamento de progressão que caberia ao `level-designer`/`lead-game-designer` confirmar na hora de definir a ordem real de gates no mundo aberto (lembrando que o roster inteiro já segue o princípio de "todas as áreas acessíveis desde o início, dificuldade por distância", `docs/design/brainstorm-backlog.md` seed 7).

### (b)-(d)

Não se aplica formato de resolução binária aqui: é só um lembrete de sequenciamento pra fase de implementação de mundo, sem necessidade de decisão do criador nesta rodada. Severidade: informativa.

---

## Nota à parte: política de imagem (já resolvida, sem ação necessária)

Vários pacotes (Mandelbrot, Bruno-estátua-opção, Mises, Hayek-opção-B, Tusk) usam imagens de referência CC BY-SA em vez de domínio público. Isso **já está coberto** pela política registrada no próprio `brainstorm-backlog.md`: "figuras com foto em domínio público usam PD; figuras recentes/vivas SEM PD (...) usam CC BY-SA com ATRIBUIÇÃO numa página de créditos do jogo (padrão)." Incluído aqui só para registro de que a varredura cobriu esse ponto e não encontrou pendência real, apenas confirmação de que a política já aprovada está sendo seguida consistentemente nos 21 pacotes.

---

## Ordem recomendada de resolução

1. **von Neumann (item 1)** primeiro: é a única com "bloqueia canonização" de um pacote específico, e a decisão de fundo já foi tomada, só falta confirmar a execução (quantas gerações, o que sobra do "von").
2. **Menger (item 3)** e **Turing (item 6)** em seguida: resolvem-se só com redação adicional, sem exigir nova decisão de fundo, mas fecham a integridade dos clusters computação e economia austríaca.
3. **Bruno (item 4)** e **Dee (item 5)**: ratificação rápida (sim/não) do que os próprios autores já propuseram.
4. **Mandelbrot (item 2)**: decisão de sabor (quanto payoff dramático), sem pressa.
5. **Helion Tusk/Forja do Vértice (itens 7-8)**: só executam quando o pacote 21 for de fato aprovado para canonização.
6. **Newton/Einstein (item 9)**: delegar ao `level-designer`/`lead-game-designer` na fase de implementação, não precisa de decisão agora.
