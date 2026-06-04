# ADR-003: DialogueManager (plugin Godot 4, MIT) como biblioteca de diálogo

| | |
|---|---|
| **Status** | Accepted (decisão criador supremo 2026-05-30, TODO F2-E.6-DECISION; ratificada como DA-2 do blueprint do NPC introdutório 2026-06-03) |
| **Date** | 2026-05-30 |
| **Decisor** | petrinhu (criador supremo) |
| **Reversibilidade** | Two-way door barata AGORA (zero diálogo escrito) / one-way cara DEPOIS (cada fala migrada custa). Janela ótima: nada de prose final ainda existe, só blueprint de stubs. |
| **Substitui** | architecture.md §4.4 + engine-modules.md §3.5 ("ADR-003 futuro pra decisão de lib") |

## Contexto

O vertical slice (VS) precisa de **1 NPC com diálogo ramificado**: Seu Bertoldo Caím, a "âncora de chegada" da cidade. O blueprint canônico está em `docs/design/narrativa/dialogue-tree-npc-intro.md` (ratificado Sprint 4 W2, F2-N.1): grafo de nós com first-visit vs revisit, 3 ramos de escolha que reconvergem (`n2a/b/c` → `n3_reconverge`), gates de leitura por flag (`npc_intro.met`, `lore_node_1.deciphered`, `combat_sentinela.cleared`, `puzzle_patrol.cleared`), set/check de flags persistidas em SaveDataV1, e um hub de revisita despachado por estado (`n7_revisit_hub`).

O módulo de diálogo já estava prometido na arquitetura como pendente de decisão de lib:

- **architecture.md §4.4** (Dialogue, Back layer): "Decisão lib (Ink vs DialogueManager vs custom): ADR-003 futuro em F2-E.6 task."
- **engine-modules.md §3.5** (`engine/back/dialogue/`): "ADR-003 futuro pra decisão de lib (custom vs Ink C# port vs DialogueManager port)."

A pasta `engine/foundation/dialogue/` existe mas está **vazia**. Como **nenhum diálogo foi escrito** ainda (o blueprint é STUB de intent, prose final delegada ao `narrative-writer`), este é o momento mais barato de fixar a lib: nada precisa ser migrado.

### Forças em jogo

1. **Branching nativo cobre o blueprint.** O grafo do NPC usa exatamente os primitivos que a lib precisa fornecer: condições por flag, jumps entre nós, reconvergência de ramos, variáveis persistidas. Reinventar isso em código custom é trabalho de plataforma, não de jogo.
2. **Integração com C# obrigatória.** Stack canon é C# .NET 8 AOT (ADR-002). A lib precisa ser chamável de C# e expor os signals que a arquitetura já declara (`UIBus.DialogueShown`, `UIBus.DialogueChoiceMade`).
3. **Licença compatível com uso comercial.** GusWorld é produto pago (G1). Qualquer dependência precisa de licença permissiva e auditável (cross-ref F2-LEG.3).
4. **Contrato de flags é do SaveSystem, não da lib.** O blueprint §4 é explícito: o diálogo só ESCREVE `npc_intro.met` e `npc_intro.choice`; o resto ele LÊ via PlayerBus. A lib só precisa ler/escrever variáveis; a persistência canônica é do SaveSystem (F2-B).

## Decisão

**Adotar DialogueManager** (plugin Godot 4, licença MIT) como biblioteca de diálogo do projeto.

Justificativa direta:

- **Branching / condition / jump nativos** cobrem o blueprint do NPC (`dialogue-tree-npc-intro.md`) sem código de plataforma custom. Gates por flag, 3 ramos com reconvergência e hub de revisita são expressáveis na sintaxe da lib.
- **Integração C#.** O plugin é chamável a partir de C# .NET 8, alinhado com ADR-002 e com os signals já declarados em architecture.md §4.4 / engine-modules.md §3.5.
- **MIT = compatível comercial.** Licença permissiva, sem copyleft, audita limpa em SCA.

Esta decisão fecha a pendência "ADR-003 futuro" das duas docs de arquitetura. A implementação do módulo (`engine/back/dialogue/`) permanece pendente em **F2-E.6** (W3).

## Alternativas consideradas

### A. Custom C# data-driven (.tres Resource)

**Pros:**
- Controle total do formato; zero dependência externa.
- Casa com o padrão "DataClass POCO + Godot Resource wrapper" (ADR-002 item 19).

**Cons:**
- Reimplementa branching, condition e jump do zero (trabalho de plataforma, não de jogo).
- Editor visual de diálogo precisaria ser construído ou abandonado.
- Solo: tempo gasto em infra de diálogo é tempo fora do conteúdo do VS.

**Decidida:** REJEITADA. Custo de construir a plataforma de branching não se paga para 1 NPC do VS.

### B. Port de Ink (ink + ink-godot / inkle)

**Pros:**
- Linguagem de narrativa madura, branching expressivo, licença MIT.
- Ecossistema de ferramentas (Inky editor).

**Cons:**
- Camada de port/binding para Godot 4 C# adiciona superfície de integração.
- Curva da linguagem Ink para um blueprint simples (1 NPC, 3 ramos reconvergentes) é overkill.

**Decidida:** REJEITADA. Poder narrativo de Ink excede a necessidade do VS; integração extra não compensa.

### C. DialogueManager (plugin Godot 4, MIT) (escolhida)

**Pros:**
- Branching / condition / jump nativos cobrem o blueprint do NPC diretamente.
- Plugin Godot 4 nativo, integração C#.
- MIT (compatível com uso comercial).
- Editor de diálogo embutido reduz fricção de autoria.

**Cons:**
- Dependência de plugin Godot externo (superfície de manutenção, acompanhar updates do plugin).
- Sintaxe própria da lib a aprender.

**Decidida:** ACEITA.

## Consequências

### Positivas

- Branching do blueprint expressável nativamente; sem código de plataforma custom para 1 NPC.
- Integração C# alinhada a ADR-002 e aos signals de `UIBus` já declarados.
- MIT mantém a árvore de licenças limpa e comercial.
- Janela barata aproveitada: nenhum diálogo escrito precisa migrar.

### Negativas (custos aceitos)

- **Dependência externa.** Plugin Godot de terceiro entra na árvore; updates da engine/plugin podem exigir manutenção.
- **Sintaxe específica.** A lib tem formato próprio de roteiro; autoria via `narrative-writer` precisa respeitá-lo.
- **Acoplamento ao Godot.** O módulo de diálogo deixa de ser POCO puro engine-agnostic (diferente do `turn_combat`); aceitável porque diálogo é inerentemente ligado à UI/cena.

### Nota de licença (F2-LEG.3)

- **DialogueManager = MIT.** Compatível com distribuição comercial de G1.
- **Antes de mergear o módulo:** rodar SCA (licensee / scancode) sobre a árvore para confirmar que nenhuma dependência transitiva introduz copyleft. Disciplina de F2-LEG.3.

## Ações imediatas

1. ✅ Este ADR criado e canonizado (fecha "ADR-003 futuro" em architecture.md §4.4 + engine-modules.md §3.5).
2. ⏳ Implementar `engine/back/dialogue/` com DialogueManager + branching base + integração SaveSystem/UIBus (F2-E.6, W3).
3. ⏳ Rodar SCA de licença antes de mergear o módulo (F2-LEG.3).
4. ⏳ `narrative-writer` produz a prose final do blueprint `dialogue-tree-npc-intro.md` no formato da lib (handoff F2-N.1).

## Cross-refs

- `docs/design/narrativa/dialogue-tree-npc-intro.md` (blueprint do NPC; DA-2 ratifica DialogueManager).
- `docs/tech/architecture.md` §4.4 (Dialogue, Back layer).
- `docs/tech/engine-modules.md` §3.5 (`engine/back/dialogue/`).
- `ADR-002-csharp-aot-over-gdscript.md` (stack C# que a lib integra).
- `TODO.md` F2-E.6-DECISION (decisão de lib), F2-E.6 (impl), F2-LEG.3 (nota de licença + SCA).

---

**Fim do ADR-003.**
