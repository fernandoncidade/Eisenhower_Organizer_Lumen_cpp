#pragma once

#include "source/services/incore_15_ProductivityFeatures.hpp"

#include <QAbstractItemView>
#include <QApplication>
#include <QBoxLayout>
#include <QCalendarWidget>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateEdit>
#include <QDateTime>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QHash>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QMessageBox>
#include <QMetaType>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSortFilterProxyModel>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QResizeEvent>
#include <QScreen>
#include <QShowEvent>
#include <QSpinBox>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextCursor>
#include <QTextEdit>
#include <QTimeEdit>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QUuid>
#include <QVector>
#include <QWindowStateChangeEvent>
#include <algorithm>
#include <functional>
#include <optional>
#include <limits>
#include <memory>
#include <utility>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

#include "source/eisenhower_app.hpp"
#include "source/ui/ui_09_Calendar.hpp"
#include "source/ui/ui_14_CalendarPanel.hpp"
#include "source/services/incore_14_EditTaskDialog.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"
#include "source/models/model_01_TaskRecordListModel.hpp"
#include "source/utils/CaminhoPersistenteUtils.hpp"
#include "source/utils/LogManager.hpp"
#include "source/utils/UnifiedTaskStore.hpp"

namespace source::InterfaceCore::ProductivityFeatures {
namespace {

class RuntimeTranslatedDialog : public QDialog {
public:
    using QDialog::QDialog;

    void set_retranslate_handler(std::function<void()> handler)
    {
        retranslate_handler_ = std::move(handler);
    }

protected:
    void changeEvent(QEvent *event) override
    {
        QDialog::changeEvent(event);
        if (event != nullptr && event->type() == QEvent::LanguageChange && retranslate_handler_) {
            retranslate_handler_();
        }
    }

private:
    std::function<void()> retranslate_handler_;
};

constexpr int kReminderPastGraceSeconds = 90;

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

void translate_standard_buttons(source::EisenhowerMatrixApp *app, QObject *dialog_or_buttons)
{
    if (app != nullptr && app->gerenciador_traducao != nullptr && dialog_or_buttons != nullptr) {
        app->gerenciador_traducao->traduzir_botoes_padrao(dialog_or_buttons);
    }
}

QTime parse_time(const QString &text);

struct MultiMonthCalendarStrip {
    QWidget *container = nullptr;
    QGridLayout *layout = nullptr;
    QVector<QCalendarWidget *> calendars;
    QSize calendar_size;
    QDate base_month;
    QDate selected_date;
    bool updating = false;
};

struct ResponsiveCheckGrid {
    QWidget *container = nullptr;
    QGridLayout *layout = nullptr;
    QVector<QWidget *> widgets;
    int current_columns = 0;
    int min_columns = 2;
    int max_columns = 4;
};

QDate month_start(const QDate &date)
{
    return QDate(date.year(), date.month(), 1);
}

QDate effective_date(const QDate &date)
{
    return date.isValid() ? date : QDate::currentDate();
}

void update_calendar_strip(MultiMonthCalendarStrip *strip);
void compact_calendar_strip_to_base(MultiMonthCalendarStrip *strip);
void update_responsive_grid(ResponsiveCheckGrid *grid);
void update_settings_when_layout(QWidget *container,
                                 QBoxLayout *layout,
                                 QWidget *left,
                                 QWidget *right,
                                 const QSize &left_compact_size = QSize(),
                                 const QSize &right_compact_size = QSize());

class CalendarStripResizeFilter final : public QObject {
public:
    explicit CalendarStripResizeFilter(MultiMonthCalendarStrip *strip, QObject *parent = nullptr)
        : QObject(parent), strip_(strip)
    {
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (strip_ == nullptr) {
            return QObject::eventFilter(watched, event);
        }
        if (event != nullptr && (event->type() == QEvent::Resize || event->type() == QEvent::Show)) {
            update_calendar_strip(strip_);
        }
        return QObject::eventFilter(watched, event);
    }

private:
    MultiMonthCalendarStrip *strip_ = nullptr;
};

class CalendarStripContainer final : public QWidget {
public:
    explicit CalendarStripContainer(const QSize &base_size, QWidget *parent = nullptr)
        : QWidget(parent), base_size_(base_size)
    {
    }

    void set_base_size(const QSize &base_size)
    {
        base_size_ = base_size;
        updateGeometry();
    }

    QSize sizeHint() const override
    {
        return base_size_.isValid() ? base_size_ : QWidget::sizeHint();
    }

    QSize minimumSizeHint() const override
    {
        return base_size_.isValid() ? base_size_ : QWidget::minimumSizeHint();
    }

private:
    QSize base_size_;
};

class ResponsiveGridResizeFilter final : public QObject {
public:
    explicit ResponsiveGridResizeFilter(ResponsiveCheckGrid *grid, QObject *parent = nullptr)
        : QObject(parent), grid_(grid)
    {
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (grid_ == nullptr) {
            return QObject::eventFilter(watched, event);
        }
        if (event != nullptr && (event->type() == QEvent::Resize || event->type() == QEvent::Show)) {
            update_responsive_grid(grid_);
        }
        return QObject::eventFilter(watched, event);
    }

private:
    ResponsiveCheckGrid *grid_ = nullptr;
};

class BoxLayoutResizeFilter final : public QObject {
public:
    BoxLayoutResizeFilter(QWidget *container,
                          QBoxLayout *layout,
                          QWidget *left,
                          QWidget *right,
                          const QSize &left_compact_size = QSize(),
                          const QSize &right_compact_size = QSize(),
                          QObject *parent = nullptr)
        : QObject(parent)
        , container_(container)
        , layout_(layout)
        , left_(left)
        , right_(right)
        , left_compact_size_(left_compact_size)
        , right_compact_size_(right_compact_size)
    {
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (container_ == nullptr || layout_ == nullptr || left_ == nullptr || right_ == nullptr) {
            return QObject::eventFilter(watched, event);
        }
        if (event != nullptr && (event->type() == QEvent::Resize || event->type() == QEvent::Show)) {
            update_settings_when_layout(container_, layout_, left_, right_, left_compact_size_, right_compact_size_);
        }
        return QObject::eventFilter(watched, event);
    }

private:
    QWidget *container_ = nullptr;
    QBoxLayout *layout_ = nullptr;
    QWidget *left_ = nullptr;
    QWidget *right_ = nullptr;
    QSize left_compact_size_;
    QSize right_compact_size_;
};

class RestoreNormalGeometryFilter final : public QObject {
public:
    RestoreNormalGeometryFilter(QWidget *dialog,
                                const QSize &initial_size,
                                const QSize &minimum_size,
                                std::function<void()> before_restore,
                                std::function<void()> after_restore,
                                bool restore_position,
                                QObject *parent = nullptr)
        : QObject(parent)
        , dialog_(dialog)
        , initial_size_(initial_size)
        , minimum_size_(minimum_size)
        , before_restore_(std::move(before_restore))
        , after_restore_(std::move(after_restore))
        , restore_position_(restore_position)
    {
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (dialog_.isNull() || watched != dialog_ || event == nullptr) {
            return QObject::eventFilter(watched, event);
        }

        if (event->type() == QEvent::Show && normal_geometry_.isNull()) {
            QPointer<QWidget> dialog = dialog_;
            QTimer::singleShot(0, dialog_, [this, dialog]() {
                if (dialog.isNull() || dialog->isMaximized() || !normal_geometry_.isNull()) {
                    return;
                }
                QRect geometry = dialog->geometry();
                if (initial_size_.isValid()) {
                    geometry.setSize(initial_size_);
                }
                normal_geometry_ = bounded_to_screen(geometry);
            });
        } else if (event->type() == QEvent::WindowStateChange) {
            auto *state_event = static_cast<QWindowStateChangeEvent *>(event);
            const bool was_maximized = state_event->oldState().testFlag(Qt::WindowMaximized);
            const bool is_maximized = dialog_->windowState().testFlag(Qt::WindowMaximized);
            if (!was_maximized && is_maximized && normal_geometry_.isNull()) {
                QRect geometry = dialog_->normalGeometry().isValid() ? dialog_->normalGeometry() : dialog_->geometry();
                if (initial_size_.isValid()) {
                    geometry.setSize(initial_size_);
                }
                normal_geometry_ = bounded_to_screen(geometry);
            }
            if (!was_maximized && is_maximized && !restore_position_) {
                QPointer<QWidget> dialog = dialog_;
                QTimer::singleShot(0, dialog_, [this, dialog]() {
                    if (!dialog.isNull()) {
                        apply_native_normal_geometry();
                    }
                });
            } else if (was_maximized && !is_maximized && !restore_pending_) {
                restore_pending_ = true;
                QPointer<QWidget> dialog = dialog_;
                const auto apply_restore = [this, dialog]() {
                    if (dialog.isNull()) {
                        return;
                    }
                    const bool updates_enabled = dialog->updatesEnabled();
                    if (restore_position_) {
                        dialog->showNormal();
                        dialog->setUpdatesEnabled(false);
                    }
                    if (before_restore_) {
                        before_restore_();
                    }
                    if (dialog->layout() != nullptr) {
                        dialog->layout()->activate();
                    }
                    const QRect target_geometry = restore_geometry();
                    dialog->setMinimumSize(minimum_size_.isValid() ? minimum_size_ : QSize(0, 0));
                    if (restore_position_) {
                        dialog->setGeometry(target_geometry);
                    } else if (!native_normal_geometry_applied_) {
                        dialog->resize(target_geometry.size());
                    }
                    if (dialog->layout() != nullptr) {
                        dialog->layout()->activate();
                    }
                    if (after_restore_) {
                        after_restore_();
                    }
                    if (restore_position_) {
                        dialog->setUpdatesEnabled(updates_enabled);
                        dialog->update();
                        dialog->repaint();
                    } else {
                        dialog->update();
                    }
                    restore_pending_ = false;
                };
                QTimer::singleShot(restore_position_ ? 0 : 120, dialog_, apply_restore);
            }
        }

        return QObject::eventFilter(watched, event);
    }

private:
    QRect bounded_to_screen(const QRect &geometry) const
    {
        QRect target = geometry.isValid() ? geometry : QRect(QPoint(0, 0), initial_size_);
        QScreen *screen = nullptr;
        if (!dialog_.isNull()) {
            screen = dialog_->screen();
        }
        if (screen == nullptr) {
            screen = QGuiApplication::screenAt(target.center());
        }
        if (screen == nullptr) {
            screen = QGuiApplication::primaryScreen();
        }
        if (screen == nullptr) {
            return target;
        }

        const QRect available = screen->availableGeometry();
        QSize target_size = target.size();
        if (initial_size_.isValid()) {
            target_size = initial_size_;
        }
        target_size.setWidth(qBound(1, target_size.width(), available.width()));
        target_size.setHeight(qBound(1, target_size.height(), available.height()));

        QPoint top_left = target.topLeft();
        top_left.setX(qBound(available.left(), top_left.x(), available.right() - target_size.width() + 1));
        top_left.setY(qBound(available.top(), top_left.y(), available.bottom() - target_size.height() + 1));
        return QRect(top_left, target_size);
    }

    QRect restore_geometry() const
    {
        QRect geometry = normal_geometry_;
        if (!geometry.isValid() && !dialog_.isNull()) {
            geometry = dialog_->normalGeometry().isValid() ? dialog_->normalGeometry() : dialog_->geometry();
        }
        if (initial_size_.isValid()) {
            geometry.setSize(initial_size_);
        }
        return bounded_to_screen(geometry);
    }

    bool apply_native_normal_geometry()
    {
#ifdef Q_OS_WIN
        if (dialog_.isNull()) {
            return false;
        }

        HWND hwnd = reinterpret_cast<HWND>(dialog_->winId());
        if (hwnd == nullptr) {
            return false;
        }

        WINDOWPLACEMENT placement;
        placement.length = sizeof(WINDOWPLACEMENT);
        if (!GetWindowPlacement(hwnd, &placement)) {
            return false;
        }

        const QRect target = restore_geometry();
        placement.rcNormalPosition.left = target.left();
        placement.rcNormalPosition.top = target.top();
        placement.rcNormalPosition.right = target.left() + target.width();
        placement.rcNormalPosition.bottom = target.top() + target.height();
        native_normal_geometry_applied_ = SetWindowPlacement(hwnd, &placement) != FALSE;
        return native_normal_geometry_applied_;
#else
        native_normal_geometry_applied_ = false;
        return false;
#endif
    }

    QPointer<QWidget> dialog_;
    QSize initial_size_;
    QSize minimum_size_;
    std::function<void()> before_restore_;
    std::function<void()> after_restore_;
    QRect normal_geometry_;
    bool restore_position_ = true;
    bool restore_pending_ = false;
    bool native_normal_geometry_applied_ = false;
};

void update_responsive_grid(ResponsiveCheckGrid *grid)
{
    if (grid == nullptr || grid->container == nullptr || grid->layout == nullptr) {
        return;
    }

    const int widget_count = grid->widgets.size();
    if (widget_count == 0) {
        return;
    }

    int available_width = grid->container->contentsRect().width();
    if (available_width <= 0) {
        available_width = grid->container->width();
    }

    int spacing = grid->layout->horizontalSpacing();
    if (spacing < 0) {
        spacing = grid->layout->spacing();
    }
    const QMargins margins = grid->layout->contentsMargins();
    int required_for_max = margins.left() + margins.right();
    for (QWidget *widget : grid->widgets) {
        if (widget != nullptr) {
            required_for_max += widget->sizeHint().width();
        }
    }
    required_for_max += spacing * qMax(0, widget_count - 1);

    const int target_columns =
        available_width >= required_for_max ? grid->max_columns : grid->min_columns;
    if (grid->current_columns == target_columns) {
        return;
    }

    grid->current_columns = target_columns;
    for (int i = 0; i < grid->widgets.size(); ++i) {
        QWidget *widget = grid->widgets.at(i);
        if (widget == nullptr) {
            continue;
        }
        const int row = i / target_columns;
        const int col = i % target_columns;
        grid->layout->addWidget(widget, row, col);
    }
    grid->layout->invalidate();
}

void update_settings_when_layout(QWidget *container,
                                 QBoxLayout *layout,
                                 QWidget *left,
                                 QWidget *right,
                                 const QSize &left_compact_size,
                                 const QSize &right_compact_size)
{
    if (container == nullptr || layout == nullptr || left == nullptr || right == nullptr) {
        return;
    }

    int available_width = container->contentsRect().width();
    if (available_width <= 0) {
        available_width = container->width();
    }

    const QMargins margins = layout->contentsMargins();
    int spacing = layout->spacing();
    if (spacing < 0) {
        spacing = 0;
    }
    const auto compact_width = [](QWidget *widget, const QSize &compact_size) {
        if (compact_size.isValid()) {
            return compact_size.width();
        }
        return widget != nullptr ? widget->minimumSizeHint().width() : 0;
    };
    const auto compact_height = [](QWidget *widget, const QSize &compact_size) {
        if (compact_size.isValid()) {
            return compact_size.height();
        }
        return widget != nullptr ? widget->minimumSizeHint().height() : 0;
    };
    const int left_min_width = compact_width(left, left_compact_size);
    const int right_min_width = compact_width(right, right_compact_size);
    const int compact_horizontal_width = margins.left() + margins.right() + spacing + left_min_width + right_min_width;
    const int enter_horizontal_width = compact_horizontal_width + 96;
    const int exit_horizontal_width = compact_horizontal_width + 48;
    const bool currently_horizontal = layout->direction() == QBoxLayout::LeftToRight;
    const QBoxLayout::Direction target_direction =
        currently_horizontal
            ? (available_width > exit_horizontal_width ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom)
            : (available_width >= enter_horizontal_width ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);

    if (layout->direction() != target_direction) {
        layout->setDirection(target_direction);
    }
    left->setMinimumWidth(left_min_width);
    right->setMinimumWidth(right_min_width);
    if (target_direction == QBoxLayout::LeftToRight) {
        const int left_height = compact_height(left, left_compact_size);
        const int right_height = compact_height(right, right_compact_size);
        left->setMaximumWidth(left_min_width);
        left->setMaximumHeight(left_height);
        left->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        right->setMaximumWidth(QWIDGETSIZE_MAX);
        right->setMaximumHeight(right_height);
        right->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        container->setMinimumWidth(compact_horizontal_width);
        container->setMaximumHeight(qMax(left_height, right_height));
        layout->setAlignment(left, Qt::AlignTop);
        layout->setAlignment(right, Qt::AlignTop);
        layout->setStretch(0, 0);
        layout->setStretch(1, 1);
    } else {
        left->setMaximumWidth(QWIDGETSIZE_MAX);
        left->setMaximumHeight(QWIDGETSIZE_MAX);
        left->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        right->setMaximumWidth(QWIDGETSIZE_MAX);
        right->setMaximumHeight(QWIDGETSIZE_MAX);
        right->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        container->setMinimumWidth(qMax(left_min_width, right_min_width));
        container->setMaximumHeight(QWIDGETSIZE_MAX);
        layout->setAlignment(left, Qt::Alignment());
        layout->setAlignment(right, Qt::Alignment());
        layout->setStretch(0, 0);
        layout->setStretch(1, 0);
    }
    left->updateGeometry();
    right->updateGeometry();
    container->updateGeometry();
    layout->invalidate();
}

QCalendarWidget *create_calendar_widget(MultiMonthCalendarStrip *strip)
{
    auto *calendar = new QCalendarWidget(strip->container);
    calendar->setGridVisible(true);
    calendar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    if (strip->calendar_size.isValid()) {
        calendar->setFixedSize(strip->calendar_size);
    }
    return calendar;
}

void attach_calendar_strip_selection(MultiMonthCalendarStrip *strip, QCalendarWidget *calendar)
{
    if (strip == nullptr || calendar == nullptr) {
        return;
    }
    QObject::connect(calendar, &QCalendarWidget::selectionChanged, strip->container, [strip, calendar]() {
        if (strip->updating) {
            return;
        }
        const QDate selected = calendar->selectedDate();
        if (!selected.isValid()) {
            return;
        }
        strip->selected_date = selected;
        strip->base_month = month_start(selected);
        update_calendar_strip(strip);
    });
}

MultiMonthCalendarStrip *create_multi_month_calendar_strip(QWidget *parent, const QDate &initial_date, int height)
{
    auto *strip = new MultiMonthCalendarStrip();
    strip->container = new CalendarStripContainer(QSize(1, height), parent);
    strip->layout = new QGridLayout(strip->container);
    strip->layout->setContentsMargins(0, 0, 0, 0);
    strip->layout->setHorizontalSpacing(6);
    strip->layout->setVerticalSpacing(6);
    strip->layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    strip->layout->setSizeConstraint(QLayout::SetNoConstraint);
    strip->container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *calendar = new QCalendarWidget(strip->container);
    calendar->setGridVisible(true);
    calendar->setFixedHeight(height);
    const QSize hint = calendar->sizeHint();
    const int width = hint.width() > 0 ? hint.width() : 240;
    strip->calendar_size = QSize(width, height);
    calendar->setFixedSize(strip->calendar_size);
    calendar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    strip->calendars.append(calendar);
    strip->layout->addWidget(calendar);

    const QDate selected = effective_date(initial_date);
    strip->selected_date = selected;
    strip->base_month = month_start(selected);
    calendar->setSelectedDate(selected);
    attach_calendar_strip_selection(strip, calendar);
    if (strip->container != nullptr) {
        strip->container->setMinimumSize(strip->calendar_size);
        strip->container->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
        static_cast<CalendarStripContainer *>(strip->container)->set_base_size(strip->calendar_size);
    }
    return strip;
}

void update_calendar_strip(MultiMonthCalendarStrip *strip)
{
    if (strip == nullptr || strip->container == nullptr || strip->layout == nullptr) {
        return;
    }

    int available_width = strip->container->contentsRect().width();
    if (available_width <= 0) {
        available_width = strip->container->width();
    }

    int available_height = strip->container->contentsRect().height();
    if (available_height <= 0) {
        available_height = strip->container->height();
    }

    int h_spacing = strip->layout->horizontalSpacing();
    if (h_spacing < 0) {
        h_spacing = strip->layout->spacing();
    }
    int v_spacing = strip->layout->verticalSpacing();
    if (v_spacing < 0) {
        v_spacing = strip->layout->spacing();
    }
    const int per_calendar = strip->calendar_size.width();
    const int per_calendar_height = strip->calendar_size.height();
    int count = 1;
    int columns = 1;
    constexpr int kCalendarResizeReleasePadding = 24;
    if (per_calendar > 0) {
        const int width_for_columns = qMax(1, available_width - kCalendarResizeReleasePadding);
        columns = qMax(1, (width_for_columns + h_spacing) / (per_calendar + h_spacing));
    }
    int rows = 1;
    if (per_calendar_height > 0) {
        const int height_for_rows = qMax(1, available_height - kCalendarResizeReleasePadding);
        rows = qMax(1, (height_for_rows + v_spacing) / (per_calendar_height + v_spacing));
    }
    count = qMax(1, columns * rows);

    while (strip->calendars.size() < count) {
        auto *calendar = create_calendar_widget(strip);
        strip->calendars.append(calendar);
        strip->layout->addWidget(calendar);
        attach_calendar_strip_selection(strip, calendar);
    }
    while (strip->calendars.size() > count) {
        QCalendarWidget *calendar = strip->calendars.takeLast();
        strip->layout->removeWidget(calendar);
        calendar->deleteLater();
    }

    strip->updating = true;
    const QDate base_month = strip->base_month.isValid() ? strip->base_month : month_start(QDate::currentDate());
    const QDate selected = effective_date(strip->selected_date);
    for (int i = 0; i < strip->calendars.size(); ++i) {
        QCalendarWidget *calendar = strip->calendars.at(i);
        const QDate month = base_month.addMonths(i);
        const int row = i / columns;
        const int col = i % columns;
        strip->layout->addWidget(calendar, row, col, Qt::AlignLeft | Qt::AlignTop);
        calendar->setCurrentPage(month.year(), month.month());
        if (selected.year() == month.year() && selected.month() == month.month()) {
            calendar->setSelectedDate(selected);
        } else {
            calendar->setSelectedDate(QDate(month.year(), month.month(), 1));
        }
    }
    strip->updating = false;
    strip->layout->invalidate();
    strip->container->updateGeometry();
}

void compact_calendar_strip_to_base(MultiMonthCalendarStrip *strip)
{
    if (strip == nullptr || strip->layout == nullptr || strip->calendars.isEmpty()) {
        return;
    }

    while (strip->calendars.size() > 1) {
        QCalendarWidget *calendar = strip->calendars.takeLast();
        strip->layout->removeWidget(calendar);
        calendar->deleteLater();
    }

    strip->layout->invalidate();
    if (strip->container != nullptr) {
        strip->container->updateGeometry();
    }
}

void set_calendar_strip_selected_date(MultiMonthCalendarStrip *strip, const QDate &date)
{
    if (strip == nullptr) {
        return;
    }
    const QDate selected = effective_date(date);
    strip->selected_date = selected;
    strip->base_month = month_start(selected);
    update_calendar_strip(strip);
}

QDate calendar_strip_selected_date(const MultiMonthCalendarStrip *strip)
{
    if (strip == nullptr) {
        return QDate::currentDate();
    }
    return effective_date(strip->selected_date);
}

QString module_label(const QString &module)
{
    const QString normalized = module.trimmed().toCaseFolded();
    if (normalized == QStringLiteral("feynman")) {
        return get_text(QStringLiteral("Feynman"));
    }
    if (normalized == QStringLiteral("pomodoro")) {
        return get_text(QStringLiteral("Pomodoro"));
    }
    if (normalized == QStringLiteral("mapamental")) {
        return get_text(QStringLiteral("Mapa Mental"));
    }
    if (normalized == QStringLiteral("audiolivro")) {
        return get_text(QStringLiteral("Audiolivro"));
    }
    return get_text(QStringLiteral("Matriz Eisenhower"));
}

QString title_for_record(const QVariantMap &record)
{
    QString title = record.value(QStringLiteral("titulo")).toString().trimmed();
    if (title.isEmpty()) {
        title = record.value(QStringLiteral("text")).toString().trimmed();
    }
    return title;
}

QString date_for_record(const QVariantMap &record)
{
    QString date = record.value(QStringLiteral("data")).toString().trimmed();
    if (date.isEmpty()) {
        date = record.value(QStringLiteral("date")).toString().trimmed();
    }
    return date;
}

QString time_for_record(const QVariantMap &record)
{
    QString time = record.value(QStringLiteral("hora")).toString().trimmed();
    if (time.isEmpty()) {
        time = record.value(QStringLiteral("time")).toString().trimmed();
    }
    return time;
}

QString searchable_text(const QVariantMap &record)
{
    QStringList parts;
    for (auto it = record.constBegin(); it != record.constEnd(); ++it) {
        parts << it.key() << it.value().toString();
    }
    return parts.join(QLatin1Char(' '));
}

QVariantList records_for_app(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return {};
    }
    app->save_tasks_now();
    return source::utils::UnifiedTaskStore::load_records(app->tasks_path);
}

QString mapamental_backup_path()
{
    return QDir(source::utils::obter_caminho_persistente()).filePath(QStringLiteral("mapa_mental_autosave.json"));
}

QString recurring_tasks_undo_path()
{
    return QDir(source::utils::obter_caminho_persistente()).filePath(QStringLiteral("recorrencias_estado_original.json"));
}

QString local_reminder_settings_path()
{
    return QDir(source::utils::obter_caminho_persistente()).filePath(QStringLiteral("lembretes_locais_config.json"));
}

QString recurring_tasks_settings_path()
{
    return QDir(source::utils::obter_caminho_persistente()).filePath(QStringLiteral("recorrencias_config.json"));
}

QJsonObject read_json_object(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    return document.isObject() ? document.object() : QJsonObject{};
}

bool write_json_object(const QString &path, const QJsonObject &object)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    const QByteArray payload = QJsonDocument(object).toJson(QJsonDocument::Indented);
    return file.write(payload) == payload.size() && file.commit();
}

QString current_audiolivro_text(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr || app->audiolivro_module_page == nullptr) {
        return {};
    }

    for (QTextEdit *edit : app->audiolivro_module_page->findChildren<QTextEdit *>()) {
        const QTextCursor cursor = edit->textCursor();
        if (cursor.hasSelection()) {
            return cursor.selectedText().replace(QChar::ParagraphSeparator, QLatin1Char('\n')).trimmed();
        }
    }
    for (QPlainTextEdit *edit : app->audiolivro_module_page->findChildren<QPlainTextEdit *>()) {
        const QTextCursor cursor = edit->textCursor();
        if (cursor.hasSelection()) {
            return cursor.selectedText().replace(QChar::ParagraphSeparator, QLatin1Char('\n')).trimmed();
        }
    }
    for (QTextEdit *edit : app->audiolivro_module_page->findChildren<QTextEdit *>()) {
        const QString text = edit->toPlainText().trimmed();
        if (!text.isEmpty()) {
            return text;
        }
    }
    for (QPlainTextEdit *edit : app->audiolivro_module_page->findChildren<QPlainTextEdit *>()) {
        const QString text = edit->toPlainText().trimmed();
        if (!text.isEmpty()) {
            return text;
        }
    }
    return {};
}

struct SpacedReviewUndoSnapshot {
    QVariantList records_before;
    QJsonObject mapamental_backup_before;
    QSet<QString> affected_modules;
    QDateTime created_at;
};

std::optional<SpacedReviewUndoSnapshot> g_spaced_review_undo;

struct RecurringTasksUndoSnapshot {
    QVariantList records_before;
    QJsonObject mapamental_backup_before;
    QSet<QString> affected_modules;
    QDateTime created_at;
};

std::optional<RecurringTasksUndoSnapshot> g_recurring_tasks_undo;

QString active_module_key(const source::EisenhowerMatrixApp *app)
{
    if (app == nullptr || app->module_tabs == nullptr) {
        return QStringLiteral("eisenhower");
    }

    QWidget *current = app->module_tabs->currentWidget();
    if (current == app->feynman_module_page) {
        return QStringLiteral("feynman");
    }
    if (current == app->pomodoro_module_page) {
        return QStringLiteral("pomodoro");
    }
    if (current == app->mapamental_module_page) {
        return QStringLiteral("mapamental");
    }
    if (current == app->audiolivro_module_page) {
        return QStringLiteral("audiolivro");
    }
    return QStringLiteral("eisenhower");
}

QVariantList records_for_module(const QVariantList &records, const QString &module)
{
    QVariantList filtered = source::utils::UnifiedTaskStore::filter_records_by_module(records, module);
    if (filtered.isEmpty()) {
        filtered = source::utils::UnifiedTaskStore::project_records_for_module(records, module);
    }
    return filtered;
}

int normalized_domain_value(const QVariant &value, int fallback)
{
    bool ok = false;
    const int as_int = value.toInt(&ok);
    if (ok) {
        if (as_int >= 0 && as_int <= 2) {
            return as_int;
        }
        if (as_int >= 1 && as_int <= 3) {
            return as_int - 1;
        }
    }

    QString text = value.toString().trimmed().toCaseFolded();
    if (text.startsWith(QStringLiteral("🥇")) || text.contains(QStringLiteral("avancado"))
        || text.contains(QStringLiteral("avançado")) || text.contains(QStringLiteral("advanced"))) {
        return 2;
    }
    if (text.startsWith(QStringLiteral("🥈")) || text.contains(QStringLiteral("intermediario"))
        || text.contains(QStringLiteral("intermediário")) || text.contains(QStringLiteral("intermediate"))) {
        return 1;
    }
    if (text.startsWith(QStringLiteral("🥉")) || text.contains(QStringLiteral("iniciante"))
        || text.contains(QStringLiteral("beginner"))) {
        return 0;
    }

    return fallback;
}

int normalized_priority_value(const QVariant &value, int fallback)
{
    bool ok = false;
    const int as_int = value.toInt(&ok);
    if (ok) {
        if (as_int >= 1 && as_int <= 4) {
            return as_int;
        }
        if (as_int >= 0 && as_int <= 3) {
            return as_int + 1;
        }
    }

    QString text = value.toString().trimmed().toCaseFolded();
    if (text.startsWith(QStringLiteral("🔴"))) {
        return 1;
    }
    if (text.startsWith(QStringLiteral("🟠"))) {
        return 2;
    }
    if (text.startsWith(QStringLiteral("🟡"))) {
        return 3;
    }
    if (text.startsWith(QStringLiteral("🟢"))) {
        return 4;
    }

    QString normalized = text;
    normalized.replace(QStringLiteral("não"), QStringLiteral("nao"));
    normalized.remove(QLatin1Char(','));
    normalized = normalized.simplified();

    if (normalized.contains(QStringLiteral("importante")) && normalized.contains(QStringLiteral("urgente"))
        && !normalized.contains(QStringLiteral("nao urgente"))) {
        if (normalized.contains(QStringLiteral("nao importante"))) {
            return 3;
        }
        return 1;
    }
    if (normalized.contains(QStringLiteral("importante")) && normalized.contains(QStringLiteral("nao urgente"))) {
        if (normalized.contains(QStringLiteral("nao importante"))) {
            return 4;
        }
        return 2;
    }
    if (normalized.contains(QStringLiteral("nao importante")) && normalized.contains(QStringLiteral("urgente"))) {
        return 3;
    }
    if (normalized.contains(QStringLiteral("nao importante")) && normalized.contains(QStringLiteral("nao urgente"))) {
        return 4;
    }

    return fallback;
}

bool record_completed(const QVariantMap &record)
{
    if (record.value(QStringLiteral("concluida")).toBool()) {
        return true;
    }
    const QString status = record.value(QStringLiteral("status")).toString().trimmed().toCaseFolded();
    return status == QStringLiteral("done") || status == QStringLiteral("completed");
}

QString normalize_module_key(const QString &module)
{
    QString normalized = module.trimmed().toCaseFolded().simplified();
    normalized.replace(QLatin1Char('_'), QLatin1Char(' '));
    normalized.replace(QLatin1Char('-'), QLatin1Char(' '));

    if (normalized == QStringLiteral("mapa mental") || normalized == QStringLiteral("mind map")
        || normalized == QStringLiteral("mindmap")) {
        return QStringLiteral("mapamental");
    }
    if (normalized.contains(QStringLiteral("eisenhower"))) {
        return QStringLiteral("eisenhower");
    }
    if (normalized.contains(QStringLiteral("pomodoro"))) {
        return QStringLiteral("pomodoro");
    }
    if (normalized.contains(QStringLiteral("feynman"))) {
        return QStringLiteral("feynman");
    }
    if (normalized.contains(QStringLiteral("mapamental"))) {
        return QStringLiteral("mapamental");
    }
    return normalized;
}

QString record_module(const QVariantMap &record, const QString &fallback = {})
{
    QString module = normalize_module_key(record.value(QStringLiteral("modulo")).toString());
    if (module.isEmpty()) {
        module = normalize_module_key(record.value(QStringLiteral("module")).toString());
    }
    if (module.isEmpty()) {
        module = normalize_module_key(fallback);
    }
    return module;
}

QStringList productivity_modules_order()
{
    return {QStringLiteral("eisenhower"),
            QStringLiteral("pomodoro"),
            QStringLiteral("feynman"),
            QStringLiteral("mapamental")};
}

bool productivity_module_available(source::EisenhowerMatrixApp *app, const QString &module)
{
    const QString normalized = normalize_module_key(module);
    if (normalized == QStringLiteral("eisenhower")) {
        return true;
    }
    if (app == nullptr) {
        return false;
    }
    if (normalized == QStringLiteral("pomodoro")) {
        return app->has_pomodoro_plugin();
    }
    if (normalized == QStringLiteral("feynman")) {
        return app->has_feynman_plugin();
    }
    if (normalized == QStringLiteral("mapamental")) {
        return app->has_mapamental_plugin();
    }
    return false;
}

QStringList available_productivity_modules(source::EisenhowerMatrixApp *app)
{
    QStringList modules;
    for (const QString &module : productivity_modules_order()) {
        if (productivity_module_available(app, module)) {
            modules.append(module);
        }
    }
    return modules;
}

QSet<QString> module_set_from_list(const QStringList &modules)
{
    QSet<QString> module_set;
    for (const QString &module : modules) {
        const QString normalized = normalize_module_key(module);
        if (!normalized.isEmpty()) {
            module_set.insert(normalized);
        }
    }
    return module_set;
}

int module_order_index(const QString &module)
{
    const QString normalized = normalize_module_key(module);
    const QStringList order = productivity_modules_order();
    const int index = order.indexOf(normalized);
    return index >= 0 ? index : order.size();
}

QStringList ordered_modules_from_set(const QSet<QString> &modules)
{
    QStringList ordered;
    const QStringList order = productivity_modules_order();
    for (const QString &module : order) {
        if (modules.contains(module)) {
            ordered.append(module);
        }
    }
    return ordered;
}

bool save_recurring_tasks_undo_snapshot(const RecurringTasksUndoSnapshot &snapshot)
{
    QJsonObject object;
    object.insert(QStringLiteral("created_at"), snapshot.created_at.toUTC().toString(Qt::ISODate));

    QJsonArray modules;
    QStringList ordered_modules = ordered_modules_from_set(snapshot.affected_modules);
    if (ordered_modules.isEmpty()) {
        ordered_modules = snapshot.affected_modules.values();
        ordered_modules.sort();
    }
    for (const QString &module : std::as_const(ordered_modules)) {
        modules.append(module);
    }
    object.insert(QStringLiteral("affected_modules"), modules);
    object.insert(QStringLiteral("records_before"), QJsonArray::fromVariantList(snapshot.records_before));
    object.insert(QStringLiteral("mapamental_backup_before"), snapshot.mapamental_backup_before);
    return write_json_object(recurring_tasks_undo_path(), object);
}

std::optional<RecurringTasksUndoSnapshot> load_recurring_tasks_undo_snapshot()
{
    const QJsonObject object = read_json_object(recurring_tasks_undo_path());
    if (object.isEmpty()) {
        return std::nullopt;
    }

    RecurringTasksUndoSnapshot snapshot;
    snapshot.records_before = object.value(QStringLiteral("records_before")).toArray().toVariantList();
    snapshot.mapamental_backup_before = object.value(QStringLiteral("mapamental_backup_before")).toObject();
    const QJsonArray modules = object.value(QStringLiteral("affected_modules")).toArray();
    for (const QJsonValue &value : modules) {
        const QString module = value.toString().trimmed();
        if (!module.isEmpty()) {
            snapshot.affected_modules.insert(module);
        }
    }
    snapshot.created_at = QDateTime::fromString(object.value(QStringLiteral("created_at")).toString(), Qt::ISODate);
    if (!snapshot.created_at.isValid()) {
        snapshot.created_at = QDateTime::currentDateTime();
    } else {
        snapshot.created_at = snapshot.created_at.toLocalTime();
    }

    if (snapshot.records_before.isEmpty() && snapshot.mapamental_backup_before.isEmpty()
        && snapshot.affected_modules.isEmpty()) {
        return std::nullopt;
    }
    return snapshot;
}

std::optional<RecurringTasksUndoSnapshot> current_recurring_tasks_undo_snapshot()
{
    if (g_recurring_tasks_undo.has_value()) {
        return g_recurring_tasks_undo;
    }

    std::optional<RecurringTasksUndoSnapshot> loaded = load_recurring_tasks_undo_snapshot();
    if (loaded.has_value()) {
        g_recurring_tasks_undo = loaded.value();
    }
    return loaded;
}

QString module_list_label(const QStringList &modules)
{
    QStringList labels;
    for (const QString &module : modules) {
        labels.append(module_label(module));
    }
    return labels.join(QStringLiteral("; "));
}

QString record_description(const QVariantMap &record)
{
    QStringList parts;
    const QStringList keys = {QStringLiteral("descricao"),
                              QStringLiteral("description"),
                              QStringLiteral("explicacao"),
                              QStringLiteral("revisao"),
                              QStringLiteral("lacunas"),
                              QStringLiteral("notas")};
    for (const QString &key : keys) {
        const QString value = record.value(key).toString().trimmed();
        if (!value.isEmpty() && !parts.contains(value)) {
            parts.append(value);
        }
    }
    return parts.join(QStringLiteral("\n\n"));
}

QString record_file_path(const QVariantMap &record)
{
    QString path = record.value(QStringLiteral("arquivo_vinculado")).toString().trimmed();
    if (path.isEmpty()) {
        path = record.value(QStringLiteral("file_path")).toString().trimmed();
    }
    return path;
}

QString folded_identity_text(QString text)
{
    text = text.normalized(QString::NormalizationForm_D).toCaseFolded();
    text.remove(QRegularExpression(QStringLiteral("\\p{Mn}+")));
    text.replace(QRegularExpression(QStringLiteral("[^\\p{L}\\p{N}]+")), QStringLiteral(" "));
    return text.simplified();
}

QString normalize_recurrence_key(const QString &recurrence)
{
    QString normalized = folded_identity_text(recurrence);
    normalized.replace(QStringLiteral("diaria"), QStringLiteral("daily"));
    normalized.replace(QStringLiteral("diario"), QStringLiteral("daily"));
    normalized.replace(QStringLiteral("semanal"), QStringLiteral("weekly"));
    normalized.replace(QStringLiteral("mensal"), QStringLiteral("monthly"));

    if (normalized == QStringLiteral("daily") || normalized == QStringLiteral("weekly")
        || normalized == QStringLiteral("monthly")) {
        return normalized;
    }
    return {};
}

QString recurrence_for_record(const QVariantMap &record)
{
    QString recurrence = record.value(QStringLiteral("recurrence")).toString().trimmed();
    if (recurrence.isEmpty()) {
        recurrence = record.value(QStringLiteral("recorrencia")).toString().trimmed();
    }
    return normalize_recurrence_key(recurrence);
}

QString recurrence_until_for_record(const QVariantMap &record)
{
    QString until = record.value(QStringLiteral("recurrence_until")).toString().trimmed();
    if (until.isEmpty()) {
        until = record.value(QStringLiteral("recorrencia_ate")).toString().trimmed();
    }
    return until;
}

bool record_is_spaced_review(const QVariantMap &record)
{
    const QString origin = record.value(QStringLiteral("integracao_origem")).toString().trimmed().toCaseFolded();
    if (origin.startsWith(QStringLiteral("spaced_review")) || origin == QStringLiteral("feynman_spaced_review")) {
        return true;
    }
    if (!record.value(QStringLiteral("review_source_module")).toString().trimmed().isEmpty()) {
        return true;
    }
    const QString title = title_for_record(record).trimmed().toCaseFolded();
    return title.startsWith(get_text(QStringLiteral("Revisão espaçada")).toCaseFolded());
}

QString record_source_id(const QVariantMap &record, const QString &fallback_prefix = QStringLiteral("productivity"))
{
    QString id = record.value(QStringLiteral("integracao_task_id")).toString().trimmed();
    if (id.isEmpty()) {
        id = record.value(QStringLiteral("id")).toString().trimmed();
    }
    if (!id.isEmpty()) {
        return id;
    }

    if (record.contains(QStringLiteral("mapa_index"))) {
        return QStringLiteral("mapamental_%1").arg(record.value(QStringLiteral("mapa_index")).toInt());
    }

    const QString title = title_for_record(record).trimmed();
    if (!title.isEmpty()) {
        return QStringLiteral("%1_%2").arg(fallback_prefix, title.left(64));
    }

    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QStringList mapamental_info_list(const QJsonObject &node)
{
    QStringList info;
    const QJsonValue value = node.value(QStringLiteral("info_associada"));
    if (value.isArray()) {
        const QJsonArray items = value.toArray();
        for (const QJsonValue &item : items) {
            info.append(item.toString());
        }
    } else if (value.isString()) {
        info.append(value.toString());
    }
    return info;
}

QString info_value_for_key(const QStringList &info, const QStringList &keys)
{
    for (const QString &item : info) {
        const QString trimmed = item.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }

        int separator = trimmed.indexOf(QLatin1Char('='));
        if (separator < 0) {
            separator = trimmed.indexOf(QLatin1Char(':'));
        }
        if (separator < 0) {
            continue;
        }

        const QString key = trimmed.left(separator).trimmed().toCaseFolded();
        const QString value = trimmed.mid(separator + 1).trimmed();
        for (const QString &candidate : keys) {
            if (key == candidate) {
                return value;
            }
        }
    }

    return {};
}

QVariantMap mapamental_record_from_node(const QJsonObject &node, int index)
{
    QVariantMap record = node.toVariantMap();
    record.insert(QStringLiteral("modulo"), QStringLiteral("mapamental"));
    record.insert(QStringLiteral("titulo"), node.value(QStringLiteral("texto")).toString());
    record.insert(QStringLiteral("descricao"), node.value(QStringLiteral("notas")).toString());
    record.insert(QStringLiteral("mapa_index"), index);

    const QStringList info = mapamental_info_list(node);
    record.insert(QStringLiteral("info_associada"), info);

    QString id = info_value_for_key(info, {QStringLiteral("node_id"), QStringLiteral("id"), QStringLiteral("mapa_id")});
    if (id.isEmpty()) {
        id = QStringLiteral("mapamental_%1").arg(index);
    }
    record.insert(QStringLiteral("id"), id);

    const QString integration_id =
        info_value_for_key(info, {QStringLiteral("integracao_task_id"), QStringLiteral("integration_id")});
    if (!integration_id.isEmpty()) {
        record.insert(QStringLiteral("integracao_task_id"), integration_id);
    }

    const QString review_source_id =
        info_value_for_key(info, {QStringLiteral("review_source_id"), QStringLiteral("source_id")});
    if (!review_source_id.isEmpty()) {
        record.insert(QStringLiteral("review_source_id"), review_source_id);
    }

    const QString review_source_module =
        info_value_for_key(info, {QStringLiteral("review_source_module"), QStringLiteral("source_module")});
    if (!review_source_module.isEmpty()) {
        record.insert(QStringLiteral("review_source_module"), review_source_module);
    }

    const QString date_text = info_value_for_key(info,
                                                 {QStringLiteral("data"),
                                                  QStringLiteral("date"),
                                                  QStringLiteral("review_due"),
                                                  QStringLiteral("due")});
    if (!date_text.isEmpty()) {
        record.insert(QStringLiteral("data"), date_text);
    }

    const QString time_text =
        info_value_for_key(info,
                           {QStringLiteral("hora"),
                            QStringLiteral("horario"),
                            QStringLiteral("horário"),
                            QStringLiteral("time")});
    if (!time_text.isEmpty()) {
        record.insert(QStringLiteral("hora"), time_text);
    }

    const QString recurrence =
        normalize_recurrence_key(info_value_for_key(info,
                                                    {QStringLiteral("recurrence"),
                                                     QStringLiteral("recorrencia"),
                                                     QStringLiteral("recorrência")}));
    if (!recurrence.isEmpty()) {
        record.insert(QStringLiteral("recurrence"), recurrence);
    }

    const QString recurrence_until =
        info_value_for_key(info,
                           {QStringLiteral("recurrence_until"),
                            QStringLiteral("recorrencia_ate"),
                            QStringLiteral("recorrência_ate")});
    if (!recurrence_until.isEmpty()) {
        record.insert(QStringLiteral("recurrence_until"), recurrence_until);
    }

    const QString recurrence_source_id = info_value_for_key(
        info, {QStringLiteral("recurrence_source_id"), QStringLiteral("recorrencia_origem_id")});
    if (!recurrence_source_id.isEmpty()) {
        record.insert(QStringLiteral("recurrence_source_id"), recurrence_source_id);
    }

    const QString dominio_text =
        info_value_for_key(info, {QStringLiteral("dominio"), QStringLiteral("domain")});
    if (!dominio_text.isEmpty()) {
        record.insert(QStringLiteral("dominio"), normalized_domain_value(dominio_text, 0));
    }

    const QString prioridade_text = info_value_for_key(
        info, {QStringLiteral("prioridade"), QStringLiteral("priority"), QStringLiteral("quadrante")});
    if (!prioridade_text.isEmpty()) {
        record.insert(QStringLiteral("prioridade"), normalized_priority_value(prioridade_text, 2));
    }

    return record;
}

QVariantList mapamental_records_from_backup(const QJsonObject &root)
{
    QVariantList records;
    const QJsonArray nodes = root.value(QStringLiteral("nodes")).toArray();
    records.reserve(nodes.size());
    for (int i = 0; i < nodes.size(); ++i) {
        const QJsonObject node = nodes.at(i).toObject();
        const QString title = node.value(QStringLiteral("texto")).toString().trimmed();
        if (title.isEmpty()) {
            continue;
        }
        records.append(mapamental_record_from_node(node, i));
    }
    return records;
}

struct TaskRecordGroup {
    bool active = true;
    QVector<QVariantMap> records;
    QSet<QString> modules;
    QSet<QString> aliases;
    QVariantMap primary;
};

bool projection_available_for_module(source::EisenhowerMatrixApp *app, const QString &module)
{
    const QString normalized = normalize_module_key(module);
    return normalized != QStringLiteral("eisenhower") && productivity_module_available(app, normalized);
}

QVariantList records_for_productivity_module(source::EisenhowerMatrixApp *app,
                                             const QVariantList &unified_records,
                                             const QJsonObject &mapamental_backup,
                                             const QString &module)
{
    const QString normalized = normalize_module_key(module);
    QVariantList records;

    if (normalized == QStringLiteral("mapamental")) {
        records = source::utils::UnifiedTaskStore::filter_records_by_module(unified_records, normalized);
        const QVariantList backup_records = mapamental_records_from_backup(mapamental_backup);
        for (const QVariant &value : backup_records) {
            records.append(value);
        }
    } else {
        records = source::utils::UnifiedTaskStore::filter_records_by_module(unified_records, normalized);
        if (records.isEmpty() && projection_available_for_module(app, normalized)) {
            records = source::utils::UnifiedTaskStore::project_records_for_module(unified_records, normalized);
        }
    }

    QVariantList normalized_records;
    normalized_records.reserve(records.size());
    for (const QVariant &value : records) {
        QVariantMap record = value.toMap();
        record.insert(QStringLiteral("modulo"), normalized);
        normalized_records.append(record);
    }
    return normalized_records;
}

QStringList grouping_aliases_for_record(const QVariantMap &record)
{
    QStringList aliases;
    const QString module = record_module(record);
    const QString title = title_for_record(record);
    const QString folded_title = folded_identity_text(title);
    const QString date = date_for_record(record).trimmed();
    const QString time = time_for_record(record).trimmed();
    const QString file_path = folded_identity_text(record_file_path(record));

    const auto append_alias = [&aliases](const QString &alias) {
        if (!alias.trimmed().isEmpty() && !aliases.contains(alias)) {
            aliases.append(alias);
        }
    };

    const QString integration_id = folded_identity_text(record.value(QStringLiteral("integracao_task_id")).toString());
    const QString id = folded_identity_text(record.value(QStringLiteral("id")).toString());
    const QString recurrence_source_id =
        folded_identity_text(record.value(QStringLiteral("recurrence_source_id")).toString());
    const QString review_source_id = folded_identity_text(record.value(QStringLiteral("review_source_id")).toString());

    if (record_is_spaced_review(record)) {
        const QString source_id = !integration_id.isEmpty() ? integration_id : review_source_id;
        const QString source_module = folded_identity_text(record.value(QStringLiteral("review_source_module")).toString());
        const QString due = record.value(QStringLiteral("review_due"), date).toString().trimmed();
        if (!source_id.isEmpty()) {
            append_alias(QStringLiteral("review:%1|%2|%3|%4")
                             .arg(source_module, source_id, due, folded_title));
        }
    } else if (!recurrence_source_id.isEmpty()) {
        append_alias(QStringLiteral("recurrence:%1|%2|%3").arg(recurrence_source_id, date, folded_title));
    } else {
        if (!integration_id.isEmpty()) {
            append_alias(QStringLiteral("integration:%1").arg(integration_id));
        }
        if (!id.isEmpty()) {
            append_alias(QStringLiteral("id:%1").arg(id));
        }
    }

    if (!folded_title.isEmpty()) {
        append_alias(QStringLiteral("signature:%1|%2|%3|%4").arg(folded_title, date, time, file_path));
        if (date.isEmpty() && time.isEmpty() && file_path.isEmpty()) {
            append_alias(QStringLiteral("title:%1").arg(folded_title));
        }
    }

    if (aliases.isEmpty() && !module.isEmpty() && !folded_title.isEmpty()) {
        aliases.append(QStringLiteral("%1:%2").arg(module, folded_title));
    }
    return aliases;
}

bool record_is_better_primary(const QVariantMap &candidate, const QVariantMap &current)
{
    if (current.isEmpty()) {
        return true;
    }

    const int candidate_order = module_order_index(record_module(candidate));
    const int current_order = module_order_index(record_module(current));
    if (candidate_order != current_order) {
        return candidate_order < current_order;
    }

    const bool candidate_has_date = QDate::fromString(date_for_record(candidate), Qt::ISODate).isValid();
    const bool current_has_date = QDate::fromString(date_for_record(current), Qt::ISODate).isValid();
    if (candidate_has_date != current_has_date) {
        return candidate_has_date;
    }

    const bool candidate_has_description = !record_description(candidate).trimmed().isEmpty();
    const bool current_has_description = !record_description(current).trimmed().isEmpty();
    return candidate_has_description && !current_has_description;
}

void add_record_to_group(TaskRecordGroup &group, const QVariantMap &record, const QStringList &aliases)
{
    group.records.append(record);
    const QString module = record_module(record);
    if (!module.isEmpty()) {
        group.modules.insert(module);
    }
    for (const QString &alias : aliases) {
        group.aliases.insert(alias);
    }
    if (record_is_better_primary(record, group.primary)) {
        group.primary = record;
    }
}

void merge_task_groups(QVector<TaskRecordGroup> &groups, QHash<QString, int> &alias_to_group, int target, int source)
{
    if (target == source || target < 0 || source < 0 || target >= groups.size() || source >= groups.size()
        || !groups.at(source).active) {
        return;
    }

    TaskRecordGroup &target_group = groups[target];
    TaskRecordGroup &source_group = groups[source];
    for (const QVariantMap &record : source_group.records) {
        target_group.records.append(record);
        if (record_is_better_primary(record, target_group.primary)) {
            target_group.primary = record;
        }
    }
    target_group.modules.unite(source_group.modules);
    target_group.aliases.unite(source_group.aliases);
    for (const QString &alias : source_group.aliases) {
        alias_to_group.insert(alias, target);
    }
    source_group.active = false;
}

QVector<TaskRecordGroup> collect_productivity_task_groups(source::EisenhowerMatrixApp *app,
                                                          const QVariantList &unified_records,
                                                          const QJsonObject &mapamental_backup,
                                                          const QSet<QString> &selected_modules)
{
    QVector<TaskRecordGroup> groups;
    QHash<QString, int> alias_to_group;

    const QStringList modules =
        selected_modules.isEmpty() ? productivity_modules_order() : ordered_modules_from_set(selected_modules);
    for (const QString &module : modules) {
        const QVariantList module_records =
            records_for_productivity_module(app, unified_records, mapamental_backup, module);
        for (const QVariant &value : module_records) {
            QVariantMap record = value.toMap();
            const QString title = title_for_record(record);
            if (title.trimmed().isEmpty()) {
                continue;
            }

            const QStringList aliases = grouping_aliases_for_record(record);
            int group_index = -1;
            for (const QString &alias : aliases) {
                const int candidate = alias_to_group.value(alias, -1);
                if (candidate >= 0 && candidate < groups.size() && groups.at(candidate).active) {
                    if (group_index < 0) {
                        group_index = candidate;
                    } else if (group_index != candidate) {
                        merge_task_groups(groups, alias_to_group, group_index, candidate);
                    }
                }
            }

            if (group_index < 0) {
                TaskRecordGroup group;
                groups.append(group);
                group_index = groups.size() - 1;
            }

            add_record_to_group(groups[group_index], record, aliases);
            for (const QString &alias : aliases) {
                alias_to_group.insert(alias, group_index);
            }
        }
    }

    QVector<TaskRecordGroup> active_groups;
    active_groups.reserve(groups.size());
    for (const TaskRecordGroup &group : groups) {
        if (group.active && !group.records.isEmpty()) {
            active_groups.append(group);
        }
    }
    return active_groups;
}

bool group_has_selected_module(const TaskRecordGroup &group, const QSet<QString> &selected_modules)
{
    if (selected_modules.isEmpty()) {
        return true;
    }
    for (const QString &module : selected_modules) {
        if (group.modules.contains(module)) {
            return true;
        }
    }
    return false;
}

QStringList ordered_modules_for_group(const TaskRecordGroup &group, const QSet<QString> &selected_modules = {})
{
    QSet<QString> modules = group.modules;
    if (!selected_modules.isEmpty()) {
        modules.intersect(selected_modules);
    }
    return ordered_modules_from_set(modules);
}

bool group_all_completed(const TaskRecordGroup &group)
{
    if (group.records.isEmpty()) {
        return false;
    }
    for (const QVariantMap &record : group.records) {
        if (!record_completed(record)) {
            return false;
        }
    }
    return true;
}

bool group_has_open_record(const TaskRecordGroup &group)
{
    for (const QVariantMap &record : group.records) {
        if (!record_completed(record)) {
            return true;
        }
    }
    return false;
}

QString first_group_date_text(const TaskRecordGroup &group)
{
    QString fallback;
    for (const QVariantMap &record : group.records) {
        const QString text = date_for_record(record).trimmed();
        if (text.isEmpty()) {
            continue;
        }
        if (fallback.isEmpty()) {
            fallback = text;
        }
        if (QDate::fromString(text, Qt::ISODate).isValid()) {
            return text;
        }
    }
    return fallback;
}

QDate first_group_date(const TaskRecordGroup &group)
{
    for (const QVariantMap &record : group.records) {
        const QDate date = QDate::fromString(date_for_record(record), Qt::ISODate);
        if (date.isValid()) {
            return date;
        }
    }
    return {};
}

QTime first_group_time(const TaskRecordGroup &group)
{
    for (const QVariantMap &record : group.records) {
        const QTime time = parse_time(time_for_record(record));
        if (time.isValid()) {
            return time;
        }
    }
    return {};
}

QString first_group_time_text(const TaskRecordGroup &group)
{
    for (const QVariantMap &record : group.records) {
        const QString text = time_for_record(record).trimmed();
        if (!text.isEmpty()) {
            return text;
        }
    }
    return {};
}

QString group_recurrence(const TaskRecordGroup &group)
{
    for (const QVariantMap &record : group.records) {
        const QString recurrence = recurrence_for_record(record);
        if (!recurrence.isEmpty()) {
            return recurrence;
        }
    }
    return {};
}

QDate group_recurrence_until(const TaskRecordGroup &group)
{
    for (const QVariantMap &record : group.records) {
        const QDate until = QDate::fromString(recurrence_until_for_record(record), Qt::ISODate);
        if (until.isValid()) {
            return until;
        }
    }
    return {};
}

QVariantMap record_for_group_module(const TaskRecordGroup &group, const QString &module)
{
    const QString normalized = normalize_module_key(module);
    for (auto it = group.records.crbegin(); it != group.records.crend(); ++it) {
        if (record_module(*it) == normalized) {
            return *it;
        }
    }
    return group.primary;
}

QString source_id_for_group(const TaskRecordGroup &group)
{
    for (const QVariantMap &record : group.records) {
        QString id = record.value(QStringLiteral("integracao_task_id")).toString().trimmed();
        if (id.isEmpty()) {
            id = record.value(QStringLiteral("id")).toString().trimmed();
        }
        if (!id.isEmpty()) {
            return id;
        }
    }
    return record_source_id(group.primary, QStringLiteral("productivity"));
}

QVariantMap display_entry_for_group(const TaskRecordGroup &group)
{
    const QStringList modules = ordered_modules_for_group(group);
    const QString primary_module =
        modules.isEmpty() ? record_module(group.primary, QStringLiteral("eisenhower")) : modules.first();
    const QString title = title_for_record(group.primary);

    QStringList detail_parts;
    const QString date = first_group_date_text(group);
    const QString time = first_group_time_text(group);
    if (!date.isEmpty()) {
        detail_parts << date;
    }
    if (!time.isEmpty()) {
        detail_parts << time;
    }
    if (group_all_completed(group)) {
        detail_parts << get_text(QStringLiteral("concluída"));
    }

    QStringList searchable_parts;
    QVariantList grouped_records;
    for (const QVariantMap &record : group.records) {
        searchable_parts << searchable_text(record);
        grouped_records.append(record);
    }

    QVariantMap entry = group.primary;
    entry.insert(QStringLiteral("module"), primary_module);
    entry.insert(QStringLiteral("modulo"), primary_module);
    entry.insert(QStringLiteral("modules"), modules);
    entry.insert(QStringLiteral("records"), grouped_records);
    entry.insert(QStringLiteral("title"), title);
    entry.insert(QStringLiteral("detail"), detail_parts.join(QStringLiteral(" | ")));

    const QString prefix = module_list_label(modules);
    const QString detail = entry.value(QStringLiteral("detail")).toString();
    entry.insert(QStringLiteral("display"),
                 detail.trimmed().isEmpty()
                     ? QStringLiteral("[%1] %2").arg(prefix, title)
                     : QStringLiteral("[%1] %2 - %3").arg(prefix, title, detail));
    entry.insert(QStringLiteral("searchable"),
                 QStringLiteral("%1 %2 %3").arg(prefix, title, searchable_parts.join(QLatin1Char(' '))));
    return entry;
}

QSet<QString> existing_review_keys_from_records(const QVariantList &records)
{
    QSet<QString> keys;
    for (const QVariant &value : records) {
        const QVariantMap record = value.toMap();
        const QString origin = record.value(QStringLiteral("integracao_origem")).toString().trimmed().toCaseFolded();
        const QString source_id = record.value(QStringLiteral("integracao_task_id")).toString().trimmed();
        if (source_id.isEmpty()) {
            continue;
        }

        QString source_module;
        QString target_module;
        if (origin == QStringLiteral("feynman_spaced_review")) {
            source_module = QStringLiteral("feynman");
            target_module = QStringLiteral("eisenhower");
        } else if (origin.startsWith(QStringLiteral("spaced_review"))
                   || !record.value(QStringLiteral("review_source_module")).toString().trimmed().isEmpty()) {
            source_module = record.value(QStringLiteral("review_source_module")).toString().trimmed().toCaseFolded();
            target_module = record.value(QStringLiteral("review_target_module")).toString().trimmed().toCaseFolded();
            if (target_module.isEmpty()) {
                target_module = record.value(QStringLiteral("modulo")).toString().trimmed().toCaseFolded();
            }
        }

        if (source_module.isEmpty() || target_module.isEmpty()) {
            continue;
        }

        const QString due = record.value(QStringLiteral("review_due"), date_for_record(record)).toString();
        keys.insert(QStringLiteral("%1|%2|%3|%4").arg(source_module, target_module, source_id, due));
    }
    return keys;
}

QSet<QString> existing_mapamental_review_keys(const QJsonObject &root)
{
    QSet<QString> keys;
    const QJsonArray nodes = root.value(QStringLiteral("nodes")).toArray();
    for (const QJsonValue &node_value : nodes) {
        const QJsonObject node = node_value.toObject();
        const QStringList info = mapamental_info_list(node);
        const QString source_id =
            info_value_for_key(info, {QStringLiteral("review_source_id"), QStringLiteral("source_id")});
        if (source_id.isEmpty()) {
            continue;
        }
        const QString source_module =
            info_value_for_key(info, {QStringLiteral("review_source_module"), QStringLiteral("source_module")})
                .trimmed()
                .toCaseFolded();
        const QString due = info_value_for_key(info, {QStringLiteral("review_due"), QStringLiteral("due")});
        if (source_module.isEmpty()) {
            continue;
        }
        keys.insert(QStringLiteral("%1|mapamental|%2|%3").arg(source_module, source_id, due));
    }
    return keys;
}

void annotate_review_record(QVariantMap &record,
                            const QString &source_module,
                            const QString &target_module,
                            const QString &source_id,
                            const QDate &due,
                            int interval_days)
{
    record.insert(QStringLiteral("integracao_origem"), QStringLiteral("spaced_review"));
    record.insert(QStringLiteral("integracao_task_id"), source_id);
    record.insert(QStringLiteral("review_source_module"), source_module);
    record.insert(QStringLiteral("review_target_module"), target_module);
    record.insert(QStringLiteral("review_interval_days"), interval_days);
    if (due.isValid()) {
        record.insert(QStringLiteral("review_due"), due.toString(Qt::ISODate));
    }
}

QVariantMap display_entry(const QString &module,
                          const QString &title,
                          const QString &detail,
                          const QString &searchable,
                          const QVariantMap &payload = {})
{
    QVariantMap entry = payload;
    entry.insert(QStringLiteral("module"), module);
    entry.insert(QStringLiteral("title"), title);
    entry.insert(QStringLiteral("detail"), detail);
    entry.insert(QStringLiteral("display"),
                 detail.trimmed().isEmpty()
                     ? QStringLiteral("[%1] %2").arg(module_label(module), title)
                     : QStringLiteral("[%1] %2 - %3").arg(module_label(module), title, detail));
    entry.insert(QStringLiteral("searchable"), QStringLiteral("%1 %2 %3").arg(module, title, searchable));
    return entry;
}

QVariantList collect_global_search_entries(source::EisenhowerMatrixApp *app)
{
    QVariantList entries;
    const QVariantList unified_records = records_for_app(app);
    const QJsonObject map = read_json_object(mapamental_backup_path());
    QSet<QString> modules;
    for (const QString &module : productivity_modules_order()) {
        modules.insert(module);
    }
    const QVector<TaskRecordGroup> groups =
        collect_productivity_task_groups(app, unified_records, map, modules);
    for (const TaskRecordGroup &group : groups) {
        entries.append(display_entry_for_group(group));
    }

    const QString audiobook_text = current_audiolivro_text(app);
    if (!audiobook_text.isEmpty()) {
        QVariantMap payload;
        payload.insert(QStringLiteral("modulo"), QStringLiteral("audiolivro"));
        payload.insert(QStringLiteral("titulo"), get_text(QStringLiteral("Texto atual do Audiolivro")));
        entries.append(display_entry(QStringLiteral("audiolivro"),
                                     get_text(QStringLiteral("Texto atual do Audiolivro")),
                                     audiobook_text.left(160).simplified(),
                                     audiobook_text,
                                     payload));
    }

    return entries;
}

void open_module_for_entry(source::EisenhowerMatrixApp *app, const QVariantMap &entry)
{
    if (app == nullptr) {
        return;
    }
    QString module = entry.value(QStringLiteral("module"), entry.value(QStringLiteral("modulo"))).toString();
    if (module.trimmed().isEmpty()) {
        const QStringList modules = entry.value(QStringLiteral("modules")).toStringList();
        if (!modules.isEmpty()) {
            module = modules.first();
        }
    }
    module = normalize_module_key(module);
    if (module == QStringLiteral("feynman")) {
        app->open_feynman();
    } else if (module == QStringLiteral("pomodoro")) {
        app->open_pomodoro();
    } else if (module == QStringLiteral("mapamental")) {
        app->open_mapamental();
    } else if (module == QStringLiteral("audiolivro")) {
        app->open_audiolivro();
    } else if (app->module_tabs != nullptr && app->eisenhower_module_page != nullptr) {
        app->set_module_tab_visible(QStringLiteral("eisenhower"), true);
        app->module_tabs->setCurrentWidget(app->eisenhower_module_page);
    }
}

void show_model_dialog(source::EisenhowerMatrixApp *app,
                       const QString &title,
                       const QVariantList &records,
                       bool enable_filter)
{
    auto *dialog = new QDialog(app);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(title);
    dialog->resize(760, 520);

    auto *layout = new QVBoxLayout(dialog);
    auto *filter = new QLineEdit(dialog);
    filter->setPlaceholderText(get_text(QStringLiteral("Pesquisar")));
    filter->setVisible(enable_filter);
    layout->addWidget(filter);

    auto *model = new source::models::TaskRecordListModel(dialog);
    model->set_records(records);
    auto *proxy = new QSortFilterProxyModel(dialog);
    proxy->setSourceModel(model);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->setFilterRole(source::models::TaskRecordListModel::SearchableRole);

    auto *view = new QListView(dialog);
    view->setModel(proxy);
    view->setUniformItemSizes(false);
    view->setAlternatingRowColors(true);
    layout->addWidget(view, 1);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, dialog);
    translate_standard_buttons(app, buttons);
    layout->addWidget(buttons);

    QObject::connect(filter, &QLineEdit::textChanged, proxy, [proxy](const QString &text) {
        proxy->setFilterFixedString(text);
    });
    QObject::connect(view, &QListView::doubleClicked, dialog, [app, proxy](const QModelIndex &index) {
        open_module_for_entry(app, proxy->data(index, source::models::TaskRecordListModel::RecordRole).toMap());
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::close);
    dialog->show();
}

QDate next_recurrence_date(const QDate &date, const QString &recurrence)
{
    if (recurrence == QStringLiteral("daily")) {
        return date.addDays(1);
    }
    if (recurrence == QStringLiteral("weekly")) {
        return date.addDays(7);
    }
    if (recurrence == QStringLiteral("monthly")) {
        return date.addMonths(1);
    }
    return {};
}

QTime parse_time(const QString &text)
{
    const QString cleaned = text.trimmed();
    if (cleaned.isEmpty()) {
        return {};
    }

    QTime time = QTime::fromString(cleaned, QStringLiteral("HH:mm"));
    if (!time.isValid()) {
        time = QTime::fromString(cleaned, QStringLiteral("HH:mm:ss"));
    }
    if (!time.isValid()) {
        time = QTime::fromString(cleaned, Qt::ISODate);
    }
    if (!time.isValid()) {
        const QDateTime date_time = QDateTime::fromString(cleaned, Qt::ISODate);
        if (date_time.isValid()) {
            time = date_time.time();
        }
    }
    return time;
}

QString ics_escape(QString text)
{
    text.replace(QStringLiteral("\\"), QStringLiteral("\\\\"));
    text.replace(QStringLiteral(";"), QStringLiteral("\\;"));
    text.replace(QStringLiteral(","), QStringLiteral("\\,"));
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\\n"));
    text.replace(QLatin1Char('\n'), QStringLiteral("\\n"));
    return text;
}

QVariantMap eisenhower_task_record(const QString &title, const QDate &date, int priority, const QString &description = {})
{
    return source::utils::UnifiedTaskStore::normalize_record({
        {QStringLiteral("id"), QUuid::createUuid().toString(QUuid::WithoutBraces)},
        {QStringLiteral("modulo"), QStringLiteral("eisenhower")},
        {QStringLiteral("tipo"), QStringLiteral("tarefa")},
        {QStringLiteral("titulo"), title},
        {QStringLiteral("descricao"), description},
        {QStringLiteral("prioridade"), priority},
        {QStringLiteral("quadrante"), priority},
        {QStringLiteral("categoria"), QStringLiteral("quadrant%1").arg(qBound(1, priority, 4))},
        {QStringLiteral("status"), QStringLiteral("Todo")},
        {QStringLiteral("concluida"), false},
        {QStringLiteral("data"), date.toString(Qt::ISODate)},
        {QStringLiteral("data_criacao"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},
    });
}

void append_records_and_reload(source::EisenhowerMatrixApp *app, const QVariantList &new_records)
{
    if (app == nullptr || new_records.isEmpty()) {
        return;
    }
    QVariantList records = records_for_app(app);
    for (const QVariant &record : new_records) {
        records.append(record);
    }
    if (source::utils::UnifiedTaskStore::save_records(app->tasks_path, records)) {
        app->reload_all_modules_from_shared_store();
        if (app->calendar_pane != nullptr && app->calendar_pane->calendar_panel != nullptr) {
            app->calendar_pane->calendar_panel->update_task_list();
        }
    }
}

bool restore_spaced_review_snapshot(source::EisenhowerMatrixApp *app, const QSet<QString> &modules)
{
    if (app == nullptr || !g_spaced_review_undo.has_value() || modules.isEmpty()) {
        return false;
    }

    const SpacedReviewUndoSnapshot snapshot = g_spaced_review_undo.value();
    QVariantList current = source::utils::UnifiedTaskStore::load_records(app->tasks_path);
    QVariantList restored = current;
    for (const QString &module : modules) {
        const QString normalized = module.trimmed().toCaseFolded();
        if (normalized.isEmpty()) {
            continue;
        }
        const QVariantList module_records =
            source::utils::UnifiedTaskStore::filter_records_by_module(snapshot.records_before, normalized);
        restored = source::utils::UnifiedTaskStore::merge_module_records(restored, normalized, module_records);
    }

    const bool saved = source::utils::UnifiedTaskStore::save_records(app->tasks_path, restored);
    if (saved) {
        app->reload_all_modules_from_shared_store();
    }

    if (modules.contains(QStringLiteral("mapamental"))) {
        write_json_object(mapamental_backup_path(), snapshot.mapamental_backup_before);
        if (app->has_mapamental_plugin()) {
            app->reload_mapamental_from_backup();
        }
    }

    return saved;
}

void show_spaced_review_undo_dialog(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }

    if (!g_spaced_review_undo.has_value()) {
        QMessageBox::information(app,
                                 get_text(QStringLiteral("Revisão espaçada")),
                                 get_text(QStringLiteral("Não há execução anterior para desfazer.")));
        return;
    }

    const SpacedReviewUndoSnapshot snapshot = g_spaced_review_undo.value();
    if (snapshot.affected_modules.isEmpty()) {
        QMessageBox::information(app,
                                 get_text(QStringLiteral("Revisão espaçada")),
                                 get_text(QStringLiteral("Nenhum módulo foi alterado na última execução.")));
        return;
    }

    QDialog dialog(app);
    dialog.setWindowTitle(get_text(QStringLiteral("Desfazer revisões espaçadas")));
    dialog.resize(520, 320);
    auto *layout = new QVBoxLayout(&dialog);

    auto *summary = new QLabel(&dialog);
    summary->setWordWrap(true);
    summary->setText(get_text(QStringLiteral("Selecione o que deseja restaurar da última execução (%1)."))
                         .arg(snapshot.created_at.toString(QStringLiteral("dd/MM/yyyy HH:mm"))));
    layout->addWidget(summary);

    QMap<QString, QCheckBox *> module_checks;
    const QStringList ordered = {QStringLiteral("eisenhower"),
                                 QStringLiteral("pomodoro"),
                                 QStringLiteral("feynman"),
                                 QStringLiteral("mapamental")};
    for (const QString &module : ordered) {
        if (!snapshot.affected_modules.contains(module)) {
            continue;
        }
        auto *check = new QCheckBox(module_label(module), &dialog);
        check->setChecked(true);
        layout->addWidget(check);
        module_checks.insert(module, check);
    }

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    translate_standard_buttons(app, buttons);
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QSet<QString> modules;
    for (auto it = module_checks.constBegin(); it != module_checks.constEnd(); ++it) {
        if (it.value()->isChecked()) {
            modules.insert(it.key());
        }
    }

    if (modules.isEmpty()) {
        return;
    }

    const bool restored = restore_spaced_review_snapshot(app, modules);
    QMessageBox::information(app,
                             get_text(QStringLiteral("Revisão espaçada")),
                             restored ? get_text(QStringLiteral("Restauração concluída."))
                                      : get_text(QStringLiteral("Não foi possível restaurar.")));
}

bool restore_recurring_tasks_snapshot(source::EisenhowerMatrixApp *app, const QSet<QString> &modules)
{
    const std::optional<RecurringTasksUndoSnapshot> snapshot_value = current_recurring_tasks_undo_snapshot();
    if (app == nullptr || !snapshot_value.has_value() || modules.isEmpty()) {
        return false;
    }

    const RecurringTasksUndoSnapshot snapshot = snapshot_value.value();
    QVariantList current = source::utils::UnifiedTaskStore::load_records(app->tasks_path);
    QVariantList restored = current;
    for (const QString &module : modules) {
        const QString normalized = module.trimmed().toCaseFolded();
        if (normalized.isEmpty()) {
            continue;
        }
        const QVariantList module_records =
            source::utils::UnifiedTaskStore::filter_records_by_module(snapshot.records_before, normalized);
        restored = source::utils::UnifiedTaskStore::merge_module_records(restored, normalized, module_records);
    }

    const bool saved_records = source::utils::UnifiedTaskStore::save_records(app->tasks_path, restored);
    if (saved_records) {
        app->reload_all_modules_from_shared_store();
    }

    bool saved_map = true;
    if (modules.contains(QStringLiteral("mapamental"))) {
        saved_map = write_json_object(mapamental_backup_path(), snapshot.mapamental_backup_before);
        if (saved_map && app->has_mapamental_plugin()) {
            app->reload_mapamental_from_backup();
        }
    }

    if ((saved_records || saved_map) && app->calendar_pane != nullptr && app->calendar_pane->calendar_panel != nullptr) {
        app->calendar_pane->calendar_panel->update_task_list();
    }

    return saved_records && saved_map;
}

bool show_recurring_tasks_restore_dialog(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return false;
    }

    const std::optional<RecurringTasksUndoSnapshot> snapshot_value = current_recurring_tasks_undo_snapshot();
    if (!snapshot_value.has_value()) {
        QMessageBox::information(app,
                                 get_text(QStringLiteral("Restaurar recorrências")),
                                 get_text(QStringLiteral("Não há geração anterior de recorrências para restaurar.")));
        return false;
    }

    const RecurringTasksUndoSnapshot snapshot = snapshot_value.value();
    if (snapshot.affected_modules.isEmpty()) {
        QMessageBox::information(app,
                                 get_text(QStringLiteral("Restaurar recorrências")),
                                 get_text(QStringLiteral("Nenhum módulo foi alterado na última geração de recorrências.")));
        return false;
    }

    QDialog dialog(app);
    dialog.setWindowTitle(get_text(QStringLiteral("Restaurar recorrências")));
    dialog.resize(520, 320);
    auto *layout = new QVBoxLayout(&dialog);

    auto *summary = new QLabel(&dialog);
    summary->setWordWrap(true);
    summary->setText(get_text(QStringLiteral(
                         "Selecione o que deseja restaurar da última geração de recorrências (%1)."))
                         .arg(snapshot.created_at.toString(QStringLiteral("dd/MM/yyyy HH:mm"))));
    layout->addWidget(summary);

    QMap<QString, QCheckBox *> module_checks;
    const QStringList ordered = {QStringLiteral("eisenhower"),
                                 QStringLiteral("pomodoro"),
                                 QStringLiteral("feynman"),
                                 QStringLiteral("mapamental")};
    for (const QString &module : ordered) {
        if (!snapshot.affected_modules.contains(module)) {
            continue;
        }
        auto *check = new QCheckBox(module_label(module), &dialog);
        check->setChecked(true);
        layout->addWidget(check);
        module_checks.insert(module, check);
    }

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    translate_standard_buttons(app, buttons);
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }

    QSet<QString> modules;
    for (auto it = module_checks.constBegin(); it != module_checks.constEnd(); ++it) {
        if (it.value()->isChecked()) {
            modules.insert(it.key());
        }
    }
    if (modules.isEmpty()) {
        return false;
    }

    const auto confirm = QMessageBox::question(
        app,
        get_text(QStringLiteral("Restaurar recorrências")),
        get_text(QStringLiteral(
            "Deseja restaurar o estado anterior às recorrências selecionadas? Alterações posteriores nesses módulos podem ser substituídas.")),
        QMessageBox::Yes | QMessageBox::No);
    if (confirm != QMessageBox::Yes) {
        return false;
    }

    const bool restored = restore_recurring_tasks_snapshot(app, modules);
    QMessageBox::information(app,
                             get_text(QStringLiteral("Restaurar recorrências")),
                             restored ? get_text(QStringLiteral("Estado original restaurado."))
                                      : get_text(QStringLiteral("Não foi possível restaurar o estado original.")));
    return restored;
}

struct SpacedReviewDraft {
    QString title;
    QString source_title;
    QString description;
    QString date_iso;
    QString time_str;
    QString file_path;
    QString source_module;
    QString source_id;
    int priority = 2;
    int domain = 0;
    int interval_days = 0;
};

QString file_path_for_record(const QVariantMap &record)
{
    QString path = record.value(QStringLiteral("arquivo_vinculado")).toString().trimmed();
    if (path.isEmpty()) {
        path = record.value(QStringLiteral("file_path")).toString().trimmed();
    }
    return path;
}

QString priority_label(int priority)
{
    switch (qBound(1, priority, 4)) {
    case 2:
        return get_text(QStringLiteral("🟠 Importante, mas Não Urgente"));
    case 3:
        return get_text(QStringLiteral("🟡 Não Importante, mas Urgente"));
    case 4:
        return get_text(QStringLiteral("🟢 Não Importante e Não Urgente"));
    default:
        return get_text(QStringLiteral("🔴 Importante e Urgente"));
    }
}

QString domain_label(int domain)
{
    switch (qBound(0, domain, 2)) {
    case 1:
        return get_text(QStringLiteral("🥈 Intermediário"));
    case 2:
        return get_text(QStringLiteral("🥇 Avançado"));
    default:
        return get_text(QStringLiteral("🥉 Iniciante"));
    }
}

QString due_key_for_draft(const SpacedReviewDraft &draft)
{
    const QDate due = QDate::fromString(draft.date_iso, Qt::ISODate);
    return due.isValid() ? due.toString(Qt::ISODate) : QString();
}

QString due_display_for_draft(const SpacedReviewDraft &draft)
{
    const QDate due = QDate::fromString(draft.date_iso, Qt::ISODate);
    return due.isValid() ? due.toString(QStringLiteral("dd/MM/yyyy")) : get_text(QStringLiteral("Sem data"));
}

QVariantMap review_record_for_target(const SpacedReviewDraft &draft, const QString &target_module)
{
    const QString target = target_module.trimmed().toCaseFolded();
    const QDate due = QDate::fromString(draft.date_iso, Qt::ISODate);
    QVariantMap review_record;

    if (target == QStringLiteral("eisenhower")) {
        review_record = eisenhower_task_record(draft.title, due, draft.priority, draft.description);
        review_record.insert(QStringLiteral("dominio"), draft.domain);
        review_record.insert(QStringLiteral("hora"), draft.time_str);
    } else if (target == QStringLiteral("pomodoro")) {
        review_record = {
            {QStringLiteral("id"), QUuid::createUuid().toString(QUuid::WithoutBraces)},
            {QStringLiteral("modulo"), QStringLiteral("pomodoro")},
            {QStringLiteral("tipo"), QStringLiteral("tarefa")},
            {QStringLiteral("titulo"), draft.title},
            {QStringLiteral("descricao"), draft.description},
            {QStringLiteral("prioridade"), draft.priority},
            {QStringLiteral("quadrante"), draft.priority},
            {QStringLiteral("categoria"), QStringLiteral("kanban")},
            {QStringLiteral("dominio"), draft.domain},
            {QStringLiteral("status"), QStringLiteral("Todo")},
            {QStringLiteral("concluida"), false},
            {QStringLiteral("data"), draft.date_iso},
            {QStringLiteral("hora"), draft.time_str},
            {QStringLiteral("data_criacao"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},
            {QStringLiteral("pomodoros"), 0},
            {QStringLiteral("etapas"), QVariantList{}},
            {QStringLiteral("sincronizada_com_eisenhower"), false},
        };
    } else if (target == QStringLiteral("feynman")) {
        review_record = {
            {QStringLiteral("id"), QUuid::createUuid().toString(QUuid::WithoutBraces)},
            {QStringLiteral("modulo"), QStringLiteral("feynman")},
            {QStringLiteral("tipo"), QStringLiteral("conceito")},
            {QStringLiteral("titulo"), draft.title},
            {QStringLiteral("descricao"), draft.description},
            {QStringLiteral("explicacao"), draft.description},
            {QStringLiteral("lacunas"), QString()},
            {QStringLiteral("revisao"), QString()},
            {QStringLiteral("descricao_customizada"), !draft.description.trimmed().isEmpty()},
            {QStringLiteral("prioridade"), draft.priority},
            {QStringLiteral("quadrante"), draft.priority},
            {QStringLiteral("categoria"), QStringLiteral("conceito")},
            {QStringLiteral("dominio"), draft.domain},
            {QStringLiteral("status"), QStringLiteral("Todo")},
            {QStringLiteral("concluida"), false},
            {QStringLiteral("data"), draft.date_iso},
            {QStringLiteral("hora"), draft.time_str},
        };
    } else if (target == QStringLiteral("mapamental")) {
        review_record = {
            {QStringLiteral("id"), QUuid::createUuid().toString(QUuid::WithoutBraces)},
            {QStringLiteral("modulo"), QStringLiteral("mapamental")},
            {QStringLiteral("tipo"), QStringLiteral("conceito")},
            {QStringLiteral("titulo"), draft.title},
            {QStringLiteral("descricao"), draft.description},
            {QStringLiteral("prioridade"), draft.priority},
            {QStringLiteral("quadrante"), draft.priority},
            {QStringLiteral("categoria"), QStringLiteral("mapa")},
            {QStringLiteral("dominio"), draft.domain},
            {QStringLiteral("status"), QStringLiteral("Todo")},
            {QStringLiteral("concluida"), false},
            {QStringLiteral("data"), draft.date_iso},
            {QStringLiteral("hora"), draft.time_str},
            {QStringLiteral("data_criacao"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},
        };
    }

    if (review_record.isEmpty()) {
        return {};
    }

    if (!draft.file_path.trimmed().isEmpty()) {
        review_record.insert(QStringLiteral("arquivo_vinculado"), draft.file_path.trimmed());
    }
    annotate_review_record(review_record,
                           draft.source_module,
                           target,
                           draft.source_id,
                           due,
                           draft.interval_days);
    return source::utils::UnifiedTaskStore::normalize_record(review_record);
}

QSet<QString> checked_review_targets(const QMap<QString, QCheckBox *> &checks)
{
    QSet<QString> targets;
    for (auto it = checks.constBegin(); it != checks.constEnd(); ++it) {
        if (it.value() != nullptr && it.value()->isChecked()) {
            targets.insert(it.key());
        }
    }
    return targets;
}

bool export_spaced_review_drafts(source::EisenhowerMatrixApp *app,
                                 const QVector<SpacedReviewDraft> &drafts,
                                 const QSet<QString> &targets)
{
    if (app == nullptr || drafts.isEmpty()) {
        return false;
    }

    if (targets.isEmpty()) {
        QMessageBox::warning(app,
                             get_text(QStringLiteral("Revisão espaçada")),
                             get_text(QStringLiteral("Selecione pelo menos uma aba de destino.")));
        return false;
    }

    const QVariantList unified_records = records_for_app(app);
    const QJsonObject map_backup = read_json_object(mapamental_backup_path());
    QSet<QString> existing_keys = existing_review_keys_from_records(unified_records);
    QSet<QString> existing_map_keys = existing_mapamental_review_keys(map_backup);
    QVariantList additions;
    QJsonObject updated_map = map_backup;
    QJsonArray nodes = updated_map.value(QStringLiteral("nodes")).toArray();
    int nodes_added = 0;
    QMap<QString, int> created_counts;
    QSet<QString> changed_modules;

    for (const SpacedReviewDraft &draft : drafts) {
        if (draft.title.trimmed().isEmpty() || draft.source_id.trimmed().isEmpty()) {
            continue;
        }

        const QString due_key = due_key_for_draft(draft);
        const QDate due = QDate::fromString(draft.date_iso, Qt::ISODate);
        for (const QString &target_module : targets) {
            const QString target = target_module.trimmed().toCaseFolded();
            const QString key = QStringLiteral("%1|%2|%3|%4")
                                    .arg(draft.source_module, target, draft.source_id, due_key);
            if (existing_keys.contains(key)) {
                continue;
            }

            if (target == QStringLiteral("mapamental")) {
                if (existing_map_keys.contains(key)) {
                    continue;
                }

                QJsonObject node;
                node.insert(QStringLiteral("texto"), draft.title);
                node.insert(QStringLiteral("x"), 80 + (nodes.size() + nodes_added) * 24);
                node.insert(QStringLiteral("y"), 80 + (nodes.size() + nodes_added) * 16);
                node.insert(QStringLiteral("cor"), QStringLiteral("#ff0078d4"));

                QStringList notes;
                notes << get_text(QStringLiteral("Revisão de: %1")).arg(draft.source_title);
                if (due.isValid()) {
                    notes << get_text(QStringLiteral("Data: %1")).arg(due.toString(QStringLiteral("dd/MM/yyyy")));
                }
                if (!draft.time_str.trimmed().isEmpty()) {
                    notes << get_text(QStringLiteral("Horário: %1")).arg(draft.time_str.trimmed());
                }
                if (!draft.file_path.trimmed().isEmpty()) {
                    notes << get_text(QStringLiteral("Arquivo: %1")).arg(draft.file_path.trimmed());
                }
                if (!draft.description.trimmed().isEmpty()) {
                    notes << draft.description.trimmed();
                }
                node.insert(QStringLiteral("notas"), notes.join(QLatin1Char('\n')));

                const QString node_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
                QJsonArray info;
                info.append(QStringLiteral("node_id=%1").arg(node_id));
                info.append(QStringLiteral("review_source_module=%1").arg(draft.source_module));
                info.append(QStringLiteral("review_source_id=%1").arg(draft.source_id));
                info.append(QStringLiteral("review_interval_days=%1").arg(draft.interval_days));
                info.append(QStringLiteral("dominio=%1").arg(draft.domain));
                info.append(QStringLiteral("prioridade=%1").arg(draft.priority));
                if (due.isValid()) {
                    info.append(QStringLiteral("review_due=%1").arg(due.toString(Qt::ISODate)));
                }
                if (!draft.file_path.trimmed().isEmpty()) {
                    info.append(QStringLiteral("arquivo_vinculado=%1").arg(draft.file_path.trimmed()));
                }
                node.insert(QStringLiteral("info_associada"), info);

                nodes.append(node);
                ++nodes_added;
                existing_map_keys.insert(key);
                existing_keys.insert(key);
                created_counts[target] += 1;
                changed_modules.insert(target);

                if (due.isValid()) {
                    const QVariantMap map_record = review_record_for_target(draft, target);
                    if (!map_record.isEmpty()) {
                        additions.append(map_record);
                    }
                }
                continue;
            }

            const QVariantMap review_record = review_record_for_target(draft, target);
            if (review_record.isEmpty()) {
                continue;
            }

            additions.append(review_record);
            existing_keys.insert(key);
            created_counts[target] += 1;
            changed_modules.insert(target);
        }
    }

    if (additions.isEmpty() && nodes_added == 0) {
        QMessageBox::information(app,
                                 get_text(QStringLiteral("Revisão espaçada")),
                                 get_text(QStringLiteral("Nenhuma revisão nova foi transferida.")));
        return false;
    }

    SpacedReviewUndoSnapshot snapshot;
    snapshot.records_before = unified_records;
    snapshot.mapamental_backup_before = map_backup;
    snapshot.affected_modules = changed_modules;
    snapshot.created_at = QDateTime::currentDateTime();
    g_spaced_review_undo = snapshot;

    bool saved_records = true;
    if (!additions.isEmpty()) {
        QVariantList merged = unified_records;
        for (const QVariant &record : additions) {
            merged.append(record);
        }
        saved_records = source::utils::UnifiedTaskStore::save_records(app->tasks_path, merged);
        if (saved_records) {
            app->reload_all_modules_from_shared_store();
        }
    }

    bool saved_map = true;
    if (nodes_added > 0) {
        updated_map.insert(QStringLiteral("version"), 2);
        updated_map.insert(QStringLiteral("saved_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
        updated_map.insert(QStringLiteral("nodes"), nodes);
        if (!updated_map.contains(QStringLiteral("connections"))) {
            updated_map.insert(QStringLiteral("connections"), QJsonArray{});
        }
        saved_map = write_json_object(mapamental_backup_path(), updated_map);
        if (saved_map && app->has_mapamental_plugin()) {
            app->reload_mapamental_from_backup();
        }
    }

    QStringList lines;
    int total_created = 0;
    const QStringList ordered_targets = {QStringLiteral("eisenhower"),
                                         QStringLiteral("pomodoro"),
                                         QStringLiteral("feynman"),
                                         QStringLiteral("mapamental")};
    for (const QString &module : ordered_targets) {
        if (created_counts.contains(module)) {
            const int count = created_counts.value(module);
            total_created += count;
            lines << get_text(QStringLiteral("%1: %2")).arg(module_label(module)).arg(count);
        }
    }

    QString header = get_text(QStringLiteral("%1 revisão(ões) transferida(s).")).arg(total_created);
    if (!saved_records || !saved_map) {
        header = get_text(QStringLiteral("Revisões transferidas com avisos."));
    }

    QMessageBox msg(app);
    msg.setWindowTitle(get_text(QStringLiteral("Revisão espaçada")));
    msg.setText(lines.isEmpty() ? header : QStringLiteral("%1\n%2").arg(header, lines.join(QLatin1Char('\n'))));
    QPushButton *undo_button = msg.addButton(get_text(QStringLiteral("Desfazer")), QMessageBox::ActionRole);
    msg.addButton(QMessageBox::Ok);
    translate_standard_buttons(app, &msg);
    msg.exec();
    if (msg.clickedButton() == undo_button) {
        show_spaced_review_undo_dialog(app);
    }

    return saved_records || saved_map;
}

void show_spaced_review_window(source::EisenhowerMatrixApp *app,
                               QVector<SpacedReviewDraft> drafts,
                               const QSet<QString> &default_targets)
{
    if (app == nullptr || drafts.isEmpty()) {
        return;
    }

    QDialog dialog(app);
    dialog.setWindowTitle(get_text(QStringLiteral("Janela de revisão espaçada")));
    dialog.resize(960, 640);
    auto *layout = new QVBoxLayout(&dialog);

    auto *summary = new QLabel(&dialog);
    summary->setWordWrap(true);
    layout->addWidget(summary);

    auto *table = new QTableWidget(&dialog);
    table->setColumnCount(7);
    table->setHorizontalHeaderLabels({get_text(QStringLiteral("Tarefa")),
                                      get_text(QStringLiteral("Data")),
                                      get_text(QStringLiteral("Horário")),
                                      get_text(QStringLiteral("Prioridade")),
                                      get_text(QStringLiteral("Domínio")),
                                      get_text(QStringLiteral("Origem")),
                                      get_text(QStringLiteral("Arquivo"))});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    layout->addWidget(table, 1);

    auto *targets_group = new QGroupBox(get_text(QStringLiteral("Transferir para")), &dialog);
    auto *targets_layout = new QGridLayout(targets_group);
    QMap<QString, QCheckBox *> target_checks;
    target_checks.insert(QStringLiteral("eisenhower"),
                         new QCheckBox(get_text(QStringLiteral("Matriz Eisenhower")), targets_group));
    target_checks.insert(QStringLiteral("pomodoro"), new QCheckBox(get_text(QStringLiteral("Pomodoro")), targets_group));
    target_checks.insert(QStringLiteral("feynman"), new QCheckBox(get_text(QStringLiteral("Feynman")), targets_group));
    target_checks.insert(QStringLiteral("mapamental"),
                         new QCheckBox(get_text(QStringLiteral("Mapa Mental")), targets_group));
    for (auto it = target_checks.begin(); it != target_checks.end(); ++it) {
        it.value()->setChecked(default_targets.contains(it.key()));
    }
    if (checked_review_targets(target_checks).isEmpty()) {
        target_checks.value(QStringLiteral("eisenhower"))->setChecked(true);
    }
    targets_layout->addWidget(target_checks.value(QStringLiteral("eisenhower")), 0, 0);
    targets_layout->addWidget(target_checks.value(QStringLiteral("pomodoro")), 0, 1);
    targets_layout->addWidget(target_checks.value(QStringLiteral("feynman")), 1, 0);
    targets_layout->addWidget(target_checks.value(QStringLiteral("mapamental")), 1, 1);
    layout->addWidget(targets_group);

    auto *button_row = new QHBoxLayout();
    auto *edit_button = new QPushButton(get_text(QStringLiteral("Editar na janela...")), &dialog);
    auto *remove_button = new QPushButton(get_text(QStringLiteral("Remover da revisão")), &dialog);
    auto *transfer_button = new QPushButton(get_text(QStringLiteral("Transferir para abas selecionadas")), &dialog);
    auto *close_button = new QPushButton(get_text(QStringLiteral("Fechar")), &dialog);
    button_row->addWidget(edit_button);
    button_row->addWidget(remove_button);
    button_row->addStretch(1);
    button_row->addWidget(transfer_button);
    button_row->addWidget(close_button);
    layout->addLayout(button_row);

    auto set_item = [table](int row, int column, const QString &text) {
        auto *item = new QTableWidgetItem(text);
        item->setData(Qt::UserRole, row);
        table->setItem(row, column, item);
    };

    auto refresh_table = [&]() {
        const int previous_row = table->currentRow();
        table->setRowCount(drafts.size());
        for (int row = 0; row < drafts.size(); ++row) {
            const SpacedReviewDraft &draft = drafts.at(row);
            set_item(row, 0, draft.title);
            set_item(row, 1, due_display_for_draft(draft));
            set_item(row, 2, draft.time_str);
            set_item(row, 3, priority_label(draft.priority));
            set_item(row, 4, domain_label(draft.domain));
            set_item(row, 5, module_label(draft.source_module));
            set_item(row, 6, QFileInfo(draft.file_path).fileName());
        }
        summary->setText(get_text(QStringLiteral("%1 revisão(ões) aguardando transferência.")).arg(drafts.size()));
        const bool has_rows = !drafts.isEmpty();
        edit_button->setEnabled(has_rows);
        remove_button->setEnabled(has_rows);
        transfer_button->setEnabled(has_rows);
        if (has_rows) {
            table->selectRow(qBound(0, previous_row, drafts.size() - 1));
        }
    };

    auto edit_selected = [&]() {
        const int row = table->currentRow();
        if (row < 0 || row >= drafts.size()) {
            return;
        }

        SpacedReviewDraft &draft = drafts[row];
        source::InterfaceCore::incore_14_EditTaskDialog::EditTaskDialog edit_dialog(app,
                                                                                    &dialog,
                                                                                    draft.title,
                                                                                    draft.file_path,
                                                                                    draft.description,
                                                                                    draft.date_iso,
                                                                                    draft.time_str,
                                                                                    draft.priority,
                                                                                    draft.domain);
        if (edit_dialog.exec() != QDialog::Accepted) {
            return;
        }

        const auto values = edit_dialog.get_values();
        if (values.text.trimmed().isEmpty()) {
            QMessageBox::warning(&dialog,
                                 get_text(QStringLiteral("Revisão espaçada")),
                                 get_text(QStringLiteral("A tarefa precisa ter um título.")));
            return;
        }

        draft.title = values.text.trimmed();
        draft.file_path = values.file_path.trimmed();
        draft.description = values.description;
        draft.date_iso = values.date_iso;
        draft.time_str = values.time_str;
        draft.priority = values.priority;
        draft.domain = values.domain;
        refresh_table();
        table->selectRow(row);
    };

    QObject::connect(edit_button, &QPushButton::clicked, &dialog, edit_selected);
    QObject::connect(table, &QTableWidget::itemDoubleClicked, &dialog, [edit_selected](QTableWidgetItem *) {
        edit_selected();
    });
    QObject::connect(remove_button, &QPushButton::clicked, &dialog, [&]() {
        const int row = table->currentRow();
        if (row < 0 || row >= drafts.size()) {
            return;
        }
        drafts.removeAt(row);
        refresh_table();
    });
    QObject::connect(transfer_button, &QPushButton::clicked, &dialog, [&]() {
        if (export_spaced_review_drafts(app, drafts, checked_review_targets(target_checks))) {
            dialog.accept();
        }
    });
    QObject::connect(close_button, &QPushButton::clicked, &dialog, &QDialog::reject);

    refresh_table();
    dialog.exec();
}

}  // namespace

namespace {
QString module_checkbox_label(const QString &module)
{
    const QString normalized = normalize_module_key(module);
    if (normalized == QStringLiteral("eisenhower")) {
        return get_text(QStringLiteral("Matriz Eisenhower"));
    }
    return module_label(normalized);
}

QMap<QString, QCheckBox *> create_productivity_module_checks(QWidget *parent,
                                                             const QSet<QString> &checked_modules,
                                                             const QStringList &visible_modules = productivity_modules_order())
{
    QMap<QString, QCheckBox *> checks;
    const QStringList modules = visible_modules.isEmpty() ? productivity_modules_order() : visible_modules;
    for (const QString &module : modules) {
        const QString normalized = normalize_module_key(module);
        if (normalized.isEmpty() || checks.contains(normalized)) {
            continue;
        }
        auto *check = new QCheckBox(module_checkbox_label(normalized), parent);
        check->setChecked(checked_modules.isEmpty() || checked_modules.contains(normalized));
        checks.insert(normalized, check);
    }
    return checks;
}

QVector<QWidget *> widgets_for_productivity_modules(const QMap<QString, QCheckBox *> &checks,
                                                    const QStringList &modules)
{
    QVector<QWidget *> widgets;
    for (const QString &module : modules) {
        QCheckBox *check = checks.value(normalize_module_key(module), nullptr);
        if (check != nullptr) {
            widgets.append(check);
        }
    }
    return widgets;
}

QStringList checked_productivity_modules(const QMap<QString, QCheckBox *> &checks,
                                         const QStringList &visible_modules = productivity_modules_order())
{
    QStringList modules;
    const QStringList modules_to_check = visible_modules.isEmpty() ? productivity_modules_order() : visible_modules;
    for (const QString &module : modules_to_check) {
        const QString normalized = normalize_module_key(module);
        QCheckBox *check = checks.value(normalized, nullptr);
        if (check != nullptr && check->isChecked()) {
            modules.append(normalized);
        }
    }
    return modules;
}

QSet<QString> checked_productivity_module_set(const QMap<QString, QCheckBox *> &checks)
{
    QSet<QString> modules;
    for (const QString &module : checked_productivity_modules(checks)) {
        modules.insert(module);
    }
    return modules;
}

void ensure_any_productivity_module_checked(const QMap<QString, QCheckBox *> &checks)
{
    if (!checked_productivity_module_set(checks).isEmpty()) {
        return;
    }
    if (QCheckBox *eisenhower = checks.value(QStringLiteral("eisenhower"), nullptr)) {
        eisenhower->setChecked(true);
        return;
    }
    for (QCheckBox *check : checks) {
        if (check != nullptr) {
            check->setChecked(true);
            return;
        }
    }
}

int group_count_for_modules(const QVector<TaskRecordGroup> &groups, const QSet<QString> &modules)
{
    int count = 0;
    for (const TaskRecordGroup &group : groups) {
        if (group_has_selected_module(group, modules)) {
            ++count;
        }
    }
    return count;
}

int recurring_group_count_for_modules(const QVector<TaskRecordGroup> &groups, const QSet<QString> &modules)
{
    int count = 0;
    for (const TaskRecordGroup &group : groups) {
        if (group_has_selected_module(group, modules) && !group_recurrence(group).isEmpty()) {
            ++count;
        }
    }
    return count;
}

int dated_group_count_for_modules(const QVector<TaskRecordGroup> &groups, const QSet<QString> &modules)
{
    int count = 0;
    for (const TaskRecordGroup &group : groups) {
        if (group_has_selected_module(group, modules) && first_group_date(group).isValid()) {
            ++count;
        }
    }
    return count;
}

bool time_in_reminder_interval(const QTime &time, const QTime &start, const QTime &end)
{
    if (!time.isValid() || !start.isValid() || !end.isValid()) {
        return false;
    }
    if (start <= end) {
        return time >= start && time <= end;
    }
    return time >= start || time <= end;
}

std::optional<QDateTime> reminder_due_for_group(const TaskRecordGroup &group,
                                                bool use_default_time,
                                                const QTime &default_time)
{
    const QDate date = first_group_date(group);
    if (!date.isValid()) {
        return std::nullopt;
    }

    QTime time = first_group_time(group);
    if (!time.isValid()) {
        if (!use_default_time || !default_time.isValid()) {
            return std::nullopt;
        }
        time = default_time;
    }

    const QDateTime due(date, time);
    return due.isValid() ? std::optional<QDateTime>(due) : std::nullopt;
}

bool reminder_due_in_window(const QDateTime &due,
                            const QDateTime &now,
                            const QDate &start_date,
                            const QDate &end_date,
                            const QTime &start_time,
                            const QTime &end_time)
{
    if (!due.isValid()) {
        return false;
    }

    const QDate date = due.date();
    if (!start_date.isValid() || !end_date.isValid() || date < start_date || date > end_date) {
        return false;
    }

    if (date < now.date() && due.addSecs(kReminderPastGraceSeconds) < now) {
        return false;
    }

    return time_in_reminder_interval(due.time(), start_time, end_time);
}

bool reminder_due_is_schedulable(const QDateTime &due, const QDateTime &now)
{
    if (!due.isValid()) {
        return false;
    }
    if (due.date() == now.date()) {
        return true;
    }
    return due.addSecs(kReminderPastGraceSeconds) >= now;
}

QDateTime suggested_custom_reminder_datetime()
{
    const QDateTime now = QDateTime::currentDateTime();
    QDateTime suggested = now.addSecs(5 * 60);
    suggested.setTime(QTime(suggested.time().hour(), suggested.time().minute()));
    if (suggested <= now) {
        suggested = suggested.addSecs(60);
    }
    return suggested;
}

struct ActiveLocalReminder {
    QString id;
    QPointer<QObject> runtime;
    QPointer<QTimer> timer;
    QDateTime due;
    QString title;
    QString modules;
};

QVector<ActiveLocalReminder> g_active_local_reminders;

void prune_active_local_reminders()
{
    const QDateTime now = QDateTime::currentDateTime();
    for (int i = g_active_local_reminders.size() - 1; i >= 0; --i) {
        const ActiveLocalReminder &reminder = g_active_local_reminders.at(i);
        if (reminder.timer.isNull() || reminder.due.addSecs(kReminderPastGraceSeconds) < now) {
            g_active_local_reminders.removeAt(i);
        }
    }
}

void unregister_active_local_reminder(const QString &id)
{
    for (int i = g_active_local_reminders.size() - 1; i >= 0; --i) {
        if (g_active_local_reminders.at(i).id == id) {
            g_active_local_reminders.removeAt(i);
        }
    }
}

void register_active_local_reminder(const QString &id,
                                    QObject *runtime,
                                    QTimer *timer,
                                    const QDateTime &due,
                                    const QString &title,
                                    const QString &modules)
{
    prune_active_local_reminders();
    g_active_local_reminders.append({id, runtime, timer, due, title, modules});
}

int cancel_active_local_reminders()
{
    prune_active_local_reminders();
    const int count = g_active_local_reminders.size();
    QSet<QObject *> runtimes;
    for (const ActiveLocalReminder &reminder : std::as_const(g_active_local_reminders)) {
        if (!reminder.timer.isNull()) {
            reminder.timer->stop();
            reminder.timer->deleteLater();
        }
        if (!reminder.runtime.isNull()) {
            runtimes.insert(reminder.runtime.data());
        }
    }
    for (QObject *runtime : std::as_const(runtimes)) {
        if (runtime != nullptr) {
            runtime->deleteLater();
        }
    }
    g_active_local_reminders.clear();
    return count;
}

QString active_local_reminders_text()
{
    prune_active_local_reminders();
    if (g_active_local_reminders.isEmpty()) {
        return get_text(QStringLiteral("Nenhum lembrete ativo."));
    }

    QVector<ActiveLocalReminder> reminders = g_active_local_reminders;
    std::sort(reminders.begin(), reminders.end(), [](const ActiveLocalReminder &left, const ActiveLocalReminder &right) {
        if (left.due == right.due) {
            return left.title.localeAwareCompare(right.title) < 0;
        }
        return left.due < right.due;
    });

    QStringList lines;
    lines << get_text(QStringLiteral("Lembretes ativos:"));
    for (const ActiveLocalReminder &reminder : std::as_const(reminders)) {
        lines << get_text(QStringLiteral("%1 - [%2] %3"))
                     .arg(reminder.due.toString(QStringLiteral("dd/MM/yyyy HH:mm")),
                          reminder.modules,
                          reminder.title);
    }
    return lines.join(QLatin1Char('\n'));
}

int reminder_candidate_count_for_window(const QVector<TaskRecordGroup> &groups,
                                        const QSet<QString> &modules,
                                        bool include_completed,
                                        bool use_default_time,
                                        const QTime &default_time,
                                        const QDate &start_date,
                                        const QDate &end_date,
                                        const QTime &start_time,
                                        const QTime &end_time)
{
    int count = 0;
    const QDateTime now = QDateTime::currentDateTime();
    for (const TaskRecordGroup &group : groups) {
        if (!group_has_selected_module(group, modules)) {
            continue;
        }
        if (!include_completed && !group_has_open_record(group)) {
            continue;
        }
        const std::optional<QDateTime> due = reminder_due_for_group(group, use_default_time, default_time);
        if (due.has_value()
            && reminder_due_in_window(due.value(), now, start_date, end_date, start_time, end_time)) {
            ++count;
        }
    }
    return count;
}

QString recurrence_duplicate_key(const QString &module,
                                 const QString &source_id,
                                 const QString &title,
                                 const QString &date_iso)
{
    return QStringLiteral("%1|%2|%3|%4")
        .arg(normalize_module_key(module), folded_identity_text(source_id), folded_identity_text(title), date_iso);
}

QString recurrence_user_label(const QString &recurrence)
{
    const QString normalized = normalize_recurrence_key(recurrence);
    if (normalized == QStringLiteral("daily")) {
        return get_text(QStringLiteral("Diária"));
    }
    if (normalized == QStringLiteral("weekly")) {
        return get_text(QStringLiteral("Semanal"));
    }
    if (normalized == QStringLiteral("monthly")) {
        return get_text(QStringLiteral("Mensal"));
    }
    return get_text(QStringLiteral("Sem recorrência"));
}

QDateTime recurrence_window_start(const QDate &date, const QTime &time)
{
    if (!date.isValid()) {
        return {};
    }
    return QDateTime(date, time.isValid() ? time : QTime(0, 0));
}

QDateTime recurrence_window_end(const QDate &date, const QTime &time)
{
    if (!date.isValid()) {
        return {};
    }
    return QDateTime(date, time.isValid() ? time : QTime(23, 59, 59));
}

bool recurrence_occurrence_in_window(const QDate &date,
                                     const QTime &time,
                                     const QDateTime &window_start,
                                     const QDateTime &window_end)
{
    if (!date.isValid() || !window_start.isValid() || !window_end.isValid() || window_end < window_start) {
        return false;
    }

    if (time.isValid()) {
        const QDateTime occurrence(date, time);
        return occurrence >= window_start && occurrence <= window_end;
    }

    return date >= window_start.date() && date <= window_end.date();
}

QString preview_recurrence_dates(const QDate &base_date,
                                 const QString &recurrence,
                                 const QDateTime &window_start,
                                 const QDateTime &window_end,
                                 const QTime &occurrence_time,
                                 int max_per_item)
{
    if (!base_date.isValid() || recurrence.trimmed().isEmpty()) {
        return get_text(QStringLiteral("Sem data base"));
    }
    if (!window_start.isValid() || !window_end.isValid() || window_end < window_start) {
        return get_text(QStringLiteral("Período inválido"));
    }

    QStringList dates;
    QDate next = next_recurrence_date(base_date, recurrence);
    int guard = 0;
    while (next.isValid() && next <= window_end.date() && dates.size() < qMin(max_per_item, 3) && guard++ < 120) {
        if (recurrence_occurrence_in_window(next, occurrence_time, window_start, window_end)) {
            dates << (occurrence_time.isValid()
                          ? QStringLiteral("%1 %2").arg(next.toString(QStringLiteral("dd/MM/yyyy")),
                                                        occurrence_time.toString(QStringLiteral("HH:mm")))
                          : next.toString(QStringLiteral("dd/MM/yyyy")));
        }
        next = next_recurrence_date(next, recurrence);
    }
    if (dates.isEmpty()) {
        return get_text(QStringLiteral("Nenhuma data no período"));
    }
    if (max_per_item > dates.size()) {
        return dates.join(QStringLiteral(", "));
    }
    return dates.join(QStringLiteral(", "));
}

QSet<QString> existing_recurrence_keys(const QVariantList &unified_records, const QJsonObject &mapamental_backup)
{
    QSet<QString> keys;
    const auto append_key = [&keys](const QVariantMap &record) {
        const QString module = record_module(record);
        const QString date = date_for_record(record).trimmed();
        const QString title = title_for_record(record);
        if (module.isEmpty() || date.isEmpty() || title.trimmed().isEmpty()) {
            return;
        }

        QString source_id = record.value(QStringLiteral("recurrence_source_id")).toString().trimmed();
        if (source_id.isEmpty()) {
            source_id = record.value(QStringLiteral("integracao_task_id")).toString().trimmed();
        }
        if (source_id.isEmpty()) {
            source_id = record.value(QStringLiteral("id")).toString().trimmed();
        }
        if (!source_id.isEmpty()) {
            keys.insert(recurrence_duplicate_key(module, source_id, title, date));
        }
    };

    for (const QVariant &value : unified_records) {
        append_key(value.toMap());
    }
    for (const QVariant &value : mapamental_records_from_backup(mapamental_backup)) {
        append_key(value.toMap());
    }
    return keys;
}

QVariantMap recurring_record_copy(const QVariantMap &record,
                                  const QString &module,
                                  const QDate &date,
                                  const QString &occurrence_id,
                                  const QString &source_id,
                                  const QString &time_override = QString())
{
    QVariantMap copy = record;
    const QString normalized_module = normalize_module_key(module);
    const int priority = normalized_priority_value(copy.value(QStringLiteral("prioridade"),
                                                              copy.value(QStringLiteral("quadrante"))),
                                                   2);
    const QString occurrence_time =
        time_override.trimmed().isEmpty() ? time_for_record(record) : time_override.trimmed();

    copy.insert(QStringLiteral("id"), occurrence_id);
    copy.insert(QStringLiteral("modulo"), normalized_module);
    copy.insert(QStringLiteral("data"), date.toString(Qt::ISODate));
    copy.insert(QStringLiteral("hora"), occurrence_time);
    copy.insert(QStringLiteral("status"), QStringLiteral("Todo"));
    copy.insert(QStringLiteral("concluida"), false);
    copy.insert(QStringLiteral("completed_at"), QString());
    copy.insert(QStringLiteral("recurrence"), QString());
    copy.insert(QStringLiteral("recurrence_until"), QString());
    copy.insert(QStringLiteral("recurrence_source_id"), source_id);
    copy.insert(QStringLiteral("integracao_origem"), QStringLiteral("recurrence"));
    copy.insert(QStringLiteral("integracao_task_id"), occurrence_id);
    copy.insert(QStringLiteral("integracao_status"), QStringLiteral("Todo"));
    copy.insert(QStringLiteral("integracao_date"), date.toString(Qt::ISODate));
    copy.insert(QStringLiteral("integracao_time"), occurrence_time);
    copy.insert(QStringLiteral("integracao_completed"), false);
    copy.insert(QStringLiteral("data_criacao"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

    if (normalized_module == QStringLiteral("feynman")) {
        copy.insert(QStringLiteral("tipo"), QStringLiteral("conceito"));
        copy.insert(QStringLiteral("categoria"), QStringLiteral("conceito"));
        if (copy.value(QStringLiteral("descricao")).toString().trimmed().isEmpty()) {
            copy.insert(QStringLiteral("descricao"), record_description(record));
        }
        copy.insert(QStringLiteral("descricao_customizada"), true);
    } else if (normalized_module == QStringLiteral("pomodoro")) {
        copy.insert(QStringLiteral("tipo"), QStringLiteral("tarefa"));
        copy.insert(QStringLiteral("categoria"), QStringLiteral("kanban"));
    } else if (normalized_module == QStringLiteral("mapamental")) {
        copy.insert(QStringLiteral("tipo"), QStringLiteral("conceito"));
        copy.insert(QStringLiteral("categoria"), QStringLiteral("mapa"));
    } else {
        copy.insert(QStringLiteral("tipo"), QStringLiteral("tarefa"));
        copy.insert(QStringLiteral("quadrante"), priority);
        copy.insert(QStringLiteral("categoria"), QStringLiteral("quadrant%1").arg(qBound(1, priority, 4)));
    }

    return source::utils::UnifiedTaskStore::normalize_record(copy);
}

QJsonObject recurring_map_node_for_record(const QVariantMap &record,
                                          const QDate &date,
                                          const QString &occurrence_id,
                                          const QString &source_id,
                                          const QString &source_modules,
                                          const QString &recurrence,
                                          int position_index,
                                          const QString &time_override = QString())
{
    QJsonObject node;
    node.insert(QStringLiteral("texto"), title_for_record(record));
    node.insert(QStringLiteral("x"), 80 + position_index * 24);
    node.insert(QStringLiteral("y"), 80 + position_index * 16);
    node.insert(QStringLiteral("cor"), QStringLiteral("#ff0078d4"));

    QStringList notes;
    const QString description = record_description(record).trimmed();
    if (!description.isEmpty()) {
        notes << description;
    }
    notes << get_text(QStringLiteral("Data: %1")).arg(date.toString(QStringLiteral("dd/MM/yyyy")));
    const QString time =
        time_override.trimmed().isEmpty() ? time_for_record(record).trimmed() : time_override.trimmed();
    if (!time.isEmpty()) {
        notes << get_text(QStringLiteral("Horário: %1")).arg(time);
    }
    notes << get_text(QStringLiteral("Recorrência de: %1")).arg(source_modules);
    node.insert(QStringLiteral("notas"), notes.join(QLatin1Char('\n')));

    QJsonArray info;
    info.append(QStringLiteral("node_id=%1").arg(occurrence_id));
    info.append(QStringLiteral("integracao_task_id=%1").arg(occurrence_id));
    info.append(QStringLiteral("recurrence_source_id=%1").arg(source_id));
    info.append(QStringLiteral("recurrence=%1").arg(recurrence));
    info.append(QStringLiteral("data=%1").arg(date.toString(Qt::ISODate)));
    if (!time.isEmpty()) {
        info.append(QStringLiteral("hora=%1").arg(time));
    }
    info.append(QStringLiteral("dominio=%1").arg(normalized_domain_value(record.value(QStringLiteral("dominio")), 0)));
    info.append(QStringLiteral("prioridade=%1")
                    .arg(normalized_priority_value(record.value(QStringLiteral("prioridade"),
                                                                 record.value(QStringLiteral("quadrante"))),
                                                   2)));
    const QString file_path = record_file_path(record);
    if (!file_path.isEmpty()) {
        info.append(QStringLiteral("arquivo_vinculado=%1").arg(file_path));
    }
    node.insert(QStringLiteral("info_associada"), info);
    return node;
}

QIcon local_reminder_icon(source::EisenhowerMatrixApp *app)
{
    QIcon icon;
    if (app != nullptr) {
        icon = app->windowIcon();
    }
    if (icon.isNull()) {
        icon = QApplication::windowIcon();
    }
    if (icon.isNull() && QApplication::style() != nullptr) {
        icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
    }
    return icon;
}

void show_in_app_reminder(source::EisenhowerMatrixApp *app, const QString &message, QWidget *parent = nullptr)
{
    QWidget *owner = parent != nullptr ? parent : static_cast<QWidget *>(app);
    auto *box = new QMessageBox(QMessageBox::Information,
                                get_text(QStringLiteral("Lembrete de tarefa")),
                                message,
                                QMessageBox::Ok,
                                owner);
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->setWindowIcon(local_reminder_icon(app));
    const bool child_of_active_dialog = owner != nullptr && owner != static_cast<QWidget *>(app);
    box->setModal(child_of_active_dialog);
    box->setWindowModality(child_of_active_dialog ? Qt::WindowModal : Qt::NonModal);
    box->setWindowFlag(Qt::WindowStaysOnTopHint, true);
    translate_standard_buttons(app, box);
    QApplication::beep();
    if (app != nullptr) {
        QApplication::alert(app, 0);
    }
    box->show();
    box->raise();
    box->activateWindow();
    QTimer::singleShot(30000, box, &QMessageBox::accept);
}

void deliver_local_reminder(source::EisenhowerMatrixApp *app,
                            QSystemTrayIcon *tray,
                            const QString &title,
                            const QString &message,
                            bool show_tray_message,
                            bool show_in_app_message,
                            QWidget *in_app_parent = nullptr)
{
    if (show_tray_message && tray != nullptr && tray->isVisible() && QSystemTrayIcon::supportsMessages()) {
        tray->showMessage(title, message, QSystemTrayIcon::Information, 15000);
    }

    if (show_in_app_message || tray == nullptr || !QSystemTrayIcon::supportsMessages()) {
        show_in_app_reminder(app, QStringLiteral("%1\n\n%2").arg(title, message), in_app_parent);
    }
}

}  // namespace

}  // namespace source::InterfaceCore::ProductivityFeatures
