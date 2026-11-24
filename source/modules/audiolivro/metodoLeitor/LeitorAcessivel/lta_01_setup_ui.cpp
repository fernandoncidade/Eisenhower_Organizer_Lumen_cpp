#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_01_setup_ui.hpp"

#include "source/modules/audiolivro/audiolivro_settings.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_27_pdf_findbar.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_29_text_findbar.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_23_pdf_mouse_handler.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_37_FileDropEdit.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_38_FileDropPDFView.hpp"
#include "source/utils/AccentTabStyle.hpp"

#include <QComboBox>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QPalette>
#include <QPushButton>
#include <QShortcut>
#include <QSlider>
#include <QSpinBox>
#include <QTabBar>
#include <QTabWidget>
#include <QVBoxLayout>

namespace source::plugin::Audiolivro {

namespace {
constexpr int kContentPageTopMargin = 6;
constexpr int kContentPageBottomMargin = 0;
}

void LeitorAcessivel::setup_ui()
{
    auto *layout = new QVBoxLayout(this);

    auto *controles = new QHBoxLayout();
    btn_carregar_ = new QPushButton(this);
    btn_play_ = new QPushButton(this);
    btn_pause_ = new QPushButton(this);
    btn_stop_ = new QPushButton(this);
    btn_limpar_ = new QPushButton(this);

    btn_pause_->setEnabled(false);
    btn_pause_->setCheckable(true);
    btn_stop_->setEnabled(false);

    controles->addWidget(btn_carregar_);
    controles->addWidget(btn_play_);
    controles->addWidget(btn_pause_);
    controles->addWidget(btn_stop_);
    controles->addWidget(btn_limpar_);
    controles->addStretch();
    layout->addLayout(controles);

    connect(btn_carregar_, &QPushButton::clicked, this, [this] { carregar_pdf(); });
    connect(btn_play_, &QPushButton::clicked, this, &LeitorAcessivel::iniciar_leitura);
    connect(btn_pause_, &QPushButton::clicked, this, &LeitorAcessivel::pausar_leitura);
    connect(btn_stop_, &QPushButton::clicked, this, &LeitorAcessivel::parar_leitura);
    connect(btn_limpar_, &QPushButton::clicked, this, &LeitorAcessivel::limpar_painel);

    auto *ajustes = new QHBoxLayout();
    label_velocidade_titulo_ = new QLabel(this);
    slider_velocidade_ = new QSlider(Qt::Horizontal, this);
    slider_velocidade_->setRange(50, 250);
    slider_velocidade_->setValue(150);
    slider_velocidade_->setTickPosition(QSlider::TicksBelow);
    slider_velocidade_->setTickInterval(25);
    label_velocidade_ = new QLabel(tr("%1 wpm").arg(150), this);

    label_volume_ = new QLabel(this);
    slider_volume_ = new QSlider(Qt::Horizontal, this);
    slider_volume_->setRange(0, 100);
    slider_volume_->setValue(100);

    ajustes->addWidget(label_velocidade_titulo_);
    ajustes->addWidget(slider_velocidade_, 2);
    ajustes->addWidget(label_velocidade_);
    ajustes->addWidget(label_volume_);
    ajustes->addWidget(slider_volume_, 1);
    layout->addLayout(ajustes);

    connect(slider_velocidade_, &QSlider::valueChanged, this, &LeitorAcessivel::on_speed_changed);
    connect(slider_volume_, &QSlider::valueChanged, this, &LeitorAcessivel::on_volume_changed);

    content_stack_ = new QTabWidget(this);
    content_stack_->setDocumentMode(true);
    source::utils::AccentTabStyle::install(content_stack_);
    source::utils::AccentTabStyle::set_alignment(content_stack_, Qt::AlignLeft);
    if (content_stack_->tabBar() != nullptr) {
        content_stack_->tabBar()->setExpanding(false);
        content_stack_->tabBar()->setContentsMargins(10, 0, 0, 0);
    }

    auto *text_tab = new QWidget(content_stack_);
    auto *text_layout = new QVBoxLayout(text_tab);
    text_layout->setContentsMargins(0, kContentPageTopMargin, 0, kContentPageBottomMargin);
    auto *text_toolbar = new QHBoxLayout();

    btn_new_ = new QPushButton(text_tab);
    btn_save_ = new QPushButton(text_tab);
    btn_undo_ = new QPushButton(text_tab);
    btn_redo_ = new QPushButton(text_tab);
    btn_find_text_ = new QPushButton(text_tab);
    btn_font_settings_ = new QPushButton(text_tab);
    btn_color_tasks_ = new QPushButton(text_tab);
    btn_reset_settings_ = new QPushButton(text_tab);
    btn_bullets_ = new QPushButton(text_tab);
    label_bullet_style_ = new QLabel(text_tab);
    combo_bullet_style_ = new QComboBox(text_tab);
    label_spacing_ = new QLabel(text_tab);
    combo_spacing_ = new QComboBox(text_tab);
    label_indent_ = new QLabel(text_tab);
    spin_indent_ = new QSpinBox(text_tab);
    label_margin_ = new QLabel(text_tab);
    spin_margin_ = new QSpinBox(text_tab);

    combo_bullet_style_->addItems({QStringLiteral("-"), QStringLiteral("*"), QStringLiteral("+"),
                                   QStringLiteral("•"), QStringLiteral("◦"), QStringLiteral("‣"),
                                   QStringLiteral("∙"), QStringLiteral("▪"), QStringLiteral("▫"),
                                   QStringLiteral("◆"), QStringLiteral("◇"), QStringLiteral("■"),
                                   QStringLiteral("□"), QStringLiteral("●"), QStringLiteral("○"),
                                   QStringLiteral("→"), QStringLiteral("✓"), QStringLiteral("☑"),
                                   QStringLiteral("1."), QStringLiteral("a."), QStringLiteral("A."),
                                   QStringLiteral("i."), QStringLiteral("I.")});
    combo_spacing_->addItems({QStringLiteral("1.0"), QStringLiteral("1.15"), QStringLiteral("1.5"), QStringLiteral("2.0")});
    spin_indent_->setRange(0, 500);
    spin_indent_->setSingleStep(5);
    spin_margin_->setRange(0, 200);
    spin_margin_->setValue(10);
    btn_undo_->setEnabled(false);
    btn_redo_->setEnabled(false);

    auto *text_toolbar_line1 = new QHBoxLayout();
    text_toolbar_line1->addWidget(btn_new_);
    text_toolbar_line1->addWidget(btn_save_);
    text_toolbar_line1->addWidget(btn_undo_);
    text_toolbar_line1->addWidget(btn_redo_);
    text_toolbar_line1->addWidget(btn_find_text_);
    text_toolbar_line1->addStretch();
    text_toolbar_line1->addWidget(btn_bullets_);
    text_toolbar_line1->addWidget(label_bullet_style_);
    text_toolbar_line1->addWidget(combo_bullet_style_);
    text_toolbar_line1->addWidget(label_spacing_);
    text_toolbar_line1->addWidget(combo_spacing_);
    text_toolbar_line1->addWidget(label_indent_);
    text_toolbar_line1->addWidget(spin_indent_);
    text_toolbar_line1->addWidget(label_margin_);
    text_toolbar_line1->addWidget(spin_margin_);
    text_layout->addLayout(text_toolbar_line1);

    auto *text_toolbar_line2 = new QHBoxLayout();
    text_toolbar_line2->addWidget(btn_font_settings_);
    text_toolbar_line2->addWidget(btn_color_tasks_);
    text_toolbar_line2->addWidget(btn_reset_settings_);
    text_toolbar_line2->addStretch();
    text_layout->addLayout(text_toolbar_line2);

    text_find_bar_ = new TextFindBar(text_tab);
    text_layout->addWidget(text_find_bar_);

    texto_area_ = new FileDropEdit(text_tab);
    texto_area_->setReadOnly(false);
    texto_area_->document()->setDocumentMargin(spin_margin_->value());
    text_layout->addWidget(texto_area_, 1);

    save_text_timer_.setSingleShot(true);
    save_text_timer_.setInterval(500);
    connect(&save_text_timer_, &QTimer::timeout, this, &LeitorAcessivel::salvar_texto_temp);

    connect(btn_new_, &QPushButton::clicked, this, [this] { criar_texto(true); });
    connect(btn_save_, &QPushButton::clicked, this, &LeitorAcessivel::salvar_como);
    connect(btn_undo_, &QPushButton::clicked, texto_area_, &QTextEdit::undo);
    connect(btn_redo_, &QPushButton::clicked, texto_area_, &QTextEdit::redo);
    connect(btn_find_text_, &QPushButton::clicked, this, &LeitorAcessivel::text_find_show);
    connect(btn_font_settings_, &QPushButton::clicked, this, &LeitorAcessivel::abrir_configuracoes_fonte);
    connect(btn_color_tasks_, &QPushButton::clicked, this, &LeitorAcessivel::abrir_colorir_tarefas);
    connect(btn_reset_settings_, &QPushButton::clicked, this, &LeitorAcessivel::restaurar_configuracoes);
    connect(btn_bullets_, &QPushButton::clicked, this, &LeitorAcessivel::toggle_bullets);
    connect(texto_area_, &QTextEdit::undoAvailable, btn_undo_, &QPushButton::setEnabled);
    connect(texto_area_, &QTextEdit::redoAvailable, btn_redo_, &QPushButton::setEnabled);
    connect(texto_area_, &QTextEdit::textChanged, this, [this]() { save_text_timer_.start(); });
    connect(combo_spacing_, &QComboBox::currentTextChanged, this, [this](const QString &value) {
        set_line_spacing(value.toDouble());
    });
    connect(spin_indent_, &QSpinBox::valueChanged, this, &LeitorAcessivel::set_indent);
    connect(spin_margin_, &QSpinBox::valueChanged, this, [this](int value) { set_margins(value); });
    connect(texto_area_, &FileDropEdit::files_dropped, this, [this](const QStringList &paths) {
        for (const QString &path : paths) {
            if (path.endsWith(QStringLiteral(".pdf"), Qt::CaseInsensitive)) {
                carregar_pdf(path, true);
                return;
            }
            QFile file(path);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                texto_area_->setPlainText(QString::fromUtf8(file.readAll()));
                return;
            }
        }
    });
    connect(text_find_bar_, &TextFindBar::find_requested, this, &LeitorAcessivel::text_find_apply);
    connect(text_find_bar_, &TextFindBar::next_requested, this, &LeitorAcessivel::text_find_next);
    connect(text_find_bar_, &TextFindBar::previous_requested, this, &LeitorAcessivel::text_find_prev);
    connect(text_find_bar_, &TextFindBar::cleared, this, &LeitorAcessivel::text_find_clear);
    connect(text_find_bar_, &TextFindBar::highlight_all_changed, this, &LeitorAcessivel::text_find_set_highlight_all);

    content_stack_->addTab(text_tab, QString());
    source::utils::AccentTabStyle::set_tab_accent(
        content_stack_, text_tab, source::utils::AccentTabStyle::accent_blue());

    auto *pdf_tab = new QWidget(content_stack_);
    auto *pdf_layout = new QVBoxLayout(pdf_tab);
    pdf_layout->setContentsMargins(0, kContentPageTopMargin, 0, kContentPageBottomMargin);
    pdf_layout->addLayout(setup_pdf_toolbar());
    pdf_find_bar_ = new PDFFindBar(pdf_tab);
    pdf_layout->addWidget(pdf_find_bar_);
    pdf_view_ = new FileDropPDFView(pdf_tab);
    pdf_mouse_handler_ = new PDFMouseHandler(pdf_view_, this);
    pdf_layout->addWidget(pdf_view_, 1);
    content_stack_->addTab(pdf_tab, QString());
    source::utils::AccentTabStyle::set_tab_accent(
        content_stack_, pdf_tab, source::utils::AccentTabStyle::accent_red());
    layout->addWidget(content_stack_, 1);

    connect(pdf_view_, &FileDropPDFView::file_dropped, this, [this](const QString &path) {
        carregar_pdf(path, true);
    });
    connect(pdf_view_, &FileDropPDFView::current_page_changed, this, &LeitorAcessivel::pdf_update_page_ui);
    connect(pdf_view_, &QPdfView::zoomFactorChanged, this, [this](qreal) { pdf_refresh_zoom_ui(); });
    connect(pdf_view_, &QPdfView::zoomModeChanged, this, [this](QPdfView::ZoomMode) { pdf_refresh_zoom_ui(); });
    connect(pdf_view_, &PDFView::effective_zoom_changed, this, &LeitorAcessivel::pdf_refresh_zoom_ui);
    connect(pdf_find_bar_, &PDFFindBar::find_requested, this, &LeitorAcessivel::pdf_find_apply);
    connect(pdf_find_bar_, &PDFFindBar::next_requested, this, &LeitorAcessivel::pdf_find_next);
    connect(pdf_find_bar_, &PDFFindBar::previous_requested, this, &LeitorAcessivel::pdf_find_prev);
    connect(pdf_find_bar_, &PDFFindBar::cleared, this, &LeitorAcessivel::pdf_find_clear);
    connect(pdf_find_bar_, &PDFFindBar::highlight_all_changed, this, &LeitorAcessivel::pdf_find_set_highlight_all);

    auto *regua_layout = new QHBoxLayout();
    btn_regua_ = new QPushButton(this);
    btn_regua_->setCheckable(true);
    dica_regua_ = new QLabel(this);
    dica_regua_->setWordWrap(true);
    regua_layout->addWidget(btn_regua_);
    regua_layout->addWidget(dica_regua_, 1);
    layout->addLayout(regua_layout);
    connect(btn_regua_, &QPushButton::clicked, this, &LeitorAcessivel::toggle_regua_foco);

    auto *leitura_layout = new QHBoxLayout();
    btn_leitura_assistida_ = new QPushButton(this);
    btn_leitura_assistida_->setCheckable(true);
    btn_leitura_assistida_->setChecked(false);
    label_modo_leitura_ = new QLabel(this);
    combo_modo_leitura_ = new QComboBox(this);
    combo_modo_leitura_->addItems({tr("Palavras"), tr("Frases"), tr("Paragrafos")});
    dica_leitura_assistida_ = new QLabel(this);
    dica_leitura_assistida_->setWordWrap(true);
    leitura_layout->addWidget(btn_leitura_assistida_);
    leitura_layout->addWidget(label_modo_leitura_);
    leitura_layout->addWidget(combo_modo_leitura_);
    leitura_layout->addWidget(dica_leitura_assistida_, 1);
    layout->addLayout(leitura_layout);
    connect(btn_leitura_assistida_, &QPushButton::clicked, this, &LeitorAcessivel::on_toggle_leitura_assistida);
    connect(combo_modo_leitura_, &QComboBox::currentIndexChanged, this, &LeitorAcessivel::on_modo_leitura_changed);

    sc_find_toggle_ = new QShortcut(QKeySequence::Find, this);
    connect(sc_find_toggle_, &QShortcut::activated, this, [this] {
        if (content_stack_ != nullptr && content_stack_->currentIndex() == 1) {
            pdf_find_toggle();
        } else {
            text_find_toggle();
        }
    });

    sc_copy_pdf_ = new QShortcut(QKeySequence::Copy, this);
    connect(sc_copy_pdf_, &QShortcut::activated, this, [this] {
        if (content_stack_ != nullptr && content_stack_->currentIndex() == 1 && pdf_mouse_handler_ != nullptr) {
            pdf_mouse_handler_->copy_selection();
        }
    });

    if (content_stack_ != nullptr) {
        connect(content_stack_, &QTabWidget::currentChanged, this, [](int index) {
            salvar_aba_ativa_temp(index);
        });
    }

    atualizar_traducoes();
    restaurar_estado_carregado_temp();
}

}  // namespace source::plugin::Audiolivro
