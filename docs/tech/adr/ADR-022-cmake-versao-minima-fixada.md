# ADR-022: CMake como sistema de build, com versão mínima fixada e medida, e Ninja como gerador

| | |
|---|---|
| **Status** | Accepted |
| **Data** | 2026-10-11 |
| **Decisor** | petrinhu (líder supremo) |
| **Reversibilidade** | Two-way door quanto à ferramenta (nenhum código de jogo existe ainda para migrar); one-way door de fato quanto ao piso de versão, uma vez que o primeiro `CMakeLists.txt` for commitado e a matriz de CI (L-09, L-20) passar a depender dele. |

## Contexto

Antes da ordem de serviço que originou este ADR, "CMake" aparecia dentro das
fatias do `TODO.md` e nas frases que descrevem o estado do repositório ("não
existe `CMakeLists.txt`"), mas em lugar nenhum como **decisão registrada**. O
líder identificou a lacuna e pediu que a fundação de build nascesse decidida, e
medida, antes da primeira fatia de código (L-20: "a fundação de build e o CI
são a primeira fatia do projeto").

A pergunta concreta não é "qual ferramenta". O GlintFx já usa CMake, e reusar a
mesma ferramenta do framework do qual o jogo assenta (LEI ZERO) é a opção óbvia,
sem alternativa real levantada pelo líder: **CMake é o sistema de build do
GusWorld por decisão do líder de 25/08/2026.** A pergunta que exige medição, e
que este ADR responde, é **qual a versão mínima que o GusWorld pode exigir sem
quebrar em alguma das cinco plataformas obrigatórias** (L-09, L-20).

## Decisão

**O GusWorld usa `cmake_minimum_required(VERSION 4.1)`.** Decisão do líder de
**11/10/2026**, por `AskUserQuestion`.

O número não foi escolhido por convenção nem copiado sem checar. Ele é o que o
GlintFx exige de quem o consome, e o GusWorld tem de poder consumi-lo pela
fronteira que a LEI ZERO protege.

## De onde vem o 4.1

Lido diretamente em `../../../../GlintFx/CMakeLists.txt`, linha 33:
`cmake_minimum_required(VERSION 4.1)`. O framework precisa desse piso por causa
de `cmake_pkg_config()`, o parser nativo de formato pkg-config do próprio CMake,
cujos subcomandos `IMPORT`/`POPULATE` só chegaram no CMake 4.1.

Isso alcança o GusWorld por consequência direta do [ADR-023](ADR-023-glintfx-submodulo-git-pinado.md):
o framework entra por `add_subdirectory(framework/GlintFx)`, e `add_subdirectory`
**executa** o `CMakeLists.txt` do framework. Um GusWorld que declarasse piso
abaixo de 4.1 não configuraria no dia em que o submódulo entrasse.

## O que foi medido, plataforma por plataforma

Todas as cinco entradas acompanham `:latest`, sem versão de imagem fixada
(L-09, decisão do líder de 11/10/2026). A resolução de imagem é a mesma do
GlintFx: container Linux sobre `ubuntu-latest` para as quatro distros, mais
runner nativo `windows-latest`.

| Plataforma | Imagem | CMake medido | Método |
|---|---|---|---|
| **Fedora** (primário) | `fedora:latest` | **4.3.0** | Medição **direta** nesta máquina, que é a do líder e o alvo primário da L-09: `cmake --version` devolve 4.3.0. O CI do GlintFx mede o mesmo valor no container. |
| **Ubuntu** | `ubuntu:latest` | **4.2.3** | Medição ao vivo do GlintFx, 27/08/2026, contra um container `ubuntu:latest` real: a imagem é o Ubuntu 26.04 ("Resolute Raccoon") e `apt-get install cmake` entrega 4.2.3. Registrada em `../../../../GlintFx/.github/workflows/ci.yml`, entradas Ubuntu. |
| **Arch** | `archlinux:latest` | **4.4.2** | Medição do GlintFx, 27/08/2026, pelo próprio `pacman`. Rolling release: o valor é fotografia da data, não garantia permanente. É a natureza do alvo, não erro de medição. |
| **CachyOS** | `cachyos/cachyos:latest` | **4.4.2** | Medição do GlintFx, 27/08/2026, pelo próprio `pacman`. Ver [ADR-024](ADR-024-ci-cinco-plataformas-cachyos-container.md) para a confirmação de que esta imagem é oficial, e a razão de CachyOS ter job próprio em vez de herdar o de Arch. |
| **Windows** | `windows-latest` | **3.31.6 de fábrica, abaixo do piso** | Leitura direta de `Windows2025-Readme.md` do repositório `actions/runner-images`. `windows-latest` aponta para a imagem Windows Server 2025 com Visual Studio 2026. |

**As quatro entradas Linux atendem 4.1 pelo próprio gerenciador de pacote, sem
passo extra algum.** Nenhuma medição falhou; não há plataforma com "não consegui
medir".

**O Windows é o único alvo que paga.** A imagem traz 3.31.6, abaixo do piso,
então o job Windows da matriz do item `C4` instala CMake explicitamente, como o
GlintFx já faz em `../../../../GlintFx/tools/ci/windows/prep.ps1`. Isso é um
passo de preparo de CI, não uma dependência do jogo: pela régua da LEI ZERO
(reforma de 28/08/2026), se o arquivo sumisse da máquina do **jogador**, o jogo
continuaria rodando, logo é ferramenta, não dependência.

## O piso ainda não é exercitado por nada na árvore, e isso é dito aqui de propósito

**Nenhum arquivo de CMake do GusWorld usa hoje recurso que precise de 4.1.**
A exigência vem inteira do framework, e o framework entra na árvore **quando a
camada `present/` nascer**, não agora (decisão do líder de 11/10/2026, que
escolheu adiar o submódulo; o [ADR-023](ADR-023-glintfx-submodulo-git-pinado.md)
continua valendo integralmente quanto ao **mecanismo**: submódulo git pinado em
`framework/GlintFx`, linkado compartilhado. O que o líder decidiu foi o
**momento**, não o mecanismo).

O 4.1 é, portanto, uma **declaração antecipada**, deliberada, e não uma
restrição que o código atual comprove. As duas razões de adotá-la agora, em vez
de no dia em que o submódulo entrar:

1. **O job Windows da matriz se escreve uma vez.** O passo de instalar CMake vai
   ser necessário de todo modo; construí-lo junto com a matriz (`C4`) evita
   reformar a matriz depois, e a L-20 exige a matriz das cinco desde o primeiro
   commit, sem entrada não bloqueante.
2. **Uma gramática só de versão mínima entre os dois repositórios irmãos**,
   em vez de dois pisos para rastrear e reconciliar.

**O que isso custa, nomeado em vez de escondido:** quem tiver CMake abaixo de
4.1 não configura o projeto a partir do fonte, mesmo que nada no fonte precise
de 4.1. A superfície disso é pequena, e a razão é de produto: o GusWorld é
**executável final, não biblioteca** (não é consumido por projeto de terceiro, e
quem joga recebe binário, não árvore de fonte). Para o GlintFx, que é biblioteca
consumível, o mesmo custo é bem maior, e ele o paga porque `cmake_pkg_config()`
o obriga.

## Gerador: Ninja nas CINCO plataformas, Windows incluído

**O GusWorld usa Ninja como gerador do CMake, `-G Ninja`, nas cinco plataformas
obrigatórias, sem exceção, Windows incluído.** Decisão do líder de 25/08/2026.

**Razão dele, registrada porque decide a forma do script de CI: um gerador só é
um comportamento só.** O mesmo comando de configuração
(`cmake -S . -B <dir> -G Ninja ...`) e o mesmo comando de build
(`cmake --build <dir>`) funcionam em qualquer entrada da matriz, Linux ou
Windows. O script local que espelha o CI (item `C7`, L-19) não precisa de dois
caminhos: um `if` de plataforma a menos, e uma ferramenta de paralelismo só para
entender.

**Sem piso de versão fixado para o Ninja, por escolha explícita do líder.**
Medição feita mesmo assim, por prudência, sem virar portão: nesta máquina
(alvo primário), `ninja --version` devolve **1.13.2**. Nenhum problema real de
versão foi encontrado. Se uma fatia futura encontrar um, o achado é reportado ao
líder, e não vira piso fixado por conta própria.

**Divergência declarada, não escondida, do próprio job Windows do GlintFx:** o
job `windows` do GlintFx **não** passa `-G Ninja`, e deixa o CMake escolher o
gerador do MSVC. O GusWorld faz diferente, conscientemente: Ninja também no
Windows. Quem comparar os dois CIs vai notar a diferença, e ela é intencional,
não descuido de quem copiou o padrão do framework sem olhar. Isso não quebra
nada entre os dois repositórios, porque o gerador é propriedade da árvore de
build de quem o invoca, não do que está sendo compilado.

## Alternativas consideradas

### A. Não fixar versão mínima, deixar o CMake do sistema decidir

**Prós:** zero trabalho.
**Contras:** é exatamente a armadilha que o líder pediu para evitar. O build
passa na máquina dele (CMake 4.3.0) e falha silenciosamente numa distro mais
velha, sem ninguém perceber até o CI, ou um jogador que compila, quebrar.
**Rejeitada.**

### B. Fixar o piso no que o runner Windows entrega de fábrica (3.31.6), para evitar o passo de instalação

**Prós:** o job Windows da matriz não precisaria instalar CMake, e a matriz
ficaria mais curta de escrever.
**Contras:** o piso não aceitaria o GlintFx no dia em que o submódulo entrasse
(ADR-023), e o conserto seria exatamente o passo de instalação que esta opção
tenta evitar, só que pago mais tarde e sob pressão, com a matriz já escrita.
Economiza um passo hoje e compra uma reforma depois.
**Rejeitada.**

### C. `cmake_minimum_required(VERSION 4.1)`, igual ao GlintFx (escolhida)

**Prós:** é o único valor que aceita o framework pela fronteira da LEI ZERO;
atendido de graça pelas quatro entradas Linux (medido, não estimado); uma
gramática só de versão mínima entre os dois repositórios irmãos; o job Windows
se escreve uma vez.
**Contras:** o Windows ganha um passo de instalação de CMake, e o piso fica
declarado antes de ser exercitado por qualquer coisa na árvore (ver a seção
própria acima, que nomeia esse custo em vez de escondê-lo).
**Aceita.**

## Consequências

**Positivas:**

- O piso é real, medido, e documentado com a fonte de cada medição.
- Alinhamento com o GlintFx elimina um segundo piso de versão para rastrear.
- As quatro entradas Linux não exigem repositório de terceiro nem tarball
  externo: o `dnf`/`apt`/`pacman` de cada uma já entrega acima do piso.
- Ninja como gerador único simplifica o script de CI: um só comando de build e
  uma só ferramenta de paralelismo, em vez de depender do gerador padrão de cada
  plataforma (Makefiles no Linux, Visual Studio no Windows), com comportamento
  de paralelismo e saída diferentes.

**Negativas, aceitas como custo:**

- O job Windows da matriz carrega um passo de instalação de CMake, com tudo que
  um passo de preparo traz: mais uma coisa que pode falhar, e mais um download
  no caminho do CI.
- Quem tiver CMake abaixo de 4.1 não configura o projeto a partir do fonte,
  mesmo enquanto nada no fonte precisar de 4.1.
- Arch e CachyOS são rolling release: os números medidos (4.4.2 nos dois) são
  fotografia de 27/08/2026, não garantia permanente. Isso não ameaça o piso
  (eles só tendem a subir), mas fica registrado como natureza do alvo.

## Riscos / pontos de atenção

- **O piso declarado não é exercitado por teste nenhum até a camada `present/`
  nascer.** Enquanto isso, nada no projeto falharia se um CMake abaixo de 4.1
  rodasse: a declaração é uma promessa, não um portão provado. Quando o
  submódulo entrar (ADR-023), ela passa a ser verificada pelo próprio configure,
  e é esse o momento em que o número vira restrição medida.
- Se o GlintFx subir o próprio `cmake_minimum_required` outra vez, o GusWorld
  precisa acompanhar, porque a fronteira da LEI ZERO obriga o jogo a compilar
  contra o commit pinado do framework. Este ADR é o lugar onde isso se registra,
  com medição nova, nunca por presunção de que o novo número também é atendido.
- Quando o Ubuntu usado em CI mudar de série, o valor medido muda junto, e este
  ADR precisa ser revisitado com medição nova. Não presumir que o LTS seguinte
  mantém o mesmo CMake.
- Sem piso de Ninja fixado, um Ninja muito velho em alguma plataforma poderia
  falhar de forma obscura. Nenhuma medição feita até aqui indica isso: é risco
  teórico, registrado por completude, não achado real.

## Reversibilidade

Two-way door hoje, porque nenhum código de jogo existe para migrar. Torna-se um
one-way door de fato assim que a matriz de CI (L-09, L-20) e o primeiro módulo
de comportamento (L-19) passarem a depender do piso: subir o piso depois é
simples, descer exige reconciliar com o que o GlintFx exige, e a fronteira da
LEI ZERO não deixa essa reconciliação ser unilateral.

## Cross-refs

- [ADR-023](ADR-023-glintfx-submodulo-git-pinado.md) (como o GlintFx entra no build, e de onde vem o 4.1; ADR irmão, razão de mudar distinta por L-33)
- [ADR-024](ADR-024-ci-cinco-plataformas-cachyos-container.md) (a matriz de CI que consome este piso, e o passo de instalação do CMake no Windows)
- [ADR-025](ADR-025-harness-teste-proprio-minimo.md) (o harness que roda sobre esta fundação)
- `../../../GODS_LAWS.md` L-09, L-17, L-20, L-33
- `../../../../GlintFx/CMakeLists.txt` linha 33 (a exigência de `VERSION 4.1`, origem deste piso)
- `../../../../GlintFx/.github/workflows/ci.yml` (as medições de CMake por distro, e o precedente de imagem `:latest` por plataforma)
- `../../../../GlintFx/tools/ci/windows/prep.ps1` (o padrão de instalação de CMake no runner Windows)
