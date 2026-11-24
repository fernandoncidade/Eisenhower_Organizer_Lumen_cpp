#include "source/modules/audiolivro/metodoLeitor/lt_05_WordTimestamp.hpp"

namespace source::plugin::Audiolivro {

QVariantMap WordTimestamp::to_map() const
{
    QVariantMap data;
    data.insert(QStringLiteral("text"), text);
    data.insert(QStringLiteral("offset_ms"), offset_ms);
    data.insert(QStringLiteral("duration_ms"), duration_ms);
    data.insert(QStringLiteral("text_offset"), text_offset);
    return data;
}

WordTimestamp WordTimestamp::from_map(const QVariantMap &data)
{
    WordTimestamp value;
    value.text = data.value(QStringLiteral("text")).toString();
    value.offset_ms = data.value(QStringLiteral("offset_ms")).toLongLong();
    value.duration_ms = data.value(QStringLiteral("duration_ms")).toLongLong();
    value.text_offset = data.value(QStringLiteral("text_offset")).toInt();
    return value;
}

}  // namespace source::plugin::Audiolivro
