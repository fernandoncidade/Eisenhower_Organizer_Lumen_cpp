#pragma once

#include <QString>
#include <QVariantList>

namespace source::plugin::Audiolivro {

QString default_voice_id();
QVariantList available_voice_options();
QString voice_display_text(const QString &voice_id);

}  // namespace source::plugin::Audiolivro
