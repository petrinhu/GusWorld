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

Gerados em modo autonomo (sessao 2026-06-24, criador dormindo, autorizou "continue fazendo os sprites" + "o restante dos personagens"). A melhor de 4 variacoes (size 128, sidescroller) foi escolhida e promovida. **35 assets** no total, salvos em `resources/sprites/icons-m5/` (gitignored). HTML de revisao: `resources/sprites/icons-m5/REVISAO.html`.

### Status (13) -- tag m5-status-icon / m5-icon-r2
| Status | var | object_id | | Status | var | object_id |
|---|---|---|---|---|---|---|
| Stun | v3 | f0c96aba | | Decrypt | v1 | 6702d5ef |
| Poison | v0 | cb7da2fb | | Shield | v0 | afa97269 |
| Corrode | v3 | d69ac756 | | Regen | v0 | 835a8637 |
| Disrupt | v0 | 4586a554 | | Haste | v0 | a5fdeb3e |
| Silence | v0 | e742a576 | | Slow | v0 | 4022408e |
| Knockback | v0 | 0d68035b | | | | |
| Break | v1 | bb3c4edb | | | | |
| Expose | v1 | 2a2fa22a | | | | |

### Intent (4) -- tag m5-icon-r2 / m5-icon-r3
| Intent | var | object_id |
|---|---|---|
| Atacar | v0 | f933ebca |
| Defender | v2 (retry, 1o bloqueado por content policy) | bd62cea5 |
| Aplicar-status | v0 | 99b81f5b |
| Ruido/Patch-Zero | v2 | a5c4351b |

### Modificador (3) -- tag m5-icon-r3
| Modificador | var | object_id |
|---|---|---|
| Object | v1 | 2a72eca6 |
| Stream | v0 | c065fced |
| Null | v2 | c5651671 |

### Retratos (9) -- tag m5-retrato
| Personagem | var | object_id | | Personagem | var | object_id |
|---|---|---|---|---|---|---|
| Gus | v3 | 90aef2b3 | | Linda | v3 | a877d868 |
| Caua | v1 | 841213ce | | Bento | v2 | a3b2603b |
| Jaci | v3 | e8164ea4 | | Iara | v0 | dddb013d |
| Inimigo generico | v3 | db628b90 | | Dante | v1 | 2a93df63 |
| | | | | Sterling | v1 | 63f9eb6a |

### Moldura (1) -- tag m5-moldura
| Moldura de carta | v2 | 0d86d823 |

### Custo
~720 geracoes (fichas) nesta sessao (5 familia + 13 status + 4 intent com 1 retry + 3 modificador + 9 retratos + 1 moldura). Saldo: used ~911 de 2000, restam ~1089.

### Pendentes (NAO gerados autonomamente, deixados pro criador)
- **Cenario do overworld** (9 assets P0: 2 tilesets + 7 map objects): prompts PRONTOS em `WORLD_DISTRITOS_INFERIORES_PIXELLAB.md`, mas NAO gerei pra respeitar o limite de gasto auto-imposto (~900 fichas; tileset pode custar mais). O criador valida o gasto e gera.
- **Poses de CORPO INTEIRO** (cast/ataque/hit/vitoria): pendentes da decisao arena 2D-vs-3D (adiada pelo criador).
- **Revisao humana:** alguns retratos e o inimigo merecem o olho do criador (traços a 32px sao dificeis de avaliar sozinho); regenerar individualmente se algum nao convencer.
