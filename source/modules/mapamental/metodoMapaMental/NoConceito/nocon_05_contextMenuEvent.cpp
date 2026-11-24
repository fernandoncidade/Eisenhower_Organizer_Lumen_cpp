#include "source/modules/mapamental/metodoMapaMental/NoConceito/nocon_05_contextMenuEvent.hpp"

#include <QCoreApplication>
#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsView>
#include <QList>
#include <QListWidgetItem>
#include <QMenu>
#include <QTimer>
#include <QtGlobal>

#include "source/modules/mapamental/metodoMapaMental/mp_03_MapaMental.hpp"
#include "source/utils/LinkedTitleUnderlineDelegate.hpp"

namespace source::plugin::MapaMental {

void NoConceito::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    const QList<NoConceito *> selecionados = owner_ != nullptr ? owner_->selected_nodes() : QList<NoConceito *>();
    const bool remover_selecao = isSelected() && selecionados.size() > 1;

    QMenu menu;
    QAction *editar = menu.addAction(QCoreApplication::translate("MapaMentalPlugin", "✏️ Editar Conceito"));
    QAction *fonte = menu.addAction(QCoreApplication::translate("MapaMentalPlugin", "🔤 Configurações de Fonte"));
    QAction *colorir = menu.addAction(QCoreApplication::translate("MapaMentalPlugin", "🎨 Colorir tarefas"));
    menu.addSeparator();
    QAction *remover = menu.addAction(remover_selecao
                                          ? QCoreApplication::translate("MapaMentalPlugin", "🗑️ Excluir Nós Selecionados")
                                          : QCoreApplication::translate("MapaMentalPlugin", "🗑️ Excluir Nó"));

    QAction *chosen = menu.exec(event != nullptr ? event->screenPos() : QCursor::pos());
    if (chosen == nullptr) {
        return;
    }

    if (chosen == editar) {
        if (!abrir_editor_conceito()) {
            return;
        }
    } else if (chosen == fonte) {
        const QList<NoConceito *> nos_alvo = (isSelected() && !selecionados.isEmpty()) ? selecionados : QList<NoConceito *>{this};
        const bool has_linked = !nos_alvo.isEmpty() && nos_alvo.first() != nullptr && !nos_alvo.first()->arquivo_vinculado().trimmed().isEmpty();
        const auto old_global = has_linked ? source::utils::load_plugin_linked_file_title_font_settings(source::utils::PluginFontContext::MapaMental)
                                           : source::utils::load_plugin_general_task_title_font_settings(source::utils::PluginFontContext::MapaMental);
        QList<QListWidgetItem *> temp_items;
        for (NoConceito *no : nos_alvo) {
            auto *item = new QListWidgetItem(no->texto());
            item->setData(source::utils::kCustomFontSettingsRole, no->font_config());
            if (!no->arquivo_vinculado().trimmed().isEmpty()) {
                item->setData(source::utils::kHasLinkedFileRole, true);
            }
            temp_items.append(item);
        }
        const bool ok = source::utils::open_task_font_settings_dialog(owner_, temp_items, source::utils::PluginFontContext::MapaMental);
        if (ok) {
            if (scene() != nullptr) {
                for (QGraphicsItem *s_item : scene()->items()) {
                    NoConceito *other_no = dynamic_cast<NoConceito *>(s_item);
                    if (other_no != nullptr && !nos_alvo.contains(other_no) && other_no->font_config().isEmpty()) {
                        const bool other_linked = !other_no->arquivo_vinculado().trimmed().isEmpty();
                        if (other_linked == has_linked) {
                            other_no->set_font_config(source::utils::task_title_font_settings_to_map(old_global));
                        }
                    }
                }
            }
            for (int i = 0; i < nos_alvo.size() && i < temp_items.size(); ++i) {
                if (nos_alvo[i] != nullptr && temp_items[i] != nullptr) {
                    nos_alvo[i]->set_font_config(temp_items[i]->data(source::utils::kCustomFontSettingsRole).toMap());
                }
            }
        }
        qDeleteAll(temp_items);
        temp_items.clear();
        if (!ok) {
            return;
        }
    } else if (chosen == colorir) {
        const QList<NoConceito *> nos_alvo = (isSelected() && !selecionados.isEmpty()) ? selecionados : QList<NoConceito *>{this};
        QColor initial_color;
        for (NoConceito *no : nos_alvo) {
            if (no != nullptr && no->cor().isValid()) {
                initial_color = no->cor();
                break;
            }
        }
        const QColor selected_color = source::utils::choose_task_color(owner_, initial_color, source::utils::PluginFontContext::MapaMental);
        if (!selected_color.isValid()) {
            return;
        }
        for (NoConceito *no : nos_alvo) {
            if (no != nullptr) {
                no->set_cor(selected_color);
            }
        }
    } else if (chosen == remover && owner_ != nullptr) {
        MapaMental *owner = owner_;
        NoConceito *no = this;
        const QList<NoConceito *> nos_para_remover = remover_selecao ? selecionados : QList<NoConceito *>{no};
        QTimer::singleShot(0, owner, [owner, nos_para_remover]() {
            owner->remover_nos(nos_para_remover);
        });
        return;
    }

    if (owner_ != nullptr) {
        owner_->salvar_backup_automatico();
        owner_->refresh_counter();
    }
}

}  // namespace source::plugin::MapaMental
