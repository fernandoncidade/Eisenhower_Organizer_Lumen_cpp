#pragma once

#include <QByteArray>
#include <QString>

namespace source::utils {

QString get_app_base_path();
QString get_assets_base_path();
QString get_text_file_path(const QString &filename, const QString &folder = QString());
QString load_text_file(const QString &filename,
                       const QString &folder = QString(),
                       const QByteArray &encoding = QByteArrayLiteral("utf-8"));

}  // namespace source::utils
