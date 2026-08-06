// SPDX-License-Identifier: Apache-2.0
// gus/app/screens/battle_ui_sfx.hpp
//
// SFX-COCKPIT: a REGRA JA CANONIZADA pelo lider - "todo menu com botoes tem som de hover
// e de clique" - aplicada as TRES superficies clicaveis do cockpit de batalha, nos DOIS
// canais de entrada (mouse E teclado).
//
// O DEFEITO QUE ESTE MODULO FECHA (auditoria independente, 2026-08-06): das 3 superficies
// de selecao-com-realce do combate, so UMA soava.
//   pills de verbo   -> hover/clique de MOUSE soavam; de TECLADO, nao.
//   mira (inimigo)   -> nada soava (nem mouse, nem teclado) - grep play_sfx
//                       battle_input.cpp devolvia ZERO.
//   picker de ator   -> idem.
// Quem joga de TECLADO (caminho primario, controles remapeaveis) ouvia blip em todos os
// outros 6 menus do jogo e SILENCIO na batalha - regressao de acessibilidade.
//
// POR QUE UM MODULO PROPRIO (e nao mais um punhado de linhas no host):
//   1. ADR-020 (peca componivel em MODULO ESTREITO): assunto novo (som de UI do cockpit)
//      nasce em modulo proprio, nao acretado na casca gigante de battle_preview.cpp.
//   2. TESTABILIDADE - o motivo forte. Enquanto o `play_sfx` do hover morava dentro de
//      BattleScreen (classe em namespace ANONIMO dentro de battle_preview.cpp, exigindo
//      janela + contexto GL + glintfx::UiLayer vivos pra existir), NENHUM teste conseguia
//      alcanca-lo: apagar a chamada deixava a suite inteira VERDE. Aqui, todo `play_sfx`
//      de UI da batalha mora numa classe POCO que so precisa de um
//      gus::platform::audio::AudioEngine(device_active=false) - a mesma prova headless e
//      objetiva (sfx_play_count(), nao julgamento visual) que
//      save_load_menu_interaction_test.cpp ja usava.
//
// DUAS FONTES DE HOVER, de proposito (nao e duplicacao):
//   (a) FOCO (battle_focus_of + on_focus_change): "qual item esta REALCADO agora". Serve
//       aos 3 modos dirigidos pelo MOTOR (menu de verbos, mira, picker) e cobre teclado E
//       mouse de mundo no MESMO choke-point - identica receita do save_load_menu_loop.cpp
//       (indice de foco ANTES x DEPOIS do roteamento). E' a peca que da PARIDADE
//       teclado x mouse de graca.
//   (b) GEOMETRIA (on_pill_motion): os pills de verbo NAO tem cursor proprio movido pelo
//       mouse (o :hover deles e nativo do RmlUi e nao toca o motor) - passar o mouse sobre
//       um pill nao muda menu().selected_index(), entao (a) nao os enxerga. Para eles o
//       hit-test e geometrico, com edge-detect sobre as caixas do glintfx. Comportamento
//       PRESERVADO byte a byte do que ja existia em BattleScreen::handle_cockpit_hover_.
//   As duas fontes NUNCA soam pro mesmo gesto: (b) so vale na superficie VerbMenu, onde
//   (a) nao muda de indice por movimento de mouse.
//
// Cross-ref: gus/app/screens/battle_input.hpp (roteamento que estas funcoes observam);
//            gus/app/screens/ui_hover.hpp (edge-detect POCO reusado aqui);
//            gus/app/screens/battle_preview.hpp (host que instancia BattleUiSfx);
//            app/tests/battle_ui_sfx_test.cpp (suite headless desta camada);
//            docs/tech/adr/ADR-020 (peca componivel no nivel de modulo).

// ZERO SDL NESTE HEADER, de proposito: o gate `sdl-ratchet` (tools/check.sh) e um teto
// que SO CAI - hoje 24 arquivos de producao de app/ ainda tocam SDL3 direto, e a divida
// esta em plano (docs/tech/plano-camadas-sdl.md). Um modulo NOVO nao pode subir esse
// numero. Por isso o predicado de TECLADO (battle_key_activates_button, que precisa de
// SDL_Keycode) mora em gus/app/screens/battle_input.hpp - arquivo que JA toca SDL, ao
// lado dos irmaos battle_digit_for_key/battle_key_down, que e' tambem o lugar
// semanticamente certo pra ele.

#ifndef GUS_APP_SCREENS_BATTLE_UI_SFX_HPP
#define GUS_APP_SCREENS_BATTLE_UI_SFX_HPP

#include "gus/app/screens/battle_scene.hpp"
#include "gus/app/screens/ui_hover.hpp"
#include "gus/platform/audio/audio_engine.hpp"

namespace gus::app::screens {

// QUAL superficie de selecao-com-realce esta interativa AGORA. Exatamente as 3 que a
// auditoria enumerou, mais o None de "nenhuma" (abertura parada, turno do inimigo,
// animacao em voo, combate acabado).
enum class BattleFocusSurface {
    None,
    ActorPicker,  // §4.1 estagio 1: LISTA de membros elegiveis da party (badges na arena)
    Aiming,       // §3.5: cursor de mira entre os inimigos vivos
    VerbMenu,     // pills de verbo do cockpit (SCAN/ATACAR/...)
};

// "Que item esta realcado, em que superficie". Identidade do item:
//   VerbMenu    -> `index` (0..kBattleVerbCount-1); `item` fica nulo.
//   Aiming      -> `item` (ponteiro do CombatActor mirado); `index` fica -1.
//   ActorPicker -> `item` (ponteiro do CombatActor sob o cursor); `index` fica -1.
// Por que PONTEIRO e nao indice nos dois ultimos: a BattleScene nao expoe o indice do
// cursor de mira nem o do picker (so aim_target()/actor_pick_target()), e adicionar
// getter la esta FORA do escopo desta fatia. O ponteiro e' identidade suficiente e
// ESTAVEL (os CombatActor vivem na FSM enquanto o combate dura) - e ainda e' MAIS
// correto que o indice: se a lista de candidatos encolher (inimigo morreu) o mesmo
// indice pode virar outro ator, e o ponteiro nao mente sobre isso.
struct BattleFocus {
    BattleFocusSurface surface = BattleFocusSurface::None;
    int index = -1;
    const void* item = nullptr;
};

[[nodiscard]] bool operator==(const BattleFocus& a, const BattleFocus& b) noexcept;
[[nodiscard]] bool operator!=(const BattleFocus& a, const BattleFocus& b) noexcept;

// Le o foco corrente da cena. FUNCAO PURA de leitura (nao muta nada). A PRIORIDADE
// replica, na mesma ordem, a de battle_key_down/battle_mouse_click:
// abertura > picker > mira > menu de verbos.
[[nodiscard]] BattleFocus battle_focus_of(const BattleScene& scene) noexcept;

// EDGE-DETECT de foco: true quando o realce ENTROU num item NOVO da MESMA superficie -
// o unico caso em que o blip de HOVER deve soar. Deliberadamente false quando:
//   - a superficie MUDOU (menu -> mira, picker -> menu ...): isso e consequencia de um
//     CONFIRMAR/CANCELAR, que tem som proprio (clique) ou nenhum (Esc). Tocar hover ai
//     seria som duplo no mesmo gesto.
//   - a superficie de destino e None (nada realcado).
//   - o item nao mudou (mouse parado, tecla que nao navega) - mesma regra do
//     ui_hover_entered_new_item do POCO generico.
[[nodiscard]] bool battle_focus_entered_new_item(const BattleFocus& before,
                                                 const BattleFocus& after) noexcept;

// (O predicado do blip de CLIQUE no canal TECLADO - battle_key_activates_button - mora em
// gus/app/screens/battle_input.hpp, ao lado de battle_key_down/battle_digit_for_key: e'
// SDL_Keycode, e este header e SDL-free por causa do gate sdl-ratchet, ver o topo.)

// O UNICO lugar de todo o cockpit onde `play_sfx` de UI e chamado. O host so DECIDE
// QUANDO chamar; QUANTO soa (edge-detect, dedup, gate de superficie) e responsabilidade
// daqui - e' o que torna a regra testavel sem GL.
//
// Degrada em silencio (no-op) com audio nulo ou SoundId invalido: o jogo NUNCA depende
// de audio pra rodar (mesmo contrato de AudioEngine::play_sfx).
class BattleUiSfx {
public:
    BattleUiSfx() = default;

    // Ponteiro NAO-DONO (mesmo padrao de BattleScene::set_audio). Chamado uma vez por
    // entrada na batalha, depois que os 2 blips ja foram carregados.
    void bind(gus::platform::audio::AudioEngine* audio,
              gus::platform::audio::SoundId hover_sfx,
              gus::platform::audio::SoundId click_sfx) noexcept;

    // HOVER por FOCO (teclado + mouse de mundo). Toca 1 blip SE (e so se)
    // battle_focus_entered_new_item(before, after). Devolve se tocou (pros testes e pro
    // self-test sintetico).
    bool on_focus_change(const BattleFocus& before, const BattleFocus& after);

    // HOVER por GEOMETRIA (pills de verbo). `boxes`/`count` sao as caixas do glintfx ja
    // convertidas pelo host. Edge-detect interno: N pills VARRIDOS = N blips; parado
    // sobre o mesmo pill = 0; sair pra fora e voltar = 1. Devolve se tocou.
    bool on_pill_motion(float mouse_x, float mouse_y, const UiHoverBox* boxes, int count);

    // Zera a memoria do pill hovered SEM tocar nada - chamado quando o menu de verbos
    // deixa de estar interativo (mira/picker/abertura/turno do inimigo), pra o realce nao
    // "prender" no ultimo pill e o proximo hover real voltar a soar.
    void reset_pill_hover() noexcept;

    // CLIQUE: 1 blip por acionamento de botao (pill de verbo, alvo de mira, membro da
    // party), em qualquer canal de entrada. Devolve se tocou.
    bool play_click();

    // Estado do edge-detect geometrico (leitura pros testes/diagnostico).
    [[nodiscard]] int hovered_pill() const noexcept { return hovered_pill_; }

private:
    bool play_(gus::platform::audio::SoundId id);

    gus::platform::audio::AudioEngine* audio_ = nullptr;
    gus::platform::audio::SoundId hover_sfx_ = gus::platform::audio::kInvalidSound;
    gus::platform::audio::SoundId click_sfx_ = gus::platform::audio::kInvalidSound;
    int hovered_pill_ = -1;
};

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_UI_SFX_HPP
