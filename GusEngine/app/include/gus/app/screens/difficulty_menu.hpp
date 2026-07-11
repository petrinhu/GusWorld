// gus/app/screens/difficulty_menu.hpp
//
// Logica PURA da TELA DE SELECAO DE DIFICULDADE (MODOS-MORTE Fase 0, decisao do
// lider 2026-07-10 - docs/design/mecanicas/modos-morte.md §2.2/§3.2). POCO 100%
// testavel sem SDL_Init/janela/glintfx nem disco - MESMO espirito de
// gus/app/screens/title_menu.hpp (o estado/navegacao vive aqui; a RENDERIZACAO via
// glintfx vive em difficulty_menu_rml.hpp/cpp; o loop GL/I/O de disco - gravar a
// dificuldade escolhida no SaveData novo - fica com o CHAMADOR, ver
// difficulty_menu_loop.hpp).
//
// ESCOPO (§2.2/§3.2, fiel a spec + REVISAO 2026-07-10 aprovada pelo lider no
// playtest ao vivo): 4 itens (Facil/Medio/Dificil/Hardcore). O Hardcore e
// "cenoura" - fica SEMPRE VISIVEL (scope-add do lider: conteudo travado mas a
// vista estimula o jogador a chegar la; "ninguem le manual", a descoberta tem
// que estar na UI) mas SO BLOQUEADO e alcancavel nesta Fase 0
// (state.hardcore_unlocked = false, hardcoded pelo CHAMADOR - ver o TODO em
// difficulty_menu_loop.cpp). Foco inicial em Medio (default canonico §2.1).
// Selecionar dispara o Aviso #2 (splash confirmar/cancelar, MESMA mecanica
// visual/de fluxo de confirming_new_game em title_menu.hpp - variante
// "informativa", nao "danger": nao e uma acao destrutiva, e uma decisao permanente
// mas positiva). Confirmar devolve a dificuldade escolhida ao CHAMADOR (que grava
// no SaveData novo e comeca o jogo); Cancelar volta pra LISTA (nao pra tela de
// titulo, decisao do lider §2.2). O Aviso #1 (legenda fixa sempre visivel enquanto
// o jogador navega) e SO texto estatico na RML (gus/app/screens/
// difficulty_menu_rml.hpp) - nao precisa de estado aqui.
//
// HARDCORE BLOQUEADO (Fase 0, UNICO estado alcancavel): visivel na lista,
// navegavel (Cima/Baixo PODE parar nele - o jogador le a descricao "algo mais
// sombrio..."), mas NAO selecionavel (difficulty_item_selectable devolve false)
// - Enter/clique nele e no-op TOTAL (MESMO padrao de "Continuar" desabilitado em
// title_menu.hpp/title_item_selectable: nao abre o splash, nao muda nada). SO
// destrava (hardcore_unlocked=true, ainda NAO alcancavel nesta fase - fiada pra
// Fase 4, ADR-015 + modos-morte.md §2.3b) quando entao vira selecionavel como os
// demais 3 itens.
//
// GAP preenchido nesta implementacao (nao coberto explicitamente pelo §2.2/§3.2):
// ESC na LISTA (fora do splash de confirmacao) devolve Cancelled - MESMA convencao
// de "ESC/Voltar = cancela e volta" ja estabelecida em TODAS as outras telas do
// jogo (system_menu.hpp, save_load_menu.hpp) - o CHAMADOR (Maestro) trata Cancelled
// como "abortar Novo Jogo, volta pra tela de titulo". Nenhuma spec cobria esse
// caminho explicitamente (a spec so descreve o fluxo feliz de escolher uma
// dificuldade); esta e a extensao minima e reversivel pra nao deixar a tela sem
// saida - reportado ao lider/orquestrador, nao e uma decisao de design nova (so
// aplica o padrao ja canonico do resto do jogo).
//
// Cross-ref: gus/app/screens/difficulty_menu_rml.hpp (RML/RCSS data-driven deste
//            estado); gus/app/screens/difficulty_menu_loop.hpp (o loop GL, ANINHADO
//            no MESMO contexto GL da tela de titulo - MESMA tecnica de
//            run_save_load_menu_loop_gl_current); gus/app/screens/title_menu.hpp
//            (StartNewGame dispara esta tela, ver title_menu_loop.cpp);
//            gus/domain/save/save_data.hpp (DifficultyLevel).

#ifndef GUS_APP_SCREENS_DIFFICULTY_MENU_HPP
#define GUS_APP_SCREENS_DIFFICULTY_MENU_HPP

#include <SDL3/SDL.h>  // SDL_Keycode

#include "gus/app/screens/ui_hover.hpp"  // COCKPIT-SFX-HOVER-CLIQUE: UiHoverBox/ui_hover_index
#include "gus/domain/save/save_data.hpp"  // DifficultyLevel

namespace gus::app::screens {

// Indices dos 4 itens da lista (ordem fiel a spec §2.2/§3.2 + REVISAO 2026-07-10:
// Hardcore agora aparece, SEMPRE por ultimo, "cenoura" visivel-bloqueada).
enum class DifficultyMenuItem : int {
    Facil = 0,
    Medio = 1,
    Dificil = 2,
    Hardcore = 3,
};

inline constexpr int kDifficultyItemCount = 4;

// Mapeia o indice do item (0..3) pro DifficultyLevel real do dominio (save_data.hpp)
// - a UNICA ponte entre a UI (ordem de exibicao) e o dado persistido. Indice fora
// do intervalo devolve Medio (defensivo, nunca deveria acontecer).
[[nodiscard]] constexpr gus::domain::save::DifficultyLevel
difficulty_level_for_item(int index) noexcept {
    switch (static_cast<DifficultyMenuItem>(index)) {
        case DifficultyMenuItem::Facil:
            return gus::domain::save::DifficultyLevel::Facil;
        case DifficultyMenuItem::Dificil:
            return gus::domain::save::DifficultyLevel::Dificil;
        case DifficultyMenuItem::Hardcore:
            return gus::domain::save::DifficultyLevel::Hardcore;
        case DifficultyMenuItem::Medio:
            return gus::domain::save::DifficultyLevel::Medio;
    }
    return gus::domain::save::DifficultyLevel::Medio;  // defensivo
}

// Estado completo da tela (a UNICA fonte de verdade que a RML le e que o
// CHAMADOR muta via as funcoes abaixo).
struct DifficultyMenuState {
    // Foco inicial = Medio (§2.1, default canonico/recomendado). Facil/Medio/
    // Dificil sao SEMPRE selecionaveis; Hardcore SO se hardcore_unlocked (ver
    // difficulty_item_selectable).
    int selected = static_cast<int>(DifficultyMenuItem::Medio);

    // Aviso #2 (§2.2): splash "Confirmar dificuldade: [X]" / "Confirmar" /
    // "Cancelar" - disparado ao SELECIONAR um item, ANTES de devolver a escolha ao
    // CHAMADOR. confirm_selected: 0=Confirmar, 1=Cancelar (default seguro, MESMA
    // convencao de confirm_selected em title_menu.hpp/save_load_menu.hpp - mesmo
    // esta variante sendo "informativa" e nao "danger", o padrao de foco-seguro do
    // projeto e consistente em toda confirmacao).
    bool confirming = false;
    int confirm_selected = 1;

    // Scope-add REVISAO 2026-07-10: apurado pelo CHAMADOR (title_menu_loop.cpp)
    // ao chamar difficulty_menu_open, MESMO racional de TitleMenuState::
    // any_save_exists (title_menu.hpp) - a tela nao decide sozinha, so LE o que
    // o CHAMADOR ja apurou. Nesta Fase 0 e SEMPRE false (unico estado
    // alcancavel - hardcoded no CHAMADOR, ver o TODO em
    // difficulty_menu_loop.cpp); Fase 4 lera da ancora selada (ADR-015 +
    // modos-morte.md §2.3b, pos-vitoria no Dificil).
    bool hardcore_unlocked = false;
};

// true se o item em `index` (0..kDifficultyItemCount-1) e SELECIONAVEL (Enter/
// clique confirmam) no estado ATUAL: Facil/Medio/Dificil sempre; Hardcore SO se
// state.hardcore_unlocked. index fora do intervalo devolve false (defensivo).
// MESMO papel de title_item_selectable (title_menu.hpp) - mas ao contrario de
// Continuar desabilitado (que a NAVEGACAO pula), o Hardcore bloqueado continua
// VISITAVEL por Cima/Baixo (o jogador pode focar nele pra ler a descricao
// "algo mais sombrio..." - so nao CONFIRMA, ver difficulty_menu_key_down/
// difficulty_menu_click_option).
[[nodiscard]] bool difficulty_item_selectable(const DifficultyMenuState& state,
                                               int index) noexcept;

// Abre a tela no estado inicial (foco em Medio, splash fechado).
// `hardcore_unlocked` apurado PELO CHAMADOR (Fase 0: sempre false, hardcoded -
// ver o TODO em difficulty_menu_loop.cpp; Fase 4: ancora selada). Chamado 1x
// pelo CHAMADOR ao entrar na tela (apos "Novo Jogo" confirmado na tela de
// titulo).
void difficulty_menu_open(DifficultyMenuState& state,
                           bool hardcore_unlocked = false) noexcept;

// Resultado de um difficulty_menu_key_down/difficulty_menu_click_option: o que o
// CHAMADOR deve fazer a seguir.
enum class DifficultyMenuAction {
    None,       // tecla/clique sem efeito (navegacao, splash ainda aberto, ou
                // Enter/clique num item BLOQUEADO - Hardcore nesta Fase 0)
    Chosen,     // Aviso #2 confirmado ("Confirmar") - state.selected e o item
                // ESCOLHIDO (usar difficulty_level_for_item(state.selected)); o
                // CHAMADOR grava a dificuldade no SaveData novo e comeca o jogo
    Cancelled,  // ESC/Voltar na LISTA (fora do splash) - o CHAMADOR aborta Novo
                // Jogo e volta pra tela de titulo (ver o comentario do GAP no
                // header)
};

// Roteia UMA tecla pelo estado ATUAL (lista OU splash de confirmacao aberto).
// Sobe/desce por wrap-around SEM pular itens BLOQUEADOS (SETAS + WASD, MESMA
// convencao de title_menu_key_down/system_menu.hpp - mas Hardcore bloqueado
// continua no ciclo de navegacao, ver difficulty_item_selectable); Enter/Espaco
// na lista SO abre o splash (confirming=true, confirm_selected=1 default
// seguro) se o item focado for selecionavel - num item BLOQUEADO e no-op TOTAL
// (None, MESMA semantica de "Continuar" desabilitado em title_menu.hpp); dentro
// do splash, LEFT/RIGHT/UP/DOWN/WASD alternam confirm_selected (0/1) e ENTER
// confirma a escolha atual (0=Chosen, 1=fecha o splash e volta pra lista,
// None); ESC no splash equivale a "Cancelar" (fecha o splash, None - MESMA
// seguranca de confirming_new_game em title_menu.hpp); ESC fora do splash
// devolve Cancelled (ver o comentario do GAP no header).
[[nodiscard]] DifficultyMenuAction difficulty_menu_key_down(
    DifficultyMenuState& state, SDL_Keycode key) noexcept;

// MOUSE (clique numa opcao/pill, roteado pelo hit-test de geometria do host - ver
// difficulty_menu_loop.cpp): trata o clique em `index` como "focar + confirmar"
// (EQUIVALENTE a mover o foco pra `index` e apertar ENTER, MESMA convencao de
// title_menu_click_option). Fora do splash: index em DifficultyMenuItem (0..3) -
// clicar num item BLOQUEADO (Hardcore, difficulty_item_selectable==false) e
// no-op TOTAL (nao muda nem o foco, MESMO padrao de title_menu_click_option pra
// "Continuar" desabilitado). Dentro do splash: index reinterpretado como a
// escolha (0=Confirmar, 1=Cancelar). index fora do intervalo valido: no-op
// (None).
[[nodiscard]] DifficultyMenuAction difficulty_menu_click_option(
    DifficultyMenuState& state, int index) noexcept;

// COCKPIT-SFX-HOVER-CLIQUE: indice de FOCO por teclado da tela ATUAL (lista OU
// splash) - MESMO papel de title_keyboard_focus_index.
[[nodiscard]] int difficulty_keyboard_focus_index(
    const DifficultyMenuState& state) noexcept;

// COCKPIT-SFX-HOVER-CLIQUE: hit-test de HOVER (mouse) 100% PURO/testavel - MESMO
// contrato de title_hover_index (delega pro POCO generico ui_hover_index,
// gus/app/screens/ui_hover.hpp). `boxes` tem kDifficultyItemCount posicoes (o
// maior dos dois modos); fora do splash as 3 primeiras (0..2) correspondem aos
// itens, dentro dele SO as 2 primeiras (0..1) sao relevantes. Devolve -1 se
// nenhuma bater.
[[nodiscard]] int difficulty_hover_index(
    const DifficultyMenuState& state, float mouse_x, float mouse_y,
    const UiHoverBox boxes[kDifficultyItemCount]) noexcept;

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_DIFFICULTY_MENU_HPP
