# Modelo de entrega da lore (canônico)

**Decisão do líder, 2026-07-19 (via AskUserQuestion).** Como a lore chega ao jogador. Fonte única de verdade do MODELO; não duplica o conteúdo da lore (esse vive em `docs/narrative/`).

## Modelo EM CAMADAS

| Camada | Conteúdo | Canal | Obrigatório? |
|---|---|---|---|
| **Essencial** | O mínimo pra entender a história principal (quem é o Gus, o conflito com a Sterling, o objetivo) — ninguém se perde | Caminho principal (cenas, diálogos, beats do `arco-principal.md`) | **SIM** — sempre visto |
| **Profunda** | Facções, eras, tradições, o acidente multiversal, os mestres do Codex, história do reino | **Opcional**: documentos descobríveis (`in-world-docs.md`, 15 docs + 3 gate Ouro) + **sidequests** + **RunaDex/codex** (coleção estilo Pokédex) | NÃO — recompensa explorar |
| **Completa** | Os segredos mais fundos, a cosmologia inteira, o "porquê" final | Destravada só com **100% de conclusão** | NÃO — prêmio do completionista |

Princípio: premiar quem explora **sem punir** quem só quer a história principal (alvo 11+; nunca deixar a criança perdida na trama base).

## Prêmio do 100% = FINAL VERDADEIRO / cena extra jogável

O 100% de conclusão destrava um **final verdadeiro ou uma sequência jogável extra** (não só um compêndio de leitura). É o maior incentivo de completude. Encosta no arco **Dragon Victory** (ver [`project_dragon_victory_canon`] / `docs/narrative/`).

### ⚠️ Costura pendente (narrative-designer, NÃO decidida aqui)

O `arco-principal.md` já tem **3 endings**. Falta decidir COMO o "final verdadeiro do 100%" se integra:
- É um **4º ending** (o "verdadeiro", acima dos 3)?
- Ou é um **gate**: o melhor dos 3 endings (provavelmente o Dragon Victory) só fica disponível com 100%?
- Ou uma **sequência-epílogo jogável** APÓS o ending, independente de qual dos 3?

Isso é decisão de narrativa (impacto no arco + no Dragon Victory) → **brainstorm com `narrative-designer` + decisão do líder via AskUserQuestion** antes de qualquer implementação. Registrado como pendência; não inventar a integração sozinho.

## Cross-ref
- `docs/narrative/in-world-docs.md` (os discoverables), `docs/narrative/arco-principal.md` (endings), `docs/design/brainstorm-backlog.md` #1 (origem multiversal = miolo da lore profunda), RunaDex (em `docs/design/mecanicas/cartas-hardware-pirataria-energia.md` §13).
- Memória `project_dragon_victory_canon` (o arco do final).
- O que conta como "100%": RunaDex completa + sidequests + discoverables — definir a métrica exata quando a produção de conteúdo abrir.
