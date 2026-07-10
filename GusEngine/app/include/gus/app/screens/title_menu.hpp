// gus/app/screens/title_menu.hpp
//
// Logica PURA da TELA DE TITULO (SAVE-LOAD-UI etapa 4, decisao do lider). POCO
// 100% testavel sem SDL_Init/janela/glintfx nem disco - MESMO espirito de
// gus/app/screens/system_menu.hpp e gus/app/screens/save_load_menu.hpp (o
// estado/navegacao vive aqui; a RENDERIZACAO via glintfx vive em
// title_menu_rml.hpp/cpp; o I/O real de disco - varrer os slots, achar o mais
// recente, gus::platform::fs::load_game - fica com o CHAMADOR/loop, ver
// title_menu_loop.hpp).
//
// ESCOPO (mock docs/design/mockups/07-save-load.html, Tela 1 - TITULO; fiel a
// ele): 3 itens (Continuar / Novo Jogo / Sair), foco inicial em Continuar,
// navegacao Cima/Baixo com wrap, Enter seleciona. Fora de escopo (onda futura,
// ver TODO.md item SAVE-LOAD-UI): os 2 AVISOS de load (versao-incompativel/
// corrompido, controles-diferentes-do-save) - esses so existem DEPOIS de uma
// tentativa real de leitura em disco, tratados pelo CHAMADOR.
//
// REGRAS (decisao do lider, congeladas):
//   Continuar: carrega DIRETO o save mais recente por timestamp_ms entre TODOS
//     os slots ocupados (Auto + manuais) e cai no jogo - ver
//     gus::app::screens::most_recent_occupied_slot (save_load_menu.hpp), que o
//     CHAMADOR consulta apos varrer o disco. NENHUM slot ocupado => Continuar
//     DESABILITADO (nao selecionavel, nem por teclado/wrap nem por clique - ver
//     title_item_selectable).
//   Novo Jogo: se ha QUALQUER save gravado (any_save_exists), abre um
//     mini-dialogo Sim/Nao ("comecar novo jogo? o Auto sera sobrescrito
//     conforme jogar") ANTES de comecar - MESMA mecanica visual/de fluxo do
//     confirming_overwrite de save_load_menu.hpp. Confirmado (ou direto, se
//     nao ha save nenhum) => devolve StartNewGame; o CHAMADOR comeca do zero
//     (o estado FRESCO que Maestro::init() ja deixa pronto, sem retrabalho -
//     nao ha "sessao anterior" carregada ainda nesta fatia).
//   Sair: encerra o app (RequestQuit) - SEM confirmacao (mesmo padrao de
//     PauseItem::Quit em system_menu.hpp: nenhuma tela do jogo pede
//     confirmacao pra sair).
//
// Cross-ref: gus/app/screens/title_menu_rml.hpp (RML/RCSS data-driven deste
//            estado); gus/app/screens/title_menu_loop.hpp (o loop GL/glintfx +
//            o I/O real, unico ponto que le disco); gus/app/screens/
//            save_load_menu.hpp (most_recent_occupied_slot, SaveSlotPreview);
//            gus/app/maestro.cpp (show_title_screen, o CHAMADOR de producao -
//            substitui a entrada direta na cidade no boot).

#ifndef GUS_APP_SCREENS_TITLE_MENU_HPP
#define GUS_APP_SCREENS_TITLE_MENU_HPP

#include <SDL3/SDL.h>  // SDL_Keycode

namespace gus::app::screens {

// Indices dos 3 itens da tela (ordem fiel ao mock: Continuar, Novo Jogo, Sair).
enum class TitleMenuItem : int {
    Continue = 0,
    NewGame = 1,
    Quit = 2,
};

inline constexpr int kTitleItemCount = 3;

// Estado completo da tela (a UNICA fonte de verdade que a RML le e que o
// CHAMADOR muta via as funcoes abaixo).
struct TitleMenuState {
    int selected = 0;  // indice em TitleMenuItem, sempre um item SELECIONAVEL

    // Varrido do disco PELO CHAMADOR (title_menu_open nao le disco) - true se
    // QUALQUER slot (Auto ou manual) estiver ocupado. Controla 2 coisas: (1)
    // se Continuar e selecionavel (title_item_selectable); (2) se Novo Jogo
    // abre o mini-dialogo de confirmacao ou comeca direto.
    bool any_save_exists = false;

    // Mini-dialogo "comecar novo jogo? o Auto sera sobrescrito conforme
    // jogar" - MESMA mecanica de confirming_overwrite (save_load_menu.hpp).
    bool confirming_new_game = false;
    int confirm_selected = 1;  // 0=Sim (comeca), 1=Nao (default seguro)
};

// true se o item em `index` (0..kTitleItemCount-1) e SELECIONAVEL no estado
// ATUAL: Continuar so se state.any_save_exists; Novo Jogo/Sair sempre. index
// fora do intervalo devolve false (defensivo).
[[nodiscard]] bool title_item_selectable(const TitleMenuState& state,
                                          int index) noexcept;

// Abre a tela com o `any_save_exists` ja apurado pelo CHAMADOR (varredura real
// de disco - ver title_menu_loop.hpp). Foco inicial = Continuar SE
// selecionavel, senao Novo Jogo (Continuar desabilitado nunca comeca
// focado - mock: ".disabled ... nao selecionavel"). Reseta
// confirming_new_game/confirm_selected.
void title_menu_open(TitleMenuState& state, bool any_save_exists) noexcept;

// Resultado de um title_menu_key_down/title_menu_click_option: o que o
// CHAMADOR deve fazer a seguir.
enum class TitleMenuAction {
    None,           // tecla/clique sem efeito (so navegacao, ou dialogo ainda aberto)
    ContinueGame,   // Continuar confirmado - o CHAMADOR carrega o save mais
                    // recente (most_recent_occupied_slot) e aplica no jogo
    StartNewGame,   // Novo Jogo confirmado (direto, sem save nenhum; OU via
                    // "Sim" no mini-dialogo) - o CHAMADOR comeca do estado
                    // FRESCO (sem I/O de load)
    RequestQuit,    // Sair confirmado - o CHAMADOR encerra o programa
};

// Roteia UMA tecla pelo estado ATUAL (lista de itens OU mini-dialogo de Novo
// Jogo aberto). Sobe/desce SO por itens selecionaveis (title_item_selectable),
// com wrap-around (SETAS + WASD, MESMA convencao de system_menu.hpp/
// save_load_menu.hpp); Enter/Espaco delega pra regra documentada no header
// (Continuar desabilitado = None defensivo; Novo Jogo com save existente abre
// o mini-dialogo; Sair sempre RequestQuit). Dentro do mini-dialogo,
// LEFT/RIGHT/UP/DOWN/WASD alternam confirm_selected (0/1) e ENTER confirma a
// escolha atual; ESC no mini-dialogo equivale a "Nao" (mesma seguranca de
// controls_confirming_discard/confirming_overwrite). ESC fora do mini-dialogo
// e um no-op (a tela de titulo e a RAIZ - nao ha "pai" pra subir, MESMO
// espirito de Pause em system_menu.hpp, so que sem "Continuar fecha o menu"
// aqui: nao ha menu, e a tela de boot).
[[nodiscard]] TitleMenuAction title_menu_key_down(TitleMenuState& state,
                                                   SDL_Keycode key) noexcept;

// MOUSE (clique numa opcao/pill, roteado pelo hit-test de geometria do host -
// ver title_menu_loop.cpp): trata o clique em `index` como "focar + confirmar"
// (EQUIVALENTE a mover o foco pra `index` e apertar ENTER), MESMA convencao de
// system_menu_click_option/save_load_menu). Fora do mini-dialogo: index em
// TitleMenuItem (0..2) - clicar num item DESABILITADO (Continuar sem save) e
// no-op TOTAL (nao muda nem o foco, mock: "nao selecionavel"). Dentro do
// mini-dialogo: index reinterpretado como a escolha (0=Sim, 1=Nao). index
// fora do intervalo valido: no-op (None).
[[nodiscard]] TitleMenuAction title_menu_click_option(TitleMenuState& state,
                                                       int index) noexcept;

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_TITLE_MENU_HPP
