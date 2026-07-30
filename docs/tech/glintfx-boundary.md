# Fronteira glintfx ⇄ GusWorld — o que é framework, o que é jogo

**Diretriz do líder (2026-07-21):** repassar ao framework **glintfx** _todas as funções possíveis_ — lido como **todas as funções de framework 2D genérico**. O GusWorld consome; não reimplementa nem internaliza dependência que caberia no glintfx. Complementa o [freeze de framework](../../CLAUDE.md) e a memória `project_framework_freeze_glintfx`.

## A régua (o filtro único)

> **Essa função serve a QUALQUER jogo 2D (genérica/reutilizável) ou é regra específica do GusWorld?**
>
> - **Genérica → é do glintfx.** Vira PROMPT pro glintfx; **não implemento aqui**; consumo pela API dele.
> - **Específica do jogo → é do GusWorld.** `domain/` (regra POCO) ou `app/` (tela/gameplay).

**Ressalva que protege a fronteira nos dois sentidos:** a regra do jogo **NUNCA** vai pro glintfx. Se combate/cartas/save-crypto/lore vazam pro framework, ele deixa de ser genérico e vira acoplamento reverso (o framework passa a conhecer o jogo). "Todas as funções possíveis" = todo o **genérico**, nada do **específico**.

## Matriz de decisão

| Vai pro **glintfx** (framework 2D genérico) | Fica no **GusWorld** (regra do jogo) |
|---|---|
| Janela, loop principal, timing/frame, App mode | Telas do jogo, fluxo de gameplay |
| Render 2D, sprite/tilemap render, câmera 2D, VFX/partículas genéricas | O que/onde desenhar (conteúdo, level design) |
| Input (teclado/mouse), **gamepad** (v0.15.0) | Mapeamento de ação → intenção de jogo (combate/menu) |
| Áudio (mixer, playback, streaming) | Quais músicas/SFX, música adaptativa por cena |
| Fonte, texto, layout de UI (RmlUi embed) | HUD/cockpit de batalha, conteúdo das telas |
| Carregamento de asset, fs, math 2D, colisão/spatial genérica | Save-crypto (AEAD do save do jogo), motor de cartas, progressão, economia |
| i18n loader (parser/fallback) genérico | Chaves i18n e conteúdo das traduções |

> **Exceção ratificada (líder 2026-07-21): o i18n NÃO migra pro glintfx.** O translator do GusWorld vive em `domain/` (POCO puro, com os tiers de censura = regra de jogo), e o gate de CI proíbe `<glintfx` em `core/`/`domain/`. Migrar furaria a arquitetura de 4 camadas. Coexistência permanente: o módulo i18n do glintfx fica desligado; o translator próprio (pt_br/en_intl) permanece a fonte.

## O gate — quando dispara

Não é marco de calendário: é **checkpoint por onda**, no **planejamento (Caetano/CTO)**, disparado pela **fronteira `platform/`**. Roda **ANTES** de qualquer uma destas três ações:

1. Adicionar uma **dependência de terceiros** nova (`FetchContent` / vendor em `third_party/`);
2. Escrever código em **`platform/`** que fala com SDL/GL/áudio/input/janela/fs;
3. Reimplementar algo que "parece utilitário de engine".

Aplicar a régua no ponto. Se der "genérico" → **não internaliza**: manda o PROMPT/dor pro glintfx pelo bus (reportar ao glintfx é o propósito do bus, não pede permissão; só **design** do glintfx passa pelo líder) e consome quando a fatia sair (bump do pin). Motivo do gate ser no planejamento: uma vez internalizada, extrair a dep depois é o retrabalho caro que o freeze existe pra evitar.

## Passivo conhecido (candidatos ainda internalizados)

- **Casca SDL própria (janela + loop)** → migrar pro **App mode do glintfx** (v0.12.0+, `set_frame_callback`). É a maior dependência direta de SDL ainda no GusWorld; alvo natural de uma onda "input/casca via glintfx" quando o roadmap chegar lá.

(A auditoria completa das ~32 libs vendorizadas em `third_party/` — quais migram pro glintfx — foi deixada fora deste doc leve; roda sob demanda se o líder pedir.)

## Lacunas do glintfx que estamos CONTORNANDO (regra do líder, 2026-07-29)

**Ordem do líder, verbatim:** *"nao crie maneira de contornar deficiências no glintfx. Se encontrar alguma, pause, documente e encaminhe ao glintfx via bus"*.

**Por quê:** um contorno local resolve o sintoma do nosso lado e **apaga o sinal** que faria a lacuna ser consertada na origem. Fica funcionando, ninguém revisita, e o custo reaparece inteiro no próximo consumidor. É o mesmo mecanismo do `cell_px=16` (ver `D2D-2-SENTINELA`): alguém resolveu localmente uma vez, funcionou, e ninguém voltou por meses.

Esta seção é o registro vivo. Entrada nova aqui **exige** mensagem correspondente no bus — documentar sem encaminhar é meio caminho, e o meio caminho é o que cria passivo silencioso.

⚠️ **Duas categorias, e a segunda é mais grave.** Lacuna **com contorno** custa duplicação e passivo. Lacuna **sem contorno possível** (porque o único contorno seria o workaround proibido) **BLOQUEIA a fatia**: o trabalho para e espera o fix. A coluna "contorno" diz qual é o caso.

| # | lacuna | o contorno que existe hoje | custo | encaminhado |
|---|---|---|---|---|
| L3 | **`decode_image_file` lança `std::bad_alloc`, contra o contrato que o `image.hpp` deles publica em maiúsculas** (*"`ok == false` is the ONLY failure signal [...] never a crash"*). Medido com `ulimit -v 950000`: o `stbi_load` cru carrega um PNG **paletizado** grande e o `decode_image_file` lança, e a exceção atravessa a fronteira da API (o `try` da sonda envolve só a chamada deles). Causa: **duas cópias completas do buffer** onde a via crua tinha zero — um `assign` sobre buffer já alocado (`image_decode.hpp:380`) mais um `std::move` que liga na sobrecarga de **cópia** porque o objeto é `const` (`image.cpp:33`/`:39`, com o comentário do arquivo afirmando *"never a copy"*). Pico **+33%** em paletizado; em RGBA8 direto os picos **empatam**, e é por isso que a medição da fatia não viu. | **NENHUM, e é o problema.** O único contorno seria `try/catch` no nosso call-site, que é exatamente o workaround proibido. **A fatia está BLOQUEADA nesta parte:** o `IRenderer::load_texture` tem contrato de degradar e não consegue cumpri-lo. | Sem o fix, PNG paletizado grande sob pressão de memória vira `SIGABRT` em vez de textura inválida. Paletizado é formato corrente de pixel-art. | 2026-07-30 |
| L4 | **A superfície de formato do decode não é controlável pelo consumidor.** O `stb_image_impl.cpp` deles compila `STBI_ONLY_PNG` **+ `JPEG` + `TGA`**; o nosso código antigo compilava só PNG. O `tga_test` do stb é frouxo, **tolera buffer truncado** preenchendo com zeros, e é o **último** formato testado — o coador que recolhe o que os outros rejeitaram. Medido: **82 bytes** (header TGA plausível + ruído) viram textura **4096×4096**. | **NENHUM.** Validar assinatura de arquivo antes de chamá-los seria contornar. Pedimos **controle** (allowlist de formatos, variante por formato, opção de build) ou, no mínimo, documentação da superfície real. | Prova ponta-a-ponta: 7 frames trocados por arquivos de 82 B = **+446 MiB de RSS a partir de 574 bytes**, "97 frames" carregados, **zero aviso**, e o sprite **desaparece sem fallback** porque o handle é **válido** apontando para alpha zero. Antes: erro visível e depurável. Depois: silêncio. Colide com "todo efeito loga no terminal". | 2026-07-30 |
| ~~L1~~ | ~~**Não há `flush()` público no `Draw2d`**; a única forma de esvaziar o batcher era `end()`, que fecha o bracket inteiro. Na v0.23.0, `flush` aparecia 17× no `draw2d.hpp`, **todas em comentário**, nenhuma declaração.~~ | ~~O bracket `end()`/`begin()` em volta do texto, para preservar a ordem de pintura entre draws batchados e texto cru.~~ | **RESOLVIDA 2026-07-29/30.** Eles entregaram `flush()` público (`draw2d.hpp:1206`, declaração real, conferida fora de comentário) e o nosso bracket foi removido (`8ad8ee0`; `render2d_glintfx.cpp:245` registra *"não há mais bracket"*). Mantida na tabela porque **é o caso de sucesso do fluxo**: pedimos em vez de contornar, e o contorno morreu junto com a lacuna. | 2026-07-29 |
| L2 | **O motor de fonte próprio colapsa os acentos pt-br em corpo miúdo (8px).** `ÁÉÍÓÚÇ` saem numa faixa ilegível. | Não usamos `Draw2d::draw_text`; mantivemos pipeline GL própria com stb_truetype (duplicando a receita do `Render2dGl3`). | Duplicação de pipeline de texto, e o Draw2D fica com uma primitiva pública inutilizada por nós. | 2026-07-29 |

Prova visual de L2: `docs/tech/fontflip-visuals/triplet_1x_08_cockpitText_menor.png` (linha 1 = motor deles) e o dossiê `docs/tech/fontflip-draw2d-dossie.md`.

⚠️ **O que NÃO é lacuna deles** e não entra aqui: defeito nosso que por acaso aparece na fronteira. O `cell_px=16` era nosso, o bake é nosso, o stb_truetype é nosso — consertamos em casa, sem pedir nada. A pergunta que separa os dois casos: **o código problemático é nosso, ou é a ausência de uma API deles?**
