// gus/app/screens/system_menu.hpp
//
// Logica PURA do MENU DE SISTEMA (pausa + config de som), MENU-PAUSA-CONFIG-SOM
// (M7-COSTURA). POCO 100% testavel sem SDL_Init/janela/glintfx - so o estado de
// navegacao/selecao/volume (mesmo espirito de gus/app/maestro_logic.hpp e da
// funcao-livre battle_key_down em battle_preview.hpp). Consome SDL_Keycode
// diretamente (app/ ja e camada SDL-aware, mesmo padrao de battle_key_down) mas
// NAO abre janela nem chama SDL_Init - so compara o valor do enum.
//
// REPLICA O MOCK APROVADO (docs/design/mockups/
// 01-menu-sistema-proposta-a-console-centralizado.html, "Console Centralizado"):
//   Tela PAUSA: 3 verbos (Continuar/Configuracoes/Sair), foco inicial=Continuar.
//   Tela CONFIG: 2 sliders (Musica/SFX) + Voltar, foco inicial=Musica.
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
enum class SystemMenuScreen {
    Hidden,
    Pause,
    Config,
};

// Numero de itens navegaveis de cada tela (fonte unica do wrap de navegacao).
inline constexpr int kPauseItemCount = 3;   // Continuar / Configuracoes / Sair
inline constexpr int kConfigItemCount = 3;  // Musica / SFX / Voltar

// Indices dos itens de Pause (ordem do mock: Continuar, Configuracoes, Sair).
enum class PauseItem : int {
    Continue = 0,
    Settings = 1,
    Quit = 2,
};

// Indices dos itens de Config (ordem do mock: Volume da Musica, Volume dos
// Efeitos, Voltar).
enum class ConfigItem : int {
    Music = 0,
    Sfx = 1,
    Back = 2,
};

// Passo de ajuste de volume por tecla LEFT/RIGHT (5% - granularidade fina o
// bastante pra sentir a mudanca sem exigir dezenas de toques pra atravessar a
// faixa inteira). O mouse (system_menu_set_slider_ratio) nao usa este passo -
// aplica a fracao exata da posicao no track.
inline constexpr float kVolumeStep = 0.05f;

// Estado completo do menu (a UNICA fonte de verdade que a RML/RCSS le e que o
// chamador muta via as funcoes abaixo). music_volume/sfx_volume comecam em 1.0f
// (volume cheio) - o chamador os SOBRESCREVE apos carregar settings.json no
// boot (system_menu_open nao reseta esses dois campos, so a navegacao).
struct SystemMenuState {
    SystemMenuScreen screen = SystemMenuScreen::Hidden;
    int pause_selected = 0;   // indice em PauseItem, valido quando screen==Pause
    int config_selected = 0;  // indice em ConfigItem, valido quando screen==Config
    float music_volume = 1.0f;
    float sfx_volume = 1.0f;
};

// Resultado de um system_menu_key_down: o que o CHAMADOR deve fazer a seguir.
// A funcao so muta o SystemMenuState (navegacao/volume); o efeito de mundo
// (fechar o menu de fato, encerrar o programa, empurrar volume pro AudioEngine,
// persistir em disco) fica com quem chama, que le a action devolvida.
enum class SystemMenuAction {
    None,           // tecla sem efeito (nao mudou nada)
    Continue,       // Continuar confirmado (ou ESC na tela Pause) - feche o menu
    OpenSettings,    // entrou na tela Config (screen ja virou Config)
    BackToPause,    // saiu de Config de volta pra Pause (screen ja virou Pause)
    RequestQuit,    // Sair confirmado - o chamador decide encerrar o programa
    VolumeChanged,  // music_volume OU sfx_volume mudou - aplique no AudioEngine
                    // e persista em settings.json
};

// Abre o menu na tela PAUSA com foco inicial em Continuar (item 0, mock). NAO
// mexe em music_volume/sfx_volume (o chamador os preenche uma vez no boot, a
// partir do settings.json carregado - system_menu_open so reresenta a
// NAVEGACAO, chamada toda vez que o jogador aperta Esc na pilha vazia).
void system_menu_open(SystemMenuState& state) noexcept;

// Fecha o menu incondicionalmente (screen -> Hidden), de qualquer tela atual.
// Usado pelo chamador quando a action devolvida por system_menu_key_down for
// Continue OU RequestQuit (ou por simetria/defensivamente a qualquer momento).
void system_menu_close(SystemMenuState& state) noexcept;

// Roteia UMA tecla pelo estado ATUAL do menu (Pause ou Config) e devolve a
// action resultante. No-op seguro (devolve None) se screen==Hidden (o menu nao
// esta aberto - o chamador nao deveria chamar aqui, mas e defensivo) ou se a
// tecla nao tem efeito no estado/tela atual.
[[nodiscard]] SystemMenuAction system_menu_key_down(SystemMenuState& state,
                                                     SDL_Keycode key) noexcept;

// MOUSE (clique/arrasto no track do slider, tela Config): define o volume do
// item dado (0=Music, 1=Sfx) pela FRACAO exata `ratio` (0.0=esquerda do track,
// 1.0=direita), clampada em [0,1]. Usado pelo host quando o clique/drag do
// mouse cai dentro do track (a casca calcula `ratio` a partir da geometria do
// elemento via glintfx::UiLayer::get_element_box + a posicao do cursor - ver
// system_menu_rml.hpp). item fora de {0,1} e no-op (Voltar nao e um slider).
void system_menu_set_slider_ratio(SystemMenuState& state, int item,
                                   float ratio) noexcept;

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_SYSTEM_MENU_HPP
