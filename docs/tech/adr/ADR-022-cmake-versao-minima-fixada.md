# ADR-022: CMake como sistema de build, com versão mínima fixada e medida, e Ninja como gerador

| | |
|---|---|
| **Status** | Accepted (decisão do líder, 25/08/2026) |
| **Data** | 2026-08-25 |
| **Decisor** | petrinhu (líder supremo) |
| **Reversibilidade** | Two-way door quanto à ferramenta (nenhum código de jogo existe ainda para migrar); one-way door de fato quanto ao piso de versão, uma vez que o primeiro `CMakeLists.txt` for commitado e a matriz de CI (L-09, L-20) passar a depender dele. |

## Contexto

Antes desta ordem de serviço, "CMake" aparecia dentro das fatias do `TODO.md` e
nas frases que descrevem o estado do repositório ("não existe
`CMakeLists.txt`"), mas em lugar nenhum como **decisão registrada**. O líder
identificou a lacuna e pediu que a fundação de build nascesse decidida, e
medida, antes da primeira fatia de código (L-20: "a fundação de build e o CI
são a primeira fatia do projeto").

A pergunta concreta que motiva esta ADR não é "qual ferramenta" — o GlintFx já
usa CMake, e reusar a mesma ferramenta do framework do qual o jogo assenta
(LEI ZERO) é a opção óbvia, sem alternativa real levantada pelo líder. A
pergunta que faltava responder era **qual a versão mínima que o GusWorld pode
exigir sem quebrar em alguma das cinco plataformas obrigatórias (L-09, L-20)**,
e essa resposta exige medição, não estimativa.

## Decisão

**O GusWorld usa CMake como sistema de build, com `cmake_minimum_required(VERSION 3.28)`.**

Este número não foi escolhido por convenção nem copiado sem checar: é o
resultado de medir o que cada uma das cinco plataformas obrigatórias entrega,
e verificar que o valor sobrevive ao mais velho dos cinco (o piso) **e** ao
que o GlintFx já exige de quem o consome.

## O que foi medido, plataforma por plataforma

O GlintFx (framework do qual este jogo depende, LEI ZERO) já roda uma matriz
de CI com as mesmas cinco plataformas, resolvidas como container Linux sobre
`ubuntu-latest` (Fedora, Ubuntu, Arch, CachyOS) mais um runner nativo
`windows-latest` (Windows). Adoto a mesma resolução de imagem por precedente
direto: é decisão do mesmo líder, já em produção e verde no projeto do qual
o GusWorld depende.

| Plataforma | Imagem/imagem-base usada (precedente GlintFx) | CMake medido | Método |
|---|---|---|---|
| **Fedora 44** (primário, pinado) | `fedora:44` | **4.3.0** (`cmake-4.3.0-1.fc44.x86_64`) | Medição **direta**: `rpm -q cmake` e `cmake --version` rodados nesta mesma máquina, que é a do líder e o alvo primário da L-09. |
| **Ubuntu** | `ubuntu:24.04` | **3.28.3** (`3.28.3-1build7`, repositório `noble`) | Busca web contra `packages.ubuntu.com` e Launchpad. Série assumida: **24.04 LTS**, porque é a que o GlintFx já usa em produção (lido em `../GlintFx/.github/workflows/ci.yml`), não uma escolha nova deste ADR. |
| **Arch** | `archlinux:latest` | **4.4.2** (repositório `extra`) | Busca web contra `archlinux.org/packages`. Rolling release: valor válido em 25/08/2026, pode mudar a qualquer momento — é a natureza do alvo, não um erro de medição. |
| **CachyOS** | `cachyos/cachyos:latest` | **4.3.2** (repositório `extra`, `x86_64`) | Busca web contra `packages.cachyos.org`. Ver ADR-024 para a confirmação de que esta imagem é oficial. |
| **Windows** | `windows-latest` | **3.31.6** | Leitura direta de `Windows2025-Readme.md` do repositório `actions/runner-images`. `windows-latest` aponta hoje para a imagem **Windows Server 2025 com Visual Studio 2026** (migração concluída em junho de 2026); a mesma versão de CMake (3.31.6) está documentada tanto na imagem 2022 quanto na 2025. |

**O piso das cinco é 3.28.3, do Ubuntu 24.04.** Nenhuma medição falhou; não há
plataforma com "não consegui medir".

## O que o GlintFx exige, e a ausência de conflito

Lido diretamente em `../GlintFx/CMakeLists.txt`, linha 11:
`cmake_minimum_required(VERSION 3.28)`. A mesma exigência se repete em
`tests/package/CMakeLists.txt`, `tests/embed/CMakeLists.txt` e
`tests/embed_name_collision/CMakeLists.txt` do GlintFx.

**Não há conflito.** O piso medido das cinco plataformas (3.28.3) já satisfaz
`>= 3.28.0`. Fixar o GusWorld em `VERSION 3.28` não pede nada que o GlintFx já
não peça, e o valor sobrevive ao pior caso real medido.

Uma única exceção foi encontrada e não altera a conclusão: `../GlintFx/.github/workflows/ci.yml:252`
gera, dentro do job `windows`, um `CMakeLists.txt` sintético e descartável só
para testar se o MSVC aceita `if consteval` de C++23 — esse probe declara
`cmake_minimum_required(VERSION 3.25)`. É um artefato de teste isolado, nunca
parte do link real do GlintFx nem do GusWorld, e por isso não é uma segunda
exigência a reconciliar.

## Gerador: Ninja (decisão do líder, 25/08/2026)

**O GusWorld usa Ninja como gerador do CMake, `-G Ninja`, em toda entrada da
matriz onde isso é aplicável.** Antes desta decisão, "Ninja" já vinha
implícito nas fatias (a fundação de build presumia Ninja como se fosse óbvio),
sem nunca ter sido registrado como escolha do líder. Isto encerra essa
suposição: passa a ser uma **decisão registrada**, não uma herança tácita.

**Sem piso de versão fixado para o Ninja, por escolha explícita do líder.**
Medição feita mesmo assim, por prudência, sem virar portão: nesta máquina
(Fedora 44, alvo primário), `ninja --version` devolve **1.13.2**
(`ninja-build-1.13.2-2.fc44.x86_64`). O próprio GlintFx instala Ninja pelo
gerenciador de pacote nativo de cada distro Linux da sua matriz
(`ninja-build` no `dnf`, `ninja` no `pacman`) e o invoca com `-G Ninja`
explícito no job `linux` (`../GlintFx/.github/workflows/ci.yml:212`). Nenhum
problema real de versão foi encontrado nesta medição; se uma fatia futura
encontrar um, o achado é reportado ao líder, e não vira piso fixado por
conta própria — foi assim que ele decidiu.

**Onde diverge do precedente do GlintFx, registrado para não confundir quem
comparar os dois CIs:** o job `windows` do GlintFx **não** passa `-G Ninja`
— deixa o CMake escolher o gerador padrão do MSVC (Visual Studio), como o
próprio comentário do workflow diz: "Sem action de terceiro: o próprio CMake
localiza o MSVC." A decisão do líder para o GusWorld ("Ninja como gerador",
sem exceção mencionada) é lida aqui como valendo também para Windows, o que
é uma escolha **diferente** da que o GlintFx faz hoje no próprio job Windows
dele. Isto não é um conflito a resolver — GusWorld e GlintFx podem usar
geradores diferentes no mesmo CMake sem se afetarem, já que o gerador é
propriedade da árvore de build de quem o invoca, não do que está sendo
compilado — mas fica registrado como divergência consciente, não como
suposição de que "é assim que o GlintFx faz".

## Alternativas consideradas

### A. Não fixar versão mínima, deixar o CMake do sistema decidir

**Prós:** zero trabalho.
**Contras:** é exatamente a armadilha que o líder pediu para evitar — o build
passa na máquina dele (Fedora 44, CMake 4.3.0) e falha silenciosamente numa
distro mais velha, sem ninguém perceber até o CI (ou um jogador que compila)
quebrar.
**Rejeitada.**

### B. Fixar um número alto (ex.: 3.31 ou 4.0), por segurança

**Prós:** nenhum ganho real hoje; nenhuma feature do GusWorld precisa de CMake
acima de 3.28 ainda.
**Contras:** quebraria o Ubuntu 24.04 (3.28.3 < 3.31 ou 4.0), o alvo que hoje é
o piso. Seria inventar um número sem medição, o oposto do que foi pedido.
**Rejeitada.**

### C. `cmake_minimum_required(VERSION 3.28)`, igual ao GlintFx (escolhida)

**Prós:** medido, sobrevive ao piso real, idêntico ao que o framework já
exige (uma gramática só de versão mínima entre os dois repositórios
irmãos), sem custo de manutenção extra.
**Contras:** nenhum identificado — é o valor mais baixo que já funciona em
todas as plataformas medidas.
**Aceita.**

## Consequências

**Positivas:**
- O piso é real, medido, e documentado com a fonte de cada medição.
- Alinhamento com o GlintFx elimina uma segunda versão mínima para rastrear.
- Ubuntu 24.04 (o alvo mais restrito hoje) segue funcionando sem exigir
  repositório de terceiro (Kitware PPA) só para elevar o CMake do sistema.
- Ninja como gerador único simplifica o script de CI: um só comando de build
  (`cmake --build <dir>`) e uma só ferramenta de paralelismo, em vez de
  depender do gerador padrão de cada plataforma (Makefiles no Linux, Visual
  Studio no Windows) com comportamento de paralelismo e saída diferentes.

**Negativas / aceitas como custo:**
- CMake 3.28 é conservador frente ao que Fedora, Arch e CachyOS já entregam
  (4.x); features de CMake mais novas (ex.: de 3.29 em diante) ficam fora de
  uso até o líder decidir subir o piso, o que só deve acontecer quando o
  Ubuntu LTS usado em CI também subir.
- Arch e CachyOS são rolling release: o número medido aqui (4.4.2 / 4.3.2)
  é uma fotografia de 25/08/2026, não uma garantia permanente. Isso não
  ameaça o piso (eles só tendem a subir, nunca descer abaixo do que Ubuntu
  24.04 já tem), mas fica registrado como natureza do alvo.

## Riscos / pontos de atenção

- Quando o Ubuntu usado em CI mudar de série (24.04 para 26.04, por exemplo),
  o piso muda junto, e este ADR precisa ser revisitado com nova medição —
  não presumir que o novo LTS mantém o mesmo CMake.
- Se o GlintFx um dia subir o próprio `cmake_minimum_required`, o GusWorld
  precisa acompanhar, porque a fronteira LEI ZERO obriga o jogo a compilar
  contra o commit pinado do framework (ADR-023).
- Sem piso de Ninja fixado, um Ninja muito velho em alguma plataforma poderia
  falhar de forma obscura (ex.: sem suporte a alguma feature de geração que o
  CMake 3.28+ emite). Nenhuma medição feita até aqui indica isso — é um risco
  teórico, registrado por completude, não um achado real.

## Reversibilidade

Two-way door hoje (nenhum código de jogo existe para migrar). Torna-se um
one-way door de fato assim que a matriz de CI (L-09, L-20) e o primeiro
módulo de comportamento (L-19) passarem a depender do piso 3.28 — subir o
piso depois é simples; descer é impossível sem quebrar o que o Ubuntu 24.04
entrega hoje.

## Cross-refs

- [ADR-023](ADR-023-glintfx-submodulo-git-pinado.md) (como o GlintFx entra no build; ADR irmão, razão de mudar distinta por L-33)
- [ADR-024](ADR-024-ci-cinco-plataformas-cachyos-container.md) (a matriz de CI que consome este piso)
- `../../../GODS_LAWS.md` L-09, L-17, L-20, L-33
- `../../../GlintFx/CMakeLists.txt` (a exigência de `VERSION 3.28` que este ADR confirma não conflitar)
- `../../../GlintFx/.github/workflows/ci.yml` (precedente de imagem por plataforma, reusado aqui)
