// gus/platform/src/platform_info.cpp
// Ver header. Reporta a versao do SDL em runtime (pos repivot ADR-008).

#include "gus/platform/platform_info.hpp"

#include <string>

#include <SDL3/SDL_version.h>

namespace gus::platform {

std::string sdl_runtime_version() {
    const int v = SDL_GetVersion();  // major*1000000 + minor*1000 + micro
    const int major = SDL_VERSIONNUM_MAJOR(v);
    const int minor = SDL_VERSIONNUM_MINOR(v);
    const int micro = SDL_VERSIONNUM_MICRO(v);
    return std::to_string(major) + "." + std::to_string(minor) + "." +
           std::to_string(micro);
}

}  // namespace gus::platform
