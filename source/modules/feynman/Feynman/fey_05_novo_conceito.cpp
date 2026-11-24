#include "source/modules/feynman/Feynman/fey_05_novo_conceito.hpp"

#include "source/modules/feynman/Feynman/feynman_metodo_helpers.hpp"

namespace source::plugin::Feynman {

void MetodoFeynman::novo_conceito()
{
	conceito_atual_ = {
		{QStringLiteral("titulo"), QString()},
		{QStringLiteral("explicacao"), QString()},
		{QStringLiteral("lacunas"), QString()},
		{QStringLiteral("revisao"), QString()},
		{QStringLiteral("descricao"), QString()},
		{QStringLiteral("descricao_customizada"), false},
		{QStringLiteral("prioridade"), kDefaultPriority},
		{QStringLiteral("dominio"), kDefaultDomain},
		{QStringLiteral("arquivo_vinculado"), QVariant()},
	};
	has_conceito_atual_ = true;
	limpar_campos();
}

}  // namespace source::plugin::Feynman

namespace source::plugin::Feynman::MetodoFeynmanFns {
void novo_conceito(MetodoFeynman &self) { self.novo_conceito(); }
}
