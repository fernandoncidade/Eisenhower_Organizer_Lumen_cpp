#pragma once

#include <QDialog>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QWidget>

#include "source/modules/feynman/feynman_host_bridge.hpp"
#include "source/utils/CaminhoPersistenteUtils.hpp"
#include "source/utils/LogManager.hpp"

namespace source::plugin::Feynman::compat {

enum class MissingLinkedFileChoice {
    None,
    RemoveLink,
    Search,
};

class GerenciadorBotoesUI {
public:
    explicit GerenciadorBotoesUI(QWidget *owner = nullptr);

    QPushButton *create_button(const QString &text = QString()) const;
    void set_button_text(QPushButton *button, const QString &text) const;

private:
    QPointer<QWidget> owner_;
};

class MessageBox : public QMessageBox {
    Q_OBJECT

public:
    using QMessageBox::QMessageBox;

    static MessageBox *create(QWidget *parent = nullptr);
    static void information(QWidget *parent, const QString &title, const QString &text);
    static void warning(QWidget *parent, const QString &title, const QString &text);

    static void feynman_warn_missing_title(QWidget *parent);
    static void feynman_save_success(QWidget *parent, const QString &title);
    static void feynman_edit_task_success(QWidget *parent);
    static bool feynman_confirm_delete_concepts(QWidget *parent, const QStringList &titles);
    static void feynman_delete_success(QWidget *parent);
    static void feynman_delete_error(QWidget *parent);
    static void feynman_unlinked_file_removed(QWidget *parent);
    static void feynman_linked_file_not_found(QWidget *parent, const QString &path);
    static void feynman_linked_file_open_error(QWidget *parent);
};

void set_host_bridge(source::plugin::FeynmanHostBridge *host_bridge);
source::plugin::FeynmanHostBridge *host_bridge();
QString effective_language_code(const QWidget *context = nullptr);
bool is_english(const QWidget *context = nullptr);
QString tr(const QWidget *context, const QString &pt_br, const QString &en_us);
QString plugin_persistent_dir();
void translate_standard_buttons(QDialog *dialog, const QWidget *context = nullptr);
MissingLinkedFileChoice prompt_missing_linked_file(QWidget *parent = nullptr);
QString browse_linked_file_replacement(QWidget *parent, const QString &current_path = QString());
bool open_local_path(const QString &path, QWidget *parent = nullptr);
QString normalized_title(const QString &text);

}  // namespace source::plugin::Feynman::compat
