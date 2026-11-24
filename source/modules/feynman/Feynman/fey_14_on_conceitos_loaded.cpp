#include "source/modules/feynman/Feynman/fey_14_on_conceitos_loaded.hpp"

#include "source/modules/feynman/Feynman/feynman_metodo_helpers.hpp"

namespace source::plugin::Feynman {

void MetodoFeynman::_on_conceitos_loaded(const QVariantList &conceitos)
{
	conceitos_ = normalized_concepts(conceitos);
	atualizar_lista();
}

}  // namespace source::plugin::Feynman

namespace source::plugin::Feynman::MetodoFeynmanFns {
void _on_conceitos_loaded(MetodoFeynman &self, const QVariantList &conceitos) { self._on_conceitos_loaded(conceitos); }
}
