// GusEngine/app/main.cpp
// Ponto de entrada do executavel gusworld_app (marco M0, andaime).
//
// Objetivo unico nesta fase: provar que a cadeia de build/link esta inteira,
// ou seja, que core/ + domain/ + platform/ (Qt) linkam num binario que roda.
// Imprime a versao da engine, a versao do Qt em runtime e o rotulo do dominio,
// e retorna 0. Sem janela: a janela (platform/window) chega no M1.

#include <QCoreApplication>
#include <QtGlobal>

#include <iostream>

#include "gus/core/version.hpp"
#include "gus/domain/domain_info.hpp"
#include "gus/platform/platform_info.hpp"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "GusEngine " << gus::core::engine_version()
              << " (andaime M0)\n";
    std::cout << "Qt runtime: "
              << gus::platform::qt_runtime_version().toStdString() << "\n";
    std::cout << "Dominio: " << gus::domain::domain_label()
              << " (save schema v" << gus::domain::kSaveSchemaVersion << ")\n";

    // Sem event loop ainda: nada a processar, saida imediata e limpa.
    return 0;
}
