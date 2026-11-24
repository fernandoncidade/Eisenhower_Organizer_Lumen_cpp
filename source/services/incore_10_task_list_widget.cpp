#include "source/services/incore_10_task_list_widget.hpp"

#include <QAbstractItemView>
#include <QApplication>
#include <QCursor>
#include <QDrag>
#include <QDropEvent>
#include <QFrame>
#include <QItemSelectionModel>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QScreen>
#include <QSizePolicy>
#include <QStyle>
#include <QStyleOptionViewItem>

#include "source/ui/ui_09_Calendar.hpp"
#include "source/ui/ui_14_CalendarPanel.hpp"
#include "source/eisenhower_app.hpp"
#include "source/utils/LinkedTitleUnderlineDelegate.hpp"

namespace source::InterfaceCore::incore_10_task_list_widget {

TaskListWidget::TaskListWidget(source::EisenhowerMatrixApp *app, bool is_completed, QWidget *parent)
    : QListWidget(parent), app_(app), is_completed_(is_completed)
{
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionRectVisible(true);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDefaultDropAction(Qt::MoveAction);
    setDragDropMode(QAbstractItemView::DragDrop);
    source::utils::install_linked_title_underline_delegate(this);
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(0);

    if (app_ != nullptr) {
        QPalette palette = this->palette();
        const QColor window_color = app_->palette().color(QPalette::Window);
        palette.setColor(QPalette::Base, window_color);
        palette.setColor(QPalette::AlternateBase, window_color);
        setPalette(palette);
        setAutoFillBackground(true);
        if (viewport() != nullptr) {
            viewport()->setAutoFillBackground(true);
        }
    }
}

QLabel *TaskListWidget::create_preview_widget()
{
    auto *label = new QLabel(nullptr, Qt::ToolTip);
    label->setAttribute(Qt::WA_ShowWithoutActivating);
    label->setWordWrap(true);
    QFont font = label->font();
    font.setPointSize(qMax(9, font.pointSize() - 1));
    label->setFont(font);

    QPalette palette = label->palette();
    palette.setColor(QPalette::Window, QColor(QStringLiteral("#ffffe0")));
    palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#000000")));
    label->setPalette(palette);
    label->setAutoFillBackground(true);
    label->setContentsMargins(6, 6, 6, 6);
    label->setMargin(0);
    label->setFrameStyle(QFrame::Panel | QFrame::Plain);
    label->setLineWidth(1);
    return label;
}

void TaskListWidget::show_preview_for_item(QListWidgetItem *item, const QPoint &global_pos)
{
    if (item == nullptr) {
        hide_preview();
        return;
    }

    // The task item already exposes the canonical Qt tooltip. Showing the
    // lightweight hover preview on top of it creates two competing tooltips.
    if (!item->toolTip().trimmed().isEmpty()) {
        hide_preview();
        return;
    }

    const QVariantMap data = item->data(Qt::UserRole).toMap();
    const QString description = data.value(QStringLiteral("description")).toString().trimmed();
    if (description.isEmpty()) {
        hide_preview();
        return;
    }

    QStringList lines;
    for (const QString &line : description.split(QLatin1Char('\n'))) {
        if (!line.trimmed().isEmpty()) {
            lines << line.trimmed();
        }
        if (lines.size() == 3) {
            break;
        }
    }
    if (lines.isEmpty()) {
        hide_preview();
        return;
    }

    if (hover_preview_ == nullptr) {
        hover_preview_ = create_preview_widget();
    }

    hover_preview_->setText(lines.join(QLatin1Char('\n')));
    hover_preview_->adjustSize();
    QRect geometry = hover_preview_->frameGeometry();
    int x = global_pos.x() + 16;
    int y = global_pos.y() + 16;

    QScreen *screen = window() != nullptr ? window()->screen() : nullptr;
    if (screen != nullptr) {
        const QRect available = screen->availableGeometry();
        if (x + geometry.width() > available.right()) {
            x = global_pos.x() - geometry.width() - 16;
        }
        if (y + geometry.height() > available.bottom()) {
            y = available.bottom() - geometry.height() - 8;
        }
    }

    hover_preview_->move(x, y);
    hover_preview_->show();
    current_hover_item_ = item;
}

void TaskListWidget::hide_preview()
{
    if (hover_preview_ != nullptr) {
        hover_preview_->hide();
    }
    current_hover_item_ = nullptr;
}

QPair<QPixmap, QRect> TaskListWidget::build_drag_pixmap(const QList<QListWidgetItem *> &items) const
{
    QList<QRect> rects;
    for (QListWidgetItem *item : items) {
        if (item == nullptr) {
            continue;
        }
        const QRect rect = visualItemRect(item);
        if (rect.isValid()) {
            rects << rect;
        }
    }
    if (rects.isEmpty()) {
        return {};
    }

    QRect united = rects.first();
    for (int i = 1; i < rects.size(); ++i) {
        united = united.united(rects.at(i));
    }

    QPixmap pixmap(united.size());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setOpacity(0.6);
    for (QListWidgetItem *item : items) {
        const QRect rect = visualItemRect(item);
        if (rect.isValid()) {
            painter.drawPixmap(rect.topLeft() - united.topLeft(), viewport()->grab(rect));
        }
    }
    painter.end();
    return {pixmap, united};
}

bool TaskListWidget::is_check_indicator_at(const QPoint &pos) const
{
    QListWidgetItem *item = itemAt(pos);
    if (item == nullptr || !(item->flags() & Qt::ItemIsUserCheckable)) {
        return false;
    }

    const QModelIndex index = indexFromItem(item);
    if (!index.isValid()) {
        return false;
    }

    QStyleOptionViewItem option;
    initViewItemOption(&option);
    option.rect = visualRect(index);
    option.features |= QStyleOptionViewItem::HasCheckIndicator;
    option.checkState = item->checkState();
    option.widget = viewport();
    if (item->isSelected()) {
        option.state |= QStyle::State_Selected;
    }

    const QRect indicator_rect =
        style()->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &option, viewport()).adjusted(-4, -4, 4, 4);
    return indicator_rect.contains(pos);
}

void TaskListWidget::clear_other_task_list_selections()
{
    if (app_ != nullptr) {
        app_->clear_eisenhower_task_selections(this);
    }
}

void TaskListWidget::mouseMoveEvent(QMouseEvent *event)
{
    QListWidgetItem *item = itemAt(event->position().toPoint());
    if (item != nullptr && item != current_hover_item_) {
        show_preview_for_item(item, mapToGlobal(event->position().toPoint()));
    } else if (item == nullptr) {
        hide_preview();
    }

    if ((event->buttons() & Qt::LeftButton) && has_drag_start_) {
        QListWidgetItem *start_item = drag_start_item_ != nullptr ? drag_start_item_ : itemAt(drag_start_pos_);
        if (start_item != nullptr && start_item->isSelected()) {
            if ((event->position().toPoint() - drag_start_pos_).manhattanLength() >= QApplication::startDragDistance()) {
                if (selectedItems().isEmpty()) {
                    setCurrentItem(start_item);
                }
                pending_single_selection_on_release_ = false;
                startDrag(Qt::MoveAction);
                has_drag_start_ = false;
                drag_start_item_ = nullptr;
                return;
            }
        }
    }

    QListWidget::mouseMoveEvent(event);
}

void TaskListWidget::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QListWidget::selectionChanged(selected, deselected);
    if (!selectedItems().isEmpty()) {
        clear_other_task_list_selections();
    }
}

void TaskListWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        drag_start_pos_ = event->position().toPoint();
        has_drag_start_ = true;
        drag_start_item_ = itemAt(drag_start_pos_);
        pending_single_selection_on_release_ = false;
        const Qt::KeyboardModifiers modifiers = event->modifiers();
        const bool clicked_selectable_item =
            drag_start_item_ != nullptr && (drag_start_item_->flags() & Qt::ItemIsSelectable);
        if (clicked_selectable_item) {
            clear_other_task_list_selections();
            if (is_check_indicator_at(drag_start_pos_)) {
                selection_rect_enabled_ = true;
                setSelectionRectVisible(true);
                has_drag_start_ = false;
                drag_start_item_ = nullptr;
                QListWidget::mousePressEvent(event);
                return;
            }
        } else {
            clear_other_task_list_selections();
            if (!(modifiers & (Qt::ControlModifier | Qt::ShiftModifier))) {
                if (QItemSelectionModel *sm = selectionModel(); sm != nullptr) {
                    sm->clear();
                } else {
                    clearSelection();
                }
            }
        }
        if (drag_start_item_ != nullptr && drag_start_item_->isSelected()
            && !(modifiers & (Qt::ControlModifier | Qt::ShiftModifier))) {
            selection_rect_enabled_ = false;
            setSelectionRectVisible(false);
            pending_single_selection_on_release_ = true;
            setFocus(Qt::MouseFocusReason);
            setCurrentItem(drag_start_item_, QItemSelectionModel::NoUpdate);
            event->accept();
            return;
        }
        selection_rect_enabled_ = true;
        setSelectionRectVisible(true);
    }

    QListWidget::mousePressEvent(event);
}

void TaskListWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (pending_single_selection_on_release_) {
            QListWidgetItem *release_item = itemAt(event->position().toPoint());
            if (drag_start_item_ != nullptr && release_item == drag_start_item_) {
                setCurrentItem(drag_start_item_, QItemSelectionModel::ClearAndSelect);
            }

            pending_single_selection_on_release_ = false;
            selection_rect_enabled_ = true;
            setSelectionRectVisible(true);
            has_drag_start_ = false;
            drag_start_item_ = nullptr;
            event->accept();
            return;
        }

        QListWidget::mouseReleaseEvent(event);
        if (!selection_rect_enabled_) {
            setSelectionRectVisible(false);
        }
        pending_single_selection_on_release_ = false;
        has_drag_start_ = false;
        drag_start_item_ = nullptr;
        return;
    }

    QListWidget::mouseReleaseEvent(event);
}

void TaskListWidget::leaveEvent(QEvent *event)
{
    hide_preview();
    QListWidget::leaveEvent(event);
}

void TaskListWidget::startDrag(Qt::DropActions supported_actions)
{
    Q_UNUSED(supported_actions);
    if (selectedItems().isEmpty()) {
        return;
    }

    std::unique_ptr<QMimeData> mime(model()->mimeData(selectedIndexes()));
    if (!mime) {
        return;
    }

    auto *drag = new QDrag(this);
    drag->setMimeData(mime.release());

    const auto preview = build_drag_pixmap(selectedItems());
    if (!preview.first.isNull()) {
        drag->setPixmap(preview.first);
        const QPoint hotspot = has_drag_start_
                                   ? drag_start_pos_ - preview.second.topLeft()
                                   : viewport()->mapFromGlobal(QCursor::pos()) - preview.second.topLeft();
        drag->setHotSpot(hotspot);
    }

    drag->exec(Qt::MoveAction | Qt::CopyAction);
}

void TaskListWidget::dropEvent(QDropEvent *event)
{
    auto *source = qobject_cast<QListWidget *>(event->source());
    if (source == nullptr || source == this || app_ == nullptr) {
        event->ignore();
        return;
    }

    QList<QListWidgetItem *> dragged_items;
    for (QListWidgetItem *item : source->selectedItems()) {
        if (item != nullptr && (item->flags() & Qt::ItemIsSelectable)) {
            dragged_items << item;
        }
    }
    if (dragged_items.isEmpty()) {
        event->ignore();
        return;
    }

    const Qt::CheckState new_state = is_completed_ ? Qt::Checked : Qt::Unchecked;
    source->blockSignals(true);
    blockSignals(true);
    for (QListWidgetItem *item : dragged_items) {
        app_->move_item_between_lists(item, source, this, new_state);
    }
    source->blockSignals(false);
    blockSignals(false);

    app_->save_tasks();
    if (app_->calendar_pane != nullptr && app_->calendar_pane->calendar_panel != nullptr) {
        app_->calendar_pane->calendar_panel->update_task_list();
    }

    event->setDropAction(Qt::MoveAction);
    event->accept();
}

}  // namespace source::InterfaceCore::incore_10_task_list_widget
