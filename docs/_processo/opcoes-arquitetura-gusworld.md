# GusWorld: opções de arquitetura de aplicação (Caetano, CTO)

Documento de OPÇÕES para decisão do líder. Nada aqui é decisão; toda escolha marcada como recomendação vai ao líder via AskUserQuestion pelo orquestrador. Convenção deste documento: toda afirmação vem marcada como **FATO** (com fonte: arquivo, seção ou verbatim do líder) ou **INFERÊNCIA** (juízo meu, do Caetano, discutível).

Premissas fechadas pelo líder nesta sessão (não reabertas aqui): jogo 2D pixel-art em runtime; dublê de plataforma PROIBIDO (verbatim: "nao quero nada falso. Ou liga com glintfx ou nao liga. Isso que estragou o projeto todo anterior."); núcleo de regra em TDD estrito AGORA, camada de desenho só quando o GlintFx existir, ligando DIRETO nele; licença AGPL-3.0-or-later.

---

## A. Diagnóstico: que sistema é este, de verdade

### A.1 A forma do dado e do fluxo

**FATO.** O GusWorld é um RPG por turnos, single-player, offline, determinístico por desenho:

- Combate é uma máquina de estados finita por ator (`combat.md` §3: SetupPhase, TurnStart, ActionSelect, ActionResolve, TurnEnd, CheckEnd, CombatEnd), com fila de iniciativa visível (§4), recursos discretos AP/Mana (§5) e fórmula de dano canonizada (§11) que inclui "Ordem de consumo do RNG (determinismo dos testes)" como contrato explícito.
- A mão de cartas é loadout determinístico, sem sorteio em combate (`deck-mao-sistema.md` §2: "Sem compra/sorteio em combate", "o motor determinístico (conta cada saque de RNG)").
- O RNG que existe é seedable e contado (`deck-mao-sistema.md` §6.1: "seed CONTÁVEL (o motor conta o RNG)"; `pillars.md` Pillar 2: "Ruído Coerente (Perlin/Simplex) como standard de RNG natural, determinístico, seedable").
- Deck e coleção são um sistema transacional com invariantes anti-exploit numerados (`deck-mao-sistema.md` §7: instância única com ID, mão como lista de IDs e não container, one-way ativo→morto por API, venda atômica e idempotente, classe protegida ESPECIAL/SUPER).
- A economia é faucets/sinks com números em tabela e uma propriedade global testável (`economia.md` §0.1, régua de validação: "o jogador comedido sai melhor no final do que o esbanjador, nesta mecânica?" respondida antes de fechar qualquer mecânica).
- Save, mapas e configs do jogador exigem proteção contra edição (`inicial.md` itens 25-26), com modos de morte que dependem disso (`modos-morte.md` §1: Hardcore com save isolado, anti-rollback, wipe).
- Exploração é hub central + radiais, sem open-world (`pillars.md` anti-pillars; `gdd.md` §7.1: gating por dificuldade ou por atalho contornável, nunca trava dura).
- UI e formatação são HTML/RML/CSS traduzidos pelo GlintFx (`inicial.md` item 21: "usaremos html/rml/css para criar formatacoes etc. Glintfx que dará essas traducoes").

**INFERÊNCIA.** Em termos de engenharia, isso é um **simulador determinístico de regras discretas sobre dados imutáveis de conteúdo + estado mutável de jogador**, com uma casca de apresentação em tempo real. Três massas de dado bem distintas:

1. **Conteúdo** (catálogo): centenas de cartas, inimigos, receitas, diálogos, mapas, statlines. Imutável em jogo, versionado no repo, balanceável (`//PLAYTEST` em `deck-mao-sistema.md` §8c).
2. **Estado de jogador** (save): coleção, progresso, knowledge, crédito, dificuldade fixa por save. Mutável, serializável, protegido (itens 25-26).
3. **Estado efêmero de sessão** (combate corrente, cena corrente). Derivado dos dois acima + comandos do jogador.

E dois regimes de tempo bem distintos:

- **Tempo de regra**: avança por comando do jogador ou tick de turno. Frequência humana (dezenas de eventos por minuto). Nenhum frame envolvido.
- **Tempo de frame**: 60 Hz, render/input/áudio. Pertence integralmente ao GlintFx e à camada que liga nele.

Essa separação de regimes é o fato arquitetural mais importante do projeto: **quase todo o jogo vive no tempo de regra e não precisa de tela para existir nem para ser provado**. A ordem de construção aprovada pelo líder (núcleo agora em TDD, tela depois) não é um contorno da ausência do GlintFx; é o desenho natural deste sistema.

### A.2 Fronteiras naturais que o design já impõe

**FATO** (cada linha com fonte):

| Fronteira | Fonte | Natureza |
|---|---|---|
| Combate (FSM, fila, AP/Mana, dano, status) | `combat.md` §3-§11, §9 | regra pura + eventos |
| Deck/mão/coleção (invariantes anti-exploit) | `deck-mao-sistema.md` §4-§7 | agregado transacional |
| Hardware de carta (bateria, vírus, pirataria) | `cartas-spec-logica.md` §1 (embrulho pré/pós-cast, agnóstico de tier); `cartas-spec-dados.md` §5 (`CardPhysicalState`) | extensão de regra + dado por instância |
| Economia (crédito, hospital, craft, dívida) | `economia.md` §1-§7 | faucets/sinks + números em config |
| Progressão (Knowledge, skill trees, XP) | `pillars.md` Sistemas-âncora | contadores por espécie de inimigo |
| Modos de morte / save / anti-tamper | `modos-morte.md`; `inicial.md` 25-26 | persistência + política por dificuldade |
| Diálogo / narrativa curada | `pillars.md` anti-pillars ("escolhas curadas"); `gdd.md` §7 | dado de conteúdo + estado de flags |
| Mapa/exploração (hub + radiais, topologia soft-gated) | `gdd.md` §7.1; `pillars.md` P5 | dado de conteúdo + posição do jogador |
| HUD/UI em RML | `inicial.md` item 21 | apresentação, tradução pelo GlintFx |
| Barramentos de evento CombatBus / PlayerBus | `combat.md` §16 | contrato entre combate e sistemas persistentes |

**FATO.** Os dois specs de cartas (`cartas-spec-dados.md` §2, `cartas-spec-logica.md` cabeçalho) já declararam uma fronteira de responsabilidade em três camadas na tentativa anterior: `domain/` define campos e invariantes de forma, sem regra de apresentação; `app/` (gameplay) decide QUANDO regras rodam; UI decide O QUE MOSTRAR, com o invariante "nunca vaza is_infected antes de is_diagnosed" nomeado como contrato ENTRE camadas.

**FATO.** O repositório git do GusWorld não tem nenhum commit; os caminhos de código citados nesses specs (`GusEngine/domain/include/gus/...`) são da tentativa anterior, que o líder mandou refazer do zero (`inicial.md` item 18: "estou fazendo DO ZERO tudo com relacao ao codigo, sempre assentado sobre GlintFx"). **INFERÊNCIA:** os specs valem como design canônico (comportamento, invariantes, fronteiras), mas nenhum struct antigo é forma final: cada record renasce por TDD, no vocabulário novo (C++23, snake_case se o líder mantiver a convenção da casa GlintFx, decisão dele).

**FATO.** Estado do GlintFx medido hoje (21/08/2026): `include/` contém um único header, `glintfx/core/version.hpp`. Não existe janela, input, contexto gráfico nem texto na tela. Sem data prometida. O `inicial.md` item 2 ordena: "Se ainda não existir a funcao em GlintFx, registre a necessidade no bus e espere parado a resposta dele."

**INFERÊNCIA (consequência direta):** a camada de apresentação do GusWorld não pode nascer agora de forma nenhuma que respeite as ordens (ligar direto + sem dublê + GlintFx sem superfície). O plano de arquitetura precisa, portanto, de uma propriedade específica: **o conjunto de módulos que NÃO dependem do GlintFx tem que ser máximo, e a fronteira com o GlintFx tem que ser fina e nomeada cedo**, para registrar as necessidades no bus com precisão e não parar o projeto.

---

## B. Opções de arquitetura, comparadas

O líder perguntou nominalmente por "hexagonal" e "espinha" (`inicial.md` item 3: "vamos discutir as camadas: hexagonal? espinha? Sugira. PROIBIDO monolitos."). Trato as duas e proponho uma terceira, que é a que recomendo.

### B.0 A pergunta do despacho virtual, respondida antes das opções

A L-19 do GlintFx rejeitou interface virtual em runtime para o framework porque o caminho quente dele é por frame e por draw (60 Hz vezes N chamadas). A pergunta do encarte: um jogo por turnos 2D tem o mesmo problema?

**Resposta com número, não com dogma (INFERÊNCIA quantitativa):**

- O **núcleo de regra** do GusWorld executa na frequência do jogador: uma ação de combate a cada poucos segundos, um tick de status por turno. Mesmo no pior caso computacional do design, o auto-resolve ("FSM headless + IA sub-ótima", `combat.md` §19.6, FATO) e as suítes de teste em massa, estamos falando de milhares a milhões de resoluções por segundo em batch. Um despacho virtual custa da ordem de nanosegundos (indireção + provável erro de branch predictor). Um combate inteiro auto-resolvido com centenas de despachos virtuais custaria microssegundos a mais. **Conclusão: no núcleo de regra, o argumento de performance da L-19 NÃO se aplica. Virtual ali não é crime de custo.**
- O **caminho por frame** (desenhar sprites, processar input, tocar áudio) tem exatamente o problema que a L-19 descreve. Só que esse caminho, no GusWorld, é do GlintFx e da camada que liga nele, e o líder já ordenou que essa ligação é DIRETA, sem intermediário (premissa fechada: "Ou liga com glintfx ou nao liga"). **Conclusão: no único lugar onde a porta virtual custaria caro, ela é proibida por ordem antes de ser cara.**

Então a questão do virtual no GusWorld não se decide por performance: decide-se por **necessidade**. Interface em runtime se justifica quando existe mais de uma implementação real viva ao mesmo tempo, escolhida em execução. No GusWorld isso quase não existe: efeitos de carta são despachados por dado (`EffectKind`/tabela, `combat.md` §10-§11 e `cartas-spec-logica.md` §1, FATO: o design já rejeitou "um EffectKind novo por vírus" e preferiu embrulho único agnóstico de tier), variação de dificuldade é política por dado (`modos-morte.md` §1, tabela por modo), e a plataforma é uma só (GlintFx). O que precisa de substituição é o que o TESTE precisa trocar: relógio, RNG, fonte de conteúdo, storage de save. E isso se resolve por parâmetro, não por hierarquia.

### B.1 Opção 1: Hexagonal clássico (ports & adapters com interfaces em runtime)

**Desenho.** Domínio no centro (entidades + serviços de regra), portas como interfaces abstratas (`class RenderPort { virtual ... }`, `SaveStorePort`, `ClockPort`, `RngPort`), adaptadores na borda (GlintFx, filesystem, relógio do SO), application services orquestrando casos de uso, wiring por injeção em runtime na composição root.

**Quem depende de quem.** Tudo aponta para o domínio; adaptadores implementam portas; o main monta o grafo.

**Onde entra o GlintFx.** Como adaptador de uma `RenderPort`/`InputPort` abstrata que o domínio ou a camada de aplicação define.

**Como o dado atravessa.** Entidade de domínio -> DTO da porta -> adaptador desenha.

**Como se testa sem tela.** Substituindo adaptadores por fakes que implementam as portas. Funciona bem; é o ponto forte clássico do padrão.

**Prós.**
- Vocabulário consagrado, dependências invertidas por construção, regra protegida de I/O (que, ao contrário do GlintFx, aqui EXISTE de verdade: o GusWorld tem regra de negócio, o framework não tinha, e a própria L-19 diz "Hexagonal clássico existe para proteger regra de negócio, que um framework 2D não tem", FATO).
- Teste por substituição de porta é mecânico e conhecido.

**Contras (e aqui o padrão tropeça neste projeto especificamente).**
- **A porta de plataforma é ilegal.** Uma `RenderPort` abstrata com o GlintFx como "um adaptador" é, com outro nome, exatamente a camada `nossa_janela` que embrulha a do framework, que o líder proibiu (premissa fechada, dublê proibido; e um fake de `RenderPort` em teste é um mock de plataforma). O hexagonal clássico perde o seu lado mais valioso aqui por ordem expressa. Sobram as portas de infra (save, relógio, RNG), que são pequenas demais para justificar a cerimônia inteira do padrão.
- **Cerimônia sem consumidor.** Interfaces + fábricas + wiring em runtime para um executável single-player com uma plataforma e um storage. INFERÊNCIA: é a versão de aplicação da "porta gorda" que a L-19 nomeia.
- Convida despacho virtual onde tabela de dados resolve melhor (efeitos de carta como hierarquia de classes é um anti-padrão conhecido de TCG; o design já escolheu dado, `combat.md` §10).

**Impacto/esforço.** Esforço ALTO relativo (mais arquivos, mais indireção, wiring). Impacto de performance irrelevante no núcleo (ver B.0), mas custo permanente de leitura e manutenção.

### B.2 Opção 2: Espinha de camadas (layered core-out, o desenho da L-19 adaptado a aplicação)

**Desenho.** Camadas empilhadas, dependência só para baixo, gate de CI reprovando violação (cópia consciente da L-19 item 1 do GlintFx):

```
present/   liga DIRETO no GlintFx: janela, loop de frame, sprites, RML, input, áudio.
           NASCE SÓ QUANDO O GLINTFX EXISTIR. Única camada com #include <glintfx/...>.
app/       orquestração de jogo: cenas/fluxo (título, mapa, combate, bancada),
           avanço de turno, coordenação entre domínios, save/load como caso de uso.
domain/    a regra: combate, deck/coleção, economia, progressão, diálogo, mapa-como-dado,
           modelo de save. POCOs + serviços puros. Zero I/O, zero GlintFx, zero SO.
content/   catálogo de conteúdo (cartas, inimigos, receitas, statlines) + seu carregador/validador.
core/      tipos básicos do jogo, RNG determinístico contado, ids, resultado/erro, versão.
```

(`content/` acima de `core/` e abaixo de `domain/` porque o domínio consome catálogo; detalhe de ordem é ajustável, o que importa é a direção única e o gate.)

**Quem depende de quem.** Cada camada só das de baixo. `domain/` nunca sabe que existe tela. `present/` conhece `app/` e lê estado; nunca o contrário.

**Onde exatamente entra o GlintFx.** Só em `present/`, por include direto e chamada direta, sem interface intermediária, sem wrapper genérico. Gate de CI: qualquer `#include <glintfx/` fora de `present/` reprova o build (mesmo mecanismo do `tests/tools/check_layers.sh` que o GlintFx já usa, FATO: citado no comentário de `include/glintfx/core/version.hpp`).

**Como o dado de jogo atravessa.** POCO de domínio -> view-model simples (struct de apresentação montado em `present/` a partir do estado + eventos) -> template RML. A regra do que pode ser mostrado (ex.: infecção só depois de diagnóstico) é invariante declarado no domínio e apenas OBEDECIDO pela UI (`cartas-spec-dados.md` §2, FATO).

**Como se testa sem tela.** Por construção, não por simulação: os testes de `core/`, `content/`, `domain/` e `app/` nem LINKAM `present/`. Não existe modo headless, não existe fake de janela; existe código que não precisa de janela. Isso respeita a proibição de dublê pela raiz: nada é fingido porque nada de plataforma é tocado. (A única exceção futura: teste de integração da `present/` real, que é assunto de container/compositor, fora deste documento.)

**Substituição em teste** (relógio, RNG, storage): parâmetro explícito da função ou struct de serviços passada por referência; se quisermos consistência total com a casa, concept de C++23 resolvido em compile-time como na L-19 item 2. INFERÊNCIA honesta: aqui o concept é escolha de estilo, não de necessidade; a taxa de turnos não paga nada por uma indireção. Recomendo concept apenas onde o par real existir (RNG real × RNG de teste; disco real × memória), nunca especulativo.

**Prós.**
- Alinhado com todas as ordens vigentes: sem dublê, ligação direta, núcleo TDD agora, PROIBIDO monólito (cada domínio é módulo estreito, L-17 traduzida), rigor idêntico ao GlintFx (`inicial.md` item 23).
- Esforço mínimo de cerimônia; cada linha de arquitetura tem consumidor imediato.
- A fronteira com o GlintFx fica fina e enumerável: a lista do que `present/` precisa (janela, sprite batch, RML, input, áudio) é exatamente a lista de necessidades a registrar no bus.

**Contras.**
- `app/` é o ponto de risco: orquestrador que pode engordar até virar o monólito proibido (mitigação em D).
- A fronteira domain/app exige critério enunciado (regra vs orquestração) e revisão disciplinada; camadas sem enforcement degeneram, por isso o gate de CI não é opcional.
- Menos "nome de padrão de livro" que hexagonal; a proteção vem da direção única + gate, não de interfaces.

**Impacto/esforço.** Esforço BAIXO-MÉDIO. Melhor custo-benefício se o item B.3 parecer sobre-especificado.

### B.3 Opção 3 (recomendada): Espinha de camadas + núcleo determinístico comando/evento

A Opção 3 é a Opção 2 com um contrato a mais DENTRO de `domain/`: **a regra é exposta como transição determinística de estado**.

**Desenho.** Tudo da Opção 2, mais:

- Cada sistema de regra expõe a forma `apply(estado, comando) -> (estado novo, eventos)`, onde estado é POCO serializável, comando é POCO (a intenção do jogador ou do scheduler: `use_card`, `swap_battery`, `travel_to`), e eventos são POCOs notificáveis (`damage_dealt`, `status_applied`, `card_infected_revealed`). O combate JÁ É isso no design: FSM + CombatBus/PlayerBus com eventos nomeados e contrato de evento de status canonizado (`combat.md` §3 e §16, FATO). A opção 3 apenas promove esse desenho, que o design já adotou para combate, a contrato de TODO o domínio.
- O RNG entra como parte do estado (seed + contador), nunca como global: consequência direta do canon "conta cada saque de RNG" (`deck-mao-sistema.md` §2, FATO) e da "Ordem de consumo do RNG" (`combat.md` §11, FATO).
- `app/` vira um despachante fino: recebe input traduzido, monta comando, chama `apply`, persiste/encaminha eventos. `present/` (futura) assina eventos e lê estado para desenhar.

**Onde entra o GlintFx.** Idêntico à Opção 2: só `present/`, direto.

**Como o dado atravessa.** Comando entra, evento sai; a UI RML é projeção de estado + fila de eventos. Nenhuma chamada de volta da regra para a tela, nunca.

**Como se testa sem tela.** É o ponto máximo desta opção:
- TDD unitário natural: dado estado X e comando Y, saem estado X' e eventos E. Vermelho-verde sem mock nenhum.
- **Replay como prova de determinismo**: um combate inteiro é uma lista de comandos + seed; re-executar tem que produzir byte a byte o mesmo estado final. Isso transforma o canon de determinismo (FATO acima) de intenção em teste automatizado permanente, e dá de graça o harness do auto-resolve (`combat.md` §19.6) e do balanceamento em massa.
- Property-based na economia: a régua do comedimento (`economia.md` §0.1, FATO) vira propriedade executável (política comedida termina com resultado maior ou igual à esbanjadora sob as mesmas seeds).
- Os invariantes anti-exploit do deck (`deck-mao-sistema.md` §7, FATO, incluindo o inv.8 que EXIGE testes dedicados) são testados na transição: nenhuma sequência de comandos leva a carta duplicada ou a especial no deck morto.
- Save/anti-tamper (itens 25-26) se beneficia: existe UM estado canônico serializável por construção, então hash/assinatura cobre o jogo inteiro, sem caçar estado espalhado.

**Prós.** Tudo da Opção 2, mais: determinismo provado e não prometido; save trivialmente completo; depuração por replay (bug report = seed + comandos); UI desacoplada por evento como o design já pede.

**Contras.**
- Exige disciplina para não virar dogma: nem tudo é redutor puro elegante. Coleção/inventário com invariantes transacionais fica melhor como agregado com métodos que preservam invariantes (o próprio `deck-mao-sistema.md` §7 fala em "API ativo→morto" e "NÃO existe API morto→ativo", que é linguagem de agregado, FATO). A regra prática: a FORMA comando/evento é obrigatória na fronteira do sistema de regra; por dentro, o sistema se organiza como quiser (funções, agregados), desde que atômico (L-17 da casa).
- Um degrau a mais de desenho antecipado que a Opção 2 (definir comandos e eventos por sistema).
- Estado imutável ingênuo (copiar tudo a cada comando) pode custar em batch; mitigação simples: `apply` recebe estado por valor mutável ou referência com contrato de "transação inteira ou nada". Detalhe de implementação, não de arquitetura.

**Impacto/esforço.** Esforço MÉDIO (acima da 2 no desenho inicial, abaixo dela no custo de teste e depuração ao longo da vida).

### B.4 Comparação e recomendação

| Critério | 1 Hexagonal clássico | 2 Espinha | 3 Espinha + comando/evento |
|---|---|---|---|
| Compatível com "sem dublê / liga direto" | NÃO no ponto central (porta de render é o dublê com outro nome) | SIM | SIM |
| Testa sem tela | sim, via fakes de porta | sim, por construção | sim, por construção + replay |
| Determinismo canônico vira teste | não naturalmente | parcial | SIM (replay byte a byte) |
| Cerimônia/indireção | alta | baixa | média |
| Risco de monólito | médio (application service gordo) | médio (`app/` gordo) | baixo-médio (`app/` é despachante fino) |
| Aderência ao canon já escrito (FSM, bus, RNG contado, invariantes) | neutra | boa | máxima (o canon JÁ tem essa forma) |
| Esforço relativo | alto | baixo-médio | médio |

**Recomendação (INFERÊNCIA minha, decisão do líder): Opção 3.** Motivo central: o design canônico já escolheu essa forma sem dizer o nome; FSM de combate, barramentos de evento, RNG contado, mão como lista de IDs, one-way por API e régua de economia são, todos, contratos de transição de estado determinística. A arquitetura só precisa não atrapalhar o que o design já é. A Opção 2 é o fallback digno se o líder preferir menos desenho antecipado (e a 3 pode ser adotada sistema a sistema por cima da 2, começando pelo combate). A Opção 1 eu desaconselho neste projeto: o seu benefício central é ilegal aqui, e o que sobra dela a Opção 2 entrega mais barato.

### B.5 Onde copio a L-19 do GlintFx e onde divirjo, e por quê

A L-19 é a arquitetura DA BIBLIOTECA; o GusWorld é APLICAÇÃO. Item a item:

| Compromisso L-19 | GusWorld copia? | Por quê |
|---|---|---|
| Camadas com dependência só para baixo + gate de CI | **COPIA integralmente** | vale para qualquer sistema; o gate é o que impede degeneração (e o GusWorld deve rigor idêntico, `inicial.md` item 23) |
| Uma única camada toca a fronteira externa | **COPIA, com a fronteira trocada**: no GlintFx a fronteira é o SO; no GusWorld a fronteira é o GLINTFX (e o SO só através dele, `inicial.md` item 2) | a aplicação não pode tocar SO direto por ordem; exceção a discutir com o líder: save em disco (o GlintFx pode não ter I/O de arquivo no escopo; se não tiver, é necessidade a registrar no bus OU exceção explícita do item 2, decisão do líder) |
| Porta como concept compile-time, sem virtual em caminho quente | **COPIA onde há par real de implementações** (RNG teste/real, storage memória/disco); **NÃO cria porta para o GlintFx** | porta de plataforma é o dublê proibido; e o caminho quente por frame nem existe fora de `present/` (ver B.0) |
| Superfície pública opaca (PIMPL/handle, ABI) | **DIVERGE: não copia** | motivação da L-19 item 3 é ABI de `.so` consumida por terceiros; o GusWorld é executável final sem consumidor de headers; opacidade custaria indireção e cerimônia sem nenhum ganho. Os POCOs de domínio são visíveis por desenho (são o contrato interno e o formato de save) |
| Nenhuma exceção cruza a API pública (L-22) | **ADAPTA** | não há API pública de biblioteca; mas recomendo (INFERÊNCIA, estilo da casa) erro por `std::expected`/resultado no domínio, porque falha de regra é VALOR do jogo (erro de compilação de carta é mecânica visível, `combat.md` §10), e `CONTRACT.md` §6.4 já proíbe engolir erro |
| Proibição de monolito, módulo estreito, um assunto por arquivo (L-17) | **COPIA integralmente** | é ordem do líder também no GusWorld (`inicial.md` item 3) |

---

## C. Os átomos e os POCOs

Ordem do líder (`inicial.md` item 3, verbatim): "OS itens/cartas e demais elementos do jogo devem ser atomos com POCO proprio!". Proposta concreta do que isso significa, para o líder validar.

### C.1 O que é POCO aqui

**Proposta (INFERÊNCIA, ancorada nos specs).** POCO = struct C++ de dados puros:

- membros públicos, sem herança, sem `virtual`, sem ponteiro cru dono de recurso;
- dependências: só `core/` e stdlib; jamais GlintFx, jamais I/O;
- comparável (`operator==` default) e serializável;
- invariante de FORMA checável (`validate()` ou construtor fail-fast), sem regra de jogo dentro;
- zero-value seguro (o default é o estado inofensivo).

**FATO.** É exatamente a forma que o spec da tentativa anterior já praticava: `CardInstance` com `operator==(...) = default`, `CardPhysicalState` com "default = ROM original legítima, bateria cheia, sem infecção, ver §5.2 sobre por que o zero-value é o estado SEGURO", enums fortes (`CardOrigin`, `VirusKind`) (`cartas-spec-dados.md` §4-§5). E a fronteira "domain define campos e invariantes; app decide quando; UI decide o que mostrar" está tabelada em `cartas-spec-dados.md` §2.

### C.2 Um átomo por elemento, e a separação espécie × exemplar

**Proposta.** Cada elemento do jogo é um átomo com POCO próprio e arquivo próprio (um assunto por arquivo, L-17 da casa espelhada no GusWorld):

- carta (espécie de catálogo), instância de carta, estado físico de carta (bateria/infecção), bateria como item, status, receita de combo, statline de inimigo, espécie de inimigo, entrada de bestiário/RunaDex, item de bolsa (capacitor), receita de craft, ingrediente, nó de diálogo, flag narrativa, mapa/área, encontro, save.

**FATO que sustenta a separação espécie × exemplar:** "carta = instância única com ID, vive em EXATAMENTE UM container" (`deck-mao-sistema.md` §7 inv.1); "RunaDex é por espécie, não por cópia" (`cartas-spec-logica.md` §2); "todo exemplar de cardExec-faraday-fake finge a MESMA especial... é um fato do que esta carta É, não do que esta cópia específica" (`cartas-spec-dados.md` §3). Ou seja: o design já distingue **catálogo** (imutável, conteúdo) de **instância** (mutável, save). A arquitetura consagra isso: POCOs de catálogo vivem em `content/`-side e nunca mudam em jogo; POCOs de instância vivem no estado/save e referenciam o catálogo por id, sem duplicar campos (mesmo princípio do FATO em `cartas-spec-dados.md` §3: instância não duplica `charge_cost`/`power`).

### C.3 Onde vive a regra

**Proposta.** Regra vive em serviços/funções do `domain/`, nunca no POCO:

- Funções puras derivadas (SoH da bateria, classe de hardware) junto do dado, como o spec já fazia (`cartas-spec-dados.md` §5.3, FATO).
- Transições e invariantes transacionais em serviços de agregado: o descarte que RECUSA especial ("guard por tier no service de descarte; custo ~1 if, risco altíssimo se faltar", `deck-mao-sistema.md` §7 inv.9, FATO), a venda atômica e idempotente (inv.5), o embrulho pré/pós-cast do hardware de carta ("embrulha o resolve_use_card inteiro, ponto único, agnóstico de tier", `cartas-spec-logica.md` §1, FATO).
- Na Opção 3, a fronteira de cada sistema é `apply(estado, comando) -> (estado', eventos)`; os serviços acima são o interior dela.

### C.4 Como o conteúdo entra no programa (centenas de cartas, inimigos, diálogos)

Três formas possíveis; a decisão é do líder, e ela interage com o item 2 do `inicial.md` (só GlintFx + SO) e com o editor de mapas e o bus (item 12).

**a) Arquivo de dado lido em runtime, parser próprio.** Conteúdo em arquivos de texto versionados (um arquivo por átomo de conteúdo ou por família), formato simples definido por nós, parser escrito em casa por TDD (sem dependência de lib de parsing, coerente com a postura da casa; INFERÊNCIA: o item 2 fala de LINK com framework e SO, e um parser próprio não é link com nada).
- Prós: balancear números sem recompilar (o design é cheio de `//PLAYTEST` afináveis, `deck-mao-sistema.md` §8c, FATO; `cartas-spec-dados.md` §9 manda "config, não hardcode", FATO); o editor de mapas externo (FATO: `inicial.md` item 12) produz dado, não código; diff de balanceamento legível em PR.
- Contras: parsing e validação em runtime (mitigado por fail-fast no boot + testes de schema); parser é código nosso a manter.

**b) Tabela gerada em compile-time.** Fonte de dado versionada -> gerador (script do repo) -> arrays `constexpr` compilados no binário.
- Prós: zero parsing em runtime, erro de conteúdo vira erro de build, binário auto-contido.
- Contras: rebuild a cada ajuste de número (fricção direta contra a cadência `//PLAYTEST`); editor de mapas teria que disparar build; gerador é uma ferramenta a mais.

**c) Híbrido (minha recomendação, INFERÊNCIA):** conteúdo como DADO em runtime (forma a), com **validador rodando duas vezes**: no CI (suite que carrega o catálogo inteiro e roda `validate()` + checagens cruzadas: toda receita referencia cartas existentes, toda fraqueza fecha a roda de `combat.md` §6) e no boot (fail-fast). Integridade por hash simples anti-corrupção. Sem assinatura forte no CONTEÚDO: o jogo é FOSS/AGPL (premissa fechada + `pillars.md` boundaries comerciais), o dado do jogo é aberto por natureza; a proteção criptográfica exigida pelos itens 25-26 (FATO) é sobre SAVE, MAPAS e CONFIGS DO JOGADOR, não sobre o catálogo. Detalhe importante a discutir com o líder na fase própria: hash/cripto de save precisa de fonte de primitivas (SO? GlintFx? implementação própria?), e isso é candidato a necessidade registrada no bus ou a exceção explícita do item 2. NÃO decido aqui.

### C.5 Como se prova por teste

- `validate()` de cada POCO com casos vermelho-primeiro (TDD estrito, premissa fechada).
- Round-trip de serialização byte-exato (POCO -> bytes -> POCO idêntico), por átomo.
- Suite de catálogo: carrega TODO o conteúdo real e aplica as checagens cruzadas (nenhuma carta órfã, roda de fraqueza fechada e determinística, `combat.md` §6, FATO).
- Invariantes anti-exploit como testes dedicados, exigidos pelo próprio canon (`deck-mao-sistema.md` §7 inv.8: "o qa-engineer DEVE ter testes dedicados de dup/slot-extra/one-way/atomicidade/round-trip no save", FATO; inv.9: "tentar mandar especial pro morto → rejeitado", FATO).
- Na Opção 3: replay determinístico (seed + comandos => estado final byte-exato) e propriedades de economia (`economia.md` §0.1, FATO).

---

## D. Riscos e armadilhas deste desenho

### D.1 As três armadilhas da L-19, traduzidas para aplicação

1. **Fachada que vira dona de tudo -> aqui: o objeto `Game`/`World` e a camada `app/`.** Numa aplicação, o PIMPL nem existe, mas o equivalente exato é a classe central que agrega todos os sistemas e cresce método a método até ser o monólito com outro nome, e a cena que recebe "o mundo inteiro" e mexe em tudo. Guarda: `app/` é despachante fino (recebe input, monta comando, chama o domínio, roteia evento); cada cena declara e recebe SÓ os sistemas que usa; contexto por assunto, nunca um "GameContext" universal. Achado de revisão, não gosto.
2. **Porta gorda -> aqui: o struct de serviços e o barramento de eventos.** Um `Services` com vinte campos ou um `Event` variant com cem alternativas é a interface inchada da L-19 em roupa de aplicação. Guarda: um serviço por assunto (relógio, RNG, storage), composição sob demanda; barramentos separados e tipados como o canon já faz (CombatBus interno × PlayerBus persistente, `combat.md` §16, FATO), com contrato de evento canonizado, nunca um bus universal de "mensagem genérica".
3. **`#ifdef` dentro de função -> aqui: o `if (dificuldade == ...)` e o número mágico de playtest.** A aplicação não tem variante de plataforma (o GlintFx a esconde), mas tem variante de MODO: quatro dificuldades com consequências próprias (`modos-morte.md` §1, FATO) e dezenas de números `//PLAYTEST`. O monólito montado por condicionais é o mesmo defeito: `if` de dificuldade picotando o corpo das funções de regra some da leitura igual ao `#ifdef`. Guarda: política por DADO (uma tabela de parâmetros por modo, escolhida uma vez na carga do save) e números nomeados em config (`cartas-spec-dados.md` §9, FATO), nunca literal no corpo da regra.

### D.2 Riscos adicionais que eu enxergo (INFERÊNCIA, cada um com guarda)

4. **Recriar o dublê proibido por hábito.** O reflexo de "abstrair a engine para testar" vai reaparecer em revisão e em agente mal-briefado, e foi isso que "estragou o projeto todo anterior" (verbatim do líder). Guarda: o gate de CI de camadas + regra escrita: teste de domínio não conhece tela; teste de `present/` usa o GlintFx REAL em ambiente isolado; não existe terceira via.
5. **Bloqueio pelo GlintFx mal gerido.** O GlintFx tem hoje um header e nenhuma promessa de data (FATO medido). Se a fronteira `present/` não for especificada cedo, as necessidades chegam tarde ao bus e o projeto para na hora errada. Guarda: derivar já, do design, a lista de necessidades (janela, loop, sprite/atlas 2D, tradução RML/CSS que o item 21 promete, input, áudio, texto) e registrá-las no bus como manda o item 2, enquanto o domínio avança em TDD. O item 2 também ordena avisar quando há fatia avançável em paralelo: há, e é o domínio inteiro.
6. **Spec antigo tratado como código.** Os specs citam `GusEngine/...` e records da tentativa anterior (FATO, cabeçalhos de `cartas-spec-dados.md` e `cartas-spec-logica.md`); repo atual sem commit (FATO). Guarda: specs são canon de COMPORTAMENTO; toda forma renasce por TDD; proibido restaurar código antigo (paralelo direto da L-01 do GlintFx e do item 18 do `inicial.md`).
7. **Regra vazando para a UI RML.** Com UI declarativa é tentador decidir na view ("mostra infectada se diagnosticada E..."). O canon já nomeia o invariante como contrato entre camadas ("nunca vaza is_infected antes de is_diagnosed", `cartas-spec-dados.md` §2, FATO). Guarda: view-model montado do lado de cá com SÓ o que pode ser visto; teste do view-model no domínio/app, sem tela.
8. **Determinismo quebrado por detalhe de C++.** Iteração de `unordered_map`, ponto flutuante em fórmula de dano, ordem de avaliação: qualquer um quebra o replay byte-exato em silêncio. Guarda: contêineres de iteração estável no estado, aritmética inteira/fixa nas fórmulas de regra onde couber, e o próprio teste de replay como detector permanente.
9. **Bus de eventos virando sopa.** Evento genérico demais vira acoplamento invisível (todo mundo escuta tudo). Guarda: eventos tipados por sistema, payload canonizado (o contrato de evento de status já existe, `combat.md` §9/§16, FATO), consumidor declarado.
10. **Segurança de save como criptografia caseira improvisada.** Itens 25-26 (FATO) pedem hash/cripto e o Hardcore pede anti-rollback e wipe (`modos-morte.md` §2.3, FATO). É exatamente o tipo de assunto que não se decide de improviso no meio de uma fatia: fonte das primitivas (SO/GlintFx/própria), modelo de ameaça (anti-trapaça honesta em jogo FOSS, não DRM) e o limite físico (usuário com root sempre pode editar RAM) merecem rodada própria com o líder e o security-engineer, ANTES do primeiro save real.

---

## Encerramento

Nada acima foi implementado nem criado dentro do projeto; este arquivo vive fora da árvore do repo. Próximo passo sugerido ao orquestrador: levar ao líder, via AskUserQuestion (sem painel lateral), a escolha entre as opções B.1/B.2/B.3 (recomendada primeiro: B.3), e, em pergunta separada quando chegar a hora, as decisões C.4 (forma de entrada do conteúdo) e D.2 item 10 (fonte de primitivas do save), que este documento deixa explicitamente abertas.

Caetano (CTO), 21/08/2026.
