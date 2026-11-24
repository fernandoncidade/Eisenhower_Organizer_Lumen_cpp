#pragma once

#include <QVariantMap>

#include "source/modules/pomodoro/Pomodoro/tmp_01_Tarefa.hpp"

namespace source::plugin::Pomodoro::TarefaFns {

QVariantMap to_dict(const Tarefa &self);

}  // namespace source::plugin::Pomodoro::TarefaFns
