#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_04_ajustar_fonte.hpp"

#include "source/modules/audiolivro/audiolivro_settings.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_37_FileDropEdit.hpp"
#include "source/utils/LinkedTitleUnderlineDelegate.hpp"

#include <QApplication>
#include <QColor>
#include <QComboBox>
#include <QFile>
#include <QPalette>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::abrir_configuracoes_fonte()
{
    if (texto_area_ == nullptr) {
        return;
    }

    QTextCursor cursor = texto_area_->textCursor();
    const bool has_selection = cursor.hasSelection();

    QTextCharFormat char_fmt = cursor.charFormat();
    QFont cur_font = char_fmt.font();
    if (cur_font.family().isEmpty()) {
        cur_font = texto_area_->font();
    }

    source::utils::TaskTitleFontSettings initial_settings;
    initial_settings.family = cur_font.family();
    initial_settings.style = cur_font.styleName();
    initial_settings.point_size = cur_font.pointSize() > 0 ? cur_font.pointSize() : 10;
    initial_settings.bold = cur_font.bold();
    initial_settings.italic = cur_font.italic();
    initial_settings.underline = cur_font.underline();
    initial_settings.strike_out = cur_font.strikeOut();

    const QFont default_font = fonte_audiolivro_padrao();
    source::utils::TaskTitleFontSettings default_settings;
    default_settings.family = default_font.family();
    default_settings.style = default_font.styleName();
    default_settings.point_size = default_font.pointSize();
    default_settings.bold = default_font.bold();
    default_settings.italic = default_font.italic();
    default_settings.underline = default_font.underline();
    default_settings.strike_out = default_font.strikeOut();

    const QString dialog_title = tr("Configurações de Fonte");
    const QString preview_text = has_selection ? cursor.selectedText() : tr("Exemplo de Texto Selecionado");

    source::utils::TaskTitleFontSettings out_settings;
    if (!source::utils::open_task_font_settings_dialog(this, dialog_title, preview_text, initial_settings, default_settings, &out_settings, source::utils::PluginFontContext::Global)) {
        return;
    }

    QFont new_font = source::utils::task_title_font(QApplication::font(), out_settings);
    new_font.setUnderline(out_settings.underline);
    new_font.setStrikeOut(out_settings.strike_out);

    if (has_selection) {
        QTextCharFormat fmt;
        fmt.setFont(new_font);
        cursor.mergeCharFormat(fmt);
        texto_area_->setTextCursor(cursor);
    } else {
        QTextCursor all_cursor(texto_area_->document());
        all_cursor.select(QTextCursor::Document);
        QTextCharFormat fmt;
        fmt.setFont(new_font);
        all_cursor.mergeCharFormat(fmt);
        texto_area_->setFont(new_font);
        texto_area_->document()->setDefaultFont(new_font);
    }
    salvar_fonte_configurada(new_font);

    texto_area_->viewport()->update();
}

void LeitorAcessivel::abrir_colorir_tarefas()
{
    if (texto_area_ == nullptr) {
        return;
    }

    QTextCursor cursor = texto_area_->textCursor();
    const bool has_selection = cursor.hasSelection();

    QColor initial_color = cursor.charFormat().foreground().color();
    if (!initial_color.isValid()) {
        initial_color = carregar_cor_configurada();
    }
    if (!initial_color.isValid()) {
        initial_color = texto_area_->palette().text().color();
    }

    const QColor selected_color = source::utils::choose_task_color(this, initial_color, source::utils::PluginFontContext::Global, tr("Colorir fonte/texto"));
    if (!selected_color.isValid()) {
        return;
    }

    if (has_selection) {
        QTextCharFormat fmt;
        fmt.setForeground(selected_color);
        cursor.mergeCharFormat(fmt);
        texto_area_->setTextCursor(cursor);
    } else {
        QTextCursor all_cursor(texto_area_->document());
        all_cursor.select(QTextCursor::Document);
        QTextCharFormat fmt;
        fmt.setForeground(selected_color);
        all_cursor.mergeCharFormat(fmt);
        QPalette p = texto_area_->palette();
        p.setColor(QPalette::Text, selected_color);
        texto_area_->setPalette(p);
        salvar_cor_configurada(selected_color);
    }

    salvar_texto_temp();
    texto_area_->viewport()->update();
}

void LeitorAcessivel::aplicar_fonte_configurada()
{
    const QFont font = carregar_fonte_configurada();
    const QColor color = carregar_cor_configurada();

    if (texto_area_ != nullptr) {
        texto_area_->setFont(font);
        texto_area_->document()->setDefaultFont(font);
        
        QTextCursor all_cursor(texto_area_->document());
        all_cursor.select(QTextCursor::Document);
        QTextCharFormat fmt;
        fmt.setFont(font);

        if (color.isValid()) {
            QPalette p = texto_area_->palette();
            p.setColor(QPalette::Text, color);
            texto_area_->setPalette(p);
            fmt.setForeground(color);
        }

        all_cursor.mergeCharFormat(fmt);

        texto_area_->viewport()->update();
    }
}

void LeitorAcessivel::definir_fonte_configurada(const QFont &font)
{
    salvar_fonte_configurada(font);
    aplicar_fonte_configurada();
}

void LeitorAcessivel::restaurar_configuracoes()
{
    const QFont default_font = fonte_audiolivro_padrao();
    salvar_fonte_configurada(default_font);
    salvar_cor_configurada(QColor());

    if (texto_area_ != nullptr) {
        texto_area_->setFont(default_font);
        texto_area_->document()->setDefaultFont(default_font);
        QPalette p = texto_area_->palette();
        p.setColor(QPalette::Text, QApplication::palette().text().color());
        texto_area_->setPalette(p);

        QTextCursor all_cursor(texto_area_->document());
        all_cursor.select(QTextCursor::Document);
        QTextCharFormat clear_fmt;
        clear_fmt.setFont(default_font);
        clear_fmt.setForeground(QApplication::palette().text().color());
        all_cursor.setCharFormat(clear_fmt);
    }

    replace_current_line_prefixes(false);
    if (combo_bullet_style_ != nullptr) {
        combo_bullet_style_->setCurrentIndex(0);
    }

    if (combo_spacing_ != nullptr) {
        const QSignalBlocker blocker(combo_spacing_);
        combo_spacing_->setCurrentText(QStringLiteral("1.0"));
    }
    set_line_spacing(1.0);

    if (spin_indent_ != nullptr) {
        const QSignalBlocker blocker(spin_indent_);
        spin_indent_->setValue(0);
    }
    set_indent(0);

    if (spin_margin_ != nullptr) {
        const QSignalBlocker blocker(spin_margin_);
        spin_margin_->setValue(10);
    }
    set_margins(10);

    if (texto_area_ != nullptr) {
        texto_area_->viewport()->update();
    }
}

void LeitorAcessivel::restaurar_estado_carregado_temp()
{
    const QString pdf_path = carregar_pdf_carregado_temp();
    if (!pdf_path.isEmpty() && QFile::exists(pdf_path)) {
        carregar_pdf(pdf_path, true);
    }

    const QString text_temp = carregar_texto_carregado_temp();
    if (!text_temp.isEmpty() && texto_area_ != nullptr) {
        const QSignalBlocker blocker(texto_area_);
        if (Qt::mightBeRichText(text_temp)) {
            texto_area_->setHtml(text_temp);
        } else {
            texto_area_->setPlainText(text_temp);
        }
    }

    const int tab_index = carregar_aba_ativa_temp();
    if (content_stack_ != nullptr && tab_index >= 0 && tab_index < content_stack_->count()) {
        content_stack_->setCurrentIndex(tab_index);
    }
}

void LeitorAcessivel::salvar_texto_temp()
{
    if (texto_area_ != nullptr) {
        salvar_texto_carregado_temp(texto_area_->toHtml());
    }
}

}  // namespace source::plugin::Audiolivro
