#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_22_eventFilter.hpp"

#include <QCoreApplication>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMimeData>
#include <QUrl>

#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_21_atualizar_tema.hpp"
#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_39_processar_documento_ia.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_07_SmoothGraphicsView.hpp"

namespace source::plugin::MapaMental {
namespace {

QString primeiro_arquivo_local(const QMimeData *mime_data)
{
    if (mime_data == nullptr || !mime_data->hasUrls()) {
        return QString();
    }

    const QList<QUrl> urls = mime_data->urls();
    for (const QUrl &url : urls) {
        const QString caminho = url.toLocalFile().trimmed();
        if (!caminho.isEmpty() && QFileInfo(caminho).isFile()) {
            return caminho;
        }
    }

    return QString();
}

}  // namespace

void MapaMental::changeEvent(QEvent *event)
{
    if (event != nullptr && event->type() == QEvent::LanguageChange) {
        atualizar_traducoes();
    }

    QWidget::changeEvent(event);
}

bool MapaMental::eventFilter(QObject *obj, QEvent *event)
{
    if (event != nullptr) {
        const QEvent::Type type = event->type();
        if (((type == QEvent::ApplicationPaletteChange || type == QEvent::ThemeChange)
             && (obj == QCoreApplication::instance() || obj == this))
            || ((type == QEvent::PaletteChange || type == QEvent::StyleChange) && obj == this)) {
            atualizar_tema();
        }
    }

    const bool alvo_area_mapa = view_ != nullptr && (obj == view_ || obj == view_->viewport());
    if (alvo_area_mapa && event != nullptr) {
        if (event->type() == QEvent::DragEnter) {
            auto *drag = static_cast<QDragEnterEvent *>(event);
            if (!primeiro_arquivo_local(drag->mimeData()).isEmpty()) {
                drag->acceptProposedAction();
                return true;
            }
        } else if (event->type() == QEvent::DragMove) {
            auto *drag = static_cast<QDragMoveEvent *>(event);
            if (!primeiro_arquivo_local(drag->mimeData()).isEmpty()) {
                drag->acceptProposedAction();
                return true;
            }
        } else if (event->type() == QEvent::Drop) {
            auto *drop = static_cast<QDropEvent *>(event);
            const QString caminho = primeiro_arquivo_local(drop->mimeData());
            if (!caminho.isEmpty()) {
                drop->acceptProposedAction();

                processar_documento_com_ia(caminho, QCoreApplication::translate("MapaMentalPlugin", "Importar documento"));
                return true;
            }
        } else if (event->type() == QEvent::KeyPress) {
            auto *key_event = static_cast<QKeyEvent *>(event);
            if (key_event->key() == Qt::Key_Delete || key_event->key() == Qt::Key_Backspace) {
                const QList<NoConceito *> selecionados = selected_nodes();
                if (!selecionados.isEmpty()) {
                    remover_nos(selecionados);
                    key_event->accept();
                    return true;
                }
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}

}  // namespace source::plugin::MapaMental
