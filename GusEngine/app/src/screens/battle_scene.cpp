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
// RENDER 1:1: a camera do IRenderer e dirigida pro retangulo logico 960x540 (D1), entao
// cada Rect do battle_layout (em px logico) vira um Rect de mundo identico. O backend
// (Render2dSdl) escala 960x540 -> janela por inteiro (x2 = 1080p, pixel-perfect),
// letterbox a cargo do backend/janela.

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
#include "gus/app/screens/sprite_anchor.hpp"  // pe real do sprite (W3: bbox do idle)
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_constants.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"  // CombatState (preview_intent)
#include "gus/domain/combat/weakness_wheel.hpp"  // WeaknessWheel (pre-selecao D3 de mira)
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

// Tamanho de texto (px logico) das zonas. Pixel Operator e crisp a multiplos de 8/16. Os
// textos de corpo subiram 8px->16px com a resolucao 960x540 (lider 2026-06-25), mantendo
// a legibilidade proporcional ao canvas 1.5x maior (e crisp no x2 ate 1080p).
constexpr float kVerbTextPx = 12.0f;    // rotulo do verbo na caixa do menu (cockpit fino)
constexpr float kLogTextPx = 11.0f;     // linha do log (terminal fino)
constexpr float kPanelTextPx = 13.0f;   // numeros gerais
// Texto do COCKPIT (variante C): nome do ator, numeros, rotulos.
constexpr float kCockpitNamePx = 15.0f;   // nome grande do ator ativo
constexpr float kCockpitTextPx = 8.0f;    // numero "hp/max" sobre a barra (compacto)
constexpr float kCockpitLabelPx = 9.0f;   // rotulos "AP"/"MANA"/"ACAO"
constexpr float kCockpitResLabelW = 34.0f;  // largura reservada pro rotulo antes dos pips

// --- PALETA CANONICA "Tatico Cockpit" (variante C, HEX exatos do _common.css aprovado
//     pelo criador 2026-06-25). Cores em [0,1]. ---
constexpr DrawColor kBgColor{0.047f, 0.059f, 0.102f, 1.0f};        // #0c0f1a fundo
constexpr DrawColor kBg1Color{0.106f, 0.133f, 0.220f, 1.0f};       // #1B2238 cockpit topo
constexpr DrawColor kBg2Color{0.078f, 0.102f, 0.173f, 1.0f};       // #141a2c cockpit base
constexpr DrawColor kTerminalColor{0.055f, 0.075f, 0.133f, 1.0f};  // #0e1322 terminal/log
constexpr DrawColor kSlotDarkColor{0.039f, 0.051f, 0.086f, 1.0f};  // #0a0d16 slot escuro
constexpr DrawColor kCyan{0.133f, 0.827f, 0.933f, 1.0f};           // #22D3EE party/ativo/CRIT
constexpr DrawColor kCyanDim{0.082f, 0.369f, 0.420f, 1.0f};        // #155e6b
constexpr DrawColor kMagenta{0.882f, 0.114f, 0.455f, 1.0f};        // #E11D74 inimigo/intent
constexpr DrawColor kMagentaDim{0.361f, 0.071f, 0.188f, 1.0f};     // #5c1230
constexpr DrawColor kBrass{0.910f, 0.639f, 0.239f, 1.0f};          // #E8A33D Compilar/AP/fraqueza
constexpr DrawColor kHp{0.247f, 0.725f, 0.478f, 1.0f};             // #3FB97A HP verde
constexpr DrawColor kHpDim{0.114f, 0.361f, 0.235f, 1.0f};          // #1d5c3c
constexpr DrawColor kErr{0.957f, 0.247f, 0.369f, 1.0f};            // #F43F5E erro/FALHA
constexpr DrawColor kInk{0.812f, 0.902f, 0.933f, 1.0f};            // #cfe6ee tinta
constexpr DrawColor kInkDim{0.435f, 0.522f, 0.576f, 1.0f};         // #6f8593 tinta-dim
constexpr DrawColor kLine{0.165f, 0.204f, 0.314f, 1.0f};           // #2a3450 borda
constexpr DrawColor kWhite{1.0f, 1.0f, 1.0f, 1.0f};                // tint neutro

// Mapeamentos semanticos sobre a paleta canonica:
// (kCtbBandColor removido: a faixa/caixa de fundo da fila CTB foi aposentada - a fila
//  agora fica seamless sobre a vinheta da arena. Ver o bloco de render da fila CTB.)
constexpr DrawColor kCtbCellColor = kSlotDarkColor;   // celula CTB ocupada
constexpr DrawColor kCtbNextColor = kCyan;            // marca "proximo" (cyan)
constexpr DrawColor kPartyColor = kCyanDim;           // placeholder party (cyan dim)
constexpr DrawColor kEnemyColor = kMagentaDim;        // placeholder inimigo (magenta dim)
constexpr DrawColor kActiveHiColor = kCyan;           // highlight ator ativo (cyan)
constexpr DrawColor kPanelColor = kBg1Color;          // fundo cockpit (gradiente -> usa bg1)
constexpr DrawColor kLogColor = kTerminalColor;       // caixa de log (terminal)
constexpr DrawColor kHudBorderColor = kLine;          // contorno HUD (borda)
constexpr DrawColor kBarBackColor = kSlotDarkColor;   // fundo de barra
constexpr DrawColor kHpFillColor = kHp;               // HP cheio (verde)
constexpr DrawColor kApLitColor = kBrass;             // pip de AP aceso (latao)
constexpr DrawColor kApOffColor = kSlotDarkColor;     // pip de AP apagado
constexpr DrawColor kManaLitColor = kCyan;            // pip de Mana aceso (cyan)
constexpr DrawColor kManaOffColor = kSlotDarkColor;   // pip de Mana apagado
constexpr DrawColor kStatusBoxColor = kMagentaDim;    // placeholder de status
constexpr DrawColor kActiveCtbColor = kCyan;          // marca de ativo na fila (cyan)
constexpr DrawColor kVerbColor = kInk;                // texto de verbo neutro
constexpr DrawColor kVerbBoxColor{0.063f, 0.090f, 0.165f, 1.0f};   // #10172a fundo do verbo
constexpr DrawColor kVerbDisabledColor = kSlotDarkColor;           // verbo sem AP
constexpr DrawColor kVerbSelectColor = kCyan;         // verbo selecionado (cyan)
constexpr DrawColor kVerbPrimaryColor = kBrass;       // COMPILAR (latao)
// --- log: cor por categoria de linha (D7), na paleta canonica ---
constexpr DrawColor kLogSystemColor = kCyan;          // sistema (cyan, bold)
constexpr DrawColor kLogDamageColor = kInk;           // dano (tinta clara)
constexpr DrawColor kLogHealColor = kHp;              // cura (verde)
constexpr DrawColor kLogStatusColor = kMagenta;       // status (magenta)
constexpr DrawColor kLogDefeatColor = kErr;           // derrota (erro)

// Cor de DESTAQUE de um verbo (selecionado=cyan, Compilar=latao, demais=neutro).
DrawColor verb_color(BattleVerb v, bool selected) noexcept {
    if (selected) {
        return kVerbSelectColor;
    }
    if (v == BattleVerb::Compilar) {
        return kVerbPrimaryColor;
    }
    return kVerbColor;
}

// Intent (telegraph, incremento 5): tamanho do icone + cor da marca placeholder. Subiu
// com a resolucao 960x540 (lider 2026-06-25), proporcional ao slot de ator maior.
constexpr float kIntentIconSize = 18.0f;
constexpr DrawColor kIntentMarkColor{0.95f, 0.75f, 0.20f, 1.0f};  // placeholder ambar
// Numero flutuante de dano: tamanho do texto (px logico). 24px (1.5x os 16px do 640x360):
// Pixel Operator densa/nitida e GRANDE o bastante pra o criador VER o dano no canvas
// maior. Sobe sobre o alvo e some por fade.
constexpr float kFloaterTextPx = 24.0f;

// --- MODO-MIRA (§3.5, D3): destaque MULTIMODAL do inimigo mirado (Pillar 4/WCAG) ---
// A mira e CYAN na paleta canonica ("party/ativo/mira/CRIT"). Mas cor NUNCA basta
// (daltonismo): o alvo mirado ganha (1) CONTORNO reticulo (outline duplo, distinto do
// outline simples do ativo), (2) SETA em forma pura (caret ►, a esquerda do slot), e
// (3) NOME + HP em texto legivel. As 3 pistas sobrevivem em escala de cinza.
constexpr DrawColor kMiraColor = kCyan;         // cor da mira (cyan canonico)
constexpr float kMiraLabelPx = 12.0f;           // texto do nome/HP do alvo mirado
constexpr float kMiraCaretW = 3.0f;             // largura de cada coluna do caret ►
constexpr float kMiraFracoPipSize = 6.0f;       // pip do tier "fraco" (latao) se escaneado

// Banner de turno (D9/D10): tamanho + cor. Texto grande e centrado abaixo da fila CTB.
// 24px (1.5x os 16px do 640x360) pra dominar a tela maior na abertura.
constexpr float kBannerTextPx = 24.0f;
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
using gus::domain::combat::CardFamily;
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

const CombatActor* BattleScene::actor_by_id(const std::string& id) const {
    for (const CombatActor* a : machine_->queue().order()) {
        if (a != nullptr && a->id() == id) {
            return a;
        }
    }
    return nullptr;
}

const SpriteClip* BattleScene::resolve_sprite_clip(const std::string& id,
                                                   BattleClipId* out_clip,
                                                   float* out_elapsed) const {
    const auto sit = sprites_.find(id);
    if (sit == sprites_.end()) {
        return nullptr;  // ator sem sprite set: retrato placeholder
    }
    const BattleSpriteAnimator::Playback pb = sprite_anim_.playback_for(id);
    const SpriteClip* clip = sit->second.find(pb.clip);
    BattleClipId cid = pb.clip;
    if (clip == nullptr) {
        // Clip da fase sem frames no disco: degrau 1 = fallback do modulo POCO
        // (perfil ausente -> front-facing equivalente: run_east/run_west -> run;
        // attack_melee_east -> attack_melee). O relogio corrente vale (a cadencia
        // difere, mas o reset ja aconteceu na troca de clip).
        cid = clip_fallback(pb.clip);
        clip = sit->second.find(cid);
    }
    if (clip == nullptr) {
        // Degrau 2: Idle (melhor um idle vivo que um buraco).
        clip = sit->second.find(BattleClipId::Idle);
        cid = BattleClipId::Idle;
    }
    if (clip == nullptr) {
        return nullptr;  // nem Idle tem frames: retrato placeholder
    }
    if (out_clip != nullptr) {
        *out_clip = cid;
    }
    if (out_elapsed != nullptr) {
        *out_elapsed = pb.elapsed;
    }
    return clip;
}

std::optional<std::pair<BattleClipId, int>> BattleScene::actor_sprite_frame(
    const std::string& id) const {
    BattleClipId cid = BattleClipId::Idle;
    float elapsed = 0.0f;
    const SpriteClip* clip = resolve_sprite_clip(id, &cid, &elapsed);
    if (clip == nullptr) {
        return std::nullopt;
    }
    const int fi = clip_frame_index(static_cast<int>(clip->frames.size()), clip->fps,
                                    clip->loop, elapsed);
    return std::make_pair(cid, fi);
}

bool BattleScene::start_melee_toward(const std::string& attacker_id,
                                     const std::string& target_id, float seconds) {
    // Desloca-golpeia-volta (battle-anim.md par.2.2): o atacante anda da posicao de
    // repouso ate ADJACENTE ao slot do alvo (folga kMeleeContactGapPx), SEM flip
    // (Pillar 3): party avanca pra direita, inimigo pra esquerda - so translacao.
    // Distancia = derivada dos SLOTS REAIS (arena_rect_for_actor), entao a troca
    // placeholder -> sprite nao muda nada aqui (par.1.1/3.2).
    const std::optional<Rect> ar = arena_rect_for_actor(attacker_id);
    const std::optional<Rect> tr = arena_rect_for_actor(target_id);
    const CombatActor* atk = actor_by_id(attacker_id);
    if (!ar.has_value() || !tr.has_value() || atk == nullptr) {
        return false;  // slot invisivel: degrada (caller resolve sem animacao)
    }
    const float dest_x = atk->is_player_side()
                             ? (tr->x - ar->w - kMeleeContactGapPx)   // para a ESQ do alvo
                             : (tr->x + tr->w + kMeleeContactGapPx);  // para a DIR do alvo
    anim_.start_melee_approach(
        attacker_id, Vec2{dest_x - ar->x, tr->y - ar->y}, seconds);
    return true;
}

void BattleScene::debug_cast_demo() {
    // DIAGNOSTICO (dormante, par.2.1): cast cosmetico do 1o player vivo no 1o inimigo
    // vivo. O update(dt) spawna o projetil quando o windup termina; o impacto dispara o
    // hit-react visual. ZERO motor (sem dano/log) - so o esqueleto de timing que o
    // COMPILAR real vai reusar.
    const CombatActor* caster = first_alive_player();
    const CombatActor* target = first_alive_enemy();
    if (caster == nullptr || target == nullptr) {
        return;
    }
    anim_.start_cast(caster->id(), kCastWindupSeconds);
    demo_cast_active_ = true;
    demo_cast_caster_ = caster->id();
    demo_cast_target_ = target->id();
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

        // HIT-REACT do alvo (W2, battle-anim.md par.2.3): TODO golpe que CONECTA da um
        // tranco visual pra tras (cosmetico) - aqui e exatamente o CONTATO (o floater
        // nasce junto). FALHA (canal Fail, dano 0) NAO tem hit-react (par.2.3: o efeito
        // dissolve antes de tocar o alvo; o alvo fica em battle-idle) - GANCHO DORMANTE:
        // o ataque basico atual e deterministico (dano minimo >= 1; o motor loga o dano
        // BRUTO pre-Shield), entao Fail so vai nascer da formula de cartas (combat.md
        // par.11) quando o COMPILAR ligar; o gate ja esta pronto aqui. Direcao: "pra
        // tras" = pra longe dos oponentes (party recua pra ESQ, inimigo pra DIR). Nunca
        // em Heal (cura nao e golpe).
        if (hit.channel == HitChannel::Common || hit.channel == HitChannel::Crit) {
            const CombatActor* victim = actor_by_id(*e.target_id);
            const float dir =
                (victim != nullptr && victim->is_player_side()) ? -1.0f : 1.0f;
            anim_.start_hit_react(*e.target_id, dir, kHitReactSeconds,
                                  kHitReactKnockbackPx);
        }
    }
    log_cursor_ = log.size();
}

void BattleScene::narrate_new_logs() {
    // POLISH 3 (veredito do lider: log ENXUTO, estilo terminal/tatico): cada linha de dano
    // vira a forma CURTA "<atacante> -> <alvo> -<dano>" (ex.: "inimigo3 -> caua -5") em vez
    // da frase longa do motor ("inimigo3 ataca caua por 5.; caua ficou com Aceleracao") que
    // estourava a coluna estreita e era truncada por reticencias. A CATEGORIA (cor) continua
    // vindo de classify() (kind inalterado - os testes cravam kind, nao o texto). A message
    // CRUA do motor (dominio/POCO, testada) NAO e alterada: so a APRESENTACAO monta a string
    // curta a partir dos campos ESTRUTURADOS do evento (actor_id/target_id/value). O sufixo
    // verboso de status ("; X ficou com Y") foi REMOVIDO do log - o status ja aparece como
    // ICONE sob o ator (incremento 2), entao a repeticao textual e redundante e so gastava
    // largura. Linhas nao-dano (COMPILADO/Scan/Defender...) mantem a message do motor, que ja
    // e curta; o nowrap+ellipsis do RCSS segue como rede de seguranca.
    const auto& log = machine_->log();
    for (std::size_t i = narration_cursor_; i < log.size(); ++i) {
        const auto& e = log[i];
        LogLine line = classify(e);  // categoria (cor); texto reescrito abaixo se for dano
        // Linha de DANO (golpe com alvo): forma curta terminal "<atacante> -> <alvo> -<dano>".
        // Usa os IDS crus do evento (curtos e INEQUIVOCOS - 2 "Drone"/2 "Sentinela" do demo
        // colidiriam em display_name; o lider usou o id "inimigo3" no exemplo). Demais linhas
        // ficam com a message do motor que classify() ja copiou (system/status/info, ja curtas).
        if (line.kind == LogLineKind::Damage && e.target_id.has_value()) {
            line.text =
                e.actor_id + " -> " + *e.target_id + " -" + std::to_string(e.value);
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

    // ANIMACAO (W2): avanca os offsets/projeteis ANTES do pacing, pro contato do
    // jogador (abaixo) e o beat 2 do inimigo enxergarem o estado deste frame.
    anim_.update(dt_seconds);

    // CONTATO DO JOGADOR (battle-anim.md par.3.1): a aproximacao do [Atacar] chegou ao
    // alvo -> AGORA o motor resolve (dano + floater + hit-react nascem juntos) e o
    // ritmo entra no delay pos-acao (player_acted). O Return do atacante e disparado
    // dentro de resolve_one_turn (comum aos dois lados). A aproximacao FOI o anuncio.
    if (player_strike_pending_ && anim_.melee_arrived(player_strike_attacker_)) {
        player_strike_pending_ = false;
        resolve_one_turn();
        pacing_.player_acted();
        menu_.refresh(active_actor() != nullptr ? active_actor()->ap() : 0);
    }

    // DEMO DE CAST (diagnostico, dormante): o windup terminou -> spawna o projetil
    // placeholder do centro do caster ao centro do alvo (par.2.1: cast no lugar +
    // bolinha viaja). Cosmetico: zero motor.
    if (demo_cast_active_ && !anim_.is_casting(demo_cast_caster_)) {
        demo_cast_active_ = false;
        const std::optional<Rect> cr = arena_rect_for_actor(demo_cast_caster_);
        const std::optional<Rect> tr = arena_rect_for_actor(demo_cast_target_);
        if (cr.has_value() && tr.has_value()) {
            anim_.spawn_projectile(demo_cast_target_, cr->x + cr->w * 0.5f,
                                   cr->y + cr->h * 0.5f, tr->x + tr->w * 0.5f,
                                   tr->y + tr->h * 0.5f, kProjectileTravelSeconds);
        }
    }

    // IMPACTOS de projetil (mecanismo dormante da magia): cada chegada dispara o
    // hit-react VISUAL do alvo (a resolucao de carta real vai plugar aqui quando o
    // COMPILAR ligar; o canal FALHA entao suprimira o hit-react, par.2.3).
    for (const std::string& id : anim_.take_impacts()) {
        const CombatActor* victim = actor_by_id(id);
        const float dir =
            (victim != nullptr && victim->is_player_side()) ? -1.0f : 1.0f;
        anim_.start_hit_react(id, dir, kHitReactSeconds, kHitReactKnockbackPx);
    }

    // RITMO (incremento 6): avanca o diretor pelo tempo; quando ele LIBERA um passo (a
    // intro/delay terminou), conduz UM turno (de inimigo) ou entra em espera-do-jogador.
    pacing_.tick(dt_seconds);
    if (pacing_.ready_to_step()) {
        advance_pacing();
    }

    // SPRITES (W3): dirige o relogio de clip de cada ator COM sprite set pela fase
    // corrente do director - POR ULTIMO, pro clip refletir o estado DESTE frame
    // (pos-contato/pos-pacing). O swing do attack_melee entra na CAUDA do Approach
    // (clip_for_kind; decisao documentada em battle_sprite_anim.hpp). A troca de clip
    // reseta o relogio; o render so LE (resolve_sprite_clip).
    for (const auto& entry : sprites_) {
        const std::string& id = entry.first;
        const BattleClipId clip = clip_for_kind(anim_.kind_for(id),
                                                anim_.phase_remaining_seconds(id),
                                                kMeleeSwingSeconds);
        sprite_anim_.tick(id, clip, dt_seconds);
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
        enemy_announced_ = false;  // limpa o beat de anuncio (proximo inimigo re-anuncia)
        if (pacing_.state() != PacingState::WaitingPlayerInput) {
            pacing_.begin_player_turn();
        }
        return;
    }

    // Vez do INIMIGO em 2 BEATS (o lider: "a tela aparece com o ataque ja feito"):
    if (!enemy_announced_) {
        // BEAT 1 ANUNCIO: mostra "Vez de <nome>" + highlight forte. NADA resolveu ainda
        // (HP intacto, sem floater, sem linha de acao no log). Pausa ~0.7s OU tecla.
        //
        // ANIMACAO (W2, battle-anim.md par.3.1): o gancho previsto aqui AGORA toca - a
        // APROXIMACAO do melee do inimigo preenche o anuncio (windup/telegraph, Pillar
        // 1: o jogador VE o golpe vir), com duracao = o proprio beat, entao o ator
        // CHEGA no alvo exatamente quando o Beat 2 resolve (o contato coincide com a
        // resolucao). O alvo e o MESMO da acao auto (first_alive_player; nada resolve
        // entre o anuncio e o beat 2, entao ele e estavel). Cosmetico: se o slot nao
        // estiver visivel, degrada sem animacao (o beat anuncia parado, como antes).
        enemy_announced_ = true;
        pacing_.begin_enemy_announce();
        const CombatActor* atk = active_actor();
        const CombatActor* tgt = first_alive_player();
        if (atk != nullptr && tgt != nullptr) {
            start_melee_toward(atk->id(), tgt->id(), kPacingAnnounceSeconds);
        }
        return;
    }
    // BEAT 2 RESOLUCAO: AGORA aplica a acao (dano + floater + queda de HP + log de
    // consequencia) e segura o delay pro jogador LER (D8/D11). Limpa o flag de anuncio
    // pro proximo turno de inimigo re-anunciar.
    enemy_announced_ = false;
    resolve_one_turn();
    pacing_.begin_enemy_step();
}

void BattleScene::skip() {
    // D8: encurta o delay/anuncio entre turnos. NAO pula a abertura (espera Encarar) nem
    // o turno do jogador (o PacingDirector ignora skip nesses estados).
    pacing_.skip();
}

void BattleScene::start_combat() {
    // ENCARAR (lider 2026-06-25): sai da abertura PARADA e libera o 1o turno. Os 2 beats
    // (anuncio -> resolucao) animam normalmente a partir do proximo update/advance_pacing.
    pacing_.begin_combat();
}

bool BattleScene::offers_auto_resolve() const {
    // Verbo "[Q] Resolver sem encarar" so e OFERECIDO quando TODOS os inimigos vivos sao
    // TRASH (nao-boss). No demo todos sao trash, entao aparece. Boss/elite (is_boss)
    // escondem o verbo (precisa encarar). So faz sentido na abertura.
    if (!is_intro()) {
        return false;
    }
    bool any_enemy = false;
    for (const CombatActor* a : machine_->queue().order()) {
        if (a != nullptr && !a->is_player_side() && a->is_alive()) {
            any_enemy = true;
            if (a->is_boss()) {
                return false;  // tem boss/elite -> precisa encarar
            }
        }
    }
    return any_enemy;
}

void BattleScene::request_auto_resolve() {
    // PLACEHOLDER (lider 2026-06-25): o auto-resolve real (motor headless + penalidade por
    // selo) vira num incremento SEPARADO, apos o design ser canonizado nos docs. So plante
    // o gancho do verbo aqui: sinaliza no log e NAO faz nada destrutivo (nem sai do Intro).
    if (!offers_auto_resolve()) {
        return;  // so quando oferecido (trash, na abertura)
    }
    ui_log_.push_back(LogLine{LogLineKind::System,
                              "[auto-resolve: a implementar]"});
}

std::string_view BattleScene::turn_banner_key() const noexcept {
    // D9/D10: a casca resolve via tr() (+ nome do ator ativo pro turno do jogador).
    if (combat_over()) {
        return "";
    }
    if (is_intro()) {
        return "COMBAT_BANNER_BATTLE";  // "BATALHA!"
    }
    if (choosing_actor_) {
        return "COMBAT_BANNER_CHOOSE_ACTOR";  // "ESCOLHA QUEM AGE" (§4.1, comando-livre 1B)
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
    // ESCOLHA DE ATOR (§4.1, comando-livre 1B): enquanto o picker esta aberto, o begin_turn
    // fica DEFERIDO - so o actor_picker_confirm inicia o turno (via begin_active_turn_now).
    if (choosing_actor_) {
        return;
    }
    // E a vez do BLOCO da party com >1 elegivel? Entao NAO inicia o turno ainda: entra no modo
    // de escolha e espera o jogador comandar QUAL membro age (a SPD so SUGERE o pre-selecionado,
    // §4.1). O begin_turn vem depois, no confirm. Com <=1 elegivel (ou vez de inimigo/intro), o
    // fluxo segue direto (begin_active_turn_now) => identico ao motor pre-picker.
    if (should_offer_actor_picker()) {
        enter_actor_picker();
        return;
    }
    begin_active_turn_now();
}

void BattleScene::begin_active_turn_now() {
    // O "begin" real do turno ativo (parte extraida de start_active_turn). Chamado por
    // start_active_turn (quando NAO ha picker) e pelo actor_picker_confirm (apos gravar a
    // escolha). O confirm NAO reentra por start_active_turn de proposito: aquele re-checaria
    // should_offer_actor_picker (que ainda veria pending>1) e re-abriria o modo em loop.
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
    //
    // ANIMACAO (W2): captura o id do atacante ANTES de resolver/avancar - se ele estava
    // em aproximacao melee (jogador OU inimigo), o CONTATO e agora, e a VOLTA ao repouso
    // (Return) toca dentro do delay pos-resolucao (par.3.1, recovery no Beat 2).
    const CombatActor* atk_before = active_actor();
    const std::string atk_id = atk_before != nullptr ? atk_before->id() : std::string{};
    if (machine_->phase() == gus::domain::combat::CombatPhase::ActionSelect) {
        machine_->run_active_turn_to_end();
    }
    // Floater (D11) + narracao com consequencia (D12) ANTES do advance: o alvo ainda
    // esta vivo/visivel na arena pra ancorar o numero.
    spawn_floaters_from_new_logs();
    narrate_new_logs();
    // Recovery do melee (no-op se o atacante nao estava em aproximacao/hold - Defender/
    // Scan/Flee nao animam; robusto a skip: parte do offset ATUAL, termina no repouso).
    // A VOLTA e por LADO: o jogador usa kPlayerMeleeReturnSeconds (alongada, proporcional
    // ao approach dele; cabe no delay 0.8s); o inimigo mantem kMeleeReturnSeconds (0.4s,
    // ritmo aprovado no W1, INTOCADO). atk_before define o lado (o atacante deste turno).
    if (!atk_id.empty() && anim_.melee_in_flight(atk_id)) {
        const float return_seconds =
            (atk_before != nullptr && atk_before->is_player_side())
                ? kPlayerMeleeReturnSeconds
                : kMeleeReturnSeconds;
        anim_.begin_melee_return(atk_id, return_seconds);
    }
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
    if (choosing_actor_) {
        return;  // escolha de ator (§4.1): o menu de verbos ainda nao existe (begin deferido)
    }
    if (player_strike_pending_) {
        return;  // golpe em VOO (W2): o turno ja foi comandado; menu inerte ate resolver
    }
    menu_.move(delta);
}

void BattleScene::menu_confirm() {
    if (combat_over() || !current_actor_is_player()) {
        return;
    }
    if (player_strike_pending_) {
        return;  // golpe em VOO (W2): nao aceita re-confirmar durante o windup
    }
    if (choosing_actor_) {
        return;  // escolha de ator (§4.1): confirme o ATOR primeiro (actor_picker_confirm);
                 // o turno (e o menu de verbos) so comeca depois. Guarda o begin deferido.
    }
    if (aiming_) {
        return;  // ja em modo-mira: o jogador usa aim_confirm/aim_cancel, nao o menu
    }
    const BattleVerb verb = menu_.selected_verb();
    if (!menu_.is_enabled(verb)) {
        return;  // verbo sem AP: confirm e no-op (o item fica visivel acinzentado)
    }

    // MODO-MIRA (§3.5, D3): [Atacar]/[Scan] NAO resolvem na hora (fim do hardcode
    // first_alive_enemy). Entram em modo-mira; o jogador navega e confirma o ALVO (o
    // motor ja resolve qualquer target_id). aim_confirm() monta a acao com o alvo escolhido.
    if (verb == BattleVerb::Atacar || verb == BattleVerb::Scan) {
        enter_aim_mode(verb);  // no-op se nao ha inimigo vivo (vitoria iminente)
        return;
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

    // Monta a CombatAction real dos verbos SEM mira (self pro Defender; sem alvo pro Flee;
    // Gambito ainda placeholder). Atacar/Scan sairam do switch: agora passam pelo modo-mira
    // (acima) e resolvem no aim_confirm. combat.md par.5 (custos de AP vem das factories).
    CombatAction action = CombatAction::pass();
    switch (verb) {
        case BattleVerb::Atacar:
        case BattleVerb::Scan:
            return;  // tratados via modo-mira (enter_aim_mode acima); nunca caem aqui
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

// ---- Modo-mira / target selection (§3.5, D3) -------------------------------------
// APRESENTACAO PURA: navega/confirma o ALVO; o motor (resolve_primary_target) ja aceita
// qualquer target_id. Substitui o hardcode first_alive_enemy de Atacar/Scan. So teclado.

void BattleScene::rebuild_aim_candidates() {
    // Inimigos VIVOS em ordem de fila (frente->tras): o [0] == first_alive_enemy (o mais a
    // frente = age antes, fallback D3 (b)). Exclui MORTOS (a mira nunca pousa num morto).
    aim_candidates_.clear();
    for (const CombatActor* a : machine_->queue().order()) {
        if (a != nullptr && !a->is_player_side() && a->is_alive()) {
            aim_candidates_.push_back(a);
        }
    }
}

std::optional<CardFamily> BattleScene::action_family(BattleVerb verb) const {
    // FALLBACK documentado (§3.5): o motor NAO aplica a roda de fraqueza no ataque BASICO
    // (dano = atk - def, resolve_basic_attack) e o Scan e utilitario sem familia. Pra a
    // sugestao D3 (a) ser significativa e "premiar o Scan", ATACAR usa a familia-assinatura
    // do ATOR ATIVO como "familia da acao". SCAN nao tem familia (nullopt): cai sempre no
    // fallback (b) (o front da fila = ameaca iminente; escanear um ja-escaneado nao ajuda).
    // COMPILAR (carta COM familia propria) entra num incremento futuro.
    if (verb == BattleVerb::Atacar) {
        const CombatActor* self = active_actor();
        if (self != nullptr) {
            return self->family();
        }
    }
    return std::nullopt;
}

int BattleScene::preselect_aim_index(BattleVerb verb) const {
    // D3 (a): havendo info de Scan, mira o 1o inimigo (na ordem de fila) ESCANEADO e FRACO
    // (multFraqueza 1.5) a familia da acao. Premia o Scan (Pillar 1: info habilita acao).
    const std::optional<CardFamily> fam = action_family(verb);
    if (fam.has_value()) {
        for (int i = 0; i < static_cast<int>(aim_candidates_.size()); ++i) {
            const CombatActor* e = aim_candidates_[static_cast<std::size_t>(i)];
            if (e->is_scanned() && !e->is_universal_compiler() &&
                gus::domain::combat::WeaknessWheel::multiplier(*fam, e->family()) ==
                    gus::domain::combat::combat_constants::kMultFraco) {
                return i;
            }
        }
    }
    // D3 (b) fallback: front da fila (== first_alive_enemy) = aim_candidates_[0].
    return 0;
}

bool BattleScene::enter_aim_mode(BattleVerb verb) {
    rebuild_aim_candidates();
    if (aim_candidates_.empty()) {
        return false;  // sem inimigo vivo: nao entra na mira (vitoria iminente)
    }
    aim_verb_ = verb;
    aiming_ = true;
    aim_index_ = preselect_aim_index(verb);
    return true;
}

void BattleScene::aim_move(int delta) noexcept {
    if (!aiming_ || aim_candidates_.empty()) {
        return;
    }
    const int n = static_cast<int>(aim_candidates_.size());
    aim_index_ = ((aim_index_ + delta) % n + n) % n;  // WRAP nos extremos
}

void BattleScene::aim_select(int index) noexcept {
    // MOUSE (Incremento A2): pousa a mira DIRETO no index (sem wrap). No-op fora da mira ou
    // com index invalido (nao mexe no cursor corrente).
    if (!aiming_ || index < 0 || index >= static_cast<int>(aim_candidates_.size())) {
        return;
    }
    aim_index_ = index;
}

int BattleScene::aim_index_at_arena(float world_x, float world_y) const {
    // Casa o ponto (mundo/logico) com o SLOT de cada inimigo MIRAVEL. aim_candidates_ segue a
    // ordem de fila dos inimigos vivos, a MESMA base que arena_rect_for_actor usa pra ordenar
    // os slots -> o i-esimo candidato casa o i-esimo slot. Fora da mira a lista e vazia (-1).
    for (int i = 0; i < static_cast<int>(aim_candidates_.size()); ++i) {
        const CombatActor* e = aim_candidates_[static_cast<std::size_t>(i)];
        if (e == nullptr) {
            continue;
        }
        const std::optional<gus::core::spatial::Rect> slot = arena_rect_for_actor(e->id());
        if (!slot.has_value()) {
            continue;  // alvo sem slot visivel (nao deveria, mira so tem vivos): pula
        }
        const gus::core::spatial::Rect& r = *slot;
        if (world_x >= r.x && world_x <= r.x + r.w && world_y >= r.y &&
            world_y <= r.y + r.h) {
            return i;
        }
    }
    return -1;
}

const CombatActor* BattleScene::aim_target() const noexcept {
    if (!aiming_ || aim_index_ < 0 ||
        aim_index_ >= static_cast<int>(aim_candidates_.size())) {
        return nullptr;
    }
    return aim_candidates_[static_cast<std::size_t>(aim_index_)];
}

void BattleScene::aim_cancel() noexcept {
    // Volta ao menu de verbos SEM consumir o turno: nada resolve, o pacing segue em
    // WaitingPlayerInput e o menu do ator ativo continua ativo.
    aiming_ = false;
    aim_index_ = 0;
    aim_candidates_.clear();
}

void BattleScene::aim_hotkey(int nth) {
    // TECLA-ATALHO 1-9 (mira): escolhe o nth-esimo inimigo miravel (1-based) e CONFIRMA na
    // hora. Espelha actor_picker_hotkey: no-op se fora do modo ou nth sem candidato (fora de
    // faixa) - "apertar 5 com so 4 miraveis" nao faz nada (nem mira, nem confirma). Fonte
    // unica do host (teclas) e dos testes.
    if (!aiming_) {
        return;
    }
    const int idx = nth - 1;  // 1-based -> 0-based
    if (idx < 0 || idx >= static_cast<int>(aim_candidates_.size())) {
        return;  // numero sem inimigo miravel: no-op
    }
    aim_index_ = idx;
    aim_confirm();
}

void BattleScene::aim_confirm() {
    if (!aiming_) {
        return;
    }
    const CombatActor* t = aim_target();
    if (t == nullptr) {
        aim_cancel();  // sem alvo valido: aborta a mira sem consumir turno
        return;
    }
    const bool is_scan = (aim_verb_ == BattleVerb::Scan);
    // Monta a acao com o ALVO ESCOLHIDO (nao mais o hardcode first_alive_enemy).
    CombatAction action = is_scan ? CombatAction::scan(t->id())
                                  : CombatAction::attack(t->id());
    const std::string target_id = t->id();
    // Sai da mira ANTES de resolver (o turno avanca; o proximo turno reabre o menu).
    aiming_ = false;
    aim_index_ = 0;
    aim_candidates_.clear();
    pending_action_ = std::move(action);

    // [ATACAR] = golpe MELEE (W2, battle-anim.md par.2.2/3.1): a confirmacao inicia o
    // WINDUP na hora (regra de ouro < 100ms: o sprite ja parte no proximo frame) e a
    // RESOLUCAO fica DEFERIDA ate o CONTATO (fim da aproximacao; a acao espera no
    // mailbox) - a aproximacao E o proprio anuncio do turno do jogador. O ritmo
    // continua em WaitingPlayerInput durante o voo (o menu fica inerte pelos guards);
    // o update(dt) resolve no contato e entra no delay (player_acted). A duracao usa
    // kPlayerMeleeApproachSeconds (DESACOPLADA do Beat 1 do inimigo, que segue em
    // kPacingAnnounceSeconds cru): o approach do jogador NAO tem beat de pacing atrelado
    // (o contato e gated por anim_.melee_arrived, nao por timer), entao dura o que a
    // LEITURA pede - o playtest (lider 2026-07-02) pediu mais tempo pra VER a corrida.
    // Se o slot nao estiver visivel (nunca no fluxo normal), degrada: resolve na hora.
    if (!is_scan) {
        const CombatActor* self = active_actor();
        if (self != nullptr &&
            start_melee_toward(self->id(), target_id, kPlayerMeleeApproachSeconds)) {
            player_strike_pending_ = true;
            player_strike_attacker_ = self->id();
            return;  // contato (e resolucao) vem no update(dt)
        }
    }
    // Scan (utilitario, nao e golpe) e o fallback degradado resolvem NA HORA (mesmo
    // fluxo do menu_confirm antigo): o update(dt) anima os inimigos um a um depois.
    resolve_one_turn();
    pacing_.player_acted();
    menu_.refresh(active_actor() != nullptr ? active_actor()->ap() : 0);
}

// ---- Escolha de ator / Janela de Comando da Party (§4.1, comando-livre 1B) ----------
// APRESENTACAO PURA sobre o motor 1B: a cena so deixa o jogador ESCOLHER qual membro
// pendente da party age; o motor (pending_party_actors/select_party_actor) faz o resto
// (agrupar por lado, recomputar quem abre, regroup Gambito-safe). O begin_turn e DEFERIDO
// ate o confirm (start_active_turn e no-op enquanto choosing_actor_).

bool BattleScene::should_offer_actor_picker() const {
    // Gate do modo: e a vez do bloco da party, o combate esta rolando (nao intro/nao fim) e ha
    // MAIS DE UM elegivel. active_actor() == queue_.current(); apos o regroup por lado (§4.1) o
    // bloco da party e contiguo, entao "current e player-side vivo" == "abriu o bloco da party".
    //
    // >1 (DECISAO documentada): com 1 so elegivel NAO ha escolha a fazer - a cena auto-inicia o
    // turno (sem friccao). O picker existe pra dar AGENCIA sobre QUEM age; com um unico pendente
    // nao ha agencia a exercer (distinto da mira, cujo confirm COMPROMETE um ataque com previa
    // de dano). Isto reforca o modelo mental "escolho quando ha varios". Trocar pra ">= 1"
    // (mostrar sempre, como a mira com 1 alvo) e UMA LINHA, se o lider preferir consistencia.
    if (combat_over() || is_intro()) {
        return false;
    }
    const CombatActor* a = active_actor();
    if (a == nullptr || !a->is_player_side() || !a->is_alive()) {
        return false;
    }
    return machine_->pending_party_actors().size() > 1;
}

void BattleScene::enter_actor_picker() {
    // Snapshot dos elegiveis (maior SPD -> menor; front = pre-selecionado). Cursor no front.
    // NAO chama begin_turn: o turno so comeca no confirm (begin_active_turn_now).
    actor_choices_ = machine_->pending_party_actors();
    choosing_actor_ = true;
    actor_pick_index_ = 0;  // front de pending_party_actors == preselected_party_actor (SPD)
}

std::vector<const CombatActor*> BattleScene::actor_choices() const {
    return std::vector<const CombatActor*>(actor_choices_.begin(), actor_choices_.end());
}

const CombatActor* BattleScene::actor_pick_target() const noexcept {
    if (!choosing_actor_ || actor_pick_index_ < 0 ||
        actor_pick_index_ >= static_cast<int>(actor_choices_.size())) {
        return nullptr;
    }
    return actor_choices_[static_cast<std::size_t>(actor_pick_index_)];
}

void BattleScene::actor_picker_move(int delta) noexcept {
    if (!choosing_actor_ || actor_choices_.empty()) {
        return;
    }
    const int n = static_cast<int>(actor_choices_.size());
    actor_pick_index_ = ((actor_pick_index_ + delta) % n + n) % n;  // WRAP nos extremos
}

void BattleScene::actor_picker_select(int index) noexcept {
    // MOUSE/atalho: poe o cursor DIRETO no index (sem wrap). No-op fora do modo ou invalido.
    if (!choosing_actor_ || index < 0 ||
        index >= static_cast<int>(actor_choices_.size())) {
        return;
    }
    actor_pick_index_ = index;
}

int BattleScene::actor_pick_index_at_arena(float world_x, float world_y) const {
    // Casa o ponto (mundo/logico) com o SLOT de cada elegivel. arena_rect_for_actor e generico
    // por id (trata party E inimigo), entao vale pros membros da party (confirmado: ele separa
    // os lados e devolve o slot certo). Fora do modo a lista e vazia (-1). Mesma logica de
    // aim_index_at_arena, sobre actor_choices_ (party) -> reusa o pipeline de hit-test do A2.
    for (int i = 0; i < static_cast<int>(actor_choices_.size()); ++i) {
        const CombatActor* p = actor_choices_[static_cast<std::size_t>(i)];
        if (p == nullptr) {
            continue;
        }
        const std::optional<gus::core::spatial::Rect> slot = arena_rect_for_actor(p->id());
        if (!slot.has_value()) {
            continue;
        }
        const gus::core::spatial::Rect& r = *slot;
        if (world_x >= r.x && world_x <= r.x + r.w && world_y >= r.y &&
            world_y <= r.y + r.h) {
            return i;
        }
    }
    return -1;
}

void BattleScene::actor_picker_confirm() {
    if (!choosing_actor_) {
        return;
    }
    if (actor_pick_index_ < 0 ||
        actor_pick_index_ >= static_cast<int>(actor_choices_.size())) {
        return;  // defensivo (cursor invalido): nao confirma
    }
    CombatActor* chosen = actor_choices_[static_cast<std::size_t>(actor_pick_index_)];
    // Grava a escolha no motor (comando livre 1B, §4.1). machine_-> = acesso NAO-const (mesmo
    // padrao de menu_confirm/aim_confirm; o machine() publico e const). select_party_actor
    // lanca std::invalid_argument se `chosen` nao e elegivel; como actor_choices_ veio de
    // pending_party_actors e nada mutou o motor no meio, ele SEMPRE e elegivel.
    machine_->select_party_actor(chosen);
    choosing_actor_ = false;
    actor_choices_.clear();
    actor_pick_index_ = 0;
    // begin_turn consome a escolha (traz o ator ao cursor) e prossegue como hoje: menu de
    // verbos do ATOR ESCOLHIDO. NAO reentra por start_active_turn (evita re-abrir o picker).
    begin_active_turn_now();
}

void BattleScene::actor_picker_hotkey(int nth) {
    // TECLA-ATALHO 1/2/3: escolhe o nth-esimo elegivel (1-based) e CONFIRMA na hora. No-op se
    // fora do modo ou nth sem elegivel (fora de faixa) - assim "apertar 3 com so 2 pendentes"
    // nao faz nada (nem seleciona, nem confirma). Fonte unica do host (teclas) e dos testes.
    if (!choosing_actor_) {
        return;
    }
    const int idx = nth - 1;  // 1-based -> 0-based
    if (idx < 0 || idx >= static_cast<int>(actor_choices_.size())) {
        return;  // numero sem elegivel: no-op
    }
    actor_pick_index_ = idx;
    actor_picker_confirm();
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

std::vector<const CombatActor*> BattleScene::ctb_window() const {
    const std::vector<CombatActor*>& order = machine_->queue().order();
    const int n = static_cast<int>(order.size());
    std::vector<const CombatActor*> out;
    if (n == 0) {
        return out;  // fila vazia (nunca em pratica): janela vazia, sem UB no cursor
    }
    // Indice do ator ATIVO dentro da fila. active_actor() == queue().current() ==
    // order[cursor_] do dominio; achamos esse cursor por IDENTIDADE de ponteiro (os atores
    // sao unicos). Assim a rotacao vive INTEIRA na camada app/ (consumidor), SEM expor o
    // cursor do dominio nem tocar a invariante das 4 camadas (domain/ POCO intocado).
    const CombatActor* active = active_actor();
    int start = 0;
    for (int i = 0; i < n; ++i) {
        if (order[static_cast<std::size_t>(i)] == active) {
            start = i;
            break;  // fallback graceful (start=0) se active nao esta na fila
        }
    }
    // Janela ROTACIONADA: ate kCtbVisibleCells atores consecutivos (mod n) a partir do
    // ativo. O teto min(n, kCtbVisibleCells) garante que NENHUM ator se repete quando a fila
    // e curta (<=5): damos no maximo uma volta PARCIAL, nunca reincluindo o slot 0.
    const int count = std::min(n, kCtbVisibleCells);
    out.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        out.push_back(order[static_cast<std::size_t>((start + i) % n)]);
    }
    return out;
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
    // Camera 1:1 no retangulo logico 960x540 (D1). begin_frame recebe os PIXELS REAIS
    // pro backend escalar por inteiro. Cada Rect de layout (px logico) == Rect de mundo.
    const Rect screen = battle_screen_rect();
    renderer.begin_frame(screen, static_cast<int>(viewport_px_w),
                         static_cast<int>(viewport_px_h));

    // --- fundo (D7: camera estatica) ---
    // O FUNDO da arena agora vem da VINHETA/glow radial do renderer (Render2dGl3 desenha
    // um quad full-window LOGO APOS o clear, dando profundidade: bordas escuras + leve
    // lift no centro). NAO repintamos um fill chapado #0c0f1a aqui senao ele cobriria a
    // vinheta. (kBgColor mantido como referencia da cor-base canonica.)
    (void)kBgColor;

    // ====================================================================
    // VARIANTE C "Tatico Cockpit": cockpit lateral esq + arena a direita +
    // CTB topo + banner em faixa propria + terminal/log no rodape.
    // Ordem de render: fundo -> COCKPIT (opaco) -> CTB -> banner -> atores
    // -> log (opaco) -> floaters (por cima). O cockpit/log opacos garantem
    // que nenhum ator invada (e a arena ja vive a DIREITA do cockpit).
    // ====================================================================

    const CombatActor* active = active_actor();

    // --- COCKPIT lateral esquerdo (painel do ator ativo + verbos) ---
    // So mostra os DADOS do ator quando NAO e abertura (na abertura o ativo e o 1o da
    // fila por SPD = inimigo; o cockpit fica vazio/dim). A faixa do cockpit e sempre
    // desenhada (opaca) pra demarcar a coluna.
    // ADR-009: quando o HUD e EXTERNO (RmlUi-GL3), o cockpit inteiro (painel/retrato/
    // pips/menu) NAO e desenhado a mao - o RmlUi o desenha por cima. Evita 2 cockpits.
    if (!hud_external_) {
        const Rect cp = cockpit_rect();
        renderer.draw_filled_rect(cp, kPanelColor);
        renderer.draw_rect_outline(
            Rect{cp.x + cp.w - 1.0f, cp.y, 1.0f, cp.h}, kHudBorderColor, 1.0f);

        if (!is_intro() && active != nullptr) {
            const CombatActor* a = active;
            // Retrato grande do ator ativo (cyan border via highlight). Sem retrato => box.
            const Rect pr = cockpit_portrait_rect();
            const TextureId ptex = portraits_.find(a->id());
            if (ptex != kInvalidTexture) {
                renderer.draw_textured_rect(pr, ptex, UvRect{}, kWhite);
            } else {
                renderer.draw_filled_rect(pr, kSlotDarkColor);
            }
            renderer.draw_rect_outline(pr, kCyan, 1.0f);

            // Nome do ator (cyan), centrado sob o retrato.
            const std::string who = a->display_name();
            const float nw = gus::platform::render2d::text_width(who, kCockpitNamePx);
            renderer.draw_text(who.c_str(), pr.x + (pr.w - nw) * 0.5f,
                               pr.y + pr.h + 4.0f, kCockpitNamePx, kCyan, /*bold=*/true);

            // HP: barra (cockpit largura) + "hp/max" por cima, centrado.
            const Rect hp = cockpit_hp_bar_rect();
            draw_bar(renderer, hp, bar_fill(a->hp(), a->max_hp()), kHpFillColor);
            char num[40];
            std::snprintf(num, sizeof(num), "%d/%d", a->hp(), a->max_hp());
            const float hw = gus::platform::render2d::text_width(num, kCockpitTextPx);
            renderer.draw_text(num, hp.x + (hp.w - hw) * 0.5f,
                               hp.y + (hp.h - kCockpitTextPx) * 0.5f, kCockpitTextPx,
                               kInk, /*bold=*/false);

            // AP pips (latao) + rotulo "AP".
            const Vec2 apo = cockpit_ap_pips_origin();
            renderer.draw_text("AP", apo.x, apo.y - kCockpitLabelPx - 1.0f,
                               kCockpitLabelPx, kInkDim, /*bold=*/false);
            draw_pips(renderer, apo.x + kCockpitResLabelW, apo.y, a->max_ap(), a->ap(),
                      gus::domain::combat::combat_constants::kBaseApPerTurn,
                      kApLitColor, kApOffColor);
            // Mana pips (cyan) + rotulo "MANA".
            const Vec2 mno = cockpit_mana_pips_origin();
            renderer.draw_text("MANA", mno.x, mno.y - kCockpitLabelPx - 1.0f,
                               kCockpitLabelPx, kInkDim, /*bold=*/false);
            draw_pips(renderer, mno.x + kCockpitResLabelW, mno.y, a->max_mana(),
                      a->mana(), gus::domain::combat::combat_constants::kManaCap,
                      kManaLitColor, kManaOffColor);

            // MENU de verbos EMPILHADO (so no turno de jogador vivo). Suprimido durante a
            // ESCOLHA DE ATOR (§4.1): o begin_turn esta deferido, entao nao ha turno cujo
            // menu mostrar - o jogador escolhe QUEM age primeiro (destaque na arena, abaixo).
            // Suprimido tambem com o GOLPE EM VOO (W2): o turno ja foi comandado; o menu
            // some enquanto o sprite desloca/golpeia (re-aparece no proximo turno).
            if (current_actor_is_player() && !combat_over() && !choosing_actor_ &&
                !player_strike_pending_) {
                const Rect mz = cockpit_menu_zone();
                renderer.draw_text("ACAO", mz.x, mz.y - kCockpitLabelPx - 2.0f,
                                   kCockpitLabelPx, kInkDim, /*bold=*/false);
                const auto items = menu_.layout(mz);
                for (int i = 0; i < kBattleVerbCount; ++i) {
                    const MenuItem& it = items[static_cast<std::size_t>(i)];
                    const bool sel = (i == menu_.selected_index());
                    // Caixa do verbo (fundo escuro) + borda na cor do verbo.
                    renderer.draw_filled_rect(
                        it.rect, it.enabled ? kVerbBoxColor : kVerbDisabledColor);
                    const DrawColor vc =
                        it.enabled ? verb_color(it.verb, sel) : kInkDim;
                    renderer.draw_rect_outline(it.rect, vc, sel ? 2.0f : 1.0f);
                    // Nome do verbo (na cor do verbo), centrado vertical, margem esq.
                    const std::string label = tr_verb_label(it.verb);
                    const float ty = it.rect.y + (it.rect.h - kVerbTextPx) * 0.5f;
                    renderer.draw_text(label.c_str(), it.rect.x + 6.0f, ty,
                                       kVerbTextPx, vc, /*bold=*/sel);
                }
            }
        }
    }

    // --- fila CTB (topo, a direita do cockpit; D4) ---
    // SEM FAIXA/CAIXA (lider 2026-06-30): a banda de fundo (kCtbBandColor = kBg2Color)
    // foi REMOVIDA. Ela pintava um retangulo de tom distinto do fundo da arena (que agora
    // e a VINHETA radial, nao mais o flat kBgColor), saltando como uma "caixa" de borda
    // dura atras dos retratos. Agora as celulas/retratos da fila ficam DIRETO sobre a
    // vinheta - seamless, igual a party e a coluna de inimigos (que nao tem faixa). As
    // CELULAS por-retrato (kCtbCellColor), a marca de ativo/proximo (cyan) e o banner
    // seguem intactos. kCtbBandColor deixa de ser usado aqui.
    {
        const CtbStrip strip = ctb_strip(queue_len());
        // JANELA ROTACIONADA (fix da fila CTB): os atores a mostrar vem de ctb_window(), que
        // COMECA no ator ATIVO e segue a ordem de jogo com wrap - NAO mais os top-5 fixos por
        // SPD (bug: atores de SPD baixa, ex. Jaci, nunca apareciam, e o destaque "ativo"
        // ficava preso no slot de maior SPD). window[0] = ativo; window[1] = proximo; etc.
        const std::vector<const CombatActor*> window = ctb_window();
        const CombatActor* active = active_actor();
        for (int i = 0; i < kCtbVisibleCells; ++i) {
            const CtbCell& cell = strip.cells[static_cast<std::size_t>(i)];
            if (!cell.occupied) {
                continue;
            }
            // O ator desta celula vem da janela rotacionada (a fila do motor ja exclui
            // mortos/incapacitados, que o motor remove em advance/prune: a fila CTB reflete
            // so quem ainda joga).
            const CombatActor* who = (i < static_cast<int>(window.size()))
                                         ? window[static_cast<std::size_t>(i)]
                                         : nullptr;

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
            // Banner na FAIXA PROPRIA do topo da arena (a direita do cockpit), centrado
            // NESSE espaco - nao sobre os atores. Na abertura, sobe pro centro da arena.
            const Rect bz = arena_banner_rect();
            const float center_x = bz.x + bz.w * 0.5f;
            const float by = is_intro() ? (static_cast<float>(kArenaTop) + 40.0f)
                                        : (bz.y + (bz.h - kBannerTextPx) * 0.5f);
            const float bigPx = is_intro() ? kBannerTextPx + 6.0f : kBannerTextPx;
            const float tw = gus::platform::render2d::text_width(text, bigPx);
            renderer.draw_text(text.c_str(), center_x - tw * 0.5f, by, bigPx, col,
                               /*bold=*/true);

            // ABERTURA: prompt de input abaixo do "BATALHA!" (lider). A luta so comeca
            // quando o jogador ENCARA. "[Enter] Encarar" sempre; "[Q] Resolver sem
            // encarar" so se oferecido (TRASH). Centrado na faixa da arena.
            if (is_intro()) {
                float py = by + bigPx + 12.0f;
                const auto draw_prompt = [&](const char* tr_key, DrawColor pc) {
                    const std::string t = translator_->tr(tr_key);
                    const float pw =
                        gus::platform::render2d::text_width(t, kPanelTextPx);
                    renderer.draw_text(t.c_str(), center_x - pw * 0.5f, py,
                                       kPanelTextPx, pc, /*bold=*/false);
                    py += kPanelTextPx + 6.0f;
                };
                draw_prompt("COMBAT_INTRO_ENCARAR", kCyan);  // Encarar = cyan (primary)
                if (offers_auto_resolve()) {
                    draw_prompt("COMBAT_INTRO_AUTORESOLVE", kInkDim);
                }
            }
        }
    }

    // --- arena: party (esq-da-arena) e inimigos (dir), distribuidos space-around ---
    {
        const ArenaLayout arena =
            arena_layout(party_count(), enemy_count(), gus_party_index());

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
        // Ator na arena = RETRATO (placeholder; sprite de corpo depois). Moldura na cor
        // do lado (cyan party / magenta inimigo), highlight cyan no ativo (D7/D9),
        // mini-barra de HP (verde party / magenta inimigo via o body color), nome curto.
        auto draw_side = [&](int count, const auto& slots,
                             const std::vector<const CombatActor*>& alive,
                             const DrawColor& side, const DrawColor& hp_fill,
                             bool is_party) {
            for (int i = 0; i < count; ++i) {
                const Rect& r = slots[static_cast<std::size_t>(i)].rect;
                if (i >= static_cast<int>(alive.size())) {
                    continue;
                }
                const CombatActor* a = alive[static_cast<std::size_t>(i)];
                // ANIMACAO (W2, battle-anim.md par.3.2): o ATOR (retrato-placeholder;
                // sprite depois) desenha na posicao-base do slot + o OFFSET de animacao
                // (melee/hit-react/cast). Os elementos de HUD ancorados no ator (barra
                // de HP, status, intent, mira, picker) FICAM no slot-base: o sprite se
                // desloca, a leitura tatica nao dança junto (camera estatica, D7).
                const Vec2 aoff = anim_.offset_for(a->id());
                const Rect ra{r.x + aoff.x, r.y + aoff.y, r.w, r.h};
                const bool act = (active != nullptr && a == active);
                // SPRITE ANIMADO (W3, battle-anim.md par.1.1/3.2): ator COM sprite set
                // desenha o FRAME corrente da animacao da fase (idle/run/golpe/hurt),
                // ESCALADO (frame 256x256 quadrado -> quad de kActorSpriteScale x o
                // slot, proporcao mantida) e ANCORADO PELO PE na BASE do slot ("em pe
                // no chao", nao centrado). O pe REAL vem do alpha-bbox do frame 0 do
                // Idle (referencia ESTAVEL - ancorar pelo idle nao faz o tronco pular
                // entre frames; mesma disciplina do overworld/sprite_anchor). Headless/
                // Null (bbox invalido) degrada pro anchor legado (margem 0). O offset
                // de animacao (aoff) desloca o sprite; o HUD ancorado (HP/status/
                // intent/mira) SEGUE no slot-base, como no placeholder.
                bool drew_sprite = false;
                {
                    BattleClipId sclip = BattleClipId::Idle;
                    float selapsed = 0.0f;
                    const SpriteClip* clip =
                        resolve_sprite_clip(a->id(), &sclip, &selapsed);
                    if (clip != nullptr) {
                        const int fi = clip_frame_index(
                            static_cast<int>(clip->frames.size()), clip->fps,
                            clip->loop, selapsed);
                        const TextureId ftex =
                            clip->frames[static_cast<std::size_t>(fi)];
                        if (ftex != kInvalidTexture) {
                            const float quad = static_cast<float>(kActorSlotH) *
                                               kActorSpriteScale;
                            // Pe real: margem inferior transparente do Idle f0.
                            float foot = 0.0f;
                            const auto sit = sprites_.find(a->id());
                            const SpriteClip* idle =
                                sit != sprites_.end()
                                    ? sit->second.find(BattleClipId::Idle)
                                    : nullptr;
                            const TextureId anchor_tex =
                                idle != nullptr ? idle->frames.front() : ftex;
                            const auto bbox =
                                renderer.texture_content_bbox(anchor_tex);
                            if (bbox.valid()) {
                                foot = bottom_margin_fraction(bbox.bottom_margin(),
                                                              bbox.canvas_h);
                            }
                            const float top_y =
                                sprite_top_y(r.y + r.h, quad, foot,
                                             /*manual_offset_world=*/0.0f);
                            const Rect quad_rect{r.x + (r.w - quad) * 0.5f + aoff.x,
                                                 top_y + aoff.y, quad, quad};
                            renderer.draw_textured_rect(quad_rect, ftex, UvRect{},
                                                        kWhite);
                            // Highlight do ATIVO (D7) segue multimodal: contorno cyan
                            // no quad do sprite. A moldura fina de "lado" do
                            // placeholder NAO se aplica (a silhueta do sprite ja
                            // marca o ator) - decisao W3, lider valida ao vivo.
                            if (act) {
                                renderer.draw_rect_outline(quad_rect, kCyan, 2.0f);
                            }
                            drew_sprite = true;
                        }
                    }
                }
                if (!drew_sprite) {
                    // Retrato (ou slot escuro de fallback) - placeholder de hoje.
                    const TextureId tex = portraits_.find(a->id());
                    if (tex != kInvalidTexture) {
                        renderer.draw_textured_rect(ra, tex, UvRect{}, kWhite);
                    } else {
                        renderer.draw_filled_rect(ra, kSlotDarkColor);
                    }
                    // Moldura: ativo = cyan grossa; senao a cor do lado, fina.
                    renderer.draw_rect_outline(ra, act ? kCyan : side,
                                               act ? 2.0f : 1.0f);
                }
                // Mini-barra de HP sob o ator.
                const Rect hpbar = arena_hp_bar_frame(r);
                draw_bar(renderer, hpbar, bar_fill(a->hp(), a->max_hp()), hp_fill);
                // STATUSROW: icones de status do ator (sob a barra de HP), como no mock.
                {
                    float sx = r.x;
                    const float sy = hpbar.y + hpbar.h + 2.0f;
                    for (const auto& st : a->status_effects()) {
                        const Rect sbox{sx, sy, static_cast<float>(kStatusIconSize),
                                        static_cast<float>(kStatusIconSize)};
                        const TextureId stex = status_icons_.find(st.id);
                        if (stex != kInvalidTexture) {
                            renderer.draw_textured_rect(sbox, stex, UvRect{}, kWhite);
                        } else {
                            renderer.draw_filled_rect(sbox, kStatusBoxColor);
                        }
                        sx += static_cast<float>(kStatusIconSize + kStatusIconGap);
                        if (sx > r.x + r.w) {
                            break;  // nao vaza a largura do slot
                        }
                    }
                }
                // INTENT sobre cada INIMIGO vivo (telegraph). Sem icone => marca magenta.
                if (!is_party) {
                    const auto intent = intent_for(*a);
                    if (intent.has_value()) {
                        const Rect ibox{r.x + (r.w - kIntentIconSize) * 0.5f,
                                        r.y - kIntentIconSize - 2.0f,
                                        kIntentIconSize, kIntentIconSize};
                        const TextureId itex =
                            intent_icon_tex(intent_icons_, *intent);
                        if (itex != kInvalidTexture) {
                            renderer.draw_textured_rect(ibox, itex, UvRect{}, kWhite);
                        } else {
                            renderer.draw_filled_rect(ibox, kMagentaDim);
                            renderer.draw_rect_outline(ibox, kMagenta, 1.0f);
                        }
                    }
                }

                // MIRA (§3.5, D3): destaque MULTIMODAL do inimigo mirado (contorno + seta
                // + nome/HP), NUNCA so cor (Pillar 4/WCAG). Reusa as primitivas de outline/
                // rect/text (mesma tecnica do highlight do ativo). So o alvo mirado.
                if (!is_party && aiming_ && a == aim_target()) {
                    // (1) CONTORNO: reticulo = outline DUPLO (externo grosso + interno
                    //     fino), distinto do outline simples do ator ativo.
                    renderer.draw_rect_outline(r, kMiraColor, 3.0f);
                    const Rect inner{r.x + 3.0f, r.y + 3.0f, r.w - 6.0f, r.h - 6.0f};
                    renderer.draw_rect_outline(inner, kMiraColor, 1.0f);
                    // (2) SETA: caret >> (forma PURA por rects, nao so cor), a ESQUERDA do
                    //     slot, centrado na vertical - nao colide com o intent (acima) nem
                    //     a barra de HP (abaixo). 3 colunas de altura decrescente = aponta
                    //     pro slot.
                    const float acy = r.y + r.h * 0.5f;
                    for (int c = 0; c < 3; ++c) {
                        const float ch = 12.0f - static_cast<float>(c) * 4.0f;  // 12,8,4
                        const Rect bar{r.x - 12.0f + static_cast<float>(c) * kMiraCaretW,
                                       acy - ch * 0.5f, kMiraCaretW, ch};
                        renderer.draw_filled_rect(bar, kMiraColor);
                    }
                    // (3) NOME + HP do alvo em TEXTO legivel (WCAG): nome acima do slot
                    //     (sobre o intent), "hp/max" a direita. Usa display_name direto (sem
                    //     translator), como o cockpit faz com o ator ativo.
                    const std::string nm = a->display_name();
                    renderer.draw_text(nm.c_str(), r.x,
                                       r.y - kIntentIconSize - kMiraLabelPx - 3.0f,
                                       kMiraLabelPx, kMiraColor, /*bold=*/true);
                    char hpn[40];
                    std::snprintf(hpn, sizeof(hpn), "%d/%d", a->hp(), a->max_hp());
                    renderer.draw_text(hpn, r.x + r.w + 2.0f,
                                       r.y + (r.h - kMiraLabelPx) * 0.5f, kMiraLabelPx,
                                       kMiraColor, /*bold=*/false);
                    // (4) TIER DE FRAQUEZA (§3.5 "se ja escaneado, o tier vs a acao"): alvo
                    //     ESCANEADO e FRACO a familia da acao ganha um pip LATAO (paleta:
                    //     "latao = fraqueza"). APRESENTACAO PURA: usa o fallback de familia
                    //     (action_family) - o ataque BASICO nao aplica a roda no motor.
                    if (a->is_scanned()) {
                        const std::optional<CardFamily> fam = action_family(aim_verb_);
                        if (fam.has_value() && !a->is_universal_compiler() &&
                            gus::domain::combat::WeaknessWheel::multiplier(
                                *fam, a->family()) ==
                                gus::domain::combat::combat_constants::kMultFraco) {
                            const Rect fp{r.x + r.w + 2.0f, r.y, kMiraFracoPipSize,
                                          kMiraFracoPipSize};
                            renderer.draw_filled_rect(fp, kBrass);
                        }
                    }
                    // (5) PREVIA DE DANO (feedback do lider no display): SO em [Atacar]
                    //     (Scan e utilitario, nao bate). Mostra a perda de HP PREVISTA no
                    //     alvo mirado ANTES de confirmar, atualizando AO VIVO por alvo.
                    //     Numero PURO do motor (preview_basic_attack_damage: dano bruto -
                    //     absorcao de Shield, piso 0) - a cena NUNCA recalcula regra, so LE.
                    //     "-N" = HP que sai (mesma convencao "-N" da narracao do log); cor =
                    //     a do numero de dano COMUM do floater (destaca do HP ciano). Pillar
                    //     4/WCAG: e NUMERO, nao so cor. Fecha o cluster de info logo ABAIXO
                    //     do "hp/max", no lado direito do slot.
                    if (aim_verb_ == BattleVerb::Atacar) {
                        const CombatActor* attacker = active_actor();
                        if (attacker != nullptr) {
                            const int dano =
                                machine().preview_basic_attack_damage(*attacker, *a);
                            char dmg[24];
                            std::snprintf(dmg, sizeof(dmg), "-%d", dano);
                            const float hp_y = r.y + (r.h - kMiraLabelPx) * 0.5f;
                            renderer.draw_text(
                                dmg, r.x + r.w + 2.0f, hp_y + kMiraLabelPx + 2.0f,
                                kMiraLabelPx, floater_color_for_channel(HitChannel::Common),
                                /*bold=*/true);
                        }
                    }
                }

                // ESCOLHA DE ATOR (§4.1, comando-livre 1B): destaque MULTIMODAL dos membros
                // ELEGIVEIS da party e do PRE-SELECIONADO/cursor. NUNCA so cor (Pillar 4/WCAG):
                // (a) TODO elegivel ganha um BADGE numerado (= a tecla-atalho 1/2/3) + contorno;
                // (b) o CURSOR ganha reticulo (outline duplo) + caret + NOME. So no lado da
                // party. Reusa a tecnica do realce da mira (kMiraColor/caret/nome), aqui em cyan
                // (cor da party) - sem conflito visual com a mira (inimigos, a direita).
                if (is_party && choosing_actor_) {
                    int choice_idx = -1;
                    for (int ci = 0; ci < static_cast<int>(actor_choices_.size()); ++ci) {
                        if (actor_choices_[static_cast<std::size_t>(ci)] == a) {
                            choice_idx = ci;
                            break;
                        }
                    }
                    if (choice_idx >= 0) {
                        const bool is_cursor = (a == actor_pick_target());
                        // (1) CONTORNO cyan. Elegivel = fino; cursor = reticulo (outline duplo).
                        renderer.draw_rect_outline(r, kMiraColor, is_cursor ? 3.0f : 1.0f);
                        if (is_cursor) {
                            const Rect inr{r.x + 3.0f, r.y + 3.0f, r.w - 6.0f, r.h - 6.0f};
                            renderer.draw_rect_outline(inr, kMiraColor, 1.0f);
                        }
                        // (2) BADGE numerado (tecla-atalho): quadrado no canto sup-esq + o
                        //     digito (i+1). Forma PURA (rect) + TEXTO (digito) => multimodal; o
                        //     digito ENSINA a tecla 1/2/3. Cursor = badge PREENCHIDO (contraste).
                        const Rect badge{r.x - 2.0f, r.y - 2.0f, 14.0f, 14.0f};
                        renderer.draw_filled_rect(
                            badge, is_cursor ? kMiraColor : kSlotDarkColor);
                        renderer.draw_rect_outline(badge, kMiraColor, 1.0f);
                        char num[4];
                        std::snprintf(num, sizeof(num), "%d", choice_idx + 1);
                        renderer.draw_text(num, badge.x + 4.0f, badge.y + 2.0f, 10.0f,
                                           is_cursor ? kBgColor : kMiraColor, /*bold=*/true);
                        // (3) CURSOR: caret (forma pura, 3 colunas) a ESQUERDA do slot +
                        //     NOME (texto legivel) acima. Mesmo vocabulario da mira (WCAG).
                        if (is_cursor) {
                            const float acy = r.y + r.h * 0.5f;
                            for (int c = 0; c < 3; ++c) {
                                const float ch = 12.0f - static_cast<float>(c) * 4.0f;
                                const Rect bar{
                                    r.x - 12.0f + static_cast<float>(c) * kMiraCaretW,
                                    acy - ch * 0.5f, kMiraCaretW, ch};
                                renderer.draw_filled_rect(bar, kMiraColor);
                            }
                            const std::string nm = a->display_name();
                            renderer.draw_text(nm.c_str(), r.x, r.y - kMiraLabelPx - 3.0f,
                                               kMiraLabelPx, kMiraColor, /*bold=*/true);
                        }
                    }
                }
            }
        };
        draw_side(arena.party_count, arena.party, alive_party, kCyan, kHp,
                  /*is_party=*/true);
        draw_side(arena.enemy_count, arena.enemies, alive_enemies, kMagenta, kMagenta,
                  /*is_party=*/false);
    }

    // --- PROJETEIS de magia (W2, battle-anim.md par.2.1), por cima dos atores ---
    // Bolinha placeholder NEUTRA de proposito (so valida o esqueleto de timing; o VFX
    // por familia entra depois, vfx-combate-familias.md). O IRenderer so tem rects:
    // a bolinha e aproximada por 3 fatias horizontais (circulo chunky pixel-art).
    for (const auto& p : anim_.projectiles()) {
        const Vec2 pos = p.position();
        const float pr = kProjectileRadiusPx;
        renderer.draw_filled_rect(
            Rect{pos.x - pr, pos.y - pr * 0.5f, 2.0f * pr, pr}, kInk);
        renderer.draw_filled_rect(
            Rect{pos.x - pr * 0.6f, pos.y - pr, 1.2f * pr, pr * 0.5f}, kInk);
        renderer.draw_filled_rect(
            Rect{pos.x - pr * 0.6f, pos.y + pr * 0.5f, 1.2f * pr, pr * 0.5f}, kInk);
    }

    // --- caixa de log (base): ESTRUTURA por evento (D7), 1 marca colorida por linha ---
    // FIX (lider no display): NAO renderiza na ABERTURA - na intro o log esta vazio e a
    // caixa virava um "quadrao preto" grande. So aparece DEPOIS de Encarar (quando ha
    // narracao). A abertura fica limpa (CTB + arena + banner + prompt).
    // ADR-009: com HUD externo (RmlUi), o log/terminal e 100% RmlUi - nao desenha a mao.
    if (!is_intro() && !hud_external_) {
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
