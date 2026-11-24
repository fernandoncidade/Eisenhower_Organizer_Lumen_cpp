#include "source/modules/audiolivro/audiolivro_font_settings_dialog.hpp"

#include "source/modules/audiolivro/audiolivro_settings.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QEvent>
#include <QFontComboBox>
#include <QFontDatabase>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>

namespace source::plugin::Audiolivro {

QString translate_audiolivro_font_style(const QString &raw_style)
{
    const QString trimmed = raw_style.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }
    const QString translated = QCoreApplication::translate("source::plugin::Audiolivro::FontSettingsDialog", trimmed.toUtf8().constData());
    if (!translated.isEmpty() && translated != trimmed) {
        return translated;
    }

    const QString qt_tr = QCoreApplication::translate("QFontDatabase", trimmed.toUtf8().constData());
    if (!qt_tr.isEmpty() && qt_tr != trimmed) {
        return qt_tr;
    }

    const QString language = QLocale().name();
    const bool is_english = language.startsWith(QStringLiteral("en"), Qt::CaseInsensitive);

    if (is_english) {
        return trimmed;
    }

    // ── Step 3: Exhaustive exact map for all known styles on this system ──
    static const QMap<QString, QString> exact_pt = {
        // ── Basic weights ──
        {QStringLiteral("Normal"), QStringLiteral("Normal")},
        {QStringLiteral("Regular"), QStringLiteral("Normal")},
        {QStringLiteral("Bold"), QStringLiteral("Negrito")},
        {QStringLiteral("Italic"), QStringLiteral("Itálico")},
        {QStringLiteral("Bold Italic"), QStringLiteral("Negrito Itálico")},
        {QStringLiteral("Oblique"), QStringLiteral("Oblíquo")},
        {QStringLiteral("Bold Oblique"), QStringLiteral("Negrito Oblíquo")},
        {QStringLiteral("BoldOblique"), QStringLiteral("Negrito Oblíquo")},
        {QStringLiteral("Slanted"), QStringLiteral("Inclinado")},
        {QStringLiteral("Upright"), QStringLiteral("Vertical")},
        {QStringLiteral("Roman"), QStringLiteral("Romano")},
        // ── Black ──
        {QStringLiteral("Black"), QStringLiteral("Preto")},
        {QStringLiteral("Black Italic"), QStringLiteral("Preto Itálico")},
        // ── Light ──
        {QStringLiteral("Light"), QStringLiteral("Fina")},
        {QStringLiteral("Light Italic"), QStringLiteral("Fina Itálico")},
        // ── Semilight ──
        {QStringLiteral("Semilight"), QStringLiteral("Semileve")},
        {QStringLiteral("SemiLight"), QStringLiteral("Semileve")},
        {QStringLiteral("Semi Light"), QStringLiteral("Semileve")},
        {QStringLiteral("Semi-Light"), QStringLiteral("Semileve")},
        {QStringLiteral("Semilight Italic"), QStringLiteral("Semileve Itálico")},
        {QStringLiteral("SemiLight Italic"), QStringLiteral("Semileve Itálico")},
        {QStringLiteral("Semi Light Italic"), QStringLiteral("Semileve Itálico")},
        {QStringLiteral("Semi-Light Italic"), QStringLiteral("Semileve Itálico")},
        // ── Medium ──
        {QStringLiteral("Medium"), QStringLiteral("Média")},
        {QStringLiteral("Medium Italic"), QStringLiteral("Média Itálico")},
        // ── Thin / Hairline / Hair ──
        {QStringLiteral("Thin"), QStringLiteral("Muito Fina")},
        {QStringLiteral("Thin Italic"), QStringLiteral("Muito Fina Itálico")},
        {QStringLiteral("Hairline"), QStringLiteral("Ultrafina")},
        {QStringLiteral("Hair"), QStringLiteral("Ultrafina")},
        {QStringLiteral("Hair Italic"), QStringLiteral("Ultrafina Itálico")},
        // ── ExtraLight ──
        {QStringLiteral("Extra Light"), QStringLiteral("Extra Fina")},
        {QStringLiteral("ExtraLight"), QStringLiteral("Extra Fina")},
        {QStringLiteral("Extralight"), QStringLiteral("Extra Fina")},
        {QStringLiteral("Extra-Light"), QStringLiteral("Extra Fina")},
        {QStringLiteral("Extra Light Italic"), QStringLiteral("Extra Fina Itálico")},
        {QStringLiteral("ExtraLight Italic"), QStringLiteral("Extra Fina Itálico")},
        {QStringLiteral("Extralight Italic"), QStringLiteral("Extra Fina Itálico")},
        {QStringLiteral("Extra-Light Italic"), QStringLiteral("Extra Fina Itálico")},
        // ── Semibold ──
        {QStringLiteral("Semibold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("SemiBold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Semi Bold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Semi-Bold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Semibold Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("SemiBold Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("Semi Bold Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("Semi-Bold Italic"), QStringLiteral("Seminegrito Itálico")},
        // ── Demi ──
        {QStringLiteral("Demi"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Demibold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Demi Bold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Demi-Bold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Demi Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("Demibold Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("Demi Bold Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("Demi-Bold Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("Demibold Roman"), QStringLiteral("Seminegrito Romano")},
        // ── Extra Bold ──
        {QStringLiteral("Extra Bold"), QStringLiteral("Extra Negrito")},
        {QStringLiteral("ExtraBold"), QStringLiteral("Extra Negrito")},
        {QStringLiteral("Extrabold"), QStringLiteral("Extra Negrito")},
        {QStringLiteral("Extra-Bold"), QStringLiteral("Extra Negrito")},
        {QStringLiteral("Extra Bold Italic"), QStringLiteral("Extra Negrito Itálico")},
        {QStringLiteral("ExtraBold Italic"), QStringLiteral("Extra Negrito Itálico")},
        {QStringLiteral("Extrabold Italic"), QStringLiteral("Extra Negrito Itálico")},
        {QStringLiteral("Extra-Bold Italic"), QStringLiteral("Extra Negrito Itálico")},
        // ── Ultra Light ──
        {QStringLiteral("Ultra Light"), QStringLiteral("Ultrafina")},
        {QStringLiteral("UltraLight"), QStringLiteral("Ultrafina")},
        {QStringLiteral("Ultralight"), QStringLiteral("Ultrafina")},
        {QStringLiteral("Ultra-Light"), QStringLiteral("Ultrafina")},
        {QStringLiteral("Ultra Light Italic"), QStringLiteral("Ultrafina Itálico")},
        {QStringLiteral("UltraLight Italic"), QStringLiteral("Ultrafina Itálico")},
        {QStringLiteral("Ultralight Italic"), QStringLiteral("Ultrafina Itálico")},
        {QStringLiteral("Ultra-Light Italic"), QStringLiteral("Ultrafina Itálico")},
        // ── Ultra Bold ──
        {QStringLiteral("Ultra Bold"), QStringLiteral("Ultra Negrito")},
        {QStringLiteral("UltraBold"), QStringLiteral("Ultra Negrito")},
        {QStringLiteral("Ultrabold"), QStringLiteral("Ultra Negrito")},
        {QStringLiteral("Ultra-Bold"), QStringLiteral("Ultra Negrito")},
        {QStringLiteral("Ultra Bold Italic"), QStringLiteral("Ultra Negrito Itálico")},
        {QStringLiteral("UltraBold Italic"), QStringLiteral("Ultra Negrito Itálico")},
        {QStringLiteral("Ultrabold Italic"), QStringLiteral("Ultra Negrito Itálico")},
        {QStringLiteral("Ultra-Bold Italic"), QStringLiteral("Ultra Negrito Itálico")},
        // ── Heavy ──
        {QStringLiteral("Heavy"), QStringLiteral("Pesada")},
        {QStringLiteral("Heavy Italic"), QStringLiteral("Pesada Itálico")},
        // ── Book ──
        {QStringLiteral("Book"), QStringLiteral("Livro")},
        {QStringLiteral("Book Italic"), QStringLiteral("Livro Itálico")},
        // ── Bold Outline ──
        {QStringLiteral("Bold Outline"), QStringLiteral("Negrito Contorno")},
        // ── Condensed ──
        {QStringLiteral("Condensed"), QStringLiteral("Condensada")},
        {QStringLiteral("Condensed Italic"), QStringLiteral("Condensada Itálico")},
        {QStringLiteral("Condensed Bold"), QStringLiteral("Negrito Condensado")},
        {QStringLiteral("Bold Condensed"), QStringLiteral("Negrito Condensado")},
        {QStringLiteral("Condensed Bold Italic"), QStringLiteral("Negrito Condensado Itálico")},
        {QStringLiteral("Bold Condensed Italic"), QStringLiteral("Negrito Condensado Itálico")},
        {QStringLiteral("Condensed Extra Bold"), QStringLiteral("Condensada Extra Negrito")},
        {QStringLiteral("Condensed Ultra Bold"), QStringLiteral("Condensada Ultra Negrito")},
        {QStringLiteral("Light Condensed"), QStringLiteral("Fina Condensada")},
        // ── SemiCondensed ──
        {QStringLiteral("SemiCondensed"), QStringLiteral("Semicondensada")},
        {QStringLiteral("Semi Condensed"), QStringLiteral("Semicondensada")},
        {QStringLiteral("Semi-Condensed"), QStringLiteral("Semicondensada")},
        {QStringLiteral("Bold SemiCondensed"), QStringLiteral("Negrito Semicondensado")},
        {QStringLiteral("Bold Semi Condensed"), QStringLiteral("Negrito Semicondensado")},
        {QStringLiteral("Light SemiCondensed"), QStringLiteral("Fina Semicondensada")},
        {QStringLiteral("Light Semi Condensed"), QStringLiteral("Fina Semicondensada")},
        {QStringLiteral("SemiBold Condensed"), QStringLiteral("Seminegrito Condensado")},
        {QStringLiteral("SemiBold SemiCondensed"), QStringLiteral("Seminegrito Semicondensado")},
        {QStringLiteral("SemiLight Condensed"), QStringLiteral("Semileve Condensada")},
        {QStringLiteral("SemiLight SemiCondensed"), QStringLiteral("Semileve Semicondensada")},
        // ── Ext / Extra / Ultra Condensed (abbreviations) ──
        {QStringLiteral("Ext Condensed Bold"), QStringLiteral("Extra Condensada Negrito")},
        {QStringLiteral("Extra Condensed"), QStringLiteral("Extra Condensada")},
        {QStringLiteral("Extra-Condensed"), QStringLiteral("Extra Condensada")},
        {QStringLiteral("Ultra Condensed"), QStringLiteral("Ultra Condensada")},
        {QStringLiteral("Ultra-Condensed"), QStringLiteral("Ultra Condensada")},
        // ── Cond (abbreviation) ──
        {QStringLiteral("Cond Demi"), QStringLiteral("Condensada Seminegrito")},
        {QStringLiteral("Cond Medium"), QStringLiteral("Condensada Média")},
        // ── Compressed ──
        {QStringLiteral("Compressed"), QStringLiteral("Comprimida")},
        {QStringLiteral("Compressed Italic"), QStringLiteral("Comprimida Itálico")},
        {QStringLiteral("Poster Compressed"), QStringLiteral("Poster Comprimida")},
        // ── Narrow ──
        {QStringLiteral("Narrow"), QStringLiteral("Estreita")},
        {QStringLiteral("Narrow Italic"), QStringLiteral("Estreita Itálico")},
        {QStringLiteral("Narrow Bold"), QStringLiteral("Negrito Estreito")},
        {QStringLiteral("Narrow Bold Italic"), QStringLiteral("Negrito Estreito Itálico")},
        // ── Expanded / Extended ──
        {QStringLiteral("Expanded"), QStringLiteral("Expandida")},
        {QStringLiteral("Expanded Italic"), QStringLiteral("Expandida Itálico")},
        {QStringLiteral("Extended"), QStringLiteral("Estendida")},
        {QStringLiteral("Extended Italic"), QStringLiteral("Estendida Itálico")},
        {QStringLiteral("SemiExpanded Italic"), QStringLiteral("Semiexpandida Itálico")},
        {QStringLiteral("Semi Expanded"), QStringLiteral("Semiexpandida")},
        {QStringLiteral("Semi-Expanded"), QStringLiteral("Semiexpandida")},
        {QStringLiteral("Extra Expanded"), QStringLiteral("Extra Expandida")},
        {QStringLiteral("Extra-Expanded"), QStringLiteral("Extra Expandida")},
        {QStringLiteral("Ultra Expanded"), QStringLiteral("Ultra Expandida")},
        {QStringLiteral("Ultra-Expanded"), QStringLiteral("Ultra Expandida")},
        // ── Optical sizes (Sitka) ──
        {QStringLiteral("Small"), QStringLiteral("Pequena")},
        {QStringLiteral("Small Italic"), QStringLiteral("Pequena Itálico")},
        {QStringLiteral("Small Bold"), QStringLiteral("Pequena Negrito")},
        {QStringLiteral("Small Bold Italic"), QStringLiteral("Pequena Negrito Itálico")},
        {QStringLiteral("Small Semibold"), QStringLiteral("Pequena Seminegrito")},
        {QStringLiteral("Small Semibold Italic"), QStringLiteral("Pequena Seminegrito Itálico")},
        {QStringLiteral("Small Light"), QStringLiteral("Pequena Fina")},
        {QStringLiteral("Small Semilight"), QStringLiteral("Pequena Semileve")},
        {QStringLiteral("Text"), QStringLiteral("Texto")},
        {QStringLiteral("Text Italic"), QStringLiteral("Texto Itálico")},
        {QStringLiteral("Text Bold"), QStringLiteral("Texto Negrito")},
        {QStringLiteral("Text Bold Italic"), QStringLiteral("Texto Negrito Itálico")},
        {QStringLiteral("Text Semibold"), QStringLiteral("Texto Seminegrito")},
        {QStringLiteral("Text Semibold Italic"), QStringLiteral("Texto Seminegrito Itálico")},
        {QStringLiteral("Text Light"), QStringLiteral("Texto Fina")},
        {QStringLiteral("Text Semilight"), QStringLiteral("Texto Semileve")},
        {QStringLiteral("Heading"), QStringLiteral("Título")},
        {QStringLiteral("Heading Italic"), QStringLiteral("Título Itálico")},
        {QStringLiteral("Heading Bold"), QStringLiteral("Título Negrito")},
        {QStringLiteral("Heading Bold Italic"), QStringLiteral("Título Negrito Itálico")},
        {QStringLiteral("Heading Semibold"), QStringLiteral("Título Seminegrito")},
        {QStringLiteral("Heading Semibold Italic"), QStringLiteral("Título Seminegrito Itálico")},
        {QStringLiteral("Subheading"), QStringLiteral("Subtítulo")},
        {QStringLiteral("Subheading Italic"), QStringLiteral("Subtítulo Itálico")},
        {QStringLiteral("Subheading Bold"), QStringLiteral("Subtítulo Negrito")},
        {QStringLiteral("Subheading Bold Italic"), QStringLiteral("Subtítulo Negrito Itálico")},
        {QStringLiteral("Subheading Semibold"), QStringLiteral("Subtítulo Seminegrito")},
        {QStringLiteral("Subheading Semibold Italic"), QStringLiteral("Subtítulo Seminegrito Itálico")},
        {QStringLiteral("Display"), QStringLiteral("Exibição")},
        {QStringLiteral("Display Italic"), QStringLiteral("Exibição Itálico")},
        {QStringLiteral("Display Bold"), QStringLiteral("Exibição Negrito")},
        {QStringLiteral("Display Bold Italic"), QStringLiteral("Exibição Negrito Itálico")},
        {QStringLiteral("Display Semibold"), QStringLiteral("Exibição Seminegrito")},
        {QStringLiteral("Display Semibold Italic"), QStringLiteral("Exibição Seminegrito Itálico")},
        {QStringLiteral("Display Light"), QStringLiteral("Exibição Fina")},
        {QStringLiteral("Display Semilight"), QStringLiteral("Exibição Semileve")},
        {QStringLiteral("Banner"), QStringLiteral("Faixa")},
        {QStringLiteral("Banner Italic"), QStringLiteral("Faixa Itálico")},
        {QStringLiteral("Banner Bold"), QStringLiteral("Faixa Negrito")},
        {QStringLiteral("Banner Bold Italic"), QStringLiteral("Faixa Negrito Itálico")},
        {QStringLiteral("Banner Semibold"), QStringLiteral("Faixa Seminegrito")},
        {QStringLiteral("Banner Semibold Italic"), QStringLiteral("Faixa Seminegrito Itálico")},
        {QStringLiteral("Caption"), QStringLiteral("Legenda")},
        {QStringLiteral("Poster"), QStringLiteral("Poster")},
        {QStringLiteral("Headline"), QStringLiteral("Manchete")},
        {QStringLiteral("Subhead"), QStringLiteral("Subtítulo")},
        // ── 18pt (Noto Sans CJK special) ──
        {QStringLiteral("18pt"), QStringLiteral("18pt")},
        {QStringLiteral("18pt Bold"), QStringLiteral("18pt Negrito")},
        {QStringLiteral("18pt Light"), QStringLiteral("18pt Fina")},
        {QStringLiteral("18pt Medium"), QStringLiteral("18pt Média")},
        {QStringLiteral("18pt Thin"), QStringLiteral("18pt Muito Fina")},
    };

    auto it = exact_pt.constFind(trimmed);
    if (it != exact_pt.constEnd()) {
        return it.value();
    }
    for (auto map_it = exact_pt.constBegin(); map_it != exact_pt.constEnd(); ++map_it) {
        if (map_it.key().compare(trimmed, Qt::CaseInsensitive) == 0) {
            return map_it.value();
        }
    }

    // ── Step 4: Component-based regex tokenizer for unknown compound styles ──
    struct ReplacementRule {
        const char *en;
        const char *pt;
    };
    static const ReplacementRule rules[] = {
        {"Ext Condensed", "Extra Condensada"},
        {"Ultra Light", "Ultrafina"},
        {"Ultra-Light", "Ultrafina"},
        {"Ultralight", "Ultrafina"},
        {"Extra Light", "Extra Fina"},
        {"Extra-Light", "Extra Fina"},
        {"ExtraLight", "Extra Fina"},
        {"Extralight", "Extra Fina"},
        {"Ultra Bold", "Ultra Negrito"},
        {"Ultra-Bold", "Ultra Negrito"},
        {"UltraBold", "Ultra Negrito"},
        {"Ultrabold", "Ultra Negrito"},
        {"Extra Bold", "Extra Negrito"},
        {"Extra-Bold", "Extra Negrito"},
        {"ExtraBold", "Extra Negrito"},
        {"Extrabold", "Extra Negrito"},
        {"Semi Bold", "Seminegrito"},
        {"Semi-Bold", "Seminegrito"},
        {"SemiBold", "Seminegrito"},
        {"Semibold", "Seminegrito"},
        {"Semi Light", "Semileve"},
        {"Semi-Light", "Semileve"},
        {"SemiLight", "Semileve"},
        {"Semilight", "Semileve"},
        {"Demi Bold", "Seminegrito"},
        {"Demi-Bold", "Seminegrito"},
        {"Demibold", "Seminegrito"},
        {"Semi Condensed", "Semicondensada"},
        {"Semi-Condensed", "Semicondensada"},
        {"SemiCondensed", "Semicondensada"},
        {"Semicondensed", "Semicondensada"},
        {"Extra Condensed", "Extra Condensada"},
        {"Extra-Condensed", "Extra Condensada"},
        {"Extracondensed", "Extra Condensada"},
        {"Ultra Condensed", "Ultra Condensada"},
        {"Ultra-Condensed", "Ultra Condensada"},
        {"Ultracondensed", "Ultra Condensada"},
        {"Semi Expanded", "Semiexpandida"},
        {"Semi-Expanded", "Semiexpandida"},
        {"SemiExpanded", "Semiexpandida"},
        {"Extra Expanded", "Extra Expandida"},
        {"Extra-Expanded", "Extra Expandida"},
        {"Extraexpanded", "Extra Expandida"},
        {"Ultra Expanded", "Ultra Expandida"},
        {"Ultra-Expanded", "Ultra Expandida"},
        {"Ultraexpanded", "Ultra Expandida"},
        {"Bold Outline", "Negrito Contorno"},
        {"Cond", "Condensada"},
        {"Ext", "Extra"},
        {"Subheading", "Subtítulo"},
        {"Thin", "Muito Fina"},
        {"Hairline", "Ultrafina"},
        {"Hair", "Ultrafina"},
        {"Light", "Fina"},
        {"Medium", "Média"},
        {"Regular", "Normal"},
        {"Normal", "Normal"},
        {"Demi", "Seminegrito"},
        {"Bold", "Negrito"},
        {"Black", "Preto"},
        {"Heavy", "Pesada"},
        {"Book", "Livro"},
        {"Roman", "Romano"},
        {"Italic", "Itálico"},
        {"Oblique", "Oblíquo"},
        {"Slanted", "Inclinado"},
        {"Upright", "Vertical"},
        {"Outline", "Contorno"},
        {"Condensed", "Condensada"},
        {"Compressed", "Comprimida"},
        {"Narrow", "Estreita"},
        {"Expanded", "Expandida"},
        {"Extended", "Estendida"},
        {"Small", "Pequena"},
        {"Text", "Texto"},
        {"Display", "Exibição"},
        {"Banner", "Faixa"},
        {"Heading", "Título"},
        {"Subhead", "Subtítulo"},
        {"Caption", "Legenda"},
        {"Poster", "Poster"},
        {"Headline", "Manchete"},
    };

    QString result = trimmed;
    for (const auto &r : rules) {
        const QString pattern = QString::fromUtf8(r.en);
        const QString replacement = QString::fromUtf8(r.pt);
        result.replace(QRegularExpression(QStringLiteral("\\b") + QRegularExpression::escape(pattern) + QStringLiteral("\\b"),
                                           QRegularExpression::CaseInsensitiveOption),
                       replacement);
    }

    return result;
}

FontSettingsDialog::FontSettingsDialog(const QFont &initial_font, QWidget *parent)
    : QDialog(parent)
{
    setModal(true);
    setMinimumWidth(480);

    auto *main_layout = new QVBoxLayout(this);
    auto *form_layout = new QGridLayout();
    form_layout->setColumnStretch(1, 1);

    family_label_ = new QLabel(this);
    family_combo_ = new QFontComboBox(this);

    style_label_ = new QLabel(this);
    style_combo_ = new QComboBox(this);

    size_label_ = new QLabel(this);
    size_spin_ = new QSpinBox(this);
    size_spin_->setRange(6, 72);
    size_spin_->setSuffix(QStringLiteral(" pt"));

    format_label_ = new QLabel(this);
    bold_check_ = new QCheckBox(this);
    italic_check_ = new QCheckBox(this);
    underline_check_ = new QCheckBox(this);
    strike_out_check_ = new QCheckBox(this);

    auto *format_layout = new QHBoxLayout();
    format_layout->addWidget(bold_check_);
    format_layout->addWidget(italic_check_);
    format_layout->addWidget(underline_check_);
    format_layout->addWidget(strike_out_check_);
    format_layout->addStretch();

    form_layout->addWidget(family_label_, 0, 0);
    form_layout->addWidget(family_combo_, 0, 1);
    form_layout->addWidget(style_label_, 1, 0);
    form_layout->addWidget(style_combo_, 1, 1);
    form_layout->addWidget(size_label_, 2, 0);
    form_layout->addWidget(size_spin_, 2, 1);
    form_layout->addWidget(format_label_, 3, 0);
    form_layout->addLayout(format_layout, 3, 1);
    main_layout->addLayout(form_layout);

    preview_title_label_ = new QLabel(this);
    main_layout->addWidget(preview_title_label_);

    auto *preview_frame = new QFrame(this);
    preview_frame->setFrameShape(QFrame::StyledPanel);
    preview_frame->setMinimumHeight(110);
    auto *preview_layout = new QVBoxLayout(preview_frame);
    preview_label_ = new QLabel(preview_frame);
    preview_label_->setWordWrap(true);
    preview_label_->setAlignment(Qt::AlignCenter);
    preview_layout->addWidget(preview_label_, 1);
    main_layout->addWidget(preview_frame);

    auto *button_layout = new QHBoxLayout();
    restore_button_ = new QPushButton(this);
    ok_button_ = new QPushButton(this);
    cancel_button_ = new QPushButton(this);
    button_layout->addWidget(restore_button_);
    button_layout->addStretch();
    button_layout->addWidget(ok_button_);
    button_layout->addWidget(cancel_button_);
    main_layout->addLayout(button_layout);

    connect(family_combo_, &QFontComboBox::currentFontChanged, this, [this](const QFont &) {
        update_style_options(QString(), true);
        update_preview();
    });
    connect(style_combo_, &QComboBox::currentTextChanged, this, [this](const QString &) { update_preview(); });
    connect(size_spin_, &QSpinBox::valueChanged, this, [this](int) { update_preview(); });
    connect(bold_check_, &QCheckBox::toggled, this, [this](bool) { update_preview(); });
    connect(italic_check_, &QCheckBox::toggled, this, [this](bool) { update_preview(); });
    connect(underline_check_, &QCheckBox::toggled, this, [this](bool) { update_preview(); });
    connect(strike_out_check_, &QCheckBox::toggled, this, [this](bool) { update_preview(); });
    connect(restore_button_, &QPushButton::clicked, this, [this]() { restore_defaults(); });
    connect(ok_button_, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancel_button_, &QPushButton::clicked, this, &QDialog::reject);

    set_selected_font(initial_font);
    retranslate_ui();
    update_preview();
}

QFont FontSettingsDialog::selected_font() const
{
    QFont font = family_combo_ != nullptr ? family_combo_->currentFont() : fonte_audiolivro_padrao();
    font.setPointSize(size_spin_ != nullptr ? size_spin_->value() : 10);
    if (style_combo_ != nullptr) {
        const QString raw_style = style_combo_->currentData(Qt::UserRole).toString().trimmed();
        const QString style = !raw_style.isEmpty() ? raw_style : style_combo_->currentText().trimmed();
        if (!style.isEmpty()) {
            font.setStyleName(style);
        }
    }
    font.setBold(bold_check_ != nullptr && bold_check_->isChecked());
    font.setItalic(italic_check_ != nullptr && italic_check_->isChecked());
    font.setUnderline(underline_check_ != nullptr && underline_check_->isChecked());
    font.setStrikeOut(strike_out_check_ != nullptr && strike_out_check_->isChecked());
    return font;
}

void FontSettingsDialog::changeEvent(QEvent *event)
{
    if (event != nullptr && event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
    QDialog::changeEvent(event);
}

void FontSettingsDialog::set_selected_font(const QFont &font)
{
    if (family_combo_ != nullptr) {
        const QSignalBlocker blocker(family_combo_);
        family_combo_->setCurrentFont(font);
    }
    update_style_options(font.styleName(), false);

    if (size_spin_ != nullptr) {
        const QSignalBlocker blocker(size_spin_);
        size_spin_->setValue(qBound(6, font.pointSize() > 0 ? font.pointSize() : 10, 72));
    }
    if (bold_check_ != nullptr) {
        const QSignalBlocker blocker(bold_check_);
        bold_check_->setChecked(font.bold());
    }
    if (italic_check_ != nullptr) {
        const QSignalBlocker blocker(italic_check_);
        italic_check_->setChecked(font.italic());
    }
    if (underline_check_ != nullptr) {
        const QSignalBlocker blocker(underline_check_);
        underline_check_->setChecked(font.underline());
    }
    if (strike_out_check_ != nullptr) {
        const QSignalBlocker blocker(strike_out_check_);
        strike_out_check_->setChecked(font.strikeOut());
    }
}

void FontSettingsDialog::update_style_options(const QString &preferred_style, bool keep_current_style_when_empty)
{
    if (family_combo_ == nullptr || style_combo_ == nullptr) {
        return;
    }

    const QString trimmed_preferred_style = preferred_style.trimmed();
    const QString current_raw_style = style_combo_->currentData(Qt::UserRole).toString().trimmed();
    const QString target_style = trimmed_preferred_style.isEmpty() && keep_current_style_when_empty
                                     ? (current_raw_style.isEmpty() ? style_combo_->currentText().trimmed() : current_raw_style)
                                     : trimmed_preferred_style;
    const QString family = family_combo_->currentFont().family();
    QFontDatabase database;
    QStringList styles = database.styles(family);
    if (styles.isEmpty()) {
        styles << QStringLiteral("Normal");
    }

    const QSignalBlocker blocker(style_combo_);
    style_combo_->clear();
    for (const QString &raw_style : styles) {
        style_combo_->addItem(translate_audiolivro_font_style(raw_style), raw_style);
    }

    int target_index = -1;
    if (!target_style.isEmpty()) {
        for (int i = 0; i < style_combo_->count(); ++i) {
            const QString item_raw = style_combo_->itemData(i, Qt::UserRole).toString();
            if (item_raw == target_style || style_combo_->itemText(i) == target_style) {
                target_index = i;
                break;
            }
        }
    }

    if (target_index >= 0) {
        style_combo_->setCurrentIndex(target_index);
    } else {
        int regular_index = -1;
        for (int i = 0; i < style_combo_->count(); ++i) {
            const QString item_raw = style_combo_->itemData(i, Qt::UserRole).toString();
            if (item_raw == QStringLiteral("Regular") || item_raw == QStringLiteral("Normal")) {
                regular_index = i;
                break;
            }
        }
        style_combo_->setCurrentIndex(regular_index >= 0 ? regular_index : 0);
    }
}

void FontSettingsDialog::restore_defaults()
{
    set_selected_font(fonte_audiolivro_padrao());
    update_preview();
}

void FontSettingsDialog::update_preview()
{
    if (preview_label_ == nullptr) {
        return;
    }

    preview_label_->setFont(selected_font());
    preview_label_->setText(tr("O texto do Audiolivro usara esta fonte."));
}

void FontSettingsDialog::retranslate_ui()
{
    setWindowTitle(tr("Configuracoes de Fonte"));
    if (family_label_ != nullptr) family_label_->setText(tr("Tipo de fonte:"));
    if (style_label_ != nullptr) style_label_->setText(tr("Estilo da fonte:"));
    if (size_label_ != nullptr) size_label_->setText(tr("Tamanho:"));
    if (format_label_ != nullptr) format_label_->setText(tr("Formatacao:"));
    if (bold_check_ != nullptr) bold_check_->setText(tr("Negrito"));
    if (italic_check_ != nullptr) italic_check_->setText(tr("Italico"));
    if (underline_check_ != nullptr) underline_check_->setText(tr("Sublinhado"));
    if (strike_out_check_ != nullptr) strike_out_check_->setText(tr("Tachado"));
    if (preview_title_label_ != nullptr) preview_title_label_->setText(tr("Previa"));
    if (restore_button_ != nullptr) restore_button_->setText(tr("Restaurar"));
    if (ok_button_ != nullptr) ok_button_->setText(tr("OK"));
    if (cancel_button_ != nullptr) cancel_button_->setText(tr("Cancelar"));

    if (style_combo_ != nullptr) {
        const QSignalBlocker blocker(style_combo_);
        for (int i = 0; i < style_combo_->count(); ++i) {
            const QString raw_style = style_combo_->itemData(i, Qt::UserRole).toString();
            if (!raw_style.isEmpty()) {
                style_combo_->setItemText(i, translate_audiolivro_font_style(raw_style));
            }
        }
    }
    update_preview();
}

}  // namespace source::plugin::Audiolivro
