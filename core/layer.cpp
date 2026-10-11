// core layer: minimal symbol that proves this layer compiles and links.
#include <string_view>

namespace gusworld::core {

[[nodiscard]] std::string_view layer_name() noexcept {
  return "core";
}

}  // namespace gusworld::core
