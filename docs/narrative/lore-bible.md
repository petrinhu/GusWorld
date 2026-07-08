# Lore Bible: GusWorld

> **Status:** Revisão 1 concluída em sessão colaborativa com criador supremo (2026-05-15). Substitui versão prévia (que tinha Iolanda como antagonista) integralmente. **Canônico.**
>
> **Escopo:** RPG turn-based + puzzle + aventura, 4-8h gameplay, solo indie G1. Lore densa o suficiente pra sustentar o arco principal (8 beats macro, 6 arcos companion, climax 2 fases) e três variações de ending knowledge-gated. **Não** é enciclopédia AAA.
>
> **Imutáveis cross-ref:** `sinopse.md`, `docs/design/pillars.md`, `characters/prelore_vilao.md`, `Resources/gusworld/_INDEX.md`.
>
> **Inspiração / referência:** `docs/narrative/bibliografia-rag.md` (as 306 obras indexadas no RAG que inspiram esta lore; inspiração, não cópia).

---

## 1. Premissa do mundo

GusWorld é uma civilização **reconstruída sobre as ruínas de outra**. A história não começa com o mundo nascendo; começa com ele tentando lembrar o que esqueceu.

Há três eras estruturais. A primeira ruiu antes que alguém vivo se lembrasse. A segunda ergueu cidades sobre o que sobrou. A terceira é o agora, e está sendo lentamente esvaziada por uma empresa.

Três princípios atravessam as eras:

1. **Magia é sistema formal computável** (Pillar 2). Não há "força arcana"; há instruções, tipos, escopos, custos. Algumas instruções rodam em silício; outras em latão e mola; outras em proteína e seiva. Todas obedecem regras.
2. **Natureza é matemática rígida** (Pillar 2). Fauna e flora seguem sequências numéricas recorrentes, fractais, ruído coerente. O caos aparente é caos determinístico até a fronteira final.
3. **Inteligência prodigiosa serve à vida ou se vira contra ela** (Theme do arco). É escolha. Esta é a história que o jogo conta.

---

## 2. Theme central da lore

**A inteligência mais alta é a que serve à vida.**

Gus respeita o substrato: compila código que honra o hardware, lê a Selve sem capturá-la. Sterling consome o substrato: interpreta tudo dentro de uma máquina virtual proprietária, deletando o que não cabe.

A lore inteira é o palco desse contraste.

---

## 3. As três eras

### Era 1: Pré-Código (Neo-Sylvania)

A civilização anterior. Caiu. O nome que sobrou nos registros é **Neo-Sylvania** (cunhado pelos arqueólogos da Era 2; o nome original se perdeu).

**O que se sabe:**

- Construíram em pedra, vidro e biomassa programada. Catedrais altas, com vitrais que reagiam à temperatura e à hora.
- Não usavam silício. Sua "computação" rodava em substratos analógicos: relojoaria de precisão, redes hidráulicas, cristais ressonantes, fungos em malha.
- Tinham relação cooperativa com a Selve. Documentos sobreviventes mostram esquemas de irrigação que seguem os mesmos fractais que a Selve usa hoje espontaneamente.
- Não se sabe por que caíram. Hipóteses arqueológicas: catástrofe ecológica, conflito interno, ou tentativa fracassada de domesticar algo que se recusou a ser domesticado.

**O que sobrou visível:**

- **Catedrais de Neo-Sylvania** (setting 2): estrutura gótica arcaica em ruína, com mecanismos de latão ainda funcionando depois de séculos. A Ordem Recursiva guarda essas catedrais hoje. Bento aprendeu Asmódico (a linguagem analógica das catedrais) com mestres dessa Ordem.
- **Cripto-glifos** em pedras espalhadas pela Selve: matemática avançada gravada em rocha, decodificável com paciência.
- **Sementes-relíquia**: bancos de DNA preservados em câmaras herméticas dentro das catedrais. Jaci usa algumas dessas sementes na sua farmacopeia.

**Atitude in-world:**

A maioria dos cidadãos da Era 2 trata Neo-Sylvania como folclore: "lendas antes da cidade". A Ordem Recursiva trata como verdade venerável. Sterling Corp trata como **estoque arqueológico explorável** (já saqueou três catedrais menores antes do início do jogo).

### Era 2: Era do Compilador

A reconstrução. Há cerca de 150 anos antes do jogo, a civilização atual emergiu, não como continuação direta de Neo-Sylvania, mas como retomada parcial sobre os escombros.

**O que aconteceu:**

- Pioneiros descobriram que o substrato natural (a Selve, os mecanismos de Neo-Sylvania sobreviventes, os cripto-glifos) era **legível** se tratado como linguagem.
- A primeira geração de engenheiros desenvolveu **C-Arcane**: linguagem de baixo nível compilada para o substrato físico do mundo. C-Arcane respeita o hardware. Cada conjuro é otimizado para a arquitetura local.
- Outras linguagens surgiram em paralelo:
  - **Asmódico** (Bento): analógico, herdado das catedrais Neo-Sylvania, executado em relojoaria e ressonância acústica.
  - **Óxido** (Iara, Linda): linguagem de baixo nível voltada a segurança e precisão; quase ilegível para iniciantes mas indestrutível em runtime.
  - **Pythia** (Cauã, Jaci): linguagem de scripting rápido, interpretada, ideal para protótipos e bio-hacking. Compila menos, escreve mais.
- A **Federação Industrial de Reciclagem** (FIR) emergiu nesse período como esforço cooperativo de aproveitamento de hardware antigo. Vermelha de boa-fé no começo. Capturada por interesses corporativos depois.
- Cidades cresceram em torno dos sítios Neo-Sylvania melhor preservados. **GusWorld City** é a maior dessas cidades; outras três grandes existem nesta região do mundo (ver §10).
- Tom geral da Era 2: pragmatismo cooperativo, código aberto, conhecimento compartilhado. O **Tomo da Pilha Sobrecarregada** (referenciado em `comic-reliefs.md` cena 7) data dessa era: documentação coletiva de soluções, mantida por séculos.

**A Era 2 era estável.** Não perfeita, mas estável. O Sistema funcionava.

### Era 3: Era Sterling (presente do jogo)

Há cerca de 25 anos do início do jogo, **Sterling Locke** publicou sua tese de doutorado em Lógica Computacional. A tese argumentava que a compilação era "submissão ao hardware" e propunha o paradigma **DRE (Dynamic-Runtime Evaluation)**: uma máquina virtual universal interpretada, capaz de reescrever a sintaxe do mundo em runtime.

Ver `characters/prelore_vilao.md` para o dossiê integral.

Nos 25 anos seguintes, Sterling:

1. Foi rejeitado pela academia ortodoxa (defensora de C-Arcane).
2. Migrou para o setor privado.
3. Canibalizou três conglomerados (**Apex-Data Systems**, **Nexus-Cloud Conglomerate**, **Core-Synth Bio-Tech**), levando-os à falência calculada e extraindo seus ativos críticos.
4. Consolidou tudo em **Sterling Corp** (sede: Cúpula Sterling, ato 3).
5. Subordinou a FIR como vassala (ela ainda finge cooperativa; é cartel mafioso lavando ativos pra Sterling).
6. Lançou hardware de consumo agressivo: Janelarum (sistema operacional comercial frágil que vende serviços de manutenção; ver `comic-reliefs.md` cena 11), terminais públicos, infraestrutura privatizada de rede.
7. Iniciou a operação **GRE (Global Runtime Environment)**: envelopar a Selve Sombria em uma máquina virtual interpretada, transformando ecossistemas em variáveis editáveis por linha de comando.

A Era 3 está só na metade. O Sistema ainda funciona aparentemente. Mas a Selve já mostra sinais: anomalias inexplicáveis, fauna se comportando fora do padrão fractal, ruído estranho na rede.

**A maioria dos cidadãos não percebe.** O Janelarum trava, a água continua chegando, o trabalho continua. A erosão é silenciosa.

Gus percebe. Gus tem 11 anos.

---

## 4. Cosmovisão (pós-apocalipse digital, sem meta-simulação)

GusWorld **não é uma simulação**. Ninguém vive dentro de um computador. O mundo é físico, biológico, material.

O que aconteceu é mais simples e mais triste: uma civilização anterior caiu por motivos que ninguém mais pode reconstruir com certeza, e a civilização atual cresceu sobre os escombros, reaproveitando o que entendeu e ignorando o que não conseguiu decodificar. As Catedrais Neo-Sylvania não são "templos sagrados": são prédios velhos, lindos, parcialmente funcionais. A Selve Sombria não é um reino encantado: é o ecossistema da Era 1 sobrevivendo orgânico, e seguindo a matemática que ninguém ensinou pra ela mas que ela sempre soube.

Cidades como GusWorld City são reconstruções tecnológicas sobre ruínas vivas. Quem cava fundo demais encontra coisas funcionando ainda.

**Implicação narrativa:** o passado importa. Não como mito; como infraestrutura subjacente. Gus, sem saber inicialmente, está reativando partes do legado Neo-Sylvania ao usar C-Arcane no substrato certo. Sterling, sem saber também, está repetindo o erro que possivelmente derrubou Neo-Sylvania: tentar domesticar o que pede cooperação.

---

## 5. Geografia: GusWorld City (megacidade ciber-gótica)

Hub central persistente. Visualmente evolui conforme o jogo progride (Pillar 5): no início, neon vibrante e movimento; conforme Sterling expande controle, frio, drones aumentam, NPCs somem.

### 5.1 Distritos urbanos (5)

| # | Distrito | Identidade | Tom dominante | Companion-âncora |
|---|---|---|---|---|
| 1 | **Núcleo Metropolitano** | Hub central, circuito impresso 3D, neon ciano, grid ortogonal. Onde Gus mora e onde a party se reúne. | Otimista degradante | Gus |
| 2 | **Periferia Industrial (Ferrovelhos)** | Sucata, hardware descartado, oficinas independentes. Lar de Dante. Onde a FIR opera abertamente. | Cinza-fuligem, fagulhas | Dante |
| 3 | **Setor Mirage** | Distrito de entretenimento. Holografia sobreposta densa, refração instável, publicidade Sterling Corp em todo poste. Sede do Cult Mirage. | Saturado, vertiginoso | Iara |
| 4 | **Zona do Silêncio** | Antigo distrito de transmissão analógica. Antenas de rádio mortas, ruído branco residual, ecos longos. Underground do Silêncio se esconde aqui. | Tons frios, vazios | Linda |
| 5 | **Anel Verde** | Última fronteira urbana antes da Selve. Estações botânicas, checkpoint federal corrompido. Anomalias começam a vazar aqui. | Concreto musgado, biolúmen | trânsito |

Os Dutos Infernais (setting Cauã, Pillar 5) **não são um distrito de superfície**. São a rede subterrânea reconfigurada que conecta os cinco distritos. Catacumbas tecnológicas, plasma luminescente, arcos voltaicos. Quem domina os Dutos navega por baixo de toda a cidade.

### 5.2 Catedrais de Silício + Cúpula Sterling (ato 3)

Sede de Sterling Corp. Visualmente é o mesmo setting com duas zonas opostas:

- **Exterior (Catedrais de Silício corrompidas):** torres híbridas onde Sterling tentou fundir arquitetura cidade + biomassa Selve. Falhou. Resultado: estrutura morta-viva, mistura repulsiva e tecnicamente fascinante. Cresce como tumor entre o Núcleo Metropolitano e o Anel Verde.
- **Interior (Cúpula Sterling):** santuário euclidiano estéril. Geometria perfeita, branco-cromo, sem qualquer traço orgânico. Espelhos. Servidores silenciosos. É onde Sterling vive. É onde o boss final acontece (2 fases: Rede Distribuída → Locke Core).

---

## 6. Geografia: Selve Sombria (fronteira tecnorgânica)

Ecossistema sobrevivente da Era 1. Auto-organizado em padrões matemáticos. Hostil a quem ignora as regras; legível para quem aprende.

### 6.1 Regiões (3)

| # | Região | Padrão matemático dominante | Bioma | Função no arco |
|---|---|---|---|---|
| 1 | **Orla Recursiva** | L-systems, ramificação fractal | Floresta densa, biolúmen verde-ciano, copa cerrada | Tutorial / primeira incursão |
| 2 | **Pântano de Markov** | Cadeias estocásticas, transições probabilísticas | Brejo, névoa, criaturas que mudam estado | Meio-jogo, primeiros bosses-vírus |
| 3 | **Núcleo Mandelbrot** | Fractais aninhados infinitos | Clareira geometricamente impossível, paisagem não-euclidiana suave | Climax-âncora do Patch-Zero |

Jaci nasceu numa aldeia da fronteira-Selve (vilarejo do Pelicano Branco, EE-14 em `comic-reliefs.md`). Ela navega esses três terrenos como quem caminha em casa.

### 6.2 Ecologia algorítmica (regras imutáveis)

1. **Toda criatura roda um script.** Comportamento previsível se você lê o padrão.
2. **Bug = anomalia visível.** Glitch visual, ruído, geometria quebrada são sinais diagnósticos, não decoração.
3. **Vírus se propaga por contato com runtime hospedeiro.** Estagiar a contenção é mais barato que purgar tarde.
4. **Bioluminescência é log.** Cor e pulsação codificam estado do organismo. Gus lê com óculos + matriz; outros aprendem com tempo.
5. **A Selve não é hostil, e sim um sistema sob ataque.** Inimigos verdadeiros são os que injetam vírus.

### 6.3 Catedrais Neo-Sylvania dentro da Selve

Pelo menos cinco catedrais Neo-Sylvania sobreviventes estão dentro da Selve Sombria, não na cidade. Duas estão dominadas pela Ordem Recursiva (a Ordem de Bento controla acesso). Uma foi saqueada por Sterling Corp três anos antes do jogo (vazio agora; expedição secundária revisita as ruínas). Duas continuam perdidas: ninguém vivo sabe a localização exata; os cripto-glifos sugerem coordenadas mas codificadas.

---

## 7. C-Arcane e as linguagens irmãs (sistema de magia)

### 7.1 Conceito

Magia em GusWorld é **sistema formal computável**. Toda magia obedece a:

- **Linguagem**: gramática que define o que é um conjuro válido.
- **Compilador ou interpretador**: máquina (silício, latão, biológica) que transforma a linguagem em efeito físico.
- **Hardware**: substrato físico onde o efeito acontece (Tavus-Drive, relógio mecânico, ampola química, fone industrial).
- **Custo**: energia, tempo, recurso. Conjuro grátis não existe.

### 7.2 As cinco famílias técnico-naturais

Cada companion é especialista em uma família. Gus mistura todas (alta complexidade combinatória; é o que o torna prodígio).

| Família | Efeito-tipo | Companion-âncora |
|---|---|---|
| **Elétrico** | Pulso EM, corrente, indução, descarga | Cauã |
| **Bioquímico** | Antídoto, mutação temporária, bio-script | Jaci |
| **Sônico** | Onda infra/ultra, ressonância destrutiva, atordoamento | Linda |
| **Cinético** | Força aplicada, gravidade, vetor de massa | Bento (analógico) |
| **Criptográfico** | Cifragem, ofuscamento, ilusão, refração | Iara |

Dante é **exceção Pillar 3 invertida (meta-hardware)**: NÃO tem vértice próprio do triângulo, não opera em família elétrica nem cinética isolada. Opera **root nos vértices dos outros**, sua "magia" é instalar componentes que rodam conjuros pré-existentes (torres, escudos modulares, manutenção de implantes alheios). Paralelo embrionário do **4º elemento Sterling** (Locke Core + Rede Distribuída): Dante é mini-Sterling em formação, foreshadow estrutural da traição (D.1 rootkit). Mecanicamente útil em party; narrativamente significativo como personagem que manipula hardware alheio. Anti-Pillar 3 invertida confirma o triângulo dos outros companions por contraste.

### 7.3 Subtipos elementais

Cada família tem subtipos. Exemplos:

- **Elétrico** → Plasma / Indução / Eletroestática / Magnetismo / Refrigeração (criotermia inversa).
- **Bioquímico** → Polímero / Antiviral / Endorfínico / Mutagênico controlado.
- **Sônico** → Infrassom / Ultrassom / Eco / Ruído branco.
- **Cinético** → Gravitacional / Inercial / Compressivo / Rotacional.
- **Criptográfico** → Refração / Hash / Esteganografia / Decoy.

### 7.4 Modificadores transversais (tipos de dado)

Modificadores aplicam-se a qualquer família. Estes são os seis tipos de dado canônicos:

| Tipo | Efeito |
|---|---|
| **Bool** | Sim/não. Polaridade. Liga/desliga. Custo baixo, efeito binário. |
| **Int** | Magnitude numérica. Quantifica intensidade. |
| **String** | Texto, identidade, nome. Permite mirar individualmente. |
| **Object** | Coisa completa. Conjuros que afetam alvo inteiro (não só parte). |
| **Stream** | Fluxo contínuo. Conjuros que sustentam efeito por turnos. |
| **Null** | Negação, ausência. Anula outro efeito. Conjuro de contra-conjuro. |

### 7.5 Sintaxe: stack 3 slots

Cada conjuro = combinação de **três tokens** carregados no Tavus-Drive:

```
[FAMÍLIA] + [MODIFICADOR de TIPO] + [ALVO]
```

Exemplos:

- `Elétrico + Int + Inimigo` → descarga calibrada (dano numérico).
- `Sônico + Stream + Área` → ruído sustentado (atordoamento em área, vários turnos).
- `Criptográfico + Bool + Aliado` → cifragem on/off (aliado fica invisível ao próximo scan inimigo).
- `Bioquímico + Null + Aliado` → cura por anulação de status (remove veneno, debuff).
- `Cinético + Object + Inimigo` → empurra inimigo inteiro (knockback).

A ordem importa. Famílias diferentes na mesma stack produzem **conjuros híbridos** (poderosos, mais difíceis de compilar). Tokens incompatíveis falham e gastam 1 ciclo do Tavus-Drive sem efeito.

Combater por scripts é confrontar processos em execução. Cada combatente sustenta, sobre o hardware frágil que é o corpo, um sistema vivo: a alma como núcleo que orquestra, a mente como camada que interpreta, e os Conjuros como rotinas disparadas em sequência contra o estado adversário. Vencer um duelo não é incendiar a máquina alheia: é forçar o processo dela ao colapso de pilha, conduzi-lo ao ponto em que nenhuma instrução seguinte pode resolver, em que a execução trava sobre si mesma e suspende. O corpo permanece intacto, pedra ainda não talhada à espera de retomada; só a corrente de comandos cessa. Destruir o hardware de modo permanente jamais foi considerado triunfo entre os que compreendem o ofício: é vandalismo de sistema, falha grosseira, o oposto exato da maestria.

Por isso o derrotado não morre: adormece em estado suspenso. Estabelecida a dominância, o processo vencido reconhece a posição irrecuperável e cede, exatamente como um enxadrista tomba o rei diante do mate inevitável: o rei nunca é removido do tabuleiro, apenas admite que toda continuação leva ao mesmo desfecho. As três camadas do vencido permanecem íntegras (núcleo, interpretador, rotinas dormentes), congeladas até que outra mão reative a execução. Provou-se domínio de estado, não aniquilação. O adversário cede, e o silêncio que se segue não é o da máquina quebrada, mas o da máquina compilada e posta em pausa, à espera do próximo ciclo que a devolverá, inteira, ao mundo.

### 7.6 Codex de Conjuros (mecânica-âncora #2)

O **Codex de Conjuros** é o deck pessoal. **Compilação do Codex** é o ato de combinar tokens disponíveis para gerar conjuros executáveis. Substitui a antiga "Compilação de Deck Rúnico" por terminologia mais limpa (sem "runa"; ver §7.10).

Escopo G1:

- **40-60 tokens** totais no jogo (não conjuros, mas tokens, que combinam). Combinatoriamente, ~200 conjuros pré-planejados surgem.
- **15 tokens em campo** (deck ativo). Trocáveis fora de combate.
- **Mestria por uso**: token cresce com repetição (Pillar 1).
- **5-10 combos secretos**: combinações não-óbvias que destravam efeitos surpresa.

### 7.7 Biblioteca de conjuros: amostra-template

Não vamos listar os 200 aqui. Apenas amostra para fixar o tom:

| Conjuro | Composição | Efeito |
|---|---|---|
| **Armadilha Sináptica** | Elétrico + Stream + Área | Zona elétrica persistente por 3 turnos |
| **Mantra do Silêncio** | Sônico + Null + Inimigo | Silencia conjuros do alvo por 2 turnos |
| **Refração Aliada** | Criptográfico + Bool + Aliado | Aliado fica "fora do scan" 1 turno |
| **Bio-Sutura Rápida** | Bioquímico + Int + Aliado | Cura proporcional ao Int investido |
| **Vetor de Recuo** | Cinético + Object + Inimigo | Empurra inimigo 1 casa |
| **Decoy Lumen** | Criptográfico + Stream + Área | Clones holográficos por N turnos (Iara signature) |
| **Pulso EM Concêntrico** | Elétrico + Stream + Área | Atordoa hardware inimigo (Cauã signature) |
| **Eco do Cânion** | Sônico + Stream + Área | Linda signature, dano em onda |
| **Cronômetro Ressonante** | Cinético + Stream + Aliado | Bento Asmódico: amplia defesa party (analógico) |
| **Antídoto Sintético** | Bioquímico + Null + Aliado | Jaci signature, remove status hostil |

### 7.8 Combos secretos (hint)

Combos não óbvios. Total: 5-10 destraváveis. Exemplos sem revelação completa:

- **Compilação Reversa** (3 tokens específicos em ordem invertida da padrão) → restaura HP de aliado proporcional ao dano que ele sofreu naquela cena.
- **Recursão de Eco** → conjuro Sônico que persiste 1 turno extra a cada vez que é refletido por superfície dura no setting (mecânica diegética: posicionamento conta).
- **Null x Null** → contra-conjuro perfeito; anula totalmente o próximo conjuro inimigo, com custo alto.
- Outros documentados no Diário do Gus conforme jogador descobre.

### 7.9 Asmódico: exceção declarada (Bento)

Asmódico é **a linguagem analógica de Neo-Sylvania**. Roda em relojoaria de latão, ressonância acústica, ancoragem mecânica. Bento herdou da Ordem Recursiva (Catedrais de Neo-Sylvania, setting 2, Pillar 5).

**Diferença mecânica:**

- Não usa Tavus-Drive nem tokens digitais.
- Tem **slots fixos**: 5-6 habilidades em rotação mecânica, ativadas por engrenagens do escudo-catedral.
- Cada habilidade tem ciclo próprio (cronômetro físico no peitoral; ver spec Bento).
- Não é flexível como C-Arcane. É **previsível, lento e firme**.

**Justificativa de pillar:** relógio mecânico é state machine, só substrato diferente. Asmódico segue Pillar 2 ("sistema formal computável"); apenas opera em latão e mola em vez de silício. Drama interno party: Asmódico é tese rival ao C-Arcane do Gus. Atrito narrativo (ver `comic-reliefs.md` cena 2, "Tabulações vs Espaços" e cena 8, "Comentários em Latim").

### 7.10 Glossário terminológico (sem "runa")

| Termo | Significado |
|---|---|
| **Glyph** | Elemento visual desenhado na carta-token. Identidade gráfica. |
| **Token** | Elemento de código/dado. A peça em si que vai pro Tavus-Drive. |
| **Conjuro** | Resultado executável. 3 tokens compilados = 1 conjuro. |
| **Codex de Conjuros** | Deck pessoal de tokens disponíveis. |
| **Compilação do Codex** | Ato de combinar tokens para gerar conjuro em combate. Mecânica-âncora #2. |
| **C-Arcane** | Linguagem digital de baixo nível. Padrão da Era 2. Idioma de Gus. |
| **Asmódico** | Linguagem analógica herdada de Neo-Sylvania. Idioma de Bento. |
| **Óxido** | Linguagem de baixo nível voltada a segurança. Idioma de Iara e Linda. |
| **Pythia** | Linguagem de scripting interpretada. Idioma de Cauã e Jaci. |
| **DRE** | Dynamic-Runtime Evaluation. Paradigma de Sterling. Anti-tese de C-Arcane. |
| **GRE** | Global Runtime Environment. Plano de Sterling. Máquina virtual universal. |
| **Tavus-Drive** | Executor pessoal de tokens. Implante de pulso do Gus. |
| **Matriz Ortodôntica** | Antena bio-integrada do Gus. Amplificador de alcance. |

---

## 8. Patch-Zero: natureza, origem, manifestação

### 8.1 O que é

Patch-Zero é **dois fenômenos ao mesmo tempo**, indistinguíveis entre si até o late game:

1. **Anti-padrão**: rompe a previsibilidade da Selve. Onde Patch-Zero passa, as sequências numéricas recorrentes falham localmente, ruído coerente vira ruído genuíno, Knowledge Progression (Pillar 1) é **invalidada para inimigos contaminados** (não dá pra prever o próximo movimento; o óculos tático mostra estática).
2. **Consciência alien**: tem voz. Planeja. Manipula. Não é simplesmente bug; é entidade pensante operando através do bug.

A coexistência dos dois aspectos é deliberada. Patch-Zero não é "monstro" nem "vírus"; é **o limite do conhecimento** se manifestando como ameaça.

### 8.2 Origem multi-causal

Patch-Zero não foi criado por uma fonte só. Quatro vetores convergiram:

1. **Bug primordial do Sistema**: emergiu na Selve Sombria sozinho, antes de Sterling. Era pequeno, contido em uma região do Núcleo Mandelbrot Interno. Pré-existente.
2. **Captura corporativa**: Sterling Corp localizou o bug e **tentou domesticar** em laboratório. Quis transformar em arma proprietária (vazado em redes inimigas, derruba concorrência). Falhou em conter.
3. **Amostra importada**: laboratórios Sterling Corp em uma cidade-irmã (ver §10) capturaram amostra adicional e enviaram para a sede. Cross-contamination.
4. **Escape**: Patch-Zero escapou do laboratório central três meses antes do jogo. Agora se propaga sozinho. Foge do controle de Sterling e do controle de qualquer um.

### 8.3 Manifestação multi-canal

Patch-Zero não aparece como inimigo único. Aparece **simultaneamente em quatro canais diegéticos**:

#### Canal 1: Texto no Diário do Gus

Entries surgem **sem que o jogador as escreva**. Pseudocódigo corrompido, com glitches tipográficos:

```
[ENTRY 0x????]
oi gus
oi vector
não sou seu inimigo. ainda.
[ERRO: parse falhou na linha ?]
```

Frequência: 1-3 entries por arco companion. Aparecem em capítulos onde a infecção da Selve está mais ativa.

#### Canal 2: Áudio ambient

Zonas infectadas têm sussurros baixos misturados ao soundscape. Não são vozes claras: são padrões reconhecíveis se você ouve com atenção. Áudio diegético (gravado, não trilha sonora). Sterling Corp não consegue limpar o ruído.

#### Canal 3: Persona dialogável em boss arenas

Em três encontros chave (mid-ato 2, final ato 2, mid-ato 3), Patch-Zero **fala diretamente com Gus** via terminal projetado, espelho rachado, ou superfície reflexiva qualquer. Oferece propostas:

- Trégua falsa ("eu te ajudo a derrotar Sterling se você me liberta na rede").
- Provocação filosófica ("você é igual a ele, gus. ambos otimizam. apenas em direções opostas.").
- Negociação aparente ("eu posso te dar acesso ao núcleo mandelbrot. basta deixar uma janela aberta.").

Gus pode aceitar, recusar, negociar. **Todas as opções têm consequência mas nenhuma é "boa"**: Patch-Zero mente sobre uma porcentagem variável das próprias afirmações.

#### Canal 4: Bug visual ambient

Em zonas infectadas, o shader Perlin que rege flora/fauna **glitcha visualmente**: descontinuidades, faixas de gradiente quebradas, mesh popping. Não é bug do jogo, e sim bug **do mundo**. Reforça lore-via-imagem (mostre não conte).

### 8.4 Relação com Sterling

Sterling **liberou Patch-Zero achando que controlava**. Quando percebeu que não controla, tentou contenção massiva. Falhou. Agora finge publicamente que Patch-Zero não existe ("comportamentos emergentes inesperados"; ver `comic-reliefs.md` cena 6).

Patch-Zero **odeia** Sterling especificamente. Sterling tentou capturar e domesticar; Patch-Zero não esquece. Em diálogos com Gus, Patch-Zero refere a Sterling como "o que tentou nos enjaular". Plural deliberado.

### 8.5 Destino narrativo

Patch-Zero é **selado, não destruído**. No clímax, Gus consegue contenção parcial (Pillar 2, fronteira final = caos irredutível). Sterling cai. Patch-Zero adormece, mas continua existindo. Hook sequel: stinger pós-créditos (B.8) mostra célula pulsando em laboratório distante. Diário entry final: "Sinal anômalo detectado em [cidade-irmã]."

Detalhes técnicos completos em `docs/narrative/characters/patch-zero.md`.

---

## 9. Cidades-irmãs (rede internacional)

GusWorld City não é única. Esta região do mundo tem **quatro grandes cidades urbanizadas** na Era 2, conectadas por linhas de transmissão e por uma rede comercial. Cada uma tem ramificação Sterling Corp.

### 9.1 Tabela das cidades-irmãs

| Cidade | Status Patch-Zero | Ramificação Sterling | Relação com GusWorld | Mencionada onde |
|---|---|---|---|---|
| **GusWorld City** | Infecção crescente, ainda contida | Sede global da Sterling Corp | Onde Gus mora; epicentro do arco | Todo o jogo |
| **Polis-Vermelha** | **Infectada off-screen** há 2 anos. Quarentena Sterling. | Laboratório regional. Foi de lá que veio a amostra importada (§8.2 vetor 3). | Cidade-irmã caída. Avisos cifrados chegam por antenas. | Diário entries, terminais Setor Mirage |
| **Cidades-Gêmeas (Norte/Sul)** | Saudável aparentemente. Underground forte. | Escritório administrativo. Resistência local infiltrada. | Aliada potencial. Underground do Silêncio mantém contato. | Underground (Linda), late ato 2 |
| **Heliópolis-Nova** | Hostil. Submissa a Sterling há mais tempo. | Núcleo de mineração de hardware. Quase colônia. | Distante; pouca informação confiável. | Lore secundário, sidequest opcional |

### 9.2 Como aparecem no jogo

Não visitáveis (escopo G1). Mencionadas via:

- **Diário do Gus** (entries de lore destraváveis por Knowledge Progression).
- **Terminais públicos** com notícias filtradas.
- **Underground do Silêncio** (Linda traz informações de rádio analógica de longa distância).
- **Patch-Zero** referindo-se a si mesmo como múltiplo ("nós em Polis-Vermelha já são livres").
- **Stinger pós-créditos** (Diário entry final menciona sinal em cidade-irmã não nomeada).

Polis-Vermelha caída é **lição visível**: o que aconteceu lá pode acontecer aqui se Sterling vencer. Tom da menção: melancólico, contido, sem catastrofismo.

---

## 10. Facções (resumo; detalhes em `factions.md`)

Seis facções paralelas operam em GusWorld City e adjacências. Pirâmide de poder no topo: Sterling Corp.

| # | Facção | Postura vs Gus | Companion |
|---|---|---|---|
| 1 | **Sterling Corp** | Antagonista absoluta | (nenhum) |
| 2 | **Ordem Recursiva** | Aliada cautelosa, tradicionalista | Bento |
| 3 | **Federação Industrial de Reciclagem (FIR)** | Hostil disfarçada (vassala Sterling) | Dante (ex-funcionário; contato corrompido) |
| 4 | **Selve Sombria** (facção natural) | Aliada se respeitada | Jaci |
| 5 | **Cult Mirage** | Ambígua, manipuladora; alguns aliados internos | Iara (desertora) |
| 6 | **Underground do Silêncio** | Aliada clandestina | Linda |

Detalhes ideológicos, estruturais, simbólicos em `docs/narrative/factions.md`.

---

## 11. Cultura, costumes, vida cotidiana

### 11.1 Vida diária em GusWorld City

Trabalho começa cedo. Lojas físicas dividem espaço com terminais Sterling. Crianças vão a escolas que ensinam **C-Arcane básico** desde os 7 anos (currículo público, ainda da Era 2; Sterling tentou substituir por DRE, encontrou resistência docente; a escola pública resistiu até agora). Família de Gus mora num apartamento modesto no Núcleo Metropolitano.

### 11.2 Calendário e festividades

- **Festa da Compilação** (anual, primavera): comemoração da primeira linha C-Arcane compilada com sucesso há 150 anos. Sterling Corp comprou o evento e usa pra propaganda.
- **Dia do Tomo** (verão): homenagem coletiva à Pilha Sobrecarregada. Pessoas trocam soluções técnicas em praça pública. Mantida pela comunidade.
- **Vigília Neo-Sylvania** (outono, no equinócio): Ordem Recursiva abre as catedrais ao público uma noite por ano. Multidões silenciosas observam relojoaria antiga funcionar.

### 11.3 Música e arte

Pós-punk eletrônico domina a cidade. Underground do Silêncio cultiva rádio analógica como resistência. Cult Mirage produz pop hipersaturado para massa. Ordem Recursiva preserva canto coral de tradição Neo-Sylvania (raro, ouvido só em vigílias).

### 11.4 Economia

Moeda corrente: **crédito** (digital, gerido por Sterling Corp; rastreável). Underground usa **token-rádio** (analógico, anônimo). Companions usam o que tem; Gus prefere crédito para invisibilidade civil, troca pra token-rádio quando vai em zonas underground.

Hospital cobra crédito por cura rápida (Pillar 4 / hospital + economia). Cura gratuita demora dias de jogo; cura paga é instantânea. Cria pressão econômica natural.

### 11.5 Tabus e honra

- **Histórico apagado é tabu** entre tradicionalistas (Ordem Recursiva, Underground): "histórico é como a gente sabe pra onde a coisa vai" (citação de Gus em `comic-reliefs.md` cena 4; foreshadow Dante).
- **Honrar o substrato** é virtude técnica: bom programador respeita o hardware. Mau programador (paradigma Sterling) trata hardware como descartável.
- **Quebrar contrato com a Selve** (entrar sem respeito, extrair sem dar) é desonra entre pesquisadores éticos. Sterling Corp violou esse contrato sistematicamente.

---

## 12. Anti-pillars cumpridos (checklist de lore)

| Anti-pillar | Como esta lore evita |
|---|---|
| Magia misteriosa sem regra | Tudo tem família + modificador + custo |
| Selve como floresta encantada | Selve = sistema matemático; mística é resíduo de Neo-Sylvania documentado |
| Antagonista sem ideologia | Sterling tem tese filosófica clara (DRE vs C-Arcane) |
| Multi-ending solto | 1 ending canônico + 3 variações knowledge-gated (Bronze/Prata/Ouro) |
| Adulto mentor que resolve | Adultos são antagonistas ou ausentes; Gus + companions peers resolvem |
| Bug = decoração | Bug é diagnóstico; jogador aprende a ler |
| Lore dump | Lore distribuída entre Diário, ambient, NPCs ocasionais; nunca monólogo expositivo |

---

## 13. Eventos históricos cross-eras (referência ampliada)

> **Referência primária:** [[timeline]] documenta cronologia detalhada com 50+ eventos datados.
>
> Esta seção sumariza os **eventos com peso narrativo direto no arco**, vinculados a personagens jogáveis ou facções centrais.

### Era 1 (Pré-Código)

- **~-820**: *Anomalia Primeira* registrada em cripto-glifo, "o limite que escolheu falar". Hipótese arqueológica: ancestral de [[patch-zero]]. Cripto-glifo lido por [[gus]] + [[bento-requiem]] no arco Bento. Sterling rejeita publicamente; Ordem Recursiva considera credível.
- **~-800**: Coletivo de mestres-engenheiros Neo-Sylvania tenta domesticar a anomalia. Precedente direto do fracasso Sterling. **História se repete.**
- **~-750**: Queda parcial Neo-Sylvania. Catedral-Mãe lacrada por dentro. Coordenadas preservadas em cripto-glifo (descobrível apenas em ending Ouro; ver [[in-world-docs]] doc 15).
- **~-720**: Êxodo. Aldeias-fronteira atuais (vilarejo do Pelicano Branco inclusive) descendem desses sobreviventes.

### Era 2 (Compilador)

- **-150**: Primeira linha C-Arcane compilada. Festa da Compilação anual ([[tradicoes-cultura]] §1).
- **-115**: Asmódico codificado formalmente. **Mestre Atelaiá Chevalier** (ancestral direto Bento, 5 gerações antes) é codificador-mãe.
- **-110**: Óxido desenvolvido. **Engenheira-mãe Tamara Neumann** (ancestral materna direta Linda).
- **-100**: Tomo da Pilha Sobrecarregada nasce. Documentação coletiva canônica.
- **-95**: Pythia desenvolvida em parceria **Iremar Berenger + Anhuera Vanderbist** (ancestrais de Cauã e Jaci).
- **-80**: *Tragédia da Catedral Menor de São Camilo.* 17 mestres morrem em uma noite por contaminação anomalia. Atelaiá Chevalier registra em diário pessoal (preservado até hoje; ver [[in-world-docs]] doc 3). Protocolo "Selagem Asmódica" criado em resposta. Trauma fundador Ordem Recursiva.
- **-50**: Federação Industrial de Reciclagem (FIR) fundada como cooperativa de boa-fé.
- **-45**: Vilarejo do Pelicano Branco formalizado. **Anciã Soraia Vanderbist** (bisavó Jaci) lidera.
- **-45**: Tratado anônimo *Sobre o Comportamento Emergente dos Padrões Fechados* publicado em rede acadêmica clandestina. Atribuído hipoteticamente à **Verônica Atelaiá** (descendente Atelaiá Chevalier; última grande pensadora pré-Sterling). Teorizou Patch-Zero 42 anos antes da localização Sterling em -3 (ver [[in-world-docs]] doc 10). Ignorada na época.
- **-40**: Underground do Silêncio surge informalmente.
- **-35**: Cult Mirage surge como movimento artístico genuíno. Co-fundadora **Sonja Murmúrio** (mãe biológica da futura hierofante manipuladora Adila).
- **-34**: **Sonja Murmúrio morre** em circunstâncias nunca esclarecidas. Adila Murmúrio assume formalmente liderança facção pró-Sterling do Cult Mirage no mesmo ano sob tutela transitória; facção dual artista + pró-Sterling consolidada desde então. Versão pública oficial Adila: "afastamento por saúde, retiro contemplativo".

### Era 3 (Sterling)

- **-25**: Sterling Locke publica tese DRE. Rejeitado pela academia. Ponto de inflexão.
- **-19**: Apex-Data Systems entra em Chapter 11. **Inácia Berenger** (mãe Cauã) demitida; trauma família.
- **-16**: Audit interno Apex-Data vazado por engenheiro anônimo (provavelmente **Bartolo Penkin**, desaparecido em -12; ver [[in-world-docs]] doc 5). Underground preserva.
- **-15**: Nexus-Cloud colapsa.
- **-5**: *Acidente da Subestação 7.* **Davi Berenger** (16, irmão Cauã) morre. Causa oficial: negligência. Causa real (ending Ouro): alvo deliberado por documentar irregularidades FIR. Bilhete final preservado por Inácia ([[in-world-docs]] doc 13). Cauã tinha 8 anos.
- **-12**: Core-Synth liquidada. Sterling consolida Sterling Corp. Construção da Cúpula começa.
- **-10**: Operação GRE iniciada (projeto interno classificado "Projeto Continente").
- **-8**: Cooperativa familiar **Alencar destruída** por engenharia financeira FIR-Sterling. **Salviano Alencar** (pai Dante) morre 6 meses depois. Wound canônico Dante.
- **-8**: *Batida na casa Neumann.* Toca-discos histórico apreendido. Linda (4) vê. **Otmar Neumann** salva agulha intacta antes da apreensão; entrega à filha. Pingente Linda.
- **-8**: *Surto silencioso no Pelicano Branco.* **Lia e Solano Vanderbist** (pais Jaci) morrem. Causa oficial: anomalia botânica. Causa real (ending Ouro): vetor experimental Sterling. Mariana suspeita; nunca prova.
- **-7**: *Contaminação da Catedral Menor de Atelaiá.* Mestre-Aprendiz **Hilário Tepenkov** (companheiro estudos Bento) morre. Bento (7) presente; sobrevive por estar perto do escudo Velhusto. Patch-Zero pré-existente confirmado.
- **-6**: **Adila Murmúrio** consolida tutela administrativa plena Cult Mirage pró-Sterling (passa de Hierofante nominal a operacional plena). Sonja Murmúrio, morta em -34 aos 33 anos (canon timeline:86 + CHARS:82 + factions:505), deixa o vínculo materno operando há vinte e oito anos como ausência calibrada.
- **-5**: **Dante (8 anos) recrutado por Diretor Cassiano Vorto da FIR.** Aliciamento canônico começa.
- **-3**: Sterling Corp localiza bug primordial Patch-Zero. Tentativa de captura. Falha parcial. **Catedral Menor de São Vargas saqueada** por Sterling Corp. Mestres **Ardenia Falke** e **Cândido Rui** desaparecem. Wound institucional Ordem Recursiva.
- **-2**: Cross-contamination via amostra Polis-Vermelha. **Polis-Vermelha cai.** **Padrinho Tiago** sai da aposentadoria.
- **-1**: **Iara deserta do Cult Mirage** aos 11 anos.
- **-0.5**: **Aldebrando Chevalier (pai Bento) morre** na Catedral Menor de Atelaiá, durante Vigília restrita de doze mestres: interferência ressonante calibrada na câmara central (onze mestres sobrevivem; Aldebrando não). Eliminação atribuída a contrato Sterling Corp não verbalizado em fólio público. Diário pessoal de Aldebrando preservado por Velhusto, entregue a Bento no início do jogo.
- **-0.25 (3 meses antes)**: **Patch-Zero escapa do laboratório central Sterling.** Catalyst macro.

---

## 14. NPCs lore-importantes (não-jogáveis, nomeados)

Personagens secundários canônicos com peso narrativo no arco principal ou sidequests. Detalhes operacionais e relacionais em [[factions]] e [[characters]].

### Aliados e mentores

- **Anciã Mariana Vanderbist** (89), avó da Jaci. Líder informal Pelicano Branco. Lê Selve. Mentora cautelosa (não toma protagonismo).
- **Mestre-Hierofante Velhusto** (70), líder Ordem Recursiva. Mentor distante Bento. Apoia tradicionalistas contra Sterling. Perdeu 3 aliados próximos no processo.
- **Padrinho Tiago** (55), coordenador atual Underground em GusWorld City. Mentor distante Linda. Engenheiro telecom aposentado que voltou ao serviço.
- **Inácia Berenger** (45), mãe Cauã. Técnica QA ex-Apex-Data. Vive contida. Sabe mais do que conta. Guarda documentos comprometedores de Davi.
- **Gargi Vance** (43), mãe Gus. Técnica de bancada eletrônica. Trabalha em casa. Safe-base canônica Gus. Personagem âncora emocional, ofuscada deliberadamente (não toma protagonismo).
- **Pyotor Vance** (46), pai Gus. Médico-cyber itinerante (medicina + bioengenharia + bioeletrônica + eng robótica + implantes) em rotação fronteira-Selve atendendo vilarejos pós-Êxodo (Pelicano Branco, Garça-Preta-Nova, Caracará-Cinza, Sabiá-de-Bronze). Hobby: programação. Separado amigavelmente de Gargi desde Gus aos 6 anos (ano -5). Cartas raras (3-4 no jogo total).
- **Yakov Vance** (42), tio paterno do Gus, irmão mais novo de Pyotor (4 anos a menos). Um dos melhores engenheiros de software do reino, também geólogo; combina ambas formações na maior mineradora do reino. Stack canon de prospecção subterrânea não-invasiva em 5 tecnologias: (a) radar GPR de penetração subterrânea, (b) sísmica de reflexão por explosões controladas, (c) tomografia muônica passiva por raios cósmicos, (d) magnetotelúrica de condutividade por fluxo iônico, (e) espectrometria microbiana bioluminescente de sub-superfície. Inovação canon registrada: 89% de redução de mortalidade em prospecção subterrânea via fusão multi-sensor. Apresentará xadrez ao Gus em visita esporádica anos depois (complementar à programação de Pyotor).
- **Atelaiana de Sevra Chevalier**, mãe biológica viva do Bento, irmã consanguínea de Mestre Lavínia Sevra. Partiu da Catedral três meses após o parto (canon R4 deep-lore).
- **Irmã biológica do Bento** (nome intencionalmente não-escrito; canon R9 conto 4 antologia "minha irmã, pulso clavícula esquerda, nome não-escrito"). Morta ou desaparecida antes de -7. Wound primeiro Bento, antecede Hilário Tepenkov. Pulso na clavícula esquerda como marca canônica. Detalhes a definir em sequel.
- **Edilma Alencar** (44), mãe Dante. Operária têxtil. **Aparência pública canon:** vive reclusa em apartamento subsidiado pós-morte de Salviano, mantido como fachada Sterling Corp. **Estado real (revelado arco Dante Beat Ten + Dragon Victory):** sequestrada na Caverna dos Perdidos (Selve Sombria, nordeste do Pântano Markov) pelos Sterling brass para chantagear Dante. Apartamento conservado por equipe FIR que rotaciona luz acesa, lixo semanal e cortina trocada de posição, para que vizinhança de baixa atenção registre "viúva calada" e não inquilino ausente. Interações mínimas no jogo; aparece em cena pós-reveal Dante (silêncio). Cross-ref [[PLACES]]:76 (Caverna dos Perdidos) + [[characters/dante-grid]]:117 (chantagem operacional Vorto-Sterling).
- **Otmar Neumann** (50), pai Linda. Engenheiro analógico. Vive em apartamento simples Zona do Silêncio.
- **Brígida Neumann** (49), mãe Linda. Musicista (violão). Hospeda versão antiga da Festa da Compilação ([[tradicoes-cultura]] §1).

### Antagonistas secundários e tenentes Sterling

- **Diretor Cassiano Vorto** (50), cabeça FIR em GusWorld City. Vassalo direto Sterling. Handler de Dante. Fala doce, frieza mafiosa. Vilão funcional (sem tese filosófica; só interesse). Mini-boss possível arco Cauã ou Dante.
- **Hierofante Adila Murmúrio** (40), líder facção pró-Sterling Cult Mirage. Manipuladora canônica Iara. Carismática, sedutora. Mini-boss arco Iara.
- **Octávia Penedo** (55), Diretora Departamento de Contenção Sterling Corp. Subordinada direta Sterling. Falha operacional com Patch-Zero ([[in-world-docs]] doc 14). Não aparece pessoalmente em combate; aparece por holograma em arco Bento ou ato 3.
- **Theodoro Calveri** (52), Diretor de Aquisições Sterling Corp. Autor canônico da engenharia financeira que destruiu cooperativa Alencar em -8 e oficina Neumann no mesmo ano, mais 12 operações similares. Assinatura "deprecação eficiente, sem overhead emocional" (ver [[in-world-docs]] DD-016).
- **Solange Vix** (46), Diretora de Imagem Pública Sterling Corp. Roteirista da holografia corporativa. Autora canônica de 14 campanhas Sterling em 13 anos (inclui "Sustento Eficiente, Cidadania Tranquila"). Nunca aparece em pessoa, em holograma ou em voz.
- **Sub-Diretor de Distrito FIR (3 outros)**, coronéis regionais FIR. Não nomeados in-game canonicamente; podem ser introduzidos em sidequest ato 2.
- **Hierofantes Cult Mirage subordinados (3 menores):** **Cleomir Vasta**, **Tâmela Brida**, **Otoniel Rens**. Pró-Sterling. Subordinados Adila. Aparecem em cena coletiva festival Mirage.

### Aliados internos das facções inimigas

- **Hierofante Florín Estopa** (38), Cult Mirage, facção artista genuína. Aliado potencial pós-arco Iara. Mantém ala artista resistente.
- **Hierofante Marlena Aurora** (35), Cult Mirage, facção artista. Detida por Adila; resgatável em sub-quest pós-recrutamento Iara.
- **Mestre Lavínia Sevra** (62), Ordem Recursiva. Mestre tradicionalista aliada Velhusto. Aparece em cerimônias.
- **Mestre Hugo Tirol** (55), Ordem Recursiva. Outro tradicionalista; participa expedição catedral perdida arco Bento.

### Companions Underground (célula GusWorld City)

- **Tiago "Padrinho" Sevroski** (55), já listado. Coordenador.
- **Mara Bento** (28), operadora rádio analógica. Coordenadora sub-célula Norte.
- **Joaquim Bartolomeu** (32), técnico analógico. Reconstrói toca-discos artesanalmente.
- **Helga Riza** (45), escritora subterrânea. Compila pichações Polis-Vermelha ([[in-world-docs]] doc 11).

### Runners juvenis dos Dutos (grupo Cauã)

- **Cauã** (13), líder informal. Já canônico.
- **Tao Berisi** (12), segundo Cauã. Especialista navegação vertical. Comutador-Antigo de jogos ([[comic-reliefs]] EE-16).
- **Inês Marçal** (11), pequena, ágil, conhecedora dos Dutos profundos. Salva por Cauã em -3.
- **Bel Galvão** (14), mais velha do grupo. Treina os menores em combate básico.
- **Pirilampo** (apelido, 13), sem nome formal conhecido (órfão registrado). Especialista em improvisar luz com baterias descartadas.

### Aldeias-fronteira Pelicano Branco (NPCs secundários)

- **Anciã Mariana Vanderbist** (89), já listada.
- **Bito Caldeira** (60), prefeito informal vilarejo ([[comic-reliefs]] EE-14).
- **Helena Sirinhaém** (50), herborista júnior. Aliada Jaci.
- **Solane Vanderbist** (8), primo distante Jaci. Criança do vilarejo. Aparece em sub-quest plantio.

### Acadêmicos/historiadores citados (lore-fundo)

- **Verônica Atelaiá** (morta em -35, aos 43 anos canônicos, canon F5-BK.AUDIT T3-CR-16). Última grande pensadora pré-Sterling. Nasc -78, ativa -45. Primeira teorizadora Patch-Zero como anomalia ontológica genuína. Tratado anônimo classificado "anomalia folclórica" por Sterling Locke em -25 (post-mortem). Atribuída ao tratado anônimo doc 10. Morreu pobre, em registro institucional preservado pela linhagem matrilinear cronistas Atelaiá.
- **Bartolo Penkin** (~50 quando desapareceu, em -12), engenheiro QA sênior Apex-Data. Atribuído ao relatório audit doc 5. Desaparecido pós-vazamento.

---

## 15. Tradições culturais (resumo; detalhes em `tradicoes-cultura.md`)

> **Referência primária:** [[tradicoes-cultura]] detalha 9 tradições (5 maiores + 4 menores) com origem, rituais, comidas, gestos, reação de facções, integração no jogo.

### Festividades canônicas principais

| Festividade | Data | Origem | Facção líder |
|---|---|---|---|
| **Festa da Compilação** | 1º setembro | Era 2 (~-150) | Cidadania geral (cooptada Sterling) |
| **Dia do Tomo** | 14 janeiro | Era 2 (~-100) | Comunidade técnica + Pilha |
| **Vigília Neo-Sylvania** | 21 março | Era 2 (~-148) | Ordem Recursiva |
| **Dia das Sementes** | 22 junho | Pré-Era 2 (oral) | Pelicano Branco |
| **Noite Calada** | 8 agosto | Era 2 tardia (~-50) | Underground |

### Calendário in-world

12 meses de 30-31 dias. Estações: primavera (set-nov), verão (dez-fev), outono (mar-mai), inverno (jun-ago). Hemisfério sul.

### Costumes universais

Cumprimentos variam por setting (Cidade: "Compila bem"; Catedrais: "A engrenagem te guie"; Pelicano: "Boa raiz"; Silêncio: gesto silente; Mirage: "Brilhe bem"). Comidas: feijão-computacional, café-de-neurônio, pão-de-bit, caldo-de-folha-fractal, pamonha-de-fronteira (regionais Pelicano), cubos-energéticos Sterling (corporativos satirizados). Brincadeiras infantis: Compila ou Crasha, Vetor do Cego, Tomo da Pergunta Boba. Detalhes em [[tradicoes-cultura]] §Costumes universais.

### Tabus principais

- Não force-push (Asmódico).
- Não pisar em cripto-glifo Neo-Sylvania.
- Não falar alto na Zona do Silêncio.
- Não tocar cronômetro mecânico de mestre Asmódico sem permissão.
- Não desperdiçar semente-relíquia.
- Não comentar diretamente desaparecidos por Sterling Corp (eufemismo "transferido pra Heliópolis-Nova").

---

## 16. Cross-refs

- **Pillars de design:** `docs/design/pillars.md`
- **Sinopse base imutável:** `sinopse.md`
- **Dossiê Sterling:** `characters/prelore_vilao.md`
- **Specs visuais canônicas:** `Resources/gusworld/_INDEX.md`
- **Arco principal:** `docs/narrative/arco-principal.md`
- **Facções detalhadas:** `docs/narrative/factions.md`
- **Sterling spec narrativa:** `docs/narrative/characters/sterling-locke.md`
- **Patch-Zero spec narrativa:** `docs/narrative/characters/patch-zero.md`
- **Party:** `docs/narrative/characters/party.md`
- **Alívios cômicos canônicos:** `docs/narrative/comic-reliefs.md`
- **Timeline cronológica:** [[timeline]]
- **Documentos in-world descobríveis:** [[in-world-docs]]
- **Tradições e cultura detalhadas:** [[tradicoes-cultura]]

---

**Última revisão:** 2026-05-15. Lore-bible canônica (expansão Bloco G). Atualizações exigem aprovação do criador supremo.
