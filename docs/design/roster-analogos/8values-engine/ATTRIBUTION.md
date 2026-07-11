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
