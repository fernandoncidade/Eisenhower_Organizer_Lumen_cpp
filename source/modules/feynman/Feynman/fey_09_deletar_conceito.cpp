#include "source/modules/feynman/Feynman/fey_09_deletar_conceito.hpp"

#include <QListWidget>

namespace source::plugin::Feynman {

void MetodoFeynman::deletar_conceito()
{
	const QList<QListWidgetItem *> items = lista_conceitos->selectedItems();
	if (items.isEmpty()) {
		return;
	}

	QStringList titles;
	for (QListWidgetItem *item : items) {
		const QVariantMap conceito = obter_conceito_do_item(item);
		const QString title = conceito.value(QStringLiteral("titulo")).toString().trimmed();
		if (!title.isEmpty() && !titles.contains(title, Qt::CaseInsensitive)) {
			titles.append(title);
		}
	}

	if (titles.isEmpty()) {
		return;
	}

	if (!compat::MessageBox::feynman_confirm_delete_concepts(this, titles)) {
		return;
	}

	QVariantList deleted_concepts;
	for (const QVariant &value : conceitos_) {
		const QVariantMap conceito = value.toMap();
		if (titles.contains(conceito.value(QStringLiteral("titulo")).toString(), Qt::CaseInsensitive)) {
			deleted_concepts.append(conceito);
		}
	}

	if (!deleted_concepts.isEmpty() && compat::host_bridge() != nullptr) {
		compat::host_bridge()->show_sync_operation_info(QStringLiteral("delete"), QStringLiteral("feynman"));
		for (const QVariant &value : deleted_concepts) {
			compat::host_bridge()->publish_task_deleted(value.toMap(), QStringLiteral("feynman"));
		}
	}

	const int before = conceitos_.size();
	QVariantList remaining;
	remaining.reserve(conceitos_.size());
	for (const QVariant &value : conceitos_) {
		const QVariantMap conceito = value.toMap();
		if (!titles.contains(conceito.value(QStringLiteral("titulo")).toString(), Qt::CaseInsensitive)) {
			remaining.append(conceito);
		}
	}

	conceitos_ = remaining;
	if (before != conceitos_.size()) {
		has_conceito_atual_ = false;
		conceito_atual_.clear();
		salvar_conceitos();
		atualizar_lista();
		limpar_campos();
		compat::MessageBox::feynman_delete_success(this);
	} else {
		compat::MessageBox::feynman_delete_error(this);
	}
}

}  // namespace source::plugin::Feynman

namespace source::plugin::Feynman::MetodoFeynmanFns {
void deletar_conceito(MetodoFeynman &self) { self.deletar_conceito(); }
}
