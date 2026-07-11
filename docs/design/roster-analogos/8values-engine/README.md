# 8values Engine (GusWorld) -- README

Ferramenta de design (nao codigo do jogo) para classificar o espectro
politico de figuras historicas do roster GusWorld
(`docs/design/roster-analogos/`), usando um port Python fiel do quiz
**8values**.

**Este e um fork/port derivado do projeto 8values** -- ver `ATTRIBUTION.md`
nesta pasta para creditos completos e `LICENSE_8VALUES_ORIGINAL.txt` para a
licenca (MIT) do trabalho original. Resumo: as 70 perguntas, os pesos por
eixo, a formula de scoring e a tabela de 52 ideologias sao do 8values
(https://github.com/8values/8values.github.io, MIT); o escalar `rightness`
0-1, o port Python e a documentacao sao adicoes do GusWorld.

## Arquivos desta pasta

- `8values_engine.py` -- o engine (dados + logica + CLI). Ver docstring do
  modulo para a formula de scoring completa.
- `gus_rightness.py` -- camada v2 SEPARADA, EXCLUSIVA DO JOGO (ver aviso
  abaixo). NAO modifica o engine: importa o scoring dele verbatim e imprime
  os dois escalares (`RIGHTNESS_V1`, `RIGHTNESS_V2`) + bloco descritivo.
- `PERGUNTAS.md` -- as 70 perguntas (texto EN original), numeradas 1-70,
  para dar a um persona-agent responder.
- `ATTRIBUTION.md` -- creditos ao 8values original + o que o GusWorld
  adicionou.
- `LICENSE_8VALUES_ORIGINAL.txt` -- copia verbatim da MIT License do repo
  8values original.

## AVISO: `gus_rightness.py` e EXCLUSIVO DO JOGO (nao e o 8values)

O script `gus_rightness.py` aplica pesos por eixo
(`0.50*Markets + 0.25*Liberty + 0.15*Tradition + 0.10*Nation`) que refletem
a importancia de cada eixo DENTRO DA FICCAO do GusWorld, unica e
exclusivamente (mecanica de jogo: qual arquetipo de sidequest uma carta de
personagem ficticio recebe). **NAO e uma modificacao do teste 8values** (o
scoring original em `8values_engine.py` permanece intocado), **NAO e uma
afirmacao sobre politica do mundo real** e **NAO deve ser levado ao mundo
real** nem usado para avaliar pessoas, testes ou ideologias reais.
Racional completo, historia da refatoracao e log de decisoes:
`../OBRA-DE-FICCAO-E-METODOLOGIA.md` (secao "Pesos dos eixos v2").

## Como usar (fluxo persona-agent -> engine)

1. De `PERGUNTAS.md` a um agent personificando a figura historica (ex.:
   Hayek, Mises, Bastiat...). Peca que responda cada uma das 70 perguntas
   com EXATAMENTE um token: `sa` / `a` / `n` / `d` / `sd`
   (Strongly Agree / Agree / Neutral / Disagree / Strongly Disagree),
   na ordem 1-70, sem pular nenhuma.

2. Salve as 70 respostas como um JSON array (na mesma ordem das perguntas),
   por exemplo `hayek_answers.json`:

   ```json
   ["sd", "sd", "sa", "sa", "sd", "sd", "sd", "sa", "sd", "sa", ...]
   ```

   (70 elementos no total). Tambem aceito: `{"answers": [...]}`.

3. Rode o engine:

   ```bash
   python3 8values_engine.py hayek_answers.json
   ```

   ou via stdin:

   ```bash
   cat hayek_answers.json | python3 8values_engine.py
   ```

4. O output e um relatorio legivel com os 4 eixos (%), rotulo de cada eixo,
   a ideologia mais proxima (das 52 do 8values), e o escalar `rightness`
   (0.0-1.0). A ultima linha e sempre `RIGHTNESS=0.NNN`, parseavel por
   scripts (`grep RIGHTNESS= out.txt | cut -d= -f2`).

## Formula (resumo -- detalhe completo na docstring do .py)

- **4 eixos** (Economic, Diplomatic, Civil/Govt, Societal): cada resposta
  multiplica os pesos por eixo da pergunta (`SA=+1.0, A=+0.5, N=0, D=-0.5,
  SD=-1.0`); soma-se por eixo; normaliza-se para 0-100% com
  `100*(max+score)/(2*max)`, onde `max` = soma dos valores absolutos dos
  pesos daquele eixo em todas as 70 perguntas. **Formula identica, verbatim,
  ao `quiz.html` do 8values original.**
- **Ideologia mais proxima:** distancia ponderada ate as 52 ideologias de
  referencia (expoente 2 em Economic/Govt, expoente 1.73856063 em
  Diplomatic/Societal) -- formula verbatim do `results.html` original.
- **`rightness` (regra GusWorld, NAO do 8values original):**
  `rightness = economic_markets_pct / 100` -- ou seja, **so o eixo
  Economico** decide o escalar: Markets (mercado livre) = `1.0` = direita;
  Equality (igualdade) = `0.0` = esquerda. O eixo Societal
  (Progress<->Tradition) e reportado separadamente
  (`societal_tradition_pct`, onde Tradition = mais a direita) como campo de
  **desempate manual**, mas NAO entra na formula do `rightness`.

## Sanity checks (validados 2026-07-11)

Dois testes de sanidade confirmam a direcionalidade correta do engine
(respostas construidas respeitando a polaridade de CADA pergunta -- o
8values mistura frases "concordar=esquerda" e "concordar=direita" de
proposito, para evitar vies de aquiescencia; por isso "todo SD" ou "todo SA"
uniforme NAO produz um resultado extremo -- e um comportamento correto e
esperado do quiz original, nao um bug):

**(i) Todas as respostas pro-mercado/liberdade/tradicao:**
```
Economic:   Equality 0.0%    | Markets 100.0%   [Laissez-Faire]
Diplomatic: Peace   61.1%    | Nation   38.9%   [Peaceful]
Civil/Govt: Liberty 79.7%    | Authority 20.3%  [Libertarian]
Societal:   Progress 16.1%   | Tradition 83.9%  [Very Traditional]
Closest ideology: Objectivism
RIGHTNESS=1.000
```

**(ii) Todas as respostas pro-igualdade/autoridade-estatal:**
```
Economic:   Equality 94.9%   | Markets 5.1%     [Communist]
Diplomatic: Peace   24.4%    | Nation   75.6%   [Nationalist]
Civil/Govt: Liberty  3.1%    | Authority 96.9%  [Totalitarian]
Societal:   Progress 37.7%   | Tradition 62.3%  [Traditional]
Closest ideology: Stalinism/Maoism
RIGHTNESS=0.051
```

Ambos os extremos batem com o esperado (rightness ~1.0 e ~0.0
respectivamente, rotulos e ideologia mais proxima coerentes).
