# 08. Organização: CLAUDE.md, índices e registro (C8)

> Consolidação: internal-auditor, 2026-07-08. Fontes brutas: `raw/M2.md` (C8, Cosimo) + `raw/L1.md` + `raw/L8a.md`.

## Contexto e método

Saúde do mapa de onboarding (CLAUDE.md), dos índices (`deep/_INDEX.md`), da taxonomia de pastas e do registro de foreshadows. `INCOHERENCES.md` verificado SEM achado (C1-C19 todos RESOLVIDO com data, nenhum aberto sem dono).

## Achados (7: 2 críticos, 4 importantes, 1 cosmético)

### AL-C8-01 | 🔴 CRÍTICO

Seção "Estrutura de repositório" do `CLAUDE.md` descreve a era Godot: `engine/` (Godot) + `game/` (Godot) + `assets/{models,textures,...}` + `build/` na raiz. Real: `GusEngine/` (C++ core/domain/platform/app) + `resources/`; engine/game são legado dormente. O mapa de onboarding (lido por TODA sessão e todo agent) aponta para os lugares errados. Fix mecânico: reescrever a árvore (e conferir a seção "Comandos", também da era Godot). Origem: M2-ML-10. Estado: —

### AL-C8-02 | 🔴 CRÍTICO

`docs/narrative/deep/_INDEX.md` sistemicamente divergente do disco: era-2-compilador vs boom-tecnico; cult-mirage-reality vs cult-mirage; magic/* e stinger/* sem sufixo -deep; antologia ANT-NNN vs NN-slug + `_PLANO.md` ausente; e a pasta `antagonists/` INTEIRA (3 docs) invisível ao índice (que atribui Sterling/Patch-Zero/npcs a characters/). Só settings/ bate 100%. A tabela "Status rodadas R2-R10 pendente" também defasada (há muito mais escrito). Fix mecânico: regravar o índice a partir do find real + revisar a tabela de status. Origem: M2-ML-12. Estado: —

### AL-C8-03 | 🟠 IMPORTANTE

`CLAUDE.md` marca pillars/gdd/architecture/engine-modules/build como "AGUARDA REVISÃO USER"; real: pillars = Canônico, gdd = v0.2, os 3 tech = SUPERADOS (era Godot). Fix: atualizar os status na árvore de docs. Origem: M2-ML-09. Estado: —

### AL-C8-04 | 🟢 COSMÉTICO

Árvore `characters/` do CLAUDE.md lista 10 docs; disco tem 13 (faltam brunus-vetorial, brunus-conto, prelore_vilao). Fix junto com AL-C8-01. Origem: M2-ML-11. Estado: —

### AL-C8-05 | 🟠 IMPORTANTE | DECISÃO-LITE

Três camadas de "characters" (`characters/` raso, `deep/characters/`, `deep/antagonists/`) sem nota de fronteira em índice nenhum; `npcs-antologia.md` colide semanticamente com a pasta `antologia/`. Fix: nota de fronteira no `_INDEX.md` (mecânico) + criador decide se renomeia `npcs-antologia.md` (sugestão do executor: npcs-antagonistas; rename exige passe de refs). Origem: M2-ML-13. Estado: —

### AL-C8-06 | 🟠 IMPORTANTE

Em-dash em docs de design: `docs/design/pillars.md` (32) + `docs/design/gdd.md` (15) violam o anti-pattern global "zero em-dash" (os 6 docs narrativos do lote L1 têm 0). Consequência operacional: edits futuros nesses docs esbarram no hook `no_mdash.py`. Fix mecânico: find-replace (vírgula/parêntese/dois-pontos conforme o caso). Origem: L1-10. Estado: —

### AL-C8-07 | 🟠 IMPORTANTE

Nó de sequel órfão: `deep/settings/03-catedrais.md:72` planta setup (2 catedrais reservadas + expedição Hugo Tirol) sem registro em `foreshadowing.md` nem em `brainstorm-backlog.md`. Fix mecânico: catalogar (entrada F-numerada ou seed no backlog). Origem: L8a-08. Estado: —

## Conclusão

Os dois críticos são mapas mentirosos (CLAUDE.md e deep/_INDEX.md): não corrompem canon, mas direcionam TODO trabalho futuro para o lugar errado, o que os torna os fixes mecânicos de melhor custo-benefício do dossiê inteiro. INCOHERENCES.md, em contraste, é o artefato de registro mais saudável do projeto.
