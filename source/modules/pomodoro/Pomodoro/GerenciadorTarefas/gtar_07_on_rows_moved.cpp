#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_07_on_rows_moved.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

void GerenciadorTarefas::_on_rows_moved(QListWidget *lista)
{
    try {
        _sincronizar_status_pos_movimento(lista);
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao tratar rowsMoved: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao tratar rowsMoved"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { void on_rows_moved(GerenciadorTarefas &self, QListWidget *lista) { self._on_rows_moved(lista); } }
