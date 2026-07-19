# ADR-019: Arquitetura de conteúdo atômica / data-driven (canonização do padrão vigente)

**Status:** Accepted (decisão do líder supremo, 2026-07-18/19; canoniza um padrão JÁ emergente na base, não introduz decisão nova)
**Data:** 2026-07-19
**Decisores:** líder supremo (petrus) — pergunta disparadora ("nossa arquitetura de cartas/itens/dungeons/mapas é atômica?"); Caetano/CTO + software-architect (levantamento da evidência e redação)

Cross-ref: [ADR-016](ADR-016-techmagic-effect-engine-data-driven.md) (motor `techMagic` data-driven, já rejeitou VM/scripting genérico — este ADR generaliza o mesmo racional pra toda a arquitetura de conteúdo), [ADR-006](ADR-006-crypto-hmac-formato-domain.md)/[ADR-007](ADR-007-controls-json-hash128-save-v4.md)/[ADR-015](ADR-015-save-security-v2-offline.md) (save JSON versionado + migrators), [ADR-014](ADR-014-dialogue-runtime-poco.md) (runtime de diálogo POCO data-driven), `docs/tech/pivot/engine-design.md` (as 4 camadas core/domain/platform/app), `docs/design/mecanicas/cartas-technomagik.md`, item `AC-E11` do `TODO.md` (decomposição de `battle_preview.cpp`/`battle_scene.cpp` — a onda que aplica este princípio à camada de apresentação), memórias `reference_techmagic_engine_impl`, `feedback_todo_efeito_loga_terminal`, `feedback_auditoria_domino`.

## Contexto

Em 2026-07-18/19 o líder perguntou diretamente se a arquitetura de cartas/itens/dungeons/mapas do GusWorld é **atômica** — isto é, se conteúdo novo se adiciona como dado, sem ramificar código. A pergunta chegou junto da onda `AC-E11` (o achado de auditoria de que `battle_preview.cpp` e `battle_scene.cpp` crescem monotonicamente como "hosts monolito" a cada incremento de feature).

A resposta é sim, mas até este ADR o padrão vivia **implícito** — espalhado em decisões pontuais (`ADR-016` pro motor de cartas, `ADR-014` pro diálogo, o formato `.gmap` selado pros mapas, o catálogo `canonical_templates` pros templates) sem um nome comum nem uma disciplina documentada que amarrasse os riscos que o próprio padrão cria (enums de contrato que não podem reordenar, `switch` que precisa ser exaustivo, o ponto cego do `default:`). O item `HUD-STATUS-ICONS-STALE` (fechado em 2026-07-18) expôs exatamente esse risco: um array hardcoded fora de sincronia com o enum de conteúdo — a causa-raiz sistêmica que motivou instalar `-Werror=switch` como rede em todo o projeto.

Este ADR não decide nada novo em termos de mecanismo: **canoniza, nomeia e documenta a disciplina** de um padrão já em produção, para que toda extensão futura (quests, rotina de NPC, hardware/pirataria de cartas, a decomposição da `AC-E11`) seja avaliada contra ele deliberadamente, em vez de reinventado caso a caso.

## Decisão

**Princípio: motor genérico + conteúdo como dado atômico composável.** Cada sistema do jogo se divide em (a) um **executor/motor pequeno e fixo**, escrito uma vez em C++, testado uma vez, e (b) uma **coleção de dados** (structs/registros/arquivos) que o motor interpreta. Adicionar conteúdo novo — uma carta, um mapa, um item, uma entrada de save — é adicionar uma **linha de dado** (uma entrada em `master_cards.cpp`, um arquivo `.gmap`, uma entrada de catálogo, uma chave i18n). **Nunca é ramificar o motor com um `if`/`case` novo por instância de conteúdo.**

O anti-padrão rejeitado é o interpretador/VM genérico de propósito amplo — já recusado explicitamente pelo `ADR-016` (§Decisão: "a VM foi RECUSADA... o vocabulário de efeitos é FECHADO e conhecido... a VM não elimina nenhum trabalho real, só adiciona indireção"). Esse racional generaliza: o GusWorld não constrói uma linguagem de script genérica para NENHUM domínio de conteúdo enquanto não houver um consumidor real de "conteúdo sem recompilar" (modding, editor de usuário, UGC) — o vocabulário de cada domínio (efeitos de carta, nós de diálogo, tiles de mapa) é fechado, curado pelo líder, e conhecido em tempo de compilação.

## Por quê

- **Separação lógica/dado.** O executor decide COMO um efeito se resolve; o dado decide QUAL efeito e com quais parâmetros. Trocar um número (dano, duração, chance%) nunca exige recompilar uma regra nova.
- **DRY estrutural.** Um handler cobre N cartas/N nós/N entidades. O `techMagic::execute` de `ADR-016` tem hoje 12 `EffectKind`s cobrindo o catálogo inteiro de especiais — nenhuma carta nova precisou de um handler dedicado além do seu `EffectKind`.
- **Testabilidade concentrada.** Testar o handler uma vez (TDD + mutation testing adversarial, disciplina já em vigor desde o primeiro `EffectKind`) cobre TODO dado presente e futuro que o exercite. O teste não precisa ser reescrito quando o balanceamento de uma carta muda um número.
- **Extensibilidade sem tocar no executor.** Conteúdo novo dentro do vocabulário existente (mais uma carta com um `EffectKind` já implementado, mais um mapa `.gmap`, mais uma entrada de i18n) nunca abre um PR no motor — só no catálogo de dados.
- **Auditabilidade.** Dado é grep-ável, diff-ável, balanceável por alguém que não lê C++. O playtest N=3 mexe em números num catálogo, não em lógica; a auditoria-dominó (`feedback_auditoria_domino`) consegue varrer "todo dado que usa este handler" com uma busca textual.

## Onde já vale (evidência no código, antes deste ADR)

- **Cartas (`techMagic`, ADR-016):** `Card { tier, category, target_shape, std::vector<EffectSpec> }` + executor `techMagic::execute(card, contexto)` despachado por `TriggerHook`/`EffectKind`. Catálogo `MasterCards::build_registry()` — hoje 11/12 `EffectKind`s do manifesto `CARD-ENGINE-MANIFESTO` entregues, cada carta nova = uma entrada em `master_cards.cpp`, zero handler novo quando o `EffectKind` já existe (Maxwell reusou `TargetShape::Grupo` sem nenhum código novo).
- **Mapas `.gmap`:** formato binário selado (HMAC + UUID, anti-tamper) compilado a partir de CSV fonte (`reference_formato_mapa_gmap`) — o loop de exploração (M4) lê o mesmo parser pra qualquer mapa; mapa novo = arquivo novo, zero código novo.
- **Save JSON versionado + migrators (ADR-006/ADR-007/ADR-015):** o formato de save é dado versionado; cada shape nova de save vira um migrator, o motor de carregamento é único e fixo desde D1.
- **Roster/itens POCO:** entidades de domínio como structs de dados puros, sem lógica de instância embutida.
- **Diálogo runtime POCO (ADR-014):** grafo `DialogueGraph` em formato-texto próprio (nós, opções, `FlagCondition`/`FlagEffect`) interpretado por um parser+runtime fixo; NPC novo = arquivo de diálogo novo.
- **Catálogo de animação (`anim_catalog`):** poses/timing como dado consumido por um player de animação único.
- **i18n por chaves:** `pt_br.md`/`en_intl.md` como catálogo de dados; o `Translator` C++ é o único motor, texto novo = chave nova.

## Disciplinas obrigatórias (o preço do padrão)

O padrão não é grátis — ele move o risco de "lógica espalhada" para "sincronia entre enum e dado", e essa sincronia exige disciplina ativa:

1. **Enums de contrato são append-only.** Qualquer enum que participe de serialização (save) ou de um `switch` de conteúdo (`StatusId`, `EffectKind`, `CardFamily` etc.) só cresce no fim. Reordenar ou inserir no meio invalida save existente e desalinha índices — violação é one-way door de dado, não decisão trivial.
2. **`-Werror=switch` é a rede de exaustividade.** Instalada como fix sistêmico na onda `HUD-STATUS-ICONS-STALE` (2026-07-18, commit `620b304`) — antes disso o warning de `switch` não exaustivo só existia no `box2d` vendorizado, não nos alvos do projeto. Todo `switch` sobre um enum de conteúdo agora FALHA a compilação se um caso novo não for tratado.
3. **`switch` com `default:` sobre enum de conteúdo é ponto cego.** O `default:` engole silenciosamente o caso novo que o `-Werror=switch` teria pegado — a causa-raiz real de `HUD-STATUS-ICONS-STALE` não foi o `switch` em si, foi um `std::array` hardcoded (`kStatusIdCount`) fora de sincronia com o enum, sem guard de exaustividade nenhum. Todo `switch` novo sobre enum de conteúdo é item de checagem na auditoria-dominó (`feedback_auditoria_domino`): "esse guard existe num irmão e falta aqui?".
4. **TDD + verificação adversarial em todo handler/`EffectKind` novo.** Mantido desde o primeiro handler de `ADR-016` (Volta/leech): implementer escreve o handler com testes; `qa-engineer` (agente DIFERENTE do implementer) roda mutation testing sobre ele; o orquestrador re-verifica antes de aceitar. Essa disciplina é o que torna "um handler cobre N cartas" seguro — sem ela, o handler genérico seria um ponto único de falha silenciosa pra todo dado que o usa.

## Extensão à camada de apresentação

O princípio não é exclusivo de domínio/lógica — vale para UI. Telas devem ser **composição de módulos atômicos** (modelo de dados + layout RML + input + loop de render), cada módulo com seu próprio self-test, em vez de um host monolítico que acumula responsabilidades a cada feature nova. O cockpit de batalha já é dirigido por data-model (`battle_hud_model`) nesse espírito.

A onda `AC-E11` é a aplicação concreta e pendente deste princípio: `battle_preview.cpp` (2943 linhas) e `battle_scene.cpp` (2094 linhas) são hoje hosts monolíticos do `app/` — todo `EffectKind`/carta/status novo os toca, o padrão inverso do que este ADR recomenda pro domínio. Decompor por responsabilidade (separar modelo, layout, input e loop de render em módulos testáveis isoladamente) é o objetivo declarado de `AC-E11`, planejado com opus-latest + líder por ser trabalho de refator estrutural, não mecânico.

## Onde aplicar a seguir

- **Quests/missões:** catálogo de dados (objetivos, gatilhos, recompensas) + um executor de steps genérico, no mesmo espírito do grafo de diálogo — não uma classe C++ por missão.
- **Rotina/horário de NPCs (`NPCS-INIMIGOS-ROTINA-HORARIO`):** tabela de dados (posição × horário × estado), não um script por NPC.
- **Sistema de cartas hardware/pirataria (`reference_sistema_cartas_hardware_energia`):** ROM/EPROM, baterias CR2032, degradação — modelados como dados que um motor de energia/durabilidade consome, não lógica condicional por item.
- **Efeitos adiados (`EFEITOS-ADIADOS-OCULTOS`, o item Bastiat bloqueado de `CARD-ENGINE-MANIFESTO`):** a categoria de efeito "condição oculta que dispara depois" precisa de um mecanismo genérico de agendamento de dado — não um `EffectKind` ad-hoc por carta que precisar de atraso.

## Consequências

**Positivas:**
- Toda a lista de "por quê" acima — DRY, testabilidade concentrada, extensão sem tocar no executor, auditabilidade por não-programador.
- A disciplina de enum append-only + `-Werror=switch` já provou valor concreto: `HUD-STATUS-ICONS-STALE` foi pego e fechado (5 mutantes mortos, 0 sobreviventes, re-verificação do orquestrador) exatamente porque o padrão data-driven tornou o bug **mecanicamente detectável** (um `switch`/array fora de sincronia), em vez de um comportamento errado escondido dentro de um `if` de instância.
- Onboarding de conteúdo (playtest N=3, balanceamento, curadoria de bestiário) não exige tocar C++.

**Negativas / aceitas como custo:**
- **Indireção.** Entender uma carta exige ler DOIS lugares — o dado (`master_cards.cpp`) e o handler (`techmagic.cpp`) — nunca um só. Aceito: o custo é pago uma vez por `EffectKind`, não uma vez por carta.
- **Disciplina de migração de schema.** Todo enum append-only e todo save versionado empurra custo pra frente (migrators, `static_assert` de contagem, cuidado com ordinal) em troca de nunca reescrever o motor. É o preço já pago desde `ADR-006`/`ADR-007`.
- **A tentação do "dado esperto demais".** Quando um campo de dado começa a carregar lógica condicional própria (um `EffectSpec` com um parâmetro que só faz sentido combinado com outro campo de um jeito ad-hoc, ou um catálogo que precisa de um `if` de leitura pra decidir o que fazer com uma entrada), isso é sinal de que falta um `EffectKind`/handler novo — não de que o dado precisa ficar mais complexo. A disciplina do motor é: quando o dado quer virar lógica, ele vira um handler novo, pequeno e testado, nunca um parâmetro extra interpretado condicionalmente dentro de um handler existente.

## Reversibilidade

Two-way door quanto ao MECANISMO (nenhum código muda com este ADR — é canonização, não implementação); one-way door quanto à DISCIPLINA de enum append-only quando aplicada a formatos já persistidos (save, `.gmap`) — ali, reordenar um enum já em produção quebra dado existente e não é reversível sem migrator.
