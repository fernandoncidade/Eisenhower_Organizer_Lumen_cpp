#include "source/modules/feynman/Feynman/fey_17_limpar_abas.hpp"

#include <exception>
#include <QPushButton>
#include <QTextEdit>

#include "source/modules/feynman/Feynman/utils/EventBus.hpp"

namespace source::plugin::Feynman {

void MetodoFeynman::limpar_abas()
{
	limpar_campos();
	if (input_titulo != nullptr) {
		input_titulo->setFocus();
	}
}

bool MetodoFeynman::limpar_todas_as_tarefas()
{
	try {
		if (auto *event_bus = utils::get_event_bus(); event_bus != nullptr) {
			event_bus->clear_pending();
		}

		conceitos_.clear();
		has_conceito_atual_ = false;
		conceito_atual_.clear();
		pending_shared_origin_ = QStringLiteral("feynman");

		atualizar_lista();
		limpar_campos();
		salvar_conceitos();

		if (input_titulo != nullptr) {
			input_titulo->setFocus();
		}

		return true;
	} catch (const std::exception &e) {
		if (logger_ != nullptr) {
			logger_->error(QStringLiteral("Erro ao limpar todas as tarefas do Método Feynman: %1")
							   .arg(QString::fromUtf8(e.what())),
						   true);
		}
	} catch (...) {
		if (logger_ != nullptr) {
			logger_->error(QStringLiteral("Erro ao limpar todas as tarefas do Método Feynman"), true);
		}
	}

	return false;
}

void MetodoFeynman::adicionar_botao_limpar()
{
	btn_limpar = gerenciador_botoes_.create_button();
	gerenciador_botoes_.set_button_text(btn_limpar, compat::tr(this, QStringLiteral("🧽 Limpar"), QStringLiteral("🧽 Clear")));
	connect(btn_limpar, &QPushButton::clicked, this, &MetodoFeynman::limpar_abas);
}

}  // namespace source::plugin::Feynman

namespace source::plugin::Feynman::MetodoFeynmanFns {
void limpar_abas(MetodoFeynman &self) { self.limpar_abas(); }
void adicionar_botao_limpar(MetodoFeynman &self) { self.adicionar_botao_limpar(); }
}
