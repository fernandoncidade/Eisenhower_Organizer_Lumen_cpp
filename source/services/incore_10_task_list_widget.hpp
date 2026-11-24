#pragma once

#include <QListWidget>
#include <QPoint>

class QLabel;
class QDropEvent;
class QMouseEvent;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore::incore_10_task_list_widget {

class TaskListWidget : public QListWidget {
public:
    TaskListWidget(source::EisenhowerMatrixApp *app, bool is_completed, QWidget *parent = nullptr);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
    void leaveEvent(QEvent *event) override;
    void startDrag(Qt::DropActions supported_actions) override;
    void dropEvent(QDropEvent *event) override;

private:
    QLabel *create_preview_widget();
    void show_preview_for_item(QListWidgetItem *item, const QPoint &global_pos);
    void hide_preview();
    QPair<QPixmap, QRect> build_drag_pixmap(const QList<QListWidgetItem *> &items) const;
    bool is_check_indicator_at(const QPoint &pos) const;
    void clear_other_task_list_selections();

    source::EisenhowerMatrixApp *app_ = nullptr;
    bool is_completed_ = false;
    QLabel *hover_preview_ = nullptr;
    QListWidgetItem *current_hover_item_ = nullptr;
    QPoint drag_start_pos_;
    bool has_drag_start_ = false;
    QListWidgetItem *drag_start_item_ = nullptr;
    bool selection_rect_enabled_ = true;
    bool pending_single_selection_on_release_ = false;
};

}  // namespace source::InterfaceCore::incore_10_task_list_widget
