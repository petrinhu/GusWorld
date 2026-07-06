// gus/app/src/screens/npc_dialogue_loop.cpp
//
// Implementacao do loop interativo do dialogo do NPC. Ver header. Irredutivel (poll
// de SDL_Event + desenho) - sem unidade de teste direta, mesmo racional de
// run_battle_preview_embedded/run_system_menu_loop_gl_current: a logica PURA
// testavel ja fica em npc_dialogue_overlay.hpp/.cpp (apply_npc_dialogue_input,
// npc_dialogue_overlay_lines), exercitada headless em npc_dialogue_overlay_test.cpp.

#include "gus/app/screens/npc_dialogue_loop.hpp"

#include <SDL3/SDL.h>

#include "gus/app/screens/npc_dialogue_overlay.hpp"

namespace gus::app::screens {

bool run_npc_dialogue_loop(gus::app::SdlWindow& city,
                            gus::domain::dialogue::DialogueRuntime& runtime,
                            const gus::app::i18n::Translator& translator) {
    int selected_option = 0;

    // Desenha o frame INICIAL (no de entrada, ja posicionado por runtime.enter() no
    // chamador) antes do 1o poll - senao a caixa so apareceria apos a 1a tecla.
    city.render_dialogue_overlay_frame(
        npc_dialogue_overlay_lines(runtime.current(), translator, selected_option));

    while (!runtime.finished()) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) {
                return true;  // fechou a JANELA - mesmo contrato de to_battle()
            }
            if (ev.type != SDL_EVENT_KEY_DOWN || ev.key.repeat) {
                continue;
            }
            NpcDialogueInputAction action = NpcDialogueInputAction::None;
            switch (ev.key.key) {
                case SDLK_UP:
                case SDLK_W:
                    action = NpcDialogueInputAction::MoveUp;
                    break;
                case SDLK_DOWN:
                case SDLK_S:
                    action = NpcDialogueInputAction::MoveDown;
                    break;
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                case SDLK_SPACE:
                    action = NpcDialogueInputAction::Confirm;
                    break;
                default:
                    break;
            }
            if (action == NpcDialogueInputAction::None) {
                continue;
            }
            selected_option = apply_npc_dialogue_input(runtime, action, selected_option);
            if (runtime.finished()) {
                break;
            }
            city.render_dialogue_overlay_frame(npc_dialogue_overlay_lines(
                runtime.current(), translator, selected_option));
        }
    }
    return false;  // conversa encerrada (@exit) - a cidade retoma de onde estava
}

}  // namespace gus::app::screens
