#include "source/modules/feynman/Feynman/fey_08_selecionar_conceito.hpp"

#include <QComboBox>
#include <QListWidgetItem>
#include <QTextEdit>

#include "source/modules/feynman/Feynman/feynman_metodo_helpers.hpp"

namespace source::plugin::Feynman {

void MetodoFeynman::selecionar_conceito(QListWidgetItem *item)
{
	const QVariantMap conceito = normalized_concept(obter_conceito_do_item(item));
	if (conceito.isEmpty()) {
		return;
	}

	conceito_atual_ = conceito;
	has_conceito_atual_ = true;
	sincronizando_descricao_ = true;
	input_titulo->setPlainText(conceito.value(QStringLiteral("titulo")).toString());
	texto_explicacao->setPlainText(conceito.value(QStringLiteral("explicacao")).toString());
	texto_lacunas->setPlainText(conceito.value(QStringLiteral("lacunas")).toString());
	texto_revisao->setPlainText(conceito.value(QStringLiteral("revisao")).toString());
	descricao_gerada_atual_ = composed_description(this,
												   conceito.value(QStringLiteral("titulo")).toString(),
												   conceito.value(QStringLiteral("explicacao")).toString(),
												   conceito.value(QStringLiteral("lacunas")).toString(),
												   conceito.value(QStringLiteral("revisao")).toString());
	if (texto_descricao != nullptr) {
		texto_descricao->setPlainText(concept_description_text(this, conceito));
	}
	sincronizando_descricao_ = false;
	descricao_editada_manualmente_ =
		texto_descricao != nullptr && texto_descricao->toPlainText().trimmed() != descricao_gerada_atual_.trimmed();
	{
		const int priority_index =
			combo_prioridade->findData(normalized_priority_value(conceito.value(QStringLiteral("prioridade"))));
		combo_prioridade->setCurrentIndex(priority_index >= 0 ? priority_index : 0);
	}
	{
		const int domain_index = combo_dominio->findData(normalized_domain_value(conceito.value(QStringLiteral("dominio"))));
		combo_dominio->setCurrentIndex(domain_index >= 0 ? domain_index : 0);
	}
}

}  // namespace source::plugin::Feynman

namespace source::plugin::Feynman::MetodoFeynmanFns {
void selecionar_conceito(MetodoFeynman &self, QListWidgetItem *item) { self.selecionar_conceito(item); }
}
