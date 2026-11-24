#include "source/modules/feynman/Feynman/utils/EventBus.hpp"

#include <QCoreApplication>
#include <QPointer>

namespace source::plugin::Feynman::utils {
namespace {

QPointer<EventBus> g_event_bus_instance;

}  // namespace

void EventBus::send_conceito(const QVariantMap &dados)
{
    pending_conceitos_.append(dados);
    emit conceito_atualizado(dados);
}

void EventBus::drain_pending_conceitos()
{
    const QList<QVariantMap> pending = pending_conceitos_;
    pending_conceitos_.clear();
    for (const QVariantMap &item : pending) {
        emit conceito_atualizado(item);
    }
}

void EventBus::send_tarefa(const QVariantMap &dados)
{
    emit tarefa_integracao(dados);
}

void EventBus::receive_tarefa_from_host(const QVariantMap &dados)
{
    pending_tarefas_from_host_.append(dados);
    emit tarefa_integracao(dados);
}

void EventBus::drain_pending_tarefas()
{
    const QList<QVariantMap> pending = pending_tarefas_from_host_;
    pending_tarefas_from_host_.clear();
    for (const QVariantMap &item : pending) {
        emit tarefa_integracao(item);
    }
}

void EventBus::clear_pending()
{
    pending_conceitos_.clear();
    pending_tarefas_from_host_.clear();
}

EventBus *EventBus::instance()
{
    if (g_event_bus_instance == nullptr) {
        g_event_bus_instance = new EventBus(QCoreApplication::instance());
    }

    if (g_event_bus_instance != nullptr && g_event_bus_instance->parent() == nullptr) {
        if (auto *app = QCoreApplication::instance()) {
            g_event_bus_instance->setParent(app);
        }
    }

    return g_event_bus_instance;
}

EventBus *get_event_bus()
{
    return EventBus::instance();
}

}  // namespace source::plugin::Feynman::utils
