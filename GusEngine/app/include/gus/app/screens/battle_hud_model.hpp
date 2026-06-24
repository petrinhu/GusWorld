// gus/app/screens/battle_hud_model.hpp
//
// MODELO PURO do HUD da BattleScreen (M5, incremento 2): POCO 100% testavel SEM SDL.
// Calcula tudo que o painel do ator ativo, a fila CTB e as barras de HP da arena
// precisam, a partir do ESTADO LIDO do motor (HP/AP/Mana/status), em PIXELS LOGICOS
// (640x360, D1). O render (battle_scene) so consome esses retangulos/fracoes/indices.
//
// SEM TEXTO (decisao reportada ao coordenador no incremento 2): a engine ainda nao tem
// sistema de fonte. Os VALORES numericos sao mostrados de forma FONT-FREE, reversivel:
//   - HP   -> BARRA com preenchimento 0..1 (fracao hp/max_hp);
//   - AP   -> PIPS discretos (max_ap pips; ap acesos), AP e baixo (3);
//   - Mana -> PIPS discretos (max_mana pips, cap kManaCap=8; mana acesos);
//   - Status -> ICONES (assets em resources/sprites/icons-m5/status), 1 por efeito.
// Quando um sistema de fonte entrar, numeros podem se sobrepor a estas barras/pips SEM
// mudar este layout (a barra/pip continua valida). Nada aqui forca dependencia nova.
//
// Cross-ref: docs/design/mecanicas/battle-screen.md par.2/3 (painel/HP/AP/Mana/status);
//            gus/domain/combat/combat_enums.hpp (StatusId);
//            gus/app/screens/battle_layout.hpp (zonas em px logico).

#ifndef GUS_APP_SCREENS_BATTLE_HUD_MODEL_HPP
#define GUS_APP_SCREENS_BATTLE_HUD_MODEL_HPP

#include <string_view>
#include <vector>

#include "gus/core/spatial/camera_clamp.hpp"  // Rect
#include "gus/domain/combat/combat_enums.hpp"  // StatusId

namespace gus::app::screens {

using gus::core::spatial::Rect;
using gus::domain::combat::StatusId;

// Numero de StatusId distintos (Stun..Slow). Mantido em sincronia com o enum: se um
// StatusId novo entrar, atualize aqui E o catalogo de arquivos (status_icon_file).
inline constexpr int kStatusIdCount = 13;

// ----------------------------------------------------------------------------
// Constantes de layout do HUD (px logico). Ponto unico; ux-ui-designer ajusta aqui.
// ----------------------------------------------------------------------------

// Barra de HP do painel do ator ativo.
inline constexpr int kPanelPad = 5;          // padding interno do painel
inline constexpr int kHpBarH = 8;            // altura da barra de HP do painel
inline constexpr int kHpBarW = 120;          // largura da barra de HP do painel

// Pip de recurso (AP/Mana) no painel.
inline constexpr int kPipSize = 7;           // lado do pip quadrado
inline constexpr int kPipGap = 3;            // espaco entre pips
inline constexpr int kResourceRowGap = 4;    // espaco vertical entre linhas (HP/AP/Mana)

// Icone de status no painel (assets sao quadrados; aqui o quadro de exibicao).
inline constexpr int kStatusIconSize = 14;
inline constexpr int kStatusIconGap = 3;

// Mini-barra de HP sob cada ator NA ARENA.
inline constexpr int kArenaHpBarH = 4;       // altura da mini-barra
inline constexpr int kArenaHpBarGapY = 2;    // espaco entre o sprite e a mini-barra

// ----------------------------------------------------------------------------
// Mapeamento StatusId -> arquivo de icone (catalogo unico, ordem do enum).
// ----------------------------------------------------------------------------

// Nome do arquivo PNG do icone de um StatusId (em resources/sprites/icons-m5/status/).
// Determinismo total. Decrypt/Expose NAO tem stat-delta mas tem icone proprio. Stun
// inclusive. Devolve um nome valido pra TODO StatusId do enum (sem default vazio).
[[nodiscard]] std::string_view status_icon_file(StatusId id) noexcept;

// Indice estavel 0..kStatusIdCount-1 de um StatusId (= valor do enum). Util pra o
// render cachear handles por indice (1 load_texture por status) sem map em hot path.
[[nodiscard]] int status_icon_index(StatusId id) noexcept;

// ----------------------------------------------------------------------------
// Calculo de preenchimento e pips.
// ----------------------------------------------------------------------------

// Fracao de preenchimento de uma barra (0..1), clampada. max <= 0 => 0 (sem divisao
// por zero). current negativo => 0; current > max => 1. Deterministica.
[[nodiscard]] float bar_fill(int current, int max) noexcept;

// Retangulo PREENCHIDO de uma barra dado o quadro total e a fracao (0..1). O fill
// cresce da ESQUERDA. fraction fora de [0,1] e clampada. frame e o fundo da barra.
[[nodiscard]] Rect bar_fill_rect(const Rect& frame, float fraction) noexcept;

// Um pip de recurso (AP/Mana): retangulo + se esta ACESO.
struct ResourcePip {
    Rect rect;
    bool lit = false;
};

// Linha de pips a partir de (x,y): total pips, lit acesos (lit <= total). Cada pip
// kPipSize, separados por kPipGap, da esquerda pra direita. total saturado em max_pips
// (guarda contra mana_max absurdo). Pura/deterministica.
[[nodiscard]] std::vector<ResourcePip> resource_pips(float x, float y, int total,
                                                     int lit, int max_pips) noexcept;

// ----------------------------------------------------------------------------
// Mini-barra de HP na arena (sob o sprite do ator).
// ----------------------------------------------------------------------------

// Quadro (fundo) da mini-barra de HP de um ator, dado o retangulo do slot dele na
// arena. Centrada horizontalmente sob o slot, logo abaixo da base do sprite.
[[nodiscard]] Rect arena_hp_bar_frame(const Rect& actor_slot) noexcept;

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_HUD_MODEL_HPP
