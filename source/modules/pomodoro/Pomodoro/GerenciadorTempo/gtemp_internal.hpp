#pragma once

#include <algorithm>
#include <tuple>

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QApplication>
#include <QBrush>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDate>
#include <QEvent>
#include <QFileInfo>
#include <QFont>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLocale>
#include <QMap>
#include <QPalette>
#include <QResizeEvent>
#include <QScopedValueRollback>
#include <QScreen>
#include <QSet>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSplitter>
#include <QSplitterHandle>
#include <QStringList>
#include <QTabBar>
#include <QTabWidget>
#include <QThread>
#include <QTimer>
#include <QUuid>
#include <QVBoxLayout>
#include <QVariantMap>

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_25_prioridade_display.hpp"
#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_27_RotatedTabButton.hpp"
#include "source/modules/pomodoro/Pomodoro/tmp_01_Tarefa.hpp"
#include "source/modules/pomodoro/Pomodoro/tmp_02_PomodoroTimer.hpp"
#include "source/modules/pomodoro/Pomodoro/tmp_03_GerenciadorTarefas.hpp"
#include "source/modules/pomodoro/Pomodoro/tmp_05_GerenciadorTempo.hpp"
#include "source/modules/pomodoro/Pomodoro/tmp_06_TimeWorker.hpp"
#include "source/utils/LinkedTitleUnderlineDelegate.hpp"

namespace source::plugin::Pomodoro::GerenciadorTempoInternal {

constexpr int kTempoSplitterExpandedHandleWidth = 6;
constexpr int kTempoSplitterCollapsedHandleWidth = 0;
constexpr int kTempoDefaultRightPanelWidth = 360;

inline source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

inline QString normalized_ui_layout_key(const QString &value)
{
    return layout::normalized_ui_layout_key(value);
}

inline bool is_robust_ui_layout(const QString &value)
{
    return layout::is_robust_ui_layout(value);
}

inline QString localized_text(const QWidget *context, const QString &pt_br, const QString &en_us)
{
    return compat::effective_language_code(context).startsWith(QStringLiteral("en"), Qt::CaseInsensitive) ? en_us : pt_br;
}

inline QString expand_in_progress_panel_text(const QWidget *context)
{
    return localized_text(context,
                          QStringLiteral("Expandir Painel Em Progresso"),
                          QStringLiteral("Expand In Progress Panel"));
}

inline QString collapse_in_progress_panel_text(const QWidget *context)
{
    return localized_text(context,
                          QStringLiteral("Recolher Painel Em Progresso"),
                          QStringLiteral("Collapse In Progress Panel"));
}

inline int rotated_button_width(const source::GerenciamentoUI::RotatedTabButton *button)
{
    return button != nullptr ? qMax(button->minimumWidth(), button->sizeHint().width()) : 0;
}

inline void delete_layout_item(QLayoutItem *item)
{
    if (item == nullptr) {
        return;
    }

    if (QWidget *widget = item->widget()) {
        delete widget;
    } else if (QLayout *layout = item->layout()) {
        while (QLayoutItem *child = layout->takeAt(0)) {
            delete_layout_item(child);
        }
        delete layout;
        return;
    }

    delete item;
}

inline void clear_layout(QLayout *layout)
{
    if (layout == nullptr) {
        return;
    }

    while (QLayoutItem *item = layout->takeAt(0)) {
        delete_layout_item(item);
    }
}

inline QSize available_window_size(const QWidget *widget)
{
    const QWidget *top_level = widget != nullptr && widget->window() != nullptr ? widget->window() : widget;
    QScreen *screen = top_level != nullptr ? top_level->screen() : nullptr;
    if (screen == nullptr && widget != nullptr) {
        screen = widget->screen();
    }
    if (screen == nullptr) {
        screen = QApplication::primaryScreen();
    }

    return screen != nullptr ? screen->availableGeometry().size() : QSize(1380, 840);
}

inline int bounded_module_min_height(const QWidget *widget)
{
    const QSize available = available_window_size(widget);
    return qMin(520, qMax(360, available.height() - 260));
}

inline QDate parse_date(const QString &value)
{
    if (value.isEmpty()) {
        return {};
    }

    QDate date = QDate::fromString(value, Qt::ISODate);
    if (!date.isValid()) {
        date = QDate::fromString(value, QStringLiteral("yyyy-MM-dd"));
    }
    return date;
}

inline QPair<int, int> parse_time_pair(const QString &value)
{
    const QStringList parts = value.split(QLatin1Char(':'));
    if (parts.size() < 2) {
        return {99, 99};
    }
    return {parts.value(0).toInt(), parts.value(1).toInt()};
}

inline QString human_date_for_display(const QWidget *context, const QString &date_value)
{
    const QDate date = parse_date(date_value);
    return date.isValid() ? date.toString(compat::effective_date_format(context)) : date_value.trimmed();
}

inline QString visible_task_title(const QWidget *context,
                           const QString &base_text,
                           const QString &date_value,
                           const QString &time_value)
{
    QString display_text = base_text.trimmed();
    if (compat::host_bridge() == nullptr || !compat::host_bridge()->reserved_space_enabled()) {
        return display_text;
    }

    const QString human_date = human_date_for_display(context, date_value);
    if (human_date.isEmpty()) {
        return display_text;
    }

    const QString cleaned_time = time_value.trimmed();
    return cleaned_time.isEmpty() ? QStringLiteral("%1 — %2").arg(display_text, human_date)
                                  : QStringLiteral("%1 — %2 %3").arg(display_text, human_date, cleaned_time);
}

inline QString watermark_task_line(const QWidget *context, const QString &date_value, const QString &time_value)
{
    if (compat::host_bridge() == nullptr || !compat::host_bridge()->watermark_enabled()) {
        return {};
    }

    QStringList parts;
    const QString human_date = human_date_for_display(context, date_value);
    if (!human_date.trimmed().isEmpty()) {
        parts << QStringLiteral("%1: %2")
                     .arg(QCoreApplication::translate("PomodoroPlugin", "Data"), human_date.trimmed());
    }

    const QString cleaned_time = time_value.trimmed();
    if (!cleaned_time.isEmpty()) {
        parts << QStringLiteral("%1: %2")
                     .arg(QCoreApplication::translate("PomodoroPlugin", "Horário"), cleaned_time);
    }

    return parts.join(QStringLiteral("  |  "));
}

inline QString task_group_key(const QString &date_value, const QString &time_value)
{
    const QString cleaned_time = time_value.trimmed();
    if (cleaned_time.isEmpty()) {
        return {};
    }

    const QString cleaned_date = date_value.trimmed().isEmpty() ? QStringLiteral("9999-99-99") : date_value.trimmed();
    return QStringLiteral("%1|%2").arg(cleaned_date, cleaned_time);
}

inline bool is_group_header(QListWidgetItem *item)
{
    return item != nullptr && item->data(Qt::UserRole + 1).toString() == QStringLiteral("group_header");
}

inline int priority_rank(const QString &value)
{
    static const QMap<QString, int> rank = {
        {QStringLiteral("Importante e Urgente"), 1},
        {QStringLiteral("Importante, mas Não Urgente"), 2},
        {QStringLiteral("Não Importante, mas Urgente"), 3},
        {QStringLiteral("Não Importante e Não Urgente"), 4},
    };
    return rank.value(Tarefa::normalizar_prioridade(value), 1);
}

inline QString item_key(QListWidgetItem *item)
{
    if (item == nullptr || is_group_header(item) || !(item->flags() & Qt::ItemIsSelectable)) {
        return {};
    }

    const QString id = item->data(Qt::UserRole).toString();
    if (!id.isEmpty()) {
        return QStringLiteral("id:%1").arg(id);
    }

    return QStringLiteral("titulo:%1").arg(
        compat::strip_priority_emoji(item->text().split(QLatin1Char('\n')).value(0)).trimmed());
}

inline QVariantMap sort_payload_from_task(const Tarefa &task)
{
    return {
        {QStringLiteral("id"), task.id},
        {QStringLiteral("text"), task.titulo},
        {QStringLiteral("date"), task.date},
        {QStringLiteral("time"), task.time},
    };
}

inline bool has_host_task_sort()
{
    return compat::host_bridge() != nullptr && compat::host_bridge()->has_active_task_sort();
}

inline bool host_task_sort_less(const Tarefa *lhs, const Tarefa *rhs)
{
    if (lhs == nullptr || rhs == nullptr || compat::host_bridge() == nullptr) {
        return false;
    }

    return compat::host_bridge()->task_sort_less(sort_payload_from_task(*lhs), sort_payload_from_task(*rhs));
}

}  // namespace source::plugin::Pomodoro::GerenciadorTempoInternal
