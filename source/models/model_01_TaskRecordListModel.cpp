#include "source/models/model_01_TaskRecordListModel.hpp"

namespace source::models {

TaskRecordListModel::TaskRecordListModel(QObject *parent) : QAbstractListModel(parent) {}

int TaskRecordListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : records_.size();
}

QVariant TaskRecordListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= records_.size()) {
        return {};
    }

    const QVariantMap record = records_.at(index.row()).toMap();
    if (role == Qt::DisplayRole) {
        return record.value(QStringLiteral("display")).toString();
    }
    if (role == SearchableRole) {
        return record.value(QStringLiteral("searchable")).toString();
    }
    if (role == RecordRole) {
        return record;
    }
    return {};
}

QHash<int, QByteArray> TaskRecordListModel::roleNames() const
{
    auto roles = QAbstractListModel::roleNames();
    roles.insert(SearchableRole, "searchable");
    roles.insert(RecordRole, "record");
    return roles;
}

void TaskRecordListModel::set_records(const QVariantList &records)
{
    beginResetModel();
    records_ = records;
    endResetModel();
}

QVariantMap TaskRecordListModel::record_at(int row) const
{
    if (row < 0 || row >= records_.size()) {
        return {};
    }
    return records_.at(row).toMap();
}

}  // namespace source::models
