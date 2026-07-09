# Spec da moldura das cartas (Codex de Conjuros)

> **STATUS: PROPOSTA (design aprovado pelo criador 2026-07-09; produção de arte pendente).** Fecha a DIREÇÃO da moldura das cartas do roster de análogos (ver `brainstorm-backlog.md` + `roster-analogos/`). A moldura é a MESMA estrutura pra todas as cartas; muda só a cor por domínio + o tratamento lendário do Tusk.

## Direção aprovada

- **Conceito: Híbrido Vetorial** (mock `mockups/16-moldura-cartas.html`, conceito 3): estrutura gótica em pedra/ouro (topo em arco, tipografia serifada small-caps no nome) + **canais de runa com energia** correndo na cor do domínio. É o próprio núcleo do jogo (ciber-gótico × magia=software). Descartados: cartucho tech puro (1) e grimório gótico puro (2).
- **Cor por DOMÍNIO** (mock `17-moldura-cores-dominio.html`): mesma estrutura/tipografia/layout; muda a **cor do acento/borda + o sigilo**. Paleta aprovada:
  - ⚡ Eletromagnetismo = **ciano** `#22D3EE` (glow do Cauã)
  - 🌌 Física = **violeta** `#A78BFA` (espaço-tempo)
  - 🔢 Matemática = **verde** `#34D399` (Selve fractal)
  - 💻 Computação = **azul** `#60A5FA` (software/dado)
  - 🜀 Ocultistas = **carmesim** `#F43F5E` (ritual/obsidiana)
  - 📈 Economia = **âmbar/ouro** `#F59E0B` (valor)
- **A Carta Perdida de Tusk (capstone) = tratamento LENDÁRIO** (mocks `18` + `19`): base **B** (moldura de ouro + canais de runa pulsando nas 6 cores de domínio, fininho, "contém os 20") **+ GLOW DOURADO PULSANTE** (respira entre médio e forte, ~2.4s). Descartados: prismática de borda (arco-íris, demais), branco-dourado radiante, ônix.

## Layout comum (todas as cartas)

- Topo em arco: **nome** da carta (serifado, small-caps).
- **Janela de arte** (~45-55% da altura): onde entra a FIGURA INTERIOR da carta (gravura/desenho, discussão futura separada; NÃO a foto de referência de sprite).
- **Sigilo do domínio** (canto sup. esq. da arte, com glow na cor do domínio) + **custo** (canto sup. dir.).
- **Linha de tipo** (ex.: `passiva // chave`, monoespaçada, na cor do domínio).
- **Caixa de texto de efeito**.
- **Rodapé**: marcador de **raridade** (comum / rara / lendária) + **de quem se descobre** (ex.: "Faraday").
- Emojis dos sigilos = placeholder; viram **sigilos desenhados** na produção.

## Produção de arte — plano de DUAS TRILHAS (criador 2026-07-09)

- **Trilha 1 (testar primeiro):** o **PixelLab** tenta reproduzir a moldura, provavelmente melhor alimentado com um **print do mock escolhido** como referência (image-to-pixelart / style-transfer / create_ui_asset). Se sair fiel, usa-se.
- **Trilha 2 (fallback, provável para a moldura):** se o PixelLab não reproduzir bem, a **moldura é renderizada pelo glintfx** (RCSS já faz gradiente + glow + animação + recolor por domínio via variável; os mocks são literalmente CSS/RCSS-like; o glow pulsante do Tusk é animação nativa) e o **PixelLab faz SÓ as figuras internas das cartas + os sprites** das figuras no mundo. (Consumir o glintfx apenas; NÃO mexer na lib.)
- Em ambas: **PixelLab sempre faz as figuras internas + os sprites**; a dúvida é só quem faz a MOLDURA.

**DECISAO DO CRIADOR (2026-07-09): TRILHA HIBRIDA.** O **PixelLab gera a TEXTURA base da moldura** (a pedra gotica + runas + estrutura de ouro, em pixel-art nativo; teste `create_ui_asset` provou que sai lindo e on-brand, ver `resources/images/card-frame-tests/pixellab-frame-cyan-v1.png`). O **glintfx (RCSS) sobrepoe**: a COR do dominio (tint/recolor por 1 variavel), o **GLOW dourado PULSANTE** do Tusk (animacao nativa), os ESTADOS (hover/selecionada/desabilitada), e compoe a figura interior + nome + custo + texto de efeito + sigilo + raridade. Implicacao de pipeline: gerar a base PixelLab de forma RECOLORIVEL (runas/energia em tom neutro/claro que o glintfx tinge; ou base pedra+ouro neutra + canal de energia adicionado pelo glintfx). Implementacao = tarefa de engenharia UI (consumir glintfx, NAO mexer na lib; ver `reference_glintfx_api`), em GusEngine/app. PixelLab sempre faz as figuras internas + os sprites.

## Cross-refs

`docs/design/brainstorm-backlog.md` (seção Fase B, decisões do roster), `docs/design/roster-analogos/*.md`, `project_rmlui_ui_stack`, ADR-010 (glintfx embed), `reference_pixellab_mcp`, `reference_glintfx_api`.


## Recolor por dominio — mecanismo confirmado (2026-07-09)

**`image-color` (RCSS nativo do glintfx/RmlUi 6.3)** faz o recolor. UMA base neutra (runas brancas) -> 6 dominios trocando 1 variavel:
```css
.card { decorator: image( frame_neutral_arched.png ); image-color: var(--domain); transition: image-color .4s; }
.card.eletromag { --domain: #22D3EE; }  /* fisica #A78BFA, matematica #34D399, computacao #60A5FA, oculto #F43F5E, economia #F59E0B */
```
Multiply premultiplicado, interpolavel (anima com o glow), default white=no-op. **LIMITE:** tinge o ouro junto (multiply uniforme). Se incomodar no teste real -> **puxar** o luminance-key (semente no INBOX do glintfx, ver `docs/tech/glintfx-requests/REQ-decorator-image-tint.md`). Base neutra gerada: `resources/images/card-frame-tests/pixellab-frame-neutral-arched-v2.png`. **PENDENTE:** montar um render minimo glintfx (RML+RCSS) da carta pra testar o recolor de verdade e julgar o ouro.