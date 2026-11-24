#pragma once
#include "source/modules/pomodoro/Pomodoro/tmp_03_GerenciadorTarefas.hpp"
namespace source::plugin::Pomodoro::GerenciadorTarefasFns {
Tarefa *adicionar_tarefa_interna(GerenciadorTarefas &self, const QString &titulo, const QString &prioridade, const QString &status = QStringLiteral("Todo"), bool emitir_sinal = true, const QString &description = QString(), const QString &file_path = QString(), const QString &date = QString(), const QString &time = QString(), const QString &task_id = QString(), bool sincronizada_com_eisenhower = false, const QString &cor_texto = QString(), int pomodoros = -1, int ciclos_totais = -1, bool ciclos_customizados = false, double duracao = -1.0);
}
