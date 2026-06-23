// gus/app/screens/overworld_tuning.hpp
//
// ===========================================================================
//  PONTO UNICO DE TUNING do overworld (movimento + camera) do M1.
//  Diretriz do lider: DATA-DRIVEN e FASEAVEL - mexer no feel sem reescrever.
//  TUDO que o lider ajusta vendo o jogo mora AQUI, com nome e comentario.
// ===========================================================================
//
// Header-only (struct de dados puros). O OverworldSim recebe um OverworldTuning;
// a cena de teste (test_overworld.hpp) monta o default. Trocar um numero/flag
// aqui e o suficiente - nenhuma logica precisa ser reescrita.
//
// GANCHOS JA PRONTOS (alguns ligados, outros so esperando o lider virar):
//   - velocidade / corrida   -> LIGADO (o lider so ajusta os numeros);
//   - corner-assist           -> LIGADO (threshold/on-off ajustaveis);
//   - normalize_diagonal      -> GANCHO PRONTO, DESLIGADO (diagonal crua hoje;
//                                virar true normaliza, sem reescrever nada);
//   - camera_zoom             -> GANCHO PRONTO, INERTE (camera ortografica fixa
//                                hoje; o ponto exato de aplicar o zoom esta
//                                comentado em overworld_sim.cpp).

#ifndef GUS_APP_SCREENS_OVERWORLD_TUNING_HPP
#define GUS_APP_SCREENS_OVERWORLD_TUNING_HPP

#include "gus/core/spatial/grid_collision.hpp"  // CornerAssistOptions
#include "gus/platform/render2d/i_renderer.hpp"  // DrawColor

namespace gus::app::screens {

struct OverworldTuning {
    // --- MOVIMENTO ---------------------------------------------------------
    // Velocidade base de caminhada, em TILES por segundo. Placeholder ~4.5.
    float walk_speed_tiles_per_sec = 4.5f;

    // Multiplicador de corrida (Shift). Placeholder 1.6x a caminhada.
    float run_multiplier = 1.6f;

    // GANCHO (DESLIGADO por decisao do lider): normalizar o vetor diagonal.
    //   false (hoje): mover na diagonal cobre ~1.41x a distancia cardinal (cru).
    //   true:         normaliza, deixando a diagonal com a MESMA velocidade das
    //                 cardinais. So virar a flag - o OverworldSim ja le isto.
    bool normalize_diagonal = false;

    // --- COLISAO (corner-correction) --------------------------------------
    // Corner-assist (Stardew/Zelda): escorrega na quina quando ha abertura.
    // .enabled liga/desliga; .max_assist_fraction = quanto perdoa (fracao do tile,
    // default ~0.35). O lider ajusta vendo.
    gus::core::spatial::CornerAssistOptions corner{};

    // --- CAMERA ------------------------------------------------------------
    // GANCHO (INERTE por decisao do lider): zoom da camera ortografica.
    //   1.0 (hoje): sem zoom (a camera atual nao muda).
    //   >1.0:       aproxima (mostra menos mundo); <1.0 afasta. Quando o lider
    //               quiser ligar: dividir o viewport em mundo por camera_zoom nos
    //               DOIS pontos marcados "// ZOOM:" em overworld_sim.cpp. NAO esta
    //               aplicado agora (gancho pronto, camera intacta).
    float camera_zoom = 1.0f;

    // --- CORES PLACEHOLDER (sem arte ainda; o lider ajusta vendo) ----------
    gus::platform::render2d::DrawColor wall_color{0.18f, 0.20f, 0.28f, 1.0f};
    gus::platform::render2d::DrawColor player_color{0.20f, 0.85f, 0.90f, 1.0f};
    // Espessura do contorno do jogador, em unidades de mundo (fallback sem sprite).
    float player_outline_world = 2.0f;

    // --- SPRITE DO JOGADOR (Caua) ------------------------------------------
    // Altura do sprite em TILES (art-director: char ~2.75 tiles; brief: ~3). O
    // sprite e desenhado ancorado nos PES sobre a AABB de colisao (a hitbox e so
    // os pes; o corpo+cabeca "vazam" pra cima). Largura derivada da proporcao do
    // PNG (quadrado 68x68 -> mesma altura). Ajustavel pelo lider.
    float player_sprite_height_tiles = 2.75f;

    // Px de mundo percorridos por troca de quadro do walk (~8 px no tile 16,
    // locomotion.md). Run usa passada mais longa (run_px_per_frame). Escalavel.
    float anim_walk_px_per_frame = 8.0f;
    float anim_run_px_per_frame = 11.0f;
};

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_OVERWORLD_TUNING_HPP
