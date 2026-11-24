#pragma once

#include <QWidget>

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;

namespace source::plugin::Audiolivro {

class TextFindBar : public QWidget
{
    Q_OBJECT

public:
    explicit TextFindBar(QWidget *parent = nullptr);

    QString query() const;
    bool match_case() const;
    bool whole_words() const;
    void show_bar();
    void hide_bar();
    void set_result_text(const QString &text);
    void retranslate_ui();

signals:
    void find_requested(const QString &query, bool match_case, bool whole_words);
    void next_requested();
    void previous_requested();
    void cleared();
    void highlight_all_changed(bool enabled);

private:
    QLineEdit *query_edit_ = nullptr;
    QCheckBox *case_box_ = nullptr;
    QCheckBox *words_box_ = nullptr;
    QCheckBox *highlight_box_ = nullptr;
    QPushButton *previous_button_ = nullptr;
    QPushButton *next_button_ = nullptr;
    QPushButton *close_button_ = nullptr;
    QLabel *result_label_ = nullptr;
};

}  // namespace source::plugin::Audiolivro
