#include "source/services/MostrarMenuContexto/mmc_10_build_display_and_tooltip.hpp"

#include <QCoreApplication>
#include <QDate>
#include <QDateEdit>
#include <QStringList>

#include "source/eisenhower_app.hpp"

namespace source::InterfaceCore::MostrarMenuContextoModules {
namespace {

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

}  // namespace

QPair<QString, QString> build_display_and_tooltip(source::EisenhowerMatrixApp *app,
                                                  const QString &base_text,
                                                  const QString &date_iso,
                                                  const QString &time_str)
{
    QString display_text = base_text;
    QStringList tooltip_lines;
    QString date_human;

    if (!date_iso.isEmpty()) {
        const QDate qdate = QDate::fromString(date_iso, Qt::ISODate);
        if (qdate.isValid()) {
            date_human = (app != nullptr && app->date_input != nullptr) ? qdate.toString(app->date_input->displayFormat())
                                                                        : qdate.toString(Qt::ISODate);
            if (app != nullptr && app->reserved_space_eisenhower_enabled()) {
                display_text = time_str.isEmpty() ? QStringLiteral("%1 — %2").arg(base_text, date_human)
                                                  : QStringLiteral("%1 — %2 %3").arg(base_text, date_human, time_str);
            }
            tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Data")), date_human);
        }
    }

    if (!time_str.isEmpty()) {
        tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Horário")), time_str);
    }

    return {display_text, tooltip_lines.join(QLatin1Char('\n'))};
}

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
