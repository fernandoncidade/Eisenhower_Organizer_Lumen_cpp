#include "source/modules/feynman/Feynman/fey_13_receber_conceito_mapa_mental.hpp"

#include "source/modules/feynman/Feynman/feynman_metodo_helpers.hpp"

namespace source::plugin::Feynman {

void MetodoFeynman::receber_conceito_mapa_mental(const QVariantMap &dados)
{
	const QString titulo = dados.value(QStringLiteral("titulo")).toString().trimmed();
	const QString explicacao = dados.value(QStringLiteral("explicacao")).toString().trimmed().isEmpty()
								   ? dados.value(QStringLiteral("notas")).toString().trimmed()
								   : dados.value(QStringLiteral("explicacao")).toString().trimmed();
	const QString lacunas = dados.value(QStringLiteral("lacunas")).toString().trimmed();
	const QString revisao = dados.value(QStringLiteral("revisao")).toString().trimmed().isEmpty()
								? explicacao
								: dados.value(QStringLiteral("revisao")).toString().trimmed();
	const int dominio = normalized_domain_value(dados.value(QStringLiteral("dominio")));
	const bool has_prioridade = dados.contains(QStringLiteral("prioridade"));
	const int prioridade = has_prioridade ? normalized_priority_value(dados.value(QStringLiteral("prioridade")))
										  : kDefaultPriority;
	const QString arquivo_vinculado = dados.value(QStringLiteral("arquivo_vinculado")).toString().trimmed();

	if (titulo.isEmpty()) {
		if (logger_ != nullptr) {
			logger_->warning(QStringLiteral("Título vazio recebido do Mapa Mental, ignorando."));
		}
		return;
	}

	if (explicacao.isEmpty()) {
		if (logger_ != nullptr) {
			logger_->warning(QStringLiteral("Explicação vazia recebida para o conceito '%1', ignorando.").arg(titulo));
		}
		return;
	}

	const int existing_index = encontrar_indice_conceito(titulo);
	if (existing_index >= 0) {
		QVariantMap existente = normalized_concept(conceitos_.at(existing_index).toMap());
		existente.insert(QStringLiteral("revisao"), revisao);

		const QString explicacao_atual = existente.value(QStringLiteral("explicacao")).toString();
		if (explicacao_atual.trimmed().isEmpty()) {
			existente.insert(QStringLiteral("explicacao"), explicacao);
		} else if (!explicacao_atual.contains(explicacao, Qt::CaseSensitive)) {
			existente.insert(QStringLiteral("explicacao"), QStringLiteral("%1\n\n%2").arg(explicacao_atual, explicacao));
		}

		if (!lacunas.isEmpty()) {
			existente.insert(QStringLiteral("lacunas"), lacunas);
		}

		existente.insert(QStringLiteral("dominio"), dominio);
		if (has_prioridade) {
			existente.insert(QStringLiteral("prioridade"), prioridade);
		}
		if (!arquivo_vinculado.isEmpty()) {
			existente.insert(QStringLiteral("arquivo_vinculado"), arquivo_vinculado);
		}
		conceitos_[existing_index] = existente;
	} else {
		conceitos_.append(normalized_concept(QVariantMap{
			{QStringLiteral("titulo"), titulo},
			{QStringLiteral("explicacao"), explicacao},
			{QStringLiteral("lacunas"), lacunas},
			{QStringLiteral("revisao"), revisao},
			{QStringLiteral("prioridade"), prioridade},
			{QStringLiteral("dominio"), dominio},
			{QStringLiteral("arquivo_vinculado"), arquivo_vinculado.isEmpty() ? QVariant() : QVariant(arquivo_vinculado)},
		}));
	}

	salvar_conceitos();
	atualizar_lista();
}

void MetodoFeynman::receber_tarefa_compartilhada(const QVariantMap &dados)
{
	const QString titulo = dados.value(QStringLiteral("titulo")).toString().trimmed();
	const QString origem = dados.value(QStringLiteral("origin")).toString().trimmed().isEmpty()
							   ? QStringLiteral("externo")
							   : dados.value(QStringLiteral("origin")).toString().trimmed();
	const QString task_id = dados.value(QStringLiteral("id")).toString().trimmed();
	const QString descricao = dados.value(QStringLiteral("description")).toString().trimmed();
	const QString arquivo_vinculado = dados.value(QStringLiteral("file_path")).toString().trimmed();
	const QString data = dados.value(QStringLiteral("date")).toString().trimmed();
	const QString horario = dados.value(QStringLiteral("time")).toString().trimmed();
	const QString status = dados.value(QStringLiteral("status")).toString().trimmed();
	const QString cor_texto = dados.value(QStringLiteral("cor_texto"), dados.value(QStringLiteral("task_color"))).toString().trimmed();
	const bool completed = dados.value(QStringLiteral("completed")).toBool();
	const int prioridade = normalized_priority_value(dados.value(QStringLiteral("prioridade")));
	const bool has_dominio = dados.contains(QStringLiteral("dominio"));
	const int dominio = has_dominio ? normalized_domain_value(dados.value(QStringLiteral("dominio"))) : kDefaultDomain;
	const DescriptionParts description_parts = parse_description_text(descricao);

	if (titulo.isEmpty()) {
		if (logger_ != nullptr) {
			logger_->warning(QStringLiteral("Título vazio recebido na integração com Feynman, ignorando."));
		}
		return;
	}

	double dur = 1.0;
	if (dados.contains(QStringLiteral("duracao")) && dados.value(QStringLiteral("duracao")).toDouble() > 0.0) {
		dur = dados.value(QStringLiteral("duracao")).toDouble();
	} else if (dados.contains(QStringLiteral("duration")) && dados.value(QStringLiteral("duration")).toDouble() > 0.0) {
		dur = dados.value(QStringLiteral("duration")).toDouble();
	} else if (dados.contains(QStringLiteral("duracao_dias")) && dados.value(QStringLiteral("duracao_dias")).toDouble() > 0.0) {
		dur = dados.value(QStringLiteral("duracao_dias")).toDouble();
	}
	const int pom = dados.value(QStringLiteral("pomodoros"), dados.value(QStringLiteral("pomodoros_completados"), 0)).toInt();
	const int parsed_target_cycles = [dados]() -> int {
		if (dados.contains(QStringLiteral("ciclos_totais")) && dados.value(QStringLiteral("ciclos_totais")).toInt() > 0) {
			return dados.value(QStringLiteral("ciclos_totais")).toInt();
		}
		if (dados.contains(QStringLiteral("ciclos_permitidos")) && dados.value(QStringLiteral("ciclos_permitidos")).toInt() > 0) {
			return dados.value(QStringLiteral("ciclos_permitidos")).toInt();
		}
		if (dados.contains(QStringLiteral("total_ciclos")) && dados.value(QStringLiteral("total_ciclos")).toInt() > 0) {
			return dados.value(QStringLiteral("total_ciclos")).toInt();
		}
		return 0;
	}();
	const bool ciclos_customizados = dados.value(QStringLiteral("ciclos_customizados"), parsed_target_cycles > 0).toBool();
	const bool concluida = dados.value(QStringLiteral("concluida"), completed).toBool();
	const bool in_prog = dados.value(QStringLiteral("em_progresso"),
	                                 dados.value(QStringLiteral("in_progress"),
	                                             dados.value(QStringLiteral("em_andamento"),
	                                                         status.compare(QStringLiteral("Doing"), Qt::CaseInsensitive) == 0))).toBool();

	QVariantMap conceito_base = {
		{QStringLiteral("titulo"), titulo},
		{QStringLiteral("explicacao"), description_parts.structured ? description_parts.explanation : descricao},
		{QStringLiteral("lacunas"), description_parts.structured ? description_parts.gaps : QString()},
		{QStringLiteral("revisao"), description_parts.structured ? description_parts.review : QString()},
		{QStringLiteral("descricao_customizada"), false},
		{QStringLiteral("prioridade"), prioridade},
		{QStringLiteral("dominio"), dominio},
		{QStringLiteral("arquivo_vinculado"), arquivo_vinculado.isEmpty() ? QVariant() : QVariant(arquivo_vinculado)},
		{QStringLiteral("cor_texto"), cor_texto.isEmpty() ? QVariant() : QVariant(cor_texto)},
		{QStringLiteral("integracao_origem"), origem},
		{QStringLiteral("integracao_task_id"), task_id},
		{QStringLiteral("integracao_status"), status},
		{QStringLiteral("status"), status},
		{QStringLiteral("integracao_date"), data},
		{QStringLiteral("integracao_time"), horario},
		{QStringLiteral("integracao_completed"), completed},
		{QStringLiteral("concluida"), concluida},
		{QStringLiteral("completed"), completed},
		{QStringLiteral("em_progresso"), in_prog},
		{QStringLiteral("in_progress"), in_prog},
		{QStringLiteral("duracao"), dur},
		{QStringLiteral("duration"), dur},
		{QStringLiteral("pomodoros"), pom},
		{QStringLiteral("pomodoros_completados"), pom},
		{QStringLiteral("ciclos_customizados"), ciclos_customizados},
	};
	if (parsed_target_cycles > 0) {
		conceito_base.insert(QStringLiteral("ciclos_totais"), parsed_target_cycles);
		conceito_base.insert(QStringLiteral("ciclos_permitidos"), parsed_target_cycles);
		conceito_base.insert(QStringLiteral("total_ciclos"), parsed_target_cycles);
	}
	const QString descricao_base_gerada = composed_description(this,
															  conceito_base.value(QStringLiteral("titulo")).toString(),
															  conceito_base.value(QStringLiteral("explicacao")).toString(),
															  conceito_base.value(QStringLiteral("lacunas")).toString(),
															  conceito_base.value(QStringLiteral("revisao")).toString())
											 .trimmed();
	const QString descricao_base_manual = manual_description_text(descricao, descricao_base_gerada);
	if (!descricao_base_manual.isEmpty()) {
		conceito_base.insert(QStringLiteral("descricao"), merged_description_text(descricao_base_gerada, descricao_base_manual));
		conceito_base.insert(QStringLiteral("descricao_customizada"), true);
	}

	const int existing_index =
		!origem.isEmpty() && !task_id.isEmpty() ? encontrar_indice_conceito_integrado(origem, task_id)
												: encontrar_indice_conceito(titulo, Qt::CaseInsensitive);

	if (existing_index >= 0) {
		QVariantMap existente = normalized_concept(conceitos_.at(existing_index).toMap());
		existente.insert(QStringLiteral("titulo"), titulo);
		existente.insert(QStringLiteral("prioridade"), prioridade);
		if (has_dominio) {
			existente.insert(QStringLiteral("dominio"), dominio);
		}
		existente.insert(QStringLiteral("duracao"), dur);
		existente.insert(QStringLiteral("duration"), dur);
		existente.insert(QStringLiteral("pomodoros"), pom);
		existente.insert(QStringLiteral("pomodoros_completados"), pom);
		if (parsed_target_cycles > 0) {
			existente.insert(QStringLiteral("ciclos_totais"), parsed_target_cycles);
			existente.insert(QStringLiteral("ciclos_permitidos"), parsed_target_cycles);
			existente.insert(QStringLiteral("total_ciclos"), parsed_target_cycles);
		}
		if (dados.contains(QStringLiteral("ciclos_customizados")) || parsed_target_cycles > 0) {
			existente.insert(QStringLiteral("ciclos_customizados"), ciclos_customizados);
		}
		existente.insert(QStringLiteral("integracao_origem"), origem);
		existente.insert(QStringLiteral("integracao_task_id"), task_id);
		existente.insert(QStringLiteral("integracao_status"), status);
		existente.insert(QStringLiteral("status"), status);
		existente.insert(QStringLiteral("integracao_date"), data);
		existente.insert(QStringLiteral("integracao_time"), horario);
		existente.insert(QStringLiteral("integracao_completed"), completed);
		existente.insert(QStringLiteral("concluida"), concluida);
		existente.insert(QStringLiteral("completed"), completed);
		existente.insert(QStringLiteral("em_progresso"), in_prog);
		existente.insert(QStringLiteral("in_progress"), in_prog);
		if (existente.value(QStringLiteral("explicacao")).toString().trimmed().isEmpty() && !descricao.isEmpty()) {
			existente.insert(QStringLiteral("explicacao"),
							 description_parts.structured ? description_parts.explanation : descricao);
		}
		if (description_parts.structured) {
			if (existente.value(QStringLiteral("lacunas")).toString().trimmed().isEmpty() && !description_parts.gaps.isEmpty()) {
				existente.insert(QStringLiteral("lacunas"), description_parts.gaps);
			}
			if (existente.value(QStringLiteral("revisao")).toString().trimmed().isEmpty() && !description_parts.review.isEmpty()) {
				existente.insert(QStringLiteral("revisao"), description_parts.review);
			}
		}

		const QString descricao_existente_gerada =
			composed_description(this,
								 existente.value(QStringLiteral("titulo")).toString(),
								 existente.value(QStringLiteral("explicacao")).toString(),
								 existente.value(QStringLiteral("lacunas")).toString(),
								 existente.value(QStringLiteral("revisao")).toString())
				.trimmed();
		const QString descricao_existente_manual = manual_description_text(descricao, descricao_existente_gerada);
		if (!descricao_existente_manual.isEmpty()) {
			existente.insert(QStringLiteral("descricao"),
							 merged_description_text(descricao_existente_gerada, descricao_existente_manual));
			existente.insert(QStringLiteral("descricao_customizada"), true);
		}

		if (existente.value(QStringLiteral("arquivo_vinculado")).toString().trimmed().isEmpty() && !arquivo_vinculado.isEmpty()) {
			existente.insert(QStringLiteral("arquivo_vinculado"), arquivo_vinculado);
		}
		if (!cor_texto.isEmpty()) {
			existente.insert(QStringLiteral("cor_texto"), cor_texto);
		}

		conceitos_[existing_index] = normalized_concept(existente);
	} else {
		conceitos_.append(normalized_concept(conceito_base));
	}

	pending_shared_origin_ = origem;
	salvar_conceitos();
	atualizar_lista();
}

}  // namespace source::plugin::Feynman

namespace source::plugin::Feynman::MetodoFeynmanFns {
void receber_conceito_mapa_mental(MetodoFeynman &self, const QVariantMap &dados) { self.receber_conceito_mapa_mental(dados); }
}
