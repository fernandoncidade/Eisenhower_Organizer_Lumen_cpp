#include "source/modules/agenda/AgendaAvaliacoes/BancoDeDados/Banco_Dados.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/GerenciamentoInterfaceGrafica/Interface_Grafica.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/utils/ComboBoxUtils.hpp"
#include "source/utils/LogManager.hpp"

#include <functional>

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMimeData>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QTime>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

namespace {

class AgendaFileDropLineEdit final : public QLineEdit {
public:
    explicit AgendaFileDropLineEdit(QWidget *parent = nullptr) : QLineEdit(parent)
    {
        setAcceptDrops(true);
    }

    std::function<void(const QString &)> onFileDropped;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override
    {
        if (event != nullptr && event->mimeData()->hasUrls()) {
            event->acceptProposedAction();
            return;
        }
        QLineEdit::dragEnterEvent(event);
    }

    void dragMoveEvent(QDragMoveEvent *event) override
    {
        if (event != nullptr && event->mimeData()->hasUrls()) {
            event->acceptProposedAction();
            return;
        }
        QLineEdit::dragMoveEvent(event);
    }

    void dropEvent(QDropEvent *event) override
    {
        if (event != nullptr && event->mimeData()->hasUrls()) {
            const QList<QUrl> urls = event->mimeData()->urls();
            if (!urls.isEmpty()) {
                const QString path = urls.constFirst().toLocalFile();
                if (!path.isEmpty() && onFileDropped) {
                    onFileDropped(path);
                }
            }
            event->acceptProposedAction();
            return;
        }
        QLineEdit::dropEvent(event);
    }
};

}  // namespace

void InterfaceGerenciadorAtividades::create_widgets() {
    auto *layout_horizontal_1 = new QHBoxLayout();
    auto *layout_vertical_1 = new QVBoxLayout();
    auto *layout_vertical_2 = new QVBoxLayout();
    layout_horizontal_1->setSpacing(8);
    layout_vertical_1->setSpacing(4);
    layout_vertical_2->setSpacing(4);

    auto *layout_agenda_exemplo = new QVBoxLayout();
    layout_agenda_exemplo->setContentsMargins(0, 0, 0, 0);
    layout_agenda_exemplo->setSpacing(4);
    agenda_exemplo_widget = new QWidget(this);
    agenda_exemplo_widget->setLayout(layout_agenda_exemplo);

    label_curso = new QLabel(this);
    combo_curso = new QComboBox(this);
    configurar_combo_texto_livre(combo_curso);
    combo_curso->setMinimumWidth(300);
    combo_curso->setMaximumWidth(300);
    combo_curso->addItems(obter_cursos());
    connect(combo_curso, &QComboBox::currentTextChanged, this, &InterfaceGerenciadorAtividades::update_ementa);
    layout_agenda_exemplo->addWidget(label_curso);
    layout_agenda_exemplo->addWidget(combo_curso);
    label_ementa = new QLabel(this);
    entry_ementa = new QComboBox(this);
    configurar_combo_texto_livre(entry_ementa);
    entry_ementa->setMinimumWidth(300);
    entry_ementa->setMaximumWidth(300);
    connect(entry_ementa, &QComboBox::currentTextChanged, this, &InterfaceGerenciadorAtividades::update_semestre);
    layout_agenda_exemplo->addWidget(label_ementa);
    layout_agenda_exemplo->addWidget(entry_ementa);
    label_semestre = new QLabel(this);
    entry_semestre = new QComboBox(this);
    configurar_combo_texto_livre(entry_semestre);
    entry_semestre->setMinimumWidth(300);
    entry_semestre->setMaximumWidth(300);
    connect(entry_semestre, &QComboBox::currentTextChanged, this, &InterfaceGerenciadorAtividades::update_disciplinas);
    layout_agenda_exemplo->addWidget(label_semestre);
    layout_agenda_exemplo->addWidget(entry_semestre);
    label_disciplina = new QLabel(this);
    entry_disciplina = new QComboBox(this);
    configurar_combo_texto_livre(entry_disciplina);
    entry_disciplina->setMinimumWidth(300);
    entry_disciplina->setMaximumWidth(300);
    layout_agenda_exemplo->addWidget(label_disciplina);
    layout_agenda_exemplo->addWidget(entry_disciplina);
    label_codigo = new QLabel(this);
    entry_codigo = new QComboBox(this);
    configurar_combo_texto_livre(entry_codigo);
    entry_codigo->setMinimumWidth(300);
    entry_codigo->setMaximumWidth(300);
    entry_codigo->addItems(obter_turmas());
    layout_agenda_exemplo->addWidget(label_codigo);
    layout_agenda_exemplo->addWidget(entry_codigo);
    label_tipo = new QLabel(this);
    combo_tipo = new QComboBox(this);
    configurar_combo_texto_livre(combo_tipo);
    combo_tipo->setMinimumWidth(300);
    combo_tipo->setMaximumWidth(300);
    combo_tipo->addItems(obter_avaliacoes());
    layout_agenda_exemplo->addWidget(label_tipo);
    layout_agenda_exemplo->addWidget(combo_tipo);
    label_sequencia = new QLabel(this);
    combo_sequencia = new QComboBox(this);
    configurar_combo_texto_livre(combo_sequencia);
    combo_sequencia->setMinimumWidth(300);
    combo_sequencia->setMaximumWidth(300);
    combo_sequencia->addItems(obter_sequencias());
    layout_agenda_exemplo->addWidget(label_sequencia);
    layout_agenda_exemplo->addWidget(combo_sequencia);

    agenda_custom_widget = new QWidget(this);
    agenda_custom_layout = new QVBoxLayout();
    agenda_custom_layout->setContentsMargins(0, 0, 0, 0);
    agenda_custom_layout->setSpacing(4);
    agenda_custom_widget->setLayout(agenda_custom_layout);

    for (int i = 0; i < 8; ++i) {
        CustomAgendaRow row;
        row.widget = new QWidget(agenda_custom_widget);
        auto *row_layout = new QVBoxLayout(row.widget);
        row_layout->setContentsMargins(0, 0, 0, 0);
        row_layout->setSpacing(4);
        auto *title_layout = new QHBoxLayout();
        title_layout->setContentsMargins(0, 0, 0, 0);
        title_layout->setSpacing(4);
        row.enabled = new QCheckBox(row.widget);
        row.title_edit = new QLineEdit(row.widget);
        row.title_edit->setMinimumWidth(272);
        row.title_edit->setMaximumWidth(272);
        row.title_edit->setPlaceholderText(
            QCoreApplication::translate("InterfaceGerenciadorAtividades", "Insira um título para a nova lista"));
        row.title_label = new QLabel(row.widget);
        row.title_label->setMinimumWidth(272);
        row.title_label->setMaximumWidth(272);
        row.title_label->setVisible(false);
        row.combo = new QComboBox(row.widget);
        configurar_combo_texto_livre(row.combo);
        row.combo->setMinimumWidth(300);
        row.combo->setMaximumWidth(300);
        row.title_edit->setEnabled(false);
        row.title_edit->setVisible(false);
        row.combo->setEnabled(false);
        row.combo->setVisible(false);
        row.widget->setVisible(false);
        title_layout->addWidget(row.enabled);
        title_layout->addWidget(row.title_edit, 1);
        title_layout->addWidget(row.title_label, 1);
        row_layout->addLayout(title_layout);
        row_layout->addWidget(row.combo);
        agenda_custom_layout->addWidget(row.widget);

        connect(row.enabled, &QCheckBox::toggled, this, [this, i](bool enabled) {
            if (i < 0 || i >= custom_agenda_rows.size()) {
                return;
            }
            CustomAgendaRow &linha = custom_agenda_rows[i];
            const bool titulo_confirmado =
                linha.title_label != nullptr && !linha.title_label->text().trimmed().isEmpty();
            if (titulo_confirmado) {
                if (linha.enabled != nullptr) {
                    linha.enabled->setChecked(true);
                    linha.enabled->setEnabled(false);
                    linha.enabled->setVisible(false);
                }
                atualizar_visibilidade_linhas_customizadas();
                return;
            }
            if (linha.title_edit != nullptr) {
                linha.title_edit->setEnabled(enabled);
                linha.title_edit->setVisible(true);
            }
            if (linha.title_label != nullptr) {
                linha.title_label->setVisible(false);
            }
            if (linha.combo != nullptr) {
                linha.combo->setEnabled(false);
                linha.combo->setVisible(false);
            }
            salvar_agenda_customizada_atual();
            atualizar_visibilidade_linhas_customizadas();
        });
        connect(row.title_edit, &QLineEdit::editingFinished, this, [this] { salvar_agenda_customizada_atual(); });
        connect(row.title_edit, &QLineEdit::returnPressed, this, [this, i] { confirmar_titulo_linha_customizada(i); });
        if (QLineEdit *edit = row.combo->lineEdit()) {
            connect(edit, &QLineEdit::editingFinished, this, [this, i] {
                registrar_item_linha_agenda_customizada(i);
            });
        }
        connect(row.combo, &QComboBox::currentTextChanged, this, [this, i] {
            atualizar_dependencias_agenda_customizada(i);
            salvar_agenda_customizada_atual();
        });

        custom_agenda_rows.append(row);
    }
    atualizar_visibilidade_linhas_customizadas();
    agenda_custom_layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    agenda_stack = new QStackedWidget(this);
    agenda_stack->addWidget(agenda_exemplo_widget);
    agenda_stack->addWidget(agenda_custom_widget);
    agenda_stack->setCurrentWidget(agenda_exemplo_widget);
    layout_vertical_1->addWidget(agenda_stack);

    label_prioridade = new QLabel(this);
    combo_prioridade = new QComboBox(this);
    combo_prioridade->setMinimumWidth(300);
    combo_prioridade->setMaximumWidth(300);
    combo_prioridade->addItems({
        QStringLiteral("🔴 Importante e Urgente"),
        QStringLiteral("🟠 Importante, mas Não Urgente"),
        QStringLiteral("🟡 Não Importante, mas Urgente"),
        QStringLiteral("🟢 Não Importante e Não Urgente"),
    });
    combo_prioridade->setCurrentIndex(0);
    layout_vertical_1->addWidget(label_prioridade);
    layout_vertical_1->addWidget(combo_prioridade);

    label_dominio = new QLabel(this);
    combo_dominio = new QComboBox(this);
    combo_dominio->setMinimumWidth(300);
    combo_dominio->setMaximumWidth(300);
    combo_dominio->addItems({
        QStringLiteral("🥉 Iniciante (não consigo explicar)"),
        QStringLiteral("🥈 Intermediário (explico com dificuldade)"),
        QStringLiteral("🥇 Avançado (explico facilmente)"),
    });
    layout_vertical_1->addWidget(label_dominio);
    layout_vertical_1->addWidget(combo_dominio);

    label_arquivo_vinculado = new QLabel(this);
    arquivo_vinculado_edit = new AgendaFileDropLineEdit(this);
    arquivo_vinculado_edit->setReadOnly(true);
    arquivo_vinculado_edit->setClearButtonEnabled(false);
    arquivo_vinculado_edit->setMinimumWidth(300);
    arquivo_vinculado_edit->setMaximumWidth(300);
    layout_vertical_1->addWidget(label_arquivo_vinculado);
    layout_vertical_1->addWidget(arquivo_vinculado_edit);

    auto *layout_arquivo_botoes = new QHBoxLayout();
    layout_arquivo_botoes->setSpacing(4);
    button_escolher_arquivo = create_button();
    button_abrir_arquivo = create_button();
    button_remover_arquivo = create_button();
    for (QPushButton *button : {button_escolher_arquivo, button_abrir_arquivo, button_remover_arquivo}) {
        button->setMinimumWidth(0);
        button->setMaximumWidth(96);
    }
    button_abrir_arquivo->setEnabled(false);
    layout_arquivo_botoes->addWidget(button_escolher_arquivo);
    layout_arquivo_botoes->addWidget(button_abrir_arquivo);
    layout_arquivo_botoes->addWidget(button_remover_arquivo);
    layout_vertical_1->addLayout(layout_arquivo_botoes);

    const auto definir_arquivo_vinculado = [this](const QString &path) {
        const QString cleaned = path.trimmed();
        if (arquivo_vinculado_edit == nullptr) {
            return;
        }
        if (cleaned.isEmpty()) {
            arquivo_vinculado_edit->clear();
            arquivo_vinculado_edit->setToolTip(QString());
            if (button_abrir_arquivo != nullptr) {
                button_abrir_arquivo->setEnabled(false);
            }
            return;
        }

        const QFileInfo info(cleaned);
        arquivo_vinculado_edit->setText(info.fileName().isEmpty() ? cleaned : info.fileName());
        arquivo_vinculado_edit->setToolTip(info.absoluteFilePath());
        if (button_abrir_arquivo != nullptr) {
            button_abrir_arquivo->setEnabled(true);
        }
    };

    if (auto *drop_edit = dynamic_cast<AgendaFileDropLineEdit *>(arquivo_vinculado_edit)) {
        drop_edit->onFileDropped = definir_arquivo_vinculado;
    }

    connect(button_escolher_arquivo, &QPushButton::clicked, this, [this, definir_arquivo_vinculado] {
        const QString path = QFileDialog::getOpenFileName(
            this,
            QCoreApplication::translate("InterfaceGerenciadorAtividades", "Escolher arquivo"),
            QString(),
            QStringLiteral("*"));
        if (!path.isEmpty()) {
            definir_arquivo_vinculado(path);
        }
    });
    connect(button_abrir_arquivo, &QPushButton::clicked, this, [this] {
        if (arquivo_vinculado_edit == nullptr) {
            return;
        }
        const QString path = arquivo_vinculado_edit->toolTip().trimmed();
        if (!path.isEmpty()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
    });
    connect(button_remover_arquivo, &QPushButton::clicked, this, [definir_arquivo_vinculado] {
        definir_arquivo_vinculado(QString());
    });

    button_submiter = create_button();
    connect(button_submiter, &QPushButton::clicked, this, &InterfaceGerenciadorAtividades::submiter);
    button_limpar_definicoes = create_button();
    connect(button_limpar_definicoes,
            &QPushButton::clicked,
            this,
            &InterfaceGerenciadorAtividades::limpar_definicoes_selecionadas);
    button_fechar = create_button();
    connect(button_fechar, &QPushButton::clicked, this, &InterfaceGerenciadorAtividades::close);

    label_data = new QLabel(this);
    calendar = new QCalendarWidget(this);
    calendar->setMinimumWidth(300);
    calendar->setMaximumWidth(300);
    calendar->setMaximumHeight(240);
    layout_vertical_2->addWidget(label_data);
    layout_vertical_2->addWidget(calendar);

    check_vincular_data = new QCheckBox(this);
    check_vincular_data->setChecked(true);
    layout_vertical_2->addWidget(check_vincular_data);

    auto *layout_horario = new QHBoxLayout();
    check_vincular_horario = new QCheckBox(this);
    check_vincular_horario->setChecked(true);
    time_edit = new QTimeEdit(this);
    time_edit->setDisplayFormat(QStringLiteral("HH:mm"));
    time_edit->setTime(QTime::currentTime());
    layout_horario->addWidget(check_vincular_horario);
    layout_horario->addWidget(time_edit, 1);
    layout_vertical_2->addLayout(layout_horario);
    connect(check_vincular_data, &QCheckBox::toggled, this, &InterfaceGerenciadorAtividades::sync_date_time_controls);
    connect(check_vincular_horario, &QCheckBox::toggled, this, &InterfaceGerenciadorAtividades::sync_date_time_controls);

    layout_vertical_2->addWidget(button_submiter);
    layout_vertical_2->addWidget(button_limpar_definicoes);
    layout_vertical_2->addWidget(button_fechar);
    layout_vertical_2->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    if (!painel_esquerdo_apenas) {
        auto *layout_botoes = new QHBoxLayout();
        auto *layout_sub_vertical_1 = new QVBoxLayout();
        auto *layout_sub_vertical_2 = new QVBoxLayout();
        button_clear_item = create_button();
        connect(button_clear_item, &QPushButton::clicked, this, &InterfaceGerenciadorAtividades::excluir_item);
        button_clear_ultima = create_button();
        connect(button_clear_ultima, &QPushButton::clicked, this, &InterfaceGerenciadorAtividades::limpar_ultima_entrada);
        button_clear = create_button();
        connect(button_clear, &QPushButton::clicked, this, &InterfaceGerenciadorAtividades::limpar_entradas);
        button_editar_item = create_button();
        connect(button_editar_item, &QPushButton::clicked, this, &InterfaceGerenciadorAtividades::editar_item);
        button_export = create_button();
        connect(button_export, &QPushButton::clicked, this, &InterfaceGerenciadorAtividades::exportar_para_pdf);
        layout_sub_vertical_1->addWidget(button_clear_item);
        layout_sub_vertical_1->addWidget(button_clear_ultima);
        layout_sub_vertical_1->addWidget(button_clear);
        layout_sub_vertical_2->addWidget(button_editar_item);
        layout_sub_vertical_2->addWidget(button_export);
        layout_botoes->addLayout(layout_sub_vertical_1);
        layout_botoes->addLayout(layout_sub_vertical_2);
        layout_vertical_1->addLayout(layout_botoes);
    }

    layout_vertical_1->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    layout_horizontal_1->addLayout(layout_vertical_1);
    layout_horizontal_1->addLayout(layout_vertical_2);
    if (!painel_esquerdo_apenas) {
        auto *layout_vertical_3 = new QVBoxLayout();
        label_banco_dados = new QLabel(this);
        textbox = new QTextBrowser(this);
        textbox->setMinimumWidth(600);
        layout_vertical_3->addWidget(label_banco_dados);
        layout_vertical_3->addWidget(textbox);
        layout_horizontal_1->addLayout(layout_vertical_3);
    }
    auto *main_layout = new QVBoxLayout();
    main_layout->setContentsMargins(8, 8, 8, 8);
    main_layout->setSpacing(4);
    main_layout->addLayout(layout_horizontal_1);
    auto *central = new QWidget(this);
    central->setLayout(main_layout);
    setCentralWidget(central);
    gerenciamento_atividades.textbox = textbox;
    gerenciamento_atividades.combo_curso = combo_curso;
    gerenciamento_atividades.entry_codigo = entry_codigo;
    gerenciamento_atividades.combo_tipo = combo_tipo;
    gerenciamento_atividades.combo_sequencia = combo_sequencia;
    gerenciamento_atividades.calendar = calendar;
    gerenciamento_atividades.entry_disciplina = entry_disciplina;
    gerenciamento_atividades.entry_ementa = entry_ementa;
    gerenciamento_atividades.entry_semestre = entry_semestre;
    sync_date_time_controls();
}
