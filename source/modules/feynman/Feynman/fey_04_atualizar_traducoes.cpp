#include "source/modules/feynman/Feynman/fey_04_atualizar_traducoes.hpp"

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QTabWidget>

#include "source/modules/feynman/Feynman/fey_19_RotatedTabButton.hpp"
#include "source/modules/feynman/Feynman/feynman_metodo_helpers.hpp"

namespace source::plugin::Feynman {

void MetodoFeynman::atualizar_traducoes()
{
	const QString tarefas_frame = compat::tr(this, QStringLiteral("📚 Minhas Tarefas:"), QStringLiteral("📚 My Tasks:"));
	if (label_meus_conceitos != nullptr) {
		label_meus_conceitos->setText(tarefas_frame);
	}
	if (frame_meus_conceitos != nullptr) {
		frame_meus_conceitos->setTitle(QStringLiteral(" ") + tarefas_frame + QStringLiteral(" "));
	}
	gerenciador_botoes_.set_button_text(btn_novo, compat::tr(this, QStringLiteral("➕ Novo"), QStringLiteral("➕ New")));
	gerenciador_botoes_.set_button_text(btn_deletar, compat::tr(this, QStringLiteral("🗑️ Excluir"), QStringLiteral("🗑️ Delete")));

	const QString conceito_frame =
		compat::tr(this, QStringLiteral("1️⃣ Conceito"), QStringLiteral("1️⃣ Concept"));
	const QString explique_frame =
		compat::tr(this, QStringLiteral("2️⃣ Explique com Simplicidade"), QStringLiteral("2️⃣ Explain Simply"));
	const QString lacunas_frame =
		compat::tr(this, QStringLiteral("3️⃣ Lacunas Identificadas"), QStringLiteral("3️⃣ Identified Gaps"));
	const QString revisao_frame =
		compat::tr(this,
				   QStringLiteral("4️⃣ Revisão e Simplificação Final"),
				   QStringLiteral("4️⃣ Final Review and Simplification"));
	const QString descricao_frame =
		compat::tr(this, QStringLiteral("5️⃣ Descrição"), QStringLiteral("5️⃣ Description"));

	const QString conceito_placeholder =
		compat::tr(this,
				   QStringLiteral("Conceito (ponto de vista sintetizado): Exemplo: Tensão de Cisalhamento."),
				   QStringLiteral("Concept (synthesized point of view): Example: Shear Stress."));
	const QString explique_placeholder =
		compat::tr(this,
				   QStringLiteral("Explique com Simplicidade (como para uma criança): Exemplo: Escreva sua explicação usando linguagem simples e exemplos do dia a dia."),
				   QStringLiteral("Explain Simply (as if to a child): Example: Write your explanation using simple language and everyday examples."));
	const QString lacunas_placeholder =
		compat::tr(this,
				   QStringLiteral("Lacunas Identificadas (o que você não conseguiu explicar bem?): Exemplo: Liste os pontos onde você travou ou ficou confuso."),
				   QStringLiteral("Identified Gaps (what could you not explain clearly?): Example: List the points where you got stuck or felt confused."));
	const QString revisao_placeholder =
		compat::tr(this,
				   QStringLiteral("Revisão e Simplificação Final (depois de estudar as lacunas): Exemplo: Reescreva sua explicação de forma ainda mais clara e completa."),
				   QStringLiteral("Final Review and Simplification (after studying the gaps): Example: Rewrite your explanation in an even clearer and more complete way."));
	const QString descricao_placeholder =
		compat::tr(this,
				   QStringLiteral("Descrição (compatível com Pomodoro e Eisenhower): Exemplo: A descrição reúne conceito, explicação, lacunas e revisão em um único texto compatível com o campo Descrição."),
				   QStringLiteral("Description (compatible with Pomodoro and Eisenhower): Example: The description joins concept, explanation, gaps, and review into a single text compatible with the Description field."));

	const auto format_frame_title = [](const QString &text) {
		return QStringLiteral(" ") + text + QStringLiteral(" ");
	};

	if (frame_conceito != nullptr) {
		frame_conceito->setTitle(format_frame_title(conceito_frame));
	}
	if (frame_explique != nullptr) {
		frame_explique->setTitle(format_frame_title(explique_frame));
	}
	if (frame_lacunas != nullptr) {
		frame_lacunas->setTitle(format_frame_title(lacunas_frame));
	}
	if (frame_revisao != nullptr) {
		frame_revisao->setTitle(format_frame_title(revisao_frame));
	}
	if (frame_descricao != nullptr) {
		frame_descricao->setTitle(format_frame_title(descricao_frame));
	}

	set_placeholder_base_text(input_titulo, conceito_placeholder);
	set_placeholder_base_text(texto_explicacao, explique_placeholder);
	set_placeholder_base_text(texto_lacunas, lacunas_placeholder);
	set_placeholder_base_text(texto_revisao, revisao_placeholder);
	set_placeholder_base_text(texto_descricao, descricao_placeholder);
	atualizar_placeholders_dinamicos();

	label_prioridade->setText(compat::tr(this, QStringLiteral("Prioridade:"), QStringLiteral("Priority:")));
	label_nivel->setText(compat::tr(this, QStringLiteral("Nível de Domínio:"), QStringLiteral("Mastery Level:")));

	const int current_priority = normalized_priority_value(combo_prioridade->currentData());
	combo_prioridade->clear();
	combo_prioridade->addItem(priority_label(this, 1), 1);
	combo_prioridade->addItem(priority_label(this, 2), 2);
	combo_prioridade->addItem(priority_label(this, 3), 3);
	combo_prioridade->addItem(priority_label(this, 4), 4);
	const int priority_index = combo_prioridade->findData(current_priority);
	combo_prioridade->setCurrentIndex(priority_index >= 0 ? priority_index : 0);

	const int current_domain = normalized_domain_value(combo_dominio->currentData());
	combo_dominio->clear();
	combo_dominio->addItem(domain_label(this, 0, false), 0);
	combo_dominio->addItem(domain_label(this, 1, false), 1);
	combo_dominio->addItem(domain_label(this, 2, false), 2);
	const int domain_index = combo_dominio->findData(current_domain);
	combo_dominio->setCurrentIndex(domain_index >= 0 ? domain_index : 0);

	gerenciador_botoes_.set_button_text(
		btn_salvar, compat::tr(this, QStringLiteral("💾 Salvar Conceito"), QStringLiteral("💾 Save Concept")));
	gerenciador_botoes_.set_button_text(btn_limpar, compat::tr(this, QStringLiteral("🧽 Limpar"), QStringLiteral("🧽 Clear")));

	if (btn_toggle_painel_tarefas_ != nullptr) {
		const QString expand_text = expand_task_panel_text(this);
		const QString collapse_text = collapse_task_panel_text(this);
		btn_toggle_painel_tarefas_->setSizeReferenceTexts({expand_text, collapse_text});
		btn_toggle_painel_tarefas_->setText(painel_tarefas_expandido_ ? collapse_text : expand_text);
	}

	const QString conceito_tab = compat::tr(this, QStringLiteral("1️⃣ Conceito"), QStringLiteral("1️⃣ Concept"));
	const QString explique_tab = compat::tr(this, QStringLiteral("2️⃣ Explique"), QStringLiteral("2️⃣ Explain"));
	const QString lacunas_tab = compat::tr(this, QStringLiteral("3️⃣ Lacunas"), QStringLiteral("3️⃣ Gaps"));
	const QString revisao_tab = compat::tr(this, QStringLiteral("4️⃣ Revisão"), QStringLiteral("4️⃣ Review"));
	const QString descricao_tab = compat::tr(this, QStringLiteral("5️⃣ Descrição"), QStringLiteral("5️⃣ Description"));

	if (tab_widget != nullptr && tab_widget->count() >= 5) {
		tab_widget->setTabText(0, conceito_tab);
		tab_widget->setTabText(1, explique_tab);
		tab_widget->setTabText(2, lacunas_tab);
		tab_widget->setTabText(3, revisao_tab);
		tab_widget->setTabText(4, descricao_tab);
	}

	atualizar_largura_minima_painel_esquerdo();
	ajustar_largura_combo_prioridade();
	ajustar_largura_combo_dominio();
	ajustar_altura_combos_footer();
	atualizar_largura_minima_painel_direito();
	atualizar_dimensoes_minimas_iniciais();
	sincronizar_splitter_painel_tarefas();
	atualizar_aba_descricao();
	atualizar_lista();
}

}  // namespace source::plugin::Feynman

namespace source::plugin::Feynman::MetodoFeynmanFns {
void atualizar_traducoes(MetodoFeynman &self) { self.atualizar_traducoes(); }
}
