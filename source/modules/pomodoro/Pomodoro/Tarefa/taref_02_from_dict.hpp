#pragma once

#include <QVariantMap>

#include <optional>

#include "source/modules/pomodoro/Pomodoro/tmp_01_Tarefa.hpp"

namespace source::plugin::Pomodoro::TarefaFns {

std::optional<Tarefa> from_dict(const QVariantMap &data);

}  // namespace source::plugin::Pomodoro::TarefaFns
