#pragma once

#include <QByteArray>
#include <QString>
#include <QStringList>

namespace source::plugin::MapaMental::ProcessadorIAFns {

QByteArray ler_entrada_zip(const QString &arquivo_zip, const QString &entrada);
QStringList listar_entradas_zip(const QString &arquivo_zip);
QString resolver_caminho_zip(const QString &entrada_base, const QString &caminho_relativo);

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
