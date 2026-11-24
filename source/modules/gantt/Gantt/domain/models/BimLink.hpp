#pragma once

#include <QJsonObject>
#include <QString>
#include "GanttEnums.hpp"

namespace source::gantt {

class BimLink {
public:
    BimLink();
    BimLink(const QString &item_id, const QString &element_id, BimSource src = BimSource::IFC);

    QString id;
    QString schedule_item_id;
    BimSource bim_source = BimSource::IFC;
    QString bim_element_id; // IFC GUID (22 characters base64) or Revit UniqueId
    QString category;       // e.g. "IfcWall", "IfcBeam", "IfcSlab", "Parede", "Viga"
    QString level;          // e.g. "Pavimento 1", "Fundação", "Térreo"
    QString discipline;     // e.g. "Estrutura", "Arquitetura", "MEP"
    QJsonObject metadata;

    bool is_valid(QString *error_message = nullptr) const;

    QJsonObject to_json() const;
    static BimLink from_json(const QJsonObject &json);
};

} // namespace source::gantt
