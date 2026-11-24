#include "source/services/MostrarMenuContexto/mmc_09_base_text_from_item.hpp"

#include <QListWidgetItem>
#include <QRegularExpression>
#include <QVariantMap>

namespace source::InterfaceCore::MostrarMenuContextoModules {

QString base_text_from_item(QListWidgetItem *item)
{
    if (item == nullptr) {
        return QString();
    }

    const QVariantMap data = item->data(Qt::UserRole).toMap();
    const QString base = data.value(QStringLiteral("text")).toString().trimmed();
    if (!base.isEmpty()) {
        return base;
    }

    const QString text = item->text().split(QLatin1Char('\n')).value(0).trimmed();
    const int sep = text.indexOf(QStringLiteral(" — "));
    QString fallback = sep >= 0 ? text.left(sep).trimmed() : text;
    fallback.remove(QRegularExpression(QStringLiteral("^[🥇🥈🥉\\s]+")));
    fallback.remove(QChar(0x0332));
    return fallback.trimmed();
}

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
