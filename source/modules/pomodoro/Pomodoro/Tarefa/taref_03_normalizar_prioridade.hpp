#pragma once

#include <QString>
#include <QVariant>

#include "source/modules/pomodoro/Pomodoro/tmp_01_Tarefa.hpp"

namespace source::plugin::Pomodoro::TarefaFns {

QString normalizar_prioridade(const QString &valor);
int normalizar_dominio(const QVariant &valor);

}  // namespace source::plugin::Pomodoro::TarefaFns
