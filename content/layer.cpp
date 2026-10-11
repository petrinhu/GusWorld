// content layer: minimal symbol that proves this layer compiles and links.
#include <string_view>

namespace gusworld::content {

[[nodiscard]] std::string_view layer_name() noexcept {
  return "content";
}

}  // namespace gusworld::content
