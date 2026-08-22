# Diário do Gus: Índice canônico (Bloco H)

> **Status:** Canônico (Bloco H, Diário do Gus). Revisão inicial 2026-05-16. Modo autônomo autorizado pelo criador supremo. Decisões macro fechadas via AskUserQuestion prévio.
>
> **Escopo:** este `_INDEX.md` é o ponto de entrada da pasta `docs/narrative/diary/`. Define o princípio do Diário, lista os 8 docs canônicos, registra as decisões macro do Bloco H e amarra cross-refs imutáveis.
>
> **Cross-refs imutáveis:** [[lore-bible]] · [[pillars]] · [[arco-principal]] · [[in-world-docs]] · [[factions]] · [[timeline]] · [[tradicoes-cultura]] · `characters/*` · [[sinopse]] (base imutável).

---

## Princípio do Diário

O Diário do Gus é o **veículo tangível da Knowledge Progression** (Pillar 1 / sistema-âncora) e, ao mesmo tempo, **extensão diegética do prodígio analítico de 11 anos** (Pillar 4). Não é wiki abstrata flutuando fora do mundo: é caderno físico que Gus carrega, escreve à mão (cursiva infantil de 11 anos), cola adesivo, dobra ponta, mancha de café-de-neurônio do pai. Toda página que aparece dentro dele responde a uma cena vivida, um item recolhido, um inimigo derrotado, ou uma anomalia observada na Selve.

Funcionalmente, o Diário traduz "mostre, não conte" para interface: **lore não vira monólogo expositivo; vira página descobrível**. Bestiário cresce por kill repetido (Knowledge Progression). Cartas de carta cresceram por uso (Pillar 1, mestria por uso). Documentos in-world (ver [[in-world-docs]]) entram como pickup. Fichas de personagens se atualizam quando Gus aprende algo novo sobre o companion. Mapas se preenchem com exploração. Timeline cresce automatica conforme beats canônicos passam. **O número exato de Knowledge fica no HUD principal (técnico); o Diário é a versão narrativa do mesmo dado** (entries parciais com texto borrado e esboços incompletos = baixa Knowledge; entries cristalinas = alta).

Tudo o que está aqui amarra direto a Pillar 1 (Lógica vence força, Knowledge como anti-grind), Pillar 2 (sistema formal computável, Gus organiza informação como engenheiro organiza repositório), Pillar 3 (triângulo de hardware: Óculos Táticos capturam, Matriz amplifica, Tavus-Drive executa; Diário arquiva) e Pillar 4 (prodígio de 11 anos, letra de criança curiosa, não tratado acadêmico adulto).

---

## Quick-ref dos 8 docs canônicos

| # | Doc | Linha descritiva |
|---|---|---|
| 1 | [[_INDEX]] (este) | Princípio, decisões macro, cross-refs. |
| 2 | [[ui-spec]] | UI/UX do Diário in-game: caderno diegético, 4 abas, wireframes ASCII, interação, popup HUD, estados de Knowledge, acessibilidade, save, plataforma. |
| 3 | [[entries-manuscrito-glossario]] | Tipo 1: entries autobiográficas de Gus (manuscrito cursivo) + glossário técnico (C-Arcane, Asmódico, Óxido, Pythia, Tavus-Drive, etc.). |
| 4 | [[entries-docs-descobriveis]] | Tipo 2: pickup catalog dos 15 documentos in-world ([[in-world-docs]]) com metadata de descoberta, gates, função no Diário. |
| 5 | [[entries-fichas-bestiary]] | Tipo 3: fichas dos 6 companions + Sterling + Patch-Zero + bestiary (inimigos comuns/raros/bosses) com Knowledge gating progressivo. |
| 6 | [[entries-mapas-timeline]] | Tipo 4: mapas dos 8 settings (Pillar 5) com fog-of-war diegético + timeline auto-gerada (50+ eventos cross-eras, ver [[timeline]]). |
| 7 | [[knowledge-gates]] | Tabela canônica de gates Bronze/Prata/Ouro: que % de Knowledge destrava o quê, branchings que abrem entries, endings impactados. |
| 8 | [[foreshadow-links]] | Cross-link Bloco I: mapeamento dos 130 plants de foreshadowing (Dante traidor, Patch-Zero, Sterling, Catedral-Mãe) com setup, payoff, entry no Diário. |

---

## Decisões macro registradas (Bloco H, fechadas)

1. **Escopo:** pasta modular `docs/narrative/diary/` com 8 docs (este + ui-spec + 4 entries + knowledge-gates + foreshadow-links).
2. **Tipos de entry:** 4 categorias canônicas: (a) manuscrito + glossário, (b) docs descobríveis, (c) fichas de personagem + bestiary, (d) mapas + timeline.
3. **Knowledge Progression, modelo híbrido:** score numérico **visível em HUD principal** (técnico, calculado); Diário é **manifestação narrativa** do mesmo score (entries não exibem número exato; mostram estado: parcial / completo / com cross-links). Player olha HUD para saber "quanto"; abre Diário para entender "o quê".
4. **Visual, híbrido caderno diegético:**
   - **Layer 1 (frame):** caderno físico estilizado. Capa rabiscada, etiqueta escolar riscada, adesivos, dobra de canto, manchas de café-de-neurônio, fita adesiva consertando lombada, abas físicas separando categorias, sticky notes coloridos como atalhos.
   - **Layer 2 (conteúdo):** fonte clara legível dentro das páginas. Cursiva de Gus aparece em margens (anotações curtas) e cabeçalhos. Justificativa: imersão visual + acessibilidade de leitura.
5. **Trigger, misto contextual:**
   - **Manuscrito:** auto-gera pós-cenas chave (beats Kishōtenketsu, mortes off-screen, primeiros encontros emocionais); player tem insight opcional pra forçar entrada de reflexão extra.
   - **Docs descobríveis:** pickup (ver [[in-world-docs]], 15 docs).
   - **Fichas:** primeiro encontro cria stub; cada interação significativa atualiza.
   - **Bestiary:** pós 1º combate cria stub (1 pág); kills repetidos completam até 4 págs (mestres raros).
   - **Mapas:** preenche por exploração + scan dos óculos.
   - **Timeline:** automatic; cada beat canônico cumprido adiciona linha.
6. **Player agency:** **zero anotações livres.** Leitura passiva. Nenhum input de texto livre pelo jogador. (Razão: scope solo G1 + tom analítico Gus narra pelo player; agency vai pra escolhas de combate / branching, não para "diário simulator").
7. **Cross-link Bloco I:** **pesado.** Doc dedicado `foreshadow-links.md` mapeia os 130 plants de foreshadowing. Cada plant aponta entry-setup + entry-payoff. Knowledge alta destrava sticky-notes que conectam visualmente plant → payoff dentro do Diário (player vê linha de costura entre as páginas).

---

## Cross-refs imutáveis (base canônica que o Diário consome)

| Doc fonte | O que o Diário extrai |
|---|---|
| [[lore-bible]] | Eras, geografia, ecossistema Selve, linguagens, Patch-Zero, cidades-irmãs, facções, cultura, eventos cross-eras. |
| [[pillars]] | Pillar 1 (Knowledge Progression), Pillar 4 (tom de 11 anos), Pillar 5 (8 settings → 8 mapas). |
| [[arco-principal]] | 8 beats Kishōtenketsu → 8 entries-manuscrito obrigatórios; ramificações ending (Bronze/Prata/Ouro). |
| [[in-world-docs]] | 15 docs descobríveis, 3 gate Ouro (11, 13, 15). Diário replica metadata + entry-pickup. |
| [[factions]] | Fichas de aliados, antagonistas secundários, hierarquias internas. |
| [[timeline]] | 50+ eventos cross-eras → entries timeline auto-geradas. |
| [[tradicoes-cultura]] | 9 tradições → entries de glossário cultural quando Gus participa. |
| `characters/gus.md` + `characters/*.md` | Voice (cursiva 11 anos), wounds dos companions, memórias formativas. |
| [[sinopse]] | Base imutável; Diário nunca contradiz. |
| `Resources/gusworld/_INDEX.md` | Specs visuais canônicas: caderno usa paleta caderno-papel-envelhecido + acento do Gus (laranja `#FF6B1A` em sticky notes-âncora). |
| `art/style-guide.md` | Paleta restrita; Diário respeita pillars visuais (silhueta caderno reconhecível em 3s; sticky notes funcionam em daltonismo via forma + posição além de cor). |

---

## Notas de design

- **Tom da cursiva é Gus aos 11.** Não é caligrafia adulta polida. Letras irregulares, alguns "i" sem pingo, palavras técnicas misturadas com gírias infantis ("Compila ou Crasha"), pausa de 1 segundo antes de frases novas refletida em mudança de pressão da caneta. Voice canônica: `characters/gus.md` §Voice.
- **Letra de criança ≠ rabisco de criança.** Gus tem 11 e é prodígio: a caligrafia é cuidada para padrões pessoais (ex: tipografia consistente para tokens de C-Arcane). Inconsistência onde tem emoção; consistência onde tem técnica. **Subtexto via forma.**
- **Diário é safe space narrativo.** Mesmo quando o jogo escurece (arco Dante, Polis-Vermelha, ato 3), abrir o Diário **não disponibiliza nem jumpscare nem horror direto**. Patch-Zero **invade** o Diário em momentos específicos (4-canais, ver [[lore-bible]] §8.3 canal 1). Quando invade, é evento, não ambient.
- **Pillar 4 obrigatório:** zero linguagem profana, zero "droga"/"merda". Quando Gus quer expressar frustração: tecnicismo ("isso aqui é um vazamento de memória emocional") ou metáfora científica.
- **Acessibilidade:** layer 2 (texto limpo) tem fonte ajustável + opção de substituir cursiva da layer 1 por bloco-letra. WCAG AA mínimo. Detalhes em `ui-spec.md` §8.
- **Performance:** caderno é sprite atlas único; entries renderizam texto on-demand. Save serializa apenas IDs de entries destravadas + estado de leitura. JSON versionado `save_version: 1` (alinhado com decisão Fase 1).

---

## Anti-patterns padrão (checklist de rejeição)

- **Lore dump** dentro de entry (>200 palavras de texto sem quebra visual). Reprovado: entries são fragmentos curtos.
- **Adultos como autores ocultos** do Diário. Reprovado: Diário é do Gus, voz do Gus. Outros NPCs aparecem só via documentos descobríveis (in-world-docs.md), não como autores do Diário.
- **Anotação livre do jogador.** Reprovado: zero agency de texto.
- **Spoiler antecipado.** Reprovado: entry só aparece quando trigger acontece. Foreshadow é sticky-note conectiva, não vazamento.
- **Em-dash horizontal** (`U+2014`) em narrativa. Reprovado: vírgula, parênteses, dois-pontos ou ponto-final. (Anti-pattern global, ver CLAUDE.md raiz.)
- **Vocabulário fora do canônico.** Reprovado: "runa", "Iolanda", "Kira", "Dimi", "Tao" (exceto Tao Berisi canônico), "Sindicato dos Ferro-Velhos". Glossário oficial: [[lore-bible]] §7.10.
- **Romance** entre crianças. Reprovado (Pillar 4).
- **Voice adulta no Gus.** Reprovado: 11 anos, tecnicismo precoce + tic de pausa, não filosofia adulta.
- **Mistura de tipo de entry.** Reprovado: cada categoria mora na aba dela. Manuscrito não vira ficha; ficha não vira mapa.

---

## Mapa de leitura recomendado (para implementação)

1. Ler este `_INDEX.md` primeiro (princípio + decisões macro).
2. Ler [[ui-spec]] (frame de interface; define como tudo é apresentado).
3. Ler [[entries-manuscrito-glossario]] (conteúdo Tipo 1).
4. Ler [[entries-docs-descobriveis]] (conteúdo Tipo 2, espelha [[in-world-docs]]).
5. Ler [[entries-fichas-bestiary]] (conteúdo Tipo 3).
6. Ler [[entries-mapas-timeline]] (conteúdo Tipo 4).
7. Ler [[knowledge-gates]] (regra de progressão).
8. Ler [[foreshadow-links]] por último (depende de todo o resto).

---

**Última revisão:** 2026-05-16. Canônico Bloco H. Atualizações exigem aprovação do criador supremo.
