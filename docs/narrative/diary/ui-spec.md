# Diário do Gus — UI/UX Spec

> **Status:** Canônico (Bloco H — Diário UI/UX). Revisão inicial 2026-05-16.
>
> **Escopo:** spec de interface, interação, wireframes textuais. **Não** trata conteúdo das entries (ver `entries-*.md`).
>
> **Visual canônico:** híbrido caderno diegético (layer 1: frame físico) + texto limpo legível (layer 2: conteúdo).
>
> **Knowledge:** score numérico **visível em HUD principal** (separado do Diário). Diário é manifestação narrativa do mesmo dado.
>
> **Cross-refs:** [[_INDEX]] · [[pillars]] §Pillar 4 (tom 11 anos) · [[in-world-docs]] · [[foreshadow-links]] · `art/style-guide.md`.

---

## 1. Conceito visual — caderno diegético (layer 1)

O Diário é um caderno físico tangível pertencente a Gus, 11 anos. Toda a interface se apresenta como folhear esse caderno: o player não abre "menu de wiki"; ele vê **a mão do Gus virando uma capa rabiscada**.

### 1.1 Capa

- **Material:** papelão escolar barato, marrom-mostarda, manchado.
- **Decoração canônica:**
  - Etiqueta escolar Educacional Padrão (formato retangular branco, canto superior esquerdo). Campo "Nome:" preenchido com letra cursiva de Gus: "**Gus V. T. Vance**". Campo "Turma:" riscado três vezes (Gus passou de série e nunca atualizou).
  - Adesivo pequeno de **Pilha Sobrecarregada** (referência [[tradicoes-cultura]] §Dia do Tomo — pequeno logo estilizado de livro aberto com símbolo de pilha, ~3cm canto inferior direito).
  - Adesivo de **fractal Fibonacci** desenhado a caneta (não impresso; Gus copiou de fragmento Selve aos 10, ver `characters/gus.md` §Memórias formativas — primeira incursão).
  - **Dobra de canto** superior direito, vincada de uso.
  - **Mancha** circular escura inferior-centro: café-de-neurônio derramado pelo pai Pyotor (canônico, `lore-bible` §11.4 + `characters/gus.md` §pai itinerante). Mancha funciona como **leitmotiv visual**: o pai está presente mesmo ausente.
  - **Fita adesiva amarelada** consertando lombada (caderno foi rasgado uma vez; Gus consertou ele mesmo, com cuidado técnico).
- **Tipografia da etiqueta:** cursiva infantil 11 anos. Letras irregulares mas legíveis. "i" às vezes sem pingo. Tamanho consistente (Gus é organizado).
- **Paleta layer 1:** papel envelhecido `#E8DDC4` (base), tinta azul-marinho `#1B2238` (cursiva — mesmo BG mid da cidade, link visual sutil), acentos vermelho-correção `#C03030` (rabiscos de revisão), laranja Gus `#FF6B1A` em sticky notes-âncora (laranja é cor canônica exclusiva do Gus, ver `style-guide.md` §4.3).

### 1.2 Animação de abertura

- Duração: **2-3 segundos** (não pula se for primeira abertura; pulável em re-aberturas).
- Frames-chave: (1) caderno fechado no centro da tela; (2) mão direita do Gus aparece pelo canto inferior direito e abre a capa; (3) páginas folheiam até a última aba aberta (memória da última posição); (4) settle em interface idle.
- Áudio diegético: papelão dobrando + páginas viradas (foley capturado, não trilha sonora — ver [[_INDEX]] §Princípio).
- Frame zero antes da animação: blur leve do mundo de fundo (gameplay pausada visualmente; sem freeze rígido — pausa diegética).

### 1.3 Elementos persistentes ao redor do caderno

Quando aberto, o caderno ocupa ~80% da tela. Os 20% periféricos mostram:

- **Borda esquerda:** abas físicas verticais (4 abas), cores diferentes, posições fixas, dobras visíveis indicando uso.
- **Borda direita:** lombada, sticky notes coloridos pendurados (saem pra fora do caderno).
- **Topo:** tira de papel com o título "**DIÁRIO DO GUS**" escrito a mão por Gus, levemente torto.
- **Rodapé:** rodapé funcional com hint de input (texto cursivo curto) + número de página relativo à aba.

---

## 2. Layout macro — 4 abas

O caderno tem **4 abas físicas** correspondendo aos 4 tipos canônicos de entry. Cada aba tem cor própria (compatível com daltonismo: além de cor, posição vertical fixa e ícone-glyph desenhado a mão).

| Aba | Cor base | Ícone (desenho cursiva) | Tipo | Posição vertical |
|---|---|---|---|---|
| 1 | Azul-marinho `#1B2238` | Lápis riscando linha | Manuscrito + Glossário | Topo |
| 2 | Marrom-pergaminho `#7A5A3A` | Pergaminho enrolado | Docs Descobríveis | Alto-meio |
| 3 | Verde-folha `#2E5447` | Silhueta meia-cara | Fichas + Bestiary | Baixo-meio |
| 4 | Amarelo-mapa `#D4A847` | Bússola estilizada | Mapas + Timeline | Base |

**Sticky notes** (post-it artesanais) saem por fora da borda direita:

- **Laranja `#FF6B1A`** = atalho para entry foreshadow chave (sistema [[foreshadow-links]] destrava conforme Knowledge sobe).
- **Verde `#34D399`** = entry com nova atualização desde última leitura.
- **Vermelho `#C03030`** = entry crítica para arco/ending (Patch-Zero, Sterling, reveal Dante).
- **Cyan `#22D3EE`** = cross-ref entre entries (visualiza linha de costura).

Indicador de "novo conteúdo não-lido": **círculo laranja vibrante** no canto da aba + número (ex: "[3 NEW]"). Pillar 4 + acessibilidade: cor + número + forma (round badge), não só cor.

### 2.1 Aba 1: Manuscrito + Glossário

Wireframe ASCII (paisagem, 16:9):

```
+-- DIÁRIO DO GUS ----------------------------------- pag 14/47 --+
| (=) [Manuscrito*3] [Docs 1] [Fichas]  [Mapas]                  |
+-----------------+----------------------------------------------+
| MANUSCRITO      | "Dia 14 do mês do Compilador.                |
| ---             |                                              |
| > Cena Beat Ki  |  Hoje vi a fonte da Praça Central girar      |
|   [NEW]         |  exato no padrão Fibonacci. 1, 1, 2, 3, 5    |
|   (laranja)     |  oito borrifos por ciclo. Aí parou. Voltou.  |
|                 |                                              |
| > Cena Beat Sho |  Eu fiquei dois minutos olhando. A mãe       |
|   (lido)        |  perguntou se eu tinha ido no banheiro       |
|                 |  porque eu tava com cara de quem viu coisa.  |
| > Cena Beat Ten |                                              |
|   (lido)        |  Eu vi coisa. Não da que ela acha."          |
|                 |                                              |
| ---             |  [esboço cursiva: fonte com setas marcando   |
| GLOSSÁRIO       |   a sequência. Números 1,1,2,3,5 escritos.   |
| > C-Arcane      |   Pergunta no canto: "natureza compila?"]    |
| > Asmódico      |                                              |
| > Óxido         |   ___________________________________________|
| > Pythia        |   [Sticky-note laranja anexa]                 |
| > Tavus-Drive   |   "lembrar disso. Bento talvez saiba."        |
| > Matriz Ort.   |                                              |
+-----------------+----------------------------------------------+
| Knowledge desta categoria: 7/20      [foreshadow: 2 ativos]    |
+----------------------------------------------------------------+
| [J] fechar    [Tab] mudar aba    [↑↓] navegar    [Enter] abrir |
+----------------------------------------------------------------+
```

Layer 1 visual: a página interna tem linhas-horizontais sutis de caderno pautado (~12% opacidade). A cursiva flutua sobre as linhas. Esboço (frame da fonte) ocupa quadrante. Sticky note anexa por fita.

### 2.2 Aba 2: Docs Descobríveis

```
+-- DIÁRIO DO GUS ----------------------------------- pag 22/47 --+
| (=) [Manuscrito] [Docs*1] [Fichas] [Mapas]                     |
+-----------------+----------------------------------------------+
| DOCS COLETADOS  | Doc 05 — Auditoria Apex-Data                 |
| 8 de 15         |                                              |
|                 |  [imagem do documento físico]                |
| > 01 Tratado    |  papel A4 manchado, timbre Apex-Data,        |
|   Supremacia    |  carimbo "Restrito" rasgado pela metade      |
|   (Sterling)    |                                              |
|                 |  Linda me deu. Disse "presente do Padrinho   |
| > 02 Carta In.  |  Tiago, ele guardava há 16 anos".            |
|   (Berenger)    |                                              |
|                 |  Eu li tres vezes pra entender. Sterling     |
| > 03 Atelaiá    |  comprou a Apex pra DESTRUIR. Não pra        |
|   (Catedrais)   |  ter. Pra FECHAR e levar o miolo embora.     |
|                 |                                              |
| > 04 Mirage     |  [esboço: três caixas conectadas por seta:   |
|   (Adila)       |   "Apex" → "fechada" → "Sterling"]           |
|                 |                                              |
| > 05 Apex-Data  |  Eu não sabia que dava pra fazer isso com    |
|   [NEW]         |  uma empresa. Eu achava que empresa fechava  |
|                 |  por culpa dela. Não por culpa de fora."     |
| > 06 Padrinho T.|                                              |
| > 07 Selve      |  [Sticky-note vermelho]                      |
| > 14 Penedo-L.  |  "Mãe trabalhou Apex. Eu não sabia disso     |
|                 |   antes."                                    |
| > 11 ?? GATE OU |                                              |
| > 13 ?? GATE OU |                                              |
| > 15 ?? GATE OU |                                              |
+-----------------+----------------------------------------------+
| Docs coletados: 8/15  ·  3 com gate Ouro pendentes             |
+----------------------------------------------------------------+
| [J] fechar    [Tab] mudar aba    [↑↓] navegar    [V] ver doc   |
+----------------------------------------------------------------+
```

Entries com gate Ouro **aparecem na lista mas com "??"** e cadeado (sticky-note cinza). Player sabe que existem; não sabe o conteúdo. Pillar 1: scarcity de informação alimenta motivação de farming Knowledge.

### 2.3 Aba 3: Fichas + Bestiary

```
+-- DIÁRIO DO GUS ----------------------------------- pag 33/47 --+
| (=) [Manuscrito] [Docs] [Fichas*2] [Mapas]                     |
+-----------------+----------------------------------------------+
| PARTY           | Cauã Berenger — "Volt"                       |
| > Gus (auto)    |                                              |
| > Cauã [NEW]    |  [retrato esboço cursiva: cabeça meia-perfil |
| > Iara          |   com cicatriz fina sobre sobrancelha;       |
| > Bento         |   campo "olhos" preenchido: castanho-Pythia] |
| > Linda         |                                              |
| > Dante         |  Idade: 13                                   |
| > Jaci          |  Setting: Dutos Infernais                    |
|                 |  Linguagem: Pythia                           |
| ANTAGONISTAS    |  Família: 5 (família dele) [esboço esboço]  |
| > Sterling      |                                              |
|   (parcial)     |  "Cauã não fala do irmão. Eu sei do irmão.  |
| > Patch-Zero    |   Ele não sabe que eu sei. Eu não vou       |
|   ??? glitch    |   contar até ele querer contar."             |
| > Adila Murm.   |                                              |
| > Cassiano V.   |  [Sticky-note verde — atualizado]            |
|                 |  "Cauã me chamou de 'mano' hoje. Primeira   |
| BESTIARY        |   vez. Eu anotei aqui pra não esquecer."     |
| > Drone Sterli  |                                              |
|   (pag 4/4 ★)   |                                              |
| > Verme-Vírus   |  [linha foreshadow: ler doc 02 + doc 13]    |
|   (pag 2/4)     |  [linha foreshadow: ler doc 09]              |
| > Esporo Inst.  |                                              |
|   (pag 1/4)     |                                              |
+-----------------+----------------------------------------------+
| Knowledge desta categoria: 14/40    [foreshadow: 4 ativos]     |
+----------------------------------------------------------------+
| [J] fechar    [Tab] mudar aba    [↑↓] navegar    [Enter] abrir |
+----------------------------------------------------------------+
```

Bestiary mostra estrelas preenchidas (`★`) conforme páginas se completam. Inimigo mestre (4 págs) com todas as estrelas = Knowledge 100% para ele = RNG zerado contra ele (Pillar 1, Knowledge Progression).

### 2.4 Aba 4: Mapas + Timeline

```
+-- DIÁRIO DO GUS ----------------------------------- pag 41/47 --+
| (=) [Manuscrito] [Docs] [Fichas] [Mapas*1]                     |
+-----------------+----------------------------------------------+
| MAPAS (8)       | NÚCLEO METROPOLITANO — explorado 67%         |
| > Núcleo Met.   |                                              |
|   67% [NEW]     |  [grid esboço cursiva, ruas em ângulo reto.  |
| > Periferia I.  |   Áreas exploradas em tinta cheia; áreas     |
|   45%           |   não-exploradas em hachura leve. Sticky     |
| > Setor Mirage  |   marrom: "voltar aqui c/ Bento - portão"]   |
|   12%           |                                              |
| > Zona Silênc.  |  Pontos de interesse marcados:               |
|   8%            |   1. Casa Vance (seta com coração rabiscado) |
| > Anel Verde    |   2. Escola pública (gus rabiscou "ufa")     |
|   0%            |   3. Pilha Sobrecarregada (estrela ★)        |
| > Dutos Infer.  |   4. Praça Central (fonte Fibonacci 🌀)      |
|   31%           |   5. Cúpula Sterling (X vermelho ENORME)     |
| > Catedrais Sy. |                                              |
|   3%            |  [Sticky-note cyan — cross-ref doc 01]       |
| > Selve Sombria |                                              |
|   0%            |                                              |
|                 |                                              |
| TIMELINE        |                                              |
| > Era 1         |                                              |
| > Era 2         |                                              |
| > Era 3 [NEW]   |                                              |
|   12 eventos    |                                              |
+-----------------+----------------------------------------------+
| Mapas: 166/800 tiles  ·  Timeline: 14/50+ eventos              |
+----------------------------------------------------------------+
| [J] fechar  [Tab] mudar aba  [↑↓] navegar  [M] alternar map/tl |
+----------------------------------------------------------------+
```

Timeline expande por era: clicar em "Era 3 [NEW]" abre lista cronológica filtrada (15-20 itens com data, evento, link para entry-manuscrito se Gus testemunhou).

---

## 3. Layer 2 (texto limpo do conteúdo)

Dentro da página visível (área de detalhe direita), o conteúdo da entry **renderiza em fonte clara legível**, não em cursiva pura. Razões:

1. **Imersão visual** preservada (frame ainda é caderno).
2. **Acessibilidade de leitura** (cursiva infantil é difícil para dislexia, idiomas localizados, fonte pequena).
3. **Escalabilidade** (cursiva renderizada como SDF para 30+ entries longas é caro em perf).

A cursiva da layer 1 aparece em:

- **Cabeçalho da entry** ("Dia 14 do mês do Compilador.")
- **Margens** (anotações curtas, máx 5 palavras: "lembrar disso", "Bento talvez sabe")
- **Sticky notes anexos** (texto curto, todo em cursiva)
- **Esboços** (descrições visuais no estilo de garatuja técnica)

Corpo da entry: **fonte sans-serif clara (ex: Inter 11pt)**, espaçamento generoso, alinhamento à esquerda (não justificado — justificado quebra leitura cursiva). Pillar 4 + acessibilidade.

### 3.1 Tipografia detalhada

| Elemento | Fonte | Tamanho | Cor | Notas |
|---|---|---|---|---|
| Cabeçalho entry (cursiva) | Caligráfica custom 11 anos | 18pt | `#1B2238` | SDF, kerning irregular planejado |
| Corpo entry (limpo) | Inter ou similar sans-serif | 11pt (ajustável 8-16pt) | `#1B2238` sobre `#E8DDC4` | WCAG AA mínimo |
| Margens (cursiva) | Caligráfica custom 11 anos | 9pt | `#1B2238` ou `#C03030` | Itálico curto |
| Sticky notes (cursiva) | Caligráfica custom 11 anos | 10pt | `#1B2238` ou `#C03030` | Sobre fundo colorido do post-it |
| Aba (cursiva) | Caligráfica custom 11 anos | 14pt | branco/papel | Levemente torto |
| Hint de input | Inter mono | 9pt | `#1B2238` 70% | Sempre presente no rodapé |

---

## 4. Interação

### 4.1 Abertura/fechamento

- **PC keyboard:** tecla **`J`** (de Diário; mnemonic) — abrir/fechar.
- **Gamepad:** botão **MENU** (Start) → seleciona "Diário" no menu de pausa OU atalho direto via **D-pad up** (configurável).
- **Touch:** não suportado em G1 (decisão Fase 1).
- **Mouse:** clique no ícone do caderno no canto do HUD principal (sempre visível, discreto).

### 4.2 Navegação

| Input PC | Input gamepad | Ação |
|---|---|---|
| `Tab` | RB / R1 | Próxima aba |
| `Shift+Tab` | LB / L1 | Aba anterior |
| `↑` `↓` | D-pad vertical / stick L | Navegar lista de entries |
| `Enter` | A / X | Abrir entry selecionada |
| `Backspace` / `Esc` | B / O | Voltar para lista |
| `J` | START / Menu | Fechar caderno |
| `M` (na aba 4) | Y / Triangle | Alternar entre Mapa e Timeline dentro da aba 4 |
| `V` (na aba 2) | Y / Triangle | Ver doc físico fullscreen |
| `F` (em entry com foreshadow ativo) | Y / Triangle | Saltar para entry conectada (cross-link Bloco I) |

### 4.3 Sticky notes coloridos como atalhos

Sticky notes pendurados na borda direita são **clicáveis/selecionáveis**:

- **Hover/foco:** sticky se destaca + popup curto mostra entry-alvo.
- **Click/A:** salta direto para entry referenciada, sem passar pela lista.
- **Função narrativa:** o jogo (sistema [[foreshadow-links]]) **acrescenta sticky notes automaticamente** quando Knowledge atinge threshold. Player sente que o próprio Gus está "puxando uma página que conecta com outra".
- **Limite visual:** máximo 5 sticky notes visíveis por vez. Excedente vira "leque" expansível.

---

## 5. Triggers de atualização (popup HUD)

Quando nova entry chega no Diário durante gameplay, **popup discreto** aparece no canto superior direito da tela (HUD principal, fora do Diário):

```
+--- HUD popup (canto superior direito) -----+
|  [ícone caderno]  Nova página no Diário    |
|                   "Manuscrito atualizado"  |
+--------------------------------------------+
```

- **Duração:** 4 segundos. Fade-in 0.5s, idle 3s, fade-out 0.5s.
- **Cor da borda:** depende do tipo de entry:
  - Azul-marinho `#1B2238` = Manuscrito
  - Marrom-pergaminho `#7A5A3A` = Doc descobrível
  - Verde-folha `#2E5447` = Ficha/Bestiary
  - Amarelo-mapa `#D4A847` = Mapa/Timeline
- **Som diegético:** virar página de papel curto (~0.6s, foley capturado).
- **Player não-disrupted:** popup é cosmético; gameplay continua. Player pode abrir Diário depois pela tecla J.
- **Acessibilidade:** popup também tem ícone forma + texto, não só cor (compatível daltonismo). Som tem alternativa visual (vibração leve no controle se ativada).

### 5.1 Caso especial — invasão Patch-Zero (Canal 1, ver [[lore-bible]] §8.3)

Quando Patch-Zero adiciona entry sozinho:

- **Popup tem cor anômala** vermelha `#F43F5E` + glitch tipográfico ("Nov4 p4gin4...").
- **Som diegético:** página + estática curta de rádio (~1.2s).
- **Letra dentro da entry:** **NÃO é cursiva do Gus**. É tipográfica corrompida (monospace glitching). Player percebe imediatamente que Gus não escreveu aquilo.
- **Cabeçalho ao abrir:** `[ENTRY 0x????]` em vez de "Dia X do mês Y".
- **Frequência:** 1-3 invasões por arco companion (total ~10 entries no jogo). Cada uma é evento, não ambient.

---

## 6. Knowledge HUD (separado do Diário)

Knowledge é dado **técnico** mostrado no HUD principal, **não dentro do Diário**.

```
+-- HUD canto inferior direito ----+
|  Knowledge                       |
|  Total: 47%        ★★★★☆         |
|  Bestiary: 32%                   |
|  Lore: 58%                       |
|  Companions: 71%                 |
|  Mundo: 41%                      |
+----------------------------------+
```

- **Sempre visível** (opção de minimizar para ícone-só).
- **Mostra percentual exato** por categoria + total ponderado.
- **Estrelas** ★ representam tier de gating (Bronze/Prata/Ouro → 0/3/5 estrelas). Ver [[knowledge-gates]].
- **Diário NÃO repete esses números.** Diário mostra **estado narrativo** (entry parcial/completa/cross-linked). Player consulta HUD para "quanto preciso ainda"; consulta Diário para "o quê descobri".

Justificativa de separação: Pillar 1 quer Knowledge Progression como **anti-grind real**, então o número precisa ser legível e perseguível (HUD). Pillar 4 + diegese narrativa querem o Diário como caderno de criança, e criança de 11 anos prodígio **não rotula a si própria com porcentagem**. Os dois sistemas coexistem sem se anular.

---

## 7. Estados especiais de entry

### 7.1 Entry parcial (Knowledge baixa)

- **Texto borrado** em ~40% das palavras (efeito visual: censura analógica feita a caneta + manchas de tinta).
- **Esboços incompletos:** silhueta com "?" no centro, pontos de interrogação onde Gus não confirmou ainda.
- **Campos vazios:** ficha de inimigo com "ataques: ???", "fraqueza: ???".
- **Sem cross-links** (sticky notes ausentes).

### 7.2 Entry completa (Knowledge alta)

- **Texto cristalino**, sem borrões.
- **Esboços finalizados:** trace cuidadoso, anotações marginais densas.
- **Campos preenchidos:** todas as estatísticas, fraquezas, padrões, atalhos.
- **Cross-links destravados:** 1-4 sticky notes apontando para entries conectadas.

### 7.3 Entry de foreshadow (cross-link Bloco I ativo)

Quando Knowledge atinge gate específico, entry-setup ganha **sticky note adicional** apontando para entry-payoff:

```
+------------------- página entry-setup -----------+
|  "Dia 4. O Dante quis ver meu Tavus-Drive hoje. |
|   Eu mostrei. Ele perguntou se podia ajustar    |
|   o firmware. Eu disse não."                     |
|                                                  |
|  [esboço: pulso esquerdo com Tavus em destaque]  |
|                                                  |
|  ___________________________________________     |
|  [Sticky-note laranja, ADICIONADO depois]        |
|  >> doc 08 (memorando Vorto)                     |
|     "olha o que ele tinha que fazer"             |
+--------------------------------------------------+
```

Sticky note aparece **só quando jogador atinge Knowledge suficiente** OU **descobre doc 08** (cross-trigger). Antes disso, entry-setup parece banal. Depois, vira retroativamente sinistra.

Lista completa de plants/payoffs em [[foreshadow-links]] (130 plants canônicos Bloco I).

---

## 8. Acessibilidade

- **Fonte de corpo ajustável:** 8pt mínimo (texto compacto) → 16pt máximo (alta legibilidade). Slider em opções gerais.
- **Cursiva substituível:** opção "**Bloco-letra para anotações**" em opções → toda layer 1 (cabeçalhos, margens, sticky notes) renderiza em sans-serif limpo. Reduz imersão diegética mas garante leitura para dislexia/baixa visão. Setting on por padrão se localização for idioma com leitura RTL ou caractere extenso (mandarim, japonês).
- **Contraste WCAG AA mínimo:** texto sobre papel = 7.1:1 (atende AAA). Sticky notes coloridos têm contraste validado individualmente.
- **Daltonismo:** sticky notes não dependem só de cor. Cada cor tem forma distinta:
  - Laranja: post-it retangular padrão (forma 1)
  - Verde: post-it com canto dobrado (forma 2)
  - Vermelho: post-it com fita adesiva atravessada (forma 3)
  - Cyan: post-it triangular (forma 4)
- **Voice-over por entry:** opcional, ativado em opções. Lê em voz neutra (não dramatizada) o conteúdo da entry. Gus aos 11 não é dublado; voice-over é UI assistiva, não personagem. Suporte dependente de localização (G1: pt-br + en).
- **Navegação keyboard-only completa:** zero ação exige mouse. Hint de input sempre visível no rodapé.
- **Indicador de "página nova" multimodal:** cor + número + ícone + (opcional) som de notificação. Pillar 4 já exige multimodalidade (sangue + postura + ícone + barra HP no combate); Diário herda mesmo princípio.
- **Pausa de gameplay ao abrir:** combate turn-based pausa naturalmente fora do turno; em exploração, abrir Diário pausa o mundo (sem inimigos atacando enquanto lê). Reduz ansiedade temporal.

---

## 9. Performance e save

### 9.1 Render

- **Caderno (layer 1):** sprite atlas único de ~2048² contendo capa, abas, sticky notes (todas as cores), bordas, fita adesiva, manchas. Tudo pré-renderizado em Krita/Aseprite, exportado uma vez. Custo runtime: 1 draw call para frame.
- **Conteúdo (layer 2):** text rendering on-demand via Godot `Label` / `RichTextLabel`. Cache de strings recentes. Nunca pre-renderiza todas as entries simultaneamente.
- **Esboços:** sprites pequenos pré-renderizados, atlas separado por aba (4 atlas no total). Carregados sob demanda.
- **Animação de abertura:** sequência de 4-6 sprites do papelão dobrando. ~120 KB total.
- **Custo total estimado de Diário aberto:** ~5-10 draw calls + 1-2 MB VRAM. Negligível frente ao gameplay.

### 9.2 Save (JSON versionado `save_version: 1`)

Salva **apenas state mínimo**:

```json
{
  "diary": {
    "entries_unlocked": ["m_beat_ki", "m_beat_sho", "d_01", "d_02", "f_caua_lv2", "b_drone_sterling_lv4", ...],
    "entries_read": ["m_beat_ki", "d_01", ...],
    "stickynotes_active": {
      "m_dante_drive_4": ["d_08"],
      "m_fonte_fibonacci": ["b_bento_lv2"]
    },
    "current_tab": 1,
    "last_entry_open": "d_05",
    "knowledge_score": {"bestiary": 32, "lore": 58, "companions": 71, "mundo": 41, "total": 47}
  }
}
```

- **IDs canônicos** para cada entry (esquema `<tipo>_<slug>[_<lvl>]`): `m_*` manuscrito, `d_*` doc descobrível, `f_*` ficha, `b_*` bestiary, `mp_*` mapa, `t_*` timeline. Definidos individualmente nos `entries-*.md`.
- **Migrators desde D1** (alinhado com decisão Fase 1, CLAUDE.md raiz). Se schema muda, migrator converte.
- **Tamanho de save** estimado: ~3-8 KB para Diário cheio. Texto das entries vive nos `entries-*.md` (data, não save) — save só armazena qual ID está destravado/lido.

### 9.3 Loading

- Diário não pré-carrega conteúdo na boot. Lazy load por aba ao abrir pela primeira vez na sessão.
- Cache em memória durante sessão (entries já vistas ficam quentes).
- Reset de cache ao fechar caderno (recupera ~1-2 MB se RAM apertada).

---

## 10. Plataforma

| Plataforma | Input | Notas |
|---|---|---|
| **PC Linux (AppImage / .tar.gz)** | Keyboard + mouse + gamepad (Xbox/PS4/PS5 nativo via Godot) | Decisão Fase 1 canônica. |
| **PC Windows (sem signing G1)** | Idem PC Linux | Decisão Fase 1 canônica. |
| **Touch / mobile** | Não suportado em G1. | Fora de scope. |
| **VR** | Não suportado. | Fora de scope. |

Tecla padrão PC: **`J`**. Remapeable em opções (Pillar acessibilidade — `accessibility-specialist` requirement).

Gamepad padrão: **MENU + D-pad up** para abrir; **B/O** para fechar. Remapeable.

Resolução suportada: 1280×720 mínimo (caderno renderiza ok em 720p), 1920×1080 alvo, 2560×1440 e superiores escalam via UI anchors. 4:3, 16:9, 21:9 suportados (caderno mantém aspect 4:3 interno; letterbox preto orgânico nas bordas em 21:9, simulando "moldura de mesa").

---

## 11. Casos edge / regras de exceção

- **Diário durante combate:** abrir Diário **fora do turno do Gus** é instantâneo (combate pausa). **Durante o turno do Gus**: bloqueado (player não fica abrindo wiki para vencer combate; isso vai contra Pillar 1 — Knowledge precisa ser farmado antes, não consultado no meio da luta). Tooltip explica: "Você consultará o Diário melhor depois deste turno."
- **Diário durante diálogo:** bloqueado. Diálogo é prioridade narrativa; Diário disponível antes ou depois.
- **Diário durante cutscene:** bloqueado.
- **Diário durante combate boss (turno inimigo)\:** disponível (player ansioso pode ler bestiary do boss enquanto inimigo joga; permite Knowledge farming reativo).
- **Diário no menu principal (antes de game iniciar):** indisponível (faz parte do save, não meta-jogo).
- **Diário em new game+:** persiste IDs Bronze (sem reset) MAS reseta entries Prata/Ouro (incentiva re-descobrir gates). Decisão preliminar; revalidar pós-vertical-slice.
- **Diário pós-final:** sempre acessível. Player pode revisitar tudo. Knowledge total congelada no valor final atingido.

---

## 12. Cross-refs

- **[[_INDEX]]** — princípio do Diário + decisões macro Bloco H.
- **[[pillars]] §Pillar 1** — Knowledge Progression (anti-grind real, sources/decay).
- **[[pillars]] §Pillar 4** — tom de 11 anos (cursiva, voice, multimodalidade).
- **[[in-world-docs]]** — 15 documentos descobríveis que populam aba 2.
- **[[foreshadow-links]]** — 130 plants Bloco I, sticky-note conectivo.
- **[[entries-manuscrito-glossario]]** — conteúdo aba 1.
- **[[entries-docs-descobriveis]]** — conteúdo aba 2.
- **[[entries-fichas-bestiary]]** — conteúdo aba 3.
- **[[entries-mapas-timeline]]** — conteúdo aba 4.
- **[[knowledge-gates]]** — tiers Bronze/Prata/Ouro + decisão de unlock.
- **`art/style-guide.md`** — paleta canônica (papel envelhecido `#E8DDC4`, tinta `#1B2238`, laranja Gus `#FF6B1A` em sticky notes-âncora).
- **`characters/gus.md`** — voice/cursiva do Gus aos 11 anos.

---

**Última revisão:** 2026-05-16. Canônico Bloco H — UI/UX Spec do Diário. Atualizações exigem aprovação do criador supremo.
