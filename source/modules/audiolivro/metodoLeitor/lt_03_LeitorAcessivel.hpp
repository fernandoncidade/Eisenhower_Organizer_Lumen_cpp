#pragma once

#include <QPointer>
#include <QColor>
#include <QFont>
#include <QHash>
#include <QTextCursor>
#include <QTextEdit>
#include <QTimer>
#include <QVariantList>
#include <QWidget>

class QAudioOutput;
class QComboBox;
class QHBoxLayout;
class QLabel;
class QMediaPlayer;
class QPushButton;
class QShortcut;
class QSlider;
class QSpinBox;
class QTabWidget;
class QThread;

namespace source::plugin::Audiolivro {

class EdgeTTSWithTimestamps;
class FileDropEdit;
class FileDropPDFView;
class FocusRuler;
class PDFFindBar;
class PDFMouseHandler;
class TextFindBar;

class LeitorAcessivel : public QWidget
{
    Q_OBJECT

public:
    explicit LeitorAcessivel(QWidget *parent = nullptr);
    ~LeitorAcessivel() override;

    Q_INVOKABLE int total_task_count() const;
    Q_INVOKABLE int selected_task_count() const;
    Q_INVOKABLE QString voz_atual() const;
    Q_INVOKABLE QVariantList vozes_disponiveis() const;

    void reload_shared_state();
    void refresh_task_views();

public slots:
    void setup_ui();
    void atualizar_traducoes();
    void abrir_configuracoes_fonte();
    void abrir_colorir_tarefas();
    void restaurar_configuracoes();
    void restaurar_estado_carregado_temp();
    void salvar_texto_temp();
    void carregar_pdf(const QString &arquivo = QString(), bool keep_pdf_tab = false);
    void iniciar_leitura();
    void pausar_leitura();
    void parar_leitura();
    void leitura_finalizada();
    void toggle_regua_foco();
    void ativar_regua_foco();
    void desativar_regua_foco();
    void regua_fechada();
    void cleanup();
    void definir_voz(const QString &voz_id);
    void aplicar_fonte_configurada();
    void definir_fonte_configurada(const QFont &font);
    void atualizar_fonte_persistente();
    void criar_texto(bool confirmar = true);
    void salvar_como();
    void toggle_bullets();
    void set_line_spacing(double spacing);
    void set_indent(int indent);
    void set_margins(double margin);
    void limpar_painel();

signals:
    void task_counter_changed();

private slots:
    void update_pause_button();
    void play_generated_audio(const QString &path);
    void on_media_status(int status);
    void on_tts_error(const QString &err);
    void on_volume_changed(int value);
    void on_speed_changed(int value);
    void pdf_find_show();
    void pdf_find_toggle();
    void pdf_find_clear();
    void pdf_find_apply(const QString &query, bool match_case = false, bool whole_words = false);
    void pdf_find_next();
    void pdf_find_prev();
    void pdf_find_set_highlight_all(bool enable);
    void pdf_update_page_ui(int page_index, int total = -1);
    void text_find_show();
    void text_find_toggle();
    void text_find_clear();
    void text_find_apply(const QString &query, bool match_case = false, bool whole_words = false);
    void text_find_next();
    void text_find_prev();
    void text_find_goto_hit(int index);
    void text_find_apply_highlights();
    void text_find_set_highlight_all(bool enable);
    void init_speech_highlight();
    void setup_speech_highlight_for_reading(const QString &text);
    void on_timestamps_received(const QVariantList &timestamps);
    void start_speech_highlight();
    void pause_speech_highlight();
    void resume_speech_highlight();
    void stop_speech_highlight();
    void toggle_speech_highlight();
    void set_speech_highlight_mode(const QString &mode);
    void set_speech_highlight_style(const QColor &background = QColor(),
                                    const QColor &foreground = QColor(),
                                    bool bold = false,
                                    bool underline = false);
    void set_speech_highlight_auto_scroll(bool enabled);
    void on_toggle_leitura_assistida();
    void on_modo_leitura_changed(int index);
    void update_leitura_assistida_button();
    void update_regua_foco_button();
    void init_leitura_assistida_state();
    void register_generated_audio_timestamps(const QString &audio_path, const QVariantList &timestamps);
    void activate_timestamps_for_audio(const QString &audio_path);
    void notify_chunk_changed(int chunk_index, int base_offset_ms = -1);
    void notify_pdf_chunk_changed(int chunk_index, int base_offset_ms = -1);
    void init_pdf_speech_highlight();
    QString setup_pdf_speech_highlight_for_reading(const QString &pdf_path,
                                                   const QString &text,
                                                   int page_index = -1);
    void on_pdf_timestamps_received(const QVariantList &timestamps);
    void start_pdf_speech_highlight();
    void pause_pdf_speech_highlight();
    void resume_pdf_speech_highlight();
    void stop_pdf_speech_highlight();
    void toggle_pdf_speech_highlight();
    void set_pdf_speech_highlight_mode(const QString &mode);
    void cleanup_paths_with_retry(const QStringList &paths, int retries = 6, int delay_ms = 250);
    void cleanup_edge_tts_temp_in_outdir(bool remove_mp3 = true, bool remove_part = true);
    void cleanup_edge_tts_parts_in_outdir();

private:
    friend class FileDropEdit;
    friend class FileDropPDFView;
    friend class PDFMouseHandler;

    QHBoxLayout *setup_pdf_toolbar();
    void pdf_next_page();
    void pdf_prev_page();
    void pdf_goto_page(int page_num);
    void pdf_zoom_in();
    void pdf_zoom_out();
    void pdf_zoom_fit_width();
    void pdf_zoom_fit_page();
    void pdf_set_hand_mode();
    void pdf_set_selection_mode();
    void pdf_enable_toolbar(bool enable = true);
    void pdf_refresh_zoom_ui();
    void pdf_set_zoom_value(const QString &text);
    QString selected_or_full_text() const;
    QString current_pdf_text(int page_index = -1) const;
    QString reflow_pdf_text(const QString &text) const;
    void sync_text_from_pdf();
    void apply_text_extra_selections();
    QString selected_bullet_symbol() const;
    void replace_current_line_prefixes(bool add_bullets);
    bool speech_highlight_enabled() const;
    bool pdf_speech_highlight_enabled() const;

    QPointer<QThread> tts_thread_;
    QPointer<QMediaPlayer> player_;
    QPointer<QAudioOutput> audio_output_;
    QPointer<FocusRuler> regua_;
    QPointer<PDFMouseHandler> pdf_mouse_handler_;

    QString voz_id_atual_;
    bool usar_edge_tts_ = true;
    bool is_paused_ = false;
    bool speech_enabled_ = false;
    bool pdf_speech_enabled_ = false;
    bool speech_auto_scroll_ = true;
    bool highlight_all_text_hits_ = true;
    bool highlight_all_pdf_hits_ = true;
    QString speech_mode_ = QStringLiteral("word");
    QString pdf_speech_mode_ = QStringLiteral("word");

    QStringList generated_queue_;
    QString current_generated_;
    QStringList generated_files_;
    QHash<QString, QVariantList> generated_timestamps_;
    QString last_pdf_path_;
    QString current_reading_pdf_path_;
    bool current_reading_is_pdf_ = false;
    int current_reading_speed_wpm_ = 150;
    int current_chunk_index_ = 0;
    bool pending_pdf_text_refresh_ = false;

    QTimer speech_timer_;
    QVariantList current_timestamps_;
    QString current_reading_text_;
    int current_timestamp_index_ = -1;
    int last_speech_word_offset_ = -1;
    int last_speech_highlight_start_ = -1;
    int current_speech_highlight_start_ = -1;
    int current_speech_highlight_length_ = 0;
    bool speech_highlight_force_refresh_ = false;
    QString last_speech_render_mode_;

    QVector<QTextCursor> text_hits_;
    int current_text_hit_ = -1;

    QPushButton *btn_carregar_ = nullptr;
    QPushButton *btn_play_ = nullptr;
    QPushButton *btn_pause_ = nullptr;
    QPushButton *btn_stop_ = nullptr;
    QPushButton *btn_limpar_ = nullptr;
    QPushButton *btn_new_ = nullptr;
    QPushButton *btn_save_ = nullptr;
    QPushButton *btn_undo_ = nullptr;
    QPushButton *btn_redo_ = nullptr;
    QPushButton *btn_find_text_ = nullptr;
    QPushButton *btn_font_settings_ = nullptr;
    QPushButton *btn_color_tasks_ = nullptr;
    QPushButton *btn_reset_settings_ = nullptr;
    QTimer save_text_timer_;
    QPushButton *btn_bullets_ = nullptr;
    QPushButton *btn_regua_ = nullptr;
    QPushButton *btn_leitura_assistida_ = nullptr;
    QPushButton *btn_pdf_prev_ = nullptr;
    QPushButton *btn_pdf_next_ = nullptr;
    QPushButton *btn_pdf_zoom_in_ = nullptr;
    QPushButton *btn_pdf_zoom_out_ = nullptr;
    QPushButton *btn_pdf_fit_width_ = nullptr;
    QPushButton *btn_pdf_fit_page_ = nullptr;
    QPushButton *btn_pdf_hand_ = nullptr;
    QPushButton *btn_pdf_select_ = nullptr;
    QPushButton *btn_pdf_find_ = nullptr;

    QLabel *label_velocidade_titulo_ = nullptr;
    QLabel *label_velocidade_ = nullptr;
    QLabel *label_volume_ = nullptr;
    QLabel *label_fonte_ = nullptr;
    QLabel *dica_regua_ = nullptr;
    QLabel *label_modo_leitura_ = nullptr;
    QLabel *dica_leitura_assistida_ = nullptr;
    QLabel *label_bullet_style_ = nullptr;
    QLabel *label_spacing_ = nullptr;
    QLabel *label_indent_ = nullptr;
    QLabel *label_margin_ = nullptr;
    QLabel *label_pdf_page_ = nullptr;
    QLabel *label_pdf_zoom_ = nullptr;
    QLabel *label_pdf_info_ = nullptr;

    QSlider *slider_velocidade_ = nullptr;
    QSlider *slider_volume_ = nullptr;
    QComboBox *combo_fonte_ = nullptr;
    QComboBox *combo_bullet_style_ = nullptr;
    QComboBox *combo_spacing_ = nullptr;
    QComboBox *combo_modo_leitura_ = nullptr;
    QComboBox *combo_pdf_zoom_ = nullptr;
    QSpinBox *spin_indent_ = nullptr;
    QSpinBox *spin_margin_ = nullptr;
    QSpinBox *spin_page_ = nullptr;

    QTabWidget *content_stack_ = nullptr;
    FileDropEdit *texto_area_ = nullptr;
    FileDropPDFView *pdf_view_ = nullptr;
    TextFindBar *text_find_bar_ = nullptr;
    PDFFindBar *pdf_find_bar_ = nullptr;
    QShortcut *sc_find_toggle_ = nullptr;
    QShortcut *sc_copy_pdf_ = nullptr;
};

}  // namespace source::plugin::Audiolivro
