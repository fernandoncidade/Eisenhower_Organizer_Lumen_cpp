#pragma once

#include <QString>

class QListWidgetItem;
class QLocale;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore::incore_11_atualizar_itens_tarefas {

QString get_text(const QString &text);
QLocale effective_locale(source::EisenhowerMatrixApp *app);
QString base_text_from_item(QListWidgetItem *item);
void atualizar_itens_tarefas(source::EisenhowerMatrixApp *app);

}  // namespace source::InterfaceCore::incore_11_atualizar_itens_tarefas
