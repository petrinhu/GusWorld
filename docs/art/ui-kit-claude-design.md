# UI Kit "GusWorld :: UI Kit" (Claude Design)

**Tipo:** Reference. **Autoria:** o UI Kit inteiro (tokens, componentes, as duas telas, a estética e a taxonomia de família que ele usa) é decisão do líder, feito por ele; nada nele é proposta de agente a ratificar. **Escopo:** descreve o que o líder construiu no Claude Design e como esse kit se relaciona com o canon. **Não decide nada:** registra o que existe e deixa para o líder dizer qual decisão vale hoje onde há duas. Convenção: pt-br, sem travessões.

## 1. O que é e onde vive

Projeto de design-system do líder no Claude Design (claude.ai/design).

- **Nome:** `GusWorld :: UI Kit`
- **projectId:** `cbf71302-f7ed-4306-93d0-9903f9229854`
- **URL:** https://claude.ai/design/p/cbf71302-f7ed-4306-93d0-9903f9229854
- **Namespace do kit:** `GusWorldUIKit_cbf713`
- **Como se lê:** pela ferramenta `DesignSync` (métodos de leitura), depois de `/design-login`. A skill `/design-sync` é o fluxo completo.

Conteúdo, por grupo:

- **Tokens:** `00-tokens.html`
- **Components:** `10-card.html`, `20-slot.html`, `30-chip.html`, `40-button.html`
- **Screens:** `90-bancada.html`, `91-mercado.html`
- **Infraestrutura (não é conteúdo de design):** `_ds_manifest.json`, `_ds_bundle.js`, `_adherence.oxlintrc.json`

## 2. Fronteira da L-27

A L-27 (`GODS_LAWS.md`) diz: "Nenhuma interface, arquivo de marcação, folha de estilo, tela piloto ou prova de conceito é escrita antes de o GlintFx traduzir marcação de tela." Ela vincula os agentes. O kit é do líder e vive no espaço dele, no Claude Design, fora do repositório; este documento é só markdown descritivo e não contém marcação, folha de estilo nem trecho de código de interface.

Existe, à parte, uma cópia recuperada do projeto anterior em `docs/design/ui-kit/` (sete arquivos HTML com os mesmos nomes de `00-tokens` a `91-mercado`, mais `REQUISITOS-UI.md`), versionada desde 22/08/2026. Este documento não a tocou e não afirma que seja idêntica à versão do Claude Design: as frases-chave conferem (mão como seleção, loadout determinístico, regra do NPC do caminho, pilha de fonte com `Consolas`), mas igualdade completa não foi medida. O destino dessa cópia pertence ao item `G7` do `TODO.md`.

## 3. Estética e tipografia

Declaração do kit, verbatim: "estetica cockpit Tatico / terminal cyber-gotico. Fonte mono (PixelOperatorMono)."

Pilha de fonte usada: `"PixelOperatorMono", "Consolas", "DejaVu Sans Mono", ui-monospace, monospace`.

**O kit confirma o canon, não diverge dele.** A `PixelOperatorMono` já é a fonte da interface em `docs/design/mecanicas/terminal-estetica.md` (decide o repertório de glifos com base no que ela tem, e escolhe estender a própria fonte em vez de usar fallback), em `THIRD-PARTY-LICENSES.md` (CC0, Jayvee Enaguas) e nos arquivos em `resources/fonts/`. A estética "cockpit tático" é o nome da variante C aprovada em `docs/design/mecanicas/battle-screen.md` §2 ("Tatico Cockpit"). O que o kit acrescenta são os nomes de fallback depois da primeira fonte; o canon diz "sem fallback" para o texto do jogo (`terminal-estetica.md`, `copy-stubs-combate.md`). Se a pilha com `Consolas` e `DejaVu Sans Mono` do kit deve valer no jogo é pergunta ao líder (ver §8).

## 4. Tokens de cor

| Token | Valor | Token | Valor |
|---|---|---|---|
| bg | `#080b11` | amber | `#e0a53a` |
| panel | `#0d131c` | violet | `#a06be0` |
| panel2 | `#111a26` | green | `#5fe08a` |
| line | `#1d2c3d` | red | `#e05f6b` |
| cyan | `#3ad0e0` | ink | `#c9d6e2` |
| cyan-dim | `#1c6b74` | ink-dim | `#6d8299` |

Fundo de slot: `#0a1119`.

Rótulos de família usados pelo kit: eletrico `#3ad0e0`, termico `#e0703a`, grupo `#e0c93a`, biologico `#5fe08a`, anomalia `#a06be0`.

## 5. Anatomia dos componentes

**Carta.** Barra vertical de 3px na borda esquerda, na cor da família. Nome em negrito. Custo de mana num círculo à direita, com borda cyan-dim. Linha de efeito em ink-dim. Etiqueta de raridade no canto inferior direito. Três estados: normal; selecionada (contorno e brilho cyan, é o foco de teclado e gamepad); na mão (esmaecida a 42 por cento, com o sufixo "[na mao]" em verde). A carta ESPECIAL tem tratamento próprio: borda e barra violeta, fundo arroxeado, texto lilás, etiqueta ESPECIAL.

**Slot da mão.** Retângulo com borda tracejada quando vazio e sólida quando preenchido, mais a mesma barra de 3px da família. Custo no canto superior direito. Estado de foco igual ao da carta. O slot ESPECIAL tem borda violeta e é descrito no kit como "slot dedicado das cartas historicas do Gus".

**Chip de família.** Pílula de filtro. Desligada é contorno em ink-dim; ligada é preenchida na cor da própria família, com o texto escurecido para contraste. Existe um chip "todas" que acende em cyan.

**Botão.** Variantes: normal (cyan sobre fundo escuro), foco (contorno e brilho cyan), aviso (âmbar, usada em "Upload ao commons"), fantasma (contorno apagado, usada em "Comerciar"), e o estado desabilitado a 40 por cento. Faixa de dica de tecla no rodapé, no formato `[A] confirmar  [X] deck morto  [U] upload  [B] voltar`.

**Fundo das telas.** Duas camadas: varredura horizontal sutil de linhas cyan a 2,5 por cento, e brilho radial no topo. Moldura de 1px com sombra interna cyan fraca.

## 6. Anatomia das duas telas

**Bancada** (`90-bancada.html`), cabeçalho "GUS.WORLD // tavus-drive :: BANCADA". Barra superior com abas dos seis membros da party (GUS, Caua, Iara, Bento, Linda, Jaci) e contador de créditos em âmbar. Duas colunas:

- **Esquerda, deck ativo:** chips de família, grade de cartas em duas colunas e trilha de rolagem própria para gamepad (setas e indicador).
- **Direita, mão:** dividida em mão comum e slots especiais (só do Gus), mais um gráfico de curva de mana da mão, em barras por custo (1, 2, 3, 4, 5+).
- **Embaixo, deck morto:** em moldura avermelhada, rotulado "one-way, nao volta", com as cartas riscadas, o botão de upload ao commons e uma mensagem de retorno em verde explicando que o código foi para o repositório comum e rendeu créditos.
- **Rodapé:** dicas de tecla e a frase, verbatim: "loadout deterministico, sem sorteio, mao fixa fora do combate".

**Mercado** (`91-mercado.html`), cabeçalho "GUS.WORLD // mercado :: LOJA SETOR-7 REPOS". Faixa de diálogo com retrato de NPC em moldura violeta, balão de fala e menu numerado de três opções (Conversar, Comerciar, Sair). Duas colunas:

- **Vender (âmbar):** alimentada só pelo deck morto, marcada "one-way", com preço positivo por carta.
- **Comprar (verde):** estoque da loja, preço negativo, e as cartas que o jogador já tem esmaecidas com o sufixo "[possui]".
- **Barra de fechamento:** soma venda, compra e saldo final, com os botões de upload do resto e de confirmar.
- **Rodapé:** a regra, verbatim: "NPC do caminho = so a coluna VENDER (ele compra de voce). Loja = compra E vende."

## 7. O que o kit confirma do canon

1. **"a mao e uma SELECAO, nao copia".** Bate com `docs/design/mecanicas/deck-mao-sistema.md` §7, item 2: "A MÃO é uma SELEÇÃO (lista de IDs → deck ativo), não um container", o que torna a duplicação impossível.
2. **Cartas especiais só do Gus, em slot dedicado.** Bate com `deck-mao-sistema.md` (seção de cartas ESPECIAIS: "SÓ do Gus", e a tabela de parâmetros: "Slots especiais do Gus: 1 dedicado"). Ressalva de leitura: o kit rotula a área como "slots especiais" no plural e o canon fixa 1 slot dedicado (número marcado `//PLAYTEST` no canon).
3. **Deck morto one-way, nunca volta.** Bate com `deck-mao-sistema.md` §6.2: "NÃO volta ao deck ativo, nunca".
4. **Bolsa "21/34".** O 34 é o primeiro degrau da capacidade de deck/bolsa do canon (`deck-mao-sistema.md`, tabela de parâmetros: 34, 55, 89, sequência de Fibonacci). O 21 é valor de ocupação de amostra.
5. **"loadout deterministico, sem sorteio".** Bate com `deck-mao-sistema.md` §2: "Sem compra/sorteio em combate. A MÃO é um conjunto FIXO de cartas que você monta (loadout)".

## 8. Divergências em aberto (não resolvidas aqui)

### 8.1 Taxonomia de família

São duas decisões do líder em momentos diferentes, e só ele diz qual vale hoje. O canon tem cinco famílias ratificadas por ele em 24/06/2026, em `docs/art/vfx-combate-familias.md`, seção "Tabela das 5 famílias", cada uma ancorada num membro da party e com cor própria:

| Família canônica | Âncora | Cor |
|---|---|---|
| Elétrico | Cauã | `#22D3EE` |
| Bioquímico | Jaci | `#34D399` |
| Sônico | Linda | `#3B82F6` |
| Cinético | Bento | `#E8A33D` |
| Criptográfico | Iara | `#7C3AED` |

O kit usa outros cinco rótulos: eletrico, termico, grupo, biologico, anomalia. Só um nome coincide (elétrico) e, mesmo nele, a cor difere (`#3ad0e0` contra `#22D3EE`). A tabela canônica não tem família "térmico", "grupo" nem "anomalia".

**Terceira taxonomia no canon, medida:** `docs/design/card-frame-spec.md` (status PROPOSTA, aprovada pelo criador em 09/07/2026) pinta a moldura das cartas por seis DOMÍNIOS (eletromagnetismo, física, matemática, computação, ocultistas, economia), com cores próprias. Ou seja, o canon já convive com dois eixos (família elemental de combate e domínio de moldura), e o kit propõe um terceiro conjunto de rótulos.

**Leitura a confirmar pelo líder, não é conclusão.** No kit, a carta "Broadcast.sh" é marcada como família "grupo" e o efeito dela é "dano em area, todos os alvos". Isso sugere que "grupo" descreve ALVO e não família elemental, e que o kit pode estar misturando dois eixos. Apoio no canon: `docs/design/mecanicas/combat.md` define `TargetShape` com valores single, linha, área-3x3, grupo e self, onde "grupo" é forma de alvo. Fica como leitura, nunca correção; o líder julga.

### 8.2 O cyan e a paleta de interface

A paleta de `docs/art/style-guide.md` §4.1 é de arte de mundo: os usos escritos são céu, concreto, fachadas, midtone, pedra e asfalto, mais neon de holograma (linhas 44 a 58, confirmado). Então o kit não contradiz aquela paleta: as duas governam coisas diferentes, e uma não deve ser usada para "consertar" a outra.

Mas, de novo, são duas decisões dele em momentos diferentes, e a pergunta de coerência é maior do que dois cyans. O canon já tem uma paleta de INTERFACE, em `docs/design/mecanicas/battle-screen.md` §2, "Paleta canonica (variante C, HEX exatos)": fundo `#0c0f1a`, cyan `#22D3EE`, magenta hostil `#E11D74`, latão `#E8A33D`, verde HP `#3FB97A`, erro `#F43F5E`, tinta `#cfe6ee`. O kit usa fundo `#080b11`, cyan `#3ad0e0`, âmbar `#e0a53a`, verde `#5fe08a`, vermelho `#e05f6b`, tinta `#c9d6e2`: nenhum valor coincide. Pergunta ao líder, sem presumir resposta: o cyan `#3ad0e0` e o restante dos tokens do kit substituem a paleta de interface da tela de batalha, convivem com ela (telas de bancada e mercado contra tela de combate), ou são acidente?

## 9. O que no kit não é canon

Nome, efeito e custo de mana das cartas de amostra são ilustração de componente, não catálogo: Descarga.exe, Broadcast.sh, Patch-Vida, Overclock, Ping.util, Nullptr, Firewall, Daemon-Regen, Segfault, Multicast, Kernel-Panic.old, Buffer-Leak, Legacy.dll, Overclock+, e os custos 1, 2, 3, 6, 8. Os números de crédito e de preço nas duas telas também são amostra. O catálogo de cartas vive em `docs/design/mecanicas/cartas/` e em `resources/cards/`.

## 10. Governança

Este documento governa a INTERFACE da bancada e do mercado como o líder a desenhou no Claude Design; `docs/art/style-guide.md` governa a arte de MUNDO. Nenhum valor deste documento é decisão de produto até o líder resolver §8. Arte e pipeline visual são do líder (L-02): este documento registra, não propõe.
