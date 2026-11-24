#include "source/modules/feynman/Feynman/feynman_support.hpp"

#include <QAbstractButton>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QLocale>
#include <QProcess>
#include <QStringList>
#include <QUrl>

namespace source::plugin::Feynman::compat {
namespace {

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

source::plugin::FeynmanHostBridge *g_host_bridge = nullptr;

bool ask_yes_no(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Question);
    box.setWindowTitle(title);
    box.setText(text);
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    translate_standard_buttons(&box, parent);
    return box.exec() == QMessageBox::Yes;
}

QString join_titles(const QStringList &titles)
{
    QStringList lines;
    for (const QString &title : titles) {
        lines.append(QStringLiteral("• %1").arg(title));
    }
    return lines.join(QLatin1Char('\n'));
}

}  // namespace

GerenciadorBotoesUI::GerenciadorBotoesUI(QWidget *owner) : owner_(owner) {}

QPushButton *GerenciadorBotoesUI::create_button(const QString &text) const
{
    return new QPushButton(text, owner_);
}

void GerenciadorBotoesUI::set_button_text(QPushButton *button, const QString &text) const
{
    if (button != nullptr) {
        button->setText(text);
    }
}

MessageBox *MessageBox::create(QWidget *parent)
{
    auto *box = new MessageBox(parent);
    box->setAttribute(Qt::WA_DeleteOnClose, false);
    box->setStandardButtons(QMessageBox::Ok);
    translate_standard_buttons(box, parent);
    return box;
}

void MessageBox::information(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Information);
    box.setWindowTitle(title);
    box.setText(text);
    box.setStandardButtons(QMessageBox::Ok);
    translate_standard_buttons(&box, parent);
    box.exec();
}

void MessageBox::warning(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(title);
    box.setText(text);
    box.setStandardButtons(QMessageBox::Ok);
    translate_standard_buttons(&box, parent);
    box.exec();
}

void MessageBox::feynman_warn_missing_title(QWidget *parent)
{
    warning(parent,
            compat::tr(parent, QStringLiteral("Título obrigatório"), QStringLiteral("Title required")),
            compat::tr(parent,
                       QStringLiteral("Informe um título para o conceito antes de salvar."),
                       QStringLiteral("Enter a title for the concept before saving.")));
}

void MessageBox::feynman_save_success(QWidget *parent, const QString &title)
{
    information(parent,
                compat::tr(parent, QStringLiteral("Conceito salvo"), QStringLiteral("Concept saved")),
                compat::tr(parent,
                           QStringLiteral("O conceito \"%1\" foi salvo com sucesso."),
                           QStringLiteral("The concept \"%1\" was saved successfully."))
                    .arg(title));
}

void MessageBox::feynman_edit_task_success(QWidget *parent)
{
    information(parent,
                compat::tr(parent, QStringLiteral("✏️ Editar Tarefa"), QStringLiteral("✏️ Edit Task")),
                compat::tr(parent,
                           QStringLiteral("Alterações aplicadas com sucesso."),
                           QStringLiteral("Changes applied successfully.")));
}

bool MessageBox::feynman_confirm_delete_concepts(QWidget *parent, const QStringList &titles)
{
    return ask_yes_no(
        parent,
        compat::tr(parent, QStringLiteral("Excluir tarefas"), QStringLiteral("Delete tasks")),
        compat::tr(parent,
                   QStringLiteral("Deseja remover as seguintes tarefas?\n\n%1"),
                   QStringLiteral("Do you want to remove the following tasks?\n\n%1"))
            .arg(join_titles(titles)));
}

void MessageBox::feynman_delete_success(QWidget *parent)
{
    information(parent,
                compat::tr(parent, QStringLiteral("Tarefas removidas"), QStringLiteral("Tasks removed")),
                compat::tr(parent,
                           QStringLiteral("As tarefas selecionadas foram removidas com sucesso."),
                           QStringLiteral("The selected tasks were removed successfully.")));
}

void MessageBox::feynman_delete_error(QWidget *parent)
{
    warning(parent,
            compat::tr(parent, QStringLiteral("Erro"), QStringLiteral("Error")),
            compat::tr(parent,
                       QStringLiteral("Não foi possível remover as tarefas selecionadas."),
                       QStringLiteral("The selected tasks could not be removed.")));
}

void MessageBox::feynman_unlinked_file_removed(QWidget *parent)
{
    information(parent,
                compat::tr(parent, QStringLiteral("Vínculo removido"), QStringLiteral("Link removed")),
                compat::tr(parent,
                           QStringLiteral("O arquivo vinculado foi removido do conceito."),
                           QStringLiteral("The linked file was removed from the concept.")));
}

void MessageBox::feynman_linked_file_not_found(QWidget *parent, const QString &path)
{
    warning(parent,
            compat::tr(parent, QStringLiteral("Arquivo não encontrado"), QStringLiteral("File not found")),
            compat::tr(parent,
                       QStringLiteral("O arquivo vinculado não foi encontrado:\n%1"),
                       QStringLiteral("The linked file was not found:\n%1"))
                .arg(QDir::toNativeSeparators(path)));
}

void MessageBox::feynman_linked_file_open_error(QWidget *parent)
{
    warning(parent,
            compat::tr(parent, QStringLiteral("Erro ao abrir arquivo"), QStringLiteral("File open error")),
            compat::tr(parent,
                       QStringLiteral("Não foi possível abrir o arquivo vinculado."),
                       QStringLiteral("The linked file could not be opened.")));
}

void set_host_bridge(source::plugin::FeynmanHostBridge *host_bridge)
{
    g_host_bridge = host_bridge;
}

source::plugin::FeynmanHostBridge *host_bridge()
{
    return g_host_bridge;
}

QString effective_language_code(const QWidget *)
{
    if (g_host_bridge != nullptr) {
        return g_host_bridge->language_code();
    }

    return QLocale::system().name();
}

bool is_english(const QWidget *context)
{
    return effective_language_code(context).startsWith(QStringLiteral("en"), Qt::CaseInsensitive);
}

QString tr(const QWidget *context, const QString &pt_br, const QString &en_us)
{
    const QByteArray source_text = pt_br.toUtf8();
    const QString translated = QCoreApplication::translate("FeynmanPlugin", source_text.constData());
    if (is_english(context) && translated == pt_br) {
        return en_us;
    }
    return translated;
}

QString plugin_persistent_dir()
{
    return source::utils::obter_caminho_persistente();
}

void translate_standard_buttons(QDialog *dialog, const QWidget *context)
{
    if (dialog == nullptr) {
        return;
    }

    if (g_host_bridge != nullptr) {
        g_host_bridge->translate_standard_buttons(dialog);
    }

    if (auto *message_box = qobject_cast<QMessageBox *>(dialog)) {
        const struct ButtonText {
            QMessageBox::StandardButton button;
            QString pt_br;
            QString en_us;
        } buttons[] = {
            {QMessageBox::Ok, QStringLiteral("OK"), QStringLiteral("OK")},
            {QMessageBox::Cancel, QStringLiteral("Cancelar"), QStringLiteral("Cancel")},
            {QMessageBox::Yes, QStringLiteral("Sim"), QStringLiteral("Yes")},
            {QMessageBox::No, QStringLiteral("Não"), QStringLiteral("No")},
            {QMessageBox::Abort, QStringLiteral("Abortar"), QStringLiteral("Abort")},
            {QMessageBox::Retry, QStringLiteral("Tentar Novamente"), QStringLiteral("Retry")},
            {QMessageBox::Ignore, QStringLiteral("Ignorar"), QStringLiteral("Ignore")},
            {QMessageBox::Close, QStringLiteral("Fechar"), QStringLiteral("Close")},
            {QMessageBox::Help, QStringLiteral("Ajuda"), QStringLiteral("Help")},
            {QMessageBox::Apply, QStringLiteral("Aplicar"), QStringLiteral("Apply")},
            {QMessageBox::Reset, QStringLiteral("Redefinir"), QStringLiteral("Reset")},
            {QMessageBox::RestoreDefaults, QStringLiteral("Restaurar Padrões"), QStringLiteral("Restore Defaults")},
            {QMessageBox::Save, QStringLiteral("Salvar"), QStringLiteral("Save")},
            {QMessageBox::SaveAll, QStringLiteral("Salvar Tudo"), QStringLiteral("Save All")},
            {QMessageBox::Open, QStringLiteral("Abrir"), QStringLiteral("Open")},
        };

        for (const ButtonText &button : buttons) {
            if (QAbstractButton *widget = message_box->button(button.button)) {
                widget->setText(compat::tr(context, button.pt_br, button.en_us));
            }
        }
    }
}

MissingLinkedFileChoice prompt_missing_linked_file(QWidget *parent)
{
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(tr(parent, QStringLiteral("Aviso"), QStringLiteral("Warning")));
    box.setText(tr(parent,
                   QStringLiteral("O arquivo vinculado não foi encontrado ou não existe mais."),
                   QStringLiteral("The linked file was not found or no longer exists.")));
    QAbstractButton *remove_button =
        box.addButton(tr(parent, QStringLiteral("Remover Vínculo"), QStringLiteral("Remove Link")), QMessageBox::ActionRole);
    QAbstractButton *search_button =
        box.addButton(tr(parent, QStringLiteral("Buscar"), QStringLiteral("Browse")), QMessageBox::ActionRole);
    box.addButton(tr(parent, QStringLiteral("OK"), QStringLiteral("OK")), QMessageBox::AcceptRole);
    box.exec();

    if (box.clickedButton() == remove_button) {
        return MissingLinkedFileChoice::RemoveLink;
    }
    if (box.clickedButton() == search_button) {
        return MissingLinkedFileChoice::Search;
    }
    return MissingLinkedFileChoice::None;
}

QString browse_linked_file_replacement(QWidget *parent, const QString &current_path)
{
    const QString initial_dir = QFileInfo(current_path).absolutePath();
    return QFileDialog::getOpenFileName(parent,
                                        tr(parent, QStringLiteral("Buscar arquivo"), QStringLiteral("Browse file")),
                                        initial_dir,
                                        QStringLiteral("*"));
}

bool open_local_path(const QString &path, QWidget *parent)
{
    const QString cleaned = path.trimmed();
    if (cleaned.isEmpty()) {
        return false;
    }

    const QFileInfo info(cleaned);
    if (!info.exists()) {
        MessageBox::feynman_linked_file_not_found(parent, cleaned);
        return false;
    }

    if (QDesktopServices::openUrl(QUrl::fromLocalFile(info.absoluteFilePath()))) {
        return true;
    }

#if defined(Q_OS_WIN)
    if (QProcess::startDetached(QStringLiteral("explorer.exe"), {QDir::toNativeSeparators(info.absoluteFilePath())})) {
        return true;
    }
#elif defined(Q_OS_MACOS)
    if (QProcess::startDetached(QStringLiteral("open"), {info.absoluteFilePath()})) {
        return true;
    }
#else
    if (QProcess::startDetached(QStringLiteral("xdg-open"), {info.absoluteFilePath()})) {
        return true;
    }
#endif

    logger()->error(QStringLiteral("Falha ao abrir arquivo vinculado do Método Feynman: %1").arg(info.absoluteFilePath()));
    MessageBox::feynman_linked_file_open_error(parent);
    return false;
}

QString normalized_title(const QString &text)
{
    QString result = text.split(QLatin1Char('\n')).value(0).simplified().trimmed();
    result.remove(QChar(0x0332));
    const QStringList prefixes = {
        QStringLiteral("🔴"),
        QStringLiteral("🟠"),
        QStringLiteral("🟡"),
        QStringLiteral("🟢"),
        QStringLiteral("🥉"),
        QStringLiteral("🥈"),
        QStringLiteral("🥇"),
    };

    bool removed_prefix = true;
    while (removed_prefix) {
        removed_prefix = false;
        for (const QString &prefix : prefixes) {
            const QString token = prefix + QLatin1Char(' ');
            if (result.startsWith(token)) {
                result.remove(0, token.size());
                result = result.trimmed();
                removed_prefix = true;
                break;
            }
        }
    }

    const int sep = result.indexOf(QStringLiteral(" — "));
    if (sep >= 0) {
        result = result.left(sep).trimmed();
    }

    return result.simplified().trimmed().toCaseFolded();
}

}  // namespace source::plugin::Feynman::compat
