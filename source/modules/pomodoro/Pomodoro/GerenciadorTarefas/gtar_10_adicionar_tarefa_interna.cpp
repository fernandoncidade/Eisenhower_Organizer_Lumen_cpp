#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_10_adicionar_tarefa_interna.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

Tarefa *GerenciadorTarefas::_adicionar_tarefa_interna(const QString &titulo,
                                                      const QString &prioridade,
                                                      const QString &status,
                                                      bool emitir_sinal,
                                                      const QString &description,
                                                      const QString &file_path,
                                                      const QString &date,
                                                      const QString &time,
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
        const QString normalized_status = normalize_task_status(status);
        tarefas.append(Tarefa(titulo,
                      description,
                      {},
                      prioridade,
                      normalized_status,
                      task_id,
                      file_path,
                      date,
                      time,
                      dominio >= 0 ? dominio : 0,
                      cor_texto));
        if (pomodoros >= 0) {
            tarefas.last().pomodoros_completados = pomodoros;
        }
        if (ciclos_totais > 0) {
            tarefas.last().ciclos_totais = ciclos_totais;
            tarefas.last().ciclos_customizados = ciclos_customizados || (ciclos_totais != obter_ciclos_permitidos_padrao());
        } else {
            tarefas.last().ciclos_totais = obter_ciclos_permitidos_padrao();
            tarefas.last().ciclos_customizados = false;
        }
        if (duracao > 0.0) {
            tarefas.last().duracao = duracao;
        }
        tarefas.last().sincronizada_com_eisenhower = sincronizada_com_eisenhower;
        salvar_tarefas();
        atualizar_listas();

        Tarefa &task = tarefas.last();
        if (emitir_sinal) {
            emit tarefa_adicionada(task_payload(task));
        }
        return &task;
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao adicionar tarefa internamente: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao adicionar tarefa internamente"), true);
    }

    return nullptr;
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns {
Tarefa *adicionar_tarefa_interna(GerenciadorTarefas &self, const QString &titulo, const QString &prioridade, const QString &status, bool emitir_sinal, const QString &description, const QString &file_path, const QString &date, const QString &time, const QString &task_id, bool sincronizada_com_eisenhower, const QString &cor_texto, int pomodoros, int ciclos_totais, bool ciclos_customizados, double duracao) {
    return self._adicionar_tarefa_interna(titulo, prioridade, status, emitir_sinal, description, file_path, date, time, task_id, sincronizada_com_eisenhower, 0, cor_texto, pomodoros, ciclos_totais, ciclos_customizados, duracao);
}
}
