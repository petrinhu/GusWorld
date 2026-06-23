// gus/platform/platform_info.hpp
// Info da fronteira de plataforma (pos repivot ADR-008: SDL3). Reporta a versao do
// SDL em runtime - util para diagnostico e prova que o link SDL esta de pe. Esta e
// a UNICA camada (com app/) autorizada a tocar SDL.
#ifndef GUS_PLATFORM_PLATFORM_INFO_HPP
#define GUS_PLATFORM_PLATFORM_INFO_HPP

#include <string>

namespace gus::platform {

// Retorna a versao do SDL em runtime (ex.: "3.4.10"), confirmando o link SDL.
[[nodiscard]] std::string sdl_runtime_version();

}  // namespace gus::platform

#endif  // GUS_PLATFORM_PLATFORM_INFO_HPP
