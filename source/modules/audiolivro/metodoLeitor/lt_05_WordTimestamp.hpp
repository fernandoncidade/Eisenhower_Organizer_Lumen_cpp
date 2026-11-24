#pragma once

#include <QVariantMap>
#include <QString>

namespace source::plugin::Audiolivro {

struct WordTimestamp {
    QString text;
    qint64 offset_ms = 0;
    qint64 duration_ms = 0;
    int text_offset = 0;

    QVariantMap to_map() const;
    static WordTimestamp from_map(const QVariantMap &data);
};

}  // namespace source::plugin::Audiolivro
