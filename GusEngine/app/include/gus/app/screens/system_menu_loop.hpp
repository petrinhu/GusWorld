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
// FUNDO (retoque do lider via AskUserQuestion, M7-DIALOGO/MENU-PAUSA-CONFIG-SOM):
// quando o CHAMADOR fornece `frozen_background_png` (PNG de 1 frame, capturado por
// SdlWindow::capture_frame_to_png ANTES de abrir o menu - ver Maestro::open_pause_
// from_city), o loop desenha essa CENA REAL da cidade CONGELADA como fundo
// estatico, cobrindo a vinheta - mesmo padrao de Chrono Trigger/Zelda/Stardew
// Valley (o mundo "pausa" visualmente atras da UI). `frozen_background_png` vazio
// (default, uso de dentro da BATALHA hoje inexistente na producao - ver
// battle_preview.cpp) degrada pro fundo ABSTRATO de sempre: Render2dGl3::
// begin_frame (clear + vinheta radial, MESMA ambientacao usada na arena).
//
// SAVE-LOAD-UI etapa 6 (wiring REAL): "Salvar"/"Carregar" confirmados no Pause
// (SystemMenuAction::OpenSaveLoadSave/OpenSaveLoadLoad) abrem a tela REAL de
// save/load (gus/app/screens/save_load_menu_loop.hpp), ANINHADA no MESMO
// contexto GL (MESMA tecnica do proprio menu de sistema quando aninhado dentro
// da batalha). `saves_dir` + os 2 callbacks abaixo sao repassados direto pra
// essa tela - ela e quem faz o I/O de disco de fato; este loop so intercepta a
// action e delega.
//
// Cross-ref: gus/app/screens/system_menu.hpp (a maquina de estado pura);
//            gus/app/screens/system_menu_rml.hpp (o RML/RCSS gerado do estado);
//            gus/app/screens/save_load_menu_loop.hpp (a tela REAL de save/load,
//            SAVE-LOAD-UI etapa 6); gus/app/screens/battle_preview.cpp
//            (run_battle_preview_embedded, o chamador da variante gl_current -
//            Esc na pilha vazia do combate); gus/app/maestro.cpp (o chamador da
//            variante owning_gl - Esc na cidade); gus/platform/fs/
//            settings_file_store.hpp (I/O real do settings.json).

#ifndef GUS_APP_SCREENS_SYSTEM_MENU_LOOP_HPP
#define GUS_APP_SCREENS_SYSTEM_MENU_LOOP_HPP

#include <functional>
#include <string>

#include <SDL3/SDL.h>

#include "gus/app/i18n/translator.hpp"
#include "gus/domain/save/save_data.hpp"
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

    // MENU-INICIAL: true = o jogador confirmou "Sim" no mini-dialogo "voltar ao
    // menu inicial?" (SystemMenuAction::RequestToTitle) - o CHAMADOR (Maestro::
    // open_pause_from_city) deve chamar show_title_screen() em vez de encerrar
    // o processo. Mutuamente exclusivo com quit_app (so 1 dos dois fica true por
    // chamada - RequestToTitle e RequestQuit nunca coexistem no mesmo desfecho).
    bool to_title = false;
};

// Variante que ASSUME um contexto GL corrente (ver header). Seed inicial do volume
// = audio.music_volume()/audio.sfx_volume() (o estado em MEMORIA do AudioEngine,
// ja carregado do settings.json no boot pela Maestro - ver maestro.cpp::init()).
// A cada SystemMenuAction::VolumeChanged (teclado LEFT/RIGHT) OU arrasto de mouse
// no track do slider, aplica IMEDIATAMENTE em `audio` (set_music_volume/
// set_sfx_volume) e persiste em settings_dir via save_system_settings - o ouvinte
// escuta a mudanca em tempo real, sem esperar fechar o menu. `translator` resolve
// os rotulos i18n (o chamador mantem vivo; nao-dono, mesmo padrao de
// BattleScene::set_translator). `frozen_background_png` (ver o header, NOVO):
// caminho de um PNG de 1 frame capturado pelo chamador (ex.: cidade CONGELADA) pra
// desenhar como fundo estatico; vazio (default) = fundo abstrato de sempre (a
// vinheta radial). Sai (devolve) quando o jogador confirma Continuar/ESC na tela
// Pause (quit_app=false), confirma Sair (quit_app=true), ou fecha a janela
// (quit_app=true).
// `saves_dir` (SAVE-LOAD-UI etapa 6): diretorio real dos saves (gus::platform::
// fs::resolve_saves_dir(), MESMA convencao de injecao de settings_dir acima -
// testabilidade/override). `build_current_save_data`/`apply_loaded_save_data`
// (default vazio = "sem capacidade de save/load" nesta entrada - hoje so a
// Maestro na CIDADE fornece os 2; a entrada futura de dentro da BATALHA, se
// vier a existir, pode legitimamente deixar vazio e "Salvar"/"Carregar" viram
// no-op degradado, NUNCA fingem persistir): repassados direto pra
// run_save_load_menu_loop_gl_current (ver seu header pro contrato de cada um).
[[nodiscard]] SystemMenuLoopOutcome run_system_menu_loop_gl_current(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator, const std::string& settings_dir,
    const std::string& saves_dir,
    const std::function<gus::domain::save::SaveData()>& build_current_save_data =
        {},
    const std::function<void(const gus::domain::save::SaveData&)>&
        apply_loaded_save_data = {},
    const std::string& frozen_background_png = std::string());

// Variante DONA do contexto GL (ver header) - cria/destroi por conta propria. O
// CHAMADOR (Maestro) e responsavel por release_renderer()/reacquire_renderer() do
// SDL_Renderer da cidade POR FORA desta chamada (mesmo contrato de
// run_battle_preview_embedded vs a Maestro). Devolve false se a criacao do
// contexto GL ou o load do glad falhar (a janela segue viva; `*out_outcome` fica
// no default quit_app=false - o chamador decide como degradar, mesmo contrato de
// run_battle_preview_embedded). `out_outcome` pode ser nullptr se o chamador nao
// precisar do desfecho (nao ha uso previsto, mas mantido por simetria).
// `frozen_background_png` (ver o header, NOVO): repassado direto pra
// run_system_menu_loop_gl_current (ver seu comentario) - a Maestro (unica chamadora
// de producao desta variante) passa o PNG da cidade CONGELADA (Maestro::open_pause_
// from_city); vazio (default) = fundo abstrato de sempre.
// `saves_dir`/`build_current_save_data`/`apply_loaded_save_data` (SAVE-LOAD-UI
// etapa 6): repassados direto pra run_system_menu_loop_gl_current (ver seu
// comentario acima) - a Maestro (unica chamadora de producao) fornece os 3.
[[nodiscard]] bool run_system_menu_loop_owning_gl(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator, const std::string& settings_dir,
    const std::string& saves_dir, SystemMenuLoopOutcome* out_outcome,
    const std::function<gus::domain::save::SaveData()>& build_current_save_data =
        {},
    const std::function<void(const gus::domain::save::SaveData&)>&
        apply_loaded_save_data = {},
    const std::string& frozen_background_png = std::string());

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_SYSTEM_MENU_LOOP_HPP
