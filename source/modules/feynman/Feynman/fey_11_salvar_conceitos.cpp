#include "source/modules/feynman/Feynman/fey_11_salvar_conceitos.hpp"

#include "source/modules/feynman/Feynman/fey_01_FeynmanWorker.hpp"
#include "source/modules/feynman/Feynman/feynman_metodo_helpers.hpp"

namespace source::plugin::Feynman {

void MetodoFeynman::salvar_conceitos()
{
	conceitos_ = normalized_concepts(conceitos_);
	const QString origin = pending_shared_origin_.trimmed().isEmpty() ? QStringLiteral("feynman")
																	  : pending_shared_origin_.trimmed().toCaseFolded();
	pending_shared_origin_.clear();
	if (feynman_worker_ != nullptr) {
		pending_save_origins_.append(origin);
		feynman_worker_->request_save(arquivo_conceitos_, conceitos_);
		return;
	}

	if (FeynmanWorker::save_file(arquivo_conceitos_, conceitos_) && compat::host_bridge() != nullptr) {
		compat::host_bridge()->notify_shared_state_changed(origin);
	}
}

}  // namespace source::plugin::Feynman

namespace source::plugin::Feynman::MetodoFeynmanFns {
void salvar_conceitos(MetodoFeynman &self) { self.salvar_conceitos(); }
}
