#pragma once

#include <QDialog>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVector>
#include <functional>

#include "source/ui/ui_02_OpcoesSobre.hpp"

class QLabel;
class QLineEdit;
class QPushButton;
class QTextBrowser;
class QTabWidget;
class QToolButton;
class QEvent;

namespace source::GerenciamentoUI {

class SobreDialog : public QDialog {
    Q_OBJECT

public:
    explicit SobreDialog(
        QWidget *parent,
        const QString &titulo,
        const QString &texto_fixo,
        const QString &texto_history,
        const QString &detalhes,
        const QString &licencas,
        const QString &sites_licencas,
        const QString &show_history_text = QString(),
        const QString &hide_history_text = QString(),
        const QString &show_details_text = QString(),
        const QString &hide_details_text = QString(),
        const QString &show_licenses_text = QString(),
        const QString &hide_licenses_text = QString(),
        const QString &ok_text = QString(),
        const QString &site_oficial_text = QString(),
        const QString &avisos = QString(),
        const QString &show_notices_text = QString(),
        const QString &hide_notices_text = QString(),
        const QString &privacy_policy = QString(),
        const QString &show_privacy_policy_text = QString(),
        const QString &hide_privacy_policy_text = QString(),
        const QString &info_not_available_text = QStringLiteral("Information not available"),
        const QString &release_notes = QString(),
        const QString &show_release_notes_text = QString(),
        const QString &hide_release_notes_text = QString(),
        QWidget *app_for_translation = nullptr);

    void setHeaderBuilder(std::function<QString()> builder);
    void setDetailsDocuments(const QVector<DocumentoSobre> &documents);
    void setLicenseDocuments(const QVector<DocumentoSobre> &documents);
    void setContractDocuments(const QVector<DocumentoSobre> &documents);
    void setCopyrightDocuments(const QVector<DocumentoSobre> &documents);
    void setNoticeDocuments(const QVector<DocumentoSobre> &documents);
    void setPrivacyPolicyDocuments(const QVector<DocumentoSobre> &documents);

protected:
    void changeEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void _on_tab_changed(int index);

private:
    void _update_tab_labels(int current_index);
    QString _tr_multi(const QString &key) const;
    void _retranslate_ui();
    void _retranslate_about_search_controls();
    void _update_header_height();
    void _apply_about_search();
    void _navigate_about_search(int step);
    void _update_about_search_buttons();
    void _refresh_license_document_theme();
    void _set_document_tabs(QTabWidget *document_tabs,
                            const QVector<DocumentoSobre> &documents,
                            const QString &info_not_available_text,
                            const QString &fallback_title,
                            const QString &official_sites = QString(),
                            const QString &official_site_label = QString(),
                            bool append_official_sites_to_first_document = false);
    QWidget *_create_license_reference_panel(QWidget *parent,
                                             const QString &license_references,
                                             const QString &section_label);
    QWidget *_create_license_document_page(QWidget *parent,
                                           const DocumentoSobre &document,
                                           const QString &info_not_available_text,
                                           const QString &license_references,
                                           const QString &section_label);
    QString _license_document_window_title(const QString &title) const;
    void _refresh_open_license_document_titles();
    void _open_license_document(const QString &title, const QString &filename);
    void _open_license_site(const QString &url);
    void _set_details_documents(const QVector<DocumentoSobre> &documents, const QString &info_not_available_text);
    void _set_license_documents(const QVector<DocumentoSobre> &documents, const QString &info_not_available_text);
    void _set_contract_documents(const QVector<DocumentoSobre> &documents, const QString &info_not_available_text);
    void _set_copyright_documents(const QVector<DocumentoSobre> &documents, const QString &info_not_available_text);
    void _set_notice_documents(const QVector<DocumentoSobre> &documents, const QString &info_not_available_text);
    void _set_privacy_policy_documents(const QVector<DocumentoSobre> &documents,
                                       const QString &info_not_available_text);

    QLabel *fixed_label = nullptr;
    QTabWidget *tabs = nullptr;
    QTextBrowser *history_browser = nullptr;
    QTabWidget *detalhes_tabs = nullptr;
    QTabWidget *licencas_tabs = nullptr;
    QTabWidget *contrato_tabs = nullptr;
    QTabWidget *direitos_autorais_tabs = nullptr;
    QTabWidget *avisos_tabs = nullptr;
    QTabWidget *privacidade_tabs = nullptr;
    QTextBrowser *release_notes_browser = nullptr;
    QPushButton *ok_button = nullptr;
    QLabel *about_search_label = nullptr;
    QLineEdit *about_search_edit = nullptr;
    QToolButton *about_search_previous_button = nullptr;
    QToolButton *about_search_next_button = nullptr;

    QStringList _tab_show_texts;
    QStringList _tab_hide_texts;
    QString _license_sites;
    QString _site_oficial_text;
    QVector<QPointer<QDialog>> _license_document_dialogs;
    QWidget *_about_search_current_widget = nullptr;
    int _about_search_current_start = -1;
    std::function<QString()> _header_builder;
    QWidget *_app_for_translation = nullptr;
};

}  // namespace source::GerenciamentoUI
