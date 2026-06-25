// gus/app/screens/battle_scene.hpp
//
// BattleScene: a CENA DE BATALHA (M5), IRMA da city_scene. CAMADA DE APRESENTACAO em
// app/ (engine-design.md): LE o estado do motor de combate (domain/combat/) e o
// projeta no IRenderer via o layout PURO (battle_layout.hpp). O motor permanece POCO
// puro; a BattleScene nunca recalcula regra, so consulta + desenha.
//
// INCREMENTO 1 (ESQUELETO navegavel, SEM cartas/animacoes/numeros): a cena monta um
// encontro de DEMO (party + inimigos), guarda a CombatStateMachine pra ler a fila CTB
// e a contagem de atores, e desenha PLACEHOLDERS (retangulos coloridos + retratos 48px
// quando carregaveis) das zonas: arena (party esquerda / inimigos direita), fila CTB
// no topo, painel do ator ativo, caixa de log. NAO conduz turnos ainda.
//
// OWNERSHIP: a BattleScene e DONA dos CombatActor de demo (vector de unique_ptr) e da
// CombatStateMachine. A FSM/fila guardam ponteiros NAO-DONOS pros atores (mesmo padrao
// do dominio). A ordem de declaracao garante que os atores sobrevivem a FSM.
//
// RENDER 1:1: o layout vive em PIXELS LOGICOS 640x360 (D1); o render dirige a camera do
// IRenderer EXATAMENTE pra esse retangulo (px_per_world_unit = 1), entao cada Rect de
// layout vira um Rect de "mundo" identico. begin_frame recebe os PIXELS REAIS da janela
// pro backend escalar por inteiro (pixel-perfect).
//
// Cross-ref: gus/app/screens/battle_layout.hpp (layout puro, D1-D4/D7);
//            gus/domain/combat/combat_state_machine.hpp (motor, fila, atores);
//            gus/app/screens/city_scene.hpp (cena irma);
//            docs/design/mecanicas/battle-screen.md par.2/3/5.

#ifndef GUS_APP_SCREENS_BATTLE_SCENE_HPP
#define GUS_APP_SCREENS_BATTLE_SCENE_HPP

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "gus/app/screens/battle_log_model.hpp"  // LogLine
#include "gus/app/screens/battle_menu.hpp"       // BattleMenu / BattleVerb
#include "gus/domain/combat/combat_enums.hpp"    // StatusId
#include "gus/domain/combat/combat_records.hpp"  // CombatAction
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

namespace gus::domain::combat {
class CombatActor;
}

namespace gus::app::screens {

// Sprites de retrato 48px (fila CTB) ja resolvidos para TextureId pela casca SDL. POCO
// sem SDL: a cena so guarda os handles e escolhe qual mostrar por ator. kInvalidTexture
// => o render degrada pro retangulo (headless / "sem arte"). Index = posicao na fila.
struct BattlePortraitSet {
    // Retrato por ator-id (mapeado pela casca a partir de resources/.../retratos/).
    // Vazio => todas as celulas CTB caem no placeholder retangulo.
    std::vector<std::pair<std::string, gus::platform::render2d::TextureId>> by_id;

    // Lookup do handle por id de ator. kInvalidTexture se ausente.
    [[nodiscard]] gus::platform::render2d::TextureId find(
        const std::string& id) const noexcept;
};

// Icones de status (14px) ja resolvidos para TextureId pela casca SDL, indexados por
// StatusId (status_icon_index). POCO sem SDL: a cena so guarda os handles e escolhe
// qual mostrar pelos status_effects() do ator. kInvalidTexture em qualquer slot =>
// aquele status cai num quadradinho placeholder (headless / "sem arte").
struct BattleStatusIconSet {
    // [status_icon_index(id)] = handle. Vazio/ausente => placeholder.
    std::array<gus::platform::render2d::TextureId, /*kStatusIdCount=*/13> by_index{};

    // Handle do icone de um StatusId. kInvalidTexture se nao carregado.
    [[nodiscard]] gus::platform::render2d::TextureId find(
        gus::domain::combat::StatusId id) const noexcept;
};

class BattleScene {
public:
    // Monta um encontro de DEMO (party de 3 com Gus + 4 inimigos) pra validar a tela
    // navegavel do M5. Os atores e a FSM ficam prontos; a fila ja esta ordenada por
    // SPD (a cena le queue().order()). NAO inicia/conduz turnos no incremento 1.
    BattleScene();

    BattleScene(const BattleScene&) = delete;
    BattleScene& operator=(const BattleScene&) = delete;

    // Define os retratos 48px (handles ja resolvidos pelo renderer). Se nao for
    // chamado (ou incompleto), as celulas CTB caem no retangulo (fallback headless).
    void set_portraits(BattlePortraitSet portraits) noexcept {
        portraits_ = std::move(portraits);
    }

    // Define os icones de status (handles ja resolvidos pelo renderer). Se nao for
    // chamado, os status do ator caem num quadradinho placeholder (fallback headless).
    void set_status_icons(BattleStatusIconSet icons) noexcept {
        status_icons_ = icons;
    }

    // ---- Leitura do estado do motor (pro render e pros testes) ----

    // Contagem de atores VIVOS por lado (alimenta arena_layout).
    [[nodiscard]] int party_count() const;
    [[nodiscard]] int enemy_count() const;
    // Tamanho da fila de iniciativa (alimenta ctb_strip).
    [[nodiscard]] int queue_len() const noexcept;
    // Ator ativo (turno corrente) pra o highlight (D7). Nunca nullptr com fila nao-vazia.
    [[nodiscard]] const gus::domain::combat::CombatActor* active_actor() const noexcept;

    // Indice na coluna da party que e o GUS (pro recuo D3). -1 se o Gus nao esta na
    // party viva. Deriva de is_universal_compiler (so o Gus e compilador universal).
    [[nodiscard]] int gus_party_index() const;

    // A FSM (leitura): a cena consulta queue()/active_actor()/log() sem mutar.
    [[nodiscard]] const gus::domain::combat::CombatStateMachine& machine() const noexcept {
        return *machine_;
    }

    // ---- Conducao de turno (incremento 3): menu comando-first + FSM real ----
    //
    // MODELO DE DRIVE (reportado ao coordenador): o provider da FSM e um MAILBOX (uma
    // acao por vez). No turno do JOGADOR, o menu seta UMA acao e a cena roda o turno ate
    // o fim (a acao resolve, depois Pass encerra) - ou seja, 1 verbo = 1 turno no
    // incremento 3 (multi-acao por turno e refinamento do overlay, incr 4+). No turno do
    // INIMIGO, a cena auto-resolve (ataque basico num alvo vivo) e avanca. Apos qualquer
    // acao, a cena auto-encadeia os turnos de inimigo ate o proximo turno de JOGADOR ou
    // o fim do combate.

    // true se o ator ativo e do lado do JOGADOR (mostra o menu e ESPERA input). false =
    // inimigo (a cena auto-resolve via step_until_player_or_end).
    [[nodiscard]] bool current_actor_is_player() const noexcept;

    // true se o combate JA terminou (Victory/Defeat/Fled). O menu nao opera mais.
    [[nodiscard]] bool combat_over() const noexcept;

    // O menu de verbos do ator ativo (leitura, pro render).
    [[nodiscard]] const BattleMenu& menu() const noexcept { return menu_; }

    // Navega a selecao do menu (cima/baixo). No-op se nao e turno de jogador ou acabou.
    void menu_move(int delta) noexcept;

    // Confirma o verbo selecionado: monta a CombatAction real e conduz o turno (no caso
    // do Compilar, apenas sinaliza "abriria o overlay" no log; incremento 4). No-op se
    // o verbo esta desabilitado, nao e turno de jogador ou o combate acabou. Apos
    // resolver, auto-encadeia turnos de inimigo ate o proximo turno de jogador/fim.
    void menu_confirm();

    // Linhas NOTAVEIS do log (D7), ja classificadas/cortadas pra caixa (max_lines).
    [[nodiscard]] std::vector<LogLine> log_lines(int max_lines) const;

    // Desenha o ESQUELETO da batalha (placeholders): fundo, arena (party/inimigos),
    // fila CTB, painel do ator ativo, log. viewport_px_w/h = PIXELS REAIS da janela
    // (o backend escala 640x360 por inteiro). LE a contagem/ordem do motor.
    void render(gus::platform::render2d::IRenderer& renderer, float viewport_px_w,
                float viewport_px_h) const;

private:
    // Provider da FSM = mailbox: devolve pending_action_ e o reseta pra Pass. Usado
    // tanto pro jogador (acao do menu) quanto pro inimigo (acao auto). Definido no ctor.
    [[nodiscard]] gus::domain::combat::CombatAction take_pending_action(
        gus::domain::combat::CombatActor& actor);

    // Comeca o turno do ator ativo (begin_turn) e, se ele perder o turno (Stun) ou for
    // inimigo, auto-encadeia ate cair num turno de JOGADOR vivo ou o combate terminar.
    void step_until_player_or_end();

    // Resolve UM turno do ator ativo com a acao ja no mailbox (run_active_turn_to_end +
    // check_end + advance). Atualiza a habilitacao do menu pro proximo ator.
    void resolve_current_turn();

    // Acao automatica de um inimigo: ataque basico no 1o player vivo (POCO-ish; o alvo
    // vem de first_alive_player). Pass se nao houver player vivo.
    [[nodiscard]] gus::domain::combat::CombatAction enemy_auto_action(
        const gus::domain::combat::CombatActor& enemy) const;

    // Primeiro inimigo vivo (alvo default das acoes ofensivas do jogador). nullptr se
    // nenhum. Primeiro player vivo (alvo das acoes de inimigo). nullptr se nenhum.
    [[nodiscard]] gus::domain::combat::CombatActor* first_alive_enemy() const;
    [[nodiscard]] gus::domain::combat::CombatActor* first_alive_player() const;

    // Atores de demo (DONOS). Declarados ANTES da FSM: vivem mais que ela.
    std::vector<std::unique_ptr<gus::domain::combat::CombatActor>> actors_;
    std::unique_ptr<gus::domain::combat::CombatStateMachine> machine_;
    BattlePortraitSet portraits_{};
    BattleStatusIconSet status_icons_{};

    // Menu de verbos do ator ativo (incremento 3).
    BattleMenu menu_{};
    // Mailbox de acao do provider (1 por vez). Default Pass = "nada a fazer".
    gus::domain::combat::CombatAction pending_action_{
        gus::domain::combat::CombatAction::pass()};
    // Linhas de log geradas pela UI (nao pelo motor): COMPILAR/GAMBITO sinalizados antes
    // de existir mecanica (incr 4/5). Mescladas com o log do motor em log_lines().
    std::vector<LogLine> ui_log_;
};

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_SCENE_HPP
