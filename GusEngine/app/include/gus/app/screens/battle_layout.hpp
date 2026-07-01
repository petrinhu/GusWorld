// gus/app/screens/battle_layout.hpp
//
// LAYOUT PURO da BattleScreen (M5), POCO 100% testavel SEM SDL nem janela. Calcula
// TODOS os retangulos/pontos da tela de batalha em ESPACO LOGICO 960x540 (D1: base
// pixel-perfect, escala inteira x2 = 1080p) como FUNCOES PURAS da contagem de atores. A
// casca de render (battle_scene + SdlWindow) projeta esse layout 1:1 num IRenderer
// dirigindo a camera para o retangulo logico 960x540 (px_per_world_unit = 1).
//
// Por que aqui (camada app/, mas POCO): a regra das 4 camadas mantem core/ e domain/
// como POCO puros; a BattleScreen vive em app/. Toda logica "calculavel-sem-janela"
// (onde fica cada slot, a fila CTB, as zonas de HUD) e separada do render SDL e
// testavel headless (Catch2), exatamente como sprite_animation/camera_clamp.
//
// REDESIGN "TATICO COCKPIT" (variante C, aprovada pelo criador 2026-06-25). O painel do
// ator ativo deixou o RODAPE e virou um COCKPIT lateral ESQUERDO (coluna vertical fina,
// ~1/4 da largura): retrato GRANDE + nome + HP + AP/Mana + menu de verbos EMPILHADO. A
// arena ocupa o resto (a direita do cockpit), com a fila CTB no topo, um banner de turno
// numa FAIXA PROPRIA acima dos atores, e um terminal/log fino no rodape. As zonas (mock
// HTML _common.css + variante_C_tatico.html):
//   - cockpit:  x=0,   y=0,   w=174, h=540 (full).
//   - CTB:      x=188, y=10,  w=758, h=54.
//   - banner:   x=188, y=70,  w=758, h=46 (faixa propria; nao invade atores).
//   - arena:    x=188, y=120, w=758, h=328 (party-col esq, foe-col dir, space-around).
//   - log:      x=188, y=456, w=772, h=84.
// Os atores na arena sao distribuidos com ESPACO PROPRIO (space-around vertical, cada um
// na sua faixa, ~54px), NAO empilhados colados - leitura limpa de intent/fraqueza.
//
// DECISOES PRESERVADAS (par.5/5.2): D4 (fila CTB 5 proximos), D7 (camera estatica +
// highlight + floater), D8 (pacing 2-beats), D9 (banner "Vez de <nome>"), D10/D13
// (abertura-espera-input). So mudou o LAYOUT (denso -> cockpit) e a PALETA (canonica do
// _common.css). A resolucao logica 960x540 fica.
//
// Cross-ref: scratchpad/battle_mock/variante_C_tatico.html + _common.css (mock aprovado);
//            docs/design/mecanicas/battle-screen.md par.2 (zonas);
//            gus/core/spatial/camera_clamp.hpp (Rect/Vec2).

#ifndef GUS_APP_SCREENS_BATTLE_LAYOUT_HPP
#define GUS_APP_SCREENS_BATTLE_LAYOUT_HPP

#include <array>

#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Rect / Vec2

namespace gus::app::screens {

// ----------------------------------------------------------------------------
// Constantes de layout (D1-D4). Ponto unico de tuning fino da tela de batalha; o
// lider/ux-ui-designer ajusta AQUI sem tocar no render. Tudo em PIXELS LOGICOS.
// ----------------------------------------------------------------------------

// D1: resolucao base logica (16:9). A janela real escala por inteiro (x2 = 1080p / x3).
// Subiu de 640x360 (lider 2026-06-25): com 960x540 os 4 inimigos + 4 zonas de HUD cabem
// COM FOLGA, e os slots de ator voltam a TAMANHO FIXO (sem escala adaptativa).
inline constexpr int kBattleLogicalW = 960;
inline constexpr int kBattleLogicalH = 540;

// D2/D3: maximo de slots por lado na arena (party <=3, inimigos 1..4). secao 2.
inline constexpr int kMaxPartySlots = 3;
inline constexpr int kMaxEnemySlots = 4;

// --- COCKPIT lateral ESQUERDO (variante C) ---
// Coluna vertical fina a esquerda: retrato grande + nome + HP + AP/Mana + verbos.
inline constexpr int kCockpitX = 0;
inline constexpr int kCockpitW = 174;       // largura do cockpit (~1/4 da tela)
inline constexpr int kCockpitPad = 12;      // padding interno
inline constexpr int kCockpitPortraitPx = 64;  // retrato grande do ator ativo
inline constexpr int kCockpitHpBarH = 11;   // barra de HP do cockpit (com numero por cima)
inline constexpr int kCockpitPipSize = 9;   // pip de AP/Mana (circular no mock; quadrado aqui)
inline constexpr int kCockpitPipGap = 3;
// Faixa lateral util (DENTRO do cockpit, descontado o padding).
inline constexpr int kCockpitInnerX = kCockpitX + kCockpitPad;             // 12
inline constexpr int kCockpitInnerW = kCockpitW - 2 * kCockpitPad;         // 150
// X onde a arena/CTB/banner/log comecam (a direita do cockpit, com folga).
inline constexpr int kRightZoneX = 188;     // x do inicio das zonas a direita do cockpit
inline constexpr int kRightZoneMargin = 14; // margem direita das zonas (CTB/banner/arena)

// D4: a fila CTB mostra os 5 PROXIMOS; celula = retrato 48px nativo (sem downscale).
inline constexpr int kCtbVisibleCells = 5;
inline constexpr int kCtbPortraitPx = 48;

// --- Faixa da fila CTB (topo, a direita do cockpit) ---
inline constexpr int kCtbStripTop = 10;    // y do topo da faixa CTB
inline constexpr int kCtbStripH = 54;      // altura da faixa
inline constexpr int kCtbCellGap = 8;      // espaco horizontal entre celulas
// Vao INICIAL da fila CTB: recuada a direita da borda do cockpit por ~1 retrato (48px),
// pra nao colar/invadir a parede do cockpit (lider 2026-06-30). +95 sobre kRightZoneX.
inline constexpr int kCtbStripOffsetX = 95;
inline constexpr int kCtbStripLeft = kRightZoneX + kCtbStripOffsetX;  // 283

// --- Faixa do BANNER de turno (acima dos atores, faixa PROPRIA) ---
inline constexpr int kBannerBandTop = 70;  // y da faixa do banner
inline constexpr int kBannerBandH = 46;

// --- Arena (a direita do cockpit) ---
// A area dos atores fica ENTRE o banner e o log. Os atores sao distribuidos com espaco
// proprio (space-around) DENTRO da banda [kArenaTop, kArenaBottom], nao empilhados colados.
inline constexpr int kArenaTop = 120;      // abaixo do banner (faixa 70..116)
inline constexpr int kArenaBottom = 448;   // acima do log (logbar comeca em 456)

// Slot de ator (retrato-placeholder por ora; sprite de corpo depois). Menor que antes
// (o cockpit ja come 1/4 da largura): 54px, como no mock variante C.
inline constexpr int kActorSlotW = 54;
inline constexpr int kActorSlotH = 54;

// Colunas dos atores DENTRO da arena (offsets a partir de kRightZoneX). party-col a
// esquerda-da-arena (logo a direita do cockpit), foe-col a direita. Cada coluna tem
// largura propria; o slot e CENTRADO nela.
inline constexpr int kPartyColCenterX = kRightZoneX + 42 + 60;   // centro da party-col
inline constexpr int kFoeColCenterX = kBattleLogicalW - kRightZoneMargin - 44 - 60;  // centro da foe-col

// D3: GUS levemente RECUADO (Pillar 4). Deslocamento em X aplicado SO ao slot do Gus
// (pra dentro da arena = pra direita na coluna da esquerda).
inline constexpr int kGusRecuoX = 16;

// --- Terminal/log fino (rodape, a direita do cockpit) ---
inline constexpr int kLogTop = 456;
inline constexpr int kLogH = kBattleLogicalH - kLogTop;  // 84, ate a base
inline constexpr int kLogLeft = kRightZoneX;

// ----------------------------------------------------------------------------
// Saidas do layout. Todos os Rect estao em PIXELS LOGICOS (origem topo-esquerda,
// +X direita, +Y baixo), prontos para o render projetar 1:1.
// ----------------------------------------------------------------------------

using gus::core::spatial::Rect;
using gus::core::spatial::Vec2;

// Retangulo do quadro logico do ator ativo (highlight de turno, D7). is_party indica
// o lado (so para conveniencia de quem desenha a seta/borda).
struct ActorSlot {
    Rect rect;            // quadro do ator (placeholder/sprite) em px logico
    bool occupied = false;  // false = slot vazio (menos atores que o maximo)
};

// Resultado completo do layout da arena (party + inimigos), por contagem.
struct ArenaLayout {
    std::array<ActorSlot, kMaxPartySlots> party{};   // [0] = topo da coluna esquerda
    std::array<ActorSlot, kMaxEnemySlots> enemies{}; // [0] = topo da coluna direita
    int party_count = 0;
    int enemy_count = 0;
};

// Uma celula da faixa de fila CTB (D4): o retangulo do retrato 48px + se e o PROXIMO.
struct CtbCell {
    Rect rect;             // quadro 48px da celula em px logico
    bool occupied = false; // false = sem ator (fila curta) -> celula vazia
    // true na celula 1 (o PROXIMO a jogar). Pos-rotacao a celula 0 e o ATOR ATIVO (turno
    // agora), entao "proximo" e a celula 1. So marca quando ha >1 ator na janela.
    bool is_next = false;
    bool is_overflow = false;  // true na 5a celula quando ha MAIS de 5 (marca "+N")
    int overflow_count = 0;    // N de "+N" na celula de overflow (0 se nao aplica)
};

// As 5 celulas da fila CTB (D4): JANELA ROTACIONADA da fila, ordem esquerda->direita. O
// consumidor (battle_scene) entrega os atores comecando no ATIVO (celula 0 = turno agora,
// celula 1 = proximo, ...) via ctb_window(); esta struct so da a geometria + os flags.
struct CtbStrip {
    std::array<CtbCell, kCtbVisibleCells> cells{};
};

// ----------------------------------------------------------------------------
// Funcoes puras de layout. Determinismo total: mesma contagem -> mesmos retangulos.
// ----------------------------------------------------------------------------

// O retangulo logico inteiro da tela (960x540), origem (0,0). Util pro fundo/dim.
[[nodiscard]] Rect battle_screen_rect() noexcept;

// Layout da arena (variante C): party_count slots na coluna ESQUERDA-da-arena (logo a
// direita do cockpit), enemy_count na coluna DIREITA, DISTRIBUIDOS com espaco proprio
// (space-around vertical) na banda [kArenaTop, kArenaBottom] - cada ator na sua faixa,
// SEM empilhar colado. gus_party_index (>=0) marca qual party e o GUS, que recua em X
// (kGusRecuoX); -1 = sem recuo. Conta saturada nos maximos.
[[nodiscard]] ArenaLayout arena_layout(int party_count, int enemy_count,
                                       int gus_party_index = 0) noexcept;

// Layout da faixa CTB (D4): preenche ate 5 celulas, comecando em kCtbStripLeft (a direita
// do cockpit). queue_len = total de atores na fila; a celula 0 e o ATIVO e a celula 1 vira
// "proximo" (is_next; so com >1 ator); se queue_len > 5, a 5a celula vira "+N" (overflow).
// Celulas alem de queue_len ficam vazias. So GEOMETRIA + flags: quem e quem vem de
// ctb_window() no consumidor (janela rotacionada a partir do ator ativo).
[[nodiscard]] CtbStrip ctb_strip(int queue_len) noexcept;

// --- Zonas da variante C (todas em px logico) ---

// Retangulo do COCKPIT lateral esquerdo (painel do ator ativo + menu de verbos).
[[nodiscard]] Rect cockpit_rect() noexcept;

// Retangulo do RETRATO grande do ator ativo no topo do cockpit (centrado na largura).
[[nodiscard]] Rect cockpit_portrait_rect() noexcept;

// Retangulo da BARRA DE HP do cockpit (largura util do cockpit), logo abaixo do retrato+nome.
[[nodiscard]] Rect cockpit_hp_bar_rect() noexcept;

// Ponto (x,y) de origem da linha de pips de AP no cockpit (abaixo do HP).
[[nodiscard]] Vec2 cockpit_ap_pips_origin() noexcept;

// Ponto (x,y) de origem da linha de pips de Mana no cockpit (abaixo do AP).
[[nodiscard]] Vec2 cockpit_mana_pips_origin() noexcept;

// Zona do MENU de verbos no cockpit: os 6 verbos sao EMPILHADOS verticalmente nessa
// faixa (parte de baixo do cockpit). O battle_menu distribui os itens dentro dela.
[[nodiscard]] Rect cockpit_menu_zone() noexcept;

// Faixa PROPRIA do BANNER de turno (acima dos atores; nao invade ninguem).
[[nodiscard]] Rect arena_banner_rect() noexcept;

// Retangulo da AREA da arena (onde os atores vivem), a direita do cockpit.
[[nodiscard]] Rect arena_rect() noexcept;

// Retangulo do TERMINAL/log fino no rodape (a direita do cockpit).
[[nodiscard]] Rect log_panel_rect() noexcept;

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_LAYOUT_HPP
