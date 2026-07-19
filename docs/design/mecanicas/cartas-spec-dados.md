# Cartas hardware/energia/pirataria — SPEC DO MODELO DE DADOS (POCO de domínio)

**Status:** PROPOSTA — spec de design/engenharia, NÃO implementada. Escrita pelo `backend-engineer` a pedido do orquestrador, 2026-07-18. Aguarda revisão do líder + onda de implementação futura (referida hoje como `CARDS-HARDWARE` no backlog, ainda sem item formal em `TODO.md`). Nenhum código de produção foi tocado para produzir este documento; nenhum arquivo em `GusEngine/` foi alterado.

**Cross-ref (leia antes, este doc não duplica):**

- [`cartas-hardware-pirataria-energia.md`](cartas-hardware-pirataria-energia.md) — o SISTEMA (14 seções): tipos de carta, memória Runa, bateria CR2032, integridade/vírus, conector RSB, mercado negro. Fonte de VERDADE do design; este doc só traduz em structs.
- [`cartas-numeros-proposta.md`](cartas-numeros-proposta.md) — os NÚMEROS (capacidade de bateria por dificuldade, memória kR/MR, % de contaminação etc.). Este doc referencia constantes por nome, não recopia valores.
- [`cartas-technomagik.md`](cartas-technomagik.md) — os 3 tiers canônicos (COMUM/ESPECIAL/SUPER), `ManaCost`, `CardTier`/`CardCategory`, taxonomia da carta-catálogo.
- [`deck-mao-sistema.md`](deck-mao-sistema.md) — deck ativo/morto, `CardInstance`, invariantes anti-exploit (§7), classe PROTEGIDA (inv.9).
- Código já existente consultado (fonte de verdade da forma atual, não deste doc):
  `GusEngine/domain/include/gus/domain/combat/combat_records.hpp` (`struct Card`, catálogo),
  `GusEngine/domain/include/gus/domain/deck/deck_records.hpp` (`struct CardInstance`),
  `GusEngine/domain/include/gus/domain/deck/card_collection.hpp` (agregado deck ativo/morto),
  `GusEngine/domain/include/gus/domain/deck/deck_constants.hpp` (padrão `//PLAYTEST`),
  `GusEngine/domain/include/gus/domain/save/save_data.hpp` + `save_migrators.hpp` (save versionado V6, `CardCollectionState`, chain de migrators).

---

## 1. Escopo deste documento

**Dentro do escopo:** o modelo de dados ESTÁTICO (structs/enums POCO) que representa a camada física de uma carta possuída (origem ROM/EPROM/pirata, bateria, integridade/vírus oculto, conector) e como ele se encaixa no `Card` (catálogo) e `CardInstance` (posse) já existentes, sem duplicar dado. Invariantes que o dado em si deve respeitar. Notas de serialização/save (bump de schema).

**Fora do escopo (marcado explicitamente em cada seção relevante):**

- **Transações/lógica de aquisição** (rolar contaminação na compra/loot/upload homebrew, craft de homebrew via bancada, comprar carta pirata no mercado negro) — são funções de `deck_transactions.hpp` FUTURAS, análogas a `acquire()`/`sell()`/`upload()` já existentes; este doc só define o QUE elas escreveriam num `CardPhysicalState`, não COMO.
- **Execução de payload de vírus em combate** (logic bomb, backdoor, worm, zip-bomb, adware) — é lógica de `gameplay_engineer`/executor techMagic (possíveis `EffectKind` novos), fora de `domain/` puro de dados.
- **UI/RunaDex, minigame do adware (X de fechar), tela de diagnóstico do Turing** — `app/`+`platform/` (RmlUi), fora de `core/`+`domain/`.
- **Item de bateria pirata/genérica avulsa** (§5 do doc-fonte: bateria comprável que "mente sobre a carga") — é um item de INVENTÁRIO separado da carta, não modelado aqui; fica para uma spec própria quando essa feature entrar em onda.
- **Reflash de uma carta homebrew JÁ possuída para um conjuro DIFERENTE** — ver AMB-DADOS-04 (§8). Este doc assume, como default de MVP, que `card_id` de uma `CardInstance` é fixo pela vida da instância (mesma premissa que o código atual já assume implicitamente).

---

## 2. Fronteira de camada (onde isso vive)

Tudo que este doc especifica é **POCO de `domain/`** — zero SDL/GL/RmlUi/IO, mesma regra de `combat_records.hpp`/`deck_records.hpp`. Nenhum campo aqui conhece "como desenhar" nem "quando mostrar ao jogador".

| Camada | O que faz com este modelo |
|---|---|
| **`domain/`** (este doc) | Define os campos, os invariantes de forma (`validate()`), as funções puras derivadas (SoH, conector, classe-de-hardware). ZERO regra de "quando revelar ao jogador". |
| **`app/` (gameplay)** | Decide QUANDO rodar a rolagem de contaminação, QUANDO o Turing diagnostica (seta `is_diagnosed`), QUANDO um payload de vírus dispara em combate. Consome os campos, não os interpreta visualmente. |
| **`platform/`+`app/` (UI)** | Decide O QUE MOSTRAR: um card com `is_infected=true, is_diagnosed=false` deve renderizar como uma carta NORMAL (o jogador não pode ver o campo cru) — a UI só pode exibir "infectada" depois que `is_diagnosed=true`. Esse gate é regra de APRESENTAÇÃO, mas o INVARIANTE ("nunca vaza is_infected antes de is_diagnosed") é nomeado aqui em §6 porque é contrato entre camadas, não implementação de UI. |

Isso espelha a regra de "invariante no construtor / fail-fast" e ainda respeita a regra de código já registrada no `master_cards.hpp`/`combat_records.hpp` de nunca deixar `domain/` decidir apresentação.

---

## 3. Extensão mínima ao catálogo (`Card`, `combat_records.hpp`)

Um único campo ADITIVO, no fim do struct (mesmo padrão usado por `synergy_statuses`/`restore_ap` — campo aditivo, default neutro, zero mudança de comportamento nas cartas existentes):

```cpp
// gus/domain/combat/combat_records.hpp — struct Card, ACRÉSCIMO ao final

// Clone-falso de especial (cartas-hardware-pirataria-energia.md §2, "Clone-falso de
// especial"): quando preenchido, esta entrada de CATÁLOGO é uma imitação pirata que
// IMPERSONA a carta especial cujo Card::id está aqui. A imitação é sua PRÓPRIA entrada
// de catálogo (tier = CardTier::Comum, id PRÓPRIO, ex. "cardExec-faraday-fake"), NUNCA
// o mesmo id da especial real — isso preserva a unicidade (inv.9, deck-mao-sistema.md
// §7) sem exigir NENHUM caso especial no CardCollection: a imitação é só uma comum
// como qualquer outra (descartável, vendável), com display/flavor apontando pra
// especial que ela finge ser. std::nullopt = carta normal (imensa maioria; default
// preserva todo catálogo existente intacto).
std::optional<std::string> mimics_special_id;
```

**Por que catálogo, não instância:** todo exemplar de `cardExec-faraday-fake` finge a MESMA especial, por definição — é um fato do "o que esta carta É", não do "esta cópia específica". Guardar por instância duplicaria a mesma string em cada cópia, sem motivo (mesmo princípio que já rege `CardInstance` não duplicar `mana_cost`/`power` do catálogo).

---

## 4. Extensão mínima à instância (`CardInstance`, `deck_records.hpp`)

Um único campo ADITIVO, no fim do struct:

```cpp
// gus/domain/deck/deck_records.hpp — struct CardInstance, ACRÉSCIMO ao final
struct CardInstance {
    std::uint64_t instance_id = 0;
    std::string card_id;

    // NOVO — cartas-spec-dados.md. Estado físico MUTÁVEL desta cópia específica
    // (bateria/degradação/integridade são por-EXEMPLAR, não por-catálogo: duas cópias
    // da MESMA cardExec-tavusa podem ter cargas de bateria e status de infecção
    // diferentes). Default = CardPhysicalState{} = "ROM original legítima, bateria
    // cheia, sem infecção" — ver §5.2 sobre por que o zero-value é o estado SEGURO.
    CardPhysicalState physical;

    [[nodiscard]] bool operator==(const CardInstance&) const = default;
};
```

Novo arquivo-irmão `gus/domain/deck/card_hardware.hpp` (mesmo diretório de `deck_records.hpp`), contendo os enums e o struct abaixo.

---

## 5. `CardPhysicalState` e enums

### 5.1 Enums

```cpp
namespace gus::domain::deck {

// Origem física da cópia (cartas-hardware-pirataria-energia.md §2/§3).
enum class CardOrigin : std::uint32_t {
    OriginalRom   = 0,  // ROM de fábrica, nao regravável, "queima na hora" se adulterada
    HomebrewEprom = 1,  // EPROM vazia gravada em bancada de mercado negro, regravável-até-queimar
    PirateClone   = 2,  // clone pirata, ROM travada após gravar 1x
};

// Payload oculto de vírus (§8). None = sem infecção.
enum class VirusKind : std::uint32_t {
    None            = 0,
    LogicBomb       = 1,  // "sabotador de combate" — falha/vira contra o jogador
    Backdoor        = 2,  // spyware — reporta mão/deck/posição ao inimigo
    Worm            = 3,  // espalha pro deck em cadeia + degrada performance
    FalseBenign     = 4,  // isca Bastiat — parece buff, cobra custo oculto retardado
    AdwareSterling  = 5,  // propaganda intransponível antes do efeito
    ZipBomb         = 6,  // incha na hora de soltar, estoura memória/bateria
};

// Conector RSB — DERIVADO de CardOrigin (§3 do doc-fonte é 1:1: Original=nenhum,
// Homebrew=externo visível, Pirata=interno escondido). NUNCA armazenado — ver
// connector_of() em §5.3; existe aqui só como o tipo de retorno da função pura.
enum class RsbConnector : std::uint32_t {
    None            = 0,
    ExternalVisible = 1,
    InternalHidden  = 2,
};

// Classe de hardware para lookup de config (capacidade de bateria por dificuldade,
// % de contaminação — cartas-numeros-proposta.md §1a/§3). NÃO é um campo armazenado:
// é a CHAVE derivada de (CardTier do catálogo, CardOrigin, mimics_special_id da
// carta) — ver hardware_class_of() em §5.3. 5 classes porque "pirata especial"
// (clone-falso) tem risco de contaminação PRÓPRIO (8%), distinto de "pirata comum"
// (21%), mesmo os dois sendo CardTier::Comum no catálogo.
enum class HardwareClass : std::uint32_t {
    ComumOriginal       = 0,
    HomebrewEprom       = 1,
    PirataComum         = 2,
    PirataEspecialFalso = 3,  // clone-falso — Card::mimics_special_id preenchido
    EspecialSelada      = 4,  // CardTier::Especial real (sempre CardOrigin::OriginalRom)
};

}  // namespace gus::domain::deck
```

### 5.2 `CardPhysicalState`

```cpp
namespace gus::domain::deck {

struct CardPhysicalState {
    CardOrigin origin = CardOrigin::OriginalRom;

    // ---- Bateria (cartas-hardware-pirataria-energia.md §5 / cartas-numeros-proposta.md §1) ----

    // Fonte de verdade da degradação: cada RECARGA EM-LUGAR (mesma bateria física,
    // "estação de recarga" ou item Carregador) incrementa 1. SoH = 100 − 13×cycles
    // (clamp 0), derivado — ver state_of_health_percent() §5.3. NÃO é o mesmo que
    // "trocar de bateria" (item novo consumido do inventário): troca ZERA este campo
    // (bateria física NOVA, SoH 100%), recarga-em-lugar INCREMENTA (mesma bateria,
    // degradando). Essa distinção de operação é do consumidor (deck_transactions.hpp
    // futuro), não deste struct.
    std::uint16_t battery_recharge_cycles = 0;

    // Déficit de carga em relação à capacidade ATUAL (não a carga absoluta!). 0 =
    // bateria CHEIA. Carga restante utilizável = capacidade(hardware_class,
    // dificuldade) − battery_charge_deficit, clamp >= 0 (nunca negativo mesmo se a
    // capacidade cair por algum motivo futuro). Cada cast consome
    // += Card::mana_cost (cartas-numeros-proposta.md §1a: "recurso Y" = ManaCost da
    // carta, já canônico — nenhum eixo novo). Modelado como DÉFICIT, não como
    // "carga_atual" absoluta, de propósito: o valor-zero (default de struct, e o
    // valor que um save V6 migrado ganha por não ter este campo) significa "bateria
    // cheia" — o estado mais SEGURO/generoso pra retrocompatibilidade (§7).
    std::uint32_t battery_charge_deficit = 0;

    // ---- Integridade / vírus (cartas-hardware-pirataria-energia.md §8/§9/§11) ----

    // OCULTO ao jogador por regra de design (nunca exposto cru na UI antes de
    // is_diagnosed==true — ver §6 inv.6). Setado 1x, no momento da rolagem de
    // contaminação na AQUISIÇÃO (fora deste doc, ver §7), OU pelo gatilho narrativo
    // scriptado do Dante/arma-Sterling em CardTier::Especial (nunca por rolagem
    // aleatória nessas — ver §8.2).
    bool is_infected = false;

    // O Turing diagnosticou (ou o payload já disparou e se revelou sozinho). Controla
    // o que a UI pode mostrar; NÃO cura nada por si (cura é evento separado que zera
    // is_infected, ver invariante §6.4).
    bool is_diagnosed = false;

    VirusKind virus_kind = VirusKind::None;

    // Chip permanentemente destruído (queima): resultado possível da tentativa de
    // cura do Turing (cartas-hardware-pirataria-energia.md §11, "X% de queimar o
    // chipset"). Carta com is_burned_out=true fica inutilizável em combate (gate
    // futuro do gameplay_engineer, análogo ao guard de tier do CardCollection) — só
    // pode ir pro ferro-velho. Distinto de bateria morta (SoH<=piso): aquilo é
    // recuperável trocando bateria; isto é PERMANENTE.
    bool is_burned_out = false;

    [[nodiscard]] bool operator==(const CardPhysicalState&) const = default;

    // Fail-fast — ver invariantes de FORMA em §6. Lança std::invalid_argument.
    void validate() const;
};

}  // namespace gus::domain::deck
```

**Por que "zero é seguro" importa aqui:** um `CardInstance` default-constructed (ou migrado de um save V6 antigo que nunca teve este campo) vira automaticamente `{OriginalRom, 0 ciclos, 0 déficit=cheia, não-infectada, não-diagnosticada, sem vírus, não-queimada}` — exatamente o estado de "carta legítima, nova, saudável" que qualquer save pré-existente deveria ganhar retroativamente, sem punição nem benefício indevido. Isso é o mesmo princípio que `EffectSpec::side_filter = SideFilter::Any` (default = nenhum filtro, comportamento antigo preservado) já usa neste código.

### 5.3 Funções puras derivadas

Não são structs novos, são funções livres (mesmo estilo de `TierLookup` do `card_collection.hpp` — callback/lookup, não método de struct, porque cruzam pro catálogo):

```cpp
namespace gus::domain::deck {

// SoH em pontos percentuais, clamp [0,100]. 13pp por recarga (cartas-numeros-
// proposta.md §1b, FECHADO). Puro, sem I/O.
[[nodiscard]] std::uint8_t state_of_health_percent(const CardPhysicalState&) noexcept;

// SoH <= 21% (piso de descarte, cartas-numeros-proposta.md §1b) — "não serve mais
// pra recarregar em-lugar", só vender/reciclar OU trocar de bateria (que zera o
// campo, ver §5.2).
[[nodiscard]] bool is_battery_dead(const CardPhysicalState&) noexcept;

// Carga restante = capacidade_atual − battery_charge_deficit, clamp >= 0. Recebe a
// capacidade já resolvida (chamador calcula via battery_capacity_for, abaixo) — esta
// função NÃO conhece dificuldade/classe, só faz a subtração/clamp.
[[nodiscard]] std::uint32_t battery_charge_remaining(const CardPhysicalState&,
                                                      std::uint32_t capacity) noexcept;

// Conector RSB — DERIVADO 1:1 de origin (§3 doc-fonte), nunca armazenado.
[[nodiscard]] RsbConnector connector_of(CardOrigin) noexcept;

// Classe de hardware pra lookup de config (capacidade de bateria / % contaminação).
// catalog_tier vem de um TierLookup (mesmo padrão de card_collection.hpp);
// mimics_special vem de Card::mimics_special_id.has_value() do catálogo. Combina os
// 3 fatores (tier, origin, mimics_special) nas 5 classes de §5.1. CardTier::Especial
// SEMPRE mapeia para EspecialSelada (a validação de origin==OriginalRom p/ Especial
// é invariante separado, §6.5 — esta função não valida, só classifica).
[[nodiscard]] HardwareClass hardware_class_of(gus::domain::combat::CardTier catalog_tier,
                                               CardOrigin origin,
                                               bool mimics_special) noexcept;

}  // namespace gus::domain::deck
```

`battery_capacity_for(HardwareClass, DifficultyLevel)` e a tabela de `%` de contaminação por `HardwareClass` (ambas config-driven, não código de decisão) ficam em `card_hardware_constants.hpp` novo — ver §9.

---

## 6. Invariantes (mesmo estilo de deck-mao-sistema.md §7 — engenharia, não design)

1. **`CardPhysicalState::validate()` — forma:** `virus_kind != VirusKind::None` ⟹ `is_infected == true` (não existe "tipo de vírus" sem infecção). `is_diagnosed == true` ⟹ `is_infected == true` (não se diagnostica infecção que não existe — ver nuance de cura no item 4). Falha = `std::invalid_argument`.
2. **`origin` é fixo pela vida da instância** (MVP — ver AMB-DADOS-04 sobre reflash). Nenhuma API deste doc muda `physical.origin` depois da criação da `CardInstance`.
3. **`is_infected` nunca é lido cru pela UI antes de `is_diagnosed == true`.** Este é um contrato de FRONTEIRA (domain armazena a verdade; apresentação decide quando revelar), não um campo — não há como "esconder" tecnicamente o valor dentro do próprio struct (não é criptografado nem deveria ser, o save inteiro já é AEAD/HMAC, ADR-015). A obrigação é do consumidor de UI: renderizar `is_infected` como se fosse `false` sempre que `is_diagnosed == false`. Reforçar em code review / teste de UI quando a feature for implementada.
4. **Cura consome o segredo, não o inverso:** uma tentativa de cura bem-sucedida (Turing, §11 doc-fonte) seta `is_infected=false, virus_kind=None, is_diagnosed=false` (volta ao estado limpo — não precisa mais "lembrar" que já foi diagnosticada, porque não há mais nada a diagnosticar). Uma tentativa **malsucedida** seta `is_burned_out=true` (chip morre) — `is_infected`/`virus_kind` ficam como estavam (carta agora inutilizável de qualquer forma; o vírus "morre junto" na prática porque a carta não roda mais, mas o dado não precisa ser limpo).
5. **`CardTier::Especial` (catálogo real, não clone-falso) só existe com `origin == CardOrigin::OriginalRom`.** Nunca existe uma especial-de-catálogo-real com origem Homebrew/Pirata — a ÚNICA forma de algo "parecido com pirata" tocar uma especial é o clone-falso (§3, catálogo PRÓPRIO, tier Comum). Quem cria `CardInstance` de uma especial (Tavus-Eco, entrega narrativa) DEVE deixar `physical` no default (`CardPhysicalState{}`), nunca setar origin manualmente.
6. **`is_infected` de uma `CardTier::Especial` só vira `true` por gatilho narrativo explícito** (a arma do Dante/Sterling contra a Gaiola de Faraday, doc-fonte §9), nunca pela rolagem genérica de contaminação por `HardwareClass` (essa rolagem, quando implementada, DEVE recusar operar sobre `CardTier::Especial` — 0% é canônico e não-RNG, não "RNG que sempre dá 0", ver AMB-DADOS-02). Reforça inv.9 do deck-mao-sistema.md (classe protegida) por um ângulo diferente: proteção não é só "não pode ir pro deck morto", é também "não pode ser sabotada por sorte".
7. **`battery_charge_deficit` nunca é decrementado por leitura, só por operação explícita de recarga/troca** (evita drift silencioso — mesmo espírito do `CardCollection` só mutar via API pública nomeada, nunca por acesso direto a campo).
8. **`mimics_special_id` (catálogo) só é lido/setado no momento de CRIAR a entrada de catálogo** (dado estático, curadoria de conteúdo — como `display_name`), nunca em runtime de combate/save.

---

## 7. Clone-falso: como não quebrar inv.9 (deck-mao-sistema.md)

Resolvido inteiramente por modelagem, sem NENHUM caso especial em `CardCollection`/`card_collection.hpp`:

- A carta pirata que finge ser a Gaiola de Faraday é uma entrada de catálogo **PRÓPRIA** — `card_id = "cardExec-faraday-fake"` (nome exato = decisão de conteúdo, não deste doc), `tier = CardTier::Comum`, `mimics_special_id = "cardExec-faraday"`.
- Por ser `CardTier::Comum`, o guard de tier existente em `CardCollection::guard_protected_tier` (inv.9) **já a trata como descartável/vendável normalmente** — ela NUNCA entra em conflito com a proteção da especial real (que tem `card_id` DIFERENTE, `"cardExec-faraday"`).
- `hardware_class_of(CardTier::Comum, CardOrigin::PirateClone, /*mimics_special=*/true)` resolve para `HardwareClass::PirataEspecialFalso` — puxa a linha de contaminação 8% (não a 21% de pirata comum genérico) da tabela de `cartas-numeros-proposta.md` §3.
- **Zero mudança em `CardCollection`, `deck_records.hpp` (além do campo `physical`), ou nos invariantes 1-9 já fechados do deck-mão.** Isso é o motivo desta modelagem ter sido preferida a alternativas (ex.: um `CardTier::PirataEspecial` novo) — o eixo pirataria é ORTOGONAL ao eixo de raridade/proteção já fechado, não deveria mexer nele.

---

## 8. Ambiguidades registradas (decisão do líder, não técnica)

- **AMB-DADOS-01 — capacidade de bateria de `HardwareClass::PirataEspecialFalso`:** a tabela de `cartas-numeros-proposta.md` §1a lista capacidade para Homebrew/Pirata-comum/Comum-original/Especial-selada, mas **não** para "pirata especial" (clone-falso). Falta uma 5ª coluna ou uma regra de derivação (ex.: "mesma capacidade do Pirata comum, já que o hardware por baixo é igual, só o disfarce muda"). Não decidido aqui — sinalizado pro `economy-designer`/líder na próxima rodada de números.
  - **RESOLVIDO (líder autorizou fechar via `economy-designer`, 2026-07-19): Fácil 68 · Médio 34 · Difícil 17 · Hardcore 17.** A hipótese "mesma capacidade do Pirata comum" foi CONTESTADA e substituída: `PirataEspecialFalso` fica um degrau ACIMA do Pirata comum (não igual), espelhando a própria tabela de contaminação (§3, já fechada) que trata o clone-falso como hardware de qualidade intermediária (8% de risco, entre Comum original 1% e Pirata comum 21% — o vendedor que fabrica um disfarce convincente usa componentes melhores que o pirata genérico). O valor cai de graça na escada Fibonacci já em uso no §1a: no Médio, 34 é o degrau que faltava entre 21 (Pirata comum) e 55 (Comum original) na sequência 8,13,21,**34**,55,89,144 — nenhum número novo inventado. Fácil/Difícil/Hardcore seguem a mesma regra ×2/÷2 das outras 4 classes (34 é par, então Difícil/Hardcore fecha exato em 17). Ordem monotônica preservada: Homebrew < Pirata comum < Pirata especial falso < Comum original < Especial. Ver `cartas-numeros-proposta.md` §1a/§0 para a tabela completa e o racional.
- **AMB-DADOS-02 — exato `VirusKind` + mecanismo de disparo da arma do Dante/Sterling contra a Faraday:** o doc-fonte (§9) descreve a arma como "fabricada sob medida para furar e neutralizar" a Faraday especificamente, mas não fixa qual dos 6 `VirusKind` ela usa (provavelmente `LogicBomb`, mas pode ser um 7º kind exclusivo de arma-industrial, fora da lista de vírus "civis" do mercado negro). Também não fixa SE o resultado da arma é `is_infected=true` (recuperável por Turing) ou direto `is_burned_out=true` (a carta simplesmente morre no clímax, permanente — ler o §9 do doc-fonte como "a carta-companheira morre" sugere ISTO, não uma infecção curável). Esta é a decisão de maior peso dramático deste documento — recomendo o líder revisar antes de qualquer implementação, porque muda o final do arco Faraday.
  - **RESOLVIDO (líder, 2026-07-18): HÍBRIDO.** A arma **QUEIMA** a carta (`is_burned_out=true`, NÃO `is_infected` curável comum). Destino **permanente EXCETO** pelo branch de **redenção do Dante**, que é a ÚNICA forma de restaurá-la (ele remove o vírus que plantou → a carta volta). Modele o disparo da arma como um `VirusKind` próprio de arma-industrial (fora da lista de vírus "civis" do mercado negro) OU um flag dedicado; o efeito final é **queima**, e a recuperação é um evento scriptado do branch de redenção, não a cura genérica do Turing.
- **AMB-DADOS-03 — se `is_burned_out` (falha de cura OU arma-Sterling) EXCLUI a carta do `CardCollection` ativo automaticamente, ou fica "presente mas inutilizável" até o jogador a descartar manualmente:** o doc-fonte (§11) diz "conviver, arriscar remoção, ou descartar no ferro-velho" — sugere que fica no inventário, inerte, até ação do jogador. Assumido como default de leitura, mas vale confirmação (afeta se `is_burned_out=true` numa `CardTier::Especial` viola inv.9 "nunca sai por descarte" — se a carta morre sozinha por queima, isso NÃO é "sair por descarte" no sentido do inv.9, que é sobre a AÇÃO do jogador de descartar; recomendo tratar como estados ortogonais, mas é o líder quem fecha).
- **AMB-DADOS-04 — reflash de homebrew já possuída:** o doc-fonte descreve a EPROM como "regravável, mas desgasta até queimar" (8 regravações, `cartas-numeros-proposta.md` §2). Este doc assumiu, como default de MVP, que `card_id` de uma `CardInstance` NUNCA muda depois de criada (cada compilação-na-bancada = uma `CardInstance` NOVA), e que "regravações" é textura narrativa/técnica do porquê o EPROM é instável, não uma ação repetível do jogador de trocar o conjuro de uma carta já possuída. Se o líder quiser a ação "levo minha EPROM de volta à bancada e regravo com outro conjuro" como mecânica real, o modelo precisa mudar (campo `card_id` deixa de ser efetivamente imutável, ganha uma API de "reflash" com guard de ciclos restantes) — mudança não-trivial, melhor decidir antes de implementar do que depois.

---

## 9. Onde os NÚMEROS entram (config, não hardcode)

Novo arquivo `gus/domain/deck/card_hardware_constants.hpp`, mesmo padrão de `deck_constants.hpp` (todas as constantes marcadas `//PLAYTEST`, nenhum consumidor deve hardcodar valor solto):

- `battery_capacity_for(HardwareClass, DifficultyLevel) -> uint32_t` — tabela de `cartas-numeros-proposta.md` §1a (Homebrew/Pirata comum/Pirata especial falso/Comum original/Especial: Fácil 16/42/68/110/288 · Médio 8/21/34/55/144 · Difícil 4/10/17/27/72 · Hardcore 4/10/17/27/72), 5 classes completas (AMB-DADOS-01 RESOLVIDO, §8).
- `kBatteryDegradationPerRechargeCyclePp = 13`, `kBatteryDeadSohFloorPercent = 21` (§1b, FECHADO, não escala por dificuldade).
- `contamination_percent_for(HardwareClass) -> uint8_t` — tabela de `cartas-numeros-proposta.md` §3 (1/8/21/55%, Especial=0 fixo canônico), **não** parametrizada por dificuldade (FECHADO pelo líder — homebrew 55% é fixo em todo modo).
- `kWormPropagationChancePercent = 13` (§3, propagação secundária por cast de carta já infectada).
- `kEpromRewriteCyclesBeforeBurnout = 8` (§2, só relevante se AMB-DADOS-04 for resolvida a favor de reflash).
- `kTuringCureSuccessPercent = 62`, `kTuringCureBurnoutPercent = 38` (§6 numeros-proposta, split usado na tentativa de cura).

Nenhum destes valores é recopiado no corpo deste doc além do necessário para nomear a constante — a fonte é sempre `cartas-numeros-proposta.md`.

---

## 10. Serialização / save (bump V6 → V7)

Segue o padrão exato já estabelecido em `save_migrators.hpp` (comentário de cabeçalho, chain forward-only, `serialize_save_vN` fixture por versão):

- **V7 = + `CardPhysicalState` dentro de cada `CardInstance`** (ativo e morto) de `CardCollectionState`, via `CardInstance::physical` (§4). **Nenhum campo novo no nível de `SaveData` nem `CharacterSaveState`** — a mudança é inteiramente dentro do tipo já reusado (`CardCollectionState` "espelha... sem duplicar o shape", comentário já existente em `save_data.hpp`), então ela chega de graça a `CardCollectionState` assim que `CardInstance` ganha o campo.
- **`migrate_v6_to_v7`:** para cada `CardInstance` em `card_collection.active`/`dead` de cada personagem, popula `physical = CardPhysicalState{}` (default = ROM original, bateria cheia, sem infecção — §5.2, "zero é seguro"). Nenhuma leitura de RNG/relógio (migrators são funções PURAS, `CONTRACT.md` §7) — não se tenta "adivinhar" retroativamente se um save antigo teria cartas piratas; a suposição honesta e não-punitiva é que toda carta pré-existente é legítima.
- **`serialize_save_v6` (novo fixture helper, mesmo padrão de `serialize_save_v1..v5`):** serializa no layout V6 (sem `physical`), para as fixtures de migração V6→V7 do `qa-engineer`/testes.
- **`CardCollectionState::validate()`** ganha, opcionalmente, a checagem de invariante 1 (§6) propagada por instância (`physical.validate()` chamado por elemento) — mantém o padrão de "fail-fast na fronteira do save", mesmo espírito do resto do arquivo.
- **`gus::domain::kSaveSchemaVersion`** sobe de 6 para 7; teste-guarda existente (`current_schema_version() == kSaveSchemaVersion`) já cobre o esquecimento por construção.

---

## 11. Extensão de API sugerida (não implementada aqui)

Só para não deixar a costura pro consumidor "adivinhar": `CardCollection::add_to_active` (hoje `(card_id, instance_id_override)`) ganharia um 3º parâmetro opcional:

```cpp
CardInstance add_to_active(std::string card_id,
                            std::optional<std::uint64_t> instance_id_override = std::nullopt,
                            CardPhysicalState initial_physical = {});
```

Default `{}` preserva TODO call-site existente hoje (aquisição legítima, sem pirataria) sem mudança de comportamento — mesmo princípio aditivo do resto do código. A onda futura que implementar aquisição-com-origem (compra no mercado negro, craft homebrew) passa um `initial_physical` já resolvido (origin + resultado da rolagem de contaminação), calculado ANTES da chamada — `CardCollection` continua sem conhecer contaminação/tabelas, só guarda o que recebe (mesma separação de responsabilidade do `TierLookup` já existente).

---

## 12. Próximos passos (quem consome isso)

1. **Líder:** revisar AMB-DADOS-01..04 (§8), principalmente 02 (destino final da Gaiola de Faraday no clímax — maior peso dramático).
2. **`economy-designer`:** fechar o gap de AMB-DADOS-01 (capacidade de bateria do clone-falso) na próxima rodada de `cartas-numeros-proposta.md`.
3. **`backend-engineer` (onda futura, após aprovação):** implementar `card_hardware.hpp`/`card_hardware_constants.hpp`, estender `Card`/`CardInstance`, migrator V6→V7, testes unit (invariantes de `validate()`) + integration (round-trip de save com fixture V6→V7, mesmo padrão de `save_v6_test.cpp`).
4. **`gameplay_engineer` (onda futura, separada):** transações de aquisição-com-origem, rolagem de contaminação, gate de `is_burned_out`/bateria-vazia impedindo cast em combate, propagação de worm, novos `EffectKind` de payload de vírus.
5. **`qa-engineer` (onda futura):** testes adversariais dos invariantes §6 (especialmente inv.6, "Especial só infecta por gatilho narrativo, nunca RNG genérico") e do migrator V6→V7 (round-trip, fixture legada não ganha nem perde integridade).
