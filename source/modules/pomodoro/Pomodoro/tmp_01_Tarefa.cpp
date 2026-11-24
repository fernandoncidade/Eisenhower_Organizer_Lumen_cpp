#include "source/modules/pomodoro/Pomodoro/tmp_01_Tarefa.hpp"

#include <QUuid>

#include "source/utils/TaskProgressHelper.hpp"

namespace source::plugin::Pomodoro {

Tarefa::Tarefa(const QString &titulo_value,
               const QString &descricao_value,
               const QStringList &etapas_value,
               const QString &prioridade_value,
               const QString &status_value,
               const QString &id_value,
               const QString &file_path_value,
               const QString &date_value,
               const QString &time_value,
               int dominio_value,
               const QString &cor_texto_value,
               const QString &integracao_origem_value)
    : titulo(titulo_value),
      descricao(descricao_value),
      etapas(etapas_value),
      prioridade(normalizar_prioridade(prioridade_value)),
      status(status_value),
      data_criacao(QDateTime::currentDateTime().toString(Qt::ISODate)),
      pomodoros_completados(0),
      ciclos_totais(4),
      id(id_value.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : id_value),
      file_path(file_path_value),
      date(date_value),
      time(time_value),
      dominio(normalizar_dominio(dominio_value)),
      cor_texto(cor_texto_value.trimmed()),
      integracao_origem(integracao_origem_value.trimmed()),
      logger(source::utils::LogManager::get_logger())
{
}

double Tarefa::percentual_conclusao() const
{
    return source::utils::calculate_task_completion_percentage(
        pomodoros_completados, ciclos_totais, status == QStringLiteral("Done"));
}

}  // namespace source::plugin::Pomodoro
