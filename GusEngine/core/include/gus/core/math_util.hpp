// gus/core/math_util.hpp
// Utilitarios matematicos puros (sem Qt, sem alocacao). POCO testavel headless.
// No M0 expoe apenas clamp, alvo do teste dummy. Mais utils entram conforme
// os subdominios (time/rng/...) forem portados em marcos posteriores.
#ifndef GUS_CORE_MATH_UTIL_HPP
#define GUS_CORE_MATH_UTIL_HPP

namespace gus::core {

// Restringe value ao intervalo fechado [lo, hi].
// Pre-condicao: lo <= hi. Definida em math_util.cpp para dar simbolo real a
// lib estatica (o teste linka contra ela, validando a cadeia de link).
int clamp(int value, int lo, int hi) noexcept;

}  // namespace gus::core

#endif  // GUS_CORE_MATH_UTIL_HPP
