#include "source/modules/agenda/AgendaAvaliacoes/GerenciamentoInterfaceGrafica/Interface_Grafica.hpp"

void InterfaceGerenciadorAtividades::change_color_mode(const QString &codigo_modo) {
    const bool sucesso = gerenciamento_atividades.definir_modo_cores(codigo_modo);
    if (sucesso) {
        if (menu_cores != nullptr) {
            populate_color_menu(menu_cores);
        }
        update_textbox();
        emit modo_cores_alterado(codigo_modo);
        return;
    }
    if (menu_cores != nullptr) {
        populate_color_menu(menu_cores);
    }
}
