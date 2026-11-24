#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_25_prioridade_display.hpp"

#include <QCoreApplication>

#include "source/utils/LogManager.hpp"

namespace source::plugin::Pomodoro::GerenciadorTarefasFns {
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

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

}  // namespace

QString get_text(const QString &text)
{
    return QCoreApplication::translate("PomodoroPlugin", text.toUtf8().constData());
}

QString prioridade_para_texto(const QVariant &prio)
{
    try {
        const QMap<int, QString> labels = {
            {1, get_text(QStringLiteral("🔴 Importante e Urgente"))},
            {2, get_text(QStringLiteral("🟠 Importante, mas Nao Urgente"))},
            {3, get_text(QStringLiteral("🟡 Nao Importante, mas Urgente"))},
            {4, get_text(QStringLiteral("🟢 Nao Importante e Nao Urgente"))},
        };

        bool ok = false;
        const int as_int = prio.toInt(&ok);
        if (ok) {
            return labels.value(as_int, get_text(QStringLiteral("🔴 Importante e Urgente")));
        }

        const QString text = prio.toString().trimmed();
        for (const QString &value : labels) {
            if (text == value) {
                return value;
            }
        }

        QString no_emoji = text;
        while (!no_emoji.isEmpty() && QStringLiteral("🔴🟠🟡🟢").contains(no_emoji.front())) {
            no_emoji.remove(0, 1);
        }
        no_emoji = no_emoji.trimmed();

        const QMap<QString, QString> mapping = {
            {QStringLiteral("Importante e Urgente"), labels.value(1)},
            {QStringLiteral("Importante, mas Nao Urgente"), labels.value(2)},
            {QStringLiteral("Importante, mas Não Urgente"), labels.value(2)},
            {QStringLiteral("Nao Importante, mas Urgente"), labels.value(3)},
            {QStringLiteral("Não Importante, mas Urgente"), labels.value(3)},
            {QStringLiteral("Nao Importante e Nao Urgente"), labels.value(4)},
            {QStringLiteral("Não Importante e Não Urgente"), labels.value(4)},
        };

        if (mapping.contains(text)) {
            return mapping.value(text);
        }

        if (mapping.contains(no_emoji)) {
            return mapping.value(no_emoji);
        }

        if (!text.isEmpty()) {
            return get_text(text);
        }

        return QString();
    } catch (const std::exception &e) {
        logger()->debug(QStringLiteral("Erro ao converter prioridade para texto: %1").arg(QString::fromUtf8(e.what())));
        return QCoreApplication::translate("PomodoroPlugin", prio.toString().toUtf8().constData());
    } catch (...) {
        logger()->debug(QStringLiteral("Erro ao converter prioridade para texto"));
        return QCoreApplication::translate("PomodoroPlugin", prio.toString().toUtf8().constData());
    }
}

QString dominio_para_texto(const QVariant &dominio, bool short_form)
{
    try {
        const QMap<int, QString> short_labels = {
            {0, get_text(QStringLiteral("🥉 Iniciante"))},
            {1, get_text(QStringLiteral("🥈 Intermediario"))},
            {2, get_text(QStringLiteral("🥇 Avancado"))},
        };
        const QMap<int, QString> full_labels = {
            {0, get_text(QStringLiteral("🥉 Iniciante (nao consigo explicar)"))},
            {1, get_text(QStringLiteral("🥈 Intermediario (explico com dificuldade)"))},
            {2, get_text(QStringLiteral("🥇 Avancado (explico facilmente)"))},
        };

        const int value = normalized_domain_value(dominio);
        return short_form ? short_labels.value(value, short_labels.value(0)) : full_labels.value(value, full_labels.value(0));
    } catch (const std::exception &e) {
        logger()->debug(QStringLiteral("Erro ao converter domínio para texto: %1").arg(QString::fromUtf8(e.what())));
        return get_text(QStringLiteral("🥉 Iniciante"));
    } catch (...) {
        logger()->debug(QStringLiteral("Erro ao converter domínio para texto"));
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

}  // namespace source::plugin::Pomodoro::GerenciadorTarefasFns
