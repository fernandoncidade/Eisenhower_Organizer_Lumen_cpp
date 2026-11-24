#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_01_carregar_tarefas.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

QVector<Tarefa> GerenciadorTarefas::carregar_tarefas()
{
    try {
        QVector<Tarefa> loaded = load_tasks_from_path(arquivo_tarefas);
        if (!loaded.isEmpty()) {
            return loaded;
        }

        const QString legacy_path = legacy_tasks_path();
        if (!legacy_path.isEmpty() && QFileInfo::exists(legacy_path)
            && QFileInfo(legacy_path).absoluteFilePath() != QFileInfo(arquivo_tarefas).absoluteFilePath()) {
            loaded = load_tasks_from_path(legacy_path);
            if (!loaded.isEmpty()) {
                source::utils::UnifiedTaskStore::replace_module_records(arquivo_tarefas,
                                                                        QStringLiteral("pomodoro"),
                                                                        records_from_tasks(loaded));
            }
        }

        return loaded;
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao carregar tarefas: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao carregar tarefas"), true);
    }

    return {};
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { QVector<Tarefa> carregar_tarefas(GerenciadorTarefas &self) { return self.carregar_tarefas(); } }
