#include "source/ui/ui_01_SobreDialog.hpp"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QEvent>
#include <QFileInfo>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPalette>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QSizePolicy>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTextDocument>
#include <QTextEdit>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

#include "source/eisenhower_app.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"
#include "source/utils/ApplicationPathUtils.hpp"
#include "source/utils/BarraTitulosUtils.hpp"
#include "source/utils/LogManager.hpp"

namespace source::GerenciamentoUI {
namespace {

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

struct LicenseReference {
    QString name;
    QString filename;
    QString url;
};

struct AboutSearchMatch {
    QWidget *widget = nullptr;
    int start = -1;
    int length = 0;
};

constexpr const char *kLicenseDocumentBaseProperty = "licenseDocumentBaseBackground";
constexpr const char *kLicenseActionButtonProperty = "licenseActionButton";
constexpr const char *kAboutSearchTextLabelProperty = "aboutSearchTextLabel";
constexpr const char *kAboutSearchPlainTextProperty = "aboutSearchPlainText";

QString html_or_info(const QString &value, const QString &info)
{
    return value.isEmpty() ? QStringLiteral("<p>%1.</p>").arg(info) : value;
}

void configure_document_tabs(QTabWidget *document_tabs)
{
    if (document_tabs == nullptr) {
        return;
    }
    document_tabs->setDocumentMode(true);
    document_tabs->setUsesScrollButtons(true);
    document_tabs->setElideMode(Qt::ElideRight);
}

QString translated_or_fallback(const QString &value, const QString &key, const QString &fallback)
{
    return value.isEmpty() || value == key ? fallback : value;
}

QVector<LicenseReference> parse_license_references(const QString &raw_references)
{
    const QStringList lines = raw_references.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    QVector<LicenseReference> references;
    references.reserve(lines.size());

    for (const QString &raw_line : lines) {
        const QString line = raw_line.trimmed();
        if (line.isEmpty()) {
            continue;
        }

        const QStringList fields = line.split(QLatin1Char('|'));
        LicenseReference reference;
        if (fields.size() >= 3) {
            reference.name = fields.at(0).trimmed();
            reference.filename = fields.at(1).trimmed();
            reference.url = fields.at(2).trimmed();
        } else {
            reference.name = line;
            reference.url = line;
        }

        if (!reference.name.isEmpty()) {
            references.append(reference);
        }
    }

    return references;
}

void apply_document_base_background(QWidget *widget, const QPalette &source_palette)
{
    if (widget == nullptr) {
        return;
    }

    widget->setProperty(kLicenseDocumentBaseProperty, true);
    QPalette palette = widget->palette();
    const QColor base = source_palette.color(QPalette::Base);
    for (QPalette::ColorGroup group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
        palette.setColor(group, QPalette::Window, base);
        palette.setColor(group, QPalette::Base, base);
        palette.setColor(group, QPalette::AlternateBase, base);
    }
    widget->setPalette(palette);
    widget->setAutoFillBackground(true);
}

void refresh_document_base_backgrounds(QWidget *root, const QPalette &source_palette)
{
    if (root == nullptr) {
        return;
    }

    if (root->property(kLicenseDocumentBaseProperty).toBool()) {
        apply_document_base_background(root, source_palette);
    }

    const auto children = root->findChildren<QWidget *>();
    for (QWidget *child : children) {
        if (child != nullptr && child->property(kLicenseDocumentBaseProperty).toBool()) {
            apply_document_base_background(child, source_palette);
        }
    }
}

void focus_license_action_button(QPushButton *button)
{
    if (button == nullptr || !button->isEnabled()) {
        return;
    }

    QWidget *previous_focus = button->window() != nullptr ? button->window()->focusWidget() : nullptr;
    button->setFocus(Qt::MouseFocusReason);
    if (previous_focus != nullptr && previous_focus != button) {
        previous_focus->update();
    }
    button->update();
}

void select_license_action_button(QPushButton *button)
{
    if (button == nullptr || !button->isEnabled()) {
        return;
    }

    if (QWidget *root = button->window()) {
        const auto buttons = root->findChildren<QPushButton *>();
        for (QPushButton *candidate : buttons) {
            if (candidate != nullptr && candidate->property(kLicenseActionButtonProperty).toBool()) {
                candidate->setDefault(false);
                candidate->update();
            }
        }
    }

    focus_license_action_button(button);
    button->setDefault(true);
    button->repaint();
}

void configure_license_action_button(QPushButton *button)
{
    if (button == nullptr) {
        return;
    }

    button->setMinimumWidth(72);
    button->setFocusPolicy(Qt::StrongFocus);
    button->setAutoDefault(false);
    button->setDefault(false);
    button->setProperty(kLicenseActionButtonProperty, true);
    QObject::connect(button, &QPushButton::pressed, button, [button]() {
        select_license_action_button(button);
    });
}

QVector<AboutSearchMatch> browser_search_matches(QTextBrowser *browser, const QString &query)
{
    QVector<AboutSearchMatch> matches;
    if (browser == nullptr || browser->document() == nullptr || query.isEmpty()) {
        return matches;
    }

    QTextCursor cursor(browser->document());
    while (true) {
        cursor = browser->document()->find(query, cursor);
        if (cursor.isNull()) {
            break;
        }
        matches.append({browser, cursor.selectionStart(), cursor.selectionEnd() - cursor.selectionStart()});
    }
    return matches;
}

QString highlighted_label_html(const QString &plain_text,
                               const QString &query,
                               const QPalette &palette,
                               int current_start)
{
    if (query.isEmpty()) {
        return plain_text.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br/>"));
    }

    const QColor highlight = palette.color(QPalette::Highlight);
    const QColor current = QColor(highlight.red(), highlight.green(), highlight.blue(), 205);
    const QColor normal = QColor(highlight.red(), highlight.green(), highlight.blue(), 95);
    const QColor highlighted_text = palette.color(QPalette::HighlightedText);

    QString html;
    int pos = 0;
    while (pos < plain_text.size()) {
        const int found = plain_text.indexOf(query, pos, Qt::CaseInsensitive);
        if (found < 0) {
            html += plain_text.mid(pos).toHtmlEscaped();
            break;
        }

        html += plain_text.mid(pos, found - pos).toHtmlEscaped();
        const QString fragment = plain_text.mid(found, query.size()).toHtmlEscaped();
        const QColor background = found == current_start ? current : normal;
        const QString foreground =
            found == current_start ? QStringLiteral(" color:%1;").arg(highlighted_text.name()) : QString();
        html += QStringLiteral("<span style=\"background-color:%1;%2\">%3</span>")
                    .arg(background.name(QColor::HexArgb), foreground, fragment);
        pos = found + query.size();
    }

    html.replace(QLatin1Char('\n'), QStringLiteral("<br/>"));
    return html;
}

QVector<AboutSearchMatch> label_search_matches(QLabel *label, const QString &query)
{
    QVector<AboutSearchMatch> matches;
    if (label == nullptr || query.isEmpty() || !label->property(kAboutSearchTextLabelProperty).toBool()) {
        return matches;
    }

    const QString plain_text = label->property(kAboutSearchPlainTextProperty).toString();
    int pos = 0;
    while (pos < plain_text.size()) {
        const int found = plain_text.indexOf(query, pos, Qt::CaseInsensitive);
        if (found < 0) {
            break;
        }
        matches.append({label, found, static_cast<int>(query.size())});
        pos = found + query.size();
    }
    return matches;
}

QVector<AboutSearchMatch> collect_about_search_matches(QWidget *root, const QString &query, bool visible_only)
{
    QVector<AboutSearchMatch> matches;
    if (root == nullptr || query.trimmed().isEmpty()) {
        return matches;
    }

    auto append_browser = [&](QTextBrowser *browser) {
        if (browser == nullptr || (visible_only && !browser->isVisibleTo(root))) {
            return;
        }
        matches += browser_search_matches(browser, query.trimmed());
    };
    auto append_label = [&](QLabel *label) {
        if (label == nullptr || (visible_only && !label->isVisibleTo(root))) {
            return;
        }
        matches += label_search_matches(label, query.trimmed());
    };

    append_browser(qobject_cast<QTextBrowser *>(root));
    for (QTextBrowser *browser : root->findChildren<QTextBrowser *>()) {
        append_browser(browser);
    }

    append_label(qobject_cast<QLabel *>(root));
    for (QLabel *label : root->findChildren<QLabel *>()) {
        append_label(label);
    }

    return matches;
}

void apply_about_search_highlights(QWidget *root,
                                   const QString &query,
                                   QWidget *current_widget,
                                   int current_start)
{
    if (root == nullptr) {
        return;
    }

    const QString trimmed = query.trimmed();
    const QColor highlight = root->palette().color(QPalette::Highlight);
    const QColor highlighted_text = root->palette().color(QPalette::HighlightedText);

    auto apply_browser = [&](QTextBrowser *browser) {
        if (browser == nullptr) {
            return;
        }

        QList<QTextEdit::ExtraSelection> selections;
        const QVector<AboutSearchMatch> matches = browser_search_matches(browser, trimmed);
        for (const AboutSearchMatch &match : matches) {
            QTextCursor cursor(browser->document());
            cursor.setPosition(match.start);
            cursor.setPosition(match.start + match.length, QTextCursor::KeepAnchor);

            QColor background = highlight;
            background.setAlpha(browser == current_widget && match.start == current_start ? 190 : 95);

            QTextEdit::ExtraSelection selection;
            selection.cursor = cursor;
            selection.format.setBackground(background);
            if (browser == current_widget && match.start == current_start) {
                selection.format.setForeground(highlighted_text);
            }
            selections.append(selection);
        }
        browser->setExtraSelections(selections);
    };

    auto apply_label = [&](QLabel *label) {
        if (label == nullptr || !label->property(kAboutSearchTextLabelProperty).toBool()) {
            return;
        }
        const QString plain_text = label->property(kAboutSearchPlainTextProperty).toString();
        if (trimmed.isEmpty()) {
            label->setTextFormat(Qt::PlainText);
            label->setText(plain_text);
            return;
        }
        label->setTextFormat(Qt::RichText);
        label->setText(highlighted_label_html(plain_text,
                                              trimmed,
                                              root->palette(),
                                              label == current_widget ? current_start : -1));
    };

    apply_browser(qobject_cast<QTextBrowser *>(root));
    for (QTextBrowser *browser : root->findChildren<QTextBrowser *>()) {
        apply_browser(browser);
    }

    apply_label(qobject_cast<QLabel *>(root));
    for (QLabel *label : root->findChildren<QLabel *>()) {
        apply_label(label);
    }
}

int find_about_search_match_index(const QVector<AboutSearchMatch> &matches, QWidget *widget, int start)
{
    for (int index = 0; index < matches.size(); ++index) {
        if (matches.at(index).widget == widget && matches.at(index).start == start) {
            return index;
        }
    }
    return -1;
}

void scroll_about_search_match_to_view(const AboutSearchMatch &match)
{
    if (auto *browser = qobject_cast<QTextBrowser *>(match.widget)) {
        QTextCursor cursor(browser->document());
        cursor.setPosition(match.start);
        cursor.setPosition(match.start + match.length, QTextCursor::KeepAnchor);
        browser->setTextCursor(cursor);
        browser->ensureCursorVisible();
        return;
    }

    if (auto *label = qobject_cast<QLabel *>(match.widget)) {
        QWidget *parent = label->parentWidget();
        while (parent != nullptr) {
            if (auto *scroll = qobject_cast<QScrollArea *>(parent)) {
                const QPoint mapped = label->mapTo(scroll->widget(), QPoint(0, 0));
                if (scroll->verticalScrollBar() != nullptr) {
                    scroll->verticalScrollBar()->setValue(mapped.y());
                }
                return;
            }
            parent = parent->parentWidget();
        }
    }
}

}  // namespace

SobreDialog::SobreDialog(QWidget *parent,
                         const QString &titulo,
                         const QString &texto_fixo,
                         const QString &texto_history,
                         const QString &detalhes,
                         const QString &licencas,
                         const QString &sites_licencas,
                         const QString &show_history_text,
                         const QString &hide_history_text,
                         const QString &show_details_text,
                         const QString &hide_details_text,
                         const QString &show_licenses_text,
                         const QString &hide_licenses_text,
                         const QString &ok_text,
                         const QString &site_oficial_text,
                         const QString &avisos,
                         const QString &show_notices_text,
                         const QString &hide_notices_text,
                         const QString &privacy_policy,
                         const QString &show_privacy_policy_text,
                         const QString &hide_privacy_policy_text,
                         const QString &info_not_available_text,
                         const QString &release_notes,
                         const QString &show_release_notes_text,
                         const QString &hide_release_notes_text,
                         QWidget *app_for_translation)
    : QDialog(parent), _app_for_translation(app_for_translation)
{
    try {
        _license_sites = sites_licencas.trimmed();
        _site_oficial_text =
            site_oficial_text.isEmpty()
                ? translated_or_fallback(_tr_multi(QStringLiteral("site_oficial")),
                                         QStringLiteral("site_oficial"),
                                         QStringLiteral("Site oficial"))
                : site_oficial_text;

        setWindowTitle(titulo);
        setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint
                       | Qt::WindowCloseButtonHint);
        setModal(false);

        auto *layout = new QVBoxLayout(this);
        layout->setSpacing(0);

        fixed_label = new QLabel(texto_fixo, this);
        fixed_label->setTextFormat(Qt::RichText);
        fixed_label->setWordWrap(true);
        fixed_label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        fixed_label->setContentsMargins(0, 0, 0, 0);
        fixed_label->setIndent(0);
        fixed_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        layout->addWidget(fixed_label);
        _update_header_height();

        auto *search_layout = new QGridLayout();
        search_layout->setContentsMargins(0, fixed_label->fontMetrics().lineSpacing(), 0, 6);
        search_layout->setHorizontalSpacing(8);
        search_layout->setColumnStretch(0, 0);
        search_layout->setColumnStretch(1, 1);
        about_search_label = new QLabel(this);
        about_search_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        about_search_edit = new QLineEdit(this);
        about_search_edit->setClearButtonEnabled(true);
        about_search_previous_button = new QToolButton(this);
        about_search_next_button = new QToolButton(this);
        about_search_previous_button->setArrowType(Qt::LeftArrow);
        about_search_next_button->setArrowType(Qt::RightArrow);
        about_search_previous_button->setAutoRaise(true);
        about_search_next_button->setAutoRaise(true);
        about_search_previous_button->setEnabled(false);
        about_search_next_button->setEnabled(false);

        auto *search_controls = new QHBoxLayout();
        search_controls->setContentsMargins(0, 0, 0, 0);
        search_controls->setSpacing(4);
        search_controls->addWidget(about_search_edit, 1);
        search_controls->addWidget(about_search_previous_button);
        search_controls->addWidget(about_search_next_button);
        search_layout->addWidget(about_search_label, 0, 0);
        search_layout->addLayout(search_controls, 0, 1);
        layout->addLayout(search_layout);
        layout->addSpacing(about_search_edit->fontMetrics().lineSpacing());

        const QString sh_history = show_history_text.isEmpty() ? QStringLiteral("Histórico") : show_history_text;
        const QString hi_history = hide_history_text.isEmpty() ? QStringLiteral("Ocultar histórico") : hide_history_text;
        const QString sh_details = show_details_text.isEmpty() ? QStringLiteral("Detalhes") : show_details_text;
        const QString hi_details = hide_details_text.isEmpty() ? QStringLiteral("Ocultar detalhes") : hide_details_text;
        const QString sh_licenses = show_licenses_text.isEmpty() ? QStringLiteral("Licenças") : show_licenses_text;
        const QString hi_licenses =
            hide_licenses_text.isEmpty() ? QStringLiteral("Ocultar licenças") : hide_licenses_text;
        const QString sh_contract =
            translated_or_fallback(_tr_multi(QStringLiteral("show_contract")),
                                   QStringLiteral("show_contract"),
                                   QStringLiteral("CONTRATO"));
        const QString hi_contract =
            translated_or_fallback(_tr_multi(QStringLiteral("hide_contract")),
                                   QStringLiteral("hide_contract"),
                                   QStringLiteral("Ocultar contrato"));
        const QString sh_copyright =
            translated_or_fallback(_tr_multi(QStringLiteral("show_copyright")),
                                   QStringLiteral("show_copyright"),
                                   QStringLiteral("DIREITOS AUTORAIS"));
        const QString hi_copyright =
            translated_or_fallback(_tr_multi(QStringLiteral("hide_copyright")),
                                   QStringLiteral("hide_copyright"),
                                   QStringLiteral("Ocultar direitos autorais"));
        const QString sh_notices = show_notices_text.isEmpty() ? QStringLiteral("Avisos") : show_notices_text;
        const QString hi_notices = hide_notices_text.isEmpty() ? QStringLiteral("Ocultar avisos") : hide_notices_text;
        const QString sh_privacy =
            show_privacy_policy_text.isEmpty() ? QStringLiteral("Política de privacidade") : show_privacy_policy_text;
        const QString hi_privacy = hide_privacy_policy_text.isEmpty() ? QStringLiteral("Ocultar política de privacidade")
                                                                      : hide_privacy_policy_text;
        const QString sh_release =
            show_release_notes_text.isEmpty() ? QStringLiteral("Notas de versão") : show_release_notes_text;
        const QString hi_release = hide_release_notes_text.isEmpty() ? QStringLiteral("Ocultar notas de versão")
                                                                     : hide_release_notes_text;
        const QString ok_text_final = ok_text.isEmpty() ? QStringLiteral("OK") : ok_text;
        tabs = new QTabWidget(this);
        tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        history_browser = new QTextBrowser(this);
        history_browser->setReadOnly(true);
        history_browser->setOpenExternalLinks(true);
        if (!texto_history.isEmpty()) {
            history_browser->setPlainText(texto_history);
        } else {
            history_browser->setHtml(html_or_info(QString(), info_not_available_text));
        }
        tabs->addTab(history_browser, sh_history);

        detalhes_tabs = new QTabWidget(this);
        configure_document_tabs(detalhes_tabs);
        _set_details_documents({{sh_details, detalhes}}, info_not_available_text);
        tabs->addTab(detalhes_tabs, sh_details);

        licencas_tabs = new QTabWidget(this);
        configure_document_tabs(licencas_tabs);
        _set_license_documents({{sh_licenses, licencas}}, info_not_available_text);
        tabs->addTab(licencas_tabs, sh_licenses);

        contrato_tabs = new QTabWidget(this);
        configure_document_tabs(contrato_tabs);
        _set_contract_documents({}, info_not_available_text);
        tabs->addTab(contrato_tabs, sh_contract);

        direitos_autorais_tabs = new QTabWidget(this);
        configure_document_tabs(direitos_autorais_tabs);
        _set_copyright_documents({}, info_not_available_text);
        tabs->addTab(direitos_autorais_tabs, sh_copyright);

        avisos_tabs = new QTabWidget(this);
        configure_document_tabs(avisos_tabs);
        _set_notice_documents({{sh_notices, avisos}}, info_not_available_text);
        tabs->addTab(avisos_tabs, sh_notices);

        privacidade_tabs = new QTabWidget(this);
        configure_document_tabs(privacidade_tabs);
        _set_privacy_policy_documents({{sh_privacy, privacy_policy}}, info_not_available_text);
        tabs->addTab(privacidade_tabs, sh_privacy);

        release_notes_browser = new QTextBrowser(this);
        release_notes_browser->setReadOnly(true);
        release_notes_browser->setOpenExternalLinks(true);
        if (!release_notes.isEmpty()) {
            release_notes_browser->setPlainText(release_notes);
        } else {
            release_notes_browser->setHtml(html_or_info(QString(), info_not_available_text));
        }
        tabs->addTab(release_notes_browser, sh_release);

        const auto connect_document_tab_search_refresh = [this](QTabWidget *document_tabs) {
            if (document_tabs == nullptr) {
                return;
            }
            connect(document_tabs, &QTabWidget::currentChanged, this, [this]() {
                _about_search_current_widget = nullptr;
                _about_search_current_start = -1;
                _update_about_search_buttons();
            });
        };
        connect_document_tab_search_refresh(detalhes_tabs);
        connect_document_tab_search_refresh(licencas_tabs);
        connect_document_tab_search_refresh(contrato_tabs);
        connect_document_tab_search_refresh(direitos_autorais_tabs);
        connect_document_tab_search_refresh(avisos_tabs);
        connect_document_tab_search_refresh(privacidade_tabs);

        _tab_show_texts = {sh_history,
                           sh_details,
                           sh_licenses,
                           sh_contract,
                           sh_copyright,
                           sh_notices,
                           sh_privacy,
                           sh_release};
        _tab_hide_texts = {hi_history,
                           hi_details,
                           hi_licenses,
                           hi_contract,
                           hi_copyright,
                           hi_notices,
                           hi_privacy,
                           hi_release};
        connect(tabs, &QTabWidget::currentChanged, this, &SobreDialog::_on_tab_changed);
        connect(about_search_edit, &QLineEdit::textChanged, this, [this]() {
            _about_search_current_widget = nullptr;
            _about_search_current_start = -1;
            _apply_about_search();
            _update_about_search_buttons();
        });
        connect(about_search_edit, &QLineEdit::returnPressed, this, [this]() { _navigate_about_search(1); });
        connect(about_search_previous_button, &QToolButton::clicked, this, [this]() {
            _navigate_about_search(-1);
        });
        connect(about_search_next_button, &QToolButton::clicked, this, [this]() {
            _navigate_about_search(1);
        });
        _retranslate_about_search_controls();
        _update_tab_labels(tabs->currentIndex());
        layout->addWidget(tabs);

        auto *button_layout = new QHBoxLayout();
        button_layout->setContentsMargins(0, 8, 0, 0);
        ok_button = new QPushButton(ok_text_final, this);
        ok_button->setAutoDefault(false);
        ok_button->setDefault(false);
        ok_button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        connect(ok_button, &QPushButton::clicked, this, &QDialog::accept);
        button_layout->addStretch(1);
        button_layout->addWidget(ok_button);
        layout->addLayout(button_layout);

        setMinimumSize(400, 200);
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao criar dialog sobre."));
    }
}

void SobreDialog::setHeaderBuilder(std::function<QString()> builder)
{
    _header_builder = std::move(builder);
}

void SobreDialog::setDetailsDocuments(const QVector<DocumentoSobre> &documents)
{
    _set_details_documents(documents, _tr_multi(QStringLiteral("information_not_available")));
}

void SobreDialog::setLicenseDocuments(const QVector<DocumentoSobre> &documents)
{
    _set_license_documents(documents, _tr_multi(QStringLiteral("information_not_available")));
}

void SobreDialog::setContractDocuments(const QVector<DocumentoSobre> &documents)
{
    _set_contract_documents(documents, _tr_multi(QStringLiteral("information_not_available")));
}

void SobreDialog::setCopyrightDocuments(const QVector<DocumentoSobre> &documents)
{
    _set_copyright_documents(documents, _tr_multi(QStringLiteral("information_not_available")));
}

void SobreDialog::setNoticeDocuments(const QVector<DocumentoSobre> &documents)
{
    _set_notice_documents(documents, _tr_multi(QStringLiteral("information_not_available")));
}

void SobreDialog::setPrivacyPolicyDocuments(const QVector<DocumentoSobre> &documents)
{
    _set_privacy_policy_documents(documents, _tr_multi(QStringLiteral("information_not_available")));
}

void SobreDialog::_on_tab_changed(int index)
{
    _about_search_current_widget = nullptr;
    _about_search_current_start = -1;
    _update_tab_labels(index);
    _update_about_search_buttons();
}

void SobreDialog::_retranslate_about_search_controls()
{
    if (about_search_label != nullptr) {
        const QString label = _tr_multi(QStringLiteral("about_search_label"));
        about_search_label->setText(label == QStringLiteral("about_search_label") ? QStringLiteral("Buscar:")
                                                                                   : label);
    }
    if (about_search_edit != nullptr) {
        const QString placeholder = _tr_multi(QStringLiteral("about_search_placeholder"));
        about_search_edit->setPlaceholderText(placeholder == QStringLiteral("about_search_placeholder")
                                                  ? QStringLiteral("Buscar texto na janela Sobre...")
                                                  : placeholder);
    }
    if (about_search_previous_button != nullptr) {
        const QString previous = _tr_multi(QStringLiteral("about_search_previous"));
        about_search_previous_button->setToolTip(previous == QStringLiteral("about_search_previous")
                                                     ? QStringLiteral("Resultado anterior")
                                                     : previous);
        about_search_previous_button->setAccessibleName(about_search_previous_button->toolTip());
    }
    if (about_search_next_button != nullptr) {
        const QString next = _tr_multi(QStringLiteral("about_search_next"));
        about_search_next_button->setToolTip(next == QStringLiteral("about_search_next")
                                                 ? QStringLiteral("Próximo resultado")
                                                 : next);
        about_search_next_button->setAccessibleName(about_search_next_button->toolTip());
    }
}

void SobreDialog::_update_header_height()
{
    if (fixed_label == nullptr) {
        return;
    }

    QTextDocument doc;
    doc.setDefaultFont(fixed_label->font());
    doc.setHtml(fixed_label->text());
    doc.setDocumentMargin(0);
    const int available_width = qMax(1, fixed_label->contentsRect().width() > 0 ? fixed_label->contentsRect().width()
                                                                                : width());
    doc.setTextWidth(available_width);
    int height = qCeil(doc.size().height()) + 1;
    if (height <= 1) {
        height = fixed_label->sizeHint().height();
    }
    height = qMax(1, height);
    fixed_label->setMinimumHeight(height);
    fixed_label->setMaximumHeight(height);
}

void SobreDialog::_apply_about_search()
{
    if (tabs == nullptr) {
        return;
    }

    const QString query = about_search_edit != nullptr ? about_search_edit->text() : QString();
    if (query.trimmed().isEmpty()) {
        _about_search_current_widget = nullptr;
        _about_search_current_start = -1;
    }
    apply_about_search_highlights(tabs, query, _about_search_current_widget, _about_search_current_start);
}

void SobreDialog::_navigate_about_search(int step)
{
    if (tabs == nullptr || about_search_edit == nullptr || step == 0) {
        return;
    }

    QWidget *current_page = tabs->currentWidget();
    const QString query = about_search_edit->text().trimmed();
    const QVector<AboutSearchMatch> matches = collect_about_search_matches(current_page, query, true);
    if (matches.isEmpty()) {
        _about_search_current_widget = nullptr;
        _about_search_current_start = -1;
        _apply_about_search();
        _update_about_search_buttons();
        return;
    }

    int index = find_about_search_match_index(matches, _about_search_current_widget, _about_search_current_start);
    if (index < 0) {
        index = step < 0 ? matches.size() - 1 : 0;
    } else {
        index = (index + step + matches.size()) % matches.size();
    }

    const AboutSearchMatch match = matches.at(index);
    _about_search_current_widget = match.widget;
    _about_search_current_start = match.start;
    _apply_about_search();
    scroll_about_search_match_to_view(match);
    _update_about_search_buttons();
}

void SobreDialog::_update_about_search_buttons()
{
    const bool has_query = about_search_edit != nullptr && !about_search_edit->text().trimmed().isEmpty();
    const bool has_matches =
        has_query && tabs != nullptr
        && !collect_about_search_matches(tabs->currentWidget(), about_search_edit->text().trimmed(), true).isEmpty();

    if (about_search_previous_button != nullptr) {
        about_search_previous_button->setEnabled(has_matches);
    }
    if (about_search_next_button != nullptr) {
        about_search_next_button->setEnabled(has_matches);
    }
}

void SobreDialog::_set_document_tabs(QTabWidget *document_tabs,
                                     const QVector<DocumentoSobre> &documents,
                                     const QString &info_not_available_text,
                                     const QString &fallback_title,
                                     const QString &official_sites,
                                     const QString &official_site_label,
                                     bool append_official_sites_to_first_document)
{
    if (document_tabs == nullptr) {
        return;
    }

    const int previous_index = document_tabs->currentIndex();
    _about_search_current_widget = nullptr;
    _about_search_current_start = -1;
    while (document_tabs->count() > 0) {
        QWidget *page = document_tabs->widget(0);
        document_tabs->removeTab(0);
        if (page != nullptr) {
            page->deleteLater();
        }
    }

    QVector<DocumentoSobre> visible_documents = documents;
    if (visible_documents.isEmpty()) {
        visible_documents.append({fallback_title, QString()});
    }

    for (int index = 0; index < visible_documents.size(); ++index) {
        const DocumentoSobre &document = visible_documents.at(index);
        const bool append_official_sites = append_official_sites_to_first_document && index == 0
                                           && !official_sites.trimmed().isEmpty();
        QWidget *page = nullptr;
        if (append_official_sites) {
            page = _create_license_document_page(document_tabs,
                                                 document,
                                                 info_not_available_text,
                                                 official_sites,
                                                 official_site_label);
        } else {
            auto *browser = new QTextBrowser(document_tabs);
            page = browser;
            browser->setReadOnly(true);
            browser->setOpenExternalLinks(false);
            if (!document.texto.isEmpty()) {
                browser->setPlainText(document.texto);
            } else {
                browser->setHtml(html_or_info(QString(), info_not_available_text));
            }
        }
        document_tabs->addTab(page, document.titulo.isEmpty() ? fallback_title : document.titulo);
    }

    if (document_tabs->count() > 0) {
        int target_index = previous_index;
        if (target_index < 0) {
            target_index = 0;
        }
        if (target_index >= document_tabs->count()) {
            target_index = document_tabs->count() - 1;
        }
        document_tabs->setCurrentIndex(target_index);
    }
    _apply_about_search();
    _update_about_search_buttons();
}

QWidget *SobreDialog::_create_license_document_page(QWidget *parent,
                                                    const DocumentoSobre &document,
                                                    const QString &info_not_available_text,
                                                    const QString &license_references,
                                                    const QString &section_label)
{
    auto *scroll = new QScrollArea(parent);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::StyledPanel);
    apply_document_base_background(scroll, scroll->palette());
    apply_document_base_background(scroll->viewport(), scroll->palette());

    auto *content = new QWidget(scroll);
    apply_document_base_background(content, scroll->palette());
    content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(0);
    layout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    auto *text = new QLabel(content);
    text->setTextFormat(Qt::PlainText);
    text->setWordWrap(true);
    text->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    text->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    QSizePolicy text_policy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    text_policy.setHeightForWidth(true);
    text->setSizePolicy(text_policy);
    text->setText(document.texto.isEmpty() ? info_not_available_text : document.texto);
    text->setProperty(kAboutSearchTextLabelProperty, true);
    text->setProperty(kAboutSearchPlainTextProperty, text->text());
    layout->addWidget(text);

    if (auto *license_panel = _create_license_reference_panel(content, license_references, section_label)) {
        layout->addSpacing(text->fontMetrics().lineSpacing());
        layout->addWidget(license_panel);
    }

    scroll->setWidget(content);
    return scroll;
}

QWidget *SobreDialog::_create_license_reference_panel(QWidget *parent,
                                                      const QString &license_references,
                                                      const QString &section_label)
{
    const QVector<LicenseReference> references = parse_license_references(license_references);
    if (references.isEmpty()) {
        return nullptr;
    }

    auto *container = new QWidget(parent);
    apply_document_base_background(container, parent != nullptr ? parent->palette() : container->palette());
    auto *outer_layout = new QVBoxLayout(container);
    outer_layout->setContentsMargins(0, 0, 0, 0);
    outer_layout->setSpacing(0);

    auto *title = new QLabel(section_label, container);
    QFont title_font = title->font();
    title_font.setBold(true);
    title->setFont(title_font);
    title->setTextFormat(Qt::PlainText);
    title->setTextInteractionFlags(Qt::NoTextInteraction);
    outer_layout->addWidget(title);

    auto *list = new QWidget(container);
    apply_document_base_background(list, container->palette());
    auto *grid = new QGridLayout(list);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(8);
    grid->setVerticalSpacing(0);
    grid->setColumnStretch(0, 1);

    const QString open_text =
        translated_or_fallback(_tr_multi(QStringLiteral("license_open_button")),
                               QStringLiteral("license_open_button"),
                               translated_or_fallback(_tr_multi(QStringLiteral("Abrir")),
                                                      QStringLiteral("Abrir"),
                                                      QStringLiteral("Abrir")));
    const QString site_text = translated_or_fallback(_tr_multi(QStringLiteral("license_site_button")),
                                                     QStringLiteral("license_site_button"),
                                                     QStringLiteral("Site"));

    for (int row = 0; row < references.size(); ++row) {
        const LicenseReference reference = references.at(row);

        auto *name_label = new QLabel(reference.name, list);
        name_label->setTextFormat(Qt::PlainText);
        name_label->setTextInteractionFlags(Qt::NoTextInteraction);
        name_label->setWordWrap(true);
        grid->addWidget(name_label, row, 0);

        auto *open_button = new QPushButton(open_text, list);
        configure_license_action_button(open_button);
        open_button->setEnabled(!reference.filename.isEmpty());
        connect(open_button, &QPushButton::clicked, this, [this, open_button, reference]() {
            select_license_action_button(open_button);
            _open_license_document(reference.name, reference.filename);
        });
        grid->addWidget(open_button, row, 1);

        auto *site_button = new QPushButton(site_text, list);
        configure_license_action_button(site_button);
        site_button->setEnabled(!reference.url.isEmpty());
        connect(site_button, &QPushButton::clicked, this, [this, site_button, reference]() {
            select_license_action_button(site_button);
            _open_license_site(reference.url);
        });
        grid->addWidget(site_button, row, 2);
    }

    outer_layout->addWidget(list);
    return container;
}

QString SobreDialog::_license_document_window_title(const QString &title) const
{
    const QString dialog_title_prefix =
        translated_or_fallback(_tr_multi(QStringLiteral("license_document_window_title")),
                               QStringLiteral("license_document_window_title"),
                               QStringLiteral("Licença"));
    if (title.isEmpty()) {
        return dialog_title_prefix;
    }

    return translated_or_fallback(_tr_multi(QStringLiteral("license_document_window_title_format")),
                                  QStringLiteral("license_document_window_title_format"),
                                  QStringLiteral("%1 - %2"))
        .arg(dialog_title_prefix, title);
}

void SobreDialog::_refresh_open_license_document_titles()
{
    for (auto it = _license_document_dialogs.begin(); it != _license_document_dialogs.end();) {
        QDialog *dialog = it->data();
        if (dialog == nullptr) {
            it = _license_document_dialogs.erase(it);
            continue;
        }

        dialog->setWindowTitle(_license_document_window_title(dialog->property("licenseDocumentTitle").toString()));
        ++it;
    }
}

void SobreDialog::_open_license_document(const QString &title, const QString &filename)
{
    const QString file_path = source::utils::get_text_file_path(filename, QStringLiteral("LICENSES"));
    if (filename.trimmed().isEmpty() || file_path.isEmpty() || !QFileInfo::exists(file_path)) {
        const QString message =
            translated_or_fallback(_tr_multi(QStringLiteral("license_file_not_found_message")),
                                   QStringLiteral("license_file_not_found_message"),
                                   QStringLiteral("Arquivo de licença não encontrado: %1"))
                .arg(filename);
        QMessageBox::warning(this,
                             translated_or_fallback(_tr_multi(QStringLiteral("license_file_not_found_title")),
                                                    QStringLiteral("license_file_not_found_title"),
                                                    QStringLiteral("Licença não encontrada")),
                             message);
        return;
    }

    const QString text = source::utils::load_text_file(filename, QStringLiteral("LICENSES"));
    if (text.trimmed().isEmpty()) {
        const QString message =
            translated_or_fallback(_tr_multi(QStringLiteral("license_file_not_found_message")),
                                   QStringLiteral("license_file_not_found_message"),
                                   QStringLiteral("Arquivo de licença não encontrado: %1"))
                .arg(filename);
        QMessageBox::warning(this,
                             translated_or_fallback(_tr_multi(QStringLiteral("license_file_not_found_title")),
                                                    QStringLiteral("license_file_not_found_title"),
                                                    QStringLiteral("Licença não encontrada")),
                             message);
        return;
    }

    auto *dialog = new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setModal(false);
    dialog->setProperty("licenseDocumentTitle", title);
    dialog->setWindowTitle(_license_document_window_title(title));
    _license_document_dialogs.append(QPointer<QDialog>(dialog));
    connect(dialog, &QObject::destroyed, this, [this, dialog]() {
        for (auto it = _license_document_dialogs.begin(); it != _license_document_dialogs.end();) {
            if (it->isNull() || it->data() == dialog) {
                it = _license_document_dialogs.erase(it);
            } else {
                ++it;
            }
        }
    });

    auto *layout = new QVBoxLayout(dialog);
    auto *viewer = new QTextBrowser(dialog);
    viewer->setReadOnly(true);
    viewer->setOpenExternalLinks(false);
    viewer->setPlainText(text);
    layout->addWidget(viewer, 1);

    auto *button_layout = new QHBoxLayout();
    auto *close_button = new QPushButton(translated_or_fallback(_tr_multi(QStringLiteral("OK")),
                                                               QStringLiteral("OK"),
                                                               QStringLiteral("OK")),
                                         dialog);
    connect(close_button, &QPushButton::clicked, dialog, &QDialog::accept);
    button_layout->addStretch(1);
    button_layout->addWidget(close_button);
    layout->addLayout(button_layout);

    dialog->resize(760, 560);
    dialog->show();
}

void SobreDialog::_open_license_site(const QString &url)
{
    if (url.trimmed().isEmpty() || !QDesktopServices::openUrl(QUrl::fromUserInput(url))) {
        QMessageBox::warning(this,
                             translated_or_fallback(_tr_multi(QStringLiteral("license_site_open_error_title")),
                                                    QStringLiteral("license_site_open_error_title"),
                                                    QStringLiteral("Site não aberto")),
                             translated_or_fallback(_tr_multi(QStringLiteral("license_site_open_error_message")),
                                                    QStringLiteral("license_site_open_error_message"),
                                                    QStringLiteral("Não foi possível abrir o site oficial desta licença.")));
    }
}

void SobreDialog::_set_details_documents(const QVector<DocumentoSobre> &documents,
                                         const QString &info_not_available_text)
{
    _set_document_tabs(detalhes_tabs,
                       documents,
                       info_not_available_text,
                       translated_or_fallback(_tr_multi(QStringLiteral("show_details")),
                                              QStringLiteral("show_details"),
                                              QStringLiteral("Detalhes")));
}

void SobreDialog::_set_license_documents(const QVector<DocumentoSobre> &documents,
                                         const QString &info_not_available_text)
{
    _set_document_tabs(licencas_tabs,
                       documents,
                       info_not_available_text,
                       translated_or_fallback(_tr_multi(QStringLiteral("show_licenses")),
                                              QStringLiteral("show_licenses"),
                                              QStringLiteral("Licenças")),
                       _license_sites,
                       translated_or_fallback(_tr_multi(QStringLiteral("license_references_title")),
                                              QStringLiteral("license_references_title"),
                                              _site_oficial_text),
                       true);
}

void SobreDialog::_set_contract_documents(const QVector<DocumentoSobre> &documents,
                                          const QString &info_not_available_text)
{
    _set_document_tabs(contrato_tabs,
                       documents,
                       info_not_available_text,
                       translated_or_fallback(_tr_multi(QStringLiteral("show_contract")),
                                              QStringLiteral("show_contract"),
                                              QStringLiteral("CONTRATO")));
}

void SobreDialog::_set_copyright_documents(const QVector<DocumentoSobre> &documents,
                                           const QString &info_not_available_text)
{
    _set_document_tabs(direitos_autorais_tabs,
                       documents,
                       info_not_available_text,
                       translated_or_fallback(_tr_multi(QStringLiteral("show_copyright")),
                                              QStringLiteral("show_copyright"),
                                              QStringLiteral("DIREITOS AUTORAIS")));
}

void SobreDialog::_set_notice_documents(const QVector<DocumentoSobre> &documents,
                                        const QString &info_not_available_text)
{
    _set_document_tabs(avisos_tabs,
                       documents,
                       info_not_available_text,
                       translated_or_fallback(_tr_multi(QStringLiteral("show_notices")),
                                              QStringLiteral("show_notices"),
                                              QStringLiteral("Avisos")));
}

void SobreDialog::_set_privacy_policy_documents(const QVector<DocumentoSobre> &documents,
                                                const QString &info_not_available_text)
{
    _set_document_tabs(privacidade_tabs,
                       documents,
                       info_not_available_text,
                       translated_or_fallback(_tr_multi(QStringLiteral("show_privacy_policy")),
                                              QStringLiteral("show_privacy_policy"),
                                              QStringLiteral("Política")));
}

void SobreDialog::_update_tab_labels(int current_index)
{
    const int count = tabs != nullptr ? tabs->count() : 0;
    while (_tab_show_texts.size() < count) {
        _tab_show_texts.append(QString());
    }
    while (_tab_hide_texts.size() < count) {
        _tab_hide_texts.append(QString());
    }

    for (int i = 0; i < count; ++i) {
        const QString show = _tab_show_texts.value(i);
        const QString hide = _tab_hide_texts.value(i);
        tabs->setTabText(i, i == current_index ? hide : show);
    }
}

void SobreDialog::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    if (event == nullptr) {
        return;
    }

    switch (event->type()) {
        case QEvent::LanguageChange:
            _retranslate_ui();
            break;
        case QEvent::ApplicationPaletteChange:
        case QEvent::PaletteChange:
        case QEvent::StyleChange:
        case QEvent::ThemeChange:
            _refresh_license_document_theme();
            break;
        default:
            break;
    }
}

void SobreDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    _update_header_height();
}

void SobreDialog::_refresh_license_document_theme()
{
    if (licencas_tabs == nullptr) {
        return;
    }

    refresh_document_base_backgrounds(licencas_tabs, palette());
    _apply_about_search();
    licencas_tabs->update();
}

QString SobreDialog::_tr_multi(const QString &key) const
{
    QString value = QCoreApplication::translate("App", key.toUtf8().constData());
    if (!value.isEmpty() && value != key) {
        return value;
    }
    value = QCoreApplication::translate("InterfaceGrafica", key.toUtf8().constData());
    return (!value.isEmpty() && value != key) ? value : key;
}

void SobreDialog::_retranslate_ui()
{
    try {
        if (_header_builder) {
            fixed_label->setText(_header_builder());
            _update_header_height();
        }
    } catch (...) {
    }

    QString idioma;
    const auto *app = qobject_cast<const source::EisenhowerMatrixApp *>(_app_for_translation);
    if (app == nullptr) {
        app = qobject_cast<const source::EisenhowerMatrixApp *>(parentWidget());
    }

    if (app != nullptr && app->gerenciador_traducao != nullptr) {
        idioma = app->gerenciador_traducao->obter_idioma_atual();
    }

    if (idioma.isEmpty()) {
        if (auto *owner = parentWidget()) {
            if (auto *translation_mgr = owner->findChild<source::language::GerenciadorTraducao *>()) {
                idioma = translation_mgr->obter_idioma_atual();
            }
        }
    }

    if (idioma.isEmpty() && _app_for_translation != nullptr) {
        if (auto *translation_mgr = _app_for_translation->findChild<source::language::GerenciadorTraducao *>()) {
            idioma = translation_mgr->obter_idioma_atual();
        }
    }

    if (idioma.isEmpty()) {
        const QString loc_name = QLocale().name();
        if (loc_name.startsWith(QStringLiteral("en"), Qt::CaseInsensitive)) {
            idioma = QStringLiteral("en_US");
        } else {
            idioma = QStringLiteral("pt_BR");
        }
    }

    const bool pt = idioma == QStringLiteral("pt_BR");
    const bool include_pomodoro_plugin = app != nullptr && app->has_pomodoro_plugin();
    const bool include_feynman_plugin = app != nullptr && app->has_feynman_plugin();
    const bool include_mapamental_plugin = app != nullptr && app->has_mapamental_plugin();
    const bool include_audiolivro_plugin = app != nullptr && app->has_audiolivro_plugin();
    const bool include_modelo_plugin = app != nullptr && app->has_modelo_plugin();
    const bool include_agenda_plugin = app != nullptr && app->has_agenda_plugin();
    const bool include_gantt_plugin = app != nullptr && app->has_gantt_plugin();
    const QString history =
        pt ? OpcoesSobre::History_APP_pt_BR(include_pomodoro_plugin,
                                            include_feynman_plugin,
                                            include_mapamental_plugin,
                                            include_audiolivro_plugin,
                                            include_agenda_plugin,
                                            include_modelo_plugin,
                                            include_gantt_plugin)
           : OpcoesSobre::History_APP_en_US(include_pomodoro_plugin,
                                            include_feynman_plugin,
                                            include_mapamental_plugin,
                                            include_audiolivro_plugin,
                                            include_agenda_plugin,
                                            include_modelo_plugin,
                                            include_gantt_plugin);
    const QVector<DocumentoSobre> detalhes_documents =
        pt ? OpcoesSobre::About_Documents_pt_BR(include_pomodoro_plugin,
                                                include_feynman_plugin,
                                                include_mapamental_plugin,
                                                include_audiolivro_plugin,
                                                include_agenda_plugin,
                                                include_modelo_plugin,
                                                include_gantt_plugin)
           : OpcoesSobre::About_Documents_en_US(include_pomodoro_plugin,
                                                include_feynman_plugin,
                                                include_mapamental_plugin,
                                                include_audiolivro_plugin,
                                                include_agenda_plugin,
                                                include_modelo_plugin,
                                                include_gantt_plugin);
    const QVector<DocumentoSobre> license_documents =
        pt ? OpcoesSobre::License_Documents_pt_BR(include_pomodoro_plugin,
                                                  include_feynman_plugin,
                                                  include_mapamental_plugin,
                                                  include_audiolivro_plugin,
                                                  include_agenda_plugin,
                                                  include_modelo_plugin,
                                                  include_gantt_plugin)
           : OpcoesSobre::License_Documents_en_US(include_pomodoro_plugin,
                                                  include_feynman_plugin,
                                                  include_mapamental_plugin,
                                                  include_audiolivro_plugin,
                                                  include_agenda_plugin,
                                                  include_modelo_plugin,
                                                  include_gantt_plugin);
    const QVector<DocumentoSobre> contract_documents =
        pt ? OpcoesSobre::Contract_Documents_pt_BR(include_pomodoro_plugin,
                                                   include_feynman_plugin,
                                                   include_mapamental_plugin,
                                                   include_audiolivro_plugin,
                                                   include_agenda_plugin,
                                                   include_modelo_plugin,
                                                   include_gantt_plugin)
           : OpcoesSobre::Contract_Documents_en_US(include_pomodoro_plugin,
                                                   include_feynman_plugin,
                                                   include_mapamental_plugin,
                                                   include_audiolivro_plugin,
                                                   include_agenda_plugin,
                                                   include_modelo_plugin,
                                                   include_gantt_plugin);
    const QVector<DocumentoSobre> copyright_documents =
        pt ? OpcoesSobre::Copyright_Documents_pt_BR(include_pomodoro_plugin,
                                                    include_feynman_plugin,
                                                    include_mapamental_plugin,
                                                    include_audiolivro_plugin,
                                                    include_agenda_plugin,
                                                    include_modelo_plugin,
                                                    include_gantt_plugin)
           : OpcoesSobre::Copyright_Documents_en_US(include_pomodoro_plugin,
                                                    include_feynman_plugin,
                                                    include_mapamental_plugin,
                                                    include_audiolivro_plugin,
                                                    include_agenda_plugin,
                                                    include_modelo_plugin,
                                                    include_gantt_plugin);
    const QVector<DocumentoSobre> notice_documents =
        pt ? OpcoesSobre::Notice_Documents_pt_BR(include_pomodoro_plugin,
                                                 include_feynman_plugin,
                                                 include_mapamental_plugin,
                                                 include_audiolivro_plugin,
                                                 include_agenda_plugin,
                                                 include_modelo_plugin,
                                                 include_gantt_plugin)
           : OpcoesSobre::Notice_Documents_en_US(include_pomodoro_plugin,
                                                 include_feynman_plugin,
                                                 include_mapamental_plugin,
                                                 include_audiolivro_plugin,
                                                 include_agenda_plugin,
                                                 include_modelo_plugin,
                                                 include_gantt_plugin);
    const QVector<DocumentoSobre> privacy_documents =
        pt ? OpcoesSobre::Privacy_Policy_Documents_pt_BR(include_pomodoro_plugin,
                                                         include_feynman_plugin,
                                                         include_mapamental_plugin,
                                                         include_audiolivro_plugin,
                                                         include_agenda_plugin,
                                                         include_modelo_plugin,
                                                         include_gantt_plugin)
           : OpcoesSobre::Privacy_Policy_Documents_en_US(include_pomodoro_plugin,
                                                         include_feynman_plugin,
                                                         include_mapamental_plugin,
                                                         include_audiolivro_plugin,
                                                         include_agenda_plugin,
                                                         include_modelo_plugin,
                                                         include_gantt_plugin);
    const QString release =
        pt ? OpcoesSobre::RELEASE_NOTES_pt_BR(include_pomodoro_plugin,
                                              include_feynman_plugin,
                                              include_mapamental_plugin,
                                              include_audiolivro_plugin,
                                              include_agenda_plugin,
                                              include_modelo_plugin,
                                              include_gantt_plugin)
           : OpcoesSobre::RELEASE_NOTES_en_US(include_pomodoro_plugin,
                                              include_feynman_plugin,
                                              include_mapamental_plugin,
                                              include_audiolivro_plugin,
                                              include_agenda_plugin,
                                              include_modelo_plugin,
                                              include_gantt_plugin);
    const QString info_nao = _tr_multi(QStringLiteral("information_not_available"));
    _site_oficial_text = translated_or_fallback(_tr_multi(QStringLiteral("site_oficial")),
                                                QStringLiteral("site_oficial"),
                                                QStringLiteral("Site oficial"));
    _retranslate_about_search_controls();

    history_browser->setPlainText(history);
    _set_details_documents(detalhes_documents, info_nao);
    _set_license_documents(license_documents, info_nao);
    _set_contract_documents(contract_documents, info_nao);
    _set_copyright_documents(copyright_documents, info_nao);
    _set_notice_documents(notice_documents, info_nao);
    _set_privacy_policy_documents(privacy_documents, info_nao);
    release_notes_browser->setPlainText(release);

    _tab_show_texts = {_tr_multi(QStringLiteral("show_history")),
                       _tr_multi(QStringLiteral("show_details")),
                       _tr_multi(QStringLiteral("show_licenses")),
                       _tr_multi(QStringLiteral("show_contract")),
                       _tr_multi(QStringLiteral("show_copyright")),
                       _tr_multi(QStringLiteral("show_notices")),
                       _tr_multi(QStringLiteral("show_privacy_policy")),
                       _tr_multi(QStringLiteral("show_release_notes"))};
    _tab_hide_texts = {_tr_multi(QStringLiteral("hide_history")),
                       _tr_multi(QStringLiteral("hide_details")),
                       _tr_multi(QStringLiteral("hide_licenses")),
                       _tr_multi(QStringLiteral("hide_contract")),
                       _tr_multi(QStringLiteral("hide_copyright")),
                       _tr_multi(QStringLiteral("hide_notices")),
                       _tr_multi(QStringLiteral("hide_privacy_policy")),
                       _tr_multi(QStringLiteral("hide_release_notes"))};

    if (ok_button != nullptr) {
        ok_button->setText(_tr_multi(QStringLiteral("OK")));
    }
    _refresh_open_license_document_titles();
    setWindowTitle(source::utils::titulo_janela_sobre());
    _apply_about_search();
    _update_about_search_buttons();
    _update_tab_labels(tabs->currentIndex());
}

}  // namespace source::GerenciamentoUI
