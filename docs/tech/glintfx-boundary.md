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
