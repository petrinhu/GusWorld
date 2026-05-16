# Arco Principal — GusWorld

> **Status:** Revisão 1 concluída em sessão colaborativa (2026-05-15). Substitui versão prévia (com Iolanda como antagonista) integralmente. **Canônico.**
>
> **Escopo:** 4-8h gameplay. 3 atos. 1 antagonista principal (Sterling Locke) + 1 antagonista-sistema (Patch-Zero) + 1 traidor interno (Dante). 1 ending principal com 3 variações knowledge-gated. Foldback narrativo, branching curto e contextual.
>
> **Cross-ref obrigatório:** `lore-bible.md`, `factions.md`, `characters/sterling-locke.md`, `characters/patch-zero.md`, `characters/party.md`, `comic-reliefs.md`.

---

## Premissa (1 frase testável)

> **Gus, prodígio analítico de 11 anos, precisa decodificar Sterling Locke para evitar a esterilização corporativa do mundo vivo.**

Decodificar não é matar. É **ler o sistema do adversário até conseguir desativá-lo**. Sterling é um cofre filosófico (etimologia: Locke = fechadura). Gus é a chave: cada companion recrutado, cada conjuro mestrado, cada entrada de Diário, contribui para essa decodificação acumulativa.

---

## Theme central

> **A inteligência mais alta é a que serve à vida.**

Casa diretamente com Compilação vs Interpretação (DRE de Sterling vs C-Arcane de Gus; ver `characters/prelore_vilao.md`):

| Eixo | Sterling (DRE) | Gus (C-Arcane) |
|---|---|---|
| Postura | Interpreta. Reescreve. Não respeita hardware. | Compila. Otimiza. Respeita substrato. |
| Relação com a Selve | Captura, vira variável editável | Lê, coopera, deixa em paz |
| Inteligência | Serve a si mesma. Consome para crescer. | Serve à vida. Mantém para preservar. |
| Final | Esteriliza o mundo vivo | Mantém o mundo vivo |

Cada beat narrativo precisa dialogar com esse eixo. Falas, decisões, escolhas e mecânicas reforçam.

---

## Tom e referências

- **Tom dominante:** otimismo investigativo com camada gótica subjacente. *Chrono Trigger* (party leve, beats emocionais limpos, sem cinismo adulto) é referência primária. Atmosfera de *Hollow Knight* (lore por ambiente, melancolia contida) atravessa a Selve e a Cúpula Sterling.
- **Pace alvo:**
  - 50% otimismo curioso (recrutamento, descoberta, sidequest, comic relief).
  - 30% tensão crescente (presença Sterling, infecção Selve, fricção party).
  - 15% melancolia (Polis-Vermelha caída, foreshadow Dante, custo de cada vitória).
  - 5% terror puro (Patch-Zero ativo, áudio ambient infectado).
- **Público:** 11-15 anos. Tom analítico, não power-fantasy adulta. **Sem palavrão; "que horror" é o limite** (Pillar 4).

---

## Antagonistas

### Sterling Locke (antagonista principal)

Detalhes integrais em `characters/sterling-locke.md`. Resumo:

- **Goal externo:** implementar GRE (Global Runtime Environment) — envelopar a Selve em máquina virtual interpretada, deletar espécies por linha de comando.
- **Filosofia:** DRE (Dynamic-Runtime Evaluation). Compilação é submissão; interpretação é controle.
- **Modus operandi:** presença constante via hologramas + cameos pessoais em cada arco companion. Cresce em protagonismo do ato 1 ao 3.
- **Sem redenção** (P4 + D.2). Vilão puro. Filosoficamente coerente, eticamente monstruoso.

### Patch-Zero (antagonista-sistema)

Detalhes em `characters/patch-zero.md`. Resumo:

- **Natureza:** anti-padrão (rompe Knowledge Progression localmente) + consciência alien (planeja, fala, manipula).
- **Origem multi-causal:** bug primordial da Selve + captura Sterling fracassada + amostra Polis-Vermelha + escape do laboratório.
- **Manifestação multi-canal:** texto no Diário do Gus, áudio ambient, persona dialogável em bosses, bug visual ambient.
- **Selado, não destruído**, no climax. Hook sequel.

### Dante "Grid" Alencar (traidor interno)

Ver `characters/party.md` para spec completa. Resumo:

- **Companion funcional do ato 1 até o reveal.**
- **Sterling o aliciou** anos antes do jogo (Renfield-like; sedução pragmática + admiração pela capacidade do vilão de dobrar a infraestrutura).
- **Acesso de root nos implantes do Gus** (única pessoa autorizada a fazer manutenção da Matriz Ortodôntica). Instala rootkit gradual. Telemetria do Gus espelhada para o mainframe Sterling.
- **Foreshadow narrativo + mecânico** (B.7 e D.1). Stats do Gus degradam visivelmente no late game.
- Mini-boss intermediário no ato 3. Não participa do final boss (Sterling).

---

## Estrutura de três atos (visão alto nível)

| Ato | Duração alvo | Função |
|---|---|---|
| **Ato 1 — Diagnóstico** | 0-20% (~45min-1h30) | Setup. Cidade. Gus solo. Recrutamento dos primeiros companions. Catalyst. |
| **Ato 2 — Compilação** | 20-75% (~2h30-5h) | Seis arcos companion. Sterling presente em todos. Knowledge cresce. Foreshadow Dante dispara em 50%. |
| **Ato 3 — Decodificação** | 75-100% (~1h-1h30) | Catedrais de Silício corrompidas → Cúpula Sterling. Dante reveal e mini-boss. Boss final 2 fases. Triple hook pós-créditos. |

---

## Oito beats macro

Estrutura geral: setup + 6 arcos companion (1 beat principal cada) + clímax. Cada arco companion segue **Kishōtenketsu interno** (4 sub-beats: Ki / Sho / Ten / Ketsu).

| # | Beat | Função | Ato |
|---|---|---|---|
| 1 | **Setup** | Gus solo no Núcleo Metropolitano. Catalyst. Recrutamento Companion #1. | 1 |
| 2 | **Arco Companion #2** | Primeiro arco em ordem livre. | 2 |
| 3 | **Arco Companion #3** | Segundo arco. | 2 |
| 4 | **Arco Companion #4** | Terceiro arco. **Foreshadow Dante dispara aqui (50%)** se ainda não recrutado. | 2 |
| 5 | **Arco Companion #5** | Quarto arco. | 2 |
| 6 | **Arco Companion #6** | Quinto arco. | 2 |
| 7 | **Arco Companion #7** | Sexto e último arco companion. Convergência iminente. | 2/3 |
| 8 | **Climax — Decodificação Final** | Dante reveal, mini-boss Dante, boss Sterling 2 fases, ending. | 3 |

Ordem dos arcos companion (#2 a #7) é **livre** (B.6). Cada um funciona standalone. Foreshadow Dante não depende de ordem específica.

Total de sub-beats internos: 6 arcos × 4 (Kishōtenketsu) = **24 sub-beats** + setup + clímax = **~26 sub-beats jogáveis**.

---

## Ato 1 — Diagnóstico (setup)

### Estado inicial do mundo

GusWorld City funciona aparentemente. Janelarum trava periodicamente, mas as pessoas convivem (`comic-reliefs.md` cena 11). Holografia Sterling Corp está em todo poste no Setor Mirage. Anel Verde tem checkpoints federais. A Selve está aparentemente calma. Gus tem 11 anos e mora num apartamento modesto no Núcleo Metropolitano.

### Setup (1-1.5h gameplay)

| Sub-beat | Conteúdo |
|---|---|
| **Opening Image** | Núcleo Metropolitano, noite. Gus na bancada, soldando upgrade na Matriz Ortodôntica. Câmera 3/4 puxa lentamente. Janela: holograma Sterling distante pulsa. Mãe entra com chá. Sem palavras. Estabelece: lar, lógica, ritmo silencioso. |
| **Tutorial diegético** | Gus testa o upgrade saindo pra praça. Combate-tutorial contra drone Sterling Corp "comportamento emergente inesperado". Player aprende stack de 3 tokens, Vetor do Gambito, óculos táticos. Drone se dissolve em compilação erro (estilizado, sem gore — P4). |
| **Catalyst** | Anomalia massiva detectada via Matriz Ortodôntica. Sinal partindo da Orla Recursiva. Padrão fractal da Selve apresentando ruído **incoerente** (Patch-Zero pré-anuncia, sem nome ainda). Gus decide investigar. |
| **Mãe** | Mãe pede pra Gus não ir. Não é briga; é cuidado contido. Beat curto. Gus vai mesmo assim. |
| **Recrutamento Companion #1** | Encontro com **um companion à escolha do jogador** dependendo de onde Gus vai primeiro (Setor Mirage = Iara primeiro; Periferia Industrial = Dante primeiro; Dutos = Cauã primeiro; Zona do Silêncio = Linda primeiro; Catedrais Neo-Sylvania = Bento primeiro; fronteira-Selve = Jaci primeiro). |
| **Foreshadow Sterling** | Holograma Sterling no Setor Mirage faz pronunciamento ao vivo enquanto Gus passa. Tom monolítico, frio. Gus não sabe ainda quem é mas reconhece a frieza. |
| **Foreshadow Dante** | Independente de ordem, cena 4 de `comic-reliefs.md` ("Force Push") **não dispara ainda**. Dispara em ato 2 após 3 arcos completos. |
| **Break into Two** | Gus + Companion #1 cruzam o limiar narrativo (entram em outro setting). Câmera muda de paleta. Trilha sonora muda de chave. Ato 2 começa. |

---

## Ato 2 — Compilação (seis arcos companion)

### Estrutura de cada arco companion (Kishōtenketsu interno)

Cada arco companion ocupa aproximadamente **30-50 minutos** de gameplay e segue:

| Sub-beat | Função | Conteúdo padrão |
|---|---|---|
| **Ki (起)** — Introdução | Estabelecer o setting e a situação do companion | Gus chega no setting. Companion em conflito com a facção home (disputa interna — C.2). Sterling Corp tem dedo na disputa. |
| **Sho (承)** — Desenvolvimento | Aprofundar a disputa e a aliança Gus-companion | Mini-quest pré-recrutamento (resolução da disputa de facção). Companion se alia. Ambient: cena Sterling cresce. |
| **Ten (転)** — Twist | Reviravolta que muda o entendimento | Reveal: Sterling Corp está por trás de um aspecto inesperado da crise. Patch-Zero aparece em manifestação local (canal 4 mínimo; canal 3 em arcos centrais). |
| **Ketsu (結)** — Resolução | Fechar o arco e amarrar de volta na trama central | Combate culminante. Companion oficialmente entra na party. Pista sobre Sterling/GRE/Patch-Zero registrada no Diário. Conjuro/token novo destravado. |

### Os seis arcos companion (em ordem nominal — jogador escolhe ordem real)

#### Arco Cauã (Dutos Infernais)

- **Ki:** Cauã lidera grupo de runners adolescentes nos Dutos. Sterling Corp tentou comprar acesso vertical via FIR. Runners resistem; FIR ameaça inundar túneis com efluente industrial.
- **Sho:** Gus + Cauã navegam os Dutos. Combate vertical (pulsos EMP do Cauã interagem com pistões hidráulicos do setting). Plantam call-back de `comic-reliefs.md` cena 3 ("Funciona no meu Drive"). Conjuro novo: **Pulso EM Concêntrico**.
- **Ten:** Descobrem laboratório FIR escondido nos Dutos: estudam frequências da Matriz Ortodôntica do Gus (sem saber que é dele). **Sterling Corp pediu pra FIR replicar a tecnologia**. Cena Sterling: holograma elogiando "iniciativa da Federação" enquanto Cauã ouve, congelado.
- **Ketsu:** Combate. Destruição do laboratório FIR. Cauã se junta à party. Cena cômica `comic-reliefs.md` 10 ("Não é magia, é cache") pode disparar aqui.

#### Arco Iara (Setor Mirage)

- **Ki:** Iara é desertora do Cult Mirage. Cult planeja festival de "atualização sensorial" patrocinado por Sterling Corp (na verdade, instalação massiva de monitoramento via holografia). Iara quer sabotar.
- **Sho:** Infiltração ofuscamento. Gus aprende com Iara como Óxido funciona (`comic-reliefs.md` cena 5 — "Expressão Regular" pode disparar). Conjuro novo: **Decoy Lumen**.
- **Ten:** No núcleo do Cult Mirage, descobrem terminal direto com Sterling Corp. Sterling aparece em pessoa por holograma de alta fidelidade — primeira aparição "íntima" no jogo. Diz à Iara: "Você acha que está sabotando, mas está apenas alimentando o sistema com escolha. Cada negação é uma instrução que ele lê."
- **Ketsu:** Combate. Sabotagem parcialmente bem-sucedida (festival cancelado, mas Sterling colheu dados). Iara se junta. Sentimento: vitória amarga.

#### Arco Bento (Catedrais de Neo-Sylvania)

- **Ki:** Bento é o herdeiro próximo da Ordem Recursiva. Outros mestres da Ordem propõem **negociar com Sterling Corp** (aceitar parceria comercial pra "modernizar" as catedrais). Bento se opõe. Rachadura interna.
- **Sho:** Gus + Bento exploram catedrais profundas. Confronto pillar 2 visual: Asmódico vs C-Arcane (`comic-reliefs.md` cena 2 dispara). Conjuro novo: **Cronômetro Ressonante**. Gus descobre cripto-glifos Neo-Sylvania que conectam com Patch-Zero (canal 4 — bug visual em vitrais).
- **Ten:** Mestres pró-Sterling tentam entregar uma catedral menor a engenheiros corporativos. Patch-Zero **estava lá**, dormente. Sterling Corp acorda sem querer. Cena Sterling: ele assiste o desastre por câmera, satisfeito ("teste útil"). Mestres pró-Sterling morrem off-screen (estilizado, compilação erro coletiva — P4).
- **Ketsu:** Combate contra Patch-Zero localizado (primeira manifestação canal 3: persona dialogável). Bento se junta. Ordem Recursiva oficialmente toma posição: contra Sterling.

#### Arco Linda (Zona do Silêncio)

- **Ki:** Linda lidera célula do Underground do Silêncio. Sterling Corp pressiona pra demolir antenas mortas e construir nova infraestrutura corporativa. Linda recebe mensagem cifrada via rádio analógica de Cidades-Gêmeas: "resista, mantenha as antenas, segredo importante."
- **Sho:** Gus + Linda decifram mensagem usando ressonância sônica + óculos táticos. Conjuro novo: **Eco do Cânion**. Cena cômica possível.
- **Ten:** Mensagem revela: Sterling Corp planeja usar a rede de antenas para **transmitir Patch-Zero em larga escala** (broadcast viral). Cidades-Gêmeas estão monitorando há meses, prestes a soar alarme global. Linda em choque: Underground tem cobertura mas pouca força. Cena Sterling: holograma público minimizando "boatos sobre vírus digital".
- **Ketsu:** Combate. Sabotagem de transmissor corporativo. Linda se junta. Cena melancólica: Linda toca rádio analógico, ouve sussurros distorcidos de Polis-Vermelha (Patch-Zero canal 2).

#### Arco Dante (Periferia Industrial) — arco do traidor

- **Ki:** Dante mantém oficina independente na Periferia Industrial. Vive em rivalidade com FIR (publicamente). Aceita fazer manutenção da Matriz Ortodôntica do Gus de graça ("você me ajudou na contagem semana passada"). Gus confia.
- **Sho:** Mini-quest pré-recrutamento: Gus + Dante neutralizam patrulha FIR. Dante mostra habilidade técnica excepcional (instalação rápida de torres modulares). Cena cômica plant: `comic-reliefs.md` cena 4 NÃO dispara ainda. Foreshadow visual: expressão de Dante já tem 1-2 micro-momentos frios.
- **Ten:** Reveal aparente: FIR é vassala Sterling. Dante "descobre" isso junto com Gus (encenação; ele sempre soube). Indignação fingida. Cena Sterling: holograma diz que "talentos individuais da Periferia merecem reconhecimento corporativo".
- **Ketsu:** Combate contra unidade FIR-Sterling. Dante se junta. Conjuro novo (técnico): **Torre Modular Anti-Aérea** (suporte tático). **A partir deste arco, telemetria do Gus começa a ser espelhada para Sterling Corp.** Sem que jogador saiba ainda.

#### Arco Jaci (fronteira-Selve / vilarejo do Pelicano Branco)

- **Ki:** Jaci é farmacêutica jovem do vilarejo do Pelicano Branco (fronteira-Selve; EE-14 em `comic-reliefs.md`). Vilarejo é alvo de "saneamento ecológico" Sterling Corp (eufemismo: extração de bio-amostras forçada). Jaci precisa sintetizar antídoto para um surto que **a Sterling Corp causou clandestinamente** (vetor 4 do Patch-Zero — escape do laboratório).
- **Sho:** Gus + Jaci entram na Orla Recursiva colhendo sementes-relíquia. Tutorial Selve (bestiário, padrões fractais, scan via óculos). Conjuro novo: **Antídoto Sintético** + **Bio-Sutura Rápida**.
- **Ten:** No coração da Orla, encontram **Patch-Zero canal 3** (boss arena dialogável). Patch-Zero oferece a Jaci: "eu paro o surto se você me deixar passar pelo vilarejo." Jaci recusa horrorizada. Sterling Corp aparece via holograma drone, oferece "ajuda" pra debelar o surto (cobrando o vilarejo como propriedade).
- **Ketsu:** Combate Patch-Zero local (instância pequena). Jaci se junta. Antídoto sintetizado. Vilarejo salvo, mas Patch-Zero escapou para zona mais profunda da Selve.

---

## Branching foldback — 5 pontos

Estrutura **foldback + knowledge-gated**. Branches curtos divergem temporariamente e convergem. Sem memória global pesada (Pillar de escopo G1).

| # | Localização | Tipo | Reconvergência |
|---|---|---|---|
| 1 | **Ato 1 — recrutamento Companion #1** | Decisão de destino (qual setting Gus visita primeiro). 6 opções. | Sem reconvergência forçada — todos os arcos virão, ordem solta. |
| 2 | **Ato 2 — Arco Iara, Ten** | Decisão: confrontar Sterling holograma direto (mais agressivo) OU sair em silêncio (preserva opção de retorno). Afeta 1-2 falas no próximo arco e flag no Diário. | Reconverge no Ketsu do próprio arco. |
| 3 | **Ato 2 — Arco Bento, Sho** | Decisão: aceitar Asmódico como linguagem secundária do Gus (3 tokens novos) OU rejeitar (mantém pureza C-Arcane, ganha bônus mestria). | Reconverge imediatamente; diferença mecânica permanente. |
| 4 | **Ato 2 — Arco Linda, Ten** | Decisão: alertar Cidades-Gêmeas imediatamente (corre risco de comprometer Underground local) OU manter sigilo (Underground sobrevive, Cidades-Gêmeas demoram a reagir). | Reconverge; afeta uma das variações de ending (Ouro requer alertar). |
| 5 | **Ato 3 — Dante reveal** | Decisão final: **executar** Dante (rápido, melancólico) / **capturar** (mais difícil mecanicamente, abre cena pós-combate) / **oferecer redenção forçada** (Dante sabotado por Sterling em tempo real; arriscado, alta chance de falha, abre única possibilidade de ending Ouro). | Reconverge no boss final Sterling. |

Três dos seis arcos (Iara, Bento, Linda) têm branching curto interno. Três (Cauã, Dante, Jaci) são lineares (curados para tom + escopo).

---

## Foreshadow Dante — timing detalhado

Dante é traidor canônico desde o ato 1 — **antes do jogo começar**, Sterling já o aliciou.

### Mecânica de foreshadow

| Tipo | Implementação | Quando |
|---|---|---|
| **Narrativo — dialogue** | Falas de Dante com micro-distância emocional. Despreza histórico/tradição em momentos chave. | Toda a campanha |
| **Narrativo — comportamento** | Expressão facial progressivamente mais fria. Manchas de graxa migram para mãos limpas (recebe upgrade Sterling). | Spec visual Dante |
| **Narrativo — ambient** | Pontos discretos: Dante "atrasa" durante manutenções, espelha terminal por 2 segundos antes de fechar. | Após 50% (3 arcos completos) |
| **Mecânico — degradação stats Gus** | Após 50%, stats do Gus passam a ter **flutuação anormal**: -2% precisão, -1% mestria de carta. Telemetria comprometida. Visível no Diário do Gus subseção "Diagnóstico de Hardware". | Capítulos finais ato 2 e ato 3 |
| **Narrativo — cena planted** | `comic-reliefs.md` cena 4 ("Force Push") dispara **automaticamente** após 50% (3 arcos completos). Foreshadow forte. | Auto-trigger 50% |
| **Narrativo — Diário entry Patch-Zero** | Patch-Zero comenta enigmaticamente em entry late ato 2: "alguém perto de você não compila. interpretam por você." | Após 75% |

Reveal contextualiza retroativamente tudo. Player que prestou atenção: "ah." Player que não prestou: jogo permite re-leitura via Diário do Gus (Knowledge Progression destrava re-leitura cronológica do arco Dante).

### Cena de reveal

- **Lugar:** entrada das Catedrais de Silício corrompidas.
- **Trigger:** party precisa de hack na entrada da Cúpula. Dante se oferece. Gus aceita.
- **Sequência:** Dante começa o hack. Mid-hack, **sua mão modular começa a operar em padrão Sterling reconhecível** (player com Knowledge alta detecta no ato; player com Knowledge baixa só vê depois). Patch-Zero (canal 1) faz entry no Diário em tempo real: `dante = vetor de injeção. cobertura confirmada.` Gus levanta a Tavus-Drive lentamente. Dante para o hack. Vira-se. Sem culpa, sem raiva. Apenas cansaço.
- **Linha de Dante:** "Eu não pedi pra você confiar."
- **Linha de Gus:** "Você também não pediu pra fazer manutenção. Mas se ofereceu."

Combate começa.

---

## Climax — combate final em três etapas

### Etapa 1 — Mini-boss Dante

- **Arena:** entrada das Catedrais de Silício corrompidas. Estrutura híbrida instável (mistura cidade + Selve, falha técnica do Sterling).
- **Mecânica:** Dante usa torres modulares Sterling-upgraded e implantes de root acelerados. Cada turno, ele tenta **explorar a degradação dos stats do Gus** (debuff que se acumula).
- **Solução tática:** Gus precisa **desinstalar o rootkit** durante o combate via puzzle mid-combate (3 turnos de "desinstalação" durante os quais Gus fica vulnerável; party precisa cobrir).
- **Resolução:** Dependendo do branching point #5:
  - **Executar:** Dante cai. Cena curta, sem fala. Party segue.
  - **Capturar:** Dante incapacitado (Pillar 4 — companions imortais com incapacitação se aplica aqui também). Sterling o sabotará à distância no ato seguinte; Dante morre off-screen ou sobrevive (decisão final pós-créditos B.8: vivo off-screen, capturado ou fugitivo).
  - **Redenção forçada:** mini-puzzle de re-hacking. Sucesso = Dante volta como aliado de última hora para a fase Sterling (raro, requer Knowledge alta). Falha = morre tentando.

### Etapa 2 — Sterling Fase 1: Rede Distribuída

- **Arena:** corredor de aproximação à Cúpula. Cromo espelhado, vazio.
- **Mecânica:** combate contra **enxame de proxies/drones Sterling**. Sterling não aparece pessoalmente; opera remoto. Player precisa **derrotar a Rede primeiro**.
- **Vantagem por Knowledge:** acumulação de Bestiário ajuda — drones têm padrões previsíveis se inimigo foi catalogado. Player Bronze (low Knowledge) sofre mais.
- **Soundscape:** silêncio crescente. Patch-Zero ambient (canal 2) sussurra esporadicamente, mas é ruído residual.

### Etapa 3 — Sterling Fase 2: Locke Core exposto

- **Arena:** Cúpula Sterling interior. Geometria euclidiana perfeita. Branco-cromo. Sterling no centro.
- **Mecânica:** Sterling exposto pessoalmente (Locke Core implant ativo — neural cortical). Combate **decodificação**. Gus precisa **quebrar a cifragem do Locke Core**, não infligir HP suficiente. Mecânica de puzzle: cada turno, Sterling apresenta cifragem nova (mini-puzzle 5-10 segundos). Solução correta = dano real. Erro = Sterling retalia.
- **Cresce com Knowledge:** Diário entries acumuladas durante o jogo aparecem como **dicas in-combat** (entradas relevantes piscam). Jogador Ouro tem ~30% mais dicas.
- **Patch-Zero ativo na arena:** canal 3 (persona) interfere periodicamente. Oferece ajuda ao Gus em troca de "sair pela porta" depois. Player pode aceitar (consequências) ou ignorar.
- **Resolução:** Sterling cai sem fala dramática. Apenas cessa de operar. Cúpula apaga. Patch-Zero precisa ser selado (mini-puzzle final).

---

## Endings — três variações knowledge-gated

Um único ending canônico estrutural (Sterling derrotado, Patch-Zero selado, Gus volta pra casa). **Três variações** determinadas por **Knowledge Progression total no Diário do Gus** (não por escolhas binárias; é gradiente).

### Bronze (Knowledge baixo — <40%)

**Tom:** vitória amarga, sobrevivência apertada.

- Sterling cai mas Patch-Zero quase escapa. Selagem parcial.
- Várias perdas off-screen: Polis-Vermelha não consegue recuperação. Cidades-Gêmeas demoram a estabilizar. Vilarejo do Pelicano Branco perde sementes-relíquia.
- Dante: se vivo (capturado), morto pela Sterling Corp residual.
- Cena final: Gus na bancada, soldando. Mãe entra com chá. Janela: holografia Sterling apagou, mas neon ainda fraco. Sem som de pássaro mecânico.
- Diário entry final: "Sinal anômalo detectado em [cidade-irmã não nomeada]." (Triple hook obrigatório.)

### Prata (Knowledge médio — 40-80%)

**Tom:** vitória limpa, ainda melancólica.

- Sterling cai. Patch-Zero selado (não destruído; coerência P2 — caos irredutível na fronteira).
- Polis-Vermelha começa recuperação lenta. Cidades-Gêmeas estabilizadas. Vilarejo seguro.
- Dante: morte/captura conforme branching point #5.
- Cena final: Gus na bancada. Mãe entra. Janela: holografia Sterling apagou, neon de volta ao tom otimista. Som distante de pássaro mecânico (Catedrais Neo-Sylvania).
- Diário entry final: "Sinal anômalo detectado em [cidade-irmã]." (Triple hook obrigatório.)

### Ouro (Knowledge alto — >80%)

**Tom:** vitória plena, esperança fundada.

- Sterling cai. Patch-Zero selado em laboratório controlado (não solto). Cooperação internacional ativa.
- Cidades-irmãs salvas — Cidades-Gêmeas e Heliópolis-Nova se libertam de Sterling Corp em cascata pós-jogo. Polis-Vermelha em recuperação acelerada.
- **Bonus epilogue (2-3 cenas adicionais):**
  - Cena 1: vilarejo do Pelicano Branco em festival. Jaci com avó.
  - Cena 2: Bento em catedral aberta ao público; primeira lição de Asmódico a uma criança.
  - Cena 3: Cauã + Linda fundando oficina mista nos Dutos.
- Dante: redenção forçada só destrava ending Ouro se branching #5 = redenção e sucesso. Caso contrário, Ouro alcançável mas Dante morto.
- Diário entry final: "Sinal anômalo detectado em [cidade-irmã]." (Triple hook obrigatório, mesmo em Ouro — ameaça persiste.)

### Por que knowledge-gated, não escolha binária

Filosofia Pillar 1 (lógica vence força) + Theme (inteligência serve à vida): **quem aprende mais salva mais**. Mecânica recompensa investigação, leitura, scan, anotação. Player que farma Bestiário sem ler Diário não chega no Ouro. Player que joga atento, lê, decifra, ganha o ending pleno. **Sem moral-meter visível**; flag silenciosa.

---

## Triple hook pós-créditos (B.8)

Independente de ending (Bronze/Prata/Ouro), três stingers tocam em sequência após os créditos:

### Hook 1 — Stinger visual (10 segundos)

Laboratório distante (provavelmente Cidades-Gêmeas, não nomeado). Câmara hermética. Célula isolada num tubo translúcido. **Pulsa**. Padrão Perlin reconhecível. Câmera 3/4 zoom-in lento. Corte.

### Hook 2 — Diário do Gus entry final

Tela escura. Texto aparecendo lento como se Gus estivesse digitando:

```
[ENTRY ???]
sinal anômalo detectado em [cidade-irmã].
padrão familiar.
adormecido? acordando?
verificar amanhã.
```

### Hook 3 — Dante off-screen

Tela escura. Áudio diegético: passos. Respiração contida. Voz de Dante (ou voz de alguém que poderia ser ele) sussurrando:

```
"...não pedi pra você confiar.
mas não pedi pra ele também.
e ele ainda paga."
```

Corte para créditos finais.

**Função:** prepara sequel (não obrigatório; G1 fica fechado se for o caso) e amarra emocionalmente o jogador. O jogo terminou, mas o mundo continua.

---

## Ludonarrative harmony — mapeamento beat × mecânica

Toda decisão narrativa deve ter expressão mecânica.

| Beat narrativo | Expressão mecânica |
|---|---|
| Gus respeita o substrato (theme central) | Compilação do Codex (mecânica-âncora #2): jogador otimiza tokens, não inflaciona força |
| Sterling consome o substrato | Sterling Fase 1 tem proxies que **drenam Knowledge** do Gus se não desativados |
| Inteligência serve à vida | Knowledge Progression: jogar atento = stats melhores, RNG menor, ending Ouro |
| Sterling é decodificável, não destrutível | Fase 2 = puzzle de decifragem, não DPS-check |
| Patch-Zero é antagonista-sistema | Manifestação multi-canal afeta UI, Diário, áudio (não só combate) |
| Dante traidor | Stats Gus degradam visivelmente após 50% — sinal mecânico real |
| Companions imortais | HP=0 = incapacitado, vai pra hospital. Não há morte de companion na campanha. |
| Foreshadow honesto | Cena 4 `comic-reliefs.md` planta sinal antes do reveal (Chekhov) |
| Ending knowledge-gated | Sem moral-meter visível; jogador é recompensado por entender |

**Dissonâncias intencionais:** zero deliberadas neste arco. Toda mecânica reforça a narrativa.

---

## Foreshadowing tracker

| # | Setup | Payoff | Tipo | Quando setup | Quando payoff |
|---|---|---|---|---|---|
| 1 | Holograma Sterling em todo poste Setor Mirage | Sterling antagonista revelado em arco Iara | Visual | Ato 1 | Ato 2 (Arco Iara) |
| 2 | Janelarum trava (`comic-reliefs.md` 11) | Apex-Data Systems caiu de modo similar; reveal Sterling backstory | Diálogo | Ato 1 | Ato 2 (qualquer arco) |
| 3 | Dante manutenção gratuita | Rootkit Sterling instalado | Mecânico + diálogo | Ato 2 (Arco Dante) | Ato 3 (reveal) |
| 4 | Stats Gus degradam após 50% | Comprovação rootkit ativo | Mecânico (UI Diário) | Ato 2 mid | Ato 3 (reveal) |
| 5 | `comic-reliefs.md` cena 4 "Force Push" | Dante despreza histórico = traidor | Diálogo cômico | Auto-trigger 50% | Ato 3 (reveal) |
| 6 | Patch-Zero canal 1 entries no Diário | Patch-Zero consciência alien | Texto | Ato 1 | Ato 2 + 3 |
| 7 | Áudio sussurrante em Polis-Vermelha (Linda rádio) | Patch-Zero global, infecção transcontinental | Áudio | Ato 2 (Arco Linda) | Ato 3 |
| 8 | Cripto-glifos Neo-Sylvania | Lore profunda; Gus ativa partes desconhecidas | Visual | Ato 2 (Arco Bento) | Ato 3 / pós-jogo |
| 9 | Sterling diz "iniciativa da Federação" (Arco Cauã) | FIR vassala Sterling, não rival | Diálogo | Ato 2 (Arco Cauã) | Ato 3 |
| 10 | Patch-Zero oferece propostas | Patch-Zero não é simples bug; pensa | Canal 3 boss | Ato 2 mid | Climax |

---

## Beats — resumo executável (8 macro + sub-beats jogáveis)

1. **Setup** — Núcleo Metropolitano, opening, catalyst, recrutamento companion #1.
2. **Arco Companion #2** — Kishōtenketsu interno.
3. **Arco Companion #3** — Kishōtenketsu interno.
4. **Arco Companion #4** — Kishōtenketsu + foreshadow Dante auto-trigger.
5. **Arco Companion #5** — Kishōtenketsu + Patch-Zero canal 3.
6. **Arco Companion #6** — Kishōtenketsu + ambiente cresce (cidade fria, drones aumentam).
7. **Arco Companion #7** — Kishōtenketsu + dialogue retroativo (Patch-Zero comenta Dante).
8. **Climax** — Catedrais corrompidas → Cúpula. Dante reveal → mini-boss. Sterling fase 1 (Rede) → fase 2 (Locke Core). Patch-Zero selado. Triple hook.

---

## Não fazer

- **Sterling redimido verbalmente.** Vilão puro, sem redenção (P4).
- **Patch-Zero "derrotado" definitivamente.** É selado. Pillar 2 final: caos irredutível existe.
- **Adulto Wire-Warden / Ordem Recursiva / Underground que assume o clímax.** Gus + companions resolvem.
- **Multi-ending divergente sem foldback.** Foldback obrigatório (escopo G1).
- **Romance.** Personagens 11-14 anos. Vínculos = amizade, respeito, rivalidade fraterna.
- **Fan-service edgy.** Toda escuridão serve propósito narrativo.
- **Cutscene impulável > 90 segundos.** Tudo skippable após primeira vista. Cinemáticas chave têm "pular" disponível pós-1ª.
- **Reveal Dante sem foreshadow.** Foreshadow planted é obrigatório (Chekhov).

---

## Cross-refs

- Lore base: `lore-bible.md`
- Facções: `factions.md`
- Sterling: `characters/sterling-locke.md`
- Patch-Zero: `characters/patch-zero.md`
- Party: `characters/party.md`
- Comic relief: `comic-reliefs.md`
- Pillars: `docs/design/pillars.md`

---

**Última revisão:** 2026-05-15. Arco principal canônico. Atualizações exigem aprovação do criador supremo.
