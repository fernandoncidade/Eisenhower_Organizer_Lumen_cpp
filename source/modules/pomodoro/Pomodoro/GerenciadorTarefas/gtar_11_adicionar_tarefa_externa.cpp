#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_11_adicionar_tarefa_externa.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

void GerenciadorTarefas::adicionar_tarefa_externa(const QString &titulo,
                                                  const QString &prioridade,
                                                  const QString &description,
                                                  const QString &file_path,
                                                  const QString &date,
                                                  const QString &time,
                                                  const QString &status,
                                                  const QString &task_id,
                                                  bool sincronizada_com_eisenhower,
                                                  int dominio,
                                                  const QString &cor_texto,
                                                  int pomodoros,
                                                  int ciclos_totais,
                                                  bool ciclos_customizados,
                                                  double duracao)
{
    try {
        const QString normalized_title = titulo.trimmed();
        if (normalized_title.isEmpty()) {
            return;
        }

        const QVariantMap candidate = {
            {QStringLiteral("id"), task_id},
            {QStringLiteral("titulo"), normalized_title},
            {QStringLiteral("prioridade"), Tarefa::normalizar_prioridade(prioridade)},
            {QStringLiteral("date"), date},
            {QStringLiteral("time"), time},
            {QStringLiteral("description"), description},
        };

        for (Tarefa &task : tarefas) {
            if (compat::tasks_match(task, candidate)) {
                task.sincronizada_com_eisenhower = sincronizada_com_eisenhower;
                if (!task_id.trimmed().isEmpty()) {
                    task.id = task_id.trimmed();
                }
                if (dominio >= 0) {
                    task.dominio = Tarefa::normalizar_dominio(dominio);
                }
                if (!cor_texto.trimmed().isEmpty()) {
                    task.cor_texto = cor_texto.trimmed();
                }
                if (!description.isEmpty()) {
                    task.descricao = description;
                }
                if (!file_path.isEmpty()) {
                    task.file_path = file_path;
                }
                if (!date.isEmpty()) {
                    task.date = date;
                }
                if (!time.isEmpty()) {
                    task.time = time;
                }
                if (!prioridade.isEmpty()) {
                    task.prioridade = Tarefa::normalizar_prioridade(prioridade);
                }
                if (pomodoros >= 0) {
                    task.pomodoros_completados = pomodoros;
                }
                if (ciclos_totais > 0) {
                    task.ciclos_totais = ciclos_totais;
                    task.ciclos_customizados = ciclos_customizados || (ciclos_totais != obter_ciclos_permitidos_padrao());
                }
                if (duracao > 0.0) {
                    task.duracao = duracao;
                }
                salvar_tarefas();
                atualizar_listas();
                return;
            }
        }

        _adicionar_tarefa_interna(
            normalized_title,
            Tarefa::normalizar_prioridade(prioridade),
            status.trimmed().isEmpty() ? QStringLiteral("Todo") : status.trimmed(),
            false,
            description,
            file_path,
            date,
            time,
            task_id,
            sincronizada_com_eisenhower,
            dominio,
            cor_texto,
            pomodoros,
            ciclos_totais,
            ciclos_customizados,
            duracao);
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao adicionar tarefa externa: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao adicionar tarefa externa"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns {
void adicionar_tarefa_externa(GerenciadorTarefas &self, const QString &titulo, const QString &prioridade, const QString &description, const QString &file_path, const QString &date, const QString &time, const QString &status, const QString &task_id, bool sincronizada_com_eisenhower, int dominio, const QString &cor_texto, int pomodoros, int ciclos_totais, bool ciclos_customizados, double duracao) {
    self.adicionar_tarefa_externa(titulo,
                                  prioridade,
                                  description,
                                  file_path,
                                  date,
                                  time,
                                  status,
                                  task_id,
                                  sincronizada_com_eisenhower,
                                  dominio,
                                  cor_texto,
                                  pomodoros,
                                  ciclos_totais,
                                  ciclos_customizados,
                                  duracao);
}
}
