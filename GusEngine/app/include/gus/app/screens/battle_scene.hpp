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
// RENDER 1:1: o layout vive em PIXELS LOGICOS 960x540 (D1); o render dirige a camera do
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
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <unordered_map>

#include "gus/app/i18n/translator.hpp"            // Translator (tr() de UI)
#include "gus/app/screens/battle_floaters.hpp"    // Floater (numeros flutuantes)
#include "gus/app/screens/battle_log_model.hpp"   // LogLine
#include "gus/app/screens/battle_menu.hpp"        // BattleMenu / BattleVerb
#include "gus/app/screens/battle_pacing.hpp"      // PacingDirector (ritmo, D8)
#include "gus/domain/combat/combat_enums.hpp"     // StatusId
#include "gus/domain/combat/combat_records.hpp"   // CombatAction / IntentPreview
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/domain/combat/scripted_brain.hpp"   // ScriptedBrain (telegraph)
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

// Icones de INTENT (telegraph, incremento 5) ja resolvidos para TextureId. O ScriptedBrain
// expoe IntentPreview (o que o inimigo vai fazer); a cena mostra o icone sobre o inimigo.
// Os 4 icones de resources/sprites/icons-m5/intent/: atacar, defender, aplicar_status,
// ruido (Patch-Zero / caotico). kInvalidTexture => placeholder (marca) no render.
struct BattleIntentIconSet {
    gus::platform::render2d::TextureId atacar = 0;
    gus::platform::render2d::TextureId defender = 0;
    gus::platform::render2d::TextureId aplicar_status = 0;
    gus::platform::render2d::TextureId ruido = 0;  // intent caotico (is_chaotic)
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

    // Define o Translator de UI (tr() dos rotulos de verbo). Ponteiro NAO-DONO (vive na
    // casca/main). nullptr (default) => sem texto traduzido; o render cai pro fallback
    // (rotulo = a chave, ou as caixas/marcas sem texto). Mantem o headless seguro.
    void set_translator(const gus::app::i18n::Translator* tr) noexcept {
        translator_ = tr;
    }

    // Define os icones de intent (telegraph, incremento 5). Sem set => o intent vira uma
    // marca placeholder sobre o inimigo (fallback headless).
    void set_intent_icons(BattleIntentIconSet icons) noexcept {
        intent_icons_ = icons;
    }

    // HUD EXTERNO (ADR-009): quando true, o COCKPIT (painel do ator + menu de verbos) e o
    // LOG/terminal NAO sao desenhados a mao por esta cena - eles viram 100% RmlUi (GL3),
    // desenhados POR CIMA. A cena segue dona de arena/fila-CTB/banner/floaters. Evita os
    // DOIS cockpits sobrepostos (o a-mao + o RmlUi). Default false (cockpit a-mao, como
    // antes / headless / testes). So a casca (battle_preview) liga quando o RmlUi esta on.
    void set_hud_external(bool external) noexcept { hud_external_ = external; }
    [[nodiscard]] bool hud_external() const noexcept { return hud_external_; }

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

    // Linhas do log (D7 revisado, incremento 5: NARRA todo o combate), ja classificadas
    // (cor por categoria) e cortadas pra caixa (max_lines = ultimas N).
    [[nodiscard]] std::vector<LogLine> log_lines(int max_lines) const;

    // ---- Feedback (incremento 5): numeros flutuantes + intent ----

    // Avanca o tempo da cena (segundos): envelhece os numeros flutuantes e poda os
    // mortos. Chamado pela casca a cada frame (dt real). NAO toca a FSM (so animacao).
    void update(float dt_seconds);

    // Numeros flutuantes ATIVOS (leitura/teste). Cada um sobe + some pela idade.
    [[nodiscard]] const std::vector<Floater>& floaters() const noexcept {
        return floaters_;
    }

    // IntentPreview do inimigo (telegraph). nullopt se nao for inimigo vivo, sem brain,
    // ou o combate acabou. Le o ScriptedBrain registrado (preview_intent).
    [[nodiscard]] std::optional<gus::domain::combat::IntentPreview> intent_for(
        const gus::domain::combat::CombatActor& enemy) const;

    // ---- Ritmo / pacing (incremento 6, D8/D9/D10) ----

    // Estado do ritmo (intro / espera-delay / espera-input). Leitura pro render (D9/D10).
    [[nodiscard]] PacingState pacing_state() const noexcept { return pacing_.state(); }

    // true durante a abertura PARADA (D10): a casca mostra "BATALHA!" + o prompt e
    // ninguem agiu. A abertura ESPERA o jogador ENCARAR (nao auto-avanca).
    [[nodiscard]] bool is_intro() const noexcept {
        return pacing_.state() == PacingState::Intro;
    }

    // true quando e a vez do JOGADOR e o ritmo espera o menu (D9 "sua vez").
    [[nodiscard]] bool waiting_player_input() const noexcept {
        return pacing_.waiting_player_input();
    }

    // ENCARAR (decisao do lider 2026-06-25): o jogador escolheu comecar o combate na
    // abertura. Sai do "BATALHA!" parado e o 1o turno comeca a animar (2 beats). No-op se
    // nao esta na abertura. A casca chama em Enter/Espaco/KP_Enter.
    void start_combat();

    // RESOLVER SEM ENCARAR (verbo OPT-IN, so TRASH; placeholder neste incremento). Por
    // ora SO sinaliza no log "[auto-resolve: a implementar]" e NAO faz nada destrutivo.
    // O auto-resolve real (motor headless + penalidade por selo) vira num incremento
    // SEPARADO depois que o design for canonizado. A casca chama em Q (so se oferecido).
    void request_auto_resolve();

    // true se a abertura oferece o verbo "[Q] Resolver sem encarar": SO quando todos os
    // inimigos sao TRASH (no demo, todos sao). Boss/elite (futuro) escondem o verbo.
    [[nodiscard]] bool offers_auto_resolve() const;

    // Tecla de ACELERAR/avancar (D8): encurta o delay/anuncio entre turnos. NAO pula a
    // abertura (espera Encarar) nem o turno do jogador (espera o menu). A casca chama
    // numa tecla.
    void skip();

    // Chave i18n do banner de turno (D9): "TURNO DE <nome>" pro jogador / "vez do
    // inimigo" / "BATALHA!" na intro. A casca resolve via tr() + o nome do ator ativo.
    // Devolve a CHAVE base; a casca formata. Vazio se combate acabou.
    [[nodiscard]] std::string_view turn_banner_key() const noexcept;

    // ---- Modo-mira / target selection (battle-screen.md §3.5, D3) ----
    //
    // APRESENTACAO PURA (zero motor): ao escolher [Atacar] ou [Scan] no menu de verbos, a
    // cena ENTRA em modo-mira em vez de resolver na hora com o 1o inimigo (o hardcode
    // antigo). O jogador navega entre os inimigos VIVOS e confirma; a CombatAction e
    // montada com o ALVO ESCOLHIDO. Cancelar volta ao menu sem consumir o turno. O motor
    // (domain/combat) ja resolve qualquer target_id (resolve_primary_target); aqui so
    // deixamos o jogador escolher. So teclado neste incremento (mouse = item separado).

    // true enquanto o modo-mira esta ativo (esperando o jogador escolher o alvo).
    [[nodiscard]] bool is_aiming() const noexcept { return aiming_; }

    // Navega o cursor de mira entre os inimigos vivos (delta -1/+1, com WRAP). No-op fora
    // do modo-mira. A lista exclui mortos (mira nunca pousa num inimigo morto).
    void aim_move(int delta) noexcept;

    // Inimigo atualmente mirado (pro render do destaque + testes). nullptr fora da mira.
    [[nodiscard]] const gus::domain::combat::CombatActor* aim_target() const noexcept;

    // Quantidade de inimigos MIRAVEIS (vivos) na sessao de mira atual. 0 fora da mira.
    [[nodiscard]] int aim_count() const noexcept {
        return static_cast<int>(aim_candidates_.size());
    }

    // Confirma o alvo mirado: monta CombatAction::attack/scan com o ALVO ESCOLHIDO e
    // resolve o turno (mesmo fluxo do menu_confirm antigo: pending_action_ +
    // resolve_one_turn + pacing_.player_acted). No-op fora do modo-mira.
    void aim_confirm();

    // Cancela o modo-mira e volta ao menu de verbos SEM consumir o turno (Esc). No-op se
    // nao esta mirando.
    void aim_cancel() noexcept;

    // Desenha o ESQUELETO da batalha (placeholders): fundo, arena (party/inimigos),
    // fila CTB, painel do ator ativo, log. viewport_px_w/h = PIXELS REAIS da janela
    // (o backend escala 960x540 por inteiro). LE a contagem/ordem do motor.
    void render(gus::platform::render2d::IRenderer& renderer, float viewport_px_w,
                float viewport_px_h) const;

private:
    // Provider da FSM = mailbox: devolve pending_action_ e o reseta pra Pass. Usado
    // tanto pro jogador (acao do menu) quanto pro inimigo (acao auto). Definido no ctor.
    [[nodiscard]] gus::domain::combat::CombatAction take_pending_action(
        gus::domain::combat::CombatActor& actor);

    // Resolve UM turno do ator ativo com a acao ja no mailbox (run_active_turn_to_end),
    // spawna floaters + monta a linha de consequencia (D12), depois check_end + avanca
    // pro proximo ator e chama begin_turn nele (deixa pronto). NAO encadeia: 1 turno.
    void resolve_one_turn();

    // Prepara o ator ativo corrente pro seu turno (begin_turn: recarrega AP/Mana, tick de
    // status). Idempotencia controlada por turn_started_ (nao re-begin no mesmo ator).
    void start_active_turn();

    // Avanca o RITMO (incremento 6): chamado por update(dt) quando o diretor libera um
    // passo. Resolve 1 turno de inimigo (com delay depois) OU entra em espera-do-jogador.
    void advance_pacing();

    // Acao automatica de um inimigo: ataque basico no 1o player vivo (POCO-ish; o alvo
    // vem de first_alive_player). Pass se nao houver player vivo.
    [[nodiscard]] gus::domain::combat::CombatAction enemy_auto_action(
        const gus::domain::combat::CombatActor& enemy) const;

    // Primeiro inimigo vivo (alvo default das acoes ofensivas do jogador). nullptr se
    // nenhum. Primeiro player vivo (alvo das acoes de inimigo). nullptr se nenhum.
    [[nodiscard]] gus::domain::combat::CombatActor* first_alive_enemy() const;
    [[nodiscard]] gus::domain::combat::CombatActor* first_alive_player() const;

    // ---- Modo-mira (helpers privados, §3.5) ----

    // Entra em modo-mira para o verbo (Atacar/Scan): (re)constroi a lista de inimigos
    // vivos-miraveis e pre-seleciona o alvo por D3. Retorna false (e NAO entra na mira) se
    // nao ha inimigo vivo (vitoria iminente). Chamado pelo menu_confirm.
    bool enter_aim_mode(BattleVerb verb);

    // Reconstroi aim_candidates_ = inimigos VIVOS em queue().order() (frente->tras). O
    // i-esimo candidato casa a ordem de "quem age antes" (o [0] == first_alive_enemy).
    void rebuild_aim_candidates();

    // Indice do alvo pre-selecionado (D3): (a) se ha inimigo escaneado FRACO a familia da
    // acao (mult 1.5), o 1o desses na fila; senao (b) o [0] (front da fila = age antes).
    [[nodiscard]] int preselect_aim_index(BattleVerb verb) const;

    // "Familia da acao" pro tier de fraqueza D3. O ataque BASICO nao usa a roda no motor
    // (dano = atk-def) e o Scan e utilitario: FALLBACK documentado -> Atacar usa a familia
    // do ATOR ATIVO (assinatura) pra a sugestao ser significativa; Scan nao tem familia
    // (nullopt => branch (b) sempre). COMPILAR (carta com familia) fica fora deste incr.
    [[nodiscard]] std::optional<gus::domain::combat::CardFamily> action_family(
        BattleVerb verb) const;

    // Rotulo localizado de um verbo (tr() via translator_). Sem translator => devolve
    // string vazia (o render trata: a caixa colorida fica sem nome, mas nao crasha).
    [[nodiscard]] std::string tr_verb_label(BattleVerb verb) const;

    // Drena os logs NOVOS do motor (desde log_cursor_) e spawna um numero flutuante
    // sobre o alvo pra cada dano/cura. Chamado apos cada resolucao de turno.
    void spawn_floaters_from_new_logs();

    // Drena os logs NOVOS do motor (desde narration_cursor_) e monta as linhas de
    // NARRACAO (D12): acao + dano + consequencia de status (resolvida via tr()), na cor
    // da categoria. Empilha em narration_. Uma linha por evento, no ritmo do pacing.
    void narrate_new_logs();

    // Retangulo do slot do ator na arena (party/inimigo), pelo id. nullopt se nao esta
    // vivo/visivel. Usado pra posicionar o numero flutuante e o icone de intent.
    [[nodiscard]] std::optional<gus::core::spatial::Rect> arena_rect_for_actor(
        const std::string& actor_id) const;

    // Atores de demo (DONOS). Declarados ANTES da FSM: vivem mais que ela.
    std::vector<std::unique_ptr<gus::domain::combat::CombatActor>> actors_;
    // Brains dos inimigos (DONOS), antes da FSM. ScriptedBrain por inimigo (telegraph).
    std::vector<std::unique_ptr<gus::domain::combat::ScriptedBrain>> brains_;
    // Registro id-do-inimigo -> brain (NAO-dono), passado a FSM pro Gambito/intent.
    std::unordered_map<std::string, gus::domain::combat::IEnemyBrain*> brain_registry_;
    std::unique_ptr<gus::domain::combat::CombatStateMachine> machine_;
    BattlePortraitSet portraits_{};
    BattleStatusIconSet status_icons_{};
    BattleIntentIconSet intent_icons_{};

    // HUD externo (RmlUi): suprime o cockpit/log desenhados a mao (ADR-009). Default false.
    bool hud_external_ = false;

    // Menu de verbos do ator ativo (incremento 3).
    BattleMenu menu_{};
    // Mailbox de acao do provider (1 por vez). Default Pass = "nada a fazer".
    gus::domain::combat::CombatAction pending_action_{
        gus::domain::combat::CombatAction::pass()};
    // Linhas de log geradas pela UI (nao pelo motor): COMPILAR/GAMBITO sinalizados antes
    // de existir mecanica (incr 4/5). Mescladas com o log do motor em log_lines().
    std::vector<LogLine> ui_log_;
    // Translator de UI (NAO-DONO). nullptr = sem traducao (fallback no render).
    const gus::app::i18n::Translator* translator_ = nullptr;

    // Numeros flutuantes ATIVOS (incremento 5). update(dt) envelhece e poda; render
    // desenha sobre o alvo. Spawnados de logs NOVOS do motor (spawn_floaters_from_new_logs).
    std::vector<Floater> floaters_;
    // Cursor do log do motor ja consumido pra floaters (so processa entradas NOVAS).
    std::size_t log_cursor_ = 0;

    // Diretor de RITMO (incremento 6, D8): dita quando o proximo turno pode animar.
    PacingDirector pacing_{};
    // true se o begin_turn ja foi chamado pro ator ativo corrente (evita re-begin no
    // mesmo ator entre frames). Resetado ao avancar pro proximo ator.
    bool turn_started_ = false;
    // true se o INIMIGO ativo ja fez sua acao neste turno (1 ataque/turno, D11). Resetado
    // a cada novo turno (start_active_turn) pra o proximo inimigo agir uma vez.
    bool enemy_acted_this_turn_ = false;
    // true se o turno de inimigo corrente JA passou pelo BEAT 1 (anuncio, incremento 6).
    // false => o proximo advance_pacing ANUNCIA (sem resolver); true => RESOLVE. Limpa ao
    // resolver, ao cair num turno de jogador, e ao avancar de ator.
    bool enemy_announced_ = false;

    // NARRACAO do combate (D12): linhas de acao + dano + consequencia de status, ja
    // resolvidas (tr() no nome do status). Construidas por narrate_new_logs a partir dos
    // logs novos do motor. log_lines() mostra as ultimas N daqui (+ ui_log_).
    std::vector<LogLine> narration_;
    // Cursor do log do motor ja consumido pra narracao (so processa entradas NOVAS).
    std::size_t narration_cursor_ = 0;

    // ---- Modo-mira / target selection (§3.5, D3) ----
    // true enquanto o jogador escolhe o alvo (entre [Atacar]/[Scan] e o confirm/cancel).
    bool aiming_ = false;
    // Verbo que abriu a mira (so Atacar/Scan entram na mira neste incremento).
    BattleVerb aim_verb_ = BattleVerb::Atacar;
    // Cursor: indice do inimigo mirado em aim_candidates_.
    int aim_index_ = 0;
    // Inimigos VIVOS-miraveis (NAO-donos; vivem em actors_), em ordem de fila (frente->
    // tras). Reconstruida ao entrar na mira; const porque a mira so LE o alvo.
    std::vector<const gus::domain::combat::CombatActor*> aim_candidates_;
};

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_SCENE_HPP
