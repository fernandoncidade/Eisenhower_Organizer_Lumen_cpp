#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_22_pdf_toolbar.hpp"

#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_26_pdf_view.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_38_FileDropPDFView.hpp"

#include <QComboBox>
#include <QCoreApplication>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QtMath>

namespace {

constexpr const char *kLeitorAcessivelTranslationContext = "source::plugin::Audiolivro::LeitorAcessivel";

struct PaperSpec {
    const char *name;
    double width_mm;
    double height_mm;
};

QString paper_type_for_mm(const QSizeF &mm)
{
    if (!mm.isValid() || mm.isEmpty()) {
        return QCoreApplication::translate(kLeitorAcessivelTranslationContext, "Desconhecido");
    }

    static const PaperSpec kSpecs[] = {
        {"A0", 841, 1189},
        {"A1", 594, 841},
        {"A2", 420, 594},
        {"A3", 297, 420},
        {"A4", 210, 297},
        {"A5", 148, 210},
        {"A6", 105, 148},
        {"Letter", 216, 279},
        {"Legal", 216, 356},
        {"Executive", 184, 267},
        {"Tabloid", 279, 432},
    };

    double best_score = 1e9;
    QString best_name = QCoreApplication::translate(kLeitorAcessivelTranslationContext, "Personalizado");
    for (const PaperSpec &spec : kSpecs) {
        const double diff_portrait = qAbs(mm.width() - spec.width_mm) + qAbs(mm.height() - spec.height_mm);
        const double diff_landscape = qAbs(mm.width() - spec.height_mm) + qAbs(mm.height() - spec.width_mm);
        const double diff = qMin(diff_portrait, diff_landscape);
        if (diff < best_score) {
            best_score = diff;
            best_name = QString::fromLatin1(spec.name);
        }
    }

    return (best_score <= 6.0) ? best_name : QCoreApplication::translate(kLeitorAcessivelTranslationContext, "Personalizado");
}

QString orientation_for_mm(const QSizeF &mm)
{
    return (mm.width() >= mm.height()) ? QCoreApplication::translate(kLeitorAcessivelTranslationContext, "Paisagem")
                                       : QCoreApplication::translate(kLeitorAcessivelTranslationContext, "Retrato");
}

QString format_mm(const QSizeF &mm)
{
    return QStringLiteral("%1 x %2 mm")
        .arg(qRound(mm.width()))
        .arg(qRound(mm.height()));
}

}  // namespace

namespace source::plugin::Audiolivro {

QHBoxLayout *LeitorAcessivel::setup_pdf_toolbar()
{
    auto *toolbar = new QHBoxLayout();
    btn_pdf_prev_ = new QPushButton(tr("◀️ Anterior"));
    btn_pdf_next_ = new QPushButton(tr("▶️ Próxima"));
    spin_page_ = new QSpinBox();
    spin_page_->setMinimum(1);
    spin_page_->setMaximum(1);
    label_pdf_page_ = new QLabel(tr("/ 0"));
    label_pdf_info_ = new QLabel();
    btn_pdf_zoom_out_ = new QPushButton(tr("🔍−"));
    btn_pdf_zoom_in_ = new QPushButton(tr("🔍+"));
    combo_pdf_zoom_ = new QComboBox();
    combo_pdf_zoom_->addItems({QStringLiteral("25%"), QStringLiteral("50%"), QStringLiteral("75%"),
                               QStringLiteral("100%"), QStringLiteral("125%"), QStringLiteral("150%"),
                               QStringLiteral("175%"), QStringLiteral("200%"), QStringLiteral("225%"),
                               QStringLiteral("250%"), QStringLiteral("275%"), QStringLiteral("300%"),
                               tr("Largura"), tr("Pagina")});
    combo_pdf_zoom_->setCurrentText(QStringLiteral("100%"));
    combo_pdf_zoom_->setEditable(true);
    combo_pdf_zoom_->setInsertPolicy(QComboBox::NoInsert);
    label_pdf_zoom_ = new QLabel(tr("Zoom:"));
    btn_pdf_fit_width_ = new QPushButton(tr("📄 Largura"));
    btn_pdf_fit_page_ = new QPushButton(tr("📃 Página"));
    btn_pdf_hand_ = new QPushButton(tr("✋ Mão"));
    btn_pdf_select_ = new QPushButton(tr("📝 Seleção"));
    btn_pdf_hand_->setCheckable(true);
    btn_pdf_select_->setCheckable(true);
    btn_pdf_find_ = new QPushButton(tr("🔎 Buscar"));

    toolbar->addWidget(btn_pdf_prev_);
    toolbar->addWidget(btn_pdf_next_);
    toolbar->addWidget(spin_page_);
    toolbar->addWidget(label_pdf_page_);
    toolbar->addSpacing(12);
    toolbar->addWidget(label_pdf_zoom_);
    toolbar->addWidget(btn_pdf_zoom_out_);
    toolbar->addWidget(combo_pdf_zoom_);
    toolbar->addWidget(btn_pdf_zoom_in_);
    toolbar->addWidget(btn_pdf_fit_width_);
    toolbar->addWidget(btn_pdf_fit_page_);
    toolbar->addSpacing(12);
    toolbar->addWidget(btn_pdf_hand_);
    toolbar->addWidget(btn_pdf_select_);
    toolbar->addStretch();
    toolbar->addWidget(btn_pdf_find_);
    toolbar->addSpacing(12);
    toolbar->addWidget(label_pdf_info_, 1);

    connect(btn_pdf_prev_, &QPushButton::clicked, this, &LeitorAcessivel::pdf_prev_page);
    connect(btn_pdf_next_, &QPushButton::clicked, this, &LeitorAcessivel::pdf_next_page);
    connect(spin_page_, qOverload<int>(&QSpinBox::valueChanged), this, &LeitorAcessivel::pdf_goto_page);
    connect(btn_pdf_zoom_in_, &QPushButton::clicked, this, &LeitorAcessivel::pdf_zoom_in);
    connect(btn_pdf_zoom_out_, &QPushButton::clicked, this, &LeitorAcessivel::pdf_zoom_out);
    connect(btn_pdf_fit_width_, &QPushButton::clicked, this, &LeitorAcessivel::pdf_zoom_fit_width);
    connect(btn_pdf_fit_page_, &QPushButton::clicked, this, &LeitorAcessivel::pdf_zoom_fit_page);
    connect(btn_pdf_hand_, &QPushButton::clicked, this, &LeitorAcessivel::pdf_set_hand_mode);
    connect(btn_pdf_select_, &QPushButton::clicked, this, &LeitorAcessivel::pdf_set_selection_mode);
    connect(btn_pdf_find_, &QPushButton::clicked, this, &LeitorAcessivel::pdf_find_show);
    connect(combo_pdf_zoom_, &QComboBox::currentTextChanged, this, &LeitorAcessivel::pdf_set_zoom_value);

    pdf_enable_toolbar(false);
    return toolbar;
}

void LeitorAcessivel::pdf_update_page_ui(int page_index, int total)
{
    if (pdf_view_ != nullptr && total < 0) {
        total = pdf_view_->page_count();
    }

    if (spin_page_ != nullptr) {
        QSignalBlocker blocker(spin_page_);
        spin_page_->setMaximum(qMax(1, total));
        spin_page_->setValue(qBound(1, page_index + 1, qMax(1, total)));
    }
    if (label_pdf_page_ != nullptr) {
        label_pdf_page_->setText(tr("/ %1").arg(total));
    }

    if (label_pdf_info_ != nullptr) {
        QString info_text;
        QString tooltip;

        if (pdf_view_ != nullptr && total > 0) {
            const QFileInfo file_info(pdf_view_->pdf_path());
            const int safe_page = qBound(0, page_index, total - 1);
            const QSizeF points = pdf_view_->page_point_size(safe_page);
            if (points.isValid()) {
                const QSizeF mm(points.width() * 25.4 / 72.0, points.height() * 25.4 / 72.0);
                const QString paper = paper_type_for_mm(mm);
                const QString orientation = orientation_for_mm(mm);
                const QString size_mm = format_mm(mm);
                const QString file_name = file_info.fileName();

                info_text = QStringLiteral("%1 | %2 %3 | %4")
                                .arg(file_name, paper, orientation, size_mm);

                tooltip = tr("Arquivo: %1\nFolha: %2\nOrientacao: %3\nTamanho: %4")
                              .arg(file_name, paper, orientation, size_mm);

                const QString title = pdf_view_->meta_data(QPdfDocument::MetaDataField::Title).trimmed();
                if (!title.isEmpty()) {
                    tooltip += tr("\nTitulo: %1").arg(title);
                }
                const QString author = pdf_view_->meta_data(QPdfDocument::MetaDataField::Author).trimmed();
                if (!author.isEmpty()) {
                    tooltip += tr("\nAutor: %1").arg(author);
                }
            } else {
                info_text = file_info.fileName();
                tooltip = tr("Arquivo: %1").arg(file_info.fileName());
            }
        }

        label_pdf_info_->setText(info_text);
        label_pdf_info_->setToolTip(tooltip);
    }

    if (pending_pdf_text_refresh_) {
        sync_text_from_pdf();
    }
    pdf_refresh_zoom_ui();
}

void LeitorAcessivel::pdf_next_page()
{
    if (pdf_view_ != nullptr) {
        pdf_view_->goto_page(pdf_view_->current_page() + 1);
    }
}

void LeitorAcessivel::pdf_prev_page()
{
    if (pdf_view_ != nullptr) {
        pdf_view_->goto_page(pdf_view_->current_page() - 1);
    }
}

void LeitorAcessivel::pdf_goto_page(int page_num)
{
    if (pdf_view_ != nullptr) {
        pdf_view_->goto_page(page_num - 1);
    }
}

void LeitorAcessivel::pdf_zoom_in()
{
    if (pdf_view_ != nullptr) {
        pdf_view_->zoom_in();
        pdf_refresh_zoom_ui();
    }
}

void LeitorAcessivel::pdf_zoom_out()
{
    if (pdf_view_ != nullptr) {
        pdf_view_->zoom_out();
        pdf_refresh_zoom_ui();
    }
}

void LeitorAcessivel::pdf_zoom_fit_width()
{
    if (pdf_view_ != nullptr) {
        pdf_view_->fit_width();
        pdf_refresh_zoom_ui();
    }
}

void LeitorAcessivel::pdf_zoom_fit_page()
{
    if (pdf_view_ != nullptr) {
        pdf_view_->fit_page();
        pdf_refresh_zoom_ui();
    }
}

void LeitorAcessivel::pdf_set_hand_mode()
{
    if (pdf_view_ == nullptr) {
        return;
    }

    const bool enable_hand = btn_pdf_hand_ != nullptr && btn_pdf_hand_->isChecked();
    pdf_view_->set_text_selection_enabled(false);
    pdf_view_->viewport()->setCursor(enable_hand ? Qt::OpenHandCursor : Qt::ArrowCursor);
    if (btn_pdf_hand_ != nullptr) btn_pdf_hand_->setChecked(enable_hand);
    if (btn_pdf_select_ != nullptr) btn_pdf_select_->setChecked(false);
}

void LeitorAcessivel::pdf_set_selection_mode()
{
    if (pdf_view_ == nullptr) {
        return;
    }

    const bool enable_selection = btn_pdf_select_ != nullptr && btn_pdf_select_->isChecked();
    pdf_view_->set_text_selection_enabled(enable_selection);
    if (!enable_selection) {
        pdf_view_->clear_text_selection();
    }
    if (btn_pdf_hand_ != nullptr) btn_pdf_hand_->setChecked(false);
    if (btn_pdf_select_ != nullptr) btn_pdf_select_->setChecked(enable_selection);
}

void LeitorAcessivel::pdf_enable_toolbar(bool enable)
{
    const QList<QWidget *> widgets = {btn_pdf_prev_, btn_pdf_next_, spin_page_, btn_pdf_zoom_out_, btn_pdf_zoom_in_,
                                      combo_pdf_zoom_, btn_pdf_fit_width_, btn_pdf_fit_page_, btn_pdf_hand_,
                                      btn_pdf_select_, btn_pdf_find_};
    for (QWidget *widget : widgets) {
        if (widget != nullptr) {
            widget->setEnabled(enable);
        }
    }
}

void LeitorAcessivel::pdf_refresh_zoom_ui()
{
    if (pdf_view_ == nullptr || combo_pdf_zoom_ == nullptr) {
        return;
    }

    const QSignalBlocker blocker(combo_pdf_zoom_);
    const double zoom = pdf_view_->effective_zoom_factor();
    if (qIsFinite(zoom) && zoom > 0.0) {
        combo_pdf_zoom_->setEditText(QStringLiteral("%1%").arg(qRound(zoom * 100.0)));
    }
}

void LeitorAcessivel::pdf_set_zoom_value(const QString &text)
{
    if (pdf_view_ == nullptr) {
        return;
    }
    if (text == tr("Largura")) {
        pdf_view_->fit_width();
    } else if (text == tr("Pagina")) {
        pdf_view_->fit_page();
    } else {
        QString value = text;
        value.remove(QLatin1Char('%'));
        bool ok = false;
        const double percent = value.toDouble(&ok);
        if (ok) {
            pdf_view_->set_zoom_factor(percent / 100.0);
        }
    }
    pdf_refresh_zoom_ui();
}

}  // namespace source::plugin::Audiolivro
