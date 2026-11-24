#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_03_setup_integracao_eisenhower.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTempoInternal;

void GerenciadorTempo::_setup_integracao_eisenhower()
{
    try {
        event_bus_ = compat::get_event_bus();
        connect(event_bus_, &compat::EventBus::tarefa_integracao, this, &GerenciadorTempo::_on_event_bus_tarefa);
        event_bus_->drain_pending_tarefas();
    } catch (const std::exception &e) {
        logger_->debug(QStringLiteral("Falha ao configurar integração com Eisenhower: %1").arg(QString::fromUtf8(e.what())));
    } catch (...) {
        logger_->debug(QStringLiteral("Falha ao configurar integração com Eisenhower"));
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTempoFns { void setup_integracao_eisenhower(GerenciadorTempo &self) { self._setup_integracao_eisenhower(); } }
