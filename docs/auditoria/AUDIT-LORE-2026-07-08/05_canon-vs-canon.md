# 05. Contradições canon-vs-canon (C5)

> Consolidação: internal-auditor, 2026-07-08. Fontes brutas: `raw/L1.md`, `raw/L6.md`, `raw/L8a.md`, `raw/L8b.md`, `raw/L9.md`. Herda de T7/T10-v2 + `INCOHERENCES.md`.

## Contexto e método

Contradições de FATO (não de data) entre docs canônicos, incluindo splits sem resposta certa (exigem o criador) e migrações terminológicas incompletas (programas de retrofit). AXIOLOGIA verificada SEM inversão em todo o corpus de facções (Sterling = compadrio mau por distorcer; FIR boa-fé capturada; Ordem/Pilastra = austríaco bom; Polis-Vermelha coletivista = má evolução).

## Achados (5: 4 críticos, 1 importante)

### AL-C5-01 | 🔴 CRÍTICO | DECISÃO DO CRIADOR

Causa mortis oficial de Salviano Alencar (-8) divergente, e ambos os grupos afirmam ser a versão "oficial". Grupo A: `timeline.md:124` + diary (2 docs) = "acidente/overdose". Grupo B: `deep/settings/06-periferia.md:13,39` + `deep/characters/dante-grid.md:17` + `deep/factions/fir.md:15,55` + `deep/factions/sterling-corp.md:83` = "insuficiência cardíaca". Placar 4x3 pró insuficiência cardíaca (recomendação dos 2 executores, endossada), mas a escolha é do criador; retrofitar o grupo perdedor. Origem: L8a-05 + L8b-04. Estado: —

### AL-C5-02 | 🔴 CRÍTICO | DECISÃO DO CRIADOR

Morte de Aldebrando: split 2x2. Grupo A: `deep/characters/bento-requiem.md:25` "Catedral Menor de Atelaiá / Vigília dos 12 mestres / interferência ressonante" + `CHARS.md:126` concorda. Grupo B: `timeline.md:140` + `lore-bible.md:511` + `factions.md:128` = "catedral perdida / queda em ruína". Conflito novo (candidato a INCOHERENCES C20). Sem maioria clara; local E circunstância divergem. Origem: L6-02. Estado: —

### AL-C5-03 | 🔴 CRÍTICO | PROGRAMA + DECISÃO DO CRIADOR

Migração terminológica "Deck Rúnico" para Glyph/Token/Conjuro/Codex nunca propagou. Fonte A: `lore-bible.md:239` declara a substituição. Fonte B: `docs/design/pillars.md:46` + `docs/design/gdd.md:78-88` ainda usam "Deck Rúnico/cartas rúnicas"; "rúnico" sobrevive em **20+ arquivos** (deep-lore, book V1/V2, ADRs, TODO, style-guide). Decisão do criador: (a) retrofit amplo dos 20+ docs OU (b) reverter lore-bible §7.10 (re-legitimar o termo como sinônimo histórico). Item de PROGRAMA (ver onda 4 em `99_remediacao.md`). Origem: L1-04. Estado: —

### AL-C5-04 | 🟠 IMPORTANTE | PROGRAMA + DECISÃO DO CRIADOR

"Beat Ten Kishōtenketsu" usado como se fosse beat numeral 10 em ~20 docs (~15x em dante-grid + `timeline.md:124` + `factions.md:305` + `lore-bible.md:531` + `INCOHERENCES.md:127` + `diary/ui-spec.md:87`, onde é literal de UI, + bestiary); `arco-principal.md` só tem 8 beats ("Ten" 転 é o 3o sub-beat japonês, Twist, não numeral). Decisão do criador: (a) formalizar "Beat Ten" como sinônimo canônico do Twist em arco-principal.md OU (b) trocar por "Beat 8/Etapa Twist" nos ~20 docs. Atenção especial ao literal de UI (user-facing). Origem: L6-12. Estado: —

### AL-C5-05 | 🔴 CRÍTICO

Doc-autoridade de Cauã erra o empregador da família: `deep/characters/caua-volt.md:11` "apex-data sênior na **Nexus-Cloud**, Chapter 11 -19"; `timeline.md:104` = o Chapter 11 de -19 é da **Apex-Data** (Nexus é empresa distinta, -18/-15). Fix: "na Nexus-Cloud" para "na Apex-Data". Executor: narrative-writer. Origem: L6-05. Estado: —

## Conclusão

Os dois splits de morte (Salviano, Aldebrando) e os dois programas terminológicos são os 4 itens deste dossiê que NÃO podem ser corrigidos por executor nenhum sem o criador: são a pauta central da reunião de decisões (onda 0). Registrar os 2 splits em `INCOHERENCES.md` (C20/C21) quando decididos.
