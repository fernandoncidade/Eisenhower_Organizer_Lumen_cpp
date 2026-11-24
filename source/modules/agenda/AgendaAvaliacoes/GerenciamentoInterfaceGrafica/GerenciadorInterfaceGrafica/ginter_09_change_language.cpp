#include "source/modules/agenda/AgendaAvaliacoes/BancoDeDados/Banco_Dados.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/GerenciamentoInterfaceGrafica/Interface_Grafica.hpp"
#include "source/utils/LogManager.hpp"

void InterfaceGerenciadorAtividades::change_language(const QString &codigo_idioma) {
    if (!gerenciador_traducao)
        return;
    if (gerenciador_traducao->obter_idioma_atual() == codigo_idioma) {
        for (auto it = lang_actions.begin(); it != lang_actions.end(); ++it) {
            it.value()->setChecked(it.key() == codigo_idioma);
        }
        return;
    }
    const bool sucesso = gerenciador_traducao->definir_idioma(codigo_idioma);
    if (sucesso) {
        atualizar_listas_exportadas();
        update_textbox();
    }
    for (auto it = lang_actions.begin(); it != lang_actions.end(); ++it) {
        it.value()->setChecked(it.key() == gerenciador_traducao->obter_idioma_atual());
    }
}
