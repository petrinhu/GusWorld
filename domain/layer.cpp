// domain layer: minimal symbol that proves this layer compiles and links.
#include <string_view>

namespace gusworld::domain {

[[nodiscard]] std::string_view layer_name() noexcept {
  return "domain";
}

}  // namespace gusworld::domain
