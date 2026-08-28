# ADR-025: Harness de teste próprio e mínimo (e correção da leitura da LEI ZERO)

| | |
|---|---|
| **Status** | Accepted (decisão do líder, 25/08/2026) |
| **Data** | 2026-08-25 |
| **Decisor** | petrinhu (líder supremo) |
| **Reversibilidade** | Two-way door — nenhum teste de comportamento existe ainda (L-19 só começa no primeiro módulo real); adotar um framework de terceiro depois é possível, mas custa reescrever toda asserção já feita contra a API própria. |

## Contexto, e o erro que motivou esta ADR

O item `C3` do `TODO.md` descrevia o harness de teste do GusWorld como
"harness de teste próprio, sem framework de terceiro **(LEI ZERO)**". Antes
de aceitar essa descrição como fundação, a LEI ZERO foi relida por inteiro.
**Ela não menciona teste nenhuma vez.** A atribuição era inferência de um
agente anterior, não decisão do líder, e essa inferência tinha custo real: a
lente de esforço aplicada ao item `C3` devolveu **Job Size 13** — o topo da
régua — porque "sem framework de terceiro, LEI ZERO" foi lido como licença
para especificar um framework de teste completo (descoberta automática,
relatório, matchers), quando a decisão real do líder, dada nesta ordem de
serviço, é muito mais estreita.

## A leitura correta da LEI ZERO

O texto verbatim da LEI ZERO diz que "o GusWorld **liga** em exatamente duas
coisas: o GlintFx e o sistema operacional", e detalha: "janela, laço
principal, entrada, áudio e desenho são do GlintFx". A palavra-chave é
**liga** — no sentido de *link*/conexão em tempo de execução, sobre o que o
**executável do jogo distribuído** consome quando roda.

CMake é ferramenta de tempo de **build**: nunca é linkada em nenhum binário,
não existe depois que o executável está pronto. Um harness de teste produz
um **executável de teste separado**, que nunca embarca no jogo distribuído
e nunca liga em nada que o jogador rode. **Nem um, nem outro "ligam" no
sentido que a LEI ZERO usa** — ela simplesmente não fala sobre ferramenta de
build nem sobre framework de teste, nem para proibir, nem para exigir
independência de terceiros. Aplicar a LEI ZERO a este item foi leitura
errada, e fica registrado aqui para que nenhum agente futuro repita.

## De onde vem, de fato, a cultura de "sem framework de terceiro"

Não do GusWorld: **da L-07 do `GODS_LAWS.md` do GlintFx**, texto verbatim
daquele projeto: *"Zero dependência além da biblioteca padrão de C++23 e das
APIs do sistema operacional. Sem gerenciador de pacote de terceiros, sem
`FetchContent` de biblioteca externa, sem vendorizar."* O GlintFx aplica essa
lei a si mesmo, inclusive ao **próprio harness de teste**: `tests/CMakeLists.txt`
do GlintFx monta uma biblioteca `glintfx_test_harness` a partir de três
arquivos escritos em casa — `harness/test_registry.cpp`,
`harness/check.cpp`, `harness/harness_main.cpp` — sem
framework de teste de terceiro. Este é o precedente real, medido, que
justifica a decisão de hoje: não é lei do GusWorld, é harmonia deliberada
com o costume já provado do projeto do qual o GusWorld depende.

## Decisão

**O GusWorld escreve um harness de teste próprio, e MÍNIMO.** Mínimo
significa, com todas as letras, o que a decisão cobre e o que ela não
cobre:

- **Cobre:** um mecanismo de asserção (falha visível e localizável), um
  registro de casos de teste, e um `main` que executa o registro e reporta
  sucesso/falha ao processo (código de saída não-zero em qualquer falha).
- **Não cobre — e não é desta ADR decidir:** descoberta automática de teste
  em disco, relatório elaborado (XML/JUnit, cores, sumário estatístico),
  filtro de teste por nome/tag, paralelização, ou qualquer funcionalidade
  que um framework maduro ofereceria. Nenhuma dessas é vetada para sempre;
  simplesmente não fazem parte do que se decide agora, e o design concreto
  do mecanismo é trabalho da fatia `C3`, não desta ADR.

## Por que "próprio" sem virar "próprio framework"

O risco real de "vamos escrever nosso harness" é escopo inflado — foi
exatamente o que aconteceu na primeira leitura do `C3` (Job Size 13). A
decisão do líder aqui é explícita sobre o tamanho: dependência zero
continua, mas o harness não vira um produto à parte dentro do projeto. Ele
existe para servir o TDD estrito da L-19 (vermelho → verde → refatorar) com
o mínimo que isso exige: rodar um teste, ver se ele falha, ver se ele passa.

## Alternativas consideradas

### A. Adotar um framework de teste C++ maduro de terceiro

**Prós:** descoberta automática, relatórios prontos, matchers expressivos,
comunidade e manutenção de terceiros.
**Contras:** quebra a cultura de dependência zero que o próprio framework do
qual o jogo depende (GlintFx) já pratica e já provou funcionar até para o
próprio harness dele; introduz uma dependência de build (mesmo que só para
teste) num projeto cuja LEI ZERO — ainda que não fale de teste — estabelece
um tom de autossuficiência que o líder reafirmou ao aprovar esta decisão.
**Rejeitada.**

### B. Harness próprio, completo (framework interno com descoberta, relatório, filtro)

**Prós:** nenhum, frente à opção mínima — tudo que um framework completo
ofereceria aqui é trabalho não pedido.
**Contras:** é o escopo inflado que gerou o Job Size 13 errado; constrói
infraestrutura que o TDD estrito da L-19 não está pedindo hoje
(anti-over-engineering).
**Rejeitada.**

### C. Harness próprio, mínimo — asserção, registro, `main` (escolhida)

**Prós:** mantém a dependência zero sem escrever um framework; espelha
exatamente o precedente já provado no GlintFx (`check.cpp` +
`test_registry.cpp` + `harness_main.cpp`); escopo do tamanho real da
necessidade (rodar TDD, não substituir uma indústria de ferramentas de
teste).
**Contras:** sem descoberta automática, cada novo arquivo de teste precisa
ser registrado explicitamente em algum lugar (`tests/CMakeLists.txt` ou
equivalente) — custo pequeno, pago uma vez por arquivo de teste, não por
caso de teste.
**Aceita.**

## Consequências

**Positivas:**
- Corrige a estimativa de esforço do item `C3` de Job Size 13 para algo
  compatível com "escrever três arquivos pequenos", quando essa fatia for
  planejada de fato.
- Mantém uma única cultura de dependência entre os dois repositórios
  irmãos, sem inventar uma segunda regra só para teste.
- Nenhuma decisão de desenho do harness foi tomada aqui — fica inteira para
  a fatia `C3`, com liberdade de espelhar (ou não) a API exata do GlintFx.

**Negativas / aceitas como custo:**
- Sem os anos de maturidade de um framework de terceiro: nenhum matcher
  pronto, nenhum relatório de CI colorido out-of-the-box. Se o CI precisar
  de saída em formato específico (JUnit XML, por exemplo, para alguma
  integração futura), isso é trabalho adicional próprio, não herdado de
  graça.
- Registro manual de arquivo de teste é um passo a mais que um framework
  com descoberta automática dispensaria.

## Riscos / pontos de atenção

- Nenhum agente deve desenhar a API do harness a partir desta ADR — ela
  decide "próprio e mínimo", não a forma exata das funções. Isso é a fatia
  `C3`.
- Ao despachar a fatia `C3`, citar esta ADR e a L-19 (TDD estrito, cinco
  portões de qualidade) no prompt da task, para que o desenho do harness
  nasça já compatível com o ciclo vermelho/verde/refatorar exigido.

## Reversibilidade

Two-way door. Nenhum teste de comportamento existe hoje (L-19 só começa no
primeiro módulo real); trocar de harness próprio para um framework de
terceiro mais tarde é possível, mas custa reescrever toda asserção já
escrita contra a API própria — custo que cresce com o tempo, não zero.

## Cross-refs

- [ADR-022](ADR-022-cmake-versao-minima-fixada.md), [ADR-023](ADR-023-glintfx-submodulo-git-pinado.md), [ADR-024](ADR-024-ci-cinco-plataformas-cachyos-container.md) (ADRs irmãos desta mesma ordem de serviço, razões de mudar distintas por L-33)
- `../../../GODS_LAWS.md` LEI ZERO (a leitura corrigida), L-05, L-19 (TDD estrito, cinco portões), L-33
- `../../../GlintFx/GODS_LAWS.md` L-07 (a lei real de onde vem a cultura de dependência zero)
- `../../../GlintFx/tests/CMakeLists.txt`, `../../../GlintFx/tests/harness/` (o precedente medido: `test_registry.cpp`, `check.cpp`, `harness_main.cpp`)
