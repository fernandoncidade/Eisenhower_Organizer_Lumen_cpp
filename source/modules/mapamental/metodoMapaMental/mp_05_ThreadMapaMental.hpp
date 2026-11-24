#pragma once

#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

namespace source::plugin::MapaMental {

class MapasWorker final : public QObject {
    Q_OBJECT

public:
    explicit MapasWorker(QObject *parent = nullptr);

public slots:
    void process_reorganize(const QStringList &textos, const QVariantMap &conexoes_existentes, const QVariantList &visiveis);

signals:
    void reorganize_finished(QVariantList relacoes, QVariantMap conexoes_existentes, QVariantList visiveis);
};

}  // namespace source::plugin::MapaMental
