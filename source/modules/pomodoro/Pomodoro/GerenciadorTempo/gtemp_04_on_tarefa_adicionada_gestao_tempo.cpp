#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_04_on_tarefa_adicionada_gestao_tempo.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTempoInternal;

void GerenciadorTempo::_on_tarefa_adicionada_gestao_tempo(const QVariantMap &dados)
{
    try {
        if (btn_eisenhower_integrado == nullptr || !btn_eisenhower_integrado->isChecked()) {
            return;
        }

        const QString titulo = dados.value(QStringLiteral("titulo")).toString();
        const QString prioridade = dados.value(QStringLiteral("prioridade")).toString();
        if (titulo.trimmed().isEmpty()) {
            return;
        }

        QVariantMap payload = {
            {QStringLiteral("integration_id"), QUuid::createUuid().toString(QUuid::WithoutBraces)},
            {QStringLiteral("origin"), QStringLiteral("tempo")},
            {QStringLiteral("id"), dados.value(QStringLiteral("id"))},
            {QStringLiteral("titulo"), titulo},
            {QStringLiteral("prioridade"), Tarefa::normalizar_prioridade(prioridade)},
            {QStringLiteral("description"), dados.value(QStringLiteral("description"))},
            {QStringLiteral("file_path"), dados.value(QStringLiteral("file_path"))},
            {QStringLiteral("date"), dados.value(QStringLiteral("date"))},
            {QStringLiteral("time"), dados.value(QStringLiteral("time"))},
        };
        compat::get_event_bus()->send_tarefa(payload);

        const QString task_id = dados.value(QStringLiteral("id")).toString().trimmed();
        if (!task_id.isEmpty() && tarefas != nullptr) {
            bool changed = false;
            for (Tarefa &task : tarefas->tarefas) {
                if (task.id == task_id && !task.sincronizada_com_eisenhower) {
                    task.sincronizada_com_eisenhower = true;
                    changed = true;
                    break;
                }
            }

            if (changed) {
                tarefas->salvar_tarefas();
            }
        }
    } catch (const std::exception &e) {
        logger_->debug(QStringLiteral("Falha ao enviar tarefa para Eisenhower: %1").arg(QString::fromUtf8(e.what())));
    } catch (...) {
        logger_->debug(QStringLiteral("Falha ao enviar tarefa para Eisenhower"));
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTempoFns { void on_tarefa_adicionada_gestao_tempo(GerenciadorTempo &self, const QVariantMap &dados) { self._on_tarefa_adicionada_gestao_tempo(dados); } }
