#pragma once

#include <QString>

QString obter_caminho_persistente();
void definir_caminho_persistente_override(const QString &caminho);
void limpar_caminho_persistente_override();
