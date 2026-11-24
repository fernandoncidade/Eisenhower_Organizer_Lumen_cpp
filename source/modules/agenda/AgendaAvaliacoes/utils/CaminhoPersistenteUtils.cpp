#include "CaminhoPersistenteUtils.hpp"

#include <QDir>
#include <QFileInfo>
#include <QDebug>

namespace {
    QString g_caminho_persistente_override;

    QString garantir_diretorio(const QString &caminho) {
        QDir dir(caminho);
        if (!dir.exists() && !dir.mkpath(".")) {
            qWarning().noquote() << QString("Erro ao criar diretorio de configuracao: %1").arg(dir.absolutePath());
        }
        return dir.absolutePath();
    }
} // namespace

QString obter_caminho_persistente() {
    if (!g_caminho_persistente_override.trimmed().isEmpty()) {
        return garantir_diretorio(g_caminho_persistente_override);
    }

    QString base = qEnvironmentVariable("LOCALAPPDATA");
    if (base.trimmed().isEmpty()) {
        base = QDir::homePath() + "/AppData/Local";
    }
    return garantir_diretorio(QDir(base).filePath("AgendaAvaliacoesAcademicas"));
}

void definir_caminho_persistente_override(const QString &caminho) {
    const QString normalizado = caminho.trimmed();
    if (normalizado.isEmpty()) {
        limpar_caminho_persistente_override();
        return;
    }

    g_caminho_persistente_override = QFileInfo(normalizado).absoluteFilePath();
    garantir_diretorio(g_caminho_persistente_override);
}

void limpar_caminho_persistente_override() {
    g_caminho_persistente_override.clear();
}
