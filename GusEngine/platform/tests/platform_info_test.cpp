// GusEngine/platform/tests/platform_info_test.cpp
//
// Catch2 do platform_info: prova que a fronteira SDL linka e responde a versao de
// runtime (pos repivot ADR-008). Nao abre janela: SDL_GetVersion e puro.

#include <catch2/catch_test_macros.hpp>

#include "gus/platform/platform_info.hpp"

TEST_CASE("platform_info: versao do SDL nao e vazia e comeca com 3",
          "[platform_info]") {
    const std::string v = gus::platform::sdl_runtime_version();
    REQUIRE_FALSE(v.empty());
    // SDL3: a major version e 3.
    REQUIRE(v.front() == '3');
}
