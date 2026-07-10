// gus/app/screens/title_menu_loop.hpp
//
// LOOP INTERATIVO da TELA DE TITULO (SAVE-LOAD-UI etapa 4, wiring REAL do boot).
// Roda a maquina de estado pura de title_menu.hpp + o RML/RCSS de
// title_menu_rml.hpp numa glintfx::UiLayer PROPRIA, num CONTEXTO GL DONO (owning -
// MESMA receita de run_system_menu_loop_owning_gl/run_battle_preview_embedded): o
// UNICO chamador de producao (Maestro::show_title_screen, chamado no INICIO do
// boot, antes do loop cidade<->batalha) ja fez city_->release_renderer() ANTES e
// faz city_->reacquire_renderer() DEPOIS - a janela fica livre pro contexto GL
// enquanto esta tela roda.
//
// ESTE ARQUIVO E O UNICO PONTO QUE FAZ I/O REAL DE DISCO desta tela (gus::
// platform::fs::has_save/load_game, JA EXISTENTES desde M2-SAVE-IO): varre TODOS
// os slots pra decidir any_save_exists (Continuar habilitado ou nao) e achar
// most_recent_occupied_slot (save_load_menu.hpp, SAVE-LOAD-UI etapa 4) - a tela
// PURA (title_menu.hpp) nunca toca disco. Um save PRESENTE mas nao Ok
// (adulterado/corrompido/versao incompativel) degrada, POR ORA, como slot vazio -
// MESMA politica (e MESMO racional) de build_previews_and_cache em
// save_load_menu_loop.cpp (os avisos dedicados sao etapa futura, fora do escopo
// desta dispatch).
//
// Cross-ref: gus/app/screens/title_menu.hpp (estado/navegacao PURA);
//            gus/app/screens/title_menu_rml.hpp (RML/RCSS data-driven);
//            gus/app/screens/save_load_menu.hpp (SaveSlotPreview/
//            most_recent_occupied_slot, REUSADOS sem alteracao);
//            gus/platform/fs/save_file_store.hpp (has_save/load_game reais);
//            gus/app/maestro.cpp (Maestro::show_title_screen, o UNICO chamador de
//            producao).

#ifndef GUS_APP_SCREENS_TITLE_MENU_LOOP_HPP
#define GUS_APP_SCREENS_TITLE_MENU_LOOP_HPP

#include <string>

#include <SDL3/SDL.h>

#include "gus/app/i18n/translator.hpp"
#include "gus/domain/save/save_data.hpp"

namespace gus::app::screens {

// Desfecho do loop: o que o CHAMADOR (Maestro) deve fazer a seguir.
enum class TitleLoopExit {
    ContinueGame,  // "Continuar" confirmado - `*out_loaded_save` foi preenchido
                   // com o save mais recente (most_recent_occupied_slot entre
                   // TODOS os slots ocupados) - o CHAMADOR aplica no jogo VIVO.
    NewGame,       // "Novo Jogo" confirmado (direto, sem save nenhum; OU via "Sim"
                   // no mini-dialogo) - o CHAMADOR comeca do estado FRESCO que
                   // Maestro::init() ja deixou pronto (sem retrabalho - nao ha
                   // "sessao anterior" carregada ainda nesta fatia).
    QuitApp,       // "Sair" confirmado OU o jogador fechou a JANELA durante a tela
                   // - o chamador encerra o programa SEM entrar no loop de jogo
                   // (nada foi jogado ainda - nenhum autosave faz sentido aqui).
};

// Roda a tela de titulo ATE o jogador escolher Continuar/Novo Jogo/Sair (ou
// fechar a janela). CRIA seu PROPRIO contexto GL na janela dada (mesma
// receita/atributos de run_system_menu_loop_owning_gl - profile core 3.3,
// double-buffer, stencil 8), carrega o glad, roda o loop, e DESTROI o contexto
// ao sair.
//
// `saves_dir`: diretorio real dos saves (o CHAMADOR passa gus::platform::fs::
//   resolve_saves_dir() - MESMA convencao de injecao de outros loops,
//   testabilidade/override via GUSWORLD_HOME).
// `out_exit`: preenchido com o desfecho (ver TitleLoopExit acima). Nao pode ser
//   nullptr.
// `out_loaded_save`: SO valido (preenchido) quando `*out_exit ==
//   TitleLoopExit::ContinueGame`; ignorado/intocado nos demais casos. Pode ser
//   nullptr SE o chamador so precisar saber QUE o jogador quer continuar (sem
//   uso previsto - o unico chamador de producao sempre fornece).
// `frozen_background_png` (default vazio): MESMA tecnica de fundo real congelado
//   das demais telas (PNG de 1 frame capturado pelo chamador ANTES de abrir) -
//   vazio degrada pro fundo abstrato (a vinheta/scrim do proprio painel).
//
// Devolve false se a criacao do contexto GL ou o load do glad falhar (a janela
// segue viva; `*out_exit` fica no default QuitApp definido pela IMPLEMENTACAO -
// o CHAMADOR decide como degradar; o unico chamador de producao trata false como
// "sem tela de titulo, comeca fresco" em vez de fechar o app, ver
// Maestro::show_title_screen).
[[nodiscard]] bool run_title_menu_loop_owning_gl(
    SDL_Window* window, const gus::app::i18n::Translator& translator,
    const std::string& saves_dir, TitleLoopExit* out_exit,
    gus::domain::save::SaveData* out_loaded_save,
    const std::string& frozen_background_png = std::string());

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_TITLE_MENU_LOOP_HPP
