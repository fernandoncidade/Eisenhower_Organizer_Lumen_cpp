#include "source/ui/CalendarDialog/caldial_11_get_task_dates.hpp"

#include <QDate>

#include "source/ui/CalendarDialog/agregador_01_CalendarDialog.hpp"
#include "source/ui/ui_12_CalendarDialog.hpp"

namespace source::GerenciamentoUI::CalendarDialogModules {

QSet<QString> _get_task_dates(const source::GerenciamentoUI::CalendarDialog *self)
{
    QSet<QString> dates;

    try {
        if (self == nullptr) {
            return dates;
        }

        for (const QVariantMap &task : self->_collect_tasks()) {
            const QString ds = task.value(QStringLiteral("date")).toString();
            if (ds.isEmpty()) {
                continue;
            }

            const QDate qd = QDate::fromString(ds, Qt::ISODate);
            if (qd.isValid()) {
                dates.insert(qd.toString(Qt::ISODate));
            }
        }
    } catch (...) {
        detail::logger()->error(QStringLiteral("Erro ao obter datas das tarefas do calendário."));
    }

    return dates;
}

}  // namespace source::GerenciamentoUI::CalendarDialogModules
