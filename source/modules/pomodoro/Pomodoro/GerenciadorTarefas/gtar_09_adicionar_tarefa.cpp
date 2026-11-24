#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_09_adicionar_tarefa.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

void GerenciadorTarefas::adicionar_tarefa()
{
    try {
        const QString titulo = input_tarefa->text().trimmed();
        if (titulo.isEmpty()) {
            return;
        }

        static const QStringList priorities = {
            QStringLiteral("Importante e Urgente"),
            QStringLiteral("Importante, mas Não Urgente"),
            QStringLiteral("Não Importante, mas Urgente"),
            QStringLiteral("Não Importante e Não Urgente"),
        };

        QString file_path;
        if (!dragged_file_path_.isEmpty()) {
            if (compat::MessageBox::tempo_confirm_attach_file(this, QFileInfo(dragged_file_path_).fileName())) {
                file_path = dragged_file_path_;
            }
        }

        if (compat::host_bridge() != nullptr) {
            compat::host_bridge()->show_sync_operation_info(QStringLiteral("add"), QStringLiteral("pomodoro"));
        }

        Tarefa *task = _adicionar_tarefa_interna(
            titulo,
            priorities.value(combo_prioridade->currentIndex(), priorities.constFirst()),
            QStringLiteral("Todo"),
            true,
            QString(),
            file_path,
            QString(),
            QString(),
            QString(),
            false,
            combo_dominio != nullptr ? combo_dominio->currentIndex() : 0);

        if (task != nullptr) {
            Q_UNUSED(task);
            input_tarefa->clear();
            dragged_file_path_.clear();
            input_tarefa->setToolTip(QString());
        }
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao adicionar tarefa: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao adicionar tarefa"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { void adicionar_tarefa(GerenciadorTarefas &self) { self.adicionar_tarefa(); } }
