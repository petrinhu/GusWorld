# ADR-023: GlintFx entra no build como submódulo git, pinado num commit, e linkado compartilhado

| | |
|---|---|
| **Status** | Accepted (decisão do líder, 25/08/2026) |
| **Data** | 2026-08-25 |
| **Decisor** | petrinhu (líder supremo) |
| **Reversibilidade** | Two-way door quanto ao mecanismo de consumo (submódulo, `FetchContent`, vendorizado) e quanto ao modo de vínculo (compartilhado, estático) — ambos são flags de `CMakeLists.txt`, reversíveis sem retrabalhar lógica de jogo; nenhum código de jogo hoje depende de nenhum dos dois. |

## Contexto

A LEI ZERO do `GODS_LAWS.md` fixa que o GusWorld "liga em exatamente duas
coisas: o GlintFx e o sistema operacional", e cita verbatim o líder: "aceita
apenas link com framework GlintFx em `../Glintfx` e `[github]/petrinhu/GlintFx`
e com o SO". Isso decide **de onde** o código vem. Não decidia **como o build
do GusWorld busca e fixa** esse código — se por submódulo git, por
`FetchContent` do CMake, por um gerenciador de pacote C++, ou por cópia
vendorizada manual. Essa lacuna é distinta da decisão de versão mínima de
CMake (ADR-022): uma é "qual ferramenta de build", a outra é "como uma
dependência de outro repositório do mesmo líder entra nessa ferramenta" —
razões de mudar diferentes, por isso ADRs diferentes (L-33).

Os dois repositórios (GusWorld e GlintFx) são do mesmo autor e evoluem juntos,
em sessões e dias diferentes. Sem um mecanismo de fixação explícito, o
GusWorld compilaria contra "o que quer que o GlintFx seja hoje" — e
divergência de comportamento entre uma sessão e outra vira impossível de
diagnosticar: qual dos dois repositórios mudou primeiro?

## Decisão

**O GlintFx entra no build do GusWorld como submódulo git, pinado num commit
específico, em `framework/GlintFx`.** `git submodule add
https://github.com/petrinhu/GlintFx.git framework/GlintFx`, seguido de
`add_subdirectory(framework/GlintFx)` no `CMakeLists.txt` do GusWorld — o
mesmo mecanismo de consumo que os testes `tests/embed/` e
`tests/embed_name_collision/` do próprio GlintFx já exercitam e provam
funcionar (`add_subdirectory`/`FetchContent` embutido, ver `FIX-CONSUMO` nos
comentários do `CMakeLists.txt` raiz do GlintFx). A razão do nome
`framework/`, escolhido pelo líder em vez das três convenções oferecidas,
está na seção "Caminho do submódulo" abaixo.

**Atualizar o ponteiro do submódulo é ato deliberado**, nunca automático:
avançar para um commit novo do GlintFx é um commit próprio no GusWorld
(`git submodule update --remote` seguido de `git add framework/GlintFx`),
revisável e reversível como qualquer outro.

## Por que reprodutibilidade exata, dita sem retórica

Sem pino, dois clones do mesmo commit do GusWorld em dias diferentes podem
compilar contra GlintFx diferentes, porque `main` do GlintFx pode ter
avançado entre um clone e outro. Com pino, **o mesmo commit do GusWorld
sempre compila contra o mesmo commit do GlintFx**, em qualquer máquina, em
qualquer data. Quando algo quebra, a pergunta "o que mudou" tem resposta
mecânica: `git log` do ponteiro do submódulo mostra exatamente quando o
GusWorld pediu uma versão nova do framework, e o commit do lado do GlintFx
mostra o que mudou lá.

## Vínculo: compartilhado (`.so`), espelhando o GlintFx (decisão do líder, 25/08/2026)

**O GusWorld linka o GlintFx compartilhado (`BUILD_SHARED_LIBS=ON` ao configurar
o submódulo, produzindo `libglintfx.so`/`glintfx.dll` em vez de arquivo
estático), o mesmo modo que o próprio GlintFx usa por padrão.**

⚠️ **Registro honesto: esta não foi a recomendação técnica dada ao líder.**
A recomendação era **estático** — linkar o GlintFx como biblioteca estática
mataria inteira uma classe de bug de ABI (ver seção seguinte), porque não
haveria fronteira de `.so` nenhuma para a incompatibilidade atravessar dentro
do binário final do jogo. **O líder ouviu o argumento e decidiu, conscientemente,
seguir compartilhado — o mesmo padrão que o framework usa para si mesmo.**
Não é uma omissão nem uma leitura equivocada: é a decisão dele, registrada
com a alternativa e o motivo de ela ter sido preterida, como manda a boa
prática de ADR.

### Por que o GlintFx é compartilhado por padrão

Lido em `../GlintFx/GODS_LAWS.md`, a lei que fixa a arquitetura de camadas do
framework declara, verbatim: *"A superfície pública é opaca. A biblioteca é
compartilhada por padrão, então toda classe pública com layout visível ou
método virtual vira contrato de ABI."* É uma decisão estrutural do GlintFx
sobre como ele mesmo se constrói e se testa (`tests/embed_dll_colocation/`,
citado no ADR-022 e nesta ADR, existe justamente para provar colocação de
`.dll` num consumo compartilhado no Windows).

### A interação que importa: dois compiladores (ADR-024) + `.so` compartilhado

A decisão 5 do líder (ADR-024: GCC **e** Clang, os dois, obrigatórios no
Linux) e esta decisão (GlintFx compartilhado) se cruzam, e o cruzamento
precisa de resposta explícita, não de nota de rodapé.

**Onde a nota de ABI do GlintFx mora, lida diretamente:** `../GlintFx/GODS_LAWS.md`
L-22, verbatim: *"Nenhuma exceção cruza a API pública [...] Por quê, e o
motivo é de ABI, não de gosto: a biblioteca é compartilhada por padrão (L-19).
Exceção atravessando `.so` exige RTTI compatível entre a lib e o consumidor,
e quebra quando os dois foram compilados por compiladores ou bibliotecas
padrão diferentes."*

**Dentro do nosso CI, a ABI fica coerente por construção, e é preciso dizer
por quê:** o GlintFx entra como submódulo (esta ADR) e é **compilado no mesmo
build** que o jogo, dentro de cada entrada da matriz (ADR-024). Cada entrada
da matriz usa **um único compilador** para compilar tanto a biblioteca quanto
o executável do jogo (`-DCMAKE_CXX_COMPILER=${{ matrix.cc }}` aplicado ao
`add_subdirectory` inteiro, a mesma técnica que o próprio GlintFx usa em
`../GlintFx/.github/workflows/ci.yml:212`). Não existe, em nenhuma das nove
entradas da matriz (ADR-024), uma combinação onde o GlintFx nasce de um
compilador e o jogo de outro. **O nosso CI nunca exercita o cenário de
quebra**, porque ele nunca constrói a combinação que quebraria.

**Mas o jogo é FOSS sob AGPL-3.0-or-later (L-08), e quem compila da fonte
fora do nosso CI não está sujeito a essa garantia.** O cenário real de
quebra: alguém instala um `libglintfx.so` empacotado por uma distro
(construído com GCC, por exemplo) e compila o GusWorld com Clang contra esse
`.so` já pronto — ou o inverso. Se uma exceção atravessar a fronteira do
`.so` nessas condições, a nota do GlintFx se aplica ponto a ponto: RTTI
incompatível entre os dois lados, comportamento indefinido.

**O que protege, dito com precisão, sem prometer o que não se garante:** o
submódulo pinado (esta ADR) garante que **o GusWorld, compilado do jeito que
o próprio repositório prescreve** (submódulo, mesma entrada de matriz, mesmo
compilador dos dois lados), está livre desse risco — é uma garantia sobre
**o nosso processo de build**, não sobre qualquer combinação hipotética que
um terceiro decida montar. Quem sai desse caminho — linkar o jogo contra um
GlintFx pré-compilado por um compilador diferente do que compilou o jogo —
assume o risco de ABI que a lei do GlintFx já nomeia. Isto não é uma
lacuna de suporte: é a mesma fronteira de responsabilidade que qualquer
biblioteca compartilhada C++ impõe a quem a consome fora do processo de
build que a testa.

**O que esta ADR NÃO decide, porque é escopo do líder:** se o GusWorld deve
um dia oferecer (ou proibir) consumo de um GlintFx pré-compilado por
terceiro, se o `README` deve trazer aviso explícito sobre misturar
compiladores entre jogo e framework, ou se uma futura verificação em tempo
de carregamento (ex.: um símbolo de versão de ABI checado no `dlopen`/carga)
deve existir. Nenhuma política de suporte foi inventada aqui.

### Alternativas de vínculo consideradas

**A. Estático (`BUILD_SHARED_LIBS=OFF`) — a recomendação técnica original.**
**Prós:** elimina inteiramente a classe de bug de ABI entre compiladores
descrita acima — sem `.so`, não há fronteira de RTTI para atravessar; um
único binário final, sem `libglintfx.so`/`.dll` para distribuir ou versionar
à parte.
**Contras:** diverge do modo em que o próprio GlintFx se testa e se
documenta por padrão; perde a superfície de teste que o GlintFx já construiu
especificamente para o modo compartilhado (`tests/embed_dll_colocation/`,
colocação de `.dll` no Windows).
**Rejeitada pelo líder, conscientemente, depois de ouvir o argumento.**

**B. Compartilhado (`BUILD_SHARED_LIBS=ON`), espelhando o GlintFx — escolhida.**
**Prós:** mesmo padrão do framework; exercita, também no GusWorld, os
caminhos de teste que o GlintFx já construiu para consumo compartilhado.
**Contras:** reabre a classe de bug de ABI entre compiladores — mitigada
dentro do nosso CI pela coerência de compilador único por entrada de
matriz (ver acima), não eliminada para quem compila fora dele.
**Aceita.**

## Alternativas consideradas (mecanismo de consumo)

### A. `FetchContent` do CMake, apontando para uma tag/commit do GlintFx

**Prós:** não exige comando `git submodule` separado; quem faz `git clone`
comum já recebe tudo via `cmake -S . -B build` (com `FETCHCONTENT_SOURCE_DIR`
opcional para apontar a um clone local).
**Contras:** baixa o GlintFx de novo a cada `build/` limpo (custo de rede e
tempo repetido, sem cache local por padrão); o ponteiro de versão fica
enterrado dentro do `CMakeLists.txt` em vez de ser um objeto de primeira
classe do git, que é como o líder já pensa nos dois repositórios ("evoluem
juntos").
**Rejeitada**, mas registrada porque o GlintFx já tem teste próprio
(`tests/package/`) provando que também funciona por este caminho — não é
mecanismo incompatível, é a segunda melhor opção.

### B. Gerenciador de pacote C++ (Conan, vcpkg)

**Prós:** ecossistema maduro de versionamento e cache binário.
**Contras:** GlintFx não publica pacote nesses gerenciadores (nem é objetivo
dele fazê-lo, por ser um framework em desenvolvimento ativo do mesmo autor);
adotar um gerenciador de pacote para consumir UM único repositório privado ao
autor é overengineering — resolve um problema (descoberta e distribuição
para terceiros) que não existe aqui.
**Rejeitada.**

### C. Cópia vendorizada manual (`cp -r` do GlintFx para dentro do GusWorld)

**Prós:** nenhuma dependência de ferramenta.
**Contras:** viola a reprodutibilidade que motivou a pergunta — não há
registro automático de qual commit do GlintFx foi copiado, atualizar vira
um `diff` manual sujeito a erro, e o histórico de "quem mudou o quê" se
perde. É exatamente o "dublê" que a L-05 do `GODS_LAWS.md` proíbe em
espírito, ainda que a L-05 fale de runtime, não de build: um vendor manual
sem rastreio é uma cópia que finge ser a coisa original.
**Rejeitada.**

### D. Submódulo git, pinado num commit (escolhida)

**Prós:** reprodutibilidade exata e nativa do git; o ponteiro do submódulo
é ele mesmo um objeto versionado, revisável em `git diff`/`git log`;
atualizar é um ato visível e deliberado, nunca silencioso; combina bem com
`add_subdirectory` já provado pelos testes `tests/embed*` do GlintFx.
**Contras:** exige que quem clona o GusWorld rode
`git submodule update --init --recursive` (ou `git clone --recurse-submodules`)
— um passo a mais documentado no `README`, não automático por padrão do git.
**Aceita.**

## Consequências

**Positivas:**
- Todo commit do GusWorld aponta para um GlintFx exato; bisecção de bug entre
  os dois repositórios vira mecânica, não arqueologia.
- Atualizar o framework é decisão visível (um commit dedicado no GusWorld),
  nunca acontece "de graça" ao puxar `main` do GlintFx.
- Consistente com `add_subdirectory`, já testado e funcionando dentro do
  próprio GlintFx (`tests/embed/`, `tests/embed_name_collision/`,
  `tests/embed_dll_colocation/`).
- Vínculo compartilhado exercita, no GusWorld, a mesma superfície de teste
  que o GlintFx já mantém para consumo compartilhado — nenhum caminho de
  teste do framework fica órfão de um consumidor real.
- Dentro do nosso CI, ABI entre GlintFx e jogo é coerente por construção:
  compilador único por entrada de matriz (ADR-024), submódulo compilado no
  mesmo build.

**Negativas / aceitas como custo:**
- Clone "ingênuo" (`git clone` sem `--recurse-submodules`) deixa o diretório
  do GlintFx vazio até rodar `git submodule update --init`; precisa constar
  no `README` e, idealmente, num script de bootstrap.
- Submódulo git tem reputação de superfície de erro humano (esquecer de
  commitar o avanço do ponteiro, ou commitar sem querer um ponteiro "solto"
  fora de qualquer branch do GlintFx). Mitigação: revisão de PR sempre olha
  o diff do ponteiro do submódulo como qualquer outra mudança de linha.
- **Vínculo compartilhado reabre a classe de bug de ABI entre compiladores**
  que o vínculo estático teria eliminado por completo (recomendação original,
  preterida conscientemente pelo líder). Mitigado dentro do nosso CI; **não**
  mitigado para quem monta a combinação GlintFx-de-um-compilador +
  jogo-de-outro fora dele — risco assumido, não uma lacuna descoberta depois.

## Caminho do submódulo: `framework/GlintFx`, decidido pelo líder em 25/08/2026

O caminho exato do submódulo dentro da árvore do GusWorld não tinha sido
decidido na primeira leitura desta ADR — três convenções comuns foram
oferecidas ao líder (`external/GlintFx`, `vendor/GlintFx`, ou `GlintFx/` na
raiz, espelhando `../GlintFx` do ambiente de desenvolvimento).

**O líder não escolheu nenhuma das três. Escolheu `framework/GlintFx`.**

**A razão, registrada porque é boa e vale para qualquer consumidor futuro do
GlintFx, não só para este projeto:** `vendor/` sugere fornecedor de
terceiro — uma dependência de fora, cuja evolução não é responsabilidade de
quem a consome. O GlintFx **não é isso**: é projeto irmão deste, do mesmo
autor, e os dois evoluem juntos (ver "Por que reprodutibilidade exata"
acima). `external/` tem o mesmo problema em grau menor — descreve
localização, não natureza. `framework/` diz exatamente o que a pasta é, e
mantém legível, pelo próprio nome do diretório, a fronteira que a LEI ZERO
protege: o GusWorld liga em exatamente duas coisas, e uma delas mora num
diretório cujo nome não deixa dúvida sobre qual das duas é.

**Aplicação:** `git submodule add https://github.com/petrinhu/GlintFx.git
framework/GlintFx`, e `add_subdirectory(framework/GlintFx)` no
`CMakeLists.txt` raiz do GusWorld.

## Riscos / pontos de atenção

- Enquanto o GlintFx muda rápido (11 dias, 359 commits medidos em outro
  projeto irmão — ver memória de sessão), o GusWorld pode ficar "atrasado"
  de propósito, esperando uma função específica (L-07: pedido só quando o
  jogo esbarrar de fato na falta). Isso é o comportamento desejado, não um
  bug do mecanismo de pino.
- CI precisa rodar `git submodule update --init --recursive` (ou
  `actions/checkout` com `submodules: true`/`recursive`) em toda entrada da
  matriz — item para a ordem de serviço que escrever o workflow de fato.

## Reversibilidade

Two-way door. Nenhum código de jogo existe hoje que dependa do mecanismo de
consumo; trocar de submódulo para `FetchContent` (ou vice-versa) depois é
reescrever um bloco de `CMakeLists.txt` e um comando de clone, não
retrabalhar lógica de jogo.

## Riscos / pontos de atenção (vínculo compartilhado)

- **Nunca prometer, em copy ou documentação pública, que misturar
  GlintFx-empacotado-por-terceiro com jogo-compilado-por-outro-compilador é
  seguro.** Não é, e a nota de ABI do GlintFx (L-22 dele) é explícita sobre
  isso.
- Se o líder decidir no futuro adicionar uma verificação de compatibilidade
  de ABI em tempo de carregamento, ou documentar a restrição no `README`
  público, isso é decisão dele — nomeada aqui como possibilidade, não
  decidida.

## Cross-refs

- [ADR-022](ADR-022-cmake-versao-minima-fixada.md) (o sistema de build que hospeda este `add_subdirectory`, e o gerador Ninja que compila os dois lados; ADR irmão, razão de mudar distinta por L-33)
- [ADR-024](ADR-024-ci-cinco-plataformas-cachyos-container.md) (a matriz de nove entradas cujo compilador único por célula é o que torna a ABI coerente dentro do nosso CI)
- `../../../GODS_LAWS.md` LEI ZERO, L-05, L-06, L-07 (de onde vem a função, não como ela chega ao build), L-08 (AGPL-3.0-or-later — por que o jogo é FOSS e por que alguém fora do nosso CI pode compilar da fonte), L-33
- `../../../GlintFx/CMakeLists.txt` (comentários `FIX-CONSUMO`, `PROJECT_SOURCE_DIR`/`PROJECT_BINARY_DIR` — o desenho do GlintFx já assume ser consumido por `add_subdirectory`)
- `../../../GlintFx/tests/embed/`, `tests/embed_name_collision/`, `tests/embed_dll_colocation/`, `tests/package/` (os quatro modos de consumo que o próprio GlintFx já testa)
- `../../../GlintFx/GODS_LAWS.md` L-19 (biblioteca compartilhada por padrão, superfície pública opaca) e L-22 (a nota verbatim de ABI/RTTI sobre exceção atravessando `.so`)
