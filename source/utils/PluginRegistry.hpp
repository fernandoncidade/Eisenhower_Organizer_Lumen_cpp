#pragma once

#include <QString>
#include <QStringList>
#include <QVariantMap>

namespace source::utils::PluginRegistry {

QVariantMap manifest_for_key(const QString &plugin_key);
QStringList binary_names(const QString &binary_base_name);
QStringList candidate_paths(const QString &plugin_key, const QString &binary_base_name);

}  // namespace source::utils::PluginRegistry
