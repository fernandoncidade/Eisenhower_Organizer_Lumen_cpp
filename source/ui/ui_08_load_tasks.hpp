#pragma once

#include <QString>

namespace source {
class EisenhowerMatrixApp;
}

namespace source::GerenciamentoUI::ui_08_load_tasks {

void load_tasks(source::EisenhowerMatrixApp *app);
bool load_tasks_from_path(source::EisenhowerMatrixApp *app, const QString &path);

}  // namespace source::GerenciamentoUI::ui_08_load_tasks
