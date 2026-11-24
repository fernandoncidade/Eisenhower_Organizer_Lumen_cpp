#include "source/ui/CalendarDialog/agregador_01_CalendarDialog.hpp"

#include <QCoreApplication>
#include <QDate>
#include <QDateEdit>

#include "source/ui/agregador_01_ui.hpp"
#include "source/ui/ui_12_CalendarDialog.hpp"
#include "source/app/app_02_get_text.hpp"
#include "source/services/incore_13_prioridade_display.hpp"
#include "source/eisenhower_app.hpp"
#include "source/utils/LogManager.hpp"
#include "source/utils/TaskProgressHelper.hpp"
#include "source/modules/gantt/gantt_plugin_interface.hpp"

namespace source::GerenciamentoUI::CalendarDialogModules::detail {

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

QList<QVariantMap> tasks_for_date(const source::GerenciamentoUI::CalendarDialog *self, const QDate &qdate)
{
    QList<QVariantMap> result;
    if (self == nullptr || !qdate.isValid()) {
        return result;
    }

    const QString iso = qdate.toString(Qt::ISODate);
    const QList<QVariantMap> tasks_all = source::GerenciamentoUI::task_utils::collect_tasks_from_app(self->app);
    for (const QVariantMap &task : tasks_all) {
        if (task.value(QStringLiteral("date")).toString() == iso) {
            result.append(task);
        }
    }
    return result;
}

QString tooltip_for_task(const source::GerenciamentoUI::CalendarDialog *self, const QVariantMap &task)
{
    QStringList lines;
    const QString file_path = task.value(QStringLiteral("file_path")).toString();
    if (!file_path.isEmpty()) {
        lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Arquivo")), file_path);
    }

    const QString date_str = task.value(QStringLiteral("date")).toString();
    if (!date_str.isEmpty()) {
        const QDate qd = QDate::fromString(date_str, Qt::ISODate);
        QString format = QStringLiteral("dd/MM/yyyy");
        if (self != nullptr && !self->_date_format.isEmpty()) {
            format = self->_date_format;
        } else if (self != nullptr && self->app != nullptr && self->app->date_input != nullptr) {
            format = self->app->date_input->displayFormat();
        }

        lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Data")),
                                              qd.isValid() ? qd.toString(format) : date_str);
    }

    const QString time_str = task.value(QStringLiteral("time")).toString();
    if (!time_str.isEmpty()) {
        lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Horário")), time_str);
    }

    const QString desc = task.value(QStringLiteral("description")).toString();
    if (!desc.isEmpty()) {
        lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Descrição")), desc);
    }

    const QVariant prio = task.value(QStringLiteral("priority"));
    if (prio.isValid() && !prio.toString().isEmpty()) {
        lines << QStringLiteral("%1: %2")
                     .arg(get_text(QStringLiteral("Prioridade")),
                          source::InterfaceCore::prioridade_para_texto(prio, self != nullptr ? self->app : nullptr));
    }

    const QVariant domain = task.value(QStringLiteral("dominio"));
    if (domain.isValid() && !domain.toString().trimmed().isEmpty()) {
        lines << QStringLiteral("%1: %2")
                     .arg(get_text(QStringLiteral("Nível de Domínio")),
                          source::InterfaceCore::dominio_para_texto(domain, false));
    }

    const double duration_val = task.value(QStringLiteral("duracao"), task.value(QStringLiteral("duration"), 1.0)).toDouble();
    lines << source::app::format_duration_tooltip_line(duration_val > 0.0 ? duration_val : 1.0);

    const int pom = task.value(QStringLiteral("pomodoros_completados"), task.value(QStringLiteral("pomodoros"), 0)).toInt();
    const int c_tot = task.value(QStringLiteral("ciclos_totais"), task.value(QStringLiteral("ciclos_permitidos"), task.value(QStringLiteral("total_ciclos"), 4))).toInt();
    const int safe_c_tot = c_tot > 0 ? c_tot : 4;
    const bool is_done = task.value(QStringLiteral("concluida")).toBool() || task.value(QStringLiteral("status")).toString() == QStringLiteral("Done");
    if (pom > 0 || safe_c_tot > 0) {
        const double pom_pct = source::utils::calculate_task_completion_percentage(pom, safe_c_tot, is_done);
        const QString pom_line_fmt = get_text(QStringLiteral("Ciclos Pomodoro: %1/%2 (%3%)"));
        lines << pom_line_fmt
                     .arg(QString::number(pom),
                          QString::number(safe_c_tot),
                          QString::number(pom_pct, 'f', 0));
    }

    const bool in_prog = task.value(QStringLiteral("em_progresso")).toBool()
        || task.value(QStringLiteral("in_progress")).toBool()
        || task.value(QStringLiteral("em_andamento")).toBool()
        || task.value(QStringLiteral("status")).toString().compare(QStringLiteral("Doing"), Qt::CaseInsensitive) == 0;

    return lines.join(QLatin1Char('\n'));
}

}  // namespace source::GerenciamentoUI::CalendarDialogModules::detail
