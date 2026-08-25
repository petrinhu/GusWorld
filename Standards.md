# Standards.md - Índice dos Manuais do GusWorld

Hub dos manuais deste repositório. Todos vivem na **raiz** deste projeto.

**Autoridade acima de todos eles: `GODS_LAWS.md`.** Onde qualquer manual abaixo contradisser uma lei do `GODS_LAWS.md`, a lei vence e o manual está errado; corrija o manual, nunca a lei.

## Manuais canônicos

- `CONTRACT.md` : AI Coder Contract (padrões de código, SOLID, Clean Code, Git, RFC 2119). Onde a L-22 substitui uma cláusula dele, a L-22 vence.
- `TESTES.md` : guia de testes, qualidade e auditoria, adaptado ao TDD estrito e aos cinco portões da L-19.
- `AGILE.md` : metodologia ágil (Manifesto, Scrum, Kanban, PDCA, INVEST, SAFe) aplicada ao ritmo de sprint deste projeto.
- `DEPLOY_CHECKLIST.md` : checklist de release irreversível (tag, publicação, quebra de compatibilidade de save), ancorado na L-23.
- `AUDITORIAS.md` : índice das auditorias do GusWorld, ancoradas nos cinco portões da L-19 e na proteção de dado da L-25.
- `TOOLING.md` : ferramentas FOSS do stack C++23 deste projeto (build, análise estática, sanitizers, scan de segredo).

## Organização e pipeline (constelação de agents)

- `ORG.md` : como a constelação bigtech é usada neste projeto, sob a L-10 (main só orquestra; auditoria e arquitetura só com C-level `fable`; implementação com agente operacional `sonnet`).
- `pipeline_release_1.0.md` : fases do pipeline aplicáveis a um jogo FOSS single-player.
- `lideranca_pipeline_release.md` : quem, na constelação, responde por cada fase deste projeto.

## Outros manuais

- `TEXTREVIEW.md` : protocolo de revisão textual dos dois livros-companheiros do GusWorld (canon "Ordem Recursiva", ver L-08).

## Como usar com Claude

O `CLAUDE.md` da raiz deste projeto manda ler `GODS_LAWS.md` antes de agir. Os manuais acima são consultados conforme a tarefa, nunca por cima da lei.