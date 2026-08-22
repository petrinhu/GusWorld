# Dossie tecnico: PixelLab API (api.pixellab.ai)

Levantamento para o projeto GusWorld (jogo 2D pixel-art, C++23).
Item 27/28 de `inicial.md`: conta em pixellab.ai, token em `$PIXELLAB_MCP` / `PIXELLAB_MCP_BEARER`, uso web autorizado.

Legenda de marcacao:
- **[FATO: <url>]** = extraido diretamente de uma fonte, com a URL de origem.
- **[INFERENCIA]** = conclusao/opiniao minha, nao proveniente diretamente da doc.
- **[NAO DOCUMENTADO]** = a doc consultada nao respondeu a este ponto.

Status do token no ambiente: `PIXELLAB_MCP` e `PIXELLAB_MCP_BEARER` estao presentes nas variaveis de ambiente desta sessao (valor NAO exibido, NAO logado, NAO vazado neste dossie).

---

## 1. Levantamento em andamento

(secao inicial, sera substituida por conteudo real conforme a exploracao avanca)

## 2. Fonte primaria localizada

**[FATO: https://api.pixellab.ai/v1/docs]** A pagina `/v1/docs` e renderizada em JavaScript (Scalar API Reference, via `cdn.jsdelivr.net/npm/@scalar/api-reference`), servida por um backend **FastAPI**. O HTML bruto nao carrega o conteudo (WebFetch de HTML puro nao executa JS), mas referencia a especificacao real em `/v1/openapi.json`.

**[FATO: https://api.pixellab.ai/v1/openapi.json]** Especificacao OpenAPI 3.x completa foi obtida com sucesso via `curl`.
- `info.title`: "Pixel Lab API"
- `info.version`: "dev"
- `servers`: `[{"url": "/v1"}]` (ou seja, todos os paths do spec sao relativos a `https://api.pixellab.ai/v1`)
- `components.securitySchemes.HTTPBearer`: `{"type": "http", "scheme": "bearer"}`

### 2.1 Descricao geral da API (texto oficial, verbatim)

**[FATO: https://api.pixellab.ai/v1/openapi.json — campo `info.description`]**

> "The API provides endpoints for creating AI-generated pixel art images, rotations, animations, and more. Making it easy for applications to integrate pixel art generation capabilities."

Cliente Python oficial: `pip install pixellab`, repositorio `https://github.com/pixellab-code/pixellab-python` **[FATO: mesma fonte]**.

### 2.2 Autenticacao (texto oficial, verbatim)

**[FATO: https://api.pixellab.ai/v1/openapi.json — campo `info.description`]**

> "The API uses a simple token based authentication system. After creating an account, you can find your API token in your account settings (https://pixellab.ai/account). Include this token in all API requests using the Bearer authentication scheme."

Exemplo oficial de chamada:
```bash
curl -X POST https://api.pixellab.ai/v1/generate-image-pixflux \
    -H "Authorization: Bearer YOUR_API_TOKEN" \
    -H "Content-Type: application/json" \
    -d '{
        "description": "cute dragon",
        "image_size": {"width": 128, "height": 128}
    }'
```

Cliente Python oficial:
```python
import pixellab
client = pixellab.Client(secret="YOUR_API_TOKEN")
client.generate_image_pixflux(description="cute dragon", image_size=dict(width=128, height=128))
```

**Confirmacao de compatibilidade com o setup do lider**: o esquema de seguranca no schema OpenAPI e `HTTPBearer` (`type: http`, `scheme: bearer`), exatamente o formato que o header `Authorization: Bearer $PIXELLAB_MCP` (conforme `inicial.md` item 27) produz. A confirmacao pratica (chamada real de baixo custo, `GET /balance`, sem gerar imagem) foi feita e esta registrada na secao 4.

### 2.3 Lista de endpoints (FATO, direto do OpenAPI)

**[FATO: https://api.pixellab.ai/v1/openapi.json — campo `paths`]** Total: 8 endpoints, todos sob o prefixo `/v1`:

| Metodo | Path | Modelo/funcao (nome oficial na doc) |
|---|---|---|
| POST | `/generate-image-pixflux` | Generate Image Pixflux — gera pixel art a partir de descricao textual |
| POST | `/generate-image-bitforge` | Generate Image Bitforge — aplica estilo de arte customizado usando imagens de referencia |
| POST | `/animate-with-skeleton` | Animate (skeleton) — gera 4 frames de animacao a partir de poses de esqueleto |
| POST | `/animate-with-text` | Animate (text) — gera animacao a partir de descricao textual |
| POST | `/rotate` | Rotate — rotaciona um objeto ou personagem |
| POST | `/inpaint` | Inpaint — edita/modifica pixel art existente |
| POST | `/estimate-skeleton` | (nao descrito no paragrafo "Supported models"; estima esqueleto/pose a partir de imagem, ver detalhe de schema adiante) |
| GET | `/balance` | Consulta saldo de creditos da conta |

Nao ha endpoint documentado para: geracao de tileset/tile de mapa, geracao de icone de carta especificamente, remocao de fundo isolada, upscale isolado, ou paleta isolada — ver secao 5 (encaixe no pipeline) para o que isso implica. **[INFERENCIA]** alguns desses efeitos podem ser obtidos combinando os 8 endpoints existentes (ex.: pixflux ja gera com fundo transparente, ver schema).


## 3. Detalhamento de cada endpoint (FATO, direto do `openapi.json`)

Todos os endpoints POST recebem `application/json`, exigem header `Authorization: Bearer <token>` (esquema `HTTPBearer`), e devolvem os mesmos codigos de erro possiveis: `401` Invalid API token, `402` Insufficient credits, `422` Validation error, `429` Too many requests, `529` Rate limit exceeded. **[FATO]** (nao ha descricao numerica de limite de requisicoes por minuto/hora em lugar nenhum do spec; ver secao 4).

### 3.1 `POST /generate-image-pixflux` — Generate Image Pixflux

**[FATO]** Descricao oficial: "Creates a pixel art image based on the provided parameters. Called 'Create image (new)' in the plugin." Gera pixel art **a partir de descricao textual apenas** (nao usa imagem de referencia de estilo).

- Tamanho suportado: area minima 32x32 (o schema define `width`/`height` cada um entre 16 e 400, mas o texto da doc fala em "minimum area 32x32 and maximum area 400x400" — ou seja a restricao de area total pode ser mais estrita que largura/altura individuais).
- Recursos suportados (texto oficial): Init image, Forced palette, Transparent background.

**Corpo da requisicao (`GenerateImagePixfluxRequest`)**:
| Campo | Tipo | Obrigatorio | Default | Descricao oficial |
|---|---|---|---|---|
| `description` | string | sim | - | Text description of the image to generate |
| `negative_description` | string | nao | "" | (Deprecated) |
| `image_size` | `{width, height}` (16-400 cada) | sim | - | tamanho da imagem |
| `text_guidance_scale` | number | nao | 8 | How closely to follow the text description |
| `outline` | enum `Outline` ou null | nao | - | Outline style reference (weakly guiding) |
| `shading` | enum `Shading` ou null | nao | - | Shading style reference (weakly guiding) |
| `detail` | enum `Detail` ou null | nao | - | Detail style reference (weakly guiding) |
| `view` | enum `CameraView` ou null | nao | - | Camera view angle (weakly guiding) |
| `direction` | enum `Direction` ou null | nao | - | Subject direction (weakly guiding) |
| `isometric` | bool | nao | false | Generate in isometric view (weakly guiding) |
| `no_background` | bool | nao | false | Generate with transparent background (blank background over 200x200 area) |
| `init_image` | `Base64Image` ou null | nao | - | Initial image to start from |
| `init_image_strength` | integer | nao | 300 | Strength of the initial image influence |
| `color_image` | `Base64Image` ou null | nao | - | Forced color palette, imagem contendo as cores da paleta |
| `seed` | integer ou null | nao | - | Seed decides the starting noise |

**Resposta 200 (`GenerateImagePixfluxResponse`)**: `{ "usage": {"type":"usd","usd": <numero>}, "image": {"type":"base64","base64": "data:image/png;base64,..."} }`. Exemplo oficial no spec mostra `usd: 0.01` para esta chamada.

### 3.2 `POST /generate-image-bitforge` — Generate Image Bitforge

**[FATO]** Descricao oficial: "Generates a pixel art image based on the provided parameters. Called 'Generate image (style)' in the plugin." Diferenca-chave do pixflux: aceita **`style_image`** (imagem de referencia de estilo) e **`style_strength`** (0-100), alem de aceitar tambem `skeleton_keypoints`/`skeleton_guidance_scale`, `inpainting_image`+`mask_image`, `coverage_percentage`, `oblique_projection`.

- Tamanho maximo: area 200x200 (schema: width/height cada 16-200).
- Recursos suportados (texto oficial): Style image, Inpainting, Init image, Forced palette, Transparent background.

**Campos exclusivos deste endpoint em relacao ao pixflux**: `extra_guidance_scale` (Deprecated), `style_strength`, `oblique_projection`, `coverage_percentage`, `style_image`, `inpainting_image`, `mask_image`, `skeleton_guidance_scale`, `skeleton_keypoints`. Demais campos (`description`, `image_size`, `text_guidance_scale`, `outline`, `shading`, `detail`, `view`, `direction`, `isometric`, `no_background`, `init_image`, `init_image_strength`, `color_image`, `seed`) tem o mesmo papel do pixflux.

**Resposta 200**: mesma forma de `GenerateImagePixfluxResponse` (`usage` + `image` base64).

### 3.3 `POST /animate-with-skeleton` — Animate (skeleton)

**[FATO]** Descricao oficial: "Creates a pixel art animation based on the provided parameters. Called 'Animate with skeleton' in the plugin." **Gera exatamente 4 frames de animacao** a partir de poses de esqueleto (keypoints x/y/label/z_index).

- Tamanhos suportados: **exatamente** 16x16, 32x32, 64x64, 128x128 ou 256x256 (nao e um range livre, e um conjunto fechado de valores, conforme texto oficial "Supported image sizes: 16x16 / 32x32 / 64x64 / 128x128 / 256x256"; o schema tecnico so define min 16 / max 256 sem enum, entao a doc textual e mais restritiva que o schema JSON).
- Recursos: Inpainting, Init image, Forced palette.
- `skeleton_keypoints`: array de arrays de `Point` (`x`,`y`,`label` do enum `SkeletonLabel` com 18 partes do corpo tipo pose humana — NOSE, NECK, RIGHT/LEFT SHOULDER, ELBOW, ARM(mao), HIP, KNEE, LEG(pe), EYE, EAR — `z_index`). **[FATO, texto exato do campo]**: "Skeleton pose keypoints. Requires EXACTLY 3 frames — the model is a 3-frame window; other counts are rejected with a 422." Ou seja, o cliente fornece poses para **3 frames** (janela de contexto do modelo) e o modelo devolve **4 frames** de animacao (a resposta e `images` array).
- `reference_image` (`Base64Image`): **obrigatorio**, imagem de referencia do personagem/objeto a animar.
- `inpainting_images` (default `[None, None, None]`) e `mask_images`: usados para mostrar ao modelo o esqueleto conectado / mascara de onde re-pintar.
- `view` (default "side") e `direction` (default "east"), `isometric`, `oblique_projection`, `color_image`, `init_images`+`init_image_strength`, `guidance_scale` (default 4.0), `seed`.

**Resposta 200 (`AnimateWithSkeletonResponse`)**: `{ "usage": {...}, "images": [ Base64Image, ... ] }` (array de imagens, os frames gerados).

### 3.4 `POST /animate-with-text` — Animate (text)

**[FATO]** Descricao oficial: "Creates a pixel art animation based on text description and parameters." Gera animacao guiada por **texto** em vez de esqueleto explicito.

- Tamanho suportado: **apenas 64x64** (schema define width/height fixos em 64, minimo=maximo=64).
- Recursos: Text-guided animation generation, Inpainting, Init image, Forced palette, Multiple frames.
- Campos-chave: `description` (personagem), `action` (ex.: "walk"), `view` (default side), `direction` (default east), `n_frames` (default 4; texto oficial do campo diz "Length of full animation (the model will always generate 4 frames)" — ou seja mesmo pedindo outro valor o modelo sempre devolve 4 frames por chamada), `start_frame_index` (default 0, permite pedir uma "janela" de uma animacao maior), `reference_image` (`Base64Image`, obrigatorio), `inpainting_images` (default 4x None), `mask_images` (default 4x None), `image_guidance_scale` (default 1.4), `text_guidance_scale` (default 8.0), `seed` (default 0 = aleatorio, diferente dos outros endpoints onde default e null).

**Resposta 200 (`AnimateWithTextResponse`)**: `{ "usage": {...}, "images": [...] }`.

**[INFERENCIA]** O par `n_frames` + `start_frame_index`, junto com o fato de o modelo sempre gerar 4 frames por chamada, sugere que animacoes mais longas que 4 frames sao construidas por MULTIPLAS chamadas encadeadas, usando `inpainting_images` da chamada anterior como contexto da proxima janela. Isso nao esta explicitado em texto corrido na doc, e uma leitura dos nomes/defaults dos campos.

### 3.5 `POST /rotate` — Rotate character or object

**[FATO]** Descricao oficial: "Rotates a pixel art image based on the provided parameters. Called 'Rotate' in the plugin."

- Tamanhos suportados: 16x16, 32x32, 64x64, 128x128 (texto oficial lista esses 4; schema tecnico so define min16/max200).
- Recursos: Init image, Forced palette.
- Campos: `from_image` (`Base64Image`, obrigatorio, "Reference image to rotate"), `from_view`/`to_view` (enum `CameraView`, default "side"/"side"), `from_direction`/`to_direction` (enum `Direction`, default "south"/"east"), `view_change`/`direction_change` (inteiros, graus de rotacao, alternativa numerica aos enums de view/direction), `isometric`, `oblique_projection`, `init_image`+`init_image_strength`, `mask_image` ("Requires init image!" — soh funciona se `init_image` tambem for enviado), `color_image`, `image_guidance_scale` (default 3.0), `seed`.

**Resposta 200 (`RotateResponse`)**: `{ "usage": {...}, "image": Base64Image }` (uma unica imagem rotacionada).

**[INFERENCIA]** Este e o endpoint mais diretamente aplicavel a "sprite sheet de personagem em N direcoes": gerar 1 direcao manualmente (ou via pixflux/bitforge) e usar `/rotate` para obter as demais 7 direcoes do enum `Direction` (north, north-east, east, south-east, south, south-west, west, north-west), reaproveitando o mesmo objeto/personagem.

### 3.6 `POST /inpaint` — Inpaint image

**[FATO]** Descricao oficial: "Creates a pixel art image based on the provided parameters. Called 'Inpaint' in the plugin." Edita/modifica uma imagem existente numa regiao definida por mascara.

- Tamanho maximo: area 200x200.
- Recursos: Inpainting, Init image, Forced palette, Transparent background.
- Campos obrigatorios exclusivos: `inpainting_image` (`Base64Image`, imagem a ser editada) e `mask_image` (`Base64Image`, mascara preto-e-branco onde "branco = onde o modelo deve repintar"). `description` tambem obrigatorio (o que gerar na regiao mascarada).
- Demais campos: `negative_description`, `text_guidance_scale` (default 3.0, mais baixo que pixflux/bitforge), `extra_guidance_scale` (Deprecated), `outline`/`shading`/`detail`/`view`/`direction`, `isometric`, `oblique_projection`, `no_background`, `init_image`+`init_image_strength`, `color_image`, `seed`.

**Resposta 200 (`InpaintResponse`)**: `{ "usage": {...}, "image": Base64Image }`.

### 3.7 `POST /estimate-skeleton` — Estimate skeleton

**[FATO]** Descricao oficial: "Estimates the skeleton of a character, returning a list of keypoints to use with the skeleton animation tool." Endpoint auxiliar: recebe uma imagem de personagem (com fundo transparente, conforme exemplo do cliente Python: "image_of_the_character_on_a_transparent_background") e devolve os `keypoints` (pose) para alimentar `/animate-with-skeleton`.

- Tamanhos suportados: 16x16, 32x32, 64x64, 128x128, 256x256 (mesma lista do animate-with-skeleton).
- Corpo (`EstimateSkeletonRequest`): campo unico `image` (`Base64Image`), **marcado como opcional no schema tecnico** mas semanticamente necessario (sem imagem nao ha o que estimar) — **[NAO DOCUMENTADO]** o comportamento exato se `image` for omitido (provavelmente 422).

**Resposta 200 (`EstimateSkeletonResponse`)**: `{ "usage": {...}, "keypoints": [ {x, y, label, z_index}, ... ] }`. Note que o formato de retorno usa o schema `Keypoint` (label como string livre), enquanto o schema `Point` (usado no *request* de animate-with-skeleton) usa `label` como enum fechado `SkeletonLabel` — **[INFERENCIA]** provavelmente compativeis na pratica (a string retornada deve casar com os valores do enum), mas o spec nao garante isso formalmente por tipagem.

### 3.8 `GET /balance` — Get balance

**[FATO]** Descricao oficial: "Returns the current balance for your account." Sem parametros de entrada. Resposta 200 (`CreditsResponse`): `{ "type": "usd", "usd": <numero> }`. Erros possiveis: apenas `401` (Invalid API token) — nao lista 402/422/429/529 para este endpoint, unico da lista sem esses.


## 4. Verificacao pratica de autenticacao (executada nesta sessao)

**[FATO: chamada real feita por mim via `curl` nesta sessao, em 2026-08-21]** Executei `GET https://api.pixellab.ai/v1/balance` com o header `Authorization: Bearer $PIXELLAB_MCP_BEARER` (variavel de ambiente, valor nunca exibido/logado). Resultado:
- HTTP status: **200**
- Corpo: `{"type":"usd","usd":0.0}`

Conclusoes diretas:
- O token configurado pelo lider (`$PIXELLAB_MCP_BEARER`, derivado de `$PIXELLAB_MCP` conforme `inicial.md` item 27) **e valido** e autentica corretamente no formato `Authorization: Bearer <token>` documentado.
- O **saldo atual da conta e USD 0,00**. Isso significa que qualquer chamada aos endpoints de geracao (`pixflux`, `bitforge`, `animate-*`, `rotate`, `inpaint`, `estimate-skeleton`) provavelmente devolvera **HTTP 402 "Insufficient credits"** ate que creditos sejam adicionados na conta (https://pixellab.ai/account). **[INFERENCIA]**, baseada no schema de erros documentado (402 = Insufficient credits) combinado com o saldo zerado observado — nao testei de fato um endpoint de geracao para nao arriscar erro/custo desnecessario sem essa confirmacao previa.
- **Nao fiz** nenhuma chamada de geracao de imagem (pixflux/bitforge/animate/rotate/inpaint/estimate-skeleton) nesta sessao, por serem pagas (mesmo que minimas, ex.: USD 0,01 no exemplo oficial) e o saldo estar zerado.

## 5. Servidor MCP oficial

**[FATO: https://github.com/pixellab-code/pixellab-mcp]** Existe um repositorio oficial `pixellab-code/pixellab-mcp`. Pontos levantados:
- **Nao e um pacote local instalavel via npm/pip/uvx com nome fixo documentado** que eu tenha conseguido confirmar; a forma de uso documentada e um **servidor MCP remoto hospedado**, acessado via configuracao MCP genrica apontando para uma URL HTTPS, e nao via processo local.
- Bloco de configuracao MCP documentado (formato JSON para clientes MCP, ex.: Claude Code / Cursor / VS Code):
```json
{
  "mcpServers": {
    "pixellab": {
      "url": "https://api.pixellab.ai/mcp",
      "transport": "http",
      "headers": {
        "Authorization": "Bearer YOUR_API_TOKEN"
      }
    }
  }
}
```
- **Ferramentas (tools) expostas pelo MCP** (nomes documentados): `create_character` (personagens em 4 ou 8 direcoes), `animate_character` (adiciona animacoes tipo andar/correr/parado a personagens existentes), `create_tileset` (gera Wang tilesets para terrenos continuos), `create_isometric_tile` (tile isometrico individual). **[INFERENCIA]** Essas 4 ferramentas de mais alto nivel provavelmente sao compostas internamente sobre os mesmos 8 endpoints REST documentados em `/v1/openapi.json` (ex.: `create_character` provavelmente chama `generate-image-pixflux`/`bitforge` + `rotate` para as multiplas direcoes; `animate_character` provavelmente usa `estimate-skeleton` + `animate-with-skeleton` ou `animate-with-text`) — a doc do MCP nao explicita essa composicao interna, entao isso e uma leitura minha, nao um fato confirmado.
- **Licenca do repositorio do MCP**: rodape do repo mostra "Copyright (c) 2025 PixelLab. All rights reserved." **[FATO]** — ou seja, **nao ha licenca FOSS/MIT/Apache visivel** para este componente especifico; e codigo proprietario da PixelLab, hospedado publicamente para leitura/uso, mas sem licenca de redistribuicao aberta confirmada.
- **[FATO]** Existem tambem **forks/projetos de terceiros nao oficiais** com o mesmo tema (ex.: `flynnsbit/PixelLab-MCP`, descrito como "21+ AI tools", e `rabbitcannon/pixellab-forge-mcp`), que nao sao o servidor oficial e cuja confiabilidade/licenca eu nao verifiquei em profundidade (fora do escopo desta tarefa, que pediu o oficial).
- **Existe tambem o pacote Python oficial `pip install pixellab`** (`github.com/pixellab-code/pixellab-python`), que e um **cliente REST comum** (nao um servidor MCP) — ele fala diretamente com `api.pixellab.ai/v1/*`. Nao encontrei mencao de licenca de codigo explicita (MIT/Apache) nesse repositorio no conteudo que consegui inspecionar via WebFetch; **[NAO DOCUMENTADO]** de forma confirmada, precisaria abrir o arquivo `LICENSE` diretamente para confirmar.

## 6. Licenca e direitos sobre o CONTEUDO GERADO (critico para o GusWorld, que sera FOSS)

**[FATO: https://www.pixellab.ai/termsofservice]** Trechos relevantes (texto em ingles, citado o mais literal possivel conforme extraido):

> Secao 3.3: "You retain ownership of any content you create using PixelLab. You are free to use, modify, and distribute the outputs from our tools for any purpose."

> Secao 1.3: "You own the copyrights to your creations, permitting usage for both commercial and non-commercial purposes with no need for permission."

> Secao 1.2: "You agree not to use PixelLab to train your own models on the images generated by our product unless explicitly permitted by us in writing."

> Secao 2.2: "If you want to use the API for other purposes (like building your own service or reselling), reach out to us."

> Secao 3.4: "We do not use any user inputs or generated content to train our models without notifying to the user in the user interface."

**[FATO: https://www.pixellab.ai/docs/faq]** Resposta de FAQ sobre uso comercial: "Yes, we only ask that you do not train new models with the images."

### 6.1 Leitura para o GusWorld (FOSS) — minha analise, marcada como INFERENCIA onde aplicavel

- **[INFERENCIA]** O usuario (lider) parece ser dono pleno dos direitos autorais sobre as imagens geradas (sprites, tiles, animacoes), podendo usar, modificar e distribuir livremente, inclusive comercialmente. Isso e compativel com distribuir os ASSETS GERADOS dentro de um jogo FOSS.
- **[FATO]** A unica restricao explicita e clara e: nao usar as imagens geradas para **treinar outros modelos de IA** sem autorizacao por escrito da PixelLab. Isso NAO impede distribuir os assets como parte de um jogo, incluindo em repositorio publico.
- **[FATO]** Uso programatico (API) e permitido apenas "atraves da API oficial" e para os fins descritos (vibe coding / criacao de asset ao vivo dentro do jogo); usos como "construir seu proprio servico" ou "revender" pedem contato previo com a PixelLab. **[INFERENCIA]** Isso sugere que usar a API para gerar assets DURANTE O DESENVOLVIMENTO do GusWorld (offline, appended aos arquivos do jogo) e o uso pretendido e permitido; ja **expor a API do PixelLab dentro do jogo publicado, repassando chamadas de jogadores para a API do PixelLab usando o token do lider, seria mais proximo de "revenda/servico proprio" e mereceria contato previo com a PixelLab antes de lancar**, para evitar problema de ToS.
- **[NAO DOCUMENTADO]** Nao encontrei clausula explicita sobre "licenca de distribuicao de ASSETS dentro de repositorio de codigo aberto (FOSS)" versus "distribuicao apenas do jogo compilado". O ToS fala em termos gerais de "content you create" / "outputs", sem tratar especificamente do caso de um repositorio de codigo publico versionando os arquivos de imagem gerados junto do codigo-fonte do jogo. **[INFERENCIA]** Como o usuario detem os direitos autorais e pode "usar, modificar e distribuir... para qualquer proposito", distribuir os PNGs gerados dentro do repo do jogo (FOSS) parece coberto, mas recomendo ao lider uma leitura direta e completa do `termsofservice` (nao apenas os trechos que a ferramenta de busca extraiu) antes de versionar assets gerados por IA num repositorio publico do jogo, dado que ele mesmo definiu (memoria global do usuario) que codigo gerado por IA foi motivo de saida do Codeberg — o mesmo cuidado de procedencia pode valer para ASSETS gerados por IA num projeto que ele pretende manter FOSS/publico.

## 7. Custos, limites e formatos (consolidado)

**[FATO, direto do `openapi.json` + exemplo oficial]**:
- Custo por chamada e cobrado em **USD**, devolvido no campo `usage.usd` de cada resposta de geracao. Exemplo oficial no proprio spec: geracao pixflux ~ **USD 0,01**.
- **[FATO: pagina `pixellab.ai/pixellab-api`]** Exemplos adicionais de preco citados na pagina de marketing: imagens 64x64 ~USD 0,007-0,008; operacoes "Pro" 256x256 ~USD 0,095. Ha aviso explicito na pagina de que "os precos sao estimativas e variam conforme o tempo de processamento de GPU" **[FATO]** — ou seja, **o custo NAO e fixo por chamada**, varia com o tempo de GPU consumido.
- **Saldo atual da conta do lider: USD 0,00** (verificado nesta sessao, secao 4). Sera necessario adicionar creditos antes de qualquer chamada de geracao funcionar.
- **Rate limit**: a doc formal (`openapi.json`) so define os codigos HTTP `429` ("Too many requests") e `529` ("Rate limit exceeded") como possiveis respostas de erro, **sem numero exato de requisicoes por minuto/segundo documentado em lugar nenhum que eu tenha encontrado** [NAO DOCUMENTADO]. Ter dois codigos distintos de "rate limit" (429 e 529) e incomum e sugere dois mecanismos de limite diferentes (ex.: limite por usuario vs. limite de capacidade do backend de geracao/GPU), mas isso e **[INFERENCIA]** minha, nao confirmada em texto.
- **Formatos de imagem**: entrada e saida sempre em **Base64Image**: `{"type":"base64","base64":"data:image/png;base64,..."}` — ou seja, o formato binario e **PNG** codificado em base64 com data-URI embutida (`data:image/png;base64,...` no exemplo oficial). **[FATO]** Nao ha suporte documentado a outro formato de saida (ex.: GIF, sprite-sheet unico, JSON de metadados separado da imagem) alem de PNG base64 por frame.
- **Tamanhos maximos por endpoint** (resumo consolidado da secao 3):
  | Endpoint | Tamanho |
  |---|---|
  | pixflux | 16-400 px por lado (area minima textual: 32x32; area maxima textual: 400x400) |
  | bitforge | 16-200 px por lado |
  | animate-with-skeleton | 16-256 px por lado, mas textualmente restrito a {16,32,64,128,256} |
  | animate-with-text | fixo 64x64 |
  | rotate | 16-200 px por lado, textualmente restrito a {16,32,64,128} |
  | inpaint | 16-200 px por lado |
  | estimate-skeleton | 16-256 px por lado, restrito a {16,32,64,128,256} |
- **O que a API FAZ, exatamente** (resposta direta ao pedido do lider): gera **imagem estatica pixel-art a partir de texto** (pixflux) ou **a partir de texto + imagem de estilo de referencia** (bitforge); **rotaciona** um personagem/objeto existente para outra vista/direcao (rotate); **anima** um personagem em 4 frames a partir de pose de esqueleto (animate-with-skeleton) ou a partir de descricao textual de acao (animate-with-text); **edita/repinta uma regiao** de uma imagem existente via mascara (inpaint); **estima o esqueleto/pose** de um personagem numa imagem (estimate-skeleton, endpoint auxiliar para alimentar animate-with-skeleton); e **consulta saldo** (balance). **Nao ha** endpoint dedicado a: geracao de tileset/mapa (isso e feito hoje pelas ferramentas MCP de mais alto nivel `create_tileset`/`create_isometric_tile`, que provavelmente compoem os endpoints basicos, mas nao existe um path REST publico documentado `/generate-tileset` no `openapi.json` atual), remocao de fundo isolada (existe apenas como flag `no_background`/"transparent background" dentro dos endpoints de geracao, nao como servico isolado sobre uma imagem ja existente), upscale isolado, ou extracao/aplicacao de paleta como servico separado (existe apenas como `color_image`, referencia de paleta, dentro dos endpoints de geracao).
- **[FATO, com ressalva]** A pagina de marketing `pixellab.ai/pixellab-api` menciona um terceiro modelo de geracao chamado **"Pixen"** (junto de Pixflux e Bitforge), com limite citado de **512x512**. **Esse endpoint NAO aparece no `openapi.json` atual** (`/v1/openapi.json` so lista pixflux e bitforge como geradores de imagem estatica). **[INFERENCIA]** Isso sugere que "Pixen" e um modelo disponivel no app web/editor, mas **ainda nao exposto na API publica v1** documentada, ou que a pagina de marketing esta desatualizada/inclui recursos em rollout. Recomendo confirmar diretamente com o lider ou re-checar o `openapi.json` no futuro antes de assumir que Pixen esta disponivel via API.


## 8. Encaixe no pipeline de arte 2D pixel-art do GusWorld [INFERENCIA, minha analise]

### 8.1 O que DA para automatizar
- **Sprite sheet de personagem em N direcoes**: gerar 1 pose base via `pixflux` (texto) ou `bitforge` (texto + imagem de estilo de referencia, para manter consistencia com a arte ja existente do jogo), depois usar `/rotate` repetidamente para cobrir as 8 direcoes do enum `Direction` (north, north-east, east, south-east, south, south-west, west, north-west). Custo: 1 chamada de geracao + ate 7 chamadas de rotacao por personagem.
- **Animacoes de caminhada/acao**: via `animate-with-text` (mais simples, so texto + imagem de referencia, sempre 64x64, sempre 4 frames por chamada) ou `animate-with-skeleton` (mais controlavel, exige preparar poses de esqueleto para 3 frames de entrada e devolve 4 frames, usando `estimate-skeleton` primeiro para extrair a pose base do personagem de referencia).
- **Props/itens estaticos**: `pixflux` (do zero) ou `bitforge` (casando estilo com um item de referencia ja existente no jogo, via `style_image`).
- **Icones de carta**: mesmo caminho de props (`pixflux`/`bitforge`), possivelmente com `no_background=true` para fundo transparente, area ate 400x400 (pixflux) ou 200x200 (bitforge).
- **Edicao pontual de sprite existente** (ex.: adicionar acessorio, corrigir detalhe): `/inpaint` com mascara.
- **Paleta consistente**: campo `color_image` (paleta forcada) esta disponivel em praticamente todos os endpoints de geracao — util para manter a paleta de cores do GusWorld coerente entre chamadas diferentes.

### 8.2 O que NAO da para automatizar (ou nao esta documentado)
- **Tileset/tilemap completo de terreno** (ex.: grade Wang-tile de grama/agua/pedra encaixando nas bordas): **nao existe endpoint REST publico documentado** para isso; a ferramenta `create_tileset` so aparece do lado do MCP hospedado, sem contrato de API aberto equivalente em `/v1/openapi.json` — **[NAO DOCUMENTADO]** como ela realmente funciona por baixo, ou se seria preciso compor manualmente via chamadas repetidas de pixflux/bitforge + inpaint nas bordas.
- **Animacao com mais de 4 frames numa unica chamada**: cada chamada de `animate-*` devolve exatamente 4 frames; animacoes mais longas exigem encadear chamadas (usando `start_frame_index` e `inpainting_images` da chamada anterior), o que e **[INFERENCIA]** minha (nao um fluxo documentado passo a passo em texto corrido).
- **Consistencia garantida de estilo entre chamadas independentes**: sem usar `init_image`/`style_image`/`color_image` como ancora, duas chamadas separadas de `pixflux` para "personagem A" e "personagem B" podem sair com paleta/proporcao/nivel de detalhe visualmente incompativeis entre si. Os campos `outline`/`shading`/`detail`/`view` sao explicitamente descritos como "weakly guiding" no `pixflux` (nao sao garantia rigida).
- **Determinismo total**: o campo `seed` existe e permite reproducao (mesmo seed + mesmos parametros tende a reproduzir resultado semelhante), mas isso **[INFERENCIA]** nao e garantia formal de bit-exact documentada — e um modelo de geracao de imagem, nao uma funcao pura determinista no sentido estrito de software.

### 8.3 Pegadinhas identificadas para o pipeline do GusWorld
1. **Dependencia de rede em tempo de build**: qualquer script que gere assets via PixelLab **exige internet e o servico da PixelLab no ar** no momento da geracao. Para um pipeline de build reprodutivel (CI, build offline), os assets gerados devem ser **baixados uma vez e commitados/versionados como binarios finais** (PNG), nunca gerados on-the-fly durante o build do jogo. **[INFERENCIA]**, decorre diretamente do modelo de API paga por chamada.
2. **Custo acumulado**: com saldo atual de USD 0,00, e custo por chamada nao-fixo (varia com tempo de GPU), gerar um elenco completo de personagens x 8 direcoes x N animacoes pode acumular custo real não trivial; recomendo o lider orcar antes de rodar geracao em lote.
3. **Licenca dos outputs vs. natureza FOSS do projeto**: ver secao 6. Resumo: uso e distribuicao dos PNGs gerados parecem cobertos pelo ToS (usuario detem direitos), mas **usar a API dentro do jogo publicado repassando chamadas de terceiros (jogadores) pelo token do lider e um uso que o ToS pede para "entrar em contato antes"** — isso so importa se o GusWorld pretender gerar assets EM TEMPO DE EXECUCAO para o jogador final, nao para producao de arte durante o desenvolvimento.
4. **Vazamento do token**: se algum script de geracao for commitado no repositorio (que sera FOSS/publico), o token **NUNCA** deve ser hardcoded; deve vir sempre de variavel de ambiente (`PIXELLAB_MCP_BEARER`) fora do repo, conforme ja e o padrao usado pelo lider.
5. **MCP oficial e remoto, nao local**: nao ha um binario/pacote para instalar localmente e auditar; o servidor MCP oficial e um endpoint HTTPS hospedado pela PixelLab (`api.pixellab.ai/mcp`). Isso significa que usar o MCP (em vez da API REST direta) implica **confiar na PixelLab tambem como executor do protocolo MCP**, nao so como provedor de modelo — nao ha "modo local/offline" documentado para o MCP do PixelLab. **[INFERENCIA]** Para o GusWorld, chamar a API REST diretamente (via `curl`/cliente Python `pixellab`) da o mesmo resultado com uma camada a menos de dependencia (sem depender do endpoint `/mcp` adicional), a nao ser que o valor de orquestrar via MCP dentro do Claude Code justifique a camada extra.
6. **Discrepancia schema tecnico vs. texto da doc**: em varios endpoints (`animate-with-skeleton`, `rotate`, `estimate-skeleton`), o *schema* JSON aceita um range continuo de largura/altura (ex.: 16-256), mas o *texto* da descricao lista apenas um conjunto FECHADO de tamanhos suportados (16/32/64/128/256). **[INFERENCIA]** Enviar um tamanho fora dessa lista fechada (ex.: 100x100) pode passar na validacao de schema (422) mas gerar resultado de qualidade inferior ou comportamento nao documentado — recomendo sempre usar os tamanhos exatos citados no texto, nao confiar so no min/max do schema.

## 9. Resumo executivo (para o lider)

- A API do PixelLab (`api.pixellab.ai/v1`) tem **8 endpoints REST** documentados: geracao de imagem por texto (pixflux) ou por texto+estilo (bitforge), rotacao (rotate), animacao por esqueleto (animate-with-skeleton) ou por texto (animate-with-text), edicao com mascara (inpaint), estimativa de esqueleto (estimate-skeleton) e consulta de saldo (balance).
- Autenticacao: header `Authorization: Bearer <token>`. **Testado nesta sessao com sucesso** usando `$PIXELLAB_MCP_BEARER` (HTTP 200 em `/balance`). **Saldo atual: USD 0,00** — sera preciso adicionar creditos antes de gerar qualquer imagem.
- Existe **servidor MCP oficial**, mas e **hospedado remotamente** (`https://api.pixellab.ai/mcp`), nao um pacote local; expoe 4 ferramentas de alto nivel (criar personagem, animar personagem, criar tileset, criar tile isometrico) que, presumivelmente, compoem os 8 endpoints REST por baixo (nao confirmado em texto).
- **Licenca dos outputs**: o usuario detem os direitos autorais integrais e pode usar/modificar/distribuir livremente (inclusive comercialmente); a unica restricao clara e nao usar as imagens para treinar outros modelos de IA sem autorizacao. Compativel, na minha leitura, com um jogo FOSS versionando os PNGs gerados — mas recomendo o lider ler o ToS completo antes de decidir, dado o cuidado ja demonstrado com procedencia de conteudo gerado por IA em repositorios publicos.
- Serve bem para: sprite de personagem em multiplas direcoes (via rotate), animacoes curtas de 4 frames (walk/idle/etc), props, icones, edicao pontual. Nao ha endpoint REST publico documentado para tileset/mapa completo (so existe do lado do MCP, sem contrato aberto). Rate limit exato nao e documentado numericamente (so os codigos de erro 429/529). Custo varia por tempo de GPU, nao e fixo.
