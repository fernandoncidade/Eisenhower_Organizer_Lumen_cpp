#pragma once

#include <QLineEdit>
#include <QPointer>

namespace source::plugin::Pomodoro {

class FileDropLineEdit : public QLineEdit {
    Q_OBJECT

public:
    explicit FileDropLineEdit(QObject *app, QWidget *parent = nullptr);

    QString last_dropped_path() const;

signals:
    void fileDropped(const QString &path);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    QPointer<QObject> app_;
    QString last_dropped_path_;
};

}  // namespace source::plugin::Pomodoro
