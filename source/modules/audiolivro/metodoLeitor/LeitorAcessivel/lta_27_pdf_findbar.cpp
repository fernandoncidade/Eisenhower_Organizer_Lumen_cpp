#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_27_pdf_findbar.hpp"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace source::plugin::Audiolivro {

PDFFindBar::PDFFindBar(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    query_edit_ = new QLineEdit(this);
    query_edit_->setClearButtonEnabled(true);
    layout->addWidget(query_edit_, 1);

    case_box_ = new QCheckBox(this);
    words_box_ = new QCheckBox(this);
    highlight_box_ = new QCheckBox(this);
    highlight_box_->setChecked(true);
    layout->addWidget(case_box_);
    layout->addWidget(words_box_);
    layout->addWidget(highlight_box_);

    previous_button_ = new QPushButton(this);
    next_button_ = new QPushButton(this);
    close_button_ = new QPushButton(this);
    result_label_ = new QLabel(this);
    retranslate_ui();

    layout->addWidget(previous_button_);
    layout->addWidget(next_button_);
    layout->addWidget(result_label_);
    layout->addWidget(close_button_);

    connect(query_edit_, &QLineEdit::textChanged, this, [this](const QString &text) {
        emit find_requested(text, match_case(), whole_words());
    });
    connect(case_box_, &QCheckBox::toggled, this, [this] {
        emit find_requested(query(), match_case(), whole_words());
    });
    connect(words_box_, &QCheckBox::toggled, this, [this] {
        emit find_requested(query(), match_case(), whole_words());
    });
    connect(highlight_box_, &QCheckBox::toggled, this, &PDFFindBar::highlight_all_changed);
    connect(previous_button_, &QPushButton::clicked, this, &PDFFindBar::previous_requested);
    connect(next_button_, &QPushButton::clicked, this, &PDFFindBar::next_requested);
    connect(close_button_, &QPushButton::clicked, this, [this] {
        hide_bar();
        emit cleared();
    });

    hide();
}

QString PDFFindBar::query() const
{
    return query_edit_ != nullptr ? query_edit_->text() : QString();
}

bool PDFFindBar::match_case() const
{
    return case_box_ != nullptr && case_box_->isChecked();
}

bool PDFFindBar::whole_words() const
{
    return words_box_ != nullptr && words_box_->isChecked();
}

void PDFFindBar::show_bar()
{
    show();
    if (query_edit_ != nullptr) {
        query_edit_->setFocus(Qt::ShortcutFocusReason);
        query_edit_->selectAll();
    }
}

void PDFFindBar::hide_bar()
{
    hide();
}

void PDFFindBar::set_result_text(const QString &text)
{
    if (result_label_ != nullptr) {
        result_label_->setText(text);
    }
}

void PDFFindBar::retranslate_ui()
{
    if (case_box_ != nullptr) case_box_->setText(tr("Aa"));
    if (words_box_ != nullptr) words_box_->setText(tr("Palavra"));
    if (highlight_box_ != nullptr) highlight_box_->setText(tr("Todos"));
    if (previous_button_ != nullptr) previous_button_->setText(tr("Anterior"));
    if (next_button_ != nullptr) next_button_->setText(tr("Proximo"));
    if (close_button_ != nullptr) close_button_->setText(tr("Fechar"));
}

}  // namespace source::plugin::Audiolivro
