# Especificação de Personagem: Bento "Requiem" Chevalier (Companheiro — Tanque)

**Visual vigente: 2D pixel-art via PixelLab** (pivô 3D→2D, ver CLAUDE.md).

> **Status:** canônico. Documento mestre. Os TRAÇOS DE IDENTIDADE abaixo seguem valendo para qualquer mídia de renderização.
> **Cross-ref:** projeto `Projects/gusworld/` — style guide aplica esta spec.

## Traços de identidade (vigentes, agnósticos de dimensão)

- **Idade:** 14 anos, endomorfo robusto, ascendência Luso-BR+Francesa (Bendito / Cavaleiro).
- **Rosto:** angular, linhas duras, expressão severa (expressionismo gótico).
- **Armadura:** metal escuro escovado, sulcos e desgaste industrial, detalhes em latão oxidado.
- **Acessório-assinatura:** Escudo Catedral (escudo de torre cobrindo 2/3 da altura do personagem, relevos góticos e engrenagens expostas em movimento) + cronômetro mecânico de latão na placa peitoral, com ponteiros funcionais.
- **Role mecânico:** Tanque / Defensor de Área — campos de contenção gravitacional via escudo mecânico pesado.
- **Vinculação ao setting:** Catedrais de Neo-Sylvania (distrito histórico gótico).
- **Filosofia técnica:** **tradição analógica não-codificada** — relojoaria mecânica + magia heráldica. **Exceção declarada do Pillar 2** ("magia é sistema formal computável" — relógio mecânico É state machine, substrato analógico). Drama interno do party: tese rival ao mundo-código do Gus.
- **Anti-uso:** sem combate ofensivo frontal; tank é proteção pra Gus compilar atrás dele.

## Pendências — spec 2D detalhada (decisão do líder)

- Resolução de sprite: DECIDIDO em 30/08/2026 — canônico **180×180** (ver `docs/art/style-guide.md` §8); o arquivo em disco está em **256×256** e **não é regerado**, o ajuste é em tempo de execução pelo motor gráfico do GlintFx (ver `docs/art/sprites-inventory.md`).
- Profundidade de paleta: DECIDIDO em 30/08/2026 — paleta enxuta, medido **58 cores únicas**, `PaletteAlpha` (só o Gus tem paleta rica, ver `docs/art/style-guide.md` §9).
- Nº de frames/direções de sprite: não definidos, ver `docs/art/style-guide.md`.
- Prompt PixelLab: não definido.
- Como as engrenagens animadas do escudo (antes via keyframes 3D) viram frames de spritesheet 2D: pendente.

