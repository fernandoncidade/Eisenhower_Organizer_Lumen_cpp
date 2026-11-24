#include "source/modules/feynman/Feynman/fey_16_cleanup.hpp"

#include <QApplication>
#include <QListWidget>
#include <QTextEdit>

namespace source::plugin::Feynman {

void MetodoFeynman::cleanup()
{
	if (auto *app = QApplication::instance()) {
		app->removeEventFilter(this);
	}
	if (lista_conceitos != nullptr) {
		lista_conceitos->removeEventFilter(this);
	}
	if (lista_conceitos != nullptr && lista_conceitos->viewport() != nullptr) {
		lista_conceitos->viewport()->removeEventFilter(this);
	}

	const QList<QTextEdit *> placeholder_editors = {
		input_titulo,
		texto_explicacao,
		texto_lacunas,
		texto_revisao,
		texto_descricao,
	};
	for (QTextEdit *editor : placeholder_editors) {
		if (editor == nullptr) {
			continue;
		}
		editor->removeEventFilter(this);
		if (editor->viewport() != nullptr) {
			editor->viewport()->removeEventFilter(this);
		}
	}

	ocultar_tooltip_arquivo_vinculado();
	if (feynman_tooltip_widget_ != nullptr) {
		delete feynman_tooltip_widget_;
		feynman_tooltip_widget_ = nullptr;
	}

	_stop_feynman_thread();
}

}  // namespace source::plugin::Feynman

namespace source::plugin::Feynman::MetodoFeynmanFns {
void cleanup(MetodoFeynman &self) { self.cleanup(); }
}
