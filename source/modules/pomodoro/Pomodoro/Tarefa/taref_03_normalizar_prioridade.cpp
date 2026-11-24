#include "source/modules/pomodoro/Pomodoro/Tarefa/taref_03_normalizar_prioridade.hpp"

#include <exception>

#include <QHash>
#include <QtGlobal>

namespace source::plugin::Pomodoro {

QString Tarefa::normalizar_prioridade(const QString &valor)
{
    auto *logger = source::utils::LogManager::get_logger();

    try {
        const QString p_iu = QStringLiteral("Importante e Urgente");
        const QString p_inu = QStringLiteral("Importante, mas Não Urgente");
        const QString p_niu = QStringLiteral("Não Importante, mas Urgente");
        const QString p_ninnu = QStringLiteral("Não Importante e Não Urgente");

        if (valor.trimmed().isEmpty()) {
            return p_iu;
        }

        const QString raw = valor.trimmed();
        const QString lower = raw.toLower();

        if (lower.startsWith(QStringLiteral("🔴"))) {
            return p_iu;
        }

        if (lower.startsWith(QStringLiteral("🟠"))) {
            return p_inu;
        }

        if (lower.startsWith(QStringLiteral("🟢"))) {
            return p_ninnu;
        }

        if (lower.startsWith(QStringLiteral("🟡"))) {
            if (lower.contains(QStringLiteral("média")) || lower.contains(QStringLiteral("media")) || lower.contains(QStringLiteral("medium"))) {
                return p_inu;
            }

            return p_niu;
        }

        QString normalized = lower;
        normalized.replace(QStringLiteral("não"), QStringLiteral("nao"));
        normalized.replace(QStringLiteral("ã"), QStringLiteral("a"));
        normalized.replace(QStringLiteral("á"), QStringLiteral("a"));
        normalized.replace(QStringLiteral("â"), QStringLiteral("a"));
        normalized.replace(QStringLiteral("é"), QStringLiteral("e"));
        normalized.replace(QStringLiteral("ê"), QStringLiteral("e"));
        normalized.replace(QStringLiteral("í"), QStringLiteral("i"));
        normalized.replace(QStringLiteral("ó"), QStringLiteral("o"));
        normalized.replace(QStringLiteral("ô"), QStringLiteral("o"));
        normalized.replace(QStringLiteral("ú"), QStringLiteral("u"));
        normalized.replace(QLatin1Char(','), QLatin1Char(' '));
        normalized = normalized.simplified();

        if (normalized.contains(QStringLiteral("importante")) && normalized.contains(QStringLiteral("urgente"))
            && !normalized.contains(QStringLiteral("nao urgente"))) {
            if (normalized.contains(QStringLiteral("nao importante"))) {
                return p_niu;
            }

            return p_iu;
        }

        if (normalized.contains(QStringLiteral("importante")) && normalized.contains(QStringLiteral("nao urgente"))) {
            if (normalized.contains(QStringLiteral("nao importante"))) {
                return p_ninnu;
            }

            return p_inu;
        }

        if (normalized.contains(QStringLiteral("nao importante")) && normalized.contains(QStringLiteral("urgente"))) {
            return p_niu;
        }

        if (normalized.contains(QStringLiteral("nao importante")) && normalized.contains(QStringLiteral("nao urgente"))) {
            return p_ninnu;
        }

        const QHash<QString, QString> legacy = {
            {QStringLiteral("alta"), p_iu},
            {QStringLiteral("high"), p_iu},
            {QStringLiteral("media"), p_inu},
            {QStringLiteral("média"), p_inu},
            {QStringLiteral("medium"), p_inu},
            {QStringLiteral("baixa"), p_ninnu},
            {QStringLiteral("low"), p_ninnu},
        };

        if (legacy.contains(lower)) {
            return legacy.value(lower);
        }

        return p_iu;
    } catch (const std::exception &e) {
        logger->error(QStringLiteral("Erro ao normalizar prioridade: %1").arg(QString::fromUtf8(e.what())), true);
        return QStringLiteral("Importante e Urgente");
    } catch (...) {
        logger->error(QStringLiteral("Erro ao normalizar prioridade"), true);
        return QStringLiteral("Importante e Urgente");
    }
}

int Tarefa::normalizar_dominio(const QVariant &valor)
{
    auto *logger = source::utils::LogManager::get_logger();

    try {
        bool ok = false;
        const int as_int = valor.toInt(&ok);
        if (ok) {
            return qBound(0, as_int, 2);
        }

        const QString text = valor.toString().trimmed().toCaseFolded();
        if (text.startsWith(QStringLiteral("🥇")) || text.contains(QStringLiteral("advanced"))
            || text.contains(QStringLiteral("avançado")) || text.contains(QStringLiteral("avancado"))) {
            return 2;
        }
        if (text.startsWith(QStringLiteral("🥈")) || text.contains(QStringLiteral("intermediate"))
            || text.contains(QStringLiteral("intermediário")) || text.contains(QStringLiteral("intermediario"))) {
            return 1;
        }
        return 0;
    } catch (const std::exception &e) {
        logger->error(QStringLiteral("Erro ao normalizar domínio: %1").arg(QString::fromUtf8(e.what())), true);
        return 0;
    } catch (...) {
        logger->error(QStringLiteral("Erro ao normalizar domínio"), true);
        return 0;
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::TarefaFns {

QString normalizar_prioridade(const QString &valor)
{
    return Tarefa::normalizar_prioridade(valor);
}

int normalizar_dominio(const QVariant &valor)
{
    return Tarefa::normalizar_dominio(valor);
}

}  // namespace source::plugin::Pomodoro::TarefaFns
