#include "source/services/incore_13_prioridade_display.hpp"

#include <QCoreApplication>
#include <QMap>
#include <QRegularExpression>
#include <QVariant>

#include "source/utils/LogManager.hpp"

namespace source::InterfaceCore {
namespace {

int normalized_domain_value(const QVariant &value)
{
    bool ok = false;
    const int as_int = value.toInt(&ok);
    if (ok) {
        return qBound(0, as_int, 2);
    }

    const QString text = value.toString().trimmed();
    if (text.startsWith(QStringLiteral("🥇"))) {
        return 2;
    }
    if (text.startsWith(QStringLiteral("🥈"))) {
        return 1;
    }
    if (text.startsWith(QStringLiteral("🥉"))) {
        return 0;
    }

    const QString lower = text.toCaseFolded();
    if (lower.contains(QStringLiteral("advanced")) || lower.contains(QStringLiteral("avançado"))
        || lower.contains(QStringLiteral("avancado"))) {
        return 2;
    }
    if (lower.contains(QStringLiteral("intermediate")) || lower.contains(QStringLiteral("intermediário"))
        || lower.contains(QStringLiteral("intermediario"))) {
        return 1;
    }
    return 0;
}

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

}  // namespace

QString prioridade_para_texto(const QVariant &prio, source::EisenhowerMatrixApp *)
{
    try {
        const QMap<int, QString> labels = {
            {1, get_text(QStringLiteral("🔴 Importante e Urgente"))},
            {2, get_text(QStringLiteral("🟠 Importante, mas Não Urgente"))},
            {3, get_text(QStringLiteral("🟡 Não Importante, mas Urgente"))},
            {4, get_text(QStringLiteral("🟢 Não Importante e Não Urgente"))},
        };

        bool ok = false;
        const int pint = prio.toInt(&ok);
        if (ok) {
            return labels.value(pint, labels.value(1));
        }

        const QString s = prio.toString().trimmed();
        for (auto it = labels.constBegin(); it != labels.constEnd(); ++it) {
            if (s == it.value()) {
                return it.value();
            }
        }

        const QString s_no_emoji = QString(s).remove(QRegularExpression(QStringLiteral("^[🔴🟠🟡🟢\\s]+")));
        const QMap<QString, QString> mapping = {
            {QStringLiteral("Importante e Urgente"), labels.value(1)},
            {QStringLiteral("Importante, mas Não Urgente"), labels.value(2)},
            {QStringLiteral("Não Importante, mas Urgente"), labels.value(3)},
            {QStringLiteral("Não Importante e Não Urgente"), labels.value(4)},
        };

        if (mapping.contains(s)) {
            return mapping.value(s);
        }
        if (mapping.contains(s_no_emoji)) {
            return mapping.value(s_no_emoji);
        }
        if (!s.isEmpty()) {
            return get_text(s);
        }
        return QString();
    } catch (...) {
        logger()->debug(QStringLiteral("Erro ao converter prioridade para texto."));
        return prio.toString();
    }
}

QString dominio_para_texto(const QVariant &dominio, bool short_form)
{
    try {
        const QMap<int, QString> short_labels = {
            {0, get_text(QStringLiteral("🥉 Iniciante"))},
            {1, get_text(QStringLiteral("🥈 Intermediário"))},
            {2, get_text(QStringLiteral("🥇 Avançado"))},
        };
        const QMap<int, QString> full_labels = {
            {0, get_text(QStringLiteral("🥉 Iniciante (não consigo explicar)"))},
            {1, get_text(QStringLiteral("🥈 Intermediário (explico com dificuldade)"))},
            {2, get_text(QStringLiteral("🥇 Avançado (explico facilmente)"))},
        };

        const int value = normalized_domain_value(dominio);
        return short_form ? short_labels.value(value, short_labels.value(0)) : full_labels.value(value, full_labels.value(0));
    } catch (...) {
        logger()->debug(QStringLiteral("Erro ao converter domínio para texto."));
        return get_text(QStringLiteral("🥉 Iniciante"));
    }
}

QString dominio_insignia(const QVariant &dominio)
{
    switch (normalized_domain_value(dominio)) {
    case 2:
        return QStringLiteral("🥇");
    case 1:
        return QStringLiteral("🥈");
    default:
        return QStringLiteral("🥉");
    }
}

}  // namespace source::InterfaceCore
