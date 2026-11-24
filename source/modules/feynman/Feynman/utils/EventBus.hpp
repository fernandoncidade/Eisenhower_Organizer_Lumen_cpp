#pragma once

#include <QList>
#include <QObject>
#include <QVariantMap>

namespace source::plugin::Feynman::utils {

class EventBus : public QObject {
    Q_OBJECT

public:
    static EventBus *instance();

    void send_conceito(const QVariantMap &dados);
    void drain_pending_conceitos();
    void send_tarefa(const QVariantMap &dados);
    void receive_tarefa_from_host(const QVariantMap &dados);
    void drain_pending_tarefas();
    void clear_pending();

signals:
    void conceito_atualizado(const QVariantMap &dados);
    void tarefa_integracao(const QVariantMap &dados);

private:
    explicit EventBus(QObject *parent = nullptr) : QObject(parent) {}

    QList<QVariantMap> pending_conceitos_;
    QList<QVariantMap> pending_tarefas_from_host_;
};

EventBus *get_event_bus();

}  // namespace source::plugin::Feynman::utils
