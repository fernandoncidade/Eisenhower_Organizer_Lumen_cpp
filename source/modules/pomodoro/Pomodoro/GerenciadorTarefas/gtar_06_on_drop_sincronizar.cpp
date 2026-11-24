#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_06_on_drop_sincronizar.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

void GerenciadorTarefas::_on_drop_sincronizar()
{
    try {
        auto *sender_list = qobject_cast<QListWidget *>(sender());
        _sincronizar_status_pos_movimento(sender_list);
        atualizar_listas();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao sincronizar após drop: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao sincronizar após drop"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { void on_drop_sincronizar(GerenciadorTarefas &self) { self._on_drop_sincronizar(); } }
