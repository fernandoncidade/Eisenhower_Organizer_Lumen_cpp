#pragma once

#include <QTextEdit>

namespace source::plugin::Audiolivro {

class FileDropEdit : public QTextEdit
{
    Q_OBJECT

public:
    explicit FileDropEdit(QWidget *parent = nullptr);

signals:
    void files_dropped(const QStringList &paths);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

}  // namespace source::plugin::Audiolivro
