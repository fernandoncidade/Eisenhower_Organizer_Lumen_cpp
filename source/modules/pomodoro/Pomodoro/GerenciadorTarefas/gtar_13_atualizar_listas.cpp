#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_13_atualizar_listas.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

#include <QColor>

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

void GerenciadorTarefas::atualizar_listas()
{
    try {
        col_todo->lista->clear();
        col_doing->lista->clear();
        col_done->lista->clear();

        QMap<QString, QVector<const Tarefa *>> by_status = {
            {QStringLiteral("Todo"), {}},
            {QStringLiteral("Doing"), {}},
            {QStringLiteral("Done"), {}},
        };

        for (const Tarefa &task : tarefas) {
            if (task.status == QStringLiteral("Doing")) {
                by_status[QStringLiteral("Doing")].append(&task);
            } else if (task.status == QStringLiteral("Done")) {
                by_status[QStringLiteral("Done")].append(&task);
            } else {
                by_status[QStringLiteral("Todo")].append(&task);
            }
        }

        const auto default_sorter = [](const Tarefa *lhs, const Tarefa *rhs) {
            const auto lhs_key = std::make_tuple(
                priority_rank(lhs->prioridade),
                parse_task_date(lhs->date).isValid() ? parse_task_date(lhs->date) : QDate(9999, 12, 31),
                parse_task_time(lhs->time).isValid() ? parse_task_time(lhs->time) : QTime(23, 59, 59),
                lhs->titulo.toLower());
            const auto rhs_key = std::make_tuple(
                priority_rank(rhs->prioridade),
                parse_task_date(rhs->date).isValid() ? parse_task_date(rhs->date) : QDate(9999, 12, 31),
                parse_task_time(rhs->time).isValid() ? parse_task_time(rhs->time) : QTime(23, 59, 59),
                rhs->titulo.toLower());
            return lhs_key < rhs_key;
        };

        const bool use_host_sort = has_host_task_sort();
        for (QVector<const Tarefa *> &items : by_status) {
            std::stable_sort(items.begin(),
                             items.end(),
                             [use_host_sort, &default_sorter](const Tarefa *lhs, const Tarefa *rhs) {
                                 return use_host_sort ? host_task_sort_less(lhs, rhs) : default_sorter(lhs, rhs);
                             });
        }

        const QMap<QString, QString> emoji = {
            {QStringLiteral("Importante e Urgente"), QStringLiteral("🔴")},
            {QStringLiteral("Importante, mas Não Urgente"), QStringLiteral("🟠")},
            {QStringLiteral("Não Importante, mas Urgente"), QStringLiteral("🟡")},
            {QStringLiteral("Não Importante e Não Urgente"), QStringLiteral("🟢")},
        };

        for (const QString &status : {QStringLiteral("Todo"), QStringLiteral("Doing"), QStringLiteral("Done")}) {
            QString last_group_key;
            for (const Tarefa *task : by_status.value(status)) {
                const QString prio = Tarefa::normalizar_prioridade(task->prioridade);
                const QString domain = GerenciadorTarefasFns::dominio_insignia(task->dominio);
                const QString title_line = visible_task_title(
                    this,
                    QStringLiteral("%1 %2 %3").arg(emoji.value(prio, QStringLiteral("🔴")), domain, task->titulo),
                    task->date,
                    task->time);
                auto *item = new QListWidgetItem(QStringLiteral("%1\n%2")
                                                     .arg(title_line,
                                                          QCoreApplication::translate("PomodoroPlugin", "🍅 {n} pomodoros")
                                                              .replace(QStringLiteral("{n}"),
                                                                       QString::number(task->pomodoros_completados))));

                item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsDragEnabled);
                item->setCheckState(Qt::Unchecked);
                item->setData(kTaskIdRole, task->id);
                item->setData(source::utils::kCustomFontSettingsRole, task->font_config);
                item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                source::utils::apply_task_title_font(item, task->titulo, !task->file_path.isEmpty(), source::utils::PluginFontContext::Pomodoro);

                if (!task->file_path.isEmpty()) {
                    if (QColor color(task->cor_texto.trimmed()); color.isValid()) {
                        item->setForeground(QBrush(color));
                    } else {
                        item->setForeground(QBrush());
                    }
                } else if (QColor color(task->cor_texto.trimmed()); color.isValid()) {
                    item->setForeground(QBrush(color));
                }

                QStringList tooltip;
                if (!task->file_path.isEmpty()) {
                    tooltip << QStringLiteral("%1: %2")
                                   .arg(QCoreApplication::translate("PomodoroPlugin", "Arquivo"), task->file_path);
                }

                if (!task->date.isEmpty()) {
                    QString human_date = task->date;
                    const QDate parsed = QDate::fromString(task->date, Qt::ISODate);
                    if (parsed.isValid()) {
                        human_date = parsed.toString(compat::effective_date_format(this));
                    }
                    tooltip << QStringLiteral("%1: %2")
                                   .arg(QCoreApplication::translate("PomodoroPlugin", "Data"), human_date);
                }

                if (!task->time.isEmpty()) {
                    tooltip << QStringLiteral("%1: %2")
                                   .arg(QCoreApplication::translate("PomodoroPlugin", "Horário"), task->time);
                }

                if (!task->descricao.isEmpty()) {
                    tooltip << QStringLiteral("%1: %2")
                                   .arg(QCoreApplication::translate("PomodoroPlugin", "Descrição"), task->descricao);
                }

                if (!task->prioridade.isEmpty()) {
                    tooltip << QStringLiteral("%1: %2")
                                   .arg(QCoreApplication::translate("PomodoroPlugin", "Prioridade"),
                                        GerenciadorTarefasFns::prioridade_para_texto(task->prioridade));
                }

                tooltip << QStringLiteral("%1: %2")
                               .arg(QCoreApplication::translate("PomodoroPlugin", "Nível de Domínio"),
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
                const bool is_done = (status == QStringLiteral("Done"));
                const bool in_prog = (status == QStringLiteral("Doing"));
                double pom_pct = 0.0;
                if (pom > 0 || c_tot > 0) {
                    if (is_done) {
                        pom_pct = 100.0;
                    } else {
                        pom_pct = (static_cast<double>(pom) / static_cast<double>(c_tot)) * 100.0;
                        if (pom_pct >= 100.0) {
                            pom_pct = 99.0;
                        } else if (pom_pct < 0.0) {
                            pom_pct = 0.0;
                        }
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

                QListWidget *target_list = nullptr;
                if (status == QStringLiteral("Todo")) {
                    target_list = col_todo->lista;
                } else if (status == QStringLiteral("Doing")) {
                    target_list = col_doing->lista;
                } else {
                    target_list = col_done->lista;
                }

                if (target_list != nullptr) {
                    const QString group_key =
                        (compat::host_bridge() != nullptr && compat::host_bridge()->watermark_enabled())
                            ? task_group_key(task->date, task->time)
                            : QString();
                    if (!group_key.isEmpty() && group_key != last_group_key) {
                        auto *header_item = new QListWidgetItem(watermark_task_line(this, task->date, task->time));
                        header_item->setFlags((header_item->flags() & ~Qt::ItemIsSelectable) & ~Qt::ItemIsEnabled);
                        header_item->setData(kGroupHeaderRole, QStringLiteral("group_header"));
                        header_item->setData(kGroupKeyRole, group_key);
                        target_list->addItem(header_item);
                        last_group_key = group_key;
                    } else if (group_key.isEmpty()) {
                        last_group_key.clear();
                    }

                    target_list->addItem(item);
                }
            }
        }

        emit listas_atualizadas();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao atualizar listas de tarefas: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao atualizar listas de tarefas"), true);
    }
}

QString GerenciadorTarefas::strip_item_title(QListWidgetItem *item) const
{
    if (item == nullptr || is_group_header(item) || !(item->flags() & Qt::ItemIsSelectable)) {
        return {};
    }

    return compat::strip_priority_emoji(item->text().split(QLatin1Char('\n')).value(0)).trimmed();
}

Tarefa *GerenciadorTarefas::find_task(QListWidgetItem *item)
{
    if (item == nullptr) {
        return nullptr;
    }
    if (is_group_header(item) || !(item->flags() & Qt::ItemIsSelectable)) {
        return nullptr;
    }

    const QString id = item->data(Qt::UserRole).toString();
    if (!id.isEmpty()) {
        for (Tarefa &task : tarefas) {
            if (task.id == id) {
                return &task;
            }
        }
    }

    const QString title = strip_item_title(item);
    for (Tarefa &task : tarefas) {
        if (task.titulo.trimmed() == title) {
            return &task;
        }
    }

    return nullptr;
}

const Tarefa *GerenciadorTarefas::find_task(QListWidgetItem *item) const
{
    return const_cast<GerenciadorTarefas *>(this)->find_task(item);
}

void GerenciadorTarefas::open_linked_file_for_item(QListWidgetItem *item)
{
    try {
        Tarefa *task = find_task(item);
        if (task == nullptr || task->file_path.isEmpty()) {
            return;
        }

        if (QFileInfo::exists(task->file_path)) {
            compat::open_local_path(task->file_path, this);
            return;
        }

        switch (compat::prompt_missing_linked_file(this)) {
        case compat::MissingLinkedFileChoice::RemoveLink:
            task->file_path.clear();
            salvar_tarefas();
            atualizar_listas();
            compat::show_link_removed_success(this);
            return;
        case compat::MissingLinkedFileChoice::Search: {
            const QString novo_caminho = compat::browse_linked_file_replacement(this, task->file_path).trimmed();
            if (!novo_caminho.isEmpty()) {
                task->file_path = QFileInfo(novo_caminho).absoluteFilePath();
                salvar_tarefas();
                atualizar_listas();
            }
            return;
        }
        case compat::MissingLinkedFileChoice::None:
            return;
        }
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao abrir arquivo vinculado (Gestão de Tempo): %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao abrir arquivo vinculado (Gestão de Tempo)"), true);
    }
}

QVector<QListWidgetItem *> GerenciadorTarefas::collect_marked_items() const
{
    QVector<QListWidgetItem *> items;
    QSet<QString> seen;

    for (QListWidget *list : {col_todo->lista, col_doing->lista, col_done->lista}) {
        for (QListWidgetItem *item : list->selectedItems()) {
            if (item == nullptr || is_group_header(item) || !(item->flags() & Qt::ItemIsSelectable)) {
                continue;
            }
            const QString key = item->data(Qt::UserRole).toString().isEmpty()
                ? strip_item_title(item)
                : item->data(Qt::UserRole).toString();
            if (!seen.contains(key)) {
                seen.insert(key);
                items.append(item);
            }
        }

        for (int i = 0; i < list->count(); ++i) {
            if (auto *item = list->item(i);
                item != nullptr && !is_group_header(item) && (item->flags() & Qt::ItemIsSelectable)
                && item->checkState() == Qt::Checked) {
                const QString key = item->data(Qt::UserRole).toString().isEmpty()
                    ? strip_item_title(item)
                    : item->data(Qt::UserRole).toString();
                if (!seen.contains(key)) {
                    seen.insert(key);
                    items.append(item);
                }
            }
        }
    }

    return items;
}

QVariantMap GerenciadorTarefas::task_payload(const Tarefa &task) const
{
    return {
        {QStringLiteral("id"), task.id},
        {QStringLiteral("titulo"), task.titulo},
        {QStringLiteral("prioridade"), task.prioridade},
        {QStringLiteral("dominio"), task.dominio},
        {QStringLiteral("status"), task.status},
        {QStringLiteral("description"), task.descricao.isEmpty() ? QVariant() : QVariant(task.descricao)},
        {QStringLiteral("file_path"), task.file_path.isEmpty() ? QVariant() : QVariant(task.file_path)},
        {QStringLiteral("date"), task.date.isEmpty() ? QVariant() : QVariant(task.date)},
        {QStringLiteral("time"), task.time.isEmpty() ? QVariant() : QVariant(task.time)},
        {QStringLiteral("cor_texto"), task.cor_texto.isEmpty() ? QVariant() : QVariant(task.cor_texto)},
        {QStringLiteral("integracao_origem"),
         task.integracao_origem.isEmpty() ? QVariant() : QVariant(task.integracao_origem)},
        {QStringLiteral("recurrence"), task.recurrence.isEmpty() ? QVariant() : QVariant(task.recurrence)},
        {QStringLiteral("recorrencia"), task.recurrence.isEmpty() ? QVariant() : QVariant(task.recurrence)},
        {QStringLiteral("recurrence_until"), task.recurrence_until.isEmpty() ? QVariant() : QVariant(task.recurrence_until)},
        {QStringLiteral("recorrencia_ate"), task.recurrence_until.isEmpty() ? QVariant() : QVariant(task.recurrence_until)},
    };
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { void atualizar_listas(GerenciadorTarefas &self) { self.atualizar_listas(); } }
