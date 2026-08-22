# Lente de Esforço (Job Size) - GusWorld

Régua usada: **1, 2, 3, 5, 8, 13, 20** (Fibonacci modificada). Nenhum outro valor.

Leitura de base: L-19 (TDD estrito, com saída real do vermelho exigida), L-10 (três mãos: implementa, revisa, o orquestrador re-verifica), L-04 (proibido monolito, no máximo 40 linhas, 4 parâmetros, 3 níveis de aninhamento, um assunto por arquivo), L-09/L-20 (cinco plataformas desde o primeiro commit, CachyOS com job próprio), LEI ZERO (dependência quase zero, tudo escrito em casa), L-25 (envelope, validador, ferramenta de inspeção e gerador são software de verdade).

## A. Fundação do repositório

A1 | Job Size: 1 | Dificuldade: Baixa | `.gitignore` de duas pastas é edição mecânica; o único custo é fazer isso antes do primeiro commit (FATO, L-15).

A2 | Job Size: 2 | Dificuldade: Baixa | `.gitattributes` junta duas preocupações (LFS e marcação git-crypt), ainda config pura, sem lógica.

A3 | Job Size: 3 | Dificuldade: Média | Configurar `git-crypt` com chave simétrica exportada para fora da árvore exige cuidado de segurança e verificação de que cifra/decifra corretamente, mesmo com a ferramenta já instalada (INFERÊNCIA: risco de erro de configuração é maior que texto puro).

A4 | Job Size: 1 | Dificuldade: Baixa | Correção de frase em dois arquivos, edição textual pontual.

A5 | Job Size: 1 | Dificuldade: Baixa | Texto verbatim e imutável da AGPL-3.0, é colar o texto padrão (FATO, L-08).

A6 | Job Size: 1 | Dificuldade: Baixa | Dois textos de licença em `LICENSES/`, cópia de texto padrão mais o texto de reserva de asset já definido (FATO, L-08).

A7 | Job Size: 3 | Dificuldade: Média | `REUSE.toml` precisa refletir a fronteira jurídica do catálogo (código AGPL vs asset reservado, FATO L-25) e passar o lint do REUSE, exige entendimento correto da fronteira, não é cópia mecânica.

A8 | Job Size: 2 | Dificuldade: Baixa | `README.md` com seção de licença, carve-out de marca e link, é redação orientada por texto já decidido.

A9 | Job Size: 1 | Dificuldade: Baixa | `OFFLINE-NOTICE.md` bilíngue com texto já redigido pelo Cláudio (FATO, item A9), o trabalho é só posicionar e formatar.

A10 | Job Size: 1 | Dificuldade: Baixa | `THIRD-PARTY-LICENSES.md` com uma única entrada hoje (GlintFx).

A11 | Job Size: 3 | Dificuldade: Média | Cabeçalho SPDX em todo arquivo novo desde o primeiro (FATO, L-08) não é um artefato único, é uma prática contínua que precisa de modelo/gabarito e checagem repetida a cada arquivo criado durante toda a fundação (INFERÊNCIA: melhor tratado como item de processo com verificação, não como cópia simples).

A12 | Job Size: 1 | Dificuldade: Baixa | `git remote add` e primeiro commit são comandos mecânicos; o peso real do item A está distribuído em A1 a A11, que são pré-requisito dele (FATO, L-15: "depois do primeiro push, corrigir isto custa reescrita de histórico").

## B. Limpeza pendente do corpus

B1 | Job Size: 8 | Dificuldade: Alta | 28 ocorrências de `__DEP_REMOVIDA__` exigem julgamento caso a caso (separar nome de biblioteca do dano colateral em texto inocente), mais um portão de aprovação em bloco do líder (G1) antes de qualquer edição, o que já é FATO no item B1 e adiciona ida e volta.

B2 | Job Size: 2 | Dificuldade: Baixa | Escopo restrito a dois arquivos nomeados, remoção pontual de nome real, biografia e link, personagem preservado (FATO, item B2 e L-16).

B3 | Job Size: 5 | Dificuldade: Média | Troca de caminho absoluto por genérico soa simples, mas exige varrer o corpus inteiro em busca do padrão e decidir caso a caso o que fica (a URL do bus permanece por decisão do líder, FATO item B3), risco de falso negativo/positivo em busca textual.

B4 | Job Size: 5 | Dificuldade: Média | Sete pontas soltas nomeadas em arquivos distintos (`plano_vs.md`, `_INDEX.md`, oito specs de personagem, `style-guide.md`, `en_intl.md`, `raid-log.md`), cada uma pequena isoladamente mas com sete alvos distintos a rastrear e não perder nenhum (FATO, item B4).

B5 | Job Size: 8 | Dificuldade: Alta | A tensão `.dlg.txt`/traduções em Markdown contra L-18 (nada em texto na distribuição) não é só decisão, é desenho: decidir se é fonte de build ou runtime e, se for fonte de build, construir o caminho que sai do Markdown legível para o binário selado (FATO, L-18 e L-25 exigem isso; a execução se sobrepõe a E8, INFERÊNCIA minha de que o corte real de esforço está ligado ao gerador de conteúdo).

B6 | Job Size: 5 | Dificuldade: Média | Conferência visual amostral de 1266 imagens é trabalho de observação humana/QA em volume, sem atalho de ferramenta que substitua o olho (FATO, item B6: "nenhum humano olhou"), mas não é código, é metodologia de amostragem e julgamento.

## C. Fundação de build e CI

C1 | Job Size: 5 | Dificuldade: Média | `CMakeLists.txt` raiz com as cinco camadas (FATO, L-17) não exige TDD por ser fundação de build (FATO, L-20), mas já precisa nascer certo para as cinco plataformas.

C2 | Job Size: 3 | Dificuldade: Média | Gate de CI que reprova `#include <glintfx/` fora de `present/` é um script de verificação simples de escrever, mas precisa rodar igual nas cinco entradas da matriz (FATO, L-09/L-20).

C3 | Job Size: 13 | Dificuldade: Alta | Harness de teste próprio, sem depender de framework de terceiro (FATO, LEI ZERO), é escrever um mini-framework de asserção, descoberta e relatório do zero, e ele sustenta todo o TDD estrito do projeto inteiro (L-19), portanto tem de ser sólido antes de qualquer item de domínio começar.

C4 | Job Size: 13 | Dificuldade: Alta | Matriz de cinco plataformas com CachyOS em job próprio (não Arch renomeado) e Windows, nenhuma não bloqueante (FATO, L-09), é a entrada mais heterogênea do projeto: Windows diverge de toolchain (MSVC vs GCC/Clang) e CachyOS exige runner próprio, não reaproveitável do job Arch.

C5 | Job Size: 8 | Dificuldade: Alta | Quatro portões (`-Werror`, ASan/UBSan, `clang-tidy`+`cppcheck`, `gitleaks`, FATO L-19) multiplicados pelas cinco plataformas da C4, cada ferramenta se comporta diferente por SO e por compilador.

C6 | Job Size: 1 | Dificuldade: Baixa | `clang-format` base LLVM com dois parâmetros (indentação, colunas), config pura.

C7 | Job Size: 5 | Dificuldade: Média | Script local que espelha o CI antes do push (FATO, L-19) precisa reproduzir os quatro portões e a lógica multi-plataforma localmente, o que implica pelo menos uma variante para POSIX e uma para Windows.

C8 | Job Size: 3 | Dificuldade: Média | Hook de TDD do projeto (`.claude/tdd-guard.json`) ainda não existe (FATO, item C8), é configuração de tooling que precisa encaixar no fluxo de vermelho/verde exigido pela L-19.

## D. Núcleo de domínio, TDD estrito

D1 | Job Size: 5 | Dificuldade: Média | Tipos básicos, identificadores, resultado e erro são fundação de tudo abaixo; por L-04 cada conceito nasce como átomo em arquivo próprio, o que multiplica o número de unidades pequenas revisadas.

D2 | Job Size: 3 | Dificuldade: Média | Sorteio determinístico contado como parte do estado, nunca global (FATO, item D2 e L-17), escopo contido mas exige teste de determinismo desde já.

D3 | Job Size: 5 | Dificuldade: Média | Três POCOs distintos (carta de catálogo, instância, estado físico) com zero-value seguro, cada um átomo próprio por L-04, portanto três unidades pequenas e não uma.

D4 | Job Size: 8 | Dificuldade: Alta | Deck, mão e coleção com cinco invariantes anti-exploit já numerados no canon (instância única, mão como lista de ids, descarte de mão única, venda atômica e idempotente, especial protegida, FATO item D4), cada invariante precisa do próprio ciclo vermelho/verde da L-19.

D5 | Job Size: 13 | Dificuldade: Alta | Máquina de estados de combate, fila de iniciativa, recursos e fórmula de dano com ordem de consumo do sorteio fixada (FATO, item D5) é o sistema mais denso do domínio, reúne quatro subsistemas que por L-04 deveriam nascer como módulos distintos.

D6 | Job Size: 5 | Dificuldade: Média | Efeitos de status e contrato de evento, depende do contrato comando/evento já fixado por D5/L-17.

D7 | Job Size: 5 | Dificuldade: Média | Economia com a régua do comedimento como propriedade executável (FATO, item D7) sugere teste baseado em propriedade, não só caso a caso, o que soma esforço de desenho de teste.

D8 | Job Size: 5 | Dificuldade: Média | Progressão de conhecimento e mestria por uso, sistema novo de porte médio.

D9 | Job Size: 5 | Dificuldade: Média | Diálogo como dado depende da decisão de B5 (formato `.dlg.txt` como fonte de build ou runtime), o que pode reabrir o item se a decisão vier depois (INFERÊNCIA).

D10 | Job Size: 5 | Dificuldade: Média | Mapa como dado em grade quadrada, colisão por célula e marcação de bloco atravessável (FATO, item D10 e L-26), contido mas com regra de colisão a acertar.

D11 | Job Size: 8 | Dificuldade: Alta | Modelo de save como estado canônico serializável depende de D1 a D10 estarem fechados para agregar tudo sem buraco (INFERÊNCIA: é item de integração, não de feature isolada), e o canon do formato de save do corpus está desatualizado e bloqueado por L-13 até ser re-derivado.

D12 | Job Size: 8 | Dificuldade: Alta | Aplicar o contrato comando/evento a cada sistema de D3 a D10 (FATO, L-17) é trabalho transversal repetido em cada sistema já construído, e revisão de coerência entre eles é o tipo de achado que a L-17 já nomeia como armadilha (bus genérico, struct de serviços gordo).

D13 | Job Size: 8 | Dificuldade: Alta | Teste de replay determinístico byte a byte é exigido como teste permanente pela L-17, e a própria lei lista as armadilhas reais que o quebram em silêncio (iteração de contêiner não ordenado, ponto flutuante em fórmula de regra), o que é achado de revisão, não suposição.

D14 | Job Size: 8 | Dificuldade: Alta | Auto-resolve de combate para balanceamento em massa depende do combate (D5) estar pronto e exige um harness de simulação em volume, não é só reusar a função de combate uma vez.

## E. Proteção de dado (L-25), faseada

E1 | Job Size: 8 | Dificuldade: Alta | Envelope binário único (magia, versão, tipo, nonce, dado, selo) é a espinha comum a save, config, mapa e conteúdo (FATO, L-25), e depende de primitiva criptográfica que ainda não existe no GlintFx (FATO, L-25: "primitivas vêm do GlintFx"), então parte do item fica bloqueada por dependência externa sem data.

E2 | Job Size: 8 | Dificuldade: Alta | Serialização binária versionada dos POCOs, escrita em casa, com round-trip byte-exato por átomo (FATO, LEI ZERO e item E2), multiplica pelo número de POCOs já criados em D, cada um exigindo o próprio teste de round-trip.

E3 | Job Size: 8 | Dificuldade: Alta | Validador semântico que confere se o estado é alcançável pelas regras (FATO, L-25) precisa conhecer as regras inteiras do domínio para validar corretamente, é lógica não trivial e não um checador de formato.

E4 | Job Size: 5 | Dificuldade: Média | Gravação atômica e cadeia de backups é padrão conhecido (escrever em temporário, mover, rotacionar), mas atenção a diferença de comportamento de rename atômico entre SOs (INFERÊNCIA, ligada à L-09).

E5 | Job Size: 3 | Dificuldade: Média | Configuração selada que nunca impede o jogo de abrir, cai em padrão de fábrica com aviso (FATO, item E5), escopo contido, mas o caminho de erro precisa ser tratado com cuidado para nunca virar recusa de boot.

E6 | Job Size: 8 | Dificuldade: Alta | Ferramenta de inspeção e re-selo nasce junto com o formato, não depois (FATO, item E6 e L-25), é uma superfície própria de ferramenta de desenvolvedor que espelha tudo que E1/E2 fazem, ao contrário.

E7 | Job Size: 1 | Dificuldade: Baixa | Registrar a necessidade de criptografia no bus é comunicação, não código (FATO, L-07 e item E7).

E8 | Job Size: 13 | Dificuldade: Alta | Gerador de build que produz dois artefatos por carta (tabela compilada e pacote binário selado, FATO, L-25) é um compilador de conteúdo próprio, com fronteira jurídica embutida na lógica, item de maior risco de ficar grande demais para uma fatia só.

E9 | Job Size: 13 | Dificuldade: Alta | Save híbrido de estado mais registro de comandos encadeado, verificável por re-execução (FATO, item E9 e L-25 fase 2), é a peça anti-forjamento mais sofisticada do projeto, e depende do contrato comando/evento (D12/L-17) estar maduro.

E10 | Job Size: 8 | Dificuldade: Alta | Âncora anti-rollback por TPM opcional, com fallback em arquivo (FATO, item E10), é integração de plataforma nicho (API de TPM varia por SO), mesmo sendo opcional e nunca requisito para jogar.

E11 | Job Size: 3 | Dificuldade: Média | Amarra de máquina só no slot Hardcore com aviso ao jogador (FATO, item E11), escopo contido, mas fingerprint de máquina difere por SO (ligado à L-09).

## F. Ideias do Gus Dragon

F1 | Job Size: 5 | Dificuldade: Média | Carta `glitch` depende de dois pontos ainda em aberto devolvidos a ele na issue 3 (FATO, item F1), o que trava fechamento total; a lógica em si (atravessar bloco marcado, duração derivada) é de porte médio e usa a marcação de bloco atravessável de D10/L-26.

F2 | Job Size: 3 | Dificuldade: Média | Carta `urandom` com chance de dar errado precisa se apoiar no sorteio contado e determinístico (D2), efeito contido mas com duas variantes (pirata e original de recompensa).

F3 | Job Size: 3 | Dificuldade: Média | Vírus zip-bomb já tem ficha técnica no design, falta só o efeito de estourar em batalha (FATO, item F3), escopo restrito ligado a D5/D6.

F4 | Job Size: 8 | Dificuldade: Alta | Missão com relógio correndo de verdade, inclusive durante batalha, premiando escolha de rota (FATO, item F4), é sistema novo de temporização atravessando cena e combate, e depende da camada `app/` de coordenação existir de forma madura.

## G. Pendências de decisão do líder (portões, não trabalho)

G1 | Job Size: 1 | Dificuldade: Baixa | É aprovação em bloco, não implementação; o esforço real de produzir a lista está contado dentro de B1.

G2 | Job Size: 1 | Dificuldade: Baixa | É escolha entre opções já levantadas pelo CTO/CISO (FATO, item G2), o esforço de levantar as opções não é deste item; o de construir o gerador está em E8.

G3 | Job Size: 1 | Dificuldade: Baixa | É confirmação do que sobrevive de um desenho antigo, decisão pura.

G4 | Job Size: 1 | Dificuldade: Baixa | É decisão sobre duas ideias do Gus já registradas com ele via bus (FATO, item G4), a execução decorrente vai para F1 e um item ainda não criado (bateria acabando dentro da parede).

## H. Documentação de fechamento

H1 | Job Size: 5 | Dificuldade: Média | Wiki do GitHub derivada de `docs/`, linkando em vez de duplicar (regra permanente da casa, FATO), exige organizar e linkar um corpus de design grande, mas não é código.

H2 | Job Size: 8 | Dificuldade: Alta | Documentação `.md` extensa em registro para iniciante, explicando todo jargão sem assumir conhecimento (FATO, item H2), cobre um projeto com camadas, criptografia e TDD, o que é difícil de tornar acessível sem perder correção; pré-requisito de tag de versão significa que só começa perto do fim.

## I. Testes e auditorias (`TST-*` e `AUD-*`)

Não pontuado nesta lente: o item I do bruto não lista IDs individuais, diz que a skill de tabela de pendências injeta os IDs canônicos do catálogo, já podados para o stack. Sem IDs concretos não há o que atribuir Job Size. Recomendo que, assim que a skill gerar os `TST-*`/`AUD-*`, esta lente seja reaplicada a eles, porque cada um herda o mesmo multiplicador de L-19/L-10 dos itens de domínio que auditam.

---

## Itens que considero grandes demais para uma unidade só (proposta de corte, decisão de corte é do Cosmo)

- **B1** (28 ocorrências de `__DEP_REMOVIDA__`): fatiar por categoria, não por contagem. Um corte possível: (a) confirmação da lista completa e aprovação do líder (G1, já é gate separado), (b) lote de nomes de biblioteca puros, sem risco de dano colateral, (c) lote dos casos de dano colateral identificados (Stormlight Archive, pcgamer.com, `core::` do Rust) que exigem restauração cuidadosa, cada lote como fatia própria com o próprio commit.
- **C3** (harness de teste próprio): fatiar em (a) motor de asserção e execução de caso, (b) descoberta/registro de testes, (c) integração com CMake e saída de relatório legível por humano e por CI. A suíte de domínio inteira (seção D) depende deste item terminar primeiro, então ele é o maior risco de bloqueio em cadeia do projeto.
- **C4** (matriz de cinco plataformas): fatiar por plataforma, uma entrada de CI por vez (Fedora primeiro, por ser o ambiente do líder, depois Ubuntu, Arch, CachyOS, Windows), em vez de tratar como uma entrega única que só fecha quando as cinco estiverem verdes ao mesmo tempo.
- **C5** (quatro portões de qualidade): fatiar por ferramenta (`-Werror`, depois ASan/UBSan, depois `clang-tidy`+`cppcheck`, depois `gitleaks`), cada uma integrada e verde antes de somar a próxima, para não represar tudo atrás da mais difícil (ASan/UBSan cross-platform).
- **D5** (combate): fatiar em fila de iniciativa, gestão de recursos, fórmula de dano e a máquina de estados que os costura, cada um com o próprio ciclo TDD, na ordem que a fórmula de dano e a fila de iniciativa nascem antes da máquina de estados que as consome.
- **D12** (contrato comando/evento em cada sistema): em vez de item transversal isolado, recomendo dobrar como critério de aceite de cada item D3 a D10 (cada um já entrega no formato comando/evento ao fechar), e não uma fatia separada no fim, que arrisca virar retrabalho generalizado.
- **E8** (gerador de build): fatiar em (a) compilador de número e regra para tabela dentro do executável, (b) empacotador de texto de sabor em pacote binário selado à parte. São dois artefatos com fronteira jurídica distinta (L-25) e podem evoluir em paralelo depois que o envelope (E1) estiver definido.
- **E9** (save híbrido fase 2): fatiar em (a) formato de registro de comandos encadeado, (b) verificação por re-execução, (c) integração com o save de estado da fase 1. Item de fase 2, não deveria competir por WIP com a fase 1 (E1-E6) antes dela fechar.

## Limite de trabalho em progresso (WIP) recomendado

**2 a 3 itens de código em progresso ao mesmo tempo**, mais no máximo **1 build pesado por vez** (ASan/UBSan, matriz completa de CI).

Justificativa:
- O time é uma pessoa mais agentes (dado do enunciado); paralelismo é só entre arquivos disjuntos, e L-10 exige que todo item de código passe por três mãos (implementa, revisa, orquestrador re-verifica). Cada item em progresso soma fila na revisão e na re-verificação, não só na implementação; um WIP alto empilha gargalo no ponto mais estreito do fluxo, que é a revisão, não a escrita.
- L-19 exige a saída real do vermelho antes do verde em cada item; isso significa que cada unidade em progresso tem duas paradas de verificação (vermelho provado, depois verde provado), o que já dobra o número de pontos de checagem por item comparado a um fluxo sem TDD.
- Builds pesados (ASan/UBSan, a matriz de cinco plataformas) competem por recursos de máquina; rodar mais de um ao mesmo tempo é o tipo de contenção que já causou esgotamento de disco/memória em outros projetos desta casa (inferência baseada em padrão conhecido de máquina única rodando build C++ pesado).
- L-04 (proibido monolito) faz cada feature nominal virar vários arquivos pequenos; um WIP de 2-3 "itens" no sentido da tabela já corresponde a bem mais unidades revisáveis reais, então um número maior no papel esconderia uma fila ainda maior na prática.

## Riscos de capacidade

1. **Gargalo estrutural de revisão em três mãos (L-10)**: com dezenas de itens de código em D e E, cada um exigindo implementador, revisor e re-verificação do orquestrador, o ponto mais estreito do fluxo não é escrever código, é revisar e re-verificar. Se a fila de itens "prontos para revisão" crescer mais rápido que a capacidade de revisão, o WIP recomendado (2-3) já é a mitigação, mas o risco existe mesmo assim se a revisão não acompanhar o ritmo de implementação.
2. **Cinco plataformas desde o primeiro commit (L-09/L-20)**: Windows e CachyOS são as entradas de maior risco, Windows por divergir de toolchain (MSVC) do resto da matriz (GCC/Clang), CachyOS por exigir job próprio sem poder reaproveitar o job Arch. Isso pode fazer a fundação de CI (seção C) consumir mais tempo do que os itens de domínio propriamente ditos.
3. **Dependência externa sem data no GlintFx (LEI ZERO, L-05, L-25)**: E1, E2 (parcialmente), E6, E9 e E10 dependem de primitiva de criptografia que hoje não existe no GlintFx, que por sua vez "tem um header" e não tem data prometida (FATO, contexto imutável do bruto). Isso é risco de capacidade fora do controle do time: a fila de proteção de dado pode travar num ponto que ninguém aqui resolve sozinho, e L-05 proíbe simular a peça que falta.
4. **Decisões pendentes do líder como pré-requisito de trabalho (seção G)**: G1 trava B1, G2 trava o desenho final de E8, G3 trava qualquer trabalho remanescente do sistema de morte antigo, G4 trava parte de F1 e uma ideia do Gus ainda sem item próprio. Se essas decisões demorarem, o efeito é fila parada, não fila lenta.
5. **Canon desatualizado bloqueando trabalho (L-13)**: o esquema de save do corpus (`knowledge-gates.md:380`) está escrito para Godot/GDScript e precisa ser re-derivado antes de D11/E1-E3 avançarem com segurança; `docs/design/producao/*` descrevem pipeline aposentado. Cada re-derivação de canon é trabalho de análise que não estava contado como código, mas precisa acontecer antes.
6. **Volume de julgamento humano em B1 e B6**: 28 casos de redação a distinguir caso a caso e 1266 imagens a amostrar são tarefas de julgamento que não acele com mais agentes em paralelo da mesma forma que código faz; tendem a ser trabalho serializado de atenção, e podem represar atrás de si os itens que dependem de B (nomeadamente, ir a público com o corpus).
7. **Item H2 represado até o fim**: a documentação para iniciante só começa depois da tag de versão (pré-requisito explícito no bruto), então todo o esforço dela cai no fim do fluxo, num momento em que a energia do time tende a estar mais baixa; recomendo tratar como onda dedicada, não encaixe de sobra.
