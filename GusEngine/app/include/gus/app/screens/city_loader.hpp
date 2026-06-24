// gus/app/screens/city_loader.hpp
//
// Fronteira app/: le o .gmap dos Distritos Inferiores DO DISCO (I/O de arquivo so
// AQUI), passa os bytes pro load_map (dominio, PURO), e MONTA o OverworldSim da
// cidade. Se algo falhar (arquivo ausente, selo invalido, versao futura, mapa
// invalido), NAO crasha: loga claro e CAI no fallback (cena de teste do M1), pra a
// janela sempre abrir algo jogavel.
//
// Esta e a UNICA peca que junta I/O de arquivo + load_map + make_city_scene. Tanto a
// janela real (SdlWindow) quanto o smoke headless (main --smoke) a usam, pra os dois
// exercitarem o MESMO caminho de carga (inclusive o fallback).
//
// Cross-ref: gus/app/screens/city_scene.hpp (montagem pura + resolver de caminho),
//            gus/domain/map/map_serializer.hpp (load_map), test_overworld.hpp (fallback).

#ifndef GUS_APP_SCREENS_CITY_LOADER_HPP
#define GUS_APP_SCREENS_CITY_LOADER_HPP

#include <string>

#include "gus/app/screens/overworld_sim.hpp"

namespace gus::app::screens {

// O que aconteceu ao carregar a cidade. CityOk = mapa real carregou; os demais
// significam que caiu no fallback (cena de teste) - o motivo fica no log.
enum class CityLoadStatus {
    CityOk,         // .gmap lido, selado e valido: cena da cidade real montada.
    FileMissing,    // arquivo nao encontrado/ilegivel: fallback.
    LoadRejected,   // load_map recusou (HMAC/Corrupt/VersionTooNew/Invalid): fallback.
};

// Resultado: o sim (cidade real OU fallback) + o status + o caminho tentado (log).
struct CityLoadOutcome {
    OverworldSim sim;
    CityLoadStatus status = CityLoadStatus::FileMissing;
    std::string path;  // caminho do .gmap tentado (pra mensagem clara)
};

// Carrega a cidade: resolve o caminho, le os bytes (I/O), load_map, monta o
// OverworldSim. Em qualquer falha, devolve o sim do FALLBACK (cena de teste do M1) e
// o status correspondente. NAO lanca. O chamador loga e plugga sprites/loop por cima.
[[nodiscard]] CityLoadOutcome load_city_or_fallback();

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_CITY_LOADER_HPP
