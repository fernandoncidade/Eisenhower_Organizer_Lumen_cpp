#pragma once

#include <QAbstractListModel>
#include <QVariantList>
#include <QVariantMap>

namespace source::models {

class TaskRecordListModel final : public QAbstractListModel {
    Q_OBJECT

public:
    enum Role {
        SearchableRole = Qt::UserRole + 1,
        RecordRole = Qt::UserRole + 2,
    };

    explicit TaskRecordListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void set_records(const QVariantList &records);
    QVariantMap record_at(int row) const;

private:
    QVariantList records_;
};

}  // namespace source::models
