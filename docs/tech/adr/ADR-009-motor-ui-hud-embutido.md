# ADR-009: motor de UI/HUD embutido para o jogo (sobre __DEP_REMOVIDA__, dentro das 4 camadas)

**Status:** Superseded by [ADR-010](ADR-010-adopt-glintfx-embed-mode.md)
**Data:** 2026-06-25
**Decisores:** criador supremo (petrus) + software-architect (proposta)
**Cross-ref:** [ADR-008](ADR-008-repivot-qt-to-__DEP_REMOVIDA__.md) (repivot Qt->__DEP_REMOVIDA__, já previu esta camada na "fase 3"; este ADR-009 concretiza essa fase 3), [`docs/tech/pivot/engine-design.md`](../pivot/engine-design.md) (4 camadas, GATE), [`docs/design/mecanicas/battle-screen.md`](../../design/mecanicas/battle-screen.md) (Tático Cockpit, pacing 2-beats, font-free), [ADR-010](ADR-010-adopt-glintfx-embed-mode.md) (troca o COMO desta decisão, não o QUÊ).

---

## Contexto

### O problema-raiz (dor real, não imaginária)

Os mocks de UI são HTML/CSS: gradientes, glow, `border-radius`, `box-shadow`, animações — tudo barato e lindo. O engine desenhava **primitivas __DEP_REMOVIDA__ cruas** via `IRenderer` (`draw_filled_rect`, `draw_rect_outline`, `draw_textured_rect`, `draw_text` glifo-a-glifo de um atlas). Resultado medido em playtest: a UI no jogo ficava sempre aquém do mock. O criador nomeou: *"você me ilude: mock lindo, jogo com qualidade menor"*. Uma dívida de fidelidade visual real, recorrente, com custo de retrabalho a cada tela.

A causa não era falta de talento de arte: reproduzir efeitos CSS (degradê radial, blur, sombra, transição, raio de canto) à mão em primitivas cruas é caro e nunca empata com um motor de layout/estilo dedicado.

### Constraints que moldaram a decisão

1. O **motor de combate** (`domain/combat/`) já estava portado/auditado e é POCO puro. **Não podia ser tocado.**
2. A **BattleScreen** (Tático Cockpit, variante C) estava em desenvolvimento avançado: pacing 2-beats funcional, HOLD de abertura espera-input, motor drenado por eventos, `PacingDirector` POCO testado. **Isso funcionava e não podia regredir.**
3. Havia **887 casos de teste** (headless) cobrindo `battle_layout` / `battle_scene` / `battle_menu` / `battle_hud_model` / `battle_log_model` / `battle_floaters` / `battle_pacing`. O ativo mais valioso. **Não podiam ser jogados fora num big-bang.**
4. A **arena 2D** (sprites de ator, fundo, floaters de dano) já rodava com pacing e ancoragem de pé, sobre a mesma fronteira `IRenderer`. **Funcionava.**
5. Já existia um atlas de fonte próprio, rasterizando Pixel Operator Mono (ASCII + Latin-1 pt-br). **A engine já resolvia fonte uma vez.**

---

## Decisão

**Adotar um motor de UI/HUD embutido em C++, com camada de estilo tipo CSS e documento tipo HTML, para a camada de UI/HUD/telas (chrome): cockpit, menu de verbos, fila CTB, banner, log/terminal, overlay de COMPILAR, brasão animado, telas de menu/resultado.** Vive **exclusivamente em `platform/` (fronteira sobre __DEP_REMOVIDA__) + `app/` (documentos de UI, data-binding, controllers de tela)**. `core/` e `domain/` permanecem POCO intocados; o GATE de 4 camadas continua válido.

**A ARENA 2D (sprites de ator, fundo, floaters) não migra: continua na fronteira de render 2D atual.** O motor de UI compõe por cima da arena (HUD overlay). Migração **faseada**, começando pelo cockpit como prova de conceito, sem quebrar pacing/motor/abertura.

Isto é uma **adição de fronteira**, não um redesign: a fronteira de render 2D continua dona dos pixels do mundo; o motor de UI assume o chrome. Espelha a divisão que o `engine-design.md` já propunha ("render de baixo nível = pixels do mundo, motor declarativo = chrome/menus").

---

## 1. Onde a camada de UI entra nas 4 camadas

```
core/      POCO puro            [INTOCADO]  zero UI, zero __DEP_REMOVIDA__
domain/    POCO puro            [INTOCADO]  combat/save/i18n/...  zero UI, zero __DEP_REMOVIDA__
   |  (estado do jogo: HP, AP, Mana, fila CTB, intents, log, mão de cartas)
   v   via view-models POCO (battle_hud_model / battle_log_model já existiam)
platform/  fronteira __DEP_REMOVIDA__
   +-- render2d/        fronteira de render 2D  -> ARENA (sprites, fundo, floaters)   [fica]
   +-- ui/    [NOVO]    interfaces de render/sistema/fonte do motor de UI sobre __DEP_REMOVIDA__
app/       GusWorld-specific
   +-- screens/         battle_scene  -> orquestra: arena (render2d) + HUD (motor de UI)
   +-- ui/    [NOVO]    documentos de UI (cockpit, ctb, banner, log...)
                        controllers que ligam o view-model POCO ao data-model do motor de UI
```

### Fluxo de dados domain -> app -> UI (sem violar camada)

A regra de ouro: **a UI lê, nunca escreve no domínio, e o domínio nunca conhece a UI.**

1. `domain/combat/` produz o estado (já fazia: HP/AP/Mana/fila/intent/log/dano).
2. Os **view-models POCO que já existiam** (`battle_hud_model`, `battle_log_model`, `battle_floaters`, `battle_layout`) são a fonte de verdade da apresentação. Já testáveis headless e já traduzem "estado do motor -> dados de tela".
3. Em `app/ui/`, um controller faz o data-model do motor de UI espelhar esses view-models: cada campo (`hp`, `ap_pips`, `active_name`, `log_lines`, `intent_icon`...) lê do view-model POCO.
4. Input do jogador: a UI captura o evento (clique no verbo, tap na carta) e dispara um **comando POCO** (o mesmo que o `battle_menu`/`battle_scene` já produzia), que vai ao motor. A UI não toca regra de jogo.

**GATE continua válido:** o motor de UI vive só em `platform/`+`app/`, nunca em `core/`+`domain/` — invariante auditada no CI.

---

## 2. Coexistência arena vs HUD

A composição adotada: arena desenha primeiro (painter's order), HUD compõe por cima, um único present por frame. Ambos na mesma superfície de render, sem render-to-texture — mais simples e mais barato do que separar em camadas com sync de textura.

**Por que a arena não migrou para o motor de UI:**
- A arena é **espaço de jogo** (sprites com ancoragem de pé, floaters com timing, animações de ataque/windup). Lógica de game-feel, não layout de documento.
- O pacing 2-beats e a fila de eventos vivem no `battle_scene`/`PacingDirector` POCO. Mover a arena para um motor declarativo forçaria reescrever isso. Risco alto, benefício zero.

---

## 3. Plano de migração FASEADO (anti big-bang)

Princípio: **a cada fase, o jogo continua jogável; nenhuma fase quebra pacing/motor/abertura.** A arena nunca para. A UI entra tela-a-tela: andaime -> cockpit (PoC) -> fila CTB/banner -> log/terminal -> overlay de COMPILAR -> telas de menu/resultado -> (opcional) floaters.

Gate anti-big-bang: a fase do cockpit é a prova. Só avança para a próxima depois que o cockpit no jogo provar paridade com o mock e os testes de pacing/motor continuarem verdes.

---

## 4. Impacto nos TESTES / GATE

A grande maioria dos ~887 testes (save, i18n, combat, progression, knowledge, sprites, pacing, hud_model, log_model, floaters) **não toca na camada de UI e fica intacta.** A UI é camada de apresentação; o motor e os view-models POCO seguem testados como antes.

**Estratégia de teste da UI (sem display):** testar o que alimenta a tela, não a tela em si — o data-model POCO (`battle_hud_model`/`battle_log_model`) é testável headless sem a UI; a UI só consome. Complementar com smoke de carga headless (documento parseia, elementos esperados existem, data-model liga).

---

## Consequências

**Positivas:**
- A UI/HUD parou de ficar aquém do mock: estilo declarativo entrega gradiente/glow/border-radius/shadow/transition de graça.
- Iteração de UI ficou mais barata (editar estilo, não recompilar lógica de desenho).
- A divisão "arena = pixels do mundo / motor de UI = chrome" é limpa e já prevista no design (ADR-008).
- `core/`/`domain/` intocados; GATE preservado.
- Motor e pacing intocados; ~887 testes em sua quase totalidade ficam.

**Negativas / aceitas como custo:**
- +1 dependência compilada de framework de UI.
- Trabalho de migração faseada em múltiplas fases.
- Uma fração dos testes de geometria fina de HUD foi aposentada (compensada por view-model POCO + smoke de carga).

## Reversibilidade

**Hybrid.** A UI vive atrás de view-models POCO; se o motor de UI não servisse, voltava-se à fronteira de render 2D pro chrome (os view-models POCO continuam válidos, não dependem da UI). O que é **one-way** é o esforço afundado de reescrever as telas nos formatos declarativos do motor escolhido. Por isso a fase de PoC (cockpit) existiu: provar a reversibilidade barata antes de afundar custo.

## Nota de superação

Esta decisão foi **substituída em substância pelo ADR-010**: o COMO passou a ser consumir o motor de UI indiretamente, através do glintfx (biblioteca de UI própria do ecossistema, em modo embutido), em vez de integrar um motor de terceiro diretamente em `platform/`. O QUÊ (motor de UI/HUD declarativo, camada separada da arena, view-models POCO como fonte de verdade) permanece vigente.
