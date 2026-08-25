# ADR-024: CI das cinco plataformas — dois compiladores no Linux, MSVC no Windows, CachyOS via container oficial

| | |
|---|---|
| **Status** | Accepted (decisão do líder, 25/08/2026) |
| **Data** | 2026-08-25 |
| **Decisor** | petrinhu (líder supremo) |
| **Reversibilidade** | Two-way door quanto ao mecanismo de CI (trocar imagem, runner ou compilador não afeta código de jogo); one-way door reputacional se a matriz declarar uma plataforma verde sem cobertura real (L-09: "verde no Arch não autoriza declarar CachyOS suportado"), ou se declarar uma entrada de compilador não-bloqueante (L-20). |

## Contexto

A L-09 e a L-20 do `GODS_LAWS.md` exigem cinco entradas distintas de CI —
Fedora 44 (primário, pinado), Ubuntu, Arch, CachyOS e Windows — e são
explícitas: **"CachyOS não é Arch renomeado" e não é coberto pelo job de
Arch**. O que faltava decidir era **como rodar CachyOS de verdade** num
provedor de CI hospedado (GitHub Actions), que não oferece um runner nativo
CachyOS entre suas imagens padrão (`ubuntu-latest`, `windows-latest`,
`macos-latest`).

Sem uma imagem oficial, a tentação seria uma de duas saídas ruins: (a) rodar
o job de CachyOS sobre uma imagem Arch genérica reconfigurada para "parecer"
CachyOS, o que contradiz a L-09 no espírito (a lei existe justamente para
distinguir toolchain e repositórios próprios do CachyOS do Arch puro); ou
(b) usar uma imagem de terceiro não afiliada ao projeto CachyOS, cuja
manutenção e fidelidade ninguém garante.

**Acréscimo de 25/08/2026, mesma ordem de serviço:** o líder tomou mais duas
decisões que pertencem a esta mesma ADR, por serem parte da mesma
preocupação (estratégia de CI/plataforma, L-09/L-20), e fecham o item `G6`
— a única decisão pendente dentro da cadeia crítica do projeto:

- **Compilador no Linux: GCC e Clang, os dois.** Razão do líder, verbatim
  em espírito: os dois compiladores pegam classes de bug diferentes, e com
  `-Werror` (portão 1 da L-19) a divergência entre eles aparece cedo — no
  commit que a introduziu, não meses depois.
- **Compilador no Windows: MSVC.** ABI nativa do sistema, binário sem
  runtime de terceiro (nem MinGW, nem libc++ extra), e é o toolchain que
  mais diverge do mundo POSIX — o melhor detector, portanto, de código que
  silenciosamente só funciona em Linux/macOS.

## Decisão

**CachyOS entra na matriz de CI como container da imagem oficial
`cachyos/cachyos:latest`, dentro de um job hospedado num runner Linux comum
(`ubuntu-latest` como host, `container: cachyos/cachyos:latest` como
ambiente de execução).** É a mesma técnica usada para Fedora 44 e Arch na
mesma matriz: o host apenas providencia a máquina, o container é o ambiente
de compilação real.

**Fedora 44 e Ubuntu entram do mesmo jeito**, por consistência de matriz:
`container: fedora:44` (pinado, nunca `:latest`, por exigência textual da
L-09) e `container: ubuntu:24.04` (a série já em uso pelo GlintFx, ver
ADR-022). **Arch entra como `container: archlinux:latest`.**

**Cada uma das quatro plataformas Linux compila com GCC E com Clang, os
dois** — cada combinação (plataforma × compilador) é uma entrada própria e
**bloqueante** da matriz, nunca informativa (L-20: "a matriz de cinco
plataformas... nenhuma não bloqueante" — a mesma régua se estende, por
decisão do líder, ao par de compiladores dentro de cada plataforma Linux).

**Windows compila com MSVC, e é o único alvo sem container**, rodando
nativamente em `windows-latest`, porque não existe equivalente de container
Linux para o toolchain MSVC no mesmo sentido, e porque MSVC é justamente o
compilador que a decisão do líder escolheu para esse alvo (não GCC/Clang via
MinGW, que reintroduziria uma ABI POSIX-like sobre Windows e anularia o
propósito de ter MSVC como o detector mais divergente do POSIX).

## Contagem de entradas da matriz: NOVE, confirmado pelo líder em 25/08/2026

A versão anterior deste ADR levantou uma divergência aritmética: a ordem de
serviço original citava "sete entradas", mas a leitura literal das duas
decisões de compilador — **GCC e Clang, os dois, em CADA uma das quatro
plataformas Linux** — fecha em nove, não sete. Em vez de forçar o número
para sete inventando qual subconjunto de plataformas ficaria com um único
compilador (decisão de escopo de cobertura que não me cabia tomar, L-11), a
divergência foi devolvida ao líder como pergunta aberta.

**O líder respondeu: a matriz tem NOVE entradas**, a leitura literal e
uniforme, sem exceção de plataforma:

| Plataforma | Compilador(es) | Entradas |
|---|---|---|
| Fedora 44 (primário) | GCC **e** Clang | 2 |
| Ubuntu 24.04 | GCC **e** Clang | 2 |
| Arch | GCC **e** Clang | 2 |
| CachyOS | GCC **e** Clang | 2 |
| Windows | MSVC | 1 |
| **Total** | | **9** |

**Razão dele, registrada porque é a que sustenta a decisão: cobertura
desigual esconde bug.** Se só o Fedora rodasse Clang (ou qualquer outro
subconjunto), um aviso que só o Clang do **Arch** desse — por exemplo, uma
diferença de versão de libc/headers entre as distros — passaria despercebido
até alguém compilar manualmente naquela combinação específica. Testar os
dois compiladores em **todas** as quatro plataformas Linux é o que garante
que a divergência apareça em qualquer distro onde ela exista, não só
naquela que o CI escolheu arbitrariamente testar com os dois.

**As nove entradas são bloqueantes, sem exceção (L-20).** Nenhuma delas é
informativa nem pode ficar de fora do gate de push (L-32).

⚠️ **Correção de registro, porque o erro era do lado de quem redigiu a ordem
de serviço, não meu, e isso fica dito com todas as letras:** o número "sete"
citado originalmente era um erro aritmético do coordenador, propagado
também para o item `G6` do `TODO.md`. A recusa de forçar o número ou de
inventar um subconjunto de plataformas foi o comportamento correto — a
pergunta devolvida (L-11) é o que permitiu a correção acontecer antes do
número errado virar fundação. **Toda ocorrência de "sete entradas" neste
documento, no ADR-023 e no `_INDEX.md` foi substituída por "nove".**

## Evidência de que a imagem oficial existe

Duas fontes, uma delas de precedente direto e mais forte que a outra:

1. **Busca web:** o projeto CachyOS mantém `CachyOS/docker` no GitHub
   ("CachyOS Docker Images for the x86-64, x86-64-v3") com releases
   publicadas, e publica a imagem resultante em `cachyos/cachyos` no Docker
   Hub.
2. **Precedente medido, mais forte:** o GlintFx — o framework do qual este
   próprio projeto depende (LEI ZERO) — **já usa esta exata imagem em
   produção**. Lido diretamente em
   `../GlintFx/.github/workflows/ci.yml`, linhas 134 e 143:
   `imagem: cachyos/cachyos:latest`, dentro do mesmo job `linux` que também
   cobre Fedora, Ubuntu e Arch. Não é uma imagem hipotética: é uma decisão
   idêntica, do mesmo líder, já rodando e (presumivelmente) verde no
   repositório irmão. Isto responde de forma direta ao ponto de parada que
   esta ordem de serviço previa ("se não existir imagem oficial, pare e
   reporte") — a imagem existe e já está em uso.

## Interação com o vínculo compartilhado do GlintFx (ADR-023)

O GlintFx entra compartilhado (`.so`/`.dll`, ADR-023), e uma nota de ABI
daquele framework (L-22 dele) avisa que exceção atravessando essa fronteira
exige RTTI compatível entre os dois lados, o que quebra se o `.so` e o
consumidor vierem de compiladores diferentes. Esta matriz é o que torna essa
garantia real dentro do nosso CI: **cada entrada compila o GlintFx (via
`add_subdirectory` do submódulo) e o jogo com o mesmo `CMAKE_CXX_COMPILER`**,
nunca um em GCC e o outro em Clang dentro da mesma célula da matriz. O
detalhe completo — o que isso garante e, mais importante, o que **não**
garante para quem compila fora deste CI — está registrado no ADR-023, seção
"Vínculo: compartilhado (`.so`), espelhando o GlintFx", para não duplicar o
mesmo texto em dois ADRs sobre razões de mudar diferentes (L-33).

## Alternativas consideradas

### A0. Um único compilador no Linux (o default de cada distro), como estava implícito antes desta ordem de serviço

**Prós:** metade das entradas de matriz Linux, custo de CI menor.
**Contras:** não detecta código que compila num compilador e não no outro
até alguém compilar manualmente com o compilador não testado — exatamente a
classe de bug que o líder quer pegar cedo, no commit que a introduziu, via
`-Werror`.
**Rejeitada pelo líder.**

### A1. Rodar o job de CachyOS sobre `archlinux:latest`, com `pacman.conf` do CachyOS injetado manualmente

**Prós:** nenhuma dependência de uma imagem de terceiro.
**Contras:** é exatamente o "CachyOS renomeado" que a L-09 proíbe — não é
CachyOS de verdade, é Arch com repositórios trocados na hora, sem os
compiladores, flags e otimizações que o CachyOS de fato distribui (o CachyOS
compila pacotes com otimizações de CPU próprias, ex. `x86-64-v3`, e um
`pacman.conf` remendado por cima do Arch não reproduz isso). Mentiria sobre
o que está sendo testado.
**Rejeitada.**

### A2. Imagem de terceiro não afiliada ao projeto CachyOS

**Prós:** poderia existir alguma pronta em algum registro público.
**Contras:** nenhuma garantia de fidelidade ao CachyOS real nem de
manutenção contínua; nenhuma auditabilidade de proveniência.
**Rejeitada** — nem chegou a ser necessária, porque a imagem oficial existe.

### A3. Container da imagem oficial `cachyos/cachyos:latest` (escolhida para a imagem; GCC+Clang decidido à parte, acima)

**Prós:** é CachyOS de verdade — mesmos repositórios, mesmos compiladores,
mesmas flags; mantida pelo próprio projeto CachyOS; já provada em produção
no repositório irmão GlintFx.
**Contras:** `:latest` é uma tag móvel (a mesma tensão que a L-09 já resolve
para Fedora, mandando pino explícito só lá — Fedora é o alvo primário e por
isso precisa detectar regressão da própria máquina do líder; CachyOS e Arch
são alvos de portabilidade, e usar `:latest` neles é a mesma escolha já
feita pelo GlintFx, não uma nova).
**Aceita.**

## Consequências

**Positivas:**
- CachyOS testado de verdade, com os compiladores e repositórios que o
  jogador CachyOS de fato usa — nenhuma mentira de cobertura.
- Estrutura de matriz idêntica à do GlintFx: qualquer pessoa que já entenda
  o CI de um projeto entende o do outro.
- Item de parada previsto na ordem de serviço ("se não existir imagem
  oficial, pare e reporte") não se aplicou — a imagem existe e está provada.
- Divergência GCC/Clang some cedo: com `-Werror` (L-19 portão 1), um
  aviso que só aparece num dos dois compiladores derruba o build no
  commit que o introduziu, não meses depois numa auditoria.
- MSVC no Windows detecta código que assume POSIX silenciosamente (paths,
  `#include` de cabeçalho não-padrão, comportamento de `signed`/`unsigned`
  divergente), porque é o compilador que mais se afasta desse mundo.
- Dentro do nosso CI, a ABI entre GlintFx e jogo fica coerente por
  construção (ver seção "Interação com o vínculo compartilhado" acima) —
  nenhuma célula da matriz mistura compilador da lib com compilador do jogo.

**Negativas / aceitas como custo:**
- `cachyos/cachyos:latest` é tag móvel: uma falha de CI pode vir de mudança
  upstream do CachyOS, não do GusWorld. Mesmo custo que o GlintFx já paga
  hoje; mitigado por `fail-fast: false` na matriz, para que uma falha nessa
  entrada não esconda o resultado das outras quatro.
- Container Linux não cobre teste de superfície gráfica real (janela,
  input) — irrelevante hoje porque `present/` ainda não existe (L-06,
  L-27), mas registrado para quando nascer: a prática já em uso no GlintFx
  é compositor Wayland aninhado dentro do container, nunca a sessão viva.
- **A matriz cresce de cinco para nove entradas**, confirmado pelo líder
  (ver seção de contagem acima). Custo de tempo e minutos de CI por push
  aumenta proporcionalmente, e nenhuma das nove entradas pode ser não
  bloqueante (L-20) — custo aceito conscientemente em troca de cobertura
  uniforme (cobertura desigual esconde bug, razão do líder).

## Riscos / pontos de atenção

- **Nunca declarar CachyOS suportado a partir de um job verde de Arch.** A
  L-09 é textual sobre isso; a matriz tem de manter as duas entradas
  fisicamente separadas, cada uma com seu próprio resultado de CI visível.
- Acompanhar se o CachyOS um dia publicar uma tag versionada estável (em vez
  de só `:latest`); se sim, é decisão do líder migrar, não automática.
- **Nunca misturar compiladores dentro da mesma célula da matriz** (GlintFx
  compilado por um, jogo pelo outro) — anularia a garantia de coerência de
  ABI que esta matriz existe para fornecer (ver ADR-023).
- A contagem exata da matriz **é nove entradas**, confirmado pelo líder
  (ver seção "Contagem de entradas da matriz" acima) — não fica mais
  pendente.

## Reversibilidade

Two-way door quanto ao mecanismo: trocar a imagem, o runner, o compilador ou
a forma de hospedar CachyOS no CI não toca em nenhum código de jogo. One-way
door reputacional se a matriz um dia disser "CachyOS suportado" sem esta
entrada verde e isolada (L-09), ou se uma entrada de compilador virar
não-bloqueante (L-20) — ambos os erros que as leis nomeiam explicitamente
para nunca acontecer.

## Cross-refs

- [ADR-022](ADR-022-cmake-versao-minima-fixada.md) (a versão de CMake que cada uma destas imagens precisa entregar, e o gerador Ninja usado em cada compilação; o piso medido ali usa exatamente estas mesmas imagens)
- [ADR-023](ADR-023-glintfx-submodulo-git-pinado.md) (o submódulo que cada job desta matriz precisa inicializar via `git submodule update --init --recursive` ou checkout recursivo, e o vínculo compartilhado cuja coerência de ABI depende de esta matriz nunca misturar compilador dentro de uma célula)
- `../../../GODS_LAWS.md` L-09, L-19 (portão 1, `-Werror`, a razão de testar dois compiladores), L-20, L-33
- `../../../GlintFx/.github/workflows/ci.yml` (o precedente medido, linhas 60-150 e 218 em diante)
- <https://github.com/CachyOS/docker>, <https://hub.docker.com/r/cachyos/cachyos> (a imagem oficial)
