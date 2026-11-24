#include "source/services/incore_11_atualizar_itens_tarefas.hpp"

#include <QBrush>
#include <QColor>
#include <QCoreApplication>
#include <QDate>
#include <QDateEdit>
#include <QFont>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLocale>
#include <QRegularExpression>
#include <QVariantMap>

#include "source/services/incore_13_prioridade_display.hpp"
#include "source/eisenhower_app.hpp"
#include "source/app/app_02_get_text.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"
#include "source/utils/LinkedTitleUnderlineDelegate.hpp"
#include "source/utils/TaskProgressHelper.hpp"
#include "source/modules/gantt/gantt_plugin_interface.hpp"

namespace source::InterfaceCore::incore_11_atualizar_itens_tarefas {

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

QLocale effective_locale(source::EisenhowerMatrixApp *app)
{
    if (app != nullptr && app->date_input != nullptr) {
        return app->date_input->locale();
    }

    if (app != nullptr && app->gerenciador_traducao != nullptr) {
        const QString idioma = app->gerenciador_traducao->obter_idioma_atual();
        if (idioma.startsWith(QStringLiteral("pt"))) {
            return QLocale(QLocale::Portuguese, QLocale::Brazil);
        }
        if (idioma.startsWith(QStringLiteral("en"))) {
            return QLocale(QLocale::English, QLocale::UnitedStates);
        }
    }

    return QLocale::system();
}

QString base_text_from_item(QListWidgetItem *item)
{
    if (item == nullptr) {
        return QString();
    }

    const QString text = item->text().split(QLatin1Char('\n')).value(0).trimmed();
    const int sep = text.indexOf(QStringLiteral(" — "));
    QString base = sep >= 0 ? text.left(sep).trimmed() : text;
    base.remove(QRegularExpression(QStringLiteral("^[🥇🥈🥉\\s]+")));
    base.remove(QChar(0x0332));
    return base.trimmed();
}

QString title_with_domain_badge(const QString &base_text, const QVariant &domain)
{
    const QString clean_text = base_text.trimmed();
    if (clean_text.isEmpty() || !domain.isValid() || domain.toString().trimmed().isEmpty()) {
        return clean_text;
    }

    const QString domain_badge = source::InterfaceCore::dominio_insignia(domain);
    return domain_badge.isEmpty() ? clean_text : QStringLiteral("%1 %2").arg(domain_badge, clean_text);
}

QString visible_task_text(source::EisenhowerMatrixApp *app,
                          const QString &base_text,
                          const QString &date_human,
                          const QString &time_str)
{
    QString display_text = base_text.trimmed();
    if (app == nullptr || !app->reserved_space_eisenhower_enabled() || date_human.trimmed().isEmpty()) {
        return display_text;
    }

    const QString cleaned_time = time_str.trimmed();
    return cleaned_time.isEmpty() ? QStringLiteral("%1 — %2").arg(display_text, date_human)
                                  : QStringLiteral("%1 — %2 %3").arg(display_text, date_human, cleaned_time);
}

QString compose_task_display_text(source::EisenhowerMatrixApp *app,
                                  const QString &base_text,
                                  const QString &date_human,
                                  const QString &time_str)
{
    return visible_task_text(app, base_text, date_human, time_str);
}

void atualizar_itens_tarefas(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }

    const QLocale locale = effective_locale(app);
    QString date_format = locale.dateFormat(QLocale::ShortFormat);
    if (app->date_input != nullptr) {
        date_format = app->date_input->displayFormat();
    }

    const int group_header_role = Qt::UserRole + 1;
    const QList<QListWidget *> lists = {
        app->quadrant1_list,
        app->quadrant2_list,
        app->quadrant3_list,
        app->quadrant4_list,
        app->quadrant1_in_progress_list,
        app->quadrant2_in_progress_list,
        app->quadrant3_in_progress_list,
        app->quadrant4_in_progress_list,
        app->quadrant1_completed_list,
        app->quadrant2_completed_list,
        app->quadrant3_completed_list,
        app->quadrant4_completed_list,
    };

    for (QListWidget *list : lists) {
        if (list == nullptr) {
            continue;
        }

        struct TaskEntry {
            QVariantMap data;
            QString base_text;
            Qt::CheckState check_state = Qt::Unchecked;
        };

        QList<TaskEntry> tasks;
        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem *item = list->item(i);
            if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable)) {
                continue;
            }
            if (item->data(group_header_role).toString() == QStringLiteral("group_header")) {
                continue;
            }

            QVariantMap data = item->data(Qt::UserRole).toMap();
            const QString base_text = data.value(QStringLiteral("text")).toString().trimmed().isEmpty()
                                          ? base_text_from_item(item)
                                          : data.value(QStringLiteral("text")).toString().trimmed();
            if (base_text.isEmpty()) {
                continue;
            }

            tasks.append({data, base_text, item->checkState()});
        }

        if (tasks.isEmpty()) {
            continue;
        }

        list->blockSignals(true);
        list->clear();

        for (const TaskEntry &entry : tasks) {
            QVariantMap data = entry.data;
            const QString base_text = entry.base_text;
            const QString date_iso = data.value(QStringLiteral("date")).toString();
            const QString time_str = data.value(QStringLiteral("time")).toString();
            const QVariant domain = data.value(QStringLiteral("dominio"));
            const QString decorated_base_text = title_with_domain_badge(base_text, domain);

            QString human;
            QString display_text = decorated_base_text;
            QStringList tooltip_lines;
            if (!date_iso.isEmpty()) {
                const QDate qdate = QDate::fromString(date_iso, Qt::ISODate);
                human = qdate.isValid() ? locale.toString(qdate, date_format) : date_iso.trimmed();
                display_text = compose_task_display_text(app, decorated_base_text, human, time_str);
                if (!human.isEmpty()) {
                    tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Data")), human);
                }
            } else {
                display_text = compose_task_display_text(app, decorated_base_text, QString(), time_str);
            }

            if (!time_str.isEmpty()) {
                tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Horário")), time_str);
            }

            auto *new_item = new QListWidgetItem(display_text);
            new_item->setFlags(new_item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            new_item->setCheckState(entry.check_state);

            data.insert(QStringLiteral("text"), base_text);
            new_item->setData(Qt::UserRole, data);
            const QVariantMap font_config = data.value(QStringLiteral("font_config")).toMap();
            if (!font_config.isEmpty()) {
                new_item->setData(source::utils::kCustomFontSettingsRole, font_config);
            }

            const QString file_path = data.value(QStringLiteral("file_path")).toString();
            source::utils::apply_task_title_font(new_item, base_text, !file_path.isEmpty());
            if (!file_path.isEmpty()) {
                const QColor text_color(data.value(QStringLiteral("cor_texto")).toString().trimmed());
                if (text_color.isValid()) {
                    new_item->setForeground(QBrush(text_color));
                } else {
                    new_item->setForeground(QBrush());
                }
                tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Arquivo")), file_path);
            } else {
                const QColor text_color(data.value(QStringLiteral("cor_texto")).toString().trimmed());
                if (text_color.isValid()) {
                    new_item->setForeground(QBrush(text_color));
                }
            }

            const QString description = data.value(QStringLiteral("description")).toString();
            if (!description.isEmpty()) {
                tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Descrição")), description);
            }

            const QVariant prio = data.value(QStringLiteral("priority"));
            if (prio.isValid() && !prio.toString().isEmpty()) {
                tooltip_lines << QStringLiteral("%1: %2")
                                     .arg(get_text(QStringLiteral("Prioridade")),
                                          source::InterfaceCore::prioridade_para_texto(prio, app));
            }

            if (domain.isValid() && !domain.toString().trimmed().isEmpty()) {
                tooltip_lines << QStringLiteral("%1: %2")
                                     .arg(get_text(QStringLiteral("Nível de Domínio")),
                                          source::InterfaceCore::dominio_para_texto(domain, false));
            }

            double duration = 1.0;
            if (data.contains(QStringLiteral("duracao")) && data.value(QStringLiteral("duracao")).toDouble() > 0.0) {
                duration = data.value(QStringLiteral("duracao")).toDouble();
            } else if (data.contains(QStringLiteral("duration")) && data.value(QStringLiteral("duration")).toDouble() > 0.0) {
                duration = data.value(QStringLiteral("duration")).toDouble();
            }
            tooltip_lines << source::app::format_duration_tooltip_line(duration);

            const int pom = data.value(QStringLiteral("pomodoros_completados"), data.value(QStringLiteral("pomodoros"), 0)).toInt();
            const int def_c = app != nullptr ? app->default_pomodoro_cycle_count() : 4;
            const int c_tot = data.value(QStringLiteral("ciclos_totais"), data.value(QStringLiteral("ciclos_permitidos"), data.value(QStringLiteral("total_ciclos"), def_c))).toInt();
            const int safe_c_tot = c_tot > 0 ? c_tot : def_c;
            const bool is_done = data.value(QStringLiteral("concluida")).toBool() || data.value(QStringLiteral("status")).toString() == QStringLiteral("Done") || entry.check_state == Qt::Checked;
            if (pom > 0 || safe_c_tot > 0) {
                const double pom_pct = source::utils::calculate_task_completion_percentage(pom, safe_c_tot, is_done);
                const QString pom_line_fmt = get_text(QStringLiteral("Ciclos Pomodoro: %1/%2 (%3%)"));
                tooltip_lines << pom_line_fmt
                                     .arg(QString::number(pom),
                                          QString::number(safe_c_tot),
                                          QString::number(pom_pct, 'f', 0));
            }

            const bool is_in_prog = (app != nullptr && app->is_in_progress_list(list))
                || data.value(QStringLiteral("em_progresso")).toBool()
                || data.value(QStringLiteral("in_progress")).toBool()
                || data.value(QStringLiteral("em_andamento")).toBool()
                || data.value(QStringLiteral("status")).toString().compare(QStringLiteral("Doing"), Qt::CaseInsensitive) == 0;



            if (!tooltip_lines.isEmpty()) {
                new_item->setToolTip(tooltip_lines.join(QLatin1Char('\n')));
            }

            app->insert_task_into_quadrant_list(list, new_item);
        }

        app->cleanup_time_groups(list);
        list->blockSignals(false);
    }
}

}  // namespace source::InterfaceCore::incore_11_atualizar_itens_tarefas
