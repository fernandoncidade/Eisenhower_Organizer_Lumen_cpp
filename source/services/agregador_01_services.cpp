#include "source/services/agregador_01_services.hpp"

#include "source/services/incore_01_initUI.hpp"
#include "source/services/incore_02_add_placeholder.hpp"
#include "source/services/incore_03_criar_menu_configuracoes.hpp"
#include "source/services/incore_04_definir_idioma.hpp"
#include "source/services/incore_05_atualizar_textos.hpp"
#include "source/services/incore_06_atualizar_placeholders.hpp"
#include "source/services/incore_07_MostrarMenuContexto.hpp"
#include "source/services/incore_08_exibir_sobre.hpp"

namespace source::InterfaceCore {

void init_ui(source::EisenhowerMatrixApp *app)
{
    incore_01_initUI::init_ui(app);
}

void add_placeholder(source::EisenhowerMatrixApp *app, QListWidget *list_widget, const QString &text)
{
    incore_02_add_placeholder::add_placeholder(app, list_widget, text);
}

void criar_menu_configuracoes(source::EisenhowerMatrixApp *app)
{
    incore_03_criar_menu_configuracoes::criar_menu_configuracoes(app);
}

bool definir_idioma(source::EisenhowerMatrixApp *app, const QString &codigo_idioma)
{
    return incore_04_definir_idioma::definir_idioma(app, codigo_idioma);
}

void atualizar_textos(source::EisenhowerMatrixApp *app)
{
    incore_05_atualizar_textos::atualizar_textos(app);
}

void atualizar_placeholders(source::EisenhowerMatrixApp *app)
{
    incore_06_atualizar_placeholders::atualizar_placeholders(app);
}

void show_context_menu(source::EisenhowerMatrixApp *app, const QPoint &point, QListWidget *list_widget)
{
    incore_07_MostrarMenuContexto::show_context_menu(app, point, list_widget);
}

void exibir_sobre(source::EisenhowerMatrixApp *app)
{
    incore_08_exibir_sobre::exibir_sobre(app);
}

void exibir_manual(source::EisenhowerMatrixApp *app)
{
    incore_08_exibir_sobre::exibir_manual(app);
}

}  // namespace source::InterfaceCore
