#include "source/modules/pomodoro/Pomodoro/tmp_03_GerenciadorTarefas.hpp"

#include <exception>

#include <QSizePolicy>
#include <QWidget>

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"
#include "source/utils/UnifiedTaskStore.hpp"

namespace source::plugin::Pomodoro {

KanbanColumn::KanbanColumn(const QString &status_value, QWidget *parent)
    : QGroupBox(parent), status(status_value)
{
}

GerenciadorTarefas::GerenciadorTarefas(QWidget *parent)
    : GerenciadorTarefas(QStringLiteral("simple:tabs"), parent)
{
}

GerenciadorTarefas::GerenciadorTarefas(const QString &ui_layout_mode_key, QWidget *parent)
    : QWidget(parent),
      logger_(GerenciadorTarefasInternal::logger()),
      gerenciador_botoes_(this),
      ui_layout_mode_key_(GerenciadorTarefasInternal::normalized_ui_layout_key(ui_layout_mode_key))
{
    try {
        arquivo_tarefas = source::utils::UnifiedTaskStore::shared_tasks_path();
        tarefas = carregar_tarefas();

        setup_ui();
        installEventFilter(this);

        _aplicar_tema_dinamico_inputs();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao inicializar GerenciadorTarefas: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao inicializar GerenciadorTarefas"), true);
    }
}

void GerenciadorTarefas::set_pomodoro_timer(PomodoroTimer *timer)
{
    pomodoro_timer_ = timer;
}

void GerenciadorTarefas::on_default_ciclos_changed(int novos_ciclos)
{
    if (novos_ciclos <= 0) {
        return;
    }
    bool alterado = false;
    for (auto &t : tarefas) {
        if (t.status == QStringLiteral("Doing") && !t.ciclos_customizados && t.ciclos_totais != novos_ciclos) {
            t.ciclos_totais = novos_ciclos;
            alterado = true;
        }
    }
    if (alterado) {
        salvar_tarefas();
        atualizar_listas();
        emit listas_atualizadas();
    }
}

int GerenciadorTarefas::obter_ciclos_permitidos_padrao() const
{
    if (pomodoro_timer_ != nullptr) {
        return qMax(1, pomodoro_timer_->ciclos_permitidos);
    }
    const QObject *p = parent();
    while (p != nullptr) {
        if (auto *tempo = qobject_cast<const source::plugin::Pomodoro::GerenciadorTempo *>(p)) {
            if (tempo->pomodoro != nullptr) {
                return qMax(1, tempo->pomodoro->ciclos_permitidos);
            }
        }
        p = p->parent();
    }
    if (window() != nullptr) {
        if (auto *pt = window()->findChild<source::plugin::Pomodoro::PomodoroTimer *>()) {
            return qMax(1, pt->ciclos_permitidos);
        }
    }
    return source::plugin::Pomodoro::PomodoroTimer::obter_ciclos_permitidos_persistidos();
}

}  // namespace source::plugin::Pomodoro
