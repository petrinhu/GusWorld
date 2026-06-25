// gus/app/src/screens/battle_layout.cpp
//
// Implementacao do layout PURO da BattleScreen (ver header). Aritmetica de pixels
// logicos, deterministica, sem SDL. Empilhamento CENTRALIZADO na banda da arena
// (D3), espacamento FIXO (D2), recuo do Gus (D3), fila CTB de 5 com "proximo" e
// overflow "+N" (D4).

#include "gus/app/screens/battle_layout.hpp"

#include <algorithm>  // std::clamp, std::max

namespace gus::app::screens {

namespace {

// Centraliza N slots empilhados (cada um slot_h de altura, gap kActorSlotGapY) na banda
// vertical [top, bottom]. Devolve o Y do canto SUPERIOR do primeiro slot. Com a altura
// adaptativa (arena_slot_height) a coluna SEMPRE cabe na banda, entao o offset centraliza
// de verdade (nunca clampa pra dentro do painel).
float stacked_top_y(int count, int slot_h, int band_top, int band_bottom) noexcept {
    if (count <= 0) {
        return static_cast<float>(band_top);
    }
    const int total_h = count * slot_h + (count - 1) * kActorSlotGapY;
    const int band_h = band_bottom - band_top;
    const int free_h = band_h - total_h;
    const int offset = free_h > 0 ? free_h / 2 : 0;  // centraliza; clamp no topo
    return static_cast<float>(band_top + offset);
}

}  // namespace

int arena_slot_height(int count) noexcept {
    if (count <= 0) {
        return kActorSlotH;
    }
    // Altura util da banda DESCONTANDO a folga (topo+base) e os gaps entre os slots.
    const int usable = (kArenaBottom - kArenaTop) - 2 * kArenaBandMargin -
                       (count - 1) * kActorSlotGapY;
    int h = usable / count;            // maior altura que faz a coluna caber na banda
    if (h > kActorSlotH) {
        h = kActorSlotH;               // teto: nunca maior que o tamanho-base
    }
    if (h < kActorSlotMinH) {
        h = kActorSlotMinH;            // piso: legibilidade (com cap de slots, sempre cabe)
    }
    return h;
}

Rect battle_screen_rect() noexcept {
    return Rect{0.0f, 0.0f, static_cast<float>(kBattleLogicalW),
                static_cast<float>(kBattleLogicalH)};
}

ArenaLayout arena_layout(int party_count, int enemy_count,
                         int gus_party_index) noexcept {
    ArenaLayout out;
    out.party_count = std::clamp(party_count, 0, kMaxPartySlots);
    out.enemy_count = std::clamp(enemy_count, 0, kMaxEnemySlots);

    // --- coluna da party (esquerda), centralizada na banda; Gus recua em X ---
    // Altura ADAPTATIVA por contagem do PROPRIO lado: a coluna sempre cabe na banda
    // (nenhum slot invade o painel). O passo de empilhamento usa essa altura + o gap.
    {
        const int slot_h = arena_slot_height(out.party_count);
        const float step = static_cast<float>(slot_h + kActorSlotGapY);
        const float top_y =
            stacked_top_y(out.party_count, slot_h, kArenaTop, kArenaBottom);
        for (int i = 0; i < out.party_count; ++i) {
            const float y = top_y + static_cast<float>(i) * step;
            float x = static_cast<float>(kPartyColumnX);
            if (i == gus_party_index) {
                x += static_cast<float>(kGusRecuoX);  // D3: Gus recuado
            }
            out.party[static_cast<std::size_t>(i)] = ActorSlot{
                Rect{x, y, static_cast<float>(kActorSlotW),
                     static_cast<float>(slot_h)},
                /*occupied=*/true};
        }
    }

    // --- coluna dos inimigos (direita), centralizada na banda ---
    {
        const int slot_h = arena_slot_height(out.enemy_count);
        const float step = static_cast<float>(slot_h + kActorSlotGapY);
        const float top_y =
            stacked_top_y(out.enemy_count, slot_h, kArenaTop, kArenaBottom);
        for (int i = 0; i < out.enemy_count; ++i) {
            const float y = top_y + static_cast<float>(i) * step;
            out.enemies[static_cast<std::size_t>(i)] = ActorSlot{
                Rect{static_cast<float>(kEnemyColumnX), y,
                     static_cast<float>(kActorSlotW),
                     static_cast<float>(slot_h)},
                /*occupied=*/true};
        }
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

    for (int i = 0; i < kCtbVisibleCells; ++i) {
        const float x = static_cast<float>(
            kCtbStripLeft + i * (kCtbPortraitPx + kCtbCellGap));
        CtbCell cell;
        cell.rect = Rect{x, static_cast<float>(kCtbStripTop),
                         static_cast<float>(kCtbPortraitPx),
                         static_cast<float>(kCtbPortraitPx)};
        cell.occupied = (i < occupied);
        cell.is_next = (i == 0 && occupied > 0);
        if (overflow && i == kCtbVisibleCells - 1) {
            cell.is_overflow = true;
            // "+N": atores alem dos 4 primeiros visiveis representados nesta 5a casa.
            cell.overflow_count = len - (kCtbVisibleCells - 1);
        }
        out.cells[static_cast<std::size_t>(i)] = cell;
    }
    return out;
}

Rect active_panel_rect() noexcept {
    return Rect{static_cast<float>(kHudSideMargin),
                static_cast<float>(kActivePanelTop),
                static_cast<float>(kBattleLogicalW - 2 * kHudSideMargin),
                static_cast<float>(kActivePanelH)};
}

Rect log_panel_rect() noexcept {
    return Rect{static_cast<float>(kHudSideMargin),
                static_cast<float>(kLogTop),
                static_cast<float>(kBattleLogicalW - 2 * kHudSideMargin),
                static_cast<float>(kLogH)};
}

}  // namespace gus::app::screens
