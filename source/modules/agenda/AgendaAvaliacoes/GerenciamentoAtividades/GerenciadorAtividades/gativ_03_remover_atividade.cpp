#include "source/modules/agenda/AgendaAvaliacoes/GerenciamentoAtividades/Gerenciador_Atividades.hpp"
#include "source/utils/LogManager.hpp"

#include <QSqlError>
#include <QSqlQuery>

void GerenciamentoAtividades::remover_atividade(const Atividade &atividade) {
    atividades.remove(chave_atividade(atividade));
    if (!usa_atividades_locais()) {
        return;
    }
    QSqlQuery query(conexao);
    query.prepare("DELETE FROM atividades WHERE data = ? AND tipo = ? AND sequencia = ? AND nome = ? AND turma = ?");
    query.addBindValue(atividade.data);
    query.addBindValue(atividade.tipo);
    query.addBindValue(atividade.sequencia);
    query.addBindValue(atividade.nome);
    query.addBindValue(atividade.turma);
    if (!query.exec()) {
        source::utils::LogManager::get_logger()->error(QString("Erro ao remover atividade: %1").arg(query.lastError().text()));
    }
}
