#include "source/modules/feynman/Feynman/fey_10_atualizar_lista.hpp"

#include <algorithm>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QListWidget>
#include <QListWidgetItem>

#include "source/modules/feynman/Feynman/feynman_metodo_helpers.hpp"
#include "source/modules/feynman/Feynman/feynman_support.hpp"
#include "source/utils/LinkedTitleUnderlineDelegate.hpp"

namespace source::plugin::Feynman {

void MetodoFeynman::atualizar_lista()
{
	atualizar_ciclos_em_progresso();

	QStringList selected_titles;
	const QList<QListWidgetItem *> selected_items = lista_conceitos->selectedItems();
	for (QListWidgetItem *item : selected_items) {
		const QVariantMap conceito = obter_conceito_do_item(item);
		const QString title = conceito.value(QStringLiteral("titulo")).toString().trimmed();
		if (!title.isEmpty()) {
			selected_titles.append(title);
		}
	}

	const QString current_title =
		has_conceito_atual_ ? conceito_atual_.value(QStringLiteral("titulo")).toString().trimmed() : QString();
	lista_conceitos->clear();

	QVariantList ordered_concepts = conceitos_;
	if (has_host_task_sort()) {
		std::stable_sort(ordered_concepts.begin(), ordered_concepts.end(), [](const QVariant &left, const QVariant &right) {
			return host_task_sort_less(left.toMap(), right.toMap());
		});
	}

	int current_row = -1;
	QString last_group_key;

	for (int i = 0; i < ordered_concepts.size(); ++i) {
		const QVariantMap conceito = normalized_concept(ordered_concepts.at(i).toMap());
		const QString title = conceito.value(QStringLiteral("titulo")).toString().trimmed();
		const QString arquivo = conceito.value(QStringLiteral("arquivo_vinculado")).toString().trimmed();
		const QString title_line = visible_concept_title(
			this,
			conceito,
			QStringLiteral("%1 %2 %3")
				.arg(priority_emoji(conceito.value(QStringLiteral("prioridade")).toInt()),
					 domain_symbol(conceito.value(QStringLiteral("dominio")).toInt()),
					 title));
		const QString group_key =
			(compat::host_bridge() != nullptr && compat::host_bridge()->watermark_enabled()) ? concept_group_key(conceito)
																							  : QString();
		if (!group_key.isEmpty() && group_key != last_group_key) {
			auto *header_item = new QListWidgetItem(watermark_concept_text(this, conceito));
			header_item->setFlags((header_item->flags() & ~Qt::ItemIsSelectable) & ~Qt::ItemIsEnabled);
			header_item->setData(kGroupHeaderRole, QStringLiteral("group_header"));
			header_item->setData(kGroupKeyRole, group_key);
			lista_conceitos->addItem(header_item);
			last_group_key = group_key;
		} else if (group_key.isEmpty()) {
			last_group_key.clear();
		}

		auto *item = new QListWidgetItem(title_line);
		item->setData(kConceptRole, conceito);
		item->setData(kPreviewRole, preview_payload(this, conceito));
		item->setData(source::utils::kCustomFontSettingsRole, conceito.value(QStringLiteral("font_config")));
		source::utils::apply_task_title_font(item, title, !arquivo.isEmpty(), source::utils::PluginFontContext::Feynman);

		if (!arquivo.isEmpty()) {
			if (QColor color(conceito.value(QStringLiteral("cor_texto")).toString().trimmed()); color.isValid()) {
				item->setForeground(QBrush(color));
			} else {
				item->setForeground(QBrush());
			}
		} else if (QColor color(conceito.value(QStringLiteral("cor_texto")).toString().trimmed()); color.isValid()) {
			item->setForeground(QBrush(color));
		}

		const QString tooltip = tooltip_html(this, conceito);
		if (!tooltip.isEmpty()) {
			item->setToolTip(tooltip);
		}

		lista_conceitos->addItem(item);
		if (selected_titles.contains(title, Qt::CaseInsensitive)) {
			item->setSelected(true);
		}
		if (!current_title.isEmpty() && title.compare(current_title, Qt::CaseInsensitive) == 0) {
			current_row = lista_conceitos->count() - 1;
		}
	}

	if (current_row >= 0 && current_row < lista_conceitos->count()) {
		lista_conceitos->setCurrentRow(current_row);
	}

	emit task_counter_changed();
}

int MetodoFeynman::total_task_count() const
{
	return conceitos_.size();
}

int MetodoFeynman::selected_task_count() const
{
	if (lista_conceitos == nullptr) {
		return 0;
	}

	int count = 0;
	for (QListWidgetItem *item : lista_conceitos->selectedItems()) {
		if (item != nullptr && (item->flags() & Qt::ItemIsSelectable)) {
			++count;
		}
	}
	return count;
}

QString MetodoFeynman::selected_task_color() const
{
	if (lista_conceitos == nullptr) {
		return {};
	}

	for (QListWidgetItem *item : lista_conceitos->selectedItems()) {
		const QVariantMap conceito = obter_conceito_do_item(item);
		const QString color = conceito.value(QStringLiteral("cor_texto")).toString().trimmed();
		if (QColor(color).isValid()) {
			return color;
		}
	}
	return {};
}

int MetodoFeynman::apply_color_to_selected_tasks(const QString &color)
{
	const QColor chosen(color.trimmed());
	if (lista_conceitos == nullptr || !chosen.isValid()) {
		return 0;
	}

	const QString normalized_color = chosen.name(QColor::HexRgb);
	int updated = 0;
	for (QListWidgetItem *item : lista_conceitos->selectedItems()) {
		const int index = obter_indice_conceito_do_item(item);
		if (index < 0 || index >= conceitos_.size()) {
			continue;
		}

		QVariantMap conceito = conceitos_.at(index).toMap();
		conceito.insert(QStringLiteral("cor_texto"), normalized_color);
		conceitos_[index] = conceito;
		const QString current_title = conceito_atual_.value(QStringLiteral("titulo")).toString();
		if (has_conceito_atual_
			&& current_title.compare(conceito.value(QStringLiteral("titulo")).toString(), Qt::CaseSensitive) == 0) {
			conceito_atual_.insert(QStringLiteral("cor_texto"), normalized_color);
		}
		++updated;
	}

	if (updated > 0) {
		salvar_conceitos();
		atualizar_lista();
		emit task_counter_changed();
	}
	return updated;
}

QVariantMap MetodoFeynman::obter_conceito_do_item(QListWidgetItem *item) const
{
	if (item == nullptr || is_group_header(item) || !(item->flags() & Qt::ItemIsSelectable)) {
		return {};
	}

	const QVariantMap data = item->data(kConceptRole).toMap();
	if (!data.isEmpty()) {
		return normalized_concept(data);
	}

	const QString title = compat::normalized_title(item->text());
	for (const QVariant &value : conceitos_) {
		const QVariantMap conceito = normalized_concept(value.toMap());
		if (compat::normalized_title(conceito.value(QStringLiteral("titulo")).toString()) == title) {
			return conceito;
		}
	}

	return {};
}

int MetodoFeynman::obter_indice_conceito_do_item(QListWidgetItem *item) const
{
	const QVariantMap conceito = obter_conceito_do_item(item);
	return encontrar_indice_conceito(conceito.value(QStringLiteral("titulo")).toString());
}

int MetodoFeynman::encontrar_indice_conceito(const QString &titulo, Qt::CaseSensitivity sensitivity) const
{
	for (int i = 0; i < conceitos_.size(); ++i) {
		const QString current_title = conceitos_.at(i).toMap().value(QStringLiteral("titulo")).toString();
		if (current_title.compare(titulo, sensitivity) == 0) {
			return i;
		}
	}

	return -1;
}

int MetodoFeynman::encontrar_indice_conceito_integrado(const QString &origem, const QString &task_id) const
{
	const QString normalized_origin = origem.trimmed().toCaseFolded();
	const QString normalized_id = task_id.trimmed().toCaseFolded();
	if (normalized_origin.isEmpty() || normalized_id.isEmpty()) {
		return -1;
	}

	for (int i = 0; i < conceitos_.size(); ++i) {
		const QVariantMap conceito = conceitos_.at(i).toMap();
		const QString current_origin = conceito.value(QStringLiteral("integracao_origem")).toString().trimmed().toCaseFolded();
		const QString current_id = conceito.value(QStringLiteral("integracao_task_id")).toString().trimmed().toCaseFolded();
		if (current_origin == normalized_origin && current_id == normalized_id) {
			return i;
		}
	}

	return -1;
}

void MetodoFeynman::atualizar_ciclos_em_progresso()
{
	const int default_ciclos = compat::host_bridge() != nullptr
	                               ? compat::host_bridge()->default_pomodoro_cycle_count()
	                               : 4;
	if (default_ciclos <= 0) {
		return;
	}

	bool changed = false;
	for (int i = 0; i < conceitos_.size(); ++i) {
		QVariantMap c = conceitos_.at(i).toMap();
		if (is_concept_in_progress(c)) {
			const bool custom = c.value(QStringLiteral("ciclos_customizados"), false).toBool();
			const int current_c_tot = c.value(QStringLiteral("ciclos_totais"), 0).toInt();
			if (!custom && current_c_tot != default_ciclos) {
				c.insert(QStringLiteral("ciclos_totais"), default_ciclos);
				c.insert(QStringLiteral("ciclos_permitidos"), default_ciclos);
				c.insert(QStringLiteral("total_ciclos"), default_ciclos);
				conceitos_[i] = c;
				changed = true;
			}
		}
	}

	if (changed) {
		salvar_conceitos();
	}
}

void MetodoFeynman::reset_in_progress_task_cycles(int new_cycles, const QVariantList &affected_tasks)
{
	if (new_cycles <= 0) {
		return;
	}

	bool changed = false;
	for (int i = 0; i < conceitos_.size(); ++i) {
		QVariantMap c = conceitos_.at(i).toMap();
		if (!is_concept_in_progress(c)) {
			continue;
		}

		const QString cid = c.value(QStringLiteral("id")).toString().trimmed();
		const QString cinteg = c.value(QStringLiteral("integracao_task_id")).toString().trimmed();
		const QString ctitle = c.value(QStringLiteral("titulo")).toString().trimmed();

		bool match = false;
		for (const QVariant &v : affected_tasks) {
			const QVariantMap task = v.toMap();
			const QString tid = task.value(QStringLiteral("id")).toString().trimmed();
			const QString tinteg = task.value(QStringLiteral("integracao_task_id")).toString().trimmed();
			const QString ttitle = task.value(QStringLiteral("titulo")).toString().trimmed();

			if (!tid.isEmpty() && (cid == tid || (!cinteg.isEmpty() && cinteg == tid))) {
				match = true;
				break;
			}
			if (!tinteg.isEmpty() && (cid == tinteg || (!cinteg.isEmpty() && cinteg == tinteg))) {
				match = true;
				break;
			}
			if (!ctitle.isEmpty() && !ttitle.isEmpty() && ctitle.compare(ttitle, Qt::CaseInsensitive) == 0) {
				match = true;
				break;
			}
		}

		if (match) {
			c.insert(QStringLiteral("ciclos_totais"), new_cycles);
			c.insert(QStringLiteral("ciclos_permitidos"), new_cycles);
			c.insert(QStringLiteral("total_ciclos"), new_cycles);
			c.insert(QStringLiteral("ciclos_customizados"), false);
			conceitos_[i] = c;
			changed = true;
		}
	}

	if (changed) {
		salvar_conceitos();
		atualizar_lista();
	}
}

}  // namespace source::plugin::Feynman

namespace source::plugin::Feynman::MetodoFeynmanFns {
void atualizar_lista(MetodoFeynman &self) { self.atualizar_lista(); }
}
