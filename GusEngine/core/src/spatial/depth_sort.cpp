// gus/core/spatial/depth_sort.cpp
// Implementacao do Y-sort (M7-COSTURA/M7-DIALOGO). Ver header para o contrato.

#include "gus/core/spatial/depth_sort.hpp"

#include <algorithm>

namespace gus::core::spatial {

void sort_by_depth(DepthEntry* entries, int count) noexcept {
    if (entries == nullptr || count <= 1) {
        return;
    }
    // stable_sort preserva a ordem de entrada em empates exatos de depth_key
    // (determinismo, sem flicker entre itens na mesma linha).
    std::stable_sort(entries, entries + count,
                      [](const DepthEntry& a, const DepthEntry& b) noexcept {
                          return a.depth_key < b.depth_key;
                      });
}

}  // namespace gus::core::spatial
