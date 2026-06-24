// gus/app/src/screens/city_loader.cpp
//
// Ver header. Junta o I/O de arquivo (.gmap do disco), o load_map (dominio puro) e a
// montagem da cena (city_scene), com FALLBACK pra cena de teste do M1. I/O so aqui.

#include "gus/app/screens/city_loader.hpp"

#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <utility>
#include <vector>

#include "gus/app/screens/city_scene.hpp"
#include "gus/app/screens/test_overworld.hpp"
#include "gus/domain/map/map_serializer.hpp"

namespace gus::app::screens {

namespace {

// Le o arquivo inteiro em bytes. ok=false se nao abriu (ausente/ilegivel).
std::vector<std::uint8_t> read_all_bytes(const std::string& path, bool& ok) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        ok = false;
        return {};
    }
    std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)),
                                    std::istreambuf_iterator<char>());
    ok = in.good() || in.eof();  // eof apos ler tudo e normal
    return bytes;
}

// Texto curto pro log do motivo de recusa do load_map.
const char* describe(gus::domain::map::MapLoadResult r) {
    using R = gus::domain::map::MapLoadResult;
    switch (r) {
        case R::Ok:
            return "ok";
        case R::HmacInvalid:
            return "selo HMAC invalido (mapa adulterado)";
        case R::Corrupt:
            return "binario malformado/truncado";
        case R::VersionTooNew:
            return "schema futuro (binario mais novo que a engine)";
        case R::Invalid:
            return "invariante violada (dimensoes/spawn)";
        case R::IdentityMismatch:
            return "identidade divergente (map_id != esperado: map-swap)";
    }
    return "desconhecido";
}

// Cena de FALLBACK: o mapa de teste hardcoded do M1 (so TileGrid -> render de paredes
// legado). Sempre construivel; garante que a janela abre algo jogavel.
OverworldSim make_fallback() {
    return OverworldSim(make_test_map(), kTestPlayerStart, make_test_tuning());
}

}  // namespace

CityLoadOutcome load_city_or_fallback() {
    const std::string path = resolve_distritos_inferiores_gmap();

    bool read_ok = false;
    const std::vector<std::uint8_t> bytes = read_all_bytes(path, read_ok);
    if (!read_ok) {
        std::cerr << "cidade: nao consegui ler o .gmap (" << path
                  << "); usando a cena de teste (fallback).\n";
        return CityLoadOutcome{make_fallback(), CityLoadStatus::FileMissing, path};
    }

    // BINDING DE IDENTIDADE: o slot da cidade espera o UUID dos Distritos Inferiores.
    // Um .gmap autentico mas de OUTRO mapa (map-swap) e recusado (IdentityMismatch) e
    // cai no fallback, igual a um selo invalido.
    gus::domain::map::MapLoadOutcome loaded =
        gus::domain::map::load_map(bytes, kDistritosInferioresMapId);
    if (loaded.result != gus::domain::map::MapLoadResult::Ok) {
        std::cerr << "cidade: .gmap recusado (" << describe(loaded.result) << "): "
                  << path << "; usando a cena de teste (fallback).\n";
        return CityLoadOutcome{make_fallback(), CityLoadStatus::LoadRejected, path};
    }

    OverworldSim sim = make_city_scene(std::move(loaded.map), make_city_tuning());
    std::cout << "cidade: Distritos Inferiores carregados (" << sim.grid().width()
              << "x" << sim.grid().height() << " celulas, tile_size "
              << sim.grid().tile_size() << ") de " << path << "\n";
    return CityLoadOutcome{std::move(sim), CityLoadStatus::CityOk, path};
}

}  // namespace gus::app::screens
