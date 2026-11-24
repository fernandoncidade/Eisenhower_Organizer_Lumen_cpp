#include "source/services/incore_08_exibir_sobre.hpp"

#include <QCoreApplication>
#include <QColor>
#include <QDialog>
#include <QEvent>
#include <QFont>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPalette>
#include <QPushButton>
#include <QSignalBlocker>
#include <QAbstractTextDocumentLayout>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextBrowser>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextFragment>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

#include <memory>

#include "source/ui/ui_01_SobreDialog.hpp"
#include "source/ui/ui_02_OpcoesSobre.hpp"
#include "source/services/incore_12_Manual.hpp"
#include "source/eisenhower_app.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"
#include "source/utils/AppConfig.hpp"
#include "source/utils/BarraTitulosUtils.hpp"
#include "source/utils/IconUtils.hpp"

namespace source::InterfaceCore::incore_08_exibir_sobre {
namespace {

QString current_language(source::EisenhowerMatrixApp *app)
{
    return app != nullptr && app->gerenciador_traducao != nullptr ? app->gerenciador_traducao->obter_idioma_atual()
                                                                  : QStringLiteral("pt_BR");
}

void apply_window_icon(QWidget *widget)
{
    const QString icon_path = source::utils::get_default_window_icon_path();
    if (!icon_path.isEmpty()) {
        widget->setWindowIcon(QIcon(icon_path));
    }
}

QString manual_root_anchor()
{
    return QStringLiteral("manual-root");
}

QString manual_section_anchor(const QString &section_id)
{
    return QStringLiteral("manual-section-%1").arg(section_id);
}

QString description_text_for_build()
{
    const QString key = QStringLiteral("description_text");
    QString text = QCoreApplication::translate("App", key.toUtf8().constData());
    if (text.isEmpty() || text == key) {
        text = QCoreApplication::translate("InterfaceGrafica", key.toUtf8().constData());
    }
    if (text.isEmpty() || text == key) {
        text = key;
    }

    const QString product_name = source::utils::AppConfig::app_manual_name();
    text.replace(QStringLiteral("EISENHOWER ORGANIZER"), product_name, Qt::CaseSensitive);
    text.replace(QStringLiteral("LÚMEN"), product_name, Qt::CaseSensitive);
    return text;
}

void append_manual_line(QTextCursor &cursor, const QString &text, const QTextCharFormat &format = QTextCharFormat())
{
    if (format.isValid()) {
        cursor.insertText(text, format);
    } else {
        cursor.insertText(text);
    }
    cursor.insertBlock();
}

void render_manual_document(QTextBrowser *viewer,
                            const QString &lang,
                            bool include_pomodoro_plugin,
                            bool include_feynman_plugin,
                            bool include_mapamental_plugin,
                            bool include_audiolivro_plugin,
                            bool include_agenda_plugin,
                            bool include_modelo_plugin,
                            bool include_gantt_plugin)
{
    if (viewer == nullptr || viewer->document() == nullptr) {
        return;
    }

    const QString normalized = source::InterfaceCore::incore_12_Manual::normalize_language(lang);
    const auto blocks_and_order = source::InterfaceCore::incore_12_Manual::get_manual_blocks(
        normalized,
        include_pomodoro_plugin,
        include_feynman_plugin,
        include_mapamental_plugin,
        include_audiolivro_plugin,
        include_agenda_plugin,
        include_modelo_plugin,
        include_gantt_plugin);
    const QList<source::InterfaceCore::incore_12_Manual::ManualBlock> &blocks = blocks_and_order.first;
    QTextDocument *document = viewer->document();
    document->clear();

    QTextCursor cursor(document);
    cursor.beginEditBlock();

    const QColor accent_blue(QStringLiteral("#1d4ed8"));

    QTextCharFormat title_format;
    QFont title_font = viewer->font();
    title_font.setBold(true);
    title_font.setPointSize(title_font.pointSize() + 7);
    title_format.setFont(title_font);
    title_format.setAnchor(true);
    title_format.setAnchorNames({manual_root_anchor()});
    title_format.setForeground(accent_blue);

    QTextCharFormat toc_link_format;
    toc_link_format.setAnchor(true);
    toc_link_format.setForeground(accent_blue);
    toc_link_format.setFontUnderline(true);

    QTextCharFormat section_title_format;
    QFont section_title_font = viewer->font();
    section_title_font.setBold(true);
    section_title_font.setPointSize(section_title_font.pointSize() + 3);
    section_title_format.setFont(section_title_font);
    section_title_format.setAnchor(true);
    section_title_format.setForeground(accent_blue);
    section_title_format.setFontUnderline(true);

    QTextCharFormat detail_title_format;
    QFont detail_title_font = viewer->font();
    detail_title_font.setBold(true);
    detail_title_format.setFont(detail_title_font);

    QTextCharFormat body_format;
    body_format.setFont(viewer->font());

    bool title_rendered = false;
    for (const auto &block : blocks) {
        if (block.kind == QStringLiteral("blank")) {
            cursor.insertBlock();
            continue;
        }

        if (block.kind == QStringLiteral("line")) {
            if (!title_rendered) {
                append_manual_line(cursor, block.text, title_format);
                title_rendered = true;
            } else {
                append_manual_line(cursor, block.text, body_format);
            }
            continue;
        }

        if (block.kind == QStringLiteral("toc_title")) {
            append_manual_line(cursor, block.text, detail_title_format);
            continue;
        }

        if (block.kind == QStringLiteral("toc_item")) {
            QTextCharFormat link_format = toc_link_format;
            link_format.setAnchorHref(QStringLiteral("#%1").arg(manual_section_anchor(block.section_id)));
            append_manual_line(cursor, block.text, link_format);
            continue;
        }

        if (block.kind == QStringLiteral("section_title")) {
            QTextCharFormat link_format = section_title_format;
            link_format.setAnchorHref(QStringLiteral("#%1").arg(manual_root_anchor()));
            link_format.setAnchorNames({manual_section_anchor(block.section_id)});
            append_manual_line(cursor, block.text, link_format);
            continue;
        }

        if (block.kind == QStringLiteral("detail_title")) {
            append_manual_line(cursor, block.text, detail_title_format);
            continue;
        }

        if (block.kind == QStringLiteral("bullet")) {
            append_manual_line(cursor, QStringLiteral("- %1").arg(block.text), body_format);
            continue;
        }

        append_manual_line(cursor, block.text, body_format);
    }

    cursor.endEditBlock();
    QTextCursor reset_cursor(document);
    reset_cursor.movePosition(QTextCursor::Start);
    viewer->setTextCursor(reset_cursor);
    if (viewer->verticalScrollBar() != nullptr) {
        viewer->verticalScrollBar()->setValue(0);
    }
}

int find_manual_anchor_position(const QTextDocument *document, const QString &anchor_name)
{
    if (document == nullptr || anchor_name.isEmpty()) {
        return -1;
    }

    for (QTextBlock block = document->begin(); block.isValid(); block = block.next()) {
        for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
            const QTextFragment fragment = it.fragment();
            if (!fragment.isValid()) {
                continue;
            }

            const QTextCharFormat format = fragment.charFormat();
            if (format.isAnchor() && format.anchorNames().contains(anchor_name)) {
                return fragment.position();
            }
        }
    }

    return -1;
}

void scroll_manual_viewer_to_position(QTextBrowser *viewer, int pos)
{
    if (viewer == nullptr || pos < 0 || viewer->document() == nullptr || viewer->verticalScrollBar() == nullptr) {
        return;
    }

    QTextCursor cursor(viewer->document());
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::StartOfBlock);
    viewer->setTextCursor(cursor);

    const QTextBlock block = cursor.block();
    if (!block.isValid()) {
        viewer->ensureCursorVisible();
        return;
    }

    const qreal block_top = viewer->document()->documentLayout()->blockBoundingRect(block).top();
    viewer->verticalScrollBar()->setValue(block_top <= 0.0 ? 0 : static_cast<int>(block_top));
}

void scroll_manual_viewer_to_anchor(QTextBrowser *viewer, const QString &anchor_name)
{
    if (viewer == nullptr || anchor_name.isEmpty()) {
        return;
    }

    const int pos = find_manual_anchor_position(viewer->document(), anchor_name);
    if (pos >= 0) {
        scroll_manual_viewer_to_position(viewer, pos);
        return;
    }

    viewer->scrollToAnchor(anchor_name);
}

int find_manual_row_for_anchor(QListWidget *list, const QString &anchor_name)
{
    if (list == nullptr || anchor_name.isEmpty()) {
        return -1;
    }

    for (int row = 0; row < list->count(); ++row) {
        QListWidgetItem *item = list->item(row);
        if (item != nullptr && item->data(Qt::UserRole).toString() == anchor_name) {
            return row;
        }
    }

    return -1;
}

struct ManualSearchState {
    QString query;
    QVector<QTextCursor> matches;
    int current_index = -1;
};

void update_manual_search_highlights(QTextBrowser *viewer, const ManualSearchState &state)
{
    if (viewer == nullptr || viewer->document() == nullptr) {
        return;
    }

    QList<QTextEdit::ExtraSelection> selections;
    if (state.query.isEmpty()) {
        viewer->setExtraSelections(selections);
        return;
    }

    QColor highlight_color = viewer->palette().color(QPalette::Highlight);
    highlight_color.setAlpha(95);
    QColor current_color = viewer->palette().color(QPalette::Highlight);
    current_color.setAlpha(190);
    const QColor current_text_color = viewer->palette().color(QPalette::HighlightedText);

    for (int index = 0; index < state.matches.size(); ++index) {
        QTextEdit::ExtraSelection selection;
        selection.cursor = state.matches.at(index);
        selection.format.setBackground(index == state.current_index ? current_color : highlight_color);
        if (index == state.current_index) {
            selection.format.setForeground(current_text_color);
        }
        selections.append(selection);
    }

    viewer->setExtraSelections(selections);
}

void rebuild_manual_search(QTextBrowser *viewer, ManualSearchState &state, const QString &search_text)
{
    state.query = search_text.trimmed();
    state.matches.clear();
    state.current_index = -1;

    if (viewer == nullptr || viewer->document() == nullptr || state.query.isEmpty()) {
        update_manual_search_highlights(viewer, state);
        return;
    }

    QTextCursor cursor(viewer->document());
    while (true) {
        cursor = viewer->document()->find(state.query, cursor);
        if (cursor.isNull()) {
            break;
        }
        state.matches.append(cursor);
    }

    update_manual_search_highlights(viewer, state);
}

void navigate_manual_search(QTextBrowser *viewer, ManualSearchState &state, int step)
{
    if (viewer == nullptr || state.matches.isEmpty() || step == 0) {
        return;
    }

    if (state.current_index < 0) {
        state.current_index = step < 0 ? state.matches.size() - 1 : 0;
    } else {
        state.current_index = (state.current_index + step + state.matches.size()) % state.matches.size();
    }

    const QTextCursor cursor = state.matches.at(state.current_index);
    update_manual_search_highlights(viewer, state);
    viewer->setTextCursor(cursor);
    scroll_manual_viewer_to_position(viewer, cursor.selectionStart());
}

void update_manual_search_buttons(QToolButton *previous_button,
                                  QToolButton *next_button,
                                  const ManualSearchState &state)
{
    const bool enabled = !state.matches.isEmpty();
    if (previous_button != nullptr) {
        previous_button->setEnabled(enabled);
    }
    if (next_button != nullptr) {
        next_button->setEnabled(enabled);
    }
}

}  // namespace

QString tr_multi(const QString &key)
{
    QString value = QCoreApplication::translate("App", key.toUtf8().constData());
    if (!value.isEmpty() && value != key) {
        return value;
    }
    value = QCoreApplication::translate("InterfaceGrafica", key.toUtf8().constData());
    return (!value.isEmpty() && value != key) ? value : key;
}

QString build_cabecalho_fixo()
{
    return QStringLiteral("<h3>%1</h3>"
                          "<p><b>%2:</b> 2026.11.22.0</p>"
                          "<p><b>%3:</b> Fernando Nillsson Cidade</p>"
                          "<p><b>%4:</b> %5</p>")
        .arg(source::utils::nome_aplicacao_marca(),
             tr_multi(QStringLiteral("version")),
             tr_multi(QStringLiteral("authors")),
             tr_multi(QStringLiteral("description")),
             description_text_for_build());
}

void exibir_sobre(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }

    if (app->_sobre_dialog != nullptr && app->_sobre_dialog->isVisible()) {
        app->_sobre_dialog->raise();
        app->_sobre_dialog->activateWindow();
        return;
    }

    const QString idioma = current_language(app);
    const bool pt = idioma == QStringLiteral("pt_BR");
    const bool include_pomodoro_plugin = app->has_pomodoro_plugin();
    const bool include_feynman_plugin = app->has_feynman_plugin();
    const bool include_mapamental_plugin = app->has_mapamental_plugin();
    const bool include_audiolivro_plugin = app->has_audiolivro_plugin();
    const bool include_modelo_plugin = app->has_modelo_plugin();
    const bool include_agenda_plugin = app->has_agenda_plugin();
    const bool include_gantt_plugin = app->has_gantt_plugin();
    const QVector<source::GerenciamentoUI::DocumentoSobre> details_documents =
        pt ? source::GerenciamentoUI::OpcoesSobre::About_Documents_pt_BR(include_pomodoro_plugin,
                                                                         include_feynman_plugin,
                                                                         include_mapamental_plugin,
                                                                         include_audiolivro_plugin,
                                                                         include_agenda_plugin,
                                                                         include_modelo_plugin,
                                                                         include_gantt_plugin)
           : source::GerenciamentoUI::OpcoesSobre::About_Documents_en_US(include_pomodoro_plugin,
                                                                         include_feynman_plugin,
                                                                         include_mapamental_plugin,
                                                                         include_audiolivro_plugin,
                                                                         include_agenda_plugin,
                                                                         include_modelo_plugin,
                                                                         include_gantt_plugin);
    const QVector<source::GerenciamentoUI::DocumentoSobre> license_documents =
        pt ? source::GerenciamentoUI::OpcoesSobre::License_Documents_pt_BR(include_pomodoro_plugin,
                                                                           include_feynman_plugin,
                                                                           include_mapamental_plugin,
                                                                           include_audiolivro_plugin,
                                                                           include_agenda_plugin,
                                                                           include_modelo_plugin,
                                                                           include_gantt_plugin)
           : source::GerenciamentoUI::OpcoesSobre::License_Documents_en_US(include_pomodoro_plugin,
                                                                           include_feynman_plugin,
                                                                           include_mapamental_plugin,
                                                                           include_audiolivro_plugin,
                                                                           include_agenda_plugin,
                                                                           include_modelo_plugin,
                                                                           include_gantt_plugin);
    const QVector<source::GerenciamentoUI::DocumentoSobre> contract_documents =
        pt ? source::GerenciamentoUI::OpcoesSobre::Contract_Documents_pt_BR(include_pomodoro_plugin,
                                                                            include_feynman_plugin,
                                                                            include_mapamental_plugin,
                                                                            include_audiolivro_plugin,
                                                                            include_agenda_plugin,
                                                                            include_modelo_plugin,
                                                                            include_gantt_plugin)
           : source::GerenciamentoUI::OpcoesSobre::Contract_Documents_en_US(include_pomodoro_plugin,
                                                                            include_feynman_plugin,
                                                                            include_mapamental_plugin,
                                                                            include_audiolivro_plugin,
                                                                            include_agenda_plugin,
                                                                            include_modelo_plugin,
                                                                            include_gantt_plugin);
    const QVector<source::GerenciamentoUI::DocumentoSobre> copyright_documents =
        pt ? source::GerenciamentoUI::OpcoesSobre::Copyright_Documents_pt_BR(include_pomodoro_plugin,
                                                                             include_feynman_plugin,
                                                                             include_mapamental_plugin,
                                                                             include_audiolivro_plugin,
                                                                             include_agenda_plugin,
                                                                             include_modelo_plugin,
                                                                             include_gantt_plugin)
           : source::GerenciamentoUI::OpcoesSobre::Copyright_Documents_en_US(include_pomodoro_plugin,
                                                                             include_feynman_plugin,
                                                                             include_mapamental_plugin,
                                                                             include_audiolivro_plugin,
                                                                             include_agenda_plugin,
                                                                             include_modelo_plugin,
                                                                             include_gantt_plugin);
    const QVector<source::GerenciamentoUI::DocumentoSobre> notice_documents =
        pt ? source::GerenciamentoUI::OpcoesSobre::Notice_Documents_pt_BR(include_pomodoro_plugin,
                                                                          include_feynman_plugin,
                                                                          include_mapamental_plugin,
                                                                          include_audiolivro_plugin,
                                                                          include_agenda_plugin,
                                                                          include_modelo_plugin,
                                                                          include_gantt_plugin)
           : source::GerenciamentoUI::OpcoesSobre::Notice_Documents_en_US(include_pomodoro_plugin,
                                                                          include_feynman_plugin,
                                                                          include_mapamental_plugin,
                                                                          include_audiolivro_plugin,
                                                                          include_agenda_plugin,
                                                                          include_modelo_plugin,
                                                                          include_gantt_plugin);
    const QVector<source::GerenciamentoUI::DocumentoSobre> privacy_documents =
        pt ? source::GerenciamentoUI::OpcoesSobre::Privacy_Policy_Documents_pt_BR(include_pomodoro_plugin,
                                                                                  include_feynman_plugin,
                                                                                  include_mapamental_plugin,
                                                                                  include_audiolivro_plugin,
                                                                                  include_agenda_plugin,
                                                                                  include_modelo_plugin,
                                                                                  include_gantt_plugin)
           : source::GerenciamentoUI::OpcoesSobre::Privacy_Policy_Documents_en_US(include_pomodoro_plugin,
                                                                                  include_feynman_plugin,
                                                                                  include_mapamental_plugin,
                                                                                  include_audiolivro_plugin,
                                                                                  include_agenda_plugin,
                                                                                  include_modelo_plugin,
                                                                                  include_gantt_plugin);

    auto *dialog = new source::GerenciamentoUI::SobreDialog(
        nullptr,
        source::utils::titulo_janela_sobre(),
        build_cabecalho_fixo(),
        pt ? source::GerenciamentoUI::OpcoesSobre::History_APP_pt_BR(include_pomodoro_plugin,
                                                                     include_feynman_plugin,
                                                                     include_mapamental_plugin,
                                                                     include_audiolivro_plugin,
                                                                     include_agenda_plugin,
                                                                     include_modelo_plugin,
                                                                     include_gantt_plugin)
           : source::GerenciamentoUI::OpcoesSobre::History_APP_en_US(include_pomodoro_plugin,
                                                                     include_feynman_plugin,
                                                                     include_mapamental_plugin,
                                                                     include_audiolivro_plugin,
                                                                     include_agenda_plugin,
                                                                     include_modelo_plugin,
                                                                     include_gantt_plugin),
        pt ? source::GerenciamentoUI::OpcoesSobre::ABOUT_TEXT_PT_BR(include_pomodoro_plugin,
                                                                    include_feynman_plugin,
                                                                    include_mapamental_plugin,
                                                                    include_audiolivro_plugin,
                                                                    include_agenda_plugin,
                                                                    include_modelo_plugin,
                                                                    include_gantt_plugin)
           : source::GerenciamentoUI::OpcoesSobre::ABOUT_TEXT_EN_US(include_pomodoro_plugin,
                                                                    include_feynman_plugin,
                                                                    include_mapamental_plugin,
                                                                    include_audiolivro_plugin,
                                                                    include_agenda_plugin,
                                                                    include_modelo_plugin,
                                                                    include_gantt_plugin),
        pt ? source::GerenciamentoUI::OpcoesSobre::LICENSE_TEXT_PT_BR(include_pomodoro_plugin,
                                                                      include_feynman_plugin,
                                                                      include_mapamental_plugin,
                                                                      include_audiolivro_plugin,
                                                                      include_agenda_plugin,
                                                                      include_modelo_plugin,
                                                                      include_gantt_plugin)
           : source::GerenciamentoUI::OpcoesSobre::LICENSE_TEXT_EN_US(include_pomodoro_plugin,
                                                                      include_feynman_plugin,
                                                                      include_mapamental_plugin,
                                                                      include_audiolivro_plugin,
                                                                      include_agenda_plugin,
                                                                      include_modelo_plugin,
                                                                      include_gantt_plugin),
        source::GerenciamentoUI::OpcoesSobre::SITE_LICENSES(),
        tr_multi(QStringLiteral("show_history")),
        tr_multi(QStringLiteral("hide_history")),
        tr_multi(QStringLiteral("show_details")),
        tr_multi(QStringLiteral("hide_details")),
        tr_multi(QStringLiteral("show_licenses")),
        tr_multi(QStringLiteral("hide_licenses")),
        tr_multi(QStringLiteral("OK")),
        tr_multi(QStringLiteral("site_oficial")),
        pt ? source::GerenciamentoUI::OpcoesSobre::NOTICE_TEXT_PT_BR(include_pomodoro_plugin,
                                                                     include_feynman_plugin,
                                                                     include_mapamental_plugin,
                                                                     include_audiolivro_plugin,
                                                                     include_agenda_plugin,
                                                                     include_modelo_plugin,
                                                                     include_gantt_plugin)
           : source::GerenciamentoUI::OpcoesSobre::NOTICE_TEXT_EN_US(include_pomodoro_plugin,
                                                                     include_feynman_plugin,
                                                                     include_mapamental_plugin,
                                                                     include_audiolivro_plugin,
                                                                     include_agenda_plugin,
                                                                     include_modelo_plugin,
                                                                     include_gantt_plugin),
        tr_multi(QStringLiteral("show_notices")),
        tr_multi(QStringLiteral("hide_notices")),
        pt ? source::GerenciamentoUI::OpcoesSobre::Privacy_Policy_pt_BR(include_pomodoro_plugin,
                                                                        include_feynman_plugin,
                                                                        include_mapamental_plugin,
                                                                        include_audiolivro_plugin,
                                                                        include_agenda_plugin,
                                                                        include_modelo_plugin,
                                                                        include_gantt_plugin)
           : source::GerenciamentoUI::OpcoesSobre::Privacy_Policy_en_US(include_pomodoro_plugin,
                                                                        include_feynman_plugin,
                                                                        include_mapamental_plugin,
                                                                        include_audiolivro_plugin,
                                                                        include_agenda_plugin,
                                                                        include_modelo_plugin,
                                                                        include_gantt_plugin),
        tr_multi(QStringLiteral("show_privacy_policy")),
        tr_multi(QStringLiteral("hide_privacy_policy")),
        tr_multi(QStringLiteral("information_not_available")),
        pt ? source::GerenciamentoUI::OpcoesSobre::RELEASE_NOTES_pt_BR(include_pomodoro_plugin,
                                                                       include_feynman_plugin,
                                                                       include_mapamental_plugin,
                                                                       include_audiolivro_plugin,
                                                                       include_agenda_plugin,
                                                                       include_modelo_plugin,
                                                                       include_gantt_plugin)
           : source::GerenciamentoUI::OpcoesSobre::RELEASE_NOTES_en_US(include_pomodoro_plugin,
                                                                       include_feynman_plugin,
                                                                       include_mapamental_plugin,
                                                                       include_audiolivro_plugin,
                                                                       include_agenda_plugin,
                                                                       include_modelo_plugin,
                                                                       include_gantt_plugin),
        tr_multi(QStringLiteral("show_release_notes")),
        tr_multi(QStringLiteral("hide_release_notes")),
        app);

    dialog->setHeaderBuilder([]() { return build_cabecalho_fixo(); });
    dialog->setDetailsDocuments(details_documents);
    dialog->setLicenseDocuments(license_documents);
    dialog->setContractDocuments(contract_documents);
    dialog->setCopyrightDocuments(copyright_documents);
    dialog->setNoticeDocuments(notice_documents);
    dialog->setPrivacyPolicyDocuments(privacy_documents);
    dialog->setWindowModality(Qt::NonModal);
    dialog->setModal(false);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->resize(900, 500);
    apply_window_icon(dialog);
    app->_sobre_dialog = dialog;

    QObject::connect(dialog, &QObject::destroyed, app, [app]() { app->_sobre_dialog = nullptr; });
    if (app->gerenciador_traducao != nullptr) {
        QObject::connect(app->gerenciador_traducao,
                         &source::language::GerenciadorTraducao::idioma_alterado,
                         dialog,
                         [dialog](const QString &) {
                             QCoreApplication::postEvent(dialog, new QEvent(QEvent::LanguageChange));
                         });
    }
    dialog->show();
}

void exibir_manual(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }

    if (app->_manual_dialog != nullptr && app->_manual_dialog->isVisible()) {
        app->_manual_dialog->raise();
        app->_manual_dialog->activateWindow();
        return;
    }

    class ManualDialog final : public QDialog {
    public:
        using QDialog::QDialog;
    };

    auto *dialog = new ManualDialog();
    dialog->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint
                           | Qt::WindowCloseButtonHint);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->setWindowModality(Qt::NonModal);
    dialog->setModal(false);
    dialog->setMinimumSize(900, 650);
    apply_window_icon(dialog);

    auto *root = new QVBoxLayout(dialog);
    auto *search_layout = new QGridLayout();
    auto *search_label = new QLabel(dialog);
    auto *search_edit = new QLineEdit(dialog);
    auto *search_previous_button = new QToolButton(dialog);
    auto *search_next_button = new QToolButton(dialog);
    auto *search_controls = new QHBoxLayout();
    search_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    search_edit->setClearButtonEnabled(true);
    search_previous_button->setArrowType(Qt::LeftArrow);
    search_next_button->setArrowType(Qt::RightArrow);
    search_previous_button->setAutoRaise(true);
    search_next_button->setAutoRaise(true);
    search_previous_button->setEnabled(false);
    search_next_button->setEnabled(false);
    search_controls->setContentsMargins(0, 0, 0, 0);
    search_controls->setSpacing(4);
    search_controls->addWidget(search_edit, 1);
    search_controls->addWidget(search_previous_button);
    search_controls->addWidget(search_next_button);
    search_layout->setColumnStretch(0, 0);
    search_layout->setColumnStretch(1, 1);
    search_layout->addWidget(search_label, 0, 0);
    search_layout->addLayout(search_controls, 0, 1);
    root->addLayout(search_layout);

    auto *body = new QHBoxLayout();
    auto *list = new QListWidget(dialog);
    auto *viewer = new QTextBrowser(dialog);
    auto *close_button = new QPushButton(dialog);
    viewer->setReadOnly(true);
    viewer->setOpenLinks(false);
    viewer->setOpenExternalLinks(false);
    list->setMinimumWidth(300);
    body->addWidget(list, 0);
    body->addWidget(viewer, 1);
    root->addLayout(body, 1);

    auto *footer = new QHBoxLayout();
    footer->addStretch(1);
    close_button->setAutoDefault(false);
    close_button->setDefault(false);
    footer->addWidget(close_button);
    root->addLayout(footer);

    auto search_state = std::make_shared<ManualSearchState>();
    QObject::connect(search_edit, &QLineEdit::textChanged, dialog, [viewer,
                                                                    search_state,
                                                                    search_previous_button,
                                                                    search_next_button](const QString &text) {
        rebuild_manual_search(viewer, *search_state, text);
        update_manual_search_buttons(search_previous_button, search_next_button, *search_state);
    });
    QObject::connect(search_edit, &QLineEdit::returnPressed, dialog, [viewer,
                                                                     search_state,
                                                                     search_previous_button,
                                                                     search_next_button]() {
        navigate_manual_search(viewer, *search_state, 1);
        update_manual_search_buttons(search_previous_button, search_next_button, *search_state);
    });
    QObject::connect(search_previous_button, &QToolButton::clicked, dialog, [viewer,
                                                                            search_state,
                                                                            search_previous_button,
                                                                            search_next_button]() {
        navigate_manual_search(viewer, *search_state, -1);
        update_manual_search_buttons(search_previous_button, search_next_button, *search_state);
    });
    QObject::connect(search_next_button, &QToolButton::clicked, dialog, [viewer,
                                                                        search_state,
                                                                        search_previous_button,
                                                                        search_next_button]() {
        navigate_manual_search(viewer, *search_state, 1);
        update_manual_search_buttons(search_previous_button, search_next_button, *search_state);
    });

    const auto render_manual = [app,
                                dialog,
                                list,
                                viewer,
                                close_button,
                                search_label,
                                search_edit,
                                search_previous_button,
                                search_next_button,
                                search_state]() {
        int current_row = list->currentRow();
        if (current_row < 0) {
            current_row = 0;
        }

        const QString idioma = current_language(app);
        const bool include_pomodoro_plugin = app->has_pomodoro_plugin();
        const bool include_feynman_plugin = app->has_feynman_plugin();
        const bool include_mapamental_plugin = app->has_mapamental_plugin();
        const bool include_audiolivro_plugin = app->has_audiolivro_plugin();
        const bool include_agenda_plugin = app->has_agenda_plugin();
        const bool include_modelo_plugin = app->has_modelo_plugin();
        const bool include_gantt_plugin = app->has_gantt_plugin();
        const QList<source::InterfaceCore::incore_12_Manual::ManualSection> sections =
            source::InterfaceCore::incore_12_Manual::get_manual_document(
                idioma,
                include_pomodoro_plugin,
                include_feynman_plugin,
                include_mapamental_plugin,
                include_audiolivro_plugin,
                include_agenda_plugin,
                include_modelo_plugin,
                include_gantt_plugin);
        const QString manual_title = source::InterfaceCore::incore_12_Manual::get_manual_title(idioma);
        const QString search_label_text = tr_multi(QStringLiteral("manual_search_label"));
        const QString search_placeholder_text = tr_multi(QStringLiteral("manual_search_placeholder"));
        const QString search_previous_text = tr_multi(QStringLiteral("manual_search_previous"));
        const QString search_next_text = tr_multi(QStringLiteral("manual_search_next"));

        dialog->setWindowTitle(manual_title);
        search_label->setText(search_label_text == QStringLiteral("manual_search_label") ? QStringLiteral("Buscar:")
                                                                                         : search_label_text);
        search_edit->setPlaceholderText(search_placeholder_text == QStringLiteral("manual_search_placeholder")
                                            ? QStringLiteral("Buscar texto no manual...")
                                            : search_placeholder_text);
        search_previous_button->setToolTip(search_previous_text == QStringLiteral("manual_search_previous")
                                               ? QStringLiteral("Resultado anterior")
                                               : search_previous_text);
        search_next_button->setToolTip(search_next_text == QStringLiteral("manual_search_next")
                                           ? QStringLiteral("Próximo resultado")
                                           : search_next_text);
        search_previous_button->setAccessibleName(search_previous_button->toolTip());
        search_next_button->setAccessibleName(search_next_button->toolTip());
        close_button->setText(idioma == QStringLiteral("pt_BR") ? QStringLiteral("Fechar") : QStringLiteral("Close"));
        render_manual_document(viewer,
                               idioma,
                               include_pomodoro_plugin,
                               include_feynman_plugin,
                               include_mapamental_plugin,
                               include_audiolivro_plugin,
                               include_agenda_plugin,
                               include_modelo_plugin,
                               include_gantt_plugin);

        list->blockSignals(true);
        list->clear();
        auto *manual_item = new QListWidgetItem(manual_title, list);
        manual_item->setData(Qt::UserRole, manual_root_anchor());
        for (const auto &section : sections) {
            auto *section_item = new QListWidgetItem(section.title, list);
            section_item->setData(Qt::UserRole, manual_section_anchor(section.id));
        }
        list->blockSignals(false);

        QObject::disconnect(list, nullptr, dialog, nullptr);
        QObject::disconnect(viewer, nullptr, dialog, nullptr);

        const auto navigate_to_anchor = [list, viewer](const QString &anchor_name, bool sync_list_selection) {
            if (anchor_name.isEmpty()) {
                return;
            }

            scroll_manual_viewer_to_anchor(viewer, anchor_name);

            if (!sync_list_selection) {
                return;
            }

            const int row = find_manual_row_for_anchor(list, anchor_name);
            if (row < 0 || row == list->currentRow()) {
                return;
            }

            const QSignalBlocker blocker(list);
            list->setCurrentRow(row);
        };

        QObject::connect(list, &QListWidget::currentRowChanged, dialog, [list, navigate_to_anchor](int row) {
            QListWidgetItem *item = list->item(row);
            if (item == nullptr) {
                return;
            }

            navigate_to_anchor(item->data(Qt::UserRole).toString(), false);
        });

        QObject::connect(viewer, &QTextBrowser::anchorClicked, dialog, [navigate_to_anchor](const QUrl &url) {
            const QString anchor_name = url.fragment().isEmpty() ? url.toString(QUrl::FullyDecoded) : url.fragment();
            navigate_to_anchor(anchor_name, true);
        });

        if (list->count() > 0) {
            list->setCurrentRow(qBound(0, current_row, list->count() - 1));
        }
        rebuild_manual_search(viewer, *search_state, search_edit->text());
        update_manual_search_buttons(search_previous_button, search_next_button, *search_state);
    };

    QObject::connect(close_button, &QPushButton::clicked, dialog, &QDialog::close);
    if (app->gerenciador_traducao != nullptr) {
        QObject::connect(app->gerenciador_traducao,
                         &source::language::GerenciadorTraducao::idioma_alterado,
                         dialog,
                         [render_manual](const QString &) { render_manual(); });
    }
    QObject::connect(dialog, &QObject::destroyed, app, [app]() { app->_manual_dialog = nullptr; });
    app->_manual_dialog = dialog;
    render_manual();
    dialog->show();
}

}  // namespace source::InterfaceCore::incore_08_exibir_sobre
