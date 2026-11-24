#include "source/ui/ui_18_ModelDownloadDialog.hpp"

#include <QApplication>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QEvent>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QShowEvent>
#include <QStyleHints>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

#include <QGuiApplication>

#include "source/app/app_01_eisenhower_matrix_app.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"
#include "source/utils/CaminhoPersistenteUtils.hpp"

namespace source::GerenciamentoUI {
namespace {

QString fallback_models_dir()
{
    return QDir(source::utils::obter_caminho_persistente()).filePath(QStringLiteral("models"));
}

QString fallback_model_path()
{
    return QDir(fallback_models_dir()).filePath(QStringLiteral("qwen2.5-3b-instruct-q4_k_m.gguf"));
}

QString resolve_models_dir(const source::EisenhowerMatrixApp *app)
{
    return app != nullptr ? app->qwen_models_dir() : fallback_models_dir();
}

QString resolve_model_path(const source::EisenhowerMatrixApp *app)
{
    return app != nullptr ? app->qwen_model_path() : fallback_model_path();
}

QString resolve_download_url(const source::EisenhowerMatrixApp *app)
{
    return app != nullptr
        ? app->qwen_download_url()
        : QStringLiteral("https://huggingface.co/Qwen/Qwen2.5-3B-Instruct-GGUF/resolve/main/qwen2.5-3b-instruct-q4_k_m.gguf");
}

bool resolve_model_installed(const source::EisenhowerMatrixApp *app)
{
    if (app != nullptr) {
        return app->is_qwen_model_installed();
    }
    QFile file(fallback_model_path());
    return file.open(QIODevice::ReadOnly) && file.read(4) == QByteArrayLiteral("GGUF");
}

qint64 resolve_model_size_bytes(const source::EisenhowerMatrixApp *app)
{
    return app != nullptr ? app->qwen_model_size_bytes() : QFileInfo(fallback_model_path()).size();
}

void remove_installed_model(source::EisenhowerMatrixApp *app)
{
    if (app != nullptr) {
        app->remove_qwen_model();
        return;
    }
    const QString dest = fallback_model_path();
    QFile::remove(dest);
    QFile::remove(dest + QStringLiteral(".part"));
}

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

QString css_rgba(const QColor &color, int alpha = 255)
{
    return QStringLiteral("rgba(%1, %2, %3, %4)")
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue())
        .arg(alpha);
}

bool is_dark_theme_active()
{
    if (auto *hints = QGuiApplication::styleHints()) {
        const auto scheme = hints->colorScheme();
        if (scheme == Qt::ColorScheme::Dark) {
            return true;
        }
        if (scheme == Qt::ColorScheme::Light) {
            return false;
        }
    }
#if defined(Q_OS_WIN)
    QSettings settings(
        QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"),
        QSettings::NativeFormat);
    if (settings.contains(QStringLiteral("AppsUseLightTheme"))) {
        return settings.value(QStringLiteral("AppsUseLightTheme")).toInt() == 0;
    }
#endif
    const QPalette pal = QApplication::palette();
    return pal.color(QPalette::Window).lightnessF() < 0.5;
}

}  // namespace

ModelDownloadDialog::ModelDownloadDialog(source::EisenhowerMatrixApp *app, QWidget *parent)
    : QDialog(parent), app_(app)
{
    network_manager_ = new QNetworkAccessManager(this);

    if (auto *hints = QGuiApplication::styleHints()) {
        connect(hints, &QStyleHints::colorSchemeChanged, this, [this](Qt::ColorScheme) {
            agendar_refresh_tema();
        });
    }

    setWindowTitle(get_text(QStringLiteral("Gerenciador do Modelo Local Qwen 2.5 (GGUF)")));
    setMinimumSize(780, 430);
    resize(800, 460);
    setAttribute(Qt::WA_DeleteOnClose, false);

    auto *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(18, 18, 18, 18);
    main_layout->setSpacing(14);

    // Cabeçalho
    header_frame_ = new QFrame(this);
    header_frame_->setFrameShape(QFrame::StyledPanel);
    auto *header_layout = new QVBoxLayout(header_frame_);
    header_layout->setContentsMargins(14, 12, 14, 12);
    header_layout->setSpacing(5);

    title_label_ = new QLabel(header_frame_);
    title_label_->setTextFormat(Qt::PlainText);

    sub_label_ = new QLabel(header_frame_);
    sub_label_->setWordWrap(true);
    sub_label_->setTextFormat(Qt::PlainText);

    header_layout->addWidget(title_label_);
    header_layout->addWidget(sub_label_);
    main_layout->addWidget(header_frame_);

    // Card de Informações do Modelo
    info_group_ = new QGroupBox(this);
    auto *info_layout = new QGridLayout(info_group_);
    info_layout->setContentsMargins(14, 14, 14, 14);
    info_layout->setHorizontalSpacing(12);
    info_layout->setVerticalSpacing(8);

    lbl_name_title_ = new QLabel(info_group_);
    model_name_label_ = new QLabel(QStringLiteral("Qwen 2.5 3B Instruct (qwen2.5-3b-instruct-q4_k_m.gguf)"), info_group_);
    model_name_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);

    lbl_source_title_ = new QLabel(info_group_);
    model_source_label_ = new QLabel(QStringLiteral("Hugging Face (Qwen/Qwen2.5-3B-Instruct-GGUF)"), info_group_);
    model_source_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);

    lbl_size_title_ = new QLabel(info_group_);
    model_size_label_ = new QLabel(info_group_);

    lbl_path_title_ = new QLabel(info_group_);
    model_path_label_ = new QLabel(info_group_);
    model_path_label_->setWordWrap(true);
    model_path_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);

    info_layout->addWidget(lbl_name_title_, 0, 0);
    info_layout->addWidget(model_name_label_, 0, 1);
    info_layout->addWidget(lbl_source_title_, 1, 0);
    info_layout->addWidget(model_source_label_, 1, 1);
    info_layout->addWidget(lbl_size_title_, 2, 0);
    info_layout->addWidget(model_size_label_, 2, 1);
    info_layout->addWidget(lbl_path_title_, 3, 0);
    info_layout->addWidget(model_path_label_, 3, 1);
    info_layout->setColumnStretch(1, 1);
    main_layout->addWidget(info_group_);

    // Card de Status e Progresso de Download
    progress_group_ = new QGroupBox(this);
    auto *progress_layout = new QVBoxLayout(progress_group_);
    progress_layout->setContentsMargins(14, 14, 14, 14);
    progress_layout->setSpacing(10);

    status_label_ = new QLabel(progress_group_);
    status_label_->setWordWrap(true);
    progress_layout->addWidget(status_label_);

    progress_bar_ = new QProgressBar(progress_group_);
    progress_bar_->setRange(0, 100);
    progress_bar_->setValue(0);
    progress_bar_->setTextVisible(false);
    progress_bar_->setFixedHeight(18);
    progress_layout->addWidget(progress_bar_);

    auto *metrics_layout = new QHBoxLayout();
    progress_details_label_ = new QLabel(progress_group_);
    speed_eta_label_ = new QLabel(progress_group_);
    speed_eta_label_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    metrics_layout->addWidget(progress_details_label_);
    metrics_layout->addWidget(speed_eta_label_);
    progress_layout->addLayout(metrics_layout);

    main_layout->addWidget(progress_group_);

    // Botões de Ação
    auto *actions_layout = new QHBoxLayout();
    actions_layout->setSpacing(10);

    btn_start_resume_ = new QPushButton(get_text(QStringLiteral("Iniciar Download")), this);
    connect(btn_start_resume_, &QPushButton::clicked, this, &ModelDownloadDialog::on_start_clicked);

    btn_pause_ = new QPushButton(get_text(QStringLiteral("Pausar")), this);
    connect(btn_pause_, &QPushButton::clicked, this, &ModelDownloadDialog::on_pause_clicked);

    btn_cancel_ = new QPushButton(get_text(QStringLiteral("Cancelar")), this);
    connect(btn_cancel_, &QPushButton::clicked, this, &ModelDownloadDialog::on_cancel_clicked);

    btn_delete_ = new QPushButton(get_text(QStringLiteral("Excluir Modelo")), this);
    connect(btn_delete_, &QPushButton::clicked, this, &ModelDownloadDialog::on_delete_clicked);

    btn_open_folder_ = new QPushButton(get_text(QStringLiteral("Abrir Pasta de Modelos")), this);
    connect(btn_open_folder_, &QPushButton::clicked, this, &ModelDownloadDialog::on_open_folder_clicked);

    btn_close_ = new QPushButton(get_text(QStringLiteral("Fechar")), this);
    connect(btn_close_, &QPushButton::clicked, this, &QDialog::accept);

    actions_layout->addWidget(btn_start_resume_);
    actions_layout->addWidget(btn_pause_);
    actions_layout->addWidget(btn_cancel_);
    actions_layout->addWidget(btn_delete_);
    actions_layout->addStretch(1);
    actions_layout->addWidget(btn_open_folder_);
    actions_layout->addWidget(btn_close_);
    main_layout->addLayout(actions_layout);

    if (app_ != nullptr && app_->gerenciador_traducao != nullptr) {
        connect(app_->gerenciador_traducao,
                &source::language::GerenciadorTraducao::idioma_alterado,
                this,
                [this](const QString &) { retranslate_ui(); });
    }

    retranslate_ui();
    refresh_status();
}

ModelDownloadDialog::~ModelDownloadDialog()
{
    if (current_reply_ != nullptr) {
        current_reply_->disconnect();
        current_reply_->abort();
        current_reply_->deleteLater();
        current_reply_ = nullptr;
    }
    if (part_file_.isOpen()) {
        part_file_.flush();
        part_file_.close();
    }
}

void ModelDownloadDialog::changeEvent(QEvent *event)
{
    if (event != nullptr) {
        switch (event->type()) {
            case QEvent::LanguageChange:
                retranslate_ui();
                break;
            case QEvent::PaletteChange:
            case QEvent::ApplicationPaletteChange:
            case QEvent::ThemeChange:
            case QEvent::StyleChange:
                agendar_refresh_tema();
                break;
            default:
                break;
        }
    }
    QDialog::changeEvent(event);
}

void ModelDownloadDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    refresh_status();
    agendar_refresh_tema();
}

void ModelDownloadDialog::closeEvent(QCloseEvent *event)
{
    if (state_ == DownloadState::Downloading) {
        const auto resp = QMessageBox::question(
            this,
            get_text(QStringLiteral("Download em Andamento")),
            get_text(QStringLiteral("O download do modelo continuará em segundo plano enquanto o aplicativo estiver aberto.\n\n"
                                   "Deseja fechar esta janela mantendo o download ativo?")),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes);

        if (resp != QMessageBox::Yes) {
            event->ignore();
            return;
        }
    }
    event->accept();
}

void ModelDownloadDialog::retranslate_ui()
{
    setWindowTitle(get_text(QStringLiteral("Gerenciador do Modelo Local Qwen 2.5 (GGUF)")));

    if (title_label_ != nullptr) {
        title_label_->setText(get_text(QStringLiteral("Download e Gerenciamento do Modelo de IA Qwen 2.5")));
    }
    if (sub_label_ != nullptr) {
        sub_label_->setText(get_text(
            QStringLiteral("Baixe o modelo neural de linguagem (GGUF quantizado em 4 bits) para sintetizar e estruturar "
                           "documentos no Mapa Mental de forma 100% autônoma, privada e sem necessitar de internet após o download.")));
    }

    if (info_group_ != nullptr) {
        info_group_->setTitle(get_text(QStringLiteral("Informações do Modelo")));
    }
    if (lbl_name_title_ != nullptr) {
        lbl_name_title_->setText(get_text(QStringLiteral("Modelo:")));
    }
    if (lbl_source_title_ != nullptr) {
        lbl_source_title_->setText(get_text(QStringLiteral("Repositório:")));
    }
    if (lbl_size_title_ != nullptr) {
        lbl_size_title_->setText(get_text(QStringLiteral("Tamanho:")));
    }
    if (lbl_path_title_ != nullptr) {
        lbl_path_title_->setText(get_text(QStringLiteral("Caminho Local:")));
    }
    if (model_size_label_ != nullptr) {
        model_size_label_->setText(get_text(QStringLiteral("~2,10 GB (2.104.932.768 bytes)")));
    }

    if (progress_group_ != nullptr) {
        progress_group_->setTitle(get_text(QStringLiteral("Status e Progresso do Download")));
    }

    if (btn_start_resume_ != nullptr) {
        btn_start_resume_->setText(state_ == DownloadState::Paused
                                       ? get_text(QStringLiteral("Retomar Download"))
                                       : get_text(QStringLiteral("Iniciar Download")));
    }
    if (btn_pause_ != nullptr) {
        btn_pause_->setText(get_text(QStringLiteral("Pausar")));
    }
    if (btn_cancel_ != nullptr) {
        btn_cancel_->setText(get_text(QStringLiteral("Cancelar")));
    }
    if (btn_delete_ != nullptr) {
        btn_delete_->setText(get_text(QStringLiteral("Excluir Modelo")));
    }
    if (btn_open_folder_ != nullptr) {
        btn_open_folder_->setText(get_text(QStringLiteral("Abrir Pasta de Modelos")));
    }

    if (btn_close_ != nullptr) {
        btn_close_->setText(get_text(QStringLiteral("Fechar")));
    }

    atualizar_widgets_estado();
    aplicar_estilos_tema();
}

void ModelDownloadDialog::refresh_status()
{
    const QString dest_path = resolve_model_path(app_);
    if (model_path_label_ != nullptr) {
        model_path_label_->setText(QDir::toNativeSeparators(dest_path));
    }

    const bool instalado = resolve_model_installed(app_);
    const QString part_path = dest_path + QStringLiteral(".part");

    if (instalado) {
        state_ = DownloadState::Finished;
    } else if (state_ == DownloadState::Downloading) {
        // Mantém estado de download ativo
    } else if (QFile::exists(part_path) && QFileInfo(part_path).size() > 0) {
        state_ = DownloadState::Paused;
    } else {
        state_ = DownloadState::Idle;
    }

    atualizar_widgets_estado();
}

void ModelDownloadDialog::atualizar_widgets_estado()
{
    const QString dest_path = resolve_model_path(app_);
    const QString part_path = dest_path + QStringLiteral(".part");
    const bool instalado = resolve_model_installed(app_);

    switch (state_) {
        case DownloadState::Finished: {
            const qint64 tam = resolve_model_size_bytes(app_);
            status_label_->setText(QStringLiteral("<font color=\"#2e7d32\"><b>✔ %1</b></font> (%2)").arg(
                get_text(QStringLiteral("Modelo instalado e pronto para uso no Mapa Mental")),
                format_bytes(tam)));
            progress_bar_->setValue(100);
            progress_details_label_->setText(format_bytes(tam));
            speed_eta_label_->setText(get_text(QStringLiteral("Concluído")));

            btn_start_resume_->setEnabled(false);
            btn_start_resume_->setText(get_text(QStringLiteral("Iniciar Download")));
            btn_pause_->setEnabled(false);
            btn_cancel_->setEnabled(false);
            btn_delete_->setEnabled(true);
            break;
        }

        case DownloadState::Downloading: {
            status_label_->setText(QStringLiteral("<font color=\"#1976d2\"><b>⏳ %1</b></font>").arg(
                get_text(QStringLiteral("Baixando modelo do Hugging Face..."))));

            btn_start_resume_->setEnabled(false);
            btn_pause_->setEnabled(true);
            btn_cancel_->setEnabled(true);
            btn_delete_->setEnabled(false);
            break;
        }

        case DownloadState::Paused: {
            const qint64 part_size = QFileInfo(part_path).size();
            const int percent = total_expected_bytes_ > 0
                                    ? qBound(0, static_cast<int>((part_size * 100.0) / total_expected_bytes_), 100)
                                    : 0;
            status_label_->setText(QStringLiteral("<font color=\"#f57c00\"><b>⏸ %1</b></font> (%2 / %3)").arg(
                get_text(QStringLiteral("Download pausado")),
                format_bytes(part_size),
                format_bytes(total_expected_bytes_)));

            progress_bar_->setValue(percent);
            progress_details_label_->setText(QStringLiteral("%1 / %2 (%3%)").arg(
                format_bytes(part_size), format_bytes(total_expected_bytes_), QString::number(percent)));
            speed_eta_label_->setText(get_text(QStringLiteral("Pausado")));

            btn_start_resume_->setEnabled(true);
            btn_start_resume_->setText(get_text(QStringLiteral("Retomar Download")));
            btn_pause_->setEnabled(false);
            btn_cancel_->setEnabled(true);
            btn_delete_->setEnabled(part_size > 0);
            break;
        }

        case DownloadState::Error: {
            status_label_->setText(QStringLiteral("<font color=\"#d32f2f\"><b>✖ %1</b></font>").arg(
                get_text(QStringLiteral("Ocorreu uma falha no download. Clique em Retomar para tentar novamente."))));

            btn_start_resume_->setEnabled(true);
            btn_start_resume_->setText(get_text(QStringLiteral("Retomar Download")));
            btn_pause_->setEnabled(false);
            btn_cancel_->setEnabled(true);
            btn_delete_->setEnabled(QFile::exists(part_path));
            break;
        }

        case DownloadState::Idle:
        default: {
            const bool dark = is_dark_theme_active();
            status_label_->setText(QStringLiteral("<font color=\"%1\"><b>⚪ %2</b></font>").arg(
                dark ? QStringLiteral("#b5b5b5") : QStringLiteral("#555555"),
                get_text(QStringLiteral("Modelo não instalado (disponível para download sob demanda)"))));
            progress_bar_->setValue(0);
            progress_details_label_->setText(QStringLiteral("0 / %1 (0%)").arg(format_bytes(total_expected_bytes_)));
            speed_eta_label_->clear();

            btn_start_resume_->setEnabled(true);
            btn_start_resume_->setText(get_text(QStringLiteral("Iniciar Download")));
            btn_pause_->setEnabled(false);
            btn_cancel_->setEnabled(false);
            btn_delete_->setEnabled(instalado || QFile::exists(part_path));
            break;
        }
    }
}

void ModelDownloadDialog::agendar_refresh_tema()
{
    if (applying_theme_ || theme_refresh_pending_) {
        return;
    }
    theme_refresh_pending_ = true;
    QTimer::singleShot(0, this, [this]() {
        theme_refresh_pending_ = false;
        refresh_theme();
    });
    QTimer::singleShot(60, this, [this]() {
        refresh_theme();
    });
}

void ModelDownloadDialog::refresh_theme()
{
    aplicar_estilos_tema();
    atualizar_widgets_estado();
    update();
}

void ModelDownloadDialog::aplicar_estilos_tema()
{
    if (applying_theme_) {
        return;
    }
    applying_theme_ = true;

    const bool dark = is_dark_theme_active();
    const QPalette pal = QApplication::palette();
    const QColor pal_text = pal.color(QPalette::Active, QPalette::WindowText);

    // Garante alto contraste e cor correta para o tema ativo (branco no escuro, preto no claro)
    const QColor text_color = dark
        ? (pal_text.lightnessF() > 0.45 ? pal_text : QColor(255, 255, 255))
        : (pal_text.lightnessF() < 0.55 ? pal_text : QColor(0, 0, 0));
    const QColor sub_text_color = dark ? QColor(215, 215, 215) : QColor(55, 55, 55);
    const QColor border_color = dark ? QColor(80, 80, 80) : QColor(190, 190, 190);

    if (title_label_ != nullptr) {
        title_label_->setStyleSheet(
            QStringLiteral("QLabel { color: %1; font-size: 15px; font-weight: bold; }").arg(css_rgba(text_color)));
    }
    if (sub_label_ != nullptr) {
        sub_label_->setStyleSheet(
            QStringLiteral("QLabel { color: %1; font-size: 12px; }").arg(css_rgba(sub_text_color)));
    }

    const QString bold_title_style = QStringLiteral(
        "QLabel { color: %1; font-weight: bold; }").arg(css_rgba(text_color));

    if (lbl_name_title_ != nullptr) {
        lbl_name_title_->setStyleSheet(bold_title_style);
    }
    if (lbl_source_title_ != nullptr) {
        lbl_source_title_->setStyleSheet(bold_title_style);
    }
    if (lbl_size_title_ != nullptr) {
        lbl_size_title_->setStyleSheet(bold_title_style);
    }
    if (lbl_path_title_ != nullptr) {
        lbl_path_title_->setStyleSheet(bold_title_style);
    }

    const QString normal_val_style = QStringLiteral(
        "QLabel { color: %1; }").arg(css_rgba(text_color));

    if (model_name_label_ != nullptr) {
        model_name_label_->setStyleSheet(normal_val_style);
    }
    if (model_source_label_ != nullptr) {
        model_source_label_->setStyleSheet(normal_val_style);
    }
    if (model_size_label_ != nullptr) {
        model_size_label_->setStyleSheet(normal_val_style);
    }
    if (model_path_label_ != nullptr) {
        model_path_label_->setStyleSheet(normal_val_style);
    }
    if (progress_details_label_ != nullptr) {
        progress_details_label_->setStyleSheet(normal_val_style);
    }
    if (speed_eta_label_ != nullptr) {
        speed_eta_label_->setStyleSheet(normal_val_style);
    }

    const QString group_box_style = QStringLiteral(
        "QGroupBox { color: %1; font-weight: bold; }").arg(css_rgba(text_color));
    if (info_group_ != nullptr) {
        info_group_->setStyleSheet(group_box_style);
    }
    if (progress_group_ != nullptr) {
        progress_group_->setStyleSheet(group_box_style);
    }

    if (header_frame_ != nullptr) {
        header_frame_->setStyleSheet(QStringLiteral(
            "QFrame { background-color: %1; border: 1px solid %2; border-radius: 6px; }")
            .arg(dark ? QStringLiteral("rgba(255, 255, 255, 0.05)") : QStringLiteral("rgba(0, 0, 0, 0.03)"),
                 css_rgba(border_color, dark ? 80 : 60)));
    }

    applying_theme_ = false;
}

void ModelDownloadDialog::on_start_clicked()
{
    iniciar_download(true);
}

void ModelDownloadDialog::on_pause_clicked()
{
    pausar_download();
}

void ModelDownloadDialog::on_cancel_clicked()
{
    cancelar_download(true);
}

void ModelDownloadDialog::on_delete_clicked()
{
    const auto resp = QMessageBox::question(
        this,
        get_text(QStringLiteral("Confirmar Exclusão")),
        get_text(QStringLiteral("Deseja realmente remover o modelo local Qwen 2.5?\n\n"
                               "Isso liberará aproximadamente 2,1 GB de espaço no disco. "
                               "Você poderá baixá-lo novamente a qualquer momento.")),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (resp != QMessageBox::Yes) {
        return;
    }

    if (current_reply_ != nullptr) {
        current_reply_->disconnect();
        current_reply_->abort();
        current_reply_->deleteLater();
        current_reply_ = nullptr;
    }
    if (part_file_.isOpen()) {
        part_file_.close();
    }

    remove_installed_model(app_);

    state_ = DownloadState::Idle;
    refresh_status();
    emit model_download_status_changed();
    if (app_ != nullptr) {
        app_->refresh_ollama_models();
    }

    QMessageBox::information(
        this,
        get_text(QStringLiteral("Modelo Removido")),
        get_text(QStringLiteral("O modelo local foi excluído com sucesso e o espaço em disco foi liberado.")));
}

void ModelDownloadDialog::on_open_folder_clicked()
{
    const QString dir = resolve_models_dir(app_);
    QDir().mkpath(dir);
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void ModelDownloadDialog::iniciar_download(bool retomar)
{
    if (state_ == DownloadState::Downloading) {
        return;
    }

    const QString dir = resolve_models_dir(app_);
    QDir().mkpath(dir);

    const QString dest_path = resolve_model_path(app_);
    const QString part_path = dest_path + QStringLiteral(".part");

    qint64 existing_size = 0;
    if (retomar && QFile::exists(part_path)) {
        existing_size = QFileInfo(part_path).size();
        if (existing_size >= total_expected_bytes_) {
            existing_size = 0;
        }
    }

    part_file_.setFileName(part_path);
    if (existing_size > 0) {
        if (!part_file_.open(QIODevice::ReadWrite | QIODevice::Append)) {
            state_ = DownloadState::Error;
            atualizar_widgets_estado();
            QMessageBox::warning(this, get_text(QStringLiteral("Erro de Acesso")),
                                 get_text(QStringLiteral("Não foi possível abrir o arquivo parcial para gravação.")));
            return;
        }
        bytes_offset_ = existing_size;
    } else {
        if (!part_file_.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            state_ = DownloadState::Error;
            atualizar_widgets_estado();
            QMessageBox::warning(this, get_text(QStringLiteral("Erro de Acesso")),
                                 get_text(QStringLiteral("Não foi possível criar o arquivo de destino para gravação.")));
            return;
        }
        bytes_offset_ = 0;
    }

    current_download_url_ = QUrl(resolve_download_url(app_));
    redirect_count_ = 0;
    checked_http_response_ = false;
    last_bytes_received_ = 0;
    smoothed_speed_ = 0.0;
    speed_timer_.restart();

    QNetworkRequest request(current_download_url_);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) EisenhowerOrganizer/1.0");

    if (bytes_offset_ > 0) {
        request.setRawHeader("Range", QStringLiteral("bytes=%1-").arg(bytes_offset_).toUtf8());
    }

    current_reply_ = network_manager_->get(request);
    connect(current_reply_, &QNetworkReply::downloadProgress, this, &ModelDownloadDialog::on_network_reply_progress);
    connect(current_reply_, &QIODevice::readyRead, this, &ModelDownloadDialog::on_network_reply_ready_read);
    connect(current_reply_, &QNetworkReply::finished, this, &ModelDownloadDialog::on_network_reply_finished);
    connect(current_reply_, &QNetworkReply::errorOccurred, this, &ModelDownloadDialog::on_network_reply_error);

    state_ = DownloadState::Downloading;
    atualizar_widgets_estado();
}

void ModelDownloadDialog::pausar_download()
{
    if (current_reply_ != nullptr) {
        current_reply_->disconnect();
        current_reply_->abort();
        current_reply_->deleteLater();
        current_reply_ = nullptr;
    }

    if (part_file_.isOpen()) {
        part_file_.flush();
        part_file_.close();
    }

    state_ = DownloadState::Paused;
    atualizar_widgets_estado();
}

void ModelDownloadDialog::cancelar_download(bool confirmar)
{
    if (confirmar) {
        const auto resp = QMessageBox::question(
            this,
            get_text(QStringLiteral("Cancelar Download")),
            get_text(QStringLiteral("Deseja realmente cancelar o download do modelo?")),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

        if (resp != QMessageBox::Yes) {
            return;
        }
    }

    if (current_reply_ != nullptr) {
        current_reply_->disconnect();
        current_reply_->abort();
        current_reply_->deleteLater();
        current_reply_ = nullptr;
    }

    if (part_file_.isOpen()) {
        part_file_.close();
    }

    state_ = DownloadState::Paused;
    atualizar_widgets_estado();
}

void ModelDownloadDialog::on_network_reply_progress(qint64 bytes_received, qint64 bytes_total)
{
    if (bytes_total > 0) {
        total_expected_bytes_ = bytes_offset_ + bytes_total;
    }

    const qint64 total_done = bytes_offset_ + bytes_received;
    const int percent = total_expected_bytes_ > 0
                            ? qBound(0, static_cast<int>((total_done * 100.0) / total_expected_bytes_), 100)
                            : 0;

    progress_bar_->setValue(percent);
    progress_details_label_->setText(QStringLiteral("%1 / %2 (%3%)").arg(
        format_bytes(total_done), format_bytes(total_expected_bytes_), QString::number(percent)));

    const qint64 elapsed_ms = speed_timer_.elapsed();
    if (elapsed_ms >= 500) {
        const double instant_speed = (bytes_received - last_bytes_received_) * 1000.0 / elapsed_ms;
        smoothed_speed_ = (smoothed_speed_ <= 0.0) ? instant_speed : (0.75 * smoothed_speed_ + 0.25 * instant_speed);
        last_bytes_received_ = bytes_received;
        speed_timer_.restart();
    }

    if (smoothed_speed_ > 1024.0 && total_expected_bytes_ > total_done) {
        const int eta = static_cast<int>((total_expected_bytes_ - total_done) / smoothed_speed_);
        speed_eta_label_->setText(QStringLiteral("%1  |  %2").arg(
            format_speed(smoothed_speed_), format_eta(eta)));
    }
}

void ModelDownloadDialog::on_network_reply_ready_read()
{
    if (current_reply_ == nullptr || !part_file_.isOpen()) {
        return;
    }

    if (!checked_http_response_) {
        checked_http_response_ = true;
        const QVariant status_attr = current_reply_->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        const int status_code = status_attr.isValid() ? status_attr.toInt() : 0;
        if (status_code >= 400) {
            current_reply_->disconnect();
            current_reply_->abort();
            current_reply_->deleteLater();
            current_reply_ = nullptr;
            part_file_.close();
            state_ = DownloadState::Error;
            atualizar_widgets_estado();
            return;
        }
        if (bytes_offset_ > 0 && status_code == 200) {
            part_file_.seek(0);
            part_file_.resize(0);
            bytes_offset_ = 0;
        }
    }

    const QByteArray data = current_reply_->readAll();
    if (!data.isEmpty()) {
        const qint64 written = part_file_.write(data);
        if (written != data.size()) {
            current_reply_->disconnect();
            current_reply_->abort();
            current_reply_->deleteLater();
            current_reply_ = nullptr;
            part_file_.close();
            state_ = DownloadState::Error;
            atualizar_widgets_estado();
            return;
        }
    }
}

void ModelDownloadDialog::on_network_reply_finished()
{
    QNetworkReply *reply = current_reply_;
    current_reply_ = nullptr;
    if (reply == nullptr) {
        return;
    }

    // Trata redirecionamentos HTTP (Hugging Face CDN)
    const QVariant redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (redirect.isValid() && redirect_count_ < 8) {
        const QUrl new_url = reply->url().resolved(redirect.toUrl());
        redirect_count_++;
        checked_http_response_ = false;

        reply->deleteLater();

        current_download_url_ = new_url;
        QNetworkRequest req(current_download_url_);
        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        req.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) EisenhowerOrganizer/1.0");

        const qint64 cur_file_size = part_file_.size();
        if (cur_file_size > 0) {
            bytes_offset_ = cur_file_size;
            req.setRawHeader("Range", QStringLiteral("bytes=%1-").arg(bytes_offset_).toUtf8());
        }

        current_reply_ = network_manager_->get(req);
        connect(current_reply_, &QNetworkReply::downloadProgress, this, &ModelDownloadDialog::on_network_reply_progress);
        connect(current_reply_, &QIODevice::readyRead, this, &ModelDownloadDialog::on_network_reply_ready_read);
        connect(current_reply_, &QNetworkReply::finished, this, &ModelDownloadDialog::on_network_reply_finished);
        connect(current_reply_, &QNetworkReply::errorOccurred, this, &ModelDownloadDialog::on_network_reply_error);
        return;
    }

    const QNetworkReply::NetworkError reply_error = reply->error();
    const int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    reply->deleteLater();

    if (reply_error != QNetworkReply::NoError || status_code >= 400) {
        if (part_file_.isOpen()) {
            part_file_.flush();
            part_file_.close();
        }
        state_ = DownloadState::Error;
        atualizar_widgets_estado();
        return;
    }

    if (part_file_.isOpen()) {
        part_file_.flush();
        part_file_.close();
    }

    const QString dest_path = resolve_model_path(app_);
    const QString part_path = dest_path + QStringLiteral(".part");

    const QFileInfo part_info(part_path);
    if (part_info.size() >= 2000000000LL) {
        QFile check_file(part_path);
        if (check_file.open(QIODevice::ReadOnly) && check_file.read(4) == QByteArrayLiteral("GGUF")) {
            check_file.close();

            if (QFile::exists(dest_path)) {
                QFile::remove(dest_path);
            }

            if (QFile::rename(part_path, dest_path)) {
                state_ = DownloadState::Finished;
                atualizar_widgets_estado();
                if (app_ != nullptr) {
                    app_->refresh_ollama_models();
                }
                emit model_download_status_changed();

                QMessageBox::information(
                    this,
                    get_text(QStringLiteral("Download Concluído")),
                    get_text(QStringLiteral("O modelo Qwen 2.5 foi baixado, validado e instalado com sucesso!\n\n"
                                           "Ele já está pronto para uso em análises e resumos do Mapa Mental.")));
                return;
            }
        }
    }

    state_ = DownloadState::Error;
    atualizar_widgets_estado();
    QMessageBox::warning(
        this,
        get_text(QStringLiteral("Falha na Validação")),
        get_text(QStringLiteral("O arquivo baixado não pôde ser validado como um modelo GGUF íntegro.")));
}

void ModelDownloadDialog::on_network_reply_error()
{
    if (current_reply_ == nullptr) {
        return;
    }

    if (current_reply_->error() == QNetworkReply::OperationCanceledError) {
        return;
    }

    if (part_file_.isOpen()) {
        part_file_.flush();
        part_file_.close();
    }

    state_ = DownloadState::Error;
    atualizar_widgets_estado();
}

QString ModelDownloadDialog::format_bytes(qint64 bytes) const
{
    if (bytes < 1024) {
        return QStringLiteral("%1 B").arg(bytes);
    }
    if (bytes < 1024 * 1024) {
        return QStringLiteral("%1 KB").arg(QString::number(bytes / 1024.0, 'f', 1));
    }
    if (bytes < 1024 * 1024 * 1024) {
        return QStringLiteral("%1 MB").arg(QString::number(bytes / (1024.0 * 1024.0), 'f', 1));
    }
    return QStringLiteral("%1 GB").arg(QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2));
}

QString ModelDownloadDialog::format_speed(double bytes_per_sec) const
{
    if (bytes_per_sec < 1024.0 * 1024.0) {
        return QStringLiteral("%1 KB/s").arg(QString::number(bytes_per_sec / 1024.0, 'f', 1));
    }
    return QStringLiteral("%1 MB/s").arg(QString::number(bytes_per_sec / (1024.0 * 1024.0), 'f', 1));
}

QString ModelDownloadDialog::format_eta(int seconds) const
{
    if (seconds <= 0) {
        return QString();
    }
    if (seconds < 60) {
        return get_text(QStringLiteral("Restam ~%1 s")).arg(seconds);
    }
    if (seconds < 3600) {
        const int mins = seconds / 60;
        const int secs = seconds % 60;
        return get_text(QStringLiteral("Restam ~%1 min %2 s")).arg(mins).arg(secs);
    }
    const int hours = seconds / 3600;
    const int mins = (seconds % 3600) / 60;
    return get_text(QStringLiteral("Restam ~%1 h %2 min")).arg(hours).arg(mins);
}

}  // namespace source::GerenciamentoUI
