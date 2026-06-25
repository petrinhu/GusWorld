// gus/app/screens/battle_layout.hpp
//
// LAYOUT PURO da BattleScreen (M5), POCO 100% testavel SEM SDL nem janela. Calcula
// TODOS os retangulos/pontos da tela de batalha em ESPACO LOGICO 640x360 (D1: base
// pixel-perfect, escala inteira) como FUNCOES PURAS da contagem de atores. A casca de
// render (battle_scene + SdlWindow) projeta esse layout 1:1 num IRenderer dirigindo a
// camera para o retangulo logico 640x360 (px_per_world_unit = 1).
//
// Por que aqui (camada app/, mas POCO): a regra das 4 camadas mantem core/ e domain/
// como POCO puros; a BattleScreen vive em app/. Toda logica "calculavel-sem-janela"
// (onde fica cada slot, a fila CTB, as zonas de HUD) e separada do render SDL e
// testavel headless (Catch2), exatamente como sprite_animation/camera_clamp.
//
// AS DECISOES DE LAYOUT (par.5 de docs/design/mecanicas/battle-screen.md, FECHADAS):
//   D1  resolucao base 640x360, pixel-perfect, escala inteira.
//   D2/D3 arena: COLUNA UNICA de cada lado, espacamento fixo. Party empilha a ESQUERDA
//         (pose leste), inimigos a DIREITA (pose oeste), SEMPRE centralizados no eixo
//         vertical (1 a 4 inimigos, SEM escala dinamica). GUS levemente recuado.
//   D4  fila CTB no TOPO: 5 proximos, celula = retrato 48px + marca de "proximo" no 1o.
//   D5  overlay de COMPILAR (NAO neste incremento): inferior parcial ~40%.
//   D7  camera estatica + highlight do ator ativo; numero flutuante; log so sistema.
//
// Cross-ref: docs/design/mecanicas/battle-screen.md par.2 (zonas) e par.5 (D1-D7);
//            gus/core/spatial/camera_clamp.hpp (Rect/Vec2);
//            gus/app/screens/city_scene.hpp (cena IRMA, mesmo padrao POCO em app/).

#ifndef GUS_APP_SCREENS_BATTLE_LAYOUT_HPP
#define GUS_APP_SCREENS_BATTLE_LAYOUT_HPP

#include <array>

#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Rect / Vec2

namespace gus::app::screens {

// ----------------------------------------------------------------------------
// Constantes de layout (D1-D4). Ponto unico de tuning fino da tela de batalha; o
// lider/ux-ui-designer ajusta AQUI sem tocar no render. Tudo em PIXELS LOGICOS.
// ----------------------------------------------------------------------------

// D1: resolucao base logica (16:9). A janela real escala por inteiro (x2/x3).
inline constexpr int kBattleLogicalW = 640;
inline constexpr int kBattleLogicalH = 360;

// D2/D3: maximo de slots por lado na arena (party <=3, inimigos 1..4). secao 2.
inline constexpr int kMaxPartySlots = 3;
inline constexpr int kMaxEnemySlots = 4;

// D4: a fila CTB mostra os 5 PROXIMOS; celula = retrato 48px nativo (sem downscale).
inline constexpr int kCtbVisibleCells = 5;
inline constexpr int kCtbPortraitPx = 48;

// --- Faixa da fila CTB (topo) ---
inline constexpr int kCtbStripTop = 4;     // margem do topo da tela
inline constexpr int kCtbCellGap = 6;      // espaco horizontal entre celulas
inline constexpr int kCtbStripLeft = 8;    // margem esquerda da faixa

// --- Arena (centro) ---
// Banda vertical util da arena (entre a fila CTB e o painel do ator). Os slots de
// cada lado sao CENTRALIZADOS verticalmente DENTRO desta banda (D3). A banda termina
// ACIMA de kActivePanelTop: nenhum slot pode invadir o painel/menu/log (FIX 2026-06-25,
// lider pegou a coluna de 4 inimigos transbordando pra dentro do menu).
inline constexpr int kArenaTop = 64;       // logo abaixo da faixa CTB (48 + margens)
inline constexpr int kArenaBottom = 250;   // logo acima do painel do ator (kActivePanelTop=252)

// Sprite-base do ator na arena (placeholder retangulo neste incremento; o sprite
// PixelLab entra depois, par.3.4). Largura fixa; ALTURA e o TETO (com poucos atores).
inline constexpr int kActorSlotW = 56;
inline constexpr int kActorSlotH = 64;     // altura MAXIMA (1-2 atores); adapta p/ +atores
inline constexpr int kActorSlotMinH = 36;  // piso da altura adaptativa (legibilidade)

// Espacamento vertical FIXO entre slots empilhados na coluna (D2: espacamento fixo).
inline constexpr int kActorSlotGapY = 6;
// Margem de FOLGA dentro da banda (topo+base) pro empilhamento nao colar nas bordas da
// banda/painel - garante que o slot mais baixo termina ACIMA de kArenaBottom com folga.
inline constexpr int kArenaBandMargin = 4;

// Coluna da party (esquerda) e dos inimigos (direita): X do canto esquerdo do slot.
inline constexpr int kPartyColumnX = 40;
inline constexpr int kEnemyColumnX = kBattleLogicalW - 40 - kActorSlotW;  // = 544

// D3: GUS levemente RECUADO (serve Pillar 4: o fragil). Deslocamento em X (pra dentro,
// ou seja, pra DIREITA na coluna da esquerda) aplicado SO ao slot do Gus.
inline constexpr int kGusRecuoX = 14;

// --- Painel do ator ativo (base) + caixa de log (base) ---
inline constexpr int kActivePanelTop = 252;
inline constexpr int kActivePanelH = 60;
inline constexpr int kLogTop = 314;
inline constexpr int kLogH = kBattleLogicalH - kLogTop - 2;  // ate a base, margem 2
inline constexpr int kHudSideMargin = 6;

// ----------------------------------------------------------------------------
// Saidas do layout. Todos os Rect estao em PIXELS LOGICOS (origem topo-esquerda,
// +X direita, +Y baixo), prontos para o render projetar 1:1.
// ----------------------------------------------------------------------------

using gus::core::spatial::Rect;

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
    bool is_next = false;  // true so na 1a celula ocupada (marca de "proximo")
    bool is_overflow = false;  // true na 5a celula quando ha MAIS de 5 (marca "+N")
    int overflow_count = 0;    // N de "+N" na celula de overflow (0 se nao aplica)
};

// As 5 celulas da fila CTB (D4): proximos atores, ordem esquerda->direita.
struct CtbStrip {
    std::array<CtbCell, kCtbVisibleCells> cells{};
};

// ----------------------------------------------------------------------------
// Funcoes puras de layout. Determinismo total: mesma contagem -> mesmos retangulos.
// ----------------------------------------------------------------------------

// O retangulo logico inteiro da tela (640x360), origem (0,0). Util pro fundo/dim.
[[nodiscard]] Rect battle_screen_rect() noexcept;

// ALTURA ADAPTATIVA do slot de ator dado o numero de slots empilhados numa coluna (FIX
// 2026-06-25): retorna kActorSlotH (teto) com poucos atores, e ENCOLHE quando count
// cresce pra a coluna inteira (count slots + gaps + margem) caber na banda
// [kArenaTop, kArenaBottom] - assim NENHUM slot invade o painel (kActivePanelTop). Piso
// em kActorSlotMinH (legibilidade). count<=0 => kActorSlotH. Pura/deterministica.
[[nodiscard]] int arena_slot_height(int count) noexcept;

// Layout da arena (D2/D3): party_count slots empilhados na coluna ESQUERDA, enemy_count
// na coluna DIREITA, ambos CENTRALIZADOS verticalmente na banda da arena, espacamento
// FIXO. gus_party_index (>=0) marca qual indice de party e o GUS, que recua em X
// (kGusRecuoX); -1 = sem recuo (nenhum slot e o Gus). Conta saturada nos maximos.
[[nodiscard]] ArenaLayout arena_layout(int party_count, int enemy_count,
                                       int gus_party_index = 0) noexcept;

// Layout da faixa CTB (D4): preenche ate 5 celulas. queue_len = total de atores na
// fila do motor; a 1a celula ocupada vira "proximo"; se queue_len > 5, a 5a celula
// vira "+N" (overflow). Celulas alem de queue_len ficam vazias.
[[nodiscard]] CtbStrip ctb_strip(int queue_len) noexcept;

// Retangulo do painel do ator ativo (base): HP/AP/Mana/status + menu de verbos.
[[nodiscard]] Rect active_panel_rect() noexcept;

// Retangulo da caixa de log (base): mensagens de sistema (D7).
[[nodiscard]] Rect log_panel_rect() noexcept;

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_LAYOUT_HPP
