# Lente de Produto (CoD para WSJF) — GusWorld

Nota metodológica: Valor = quanto o item aproxima de um jogo que existe e é jogável (FOSS, sem monetização, então nunca pontuei por receita). Criticidade = quanto o valor decai se feito depois (janela que fecha), não urgência sentida. Risco/Viabilização = quanto elimina incerteza ou destrava outros itens. Régua só 1,2,3,5,8,13,20. `CoD = Valor + Criticidade + Risco`.

FATO vs INFERÊNCIA: quando a pontuação depende de leitura direta de `GODS_LAWS.md` ou `pillars.md`, é FATO citado. Quando é minha leitura do impacto relativo (ex.: "isto é mais central que aquilo porque X pilar diz Y"), é INFERÊNCIA minha, marcada como tal.

---

## A. Fundação do repositório

A1 | Valor: 2 | Criticidade: 20 | Risco: 8 | CoD: 30 | `.gitignore` não achega o jogo mais perto de jogável, mas L-15 é explícita: "depois do primeiro push, corrigir isto custa reescrita de histórico" (FATO). Risco alto porque evita inflar o repo com 1,3 GB regenerável/pessoal.

A2 | Valor: 2 | Criticidade: 20 | Risco: 20 | CoD: 42 | A marcação `.gitattributes` é o que direciona `docs/_secret/**` para o git-crypt; sem ela, segredo vai em texto claro para repo público (INFERÊNCIA: leio isso como pré-condição técnica do A3, e vazamento é irreversível conforme o enunciado da tarefa).

A3 | Valor: 2 | Criticidade: 20 | Risco: 20 | CoD: 42 | Configurar a chave é o mecanismo que de fato protege conteúdo sensível do líder (que é médico, com áreas pessoais na árvore). Maior risco de todo o grupo A porque sem isto a proteção declarada em A2 não existe de verdade.

A4 | Valor: 1 | Criticidade: 8 | Risco: 5 | CoD: 14 | Corrigir a frase incorreta dentro de arquivo que, se A2/A3 funcionarem, fica cifrado. Criticidade menor que A1-A3 porque o arquivo não fica exposto em claro (INFERÊNCIA, condicionada ao git-crypt funcionar).

A5 | Valor: 3 | Criticidade: 13 | Risco: 5 | CoD: 21 | Fundação legal (L-08) exigida desde o primeiro commit, mas diferente de A1/A2/A3: texto de LICENSE não é dado sensível, dá para corrigir em commit posterior sem reescrever histórico (INFERÊNCIA).

A6 | Valor: 3 | Criticidade: 8 | Risco: 5 | CoD: 16 | Administrativo REUSE, mesma lógica de A5: pode entrar depois sem custo de reescrita.

A7 | Valor: 3 | Criticidade: 8 | Risco: 8 | CoD: 19 | L-08 diz textualmente: "sem isso, ferramenta, distribuição e agregador presumem que tudo no repositório é AGPL" (FATO) — risco real de má-atribuição de licença sobre asset reservado.

A8 | Valor: 3 | Criticidade: 8 | Risco: 5 | CoD: 16 | README com seção de licença e carve-out de marca; administrativo, sem custo de reescrita se atrasar.

A9 | Valor: 2 | Criticidade: 5 | Risco: 3 | CoD: 10 | Texto já redigido pelo CLO (FATO, do item bruto); esforço baixo mas também urgência baixa, não é dado sensível nem legal-crítico.

A10 | Valor: 2 | Criticidade: 5 | Risco: 3 | CoD: 10 | Hoje só lista o GlintFx; administrativo, cresce organicamente, sem custo de atraso relevante.

A11 | Valor: 2 | Criticidade: 13 | Risco: 8 | CoD: 23 | L-08: cabeçalho SPDX "desde o primeiro commit" (FATO). Fazer depois não exige reescrita de histórico (não é dado binário/sensível) mas exige retrofit em todo arquivo, daí criticidade alta sem chegar ao nível de A1-A3.

A12 | Valor: 3 | Criticidade: 20 | Risco: 13 | CoD: 36 | O primeiro commit É a janela que fecha citada no enunciado da tarefa; depende de A1-A11 estarem prontos antes. Risco alto porque destrava literalmente todo o resto (B a E não existem sem repo).

## B. Limpeza do corpus

B1 | Valor: 2 | Criticidade: 13 | Risco: 8 | CoD: 23 | 28 ocorrências de `__DEP_REMOVIDA__` a reparar caso a caso; depende de G1 (aprovação em bloco do líder), portão que ainda não existe.

B2 | Valor: 1 | Criticidade: 20 | Risco: 13 | CoD: 34 | Nome real, biografia e link de foto de pessoa viva num repo que vai ficar público: L-16 exige aceite prévio para homenagem, e o item bruto não indica aceite do Elon Musk (INFERÊNCIA: trato como exposição de dado de terceiro sem consentimento confirmado, mesma classe de irreversibilidade citada na tarefa). Personagem Helion Tusk fica intacto, só o vínculo com a pessoa real sai.

B3 | Valor: 1 | Criticidade: 13 | Risco: 5 | CoD: 19 | Caminho absoluto de máquina (o diretório de usuário do líder, escrito por extenso) expõe usuário e estrutura de disco do líder; menor gravidade que B2 porque não é identidade de terceiro nem dado de saúde, mas ainda é vazamento pessoal evitável.

B4 | Valor: 1 | Criticidade: 5 | Risco: 5 | CoD: 11 | Sete pontas soltas de canon (vocabulário .NET, bloco histórico, títulos de asset 3D, etc.): correção de qualidade documental, sem dado sensível, sem custo de reescrita se atrasar.

B5 | Valor: 3 | Criticidade: 8 | Risco: 13 | CoD: 24 | Decisão sobre `.dlg.txt`/traduções Markdown vs L-18 ("nada em texto na distribuição") é um nó arquitetural real: destrava D9 (diálogo como dado) e parte de E8 (gerador de build). Risco alto por resolver tensão declarada explicitamente na lei.

B6 | Valor: 1 | Criticidade: 13 | Risco: 8 | CoD: 22 | Conferência visual amostral das 1266 imagens antes do primeiro commit: "o método indireto não achou captura de tela, mas nenhum humano olhou" (FATO do item bruto). Criticidade alta porque é checkpoint de pré-commit, mesma lógica de janela do grupo A, mas atinge imagem, não binário sensível já mapeado.

## C. Fundação de build e CI

C1 | Valor: 5 | Criticidade: 8 | Risco: 13 | CoD: 26 | Primeiro esqueleto técnico real (cinco camadas). Sem custo de decaimento externo, mas destrava tudo (C2-C8, D inteiro).

C2 | Valor: 3 | Criticidade: 5 | Risco: 13 | CoD: 21 | Gate de CI que reprova `#include <glintfx/` fora de `present/`: é a garantia mecânica da L-17, reduz risco de violação de arquitetura em vez de confiar em disciplina.

C3 | Valor: 3 | Criticidade: 8 | Risco: 13 | CoD: 24 | Harness de teste sem framework de terceiro é pré-requisito de todo o grupo D (TDD estrito, L-19); sem ele, D não começa.

C4 | Valor: 3 | Criticidade: 8 | Risco: 8 | CoD: 19 | Matriz das cinco plataformas desde o primeiro commit (L-20, FATO: "divergência de plataforma aparece no dia em que nasce, e não em bloco depois de muito código escrito").

C5 | Valor: 3 | Criticidade: 8 | Risco: 13 | CoD: 24 | Os quatro portões de qualidade (L-19) são a rede de segurança de todo o TDD estrito que vem a seguir.

C6 | Valor: 1 | Criticidade: 3 | Risco: 3 | CoD: 7 | Formatação; nem decai nem destrava nada de peso.

C7 | Valor: 2 | Criticidade: 5 | Risco: 8 | CoD: 15 | Script local espelhando CI evita descoberta tardia de falha só no push, reduzindo retrabalho.

C8 | Valor: 2 | Criticidade: 5 | Risco: 8 | CoD: 15 | Hook de TDD do projeto; deveria existir antes do grupo D começar de fato, para não depender só de disciplina.

## D. Núcleo de domínio, TDD estrito

D1 | Valor: 5 | Criticidade: 5 | Risco: 13 | CoD: 23 | Primeiro código de domínio de fato; destrava todo o resto de D.

D2 | Valor: 5 | Criticidade: 5 | Risco: 13 | CoD: 23 | Sorteio determinístico contado é pré-condição de D5 (combate) e D13 (replay); central ao Pilar 1 ("RNG decai com knowledge") conforme `pillars.md`.

D3 | Valor: 8 | Criticidade: 5 | Risco: 8 | CoD: 21 | POCO de carta é o átomo central do sistema de cartas descrito em `pillars.md` como eixo de design (modelo de duas camadas comum/especial). INFERÊNCIA: valor alto porque sem isto D4 e D5 não existem.

D4 | Valor: 8 | Criticidade: 5 | Risco: 8 | CoD: 21 | Deck/mão/coleção com invariantes anti-exploit é mecânica central do Pilar 1 e do modelo de duas camadas do `pillars.md`.

D5 | Valor: 13 | Criticidade: 5 | Risco: 8 | CoD: 26 | Máquina de estados de combate é, por citação direta de `pillars.md`, o próprio Pilar 1 ("Combate... resolvidos por análise, predição e combinação"). Maior valor do grupo D por ser o sistema mais citado como núcleo do jogo.

D6 | Valor: 5 | Criticidade: 3 | Risco: 5 | CoD: 13 | Efeitos de status são extensão natural do combate, mas não o núcleo em si.

D7 | Valor: 5 | Criticidade: 3 | Risco: 5 | CoD: 13 | Economia liga ao sistema-âncora Hospital+Economia do `pillars.md` (Pilar 4), mas é sistema de suporte, não o eixo central.

D8 | Valor: 8 | Criticidade: 3 | Risco: 5 | CoD: 16 | Knowledge Progression é citado em `pillars.md` como diferencial explícito anti-grind ("substitui grind"), por isso valor alto apesar de não ser combate em si.

D9 | Valor: 3 | Criticidade: 3 | Risco: 5 | CoD: 11 | Diálogo como dado é suporte narrativo; depende de B5 ainda não resolvido.

D10 | Valor: 8 | Criticidade: 5 | Risco: 8 | CoD: 21 | Mapa como dado com colisão por célula liga direto à L-26 (perspectiva 3/4 top-down, grade quadrada), que é decisão canônica já fechada e citada como custosa de reverter.

D11 | Valor: 5 | Criticidade: 5 | Risco: 8 | CoD: 18 | Modelo de save canônico é pré-requisito de todo o grupo E (envelope, serialização).

D12 | Valor: 3 | Criticidade: 5 | Risco: 8 | CoD: 16 | Contrato comando/evento aplicado a cada sistema é garantia arquitetural transversal exigida pela L-17.

D13 | Valor: 5 | Criticidade: 5 | Risco: 13 | CoD: 23 | Teste de replay determinístico é a prova exigível citada na própria L-17 ("O que a lei entrega, e que passa a ser exigível: replay byte a byte"); risco alto por ser o detector permanente de quebra de determinismo.

D14 | Valor: 3 | Criticidade: 1 | Risco: 3 | CoD: 7 | Auto-resolve de combate é ferramenta de balanceamento; sem D5 pronto, não há o que balancear ainda. Valor real menor agora (INFERÊNCIA).

## E. Proteção de dado

E1 | Valor: 8 | Criticidade: 5 | Risco: 13 | CoD: 26 | Envelope binário é a espinha comum de save, config, mapa e catálogo (L-25); maior valor do grupo E por servir a tudo.

E2 | Valor: 5 | Criticidade: 5 | Risco: 8 | CoD: 18 | Serialização versionada dos POCOs é consequência direta de E1.

E3 | Valor: 5 | Criticidade: 3 | Risco: 8 | CoD: 16 | Validador semântico é, por L-25, "o único nível que sobrevive à chave vazada" (FATO) — risco alto por ser a camada de defesa mais duradoura.

E4 | Valor: 3 | Criticidade: 3 | Risco: 8 | CoD: 14 | Gravação atômica e backups evitam perda de save, risco operacional relevante.

E5 | Valor: 3 | Criticidade: 3 | Risco: 5 | CoD: 11 | Configuração selada nunca bloqueia o boot (trava operacional da L-25); importante mas específico.

E6 | Valor: 2 | Criticidade: 3 | Risco: 8 | CoD: 13 | Ferramenta de inspeção/re-selo: L-25 exige que nasça junto com o formato, "sem ela ninguém enxerga dentro de save, mapa ou pacote quando algo dá errado" (FATO) — risco de desenvolvimento alto.

E7 | Valor: 1 | Criticidade: 5 | Risco: 8 | CoD: 14 | Registrar no bus a necessidade de cripto ao GlintFx destrava toda a cadeia de criptografia de E (L-07/L-25: primitivas vêm do GlintFx, nada de terceiro). Criticidade moderada porque só se pede "quando o jogo esbarrar de fato na falta" (FATO, L-07), não antes.

E8 | Valor: 5 | Criticidade: 3 | Risco: 8 | CoD: 16 | Gerador de build que separa número/regra (código) de texto de sabor (asset) implementa a fronteira jurídica da L-25; sem ele o binário final se contradiz.

E9 | Valor: 3 | Criticidade: 1 | Risco: 5 | CoD: 9 | Fase 2 explicitamente posterior no faseamento da L-25; valor real menor agora.

E10 | Valor: 1 | Criticidade: 1 | Risco: 2 | CoD: 4 | Fase 3, "opcional... nunca requisito para jogar" (FATO, L-25); menor CoD do levantamento inteiro.

E11 | Valor: 2 | Criticidade: 1 | Risco: 3 | CoD: 6 | Amarra de máquina só no slot Hardcore; nicho, sem urgência.

## F. Ideias do Gus Dragon

F1 | Valor: 3 | Criticidade: 2 | Risco: 3 | CoD: 8 | Carta `glitch` com regra de atravessamento e duração; ainda tem dois pontos em aberto devolvidos a ele (FATO do item bruto), então não está pronta para implementar sem G4.

F2 | Valor: 3 | Criticidade: 2 | Risco: 3 | CoD: 8 | Carta `urandom` em duas versões (pirata/original); conteúdo específico de carta, mesma ordem de grandeza de F1.

F3 | Valor: 3 | Criticidade: 2 | Risco: 3 | CoD: 8 | Vírus zip-bomb já tem ficha técnica; falta só o efeito de batalha, esforço contido.

F4 | Valor: 5 | Criticidade: 2 | Risco: 5 | CoD: 12 | Missão com relógio real correndo, inclusive em batalha, é mais estrutural que uma carta isolada (toca o fluxo de missão inteiro); risco de design a observar (ver seção de itens superestimados).

Nota sobre o grupo F, além do CoD: a régua acima mede só "quanto aproxima de jogo jogável". Ela não captura o valor relacional de honrar rápido uma ideia aprovada do filho do líder (L-07 manda entrada imediata na tabela, "nunca minta pra ele"). Esse valor existe e é real, mas é de outra natureza que a CoD não deveria fingir capturar; melhor deixá-lo explícito em texto do que inflar o número.

## G. Pendências de decisão do líder (portões, não trabalho)

Aviso: o item bruto classifica G1-G4 como "não são trabalho, são portões". Pontuei porque a instrução pede nota para cada item da lista, mas o resultado deve ser lido como "custo de atraso de uma decisão pendente", não como esforço de produto.

G1 | Valor: 2 | Criticidade: 8 | Risco: 8 | CoD: 18 | Aprovação em bloco destrava B1 inteiro (28 ocorrências); sem ela, B1 não começa.

G2 | Valor: 3 | Criticidade: 3 | Risco: 8 | CoD: 14 | Escolha de ferramenta/formato do gerador de conteúdo destrava E8 e parte do pipeline de conteúdo.

G3 | Valor: 2 | Criticidade: 3 | Risco: 5 | CoD: 10 | Confirmar o que sobrevive do desenho antigo de modos de morte resolve contradição de canon (L-13) que bloqueia trabalho dependente.

G4 | Valor: 2 | Criticidade: 3 | Risco: 3 | CoD: 8 | Decidir o destino das duas ideias do Gus (bateria na parede; glitch em batalha) destrava a conclusão de F1 e uma futura ideia irmã ainda não listada como item F próprio.

## H. Documentação de fechamento

H1 | Valor: 1 | Criticidade: 1 | Risco: 1 | CoD: 3 | Wiki do GitHub; regra permanente da casa, mas pré-requisito (tag de versão) nem existe e o jogo não tem tela. Valor real hoje é mínimo.

H2 | Valor: 1 | Criticidade: 1 | Risco: 1 | CoD: 3 | Documentação para iniciante; mesmo raciocínio de H1, com o mesmo pré-requisito de tag de versão ainda inexistente.

## I. Testes e auditorias (TST-*, AUD-*)

Não pontuado: o item bruto diz que "os itens `TST-*` e `AUD-*` vêm do catálogo, podados para este stack" via a skill, mas a lista bruta que recebi não traz IDs concretos, só a menção de que existem `TESTES.md` e `AUDITORIAS.md`. Sem ID individual não há o que pontuar nesta lente; quando a skill injetar os IDs reais, esta lente precisa rodar de novo sobre eles.

---

## Os dez maiores CoD

1. A2 (42) — marcação `.gitattributes` para LFS e git-crypt do `docs/_secret/**`.
2. A3 (42) — configurar git-crypt com chave fora da árvore.
3. A12 (36) — configurar remoto e primeiro commit.
4. B2 (34) — remover nome real/biografia/foto de pessoa viva do corpus.
5. A1 (30) — `.gitignore` de `resources/livros/` e `resources/glb/`.
6. C1 (26) — `CMakeLists.txt` raiz com as cinco camadas.
6. D5 (26) — máquina de estados de combate (empate com C1 e E1).
6. E1 (26) — envelope binário do projeto (empate com C1 e D5).
9. B5 (24) — decidir tensão `.dlg.txt`/traduções Markdown vs L-18.
9. C3 (24) — harness de teste próprio, sem framework de terceiro (empate com B5 e C5).
9. C5 (24) — os quatro portões de qualidade no CI (empate com B5 e C3).

Nota: há empate triplo em 26 (posições 6-8) e empate triplo em 24 (posições 9-11), então a lista "dez maiores" tem de fato 11 itens distintos nessas posições. Não desempatei artificialmente porque a pontuação nas três dimensões foi honesta e o empate é real dado o método; o desempate final, se necessário, é decisão de quem consolida (Cosmo/COO), não desta lente.

## Itens cujo valor considero superestimado pela lista bruta

- **A5-A10 (LICENSE, LICENSES/, REUSE.toml, README, OFFLINE-NOTICE, THIRD-PARTY-LICENSES)**: o item bruto agrupa todo o bloco A sob o cabeçalho "tudo isto ANTES do primeiro commit", como se tivessem a mesma urgência de A1/A2/A3/A11. Na minha leitura (INFERÊNCIA), só A1, A2, A3 e A11 têm a propriedade real de "corrigir depois custa reescrever histórico" (dado binário, sensível ou marcação legal presumida por ferramenta desde o primeiro arquivo). A5-A10 são documentos administrativos comuns: podem entrar num commit seguinte, sem force-push, sem custo estrutural. Recomendo não tratar o bloco A como bloco monolítico de urgência.

- **F4 (missão com relógio correndo, inclusive em batalha)**: pontuei valor 5 (mais alto que F1-F3) porque é estrutural, mas há um risco de design que a lista bruta não sinaliza: `pillars.md`, Pilar 1, diz explicitamente "Active turn-based **sem timer no turno do jogador**". Um relógio real correndo "inclusive durante as batalhas" pode entrar em tensão direta com esse anti-mecanismo se não for cuidadosamente escopado como relógio de mundo/missão e não timer de turno. Não é motivo para reduzir o valor da ideia do Gus, mas é motivo para não tratá-la como pronta para implementar sem esse alinhamento com o pilar (isto se soma a G4, que já sinaliza pendência de decisão para outra ideia dele).

- **D14 (auto-resolve de combate) e E9/E10 (fases 2 e 3 de save)**: aparecem listados lado a lado com os itens centrais dos seus grupos (D1-D13, E1-E8), o que pode passar a impressão de mesma prioridade. Na prática são corretamente de segunda ordem: D14 não tem o que balancear antes de D5 existir e ter conteúdo suficiente; E9/E10 são, pelo próprio texto da L-25, fases posteriores e a última delas é explicitamente opcional e nunca requisito para jogar.

- **H1/H2 (Wiki e documentação para iniciante)**: é regra permanente da casa e não deve ser removida da tabela, mas o valor hoje é o mais baixo do levantamento inteiro (CoD 3, empatado entre os dois): o pré-requisito declarado (tag de versão) nem existe, e sem o GlintFx não há tela para documentar ou mostrar. Reflete corretamente uma janela que ainda nem abriu.

## Sugestão de trabalho faltante (não virou item, é só sugestão)

- Não vi na lista bruta um item explícito para **editar `docs/design/pillars.md:27`** ("Cel-shaded 3D low-poly" contra L-02), citado na própria L-13 como contradição que bloqueia trabalho dependente. B4 lista sete pontas soltas mas nenhuma delas parece ser esta linha específica de `pillars.md`.
- Também não vi item explícito para a contradição de `docs/design/mecanicas/core-loop-exploracao.md:95` (DA-1, câmera orbital), que a própria L-13 diz estar "resolvida pela L-26" mas com o texto revogado ainda por apagar (a L-24 exige apagar, não arquivar).
- Não vi item para **atualizar o `CONTRACT.md` §5.1**, que a L-22 diz estar substituído (função em pt-br, `m_`, ALL_CAPS revogados), mas o texto revogado em si precisa ser apagado do `CONTRACT.md` por força da L-24.
- Não vi item para **re-derivar o esquema de save** hoje descrito em `docs/narrative/diary/knowledge-gates.md:380` para Godot/GDScript (apontado pela própria L-13 como precisando de re-derivação).

Estas quatro observações são só sinalização; não estimei esforço nem CoD para elas porque não são itens da lista bruta que recebi e a instrução veda criar item novo.
