#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_37_extrair_texto_json.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace source::plugin::MapaMental::ProcessadorIAFns {
namespace {

void append_json_value(const QJsonValue &value, QStringList &linhas, const QString &prefixo, int nivel)
{
    const QString indent(nivel * 2, QLatin1Char(' '));
    if (value.isObject()) {
        const QJsonObject obj = value.toObject();
        for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
            append_json_value(it.value(), linhas, prefixo.isEmpty() ? it.key() : prefixo + QLatin1Char('.') + it.key(), nivel + 1);
        }
    } else if (value.isArray()) {
        const QJsonArray array = value.toArray();
        for (int i = 0; i < array.size(); ++i) {
            append_json_value(array.at(i), linhas, QStringLiteral("%1[%2]").arg(prefixo).arg(i), nivel + 1);
        }
    } else if (!value.isNull() && !value.isUndefined()) {
        linhas.append(QStringLiteral("%1%2: %3").arg(indent, prefixo, value.toVariant().toString()));
    }
}

}  // namespace

QString extrair_texto_json(const QString &caminho)
{
    QFile file(caminho);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    const QByteArray data = file.readAll();
    QJsonParseError erro;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &erro);
    if (erro.error != QJsonParseError::NoError) {
        return QString::fromUtf8(data);
    }

    QStringList linhas;
    if (doc.isObject()) {
        append_json_value(doc.object(), linhas, {}, 0);
    } else if (doc.isArray()) {
        append_json_value(doc.array(), linhas, QStringLiteral("root"), 0);
    }
    return linhas.join(QLatin1Char('\n')).trimmed();
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
