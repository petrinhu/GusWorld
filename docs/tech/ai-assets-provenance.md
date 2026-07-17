# Proveniência de assets gerados por IA

> **Owner:** petrinhu. **Last-reviewed:** 2026-07-12. **Tipo Diátaxis:** reference.

Os assets próprios do GusWorld (arte, música, texto in-game) são licenciados sob CC-BY-SA 4.0 (ver [ASSETS-LICENSE.md](../../ASSETS-LICENSE.md)). Para o criador poder impor essa licença sobre um asset, ele precisa primeiro ter o direito de fazê-lo, isto é, ser o titular do copyright daquele asset. Quando o asset nasce de uma ferramenta de IA generativa de terceiro, esse direito depende dos Termos de Serviço daquela ferramenta (planos gratuitos costumam reter direitos de uso comercial ou de titularidade; planos pagos costumam ceder a titularidade ao usuário). Este documento rastreia, por asset ou lote, qual ferramenta gerou o conteúdo, quando, sob qual tier/plano, e qual é a base legal que sustenta o CC-BY-SA sobre aquele asset.

Este doc cobre apenas conteúdo **gerado por IA de terceiro**. Assets curados de bancos de terceiros já livres de direitos (ex.: kit de áudio CC0 do Kenney/OpenGameArt em `assets/AUDIO_KIT_PROVISORIO.md`) não são gerados por IA e não entram aqui; a proveniência deles vive no próprio doc de origem e, quando aplicável, em [THIRD-PARTY-LICENSES.md](../../THIRD-PARTY-LICENSES.md).

---

## Tabela de proveniência

| Asset / lote | Ferramenta | Data geração | Tier confirmado | Base legal p/ CC-BY-SA |
|---|---|---|---|---|
| `resources/sprites/*.png` (47 versionados no repo; ~692 no total em disco, pipeline PixelLab desde M2) | PixelLab | desde ~2026-06-23 | **PRO Tier 1, confirmado pelo criador em 2026-07-12** | PixelLab ToS: no plano pago o usuário é o titular do copyright do conteúdo gerado, uso comercial incluso. |
| `resources/glb/*.glb` (`gus.glb`, `heliaco_vyr.glb`; saída do bake-to-sprite image-to-3D; NÃO versionados no repo hoje, só localmente) | Tripo3D | não registrada | **plano pago, confirmado pelo criador em 2026-07-12** | Tripo3D ToS: licença comercial acompanha o asset gerado sob conta com créditos Pro. |
| `resources/images/gus_vector.png`, `resources/images/vance_dragon_glyph.png` (imagens de arte conceitual/referência, versionadas no repo) | Gemini (nano banana) / Grok | não registrada | **plano pago, confirmado pelo criador em 2026-07-12** | ToS do provedor (Google Gemini / xAI Grok): uso comercial do conteúdo gerado incluso no plano pago da conta do criador. |
| `resources/images/catedral_mae.png` (establishing shot da Catedral-Mãe, 2816x1536, ~6,6 MB; candidata a key art; prompt em [`resources/prompts_images/feitos/CATEDRAL_MAE_IMAGEPROMPT.md`](../../resources/prompts_images/feitos/CATEDRAL_MAE_IMAGEPROMPT.md), que abre com "image prompt (nano banana)") | **Gemini 2.5 Flash Image ("nano banana")** — identificado por **C2PA assinado pelo Google embutido no PNG** (`Claim Generator Info Name: Google C2PA Core Generator Library`; `Actions Description: "Created by Google Generative AI. Applied imperceptible SynthID watermark."`), com **watermark SynthID** e um ingredient `parentOf` (fluxo gerar→editar do próprio Gemini). Sem qualquer marcador de xAI/Grok. | 2026-05-25 (mtime do arquivo) | **plano pago, confirmado pelo criador em 2026-07-12** (mesma ferramenta/conta da linha do Gemini acima; herda aquela confirmação, não é declaração nova) | ToS do provedor (Google Gemini): uso comercial do conteúdo gerado incluso no plano pago da conta do criador. Mesma base legal da linha `gus_vector.png`/`vance_dragon_glyph.png`. |
| `resources/images/card-frame-tests/**` (4 arquivos versionados: `pixellab-frame-clean-v1.png`, `clean-eletromag.png`, `faraday-cage-t.png`, `cards/CARTA-gaiola-faraday-amostra.png`; texturas base + figura interior da moldura de carta, `docs/design/card-frame-spec.md`) | PixelLab (`create_ui_asset` / geração de figura) | 2026-07-09 | **PRO Tier 1, confirmado pelo criador em 2026-07-12** | Mesma base do lote de sprites acima (PixelLab ToS, titularidade do usuário no plano pago). |
| `assets/music/Arena_GusWorld.mp3` (tema de batalha/arena, único trecho musical gerado por IA no kit de áudio atual) | Suno | 2026-07-03 | **plano pago (Pro/Premier), confirmado pelo criador em 2026-07-12** | Suno ToS: no plano pago há atribuição (assignment) da titularidade do áudio gerado ao usuário. |

---

## Nota técnica: preservar o C2PA da `catedral_mae.png` (2026-07-17)

O PNG da catedral carrega **manifesto C2PA assinado pelo Google** + **watermark SynthID**. Dois fatos verificados (apuração da sessão do site, `AUD-LICENCA`):

- **Converter o arquivo (ex.: PNG para JPG) ARRANCA o manifesto C2PA** — confirmado numa cópia já convertida: zero campos C2PA. O **SynthID vive nos pixels** e provavelmente sobrevive à reencodagem, mas **o manifesto não**.
- Portanto: se algum dia exportarmos/otimizarmos a catedral (ou qualquer imagem com C2PA), **preservar o metadado**. Decisão do líder em 2026-07-17: a versão publicada no site **preserva o C2PA**, por coerência com o [`AI-DISCLOSURE.md`](../../AI-DISCLOSURE.md) — não escondemos que usamos IA, e uma versão publicada **sem** o marcador pode *parecer* que alguém o raspou de propósito.

Vale como regra geral do repo pra qualquer asset de IA que venha com proveniência embutida: **o pipeline de export não pode ser o que apaga a prova de origem.**

## Nota honesta sobre o resto do kit de áudio

O restante do kit de áudio ativo (`assets/music/cidade_tema_provisorio.mp3` + os 3 SFX documentados de `assets/sfx/`: `hit_digital_provisorio.wav`, `hit_digital_alt_provisorio.wav`, `ui_confirm_provisorio.wav`) **não é gerado por IA**: é curadoria de bancos CC0 de terceiro (Kenney, OpenGameArt), com proveniência já documentada faixa a faixa em [`assets/AUDIO_KIT_PROVISORIO.md`](../../assets/AUDIO_KIT_PROVISORIO.md). Esses arquivos ficam fora da tabela acima porque a base legal deles é a licença CC0 da fonte original, não um ToS de ferramenta de IA.

Há ainda 3 arquivos SFX versionados sem entrada de proveniência em nenhum doc (`menu_hover_provisorio.wav`, `menu_click_provisorio.wav`, `menu_blocked_provisorio.wav`, todos em `assets/sfx/`). Presumem-se do mesmo lote CC0 curado (mesmo padrão de nome `_provisorio`), mas isso **não está confirmado por escrito** em nenhum documento existente. Recomenda-se abrir um item de TODO para o criador confirmar a fonte desses 3 arquivos e atualizar `assets/AUDIO_KIT_PROVISORIO.md` de acordo (fora do escopo desta tarefa de correção de licença).

---

## Nota de fecho

A proveniência acima (ferramenta, tier, plano pago) foi **confirmada verbalmente pelo criador em 2026-07-12**, não por documento de billing anexado. O detalhamento de cobrança/plano por asset individual fica no histórico de conta de cada serviço (PixelLab, Tripo3D, Suno, Gemini, Grok), fora deste repositório.

*Recomendação técnica de rastreabilidade; validação jurídica formal cabe ao titular.*
