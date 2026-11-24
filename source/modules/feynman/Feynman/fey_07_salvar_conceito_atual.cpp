#include "source/modules/feynman/Feynman/fey_07_salvar_conceito_atual.hpp"

#include <QComboBox>
#include <QTextEdit>

#include "source/modules/feynman/Feynman/feynman_metodo_helpers.hpp"

namespace source::plugin::Feynman {

void MetodoFeynman::salvar_conceito_atual()
{
	const QString titulo = input_titulo->toPlainText().trimmed();
	if (titulo.isEmpty()) {
		compat::MessageBox::feynman_warn_missing_title(this);
		return;
	}

	QString arquivo_vinculado;
	QString cor_texto;
	if (has_conceito_atual_) {
		arquivo_vinculado = conceito_atual_.value(QStringLiteral("arquivo_vinculado")).toString().trimmed();
		cor_texto = conceito_atual_.value(QStringLiteral("cor_texto")).toString().trimmed();
	}

	const QString explicacao = texto_explicacao->toPlainText();
	const QString lacunas = texto_lacunas->toPlainText();
	const QString revisao = texto_revisao->toPlainText();
	const QString descricao_editada = texto_descricao != nullptr ? texto_descricao->toPlainText().trimmed() : QString();
	const QString descricao_composta = composed_description(this, titulo, explicacao, lacunas, revisao).trimmed();
	QString descricao_final;
	if (has_conceito_atual_ && has_external_description_base(conceito_atual_)) {
		QString descricao_base = manual_description_text(descricao_editada, descricao_gerada_atual_);
		if (descricao_base.trimmed().isEmpty()) {
			descricao_base = conceito_atual_.value(QStringLiteral("descricao")).toString();
		}
		descricao_final = description_with_external_base(descricao_base, descricao_composta).trimmed();
	} else {
		descricao_final = merge_description_edit(descricao_gerada_atual_, descricao_editada, descricao_composta).trimmed();
	}
	const bool descricao_customizada = descricao_final != descricao_composta;

	QVariantMap conceito = {
		{QStringLiteral("titulo"), titulo},
		{QStringLiteral("explicacao"), explicacao},
		{QStringLiteral("lacunas"), lacunas},
		{QStringLiteral("revisao"), revisao},
		{QStringLiteral("prioridade"), normalized_priority_value(combo_prioridade->currentData())},
		{QStringLiteral("dominio"), normalized_domain_value(combo_dominio->currentData())},
		{QStringLiteral("arquivo_vinculado"), arquivo_vinculado.isEmpty() ? QVariant() : QVariant(arquivo_vinculado)},
		{QStringLiteral("cor_texto"), cor_texto.isEmpty() ? QVariant() : QVariant(cor_texto)},
		{QStringLiteral("descricao_customizada"), descricao_customizada},
	};

	if (descricao_customizada) {
		conceito.insert(QStringLiteral("descricao"), descricao_final);
	}

	for (const QString &key : integration_metadata_keys()) {
		if (conceito_atual_.contains(key)) {
			conceito.insert(key, conceito_atual_.value(key));
		}
	}

	conceito = normalized_concept(conceito);

	const int existing_index = encontrar_indice_conceito(titulo);
	if (existing_index >= 0) {
		QVariantMap existente = conceitos_.at(existing_index).toMap();
		for (const QString &key : integration_metadata_keys()) {
			if (existente.contains(key)) {
				conceito.insert(key, existente.value(key));
			}
		}
		if (conceito.value(QStringLiteral("arquivo_vinculado")).toString().trimmed().isEmpty()) {
			const QString preserved = existente.value(QStringLiteral("arquivo_vinculado")).toString().trimmed();
			conceito.insert(QStringLiteral("arquivo_vinculado"), preserved.isEmpty() ? QVariant() : QVariant(preserved));
		}
		if (conceito.value(QStringLiteral("cor_texto")).toString().trimmed().isEmpty()) {
			const QString preserved = existente.value(QStringLiteral("cor_texto")).toString().trimmed();
			conceito.insert(QStringLiteral("cor_texto"), preserved.isEmpty() ? QVariant() : QVariant(preserved));
		}
		conceitos_[existing_index] = conceito;
	} else {
		if (compat::host_bridge() != nullptr) {
			compat::host_bridge()->show_sync_operation_info(QStringLiteral("add"), QStringLiteral("feynman"));
		}
		conceitos_.append(conceito);
	}

	conceito_atual_ = conceito;
	has_conceito_atual_ = true;
	salvar_conceitos();
	atualizar_lista();
	sincronizando_descricao_ = true;
	if (texto_descricao != nullptr) {
		texto_descricao->setPlainText(descricao_final);
	}
	descricao_gerada_atual_ = descricao_composta;
	sincronizando_descricao_ = false;
	descricao_editada_manualmente_ = descricao_customizada;
	compat::MessageBox::feynman_save_success(this, titulo);
}

}  // namespace source::plugin::Feynman

namespace source::plugin::Feynman::MetodoFeynmanFns {
void salvar_conceito_atual(MetodoFeynman &self) { self.salvar_conceito_atual(); }
}
