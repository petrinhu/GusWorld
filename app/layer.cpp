// app layer: minimal symbol that proves this layer compiles and links.
#include <string_view>

namespace gusworld::app {

[[nodiscard]] std::string_view layer_name() noexcept {
  return "app";
}

}  // namespace gusworld::app
