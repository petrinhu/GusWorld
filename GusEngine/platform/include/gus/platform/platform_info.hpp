// gus/platform/platform_info.hpp
// Stub da fronteira Qt (M0). Existe so para provar que a camada platform/
// compila e linka contra o Qt antes de janela/render/input chegarem (M1+).
// Esta e a UNICA camada (com app/) autorizada a incluir <Q...>.
#ifndef GUS_PLATFORM_PLATFORM_INFO_HPP
#define GUS_PLATFORM_PLATFORM_INFO_HPP

#include <QString>

namespace gus::platform {

// Retorna a versao do Qt em runtime (qVersion()), confirmando o link Qt.
QString qt_runtime_version();

}  // namespace gus::platform

#endif  // GUS_PLATFORM_PLATFORM_INFO_HPP
