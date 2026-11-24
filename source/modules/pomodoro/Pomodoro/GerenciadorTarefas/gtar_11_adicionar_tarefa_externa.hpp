#pragma once
#include "source/modules/pomodoro/Pomodoro/tmp_03_GerenciadorTarefas.hpp"
namespace source::plugin::Pomodoro::GerenciadorTarefasFns {
void adicionar_tarefa_externa(GerenciadorTarefas &self, const QString &titulo, const QString &prioridade = QStringLiteral("Importante e Urgente"), const QString &description = QString(), const QString &file_path = QString(), const QString &date = QString(), const QString &time = QString(), const QString &status = QStringLiteral("Todo"), const QString &task_id = QString(), bool sincronizada_com_eisenhower = true, int dominio = -1, const QString &cor_texto = QString(), int pomodoros = -1, int ciclos_totais = -1, bool ciclos_customizados = false, double duracao = -1.0);
}
