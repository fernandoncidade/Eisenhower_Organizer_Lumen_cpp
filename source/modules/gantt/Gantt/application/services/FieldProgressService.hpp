#pragma once

#include <QString>
#include <functional>
#include "../../domain/models/Schedule.hpp"

namespace source::gantt {

class FieldProgressService {
public:
    FieldProgressService();

    // Field progress updates
    bool record_progress(Schedule &schedule,
                         const QString &item_id,
                         double percent,
                         const QString &notes = QString(),
                         const QString &reporter = QStringLiteral("campo"),
                         QString *error_message = nullptr);

    // Link operational Eisenhower task <-> Gantt Schedule Item
    bool link_eisenhower_task(Schedule &schedule,
                              const QString &item_id,
                              const QString &eisenhower_task_id,
                              QString *error_message = nullptr);

    bool unlink_eisenhower_task(Schedule &schedule,
                                const QString &item_id,
                                QString *error_message = nullptr);

    // Called when an Eisenhower task status changes in the host app
    void on_eisenhower_task_status_changed(Schedule &schedule,
                                           const QString &eisenhower_task_id,
                                           bool is_completed);

    // Set callback to notify host application when Gantt item progress changes
    using HostTaskSyncCallback = std::function<void(const QString &task_id, double percent, bool is_done)>;
    void set_host_sync_callback(HostTaskSyncCallback callback);

private:
    HostTaskSyncCallback host_sync_cb_;
};

} // namespace source::gantt
