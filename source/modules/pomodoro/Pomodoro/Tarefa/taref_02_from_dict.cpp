#include "source/modules/pomodoro/Pomodoro/Tarefa/taref_02_from_dict.hpp"

#include <exception>

#include <QDateTime>

namespace source::plugin::Pomodoro {

std::optional<Tarefa> Tarefa::from_dict(const QVariantMap &data)
{
    auto *logger = source::utils::LogManager::get_logger();

    try {
        Tarefa tarefa(
            data.value(QStringLiteral("titulo")).toString(),
            data.value(QStringLiteral("descricao")).toString(),
            data.value(QStringLiteral("etapas")).toStringList(),
            normalizar_prioridade(data.value(QStringLiteral("prioridade"), QStringLiteral("Importante e Urgente")).toString()),
            data.value(QStringLiteral("status"), QStringLiteral("Todo")).toString(),
            data.value(QStringLiteral("id")).toString(),
            data.value(QStringLiteral("file_path")).toString(),
            data.value(QStringLiteral("date")).toString(),
            data.value(QStringLiteral("time")).toString(),
            normalizar_dominio(data.value(QStringLiteral("dominio"))),
            data.value(QStringLiteral("cor_texto"), data.value(QStringLiteral("task_color"))).toString(),
            data.value(QStringLiteral("integracao_origem"), data.value(QStringLiteral("origin"))).toString());

        tarefa.data_criacao = data.value(QStringLiteral("data_criacao"), QDateTime::currentDateTime().toString(Qt::ISODate)).toString();
        tarefa.pomodoros_completados = data.value(QStringLiteral("pomodoros_completados"), data.value(QStringLiteral("pomodoros"), 0)).toInt();
        int c_totais = data.value(QStringLiteral("ciclos_totais"),
                       data.value(QStringLiteral("ciclos_permitidos"),
                       data.value(QStringLiteral("total_ciclos"),
                       data.value(QStringLiteral("ciclos_planejados"),
                       data.value(QStringLiteral("ciclos_alvo"), 4))))).toInt();
        tarefa.ciclos_totais = c_totais > 0 ? c_totais : 4;
        tarefa.ciclos_customizados = data.value(QStringLiteral("ciclos_customizados"), false).toBool();
        tarefa.sincronizada_com_eisenhower = data.value(QStringLiteral("eisenhower_synced"), false).toBool();
        tarefa.recurrence = data.value(QStringLiteral("recurrence"), data.value(QStringLiteral("recorrencia"))).toString();
        tarefa.recurrence_until = data.value(QStringLiteral("recurrence_until"), data.value(QStringLiteral("recorrencia_ate"))).toString();
        tarefa.font_config = data.value(QStringLiteral("font_config")).toMap();
        if (data.contains(QStringLiteral("duracao")) && data.value(QStringLiteral("duracao")).toDouble() > 0.0) {
            tarefa.duracao = data.value(QStringLiteral("duracao")).toDouble();
        } else if (data.contains(QStringLiteral("duration")) && data.value(QStringLiteral("duration")).toDouble() > 0.0) {
            tarefa.duracao = data.value(QStringLiteral("duration")).toDouble();
        }
        return tarefa;
    } catch (const std::exception &e) {
        logger->error(QStringLiteral("Erro ao criar tarefa a partir de dicionário: %1").arg(QString::fromUtf8(e.what())), true);
        return std::nullopt;
    } catch (...) {
        logger->error(QStringLiteral("Erro ao criar tarefa a partir de dicionário"), true);
        return std::nullopt;
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::TarefaFns {

std::optional<Tarefa> from_dict(const QVariantMap &data)
{
    return Tarefa::from_dict(data);
}

}  // namespace source::plugin::Pomodoro::TarefaFns
