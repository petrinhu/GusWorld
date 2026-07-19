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
//
// AC-E11 A4 (TU-split, ADR-019): a DEFINICAO de BattleScene::render() (+ draw_bar/
// draw_pips + a paleta canonica/constantes de flavor/helpers de desenho) mudou pra
// battle_scene_render.cpp (mesma classe, header intocado) - este arquivo ficou grande
// demais. Ver o arquivo novo pro render.

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

// --- FLAVOR DE DERROTA (M7-COSTURA Inc 3, docs/design/mecanicas/combat-flavor.md §3b
//     "Derrota"): o Gus reboota, nao morre (echo Batman, Pillar 1 "magia = software"). ---
// Duracao do overlay: 5.0s (Fibonacci - canon do easter egg pervasivo do projeto,
// project_fibonacci_easter_egg - nunca anunciado, so um numero da sequencia onde um
// timing generico serviria igual). SO TIMER (sem input): anti-OE, nao e cutscene.
constexpr float kDefeatFlavorSeconds = 5.0f;

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

// Encontro de DEMO (M5 navegavel; REBALANCEADO M7-COSTURA BUG-5, playtest ao vivo do
// lider): party de 3 (Gus + Caua + Jaci) vs 2 inimigos (ERA 4 - "impossivel de
// vencer": 4 inimigos = 4 acoes/rodada contra so 3 da party, empilhado com o GUS-
// CENTRIC do BUG-4 (Gus a HP 0 = Defeat IMEDIATO) deixava o lider SEMPRE perdendo).
// NAO e balanceamento CANONICO do jogo - e placeholder de QA, so pra este encontro de
// tela ser EQUILIBRADO (vencivel jogando bem, perdivel jogando mal). Ver o teste
// empirico "make_demo_actors: equilibrio" em battle_scene_test.cpp (BUG-5) - simula os
// 2 estilos de jogo pela API PUBLICA da BattleScene (nao reimplementa a formula).
//
// NUMEROS FINAIS (dano = max(1, atk-def), secao 11):
//   Party: Gus  hp=55 atk=11 def=5 spd=9  (compilador universal - GUS-CENTRIC)
//          Caua hp=60 atk=11 def=6 spd=12 (maior SPD da party -> ScriptedBrain mira ELE
//                                          primeiro, NUNCA o Gus, enquanto vivo - ver
//                                          scripted_brain.cpp: alvo = alive_players().
//                                          front(), que segue a fila ordenada por SPD)
//          Jaci hp=50 atk=9  def=5 spd=7
//   Inimigos: Sentinela hp=28 atk=13 def=4 spd=11
//             Drone     hp=22 atk=11 def=3 spd=13
//   Dano da party (focus-fire, "jogando bem"): Gus/Caua ~7-8, Jaci ~5-6 por hit; os 2
//   inimigos (28+22=50 hp) caem em ~2-3 rodadas de fila focada.
//   Dano dos inimigos (SEMPRE em Caua, o alvo de maior SPD, enquanto ele viver):
//   ~14-15/rodada -> Caua (60 hp) cairia em ~4 rodadas SE ninguem revidasse - tempo de
//   sobra pra limpar os 2 inimigos ANTES disso jogando ativamente (vitoria). So jogando
//   PASSIVO (nunca atacar - Defender/Scan o combate inteiro) o relogio vence: Caua cai
//   (~4 rodadas), os inimigos entao migram pro proximo maior SPD vivo (Gus, 9 > Jaci,
//   7) e o Gus cai pouco depois - Defeat IMEDIATO (BUG-4).
std::vector<std::unique_ptr<CombatActor>> make_demo_actors() {
    std::vector<std::unique_ptr<CombatActor>> v;
    auto add = [&v](std::string id, std::string name, int hp, int atk, int def, int spd,
                    CardFamily fam, bool player, bool gus) {
        v.push_back(std::make_unique<CombatActor>(
            std::move(id), std::move(name), hp, atk, def, spd, fam, player,
            /*is_boss=*/false, /*knowledge_kills=*/0,
            /*is_universal_compiler=*/gus));
    };
    // Party (player_side = true). Gus = compilador universal.
    add("gus", "Gus", 55, /*atk=*/11, /*def=*/5, /*spd=*/9, CardFamily::Criptografico,
        true, true);
    add("caua", "Caua", 60, /*atk=*/11, /*def=*/6, /*spd=*/12, CardFamily::Eletrico,
        true, false);
    add("jaci", "Jaci", 50, /*atk=*/9, /*def=*/5, /*spd=*/7, CardFamily::Bioquimico,
        true, false);
    // Inimigos (player_side = false). 2 (ERA 4, BUG-5) - SPD variada pra intercalar a
    // fila.
    add("inimigo1", "Sentinela", 28, /*atk=*/13, /*def=*/4, /*spd=*/11,
        CardFamily::Cinetico, false, false);
    add("inimigo2", "Drone", 22, /*atk=*/11, /*def=*/3, /*spd=*/13, CardFamily::Sonico,
        false, false);

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

void BattleScene::play_hit_sfx() {
    // M6 F3 (ADR-011): no-op seguro se set_audio nunca foi chamado - a cena roda muda,
    // NUNCA depende de audio (mesmo espirito de degradacao graciosa do AudioEngine e do
    // resto da fronteira headless-first do jogo). O AudioEngine::play_sfx por si so ja
    // e no-op seguro pra kInvalidSound/engine indisponivel; o ponteiro nulo aqui cobre o
    // caso "ninguem plugou audio nesta cena" (testes headless, viewers antigos).
    if (audio_engine_ != nullptr) {
        audio_engine_->play_sfx(hit_sfx_id_);
    }
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
            // M6 F3 (ADR-011): o SOM do hit nasce no MESMO instante do hit-react visual
            // (o contato ja resolveu: dano aplicado, floater nascido) - cobre TANTO o
            // atacante da party QUANTO o inimigo (esta funcao processa o log dos dois
            // lados igual, resolve_one_turn e chamada nos dois caminhos de contato).
            play_hit_sfx();
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
    // FLAVOR DE DERROTA (M7-COSTURA Inc 3): envelhece o overlay SO enquanto Defeat -
    // Victory/Fled nunca avancam (defeat_flavor_active ja os exclui pela checagem de
    // outcome). Trava em kDefeatFlavorSeconds (o if evita crescer pra sempre; nao muda
    // o resultado de defeat_flavor_active apos travado).
    if (machine_->outcome() == gus::domain::combat::CombatOutcome::Defeat &&
        defeat_flavor_elapsed_ < kDefeatFlavorSeconds) {
        defeat_flavor_elapsed_ += dt_seconds;
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
        // M6 F3 (ADR-011): call-site do PROJETIL - mesmo play_sfx do melee, disparado no
        // IMPACTO real (fim do voo), nunca no windup/conjuracao.
        play_hit_sfx();
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

bool BattleScene::defeat_flavor_active() const noexcept {
    return machine_->outcome() == gus::domain::combat::CombatOutcome::Defeat &&
           defeat_flavor_elapsed_ < kDefeatFlavorSeconds;
}

void BattleScene::start_active_turn() {
    // Prepara o turno do ator ativo (begin_turn: recarrega AP/Mana, tick de status).
    // Idempotente por turno: so chama begin_turn UMA vez por ator (turn_started_).
    if (turn_started_ || combat_over()) {
        return;
    }
    // ESCOLHA DE ATOR (§4.1, comando-livre 1B): enquanto o picker (LISTA, estagio 1) OU o
    // PREVIEW do escolhido (estagio 2, motor ainda intocado) estao abertos, o begin_turn fica
    // DEFERIDO - so commit_previewed_actor() (chamado pela 1a acao de fato resolvida) inicia o
    // turno real (via begin_active_turn_now). SEM este guard, advance_pacing() (que chama
    // start_active_turn TODO FRAME) reabriria o picker do zero em cima do preview (bug: o
    // should_offer_actor_picker abaixo tambem veria o ator em preview como "ativo" via
    // active_actor(), ainda com pending_party_actors().size()>1).
    if (choosing_actor_ || actor_preview_) {
        return;
    }
    // E a vez do BLOCO da party com >1 elegivel? Entao NAO inicia o turno ainda: entra no modo
    // de escolha e espera o jogador comandar QUAL membro age (a SPD so SUGERE o pre-selecionado,
    // §4.1). O begin_turn vem depois, no commit real (commit_previewed_actor, na 1a acao). Com
    // <=1 elegivel (ou vez de inimigo/intro), o fluxo segue direto (begin_active_turn_now) =>
    // identico ao motor pre-picker (sem picker, sem preview).
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

    // PONTO-DE-NAO-RETORNO (bug1, §4.1): Defender/Flee chegaram ate aqui = a acao VAI
    // resolver agora. Se o ator ainda esta em PREVIEW (picker, motor intocado), este e o
    // commit: grava a escolha + roda o begin_turn REAL (tick de status incluso) ANTES de
    // resolver. No-op se ja commitado (sem picker, ou preview ja resolvido antes).
    commit_previewed_actor();

    // RITMO (incremento 6): submete a acao do jogador e resolve SO o turno dele (com
    // floater + narracao de consequencia). NAO encadeia os inimigos aqui: o diretor
    // entra em delay (player_acted) e o update(dt) anima os turnos de inimigo um a um.
    pending_action_ = std::move(action);
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
    // NAO chama begin_turn: o confirm so entra no PREVIEW (estagio 2); o turno real so
    // comeca no commit_previewed_actor (1a acao resolvida). Chamada tambem por
    // actor_preview_cancel (Esc no preview reabre a lista do zero, mesma fonte).
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

    // FIX bug1 (playtest do lider 2026-07: "escolho e nao consigo trocar - fica FIXO"): NAO
    // grava a escolha no motor nem chama begin_turn aqui. Antes o confirm chamava
    // begin_active_turn_now() na hora, que faz apply_status_tick (IRREVERSIVEL) colado a
    // simples SELECAO do ator - por isso "travava". Agora so entra no estagio (2) PREVIEW: o
    // menu de verbos DELE aparece (AP/mana corretos, ver refresh abaixo), mas o motor segue
    // 100% intocado (sem select_party_actor, sem begin_turn, sem tick). O commit real fica pra
    // commit_previewed_actor() (chamado so quando a 1a acao de fato resolve - menu_confirm ou
    // aim_confirm). Ate la, Esc (actor_preview_cancel) reabre esta MESMA lista do zero e o
    // jogador troca de ator sem custo algum.
    choosing_actor_ = false;
    actor_choices_.clear();
    actor_pick_index_ = 0;
    actor_preview_ = true;
    preview_actor_ = chosen;

    // AP/MANA no PREVIEW: sem isto o cockpit mostraria ap()==0 (o campo so e populado por
    // refresh_resources_for_turn, chamado ate agora SO dentro do begin_turn) e TODO verbo
    // apareceria desabilitado - preview inutil. refresh_resources_for_turn e INOCUO/
    // IDEMPOTENTE (sem RNG; so recarrega ap_/mana_ pela formula deterministica de
    // round_index) e e chamado de novo, com o MESMO round_index, dentro do begin_turn REAL em
    // commit_previewed_actor() - byte-identico, sem efeito duplicado observavel. Isto e o
    // unico ponto onde a cena toca um metodo do ATOR (POCO ja publico) fora do begin_turn do
    // motor; NENHUM arquivo de domain/ foi alterado (combat_state_machine/combat_actor
    // intocados) - reportado como checkpoint (a) da tarefa.
    chosen->refresh_resources_for_turn(machine_->queue().round_index());
    menu_.refresh(chosen->ap());
}

void BattleScene::commit_previewed_actor() {
    if (!actor_preview_) {
        return;  // sem preview pendente: <=1 elegivel (begin_active_turn_now ja rodou direto
                 // em start_active_turn) OU este ator ja foi commitado antes. No-op seguro -
                 // chamado incondicionalmente no topo de toda resolucao real de acao.
    }
    // PONTO-DE-NAO-RETORNO (bug1, regra fechada Caetano+lider): grava a escolha no motor
    // (select_party_actor) e sai do preview ANTES de chamar begin_active_turn_now - assim
    // active_actor() ja cai no caminho normal (machine_->active_actor()) dentro dele, que o
    // proprio begin_turn alinha via bring_to_current(chosen). select_party_actor lanca
    // std::invalid_argument se `preview_actor_` nao e mais elegivel; como nada mutou o motor
    // entre o confirm e este commit (o preview e 100% apresentacao), ele SEMPRE e elegivel.
    machine_->select_party_actor(preview_actor_);
    actor_preview_ = false;
    preview_actor_ = nullptr;
    // O begin_turn REAL: bring_to_current + refresh (idempotente, ja rodou uma vez no
    // preview) + o TICK DE STATUS - agora sim IRREVERSIVEL (Poison floata, Haste/Slow
    // aplicam, etc). turn_started_ segue false ate aqui (garantido por start_active_turn
    // nunca ter chamado begin_active_turn_now enquanto actor_preview_ era true).
    begin_active_turn_now();
}

void BattleScene::actor_preview_cancel() noexcept {
    if (!actor_preview_) {
        return;  // fora do preview: nada a desfazer (picker fechado, ou ja commitado - nao ha
                 // volta apos a 1a acao, ver commit_previewed_actor).
    }
    actor_preview_ = false;
    preview_actor_ = nullptr;
    // Reabre a LISTA do zero (mesma fonte, pending_party_actors()): como o preview NUNCA
    // tocou o motor, a lista e IDENTICA a de antes (nao precisamos guardar/restaurar nada).
    // should_offer_actor_picker() nao e re-checado de proposito (mesmo padrao do confirm): sabemos
    // que havia >1 elegivel quando este preview abriu, e nada consumiu nenhum deles.
    enter_actor_picker();
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
    // PREVIEW (estagio 2, §4.1 bug1): enquanto o ator escolhido no picker ainda nao foi
    // commitado (motor intocado - o queue_.current() do motor NAO e o escolhido ate o
    // bring_to_current dentro de begin_turn, que so roda em commit_previewed_actor), a cena
    // PROJETA o escolhido como "ativo" pra TUDO que le active_actor() (cockpit, banner,
    // current_actor_is_player, ctb highlight, menu). Fonte UNICA: nenhum destes precisa saber
    // sobre preview_ separadamente. Apos o commit, actor_preview_ cai e este cai no caminho de
    // sempre (que ja aponta pro MESMO ator, alinhado pelo bring_to_current).
    if (actor_preview_ && preview_actor_ != nullptr) {
        return preview_actor_;
    }
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

}  // namespace gus::app::screens
