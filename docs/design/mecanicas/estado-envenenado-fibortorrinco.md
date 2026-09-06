<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Estado envenenado (aplicação do Fibortorrinco-Zeckendorf / gota de veneno)

**Status:** DESIGN fechado pelo líder e pelo Gus Dragon em 06/09/2026, dois números-base (13% / 8%, ver §1) e a escala por dificuldade inteira (§3, decisão do líder: "opção 2, mas marca para medir quando chegar a hora"). Todo valor da escala é canon vigente, marcado `//PLAYTEST` onde ainda falta medição em combate real, no mesmo formato que `combat.md` linha 599 já usa para número fechado pelo criador e pendente de validação empírica. **CÓDIGO = feat separada**, ainda não implementada.

Cross-ref: `docs/design/mecanicas/cartas/_vocabulario.md` §5 (status `Poison`, já existente: "dano contínuo por veneno"), `docs/design/mecanicas/combat.md` §9 (Status framework, campos `Magnitude`/`Duration`/`StackRule`), `docs/design/mecanicas/modos-morte.md` §2.4 (precedente de escala percentual por dificuldade), `docs/narrative/diary/entries-fichas-bestiary.md` §5.5 (o Fibortorrinco-Zeckendorf, que aplica este estado pelo esporão) e `docs/design/mecanicas/unha-veneno-fibortorrinco-item.md` (a gota de veneno, que também o aplica).

## 1. O que este documento fecha

Este estado **reusa o status `Poison`** já canônico no vocabulário de cartas (`cartas/_vocabulario.md` §5: "dano contínuo por veneno") e no framework de combate (`combat.md` §9: "dano por tick no `TurnStart`"). Não é um `StatusId` novo. O que este documento fixa são os **parâmetros** desta aplicação específica, no mesmo espírito de `combat.md` §9 ("Magnitudes vêm sempre da carta/combo que aplica, nunca hardcoded"):

- **Dano por turno:** **13% da vida (máxima)** do alvo, por tick, no `TurnStart`. Isto é uma diferença deliberada do padrão de `Magnitude` flat que as outras fontes de `Poison` usam hoje: aqui a magnitude é percentual, não um número fixo. Faz sentido narrativo (o veneno de um bicho que resiste até HP muito alto precisa continuar relevante contra alvos de HP alto), mas é uma variação de forma que o `economy-designer` precisa ratificar antes de entrar no motor, porque muda como `Magnitude` é lida para esta fonte.
- **Chance de cura automática:** **8% por turno**, avaliada a cada `TurnStart` em que o estado segue ativo. Se a rolagem acerta, o estado é removido antes do dano daquele tick (ou depois, ver pendência abaixo).

## 2. Mecânica nova: cura automática por chance

**Não existe, em nenhum outro lugar do canon revisado, um status com chance de remoção antecipada por turno.** Os status hoje expiram por `Duration` decrementando (`combat.md` §9) ou por `Dispel` explícito (carta/item, como a Ampola de Antídoto, `economia.md` §7.2, ou o macarrão ao alho e óleo, `comidas-ingredientes-craft.md` §5.5.1c). Uma chance probabilística de auto-cura, independente de `Duration` e sem gasto de recurso do jogador, é mecânica nova, registrada aqui pela primeira vez.

**Pendência de ordem, não decidida:** se a rolagem dos 8% acontece **antes** do dano do tick (o alvo pode escapar do dano daquele turno) ou **depois** (o alvo sempre leva pelo menos aquele tick, e só então pode curar). As duas leituras são defensáveis e mudam a expectativa de dano total; fica para o líder ou para o `gameplay_engineer`, na onda de implementação.

## 3. Escala por dificuldade (canon vigente, decisão do líder em 06/09/2026)

O líder confirmou a escala fibonacciana como valor vigente do jogo, com a ressalva de marcar cada linha que ainda não passou por combate real para medição futura ("opção 2, mas marca para medir quando chegar a hora"). A escala ancora nos dois precedentes já canônicos do projeto:

- `modos-morte.md` §2.4 (percentuais crescentes de severidade por dificuldade: 5% / 34% / 89% / 100%, marcados `//PLAYTEST`, "a fixar na onda de balanceamento").
- A "sequência numérica recorrente" (Fibonacci) já usada em várias curvas do projeto (`battle-screen.md`: safe mode 13%; `economia.md` §7.9: pedágio 13 cr; §7.2: cura em degraus 1,1,2,3,5).

Os dois números do líder (13% de dano, 8% de cura automática) valem como **Médio** (dificuldade default do jogo, `modos-morte.md` §2.1), o único que não recebe marca de medição por já vir dele diretamente. As outras três dificuldades escalam os dois eixos em direções opostas (mais difícil = veneno mais perigoso e mais difícil de curar sozinho), usando só valores que já são membros da sequência de Fibonacci, no mesmo idioma numérico do resto do projeto:

| Dificuldade | Dano por turno (`Magnitude`, % de HP máx.) | Chance de cura automática por turno |
|---|---|---|
| **Fácil** | 8% `//PLAYTEST` | 13% `//PLAYTEST` |
| **Médio** *(default, dado pelo líder)* | **13%** | **8%** |
| **Difícil** | 21% `//PLAYTEST` | 5% `//PLAYTEST` |
| **Hardcore** | 34% `//PLAYTEST` | 3% `//PLAYTEST` |

**A tabela inteira é canon vigente.** A marca `//PLAYTEST` em Fácil, Difícil e Hardcore segue o mesmo formato que `combat.md` linha 599 usa para "número fechado pelo criador" ainda sem medição empírica: não significa "em aberto", significa "vale até que combate real meça e, se preciso, recalibre". Só a linha de Médio nasce sem essa marca, por já vir da medida do líder.

## 4. Quem aplica este estado

- **Esporão do Fibortorrinco-Zeckendorf** (`entries-fichas-bestiary.md` §5.5): 8% de chance por golpe, número do líder, não afetado por este documento (é a chance de **aplicar**, distinta da chance de **curar automaticamente** já ativo, §1).
- **Gota de veneno de Fibortorrinco-Zeckendorf** derramada em arma de mão (`unha-veneno-fibortorrinco-item.md` §5): 21% de chance por golpe daquela arma.

Os dois usam o mesmo estado com os mesmos parâmetros deste documento; não há uma versão "do inimigo" e uma "do item" com números diferentes, a menos que o líder decida abrir essa distinção depois.

## 5. Pendências

- Ordem da rolagem de cura automática dentro do `TurnStart` (§2).
- Ratificação, pelo `economy-designer`, de `Magnitude` como percentual de HP máximo em vez de flat, para esta fonte (§1).
- Medição em combate real das linhas `//PLAYTEST` da tabela de escala por dificuldade (§3): a escala em si é canon fechado, o que falta é validar os números contra jogo rodando.
- Interação com `Dispel`/itens que já limpam `Poison` (Ampola de Antídoto, macarrão ao molho pesto, `comidas-ingredientes-craft.md` §5.5.1f): presume-se que funcionam normalmente contra este estado, por ser o mesmo `StatusId`, mas isso não foi perguntado ao líder à parte.
