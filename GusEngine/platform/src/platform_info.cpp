// gus/platform/platform_info.cpp
#include "gus/platform/platform_info.hpp"

#include <QtGlobal>  // qVersion()

namespace gus::platform {

QString qt_runtime_version() {
    return QString::fromLatin1(qVersion());
}

}  // namespace gus::platform
