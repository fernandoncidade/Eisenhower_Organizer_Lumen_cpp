#include "source/services/Arquivo/arq_05_parse_text_date_time.hpp"

#include <QDate>
#include <QDateEdit>
#include <QTime>

#include "source/services/incore_09_Arquivo.hpp"
#include "source/eisenhower_app.hpp"

namespace source::InterfaceCore::ArquivoModules {

ParsedTextDateTime parse_text_date_time(Arquivo *self, const QString &raw_text)
{
    ParsedTextDateTime result{raw_text.trimmed(), QString(), QString()};

    const int sep = result.text.lastIndexOf(QStringLiteral(" — "));
    if (sep < 0) {
        return result;
    }

    const QString left = result.text.left(sep).trimmed();
    const QString tail = result.text.mid(sep + 3).trimmed();

    QStringList formats;
    if (self != nullptr && self->app() != nullptr && self->app()->date_input != nullptr) {
        formats << self->app()->date_input->displayFormat();
    }
    formats << QStringLiteral("dd/MM/yyyy") << QStringLiteral("d/M/yyyy") << QStringLiteral("MM/dd/yyyy")
            << QStringLiteral("M/d/yyyy") << QStringLiteral("yyyy-MM-dd");

    const QStringList parts = tail.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (parts.size() == 2) {
        for (const QString &format : formats) {
            const QDate date = QDate::fromString(parts.at(0), format);
            const QTime time = QTime::fromString(parts.at(1), QStringLiteral("HH:mm"));
            if (date.isValid() && time.isValid()) {
                result.text = left;
                result.date_iso = date.toString(Qt::ISODate);
                result.time_str = time.toString(QStringLiteral("HH:mm"));
                return result;
            }
        }
    }

    for (const QString &format : formats) {
        const QDate date = QDate::fromString(tail, format);
        if (date.isValid()) {
            result.text = left;
            result.date_iso = date.toString(Qt::ISODate);
            return result;
        }
    }

    const QTime time = QTime::fromString(tail, QStringLiteral("HH:mm"));
    if (time.isValid()) {
        result.text = left;
        result.time_str = time.toString(QStringLiteral("HH:mm"));
    }

    return result;
}

}  // namespace source::InterfaceCore::ArquivoModules
