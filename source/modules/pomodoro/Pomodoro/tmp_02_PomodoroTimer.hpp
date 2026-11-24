#pragma once

#include <QString>
#include <QWidget>

#include "source/modules/pomodoro/Pomodoro/pomodoro_support.hpp"

class QLabel;
class QComboBox;
class QCheckBox;
class QGroupBox;
class QProgressBar;
class QPushButton;
class QTimer;

namespace source::plugin::Pomodoro {

class GerenciadorTarefas;

class PomodoroTimer : public QWidget {
    Q_OBJECT

public:
    explicit PomodoroTimer(QWidget *parent = nullptr);
    PomodoroTimer(const QString &ui_layout_mode_key, QWidget *parent);

    void set_gerenciador_tarefas(GerenciadorTarefas *gt);
    GerenciadorTarefas *gerenciador_tarefas() const;
    bool verificar_e_avisar_discrepancia_ciclos();

    void setup_ui();
    void atualizar_traducoes();
    void atualizar_configuracoes();
    void toggle_timer();
    void atualizar_timer();
    void atualizar_display();
    void _executar_alerta(const QString &titulo, const QString &texto, bool fechar_automaticamente = false);
    void finalizar_ciclo();
    void resetar_timer();
    void resetar_ciclo();
    void pular_ciclo();
    void _aplicar_tema_dinamico();
    static int obter_ciclos_permitidos_persistidos();

    bool eventFilter(QObject *obj, QEvent *event) override;

protected:
    void changeEvent(QEvent *event) override;

signals:
    void ciclo_completado(const QString &tipo);
    void ciclos_permitidos_alterados(int novos_ciclos);

public:
    source::utils::LogManager *logger_ = nullptr;
    int tempo_foco = 25 * 60;
    int tempo_descanso_curto = 5 * 60;
    int tempo_descanso_longo = 15 * 60;
    int ciclos_permitidos = 4;
    int tempo_restante = 25 * 60;
    bool em_foco = true;
    int ciclos_completados = 0;
    bool timer_ativo = false;
    GerenciadorTarefas *gerenciador_tarefas_ = nullptr;
    bool mismatch_acknowledged_ = false;

    compat::GerenciadorBotoesUI gerenciador_botoes_;
    compat::SoundManager *sound_mgr_ = nullptr;

    QTimer *timer = nullptr;
    QGroupBox *timer_group = nullptr;
    QLabel *label_tempo = nullptr;
    QProgressBar *progress = nullptr;
    QLabel *label_status = nullptr;
    QPushButton *btn_iniciar = nullptr;
    QPushButton *btn_resetar = nullptr;
    QPushButton *btn_resetar_ciclo = nullptr;
    QPushButton *btn_pular = nullptr;
    QGroupBox *config_group = nullptr;
    QLabel *label_foco = nullptr;
    QComboBox *spin_foco = nullptr;
    QLabel *label_descanso = nullptr;
    QComboBox *spin_descanso = nullptr;
    QLabel *label_ciclos = nullptr;
    QComboBox *spin_ciclos = nullptr;
    QCheckBox *chk_modo_automatico = nullptr;
    int input_ref_height_ = 0;
    QString ui_layout_mode_key_ = QStringLiteral("simple:tabs");
    QTimer *config_sync_timer_ = nullptr;
    int last_broadcasted_ciclos_ = 4;
};

}  // namespace source::plugin::Pomodoro
