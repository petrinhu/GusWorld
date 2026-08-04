// SPDX-License-Identifier: Apache-2.0
// gus/app/screens/city_actors.hpp
//
// QUEM POVOA A CIDADE (DEMO-CIDADE-VESTIDA fatia C, a costura).
//
// A fatia B trocou os dois slots fixos de personagem por uma LISTA de atores e deu
// a eles a ronda em rota fixa. Faltava o DADO: quem está no mapa, em que célula, e
// andando por onde. É o que mora aqui, no mesmo espírito da vestimenta
// (city_props.hpp): uma linha por personagem, nenhuma função por personagem.
//
// DOIS GRUPOS, e a diferença entre eles não é técnica, é de papel na história:
//
//   1. Os atores COM PAPEL (Seu Bertoldo, o androide sentinela). Têm diálogo e
//      batalha ligados a eles, e quem os posiciona/arma é a Maestro, que também
//      guarda o estado (conversa aberta, inimigo derrotado). Deles, aqui, mora só
//      a CÉLULA - porque a célula é dado de mundo, e estava espalhada como offset
//      relativo ao spawn dentro da maestro.cpp.
//
//   2. Os FIGURANTES (kDistritosInferioresCast). Entram inteiros por dado: papel,
//      arte, célula e alcance de ronda. Não têm interação; existem para a cidade
//      ter gente. Personagem novo aqui é uma linha, nunca código.
//
// ⚠️ IDENTIDADE DE PERSONAGEM É CANON, E CANON É DO LÍDER. Nenhum nome nesta
// tabela foi inventado: os dois figurantes são NPCs ambientais já canônicos da
// Praça da Compilação (docs/narrative/environments/01-cidade-cyber-gotica.md §6,
// CHARS.md §7) e o androide é o mesmo do encontro do vertical slice. Povoar mais
// esbarra em arte, não em código: os outros tipos de figurante do canon
// (patrulheiro FIR em dupla, técnico da Pilha, criança Janelarum) não têm sprite.
//
// Cross-ref: docs/design/levels/blockout-distritos-inferiores.md §5 (as âncoras de
//            marcação, entre elas o banco do Bertoldo) e §7 (o Pátio da FIR como
//            espaço de patrulha declarado);
//            gus/app/screens/city_patrol.hpp (a rota construída contra o mapa);
//            gus/app/screens/world_entities.hpp (o ator já resolvido);
//            app/tests/city_actors_test.cpp.

#ifndef GUS_APP_SCREENS_CITY_ACTORS_HPP
#define GUS_APP_SCREENS_CITY_ACTORS_HPP

#include <string_view>
#include <vector>

#include "gus/app/screens/world_entities.hpp"
#include "gus/core/asset_paths.hpp"
#include "gus/core/spatial/tile_grid.hpp"
#include "gus/domain/world/patrol_route.hpp"

namespace gus::app::screens {

// ===========================================================================
//  CÉLULAS DOS ATORES COM PAPEL NA HISTÓRIA (consumidas pela Maestro)
//
//  Antes da fatia C isto era um par de offsets em CÉLULAS a partir do spawn,
//  escrito na maestro.cpp: o Bertoldo ficava 5 à esquerda e 13 abaixo de onde o
//  Gus nascesse, fosse lá o que houvesse ali. Funcionava enquanto o mapa cabia em
//  uma tela e meia. No traçado de 90x60 o mesmo offset joga os dois no meio da
//  alameda de chegada, longe do banco e longe da arena.
// ===========================================================================

// Banco do Seu Bertoldo Caím, na Praça da Compilação. É a âncora de marcação
// "banco do Bertoldo" do blockout §5, logo à esquerda de quem entra na praça pela
// boca da alameda - a ordem pedagógica do gold path (§6 passo 3) põe o NPC antes
// da lore e antes do combate.
inline constexpr int kBertoldoBenchCellX = 32;
inline constexpr int kBertoldoBenchCellY = 19;

// Androide sentinela, na arena rebaixada (R15). O encontro do vertical slice
// acontece ali por decisão de design (blockout §6 passo 5 e DA-3), não no caminho
// de chegada: a arena tem cover, pilar central e duas rotas de fuga, e é onde o
// combate foi pensado para ser lido. A célula fica 3 tiles ao sul da boca do
// choke, então o jogador o vê antes de descer.
inline constexpr int kSentinelArenaCellX = 45;
inline constexpr int kSentinelArenaCellY = 37;
// O alcance da ronda dele continua sendo o número do demo
// (city_patrol.hpp::kDemoPatrolReachTiles): seis tiles de vaivém dentro de uma
// arena de 23 de largura cobrem a boca da escadaria sem que ele suma de vista.
// Repetir o valor aqui criaria duas fontes para o mesmo botão de playtest.

// ===========================================================================
//  FIGURANTES
// ===========================================================================

// Uma linha do elenco: papel + arte + célula + alcance de ronda.
struct CityActorRow {
    WorldActorRole role = WorldActorRole::Npc;

    // Pasta e arquivo do sprite, os dois vindos de core/asset_paths.hpp (caminho
    // de asset nunca literal solto). O overworld usa a pose FRONTAL: figurante
    // não tem locomoção em 4 direções nesta onda, do mesmo jeito que o Bertoldo.
    std::string_view sprite_dir{};
    std::string_view sprite_file{};

    int cell_x = 0;
    int cell_y = 0;

    // Tiles para cada lado da célula de origem. Zero = personagem parado (o
    // default seguro). A rota real é construída contra o mapa e ENCURTA na
    // parede, então este número é uma intenção, não uma promessa.
    int patrol_reach_tiles = 0;
};

// ===========================================================================
//  ELENCO DOS DISTRITOS INFERIORES
//
//  ⚠️ PROPOSTA, pendente do aval do líder (quantidade e identidade de personagem
//  é decisão dele). Nada aqui inventa gente: os dois nomes são canon da Praça da
//  Compilação, e o androide extra é o mesmo modelo do encontro.
//
//  Por que só três: povoar quatro telas por cinco pede figurante genérico, e os
//  cinco tipos que o canon descreve (patrulheiro FIR em dupla, técnico da Pilha,
//  vendedor, criança Janelarum, engenheiro aposentado) ainda não têm sprite. Sem
//  arte, a alternativa seria clonar o retrato de um personagem nomeado pela rua,
//  o que é pior que uma cidade discreta.
// ===========================================================================
inline constexpr CityActorRow kDistritosInferioresCast[] = {
    // Vanda do Café (47, vendedora itinerante de café-de-neurônio): empurra o
    // carrinho pelo Núcleo e conhece a rotina de todo mundo. Ronda de 8 tiles na
    // metade oeste da praça, entre o banco do Bertoldo e a placa de lore - é o
    // trecho por onde o gold path passa, então ela é vista mesmo por quem corre.
    {WorldActorRole::Npc, gus::core::assets::kVandaDoCafeSpritesDir,
     gus::core::assets::kNpcSpriteSouthFile, 30, 25, /*patrol_reach_tiles=*/4},

    // Androide de ronda do Pátio da FIR (R19). O blockout declara aquele pátio
    // como "espaço de patrulha em rota fixa", e ele é branch opcional depois do
    // puzzle: encontrar a patrulha ali é recompensa de exploração, não obstáculo
    // do caminho. Vaivém de 12 tiles, o mais longo do mapa, porque o pátio é
    // aberto e a leitura pretendida é "ele está de guarda, dá para esperar
    // passar".
    {WorldActorRole::Enemy, gus::core::assets::kRetratosDir,
     gus::core::assets::kRetratoInimigoFile, 74, 50, /*patrol_reach_tiles=*/6},
};

inline constexpr int kDistritosInferioresCastCount =
    static_cast<int>(std::size(kDistritosInferioresCast));

// Um figurante já conferido contra o mapa: a célula existe e é andável, e a ronda
// já nasceu construída (e encurtada) contra as paredes reais.
struct CityActorPlacement {
    WorldActorRole role = WorldActorRole::Npc;
    std::string_view sprite_dir{};
    std::string_view sprite_file{};
    int cell_x = 0;
    int cell_y = 0;
    gus::domain::world::PatrolRoute route{};
};

// Confere o elenco contra o mapa CARREGADO e devolve quem passou, DESCARTANDO (com
// aviso no terminal) quem caiu fora dos limites ou dentro de parede - personagem
// dentro do prédio é pior que peça dentro do prédio, porque ele anda.
//
// Rota impossível NÃO custa o personagem: ele entra parado (rota inválida), que é
// a mesma degradação de sempre. Perder o ator seria trocar um defeito visível
// ("ele não anda") por um invisível ("cadê ele").
//
// `rows == nullptr` ou `count <= 0` devolve lista vazia.
[[nodiscard]] std::vector<CityActorPlacement> resolve_city_actors(
    const gus::core::spatial::TileGrid& grid, const CityActorRow* rows, int count);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_CITY_ACTORS_HPP
