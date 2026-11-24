#pragma once

#include <QObject>
#include <QString>

namespace source::gantt::language {

class GanttTranslationWorker : public QObject {
    Q_OBJECT

public:
    explicit GanttTranslationWorker(QObject *parent = nullptr);
    ~GanttTranslationWorker() override = default;

public slots:
    void prepare_translation(const QString &language_code);

signals:
    void translation_prepared(const QString &language_code, const QString &qm_file_path);

private:
    QString find_translation_file(const QString &language_code) const;
};

} // namespace source::gantt::language
