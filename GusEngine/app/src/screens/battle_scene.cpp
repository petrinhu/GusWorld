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
    // Status de demo (Regen no Gus, Poison no inimigo3) pra exercitar os icones.
    v[0]->add_status(gus::domain::combat::StatusEffect{
        gus::domain::combat::StatusId::Regen, /*magnitude=*/3, /*duration=*/2});
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

BattleScene::BattleScene() {
    actors_ = make_demo_actors();

    std::vector<CombatActor*> ptrs;
    ptrs.reserve(actors_.size());
    for (const auto& a : actors_) {
        ptrs.push_back(a.get());
    }

    // Provider NO-OP no incremento 1: a cena NAO conduz turnos ainda, so le a fila e a
    // contagem. A FSM exige um provider no ctor; um Pass deterministico basta (nunca
    // chamado enquanto nao rodarmos run_active_turn_to_end).
    auto provider = [](CombatActor&, const gus::domain::combat::CombatState&) {
        return gus::domain::combat::CombatAction::pass();
    };
    machine_ = std::make_unique<gus::domain::combat::CombatStateMachine>(
        std::move(ptrs), std::move(provider));

    // PRIMING do turno (incremento 2, LEITURA-only): begin_turn() recarrega AP/Mana do
    // ator ativo (ramp de mana) e o poe em ActionSelect, que e EXATAMENTE o estado que
    // o painel exibe ("o ator cujo turno e"). NAO conduzimos o turno (sem
    // run_active_turn_to_end): so deixamos o ator ativo com recursos reais pra mostrar.
    machine_->begin_turn();
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

            // HP: barra + preenchimento real (hp/max_hp).
            const Rect hp_frame{x, y, static_cast<float>(kHpBarW),
                                static_cast<float>(kHpBarH)};
            draw_bar(renderer, hp_frame, bar_fill(a->hp(), a->max_hp()),
                     kHpFillColor);
            y += kHpBarH + kResourceRowGap;

            // AP: pips (max_ap pips; ap acesos). max_ap = kBaseApPerTurn (3).
            draw_pips(renderer, x, y, a->max_ap(), a->ap(),
                      gus::domain::combat::combat_constants::kBaseApPerTurn,
                      kApLitColor, kApOffColor);
            y += kPipSize + kResourceRowGap;

            // Mana: pips (max_mana pips, cap kManaCap=8; mana acesos). O ramp
            // (max_mana = 2 + round_index, cap 8) ja esta no max_mana() do ator.
            draw_pips(renderer, x, y, a->max_mana(), a->mana(),
                      gus::domain::combat::combat_constants::kManaCap, kManaLitColor,
                      kManaOffColor);

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
        }
    }

    // --- caixa de log (base) ---
    {
        const Rect l = log_panel_rect();
        renderer.draw_filled_rect(l, kLogColor);
        renderer.draw_rect_outline(l, kHudBorderColor, 1.0f);
    }

    renderer.end_frame();
}

}  // namespace gus::app::screens
