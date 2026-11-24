#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_23_eventFilter.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

bool GerenciadorTarefas::eventFilter(QObject *obj, QEvent *event)
{
    try {
        if (event != nullptr) {
            switch (event->type()) {
            case QEvent::MouseButtonPress: {
                for (KanbanColumn *col : {col_todo, col_doing, col_done}) {
                    if (col != nullptr && col->lista != nullptr) {
                        if (obj == col || obj == col->lista || obj == col->lista->viewport()) {
                            auto *me = static_cast<QMouseEvent *>(event);
                            if (obj == col->lista->viewport()) {
                                QListWidgetItem *item = col->lista->itemAt(me->pos());
                                if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable)) {
                                    limpar_selecao_tarefas();
                                    emit listas_atualizadas();
                                }
                            } else if (obj == col || obj == col->lista) {
                                limpar_selecao_tarefas();
                                emit listas_atualizadas();
                            }
                            break;
                        }
                    }
                }
                break;
            }
            case QEvent::ApplicationPaletteChange:
            case QEvent::PaletteChange:
            case QEvent::StyleChange:
            case QEvent::ThemeChange:
                _aplicar_tema_dinamico_inputs();
                break;
            case QEvent::LanguageChange:
                atualizar_traducoes();
                break;
            case QEvent::Resize:
                if (obj == this || obj == parentWidget() || obj == window() || obj == status_tabs) {
                    ajustar_linha_botoes();
                }
                break;
            default:
                break;
            }
        }
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro no eventFilter do GerenciadorTarefas: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro no eventFilter do GerenciadorTarefas"), true);
    }

    return QWidget::eventFilter(obj, event);
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { bool eventFilter(GerenciadorTarefas &self, QObject *obj, QEvent *event) { return self.eventFilter(obj, event); } }
