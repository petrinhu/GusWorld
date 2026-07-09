// gus/app/screens/save_load_menu_loop.hpp
//
// LOOP INTERATIVO da tela de SALVAR/CARREGAR (SAVE-LOAD-UI etapa 6, wiring REAL
// que fecha o nucleo do M7). Roda a maquina de estado pura de save_load_menu.hpp
// + o RML/RCSS de save_load_menu_rml.hpp numa glintfx::UiLayer PROPRIA, ANINHADA
// no MESMO contexto GL que o CHAMADOR (gus/app/screens/system_menu_loop.cpp) ja
// deixou corrente - MESMO padrao de aninhamento que o proprio menu de sistema ja
// usa quando aberto de dentro da batalha (ver o header de system_menu_loop.hpp).
//
// ESTE ARQUIVO E O UNICO PONTO QUE FAZ I/O REAL DE DISCO do save (gus::platform::
// fs::has_save/load_game/save_game, ja existentes desde M2-SAVE-IO): a tela pura
// (save_load_menu.hpp) so decide NAVEGACAO/CONFIRMACAO, nunca toca disco.
//
// Modo SAVE: ao confirmar um slot (SlotChosen num vazio, ou OverwriteConfirmed num
// ocupado), chama `build_current_save_data()` (fornecido pelo CHAMADOR - Maestro -
// que sabe o SaveData VIVO: flags, posicao do jogador, timestamp fresco no
// instante exato da gravacao) e grava via save_game(). A tela NAO fecha sozinha
// apos salvar - o slot e atualizado NA HORA (novo timestamp/playtime visiveis) e
// o jogador continua na lista (pode salvar em outro slot, ou Voltar quando quiser -
// MESMO padrao "salvar e continuar navegando" de RPGs classicos, sem popup extra).
//
// Modo LOAD: os previews de TODOS os slots ja sao construidos LENDO o disco
// (has_save + load_game) na ABERTURA da tela - um slot so aparece OCUPADO/
// selecionavel se load_game devolveu LoadResult::Ok (ver o comentario de
// build_previews_and_cache no .cpp: um save CORROMPIDO/versao-incompativel
// degrada, POR ORA, como se o slot estivesse vazio - os 2 AVISOS dedicados
// (versao-incompativel/corrompido, controles-diferentes) sao ETAPA FUTURA, fora
// do escopo do nucleo desta onda, ver TODO.md). Ao confirmar um slot Ok, chama
// `apply_loaded_save_data(data)` (o CHAMADOR - Maestro - aplica flags+posicao no
// jogo VIVO) e a tela INTEIRA fecha de volta pro gameplay (ClosedAfterLoad) - o
// jogador nao volta pro menu de pausa, vai direto jogar (mesmo padrao "Carregar"
// de qualquer RPG).
//
// Cross-ref: gus/app/screens/save_load_menu.hpp (estado/navegacao PURA);
//            gus/app/screens/save_load_menu_rml.hpp (RML/RCSS data-driven);
//            gus/platform/fs/save_file_store.hpp (has_save/save_game/load_game,
//            JA EXISTENTES, M2-SAVE-IO); gus/app/screens/system_menu_loop.hpp
//            (o CHAMADOR, mesma tecnica de aninhamento no MESMO contexto GL);
//            gus/app/maestro.cpp (fornece os 2 callbacks - unico dono do SaveData
//            e da posicao VIVOS do jogo).

#ifndef GUS_APP_SCREENS_SAVE_LOAD_MENU_LOOP_HPP
#define GUS_APP_SCREENS_SAVE_LOAD_MENU_LOOP_HPP

#include <functional>
#include <string>

#include <SDL3/SDL.h>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/save_load_menu.hpp"
#include "gus/domain/save/save_data.hpp"

namespace gus::app::screens {

// Desfecho do loop: o que o CHAMADOR (system_menu_loop.cpp) deve fazer a seguir.
enum class SaveLoadLoopExit {
    BackToPause,      // Esc/Voltar na lista (fora do mini-dialogo) - reabra o Pause
    ClosedAfterLoad,  // Load confirmado com sucesso (apply_loaded_save_data ja
                      // rodou) - feche o menu de pausa INTEIRO, volte pro gameplay
                      // (MESMO efeito pratico de SystemMenuAction::Continue).
    QuitApp,          // o jogador fechou a JANELA durante a tela - o chamador
                      // encerra o programa (mesmo contrato de SystemMenuLoopOutcome).
};

// Roda a tela de save/load ATE o jogador voltar/fechar a janela/completar um Load.
//
// `mode`: Save ou Load (determina selecionabilidade + o que Enter faz, ver
//   save_load_menu.hpp). `saves_dir`: diretorio real dos saves (o CHAMADOR passa
//   gus::platform::fs::resolve_saves_dir() - MESMA convencao de injecao de
//   settings_dir em run_system_menu_loop_gl_current, testabilidade/override).
//
// `build_current_save_data`: chamado SO em modo Save, na hora exata de gravar um
//   slot (nunca antecipado) - devolve o SaveData VIVO pronto pra serializar
//   (timestamp/playtime/posicao FRESCOS). Ignorado (pode ser vazio/nullptr) em
//   modo Load.
// `apply_loaded_save_data`: chamado SO em modo Load, apos um load bem-sucedido
//   (LoadResult::Ok) - o CHAMADOR aplica flags/posicao no jogo VIVO. Ignorado em
//   modo Save.
//
// `frozen_background_png` (default vazio): MESMA tecnica de fundo real congelado
//   de run_system_menu_loop_gl_current (PNG de 1 frame capturado ANTES de abrir o
//   Pause) - vazio degrada pro fundo abstrato (vinheta radial).
[[nodiscard]] SaveLoadLoopExit run_save_load_menu_loop_gl_current(
    SDL_Window* window, const gus::app::i18n::Translator& translator,
    SaveLoadMode mode, const std::string& saves_dir,
    const std::function<gus::domain::save::SaveData()>& build_current_save_data,
    const std::function<void(const gus::domain::save::SaveData&)>&
        apply_loaded_save_data,
    const std::string& frozen_background_png = std::string());

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_SAVE_LOAD_MENU_LOOP_HPP
