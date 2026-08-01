// SPDX-License-Identifier: Apache-2.0
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
// MOUSE + SFX (retoque ao vivo do lider, bugs 1/3/6/9 + paridade de som):
// hit-test de slot/icone-de-apagar/Voltar via glintfx::UiLayer::get_element_box
// (MESMA receita de system_menu_loop.cpp), hover NATIVO (:hover RCSS, injecao de
// UiEvent::MouseMove em TODO SDL_EVENT_MOUSE_MOTION) + SOM de hover/clique
// (edge-detect, MESMO choke-point/padrao de system_menu_loop.cpp), e forwarding
// de SDL_EVENT_MOUSE_WHEEL pra `.slot-list` (MESMA tecnica de scroll de
// system_menu_loop.cpp::Controls). `audio` (NOVO parametro): o AudioEngine VIVO
// do chamador (system_menu_loop.cpp ja o tem em escopo) - toca hover_sfx/click_sfx
// (gus/core/asset_paths.hpp::kMenuHoverSfxFile/kMenuClickSfxFile), MESMOS arquivos
// dos demais menus.
//
// F4-1b.2 (onda F4 "casca SDL -> App mode do glintfx", fatia 1b.2 - SEGUNDA tela
// convertida ao contrato ScreenState, MESMO template de F4-1b.1/
// difficulty_menu_loop.hpp/.cpp): o caminho de PRODUCAO
// (run_save_load_menu_loop_gl_current, chamado por system_menu_loop.cpp) foi
// convertido de um while(true){SDL_PollEvent...} PROPRIO pra uma classe
// SaveLoadScreen (gus::app::ScreenState) + gus::app::run_screen_state
// (gus/app/screen_state.hpp) - ANINHADA (NAO cria/destroi o contexto GL, MESMA
// nota de difficulty_menu_loop.hpp). O roteamento de evento (decidir QUAL
// SaveLoadMenuAction resultou de um SDL_Event, mais SFX/resize/mouse_move) foi
// extraido pra uma FUNCAO LIVRE PURA nova, save_load_screen_step (testada
// headless em save_load_screen_step_test.cpp) - ela recebe as
// glintfx::ElementBox JA RESOLVIDAS como parametro (`boxes`, SaveLoadStepBoxes),
// nunca consulta a UiLayer. DIFERENCA-CHAVE vs. difficulty_screen_step: esta
// tela tem acoes que EXIGEM I/O real (do_save/do_delete/do_recover/
// apply_loaded_save_data) - save_load_screen_step NUNCA executa esse I/O, so
// devolve QUAL SaveLoadMenuAction resultou (`SaveLoadStepResult::action`); o
// exit (SaveLoadLoopExit) so e preenchido pela funcao PURA quando e
// DETERMINISTICO sem I/O nenhum (Back->BackToPause sempre; SlotChosen em modo
// Load->ClosedAfterLoad, assumindo o INVARIANTE "slot selecionavel em Load
// SEMPRE tem cache" - ver save_load_menu.hpp) - SaveLoadScreen::handle_event
// (save_load_menu_loop.cpp) e quem executa o I/O de fato E mantem o guard
// defensivo pro caso teorico do cache faltar (MESMO log do while(true) antigo).
// MOUSE_WHEEL fica DE FORA do escopo de save_load_screen_step (exige
// SDL_GetMouseState, uma consulta IMPURA de cursor real) - tratado direto em
// SaveLoadScreen::handle_event, MESMO racional de collect_click_boxes_ (custo
// condicional por ramo/estado) do template de difficulty.
//
// Cross-ref: gus/app/screens/save_load_menu.hpp (estado/navegacao PURA);
//            gus/app/screens/save_load_menu_rml.hpp (RML/RCSS data-driven);
//            gus/platform/fs/save_file_store.hpp (has_save/save_game/load_game/
//            delete_save, JA EXISTENTES, M2-SAVE-IO); gus/app/screens/
//            system_menu_loop.hpp (o CHAMADOR, mesma tecnica de aninhamento no
//            MESMO contexto GL); gus/app/maestro.cpp (fornece os 2 callbacks -
//            unico dono do SaveData e da posicao VIVOS do jogo);
//            gus/app/screens/difficulty_menu_loop.hpp (o TEMPLATE desta
//            conversao, F4-1b.1); gus/app/screen_state.hpp (ScreenState/
//            EventSyncHook, MESMO contrato).

#ifndef GUS_APP_SCREENS_SAVE_LOAD_MENU_LOOP_HPP
#define GUS_APP_SCREENS_SAVE_LOAD_MENU_LOOP_HPP

#include <array>
#include <functional>
#include <optional>
#include <string>

#include <SDL3/SDL.h>
#include <glintfx/element_box.hpp>  // F4-1b.2: glintfx::ElementBox, INPUT de save_load_screen_step

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screen_state.hpp"  // F4-1b.2: gus::app::EventSyncHook (MESMO contrato de F4-1a/F4-1b.1)
#include "gus/app/screens/save_load_menu.hpp"
#include "gus/domain/save/save_data.hpp"
#include "gus/domain/save/save_slots.hpp"  // gus::domain::save::kSlotCount
#include "gus/platform/audio/audio_engine.hpp"

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
//
// F4-1b.2: `sync_hook` (opcional - nullptr = nao sincroniza nada, MESMO
// comportamento de antes desta fatia) e repassado direto a
// gus::app::run_screen_state() por dentro - MESMO papel/contrato de
// difficulty_menu_loop.hpp/npc_dialogue_loop_gl.hpp (F4-1a). Esta tela e
// ANINHADA (chamada de dentro de system_menu_loop.cpp, que NAO foi convertido
// nesta fatia) - nenhum chamador de producao passa `sync_hook` ainda; o
// parametro existe pra esta tela ja seguir o MESMO contrato de ScreenState/
// EventSyncHook que o resto da onda F4 vai adotar tela a tela.
[[nodiscard]] SaveLoadLoopExit run_save_load_menu_loop_gl_current(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator, SaveLoadMode mode,
    const std::string& saves_dir,
    const std::function<gus::domain::save::SaveData()>& build_current_save_data,
    const std::function<void(const gus::domain::save::SaveData&)>&
        apply_loaded_save_data,
    const std::string& frozen_background_png = std::string(),
    const gus::app::EventSyncHook& sync_hook = nullptr);

// F4-1b.2: SFX que pode resultar de UM save_load_screen_step() - QUALQUER
// clique que acertou um alvo real (slot/icone-de-apagar/Voltar/pill de
// dialogo/aviso) soa Click, MESMA simplificacao "todo clique reconhecido soa"
// do while(true) antigo (ver o comentario de "SOM DE CLIQUE" em
// save_load_menu_loop.cpp) - o hover de MOUSE continua 100% no callback
// NATIVO glintfx::UiLayer::set_hover_callback (fora do escopo desta funcao, o
// MESMO SFX-MIGRATE-V0.9 de sempre).
//
// B4 (decisao do lider 2026-08-01 - retoque ao vivo pos-bugs 1-9): ATE esta
// fatia, teclado NUNCA tocava SFX nesta tela (diferente de
// difficulty_screen_step/title_screen_step, que ja tinham paridade teclado x
// mouse) - era uma omissao DOCUMENTADA, nao um bug acidental (ver o historico
// deste comentario). O lider pediu paridade: agora Hover (abaixo) toca quando
// a NAVEGACAO por teclado move o foco pra um item NOVO dentro do MESMO
// sub-modo (save_load_focus_mode/save_load_keyboard_focus_index,
// save_load_menu.hpp) - MESMO racional de title_screen_step (compara
// state.confirming_new_game antes/depois pra nao contar troca de sub-modo
// como "moveu de item").
enum class SaveLoadSfxKind {
    None,   // nenhum som (evento nao roteado, tecla, ou clique fora de qualquer box)
    Hover,  // hover_sfx_id - teclado moveu o foco pra um item NOVO no MESMO sub-modo (B4)
    Click,  // click_sfx_id - o CHAMADOR mapeia pro SoundId real
};

// Caixas de hit-test resolvidas pelo CHAMADOR (SaveLoadScreen::
// collect_click_boxes_, save_load_menu_loop.cpp) via
// glintfx::UiLayer::get_element_box - SO as do ramo/estado ATUAL de
// SaveLoadMenuState sao preenchidas (found=true); as demais ficam no default
// (found=false, MESMO "nao resolvida neste frame" de
// DifficultyStepResult/difficulty_menu_loop.hpp) - save_load_screen_step le
// `state.warning_kind`/`state.confirming_delete`/`state.confirming_overwrite`
// (MESMA leitura que o CHAMADOR faz pra escolher QUAIS ids resolver antes de
// chamar) pra decidir qual sub-array consultar.
struct SaveLoadStepBoxes {
    glintfx::ElementBox back;  // "slmenu-back" (Voltar) - lista normal, SEMPRE presente
    std::array<glintfx::ElementBox, gus::domain::save::kSlotCount> slots{};
        // "slmenu-slot-<i>" - lista normal
    std::array<glintfx::ElementBox, gus::domain::save::kSlotCount> delete_icons{};
        // "slmenu-delete-<i>" - lista normal, SO resolvida pra slots OCUPADOS
    std::array<glintfx::ElementBox, 2> overwrite_confirm{};
        // "slmenu-confirm-yes/no" (0/1) - state.confirming_overwrite
    std::array<glintfx::ElementBox, 2> delete_confirm{};
        // "slmenu-delete-confirm-yes/no" (0/1) - state.confirming_delete
    glintfx::ElementBox warn_recover;  // "slmenu-warn-recover" - state.warning_kind==Damaged
    glintfx::ElementBox warn_cancel;   // "slmenu-warn-cancel" - state.warning_kind!=None
};

// Resultado de UM save_load_screen_step(): o que o CHAMADOR (SaveLoadScreen::
// handle_event, save_load_menu_loop.cpp) deve fazer a seguir - TODO o I/O REAL
// (do_save/do_delete/do_recover/apply_loaded_save_data) fica do lado de FORA
// desta struct/funcao, que so decide QUAL SaveLoadMenuAction resultou do
// evento (teclado OU mouse).
struct SaveLoadStepResult {
    bool window_closed = false;  // SDL_EVENT_QUIT
    bool resize = false;         // WINDOW_RESIZED/PIXEL_SIZE_CHANGED - o estado da
                                  // tela NAO muda; o CHAMADOR reconsulta o tamanho
                                  // real da janela e reposiciona a UiLayer.
    bool mouse_move = false;     // MOUSE_MOTION - o CHAMADOR encaminha (mouse_x,
                                  // mouse_y) pro glintfx::UiEvent::MouseMove (hover
                                  // NATIVO); o SOM de hover continua no callback
                                  // nativo, FORA do escopo desta funcao.
    float mouse_x = 0.0f;
    float mouse_y = 0.0f;
    bool reload = false;  // o CHAMADOR reescreve o RML - o estado pode ter mudado
                          // (foco/dialogo/aviso abriu-fechou) mesmo sem `exit`
                          // preenchido.
    SaveLoadSfxKind sfx = SaveLoadSfxKind::None;
    // Acao que resultou do evento (save_load_menu_key_down/save_load_menu_click_*)
    // - nullopt se nenhum roteamento de acao se aplicou (resize/mouse_move/
    // window_closed, tecla/clique sem efeito ainda mapeado, OU o clique no icone
    // de apagar - que muta o estado DIRETO via save_load_menu_request_delete, sem
    // passar por uma SaveLoadMenuAction). O CHAMADOR so precisa executar I/O real
    // quando isto e SlotChosen/OverwriteConfirmed/DeleteConfirmed/
    // RecoverRequested - os demais (None/Back/*Cancelled) ja tem `reload`/`exit`
    // 100% decididos por esta funcao, sem I/O.
    std::optional<SaveLoadMenuAction> action;
    // Exit JA decidido SEM I/O: Back->BackToPause (sempre, nao precisa de disco);
    // SlotChosen em modo Load->ClosedAfterLoad (assume o INVARIANTE "slot
    // selecionavel em Load SEMPRE tem cache", ver save_load_menu.hpp - o CHAMADOR
    // AINDA aplica apply_loaded_save_data(cached) de fato e mantem o guard
    // defensivo pro caso teorico do cache faltar, MESMA mensagem de log do
    // while(true) antigo). RecoverRequested NUNCA seta isto aqui - a recuperacao
    // e I/O real (load_game_from_backup); so o CHAMADOR, apos tentar, sabe o exit
    // de fato.
    std::optional<SaveLoadLoopExit> exit;
};

// O NUCLEO PURO do roteamento de evento desta tela. Muta `state` via as MESMAS
// gus::app::screens::save_load_menu_key_down/save_load_menu_click_*/
// save_load_menu_request_delete de sempre e devolve o QUE FAZER a seguir
// (acima). `boxes` so precisa ter as caixas do ramo/estado ATUAL preenchidas
// (found=true) - esta funcao le `state` pra saber quais consultar (MESMA
// leitura que o CHAMADOR faz em collect_click_boxes_, ver o comentario de
// SaveLoadStepBoxes acima). SDL_EVENT_MOUSE_WHEEL NAO e roteado aqui (exige
// SDL_GetMouseState, consulta IMPURA - tratado direto em
// SaveLoadScreen::handle_event, save_load_menu_loop.cpp) - devolve o resultado
// default (no-op TOTAL) pra esse e qualquer outro tipo de evento nao roteado.
[[nodiscard]] SaveLoadStepResult save_load_screen_step(SaveLoadMenuState& state,
                                                        const SDL_Event& ev,
                                                        const SaveLoadStepBoxes& boxes) noexcept;

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_SAVE_LOAD_MENU_LOOP_HPP
