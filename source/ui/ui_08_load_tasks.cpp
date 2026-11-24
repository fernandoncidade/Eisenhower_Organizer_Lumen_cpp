#include "source/ui/ui_08_load_tasks.hpp"

#include "source/ui/agregador_01_ui.hpp"

namespace source::GerenciamentoUI::ui_08_load_tasks {

void load_tasks(source::EisenhowerMatrixApp *app)
{
    source::GerenciamentoUI::load_tasks(app);
}

bool load_tasks_from_path(source::EisenhowerMatrixApp *app, const QString &path)
{
    return source::GerenciamentoUI::load_tasks_from_path(app, path);
}

}  // namespace source::GerenciamentoUI::ui_08_load_tasks
