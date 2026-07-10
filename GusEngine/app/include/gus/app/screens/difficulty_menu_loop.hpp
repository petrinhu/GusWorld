// gus/app/screens/difficulty_menu_loop.hpp
//
// LOOP INTERATIVO da TELA DE SELECAO DE DIFICULDADE (MODOS-MORTE Fase 0). Roda a
// maquina de estado pura de difficulty_menu.hpp + o RML/RCSS de
// difficulty_menu_rml.hpp numa glintfx::UiLayer PROPRIA, ANINHADA no MESMO
// contexto GL que o CHAMADOR (gus/app/screens/title_menu_loop.cpp) ja deixou
// corrente - MESMO padrao de aninhamento de run_save_load_menu_loop_gl_current
// (gus/app/screens/save_load_menu_loop.hpp): o CHAMADOR DESTROI o proprio
// UiLayer ANTES de chamar esta funcao (RmlUi NAO suporta 2 UiLayer simultaneos
// no processo) e recria o dele DEPOIS se precisar seguir mostrando algo (ver o
// comentario "FIX CRITICO" em system_menu_loop.cpp pela causa raiz historica).
//
// ESTE LOOP NAO FAZ I/O DE DISCO (ao contrario de title_menu_loop.cpp/
// save_load_menu_loop.cpp) - a dificuldade escolhida so e GRAVADA no SaveData
// quando o CHAMADOR (Maestro, via title_menu_loop.cpp) de fato inicia o jogo novo
// (StartNewGame); esta tela so decide QUAL DifficultyLevel.
//
// Cross-ref: gus/app/screens/difficulty_menu.hpp (estado/navegacao PURA);
//            gus/app/screens/difficulty_menu_rml.hpp (RML/RCSS data-driven);
//            gus/app/screens/title_menu_loop.cpp (o UNICO chamador de producao -
//            dispara esta tela ao confirmar "Novo Jogo"); gus/app/screens/
//            save_load_menu_loop.hpp (MESMA tecnica de aninhamento no mesmo
//            contexto GL, precedente historico).

#ifndef GUS_APP_SCREENS_DIFFICULTY_MENU_LOOP_HPP
#define GUS_APP_SCREENS_DIFFICULTY_MENU_LOOP_HPP

#include <string>

#include <SDL3/SDL.h>

#include "gus/app/i18n/translator.hpp"
#include "gus/domain/save/save_data.hpp"
#include "gus/platform/audio/audio_engine.hpp"  // COCKPIT-SFX-HOVER-CLIQUE: paridade sonora

namespace gus::app::screens {

// Desfecho do loop: o que o CHAMADOR (title_menu_loop.cpp) deve fazer a seguir.
enum class DifficultyLoopExit {
    Chosen,     // dificuldade ESCOLHIDA + confirmada no Aviso #2 -
                // `*out_difficulty` foi preenchido - o CHAMADOR prossegue como
                // StartNewGame, gravando a dificuldade no SaveData novo.
    Cancelled,  // ESC/Voltar na LISTA (fora do splash) - o jogador desistiu de
                // Novo Jogo; o CHAMADOR volta pra tela de titulo (recria o
                // PROPRIO UiLayer, que ja foi destruido antes desta chamada).
    QuitApp,    // o jogador fechou a JANELA durante a tela - o chamador encerra
                // o programa (mesmo contrato de TitleLoopExit::QuitApp).
};

// Roda a tela de selecao de dificuldade ATE o jogador escolher+confirmar, desistir
// (Cancelled) ou fechar a janela. NAO cria/destroi contexto GL (ANINHADA no
// contexto JA CORRENTE do CHAMADOR, MESMO padrao de
// run_save_load_menu_loop_gl_current) - cria a PROPRIA glintfx::UiLayer (o
// CHAMADOR ja destruiu a dele antes de chamar, RmlUi so aceita 1 instancia viva).
//
// `audio`: AudioEngine da Maestro (nao-dono) - hover/clique, MESMOS sons dos
//   demais menus (kMenuHoverSfxFile/kMenuClickSfxFile).
// `out_difficulty`: SO valido (preenchido) quando o retorno e Chosen; intocado
//   nos demais casos. Nao pode ser nullptr.
// `frozen_background_png` (default vazio): MESMA tecnica de fundo real congelado
//   das demais telas - vazio degrada pro fundo abstrato (a vinheta do painel).
//
// Devolve Cancelled por DEFAULT se a criacao da UiLayer falhar (degradacao segura -
// o CHAMADOR volta pra tela de titulo em vez de travar o boot).
[[nodiscard]] DifficultyLoopExit run_difficulty_menu_loop_gl_current(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator,
    gus::domain::save::DifficultyLevel* out_difficulty,
    const std::string& frozen_background_png = std::string());

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_DIFFICULTY_MENU_LOOP_HPP
