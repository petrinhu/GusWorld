# ADR-020: Peças de dado componíveis em módulos estreitos (module-level data pieces) + regra do mundo data-driven

**Status:** Accepted (decisão do líder supremo, 2026-07-19)
**Data:** 2026-07-19
**Decisores:** líder supremo (petrus) — preferência declarada ("menor trabalho refatorando e atomizando AGORA que quebrando monolitos futuros"); Caetano/CTO + software-architect (levantamento e redação)

Cross-ref: [ADR-019](ADR-019-arquitetura-conteudo-atomica-data-driven.md) (arquitetura de conteúdo atômica / data-driven — este ADR generaliza o mesmo racional do nível de CONTEÚDO pro nível de MÓDULO), [ADR-016](ADR-016-techmagic-effect-engine-data-driven.md) (motor `techMagic`, recusa de VM genérica), item `AC-E11` do `TODO.md` (decomposição de `battle_preview.cpp`/`battle_scene.cpp`), item `CARDS-HW-1` (motor de cartas hardware/energia, o gatilho concreto deste ADR), `docs/design/mecanicas/cartas-hardware-pirataria-energia.md`, memórias `reference_techmagic_engine_impl`, `feedback_auditoria_domino`.

## Contexto

Idea-churn constante — o líder e o filho Gus Dragon vivem propondo mecânicas novas — faz cada ideia nova ser pendurada em struct/módulo existente até virar monolito. Foi esse padrão que motivou a onda `AC-E11` (quebra de `battle_preview.cpp` 2966→1831 linhas e `battle_scene.cpp` 2101→946 linhas), e o `CardPhysicalState` do item `CARDS-HW-1` já começava a repeti-lo: um único struct acumulando 4 assuntos (proveniência, bateria, integridade/vírus, classe de hardware). No mesmo pente, `combat_enums.hpp` tinha se tornado dono indevido do catálogo de cartas (fan-in de 19 consumidores só do vocabulário de carta), e `deck/` precisava incluir `combat/` só para pegar um enum de carta — dependência invertida.

O `ADR-019` já canoniza "motor genérico + conteúdo como dado atômico" no nível de CONTEÚDO (carta nova = linha de dado, nunca `if` novo no motor). Faltava a mesma disciplina no nível de MÓDULO: quando um ASSUNTO novo aparece (proveniência de hardware, degradação de bateria, infecção por vírus), ele não pode entrar acretando campos num struct já existente — precisa nascer em módulo próprio, testável isoladamente.

## Decisão

1. **Generalizar o [[ADR-019]] do nível de CONTEÚDO pro nível de MÓDULO.** Assim como carta nova é linha de dado, assunto novo é **peça de dado + módulo estreito** — um campo aditivo, default-neutro, no objeto composto — **nunca um campo acretado num struct existente.**

2. **Padrão técnico de decomposição sem migrator: herança de agregados + fachada de re-export por `using`.** Exemplo aplicado: `CardPhysicalState` deixa de ser struct monolítico e vira um agregado C++20 que herda publicamente de `gus::domain::hardware::CardProvenance`, `gus::domain::hardware::BatteryState` e `gus::domain::infection::IntegrityState` (`is_burned_out` fica direto no agregado, por não ter invariante correlato com outra peça). O acesso flat (`p.origin`, `p.battery_recharge_cycles`, ...) e o serializer são preservados por construção da herança — save V7 ficou byte-idêntico: md5sum do bloco serializado (`CardPhysicalState` de 2 `CardInstance`, ativa+morta, campos não-default, roundtrip real via `serialize_save`/`deserialize_save`) igual antes/depois — `a6a3e6f01c30123e1565c5269f17a61a` — e as 4 suítes de save (`card_hardware_test.cpp`, `save_v7_test.cpp`, `save_serializer_fuzz_test.cpp`, `save_migrators_test.cpp`) com zero diff. `card_hardware.hpp`/`card_hardware_constants.hpp` viram fachadas: incluem as peças e re-exportam em `gus::domain::deck` via `using`-declaration, preservando a identidade de tipo — os consumidores existentes compilam intocados, sem editar nenhum.

3. **Mapa de módulos de domínio (estado pós-`ATOM-1`/`ATOM-2`):**
   - `gus::domain::cards` (`domain/cards/`) — catálogo: `Card`, `CardTier`, `CardFamily`, `CardBaseType`, `TargetShape`, `EffectKind`, `MasterCards`/`PlaceholderCards`.
   - `gus::domain::hardware` (`domain/hardware/`) — proveniência + energia: `CardProvenance` (origin), `BatteryState` (cycles, deficit), `HardwareClass`/`hardware_class_of()` (referencia `cards::CardTier`).
   - `gus::domain::infection` (`domain/infection/`) — integridade/vírus: `IntegrityState` (infected/diagnosed/virus_kind).
   - `combat/` e `deck/` **consomem** `cards/` — dependência invertida em relação ao estado anterior (`deck/` incluía `combat/` só pelo enum de carta). `hardware/` e `infection/` não incluem `combat/` nem `deck/`, só `cards/` (para `CardTier`).

   Rejeitados por over-engineering para o porte do projeto: **ECS runtime** (party de 7 personagens + combate por turnos não justifica a indireção de um Entity-Component-System em tempo de execução) e **interfaces/capabilities virtuais** (POCO por invariante já resolve sem custo de vtable/herança dinâmica).

4. **Convenção reservada para módulos futuros.** `items/`, `inventory/`, poções como categoria de item (não motor próprio) nascem separados **quando houver consumidor real** — não criar shells vazios antecipadamente (isso seria over-engineering na direção oposta).

5. **Regra do mundo (mesma filosofia, aplicada a lugares).** Nenhum lugar novo (dungeon/interior/cidade) entra no `maestro` como método par-a-par novo. Lugar novo é **dado** — um `AreaDescriptor` (tipo, política de save, tema, encontros, grafo de portais). O refactor do `maestro` para consumir esse dado é **pago junto com a primeira área nova real** (evita projetar um contrato sem consumidor concreto); até lá vale só a regra, sem implementação antecipada.

## Por quê

- **Menor trabalho refatorando agora do que quebrando monolito depois.** Preferência explícita do líder: decompor no momento em que o segundo assunto aparece custa uma tarde; decompor depois de N assuntos acretados custa uma onda inteira (foi o preço pago pela `AC-E11`).
- **Peça de dado testa isolada.** `card_provenance_test.cpp`, `battery_state_test.cpp`, `integrity_state_test.cpp`, `hardware_class_test.cpp` — cada peça ganhou teste dedicado; nenhum teste precisa conhecer as peças-irmãs.
- **Save migra de graça.** Herança de agregados preserva o layout serializado sem escrever migrator novo — o custo da decomposição é absorvido pela estrutura de tipos, não pelo formato persistido.
- **Anti-monolito por construção.** Assunto novo = módulo novo com dependência de baixo para cima (`hardware`/`infection` → `cards`, nunca o contrário) — impossível reintroduzir o fan-in de 19 que motivou este ADR sem violar a direção da dependência.

## Consequências

**Positivas:**
- Ideia nova do líder/Gus Dragon não reescreve o módulo velho — nasce em módulo próprio desde o primeiro campo.
- Testável por peça (isolamento de responsabilidade também no nível de teste, não só de tipo).
- Save migra de graça via herança de agregados — zero migrator para reorganização puramente estrutural.
- Regra do mundo evita comprometer o `maestro` com um contrato de área antes de haver consumidor real.

**Negativas / aceitas como custo:**
- **Peça não conhece peça-irmã.** `CardProvenance` não sabe de `BatteryState` nem de `IntegrityState` — qualquer invariante que precise cruzar duas peças (como a validação de `CardPhysicalState::validate()`, que hoje delega para `CardProvenance` e `IntegrityState` separadamente) tem que morar no agregado, não em nenhuma peça individual.
- **Fachada de re-export a manter.** `combat_enums.hpp`/`combat_records.hpp` e `card_hardware.hpp`/`card_hardware_constants.hpp` viraram fachadas via `using`-declaration; todo novo consumo do vocabulário antigo tem que decidir se inclui a fachada (compat) ou o módulo novo diretamente — disciplina a não deixar acumular indefinidamente.
- **Disciplina de decisão "módulo novo vs. campo aditivo"** exige julgamento a cada assunto — o critério (peça de dado + módulo estreito) não é 100% mecânico, é aplicado caso a caso na revisão.

## Onde já vale (evidência no código, antes deste ADR ser escrito)

- **`ATOM-2`** (commit `6c1090e`): extrai catálogo de cartas de `combat/` para `domain/cards/` — `combat_enums.hpp`/`combat_records.hpp` viram fachadas; ~90 consumidores existentes compilam intocados; 2220/2220 testes verdes.
- **`ATOM-1`** (commit `36dd89b`): quebra `CardPhysicalState` em `hardware::CardProvenance` + `hardware::BatteryState` + `infection::IntegrityState`; save V7 com md5 idêntico; 2244/2244 testes verdes (2220 + 24 novos das peças).

## Onde aplicar a seguir (pendente)

- **`ATOM-3` (items):** `items/`/`inventory/` nascem separados quando houver consumidor real — ver §Decisão item 4.
- **`ATOM-4` (world):** refactor do `maestro` para consumir `AreaDescriptor` como dado, pago junto com a primeira área nova real — ver §Decisão item 5.

## Reversibilidade

Two-way door quanto ao MECANISMO de decomposição (herança de agregados + fachada é um padrão aplicável a qualquer módulo futuro, sem custo de reverter se um assunto se mostrar mal cortado). One-way door quanto ao formato já persistido: a decomposição de `CardPhysicalState` já está em save V7 de produção — reverter exigiria novo migrator, mesmo custo que qualquer mudança de shape de save já documentada em `ADR-006`/`ADR-007`/`ADR-015`.
