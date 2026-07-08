# 06. Easter eggs rotulados em docs públicos (C6)

> Consolidação: internal-auditor, 2026-07-08. Fontes brutas: `raw/L5.md`, `raw/L6.md`, `raw/L8b.md`, `raw/L9.md`. Herda de T5-v2 + memórias `project_fibonacci_easter_egg`, `project_eastereggs_maconaria_canonica`, `feedback_easter_eggs_nao_rotular_docs_publicos`.

## Contexto e método

O repo é PÚBLICO e a regra canônica é: os dois sistemas de easter eggs (Fibonacci + maçonaria) são pervasivos no CONTEÚDO mas nunca ROTULADOS (leitor familiar reconhece; leigo não nota; manual só em `docs/_secret` + memórias). Os lotes greparam rótulos literais. Resultado: a delabelização nunca foi executada como passe único de corpus; 5 achados brutos de 4 lotes convergem para 1 programa.

## Achado (1 crítico, formato PROGRAMA)

### AL-C6-01 | 🔴 CRÍTICO | PROGRAMA DE DELABELIZAÇÃO (full-corpus)

Rótulos literais dos dois sistemas de eggs presentes em docs públicos. Uma vez lido no repo público, o velado quebra permanentemente, por isso a severidade máxima e o formato de passe único.

**Frente A: rótulo "Fibonacci" literal (20+ docs).** Loci confirmados com linha: `deep/settings/02-selve-sombria.md:19,29,75` + `deep/settings/08-selve-profunda.md:31` (espécie nomeada "abelha-Fibonacci"); `environments/07-zona-do-silencio.md:157` ("padrão Fibonacci" explícito); `deep/antagonists/patch-zero-deep.md:31` (abelha-Fibonacci); `characters/jaci-proxy.md:107` ("Não é Fibonacci; é arbitrário", rótulo por negação); `deep/factions/pelicano-branco.md`. Grep do executor L5 lista ainda: in-world-docs, diary/* (incl. mapas-timeline), `deep/magic/natureza-matematica-rigida-deep.md`, antologia/01,02,03,05,07,10, lingua/01, `design/brainstorm-backlog.md`, `design/mecanicas/` (battle-screen, combat, economia).

**Frente B: rótulos maçônicos (3 docs, 6 loci).** `deep/factions/ordem-recursiva.md:15,78` "esquadro e compasso gravados"; `deep/factions/underground-silencio.md:47,95` "compasso e esquadro"; `deep/factions/cult-mirage.md:108` "olho-no-triângulo das sociedades eruditas". Contra-exemplo que FAZ CERTO (usar como padrão de reescrita): `deep/factions/fir.md:55,110` "instrumento de medida pareado cortado ao meio".

**Fixes por classe:**
1. Espécie "abelha-Fibonacci": renomear em TODAS as ocorrências (sugestões dos executores: "abelha-espiral" ou "abelha-áurea"; nome final = decisão rápida do criador dentro do programa).
2. Qualificadores ("padrão Fibonacci", "Não é Fibonacci"): perifrasear ("padrão numérico recorrente" etc.).
3. Maçonaria: substituir os 3 rótulos por perífrases veladas no padrão fir.md; em cult-mirage:108 remover a cláusula "das sociedades eruditas" e manter só "olho refratado em prisma".

**Sub-decisão de escopo (criador):** docs de NARRATIVA públicos = delabel mandatório. Docs de design/mecânicas (`design/mecanicas/*`, `brainstorm-backlog.md`) usam "Fibonacci" como termo de implementação (curvas de HP/dano/economia); decidir se (a) também perifraseiam ou (b) são aceitos como técnicos (o repo é público, então a recomendação do auditor é perifrasear também, ou mover o miolo para `docs/_secret`).

**Execução:** narrative-writer, passe único full-corpus, com grep de re-teste ao final (`Fibonacci`, `esquadro`, `compasso`, `olho-no-triângulo` = zero hits fora de `docs/_secret/` e `docs/auditoria/`). Origem: L5-05 + L6-10 + L8b-11 + L9-07 + L9-08. Estado: —

## Classes verificadas limpas

L1 (8 docs centrais): zero rótulo. L8a: nomes-criatura do Bloco F pré-existentes não são rótulos novos. Nenhum lote achou sigla, gesto ritual nomeado ou em-dash/palavrão associado.

## Conclusão

O conteúdo dos eggs está bem executado (densidade e sutileza OK); o vazamento é exclusivamente de RÓTULO, concentrado no cluster deep de 2026-05-19 e nos docs de design. Um único passe resolve, e o grep de re-teste vira check permanente (candidato a hook local pré-push).
