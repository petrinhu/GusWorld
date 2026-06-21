// gus/core/version.hpp
// Versao da engine, exposta como POCO puro (sem Qt). Usada como ponto de
// ancoragem do teste dummy do M0 e como fonte unica da versao da engine.
#ifndef GUS_CORE_VERSION_HPP
#define GUS_CORE_VERSION_HPP

#include <string_view>

namespace gus::core {

// Componentes numericos da versao da engine (semver).
inline constexpr int kEngineVersionMajor = 0;
inline constexpr int kEngineVersionMinor = 1;
inline constexpr int kEngineVersionPatch = 0;

// String da versao da engine, ex: "0.1.0". Definida em version.cpp para
// garantir que a lib estatica tenha simbolo real (e o link seja exercitado).
std::string_view engine_version() noexcept;

}  // namespace gus::core

#endif  // GUS_CORE_VERSION_HPP
