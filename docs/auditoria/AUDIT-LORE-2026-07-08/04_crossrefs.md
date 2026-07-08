# 04. Integridade de cross-refs (C4)

> Consolidação: internal-auditor, 2026-07-08. Fontes brutas: script mecânico L0 + `raw/L1.md`, `raw/L5.md`, `raw/L6.md`, `raw/L8a.md`, `raw/L8b.md`, `raw/L9.md`. Herda de T8-v2.

## Contexto e método

Varredura mecânica (L0) das ~1018 refs backtick (217 com número de linha) + verificação manual por lote de renames residuais (`cosmologia-deep.md` foi renomeada em `cosmologia-formal-deep.md` / `cosmologia-origem-deep.md`).

## Achados (1 crítico)

### AL-C4-01 | 🔴 CRÍTICO

Ref quebrada em doc canônico central: `PLACES.md:70` aponta para `cosmologia-deep.md:67`, arquivo que não existe mais pós-rename. Fonte A: `PLACES.md:70`. Fonte B: inexistência do alvo no disco (L0) + confirmação manual (L1). Fix mecânico: apontar para `deep/eras/cosmologia-origem-deep.md` (o conteúdo antropológico dos clãs vive lá) e revalidar o número de linha. Origem: L0 + L1-06. Estado: —

## Classes verificadas limpas

- L5 (characters/): cross-refs OK, nenhum nome renomeado residual.
- L6 (deep/characters + antagonists): refs OK.
- L8a/L8b (settings/environments): todos os cross-refs OK, nenhum rename obsoleto.
- L9 (factions): refs + códigos DD-nnn OK.

## Conclusão

Superfície de refs saudável: 1 única quebra, justamente na âncora PLACES.md e causada pelo rename conhecido da cosmologia. Recomendação preventiva: incluir o check mecânico de refs (script L0) como passe periódico pós-rename de qualquer doc deep (candidato a hook/rotina local).
