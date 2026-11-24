#include "source/ui/ui_17_OllamaInstrucoesDialog.hpp"

#include <QApplication>
#include <QClipboard>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QEvent>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QScrollArea>
#include <QShowEvent>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

#include "source/app/app_01_eisenhower_matrix_app.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"

namespace source::GerenciamentoUI {
namespace {

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

}  // namespace

OllamaInstrucoesDialog::OllamaInstrucoesDialog(source::EisenhowerMatrixApp *app, QWidget *parent)
    : QDialog(parent), app_(app)
{
    setMinimumSize(780, 560);
    resize(840, 620);

    auto *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(16, 16, 16, 16);
    main_layout->setSpacing(12);

    // Cabeçalho
    auto *header_frame = new QFrame(this);
    header_frame->setFrameShape(QFrame::StyledPanel);
    auto *header_layout = new QVBoxLayout(header_frame);
    header_layout->setContentsMargins(12, 10, 12, 10);
    header_layout->setSpacing(4);

    title_label_ = new QLabel(header_frame);
    title_label_->setTextFormat(Qt::PlainText);

    sub_label_ = new QLabel(header_frame);
    sub_label_->setWordWrap(true);
    sub_label_->setTextFormat(Qt::PlainText);

    header_layout->addWidget(title_label_);
    header_layout->addWidget(sub_label_);
    main_layout->addWidget(header_frame);

    tab_widget_ = new QTabWidget(this);
    main_layout->addWidget(tab_widget_, 1);

    // Rodapé com botões
    button_box_ = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(button_box_, &QDialogButtonBox::rejected, this, &QDialog::accept);
    main_layout->addWidget(button_box_);

    if (app_ != nullptr && app_->gerenciador_traducao != nullptr) {
        connect(app_->gerenciador_traducao,
                &source::language::GerenciadorTraducao::idioma_alterado,
                this,
                [this](const QString &) { retranslate_ui(); });
    }

    retranslate_ui();
}

void OllamaInstrucoesDialog::changeEvent(QEvent *event)
{
    if (event != nullptr) {
        switch (event->type()) {
            case QEvent::LanguageChange:
                retranslate_ui();
                break;
            case QEvent::ApplicationPaletteChange:
            case QEvent::PaletteChange:
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

void OllamaInstrucoesDialog::showEvent(QShowEvent *event)
{
    retranslate_ui();
    QDialog::showEvent(event);
}

void OllamaInstrucoesDialog::agendar_refresh_tema()
{
    if (applying_theme_ || theme_refresh_pending_) {
        return;
    }
    theme_refresh_pending_ = true;
    // Adia para o próximo ciclo: garante que o Qt já propagou a nova paleta do sistema
    QTimer::singleShot(0, this, [this]() {
        theme_refresh_pending_ = false;
        refresh_theme();
    });
}

void OllamaInstrucoesDialog::refresh_theme()
{
    aplicar_estilos_tema();
    update();
}

void OllamaInstrucoesDialog::aplicar_estilos_tema()
{
    if (applying_theme_) {
        return;
    }
    applying_theme_ = true;

    // Cores resolvidas a partir da paleta ativa (clara/escura) no momento da troca de tema.
    // Usar cores explícitas evita que o QStyleSheetStyle mantenha a cor "congelada" do tema anterior.
    const QPalette pal = QApplication::palette();
    const QColor text_color = pal.color(QPalette::Active, QPalette::WindowText);
    const QColor border_color = pal.color(QPalette::Active, QPalette::Mid);

    if (title_label_ != nullptr) {
        title_label_->setStyleSheet(
            QStringLiteral("QLabel { color: %1; font-size: 15px; font-weight: bold; }").arg(css_rgba(text_color)));
    }
    if (sub_label_ != nullptr) {
        sub_label_->setStyleSheet(
            QStringLiteral("QLabel { color: %1; font-size: 12px; }").arg(css_rgba(text_color, 217)));
    }

    const QString cmd_style = QStringLiteral(
        "QLabel { color: %1; background: transparent; border: 1px solid %2; padding: 5px 8px; "
        "border-radius: 4px; font-family: Consolas, monospace; }")
        .arg(css_rgba(text_color), css_rgba(border_color));

    for (auto it = cmd_labels_.begin(); it != cmd_labels_.end();) {
        if (it->isNull()) {
            it = cmd_labels_.erase(it);
            continue;
        }
        (*it)->setStyleSheet(cmd_style);
        ++it;
    }

    applying_theme_ = false;
}

QLabel *OllamaInstrucoesDialog::criar_label_comando(const QString &comando, QWidget *parent)
{
    auto *cmd_label = new QLabel(parent);
    cmd_label->setTextFormat(Qt::PlainText);
    cmd_label->setText(comando);
    cmd_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    cmd_labels_.append(cmd_label);
    return cmd_label;
}

void OllamaInstrucoesDialog::retranslate_ui()
{
    setWindowTitle(get_text(QStringLiteral("Guia e Instruções de Modelos Ollama - Mapa Mental")));

    if (title_label_ != nullptr) {
        title_label_->setText(get_text(QStringLiteral("Expansão de Inteligência Artificial com Ollama")));
    }
    if (sub_label_ != nullptr) {
        sub_label_->setText(get_text(
            QStringLiteral("Utilize modelos de linguagem locais (como Llama 3.2, Qwen 2.5, Gemma 3) "
                           "instalados via Ollama para estruturar e sintetizar árvores de tópicos do Mapa Mental.")));
    }

    recriar_abas();
    aplicar_estilos_tema();

    if (button_box_ != nullptr) {
        if (auto *btn_close = button_box_->button(QDialogButtonBox::Close)) {
            btn_close->setText(get_text(QStringLiteral("Fechar")));
        }
    }
}

void OllamaInstrucoesDialog::copiar_comando(const QString &comando)
{
    if (QClipboard *clipboard = QApplication::clipboard()) {
        clipboard->setText(comando);
    }
}

void OllamaInstrucoesDialog::recriar_abas()
{
    if (tab_widget_ == nullptr) {
        return;
    }

    const int prev_index = qMax(0, tab_widget_->currentIndex());

    while (tab_widget_->count() > 0) {
        QWidget *page = tab_widget_->widget(0);
        tab_widget_->removeTab(0);
        if (page != nullptr) {
            page->deleteLater();
        }
    }

    status_conexao_label_ = nullptr;
    lista_modelos_header_label_ = nullptr;
    btn_refresh_ = nullptr;
    lista_modelos_widget_ = nullptr;
    cmd_labels_.clear();

    criar_aba_passo_a_passo();
    criar_aba_modelos_recomendados();
    criar_aba_gerenciamento();
    criar_aba_status_local();

    if (prev_index < tab_widget_->count()) {
        tab_widget_->setCurrentIndex(prev_index);
    }

    atualizar_status();
}

void OllamaInstrucoesDialog::criar_aba_passo_a_passo()
{
    auto *scroll_area = new QScrollArea(tab_widget_);
    scroll_area->setWidgetResizable(true);
    scroll_area->setFrameShape(QFrame::NoFrame);

    auto *content_widget = new QWidget(scroll_area);
    auto *layout = new QVBoxLayout(content_widget);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(14);

    const auto add_step = [&](const QString &step_title, const QString &description, const QString &code_cmd = QString()) {
        auto *box = new QGroupBox(step_title, content_widget);
        auto *box_layout = new QVBoxLayout(box);
        box_layout->setSpacing(8);

        auto *desc_label = new QLabel(description, box);
        desc_label->setWordWrap(true);
        desc_label->setTextFormat(Qt::RichText);
        desc_label->setOpenExternalLinks(true);
        box_layout->addWidget(desc_label);

        if (!code_cmd.isEmpty()) {
            auto *cmd_row = new QHBoxLayout();
            auto *cmd_label = criar_label_comando(code_cmd, box);
            auto *copy_btn = new QPushButton(get_text(QStringLiteral("Copiar Comando")), box);
            copy_btn->setFixedWidth(140);
            connect(copy_btn, &QPushButton::clicked, this, [this, code_cmd]() {
                copiar_comando(code_cmd);
            });
            cmd_row->addWidget(cmd_label, 1);
            cmd_row->addWidget(copy_btn);
            box_layout->addLayout(cmd_row);
        }

        layout->addWidget(box);
    };

    add_step(
        get_text(QStringLiteral("Passo 1: Instalar o Ollama")),
        get_text(QStringLiteral("Baixe e instale o Ollama gratuitamente para Windows através do site oficial: "
                                "<a href=\"https://ollama.com/download\">https://ollama.com/download</a>.<br>"
                                "Após instalar, o Ollama roda silenciosamente em segundo plano na barra de tarefas do Windows.")),
        QString());

    add_step(
        get_text(QStringLiteral("Passo 2: Baixar um Modelo de Linguagem")),
        get_text(QStringLiteral("Abra o <b>PowerShell</b> ou o <b>Prompt de Comando (CMD)</b> do Windows e execute o comando abaixo para baixar o modelo recomendado:")),
        QStringLiteral("ollama run llama3.2:3b"));

    add_step(
        get_text(QStringLiteral("Passo 3: Reconhecimento Automático")),
        get_text(QStringLiteral("O aplicativo reconhece imediatamente quaisquer modelos instalados no Ollama!<br>"
                                "Vá no menu <b>Configurações → Configurações Específicas → Mapa Mental → Nós → Modelos Ollama</b> "
                                "e selecione o modelo desejado com um clique.<br>"
                                "Caso instale um novo modelo enquanto o app estiver aberto, o menu atualiza automaticamente a lista.")),
        QString());

    add_step(
        get_text(QStringLiteral("Passo 4: Geração com Aceleração e Resiliência")),
        get_text(QStringLiteral("Ao importar ou processar um documento no Mapa Mental, o modelo selecionado será utilizado "
                                "para gerar uma síntese hierárquica precisa por seções.<br>"
                                "<b>Vantagens exclusivas:</b>"
                                "<ul>"
                                "<li><b>Aceleração GPU:</b> A API do Ollama distribui camadas automaticamente na placa de vídeo.</li>"
                                "<li><b>Modo Offline / Fallback:</b> Se o serviço do Ollama estiver pausado, o aplicativo lê "
                                "diretamente o arquivo GGUF do modelo presente na pasta de blobs do Ollama via engine local!</li>"
                                "<li><b>Desinstalação Segura:</b> Se um modelo for removido pelo usuário (<code>ollama rm</code>), "
                                "o sistema detecta a ausência e retorna de forma segura ao modelo padrão.</li>"
                                "</ul>")),
        QString());

    layout->addStretch();
    scroll_area->setWidget(content_widget);
    tab_widget_->addTab(scroll_area, get_text(QStringLiteral("Passo a Passo")));
}

void OllamaInstrucoesDialog::criar_aba_modelos_recomendados()
{
    auto *scroll_area = new QScrollArea(tab_widget_);
    scroll_area->setWidgetResizable(true);
    scroll_area->setFrameShape(QFrame::NoFrame);

    auto *content_widget = new QWidget(scroll_area);
    auto *layout = new QVBoxLayout(content_widget);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    auto *intro_label = new QLabel(
        get_text(QStringLiteral("Selecione um dos modelos testados abaixo de acordo com a memória RAM/VRAM do seu computador. "
                                "Clique em <b>Copiar Comando</b> e execute-o no seu terminal para instalar:")),
        content_widget);
    intro_label->setWordWrap(true);
    layout->addWidget(intro_label);

    struct ModelCard {
        QString name;
        QString tag;
        QString size;
        QString ramReq;
        QString desc;
    };

    const QList<ModelCard> models = {
        {QStringLiteral("Llama 3.2 (3B)"), QStringLiteral("llama3.2:3b"), QStringLiteral("2.0 GB"), QStringLiteral("4 GB RAM"),
         get_text(QStringLiteral("Mais rápido e leve. Excelente para livros gerais, artigos e sínteses ágeis sem sobrecarregar o sistema."))},
        {QStringLiteral("Qwen 2.5 (7B)"), QStringLiteral("qwen2.5:7b"), QStringLiteral("4.7 GB"), QStringLiteral("8 GB RAM / VRAM"),
         get_text(QStringLiteral("Referência em estruturação analítica e fluência em língua portuguesa. Altamente recomendado para documentos técnicos."))},
        {QStringLiteral("Gemma 3 (4B)"), QStringLiteral("gemma3:4b"), QStringLiteral("3.3 GB"), QStringLiteral("6 GB RAM"),
         get_text(QStringLiteral("Modelo moderno da Google com excelente síntese conceitual, alta densidade semântica e respostas concisas."))},
        {QStringLiteral("Llama 3.1 (8B)"), QStringLiteral("llama3.1:8b"), QStringLiteral("4.9 GB"), QStringLiteral("8-12 GB RAM"),
         get_text(QStringLiteral("Capacidade de raciocínio profundo e amplo vocabulário editorial. Ótimo para capítulos longos e livros densos."))},
        {QStringLiteral("Qwen 2.5 (14B)"), QStringLiteral("qwen2.5:14b"), QStringLiteral("9.0 GB"), QStringLiteral("16 GB RAM / GPU 12GB+"),
         get_text(QStringLiteral("Máxima precisão e profundidade semântica. Indicado para quem possui GPU dedicada com 12 GB+ de VRAM."))}
    };

    for (const auto &m : models) {
        const QString header_str = QStringLiteral("%1  •  %2  •  %3").arg(
            m.name,
            get_text(QStringLiteral("Tamanho: %1")).arg(m.size),
            get_text(QStringLiteral("Requisitos: %1")).arg(m.ramReq));
        auto *box = new QGroupBox(header_str, content_widget);
        auto *box_layout = new QVBoxLayout(box);
        box_layout->setSpacing(6);

        auto *desc = new QLabel(m.desc, box);
        desc->setWordWrap(true);
        box_layout->addWidget(desc);

        auto *row = new QHBoxLayout();
        const QString cmd = QStringLiteral("ollama run %1").arg(m.tag);
        auto *cmd_label = criar_label_comando(cmd, box);

        auto *copy_btn = new QPushButton(get_text(QStringLiteral("Copiar Comando")), box);
        copy_btn->setFixedWidth(140);
        connect(copy_btn, &QPushButton::clicked, this, [this, cmd]() {
            copiar_comando(cmd);
        });

        row->addWidget(cmd_label, 1);
        row->addWidget(copy_btn);
        box_layout->addLayout(row);

        layout->addWidget(box);
    }

    layout->addStretch();
    scroll_area->setWidget(content_widget);
    tab_widget_->addTab(scroll_area, get_text(QStringLiteral("Modelos Recomendados")));
}

void OllamaInstrucoesDialog::criar_aba_gerenciamento()
{
    auto *scroll_area = new QScrollArea(tab_widget_);
    scroll_area->setWidgetResizable(true);
    scroll_area->setFrameShape(QFrame::NoFrame);

    auto *content_widget = new QWidget(scroll_area);
    auto *layout = new QVBoxLayout(content_widget);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    const auto add_cmd_entry = [&](const QString &title, const QString &cmd, const QString &explanation) {
        auto *box = new QGroupBox(title, content_widget);
        auto *box_layout = new QVBoxLayout(box);

        auto *exp_label = new QLabel(explanation, box);
        exp_label->setWordWrap(true);
        box_layout->addWidget(exp_label);

        auto *row = new QHBoxLayout();
        auto *cmd_label = criar_label_comando(cmd, box);
        auto *copy_btn = new QPushButton(get_text(QStringLiteral("Copiar")), box);
        copy_btn->setFixedWidth(100);
        connect(copy_btn, &QPushButton::clicked, this, [this, cmd]() {
            copiar_comando(cmd);
        });
        row->addWidget(cmd_label, 1);
        row->addWidget(copy_btn);
        box_layout->addLayout(row);

        layout->addWidget(box);
    };

    add_cmd_entry(
        get_text(QStringLiteral("Listar Modelos Instalados")),
        QStringLiteral("ollama list"),
        get_text(QStringLiteral("Exibe a lista de todos os modelos baixados na sua máquina com tamanho e data de modificação.")));

    add_cmd_entry(
        get_text(QStringLiteral("Baixar Modelo sem Executar")),
        QStringLiteral("ollama pull llama3.2:3b"),
        get_text(QStringLiteral("Faz o download do modelo em segundo plano para que fique disponível imediatamente para o aplicativo.")));

    add_cmd_entry(
        get_text(QStringLiteral("Excluir / Desinstalar Modelo")),
        QStringLiteral("ollama rm llama3.2:3b"),
        get_text(QStringLiteral("Libera espaço em disco desinstalando o modelo. O aplicativo reconhecerá a remoção e removerá o item do menu automaticamente.")));

    add_cmd_entry(
        get_text(QStringLiteral("Verificar Status do Serviço")),
        QStringLiteral("ollama ps"),
        get_text(QStringLiteral("Mostra quais modelos estão atualmente carregados na memória RAM/VRAM.")));

    auto *info_box = new QGroupBox(get_text(QStringLiteral("Onde os modelos ficam armazenados no Windows?")), content_widget);
    auto *info_layout = new QVBoxLayout(info_box);
    auto *info_text = new QLabel(
        get_text(QStringLiteral("Por padrão, os manifests e arquivos de pesos (blobs GGUF) são salvos em:<br>"
                                "<code>%USERPROFILE%\\.ollama\\models</code><br><br>"
                                "Você pode customizar o drive ou pasta de armazenamento configurando a variável de ambiente do sistema "
                                "<b>OLLAMA_MODELS</b> (por exemplo: <code>D:\\IA\\Ollama\\models</code>). "
                                "O aplicativo detecta automaticamente esse caminho configurado.")),
        info_box);
    info_text->setWordWrap(true);
    info_layout->addWidget(info_text);
    layout->addWidget(info_box);

    layout->addStretch();
    scroll_area->setWidget(content_widget);
    tab_widget_->addTab(scroll_area, get_text(QStringLiteral("Comandos e Gerenciamento")));
}

void OllamaInstrucoesDialog::criar_aba_status_local()
{
    auto *content_widget = new QWidget(tab_widget_);
    auto *layout = new QVBoxLayout(content_widget);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    // Barra de status com botão de recarregar
    auto *status_bar = new QHBoxLayout();
    status_conexao_label_ = new QLabel(get_text(QStringLiteral("Verificando status do serviço Ollama...")), content_widget);
    status_conexao_label_->setStyleSheet(QStringLiteral("font-weight: bold;"));

    btn_refresh_ = new QPushButton(get_text(QStringLiteral("Verificar / Recarregar")), content_widget);
    connect(btn_refresh_, &QPushButton::clicked, this, &OllamaInstrucoesDialog::atualizar_status);

    status_bar->addWidget(status_conexao_label_, 1);
    status_bar->addWidget(btn_refresh_);
    layout->addLayout(status_bar);

    lista_modelos_header_label_ = new QLabel(get_text(QStringLiteral("Modelos instalados detectados neste computador:")), content_widget);
    layout->addWidget(lista_modelos_header_label_);

    lista_modelos_widget_ = new QListWidget(content_widget);
    lista_modelos_widget_->setAlternatingRowColors(true);
    layout->addWidget(lista_modelos_widget_, 1);

    auto *btn_gguf_link = new QPushButton(get_text(QStringLiteral("Gerenciar / Baixar Modelo Local Qwen 2.5 (GGUF)...")), content_widget);
    connect(btn_gguf_link, &QPushButton::clicked, this, [this]() {
        if (app_ != nullptr) {
            app_->show_model_download_dialog();
        }
    });
    layout->addWidget(btn_gguf_link);

    tab_widget_->addTab(content_widget, get_text(QStringLiteral("Modelos Detectados no Sistema")));
}

void OllamaInstrucoesDialog::atualizar_status()
{
    if (status_conexao_label_ == nullptr || lista_modelos_widget_ == nullptr) {
        return;
    }

    const QVariantMap diag = app_ != nullptr ? app_->ollama_diagnostic() : QVariantMap{};
    const bool api_online = diag.value(QStringLiteral("api_conectada")).toBool();

    if (api_online) {
        status_conexao_label_->setText(QStringLiteral("<font color=\"#2e7d32\">✔ %1 (localhost:11434)</font>").arg(
            get_text(QStringLiteral("Serviço Ollama Online"))));
    } else {
        status_conexao_label_->setText(QStringLiteral("<font color=\"#d32f2f\">✖ %1</font>").arg(
            get_text(QStringLiteral("Serviço Ollama não detectado ou inativo (Modo de leitura direta GGUF ativo)"))));
    }

    lista_modelos_widget_->clear();
    const QVariantList modelos = diag.value(QStringLiteral("modelos")).toList();

    if (modelos.isEmpty()) {
        auto *item = new QListWidgetItem(get_text(QStringLiteral("Nenhum modelo Ollama detectado no sistema.")), lista_modelos_widget_);
        item->setFlags(Qt::NoItemFlags);
        return;
    }

    const QString tag_gguf = get_text(QStringLiteral("[GGUF local verificado]"));

    for (const QVariant &m_val : modelos) {
        const QVariantMap m = m_val.toMap();
        const QString nome = m.value(QStringLiteral("nome")).toString();
        const QString tamanho = m.value(QStringLiteral("tamanho_formatado")).toString();
        const QString params = m.value(QStringLiteral("parametros")).toString();
        const bool blob_ok = m.value(QStringLiteral("blob_gguf_valido")).toBool();

        QString display = QStringLiteral("● %1").arg(nome);
        if (!params.isEmpty()) {
            display += QStringLiteral("  [%1]").arg(params);
        }
        if (!tamanho.isEmpty()) {
            display += QStringLiteral("  (%1)").arg(tamanho);
        }
        if (blob_ok) {
            display += QStringLiteral("  - %1").arg(tag_gguf);
        }

        auto *item = new QListWidgetItem(display, lista_modelos_widget_);
        item->setToolTip(m.value(QStringLiteral("caminho_blob")).toString());
    }
}

}  // namespace source::GerenciamentoUI
