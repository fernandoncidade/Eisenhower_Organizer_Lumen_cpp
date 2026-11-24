#include "source/modules/feynman/Feynman/fey_03_setup_ui.hpp"

#include "source/utils/LinkedTitleUnderlineDelegate.hpp"

#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QProxyStyle>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSplitter>
#include <QStyleOption>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

#include "source/modules/feynman/Feynman/fey_19_RotatedTabButton.hpp"
#include "source/modules/feynman/Feynman/feynman_metodo_helpers.hpp"
#include "source/utils/AccentTabStyle.hpp"

namespace source::plugin::Feynman {
namespace {

class BorderlessReportTabPaneStyle : public QProxyStyle {
public:
	using QProxyStyle::QProxyStyle;

	void drawPrimitive(PrimitiveElement element,
					   const QStyleOption *option,
					   QPainter *painter,
					   const QWidget *widget = nullptr) const override
	{
		if (element == PE_FrameTabWidget) {
			return;
		}

		QProxyStyle::drawPrimitive(element, option, painter, widget);
	}

	QRect subElementRect(SubElement element,
						 const QStyleOption *option,
						 const QWidget *widget = nullptr) const override
	{
		QRect rect = QProxyStyle::subElementRect(element, option, widget);
		if ((element == SE_TabWidgetTabPane || element == SE_TabWidgetTabContents) && option != nullptr
			&& qobject_cast<const QTabWidget *>(widget) != nullptr) {
			rect.setBottom(option->rect.bottom());
		}
		return rect;
	}
};

}  // namespace

void MetodoFeynman::setup_ui()
{
	reset_layout_widgets();

	auto *layout = new QVBoxLayout();
	auto *splitter_row = new QHBoxLayout();
	splitter_row->setContentsMargins(0, 0, 0, 0);
	splitter_row->setSpacing(0);
	splitter_principal_ = new QSplitter(Qt::Horizontal, this);
	splitter_principal_->setChildrenCollapsible(false);
	splitter_principal_->setHandleWidth(kFeynmanSplitterExpandedHandleWidth);
	connect(splitter_principal_, &QSplitter::splitterMoved, this, [this](int, int) {
		atualizar_placeholders_dinamicos();
	});

	btn_toggle_painel_tarefas_ = new source::GerenciamentoUI::RotatedTabButton(collapse_task_panel_text(this), this);
	btn_toggle_painel_tarefas_->setMinimumWidth(25);
	btn_toggle_painel_tarefas_->setSizeReferenceTexts({expand_task_panel_text(this), collapse_task_panel_text(this)});
	btn_toggle_painel_tarefas_->setClickHandler([this]() { alternar_painel_tarefas(); });
	splitter_row->addWidget(btn_toggle_painel_tarefas_, 0, Qt::AlignLeft | Qt::AlignTop);
	splitter_row->addWidget(splitter_principal_, 1);

	painel_esquerdo_ = new QWidget(this);
	auto *layout_esquerdo = new QVBoxLayout(painel_esquerdo_);
	layout_esquerdo->setContentsMargins(0, 0, 0, 0);
	layout_esquerdo->setSpacing(6);

	frame_meus_conceitos = new QGroupBox(painel_esquerdo_);
	frame_meus_conceitos->setFlat(false);
	frame_meus_conceitos->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	frame_meus_conceitos->setMinimumHeight(0);
	auto *frame_meus_conceitos_layout = new QVBoxLayout(frame_meus_conceitos);
	frame_meus_conceitos_layout->setContentsMargins(8, 8, 8, 8);
	frame_meus_conceitos_layout->setSpacing(6);

	lista_conceitos = new QListWidget(frame_meus_conceitos);
	lista_conceitos->setSelectionMode(QAbstractItemView::ExtendedSelection);
	lista_conceitos->setSelectionRectVisible(true);
	lista_conceitos->setMouseTracking(true);
	lista_conceitos->viewport()->setMouseTracking(true);
	lista_conceitos->setContextMenuPolicy(Qt::CustomContextMenu);
	source::utils::install_linked_title_underline_delegate(lista_conceitos);
	configure_compact_scroll_area(lista_conceitos);
	connect(lista_conceitos, &QListWidget::itemClicked, this, &MetodoFeynman::selecionar_conceito);
	connect(lista_conceitos, &QListWidget::itemSelectionChanged, this, &MetodoFeynman::task_counter_changed);
	connect(lista_conceitos, &QListWidget::itemDoubleClicked, this, &MetodoFeynman::on_lista_conceitos_double_click);
	connect(lista_conceitos, &QListWidget::customContextMenuRequested, this, &MetodoFeynman::mostrar_menu_contexto);
	frame_meus_conceitos->installEventFilter(this);
	lista_conceitos->installEventFilter(this);
	if (lista_conceitos->viewport() != nullptr) {
		lista_conceitos->viewport()->installEventFilter(this);
	}
	frame_meus_conceitos_layout->addWidget(lista_conceitos, 1);
	layout_esquerdo->addWidget(frame_meus_conceitos, 1);

	btn_novo = gerenciador_botoes_.create_button();
	btn_deletar = gerenciador_botoes_.create_button();
	connect(btn_novo, &QPushButton::clicked, this, &MetodoFeynman::novo_conceito);
	connect(btn_deletar, &QPushButton::clicked, this, &MetodoFeynman::deletar_conceito);

	painel_direito_ = new QWidget(this);
	auto *layout_direito = new QVBoxLayout(painel_direito_);
	layout_direito->setContentsMargins(0, 0, 0, 0);
	layout_direito->setSpacing(6);
	layout_direito->setSizeConstraint(QLayout::SetNoConstraint);
	painel_direito_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	input_titulo = create_placeholder_text_edit(painel_direito_);
	input_titulo->setObjectName(QStringLiteral("feynman_user_input"));
	configure_compact_scroll_area(input_titulo);
	texto_explicacao = create_placeholder_text_edit(painel_direito_);
	texto_explicacao->setObjectName(QStringLiteral("feynman_user_input"));
	configure_compact_scroll_area(texto_explicacao);
	texto_lacunas = create_placeholder_text_edit(painel_direito_);
	texto_lacunas->setObjectName(QStringLiteral("feynman_user_input"));
	configure_compact_scroll_area(texto_lacunas);
	texto_revisao = create_placeholder_text_edit(painel_direito_);
	texto_revisao->setObjectName(QStringLiteral("feynman_user_input"));
	configure_compact_scroll_area(texto_revisao);
	texto_descricao = create_placeholder_text_edit(painel_direito_);
	texto_descricao->setObjectName(QStringLiteral("feynman_user_input"));
	configure_compact_scroll_area(texto_descricao);

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
		editor->installEventFilter(this);
		if (editor->viewport() != nullptr) {
			editor->viewport()->installEventFilter(this);
		}
	}

	label_prioridade = new QLabel(this);
	configure_compact_label(label_prioridade);
	label_prioridade->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	combo_prioridade = new QComboBox(this);
	label_nivel = new QLabel(this);
	configure_compact_label(label_nivel);
	label_nivel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	combo_dominio = new QComboBox(this);

	tab_widget = nullptr;
	if (normalized_ui_layout_key(ui_layout_mode_key_) == QStringLiteral("tabs")) {
		tab_widget = new QTabWidget(painel_direito_);
		tab_widget->setObjectName(QStringLiteral("feynmanReportTabs"));
		auto *report_tab_pane_style = new BorderlessReportTabPaneStyle(QStringLiteral("Fusion"));
		report_tab_pane_style->setParent(tab_widget);
		tab_widget->setStyle(report_tab_pane_style);
		source::utils::AccentTabStyle::install(tab_widget);
		tab_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		tab_widget->setMinimumHeight(0);

		const auto add_report_tab = [this](QTextEdit *editor, QGroupBox *&frame, const QColor &accent) {
			auto *tab = new QWidget(tab_widget);
			auto *tab_layout = new QVBoxLayout(tab);
			tab_layout->setContentsMargins(0, 0, 0, 0);
			tab_layout->setSpacing(0);

			auto *scroll = new QScrollArea(tab);
			scroll->setWidgetResizable(true);
			scroll->setFrameShape(QFrame::NoFrame);
			scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
			scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			scroll->setMinimumHeight(0);

			auto *content = new QWidget(scroll);
			content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			auto *reports_layout = new QVBoxLayout(content);
			reports_layout->setContentsMargins(0, 0, 0, 0);
			reports_layout->setSpacing(6);

			frame = create_report_frame(content, nullptr, editor);
			reports_layout->addWidget(frame, 1);
			scroll->setWidget(content);
			tab_layout->addWidget(scroll, 1);

			const int tab_index = tab_widget->addTab(tab, QString());
			source::utils::AccentTabStyle::set_tab_accent(tab_widget, tab_index, accent);
		};

		add_report_tab(input_titulo, frame_conceito, source::utils::AccentTabStyle::accent_blue());
		add_report_tab(texto_explicacao, frame_explique, source::utils::AccentTabStyle::accent_yellow());
		add_report_tab(texto_lacunas, frame_lacunas, source::utils::AccentTabStyle::accent_orange());
		add_report_tab(texto_revisao, frame_revisao, source::utils::AccentTabStyle::accent_red());
		add_report_tab(texto_descricao, frame_descricao, source::utils::AccentTabStyle::accent_green());

		layout_direito->addWidget(tab_widget, 1);
	} else {
		auto *scroll = new QScrollArea(painel_direito_);
		scroll->setWidgetResizable(true);
		scroll->setFrameShape(QFrame::NoFrame);
		scroll->setHorizontalScrollBarPolicy(is_horizontal_ui_layout(ui_layout_mode_key_) ? Qt::ScrollBarAsNeeded
																						  : Qt::ScrollBarAlwaysOff);
		scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		scroll->setMinimumHeight(0);

		auto *content = new QWidget(scroll);
		content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

		if (is_horizontal_ui_layout(ui_layout_mode_key_)) {
			auto *reports_layout = new QHBoxLayout(content);
			reports_layout->setContentsMargins(0, 0, 0, 0);
			reports_layout->setSpacing(6);

			frame_conceito = create_report_frame(content, nullptr, input_titulo);
			frame_explique = create_report_frame(content, nullptr, texto_explicacao);
			frame_lacunas = create_report_frame(content, nullptr, texto_lacunas);
			frame_revisao = create_report_frame(content, nullptr, texto_revisao);
			frame_descricao = create_report_frame(content, nullptr, texto_descricao);

			const QList<QGroupBox *> frames = {
				frame_conceito, frame_explique, frame_lacunas, frame_revisao, frame_descricao};
			for (QGroupBox *frame : frames) {
				if (frame != nullptr) {
					frame->setMinimumWidth(260);
					reports_layout->addWidget(frame, 1);
				}
			}
			content->setMinimumWidth(5 * 260 + 4 * reports_layout->spacing());
		} else {
			auto *reports_layout = new QVBoxLayout(content);
			reports_layout->setContentsMargins(0, 0, 0, 0);
			reports_layout->setSpacing(6);

			frame_conceito = create_report_frame(content, nullptr, input_titulo);
			frame_explique = create_report_frame(content, nullptr, texto_explicacao);
			frame_lacunas = create_report_frame(content, nullptr, texto_lacunas);
			frame_revisao = create_report_frame(content, nullptr, texto_revisao);
			frame_descricao = create_report_frame(content, nullptr, texto_descricao);

			const QList<QGroupBox *> frames = {
				frame_conceito, frame_explique, frame_lacunas, frame_revisao, frame_descricao};
			for (QGroupBox *frame : frames) {
				if (frame != nullptr) {
					frame->setMinimumHeight(150);
					reports_layout->addWidget(frame, 1);
				}
			}
		}

		scroll->setWidget(content);
		layout_direito->addWidget(scroll, 1);
	}

	btn_salvar = gerenciador_botoes_.create_button();
	connect(btn_salvar, &QPushButton::clicked, this, &MetodoFeynman::salvar_conceito_atual);

	footer_container_ = new QWidget(this);
	footer_container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	footer_layout_ = new QHBoxLayout(footer_container_);
	footer_layout_->setContentsMargins(0, 0, 0, 0);
	footer_layout_->setSpacing(0);
	combo_prioridade->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	combo_prioridade->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	combo_dominio->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	combo_dominio->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	footer_row_layout_ = new QHBoxLayout();
	footer_row_layout_->setContentsMargins(0, 0, 0, 0);
	footer_row_layout_->setSpacing(6);
	footer_row_layout_->addWidget(btn_novo);
	footer_row_layout_->addWidget(btn_deletar);
	footer_row_layout_->addStretch();
	footer_row_layout_->addWidget(label_prioridade);
	footer_row_layout_->addWidget(combo_prioridade);
	footer_row_layout_->addWidget(label_nivel);
	footer_row_layout_->addWidget(combo_dominio);
	footer_row_layout_->addWidget(btn_salvar);

	adicionar_botao_limpar();
	if (btn_limpar != nullptr) {
		footer_row_layout_->addWidget(btn_limpar);
	}

	footer_layout_->addLayout(footer_row_layout_);

	splitter_principal_->addWidget(painel_esquerdo_);
	splitter_principal_->addWidget(painel_direito_);
	splitter_principal_->setStretchFactor(0, 1);
	splitter_principal_->setStretchFactor(1, 3);
	splitter_principal_->setSizes({kFeynmanDefaultLeftPanelWidth, 940});
	painel_esquerdo_->setVisible(painel_tarefas_expandido_);

	layout->addLayout(splitter_row, 1);
	layout->addWidget(footer_container_);
	setLayout(layout);

	if (auto *app = QApplication::instance()) {
		app->installEventFilter(this);
	}
	const auto sync_description_from_fields = [this]() {
		atualizar_aba_descricao();
		atualizar_placeholders_dinamicos();
	};
	connect(input_titulo, &QTextEdit::textChanged, this, sync_description_from_fields);
	connect(texto_explicacao, &QTextEdit::textChanged, this, sync_description_from_fields);
	connect(texto_lacunas, &QTextEdit::textChanged, this, sync_description_from_fields);
	connect(texto_revisao, &QTextEdit::textChanged, this, sync_description_from_fields);
	connect(texto_descricao, &QTextEdit::textChanged, this, [this]() {
		if (sincronizando_descricao_ || texto_descricao == nullptr) {
			return;
		}

		descricao_editada_manualmente_ =
			texto_descricao->toPlainText().trimmed() != descricao_gerada_atual_.trimmed();
		if (texto_descricao->toPlainText().trimmed().isEmpty()) {
			atualizar_aba_descricao();
		}
		atualizar_placeholders_dinamicos();
	});
	if (tab_widget != nullptr) {
		connect(tab_widget, &QTabWidget::currentChanged, this, [this](int index) {
			if (index == 4) {
				atualizar_aba_descricao();
			}
		});
	}
	sync_backgrounds(*this);
	atualizar_traducoes();
	atualizar_lista();
	atualizar_largura_minima_painel_esquerdo();
	atualizar_largura_minima_painel_direito();
	atualizar_dimensoes_minimas_iniciais();
	atualizar_interacao_splitter_painel_tarefas();
	ajustar_largura_combo_prioridade();
	ajustar_largura_combo_dominio();
	ajustar_altura_combos_footer();

	QTimer::singleShot(0, this, [this]() {
		atualizar_largura_minima_painel_esquerdo();
		atualizar_largura_minima_painel_direito();
		atualizar_dimensoes_minimas_iniciais();
		sincronizar_splitter_painel_tarefas(false);
		ajustar_largura_combo_prioridade();
		ajustar_largura_combo_dominio();
		ajustar_altura_combos_footer();
		atualizar_placeholders_dinamicos();
	});
}

void MetodoFeynman::atualizar_placeholders_dinamicos()
{
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
		if (editor->viewport() != nullptr) {
			editor->viewport()->update();
		} else {
			editor->update();
		}
	}
}

void MetodoFeynman::reset_layout_widgets()
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

	ocultar_tooltip_arquivo_vinculado();

	if (QLayout *old_layout = layout()) {
		clear_layout(old_layout);
		delete old_layout;
	}

	label_meus_conceitos = nullptr;
	frame_meus_conceitos = nullptr;
	lista_conceitos = nullptr;
	btn_novo = nullptr;
	btn_deletar = nullptr;
	label_conceito = nullptr;
	frame_conceito = nullptr;
	input_titulo = nullptr;
	label_passo1 = nullptr;
	frame_explique = nullptr;
	texto_explicacao = nullptr;
	label_passo2 = nullptr;
	frame_lacunas = nullptr;
	texto_lacunas = nullptr;
	label_passo3 = nullptr;
	frame_revisao = nullptr;
	texto_revisao = nullptr;
	label_descricao = nullptr;
	frame_descricao = nullptr;
	texto_descricao = nullptr;
	label_prioridade = nullptr;
	combo_prioridade = nullptr;
	label_nivel = nullptr;
	combo_dominio = nullptr;
	btn_salvar = nullptr;
	btn_limpar = nullptr;
	tab_widget = nullptr;
	painel_esquerdo_ = nullptr;
	painel_direito_ = nullptr;
	splitter_principal_ = nullptr;
	btn_toggle_painel_tarefas_ = nullptr;
	footer_container_ = nullptr;
	footer_layout_ = nullptr;
	footer_row_layout_ = nullptr;
}

}  // namespace source::plugin::Feynman

namespace source::plugin::Feynman::MetodoFeynmanFns {
void setup_ui(MetodoFeynman &self) { self.setup_ui(); }
}
