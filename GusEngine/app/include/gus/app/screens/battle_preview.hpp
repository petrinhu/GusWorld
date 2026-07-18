// gus/app/screens/battle_preview.hpp
//
// VIEWER da BattleScreen (M5, incremento 1): abre uma janela SDL direto na BattleScene
// pra o lider VER o esqueleto navegavel (arena + fila CTB + HUD) rodando na engine, sem
// passar pelo overworld. Mesmo padrao do --anim-preview (viewer dedicado, casca SDL
// fina, Esc/fechar sai). NAO conduz combate ainda; e validacao visual do layout.
//
// FORMA DE ENTRAR/SAIR (item 4 do incremento): o main pluga este viewer no modo
// "--battle". Entrar = rodar o app com --battle; sair = Esc ou fechar a janela (volta
// ao shell). Quando o loop de telas (Overworld<->Battle) existir (incremento futuro), a
// transicao real par.3.3 substitui este atalho; por ora ele isola a tela pra inspecao.
//
// M7-COSTURA (ADR-012 Onda 1): run_battle_preview_embedded() e a forma REUSAVEL que a
// Maestro chama - roda a MESMA BattleScene/loop numa janela JA EXISTENTE (a da cidade,
// no design "trocar escondido atras do preto": SDL_Renderer <-> contexto GL na MESMA
// janela) e devolve o CombatOutcome final via out-param. run_battle_preview() (o
// --battle standalone) virou um WRAPPER fino: cria sua PROPRIA janela + SDL_Init/Quit e
// delega pra run_battle_preview_embedded (outcome descartado - ninguem consome aqui).
//
// Cross-ref: gus/app/screens/battle_scene.hpp (a cena renderizada);
//            gus/app/screens/anim_preview.hpp (viewer irmao, mesmo padrao);
//            gus/app/maestro.hpp (dono da janela compartilhada, M7-COSTURA).

#ifndef GUS_APP_SCREENS_BATTLE_PREVIEW_HPP
#define GUS_APP_SCREENS_BATTLE_PREVIEW_HPP

#include <string>
#include <string_view>

#include <SDL3/SDL.h>  // SDL_Keycode (roteamento de teclado, testavel headless - ver abaixo)

#include "gus/app/screens/battle_scene.hpp"
#include "gus/core/asset_paths.hpp"             // kCityThemeFile (default de resolve_music_path)
#include "gus/domain/combat/combat_enums.hpp"  // CombatOutcome (out-param do embedded)
#include "gus/platform/audio/audio_engine.hpp"  // AudioEngine externo (M7-COSTURA Inc 2)

namespace gus::app::screens {

// Resolve a pasta dos retratos 48px (resources/sprites/icons-m5/retratos), na MESMA
// ordem do resolver de sprites do Gus: env GUSWORLD_ASSETS > macro embutido > relativo
// ao CWD. So monta a STRING (nao abre arquivo).
[[nodiscard]] std::string resolve_retratos_dir();

// Resolve a pasta dos icones de status (resources/sprites/icons-m5/status), na mesma
// ordem do resolver de retratos. So monta a STRING (nao abre arquivo).
[[nodiscard]] std::string resolve_status_icons_dir();

// M7-COSTURA Inc 2/3: resolve o caminho de uma faixa de MUSICA, mesma receita/ordem de
// resolve_retratos_dir (env GUSWORLD_MUSIC > macro embutida GUSWORLD_MUSIC_DIR >
// relativo ao CWD) + o NOME do arquivo dado em `file` (default kCityThemeFile, o
// comportamento de sempre - todo call-site existente sem argumento continua
// identico). Inc 3: a Maestro chama com `file=kBattleThemeFile` pra resolver a faixa
// da ARENA tambem (mesma pasta kMusicDir, so o nome do arquivo muda) - generalizacao
// MINIMA (so o parametro, zero env nova) em vez de duplicar a funcao inteira.
[[nodiscard]] std::string resolve_music_path(
    std::string_view file = gus::core::assets::kCityThemeFile);

// Digito 1-9 de uma tecla numerica (fileira OU numpad); 0 se nao for numerica 1-9. Fonte
// unica do mapeamento tecla->N pros atalhos numericos (mira e escolha de ator). Exposto
// (implementacao em battle_preview.cpp) pra ser CHAMAVEL pelo self-test sintetico E pelos
// testes Catch2 do roteamento de teclado (battle_key_routing_test.cpp) - headless, sem SDL_Init.
[[nodiscard]] int battle_digit_for_key(SDL_Keycode key) noexcept;

// MENU-PAUSA-CONFIG-SOM (M7-COSTURA, INTEGRACAO FINAL): sinal devolvido por
// battle_key_down (out-param opcional) quando o Esc bateu na PILHA VAZIA (nenhum
// modal de combate aberto). None = nada disso aconteceu (tecla sem esse efeito
// especifico). OpenPauseMenu = o Esc pediu o MENU DE PAUSA - so reportado quando o
// CHAMADOR passa um `out_effect` nao-nulo (ver battle_key_down); nesse caso
// `running` NAO e mexido (o viewer continua rodando; o chamador decide o que fazer
// com o pedido, tipicamente abrindo gus/app/screens/system_menu_loop.hpp).
enum class BattleEscEffect {
    None,
    OpenPauseMenu,
};

// Roteamento de TECLADO do host (extraido do loop de eventos - ver definicao/comentario
// completo em battle_preview.cpp). Esc DESEMPILHA 1 nivel de modal por vez (FIX bug2 do
// playtest do lider: "Esc fecha a tela" com um picker/preview aberto) - mira > preview de
// ator > picker > (pilha vazia). NA PILHA VAZIA, o comportamento depende de `out_effect`:
//   out_effect == nullptr (default - PRESERVA TODO call-site existente, inclusive os 8
//     casos de app/tests/battle_key_routing_test.cpp e o --battle STANDALONE/selftests):
//     `running` vira false (sai do viewer), COMPORTAMENTO INTACTO de sempre.
//   out_effect != nullptr (o HOST REAL, run_battle_preview_embedded): `running` NAO
//     muda; `*out_effect` vira OpenPauseMenu - o chamador abre o menu de pausa (nested
//     loop) e so entao decide se running deve virar false (ex.: o jogador escolheu Sair
//     ou fechou a janela DURANTE o menu).
// Exposto pra ser testavel headless (Catch2), sem abrir janela SDL nem chamar SDL_Init.
void battle_key_down(BattleScene& scene, SDL_Keycode key, bool& running,
                      BattleEscEffect* out_effect = nullptr);

// M7-COSTURA: roda o loop de batalha (mesma BattleScene/mesmo esqueleto) numa janela JA
// CRIADA por quem chama (a Maestro). NAO chama SDL_Init/SDL_Quit nem cria/destroi a
// janela - so o CONTEXTO GL (criado na entrada, destruido na saida; ver ADR-012 Onda 1,
// "trocar escondido atras do preto"). O loop sai por 3 motivos DISTINTOS: (1) o combate
// chegou a um desfecho TERMINAL (Victory/Defeat/Fled - fix BUG-2 do playtest ao vivo do
// lider: "perdi a batalha e ficou preso, so tocando musica" - o loop antigo so saia via
// Esc explicito, entao Defeat/Fled nunca eram detectados aqui e a tela ficava parada);
// (2) o jogador apertou Esc (pilha de modais vazia); (3) o jogador FECHOU A JANELA
// (SDL_EVENT_QUIT) - este ultimo e um sinal DISTINTO de qualquer CombatOutcome (fechar a
// janela nao e "vitoria/derrota/fuga", e "encerrar o PROGRAMA INTEIRO"; fix BUG-3: sem
// isto o quit era absorvido aqui e a Maestro reabria a cidade num LOOP INFINITO se o
// jogador ainda estivesse sobre o inimigo). Todos os 3 motivos convergem no MESMO
// choke-point: grava o CombatOutcome final (Victory/Defeat/Fled/Ongoing se a janela foi
// fechada no meio) em *out_outcome (se nao-nulo) E se o motivo foi especificamente (3) em
// *out_quit_requested (se nao-nulo; default false). A Maestro usa out_quit_requested pra
// decidir "encerrar o app" (nao "voltar pra cidade"). Devolve 0 ok, !=0 se a criacao do
// contexto GL ou o load de funcoes GL falhar (a janela segue viva - quem chamou decide o
// que fazer).
//
// M7-COSTURA Inc 2 (ADR-012 decisao 5, paga a divida do ADR-011 "AudioEngine e dono da
// battle_preview"):
//   external_audio: nullptr (default) = comportamento de SEMPRE - cria/possui um
//     AudioEngine LOCAL (destruido ao sair; o kit CC0 de musica/SFX toca/para aqui
//     mesmo, com fade-in/fade-out proprios) - o caminho do --battle STANDALONE
//     (run_battle_preview() abaixo) e de todo selftest/captura, INTOCADOS. Nao-nulo =
//     a Maestro passa o SEU AudioEngine (dono real, M7-COSTURA Inc 2) - esta funcao
//     SO usa (ponteiro nao-dono, mesmo padrao de BattleScene::set_audio), NUNCA toca
//     musica (a Maestro cronometra o crossfade com o fade preto por fora, ver
//     gus/app/maestro_logic.hpp::crossfade_music) - so carrega/dispara o SFX do hit
//     no engine externo (preserva a variante A/B GUSWORLD_HIT_SFX=alt).
//   fade_in_seconds/fade_out_seconds: <=0 (default) = SEM fade visual (comportamento
//     de sempre). >0 = desenha o overlay preto (gus/core/anim/fade_transition.hpp) por
//     cima da arena+HUD na ENTRADA (kIn, tela clareando sobre o 1o frame pronto, antes
//     do loop interativo comecar) e na SAIDA (kOut, tela escurecendo sobre o ULTIMO
//     frame congelado, depois que o loop interativo termina) - SO a Maestro pede isto
//     (o --battle standalone e os selftests pedem 0, preservando 100% o comportamento
//     anterior). O fade de SAIDA e PULADO se o motivo foi o jogador fechar a janela
//     (quit_requested) - fechar e imediato, sem segurar o jogador pra um fade que ele
//     nao pediu.
int run_battle_preview_embedded(
    SDL_Window* window, gus::domain::combat::CombatOutcome* out_outcome,
    bool* out_quit_requested = nullptr,
    gus::platform::audio::AudioEngine* external_audio = nullptr,
    float fade_in_seconds = 0.0f, float fade_out_seconds = 0.0f);

// FLASH-CTX (extracao behavior-preserving, A2): NUCLEO que ASSUME um contexto
// GL JA CORRENTE e com os ponteiros de funcao (glad) JA CARREGADOS - MESMO
// espirito de run_system_menu_loop_gl_current (system_menu_loop.hpp). Mesmos
// parametros/contrato de run_battle_preview_embedded (ver acima), so sem a
// posse do contexto GL (nao cria nem destroi). Hoje SO chamada internamente
// por run_battle_preview_embedded (unico chamador de producao continua sendo
// a Maestro, via a variante que possui o contexto); exposta pra futura
// reutilizacao aninhada (Opcao C, contexto GL unico).
void run_battle_preview_embedded_gl_current(
    SDL_Window* window, gus::domain::combat::CombatOutcome* out_outcome,
    bool* out_quit_requested = nullptr,
    gus::platform::audio::AudioEngine* external_audio = nullptr,
    float fade_in_seconds = 0.0f, float fade_out_seconds = 0.0f);

// Roda o viewer da BattleScene: SDL_Init proprio, janela PROPRIA, loop de render do
// esqueleto (camera logica 960x540 escalada por inteiro x2 = 1080p), Esc/fechar encerra.
// WRAPPER fino sobre run_battle_preview_embedded (outcome descartado). Devolve 0 ok.
int run_battle_preview();

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_PREVIEW_HPP
