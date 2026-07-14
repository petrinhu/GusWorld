# ADR-014: Runtime de diálogo POCO C++20 (formato de autoria, registro, escopo, persistência)

**Status:** Accepted (4 decisões fechadas pelo criador supremo em 2026-07-06, todas na opção recomendada pelo `software-architect`)
**Supersedes:** [ADR-003](ADR-003-dialogue-library.md) (o plugin DialogueManager/Godot que este runtime substituiu no pivot; marcador de reciprocidade adicionado 2026-07-14, achado PS-Y5)
**Data:** 2026-07-06
**Decisores:** criador supremo (petrus) + `software-architect` (modo colaborativo, regra canônica do projeto: nenhum agente decide arquitetura sozinho).
**Cross-ref:** [ADR-012](ADR-012-m7-paridade-jogavel-plano.md) (plano do M7; este ADR **refina** a decisão 6 daquele — ver §"Reconciliação com ADR-012"), [ADR-006](ADR-006-crypto-hmac-formato-domain.md) (crypto própria SHA-256/HMAC, zero-dep; usada pelo pipeline de conteúdo cifrado), [ADR-007](ADR-007-controls-json-hash128-save-v4.md) (save V4 atual + parser JSON próprio dep-free), `docs/design/narrativa/dialogue-tree-npc-intro.md` (blueprint canônico do Bertoldo, `F2-N.1 ✅` — a referência de conteúdo/estrutura que este runtime precisa conseguir renderizar), `docs/design/mecanicas/combat-flavor.md` §5 (par terminal × caixa-quente + prompts por personagem — o item vizinho `DIALOGO-TERMINAL`), memória `project_i18n_canonico` (i18n-ready desde o dia 1), memória `reference_formato_mapa_gmap` (padrão fonte→compilador→artefato selado).

## Contexto

O item `M7-DIALOGO` (PI9) precisa entregar um sistema de diálogo/NPC rodando 100% na engine C++ nova (sem Godot). O plugin **DialogueManager** (addon Godot 4, MIT) que serviria a esse papel **morreu no pivot para C++20 + SDL3** (ADR-008); o blueprint de diálogo permanece válido (é dado de design agnóstico de engine, `DA-2` do blueprint), mas o **runtime precisa ser re-derivado do zero em C++20, em TDD**.

O ADR-012 (plano macro do M7) já fixou que o motor é pequeno, data-driven, com etiqueta de registro por fala, identidade de quem fala e i18n obrigatório. Faltava a **arquitetura fina**: que tipos POCO existem, como se encaixam nas 4 camadas, qual o formato de autoria, onde o registro (terminal × quente) vive nos dados, e como as flags persistem no save já existente. O `software-architect` levantou 4 pontos de decisão e apresentou opções; o criador escolheu.

A SAÍDA-ALVO do M7-DIALOGO é o mínimo que prova a costura: falar com 1 NPC (Seu Bertoldo Caím, `DA-1` do blueprint) no mapa real, uma escolha muda uma flag, e a flag sobrevive a um round-trip de save/load.

## Decisão

### 1. Formato de autoria: formato-texto próprio leve (opção A)

O grafo de diálogo é escrito num **formato de texto próprio, leve**, no mesmo espírito do CSV de mapa (`map_csv.cpp`, ~206 linhas) e do `## CHAVE` do catálogo i18n (`pt_br.md`): diretivas com prefixo (`@node`, `#meta`) + pares `chave: valor`. É a **fonte editável** no repositório, onde o `narrative-designer` trabalha; um parser POCO puro (`domain/dialogue/`, alvo ~150-250 linhas) o transforma no `DialogueGraph` validado.

**Rejeitadas:** JSON (o projeto é zero-dep por ADR-006; não há lib JSON vendorizada, e o parser JSON próprio de `controls.json` tem 735 linhas — 3-4× mais caro que o formato-texto para o mesmo resultado); pôr o **grafo** no pipeline cifra+sela do `.gmap`/catálogo (o ADR-012 item 8 sela o catálogo de **texto** i18n, não a **estrutura** do grafo — o player editar `npc_intro.choice_curioso` não desbloqueia poder, só cosmética/telemetria; selar o grafo agora é escopo que ninguém pediu). O **texto das falas** continua indo pelo catálogo i18n cifrado (decisão 8 do ADR-012, inalterada); a **estrutura** do grafo é fonte-texto simples versionada.

### 2. Registro terminal × quente: default no grafo + override opcional por nó (opção C)

O par terminal (software/máquina fala) × caixa-quente com retrato (o coração fala) de `combat-flavor.md §5` é modelado por um **`DialogueRegister default_register` no `DialogueGraph`** + um **`std::optional<DialogueRegister> register_override` por nó**. Um NPC homogêneo (Bertoldo é 100% caixa-quente) declara o default uma vez e não repete a etiqueta em cada linha; o override existe para o caso já canônico de uma fala isolada trocar de registro no meio de uma conversa (ex.: o prompt do Dante que muda de `asmodico`→`c-arcane` no late game; um NPC-máquina que injeta 1 linha de terminal numa cena quente).

O runtime **repassa o registro efetivo** (`register_override.value_or(graph.default_register)`) para a apresentação, sem nunca saber qual dos dois está sendo pintado — a spec visual fina dos dois registros é o item paralelo `DIALOGO-TERMINAL`, que consome esta etiqueta. Isso mantém o motor desacoplado da apresentação (item c do escopo do M7-DIALOGO).

**Rejeitadas:** etiqueta obrigatória em cada nó (boilerplate para 100% dos nós de um NPC homogêneo); registro só a nível de conversa inteira sem override (não serve o caso do Dante nem o NPC-máquina com 1 linha de terminal no meio de cena quente).

### 3. Escopo: capacidade genérica construída, conteúdo mínimo exercido (opção A)

O runtime **constrói a capacidade genérica** — nó de escolha (`DialogueOption`), convergência de ramos, `FlagCondition` (equalidade sobre flag) e `FlagEffect` — mas o **conteúdo do M7 exercita só o mínimo do Bertoldo**: saudação (`n0_greet`) → gancho (`n1_hook`) → 3 escolhas (`n2a/b/c`) → reconvergência (`n3_reconverge`) → saída. **NÃO** entram nesta onda o `n7_revisit_hub` nem os gates de `combat_sentinela.cleared`/`puzzle_patrol.cleared` (§7 do blueprint): esses flags nascem de combate/puzzle ainda não costurados no M7, e a SAÍDA-ALVO só pede 1 NPC / 1 escolha / 1 flag.

A **reconvergência é trivial** no modelo de dados: 2+ `DialogueOption` apontando para o mesmo `next_node_id` — não há conceito especial de "merge", nenhuma infra dedicada. `FlagCondition` fica pronta na estrutura mas é exercitada só por teste de unidade nesta onda, não pelo NPC-MVP (que só escreve flags, não ramifica por leitura — o blueprint §4 confirma: o first-visit sempre oferece as 3 escolhas, sem gate punitivo).

**Rejeitadas:** motor estritamente linear sem opções (contradiria o blueprint `F2-N.1 ✅` já aprovado — precisaria rebaixar o design do Bertoldo, o que não foi pedido); implementar também o `n7_revisit_hub` completo agora (infra especulativa; os flags de gate não existem costurados no M7).

### 4. Persistência: reusar `SaveData::flags`, sem bump de schema (opção A)

As flags de diálogo reusam o **`SaveData::flags` já existente** (`std::map<std::string, bool>`, save **V4 atual**, ADR-007) — **sem bump de schema, sem migrator novo**. O `npc_intro.choice` (enum {curioso, pragmatico, seco} previsto no `DA-4` do blueprint) é modelado como **bools separados** (`npc_intro.choice_curioso`, `npc_intro.choice_pragmatico`, `npc_intro.choice_seco`), mapeando para o mesmo dado que um enum representaria. Flags escritas pelo Bertoldo nesta onda:

| Flag | Escrita em | Semântica |
|---|---|---|
| `npc_intro.met` | `on_enter` de `n0_greet` | first-visit já ocorreu (gate reservado para revisita futura) |
| `npc_intro.choice_curioso` / `_pragmatico` / `_seco` | `FlagEffect` da opção escolhida em `n2a/b/c` | tom da resposta do Gus (telemetria + callback narrativo pós-VS reservado, `DA-4`) |

O critério de saída ("escolha muda flag, flag sobrevive ao save/load round-trip") é satisfeito literalmente pelo `SaveData::flags` sem tocar o formato binário selado nem a chain de migradores.

**Rejeitada:** bump para V5 com um `dialogue_flags` tipado (enum incluso) — fiel à letra do `DA-4`, mas custa um migrator V4→V5 sem necessidade funcional imediata; os bools no `flags` já resolvem o requisito literal.

## Estrutura de dados (POCO, `domain/dialogue/`)

Namespace `gus::domain::dialogue`. Camada `domain/` PURA (ZERO Qt, ZERO SDL, ZERO I/O — mesma invariante de `save/`, `map/`, `combat/`). Igualdade por valor e `validate()` fail-fast na mesma disciplina de `TileMap::validate()`/`SaveData::validate()`.

```cpp
namespace gus::domain::dialogue {

enum class DialogueRegister { Terminal, Warm };  // combat-flavor.md §5

// Equalidade sobre uma flag (bool). Sem expressoes/operadores: so ==, suficiente
// para o gate first-visit/revisit. Pronta na estrutura; exercitada so por teste de
// unidade no M7 (o NPC-MVP nao ramifica por leitura).
struct FlagCondition {
    std::string flag_key;
    bool expected_value = true;
};

// Efeito colateral de um no/opcao sobre o mapa de flags.
struct FlagEffect {
    std::string flag_key;
    bool value = true;
};

// Opcao de escolha do jogador. label_key = chave i18n do texto do botao.
struct DialogueOption {
    std::string label_key;
    std::string next_node_id;
    std::optional<FlagEffect> effect;   // ex: npc_intro.choice_curioso = true
};

// No de dialogo. Linear (options vazio -> segue next_node_id) OU no de escolha
// (options.size() >= 2). Convergencia = 2+ options mirando o MESMO next_node_id.
struct DialogueNode {
    std::string id;
    std::string speaker_id;                              // "bertoldo" -> retrato/prompt
    std::string text_key;                                // chave i18n da fala
    std::optional<DialogueRegister> register_override;   // vazio = herda default do grafo
    std::optional<FlagEffect> on_enter;                  // ex: npc_intro.met ao entrar
    std::string next_node_id;                            // usado quando options vazio
    std::vector<DialogueOption> options;                 // >=2 = no de escolha
};

// Grafo completo de um dialogo (1 NPC/1 conversa). O parser do formato-texto
// materializa isto; validate() e fail-fast.
struct DialogueGraph {
    std::string dialogue_id;
    DialogueRegister default_register = DialogueRegister::Warm;
    std::string entry_node_id;
    std::map<std::string, DialogueNode> nodes;   // std::map = ordem determinista

    void validate() const;  // entry existe; todo next_node_id/option aponta no existente
                            // ou "@exit"; no de options tem >=2; sem no orfao.
};

// Runtime PURO. Opera sobre uma referencia externa ao mapa de flags (o SaveData::flags
// atual, ou um std::map de teste). NAO depende de domain/save (evita acoplamento de
// subdominio) nem de UI. O app le current() e desenha caixa-quente OU terminal sem que
// o motor saiba qual.
class DialogueRuntime {
public:
    DialogueRuntime(const DialogueGraph& graph, std::map<std::string, bool>& flags);
    void enter();                              // posiciona no entry, aplica on_enter
    [[nodiscard]] const DialogueNode& current() const;
    [[nodiscard]] DialogueRegister current_register() const;  // override.value_or(default)
    void choose(std::size_t option_index);     // aplica FlagEffect + segue next_node_id
    void advance();                            // no linear: segue next_node_id
    [[nodiscard]] bool finished() const;       // proximo == "@exit"
};

}  // namespace gus::domain::dialogue
```

### Encaixe nas 4 camadas

| Camada | Responsabilidade no diálogo |
|---|---|
| `domain/dialogue/` | `DialogueGraph`/`DialogueNode`/`DialogueOption`/`FlagCondition`/`FlagEffect`/`DialogueRuntime` + o parser POCO do formato-texto (string em memória → grafo validado). PURO, testável, TDD. |
| `platform/` | I/O de arquivo: ler a fonte-texto do diálogo do disco (fronteira, como o `city_loader` lê o `.gmap` e o `Translator` lê o `.md`). Não há lógica de diálogo aqui. |
| `app/` | Apresentação: consome `DialogueRuntime::current()` + `current_register()` para pintar caixa-quente (retrato) ou terminal (prompt de linguagem); trigger de interação do NPC no grid (proximidade, reusa `core/spatial/grid_collision.hpp`); resolve `text_key`/`label_key` via `Translator::tr()` (i18n). O NPC do MVP (Bertoldo) tem posição fixa em código de jogo — mesmo padrão do "1 inimigo fixo" já aprovado no M7-COSTURA Inc.1 —, não entra no `.gmap`. |

O runtime recebe `std::map<std::string,bool>&` (não `SaveData&`) de propósito: `domain/dialogue` não depende de `domain/save`; o `app`/maestro passa `save_data.flags` por referência na integração real, e os testes passam um `std::map` local.

## Reconciliação com ADR-012

O ADR-012 decisão 6 dizia: *"Sem condicionais nem reconvergência de árvore nesta onda: não são exercidos com 1 NPC / 1 escolha, e adicioná-los agora seria infraestrutura especulativa."*

Este ADR **refina** essa decisão à luz do blueprint canônico do Bertoldo (`F2-N.1 ✅`, anterior ao ADR-012), que **exige 3 escolhas convergindo num nó comum** (`n3_reconverge`). A frase do ADR-012 era boa intenção anti-over-engineering, mas, tomada ao pé da letra, impediria renderizar o conteúdo de design já aprovado. Resolução (decisão 3 acima, escolhida pelo criador):

- A **capacidade genérica** de nó de escolha + convergência **existe** no modelo de dados — porque a reconvergência do blueprint a exige e ela é trivial (opções apontando ao mesmo `next_node_id`, zero infra dedicada).
- O **conteúdo do M7** usa só o mínimo do Bertoldo (saudação→gancho→3 escolhas→reconvergência→saída); o `n7_revisit_hub` e os gates de combate/puzzle (`FlagCondition` sobre estado de progresso) **ficam de fora desta onda** — aqui o espírito anti-especulação do ADR-012 se mantém.
- `FlagCondition` fica na estrutura mas é exercitada só por teste de unidade nesta onda (não pelo NPC-MVP). É a linha exata entre "capacidade pronta, sem porta fechada para mais NPCs" e "sem construir a árvore de gates de revisita que ninguém exercita ainda".

Nenhuma outra decisão do ADR-012 é alterada (compilador de conteúdo cifra+sela do catálogo i18n, i18n obrigatório, maestro leve, fork do save — todos inalterados). Este ADR só detalha a decisão 6.

## Consequências

**Positivas:**
- O runtime nasce POCO/puro em `domain/`, testável em TDD sem SDL/UI, seguindo a disciplina já provada de `save/`/`map/`/`combat/`.
- O formato-texto próprio custa ~150-250 linhas de parser (perto do `map_csv.cpp`), sem dependência nova, com git-diff limpo e familiar ao `narrative-designer`.
- O registro default+override renderiza o Bertoldo (homogêneo) sem boilerplate e já cobre o caso Dante/NPC-máquina sem reabrir o formato depois.
- A capacidade genérica de escolha/convergência satisfaz o blueprint aprovado sem rebaixá-lo; a `FlagCondition` deixa a porta aberta para NPCs com gate de revisita sem construir a árvore agora.
- Zero trabalho de save: reusar `SaveData::flags` (V4) satisfaz o critério de saída literalmente, sem migrator novo.
- `domain/dialogue` desacoplado de `domain/save` (recebe `map<string,bool>&`) — subdomínios independentes.

**Negativas / aceitas como custo:**
- Mais um mini-formato bespoke a documentar (o formato-texto de diálogo) — mitigado por ser simples e no mesmo espírito de formatos que o time já mantém.
- `npc_intro.choice` como 3 bools em vez de 1 enum: menos "elegante" que o `DA-4` idealizou, mas mapeia o mesmo dado; se um dia a reatividade pós-VS exigir o enum tipado, migra-se então (custo adiado, não pago à toa).
- A estrutura do grafo **não** é selada/cifrada nesta onda (só o texto i18n é, via ADR-012 item 8) — aceito: editar a estrutura do grafo não concede poder ao player, só mexe em cosmética/telemetria; selar a estrutura fica reversível para uma onda futura se surgir motivo.

**Riscos / pontos de atenção:**
- `M7-DIALOGO-RUNTIME` depende de `M2-SAVE-IO` (I/O real em disco) para provar o round-trip da flag — dependência dura herdada do ADR-012, não reaberta aqui.
- A spec visual dos dois registros (`DIALOGO-TERMINAL`) roda em paralelo; o runtime só precisa **expor** a etiqueta de registro — se a spec visual mudar, o motor não é tocado (o desacoplamento acima protege isso).

## Reversibilidade

**Two-way door.** O formato-texto pode ganhar versão nova sem quebrar grafos existentes (mesmo espírito forward-only do `.gmap`). O `DialogueRuntime` separa motor de renderizador, então trocar a pintura provisória pela definitiva (spec `DIALOGO-TERMINAL`) não toca o motor nem os dados. Passar de 3 bools para um enum de `choice`, ou de `SaveData::flags` para um `dialogue_flags` tipado, é um bump de save aditivo (V5) quando/se a reatividade pós-VS pedir — não é reescrita. Selar a estrutura do grafo no futuro é aditivo (mesmo pipeline do `.gmap`). Sem releases públicas (jogo em DEV): nenhuma decisão aqui é irreversível para o jogador final.

## Execução (prevista — NÃO iniciada neste ADR)

Este ADR é só arquitetura. As sub-peças de implementação seguem em dispatches próprias, em TDD (`backend-engineer`/`gameplay_engineer`), na ordem já mapeada pelo ADR-012 Onda 3:
- `M7-DIALOGO-FORMATO` — parser POCO do formato-texto → `DialogueGraph` (+ `validate()`).
- `M7-DIALOGO-RUNTIME` — `DialogueRuntime` (enter/current/choose/advance/finished) sobre `map<string,bool>&`. Bloqueado por `M2-SAVE-IO`.
- `M7-DIALOGO-NPC-MVP` — Bertoldo no mapa (trigger de proximidade), falas via chaves i18n, a escolha que seta a flag, flag sobrevive ao save/load.

O texto das falas (chaves i18n cadastradas em `pt_br.md`) é entregue pelo `narrative-writer` a partir dos stubs de intent do blueprint (`n0`..`n3`), no handoff — este ADR não escreve prose.
