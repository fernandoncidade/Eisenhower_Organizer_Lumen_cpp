#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_12_atualizar_coluna_doing_timer.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_internal.hpp"

#include <QColor>

namespace source::plugin::Pomodoro {
using namespace GerenciadorTempoInternal;

void GerenciadorTempo::atualizar_coluna_doing_timer()
{
    try {
        if (col_doing_timer_list == nullptr) {
            return;
        }

        col_doing_timer_list->clear();
        if (tarefas == nullptr) {
            return;
        }

        QVector<const Tarefa *> doing_tasks;
        for (const Tarefa &task : tarefas->tarefas) {
            if (task.status == QStringLiteral("Doing")) {
                doing_tasks.append(&task);
            }
        }

        const bool use_host_sort = has_host_task_sort();
        std::stable_sort(doing_tasks.begin(), doing_tasks.end(), [use_host_sort](const Tarefa *lhs, const Tarefa *rhs) {
            if (use_host_sort) {
                return host_task_sort_less(lhs, rhs);
            }

            const auto lhs_key = std::make_tuple(
                priority_rank(lhs->prioridade),
                parse_date(lhs->date).isValid() ? parse_date(lhs->date).toJulianDay() : 99999999,
                parse_time_pair(lhs->time),
                lhs->titulo.toLower());
            const auto rhs_key = std::make_tuple(
                priority_rank(rhs->prioridade),
                parse_date(rhs->date).isValid() ? parse_date(rhs->date).toJulianDay() : 99999999,
                parse_time_pair(rhs->time),
                rhs->titulo.toLower());
            return lhs_key < rhs_key;
        });

        const QMap<QString, QString> emoji = {
            {QStringLiteral("Importante e Urgente"), QStringLiteral("🔴")},
            {QStringLiteral("Importante, mas Não Urgente"), QStringLiteral("🟠")},
            {QStringLiteral("Não Importante, mas Urgente"), QStringLiteral("🟡")},
            {QStringLiteral("Não Importante e Não Urgente"), QStringLiteral("🟢")},
        };

        QString last_group_key;
        for (const Tarefa *task : doing_tasks) {
            const QString prio = Tarefa::normalizar_prioridade(task->prioridade);
            const QString domain = GerenciadorTarefasFns::dominio_insignia(task->dominio);
            const QString title_line = visible_task_title(
                this,
                QStringLiteral("%1 %2 %3").arg(emoji.value(prio, QStringLiteral("🔴")), domain, task->titulo),
                task->date,
                task->time);
            const QString group_key =
                (compat::host_bridge() != nullptr && compat::host_bridge()->watermark_enabled())
                    ? task_group_key(task->date, task->time)
                    : QString();
            if (!group_key.isEmpty() && group_key != last_group_key) {
                auto *header_item = new QListWidgetItem(watermark_task_line(this, task->date, task->time));
                header_item->setFlags((header_item->flags() & ~Qt::ItemIsSelectable) & ~Qt::ItemIsEnabled);
                header_item->setData(Qt::UserRole + 1, QStringLiteral("group_header"));
                header_item->setData(Qt::UserRole + 2, group_key);
                col_doing_timer_list->addItem(header_item);
                last_group_key = group_key;
            } else if (group_key.isEmpty()) {
                last_group_key.clear();
            }

            auto *item = new QListWidgetItem(
                QStringLiteral("%1\n%2")
                    .arg(title_line,
                         QCoreApplication::translate("PomodoroPlugin", "🍅 {n} pomodoros")
                             .replace(QStringLiteral("{n}"), QString::number(task->pomodoros_completados))));
            item->setData(Qt::UserRole, task->id);
            item->setData(source::utils::kCustomFontSettingsRole, task->font_config);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            source::utils::apply_task_title_font(item, task->titulo, !task->file_path.isEmpty(), source::utils::PluginFontContext::Pomodoro);

            QStringList tooltip;
            if (!task->date.isEmpty()) {
                QString human_date = task->date;
                const QDate parsed = QDate::fromString(task->date, Qt::ISODate);
                if (parsed.isValid()) {
                    human_date = parsed.toString(compat::effective_date_format(this));
                }
                tooltip << QStringLiteral("%1: %2").arg(QCoreApplication::translate("PomodoroPlugin", "Data"), human_date);
            }
            if (!task->time.isEmpty()) {
                tooltip << QStringLiteral("%1: %2").arg(QCoreApplication::translate("PomodoroPlugin", "Horario"), task->time);
            }
            if (!task->file_path.isEmpty()) {
                if (QColor color(task->cor_texto.trimmed()); color.isValid()) {
                    item->setForeground(QBrush(color));
                } else {
                    item->setForeground(QBrush());
                }
                tooltip << QStringLiteral("%1: %2").arg(QCoreApplication::translate("PomodoroPlugin", "Arquivo"), task->file_path);
            } else if (QColor color(task->cor_texto.trimmed()); color.isValid()) {
                item->setForeground(QBrush(color));
            }
            if (!task->descricao.isEmpty()) {
                tooltip << QStringLiteral("%1: %2").arg(QCoreApplication::translate("PomodoroPlugin", "Descricao"), task->descricao);
            }
            if (!task->prioridade.isEmpty()) {
                tooltip << QStringLiteral("%1: %2")
                               .arg(QCoreApplication::translate("PomodoroPlugin", "Prioridade"),
                                    GerenciadorTarefasFns::prioridade_para_texto(task->prioridade));
            }
            tooltip << QStringLiteral("%1: %2")
                           .arg(QCoreApplication::translate("PomodoroPlugin", "Nivel de Dominio"),
                                GerenciadorTarefasFns::dominio_para_texto(task->dominio));

            const double dur_days = task->duracao > 0.0 ? task->duracao : 1.0;
            const QString dur_text = (dur_days == 1.0)
                ? QCoreApplication::translate("PomodoroPlugin", "1 dia")
                : ((dur_days == static_cast<int>(dur_days))
                    ? QCoreApplication::translate("PomodoroPlugin", "%1 dias").arg(static_cast<int>(dur_days))
                    : QCoreApplication::translate("PomodoroPlugin", "%1 dias").arg(QString::number(dur_days, 'f', 1)));
            const QString dur_label = QCoreApplication::translate("PomodoroPlugin", "Duração:");
            tooltip << (dur_label.endsWith(QLatin1Char(':'))
                           ? QStringLiteral("%1 %2").arg(dur_label, dur_text)
                           : QStringLiteral("%1: %2").arg(dur_label, dur_text));

            const int pom = task->pomodoros_completados;
            const int c_tot = task->ciclos_totais > 0 ? task->ciclos_totais : 4;
            const bool is_done = false;
            double pom_pct = 0.0;
            if (pom > 0 || c_tot > 0) {
                pom_pct = (static_cast<double>(pom) / static_cast<double>(c_tot)) * 100.0;
                if (pom_pct >= 100.0) {
                    pom_pct = 99.0;
                } else if (pom_pct < 0.0) {
                    pom_pct = 0.0;
                }
                const QString pom_fmt = QCoreApplication::translate("PomodoroPlugin", "Ciclos Pomodoro: %1/%2 (%3%)");
                tooltip << pom_fmt
                               .arg(QString::number(pom),
                                    QString::number(c_tot),
                                    QString::number(pom_pct, 'f', 0));
            }



            if (!tooltip.isEmpty()) {
                item->setToolTip(tooltip.join(QLatin1Char('\n')));
            }

            col_doing_timer_list->addItem(item);
        }
        emit task_counter_changed();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao atualizar coluna de tarefas em progresso no timer: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao atualizar coluna de tarefas em progresso no timer"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTempoFns { void atualizar_coluna_doing_timer(GerenciadorTempo &self) { self.atualizar_coluna_doing_timer(); } }
