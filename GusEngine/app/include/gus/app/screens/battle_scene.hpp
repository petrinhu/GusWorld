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
#include "gus/app/screens/battle_anim.hpp"        // BattleAnimDirector (W2, battle-anim.md)
#include "gus/app/screens/battle_floaters.hpp"    // Floater (numeros flutuantes)
#include "gus/app/screens/battle_hud_model.hpp"   // kStatusIdCount (HUD-STATUS-ICONS-STALE)
#include "gus/app/screens/battle_log_model.hpp"   // LogLine
#include "gus/app/screens/battle_menu.hpp"        // BattleMenu / BattleVerb
#include "gus/app/screens/battle_pacing.hpp"      // PacingDirector (ritmo, D8)
#include "gus/app/screens/battle_sprite_anim.hpp" // ActorSpriteSet/BattleSpriteAnimator (W3)
#include "gus/domain/combat/combat_enums.hpp"     // StatusId
#include "gus/domain/combat/combat_records.hpp"   // CombatAction / IntentPreview
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/domain/combat/scripted_brain.hpp"   // ScriptedBrain (telegraph)
#include "gus/platform/audio/audio_engine.hpp"    // AudioEngine/SoundId (M6 F3, ADR-011)
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
    // [status_icon_index(id)] = handle. Vazio/ausente => placeholder. Tamanho AMARRADO
    // a kStatusIdCount (battle_hud_model.hpp), NAO mais um numero magico: HUD-STATUS-
    // ICONS-STALE (auditoria-domino) achou a causa-raiz de os 7 StatusId novos do
    // executor techMagic (SobrecargaTermica..Eco) nunca carregarem icone - o array
    // ficava hardcoded em 13 mesmo com status_icon_file ja cobrindo os 20. Se o enum
    // StatusId crescer de novo, este array cresce junto (static_assert de
    // battle_hud_model.hpp + -Werror=switch de status_icon_file seguram a sincronia).
    std::array<gus::platform::render2d::TextureId, kStatusIdCount> by_index{};

    // Handle do icone de um StatusId. kInvalidTexture se nao carregado.
    [[nodiscard]] gus::platform::render2d::TextureId find(
        gus::domain::combat::StatusId id) const noexcept;
};

// Trava a sincronia array<->enum em tempo de compilacao (mesmo raciocinio do
// static_assert de kStatusIdCount em battle_hud_model.hpp; previne a regressao do
// numero magico que causou este bug-raiz).
static_assert(std::tuple_size_v<decltype(BattleStatusIconSet::by_index)> ==
                  static_cast<std::size_t>(kStatusIdCount),
              "BattleStatusIconSet::by_index deve ter kStatusIdCount entradas");

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

    // AUDIO (M6 F3, ADR-011): plugga o AudioEngine DONO (vive na CASCA/main - battle_
    // preview.cpp - mais longeva que a cena; re-entradas futuras recriam a BattleScene
    // sem recriar/decodificar o device nem o SFX de novo) + o SoundId do hit ja
    // pre-carregado (load_sfx UMA vez no owner). Ponteiro NAO-DONO (mesmo padrao de
    // set_translator): engine==nullptr (default) = sem audio, a cena roda MUDA e
    // silenciosa (nunca depende de audio pra rodar - mesma degradacao graciosa do
    // AudioEngine em si). O disparo acontece SO no evento de CONTATO do golpe (o MESMO
    // instante onde o hit-react visual + o floater ja nascem hoje): melee via
    // spawn_floaters_from_new_logs (canais Common/Crit) e projetil via o impacto
    // (anim_.take_impacts(), consumido em update()) - os dois call-sites tocam o MESMO
    // play_sfx(hit_sfx_id_). Beat 1/anuncio/windup/aproximacao seguem MUDOS de proposito
    // (o golpe ainda nao conectou).
    void set_audio(gus::platform::audio::AudioEngine* engine,
                   gus::platform::audio::SoundId hit_sfx_id) noexcept {
        audio_engine_ = engine;
        hit_sfx_id_ = hit_sfx_id;
    }

    // SPRITE ANIMADO na arena (W3, battle-anim.md par.1.1/3.2): instala o conjunto de
    // clips de UM ator (handles ja resolvidos pela casca, mesmo padrao de
    // set_portraits). Com sprite set, o render desenha o FRAME CORRENTE da animacao da
    // fase (idle/run/golpe/hurt) no slot (+offset do director); SEM set (ou clips
    // vazios/headless), o ator segue no retrato placeholder de hoje. Nesta onda so o
    // GUS recebe set (demais atores aguardam as anims deles).
    void set_actor_sprites(const std::string& id, ActorSpriteSet set) {
        sprites_[id] = std::move(set);
    }

    // Clip + frame CORRENTES do sprite do ator (leitura pro render/testes/self-test).
    // nullopt se o ator nao tem sprite set instalado (ou o clip da fase E o fallback
    // Idle estao ambos vazios) - o render degrada pro retrato.
    [[nodiscard]] std::optional<std::pair<BattleClipId, int>> actor_sprite_frame(
        const std::string& id) const;

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

    // Janela da fila CTB: os ate kCtbVisibleCells atores a mostrar na faixa do topo, na
    // ordem esquerda->direita das celulas. Fonte UNICA consumida pelo render E pelos testes
    // (o render nunca reimplementa esse mapeamento). A janela COMECA no ator ATIVO (turno
    // agora, == window[0]) e segue a ordem de jogo com WRAP na fila; window[1] e o proximo a
    // jogar, e assim por diante. Nunca repete um ator quando a fila e curta (o teto min(n,5)
    // da no maximo uma volta parcial). Vazia se a fila esta vazia. Ponteiros NAO-DONOS (os
    // atores vivem em actors_/na FSM). Ver docs/design/mecanicas/battle-screen.md par.5 (D4).
    [[nodiscard]] std::vector<const gus::domain::combat::CombatActor*> ctb_window() const;

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

    // ---- Flavor da derrota (M7-COSTURA Inc 3, "reboot de sistema" nao-canonico) ----
    //
    // O Gus NUNCA morre (echo Batman; coerente com Pillar 1 "magia = software"): ao
    // Defeat (Gus-centric, ver combat_state_machine.cpp::check_end, OU wipe-total), a
    // tela NAO some na hora. Por kDefeatFlavorSeconds (battle_scene.cpp), o render
    // sobrepoe um veu quase-opaco com 3 linhas: (1) o "kernel panic" (pool de literais
    // TECNICOS nao-traduziveis, docs/design/mecanicas/combat-flavor.md §3b "Derrota" -
    // mesma convencao §4 do codigo de erro autentico), (2) a falinha blase de um
    // companion vivo (tr() COMBAT_DEFEAT_BARK, ou _GENERIC se ninguem sobrou pra falar),
    // (3) a nota-xadrez (tr() COMBAT_DEFEAT_CHESS_NOTE) explicando o Gus-centric ("o Rei
    // caiu, a partida acaba"). SO TIMER, sem input (anti-OE: nao e cutscene) - o host
    // (battle_preview) mantem o loop rodando (render+update) enquanto isto e true, so
    // ENTAO trata como combat_over() de fato (mesmo padrao "ultimo frame" do BUG-2).
    // false pra Victory/Fled (o gate abaixo so liga em CombatOutcome::Defeat) e false
    // apos o timer esgotar. CONEXAO CANONICA (nao implementada aqui): esta mesma falinha
    // e o SETUP do Dragon Victory no climax (ver project_dragon_victory_canon) - por isso
    // fica LEVE/nao-dramatica de proposito, protegendo o payoff do climax.
    [[nodiscard]] bool defeat_flavor_active() const noexcept;

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

    // ---- Animacao de combate (W2, battle-anim.md par.2/3) ----
    //
    // A animacao e APRESENTACAO PURA sobre o motor: OFFSETS por ator (battle_anim)
    // somados a posicao-base do slot no render. Encaixe nos beats (par.3.1):
    //   - INIMIGO: a aproximacao do melee toca DURANTE o Beat 1 ANUNCIO (nada resolve);
    //     o CONTATO coincide com o Beat 2 (resolve_one_turn: dano + floater + hit-react);
    //     a volta (Return) cabe no delay do Beat 2.
    //   - JOGADOR: confirmar [Atacar] inicia o windup NA HORA (regra de ouro < 100ms) e
    //     a RESOLUCAO e DEFERIDA ate o contato (fim da aproximacao) - a aproximacao E o
    //     proprio anuncio (par.3.1). Scan/Defender/Flee seguem instantaneos (nao sao
    //     golpe melee). E a extensao do padrao dos 2 beats: o motor resolve instantaneo,
    //     a ANIMACAO espalha a apresentacao no tempo.

    // true entre o confirm de [Atacar] e o CONTATO (resolucao deferida em voo). O host/
    // testes bombeiam update(dt) ate cair. Menu e inerte enquanto true.
    [[nodiscard]] bool player_action_in_flight() const noexcept {
        return player_strike_pending_;
    }

    // Diretor de animacao (leitura pro render/testes: offsets, projeteis, estados).
    [[nodiscard]] const BattleAnimDirector& anim() const noexcept { return anim_; }

    // DIAGNOSTICO (env-gated no host; mecanismo DORMANTE da magia, par.2.1): dispara um
    // cast COSMETICO do 1o membro vivo da party no 1o inimigo vivo - conjura no lugar,
    // bolinha placeholder viaja e o impacto dispara o hit-react VISUAL. NAO toca o motor
    // (zero dano/log): so valida o esqueleto cast -> viagem -> impacto que as cartas
    // (COMPILAR) vao reusar quando sairem do placeholder de UI. No-op sem caster/alvo.
    void debug_cast_demo();

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

    // MOUSE (Incremento A2): poe a mira DIRETO no i-esimo inimigo miravel (0..aim_count()-1),
    // sem WRAP/delta. No-op fora do modo-mira ou com index fora de faixa. Espelha o aim_move
    // (mesma lista aim_candidates_) pro clique/hover pousar num alvo especifico.
    void aim_select(int index) noexcept;

    // MOUSE (Incremento A2): indice do inimigo miravel (0..aim_count()-1) cujo SLOT na arena
    // contem o ponto em coordenadas de MUNDO/logicas (px logico 960x540, o mesmo espaco do
    // arena_layout/arena_rect_for_actor); -1 se o ponto nao cai em nenhum inimigo miravel.
    // FUNCAO PURA de leitura (nao muta a cena): decide "que alvo esse (x,y) corresponde",
    // separada da acao (aim_select + aim_confirm ficam no host SDL). Fora do modo-mira a
    // lista esta vazia -> sempre -1 (clicar inimigo so vale mirando). Testavel headless.
    [[nodiscard]] int aim_index_at_arena(float world_x, float world_y) const;

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

    // TECLA-ATALHO (1-9, pedido do lider): mira DIRETO o nth-esimo inimigo miravel (1-based,
    // ordem de aim_candidates_) e CONFIRMA na hora - espelha EXATAMENTE actor_picker_hotkey
    // (mira e picker nunca sao simultaneos). No-op se nth nao tem candidato (fora de faixa)
    // ou fora do modo-mira. Fonte UNICA do host (teclas) e dos testes; encapsula o
    // "aim_select + aim_confirm imediato".
    void aim_hotkey(int nth);

    // ---- Escolha de ator / Janela de Comando da Party (comando-livre 1B, combat.md §4.1) ----
    //
    // APRESENTACAO PURA sobre o motor 1B (§4.1): quando e a vez do BLOCO da party e ha MAIS DE
    // UM membro elegivel (machine().pending_party_actors()), a cena entra num modo de ESCOLHA
    // DE ATOR ANTES do menu de verbos - o jogador comanda QUAL membro age (a SPD apenas SUGERE
    // o pre-selecionado; §4.1 "comando livre"). Com 1 so elegivel NAO ha escolha: a cena
    // auto-inicia o turno (sem friccao) - o picker existe pra ESCOLHER entre varios (decisao
    // documentada no .cpp). ADITIVO: o motor (domain/) ja faz tudo (agrupa por lado, recomputa
    // quem abre, regroup Gambito-safe); aqui so deixamos o jogador escolher dentro de
    // pending_party_actors. So teclado/mouse.
    //
    // 2 ESTAGIOS pos-picker (FIX do bug "trava a selecao", playtest do lider 2026-07):
    //   (1) is_choosing_actor(): a LISTA de elegiveis (badges na arena). Confirmar aqui NAO
    //       chama begin_turn - so entra no estagio (2).
    //   (2) is_actor_preview(): o menu de VERBOS do escolhido, em PREVIEW - mostrado (com
    //       AP/mana corretos, ver commit_previewed_actor) mas o MOTOR segue 100% intocado
    //       (nenhum tick de status, nada comprometido). Nada foi commitado: Esc volta ao
    //       estagio (1) via actor_preview_cancel() (re-abre a lista do zero, mesma fonte
    //       pending_party_actors() - nada mudou no motor, entao a lista e IDENTICA). So a 1a
    //       ACAO de fato resolvida (Defender/Flee em menu_confirm, Atacar/Scan em aim_confirm)
    //       e o PONTO-DE-NAO-RETORNO: commit_previewed_actor() grava a escolha no motor
    //       (select_party_actor) e chama o begin_turn REAL (bring_to_current + refresh de
    //       recursos + o TICK de status, agora sim IRREVERSIVEL). Ate o commit, o jogador pode
    //       trocar de ator livremente (Esc + escolher outro na lista) sem custo algum.

    // true enquanto a LISTA de elegiveis esta aberta (estagio 1; ver acima). false durante o
    // preview do menu de verbos (estagio 2) ou apos o commit.
    [[nodiscard]] bool is_choosing_actor() const noexcept { return choosing_actor_; }

    // true enquanto o menu de verbos de um ator ESCOLHIDO esta em PREVIEW (estagio 2: apos
    // actor_picker_confirm, antes da 1a acao resolvida) - o motor ainda nao comprometeu esse
    // ator (sem begin_turn, sem tick de status). false fora do picker OU ja comprometido (pos-
    // commit_previewed_actor). active_actor()/current_actor_is_player() ja refletem o ator em
    // preview (ver active_actor() no .cpp) - o menu de verbos opera normalmente sobre ele.
    [[nodiscard]] bool is_actor_preview() const noexcept { return actor_preview_; }

    // Membros da party ELEGIVEIS nesta escolha (== machine().pending_party_actors() no momento
    // de entrar), front = pre-selecionado (maior SPD). Vazio fora do modo. Ponteiros NAO-donos
    // (vivem em actors_/na FSM). Fonte UNICA consumida pelo render E pelos testes.
    [[nodiscard]] std::vector<const gus::domain::combat::CombatActor*> actor_choices() const;

    // Quantidade de elegiveis na escolha atual. 0 fora do modo.
    [[nodiscard]] int actor_pick_count() const noexcept {
        return static_cast<int>(actor_choices_.size());
    }

    // Ator sob o cursor da escolha (pre-selecionado ao entrar = maior SPD). nullptr fora do
    // modo. Pro render do destaque + testes.
    [[nodiscard]] const gus::domain::combat::CombatActor* actor_pick_target() const noexcept;

    // Navega o cursor entre os elegiveis (delta -1/+1, com WRAP). No-op fora do modo. Espelha
    // aim_move (setas + Enter, a mesma linguagem do menu/mira).
    void actor_picker_move(int delta) noexcept;

    // MOUSE/atalho: poe o cursor DIRETO no i-esimo elegivel (0..actor_pick_count()-1), sem
    // WRAP. No-op fora do modo ou index fora de faixa. Espelha aim_select.
    void actor_picker_select(int index) noexcept;

    // MOUSE: indice do elegivel (0..actor_pick_count()-1) cujo SLOT na arena contem o ponto em
    // coords de MUNDO/logicas (px logico 960x540, o mesmo espaco de arena_rect_for_actor); -1
    // se o ponto nao cai em nenhum elegivel. FUNCAO PURA (nao muta). Espelha aim_index_at_arena,
    // mas sobre os slots da PARTY (arena_rect_for_actor e generico por id). Fora do modo a lista
    // esta vazia -> sempre -1. Testavel headless; reusa o pipeline de hit-test do Incremento A2.
    [[nodiscard]] int actor_pick_index_at_arena(float world_x, float world_y) const;

    // Confirma o ator sob o cursor: sai do estagio (1) LISTA e entra no estagio (2) PREVIEW -
    // mostra o menu de verbos DELE (com AP/mana corretos), mas NAO grava a escolha no motor
    // nem chama begin_turn ainda (ver bloco de comentario acima de is_choosing_actor/
    // is_actor_preview). O commit real fica pra commit_previewed_actor(), chamado so quando a
    // 1a acao de fato resolve. No-op fora do estagio (1). Clique de mouse e teclas 1/2/3
    // confirmam via este caminho (mesma filosofia "aciona na hora" do A2).
    void actor_picker_confirm();

    // TECLA-ATALHO (1/2/3, pedido do lider): escolhe DIRETO o nth-esimo elegivel (1-based) e
    // CONFIRMA na hora (entra no PREVIEW dele). No-op se nth nao tem elegivel correspondente
    // (fora de faixa) ou fora do modo. Fonte UNICA do host (teclas) e dos testes; encapsula o
    // "select + confirm imediato".
    void actor_picker_hotkey(int nth);

    // Esc no PREVIEW (estagio 2, is_actor_preview()==true): desiste do ator escolhido SEM
    // custo (nada foi commitado no motor) e VOLTA ao estagio (1) LISTA - re-abre a escolha do
    // zero (mesma fonte pending_party_actors(); a lista e IDENTICA porque nada mudou no
    // motor). No-op fora do preview (FIX do bug "Esc fecha a tela", playtest do lider 2026-07:
    // antes o Esc so via 2 estados, mira e "generico" -> sai do viewer; agora desempilha 1
    // nivel por vez, ver battle_key_down em battle_preview.cpp).
    void actor_preview_cancel() noexcept;

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

    // Dispara o SFX de hit (M6 F3, ADR-011) no evento de CONTATO. No-op seguro se
    // set_audio nunca foi chamado (audio_engine_==nullptr) - a cena roda muda sem
    // depender de audio. Chamado dos MESMOS 2 pontos onde start_hit_react ja dispara
    // (spawn_floaters_from_new_logs para melee/UseCard; o consumo de anim_.take_impacts
    // em update() para projetil), nunca antes do contato.
    void play_hit_sfx();

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

    // ---- Escolha de ator / Janela de Comando da Party (helpers privados, §4.1) ----

    // true se e a vez de um membro da party (queue_.current player-side vivo), o combate esta
    // rolando (nao intro, nao fim) e ha MAIS DE UM elegivel pendente - entao o jogador ESCOLHE
    // quem age. >1: com 1 so elegivel nao ha escolha (auto-inicia, sem friccao). Gate do modo.
    [[nodiscard]] bool should_offer_actor_picker() const;

    // Entra no modo de escolha: snapshot dos elegiveis (pending_party_actors) + cursor no
    // pre-selecionado (front = maior SPD). NAO chama begin_turn (deferido ate o commit real).
    void enter_actor_picker();

    // O "begin" real do turno ativo (begin_turn + floaters/narracao + menu.refresh), extraido
    // de start_active_turn pra commit_previewed_actor()/o caminho sem picker INICIAREM sem
    // re-checar o gate do picker (nao re-entra).
    void begin_active_turn_now();

    // PONTO-DE-NAO-RETORNO do comando-livre 1B (bug1 do playtest, regra fechada Caetano+lider):
    // se o ator ativo ainda esta em PREVIEW (is_actor_preview()==true), grava a escolha no
    // motor (machine_->select_party_actor) e chama begin_active_turn_now() - o begin_turn REAL,
    // com o tick de status IRREVERSIVEL incluso. No-op seguro se NAO ha preview pendente (sem
    // picker, ou <=1 elegivel: o caminho de sempre ja chamou begin_active_turn_now direto em
    // start_active_turn). Chamado no INICIO de toda resolucao real de acao - menu_confirm()
    // (Defender/Flee, que resolvem na hora) e aim_confirm() (Atacar/Scan, apos o alvo
    // escolhido) - NUNCA ao so abrir o menu de verbos ou entrar na mira (essas etapas
    // continuam DESFAZIVEIS: Esc/troca de ator livre ate aqui).
    void commit_previewed_actor();

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

    // ---- Animacao de combate (helpers privados, W2) ----

    // Ator (NAO-dono) pelo id na fila do motor. nullptr se ausente.
    [[nodiscard]] const gus::domain::combat::CombatActor* actor_by_id(
        const std::string& id) const;

    // Resolve o CLIP corrente do sprite de um ator: o que a fase do director pede
    // (clip_for_kind + swing na cauda), com fallback pro Idle se o clip da fase nao
    // tem frames. nullptr se o ator nao tem set/frames (degrada pro retrato).
    // Fonte UNICA usada pelo render E por actor_sprite_frame (nao divergem).
    [[nodiscard]] const SpriteClip* resolve_sprite_clip(
        const std::string& id, BattleClipId* out_clip, float* out_elapsed) const;

    // Inicia a aproximacao melee do atacante ATE perto do alvo (para adjacente ao slot,
    // kMeleeContactGapPx de folga; party avanca pra DIREITA, inimigo pra ESQUERDA - sem
    // flip, Pillar 3: o deslocamento e so translacao do sprite). false se algum slot nao
    // esta visivel (degrada: o caller resolve sem animacao).
    bool start_melee_toward(const std::string& attacker_id,
                            const std::string& target_id, float seconds);

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
    // AUDIO (M6 F3, ADR-011): AudioEngine NAO-DONO (vive na casca/main) + o SoundId do
    // hit ja carregado. nullptr = sem audio (fallback silencioso, ver set_audio).
    gus::platform::audio::AudioEngine* audio_engine_ = nullptr;
    gus::platform::audio::SoundId hit_sfx_id_ = gus::platform::audio::kInvalidSound;

    // Numeros flutuantes ATIVOS (incremento 5). update(dt) envelhece e poda; render
    // desenha sobre o alvo. Spawnados de logs NOVOS do motor (spawn_floaters_from_new_logs).
    std::vector<Floater> floaters_;
    // Cursor do log do motor ja consumido pra floaters (so processa entradas NOVAS).
    std::size_t log_cursor_ = 0;

    // Diretor de RITMO (incremento 6, D8): dita quando o proximo turno pode animar.
    PacingDirector pacing_{};
    // Diretor de ANIMACAO (W2): offsets por ator + projeteis; o render soma os offsets
    // na posicao-base dos slots. update(dt) o avanca junto do resto.
    BattleAnimDirector anim_{};
    // SPRITES (W3): conjunto de clips por ator (so quem tem anima; os demais seguem no
    // retrato) + relogio de clip por ator, dirigido no update(dt) pela fase do director.
    std::unordered_map<std::string, ActorSpriteSet> sprites_;
    BattleSpriteAnimator sprite_anim_{};
    // true entre o confirm de [Atacar] do jogador e o CONTATO (resolucao deferida). O
    // update(dt) resolve quando a aproximacao chega (melee_arrived).
    bool player_strike_pending_ = false;
    // Id do atacante do golpe em voo (pro update casar o contato + o Return).
    std::string player_strike_attacker_;
    // Demo de cast (diagnostico, dormante): aguardando o fim do windup pra spawnar o
    // projetil placeholder (caster -> alvo). Cosmetico: zero motor.
    bool demo_cast_active_ = false;
    std::string demo_cast_caster_;
    std::string demo_cast_target_;
    // true se o begin_turn ja foi chamado pro ator ativo corrente (evita re-begin no
    // mesmo ator entre frames). Resetado ao avancar pro proximo ator.
    bool turn_started_ = false;
    // true se o INIMIGO ativo ja fez sua acao neste turno (1 ataque/turno, D11). Resetado
    // a cada novo turno (start_active_turn) pra o proximo inimigo agir uma vez.
    bool enemy_acted_this_turn_ = false;

    // FLAVOR DE DERROTA (M7-COSTURA Inc 3): segundos decorridos desde que outcome() virou
    // Defeat. Envelhecido em update(dt) SO enquanto Defeat (Victory/Fled nunca tocam
    // isto); trava em kDefeatFlavorSeconds (nao ultrapassa, sem no-op extra no caller).
    // Nao serializado: existe so enquanto esta BattleScene vive (a Maestro reconstroi a
    // cena do zero a cada batalha nova).
    float defeat_flavor_elapsed_ = 0.0f;
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

    // ---- Escolha de ator / Janela de Comando da Party (§4.1) ----
    // true enquanto o estagio (1) LISTA esta aberto (badges na arena). start_active_turn e
    // no-op enquanto isto e true (o begin so vem no commit real, ver actor_preview_/
    // commit_previewed_actor).
    bool choosing_actor_ = false;
    // Cursor: indice do elegivel escolhido em actor_choices_ (0 = pre-selecionado ao entrar).
    int actor_pick_index_ = 0;
    // Elegiveis da LISTA corrente (NAO-donos; vivem em actors_/na FSM), front = maior SPD
    // (== machine().pending_party_actors()). NON-const de proposito: os MESMOS ponteiros vao
    // pra machine_->select_party_actor (que pede CombatActor*). Preenchido ao entrar no modo
    // (enter_actor_picker), limpo ao confirmar (actor_picker_confirm entra no PREVIEW).
    std::vector<gus::domain::combat::CombatActor*> actor_choices_;
    // true enquanto o estagio (2) PREVIEW esta aberto (menu de verbos do escolhido, motor
    // intocado). Ver bloco de comentario de is_choosing_actor()/is_actor_preview() acima.
    bool actor_preview_ = false;
    // O ator escolhido no picker, em PREVIEW (NAO-dono; vive em actors_/na FSM). Fonte que
    // active_actor() devolve enquanto actor_preview_==true (o motor ainda nao trouxe este
    // ator pro cursor - ver active_actor() no .cpp). nullptr fora do preview.
    gus::domain::combat::CombatActor* preview_actor_ = nullptr;
};

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_SCENE_HPP
