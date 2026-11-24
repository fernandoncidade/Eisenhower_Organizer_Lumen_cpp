#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_24_text_tools.hpp"

#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_37_FileDropEdit.hpp"

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCursor>

namespace source::plugin::Audiolivro {

namespace {

QString bullet_prefix_pattern()
{
    return QStringLiteral("[-*+]|\\d+\\.|[ivxlcdmIVXLCDM]+\\.|[A-Za-z]\\.|[•◦‣∙▪▫◆◇■□●○→✓☑]");
}

}  // namespace

QString LeitorAcessivel::selected_bullet_symbol() const
{
    return combo_bullet_style_ != nullptr ? combo_bullet_style_->currentText() : QStringLiteral("-");
}

void LeitorAcessivel::criar_texto(bool confirmar)
{
    if (texto_area_ == nullptr) {
        return;
    }
    if (confirmar && !texto_area_->toPlainText().trimmed().isEmpty()) {
        const auto answer = QMessageBox::question(this, tr("Novo texto"), tr("Limpar o texto atual?"));
        if (answer != QMessageBox::Yes) {
            return;
        }
    }
    texto_area_->clear();
    if (content_stack_ != nullptr) {
        content_stack_->setCurrentIndex(0);
    }
}

void LeitorAcessivel::salvar_como()
{
    if (texto_area_ == nullptr) {
        return;
    }
    const QString path = QFileDialog::getSaveFileName(this, tr("Salvar texto"), QString(), tr("Texto (*.txt);;Todos (*.*)"));
    if (path.trimmed().isEmpty()) {
        return;
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        on_tts_error(tr("Nao foi possivel salvar o arquivo."));
        return;
    }
    file.write(texto_area_->toPlainText().toUtf8());
}

void LeitorAcessivel::replace_current_line_prefixes(bool add_bullets)
{
    if (texto_area_ == nullptr) {
        return;
    }

    const QString bullet = selected_bullet_symbol();
    QTextCursor cursor = texto_area_->textCursor();
    const int start = cursor.hasSelection() ? cursor.selectionStart() : cursor.block().position();
    const int end = cursor.hasSelection() ? cursor.selectionEnd() : cursor.block().position() + cursor.block().length();

    cursor.beginEditBlock();
    QTextBlock block = texto_area_->document()->findBlock(start);
    while (block.isValid() && block.position() < end) {
        QTextCursor line_cursor(block);
        line_cursor.movePosition(QTextCursor::StartOfBlock);
        line_cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        QString line = line_cursor.selectedText();
        const QString stripped = line.replace(QRegularExpression(QStringLiteral("^\\s*(%1)\\s+").arg(bullet_prefix_pattern())), QString());
        line_cursor.insertText(add_bullets && !stripped.trimmed().isEmpty() ? bullet + QStringLiteral(" ") + stripped : stripped);
        block = block.next();
    }
    cursor.endEditBlock();
}

void LeitorAcessivel::toggle_bullets()
{
    if (texto_area_ == nullptr) {
        return;
    }
    const QTextCursor cursor = texto_area_->textCursor();
    const QString sample = cursor.hasSelection() ? cursor.selectedText() : cursor.block().text();
    const bool has_bullet = sample.contains(QRegularExpression(QStringLiteral("(^|\\u2029)\\s*(%1)\\s+").arg(bullet_prefix_pattern())));
    replace_current_line_prefixes(!has_bullet);
}

void LeitorAcessivel::set_line_spacing(double spacing)
{
    if (texto_area_ == nullptr) {
        return;
    }
    QTextCursor cursor = texto_area_->textCursor();
    QTextBlockFormat format;
    format.setLineHeight(qMax(1.0, spacing) * 100.0, QTextBlockFormat::ProportionalHeight);
    cursor.mergeBlockFormat(format);
}

void LeitorAcessivel::set_indent(int indent)
{
    if (texto_area_ == nullptr) {
        return;
    }
    QTextCursor cursor = texto_area_->textCursor();
    QTextBlockFormat format;
    format.setLeftMargin(indent);
    cursor.mergeBlockFormat(format);
}

void LeitorAcessivel::set_margins(double margin)
{
    if (texto_area_ != nullptr) {
        texto_area_->document()->setDocumentMargin(qMax(0.0, margin));
    }
}

}  // namespace source::plugin::Audiolivro
