#include "source/ui/CalendarDialog/caldial_10_apply_locale_to_calendar.hpp"

#include <QLocale>

#include "source/ui/CalendarDialog/agregador_01_CalendarDialog.hpp"
#include "source/ui/ui_10_BadgeCalendarWidget.hpp"
#include "source/ui/ui_12_CalendarDialog.hpp"
#include "source/eisenhower_app.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"

namespace source::GerenciamentoUI::CalendarDialogModules {

void _apply_locale_to_calendar(source::GerenciamentoUI::CalendarDialog *self)
{
    try {
        if (self == nullptr || self->calendar == nullptr) {
            return;
        }

        QString idioma;
        if (self->app != nullptr && self->app->gerenciador_traducao != nullptr) {
            idioma = self->app->gerenciador_traducao->obter_idioma_atual();
        }

        QLocale locale = QLocale::system();
        if (idioma.startsWith(QStringLiteral("pt"))) {
            locale = QLocale(QLocale::Portuguese, QLocale::Brazil);
        } else if (idioma.startsWith(QStringLiteral("en"))) {
            locale = QLocale(QLocale::English, QLocale::UnitedStates);
        }

        self->calendar->setLocale(locale);
        self->calendar->setFirstDayOfWeek(Qt::Sunday);
    } catch (...) {
        detail::logger()->error(QStringLiteral("Erro ao aplicar localidade no calendário."));
    }
}

}  // namespace source::GerenciamentoUI::CalendarDialogModules
