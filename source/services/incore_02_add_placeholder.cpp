#include "source/services/incore_02_add_placeholder.hpp"

#include <QMetaType>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSignalBlocker>
#include <Qt>

namespace source::InterfaceCore::incore_02_add_placeholder {
namespace {

constexpr int kGroupHeaderRole = Qt::UserRole + 1;
constexpr int kLegacyPlaceholderRole = Qt::UserRole + 2;
constexpr int kPlaceholderRole = Qt::UserRole + 3;

bool list_has_selectable_items(QListWidget *list)
{
    if (list == nullptr) {
        return false;
    }

    for (int i = 0; i < list->count(); ++i) {
        if (QListWidgetItem *item = list->item(i); item != nullptr && (item->flags() & Qt::ItemIsSelectable)) {
            return true;
        }
    }

    return false;
}

bool is_legacy_placeholder_item(QListWidgetItem *item)
{
    if (item == nullptr) {
        return false;
    }

    if (item->data(kGroupHeaderRole).toString() == QStringLiteral("group_header")) {
        return false;
    }

    if (item->data(kPlaceholderRole).toBool()) {
        return true;
    }

    const QVariant legacy_placeholder = item->data(kLegacyPlaceholderRole);
    if (legacy_placeholder.metaType().id() == QMetaType::Bool && legacy_placeholder.toBool()) {
        return true;
    }

    return !(item->flags() & Qt::ItemIsSelectable) && !item->data(Qt::UserRole).isValid();
}

void remove_legacy_placeholder_items(QListWidget *list)
{
    if (list == nullptr) {
        return;
    }

    const QSignalBlocker blocker(list);
    for (int i = list->count() - 1; i >= 0; --i) {
        if (is_legacy_placeholder_item(list->item(i))) {
            delete list->takeItem(i);
        }
    }
}

void add_placeholder_item(QListWidget *list_widget, const QString &text)
{
    if (list_widget == nullptr) {
        return;
    }

    auto *placeholder = new QListWidgetItem(text);
    placeholder->setFlags((placeholder->flags() & ~Qt::ItemIsSelectable) & ~Qt::ItemIsEnabled);
    placeholder->setData(kPlaceholderRole, true);
    list_widget->addItem(placeholder);
}

void remove_overlay_if_present(QListWidget *list_widget)
{
    if (list_widget == nullptr || list_widget->viewport() == nullptr) {
        return;
    }

    const auto children = list_widget->viewport()->children();
    for (QObject *child : children) {
        if (child != nullptr && child->objectName() == QStringLiteral("_eisenhower_placeholder_overlay")) {
            delete child;
        }
    }
}

}  // namespace

void add_placeholder(source::EisenhowerMatrixApp *, QListWidget *list_widget, const QString &text)
{
    if (list_widget == nullptr) {
        return;
    }

    remove_legacy_placeholder_items(list_widget);
    remove_overlay_if_present(list_widget);

    if (!list_has_selectable_items(list_widget) && !text.trimmed().isEmpty()) {
        add_placeholder_item(list_widget, text);
    }
}

}  // namespace source::InterfaceCore::incore_02_add_placeholder
