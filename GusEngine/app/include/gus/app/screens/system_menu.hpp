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
//   Pause (Continuar / Salvar / Configuracoes / Sair)
//     Save            -> placeholder "em breve"
//     ConfigCategories (Audio / Video / Lingua / Voltar)
//       Audio         -> Volume da Musica + Volume dos Efeitos + Voltar (a
//                         MESMA tela de sliders que antes se chamava "Config")
//       Video         -> placeholder "em breve"
//       Language      -> placeholder "em breve"
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
//            estado); gus/platform/audio/audio_engine.hpp (set_music_volume/
//            set_sfx_volume, consumidos pelo CHAMADOR quando VolumeChanged);
//            gus/platform/fs/settings_file_store.hpp (persistencia, idem);
//            gus/app/screens/battle_preview.hpp (battle_key_down, pilha de Esc
//            do combate - o Esc na PILHA VAZIA e o gancho que abre este menu).

#ifndef GUS_APP_SCREENS_SYSTEM_MENU_HPP
#define GUS_APP_SCREENS_SYSTEM_MENU_HPP

#include <SDL3/SDL.h>  // SDL_Keycode

namespace gus::app::screens {

// Tela ativa do menu de sistema. Hidden = menu fechado (jogo roda normal).
// Save/Video/Language sao PLACEHOLDER ("em breve" - as features reais sao
// pecas futuras: save real = M2-SAVE-IO, video/idioma ainda sem spec).
enum class SystemMenuScreen {
    Hidden,
    Pause,
    Save,              // placeholder, pai = Pause
    ConfigCategories,  // Audio/Video/Lingua/Voltar, pai = Pause
    Audio,             // sliders Musica/SFX (a antiga tela "Config"), pai = ConfigCategories
    Video,             // placeholder, pai = ConfigCategories
    Language,          // placeholder, pai = ConfigCategories
};

// Numero de itens navegaveis de cada tela (fonte unica do wrap de navegacao).
inline constexpr int kPauseItemCount = 4;             // Continuar/Salvar/Configuracoes/Sair
inline constexpr int kConfigCategoriesItemCount = 4;  // Audio/Video/Lingua/Voltar
inline constexpr int kAudioItemCount = 3;             // Musica/SFX/Voltar
inline constexpr int kPlaceholderItemCount = 1;       // so Voltar (Save/Video/Language)

// Indices dos itens de Pause (ordem da arvore aprovada: Continuar, Salvar,
// Configuracoes, Sair).
enum class PauseItem : int {
    Continue = 0,
    Save = 1,
    Settings = 2,
    Quit = 3,
};

// Indices dos itens de ConfigCategories (ordem da arvore aprovada: Audio,
// Video, Lingua, Voltar).
enum class ConfigCategoryItem : int {
    Audio = 0,
    Video = 1,
    Language = 2,
    Back = 3,
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

// Estado completo do menu (a UNICA fonte de verdade que a RML/RCSS le e que o
// chamador muta via as funcoes abaixo). music_volume/sfx_volume comecam em 1.0f
// (volume cheio) - o chamador os SOBRESCREVE apos carregar settings.json no
// boot (system_menu_open nao reseta esses dois campos, so a navegacao).
//
// Um campo *_selected POR TELA COM MAIS DE 1 ITEM (pause_selected/
// config_categories_selected/audio_selected): cada um preserva a ULTIMA
// selecao daquela tela mesmo quando a navegacao entra/sai de uma tela FILHA
// (ex.: abrir Audio e voltar pra ConfigCategories preserva
// config_categories_selected intocado - so a PROPRIA tela muda seu indice
// quando navegada). Telas placeholder (Save/Video/Language) nao precisam de
// campo proprio: tem 1 unico item (Voltar), sempre focado.
struct SystemMenuState {
    SystemMenuScreen screen = SystemMenuScreen::Hidden;
    int pause_selected = 0;               // indice em PauseItem, valido em Pause
    int config_categories_selected = 0;   // indice em ConfigCategoryItem, valido em ConfigCategories
    int audio_selected = 0;               // indice em AudioItem, valido em Audio
    float music_volume = 1.0f;
    float sfx_volume = 1.0f;
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

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_SYSTEM_MENU_HPP
