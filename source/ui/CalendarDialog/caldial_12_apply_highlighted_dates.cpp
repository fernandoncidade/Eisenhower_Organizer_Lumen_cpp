#include "source/ui/CalendarDialog/caldial_12_apply_highlighted_dates.hpp"

#include <QBrush>
#include <QDate>
#include <QFont>
#include <QTextCharFormat>

#include "source/ui/CalendarDialog/agregador_01_CalendarDialog.hpp"
#include "source/ui/ui_10_BadgeCalendarWidget.hpp"
#include "source/ui/ui_12_CalendarDialog.hpp"

namespace source::GerenciamentoUI::CalendarDialogModules {

void _apply_highlighted_dates(source::GerenciamentoUI::CalendarDialog *self)
{
    try {
        if (self == nullptr || self->calendar == nullptr) {
            return;
        }

        const QSet<QString> new_dates = self->_get_task_dates();
        const QSet<QString> to_clear = self->_highlighted_dates - new_dates;
        const QTextCharFormat blank;

        for (const QString &ds : to_clear) {
            const QDate qd = QDate::fromString(ds, Qt::ISODate);
            if (qd.isValid()) {
                self->calendar->setDateTextFormat(qd, blank);
            }
        }

        QTextCharFormat fmt;
        const QColor base = self->palette().highlight().color();
        fmt.setBackground(QBrush(QColor(base.red(), base.green(), base.blue(), 60)));
        fmt.setFontWeight(QFont::Bold);

        for (const QString &ds : new_dates) {
            const QDate qd = QDate::fromString(ds, Qt::ISODate);
            if (qd.isValid()) {
                self->calendar->setDateTextFormat(qd, fmt);
            }
        }

        self->_highlighted_dates = new_dates;
    } catch (...) {
        detail::logger()->error(QStringLiteral("Erro ao aplicar destaque nas datas do calendário."));
    }
}

}  // namespace source::GerenciamentoUI::CalendarDialogModules
