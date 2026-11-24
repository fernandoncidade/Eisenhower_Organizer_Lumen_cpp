#pragma once

#include <QObject>
#include <QVariantList>

namespace source::plugin::Feynman {

class FeynmanWorker : public QObject {
    Q_OBJECT

public:
    explicit FeynmanWorker(QObject *parent = nullptr);

    static QVariantList load_file(const QString &arquivo_conceitos);
    static bool save_file(const QString &arquivo_conceitos, const QVariantList &conceitos);

signals:
    void request_load(const QString &arquivo_conceitos);
    void request_save(const QString &arquivo_conceitos, const QVariantList &conceitos);
    void conceitos_loaded(const QVariantList &conceitos);
    void save_finished(bool ok);
    void error(const QString &message);

private slots:
    void _load(const QString &arquivo_conceitos);
    void _save(const QString &arquivo_conceitos, const QVariantList &conceitos);
};

}  // namespace source::plugin::Feynman
