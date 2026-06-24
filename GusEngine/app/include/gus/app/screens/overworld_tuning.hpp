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

    // --- CAMERA (ZOOM) -----------------------------------------------------
    // PONTO UNICO DO ZOOM (lider calibra vendo o display). Quantos PIXELS de tela
    // cada TILE do mapa ocupa. O lider pensa em "tile = N px" (estilo Zelda ALttP /
    // Stardew), INDEPENDENTE do tile_size em unidades de mundo do .gmap (o overworld
    // converte: px-por-unidade = camera_zoom_px_per_tile / tile_size). A camera
    // mostra (pixels_da_janela / px-por-unidade) unidades de mundo e CENTRA no Gus,
    // rolando quando o mapa e maior que a tela.
    //
    // ESTE NUMERO ERA O BUG DO M4: antes a camera tratava 1 unidade de mundo == 1 px
    // (zoom ausente). Com o .gmap de tile_size 2.0, o mapa de 30x20 tiles = 60x40
    // unidades virava um retangulo de ~60x40 px no centro de uma tela 1280x720. No
    // M1 a cena de teste usava tile_size 32, que por coincidencia dava ~32 px/tile e
    // "parecia ok". Agora o zoom e explicito.
    //
    // 43 px/tile @1280x720 (tile_size 2.0 => px-por-unidade ~21.5): visao ~29.8x16.7
    // tiles (~59.5x33.5 unidades); o mapa 60x40 unidades ROLA nos dois eixos seguindo o
    // Gus. O sprite do Gus (player_sprite_height_tiles ~2.75 => ~118 px) ocupa ~1/6 da
    // altura da tela (estilo Zelda ALttP / Stardew). Faixa util ~24..64: maior = mais
    // perto (personagem maior, rola mais); menor = mais longe (mostra mais mundo).
    //
    // LIDER 2026-06-23 (FEEL no display): 48 -> 43 (-10%), zoom estava "um pouco grande".
    // Ponto unico: mexer SO neste numero reescala a camera inteira (sem tocar logica).
    float camera_zoom_px_per_tile = 43.0f;

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

    // --- CADENCIA DO WALK CYCLE (troca de quadro do sprite) ----------------
    // PONTO UNICO da cadencia da caminhada (lider calibra vendo o display). Quanto o
    // Gus precisa andar para o sprite TROCAR UM QUADRO, em FRACAO DE TILE (NAO em px
    // absolutos). RELATIVO a escala do mundo de proposito: o overworld multiplica por
    // grid_.tile_size() para virar unidades de mundo, entao a passada parece NATURAL
    // EM QUALQUER tile_size (cena de teste do M1 tile 16/32 OU o .gmap real tile 2.0).
    //
    // ESTE NUMERO ERA O BUG DO WALK (irmao do bug do zoom): antes a cadencia era ABSOLUTA
    // (~8 px), calibrada para o tile 16 do M1. No .gmap real (tile_size 2.0) a velocidade
    // 4.5 tiles/s = 9 unidades/s dava ~0.15 unidade/frame contra um passo de 8 unidades:
    // o quadro quase nunca trocava, o Gus DESLIZAVA sem dar passos. Agora a cadencia e
    // por fracao de tile, imune a escala.
    //
    // LIDER 2026-06-23 (FEEL no display): foot-slide (o pe PATINA - a troca de quadro nao
    // casava com a translacao) + os passos pareciam LENTOS/arrastados. A correcao e troca
    // de quadro MAIS RAPIDA (cadencia MENOR), nao mais lenta: cadencia 0.30 -> 0.16
    // tile/quadro. Ciclo de 7 quadros do Gus span ~1.12 tile (era ~2.1): passada VISUAL
    // bem mais curta -> o pe "agarra" o chao com mais frequencia e o walk fica ENERGICO
    // (estilo Zelda ALttP / Stardew). A 4.5 tiles/s troca ~28 quadros/s -> ~4 ciclos/s.
    // Faixa util ~0.12..0.30: MAIOR = passada mais longa (troca menos, desliza/arrasta
    // mais); MENOR = passada mais curta e energica (troca mais rapido). Ponto unico:
    // afinar SO este numero no display.
    //
    // LIMITE DA ARTE (diagnostico engine-graphics 2026-06-23): a arte de 7 quadros do
    // walk do Gus e um BOB/sway de peso (torso+cabelo balancam), NAO uma passada com
    // CONTATO claro (uma perna plantada a frente, outra atras alternando). Sem keyframe
    // de pe-plantado pra ancorar a translacao, NENHUMA cadencia ELIMINA 100% o desliza;
    // cadencia menor MINIMIZA (acopla a frequencia do bob a velocidade). Eliminacao total
    // exige RE-ARTE do walk com poses de contato (candidato ARTE-RESP-4DIR/regen).
    //
    // LIDER 2026-06-23 (FEEL no display, ROUND 2): 0.16 ficou FRENETICO ("passos muito
    // rapidos"); o historico mostrou 0.30 = lento/arrastado, 0.16 = rapido demais. Buscado
    // o MEIO-TERMO em CICLOS/s (a leitura fisiologica do passo): a 4.5 tiles/s o ciclo de
    // 7 quadros do Gus span = 7*cadencia tiles, logo ciclos/s = 4.5 / (7*cadencia).
    //   0.30 -> span 2.10 -> 2.14 ciclos/s (arrastado);  0.16 -> span 1.12 -> 4.02 ciclos/s
    //   (frenetico);  0.23 -> span 1.61 -> 2.80 ciclos/s (~19.6 quadros/s) = passo NATURAL,
    // centro da janela pedida (0.22..0.24) e perto da cadencia de caminhada percebida no
    // top-down acelerado. Ponto unico: afinar SO este numero no display.
    float anim_walk_tiles_per_frame = 0.19f;
    // Idem CORRENDO: passada mais LONGA (nao fps maior). Maior que o walk garante "pe
    // colado" na corrida. 0.32 tile/quadro = ~1.4x o walk (0.23*1.4); a 7.2 tiles/s (run
    // 1.6x) span 2.24 -> 3.21 ciclos/s > walk (2.80) = leitura clara de "correndo".
    float anim_run_tiles_per_frame = 0.27f;

    // --- HISTERESE (coast) da animacao de walk (anti-deslize ao SPAMMAR) ---------
    // PROBLEMA (lider 2026-06-23, no display): SPAMMAR o botao de direcao (tap-tap-tap
    // rapido) fazia o Gus DESLIZAR pela tela sem animar os passos. CAUSA: a anim de
    // walk estava acoplada ao INPUT do frame exato - cada micro-gap entre os taps caia
    // no ramo "parado" e RESETAVA o ciclo pro neutro antes de completar um passo, entao
    // o deslocamento real acumulava mas o boneco voltava ao quadro neutro (deslize).
    //
    // CONSERTO (estilo Zelda ALttP / Stardew): a anim segue o ESTADO de movimento, nao o
    // input do frame. Ao soltar, NAO corta seco pro idle - segura o estado "andando" por
    // este BUFFER curto (em SEGUNDOS) antes de cair pro idle. Durante o buffer SEM
    // deslocamento real o quadro e SEGURADO (nao avanca: nada de "marchar parado"). Os
    // micro-gaps do spam ficam dentro do buffer e a anim segue fluida; soltar de verdade
    // (parar alem do buffer) volta ao idle normal.
    //
    // 0.10 s @60fps = ~6 frames de tolerancia: cobre taps humanos rapidos sem o boneco
    // "andar no lugar" perceptivel parado. Faixa util ~0.08..0.16: MAIOR = mais tolerante
    // ao spam (mas arrisca segurar o walk parado um tiquinho a mais); MENOR = corta mais
    // cedo (risco de voltar o deslize no spam muito rapido). 0 = corte seco (sem coast).
    // Ponto unico: afinar SO este numero no display.
    float anim_walk_coast_seconds = 0.10f;

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
    // Amplitude da ESCALA vertical da respiracao calma, em FRACAO (0.0 = DESLIGADA).
    //
    // LIDER 2026-06-23 (FEEL no display, ROUND 2): "ainda estica/achata muito" MESMO em
    // 0.008 (+-0.8%, ~1px) - suspeito, porque 0.8% deveria ser imperceptivel. CAUSA REAL
    // diagnosticada (engine-graphics): a escala e ANCORADA NA BASE (pe plantado, cresce so
    // pra cima), entao e um SQUASH/STRETCH NAO-UNIFORME: a cabeca/cabelo do Gus balanca
    // enquanto os pes ficam cravados. Em pixel-art RIGIDO o olho le QUALQUER escala
    // nao-uniforme como deformacao ELASTICA (gelatina), nao como respiracao - 1px ja
    // chama atencao. NAO era o idle ofegante (winded): aquele so dispara apos CORRIDA real
    // (>=2s de sprint -> WindedTimer), nunca apos caminhada; ao parar de ANDAR o idle e
    // sempre o CALMO procedural. CORRECAO do lider: ZERAR a escala procedural. A
    // respiracao calma fica SO no bob (sobe-desce UNIFORME, sem distorcer o sprite).
    // O mecanismo de escala continua intacto e testado (ligavel >0 a qualquer momento).
    float idle_calm_scale_amplitude = 0.0f;
    // Amplitude do BOB (sobe-desce) da respiracao calma, em TILES. Deslize vertical do
    // desenho INTEIRO (UNIFORME, nao deforma): unico componente da respiracao calma agora
    // que a escala foi zerada. 0.03 tile * tile_size 2.0 = 0.06 unidade ~= 1.3px no .gmap:
    // sobe-desce SUTIL e limpo, sem esticar/achatar. Faixa util ~0.02..0.06. 0 desliga.
    float idle_calm_bob_tiles = 0.03f;

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
