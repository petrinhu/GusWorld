// gus/app/maestro_logic.hpp
//
// Logica PURA da Maestro (M7-COSTURA, ADR-012 Onda 1): POCO 100% testavel SEM SDL/janela
// - so as decisoes que a Maestro toma ao orquestrar cidade<->batalha. A classe Maestro
// (gus/app/maestro.hpp, dona da janela/SDL) so CONSOME estas funcoes; a logica em si fica
// testavel headless, no mesmo padrao das funcoes-livres battle_* (ver
// gus/app/screens/battle_preview.hpp + app/tests/battle_key_routing_test.cpp).
//
// Escopo do Incremento 1 (esqueleto do loop, ver ADR-012 Onda 1):
//   - 1 inimigo FIXO no mapa (AABB+id, dado de app/ - NAO toca o formato .gmap/TileMap,
//     que nao tem nocao de inimigo).
//   - colisao jogador<->inimigo dispara a troca cidade->batalha.
//   - Victory marca o inimigo derrotado (some do mapa); Defeat/Fled/Ongoing (janela
//     fechada no meio) NAO marcam - o inimigo continua la, o jogador tenta de novo. O
//     FLAVOR da derrota (reboot/bark/tela-xadrez, M7-COSTURA Inc 3, FECHADO) vive na
//     BattleScene/battle_preview (gus/app/screens/battle_scene.hpp::defeat_flavor_active)
//     - AQUI e so o roteamento binario "o inimigo some ou nao", que nunca mudou.

#ifndef GUS_APP_MAESTRO_LOGIC_HPP
#define GUS_APP_MAESTRO_LOGIC_HPP

#include "gus/core/spatial/grid_collision.hpp"  // Aabb
#include "gus/core/spatial/tile_grid.hpp"       // TileGrid (posicionamento do inimigo)
#include "gus/domain/combat/combat_enums.hpp"   // CombatOutcome
#include "gus/platform/audio/audio_engine.hpp"  // AudioEngine/SoundId (M7-COSTURA Inc 2)

namespace gus::app {

// Identidade do encontro (item 1 do escopo do M7: "1 inimigo FIXO"). Nasce parametrizado
// (enum, nao bool) pra crescer sem reescrever o contrato de to_battle()/on_battle_result()
// quando uma onda futura adicionar mais encontros - so este 1 valor e exercitado agora.
enum class EncounterId : int {
    kFixedEnemy1 = 0,
};

// true se os AABBs a e b (canto sup-esq + w/h) se sobrepoem (meio-aberto: bordas
// coincidentes NAO contam como sobreposicao). POCO puro, SEM SDL - a versao PUBLICA/
// testavel do mesmo teste que overworld_sim.cpp ja faz internamente (anonymous
// namespace, sem duplicar a formula "as cegas": a Maestro reusa esta).
[[nodiscard]] bool aabb_overlaps(const gus::core::spatial::Aabb& a,
                                  const gus::core::spatial::Aabb& b) noexcept;

// true se o jogador deve ENTRAR EM BATALHA agora: o inimigo NAO foi derrotado ainda E o
// AABB do jogador sobrepoe o AABB do inimigo. Encapsula a regra "esbarrar dispara
// batalha; inimigo derrotado nunca mais dispara" (item 4 do escopo do Incremento 1).
[[nodiscard]] bool should_trigger_battle(const gus::core::spatial::Aabb& player,
                                          const gus::core::spatial::Aabb& enemy,
                                          bool enemy_defeated) noexcept;

// Roteamento outcome -> acao (item 4 do escopo): Victory marca o inimigo derrotado;
// qualquer outro resultado (Defeat, Fled, ou Ongoing se a janela fechou no meio do
// combate) NAO marca.
[[nodiscard]] bool outcome_marks_enemy_defeated(
    gus::domain::combat::CombatOutcome outcome) noexcept;

// Escolhe a posicao do inimigo FIXO: parte da celula de (player_start.x+w/2,
// player_start.y+h/2) + o offset cardinal (offset_tiles_x, offset_tiles_y) em CELULAS, e
// VARRE em espiral (raio crescente) pela celula ALCANCAVEL A PE mais proxima daquele
// alvo - "alcancavel" = mesma componente conectada do spawn (flood-fill 4-conectado
// sobre TileGrid::is_blocked, a MESMA nocao de "andavel" que o movimento real do
// jogador usa via resolve_move/resolve_move_with_corner_assist). Isto e MAIS FORTE que
// so "celula sem parede": uma celula pode ser chao livre mas estar isolada numa sala
// sem porta (bug real corrigido no M7-COSTURA - o offset caia dentro de uma sala
// fechada de distritos_inferiores.gmap, impossivel de alcancar andando); agora o
// inimigo NUNCA cai numa celula que o jogador nao consiga pisar partindo do proprio
// spawn. Se nao houver NENHUMA celula alcancavel alem do proprio spawn (mapa
// degenerado), cai de volta na propria celula de spawn do jogador (garantidamente
// alcancavel - e onde ele nasceu). Devolve a AABB do inimigo com o MESMO w/h de
// player_start, centrada na celula escolhida. POCO puro (so consome
// TileGrid::is_blocked/world_to_cell/width/height, ja permitido em app/).
[[nodiscard]] gus::core::spatial::Aabb pick_fixed_enemy_position(
    const gus::core::spatial::TileGrid& grid,
    const gus::core::spatial::Aabb& player_start, int offset_tiles_x,
    int offset_tiles_y) noexcept;

// FIX BUG-1 (playtest ao vivo do lider, M7-COSTURA: "a tela de batalha so ativou
// quando toquei o inimigo pelo sul"). Causa raiz: `pick_fixed_enemy_position` devolve
// um AABB MINUSCULO (mesmo w/h do jogador, ~0.6 tile) centrado na celula-alvo - esse
// AABB minusculo era usado DIRETO como hitbox de colisao, mas o SPRITE VISIVEL do
// androide (overworld_sim.cpp, MARCADOR DE INIMIGO FIXO) e desenhado bem maior
// (quad quadrado de player_sprite_height_tiles*tile_size, ~2.75 tiles) com a BASE do
// quad = base do anchor e centrado em X sobre ele - o corpo visivel "vaza" pra cima
// do anchor. So encostar bem no pe do androide (vindo do sul, onde o anchor mora)
// disparava; vindo de qualquer outro lado o jogador via o corpo mas nao tocava a
// hitbox (fantasma).
//
// Esta funcao deriva o AABB REAL (colisao E visual) usando a MESMA formula que o
// desenho usa (ver overworld_sim.cpp: ex = anchor.x + anchor.w*0.5 - esprite_w*0.5;
// ey = sprite_top_y(anchor.y+anchor.h, esprite_h, 0, 0) = base do anchor - esprite_h)
// - por construcao, hitbox e sprite desenhado COINCIDEM exatamente (prova matematica:
// e IDEMPOTENTE re-alimentar este resultado de volta no mesmo calculo de desenho, ver
// maestro_logic_test.cpp). NAO maior que o sprite (evita disparar de longe), NAO menor
// (evita o fantasma) - literalmente o retangulo que o jogador VE.
//
// NOTA HISTORICA (BUG-7, M7-DIALOGO): entre este fix e o de baixo (feet_trigger_aabb),
// existiu uma geracao intermediaria de patch pontual aqui: um parametro opcional
// `sprite_width_fraction` que encolhia SO a LARGURA deste footprint pra compensar o
// alpha-bbox estreito do retrato do Bertoldo (busto centralizado com muito ar
// transparente nas laterais - south.png ocupava so ~31.7% da largura do canvas contra
// ~76.7% da altura). Funcionava, mas era um REMENDO por-sprite: exigia MEDIR o
// alpha-bbox de cada retrato novo (ja mordeu o projeto 2x - o proprio Bertoldo). O
// lider propos a solucao ARQUITETURAL de verdade (ver feet_trigger_aabb abaixo):
// desacoplar de vez a hitbox de ATIVACAO (trigger de combate/dialogo) do footprint
// VISUAL (o retangulo que cobre o sprite inteiro, usado SO pro quad desenhado). Esta
// funcao agora SO serve o footprint visual (sempre quadrado, sprite_height_tiles em
// ambos os eixos) - a responsabilidade de "quando ativa" mudou de dono.
[[nodiscard]] gus::core::spatial::Aabb enemy_sprite_footprint_aabb(
    const gus::core::spatial::Aabb& anchor, float sprite_height_tiles,
    float tile_size) noexcept;

// SOLUCAO ARQUITETURAL (BUG-7 revisitado, decisao do lider apos playtest ao vivo:
// "o Gus visivelmente distante do Bertoldo quando o dialogo ja disparou" mesmo com o
// remendo `sprite_width_fraction` acima). Causa raiz de FUNDO, comum a BUG-1 E BUG-7:
// usar o FOOTPRINT VISUAL inteiro (o retangulo que cobre o corpo/retrato desenhado)
// como hitbox de ATIVACAO confunde DUAS responsabilidades que deveriam ser
// independentes - "o que o jogador VE" (pode ser alto, largo, com margem transparente,
// varia por sprite/pose) e "quando o jogador esta PERTO O BASTANTE pra interagir" (uma
// nocao de PROXIMIDADE FISICA, que nao deveria depender de quanto ar transparente o PNG
// tem ao redor do corpo). A tecnica canonica de RPGs 2D top-down (Zelda, Stardew
// Valley): a area de interacao/colisao-de-encontro e sempre uma caixa PEQUENA ancorada
// nos PES (a base onde o personagem realmente "esta parado" no grid), DESACOPLADA da
// largura/altura do sprite - resolve de vez o problema de precisar medir a
// transparencia de cada sprite individualmente (o que ja mordeu o projeto 2x com o
// Bertoldo: primeiro a escala, depois a largura).
//
// Esta funcao deriva essa caixa a partir do FOOTPRINT VISUAL ja calculado (o mesmo
// `enemy_sprite_footprint_aabb`/anchor usado pro desenho, INALTERADO - nenhuma mudanca
// no tamanho/posicao do sprite na tela) - reusa so a BASE (footprint.y+footprint.h, a
// mesma formula de base que o desenho ja usa) e o CENTRO em X (footprint.x+footprint.w
// *0.5, ja centrado por construcao). kFeetTriggerWidthTiles/kFeetTriggerHeightTiles sao
// da MESMA ordem de grandeza da hitbox real do jogador (ver city_scene.hpp::
// kPlayerHitboxTileFraction=0.6 tile) - nem tao grande quanto o footprint inteiro
// (evitaria o BUG-7 de novo, disparar "de longe"), nem minusculo a ponto de reviver o
// fantasma do BUG-1 (a caixa cobre uma faixa ao redor da base, alcancavel vindo de
// qualquer direcao cardinal, nao so do sul). GENERICA: a MESMA funcao serve o inimigo
// fixo (combate) e o Bertoldo (dialogo) - sem duplicar logica entre os dois call-sites
// (ver maestro.cpp).
inline constexpr float kFeetTriggerWidthTiles = 0.8f;
inline constexpr float kFeetTriggerHeightTiles = 0.4f;

[[nodiscard]] gus::core::spatial::Aabb feet_trigger_aabb(
    const gus::core::spatial::Aabb& sprite_footprint, float tile_size,
    float trigger_width_tiles = kFeetTriggerWidthTiles,
    float trigger_height_tiles = kFeetTriggerHeightTiles) noexcept;

// FIX BUG-3 (playtest ao vivo do lider: "fechei a janela DURANTE a batalha e ela
// reabriu a cidade, virou LOOP INFINITO ate eu dar pkill"). Contrato do run() da
// Maestro: `while (running) { ...; if (should_stop_running_after_battle(to_battle(id)))
// running = false; }`. Extraido como funcao PURA (em vez de deixar so a linha inline em
// maestro.cpp) pra travar o roteamento com um teste headless, sem precisar de janela/
// SDL_Init - Maestro::to_battle() devolve `battle_requested_quit=true` UNICAMENTE
// quando run_battle_preview_embedded sinalizou out_quit_requested (o jogador fechou a
// janela DENTRO da batalha - um sinal DISTINTO de qualquer CombatOutcome); QUALQUER
// outro desfecho (Victory/Defeat/Fled/Ongoing) devolve false daquela funcao, entao esta
// aqui devolve false tambem e o loop da cidade CONTINUA (comportamento de sempre).
[[nodiscard]] constexpr bool should_stop_running_after_battle(
    bool battle_requested_quit) noexcept {
    return battle_requested_quit;
}

// EDGE-TRIGGER (M7-COSTURA BUG-6, playtest ao vivo do lider: "apertei fugir, apareceu
// rapidamente a dungeon - eu estava tocando o inimigo - e automaticamente reabriu a
// arena"). Causa: should_trigger_battle acima e LEVEL-triggered - dispara ENQUANTO
// houver overlap. Na VITORIA nao importa (o inimigo e removido/enemy_defeated_). Mas na
// FUGA e na DERROTA o inimigo PERMANECE e o jogador volta pra cidade AINDA DENTRO da
// hitbox -> should_trigger_battle volta true na hora -> to_battle de novo -> loop.
//
// FIX: a batalha so dispara na TRANSICAO nao-overlap -> overlap (rising edge) - ou seja,
// SO quando ha overlap AGORA e NAO havia no frame anterior. Depois de uma batalha que
// NAO remove o inimigo (fuga/derrota), o chamador (Maestro) marca "was_overlapping=true"
// (o jogador esta em cima), entao o proximo disparo exige SAIR da hitbox e RE-ENTRAR -
// comportamento correto de encontro fixo (fugir, andar em paz, re-encostar redispara).
// POCO puro; o ESTADO (bool do frame anterior) vive no Maestro (maestro.hpp).
[[nodiscard]] constexpr bool should_trigger_battle_on_edge(
    bool overlapping_now, bool was_overlapping_previous_frame) noexcept {
    return overlapping_now && !was_overlapping_previous_frame;
}

// CROSSFADE DE MUSICA (M7-COSTURA Inc 2, ADR-012 decisao 5 + paga a divida do ADR-011
// "fade entre telas"): para a faixa CORRENTE com fade-out e toca next_id com fade-in,
// cronometrados com o "escurinho" - o CHAMADOR (Maestro) dispara isto no instante em
// que o overlay preto (gus/core/anim/fade_transition.hpp) atinge alpha=1 (tela 100%
// preta), ANTES de trocar a tela por baixo. MECANISMO GENERICO: next_id pode ser a
// MESMA faixa que ja tocava (o kit CC0 provisorio desta onda so tem 1 - ver a nota
// honesta em core/asset_paths.hpp/kCityThemeFile) ou uma faixa DIFERENTE quando o kit
// crescer; esta funcao nao assume qual. engine e ponteiro NAO-DONO (mesmo padrao de
// BattleScene::set_audio) - nullptr e no-op seguro (degradacao graciosa, o jogo nunca
// depende de audio pra rodar).
void crossfade_music(gus::platform::audio::AudioEngine* engine,
                      gus::platform::audio::SoundId next_id, bool loop,
                      float fade_seconds);

// ALVO do crossfade cidade->batalha (M7-COSTURA Inc 3, ADR-012): prefere battle_id (o
// tema da ARENA, kBattleThemeFile) quando o load deu certo; cai de volta pra city_id
// (o tema da cidade, que ja esta carregado e tocando de qualquer forma) se battle_id
// veio kInvalidSound - degradacao segura, o mesmo padrao de "nunca crasha por audio
// ausente" do resto da fronteira (crossfade_music/play_music ja no-opam com
// kInvalidSound). POCO puro, extraido pra ser testavel headless SEM SDL_Init/janela
// (a classe Maestro so consome, ver maestro.hpp::to_battle()).
[[nodiscard]] constexpr gus::platform::audio::SoundId battle_crossfade_target(
    gus::platform::audio::SoundId battle_id,
    gus::platform::audio::SoundId city_id) noexcept {
    return (battle_id != gus::platform::audio::kInvalidSound) ? battle_id : city_id;
}

}  // namespace gus::app

#endif  // GUS_APP_MAESTRO_LOGIC_HPP
