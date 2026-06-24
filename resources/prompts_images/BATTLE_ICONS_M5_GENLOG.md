# Battle Icons M5: log de geracao PixelLab (rastreabilidade)

Registro de COMO os icones foram gerados (parametros reais usados) e QUAL variacao virou canonica. Os prompts ficam em `BATTLE_ICONS_M5_PIXELLAB.md`; os PNGs em `resources/sprites/icons-m5/` (gitignored, assets locais). Sem em-dash.

## Parametros reais de geracao

- Ferramenta: PixelLab MCP, `create_1_direction_object`.
- size: 128 (canvas 128x128). Com size <= 170 o PixelLab entrega 4 candidatos por leva.
- view: `sidescroller` (vista frontal plana, melhor pra icone-simbolo que `top-down`).
- Candidatos por leva: 4. Custo: 20 geracoes (fichas) por leva.
- Fluxo: gera leva (4 candidatos em status review) -> escolhe 1 -> `select_object_frames` promove a escolhida e descarta o resto.
- Nota de detalhe: o guia (PARTE 1 dos prompts) sugeria 32x32; geramos a 128 (mais nitido, e 4 candidatos em vez de 64), com downscale pra 32 no HUD em runtime.

## Escolhas canonicas: 5 icones de familia (validados pelo criador 2026-06-24)

| Familia | Variacao | object_id PixelLab (tag m5-familia-icon) | PNG salvo |
|---|---|---|---|
| Eletrico | v3 | 7bacccb5-64b9-408d-8d94-ff25281ad78a | familia_eletrico.png |
| Bioquimico | v1 | 20a6c0da-4dc8-447f-a032-232cf735daa9 | familia_bioquimico.png |
| Sonico | v0 | c8a18604-d411-4ba9-b5d0-f3e4182032bd | familia_sonico.png |
| Cinetico | v2 | dfc87af9-f18d-43e3-8386-a175fb057453 | familia_cinetico.png |
| Criptografico | v3 | 06cf44a3-983a-4e1f-9458-ad12d1f6f3c6 | familia_criptografico.png |

## Demais icones P0 + retratos do elenco

Gerados em modo autonomo (sessao 2026-06-24, criador dormindo, autorizou "continue fazendo os sprites" + "o restante dos personagens"). Escolhas preenchidas abaixo conforme processados. Poses de CORPO INTEIRO ficam pendentes da decisao arena 2D-vs-3D (nao geradas autonomamente).
