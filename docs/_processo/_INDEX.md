# docs/_processo

Relatórios de análise que sustentam as decisões registradas no `GODS_LAWS.md` e na `TODO.md`.
Produzidos em 21/08/2026, na sessão de fundação do projeto reiniciado.

Não são canon de jogo nem norma: são **o raciocínio por trás** das decisões. A lei diz o que vale;
estes documentos dizem por que, com quem, e contra que alternativas.

| Arquivo | O que é | Autor |
|---|---|---|
| `opcoes-arquitetura-gusworld.md` | As três arquiteturas comparadas; base da L-17 | Caetano (CTO) |
| `opcoes-protecao-save.md` | Modelo de ameaça e opções de proteção; base da L-25 | Narciso (CISO) |
| `parecer-licenca-catalogo.md` | Statline é código ou asset; base da L-08 e da L-25 | Cláudio (CLO) |
| `auditoria-privacidade.md` | Varredura de dado pessoal antes do primeiro commit. **Movido para `docs/_secret/` em 22/08/2026**: concentra 17 ocorrências de 12 termos protegidos e é, por natureza, o relatório do próprio vazamento. Só se lê com a chave do `git-crypt`. | security-engineer |
| `mapa-corpus-gusworld.md` | Inventário do corpus documental (57 mil linhas) | technical-writer |
| `candidatos-revogacao.md` | O que estava revogado, por grupo; base da limpeza | technical-writer |
| `mineracao-roadmap-todo-antigos.md` | Rodada 1: cortes de escopo e lições; base da L-29 | technical-writer |
| `mineracao-rodada-2.md` | Rodada 2: design, roster, topologia, save por local | technical-writer |
| `dossie-pixellab.md` | A API do PixelLab, levantada exaustivamente | applied-ai-engineer |
| `lente-*.md` | As quatro lentes que geraram a tabela de pendências | quatro agentes |

**Origem:** estes arquivos nasceram no diretório temporário da sessão (`/var/tmp/...`), que é volátil.
Foram trazidos para cá em 21/08/2026 para não se perderem.

## Auditorias de lore resgatadas do `gusworld_legacy` (25/08/2026)

Trazidas inteiras pelo `technical-writer`, na varredura profunda de narrativa, com cabeçalho de aviso no topo de cada uma (o que já foi confirmado resolvido contra a árvore atual, o que não foi reconferido). Nenhuma foi declarada morta (L-14 do `GODS_LAWS.md`).

- `AUDIT-T1-NOMES.md` — auditoria v1 (22/05/2026) de nomes de personagem cross-doc, 98 achados; sucedida pela v2 abaixo.
- `AUDIT-T1-NOMES-V2.md` — auditoria v2 de nomes; os 4 críticos residuais já estão resolvidos na árvore atual (conferido 25/08/2026).
- `AUDIT-T2-LUGARES.md` — auditoria v1 de lugares cross-doc, 25 achados; sucedida pela v2 abaixo.
- `AUDIT-T2-LUGARES-V2.md` — auditoria v2 de lugares; os 2 críticos residuais já estão resolvidos na árvore atual (conferido 25/08/2026).
- `AUDIT-T3-CRONOLOGIA.md` — auditoria v1 de cronologia cross-doc, 31 achados; sucedida pela v2 abaixo.
- `AUDIT-T3-CRONOLOGIA-V2.md` — auditoria v2 de cronologia; os 4 críticos residuais (idade do Bento no incidente Patch-Zero) estão resolvidos na árvore atual, exceto uma menção em `diary/entries-manuscrito-glossario.md:425` que pode ser informação legítima e distinta — achado a confirmar com o líder.
- `AUDIT-T4-VOZ-V2.md` — auditoria de fadiga de prosa ("voz Stephenson") concentrada em `era-2-boom-tecnico.md`; 71 achados, nenhum reconferido (é julgamento editorial, não fato de canon). Candidato a passe do `narrative-writer`.
- `AUDIT-T6-PALAVRAS-V2.md` — auditoria de palavras/pontuação proibida; o achado principal (992 travessões/em-dash em prosa canônica) está hoje reduzido a 130 ocorrências em todo `docs/narrative/` (~87% de queda, conferido 25/08/2026), mas não foi feita varredura arquivo a arquivo do que resta.

A auditoria de easter eggs (T5) tem lugar em `docs/_secret/`, cifrada — ver o `_INDEX.md` de lá.

## Quatro auditorias adicionais resgatadas do `gusworld_legacy` (01/09/2026)

Completam a cascata `AUDIT-T1` a `AUDIT-T10-V2` iniciada na seção acima. `docs/narrative/INCOHERENCES.md` já citava `AUDIT-T9-VOZ-NPC-V2` e `AUDIT-T10-AXIOLOGIA-V2` como fonte de autoridade antes de estes arquivos existirem neste repositório — a citação ficou órfã entre 25/08/2026 (migração parcial de T1-T6) e esta data. Trazidas como estão, sem reconferência contra a árvore atual — quem quiser aplicar os achados críticos confere primeiro.

- `AUDIT-T7-PILLARS-V2.md` — consistência dos Pillars 1-5 cross-doc; 17 achados (3 críticos, 6 médios, 8 leves), veredito "aprovar com fixes médios"; os 3 críticos seguem pendentes de reverificação (fora do escopo das incoerências C1-C15 já fechadas).
- `AUDIT-T8-CROSSREF-V2.md` — integridade de wikilinks `[[X]]` cross-doc (115 docs, 1.293 links, ~98% de resolução correta); 20 achados (5 críticos, 7 médios, 8 leves), veredito "aprovar com fixes médios"; os 5 críticos são de sintaxe de link, não factuais, e um deles (CRIT-T8v2-05) aponta que `~/.claude/agents/revisor-textual.md` referencia `/INCOHERENCES.md` num caminho que só existe como `docs/narrative/INCOHERENCES.md`.
- `AUDIT-T9-VOZ-NPC-V2.md` — consistência de voz de NPC in-character cross-doc (14 contos da antologia, 23 DDs, 9 personagens); 9 achados (3 críticos, 4 médios, 2 leves), veredito "aprovar"; os críticos são de prosa de conto/estilo, pendentes.
- `AUDIT-T10-AXIOLOGIA-V2.md` — axiologia canônica binária cross-doc (~4.500 linhas, 9 docs de canon crítico); 31 achados (6 críticos, 11 médios, 14 leves), veredito "aprovar com ressalvas médias"; o próprio documento declara a cascata "T1-T10 v2 completo, 10/10 tópicos auditados sequencialmente".
