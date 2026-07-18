// gus/app/screens/system_menu.hpp
//
// Logica PURA do MENU DE SISTEMA (pausa + config de som/video/lingua/save),
// MENU-PAUSA-CONFIG-SOM (M7-COSTURA). POCO 100% testavel sem SDL_Init/janela/
// glintfx - so o estado de navegacao/selecao/volume (mesmo espirito de
// gus/app/maestro_logic.hpp e da funcao-livre battle_key_down em
// battle_preview.hpp). Consome SDL_Keycode diretamente (app/ ja e camada
// SDL-aware, mesmo padrao de battle_key_down) mas NAO abre janela nem chama
// SDL_Init - so compara o valor do enum.
//
// ARVORE HIERARQUICA (revisao MENU-PAUSA-CONFIG-SOM "onda arvore", aprovada ao
// vivo pelo lider - substitui as 2 telas planas originais Pause/Config por uma
// navegacao em 4 niveis):
//
//   Pause (Continuar / Salvar / Carregar / Configuracoes / Sair)
//     ConfigCategories (Audio / Video / Lingua / Voltar)
//       Audio         -> Volume da Musica + Volume dos Efeitos + Voltar (a
//                         MESMA tela de sliders que antes se chamava "Config")
//       Video         -> placeholder "em breve"
//       Language      -> placeholder "em breve"
//
// SAVE-LOAD-UI etapa 6 (wiring REAL, fecha o nucleo do M7): "Salvar" e
// "Carregar" NAO SAO MAIS filhas desta arvore (o antigo SystemMenuScreen::Save
// placeholder "em breve" foi APOSENTADO - removido do enum, ver o comentario
// historico em git blame se precisar arqueologia). Confirmar qualquer um dos
// dois dispara SystemMenuAction::OpenSaveLoadSave/OpenSaveLoadLoad (state.screen
// permanece em Pause, SEM navegar) - o CHAMADOR (system_menu_loop.cpp) intercepta
// essa action e abre a tela REAL de save/load (gus/app/screens/save_load_menu.hpp
// + save_load_menu_loop.hpp, um documento glintfx PROPRIO, ANINHADO no MESMO
// contexto GL - mesma tecnica de aninhamento ja usada por este proprio menu
// dentro da batalha). Ao voltar (Esc/Voltar na lista de slots, ou apos um Load
// bem-sucedido fechar o jogo inteiro pro gameplay), o Pause reaparece do jeito
// que estava (pause_selected preservado, nenhuma navegacao de estado ocorreu).
//
// Cada tela (exceto Pause, a raiz) tem um PAI fixo (ver parent_screen_of
// abaixo); "Voltar"/ESC sobe exatamente um nivel pro pai, preservando a
// selecao anterior do pai (o pai so muda quando o PROPRIO pai e navegado, nao
// quando um filho e visitado - ver os campos *_selected abaixo, um por tela
// COM MAIS DE 1 item navegavel).
//
// Esta e a camada de ESTADO/DECISAO; a RENDERIZACAO (RML/RCSS via glintfx) vive
// em system_menu_rml.hpp/cpp (le este estado, nao decide nada) e a INTEGRACAO
// com AudioEngine/persistencia de settings.json vive no chamador (Maestro/
// battle_preview), que reage as SystemMenuAction devolvidas por
// system_menu_key_down.
//
// Cross-ref: gus/app/screens/system_menu_rml.hpp (RML/RCSS data-driven deste
//            estado); gus/app/screens/save_load_menu.hpp/save_load_menu_loop.hpp
//            (Salvar/Carregar REAIS, SAVE-LOAD-UI etapa 6); gus/platform/audio/
//            audio_engine.hpp (set_music_volume/set_sfx_volume, consumidos pelo
//            CHAMADOR quando VolumeChanged); gus/platform/fs/settings_file_store.hpp
//            (persistencia, idem); gus/app/screens/battle_preview.hpp
//            (battle_key_down, pilha de Esc do combate - o Esc na PILHA VAZIA e
//            o gancho que abre este menu).

#ifndef GUS_APP_SCREENS_SYSTEM_MENU_HPP
#define GUS_APP_SCREENS_SYSTEM_MENU_HPP

#include <string>
#include <string_view>

#include <SDL3/SDL.h>  // SDL_Keycode

#include "gus/domain/input/input_binding.hpp"  // InputRemapConfig/KeyBinding (tela Controles)

namespace gus::app::screens {

// Tela ativa do menu de sistema. Hidden = menu fechado (jogo roda normal).
// Video/Language sao PLACEHOLDER ("em breve" - video/idioma ainda sem spec).
// Controls (M2, "a ponte pro jogador" - o motor de remap ja existia em
// domain/input/) e a UNICA das filhas de ConfigCategories que NAO e placeholder.
// SAVE-LOAD-UI etapa 6: NAO ha mais SystemMenuScreen::Save aqui - "Salvar"/
// "Carregar" abrem uma tela REAL/ANINHADA fora desta arvore (ver o comentario
// grande no topo do arquivo) - state.screen fica em Pause o tempo todo.
enum class SystemMenuScreen {
    Hidden,
    Pause,
    ConfigCategories,  // Audio/Video/Controles/Lingua/Voltar, pai = Pause
    Audio,             // sliders Musica/SFX (a antiga tela "Config"), pai = ConfigCategories
    Video,             // placeholder, pai = ConfigCategories
    Controls,          // remap de teclado (M2), pai = ConfigCategories
    Language,          // placeholder, pai = ConfigCategories
};

// Numero de itens navegaveis de cada tela (fonte unica do wrap de navegacao).
inline constexpr int kPauseItemCount = 6;  // Continuar/Salvar/Carregar/Configuracoes/
                                            // MenuInicial (MENU-INICIAL)/Sair
inline constexpr int kConfigCategoriesItemCount = 5;  // Audio/Video/Controles/Lingua/Voltar
inline constexpr int kAudioItemCount = 3;             // Musica/SFX/Voltar
inline constexpr int kPlaceholderItemCount = 1;       // so Voltar (Video/Language)

// Tela Controles (M2 -> M2 STAGED CHANGES, reforma de UX aprovada pelo lider):
// 30 actions curadas/agrupadas (Movimento/Mundo/Combate/Menu&Dialogo - ver
// controls_action_name_at/controls_group_at abaixo) + os 3 itens de rodape
// (Restaurar padrao, Aplicar - NOVO, Voltar). O rodape passou de 2 pra 3
// botoes: "aplica na hora" foi trocado por STAGED CHANGES (ver o comentario
// grande logo abaixo, secao STAGED CHANGES) - o jogador agora precisa
// confirmar "Aplicar" pra valer no jogo/disco; "Voltar" so descarta.
inline constexpr int kControlsActionCount = 30;         // ActionRegistry::count()
inline constexpr int kControlsRestoreIndex = 30;         // item de rodape "Restaurar padrao"
inline constexpr int kControlsApplyIndex = 31;           // item de rodape "Aplicar" (NOVO, STAGED CHANGES)
inline constexpr int kControlsBackIndex = 32;            // item de rodape "Voltar"
inline constexpr int kControlsItemCount = 33;            // 30 actions + Restaurar + Aplicar + Voltar

// ---------------------------------------------------------------- STAGED CHANGES (M2)
//
// Reforma de UX aprovada pelo lider na tela Controles: o modelo antigo
// "aplica na hora" (cada tecla capturada persistia em controls.json E valia
// no jogo imediatamente) foi TROCADO por "mudancas preparadas + Aplicar
// explicito", resolvendo a ambiguidade do "Voltar" sozinho (o jogador nao
// sabia se sair da tela desfazia ou nao o remap que acabou de fazer).
//
// Modelo novo (INVARIANTE central: NADA muda no jogo/disco ate Aplicar):
//   1. Ao ENTRAR na tela (chamador carrega controls.json, ver
//      system_menu_loop.cpp), controls_config = a COPIA DE TRABALHO (staged)
//      e controls_applied_config = o MESMO valor, a BASELINE (o que esta de
//      fato em disco/valendo no jogo agora). As duas comecam IGUAIS.
//   2. Remapear uma tecla (system_menu_controls_capture_key) OU confirmar
//      "Restaurar padrao" muta SO controls_config (a copia de trabalho) e
//      marca controls_dirty=true. controls_applied_config fica INTOCADO -
//      nada persiste em disco, nada aplica no input vivo do jogo.
//   3. "Aplicar" (kControlsApplyIndex): controls_applied_config =
//      controls_config (a copia de trabalho vira a nova baseline) e
//      controls_dirty=false. Devolve SystemMenuAction::ControlsApplied - o
//      CHAMADOR (loop) e quem de fato persiste em controls.json (esta funcao
//      so muta estado em MEMORIA, sem I/O). A tela NAO fecha (screen
//      intocado) - o jogador pode continuar remapeando.
//   4. "Voltar" (kControlsBackIndex) OU Esc na navegacao normal: se
//      controls_dirty, NAO navega ainda - abre controls_confirming_discard
//      (mini-dialogo "descartar alteracoes?", MESMA mecanica visual/de fluxo
//      de controls_confirming_restore). Se NAO dirty, navega direto pro pai
//      (comportamento de sempre, sem perguntar nada).
//   5. Confirmando o descarte: Sim reverte controls_config =
//      controls_applied_config (desfaz a copia de trabalho pra ultima
//      baseline aplicada/carregada), zera controls_dirty, e SO ENTAO navega
//      pro pai. Nao/Esc cancela o dialogo e permanece em Controles com a
//      copia de trabalho (e o dirty) intocados - o jogador pode continuar
//      editando ou tentar Voltar de novo.
//
// Os campos concretos (controls_dirty/controls_applied_config/
// controls_confirming_discard/controls_discard_confirm_selected) estao
// documentados junto dos demais campos de Controles em SystemMenuState, logo
// abaixo.

// Indices dos itens de Pause (ordem SAVE-LOAD-UI etapa 6 + MENU-INICIAL:
// Continuar, Salvar, Carregar, Configuracoes, Menu Inicial, Sair -
// "Carregar" inserido logo apos "Salvar" deslocando Configuracoes/Sair +1;
// "Menu Inicial" inserido ENTRE Configuracoes e Sair deslocando so Sair +1 -
// ver kPauseItemCount=6). ToTitle: volta pra tela de titulo SEM fechar o
// jogo (diferente de Quit, que encerra o processo) - ver
// SystemMenuAction::RequestToTitle e pause_confirming_to_title abaixo.
enum class PauseItem : int {
    Continue = 0,
    Save = 1,
    Load = 2,
    Settings = 3,
    ToTitle = 4,
    Quit = 5,
};

// Indices dos itens de ConfigCategories (ordem da arvore aprovada: Audio,
// Video, Controles, Lingua, Voltar - Controles inserido entre Video e Lingua,
// decisao de posicionamento do engine-graphics-programmer nao especificada pelo
// mock; trivial de reordenar, sinalizado no relatorio da dispatch).
enum class ConfigCategoryItem : int {
    Audio = 0,
    Video = 1,
    Controls = 2,
    Language = 3,
    Back = 4,
};

// Indices dos itens de Audio (ordem preservada da antiga tela "Config":
// Volume da Musica, Volume dos Efeitos, Voltar).
enum class AudioItem : int {
    Music = 0,
    Sfx = 1,
    Back = 2,
};

// Unico item navegavel das telas placeholder (Save/Video/Language): o botao
// Voltar (nao ha mais nada pra focar - "em breve", sem controle nenhum).
inline constexpr int kPlaceholderBackIndex = 0;

// Passo de ajuste de volume por tecla LEFT/RIGHT (5% - granularidade fina o
// bastante pra sentir a mudanca sem exigir dezenas de toques pra atravessar a
// faixa inteira). O mouse (system_menu_set_slider_ratio) nao usa este passo -
// aplica a fracao exata da posicao no track.
inline constexpr float kVolumeStep = 0.05f;

// Devolve o PAI fixo de `screen` na arvore (ver diagrama no topo do arquivo).
// Hidden e Pause nao tem pai na arvore de navegacao "Voltar/ESC sobe um nivel"
// (Pause e a raiz - ESC/Continuar la FECHA o menu inteiro, ver
// SystemMenuAction::Continue); chamar com Hidden ou Pause devolve o proprio
// argumento (defensivo, nunca deveria ser chamado nesses casos).
[[nodiscard]] SystemMenuScreen parent_screen_of(SystemMenuScreen screen) noexcept;

// Nome da action (ActionRegistry::action_name) na posicao `index` (0..
// kControlsActionCount-1) da lista CURADA/AGRUPADA da tela Controles
// (Movimento/Mundo/Combate/Menu&Dialogo - grupos do mock, estendidos pra cobrir
// as 30 actions do registry). index fora do intervalo devolve string_view vazia
// (defensivo). Fonte UNICA: system_menu_rml.cpp (render) e
// system_menu_controls_capture_key (aplicar o remap) usam esta MESMA funcao,
// nunca duplicam a lista.
[[nodiscard]] std::string_view controls_action_name_at(int index) noexcept;

// Indice do GRUPO visual (0=Movimento, 1=Mundo, 2=Combate, 3=Menu & Dialogo) da
// posicao `index` na mesma lista curada. index fora do intervalo devolve -1.
[[nodiscard]] int controls_group_at(int index) noexcept;

// Estado completo do menu (a UNICA fonte de verdade que a RML/RCSS le e que o
// chamador muta via as funcoes abaixo). music_volume/sfx_volume comecam em 1.0f
// (volume cheio) - o chamador os SOBRESCREVE apos carregar settings.json no
// boot (system_menu_open nao reseta esses dois campos, so a navegacao). Mesma
// convencao pra controls_config (M2): o chamador o SOBRESCREVE apos carregar
// controls.json do disco (ver gus/platform/fs/controls_file_store.hpp) - o
// default do campo (InputRemapConfig{} vazio) so vale antes desse load.
//
// Um campo *_selected POR TELA COM MAIS DE 1 ITEM (pause_selected/
// config_categories_selected/audio_selected/controls_selected): cada um
// preserva a ULTIMA selecao daquela tela mesmo quando a navegacao entra/sai de
// uma tela FILHA (ex.: abrir Audio e voltar pra ConfigCategories preserva
// config_categories_selected intocado - so a PROPRIA tela muda seu indice
// quando navegada). Telas placeholder (Save/Video/Language) nao precisam de
// campo proprio: tem 1 unico item (Voltar), sempre focado.
//
// CAMPOS DA TELA CONTROLES (M2): controls_selected indexa 0..kControlsActionCount-1
// (uma action da lista curada, ver controls_action_name_at) OU
// kControlsRestoreIndex/kControlsBackIndex (rodape). controls_capturing=true =
// "Pressione uma tecla..." (linha ciano do mock) - o PROXIMO evento de tecla
// real (fora do fluxo normal de navegacao) vira o novo binding via
// system_menu_controls_capture_key, NAO via system_menu_key_down (o CHAMADOR/
// loop decide qual dos dois rotear, consultando este flag).
// controls_confirming_restore=true = prompt "tem certeza?" antes de
// restaurar-padrao (decisao 4 do lider); controls_restore_confirm_selected
// (0=Sim, 1=Nao) comeca em Nao por seguranca (double-Enter acidental nao reseta
// os controles do jogador). controls_last_action_swapped/
// controls_last_swapped_with_*: transiente, populado logo apos um remap com
// troca (ver system_menu_controls_capture_key) pra UI mostrar o aviso "trocou
// com X" (decisao 1 do lider); limpo ao navegar pra outra linha.
//
// STAGED CHANGES (M2, ver o comentario grande acima de kControlsItemCount):
// controls_config e SEMPRE a COPIA DE TRABALHO agora (o que a UI mostra e o
// que captura/restaurar mutam) - controls_applied_config e a BASELINE (ultimo
// valor carregado do disco OU aplicado com sucesso, alvo do REVERT ao
// descartar). controls_dirty=true = ha mutacao na copia de trabalho ainda NAO
// aplicada (liga/desliga o brilho do botao Aplicar e o gate do dialogo de
// descarte no Voltar/Esc). controls_confirming_discard/
// controls_discard_confirm_selected: MESMA mecanica de
// controls_confirming_restore/controls_restore_confirm_selected acima, so
// que pro prompt "descartar alteracoes nao aplicadas?" (Voltar/Esc com
// dirty=true); default 1=Nao (fica editando) pela MESMA razao de seguranca.
struct SystemMenuState {
    SystemMenuScreen screen = SystemMenuScreen::Hidden;
    int pause_selected = 0;               // indice em PauseItem, valido em Pause
    int config_categories_selected = 0;   // indice em ConfigCategoryItem, valido em ConfigCategories
    int audio_selected = 0;               // indice em AudioItem, valido em Audio
    float music_volume = 1.0f;
    float sfx_volume = 1.0f;

    // ---- Menu Inicial (MENU-INICIAL) ----
    // Confirmacao "Voltar ao menu inicial?" (PauseItem::ToTitle) - MESMA
    // mecanica visual/de fluxo dos mini-dialogos de Controles acima
    // (controls_confirming_restore/discard): confirmar ToTitle NAO navega
    // direto, abre este prompt (substitui a lista de Pause por 2 pills
    // Sim/Cancelar, ver build_pause_body em system_menu_rml.cpp).
    bool pause_confirming_to_title = false;      // mostrando o prompt "voltar ao menu inicial?"
    int pause_to_title_confirm_selected = 1;     // 0=Sim, 1=Cancelar (default seguro, MESMA convencao dos prompts de Controles)

    // ---- Controles (M2 -> M2 STAGED CHANGES) ----
    gus::domain::input::InputRemapConfig controls_config;  // COPIA DE TRABALHO (staged) - carregada do disco pelo chamador no boot da tela
    gus::domain::input::InputRemapConfig controls_applied_config;  // BASELINE (ultimo carregado/aplicado) - alvo do revert ao descartar
    bool controls_dirty = false;                   // true = controls_config tem mudanca staged nao aplicada ainda
    int controls_selected = 0;                    // 0..kControlsItemCount-1, valido em Controls
    bool controls_capturing = false;               // aguardando a proxima tecla real
    bool controls_confirming_restore = false;      // mostrando o prompt "tem certeza?" (restaurar padrao)
    int controls_restore_confirm_selected = 1;     // 0=Sim, 1=Nao (default seguro)
    bool controls_confirming_discard = false;      // mostrando o prompt "descartar alteracoes?" (Voltar/Esc com dirty)
    int controls_discard_confirm_selected = 1;     // 0=Sim (descarta), 1=Nao (default seguro, fica editando)
    bool controls_last_action_swapped = false;     // ultimo remap aplicado trocou com outra action?
    std::string controls_last_swapped_with_action;      // nome da OUTRA action (vazio se !swapped)
    std::string controls_last_swapped_with_label_key;    // label i18n dela
};

// Resultado de um system_menu_key_down: o que o CHAMADOR deve fazer a seguir.
// A funcao so muta o SystemMenuState (navegacao/volume); o efeito de mundo
// (fechar o menu de fato, encerrar o programa, empurrar volume pro AudioEngine,
// persistir em disco) fica com quem chama, que le a action devolvida.
enum class SystemMenuAction {
    None,        // tecla sem efeito (nao mudou nada)
    Continue,    // Continuar confirmado (ou ESC na tela Pause, raiz da arvore) - feche o menu
    RequestQuit,  // Sair confirmado - o chamador decide encerrar o programa
    VolumeChanged,  // music_volume OU sfx_volume mudou - aplique no AudioEngine
                    // e persista em settings.json
    Navigated,   // state.screen mudou de tela (entrou numa FILHA OU voltou pro
                 // PAI) - screen/selected ja refletem o destino; o chamador so
                 // precisa recarregar o RML (mesmo efeito pratico de OpenX/
                 // BackToX de antes da arvore - unificado numa unica action
                 // generica porque o loop ja trata todas elas igual: reload).
    ControlsChanged,  // controls_config (COPIA DE TRABALHO, M2 STAGED CHANGES)
                      // mudou (remap aplicado OU restaurado pro padrao NA
                      // COPIA STAGED) - so recarregue o RML (controls_dirty
                      // ja ficou true); NAO persista em disco ainda (isso e
                      // papel EXCLUSIVO de ControlsApplied, abaixo).
    ControlsApplied,  // "Aplicar" confirmado - controls_applied_config ja foi
                      // atualizado pra controls_config e controls_dirty ja
                      // ficou false (efeito em MEMORIA, feito por esta
                      // camada); persista controls_config em controls.json
                      // (ver gus/platform/fs/controls_file_store.hpp) - o
                      // UNICO ponto que escreve em disco nesta tela agora. A
                      // tela NAO fecha (screen intocado).
    OpenSaveLoadSave,  // "Salvar" confirmado no Pause (SAVE-LOAD-UI etapa 6) -
                       // state.screen NAO muda (fica em Pause); o CHAMADOR
                       // (system_menu_loop.cpp) abre a tela REAL de save/load em
                       // modo Save (gus/app/screens/save_load_menu_loop.hpp),
                       // ANINHADA no MESMO contexto GL.
    OpenSaveLoadLoad,  // "Carregar" confirmado no Pause - idem, modo Load.
    RequestToTitle,    // "Sim" confirmado no mini-dialogo de Menu Inicial
                       // (MENU-INICIAL) - o CHAMADOR (system_menu_loop.cpp/
                       // Maestro) volta pra tela de TITULO (show_title_screen())
                       // SEM encerrar o processo - diferente de RequestQuit
                       // acima, que fecha o jogo. O menu de pausa nao se fecha
                       // sozinho aqui (mesmo espirito de RequestQuit: quem
                       // decide o efeito de mundo e o chamador).
};

// Abre o menu na tela PAUSA com foco inicial em Continuar (item 0, arvore). NAO
// mexe em music_volume/sfx_volume (o chamador os preenche uma vez no boot, a
// partir do settings.json carregado - system_menu_open so reresenta a
// NAVEGACAO, chamada toda vez que o jogador aperta Esc na pilha vazia).
void system_menu_open(SystemMenuState& state) noexcept;

// Fecha o menu incondicionalmente (screen -> Hidden), de qualquer tela atual.
// Usado pelo chamador quando a action devolvida por system_menu_key_down for
// Continue OU RequestQuit (ou por simetria/defensivamente a qualquer momento).
void system_menu_close(SystemMenuState& state) noexcept;

// Roteia UMA tecla pelo estado ATUAL do menu (qualquer tela da arvore) e
// devolve a action resultante. No-op seguro (devolve None) se screen==Hidden
// (o menu nao esta aberto - o chamador nao deveria chamar aqui, mas e
// defensivo) ou se a tecla nao tem efeito no estado/tela atual.
[[nodiscard]] SystemMenuAction system_menu_key_down(SystemMenuState& state,
                                                     SDL_Keycode key) noexcept;

// CAPTURA DE TECLA (tela Controles, M2): chamado pelo LOOP (nao por
// system_menu_key_down) quando state.controls_capturing==true e uma tecla real
// chega - nesse modo TODA tecla e candidata a virar o novo binding (nao so
// UP/DOWN/ENTER/ESC com o significado especial de navegacao), entao o
// roteamento generico de system_menu_key_down nao serve aqui. `is_escape=true`
// CANCELA a captura (Esc nunca vira binding, "Esc cancela a captura" - footer
// do mock); caso contrario `godot_keycode` (JA TRADUZIDO pelo chamador via
// gus::platform::input::sdl_key_to_godot_keycode - este header fica limpo de
// dependencia de platform/) vira o novo binding da action em
// state.controls_selected, via apply_key_remap (domain/input/
// controls_remap_apply.hpp - resolucao de conflito por TROCA, decisao 1 do
// lider). godot_keycode==0 (SENTINELA "tecla sem correspondente conhecida", ver
// key_translation.hpp) e ignorado defensivamente - permanece capturando (devolve
// None). Em qualquer desfecho que MUDE controls_config (novo binding aplicado
// OU cancelamento por Esc encerra so o modo, sem mudar config), controls_capturing
// volta a false. Devolve SystemMenuAction::ControlsChanged quando controls_config
// de fato mudou (o chamador deve persistir em disco); None caso contrario
// (cancelado, ou tecla sem correspondente/no-op).
[[nodiscard]] SystemMenuAction system_menu_controls_capture_key(
    SystemMenuState& state, bool is_escape, long long godot_keycode) noexcept;

// MOUSE (clique/arrasto no track do slider, tela Audio): define o volume do
// item dado (0=Music, 1=Sfx) pela FRACAO exata `ratio` (0.0=esquerda do track,
// 1.0=direita), clampada em [0,1]. Usado pelo host quando o clique/drag do
// mouse cai dentro do track (a casca calcula `ratio` a partir da geometria do
// elemento via glintfx::UiLayer::get_element_box + a posicao do cursor - ver
// system_menu_rml.hpp). item fora de {0,1} e no-op (Voltar nao e um slider).
void system_menu_set_slider_ratio(SystemMenuState& state, int item,
                                   float ratio) noexcept;

// MOUSE (clique numa OPCAO/pill/botao, roteado pelo hit-test de geometria do
// host via glintfx::UiLayer::get_element_box - ver system_menu_loop.cpp): trata
// o clique em `index` como "focar + confirmar" (EQUIVALENTE a mover o foco pra
// `index` e apertar ENTER), interpretado pela tela ATUAL (state.screen) -
// MESMA convencao de indices de system_menu_key_down:
//   Pause: index em PauseItem (Continuar/Salvar/Configuracoes/Sair) - SEMPRE
//     confirma na hora (mesma action de ENTER).
//   ConfigCategories: index em ConfigCategoryItem (Audio/Video/Lingua/Voltar) -
//     SEMPRE confirma na hora (categorias sao botoes simples, sem slider).
//   Audio: index em AudioItem (Musica/SFX/Voltar) - Voltar confirma na hora;
//     Musica/SFX SO FOCAM (index vira state.audio_selected, devolve None) -
//     clicar no NOME/rotulo de um slider nao ajusta o volume (isso e papel do
//     clique/arrasto no TRACK via system_menu_set_slider_ratio); focar so muda
//     o destaque visual/rota do teclado (LEFT/RIGHT/A/D passam a mexer naquele
//     item).
//   Save/Video/Language (placeholder): so index==kPlaceholderBackIndex existe
//     (Voltar) - confirma na hora.
// Hidden ou index fora do intervalo valido da tela atual: no-op (None).
[[nodiscard]] SystemMenuAction system_menu_click_option(SystemMenuState& state,
                                                         int index) noexcept;

// ---------------------------------------------------------------- HOVER (mouse)
//
// SOM DE HOVER (retoque ao vivo do lider, pos-ONDA ARVORE): o visual do hover e
// NATIVO do glintfx (RCSS :hover, ver system_menu_rml.cpp) - so o SOM precisa de
// logica PROPRIA de edge-detection (tocar so quando o item hovered MUDA, nao a
// cada frame parado sobre o mesmo item). As duas funcoes abaixo sao a fatia
// PURA/testavel dessa logica (o CHAMADOR - system_menu_loop.cpp - faz a consulta
// GL-heavy via glintfx::UiLayer::get_element_box e converte pro tipo local
// SystemMenuHoverBox abaixo, DE PROPOSITO sem depender de glintfx neste header -
// mesmo espirito POCO documentado no topo do arquivo).

// Caixa retangular MINIMA pro hit-test de hover - MESMO layout de campos que
// glintfx::ElementBox (found/x/y/w/h, ver glintfx/element_box.hpp), mas um tipo
// PROPRIO: este arquivo continua testavel sem incluir glintfx/GL/janela. O
// CHAMADOR converte glintfx::ElementBox -> SystemMenuHoverBox campo a campo
// (trivial) antes de chamar system_menu_hover_index.
struct SystemMenuHoverBox {
    bool found = false;
    float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
};

// BUG-A (achado NOVO por probe headless Xvfb :99 nesta investigacao M2 - causa
// raiz do "botao Voltar da tela Controles morto pro mouse"): `.ctrl-list` (M2,
// system_menu_rml.cpp) e `overflow-y:auto` com ALTURA FIXA (220dp) - o
// overflow SO CLIPA O PAINT (o que aparece na tela), NAO a geometria que
// get_element_box devolve. Com 30 actions e so ~6 linhas visiveis por vez, as
// linhas ROLADAS PRA FORA da janela visivel continuam tendo uma caixa REAL
// (posicao/tamanho de layout), que pode coincidir NUMERICAMENTE com a posicao
// do RODAPE (`.ctrl-foot`, Restaurar padrao/Voltar) - ele fica LOGO ABAIXO da
// altura reservada de 220dp da lista, e uma linha invisivel "ainda dentro" do
// fluxo de layout pode cair exatamente ali (medido empiricamente: a linha do
// item 6 - "Abrir inventario", ja rolada pra fora da vista - tinha uma caixa
// y=[419.4, 452.6] contra Voltar em y=[409.2, 436.4] - SOBREPOSICAO real). O
// hit-test/hover do CHAMADOR (system_menu_loop.cpp) testa os indices em ORDEM
// (0..31) e para no PRIMEIRO que bater - a linha invisivel (indice menor)
// ROUBA o clique/hover ANTES de chegar no rodape (indice 30/31), fazendo
// Voltar (e potencialmente Restaurar) parecerem MORTOS. FIX:
// controls_row_visible_in_list abaixo (POCO, 100% testavel) - o CHAMADOR
// consulta a caixa de `ctrl-list` UMA VEZ e filtra as 30 linhas de action
// (indices 0..kControlsActionCount-1, NUNCA o rodape - esse fica FORA da
// lista, sempre valido) por essa funcao ANTES de hit-test/hover-test:
// linhas sem NENHUMA sobreposicao vertical com o recorte visivel da lista
// nao contam mais como candidatas (elimina o roubo de clique/hover sem
// exigir scroll/windowing novo - so filtra o hit-test pela geometria ja
// existente).
[[nodiscard]] bool controls_row_visible_in_list(float row_top, float row_h,
                                                 float list_top, float list_h) noexcept;

// ------------------------------------------------ SCROLL (M2/GLINTFX-SCROLL)
//
// A v0.4.0 do glintfx entrega scroll DE VERDADE em embed mode
// (UiLayer::scroll_element_into_view + UiEvent::Type::MouseWheel) - antes
// disso `.ctrl-list` (overflow-y:auto) nunca rolava de fato (scrollTop ficava
// sempre 0), so ~6 das 30 actions eram alcancaveis por teclado/mouse (o resto
// ficava fora do recorte visivel de 220dp, ver controls_row_visible_in_list
// acima). As 2 funcoes abaixo sao a fatia PURA/testavel da integracao (o
// CHAMADOR - system_menu_loop.cpp - faz as chamadas GL-heavy de verdade:
// glintfx::UiLayer::scroll_element_into_view/process_event).

// Devolve o INDICE (0..kControlsItemCount-1) da linha que o CHAMADOR deve
// rolar pra dentro da vista via UiLayer::scroll_element_into_view (mapeando
// indice->id via controls_item_id, a MESMA convencao "controls-item-<indice>"
// ja usada pelo hit-test de clique em system_menu_loop.cpp), ou -1 quando
// nenhuma rolagem programatica faz sentido no estado ATUAL. -1 em 2 casos:
// (1) screen != Controls (as demais telas nao tem lista rolavel); (2) um dos
// mini-dialogos modais esta aberto (controls_confirming_restore/
// controls_confirming_discard) - as pills Sim/Nao desses prompts ficam FORA
// de `.ctrl-list` (ver system_menu_rml.cpp), entao a lista de actions por
// baixo NAO deve pular de posicao enquanto o prompt esta em foco. Fora desses
// 2 casos (inclui controls_capturing=true - "Pressione uma tecla...", MESMA
// linha state.controls_selected de antes de entrar em captura): devolve
// state.controls_selected - chamar scroll_element_into_view de novo pra uma
// linha que ja esta visivel e um no-op seguro (RmlUi so move o scroll quando
// precisa), entao nao ha necessidade de mais um caso especial aqui.
[[nodiscard]] int controls_scroll_target_index(const SystemMenuState& state) noexcept;

// Converte o delta de wheel do SDL (SDL_MouseWheelEvent.y - positivo = "away
// from user", rolar a roda pra CIMA no sentido tradicional) pro delta que o
// glintfx::UiEvent::Type::MouseWheel espera (positivo ROLA PRA BAIXO - ver o
// doc-comment do enum na v0.4.0, que cita Rml::Context::ProcessMouseWheel).
// Rolar a roda pra cima deve revelar conteudo ACIMA (scrollTop diminui, delta
// NEGATIVO no RmlUi) - por isso o sinal invertido. `flipped` cobre
// SDL_MOUSEWHEEL_FLIPPED (device/driver que inverte X/Y - a doc do SDL3
// manda multiplicar por -1 pra desfazer ANTES de aplicar qualquer outra
// convencao). Pura o bastante pra nao precisar de SDL_Event completo - so os
// 2 campos que importam pro calculo.
[[nodiscard]] float system_menu_wheel_delta_to_rmlui(float sdl_wheel_y, bool flipped) noexcept;

// Numero MAXIMO de itens hover-testaveis numa unica tela (M2: Controles tem
// kControlsItemCount=33, agora a MAIOR de todas - era Pause/ConfigCategories
// com 4 cada antes da tela Controles existir; Audio tem 3, Save/Video/Language
// tem 1) - dimensiona o array fixo de system_menu_hover_index abaixo. Posicoes
// do array alem do count relevante da tela atual sao ignoradas (o CHAMADOR so
// preenche as que importam).
inline constexpr int kSystemMenuMaxHoverItems = kControlsItemCount;

// HOVER (mouse, SEM clique): dado o mouse (mouse_x,mouse_y, espaco-janela) e as
// caixas dos itens NAVEGAVEIS da tela ATUAL (state.screen decide QUANTAS
// posicoes de `boxes` sao relevantes - Pause/ConfigCategories usam as 4,
// Audio usa as 3 primeiras (Musica/SFX/Voltar), Save/Video/Language usa so
// boxes[0] (Voltar); Hidden nao tem itens), devolve o INDICE (0..count-1) do
// PRIMEIRO item cuja caixa contem o ponto, ou -1 se nenhum bater (ou
// screen==Hidden). MESMO contrato de "found=false conta como fora" do hit-test
// de clique (system_menu_loop.cpp) - generalizado pra N caixas de uma vez e
// 100% testavel sem GL/janela (a QUERY GL-heavy - get_element_box - fica no
// CHAMADOR; aqui so a decisao geometrica "qual bateu").
[[nodiscard]] int system_menu_hover_index(
    const SystemMenuState& state, float mouse_x, float mouse_y,
    const SystemMenuHoverBox boxes[kSystemMenuMaxHoverItems]) noexcept;

// EDGE-DETECT: devolve true quando o hover ENTROU num item NOVO e VALIDO
// (current_index >= 0 && current_index != previous_index) - usado pelo
// chamador pra decidir se toca o SFX de hover SO na TRANSICAO. Parado sobre o
// MESMO item (current==previous) nao dispara de novo (evita o som repetir a
// cada frame/MouseMove parado). SAIR de um item pra fora de qualquer um
// (current_index==-1) tambem NAO dispara (so ENTRAR soa - mesmo espirito de
// "OnPointerEnter" das engines de UI convencionais, que nao tocam som no
// "OnPointerExit"). Sair e depois voltar pro MESMO item (com -1 no meio) volta
// a disparar - a "memoria" e so o ULTIMO indice visto, nao um historico.
[[nodiscard]] bool system_menu_hover_entered_new_item(int previous_index,
                                                       int current_index) noexcept;

// ------------------------------------------------ NAVEGACAO POR TECLADO (paridade de SFX)
//
// SOM DE HOVER PARIDADE TECLADO x MOUSE (retoque ao vivo do lider, pos-tela
// Controles aprovada): navegar por TECLADO/gamepad (mudar a selecao com
// setas/WASD, SEM confirmar) deve tocar o MESMO SFX de hover que o mouse ja
// toca ao entrar num item novo (ver system_menu_hover_entered_new_item acima).
// Diferente do hover de mouse (que precisa de uma consulta GL -
// glintfx::UiLayer::get_element_box - pra saber ONDE o cursor esta), o foco de
// TECLADO e 100% estado: cada tela ja guarda seu proprio indice selecionado
// (pause_selected/config_categories_selected/audio_selected/
// controls_selected, ou a pill Sim/Nao de um mini-dialogo aberto). Por isso a
// funcao abaixo e PURA desde sempre (sem contraparte GL-heavy no CHAMADOR, ao
// contrario de current_hover_index/system_menu_hover_index em
// system_menu_loop.cpp) - so LE o campo relevante da tela ATUAL.

// Indice do item ATUALMENTE com foco de TECLADO na tela state.screen - MESMA
// convencao de indices de system_menu_click_option: Pause=pause_selected,
// ConfigCategories=config_categories_selected, Audio=audio_selected,
// Controls= a pill do mini-dialogo ativo
// (controls_restore_confirm_selected/controls_discard_confirm_selected) OU
// controls_selected na navegacao normal - EXCETO controls_capturing==true
// (aguardando uma tecla FISICA virar binding - nao ha "navegacao" nesse modo,
// devolve -1 defensivamente, MESMO valor que "fora de qualquer item" no
// hover de mouse). Save/Video/Language (placeholder, 1 unico item) devolvem
// SEMPRE kPlaceholderBackIndex (nunca dispara edge-detect - nao ha pra onde
// "entrar", o unico item ja esta sempre focado). Hidden devolve -1 (menu
// fechado). Usado pelo CHAMADOR (system_menu_loop.cpp) junto com
// system_menu_hover_entered_new_item pra decidir quando tocar o SFX de hover
// na navegacao por teclado (MESMO choke-point audio.play_sfx(hover_sfx_id)
// que o hover de mouse ja usa - sem duplicar logica de som). O CHAMADOR e
// responsavel por so comparar indices ANTES/DEPOIS de uma MESMA tela (troca
// de tela muda o SIGNIFICADO do indice - comparar entre telas diferentes nao
// faz sentido, ver o guard `state.screen == screen_before` em
// system_menu_loop.cpp).
[[nodiscard]] int system_menu_keyboard_focus_index(const SystemMenuState& state) noexcept;

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_SYSTEM_MENU_HPP
