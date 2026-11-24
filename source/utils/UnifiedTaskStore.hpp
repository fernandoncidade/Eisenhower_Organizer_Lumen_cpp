#pragma once

#include <QVariantList>
#include <QVariantMap>
#include <QString>
#include <QStringList>

namespace source::utils::UnifiedTaskStore {

QString shared_tasks_path();
QVariantMap normalize_record(const QVariantMap &record);
QVariantList normalize_records(const QVariantList &records);
QVariantList load_records(const QString &path);
bool save_records(const QString &path, const QVariantList &records);
bool replace_module_records(const QString &path,
                            const QString &module,
                            const QVariantList &module_records);
QVariantList project_records_for_module(const QVariantList &records, const QString &module);
QVariantList filter_records_by_module(const QVariantList &records, const QString &module);
QVariantList merge_module_records(const QVariantList &records,
                                  const QString &module,
                                  const QVariantList &module_records);
QVariantList remove_linked_records(const QVariantList &records,
                                   const QVariantMap &task,
                                   const QStringList &modules);

}  // namespace source::utils::UnifiedTaskStore
