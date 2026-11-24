#include "source/modules/agenda/AgendaAvaliacoes/BancoDeDados/Banco_Dados.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/GerenciamentoAtividades/Gerenciador_Atividades.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/utils/ComboBoxUtils.hpp"

void GerenciamentoAtividades::update_disciplinas() {
    const QString curso = combo_curso ? combo_curso->currentText() : QString();
    const QString ementa = entry_ementa ? entry_ementa->currentText() : QString();
    const QString semestre = entry_semestre ? entry_semestre->currentText() : QString();
    substituir_itens_combo(entry_disciplina, QStringList{QString()} + obter_disciplinas(curso, ementa, semestre));
}
