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

#include "gus/app/screens/sprite_animation.hpp"  // DiagonalFacing
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

    // --- OLHAR NA DIAGONAL (qual direcao o sprite mostra) ------------------
    // BUG-FIX (lider 2026-06-22): andando pro lado e acionando Norte/Sul, o boneco
    // nao virava (regra antiga = horizontal sempre vence). LastAxisWins faz o
    // boneco VIRAR pro eixo recem-acionado: lado + W/S -> vira N/S; N/S + A/D ->
    // vira pro lado. O lider troca aqui se preferir:
    //   DiagonalFacing::VerticalWins    -> Norte/Sul sempre ganham na diagonal;
    //   DiagonalFacing::HorizontalWins  -> regra legada (lado sempre ganha).
    DiagonalFacing diagonal_facing = DiagonalFacing::LastAxisWins;

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

    // AJUSTE FINO da ancoragem dos pes (M1-BUG.SUL, lider 2026-06-22). O grosso e
    // AUTOMATICO: o jogo MEDE a sobra transparente embaixo de cada sprite (alpha-bbox
    // no load) e desce o desenho ate o PE REAL encostar na base da hitbox - por
    // personagem/direcao, sem numero magico (ver player_sprites_loader + sprite_anchor).
    // Este offset (em TILES) e SOMADO POR CIMA do automatico, so pra refino a gosto:
    // >0 afunda mais (desce a base), <0 levanta. Default 0 = so o automatico (ja cola
    // o pe). NAO mexe na colisao, so no desenho. (Alavancas-irma: player_sprite_height_tiles,
    // ou a altura da hitbox em test_overworld.hpp / kTestPlayerStart.) NAO ha dois
    // mecanismos concorrentes: automatico = base; manual = ajuste somado.
    float sprite_foot_offset_tiles = 0.0f;

    // Px de mundo percorridos por troca de quadro do walk (~8 px no tile 16,
    // locomotion.md). Run usa passada mais longa (run_px_per_frame). Escalavel.
    float anim_walk_px_per_frame = 8.0f;
    float anim_run_px_per_frame = 11.0f;

    // --- IDLE ANIMADO (breathing) ------------------------------------------
    // RESPIRACAO PARADA em CICLOS POR MINUTO (semantico, fisiologico), nao fps cru.
    // O loop de quadros do breathing representa 1 CICLO COMPLETO (inspira + expira):
    // no Gus os 5 quadros vao da postura baixa (f0) ao pico inflado (f2) e voltam (f4),
    // fechando um ciclo ao dar wrap pro f0.
    //
    // NOTA (lider 2026-06-23): o idle agora tem DOIS modos por STAMINA (ver abaixo). A
    // troca de QUADROS do breathing ficou SO no idle OFEGANTE (cansado), num ritmo
    // rapido. O idle CALMO (descansado) NAO troca quadro: e uma senoide procedural
    // (idle_calm_*). Por isso a cadencia do AnimClock vem de idle_tired_breaths_per_minute,
    // nao mais de um unico "breaths_per_minute" calmo (que ficava travado/staccato).

    // FPS derivado para o AnimClock do idle OFEGANTE (cansado), dado o numero de
    // quadros de UM ciclo (loop completo de breathing). Usado SO no idle ofegante,
    // que toca os 5 quadros do breathing num ritmo bem mais rapido que a respiracao
    // calma (ver idle_tired_breaths_per_minute). fps = loop_frames * bpm / 60.
    [[nodiscard]] float idle_fps_for_loop(int loop_frames) const noexcept {
        const int n = loop_frames < 1 ? 1 : loop_frames;
        return static_cast<float>(n) * idle_tired_breaths_per_minute / 60.0f;
    }

    // --- IDLE EM DOIS MODOS por STAMINA/CARGA (lider 2026-06-23) -----------
    // Decisao do lider: a respiracao do idle muda com a CARGA do aparato do Gus.
    //   DESCANSADO (Carga >= tired_threshold): respiracao CALMA, PROCEDURAL - uma
    //       senoide continua e suave (bob/escala) no sprite parado, FLUIDA, SEM trocar
    //       quadro (acaba com o staccato dos 5 quadros lentos). Usa o quadro NEUTRO do
    //       idle (frame 0). Cadencia = idle_calm_breaths_per_minute.
    //   CANSADO (Carga < tired_threshold): respiracao OFEGANTE (overflow do aparato no
    //       corpo) - aI sim toca os 5 quadros do breathing RAPIDO
    //       (idle_tired_breaths_per_minute), comunicando fadiga. Reusa o idle_frames.

    // --- CARGA DO APARATO ("stamina") - NUMEROS CANONICOS ------------------
    // Carga do Tavus-Drive (NAO stamina fisica) - docs/design/mecanicas/stamina.md.
    // Numeros aprovados pelo lider 2026-06-23 (economy-designer), todos em Fibonacci.
    // Correr (Shift) DRENA; PARADO regenera rapido; ANDANDO regenera devagar; a Carga
    // NUNCA trava o deslocamento (regra de ouro). O OverworldSim alimenta o POCO
    // core::Stamina passando o MoveState real (correndo/andando/parado).
    float stamina_max = 89.0f;             // teto e valor inicial (cheio = descansado).
    float run_drain_per_sec = 8.0f;        // Carga perdida/s CORRENDO (cheio -> 0 em ~11s).
    float recover_walk_per_sec = 5.0f;     // Carga ganha/s ANDANDO (devagar; nunca trava).
    float recover_idle_per_sec = 13.0f;    // Carga ganha/s PARADO (rapido; parar recompensa).
    // Abaixo deste valor de Carga (em UNIDADE, ~38% de 89, NAO percentual) o Gus fica
    // CANSADO (idle ofegante / overflow). >= => calmo. Drains curtos quase nunca ofegam.
    float tired_threshold = 34.0f;

    // --- IDLE CALMO (procedural, senoide) ----------------------------------
    // Cadencia da respiracao CALMA, em ciclos/min (humano em repouso ~16). Dirige a
    // senoide do core::BreathOscillator. NAO troca quadro: e bob/escala continuo.
    float idle_calm_breaths_per_minute = 16.0f;
    // Amplitude da ESCALA vertical da respiracao calma, em FRACAO (0.025 = +-2.5%):
    // o sprite estica/encolhe levemente na vertical (peito subindo/descendo). Faixa
    // sugerida do lider: 0.02 a 0.03. 0 desliga a escala.
    float idle_calm_scale_amplitude = 0.025f;
    // Amplitude do BOB (sobe-desce) da respiracao calma, em TILES (0.06 tile ~= 1px
    // no tile 16; ~0.12 no tile 32). Pequeno deslize vertical do desenho. Faixa
    // sugerida 0.03 a 0.12 tile. 0 desliga o bob (so a escala respira).
    float idle_calm_bob_tiles = 0.06f;

    // --- IDLE OFEGANTE (quadros do breathing, rapido) ----------------------
    // Cadencia da respiracao OFEGANTE em ciclos/min: bem mais rapida que a calma pra
    // "comunicar cansaco". Com 5 quadros num ciclo, ~72/min da ~6 fps (5*72/60),
    // pedido do lider ("~6 fps, ofegante"). Dirige o AnimClock do idle ofegante.
    float idle_tired_breaths_per_minute = 72.0f;

    // --- TIMER DE FOLEGO (corpo) vs CARGA (aparato) - lider 2026-06-23 -------
    // PROBLEMA: amarrar a ofegancia so a Carga (Stamina) deixava o Gus ofegar ~2-3 s,
    // porque a Carga regenera RAPIDO ao parar (~13/s). DECISAO do lider: um TIMER DE
    // FOLEGO do CORPO SEPARADO - ao PARAR de correr (apos correr o bastante), o Gus
    // ofega por um MINIMO de 5 s que ESCALA com quanto tempo correu (ate 8 s),
    // INDEPENDENTE de a Carga ja ter recarregado. O idle ofegante e FORCADO enquanto
    // este timer esta ativo (OU a Carga estiver abaixo do limiar). Dirige o POCO
    // core::player::WindedTimer. Ver docs/design/mecanicas/stamina.md.
    float winded_min_seconds = 5.0f;        // piso da ofegancia ao parar. CANON: 5 s.
    float winded_max_seconds = 8.0f;        // teto da ofegancia (corrida longa). CANON: 8 s.
    float winded_run_for_max_seconds = 8.0f;  // correr este tanto (s) atinge o teto.
    float winded_run_threshold_seconds = 2.0f;  // correr menos que isso e parar nao ofega.
};

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_OVERWORLD_TUNING_HPP
