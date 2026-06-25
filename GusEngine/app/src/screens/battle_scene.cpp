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

#include <algorithm>  // std::remove_if (poda floaters mortos)
#include <cstdio>     // std::snprintf (numeros do painel)
#include <optional>
#include <string>
#include <utility>    // std::move
#include <vector>

#include "gus/app/screens/battle_floaters.hpp"
#include "gus/app/screens/battle_hud_model.hpp"
#include "gus/app/screens/battle_layout.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_constants.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"  // CombatState (preview_intent)
#include "gus/platform/render2d/text_metrics.hpp"  // text_width (centrar floater)

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

// Intent (telegraph, incremento 5): tamanho do icone + cor da marca placeholder.
constexpr float kIntentIconSize = 12.0f;
constexpr DrawColor kIntentMarkColor{0.95f, 0.75f, 0.20f, 1.0f};  // placeholder ambar
// Numero flutuante de dano: tamanho do texto (px logico). 16px = tamanho NATIVO da
// Pixel Operator (denso/nitido) e GRANDE o bastante pra o criador VER o dano (o teste no
// display mostrou que o numero pequeno sumia). Sobe sobre o alvo e some por fade.
constexpr float kFloaterTextPx = 16.0f;

// Banner de turno (D9/D10): tamanho + cor. Texto grande e centrado abaixo da fila CTB.
constexpr float kBannerTextPx = 16.0f;
constexpr DrawColor kBannerBgColor{0.05f, 0.05f, 0.09f, 0.85f};   // faixa semi-opaca
constexpr DrawColor kBannerPlayerColor{0.55f, 0.95f, 0.65f, 1.0f};  // sua vez (verde)
constexpr DrawColor kBannerEnemyColor{0.95f, 0.55f, 0.45f, 1.0f};   // vez do inimigo
constexpr DrawColor kBannerIntroColor{0.98f, 0.92f, 0.45f, 1.0f};   // BATALHA! (ambar)

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

// Textura do icone de intent pra um IntentPreview (incremento 5). is_chaotic => ruido
// (Patch-Zero); senao pelo predicted_action_id (attack/defend/status). kInvalidTexture
// se o icone correspondente nao foi setado (o render cai na marca placeholder).
TextureId intent_icon_tex(const BattleIntentIconSet& icons,
                          const gus::domain::combat::IntentPreview& intent) noexcept {
    if (intent.is_chaotic) {
        return icons.ruido;
    }
    const std::string& a = intent.predicted_action_id;
    if (a.find("defend") != std::string::npos) {
        return icons.defender;
    }
    if (a.find("status") != std::string::npos || a.find("card") != std::string::npos) {
        return icons.aplicar_status;
    }
    // "attack"/"pass"/default: o telegraph mais comum e o ataque.
    return icons.atacar;
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

    // ABERTURA LIMPA (incremento 6, BUG B): os atores comecam com HP CHEIO - "ninguem
    // agiu" (D10). Antes o demo pre-aplicava dano (HP parcial) pra exibir as barras, mas
    // no display isso parecia que um inimigo ja tinha atacado, atropelando o BATALHA!. As
    // barras de HP continuam visiveis (cheias) e enchem/esvaziam no ritmo do combate.
    //
    // Status de demo como CONDICAO INICIAL (nao "ataque"): Caua (v[1]) ganha Haste pra o
    // painel do 1o jogador a agir mostrar o icone; Gus (v[0]) Regen. Sao buffs/debuffs de
    // partida (icone sob o ator), nao combate resolvido. NAO ha Poison no inimigo no
    // start (evitaria parecer "dano ja aplicado" quando ticasse no 1o turno).
    v[0]->add_status(gus::domain::combat::StatusEffect{
        gus::domain::combat::StatusId::Regen, /*magnitude=*/3, /*duration=*/2});
    v[1]->add_status(gus::domain::combat::StatusEffect{
        gus::domain::combat::StatusId::Haste, /*magnitude=*/2, /*duration=*/3});
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

    // BRAINS dos inimigos (incremento 5): cada inimigo ganha um ScriptedBrain (telegraph
    // honesto, ataca o 1o player vivo). O brain_registry (id -> brain) habilita o
    // IntentPreview (intent sobre o inimigo) e o Gambito-Prever. DONOS em brains_; o
    // registry guarda ponteiros NAO-donos, vivos enquanto a cena viver.
    for (const auto& a : actors_) {
        if (!a->is_player_side()) {
            brains_.push_back(
                std::make_unique<gus::domain::combat::ScriptedBrain>());
            brain_registry_[a->id()] = brains_.back().get();
        }
    }

    // Provider = MAILBOX (incremento 3): devolve a acao pendente (do menu, pro jogador;
    // ou auto, pro inimigo) e a reseta pra Pass. Captura `this` pra acessar o mailbox.
    auto provider = [this](CombatActor& actor,
                           const gus::domain::combat::CombatState&) {
        return take_pending_action(actor);
    };
    machine_ = std::make_unique<gus::domain::combat::CombatStateMachine>(
        std::move(ptrs), std::move(provider), /*card_registry=*/nullptr,
        &brain_registry_);

    // D10 ABERTURA PARADA (incremento 6): NINGUEM age no ctor. O diretor de pacing comeca
    // em Intro ("BATALHA!"); so prepara o turno do 1o ator (begin_turn pra o painel
    // mostrar AP/Mana), SEM resolver. Os turnos so comecam a animar quando a intro termina
    // (update/skip), um a um com o ritmo. Resolve o problema do playtest ("inimigos ja
    // atacaram antes de eu ver").
    start_active_turn();
    menu_.refresh(active_actor() != nullptr ? active_actor()->ap() : 0);
}

CombatAction BattleScene::take_pending_action(CombatActor& actor) {
    // Inimigo: age UMA vez por turno (1 ataque) e depois Pass. Assim cada ataque de
    // inimigo aparece NO SEU TEMPO com seu floater (D11) - sem gastar os 3 AP de uma vez
    // (que faria 3 ataques instantaneos num turno so). Jogador: consome o mailbox (acao
    // do menu) e reseta pra Pass.
    if (!actor.is_player_side()) {
        if (enemy_acted_this_turn_) {
            return CombatAction::pass();  // ja agiu neste turno: encerra
        }
        enemy_acted_this_turn_ = true;
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

std::optional<gus::core::spatial::Rect> BattleScene::arena_rect_for_actor(
    const std::string& actor_id) const {
    const ArenaLayout arena =
        arena_layout(party_count(), enemy_count(), gus_party_index());
    // Reconstroi a ordem dos vivos por lado (mesma do render): o i-esimo vivo casa o
    // i-esimo slot. Acha o lado e o indice do ator pedido.
    int p_idx = 0, e_idx = 0;
    for (const CombatActor* a : machine_->queue().order()) {
        if (a == nullptr || !a->is_alive()) {
            continue;
        }
        if (a->is_player_side()) {
            if (a->id() == actor_id && p_idx < arena.party_count) {
                return arena.party[static_cast<std::size_t>(p_idx)].rect;
            }
            ++p_idx;
        } else {
            if (a->id() == actor_id && e_idx < arena.enemy_count) {
                return arena.enemies[static_cast<std::size_t>(e_idx)].rect;
            }
            ++e_idx;
        }
    }
    return std::nullopt;
}

void BattleScene::spawn_floaters_from_new_logs() {
    const auto& log = machine_->log();
    for (std::size_t i = log_cursor_; i < log.size(); ++i) {
        const auto& e = log[i];
        // So Attack/UseCard geram numero flutuante de dano (Defend/Scan/Gambito/Flee
        // narram no log, nao "batem" num alvo). Precisa de alvo pra ancorar.
        const bool is_hit = e.action == gus::domain::combat::CombatActionType::Attack ||
                            e.action == gus::domain::combat::CombatActionType::UseCard;
        if (!is_hit || !e.target_id.has_value()) {
            continue;
        }
        // COMPILADO: <combo> e anuncio de sistema (value 0, sem dano) - nao floata.
        if (e.message.rfind("COMPILADO:", 0) == 0) {
            continue;
        }
        const std::optional<gus::core::spatial::Rect> slot =
            arena_rect_for_actor(*e.target_id);
        if (!slot.has_value()) {
            continue;  // alvo ja saiu da arena (morreu): sem ancora, pula
        }
        const HitResult hit = parse_hit(e.message, e.value, /*is_heal=*/false);
        // Posicao: topo-centro do slot do alvo (o numero nasce sobre a cabeca).
        Floater f;
        f.text = hit.text;
        f.channel = hit.channel;
        f.origin_x = slot->x + slot->w * 0.5f;
        f.origin_y = slot->y - 2.0f;
        f.age = 0.0f;
        floaters_.push_back(std::move(f));
    }
    log_cursor_ = log.size();
}

void BattleScene::narrate_new_logs() {
    // D12: monta a NARRACAO de cada evento NOVO do motor = a message do motor (ja diz
    // "X ataca Y por N") + a CONSEQUENCIA de status aplicado no alvo naquele evento
    // ("; Y ficou com <Status>"), com o nome do status resolvido via tr(). Uma linha por
    // evento. A cor vem da categoria (classify).
    const auto& log = machine_->log();
    const auto& changes = machine_->status_changes();
    for (std::size_t i = narration_cursor_; i < log.size(); ++i) {
        const auto& e = log[i];
        LogLine line = classify(e);  // categoria + message crua do motor
        // Consequencia: status aplicado no alvo do golpe. consequence_suffix devolve a
        // forma com a CHAVE i18n; resolvemos a chave via tr() pra exibir o nome.
        if (e.target_id.has_value()) {
            std::string sfx = consequence_suffix(*e.target_id, changes);
            if (!sfx.empty() && translator_ != nullptr) {
                // Troca cada STATUS_*_NAME pela traducao (a chave aparece literal no sfx).
                for (int s = 0; s < 13; ++s) {
                    const auto id = static_cast<gus::domain::combat::StatusId>(s);
                    const std::string key(status_name_key(id));
                    std::size_t pos = sfx.find(key);
                    while (pos != std::string::npos) {
                        sfx.replace(pos, key.size(), translator_->tr(key));
                        pos = sfx.find(key, pos + 1);
                    }
                }
            }
            line.text += sfx;
        }
        narration_.push_back(std::move(line));
    }
    narration_cursor_ = log.size();
}

void BattleScene::update(float dt_seconds) {
    if (dt_seconds <= 0.0f) {
        return;
    }
    // Anima os numeros flutuantes (envelhece + poda).
    for (Floater& f : floaters_) {
        f.age += dt_seconds;
    }
    floaters_.erase(
        std::remove_if(floaters_.begin(), floaters_.end(),
                       [](const Floater& f) { return !floater_alive(f.age); }),
        floaters_.end());

    // RITMO (incremento 6): avanca o diretor pelo tempo; quando ele LIBERA um passo (a
    // intro/delay terminou), conduz UM turno (de inimigo) ou entra em espera-do-jogador.
    pacing_.tick(dt_seconds);
    if (pacing_.ready_to_step()) {
        advance_pacing();
    }
}

void BattleScene::advance_pacing() {
    if (combat_over()) {
        return;
    }
    // Garante que o turno do ator ativo foi preparado (begin_turn).
    start_active_turn();
    if (combat_over()) {
        return;
    }
    // Vez do JOGADOR: nao auto-resolve - entra em espera-do-input (D9 "sua vez"). O menu
    // ja esta habilitado pelo start_active_turn. So retoma o ritmo no menu_confirm.
    if (current_actor_is_player()) {
        if (pacing_.state() != PacingState::WaitingPlayerInput) {
            pacing_.begin_player_turn();
        }
        return;
    }
    // Vez do INIMIGO: resolve UM turno (com floater + log de consequencia) e segura o
    // delay pro jogador LER (D8/D11). O proximo turno so anima apos o delay.
    resolve_one_turn();
    pacing_.begin_enemy_step();
}

void BattleScene::skip() {
    // D8: acelera a intro / encurta o delay entre turnos. Nao pula o turno do jogador.
    pacing_.skip();
}

std::string_view BattleScene::turn_banner_key() const noexcept {
    // D9/D10: a casca resolve via tr() (+ nome do ator ativo pro turno do jogador).
    if (combat_over()) {
        return "";
    }
    if (is_intro()) {
        return "COMBAT_BANNER_BATTLE";  // "BATALHA!"
    }
    if (current_actor_is_player()) {
        return "COMBAT_BANNER_PLAYER_TURN";  // "TURNO DE {0}" / "SUA VEZ"
    }
    return "COMBAT_BANNER_ENEMY_TURN";  // "vez do inimigo"
}

std::optional<gus::domain::combat::IntentPreview> BattleScene::intent_for(
    const CombatActor& enemy) const {
    if (combat_over() || enemy.is_player_side() || !enemy.is_alive()) {
        return std::nullopt;
    }
    const auto it = brain_registry_.find(enemy.id());
    if (it == brain_registry_.end() || it->second == nullptr) {
        return std::nullopt;
    }
    // Monta a CombatState read-only pro brain prever (mesmo padrao do motor).
    const gus::domain::combat::CombatState state(
        machine_->queue(), machine_->active_actor(), machine_->queue().round_index());
    return it->second->preview_intent(state, enemy);
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

void BattleScene::start_active_turn() {
    // Prepara o turno do ator ativo (begin_turn: recarrega AP/Mana, tick de status).
    // Idempotente por turno: so chama begin_turn UMA vez por ator (turn_started_).
    if (turn_started_ || combat_over()) {
        return;
    }
    machine_->begin_turn();
    turn_started_ = true;
    enemy_acted_this_turn_ = false;  // novo turno: o inimigo (se for) pode agir 1 vez
    // Se o begin_turn drenou status que viraram dano (Poison no TurnStart), narra/floata.
    spawn_floaters_from_new_logs();
    narrate_new_logs();
    menu_.refresh(active_actor() != nullptr ? active_actor()->ap() : 0);
}

void BattleScene::resolve_one_turn() {
    // A acao ja esta no mailbox (jogador) ou sera auto (inimigo). run_active_turn_to_end
    // consome AP ate Pass/0. Depois: floaters + narracao de consequencia (D11/D12),
    // check_end, avanca pro proximo ator e prepara o turno dele (start_active_turn).
    if (machine_->phase() == gus::domain::combat::CombatPhase::ActionSelect) {
        machine_->run_active_turn_to_end();
    }
    // Floater (D11) + narracao com consequencia (D12) ANTES do advance: o alvo ainda
    // esta vivo/visivel na arena pra ancorar o numero.
    spawn_floaters_from_new_logs();
    narrate_new_logs();
    if (!machine_->check_end()) {
        machine_->advance_to_next_actor();
        turn_started_ = false;       // o proximo ator ainda nao teve begin_turn
        start_active_turn();         // prepara o proximo (AP/Mana/painel) ja
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

    // RITMO (incremento 6): submete a acao do jogador e resolve SO o turno dele (com
    // floater + narracao de consequencia). NAO encadeia os inimigos aqui: o diretor
    // entra em delay (player_acted) e o update(dt) anima os turnos de inimigo um a um.
    pending_action_ = std::move(action);
    resolve_one_turn();
    pacing_.player_acted();
    menu_.refresh(active_actor() != nullptr ? active_actor()->ap() : 0);
}

std::vector<LogLine> BattleScene::log_lines(int max_lines) const {
    // NARRACAO do combate (D12, incremento 6): as linhas ja vem montadas em narration_
    // (acao + dano + consequencia de status), no ritmo do pacing. Junta as linhas de UI
    // (COMPILAR/GAMBITO sinalizados) ao fim e corta pras ULTIMAS N (a caixa rola).
    std::vector<LogLine> lines = narration_;
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

    // --- BANNER DE TURNO (D9/D10, incremento 6): quem joga, abaixo da fila CTB ---
    // "BATALHA!" na intro / "SUA VEZ: escolha uma acao" no jogador / "Vez do inimigo".
    // Resolve a chave via tr() (+ nome do ator ativo no turno do jogador). Sem translator
    // ou combate acabado, nao desenha (a casca degrada sem texto).
    if (!combat_over() && translator_ != nullptr) {
        const std::string_view key = turn_banner_key();
        if (!key.empty()) {
            std::string text = translator_->tr(std::string(key));
            // "Vez de {0}": interpola o NOME do ator ativo (D9; jogador e inimigo). O
            // banner de turno usa display_name (ex: "Vez de Gus", "Vez de Drone").
            const auto pos = text.find("{0}");
            if (pos != std::string::npos) {
                const auto* a = active_actor();
                text.replace(pos, 3, a != nullptr ? a->display_name() : "");
            }
            DrawColor col = kBannerIntroColor;
            if (current_actor_is_player()) {
                col = kBannerPlayerColor;
            } else if (!is_intro()) {
                col = kBannerEnemyColor;
            }
            // Faixa de fundo + texto centrado horizontalmente, logo abaixo da fila CTB.
            const float by = static_cast<float>(kCtbStripTop + kCtbPortraitPx +
                                                kCtbStripTop + 2);
            const Rect band{0.0f, by, static_cast<float>(kBattleLogicalW),
                            kBannerTextPx + 4.0f};
            renderer.draw_filled_rect(band, kBannerBgColor);
            const float tw = gus::platform::render2d::text_width(text, kBannerTextPx);
            const float tx = (static_cast<float>(kBattleLogicalW) - tw) * 0.5f;
            renderer.draw_text(text.c_str(), tx, by + 2.0f, kBannerTextPx, col,
                               /*bold=*/true);
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
                    // INTENT (telegraph, incremento 5): icone sobre cada INIMIGO, lendo o
                    // IntentPreview do ScriptedBrain (o que ele vai fazer). Sem icone
                    // setado => marca ambar placeholder. So pra inimigos vivos.
                    if (!a->is_player_side()) {
                        const auto intent = intent_for(*a);
                        if (intent.has_value()) {
                            const Rect ibox{r.x + (r.w - kIntentIconSize) * 0.5f,
                                            r.y - kIntentIconSize - 1.0f,
                                            kIntentIconSize, kIntentIconSize};
                            const TextureId itex = intent_icon_tex(
                                intent_icons_, *intent);
                            if (itex != kInvalidTexture) {
                                renderer.draw_textured_rect(ibox, itex, UvRect{},
                                                            kWhite);
                            } else {
                                renderer.draw_filled_rect(ibox, kIntentMarkColor);
                            }
                        }
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

    // --- NUMEROS FLUTUANTES de dano (incremento 5), POR CIMA de tudo ---
    // Cada floater sobe + some pela idade (battle_floaters). Cor por canal; CRIT em bold.
    // O texto e centrado sobre o alvo (origin_x e o centro). Alpha = fade pela idade.
    for (const Floater& f : floaters_) {
        if (!floater_alive(f.age)) {
            continue;
        }
        DrawColor col = floater_color_for_channel(f.channel);
        col.a = floater_alpha(f.age);  // fade
        const float w = gus::platform::render2d::text_width(f.text, kFloaterTextPx);
        const float fx = f.origin_x - w * 0.5f;  // centra o texto sobre o alvo
        const float fy = f.origin_y + floater_offset_y(f.age);  // sobe
        const bool bold = f.channel == HitChannel::Crit;
        renderer.draw_text(f.text.c_str(), fx, fy, kFloaterTextPx, col, bold);
    }

    renderer.end_frame();
}

}  // namespace gus::app::screens
