# BRIEF A — revisor-textual (auditoria de CONTEÚDO do lore, em lotes)

> Você recebeu um LOTE-ID no prompt (ex.: L1, L5, L6, L8a, L8b, L9...). Leia este preâmbulo + o bloco do seu lote na tabela abaixo, execute, e devolva os achados como TEXTO na resposta final (NÃO escreva arquivo de relatório). READ-ONLY absoluto: não edite, não commite, não pushe.

## Preâmbulo comum (vale para todo lote)

Você é o revisor-textual executando um lote da AUDIT-LORE do GusWorld. Dir: `/home/petrus/IDrive/Documentos/projetos_claudebrain/Projects/gusworld`.

**ÂNCORAS DE VERDADE** (leia primeiro, sempre): `CHARS.md` (personagens canônicos: nome exato, idade, aparato), `PLACES.md` (lugares canônicos), `docs/narrative/timeline.md` (cronologia 3 eras), `sinopse.md` (base imutável). Em conflito doc-vs-âncora, a âncora costuma ser o canon — mas REPORTE o conflito, não presuma (há casos em que o deep-lore atualizou e a âncora ficou pra trás; `docs/narrative/INCOHERENCES.md` C1 é precedente).

**CONVENÇÕES DE DATA:** timeline usa anos relativos (negativos = passado). Era 1 pré-código; Era 2 boom técnico (Gustaf I funda setor GusWorld City ~-150); Era 3 Sterling (presente, Gus tem 11). Encapsulamento = -800. TODA idade/intervalo deve fechar aritmeticamente: se um doc diz "X tinha N anos no evento E (ano A)", nasceu em A-N; confira contra qualquer outra menção de nascimento/idade/morte de X em qualquer doc do lote ou âncora. Caso-exemplo real: um doc dizia idade-21 de Helíaco Vyr = -950, o que o tornaria 171 anos no Encapsulamento; a cadeia correta (sonho aos 34 = -820) dá ~-833. Cace ESSA classe de erro sistematicamente: monte tabela mental (personagem, evento, ano, idade declarada) e verifique cada linha.

**CLASSES DE ACHADO (numere pela classe):**
- **K1 CRONOLOGIA** — data/idade que não fecha aritmeticamente, dentro do doc ou cross-doc.
- **K2 PERSONAGEM** — nome fora do formato canônico exato (Gus Vector Tavus Vance; companions `Nome "Codinome" Sobrenome`; Sterling Locke; Patch-Zero), idade divergente, traço/aparato contradito (óculos táticos / Matriz Ortodôntica / Tavus-Drive), personagem citado que NÃO existe em CHARS.md (= erro OU CHARS.md desatualizado — reportar ambos os lados).
- **K3 LUGAR** — lugar citado ausente de PLACES.md, grafia divergente, geografia contradita.
- **K4 CROSS-REF** — ref backtick `arquivo.md` ou `arquivo.md:linha` cujo alvo não existe, foi renomeado (renames de 2026-07-08: `ontologia/cosmologia-deep.md` → `cosmologia-formal-deep.md`; `eras/cosmologia-deep.md` → `eras/cosmologia-origem-deep.md`), ou cuja linha citada não contém mais o conteúdo referido.
- **K5 CANON-VS-CANON** — dois docs canônicos afirmam fatos incompatíveis (evento, nº de mortos, causa, facção, mecânica de magia, axiologia). Consulte `INCOHERENCES.md` antes de reportar: se já listado como RESOLVIDO, verifique se a resolução pegou nos arquivos do seu lote; se pegou, não re-reporte.
- **K6 EASTER EGG ROTULADO** — o repo é PÚBLICO. Fibonacci e simbologia maçônica são canon PERVASIVO mas VELADO: os elementos ficam, os RÓTULOS não. Reporte qualquer menção explícita tipo "easter egg", "Fibonacci" como rótulo, "maçonaria/maçônico", "Pigpen", "Hiram Abiff" em docs públicos (`docs/_secret` e `docs/auditoria` estão FORA do escopo K6). A sequência numérica em si (3,5,8,13,21...) NÃO é achado.
- **K7 NÓ MORTO** — setup/foreshadow/personagem introduzido que nenhum outro doc paga ou referencia, sem registro em `foreshadowing.md` ou `brainstorm-backlog.md`.
- **K8 TEXTO** — ortografia/gramática/em-dash fora das exceções (exceções canônicas de em-dash: prosa in-character de `in-world-docs.md`; travessão de fala em contos da antologia).

**FORMATO DE ACHADO** (um por linha): `AL-<lote>-<nn> | severidade | classe K1-K8 | Fonte A arquivo:linha + citação curta | Fonte B arquivo:linha + citação curta | descrição 1-2 frases | cascata (outros docs que repetem o erro — grep antes de fechar) | fix sugerido (NÃO aplicar)`. Severidade: **CRITICO** = contradição que propaga cross-doc / aritmética impossível / ref quebrada em doc central / easter egg rotulado; **IMPORTANTE** = inconsistência local/não-propagante / ref linha-errada / arco órfão / stale; **COSMETICO** = texto/formatação. **Sem evidência dupla verificada = não reporte.** Zero achados numa classe = declarar "K<n>: nada encontrado" (silêncio não é evidência de limpeza). Um erro nunca é assumido isolado: dê grep de cascata antes de fechar cada achado cross-doc.

---

## Tabela de lotes (execute SÓ o seu LOTE-ID)

| Lote | Arquivos (além das 4 âncoras) | Foco especial |
|---|---|---|
| **L1 CANON CENTRAL** | `docs/narrative/lore-bible.md`, `timeline.md`, `arco-principal.md`, `factions.md`, `tradicoes-cultura.md`, `comic-reliefs.md`, `docs/design/pillars.md`, `docs/design/gdd.md` | O hub contra o hub: as âncoras se contradizem ENTRE SI? Toda data da timeline fecha? Endings do arco vs gdd? |
| **L2a/b/c ERA-1** | `deep/eras/era-1-pre-codigo.md` fatiado em 3 (por marcadores `## §`) | Cadeia Helíaco Vyr completa; datas negativas vs timeline; conto 14 da antologia como contraprova |
| **L3a/b ERA-2** | `deep/eras/era-2-boom-tecnico.md` fatiado em 2 | Gustaf I ~-150; genealogia 7 gerações até Gustaf VII (aritmética de gerações); fundação do setor |
| **L4 ERAS-RESTO + ONTOLOGIA** | `deep/eras/era-3-sterling.md`, `transicoes-entre-eras.md`, `eras/cosmologia-origem-deep.md`, `deep/ontologia/cosmologia-formal-deep.md`, `tech-3-eras-deep.md`, `leitmotivs-deep.md`, `leitmotivs-musicais-detalhados.md` | Os DOIS cosmologia-* pós-rename: escopos disjuntos ou se contradizem? transições batem com era-1/2/3? leitmotivs duplicados entre os 2 docs? |
| **L5 PERSONAGENS RASOS** | `docs/narrative/characters/` (todos, ~15 docs incl. brunus-vetorial, conto, prelore_vilao, party.md) | Matriz de linguagens em party.md vs fichas; brunus vs CHARS.md; prelore_vilao vs sterling-locke canônico |
| **L6 PERSONAGENS DEEP + ANTAGONISTAS** | `deep/characters/` + `deep/antagonists/` | dante-grid double-layer coerente com arco-principal (traição); gus-dragon vs endings; npcs vs CHARS.md |
| **L7a/b ANTOLOGIA** | `deep/antologia/01-07` / `08-14` | Datas de vida dos biografados vs timeline/CHARS; conto 14 (Helíaco) = ponto quente K1; travessão de fala PERMITIDO aqui (não reporte K8 por isso) |
| **L8a SETTINGS DEEP** | `deep/settings/01-08` | Cada setting vs PLACES.md linha a linha |
| **L8b ENVIRONMENTS** | `docs/narrative/environments/01-08 + _INDEX` | **Par a par com o settings homólogo** (01↔01 ... 08↔08): dois clusters paralelos do mesmo tema, maior risco K5 do corpus. Leia o par settings correspondente como quinta âncora |
| **L9 FACÇÕES DEEP** | `deep/factions/` (7) + releia `factions.md` | NPCs de facção vs CHARS.md; eventos vs timeline; axiologia (coletivismo→ruim, capitalismo/valores conservadores→bom; Sterling = compadrio) sem inversão acidental |
| **L10 MAGIA + LÍNGUA** | `deep/magic/` (3) + `deep/lingua/00-03` | Glyph/Token/Conjuro/Codex uniforme; 4 linguagens vs matriz party.md; gramática Sylvarin consistente entre os 4 docs (mutação consonantal, 13 raízes) |
| **L11a DIARY** | `docs/narrative/diary/` (8) | IDs DD-nnn únicos e consistentes com in-world-docs.md; knowledge-gates |
| **L11b IN-WORLD + FORESHADOW + STINGER** | `in-world-docs.md`, `foreshadowing.md`, `deep/stinger/` | K7 em massa: cada foreshadow tem payoff? stinger vs sequel-hooks coerentes; exceção em-dash vale em in-world-docs |
| **L12 BOOK BIBLE** | `docs/book/` (11) | Docs DERIVADOS: divergência V1 vs V2; GLOSSARIO vs terminologia canon; INDICE aponta pra estrutura real? |

Se o seu lote tiver sufixo (a/b/c), audite só a fração indicada (divida o arquivo grande por marcadores `## §`).
