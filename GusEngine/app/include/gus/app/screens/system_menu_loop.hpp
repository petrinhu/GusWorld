// gus/app/screens/system_menu_loop.hpp
//
// LOOP INTERATIVO do MENU DE SISTEMA (pausa + config de som), MENU-PAUSA-CONFIG-SOM
// (M7-COSTURA, INTEGRACAO FINAL). Roda a maquina de estado pura de system_menu.hpp +
// o RML/RCSS de system_menu_rml.hpp numa glintfx::UiLayer PROPRIA (independente de
// qualquer HUD do chamador), num CONTEXTO GL. Aplica volume no AudioEngine em tempo
// real e PERSISTE em settings.json a cada mudanca (fundacao ja pronta: gus/platform/
// fs/settings_file_store.hpp).
//
// DUAS FORMAS DE ENTRAR (o contexto GL e MAIS CARO/dificil de possuir que de so
// USAR - a maioria dos chamadores ja tem um contexto corrente):
//   run_system_menu_loop_gl_current: ASSUME um contexto GL JA CORRENTE e com os
//     ponteiros de funcao (glad) JA CARREGADOS (o chamador e dono - cria/destroi por
//     fora). Uso: BATALHA (a battle_preview ja roda dentro do MESMO contexto GL -
//     Esc na pilha vazia do combate abre este loop ANINHADO, sem sair do contexto).
//   run_system_menu_loop_owning_gl: CRIA seu PROPRIO contexto GL na janela dada
//     (mesma receita/atributos de run_battle_preview_embedded - profile core 3.3,
//     double-buffer, stencil 8), carrega o glad, roda o loop, e DESTROI o contexto
//     ao sair. Uso: CIDADE (Render2dSdl puro, SEM GL nenhum - o CHAMADOR, a Maestro,
//     faz city_->release_renderer() ANTES e city_->reacquire_renderer() DEPOIS,
//     REUSANDO a MESMA tecnica de "trocar escondido atras do preto" ja provada
//     empiricamente pela troca cidade<->batalha, ver maestro.cpp::to_battle).
//
// FUNDO: deliberadamente ABSTRATO/ESTATICO (decisao do lider - o mock ja assume "o
// jogo pausado atras" sem exigir captura do frame real). Reusa Render2dGl3::
// begin_frame (clear + vinheta radial, MESMA ambientacao usada na arena) em vez de
// tentar capturar/borrar o frame de cidade/batalha - zero codigo novo de captura.
//
// Cross-ref: gus/app/screens/system_menu.hpp (a maquina de estado pura);
//            gus/app/screens/system_menu_rml.hpp (o RML/RCSS gerado do estado);
//            gus/app/screens/battle_preview.cpp (run_battle_preview_embedded, o
//            chamador da variante gl_current - Esc na pilha vazia do combate);
//            gus/app/maestro.cpp (o chamador da variante owning_gl - Esc na cidade);
//            gus/platform/fs/settings_file_store.hpp (I/O real do settings.json).

#ifndef GUS_APP_SCREENS_SYSTEM_MENU_LOOP_HPP
#define GUS_APP_SCREENS_SYSTEM_MENU_LOOP_HPP

#include <string>

#include <SDL3/SDL.h>

#include "gus/app/i18n/translator.hpp"
#include "gus/platform/audio/audio_engine.hpp"

namespace gus::app::screens {

// Desfecho do loop: o chamador decide o que fazer a seguir (fechar a janela real ou
// so retomar a cena de onde parou).
struct SystemMenuLoopOutcome {
    // true = o jogador confirmou "Sair" no menu OU fechou a janela (SDL_EVENT_QUIT)
    // enquanto o menu estava aberto. Sinal DISTINTO de "Continuar" (que devolve
    // false - o chamador so retoma a cena, sem encerrar nada). Mesmo espirito do
    // quit_requested de run_battle_preview_embedded.
    bool quit_app = false;
};

// Variante que ASSUME um contexto GL corrente (ver header). Seed inicial do volume
// = audio.music_volume()/audio.sfx_volume() (o estado em MEMORIA do AudioEngine,
// ja carregado do settings.json no boot pela Maestro - ver maestro.cpp::init()).
// A cada SystemMenuAction::VolumeChanged (teclado LEFT/RIGHT) OU arrasto de mouse
// no track do slider, aplica IMEDIATAMENTE em `audio` (set_music_volume/
// set_sfx_volume) e persiste em settings_dir via save_system_settings - o ouvinte
// escuta a mudanca em tempo real, sem esperar fechar o menu. `translator` resolve
// os rotulos i18n (o chamador mantem vivo; nao-dono, mesmo padrao de
// BattleScene::set_translator). Sai (devolve) quando o jogador confirma
// Continuar/ESC na tela Pause (quit_app=false), confirma Sair (quit_app=true), ou
// fecha a janela (quit_app=true).
[[nodiscard]] SystemMenuLoopOutcome run_system_menu_loop_gl_current(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator, const std::string& settings_dir);

// Variante DONA do contexto GL (ver header) - cria/destroi por conta propria. O
// CHAMADOR (Maestro) e responsavel por release_renderer()/reacquire_renderer() do
// SDL_Renderer da cidade POR FORA desta chamada (mesmo contrato de
// run_battle_preview_embedded vs a Maestro). Devolve false se a criacao do
// contexto GL ou o load do glad falhar (a janela segue viva; `*out_outcome` fica
// no default quit_app=false - o chamador decide como degradar, mesmo contrato de
// run_battle_preview_embedded). `out_outcome` pode ser nullptr se o chamador nao
// precisar do desfecho (nao ha uso previsto, mas mantido por simetria).
[[nodiscard]] bool run_system_menu_loop_owning_gl(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator, const std::string& settings_dir,
    SystemMenuLoopOutcome* out_outcome);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_SYSTEM_MENU_LOOP_HPP
