# Requisitos de interface

**Proveniência:** este documento deriva da leitura direta dos 7 arquivos de `docs/design/ui-kit/` (`00-tokens.html` a `91-mercado.html`) e dos 11 mockups HTML de `docs/design/mockups/`, todos recuperados do backup do repositório anterior em 21/08/2026. Ele substitui `PORTING-RCSS.md`, um documento do mesmo diretório escrito em 2026-07-16 por um desenvolvedor do glintfx anterior, framework que foi descartado e reiniciado do zero: qualquer veredito daquele documento sobre o que "roda direto" ou o que um framework "suporta" descreve capacidade de algo que não existe mais, e por isso não foi reaproveitado. O que segue é só o que os mockups exigem, sem opinião sobre qual motor de marcação atende.

## 1. Recursos de layout e estilo em uso real nos mockups

### Layout
- **Flexbox como layout primário**: `display:flex` (97 ocorrências) e `display:inline-flex` (4), com `flex-direction`, `align-items`, `justify-content`, `flex:1`, `flex-wrap` e `gap`. É o mecanismo de disposição dominante em todos os 18 arquivos.
- **Grade bidimensional** (`display:grid`) em 6 arquivos: `10-card.html` (3 colunas fixas de 220px), `91-mercado.html` e `90-bancada.html`/`deck-mao-bancada.html` (2 colunas, uma delas com `grid-auto-rows` fixo), `08-app-icon.html` e `11-terminal-glitch-glyphs.html` (`repeat(auto-fill,minmax(...,1fr))`, galeria responsiva).
- **Posicionamento absoluto e relativo**: 72 usos de `position:absolute` e 20 de `position:relative`, para selos, ícones sobrepostos e âncoras de pseudo-elemento.
- **Unidade de viewport** (`100vh`) para painéis de altura total de tela.
- **`float:right`** em duas linhas (`90-bancada.html`, `deck-mao-bancada.html`), só para alinhar um valor à direita dentro de uma linha de rótulo.

### Estilo visual
- **Variáveis customizadas** (`var(--...)`) em todos os 18 arquivos, incluindo variáveis de família de carta (`--fam`) resolvidas com fallback (`var(--fam,#555)`). É o mecanismo de tema/paleta do kit inteiro.
- **Gradientes**: linear (51 ocorrências), radial (20) e repetido (`repeating-linear-gradient`, 8), usados em fundo de painel, borda de destaque e textura de fundo listrada/scanline.
- **Pseudo-elementos com conteúdo gerado** (`::before`/`::after` com `content:`): barra de cor de 3px na borda esquerda de card/slot/item indicando a família (`--fam`); sufixo textual de estado (`" [na mao]"`, `" [possui]"`); cantos de moldura de terminal (`term-corner`).
- **Sombra de texto** (`text-shadow`), em títulos e texto de destaque em terminal, sempre como halo de cor (glow), nunca como sombra de profundidade.
- **Sombra de caixa** (`box-shadow`), presente em 14 dos 18 arquivos, para elevação de painel e contorno de destaque.
- **Filtro** (`filter:`): `blur()+saturate()` no fundo desfocado atrás de diálogos e telas de menu (efeito de profundidade de campo); `drop-shadow()` em ícone de destaque.
- **Raio de borda**: em pixels (uso comum), em `50%` (9 ocorrências, sempre em elemento quadrado de tamanho fixo pequeno como selo de custo de mana ou marcador circular) e em `22%` (2 ocorrências, cantos arredondados do ícone do aplicativo).
- **Transformação**: `translate`/`translateX`/`translateY` para centralização de elemento sobre âncora absoluta, e `text-transform:uppercase` (38 ocorrências) como padrão tipográfico de rótulo.
- **Recorte de forma** (`clip-path:polygon(...)`), em 4 arquivos, para painel hexagonal e painel com cantos cortados (estética diegética).
- **Animação por quadros-chave** (`@keyframes`) com `animation:`, em 5 arquivos: pulso (`pulse`), efeito de máquina de escrever com função de passos (`typing ... steps(34,end)`), cursor piscando (`blink ... step-end infinite`) e desvanecimento de palavra (`fadeword`).
- **Modo de mesclagem** (`mix-blend-mode:overlay`), uma ocorrência, na textura de scanline sobre o texto de diálogo.
- **Espaçamento entre letras** (`letter-spacing`) e **opacidade** (`opacity`), de uso disseminado nos 18 arquivos.
- **SVG vetorial embutido inline**, em 4 mockups, para ícone de navegação (play, engrenagem, seta), moldura hexagonal e colchete de canto de terminal.

## 2. Onde os mockups exigem recurso que costuma faltar em motor de marcação enxuto

Para cada item: a exigência que o mockup faz, e a alternativa equivalente que o próprio mockup poderia usar para chegar ao mesmo resultado visual sem esse recurso específico. Nenhuma afirmação abaixo diz se algum framework atende ou não atende; isso não é deste documento.

### 2.1 Grade bidimensional (CSS Grid)
**Exigência:** três layouts usam grade de verdade: painel de 2 colunas fixas (`90-bancada.html`, `91-mercado.html`, `deck-mao-bancada.html`), grade de cartas de largura fixa repetida (`10-card.html`, 3 colunas de 220px) e galeria responsiva de largura mínima (`08-app-icon.html`, `11-terminal-glitch-glyphs.html`, `repeat(auto-fill,minmax(...))`).
**Alternativa equivalente:** como o número de colunas é sempre fixo ou de largura mínima conhecida (nenhum uso pede posicionamento arbitrário de item em linha/coluna específica), o mesmo resultado se obtém com flexbox e `flex-wrap`, atribuindo a cada item uma largura ou `flex-basis` explícita equivalente à coluna da grade.

### 2.2 Pseudo-elemento com conteúdo gerado (`::before`/`::after` com `content`)
**Exigência:** a barra de cor de família na borda de card/slot/item, o sufixo de estado (`" [na mao]"`, `" [possui]"`) e os quatro cantos de moldura de terminal são todos gerados por pseudo-elemento, sem elemento próprio no HTML.
**Alternativa equivalente:** cada um vira um elemento real na marcação (`<div class="fambar">`, `<span class="sufixo">`, quatro `<div class="canto">`), posicionado do mesmo jeito por CSS absoluto; o resultado visual é idêntico, só a marcação fica mais explícita.

### 2.3 Gradiente repetido (`repeating-linear-gradient`)
**Exigência:** textura de scanline sutil sobre painel de diálogo, e fundo listrado diegético atrás de menus e telas de configuração de controle.
**Alternativa equivalente:** como a área coberta tem tamanho conhecido em cada tela, o mesmo padrão se obtém com uma imagem de textura pré-gerada e ladrilhada (tile), ou com uma lista explícita e finita de paradas de cor num gradiente linear único cobrindo a mesma extensão.

### 2.4 Sombra de texto (`text-shadow`)
**Exigência:** halo de cor (glow) ciano ao redor de título e texto de destaque em terminal, em 2 arquivos.
**Alternativa equivalente:** um filtro de sombra projetada (`drop-shadow`) aplicado ao elemento de texto produz o mesmo halo; alternativa mais indireta é uma camada de texto duplicada, borrada e posicionada atrás do texto original.

### 2.5 Raio de borda em porcentagem (`border-radius: 50%`, `22%`)
**Exigência:** selo circular de custo de mana (elemento quadrado de 19 a 21px), marcador circular de posição e cantos arredondados do ícone do aplicativo.
**Alternativa equivalente:** como todo uso é sobre elemento de tamanho fixo já conhecido no CSS, o raio em porcentagem é substituível por um raio em pixels igual à metade da largura/altura do elemento (equivalente funcional exata enquanto o elemento não mudar de tamanho em tempo real).

### 2.6 Recorte de forma (`clip-path: polygon(...)`)
**Exigência:** painel hexagonal decorativo e painel com cantos diagonalmente cortados, usados na proposta de menu lateral diegético e nas telas de remapeamento de controle e save/load.
**Alternativa equivalente:** como as formas são estáticas (não calculadas em tempo real a partir de conteúdo variável), uma imagem de fundo pré-recortada com cantos transparentes, ou uma borda em nove fatias (nine-slice) já desenhada com os cantos cortados, produz o mesmo contorno.

### 2.7 Modo de mesclagem (`mix-blend-mode: overlay`)
**Exigência:** uma única ocorrência, a textura de scanline sobre o texto de diálogo combinada em modo de mesclagem "overlay" para o efeito de tela CRT.
**Alternativa equivalente:** um asset de textura semitransparente já pré-composto com o efeito de mesclagem embutido na própria imagem, aplicado como sobreposição simples, sem depender de cálculo de mesclagem em tempo real.

### 2.8 Animação por quadros-chave com função de passos (`@keyframes` + `steps()`)
**Exigência:** efeito de máquina de escrever revelando o texto de diálogo caractere a caractere (`steps(34,end)`) e cursor piscando (`step-end infinite`), em 2 mockups de diálogo.
**Alternativa equivalente:** como a revelação já é inerentemente sequencial e ligada ao ritmo da fala, o mesmo efeito se obtém por lógica de aplicação avançando um índice de substring a cada intervalo de tempo, sem depender de animação declarativa por CSS.

### 2.9 SVG vetorial embutido inline
**Exigência:** ícones vetoriais pequenos (triângulo de play, engrenagem, seta de voltar, moldura hexagonal, colchete de canto), nítidos em mais de um tamanho de exibição.
**Alternativa equivalente:** um sprite raster pré-renderizado no tamanho final necessário para cada uso; coerente, inclusive, com o registro pixel-art 2D do jogo (`GODS_LAWS.md`, L-02), que já não pede nitidez vetorial em tempo real.
