#include "source/modules/feynman/Feynman/fey_18_arquivo_vinculado.hpp"

#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>

#include "source/modules/feynman/Feynman/feynman_metodo_helpers.hpp"

namespace source::plugin::Feynman {

void MetodoFeynman::atualizar_arquivo_vinculado(QListWidgetItem *item, const QString &caminho)
{
	const int index = obter_indice_conceito_do_item(item);
	if (index < 0 || index >= conceitos_.size()) {
		return;
	}

	QVariantMap conceito = normalized_concept(conceitos_.at(index).toMap());
	const QString caminho_limpo = caminho.trimmed();
	conceito.insert(QStringLiteral("arquivo_vinculado"), caminho_limpo.isEmpty() ? QVariant() : QVariant(caminho_limpo));
	conceitos_[index] = conceito;

	if (has_conceito_atual_
		&& conceito_atual_.value(QStringLiteral("titulo")).toString().compare(conceito.value(QStringLiteral("titulo")).toString(),
																			  Qt::CaseInsensitive)
			   == 0) {
		conceito_atual_ = conceito;
	}

	salvar_conceitos();
	atualizar_lista();
}

void MetodoFeynman::vincular_arquivo_conceito(QListWidgetItem *item)
{
	const int index = obter_indice_conceito_do_item(item);
	if (index < 0 || index >= conceitos_.size()) {
		return;
	}

	QVariantMap conceito = normalized_concept(conceitos_.at(index).toMap());
	const QString caminho_atual = conceito.value(QStringLiteral("arquivo_vinculado")).toString().trimmed();
	const QString diretorio_base =
		caminho_atual.isEmpty() ? QString() : QFileInfo(caminho_atual).absoluteDir().absolutePath();

	const QString caminho = QFileDialog::getOpenFileName(
		this,
		compat::tr(this,
				   QStringLiteral("Selecionar arquivo para vincular"),
				   QStringLiteral("Select a file to link")),
		diretorio_base,
		compat::tr(this, QStringLiteral("Todos os arquivos (*)"), QStringLiteral("All files (*)")));

	if (caminho.isEmpty()) {
		return;
	}

	const QString absoluto = QFileInfo(caminho).absoluteFilePath();
	atualizar_arquivo_vinculado(item, absoluto);
}

void MetodoFeynman::remover_arquivo_vinculado(QListWidgetItem *item)
{
	const int index = obter_indice_conceito_do_item(item);
	if (index < 0 || index >= conceitos_.size()) {
		return;
	}

	QVariantMap conceito = normalized_concept(conceitos_.at(index).toMap());
	if (conceito.value(QStringLiteral("arquivo_vinculado")).toString().trimmed().isEmpty()) {
		return;
	}

	atualizar_arquivo_vinculado(item, QString());
	compat::MessageBox::feynman_unlinked_file_removed(this);
}

void MetodoFeynman::abrir_arquivo_conceito(QListWidgetItem *item)
{
	const QVariantMap conceito = obter_conceito_do_item(item);
	const QString caminho = conceito.value(QStringLiteral("arquivo_vinculado")).toString().trimmed();
	if (caminho.isEmpty()) {
		return;
	}

	if (QFileInfo::exists(caminho)) {
		compat::open_local_path(caminho, this);
		return;
	}

	switch (compat::prompt_missing_linked_file(this)) {
	case compat::MissingLinkedFileChoice::RemoveLink:
		remover_arquivo_vinculado(item);
		return;
	case compat::MissingLinkedFileChoice::Search: {
		const QString novo_caminho = compat::browse_linked_file_replacement(this, caminho).trimmed();
		if (!novo_caminho.isEmpty()) {
			atualizar_arquivo_vinculado(item, QFileInfo(novo_caminho).absoluteFilePath());
		}
		return;
	}
	case compat::MissingLinkedFileChoice::None:
		return;
	}
}

void MetodoFeynman::on_lista_conceitos_double_click(QListWidgetItem *item)
{
	abrir_arquivo_conceito(item);
}

void MetodoFeynman::mostrar_tooltip_arquivo_vinculado(QListWidgetItem *item)
{
	Q_UNUSED(item);
}

void MetodoFeynman::ocultar_tooltip_arquivo_vinculado()
{
	if (feynman_tooltip_widget_ != nullptr) {
		feynman_tooltip_widget_->hide();
	}
}

void MetodoFeynman::configurar_tooltip_arquivo_vinculado()
{
}

}  // namespace source::plugin::Feynman

namespace source::plugin::Feynman::MetodoFeynmanFns {
void vincular_arquivo_conceito(MetodoFeynman &self, QListWidgetItem *item) { self.vincular_arquivo_conceito(item); }
void remover_arquivo_vinculado(MetodoFeynman &self, QListWidgetItem *item) { self.remover_arquivo_vinculado(item); }
void abrir_arquivo_conceito(MetodoFeynman &self, QListWidgetItem *item) { self.abrir_arquivo_conceito(item); }
void on_lista_conceitos_double_click(MetodoFeynman &self, QListWidgetItem *item) { self.on_lista_conceitos_double_click(item); }
void mostrar_tooltip_arquivo_vinculado(MetodoFeynman &self, QListWidgetItem *item) { self.mostrar_tooltip_arquivo_vinculado(item); }
void ocultar_tooltip_arquivo_vinculado(MetodoFeynman &self) { self.ocultar_tooltip_arquivo_vinculado(); }
void configurar_tooltip_arquivo_vinculado(MetodoFeynman &self) { self.configurar_tooltip_arquivo_vinculado(); }
}
