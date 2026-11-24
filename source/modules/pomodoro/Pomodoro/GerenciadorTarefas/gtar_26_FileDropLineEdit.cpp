#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_26_FileDropLineEdit.hpp"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QMimeData>

#include "source/utils/LogManager.hpp"

namespace source::plugin::Pomodoro {
namespace {

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

}  // namespace

FileDropLineEdit::FileDropLineEdit(QObject *app, QWidget *parent) : QLineEdit(parent), app_(app)
{
    setAcceptDrops(true);
}

QString FileDropLineEdit::last_dropped_path() const
{
    return last_dropped_path_;
}

void FileDropLineEdit::dragEnterEvent(QDragEnterEvent *event)
{
    if (event != nullptr && event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        return;
    }

    QLineEdit::dragEnterEvent(event);
}

void FileDropLineEdit::dragMoveEvent(QDragMoveEvent *event)
{
    if (event != nullptr && event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        return;
    }

    QLineEdit::dragMoveEvent(event);
}

void FileDropLineEdit::dropEvent(QDropEvent *event)
{
    try {
        if (event != nullptr && event->mimeData()->hasUrls()) {
            const auto urls = event->mimeData()->urls();
            if (!urls.isEmpty()) {
                const QString path = urls.constFirst().toLocalFile();
                if (!path.isEmpty()) {
                    last_dropped_path_ = path;
                    setText(QFileInfo(path).fileName());
                    setToolTip(path);
                    emit fileDropped(path);
                }
            }

            event->acceptProposedAction();
            return;
        }
    } catch (const std::exception &e) {
        logger()->error(QStringLiteral("Erro ao processar drop no campo de tarefa: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao processar drop no campo de tarefa"), true);
    }

    QLineEdit::dropEvent(event);
}

}  // namespace source::plugin::Pomodoro
