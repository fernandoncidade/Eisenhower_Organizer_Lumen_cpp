#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_05_on_event_bus_tarefa.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTempoInternal;

void GerenciadorTempo::_on_event_bus_tarefa(const QVariantMap &dados)
{
    try {
        const QString origin = dados.value(QStringLiteral("origin")).toString().trimmed().toCaseFolded();
        if (origin != QStringLiteral("eisenhower") && origin != QStringLiteral("feynman")) {
            return;
        }

        const QString integration_id = dados.value(QStringLiteral("integration_id")).toString();
        if (!integration_id.isEmpty() && integracao_ids_recebidos_.contains(integration_id)) {
            return;
        }

        if (!integration_id.isEmpty()) {
            integracao_ids_recebidos_.insert(integration_id);
        }

        const QString titulo = dados.value(QStringLiteral("titulo")).toString().trimmed();
        if (titulo.isEmpty() || tarefas == nullptr) {
            return;
        }

        const int pomodoros = dados.value(QStringLiteral("pomodoros"), dados.value(QStringLiteral("pomodoros_completados"), -1)).toInt();
        const int ciclos_totais = dados.value(QStringLiteral("ciclos_totais"), dados.value(QStringLiteral("ciclos_permitidos"), dados.value(QStringLiteral("total_ciclos"), -1))).toInt();
        const bool ciclos_customizados = dados.value(QStringLiteral("ciclos_customizados"), false).toBool();
        const double duracao = dados.value(QStringLiteral("duracao"), -1.0).toDouble();

        tarefas->pending_shared_origin_ = origin;
        tarefas->adicionar_tarefa_externa(
            titulo,
            dados.value(QStringLiteral("prioridade")).toString(),
            dados.value(QStringLiteral("description")).toString(),
            dados.value(QStringLiteral("file_path")).toString(),
            dados.value(QStringLiteral("date")).toString(),
            dados.value(QStringLiteral("time")).toString(),
            dados.value(QStringLiteral("status")).toString(),
            dados.value(QStringLiteral("id")).toString(),
            origin == QStringLiteral("eisenhower"),
            dados.contains(QStringLiteral("dominio")) ? Tarefa::normalizar_dominio(dados.value(QStringLiteral("dominio"))) : -1,
            dados.value(QStringLiteral("cor_texto"), dados.value(QStringLiteral("task_color"))).toString(),
            pomodoros,
            ciclos_totais,
            ciclos_customizados,
            duracao);
    } catch (const std::exception &e) {
        logger_->debug(QStringLiteral("Falha ao receber tarefa externa: %1").arg(QString::fromUtf8(e.what())));
    } catch (...) {
        logger_->debug(QStringLiteral("Falha ao receber tarefa externa"));
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTempoFns { void on_event_bus_tarefa(GerenciadorTempo &self, const QVariantMap &dados) { self._on_event_bus_tarefa(dados); } }
