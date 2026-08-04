// SPDX-License-Identifier: Apache-2.0
// gus/app/screens/city_props.hpp
//
// VESTIR A CIDADE (DEMO-CIDADE-VESTIDA fatia B1): a ponte entre o DADO do mundo
// (gus::domain::world::ScenePropPlacement, que é só "qual peça, em que célula") e
// as instâncias que o OverworldSim desenha e faz colidir.
//
// Três responsabilidades, todas puras exceto o carregamento de textura:
//   1. resolver onde cada peça vai (validando contra o mapa REAL);
//   2. carregar a arte de cada peça do catálogo (a única parte que toca disco,
//      via IRenderer, com o caminho vindo de core/asset_paths.hpp);
//   3. montar as instâncias com a geometria calculada pelo domínio.
//
// A TABELA É ABSOLUTA E VEM DO LEVEL DESIGN (fatia C, a costura). Até a fatia B
// ela era relativa ao spawn, porque o mapa estava sendo redesenhado de 30x20 para
// 90x60 e qualquer célula fixa escrita naquele momento apontaria para dentro de
// uma parede nova. O traçado de 90x60 chegou, e com ele as âncoras da §5 do
// blockout: cada linha daqui é uma daquelas âncoras, transcrita, não escolhida.
// São 50 âncoras no CSV desde a fatia H (adensamento urbano, 16 fachadas novas) e
// 57 linhas aqui - a diferença são as 14 barricadas, que por convenção não têm
// âncora própria (a peça É a célula de Parede).
//
// Cross-ref: docs/design/levels/blockout-distritos-inferiores.md §2 (convenção de
//            âncora) e §5 (a tabela de peça por âncora);
//            gus/domain/world/scene_prop.hpp (o catálogo, uma linha por peça);
//            gus/app/screens/world_entities.hpp (a instância);
//            app/tests/city_props_test.cpp.

#ifndef GUS_APP_SCREENS_CITY_PROPS_HPP
#define GUS_APP_SCREENS_CITY_PROPS_HPP

#include <vector>

#include "gus/app/screens/world_entities.hpp"
#include "gus/core/spatial/grid_collision.hpp"  // Aabb
#include "gus/core/spatial/tile_grid.hpp"
#include "gus/domain/world/scene_prop.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

namespace gus::app::screens {

// O que a célula da tabela PRECISA ser no mapa para a peça ser aceita.
//
// Existe porque o blockout tem duas convenções de âncora, e uma régua só reprova
// metade das peças: peça de rua (casa, fonte, poste) fica numa célula ANDÁVEL, em
// frente à fachada; caixa de cobertura não fica ao lado de um obstáculo, ela É o
// obstáculo - uma célula de Parede solta que a arte cobre. Validar as duas com a
// régua da primeira descartaria toda cobertura da arena em silêncio.
enum class PropCellRule : int {
    WalkableCell = 0,  // a peça pisa em chão livre (a esmagadora maioria)
    WallCell = 1,      // a peça É a parede daquela célula (caixa de cobertura)
};

// Uma linha da vestimenta: peça + a CÉLULA ABSOLUTA do mapa + a régua de validação.
struct CityPropRow {
    gus::domain::world::ScenePropKind kind{};
    int cell_x = 0;
    int cell_y = 0;
    PropCellRule cell_rule = PropCellRule::WalkableCell;
};

// ===========================================================================
//  VESTIMENTA DOS DISTRITOS INFERIORES (90x60)
//
//  Uma linha por peça, na ORDEM DO PERCURSO (norte para sul), transcrita da §5 do
//  blockout. A coluna "por que ali" mora no doc, não aqui: duplicá-la produziria
//  duas fontes de verdade que divergem na primeira revisão de level design.
//
//  ⚠️ AS CÉLULAS NÃO SÃO ESCOLHIDAS AQUI. São as âncoras (tile Marco) que o
//  traçado já reserva, e um teste as confere contra o .gmap REAL célula a célula
//  ("toda peça de rua pisa numa âncora do level design"). As duas primeiras
//  versões desta tabela foram escritas "no olho" contra o mapa antigo e perderam
//  4 das 10 peças sem o jogo reclamar - só ficava com menos casa.
//
//  Fora desta tabela, de propósito:
//   - as 5 âncoras de MARCAÇÃO sem sprite (pedestal Era 3, banco do Bertoldo,
//     fundo do beco morto, save point, pichação). Duas delas viram POSIÇÃO DE
//     ATOR (ver city_actors.hpp); as outras esperam arte;
//   - o PORTÃO SUL (44,55)/(45,55). A peça do catálogo é sólida e o vão é o único
//     do muro sul: plantá-la hoje sela 28 células (o vestíbulo inteiro, o save
//     point e a saída), e nada no jogo ainda abre portão. O próprio blockout §8
//     pede o vão livre até o bloqueio virar regra de jogo. Decisão do líder
//     pendente; quando houver a lógica de abertura, entram duas linhas aqui.
// ===========================================================================
inline constexpr CityPropRow kDistritosInferioresDressing[] = {
    // --- Chegada: alameda e terraço (R1/R2) ---
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 17, 4},
    {gus::domain::world::ScenePropKind::HoloSterling, 35, 4},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 52, 4},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 72, 4},
    // OS DOIS POSTES QUE SAÍRAM DO MEIO DA RUA (fatia G, playtest do líder: "tem um
    // poste no meio da rua"). Estavam em (40,7) e (48,9) - chão liso absoluto, sem
    // uma parede em nove por cinco células ao redor, dentro da Alameda de Chegada,
    // que é o corredor por onde o jogador entra no jogo. Poste solto em corredor de
    // passagem não é iluminação, é obstáculo sem razão; poste solto na PRAÇA é
    // deliberado (blockout §3 R8: os quatro de (33,15)/(56,15)/(33,27)/(56,27)
    // existem para quebrar o vão de 38x17) e NÃO foi tocado.
    //
    // (40,7) -> (40,4): encosta na face leste do canteiro de (38..39, 3..4). Vira o
    // ESPELHO exato do poste que já existia em (52,4), que encosta na face oeste do
    // canteiro de (53..54, 3..4). Os dois canteiros que a §3 R2 diz existirem "para
    // quebrar a largura" do terraço passam a ter, cada um, a sua luminária - e a
    // alameda continua com âncora, em vez de virar a tela vazia que a §10 existe
    // para evitar.
    //
    // (48,9) -> (50,11): encosta no batente LESTE da boca da praça (a célula de
    // Parede (50,12) do muro R7). Ilumina o único choke de entrada do hub, que é a
    // função declarada daquele muro ("enquadra a primeira visão da fonte"), e é o
    // menor deslocamento que tira o poste do meio do Anel Norte.
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 40, 4},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 50, 11},
    // --- FACHADAS DA RUA DO TERRAÇO (R3/R5, fatia H) --------------------------
    //
    // Decisão do líder: "pode repetir os prédios, como em uma cidade de verdade".
    // Os quarteirões do norte eram massa preta sem uma fachada sequer, e é a
    // SEGUNDA tela do jogo - a primeira rua depois da alameda de chegada.
    //
    // As três regras que escolhem CADA célula abaixo (nenhuma é gosto):
    //  1. a âncora fica na célula de rua imediatamente ABAIXO da face sul de um
    //     quarteirão. Assim a caixa sólida (5,49 x 1,83 tiles na escala 1,83)
    //     cobre uma fileira que JÁ era parede mais uma só de rua - é o placement
    //     mais barato que existe em conectividade, e é o mesmo das casas que já
    //     estavam de pé em (8,21)/(17,21)/(82,21);
    //  2. o desenho (5,49 tiles de altura na térrea, 7,32 na torre) tem que cair
    //     inteiro sobre massa construída. Prédio com rua ANDÁVEL atrás esconde o
    //     jogador que passa por lá - é o defeito conhecido de (73,17), e nenhuma
    //     peça nova daqui repete isso;
    //  3. duas fachadas nunca ficam a menos de 5,5 células em x na mesma fileira,
    //     senão os desenhos se sobrepõem (mesma base = empate de Y-sort).
    //
    // A alternância é IRREGULAR de propósito: duas térreas seguidas a oeste, uma
    // torre isolada na boca da alameda, e duas térreas seguidas a leste. Cidade
    // real não faz A-B-A-B; padrão regular lê mais artificial que repetição.
    {gus::domain::world::ScenePropKind::CasaCibergoticaB, 27, 7},
    {gus::domain::world::ScenePropKind::CasaCibergoticaA, 77, 7},
    {gus::domain::world::ScenePropKind::CasaCibergoticaA, 84, 7},
    {gus::domain::world::ScenePropKind::CasaCibergoticaA, 6, 8},
    {gus::domain::world::ScenePropKind::CasaCibergoticaA, 13, 8},
    {gus::domain::world::ScenePropKind::CasaCibergoticaA, 61, 8},
    // --- Praça da Compilação e os dois bairros (R8/R10/R12) ---
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 33, 15},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 56, 15},
    {gus::domain::world::ScenePropKind::TerminalHack, 69, 15},
    {gus::domain::world::ScenePropKind::CasaCibergoticaB, 73, 17},
    {gus::domain::world::ScenePropKind::CasaCibergoticaA, 8, 21},
    {gus::domain::world::ScenePropKind::CasaCibergoticaB, 17, 21},
    {gus::domain::world::ScenePropKind::FonteLatao, 43, 21},
    {gus::domain::world::ScenePropKind::CasaCibergoticaA, 82, 21},
    {gus::domain::world::ScenePropKind::PlacaLore, 36, 24},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 33, 27},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 56, 27},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 13, 28},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 66, 28},
    // --- Pátio de Sucata e arena (R14/R15) ---
    // Fachada norte do pátio (fatia H): a face sul do quarteirão x4..11/y23..33.
    // Dá fundo construído ao branch oeste, que era muro liso com duas barricadas.
    // O vão útil do pátio fica a oeste da casa (x1..5,4) e no corredor entre as
    // duas pilhas de barricada - medido, não estimado.
    {gus::domain::world::ScenePropKind::CasaCibergoticaA, 8, 34},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 36, 34},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 54, 34},
    // Cobertura: UMA caixa POR CÉLULA de Parede do bloco, de baixo para cima.
    //
    // A convenção do blockout (§2) é que a barricada É a célula de Parede. Os
    // blocos do Pátio de Sucata e da arena têm DUAS células (a §5 chama de "4
    // grupos verticais 1x2 simétricos"), e uma caixa só cobre 1,09 tile de
    // conteúdo: sobrava meio tile de parede nua ACIMA dela, medido pelo laudo
    // visual da fatia D (achado A3). Duas caixas empilhadas cobrem o bloco inteiro
    // e leem como o que a barricada é - caixotes empilhados -, sem tocar em arte,
    // em escala nem no traçado do mapa. Os blocos de UMA célula (Pátio da FIR)
    // seguem com uma caixa só, pelo mesmo critério.
    {gus::domain::world::ScenePropKind::CoverBox, 6, 36, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 6, 37, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 10, 36, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 10, 37, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 38, 36, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 38, 37, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 52, 36, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 52, 37, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 38, 42, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 38, 43, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 52, 42, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 52, 43, PropCellRule::WallCell},
    // --- Travessa Leste (R16, fatia H) ---------------------------------------
    // A rota do explorador descia entre dois paredões pretos de 18 fileiras. Duas
    // fachadas na face sul dos quarteirões x68..75 e x79..85 fazem a travessa ler
    // como rua entre prédios, que é o que ela é. A torre fica no bloco de fora
    // (x79..85), isolada entre térreas - arranha-céu é exceção numa cidade.
    {gus::domain::world::ScenePropKind::CasaCibergoticaA, 71, 36},
    {gus::domain::world::ScenePropKind::CasaCibergoticaB, 82, 36},
    // --- Corredor-puzzle e Pátio da FIR (R17/R19) ---
    // Fachadas dos dois branches pós-puzzle (fatia H). Todas na face sul do muro
    // que já separava o corredor-puzzle dos becos, então nenhuma custa fileira de
    // rua além da própria. A do beco fecha o topo LESTE dele, deixando a pichação
    // de (12,49) livre a oeste; as três da FIR são uma FILEIRA de sobrados iguais,
    // que é como conjunto habitacional de periferia se forma de verdade.
    //
    // ⚠️ O ESPAÇAMENTO DAS TRÊS NÃO É GOSTO, É CONSERTO DE BUG MEDIDO. A primeira
    // tentativa foi 68/74/82 (gaps irregulares, 6 e 8) e o teste "nenhuma célula
    // andável fica ilhada" REPROVOU: a folga entre a casa de 74 e a de 82 caía
    // exatamente em cima da barricada de (78,49), e sobrava um bolsão de rua de
    // 2 sub-fileiras em y47 cercado por casa a oeste, casa a leste, o muro y46 ao
    // norte e a barricada ao sul. Com 68/74/80 as caixas das casas se encostam e
    // não existe folga onde um bolsão possa se formar. Regra que fica: entre duas
    // fachadas vizinhas, a folga NUNCA pode cair sobre uma barricada da fileira
    // de baixo - ou ela vira quarto sem porta.
    {gus::domain::world::ScenePropKind::CasaCibergoticaA, 18, 47},
    {gus::domain::world::ScenePropKind::BoardPuzzle, 42, 47},
    {gus::domain::world::ScenePropKind::CasaCibergoticaA, 68, 47},
    {gus::domain::world::ScenePropKind::CasaCibergoticaA, 74, 47},
    {gus::domain::world::ScenePropKind::CasaCibergoticaA, 80, 47},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 75, 48},
    {gus::domain::world::ScenePropKind::CoverBox, 70, 49, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 78, 49, PropCellRule::WallCell},
    // --- Rua de ligação e antepraça (R20/R20b) ---
    // Fachadas do último respiro antes do portão sul (fatia H). Face sul do muro
    // que separa o corredor-puzzle da antepraça, portanto custo zero de fileira
    // nova. Uma isolada a oeste e um PAR encostado a leste (52 e 58), com a torre
    // no fim - a última silhueta alta antes da saída.
    //
    // ⚠️ NADA entra na faixa x21..29 desta fileira, de propósito: ali a fileira
    // y54 já é parede, e uma casa deixaria a rua de ligação y53 com 2 sub-fileiras
    // úteis. A linha y53 é INVARIANTE de traçado (blockout §3 R20b) - é o que
    // mantém o Beco da Pichação e o Pátio da FIR vivos -, e não se estreita uma
    // invariante para ganhar decoração.
    {gus::domain::world::ScenePropKind::CasaCibergoticaA, 35, 52},
    {gus::domain::world::ScenePropKind::CasaCibergoticaA, 52, 52},
    {gus::domain::world::ScenePropKind::CasaCibergoticaB, 58, 52},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 10, 53},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 33, 53},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 78, 53},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 52, 54},
};

inline constexpr int kDistritosInferioresDressingCount =
    static_cast<int>(std::size(kDistritosInferioresDressing));

// Textura já resolvida de cada peça do catálogo, indexada pelo ScenePropKind.
struct ScenePropTextures {
    gus::platform::render2d::TextureId
        by_kind[gus::domain::world::kScenePropKindCount] = {};

    [[nodiscard]] gus::platform::render2d::TextureId at(
        gus::domain::world::ScenePropKind kind) const noexcept {
        const int i = static_cast<int>(kind);
        if (i < 0 || i >= gus::domain::world::kScenePropKindCount) {
            return gus::platform::render2d::kInvalidTexture;
        }
        return by_kind[i];
    }
};

// Confere a tabela contra o mapa CARREGADO e devolve os placements que passaram,
// DESCARTANDO (com aviso no terminal) o que não cabe: célula fora dos limites, ou
// célula que contraria a régua da linha. Descartar é de propósito: uma casa dentro
// de uma parede é o erro mais difícil de enxergar no playtest, porque a arte fica
// meio enterrada e parece proposital.
// `rows == nullptr` ou `count <= 0` devolve lista vazia.
[[nodiscard]] std::vector<gus::domain::world::ScenePropPlacement> resolve_city_props(
    const gus::core::spatial::TileGrid& grid, const CityPropRow* rows, int count);

// Carrega a arte de TODAS as peças do catálogo no renderer corrente. O caminho de
// cada uma é montado a partir do header central de assets (pasta da área + o
// arquivo da linha do catálogo) - nunca literal espalhado. Arte ausente devolve
// textura inválida naquele slot, e a peça simplesmente não entra no mundo.
[[nodiscard]] ScenePropTextures load_scene_prop_textures(
    gus::platform::render2d::IRenderer& renderer);

// Monta as instâncias prontas para o OverworldSim. Peça sem textura é OMITIDA (e
// não adicionada invisível): peça invisível que bloqueia vira parede fantasma, e
// o jogador trava contra algo que não vê.
[[nodiscard]] std::vector<ScenePropInstance> build_scene_prop_instances(
    const gus::domain::world::ScenePropPlacement* placements, int count,
    float tile_size, float scale, const ScenePropTextures& textures);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_CITY_PROPS_HPP
