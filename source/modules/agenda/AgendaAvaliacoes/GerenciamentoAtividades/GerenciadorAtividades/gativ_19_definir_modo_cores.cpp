#include "source/modules/agenda/AgendaAvaliacoes/GerenciamentoAtividades/Gerenciador_Atividades.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/utils/CaminhoPersistenteUtils.hpp"
#include "source/utils/LogManager.hpp"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

bool GerenciamentoAtividades::definir_modo_cores(const QString &codigo_ou_modo, const QString &modulo_param) {
    QString modo;
    QString target_module;

    const QString lower_code = codigo_ou_modo.trimmed().toLower();
    if (lower_code.startsWith(QStringLiteral("preto_"))) {
        modo = QStringLiteral("preto");
        target_module = lower_code.mid(6);
    } else if (lower_code.startsWith(QStringLiteral("coloridas_"))) {
        modo = QStringLiteral("coloridas");
        target_module = lower_code.mid(10);
    } else if (lower_code == QStringLiteral("preto") || lower_code == QStringLiteral("coloridas")) {
        modo = lower_code;
        target_module = modulo_param.trimmed().toLower();
        if (target_module.isEmpty()) {
            target_module = QStringLiteral("geral");
        }
    } else {
        return false;
    }

    if (modo != QStringLiteral("preto") && modo != QStringLiteral("coloridas")) {
        return false;
    }

    if (target_module == QStringLiteral("eisenhower")) {
        modo_cores_eisenhower = modo;
    } else if (target_module == QStringLiteral("pomodoro")) {
        modo_cores_pomodoro = modo;
    } else if (target_module == QStringLiteral("feynman")) {
        modo_cores_feynman = modo;
    } else if (target_module == QStringLiteral("geral") || target_module.isEmpty()) {
        modo_cores_eisenhower = modo;
        modo_cores_pomodoro = modo;
        modo_cores_feynman = modo;
        modo_cores = modo;
    } else {
        return false;
    }

    QDir dir(obter_caminho_persistente());
    dir.mkpath(".");
    QFile file(dir.filePath("agenda_config.json"));
    QJsonObject obj;
    if (!file.exists()) {
        file.setFileName(dir.filePath("config.json"));
    }
    if (file.open(QIODevice::ReadOnly)) {
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        if (document.isObject()) {
            obj = document.object();
        }
        file.close();
    }
    file.setFileName(dir.filePath("agenda_config.json"));
    obj.insert("modo_cores", modo_cores);
    obj.insert("modo_cores_eisenhower", modo_cores_eisenhower);
    obj.insert("modo_cores_pomodoro", modo_cores_pomodoro);
    obj.insert("modo_cores_feynman", modo_cores_feynman);

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    } else {
        source::utils::LogManager::get_logger()->error(QString("Erro ao salvar modo de cores da agenda: %1").arg(file.errorString()));
    }
    return true;
}
