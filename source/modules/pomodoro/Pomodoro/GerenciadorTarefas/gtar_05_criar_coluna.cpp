#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_05_criar_coluna.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

KanbanColumn *GerenciadorTarefas::criar_coluna(const QString &status)
{
    try {
        auto *group = new KanbanColumn(status, this);
        auto *layout = new QVBoxLayout(group);
        layout->setContentsMargins(8, 8, 8, 8);
        layout->setSpacing(0);
        group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        group->lista = new _KanbanListWidget(group);
        group->lista->setDragEnabled(true);
        group->lista->setAcceptDrops(true);
        group->lista->setDropIndicatorShown(true);
        group->lista->setDefaultDropAction(Qt::MoveAction);
        group->lista->setDragDropMode(QListWidget::DragDrop);
        group->lista->setSelectionMode(QAbstractItemView::ExtendedSelection);
        group->lista->setSelectionBehavior(QAbstractItemView::SelectItems);
        group->lista->setSelectionRectVisible(true);
        group->lista->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        group->lista->setMinimumHeight(0);
        source::utils::install_linked_title_underline_delegate(group->lista);

        connect(group->lista, &QListWidget::itemSelectionChanged, this, [this, group]() {
            if (group->lista != nullptr && !group->lista->selectedItems().isEmpty()) {
                for (KanbanColumn *other_col : {col_todo, col_doing, col_done}) {
                    if (other_col != nullptr && other_col != group && other_col->lista != nullptr) {
                        if (!other_col->lista->selectedItems().isEmpty() || other_col->lista->currentItem() != nullptr) {
                            QSignalBlocker blocker(other_col->lista);
                            if (QItemSelectionModel *sm = other_col->lista->selectionModel(); sm != nullptr) {
                                sm->clear();
                            } else {
                                other_col->lista->clearSelection();
                            }
                        }
                    }
                }
            }
            emit listas_atualizadas();
        });

        connect(group->lista, &_KanbanListWidget::tarefas_soltadas, this, &GerenciadorTarefas::_on_drop_sincronizar);
        connect(group->lista,
                &QListWidget::itemDoubleClicked,
                this,
                [this](QListWidgetItem *item) { open_linked_file_for_item(item); });

        group->lista->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(group->lista,
                &QListWidget::customContextMenuRequested,
                this,
                [this, group](const QPoint &pos) { mostrar_menu_contexto(pos, group->lista); });

        if (auto *model = group->lista->model()) {
            connect(model, &QAbstractItemModel::rowsMoved, this, [this, group]() { _on_rows_moved(group->lista); });
        }

        layout->addWidget(group->lista, 1);
        group->installEventFilter(this);
        group->lista->installEventFilter(this);
        if (group->lista->viewport() != nullptr) {
            group->lista->viewport()->installEventFilter(this);
        }
        return group;
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao criar coluna '%1': %2").arg(status, QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao criar coluna '%1'").arg(status), true);
    }

    return new KanbanColumn(status, this);
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { KanbanColumn *criar_coluna(GerenciadorTarefas &self, const QString &status) { return self.criar_coluna(status); } }
