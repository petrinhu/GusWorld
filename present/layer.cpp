// present layer: minimal symbol that proves this layer compiles and links.
#include <string_view>

namespace gusworld::present {

[[nodiscard]] std::string_view layer_name() noexcept {
  return "present";
}

}  // namespace gusworld::present
