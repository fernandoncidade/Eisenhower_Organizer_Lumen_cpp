#include "source/ui/CalendarDialog/caldial_04_update_task_list.hpp"

#include <QBrush>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QDate>
#include <QFont>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLocale>
#include <QStringList>
#include <algorithm>

#include "source/ui/CalendarDialog/agregador_01_CalendarDialog.hpp"
#include "source/ui/agregador_01_ui.hpp"
#include "source/services/incore_13_prioridade_display.hpp"
#include "source/ui/ui_10_BadgeCalendarWidget.hpp"
#include "source/ui/ui_12_CalendarDialog.hpp"
#include "source/eisenhower_app.hpp"
#include "source/utils/LinkedTitleUnderlineDelegate.hpp"

namespace source::GerenciamentoUI::CalendarDialogModules {
namespace {

void apply_task_title_appearance(QListWidgetItem *item, const QVariantMap &task)
{
    if (item == nullptr) {
        return;
    }

    const QVariantMap font_config = task.value(QStringLiteral("font_config")).toMap();
    if (!font_config.isEmpty() && item->data(source::utils::kCustomFontSettingsRole).toMap().isEmpty()) {
        item->setData(source::utils::kCustomFontSettingsRole, font_config);
    }

    const bool has_file = !task.value(QStringLiteral("file_path")).toString().trimmed().isEmpty();
    source::utils::apply_task_title_font(item, task.value(QStringLiteral("text")).toString(), has_file);

    const QColor text_color(task.value(QStringLiteral("cor_texto")).toString().trimmed());
    if (text_color.isValid()) {
        item->setForeground(QBrush(text_color));
    } else {
        item->setForeground(QBrush());
    }
}

} // namespace

void update_task_list(source::GerenciamentoUI::CalendarDialog *self)
{
    try {
        if (self == nullptr || self->tasks_list == nullptr) {
            return;
        }

        self->tasks_list->clear();
        self->_apply_highlighted_dates();
        self->_update_badge_counts();

        const QDate selected_date = self->calendar != nullptr ? self->calendar->selectedDate() : QDate::currentDate();
        QStringList selected_dates;
        if (selected_date.isValid()) {
            selected_dates << selected_date.toString(Qt::ISODate);
        }
        const QSet<QString> selected_dates_set(selected_dates.begin(), selected_dates.end());

        const QString filter_mode =
            self->filter_combo != nullptr ? self->filter_combo->currentData().toString() : QStringLiteral("day");
        const QList<QVariantMap> tasks_all = self->_collect_tasks();
        const bool show_priority =
            self->show_priority_checkbox == nullptr || self->show_priority_checkbox->isChecked();
        const bool show_domain =
            self->show_domain_checkbox == nullptr || self->show_domain_checkbox->isChecked();

        QMap<QString, QStringList> tooltip_map;
        QMap<QString, QList<QVariantMap>> tasks_by_date_data;

        for (const QVariantMap &task : tasks_all) {
            const QString ds = task.value(QStringLiteral("date")).toString();
            if (ds.isEmpty()) {
                continue;
            }

            const QString text = task.value(QStringLiteral("text")).toString().trimmed();
            if (text.isEmpty()) {
                continue;
            }

            const QString time_str = task.value(QStringLiteral("time")).toString();
            const QString emoji =
                show_priority ? source::GerenciamentoUI::task_utils::priority_emoji(task.value(QStringLiteral("priority")),
                                                                                    self->app)
                              : QString();
            const QString domain_badge =
                show_domain ? source::InterfaceCore::dominio_insignia(task.value(QStringLiteral("dominio"))) : QString();
            const QString display_title = text;
            const QString decorated_text =
                domain_badge.isEmpty() ? display_title : QStringLiteral("%1 %2").arg(domain_badge, display_title);
            const QString status_text =
                source::GerenciamentoUI::task_utils::task_status_text(task);

            const QString line = time_str.isEmpty()
                                     ? QStringLiteral("%1 %2 (%3)").arg(emoji, decorated_text, status_text).trimmed()
                                     : QStringLiteral("%1 %2 — %3 (%4)")
                                           .arg(emoji, time_str, decorated_text, status_text)
                                           .trimmed();

            tooltip_map[ds].append(line);
            tasks_by_date_data[ds].append(task);
        }

        for (auto it = tooltip_map.begin(); it != tooltip_map.end(); ++it) {
            std::sort(it.value().begin(), it.value().end());
        }

        if (self->calendar != nullptr) {
            self->calendar->set_task_tooltip_map(tooltip_map);
            self->calendar->set_tasks_by_date_data(tasks_by_date_data);
        }

        QList<QPair<QDate, QVariantMap>> filtered;
        for (const QVariantMap &task : tasks_all) {
            const QString date_str = task.value(QStringLiteral("date")).toString();
            if (date_str.isEmpty()) {
                continue;
            }

            const QDate qdate = QDate::fromString(date_str, Qt::ISODate);
            if (!qdate.isValid()) {
                continue;
            }

            if (filter_mode == QStringLiteral("day")) {
                if (!selected_dates_set.isEmpty()) {
                    if (!selected_dates_set.contains(date_str)) {
                        continue;
                    }
                } else if (qdate != selected_date) {
                    continue;
                }
            }

            if (filter_mode == QStringLiteral("week")) {
                const QDate selected_start = selected_date.addDays(-(selected_date.dayOfWeek() % 7));
                const QDate task_start = qdate.addDays(-(qdate.dayOfWeek() % 7));
                if (selected_start != task_start) {
                    continue;
                }
            }

            if (filter_mode == QStringLiteral("month")
                && (qdate.month() != selected_date.month() || qdate.year() != selected_date.year())) {
                continue;
            }

            if (filter_mode == QStringLiteral("year") && qdate.year() != selected_date.year()) {
                continue;
            }

            filtered.append({qdate, task});
        }

        if (filtered.isEmpty()) {
            auto *placeholder =
                new QListWidgetItem(detail::get_text(QStringLiteral("Nenhuma tarefa para o período selecionado.")));
            placeholder->setFlags((placeholder->flags() & ~Qt::ItemIsSelectable) & ~Qt::ItemIsEnabled);
            self->tasks_list->addItem(placeholder);
            return;
        }

        std::sort(filtered.begin(), filtered.end(), [](const auto &left, const auto &right) {
            if (left.first != right.first) {
                return left.first < right.first;
            }

            const QString left_time = left.second.value(QStringLiteral("time")).toString();
            const QString right_time = right.second.value(QStringLiteral("time")).toString();
            if (left_time != right_time) {
                return left_time < right_time;
            }

            return left.second.value(QStringLiteral("text")).toString().toLower()
                   < right.second.value(QStringLiteral("text")).toString().toLower();
        });

        const bool should_insert_month_headers =
            filter_mode == QStringLiteral("month") || filter_mode == QStringLiteral("year");
        QPair<int, int> last_month_key(-1, -1);
        QString last_group_key;
        const QLocale list_locale = self->calendar != nullptr ? self->calendar->locale() : QLocale::system();

        for (const auto &entry : filtered) {
            const QDate qdate = entry.first;
            const QVariantMap task = entry.second;

            if (should_insert_month_headers) {
                const QPair<int, int> month_key(qdate.year(), qdate.month());
                if (month_key != last_month_key) {
                    QString month_name = list_locale.standaloneMonthName(qdate.month(), QLocale::LongFormat);
                    if (month_name.isEmpty()) {
                        month_name = list_locale.monthName(qdate.month(), QLocale::LongFormat);
                    }
                    if (month_name.isEmpty()) {
                        month_name = qdate.toString(QStringLiteral("MMMM"));
                    }

                    auto *header_item = new QListWidgetItem(
                        QStringLiteral("%1 %2")
                            .arg(month_name.left(1).toUpper() + month_name.mid(1))
                            .arg(qdate.year()));
                    header_item->setFlags((header_item->flags() & ~Qt::ItemIsSelectable) & ~Qt::ItemIsEnabled);
                    QFont header_font = header_item->font();
                    header_font.setBold(true);
                    header_item->setFont(header_font);
                    self->tasks_list->addItem(header_item);
                    last_month_key = month_key;
                    last_group_key.clear();
                }
            }

            const QString status_text =
                source::GerenciamentoUI::task_utils::task_status_text(task);
            const QString date_text = qdate.toString(self->_date_format);
            const QString time_str = task.value(QStringLiteral("time")).toString();
            const QString emoji =
                show_priority ? source::GerenciamentoUI::task_utils::priority_emoji(task.value(QStringLiteral("priority")),
                                                                                    self->app)
                              : QString();
            const QString domain_badge =
                show_domain ? source::InterfaceCore::dominio_insignia(task.value(QStringLiteral("dominio"))) : QString();
            const QString plain_title = task.value(QStringLiteral("text")).toString();
            const QString display_title = plain_title;
            const QString decorated_text = domain_badge.isEmpty()
                                               ? display_title
                                               : QStringLiteral("%1 %2").arg(domain_badge, display_title);
            const QString group_key =
                (self->app != nullptr && self->app->reserved_watermark_eisenhower_enabled() && !time_str.trimmed().isEmpty())
                    ? QStringLiteral("%1|%2").arg(task.value(QStringLiteral("date")).toString().trimmed(), time_str.trimmed())
                    : QString();
            if (!group_key.isEmpty() && group_key != last_group_key) {
                QStringList parts;
                if (!date_text.trimmed().isEmpty()) {
                    parts << QStringLiteral("%1: %2")
                                 .arg(detail::get_text(QStringLiteral("Data")), date_text.trimmed());
                }
                if (!time_str.trimmed().isEmpty()) {
                    parts << QStringLiteral("%1: %2")
                                 .arg(detail::get_text(QStringLiteral("Horário")), time_str.trimmed());
                }
                auto *header_item = new QListWidgetItem(parts.join(QStringLiteral("  |  ")));
                header_item->setFlags((header_item->flags() & ~Qt::ItemIsSelectable) & ~Qt::ItemIsEnabled);
                header_item->setData(Qt::UserRole + 1, QStringLiteral("group_header"));
                header_item->setData(Qt::UserRole + 2, group_key);
                self->tasks_list->addItem(header_item);
                last_group_key = group_key;
            } else if (group_key.isEmpty()) {
                last_group_key.clear();
            }

            QString display_text =
                QStringLiteral("%1 %2 — [%3]").arg(emoji, decorated_text, status_text).trimmed();
            if (self->app != nullptr && self->app->reserved_space_eisenhower_enabled() && !date_text.trimmed().isEmpty()) {
                const QString dt_str = time_str.isEmpty() ? date_text : QStringLiteral("%1 %2").arg(date_text, time_str);
                display_text = QStringLiteral("%1 %2 — %3 — [%4]").arg(emoji, dt_str, decorated_text, status_text).trimmed();
            }

            auto *item = new QListWidgetItem(display_text);
            item->setFlags(item->flags() | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            item->setData(Qt::UserRole, task);

            apply_task_title_appearance(item, task);

            item->setToolTip(detail::tooltip_for_task(self, task));
            self->tasks_list->addItem(item);
        }
    } catch (...) {
        detail::logger()->error(QStringLiteral("Erro ao atualizar lista de tarefas do calendário."));
    }
}

}  // namespace source::GerenciamentoUI::CalendarDialogModules
