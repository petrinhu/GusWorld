// gus/app/src/screens/battle_scene.cpp
//
// Implementacao da BattleScene (ver header). INCREMENTO 1: esqueleto navegavel.
// INCREMENTO 2 (este): o painel do ator ativo e a arena passam a mostrar DADOS REAIS
// lidos do motor (HP/AP/Mana/status), e a fila CTB marca o ator ATIVO. Tudo LIDO da
// CombatStateMachine; o calculo de barra/pip/icone fica no modelo POCO battle_hud_model.
// AINDA SEM: menu de verbos, conduzir turno, cartas, numeros flutuantes, intent,
// transicao. So leitura-e-exibicao.
//
// SEM TEXTO: a engine ainda nao tem fonte; os numeros sao mostrados FONT-FREE (HP=barra,
// AP/Mana=pips, status=icone). Decisao reportada ao coordenador (reversivel; numeros
// podem se sobrepor depois sem mudar o layout).
//
// RENDER 1:1: a camera do IRenderer e dirigida pro retangulo logico 640x360 (D1), entao
// cada Rect do battle_layout (em px logico) vira um Rect de mundo identico. O backend
// (Render2dSdl) escala 640x360 -> janela por inteiro (pixel-perfect), letterbox a cargo
// do backend/janela.

#include "gus/app/screens/battle_scene.hpp"

#include <cstdio>   // std::snprintf (numeros do painel)
#include <string>
#include <utility>  // std::move
#include <vector>

#include "gus/app/screens/battle_hud_model.hpp"
#include "gus/app/screens/battle_layout.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_constants.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"

namespace gus::app::screens {

namespace {

using gus::core::spatial::Rect;
using gus::domain::combat::CardFamily;
using gus::domain::combat::CombatActor;
using gus::domain::combat::StatusId;
using gus::platform::render2d::DrawColor;
using gus::platform::render2d::IRenderer;
using gus::platform::render2d::kInvalidTexture;
using gus::platform::render2d::TextureId;
using gus::platform::render2d::UvRect;

// Tamanho de texto (px logico) das zonas. Pixel Operator e crisp a multiplos de 8.
constexpr float kVerbTextPx = 8.0f;     // rotulo do verbo na caixa do menu
constexpr float kLogTextPx = 8.0f;      // linha do log
constexpr float kPanelTextPx = 8.0f;    // numeros do painel (HP/AP/Mana)
// Cor do texto sobre a caixa do verbo (claro p/ contraste) e do numero do painel.
constexpr DrawColor kVerbTextColor{0.06f, 0.06f, 0.08f, 1.0f};   // escuro sobre caixa clara
constexpr DrawColor kPanelTextColor{0.92f, 0.92f, 0.96f, 1.0f};  // claro sobre painel escuro

// --- Paleta placeholder do esqueleto (ponto unico; arte real entra depois) ---
// Cores em [0,1] (DrawColor). Graybox legivel: distingue zonas e lados sem arte.
constexpr DrawColor kBgColor{0.07f, 0.07f, 0.10f, 1.0f};           // fundo da arena
constexpr DrawColor kCtbBandColor{0.14f, 0.14f, 0.20f, 1.0f};      // faixa CTB
constexpr DrawColor kCtbCellColor{0.30f, 0.30f, 0.42f, 1.0f};      // celula CTB ocupada
constexpr DrawColor kCtbNextColor{0.95f, 0.82f, 0.25f, 1.0f};      // marca "proximo"
constexpr DrawColor kPartyColor{0.30f, 0.55f, 0.85f, 1.0f};        // party (esquerda)
constexpr DrawColor kEnemyColor{0.80f, 0.32f, 0.32f, 1.0f};        // inimigos (direita)
constexpr DrawColor kActiveHiColor{0.98f, 0.92f, 0.45f, 1.0f};     // highlight ator ativo
constexpr DrawColor kPanelColor{0.12f, 0.12f, 0.16f, 1.0f};        // painel do ator
constexpr DrawColor kLogColor{0.09f, 0.09f, 0.12f, 1.0f};          // caixa de log
constexpr DrawColor kHudBorderColor{0.45f, 0.45f, 0.55f, 1.0f};    // contorno HUD
constexpr DrawColor kWhite{1.0f, 1.0f, 1.0f, 1.0f};                // tint neutro
// --- dados reais (incremento 2): barras de HP, pips de AP/Mana, marca de ativo ---
constexpr DrawColor kBarBackColor{0.05f, 0.05f, 0.07f, 1.0f};      // fundo de barra
constexpr DrawColor kHpFillColor{0.35f, 0.80f, 0.40f, 1.0f};       // HP cheio (verde)
constexpr DrawColor kApLitColor{0.95f, 0.85f, 0.30f, 1.0f};        // pip de AP aceso
constexpr DrawColor kApOffColor{0.25f, 0.23f, 0.16f, 1.0f};        // pip de AP apagado
constexpr DrawColor kManaLitColor{0.40f, 0.65f, 0.95f, 1.0f};      // pip de Mana aceso
constexpr DrawColor kManaOffColor{0.16f, 0.20f, 0.28f, 1.0f};      // pip de Mana apagado
constexpr DrawColor kStatusBoxColor{0.30f, 0.30f, 0.42f, 1.0f};    // placeholder de status
constexpr DrawColor kActiveCtbColor{0.30f, 0.95f, 0.55f, 1.0f};    // marca de ativo na fila
// --- menu de verbos (incremento 3): 1 cor por verbo (sem set de icones de verbo) ---
constexpr DrawColor kVerbScanColor{0.35f, 0.70f, 0.85f, 1.0f};
constexpr DrawColor kVerbGambitoColor{0.70f, 0.45f, 0.85f, 1.0f};
constexpr DrawColor kVerbAtacarColor{0.85f, 0.40f, 0.35f, 1.0f};
constexpr DrawColor kVerbDefenderColor{0.45f, 0.65f, 0.45f, 1.0f};
constexpr DrawColor kVerbCompilarColor{0.85f, 0.75f, 0.30f, 1.0f};
constexpr DrawColor kVerbFleeColor{0.55f, 0.55f, 0.60f, 1.0f};
constexpr DrawColor kVerbDisabledColor{0.18f, 0.18f, 0.20f, 1.0f};  // verbo sem AP
constexpr DrawColor kVerbSelectColor{0.98f, 0.98f, 0.70f, 1.0f};    // borda do selecionado
// --- log: cor por categoria de linha (D7) ---
constexpr DrawColor kLogSystemColor{0.70f, 0.70f, 0.78f, 1.0f};
constexpr DrawColor kLogDamageColor{0.88f, 0.40f, 0.35f, 1.0f};
constexpr DrawColor kLogHealColor{0.40f, 0.82f, 0.45f, 1.0f};
constexpr DrawColor kLogStatusColor{0.75f, 0.60f, 0.85f, 1.0f};
constexpr DrawColor kLogDefeatColor{0.55f, 0.20f, 0.20f, 1.0f};

// Cor de fundo de um verbo (1 por verbo; placeholder ate haver set de icones de verbo).
DrawColor verb_color(BattleVerb v) noexcept {
    switch (v) {
        case BattleVerb::Scan:     return kVerbScanColor;
        case BattleVerb::Gambito:  return kVerbGambitoColor;
        case BattleVerb::Atacar:   return kVerbAtacarColor;
        case BattleVerb::Defender: return kVerbDefenderColor;
        case BattleVerb::Compilar: return kVerbCompilarColor;
        case BattleVerb::Flee:     return kVerbFleeColor;
    }
    return kVerbAtacarColor;
}

// Cor de uma linha do log pela categoria (D7).
DrawColor log_line_color(LogLineKind k) noexcept {
    switch (k) {
        case LogLineKind::System: return kLogSystemColor;
        case LogLineKind::Damage: return kLogDamageColor;
        case LogLineKind::Heal:   return kLogHealColor;
        case LogLineKind::Status: return kLogStatusColor;
        case LogLineKind::Defeat: return kLogDefeatColor;
    }
    return kLogSystemColor;
}

// Conta atores vivos de um lado na fila do motor.
int count_alive(const gus::domain::combat::CombatStateMachine& m, bool player_side) {
    int n = 0;
    for (const CombatActor* a : m.queue().order()) {
        if (a != nullptr && a->is_player_side() == player_side && a->is_alive()) {
            ++n;
        }
    }
    return n;
}

// Encontro de DEMO (M5 navegavel): party de 3 (Gus + Caua + Jaci) vs 4 inimigos. Stats
// arbitrarios so pra dar SPD distinta (a fila ordena por SPD desc) e exercitar o
// is_universal_compiler do Gus (recuo D3). NAO e balanceamento: e cenario de tela.
std::vector<std::unique_ptr<CombatActor>> make_demo_actors() {
    std::vector<std::unique_ptr<CombatActor>> v;
    auto add = [&v](std::string id, std::string name, int hp, int spd,
                    CardFamily fam, bool player, bool gus) {
        v.push_back(std::make_unique<CombatActor>(
            std::move(id), std::move(name), hp, /*atk=*/10, /*def=*/5, spd, fam,
            player, /*is_boss=*/false, /*knowledge_kills=*/0,
            /*is_universal_compiler=*/gus));
    };
    // Party (player_side = true). Gus = compilador universal.
    add("gus", "Gus", 55, 9, CardFamily::Criptografico, true, true);
    add("caua", "Caua", 60, 12, CardFamily::Eletrico, true, false);
    add("jaci", "Jaci", 50, 7, CardFamily::Bioquimico, true, false);
    // Inimigos (player_side = false). SPD variada pra intercalar a fila.
    add("inimigo1", "Sentinela", 40, 11, CardFamily::Cinetico, false, false);
    add("inimigo2", "Sentinela", 40, 6, CardFamily::Cinetico, false, false);
    add("inimigo3", "Drone", 30, 13, CardFamily::Sonico, false, false);
    add("inimigo4", "Drone", 30, 8, CardFamily::Sonico, false, false);

    // Estado de DEMO pra a tela mostrar dados nao-triviais (incremento 2): alguns
    // atores levam dano (barra de HP parcial) e ganham status (icone no painel/arena).
    // Tudo via a API real do CombatActor; nada hardcoded no render. v[0]=gus ...
    v[0]->take_damage(20);  // Gus ~64% HP (35/55)
    v[2]->take_damage(35);  // Jaci ~30% HP (15/50)
    v[3]->take_damage(10);  // inimigo1 75% HP
    // Status de demo pra exercitar os icones no painel/arena. Caua (v[1]) ganha Haste:
    // como Caua e o 1o JOGADOR a agir (SPD 12, depois do inimigo3), o painel do ator
    // ativo mostra o icone dele na 1a parada. Gus = Regen, inimigo3 = Poison.
    v[0]->add_status(gus::domain::combat::StatusEffect{
        gus::domain::combat::StatusId::Regen, /*magnitude=*/3, /*duration=*/2});
    v[1]->add_status(gus::domain::combat::StatusEffect{
        gus::domain::combat::StatusId::Haste, /*magnitude=*/2, /*duration=*/3});
    v[5]->add_status(gus::domain::combat::StatusEffect{
        gus::domain::combat::StatusId::Poison, /*magnitude=*/4, /*duration=*/3});
    return v;
}

}  // namespace

TextureId BattlePortraitSet::find(const std::string& id) const noexcept {
    for (const auto& [aid, tex] : by_id) {
        if (aid == id) {
            return tex;
        }
    }
    return kInvalidTexture;
}

TextureId BattleStatusIconSet::find(StatusId id) const noexcept {
    const int i = status_icon_index(id);
    if (i < 0 || i >= static_cast<int>(by_index.size())) {
        return kInvalidTexture;
    }
    return by_index[static_cast<std::size_t>(i)];
}

namespace {

// Desenha uma barra (fundo + preenchimento por fracao 0..1, da esquerda). Pura no
// modelo (bar_fill_rect); aqui so emite os dois quads.
void draw_bar(IRenderer& r, const Rect& frame, float fraction,
              const DrawColor& fill) {
    r.draw_filled_rect(frame, kBarBackColor);
    r.draw_filled_rect(bar_fill_rect(frame, fraction), fill);
    r.draw_rect_outline(frame, kHudBorderColor, 1.0f);
}

// Desenha uma linha de pips de recurso (AP/Mana): total pips, lit acesos. Cores
// distintas pra aceso/apagado. Layout puro vem de resource_pips.
void draw_pips(IRenderer& r, float x, float y, int total, int lit, int max_pips,
               const DrawColor& on, const DrawColor& off) {
    for (const ResourcePip& p : resource_pips(x, y, total, lit, max_pips)) {
        r.draw_filled_rect(p.rect, p.lit ? on : off);
    }
}

}  // namespace

// Aliases de dominio usados pelas DEFINICOES de metodo abaixo (fora do anon namespace).
using gus::domain::combat::CombatAction;
using gus::domain::combat::CombatActor;
using gus::domain::combat::CombatOutcome;
using gus::domain::combat::CombatPhase;

BattleScene::BattleScene() {
    actors_ = make_demo_actors();

    std::vector<CombatActor*> ptrs;
    ptrs.reserve(actors_.size());
    for (const auto& a : actors_) {
        ptrs.push_back(a.get());
    }

    // Provider = MAILBOX (incremento 3): devolve a acao pendente (do menu, pro jogador;
    // ou auto, pro inimigo) e a reseta pra Pass. Captura `this` pra acessar o mailbox.
    auto provider = [this](CombatActor& actor,
                           const gus::domain::combat::CombatState&) {
        return take_pending_action(actor);
    };
    machine_ = std::make_unique<gus::domain::combat::CombatStateMachine>(
        std::move(ptrs), std::move(provider));

    // Comeca a batalha: begin_turn do 1o ator + auto-encadeia turnos de inimigo ate cair
    // num turno de JOGADOR vivo (onde o menu opera) ou o combate terminar. O painel
    // mostra o ator ativo (AP/Mana recarregados pelo begin_turn).
    step_until_player_or_end();
    menu_.refresh(active_actor() != nullptr ? active_actor()->ap() : 0);
}

CombatAction BattleScene::take_pending_action(CombatActor& actor) {
    // Inimigo: ignora o mailbox e age sozinho (ataque basico). Jogador: consome o
    // mailbox (acao do menu) e o reseta pra Pass (proxima chamada encerra o turno).
    if (!actor.is_player_side()) {
        return enemy_auto_action(actor);
    }
    CombatAction a = pending_action_;
    pending_action_ = CombatAction::pass();
    return a;
}

CombatActor* BattleScene::first_alive_enemy() const {
    for (CombatActor* a : machine_->queue().order()) {
        if (a != nullptr && !a->is_player_side() && a->is_alive()) {
            return a;
        }
    }
    return nullptr;
}

CombatActor* BattleScene::first_alive_player() const {
    for (CombatActor* a : machine_->queue().order()) {
        if (a != nullptr && a->is_player_side() && a->is_alive()) {
            return a;
        }
    }
    return nullptr;
}

std::string BattleScene::tr_verb_label(BattleVerb verb) const {
    if (translator_ == nullptr) {
        return std::string{};  // sem translator: o render so desenha a caixa colorida
    }
    return translator_->tr(std::string(gus::app::i18n::verb_label_key(verb)));
}

CombatAction BattleScene::enemy_auto_action(const CombatActor& /*enemy*/) const {
    const CombatActor* target = first_alive_player();
    if (target == nullptr) {
        return CombatAction::pass();
    }
    return CombatAction::attack(target->id());
}

bool BattleScene::current_actor_is_player() const noexcept {
    const CombatActor* a = active_actor();
    return a != nullptr && a->is_player_side() && a->is_alive();
}

bool BattleScene::combat_over() const noexcept {
    return machine_->outcome() != gus::domain::combat::CombatOutcome::Ongoing;
}

void BattleScene::resolve_current_turn() {
    // A acao ja esta no mailbox (jogador) ou sera auto (inimigo). run_active_turn_to_end
    // consome AP ate Pass/0; depois CheckEnd e, se nao acabou, avanca pro proximo ator.
    if (machine_->phase() == gus::domain::combat::CombatPhase::ActionSelect) {
        machine_->run_active_turn_to_end();
    }
    if (!machine_->check_end()) {
        machine_->advance_to_next_actor();
    }
}

void BattleScene::step_until_player_or_end() {
    // begin_turn do ator atual; se ele for inimigo (ou perder o turno), resolve e avanca
    // ate cair num turno de JOGADOR vivo ou o combate terminar. Guarda contra loop.
    int guard = 0;
    const int max_steps = machine_->queue().count() * 4 + 8;
    while (!combat_over()) {
        if (++guard > max_steps) {
            break;  // seguranca: nunca pendura o frame
        }
        // Inicia o turno do ator corrente (recarrega AP/Mana, aplica tick de status).
        machine_->begin_turn();
        if (combat_over()) {
            break;
        }
        // Turno de JOGADOR vivo: para aqui e espera o menu (o player escolhe o verbo).
        if (current_actor_is_player()) {
            return;
        }
        // Inimigo (ou stunned): resolve automaticamente e avanca pro proximo.
        resolve_current_turn();
    }
}

void BattleScene::menu_move(int delta) noexcept {
    if (combat_over() || !current_actor_is_player()) {
        return;  // menu so opera no turno de jogador vivo
    }
    menu_.move(delta);
}

void BattleScene::menu_confirm() {
    if (combat_over() || !current_actor_is_player()) {
        return;
    }
    const BattleVerb verb = menu_.selected_verb();
    if (!menu_.is_enabled(verb)) {
        return;  // verbo sem AP: confirm e no-op (o item fica visivel acinzentado)
    }

    CombatActor* self = active_actor() != nullptr
                            ? const_cast<CombatActor*>(active_actor())
                            : nullptr;
    if (self == nullptr) {
        return;
    }

    // COMPILAR (incremento 3): NAO abre o overlay ainda (incr 4). So sinaliza no log que
    // abriria, e NAO consome o turno (o jogador continua podendo escolher outro verbo).
    if (verb == BattleVerb::Compilar) {
        ui_log_.push_back(LogLine{LogLineKind::System,
                                  "COMPILAR: abriria o overlay de cartas (incr 4)"});
        return;
    }

    // Monta a CombatAction real conforme o verbo (alvo default: 1o inimigo vivo pras
    // ofensivas; self pro Defender; sem alvo pro Flee). combat.md par.5 (custos de AP ja
    // vem das factories).
    CombatAction action = CombatAction::pass();
    switch (verb) {
        case BattleVerb::Atacar: {
            const CombatActor* t = first_alive_enemy();
            if (t == nullptr) {
                return;
            }
            action = CombatAction::attack(t->id());
            break;
        }
        case BattleVerb::Scan: {
            const CombatActor* t = first_alive_enemy();
            if (t == nullptr) {
                return;
            }
            action = CombatAction::scan(t->id());
            break;
        }
        case BattleVerb::Gambito: {
            // Gambito-Prever exige IEnemyBrain registrado (a demo nao registra brains):
            // no incremento 3 sinaliza no log e NAO submete (evita excecao do motor).
            ui_log_.push_back(LogLine{
                LogLineKind::System,
                "GAMBITO: requer brain do alvo (telegraph entra no incr 5)"});
            return;
        }
        case BattleVerb::Defender:
            action = CombatAction::defend();
            break;
        case BattleVerb::Flee:
            action = CombatAction::flee();
            break;
        case BattleVerb::Compilar:
            return;  // tratado acima
    }

    // Submete a acao (mailbox) e conduz o turno; depois auto-encadeia turnos de inimigo
    // ate o proximo turno de jogador ou o fim. Atualiza a habilitacao do menu.
    pending_action_ = std::move(action);
    resolve_current_turn();
    if (!combat_over()) {
        step_until_player_or_end();
    }
    menu_.refresh(active_actor() != nullptr ? active_actor()->ap() : 0);
}

std::vector<LogLine> BattleScene::log_lines(int max_lines) const {
    // Junta as linhas NOTAVEIS do motor (D7) com as linhas de UI (COMPILAR/GAMBITO
    // sinalizados aqui), preservando a ordem: motor primeiro, UI depois (as de UI sao as
    // mais recentes ao escolher um verbo que ainda nao age). Corta pro tamanho da caixa.
    std::vector<LogLine> lines = build_log_lines(
        machine_->log(), machine_->status_changes(), /*max_lines=*/0);
    lines.insert(lines.end(), ui_log_.begin(), ui_log_.end());
    if (max_lines > 0 && static_cast<int>(lines.size()) > max_lines) {
        lines.erase(lines.begin(), lines.end() - max_lines);
    }
    return lines;
}

int BattleScene::party_count() const { return count_alive(*machine_, true); }
int BattleScene::enemy_count() const { return count_alive(*machine_, false); }

int BattleScene::queue_len() const noexcept { return machine_->queue().count(); }

const CombatActor* BattleScene::active_actor() const noexcept {
    return machine_->active_actor();
}

int BattleScene::gus_party_index() const {
    int idx = 0;
    for (const CombatActor* a : machine_->queue().order()) {
        if (a == nullptr || !a->is_player_side() || !a->is_alive()) {
            continue;
        }
        if (a->is_universal_compiler()) {
            return idx;  // posicao do Gus DENTRO da coluna de party (so vivos contam)
        }
        ++idx;
    }
    return -1;
}

void BattleScene::render(IRenderer& renderer, float viewport_px_w,
                         float viewport_px_h) const {
    // Camera 1:1 no retangulo logico 640x360 (D1). begin_frame recebe os PIXELS REAIS
    // pro backend escalar por inteiro. Cada Rect de layout (px logico) == Rect de mundo.
    const Rect screen = battle_screen_rect();
    renderer.begin_frame(screen, static_cast<int>(viewport_px_w),
                         static_cast<int>(viewport_px_h));

    // --- fundo da arena (D7: camera estatica, fundo plano no M5) ---
    renderer.draw_filled_rect(screen, kBgColor);

    // --- faixa da fila CTB (topo, D4) ---
    {
        const Rect band{0.0f, 0.0f, static_cast<float>(kBattleLogicalW),
                        static_cast<float>(kCtbStripTop + kCtbPortraitPx +
                                           kCtbStripTop)};
        renderer.draw_filled_rect(band, kCtbBandColor);

        const CtbStrip strip = ctb_strip(queue_len());
        const auto& order = machine_->queue().order();
        const CombatActor* active = active_actor();
        for (int i = 0; i < kCtbVisibleCells; ++i) {
            const CtbCell& cell = strip.cells[static_cast<std::size_t>(i)];
            if (!cell.occupied) {
                continue;
            }
            // O ator desta celula (a fila do motor ja exclui mortos/incapacitados, que
            // o motor remove em advance/prune: a fila CTB reflete so quem ainda joga).
            const CombatActor* who =
                (i < static_cast<int>(order.size())) ? order[i] : nullptr;

            // Retrato 48px se carregavel; senao retangulo da celula.
            TextureId tex = kInvalidTexture;
            if (who != nullptr) {
                tex = portraits_.find(who->id());
            }
            if (tex != kInvalidTexture) {
                renderer.draw_textured_rect(cell.rect, tex, UvRect{}, kWhite);
            } else {
                renderer.draw_filled_rect(cell.rect, kCtbCellColor);
            }
            // Marca do ator ATIVO (incremento 2): borda verde grossa onde o turno e.
            // Tem prioridade visual sobre a marca de "proximo" (que e o 1o da fila).
            if (who != nullptr && who == active) {
                renderer.draw_rect_outline(cell.rect, kActiveCtbColor, 3.0f);
            } else if (cell.is_next) {
                // "proximo" (D4): so destaca se NAO for ja o ativo.
                renderer.draw_rect_outline(cell.rect, kCtbNextColor, 2.0f);
            }
            // Overflow "+N" (D4): contorno discreto na 5a celula (rotulo vem com fonte).
            if (cell.is_overflow) {
                renderer.draw_rect_outline(cell.rect, kHudBorderColor, 1.0f);
            }
        }
    }

    // --- arena: party (esquerda) e inimigos (direita), D2/D3 ---
    {
        const ArenaLayout arena =
            arena_layout(party_count(), enemy_count(), gus_party_index());
        const CombatActor* active = active_actor();

        // Mapeia o slot da party/inimigo de volta pro ator (mesma ordem dos vivos) pra
        // saber QUEM e o ator ativo (highlight D7).
        std::vector<const CombatActor*> alive_party;
        std::vector<const CombatActor*> alive_enemies;
        for (const CombatActor* a : machine_->queue().order()) {
            if (a == nullptr || !a->is_alive()) {
                continue;
            }
            (a->is_player_side() ? alive_party : alive_enemies).push_back(a);
        }

        // Desenha um lado da arena: placeholder do ator + mini-barra de HP REAL sob ele
        // (incremento 2) + highlight se for o ativo (D7). A i-esima slot casa o i-esimo
        // ator vivo daquele lado (mesma ordem do layout/arena_layout).
        auto draw_side = [&](int count, const auto& slots,
                             const std::vector<const CombatActor*>& alive,
                             const DrawColor& body) {
            for (int i = 0; i < count; ++i) {
                const Rect& r = slots[static_cast<std::size_t>(i)].rect;
                renderer.draw_filled_rect(r, body);
                if (i < static_cast<int>(alive.size())) {
                    const CombatActor* a = alive[static_cast<std::size_t>(i)];
                    // Mini-barra de HP REAL sob o sprite (lida do motor).
                    draw_bar(renderer, arena_hp_bar_frame(r),
                             bar_fill(a->hp(), a->max_hp()), kHpFillColor);
                    if (active != nullptr && a == active) {
                        renderer.draw_rect_outline(r, kActiveHiColor, 2.0f);  // D7
                    }
                }
            }
        };
        draw_side(arena.party_count, arena.party, alive_party, kPartyColor);
        draw_side(arena.enemy_count, arena.enemies, alive_enemies, kEnemyColor);
    }

    // --- painel do ator ativo (base): DADOS REAIS do motor (incremento 2) ---
    {
        const Rect p = active_panel_rect();
        renderer.draw_filled_rect(p, kPanelColor);
        renderer.draw_rect_outline(p, kHudBorderColor, 1.0f);

        const CombatActor* a = active_actor();
        if (a != nullptr) {
            const float x = p.x + kPanelPad;
            float y = p.y + kPanelPad;

            // NUMEROS REAIS (incremento 3.5): texto complementa as barras/pips, nao as
            // remove. Buffer pequeno; strings tecnicas de HUD (numero) nao passam por
            // tr() (sao numerais universais).
            char num[32];

            // HP: barra + preenchimento real (hp/max_hp) + "hp/max" a direita da barra.
            const Rect hp_frame{x, y, static_cast<float>(kHpBarW),
                                static_cast<float>(kHpBarH)};
            draw_bar(renderer, hp_frame, bar_fill(a->hp(), a->max_hp()),
                     kHpFillColor);
            std::snprintf(num, sizeof(num), "%d/%d", a->hp(), a->max_hp());
            renderer.draw_text(num, x + kHpBarW + 4.0f,
                               y + (kHpBarH - kPanelTextPx) * 0.5f, kPanelTextPx,
                               kPanelTextColor, /*bold=*/false);
            y += kHpBarH + kResourceRowGap;

            // AP: pips (max_ap pips; ap acesos) + "AP n/m" apos os pips.
            const int ap_max = gus::domain::combat::combat_constants::kBaseApPerTurn;
            draw_pips(renderer, x, y, a->max_ap(), a->ap(), ap_max, kApLitColor,
                      kApOffColor);
            std::snprintf(num, sizeof(num), "AP %d/%d", a->ap(), a->max_ap());
            renderer.draw_text(
                num, x + static_cast<float>(ap_max) * (kPipSize + kPipGap) + 4.0f, y,
                kPanelTextPx, kPanelTextColor, /*bold=*/false);
            y += kPipSize + kResourceRowGap;

            // Mana: pips (max_mana pips, cap kManaCap=8; mana acesos) + "Mana n/m". O
            // ramp (max_mana = 2 + round_index, cap 8) ja esta no max_mana() do ator.
            const int mana_cap = gus::domain::combat::combat_constants::kManaCap;
            draw_pips(renderer, x, y, a->max_mana(), a->mana(), mana_cap, kManaLitColor,
                      kManaOffColor);
            std::snprintf(num, sizeof(num), "Mana %d/%d", a->mana(), a->max_mana());
            renderer.draw_text(
                num, x + static_cast<float>(mana_cap) * (kPipSize + kPipGap) + 4.0f, y,
                kPanelTextPx, kPanelTextColor, /*bold=*/false);

            // Status effects ATIVOS: 1 icone por efeito, na lateral direita do painel.
            float sx = p.x + p.w - kPanelPad - kStatusIconSize;
            const float sy = p.y + kPanelPad;
            for (const auto& st : a->status_effects()) {
                const Rect box{sx, sy, static_cast<float>(kStatusIconSize),
                               static_cast<float>(kStatusIconSize)};
                const TextureId tex = status_icons_.find(st.id);
                if (tex != kInvalidTexture) {
                    renderer.draw_textured_rect(box, tex, UvRect{}, kWhite);
                } else {
                    renderer.draw_filled_rect(box, kStatusBoxColor);  // placeholder
                }
                sx -= (kStatusIconSize + kStatusIconGap);  // empilha pra esquerda
                if (sx < x + kHpBarW) {
                    break;  // nao invade a area de HP/recursos
                }
            }

            // --- menu de verbos comando-first (incremento 3) ---
            // So no turno de JOGADOR vivo (CTB por-ator) e enquanto o combate roda. Ocupa
            // a METADE DIREITA do painel, em coluna. Cada verbo = caixa colorida (sem set
            // de icones de verbo); desabilitado (sem AP) = cinza; selecionado = borda.
            if (current_actor_is_player() && !combat_over()) {
                const Rect menu_zone{p.x + p.w * 0.5f, p.y, p.w * 0.5f, p.h};
                const auto items = menu_.layout(menu_zone);
                for (int i = 0; i < kBattleVerbCount; ++i) {
                    const MenuItem& it = items[static_cast<std::size_t>(i)];
                    const DrawColor col =
                        it.enabled ? verb_color(it.verb) : kVerbDisabledColor;
                    renderer.draw_filled_rect(it.rect, col);
                    if (i == menu_.selected_index()) {
                        renderer.draw_rect_outline(it.rect, kVerbSelectColor, 2.0f);
                    }
                    // ROTULO do verbo (incremento 3.5): NOME legivel via tr(), centrado
                    // verticalmente na caixa, com pequena margem a esquerda. Resolve o
                    // "injogavel" (caixa colorida sem nome). Sem translator => no-op
                    // (draw_text degrada; a caixa colorida ainda orienta).
                    const std::string label = tr_verb_label(it.verb);
                    const float ty = it.rect.y + (it.rect.h - kVerbTextPx) * 0.5f;
                    renderer.draw_text(label.c_str(), it.rect.x + 3.0f, ty,
                                       kVerbTextPx, kVerbTextColor, /*bold=*/false);
                }
            }
        }
    }

    // --- caixa de log (base): ESTRUTURA por evento (D7), 1 marca colorida por linha ---
    {
        const Rect l = log_panel_rect();
        renderer.draw_filled_rect(l, kLogColor);
        renderer.draw_rect_outline(l, kHudBorderColor, 1.0f);

        // Linha de log = TEXTO (incremento 3.5) da message real do motor, na cor da
        // categoria (D7). Uma marca curta colorida a esquerda ancora a cor (legivel
        // mesmo se a fonte faltar: a marca sobrevive headless/sem-fonte). Bold pra
        // sistema/dano notavel (criticos/COMPILADO).
        constexpr float kLogLineH = kLogTextPx;  // altura da linha = altura do glifo
        constexpr float kLogLineGap = 2.0f;
        const float pad = 2.0f;
        const int capacity = static_cast<int>(
            (l.h - 2.0f * pad) / (kLogLineH + kLogLineGap));
        if (capacity > 0) {
            const std::vector<LogLine> lines = log_lines(capacity);
            float ly = l.y + pad;
            for (const LogLine& line : lines) {
                const DrawColor col = log_line_color(line.kind);
                // Marca de cor (ancora a categoria; fallback sem-fonte).
                const Rect mark{l.x + pad, ly + 1.0f, 3.0f, kLogLineH - 2.0f};
                renderer.draw_filled_rect(mark, col);
                // Texto da message (sistema/dano em bold pra enfase).
                const bool bold = line.kind == LogLineKind::System ||
                                  line.kind == LogLineKind::Damage ||
                                  line.kind == LogLineKind::Defeat;
                renderer.draw_text(line.text.c_str(), l.x + pad + 5.0f, ly,
                                   kLogTextPx, col, bold);
                ly += kLogLineH + kLogLineGap;
            }
        }
    }

    renderer.end_frame();
}

}  // namespace gus::app::screens
