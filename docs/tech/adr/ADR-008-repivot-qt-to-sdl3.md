# ADR-008: Re-pivot da camada de plataforma Qt6 -> SDL3 + RmlUi + miniaudio

Status: Accepted
Data: 2026-06-22
Decisores: lider supremo (petrus), software-architect (Caetano/CTO)
Supera: a escolha "Qt RHI + Qt Quick" do `docs/tech/pivot/engine-design.md` (so a camada de plataforma; o resto do engine-design segue valido).

## Contexto

O pivot Godot/C# -> C++/Qt6 (engine-design.md, 2026-06-21) escolheu Qt6 como camada de plataforma: Qt RHI pro mundo + Qt Quick/QML pra UI. Com o M1 (janela + render2d + input + 1 sprite animado) entregue em Qt, o lider levantou a SDL como alternativa. Analise de stack do software-architect (eixo a eixo) concluiu:

- **SDL3 vence em 7-8 de ~10 eixos.** Unico eixo onde o Qt ganha de verdade = UI (Qt Quick), mas e "poder que nao precisamos": arrasta o runtime QML inteiro, duplica o i18n (ja temos o nosso loader `.md`), e mete uma linguagem extra (QML) num projeto C++20 puro.
- **Risco R1 (Qt RHI = API semi-privada)** desaparece: SDL tem render 2D publico e estavel (`SDL_Renderer`) + a SDL3 GPU API.
- **Gamepad**: o Qt6 REMOVEU o modulo QtGamepad; o SDL tem gamepad nativo de classe-AAA (hot-plug, DB de mapeamento, rumble).
- **Peso/distribuicao**: binario ~10x menor, empacotamento trivial, licenca zlib.
- **Portabilidade**: SDL e o ABI portavel pra mobile e CONSOLE (Qt nao vai pra console).
- **Custo do re-pivot e baixo AGORA**: a invariante das 4 camadas garante que `core/`+`domain/` (POCO puro, ~590 testes, auditado) NAO muda; so a fronteira `platform/` + a casca `app/` sao reescritas. E a UI (maior trabalho a frente, M5+) ainda NAO foi construida em QML, entao pivotar agora nao joga UI fora, so redireciona. A janela barata FECHA no M5.

## Decisao

1. **Camada de plataforma = SDL3** (janela, loop proprio, input + gamepad nativo, eventos). Render 2D via **`SDL_Renderer`** (suficiente e simples pra sprite/tilemap; GPU API fica como opcao futura).
2. **UI = RmlUi** (HTML/CSS-like, retido, data binding MVC + localizacao nativos; desenha pelo nosso `render2d`) pro JOGADOR; **Dear ImGui** como overlay de DEBUG/dev. Sem Qt Quick/QML.
3. **Audio = miniaudio** (ja vendorizado em third_party; roda em qualquer stack; aposenta o "RF-8 sobre Qt Multimedia").
4. **Logica pura intacta**: `core/`+`domain/` permanecem POCO sem framework. A invariante das 4 camadas passa a proibir Qt E SDL em core/domain (SDL so em platform/+app/).
5. **Reaproveitar o agnostico**: `IRenderer` (interface), `viewport_transform`, `sprite_animation`, `core/time`, `core/spatial`, `domain/input` (input_remap) sao POCO e ficam; so o backend de plataforma (Render2dRhi -> Render2dSdl, window/input Qt -> SDL) e reescrito.
6. **Build**: SDL3 e RmlUi entram via FetchContent/submodule com pin de versao (build reprodutivel Linux+Windows); nao sao header-only vendorizaveis como as 32 libs de third_party.

## Consequencias

Positivas: elimina o risco R1; gamepad de primeira classe; binario e deploy ~10x menores; licenca zlib/MIT; caminho aberto pra mobile/console; C++20 puro sem QML; i18n proprio sem redundancia; audio miniaudio (melhor que Qt Multimedia e SDL_audio). 

Negativas (aceitas): reescrever a fronteira do M1 (janela/render2d/input ja feitos em Qt) + as metades visuais de M2/M4 nascem em SDL; curva de integracao do RmlUi (renderer proprio); SDL3 e novo (jan/2025), mitigado por SDL2 ter 12 anos de campo e API estavel. Esforco estimado ~1-3 semanas SO na fronteira; zero na logica pura.

## Reversibilidade

One-way door de stack (como o pivot anterior). Mitigado por: (a) a logica pura nao se perde em nenhum cenario; (b) downgrade SDL3 -> SDL2 e trivial se preciso; (c) a interface IRenderer isola o backend (trocar render = 1 arquivo, mesma licao do R1). Releases ja publicadas nao existem (jogo em DEV).

## Execucao (faseada)

- **Fase 1 (M1-SDL):** SDL3+RmlUi no build; reescrever platform/window + render2d (SDL_Renderer) + input (+ gamepad); reintegrar o sprite do Caua andando; aposentar o backend Qt; smoke headless via `SDL_VIDEODRIVER=dummy`.
- **Fase 2:** metade visual de M2 (input/IO) e M4 (tilemap) nascem em SDL.
- **Fase 3 (M5+):** UI do jogador em RmlUi (menus, batalha estilo Pokemon, dialogo, inventario).
- Atualizar engine-design.md (camada de plataforma), README, CLAUDE.md e o board M0-M9 conforme as fases fecharem.
