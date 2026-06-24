// gus/app/src/screens/battle_scene.cpp
//
// Implementacao da BattleScene (ver header). INCREMENTO 1: monta o encontro de DEMO,
// le o estado do motor (contagem de atores + fila CTB + ator ativo) e desenha o
// ESQUELETO com PLACEHOLDERS. SEM cartas/animacoes/numeros flutuantes/transicao.
//
// RENDER 1:1: a camera do IRenderer e dirigida pro retangulo logico 640x360 (D1), entao
// cada Rect do battle_layout (em px logico) vira um Rect de mundo identico. O backend
// (Render2dSdl) escala 640x360 -> janela por inteiro (pixel-perfect), letterbox a cargo
// do backend/janela.

#include "gus/app/screens/battle_scene.hpp"

#include <utility>  // std::move

#include "gus/app/screens/battle_layout.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"

namespace gus::app::screens {

namespace {

using gus::core::spatial::Rect;
using gus::domain::combat::CardFamily;
using gus::domain::combat::CombatActor;
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
        for (int i = 0; i < kCtbVisibleCells; ++i) {
            const CtbCell& cell = strip.cells[static_cast<std::size_t>(i)];
            if (!cell.occupied) {
                continue;
            }
            // Retrato 48px se carregavel; senao retangulo da celula.
            TextureId tex = kInvalidTexture;
            if (i < static_cast<int>(order.size()) && order[i] != nullptr) {
                tex = portraits_.find(order[i]->id());
            }
            if (tex != kInvalidTexture) {
                renderer.draw_textured_rect(cell.rect, tex, UvRect{}, kWhite);
            } else {
                renderer.draw_filled_rect(cell.rect, kCtbCellColor);
            }
            // Marca de "proximo" (D4): borda destacada na 1a celula ocupada.
            if (cell.is_next) {
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

        for (int i = 0; i < arena.party_count; ++i) {
            const Rect& r = arena.party[static_cast<std::size_t>(i)].rect;
            renderer.draw_filled_rect(r, kPartyColor);
            if (active != nullptr && i < static_cast<int>(alive_party.size()) &&
                alive_party[static_cast<std::size_t>(i)] == active) {
                renderer.draw_rect_outline(r, kActiveHiColor, 2.0f);  // D7 highlight
            }
        }
        for (int i = 0; i < arena.enemy_count; ++i) {
            const Rect& r = arena.enemies[static_cast<std::size_t>(i)].rect;
            renderer.draw_filled_rect(r, kEnemyColor);
            if (active != nullptr && i < static_cast<int>(alive_enemies.size()) &&
                alive_enemies[static_cast<std::size_t>(i)] == active) {
                renderer.draw_rect_outline(r, kActiveHiColor, 2.0f);  // D7 highlight
            }
        }
    }

    // --- painel do ator ativo (base) ---
    {
        const Rect p = active_panel_rect();
        renderer.draw_filled_rect(p, kPanelColor);
        renderer.draw_rect_outline(p, kHudBorderColor, 1.0f);
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
