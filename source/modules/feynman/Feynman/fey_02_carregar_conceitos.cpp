#include "source/modules/feynman/Feynman/fey_02_carregar_conceitos.hpp"

#include "source/modules/feynman/Feynman/fey_01_FeynmanWorker.hpp"
#include "source/modules/feynman/Feynman/feynman_metodo_helpers.hpp"

namespace source::plugin::Feynman {

void MetodoFeynman::carregar_conceitos()
{
	if (feynman_worker_ != nullptr) {
		feynman_worker_->request_load(arquivo_conceitos_);
		return;
	}

	conceitos_ = normalized_concepts(FeynmanWorker::load_file(arquivo_conceitos_));
	atualizar_lista();
}

}  // namespace source::plugin::Feynman

namespace source::plugin::Feynman::MetodoFeynmanFns {
void carregar_conceitos(MetodoFeynman &self) { self.carregar_conceitos(); }
}
