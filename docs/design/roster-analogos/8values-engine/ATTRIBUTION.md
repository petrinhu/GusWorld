# ATTRIBUTION.md -- 8values Engine (GusWorld fork/port)

## O que isto e

`8values_engine.py`, nesta pasta, e um **fork/port explicito e derivado** do
projeto **8values** (o quiz de espectro politico de 8 valores / 4 eixos),
reescrito em Python para uso offline como ferramenta de design do GusWorld
(classificar o espectro politico de figuras historicas do roster em
`docs/design/roster-analogos/`).

Nao e um produto/feature do jogo GusWorld -- e uma ferramenta interna de
producao/design, assim como as demais specs em `docs/design/`.

## Autor e projeto original

- **Projeto:** 8values
- **Repositorio:** https://github.com/8values/8values.github.io
- **Site:** http://8values.github.io
- **Copyright original:** `Copyright (c) 2020 8values. http://8values.github.io.`
- **Licenca original:** MIT License (texto completo em
  `LICENSE_8VALUES_ORIGINAL.txt`, nesta mesma pasta -- copia verbatim do
  arquivo `LICENSE` do repositorio original, fetch em 2026-07-11 do branch
  `master`)

A MIT License e uma licenca permissiva: permite uso, copia, modificacao,
fusao, publicacao, distribuicao, sublicenciamento e venda, **desde que** o
aviso de copyright e o aviso de permissao acima sejam incluidos em todas as
copias ou partes substanciais do software. E o que este fork faz:
`LICENSE_8VALUES_ORIGINAL.txt` preserva o aviso original verbatim, e este
`ATTRIBUTION.md` + os comentarios de cabecalho em `8values_engine.py`
declaram a origem e a licenca de forma explicita.

Como a MIT nao e copyleft, o GusWorld nao e obrigado a licenciar o fork
inteiro sob os mesmos termos -- mas optamos por manter `8values_engine.py`
tambem sob **MIT** (mesma licenca do original), por simplicidade e por ser
a pratica mais segura/idiomatica para um fork direto.

## Licenca de CADA arquivo desta pasta (atualizado em 2026-08-06)

A pasta tem dois arquivos de codigo e eles **nao tem a mesma licenca**. A
diferenca e proposital e segue o criterio de quem escreveu o que:

| Arquivo | Licenca | Por que |
|---|---|---|
| `8values_engine.py` | **MIT** (herdada) | Obra derivada do 8values: as 70 perguntas, os pesos por eixo, a formula de scoring, os thresholds de rotulo e a tabela de 52 ideologias sao transcritos verbatim do projeto original. A licenca acompanha o material do autor original. Este arquivo esta na allowlist do `GATE(spdx-required)` (`tools/spdx_allowlist.txt`) exatamente por isso: por o SPDX Apache-2.0 do GusWorld nele seria declaracao de licenca falsa. |
| `gus_rightness.py` | **Apache-2.0** (nossa) | Codigo 100% do GusWorld: a camada v2 ponderada (pesos 0.50/0.25/0.15/0.10) e decisao de design do lider do projeto em 2026-07-11 e nao tem contraparte no 8values. O arquivo **importa** o engine em runtime (`importlib`), nao copia nem transcreve nenhuma linha dele. Ate 2026-08-06 declarava MIT por acompanhar o vizinho; o lider decidiu padronizar com o resto do repo (item `LICENCA-GUS-RIGHTNESS`). |
| `PERGUNTAS.md` | conteudo do 8values | As 70 perguntas sao texto do projeto original, so renumeradas/reformatadas para uso com persona-agents. |
| `LICENSE_8VALUES_ORIGINAL.txt` | (o proprio aviso) | Copia verbatim do `LICENSE` do repositorio original. Nao editar. |

**A troca de `gus_rightness.py` para Apache-2.0 nao afeta a atribuicao ao
8values**, por tres razoes que ficam registradas aqui para quem auditar isto
no futuro sem reabrir a analise:

1. **Nenhuma expressao do upstream esta naquele arquivo.** O que ele repete do
   vizinho (decodificacao do JSON de entrada, formato do relatorio CLI) e
   justamente o que o proprio ATTRIBUTION lista, na secao seguinte, como
   adicao do GusWorld ao fork ("plumbing de I/O", "relatorio CLI legivel"),
   nao material do 8values.
2. **A licenca original permite sublicenciar.** Mesmo na leitura conservadora
   de que `gus_rightness.py` fosse obra derivada do fork, a MIT autoriza
   expressamente "sublicense", desde que o aviso de copyright e o aviso de
   permissao permanecam nas copias.
3. **O aviso permanece.** `LICENSE_8VALUES_ORIGINAL.txt` continua verbatim, o
   cabecalho de `8values_engine.py` continua declarando a origem, e este
   documento continua distribuido junto. A condicao da MIT segue cumprida.

## O que foi transcrito verbatim do 8values (dados/formula deles)

Fetch feito em 2026-07-11 via `raw.githubusercontent.com/8values/8values.github.io/master/...`:

| Arquivo original      | Conteudo                                                                 | Onde foi para                                  |
|------------------------|---------------------------------------------------------------------------|-------------------------------------------------|
| `questions.js`         | 70 perguntas do quiz + pesos por eixo (`econ`/`dipl`/`govt`/`scty`)       | `QUESTIONS` em `8values_engine.py` + `PERGUNTAS.md` |
| `ideologies.js`        | 52 ideologias de referencia (nome + coordenadas nos 4 eixos)              | `IDEOLOGIES` em `8values_engine.py`             |
| `quiz.html`             | Formula de scoring: `max_axis = sum(abs(effect))`, `calc_score = 100*(max+score)/(2*max)`, multiplicadores SA=+1.0/A=+0.5/N=0/D=-0.5/SD=-1.0 | `score_answers()` / `_calc_score()`             |
| `results.html`         | Thresholds de rotulo por eixo (`setLabel`) + formula de distancia ponderada para achar a ideologia mais proxima (expoente 2 em econ/govt, expoente 1.73856063 em dipl/scty) | `_axis_label()` / `_closest_ideology()`          |

Todos esses valores (perguntas, pesos, formula de normalizacao, tabela de
ideologias, thresholds de rotulo, expoente de distancia) sao **do projeto
8values**, nao inventados pelo GusWorld. Sao dados/algoritmo do autor
original, protegidos pela MIT License dele.

## O que o GusWorld ADICIONOU (nao existe no original)

1. **O port Python em si** -- reimplementacao fiel, standalone, sem browser,
   chamavel via CLI/stdin/arquivo/import de modulo. O 8values original roda
   inteiramente em JavaScript no navegador (query string entre `quiz.html` e
   `results.html`); nao existia nenhuma forma de rodar o scoring fora do
   browser antes deste fork.
2. **O escalar `rightness` (0.0-1.0)** -- regra de design especifica do
   GusWorld (decisao do lider do projeto): `rightness = economic_markets_pct
   / 100`, ou seja, **so o eixo Economico** decide o escalar principal
   (Markets = 1.0 = direita, Equality = 0.0 = esquerda). Isso NAO existe no
   8values original (que so mostra 4 eixos + rotulo + ideologia, sem reduzir
   a um unico numero direita-esquerda).
3. **O campo `societal_tradition_pct`** exposto separadamente como
   desempate manual (Tradition = mais a direita) -- tambem uma decisao de
   design do GusWorld, nao parte do 8values original.
4. **`PERGUNTAS.md`** -- as 70 perguntas numeradas 1-70 num arquivo separado,
   formatadas para serem dadas a persona-agents (LLMs interpretando figuras
   historicas do roster) responderem.
5. Plumbing de I/O (JSON via arquivo/stdin), relatorio CLI legivel, e a
   linha `RIGHTNESS=0.NNN` parseavel por scripts.

## Proposito no GusWorld

Classificar o espectro politico (eixo Economico como primario) de figuras
historicas do roster de 21 personagens-analogos em
`docs/design/roster-analogos/` (Faraday, Maxwell, Tesla, Hayek, Mises,
Menger, Bastiat, etc.), para consistencia narrativa/de design -- ver
`project_axiologia_canonica.md` (memoria do projeto): coletivismo evolui
para mau, capitalismo/libertarianismo evolui para bom no canon GusWorld.
Este engine e uma FERRAMENTA DE DESIGN, nao codigo do jogo (nao roda em
runtime, nao e distribuido com o build do GusEngine).

## Contato do projeto original

Para duvidas/creditos sobre a metodologia original: `eightvalues@gmail.com`
(email listado em `results.html` do repo original para calibracao/feedback
do quiz).
