#include "source/modules/agenda/AgendaAvaliacoes/GerenciamentoAtividades/Gerenciador_Atividades.hpp"
#include "source/utils/LogManager.hpp"

#include <QSqlError>
#include <QSqlQuery>

bool GerenciamentoAtividades::atualizar_atividade(const Atividade &atividade, const Atividade &novos_dados) {
    if (!usa_atividades_locais()) {
        return false;
    }
    if (atividades.isEmpty()) {
        listar_atividades();
    }
    const QString chaveAntiga = chave_atividade(atividade);
    if (!atividades.contains(chaveAntiga)) {
        return false;
    }
    atividades.remove(chaveAntiga);
    atividades.insert(chave_atividade(novos_dados), novos_dados);
    QSqlQuery query(conexao);
    query.prepare("UPDATE atividades SET data = ?, tipo = ?, sequencia = ?, nome = ?, turma = ? WHERE data = ? AND "
                  "tipo = ? AND sequencia = ? AND nome = ? AND turma = ?");
    query.addBindValue(novos_dados.data);
    query.addBindValue(novos_dados.tipo);
    query.addBindValue(novos_dados.sequencia);
    query.addBindValue(novos_dados.nome);
    query.addBindValue(novos_dados.turma);
    query.addBindValue(atividade.data);
    query.addBindValue(atividade.tipo);
    query.addBindValue(atividade.sequencia);
    query.addBindValue(atividade.nome);
    query.addBindValue(atividade.turma);
    if (!query.exec()) {
        source::utils::LogManager::get_logger()->error(QString("Erro ao atualizar atividade: %1").arg(query.lastError().text()));
        return false;
    }
    update_textbox();
    return true;
}
