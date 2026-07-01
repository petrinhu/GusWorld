// gus/app/src/screens/battle_layout.cpp
//
// Implementacao do layout PURO da BattleScreen (variante C "Tatico Cockpit"). Aritmetica
// de pixels logicos (960x540, D1), deterministica, sem SDL. Cockpit lateral esquerdo +
// arena com atores DISTRIBUIDOS (space-around) a direita + fila CTB no topo + banner numa
// faixa propria + terminal/log no rodape.

#include "gus/app/screens/battle_layout.hpp"

#include <algorithm>  // std::clamp, std::max

namespace gus::app::screens {

namespace {

// Centro vertical da i-esima faixa de N, distribuidas com ESPACO PROPRIO (space-around)
// na banda [top, bottom]: cada faixa tem altura band/N e o ator fica no centro dela. Isso
// da folga igual entre os atores e nas pontas (igual ao flex space-around do mock).
float space_around_center_y(int i, int count, int band_top, int band_bottom) noexcept {
    if (count <= 0) {
        return static_cast<float>((band_top + band_bottom) / 2);
    }
    const float band_h = static_cast<float>(band_bottom - band_top);
    const float slice = band_h / static_cast<float>(count);
    return static_cast<float>(band_top) + slice * (static_cast<float>(i) + 0.5f);
}

}  // namespace

Rect battle_screen_rect() noexcept {
    return Rect{0.0f, 0.0f, static_cast<float>(kBattleLogicalW),
                static_cast<float>(kBattleLogicalH)};
}

ArenaLayout arena_layout(int party_count, int enemy_count,
                         int gus_party_index) noexcept {
    ArenaLayout out;
    out.party_count = std::clamp(party_count, 0, kMaxPartySlots);
    out.enemy_count = std::clamp(enemy_count, 0, kMaxEnemySlots);

    const float half_w = static_cast<float>(kActorSlotW) * 0.5f;
    const float half_h = static_cast<float>(kActorSlotH) * 0.5f;

    // --- party na coluna ESQUERDA-da-arena (centrada em kPartyColCenterX); space-around;
    //     o Gus recua em X (D3, pra dentro = pra direita). ---
    for (int i = 0; i < out.party_count; ++i) {
        const float cy =
            space_around_center_y(i, out.party_count, kArenaTop, kArenaBottom);
        float cx = static_cast<float>(kPartyColCenterX);
        if (i == gus_party_index) {
            cx += static_cast<float>(kGusRecuoX);  // D3: Gus recuado
        }
        out.party[static_cast<std::size_t>(i)] = ActorSlot{
            Rect{cx - half_w, cy - half_h, static_cast<float>(kActorSlotW),
                 static_cast<float>(kActorSlotH)},
            /*occupied=*/true};
    }

    // --- inimigos na coluna DIREITA (centrada em kFoeColCenterX); space-around ---
    for (int i = 0; i < out.enemy_count; ++i) {
        const float cy =
            space_around_center_y(i, out.enemy_count, kArenaTop, kArenaBottom);
        const float cx = static_cast<float>(kFoeColCenterX);
        out.enemies[static_cast<std::size_t>(i)] = ActorSlot{
            Rect{cx - half_w, cy - half_h, static_cast<float>(kActorSlotW),
                 static_cast<float>(kActorSlotH)},
            /*occupied=*/true};
    }

    return out;
}

CtbStrip ctb_strip(int queue_len) noexcept {
    CtbStrip out;
    const int len = std::max(queue_len, 0);
    // Quantas celulas ficam OCUPADAS (no maximo 5). Se a fila for maior que 5, todas
    // as 5 sao ocupadas e a ULTIMA vira o marcador de overflow "+N".
    const int occupied = std::min(len, kCtbVisibleCells);
    const bool overflow = len > kCtbVisibleCells;

    // Centra a celula CTB verticalmente na faixa da CTB.
    const float cell_y = static_cast<float>(kCtbStripTop) +
                         (static_cast<float>(kCtbStripH - kCtbPortraitPx)) * 0.5f;
    for (int i = 0; i < kCtbVisibleCells; ++i) {
        const float x = static_cast<float>(
            kCtbStripLeft + i * (kCtbPortraitPx + kCtbCellGap));
        CtbCell cell;
        cell.rect = Rect{x, cell_y, static_cast<float>(kCtbPortraitPx),
                         static_cast<float>(kCtbPortraitPx)};
        cell.occupied = (i < occupied);
        // CONVENCAO pos-rotacao (fix da fila CTB 2026-07-01): o consumidor entrega a fila
        // como uma JANELA ROTACIONADA que COMECA no ator ATIVO (celula 0 = turno AGORA,
        // marcada como "ativo" via who==active no render). Logo o "proximo" a jogar e a
        // celula 1 (nao a 0). is_next marca a celula 1 SO quando ha mais de um ator na
        // janela (occupied > 1); com um so ator (o ativo) nao ha "proximo".
        cell.is_next = (i == 1 && occupied > 1);
        if (overflow && i == kCtbVisibleCells - 1) {
            cell.is_overflow = true;
            cell.overflow_count = len - (kCtbVisibleCells - 1);
        }
        out.cells[static_cast<std::size_t>(i)] = cell;
    }
    return out;
}

// ---- Zonas da variante C ----

Rect cockpit_rect() noexcept {
    return Rect{static_cast<float>(kCockpitX), 0.0f,
                static_cast<float>(kCockpitW),
                static_cast<float>(kBattleLogicalH)};
}

Rect cockpit_portrait_rect() noexcept {
    // Centrado na largura util do cockpit, perto do topo.
    const float x =
        static_cast<float>(kCockpitInnerX) +
        (static_cast<float>(kCockpitInnerW - kCockpitPortraitPx)) * 0.5f;
    const float y = static_cast<float>(kCockpitPad);
    return Rect{x, y, static_cast<float>(kCockpitPortraitPx),
                static_cast<float>(kCockpitPortraitPx)};
}

Rect cockpit_hp_bar_rect() noexcept {
    // Largura util do cockpit; abaixo do retrato (64) + nome (~22) com folga.
    const float y = static_cast<float>(kCockpitPad + kCockpitPortraitPx + 28);
    return Rect{static_cast<float>(kCockpitInnerX), y,
                static_cast<float>(kCockpitInnerW),
                static_cast<float>(kCockpitHpBarH)};
}

Vec2 cockpit_ap_pips_origin() noexcept {
    const Rect hp = cockpit_hp_bar_rect();
    return Vec2{static_cast<float>(kCockpitInnerX),
                hp.y + hp.h + 12.0f};  // abaixo do HP (com rotulo "AP")
}

Vec2 cockpit_mana_pips_origin() noexcept {
    const Vec2 ap = cockpit_ap_pips_origin();
    return Vec2{ap.x, ap.y + static_cast<float>(kCockpitPipSize) + 10.0f};
}

Rect cockpit_menu_zone() noexcept {
    // Faixa de baixo do cockpit pros 6 verbos empilhados. Reserva folga acima (recursos)
    // e abaixo (margem). Comeca apos a area de Mana e vai ate quase a base do cockpit.
    const Vec2 mana = cockpit_mana_pips_origin();
    const float top = mana.y + static_cast<float>(kCockpitPipSize) + 14.0f;
    const float bottom = static_cast<float>(kBattleLogicalH - kCockpitPad);
    return Rect{static_cast<float>(kCockpitInnerX), top,
                static_cast<float>(kCockpitInnerW), bottom - top};
}

Rect arena_banner_rect() noexcept {
    const float x = static_cast<float>(kRightZoneX);
    const float w = static_cast<float>(kBattleLogicalW - kRightZoneMargin) - x;
    return Rect{x, static_cast<float>(kBannerBandTop), w,
                static_cast<float>(kBannerBandH)};
}

Rect arena_rect() noexcept {
    const float x = static_cast<float>(kRightZoneX);
    const float w = static_cast<float>(kBattleLogicalW - kRightZoneMargin) - x;
    return Rect{x, static_cast<float>(kArenaTop), w,
                static_cast<float>(kArenaBottom - kArenaTop)};
}

Rect log_panel_rect() noexcept {
    const float x = static_cast<float>(kLogLeft);
    const float w = static_cast<float>(kBattleLogicalW) - x;  // ate a borda direita
    return Rect{x, static_cast<float>(kLogTop), w,
                static_cast<float>(kLogH)};
}

}  // namespace gus::app::screens
