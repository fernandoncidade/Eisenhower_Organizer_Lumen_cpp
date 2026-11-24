#pragma once

#include <QString>
#include <QVariantList>
#include <QVariantMap>

class QWidget;

namespace source::gantt {

class IEisenhowerHost {
public:
    virtual ~IEisenhowerHost() = default;

    virtual QVariantList get_available_tasks() const = 0;
    virtual QVariantMap get_task_by_id(const QString &task_id) const = 0;
    virtual void sync_task_progress(const QString &task_id, double percent, bool is_done) = 0;
    virtual void sync_task_duration(const QString &task_id, double duration_days) = 0;
    virtual void sync_task_title(const QString &task_id, const QString &new_title) = 0;
    virtual bool remove_task_by_id(const QString &task_id) = 0;
    virtual void show_task_sync_delete_info(const QString &origin, QWidget *parent = nullptr) = 0;
    virtual QWidget *as_widget() = 0;
};

} // namespace source::gantt
