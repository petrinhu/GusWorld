// GusEngine/app/main.cpp
// Ponto de entrada do gusworld_app (overworld jogavel). Pos repivot ADR-008: a
// casca de plataforma e SDL3 (nos possuimos o loop), nao mais Qt.
//
// DOIS MODOS:
//   (1) normal (sem args): SDL_Init -> SdlWindow (janela + renderer + loop proprio)
//       -> o lider joga (move o Caua com WASD/setas/GAMEPAD, desliza nas paredes,
//       camera presa ao mapa, animacao de walk por distancia).
//   (2) --smoke[=N]: modo HEADLESS pro CI/hook. Inicializa tudo, roda N ticks do
//       loop logico (default 120) com input roteirizado, faz 1 render OFFSCREEN
//       (Render2dSdl em modo headless - renderer nulo, sem display nem GPU),
//       imprime um resumo e sai 0 SEM entrar no loop interativo. E o que o
//       tools/check.sh executa com SDL_VIDEODRIVER=dummy / SDL_AUDIODRIVER=dummy.
//
// O smoke exercita a MESMA cena (test_overworld.hpp), o MESMO passo de simulacao
// (OverworldSim::step_fixed) e o MESMO renderer (Render2dSdl) que a janela - so
// troca o loop interativo por um for de N ticks e a janela por um render headless.

#include <SDL3/SDL.h>

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "gus/app/sdl_window.hpp"
#include "gus/app/screens/anim_catalog.hpp"  // resolve_gus_sprites_dir
#include "gus/app/screens/anim_preview.hpp"
#include "gus/app/screens/city_loader.hpp"   // load_city_or_fallback (cena real headless)
#include "gus/app/screens/overworld_sim.hpp"
#include "gus/app/screens/player_sprites_loader.hpp"
#include "gus/core/spatial/grid_collision.hpp"
#include "gus/core/time/fixed_timestep.hpp"
#include "gus/core/version.hpp"
#include "gus/domain/map/map_csv.hpp"
#include "gus/domain/map/map_serializer.hpp"
#include "gus/platform/render2d/render2d_sdl.hpp"

namespace {

// --- modo ferramenta: compilar mapa CSV -> .gmap selado ---------------------
//
// O I/O de arquivo (ler o .csv, escrever o .gmap) vive SO AQUI, na fronteira app/.
// A logica de transformacao e POCO do domain (parse_csv_to_tilemap + serialize_map).
// Uso: gusworld_app --compile-map <entrada.csv> <saida.gmap>. Saida 0 = ok.
int run_compile_map(const std::string& in_csv, const std::string& out_gmap) {
    std::ifstream in(in_csv, std::ios::binary);
    if (!in) {
        std::cerr << "compile-map: nao consegui abrir o CSV de entrada: " << in_csv
                  << "\n";
        return 1;
    }
    std::ostringstream buf;
    buf << in.rdbuf();
    const std::string csv = buf.str();

    gus::domain::map::TileMap map;
    try {
        map = gus::domain::map::parse_csv_to_tilemap(csv);
    } catch (const std::exception& e) {
        std::cerr << "compile-map: CSV invalido (" << in_csv << "): " << e.what()
                  << "\n";
        return 2;
    }

    std::vector<std::uint8_t> bytes;
    try {
        bytes = gus::domain::map::serialize_map(map);
    } catch (const std::exception& e) {
        std::cerr << "compile-map: falha ao selar o mapa: " << e.what() << "\n";
        return 3;
    }

    std::ofstream out(out_gmap, std::ios::binary | std::ios::trunc);
    if (!out) {
        std::cerr << "compile-map: nao consegui abrir a saida para escrita: "
                  << out_gmap << "\n";
        return 4;
    }
    out.write(reinterpret_cast<const char*>(bytes.data()),
              static_cast<std::streamsize>(bytes.size()));
    if (!out) {
        std::cerr << "compile-map: falha ao escrever o .gmap: " << out_gmap << "\n";
        return 5;
    }

    std::cout << "compile-map OK: " << in_csv << " -> " << out_gmap << " ("
              << map.width() << "x" << map.height() << " tiles, tile_size "
              << map.tile_size() << ", " << bytes.size() << " bytes selados)\n";
    return 0;
}

// Parseia "--compile-map <in.csv> <out.gmap>". Devolve true se pedido; in/out vazios
// = uso incorreto (o chamador reporta).
bool parse_compile_map(int argc, char** argv, std::string& in, std::string& out) {
    for (int i = 1; i < argc; ++i) {
        if (std::string_view(argv[i]) == "--compile-map") {
            in = (i + 1 < argc) ? argv[i + 1] : "";
            out = (i + 2 < argc) ? argv[i + 2] : "";
            return true;
        }
    }
    return false;
}

// Parseia "--smoke" ou "--smoke=N". Devolve true se o modo smoke foi pedido e
// escreve o numero de ticks em out_ticks (default 120). N invalido/ausente -> 120.
// True se "--anim-preview" estiver entre os argumentos. Modo VIEWER: abre uma
// janela que mostra as animacoes do Gus em loop (catalogo varrido em runtime),
// pro lider VER a arte rodando na nossa engine. Reusa o mesmo backend (Render2dSdl).
bool parse_anim_preview(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::string_view(argv[i]) == "--anim-preview") {
            return true;
        }
    }
    return false;
}

bool parse_smoke(int argc, char** argv, int& out_ticks) {
    out_ticks = 120;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);
        if (arg == "--smoke") {
            return true;
        }
        if (arg.rfind("--smoke=", 0) == 0) {
            const std::string n(arg.substr(8));
            try {
                const int v = std::stoi(n);
                if (v > 0) {
                    out_ticks = v;
                }
            } catch (...) {
                // mantem o default
            }
            return true;
        }
    }
    return false;
}

// Roda o smoke HEADLESS: carrega a CENA REAL (cidade dos Distritos Inferiores do
// .gmap, com fallback) tambem no headless, roda N ticks fixos com input roteirizado +
// 1 render no Render2dSdl em modo headless (renderer nulo). Sem display/GPU. Devolve 0
// se tudo ok. Exercita o MESMO caminho de carga da janela (city_loader, inclusive o
// fallback) + o render do tilemap por TileKind.
int run_smoke(int ticks) {
    // Render headless: renderer nulo -> draws viram no-op contabilizado, sprites
    // degradam para o contorno. Prova que a cadeia monta e roda offscreen.
    gus::platform::render2d::Render2dSdl renderer(nullptr);

    // CENA REAL headless: le o .gmap selado e monta a cidade (ou cai no fallback). O
    // I/O + load_map + fallback estao no city_loader (fronteira app/), os mesmos da
    // janela. Loga o que carregou (cidade ou fallback) no proprio loader.
    gus::app::screens::CityLoadOutcome city =
        gus::app::screens::load_city_or_fallback();
    gus::app::screens::OverworldSim sim(std::move(city.sim));

    // Exercita o caminho de SPRITE do GUS tambem no headless: o loader degrada
    // (renderer nulo -> kInvalidTexture) e o sim cai pro contorno. Nao crasha.
    const std::string assets = gus::app::screens::resolve_gus_sprites_dir();
    sim.set_player_sprites(gus::app::screens::load_gus_sprites(renderer, assets));

    gus::core::time::FixedTimestep clock(1.0 / 60.0, 5);
    const float dt = static_cast<float>(clock.fixed_dt());
    for (int i = 0; i < ticks; ++i) {
        sim.step_fixed(/*dx=*/1, /*dy=*/0, /*run=*/false, dt);
    }

    // Um render offscreen (exercita o caminho de render do tilemap, sem display).
    sim.render(renderer, 256.0f, 256.0f, /*alpha=*/0.0f);

    const char* scene = (city.status == gus::app::screens::CityLoadStatus::CityOk)
                            ? "cidade real"
                            : "fallback (cena de teste)";
    const gus::core::spatial::Aabb& p = sim.player();
    std::cout << "GusEngine " << gus::core::engine_version()
              << " smoke OK (SDL): " << ticks << " ticks, cena=" << scene
              << ", jogador em (" << p.x << ", " << p.y << "), "
              << renderer.last_draw_count() << " primitivos desenhados\n";
    return 0;
}

}  // namespace

int main(int argc, char* argv[]) {
    // Modo FERRAMENTA: compila um mapa CSV -> .gmap selado. Headless (sem SDL), I/O
    // de arquivo so aqui na fronteira app/.
    {
        std::string in_csv;
        std::string out_gmap;
        if (parse_compile_map(argc, argv, in_csv, out_gmap)) {
            if (in_csv.empty() || out_gmap.empty()) {
                std::cerr << "uso: gusworld_app --compile-map <entrada.csv> "
                             "<saida.gmap>\n";
                return 64;  // EX_USAGE
            }
            return run_compile_map(in_csv, out_gmap);
        }
    }

    // Modo VIEWER de animacao: --anim-preview. Cuida do proprio SDL_Init/Quit.
    if (parse_anim_preview(argc, argv)) {
        return gus::app::screens::run_anim_preview();
    }

    int ticks = 0;
    const bool smoke = parse_smoke(argc, argv, ticks);

    if (smoke) {
        // Headless: nao precisa nem inicializar video. Roda, valida e sai 0.
        return run_smoke(ticks);
    }

    // Modo normal: inicializa SDL (video + gamepad), abre a janela e roda o loop.
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        std::cerr << "SDL_Init falhou: " << SDL_GetError() << "\n";
        return 1;
    }

    int rc = 0;
    {
        gus::app::SdlWindow window;
        if (!window.init()) {
            std::cerr << "Falha ao inicializar a janela SDL.\n";
            rc = 1;
        } else {
            window.run();  // o lider joga; retorna ao fechar a janela
        }
    }  // window destruido antes do SDL_Quit

    SDL_Quit();
    return rc;
}
