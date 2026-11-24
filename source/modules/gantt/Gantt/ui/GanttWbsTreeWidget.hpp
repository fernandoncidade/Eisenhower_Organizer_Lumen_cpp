#pragma once

#include <QTreeWidget>
#include "../domain/models/Schedule.hpp"

namespace source::gantt {

class GanttWbsTreeWidget : public QTreeWidget {
    Q_OBJECT

public:
    explicit GanttWbsTreeWidget(QWidget *parent = nullptr);

    void populate(const Schedule &schedule);
    QString selected_item_id() const;
    void select_item_by_id(const QString &item_id);
    void retranslate_ui();
    static QString delay_tooltip(bool with_icon = true);

signals:
    void item_selected(const QString &item_id);
    void item_double_clicked(const QString &item_id);
    void request_delete_item(const QString &item_id);
    void request_link_task(const QString &item_id);
    void request_unlink_task(const QString &item_id);
    void request_sync_task(const QString &item_id);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void changeEvent(QEvent *event) override;

private slots:
    void on_current_item_changed(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_item_double_clicked(QTreeWidgetItem *item, int column);
    void on_custom_context_menu(const QPoint &pos);

private:
    Schedule current_schedule_;
    int last_clicked_col_ = -1;
};

} // namespace source::gantt
