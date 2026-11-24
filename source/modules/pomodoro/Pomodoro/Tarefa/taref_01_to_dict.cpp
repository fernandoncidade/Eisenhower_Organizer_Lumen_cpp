#include "source/modules/pomodoro/Pomodoro/Tarefa/taref_01_to_dict.hpp"

#include <exception>

namespace source::plugin::Pomodoro {

QVariantMap Tarefa::to_dict() const
{
    try {
        return {
            {QStringLiteral("id"), id},
            {QStringLiteral("titulo"), titulo},
            {QStringLiteral("descricao"), descricao},
            {QStringLiteral("etapas"), etapas},
            {QStringLiteral("prioridade"), prioridade},
            {QStringLiteral("status"), status},
            {QStringLiteral("data_criacao"), data_criacao},
            {QStringLiteral("pomodoros"), pomodoros_completados},
            {QStringLiteral("pomodoros_completados"), pomodoros_completados},
            {QStringLiteral("ciclos_totais"), ciclos_totais},
            {QStringLiteral("ciclos_permitidos"), ciclos_totais},
            {QStringLiteral("total_ciclos"), ciclos_totais},
            {QStringLiteral("ciclos_customizados"), ciclos_customizados},
            {QStringLiteral("percentual"), percentual_conclusao()},
            {QStringLiteral("file_path"), file_path.isEmpty() ? QVariant() : QVariant(file_path)},
            {QStringLiteral("date"), date.isEmpty() ? QVariant() : QVariant(date)},
            {QStringLiteral("time"), time.isEmpty() ? QVariant() : QVariant(time)},
            {QStringLiteral("dominio"), dominio},
            {QStringLiteral("duracao"), duracao},
            {QStringLiteral("duration"), duracao},
            {QStringLiteral("cor_texto"), cor_texto.isEmpty() ? QVariant() : QVariant(cor_texto)},
            {QStringLiteral("integracao_origem"), integracao_origem.isEmpty() ? QVariant() : QVariant(integracao_origem)},
            {QStringLiteral("eisenhower_synced"), sincronizada_com_eisenhower},
            {QStringLiteral("recurrence"), recurrence},
            {QStringLiteral("recorrencia"), recurrence},
            {QStringLiteral("recurrence_until"), recurrence_until},
            {QStringLiteral("recorrencia_ate"), recurrence_until},
            {QStringLiteral("font_config"), font_config.isEmpty() ? QVariant() : QVariant(font_config)},
        };
    } catch (const std::exception &e) {
        logger->error(QStringLiteral("Erro ao converter tarefa para dicionário: %1").arg(QString::fromUtf8(e.what())), true);
        return {};
    } catch (...) {
        logger->error(QStringLiteral("Erro ao converter tarefa para dicionário"), true);
        return {};
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::TarefaFns {

QVariantMap to_dict(const Tarefa &self)
{
    return self.to_dict();
}

}  // namespace source::plugin::Pomodoro::TarefaFns
