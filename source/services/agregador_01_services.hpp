#pragma once

#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

#include "source/services/incore_01_initUI.hpp"
#include "source/services/incore_02_add_placeholder.hpp"
#include "source/services/incore_03_criar_menu_configuracoes.hpp"
#include "source/services/incore_04_definir_idioma.hpp"
#include "source/services/incore_05_atualizar_textos.hpp"
#include "source/services/incore_06_atualizar_placeholders.hpp"
#include "source/services/incore_07_MostrarMenuContexto.hpp"
#include "source/services/incore_08_exibir_sobre.hpp"
#include "source/services/incore_09_Arquivo.hpp"
#include "source/services/incore_10_task_list_widget.hpp"
#include "source/services/incore_11_atualizar_itens_tarefas.hpp"
#include "source/services/incore_12_Manual.hpp"
#include "source/services/incore_16_manual_pt-BR.hpp"
#include "source/services/incore_17_manual_en-US.hpp"
#include "source/services/incore_13_prioridade_display.hpp"
#include "source/services/incore_14_EditTaskDialog.hpp"

#include <QString>

class QListWidget;
class QListWidgetItem;
class QPoint;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore {

class Arquivo;

void init_ui(source::EisenhowerMatrixApp *app);
void add_placeholder(source::EisenhowerMatrixApp *app, QListWidget *list_widget, const QString &text);
void criar_menu_configuracoes(source::EisenhowerMatrixApp *app);
bool definir_idioma(source::EisenhowerMatrixApp *app, const QString &codigo_idioma);
void atualizar_textos(source::EisenhowerMatrixApp *app);
void atualizar_placeholders(source::EisenhowerMatrixApp *app);
void show_context_menu(source::EisenhowerMatrixApp *app, const QPoint &point, QListWidget *list_widget);
void exibir_sobre(source::EisenhowerMatrixApp *app);
void exibir_manual(source::EisenhowerMatrixApp *app);

}  // namespace source::InterfaceCore
