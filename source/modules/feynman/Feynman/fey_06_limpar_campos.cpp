#include "source/modules/feynman/Feynman/fey_06_limpar_campos.hpp"

#include <QComboBox>
#include <QTextEdit>

namespace source::plugin::Feynman {

void MetodoFeynman::limpar_campos()
{
	sincronizando_descricao_ = true;
	descricao_editada_manualmente_ = false;
	descricao_gerada_atual_.clear();
	input_titulo->clear();
	texto_explicacao->clear();
	texto_lacunas->clear();
	texto_revisao->clear();
	if (texto_descricao != nullptr) {
		texto_descricao->clear();
	}
	sincronizando_descricao_ = false;
	combo_prioridade->setCurrentIndex(0);
	combo_dominio->setCurrentIndex(0);
	atualizar_aba_descricao();
	atualizar_placeholders_dinamicos();
}

}  // namespace source::plugin::Feynman

namespace source::plugin::Feynman::MetodoFeynmanFns {
void limpar_campos(MetodoFeynman &self) { self.limpar_campos(); }
}
