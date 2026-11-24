#pragma once

#include <QColor>
#include <Qt>
#include <QLocale>
#include <QPair>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

class QAbstractScrollArea;
class QGroupBox;
class QLabel;
class QListWidgetItem;
class QLayout;
class QLayoutItem;
class QTextEdit;
class QWidget;

namespace source::GerenciamentoUI {
class RotatedTabButton;
}

namespace source::plugin::Feynman {

class MetodoFeynman;

constexpr int kDefaultDomain = 0;
constexpr int kDefaultPriority = 1;
constexpr int kConceptRole = Qt::UserRole;
constexpr int kPreviewRole = Qt::UserRole + 1;
constexpr int kGroupHeaderRole = Qt::UserRole + 2;
constexpr int kGroupKeyRole = Qt::UserRole + 3;
constexpr int kFeynmanSplitterExpandedHandleWidth = 6;
constexpr int kFeynmanSplitterCollapsedHandleWidth = 0;
constexpr int kFeynmanDefaultLeftPanelWidth = 360;
constexpr int kFeynmanLeftPanelCompactWidth = 220;
constexpr int kFeynmanTabTextPadding = 32;

struct PanelMinimums {
    int left = 0;
    int right = 0;
};

struct DescriptionParts {
    QString concept_text;
    QString explanation;
    QString gaps;
    QString review;
    bool structured = false;
};

QString first_n_lines(const QString &text, int max_lines = 5);
bool is_concept_completed(const QVariantMap &conceito);
bool is_concept_in_progress(const QVariantMap &conceito);
bool is_concept_pending(const QVariantMap &conceito);
QString expand_task_panel_text(const QWidget *context);
QString collapse_task_panel_text(const QWidget *context);
int rotated_button_width(const source::GerenciamentoUI::RotatedTabButton *button);
QString normalized_ui_layout_key(const QString &value);
bool is_horizontal_ui_layout(const QString &value);
bool is_vertical_ui_layout(const QString &value);
void delete_layout_item(QLayoutItem *item);
void clear_layout(QLayout *layout);
void configure_compact_label(QLabel *label);
void configure_compact_scroll_area(QAbstractScrollArea *widget);
PanelMinimums effective_panel_minimums(int total_width,
                                       int left_base,
                                       int right_base,
                                       bool left_visible);
QGroupBox *create_report_frame(QWidget *parent, QLabel *label, QTextEdit *editor);
QTextEdit *create_placeholder_text_edit(QWidget *parent);
void set_placeholder_base_text(QTextEdit *editor, const QString &text);
QSize available_window_size(const QWidget *widget);
int normalized_domain_value(const QVariant &value);
int normalized_priority_value(const QVariant &value);
QString domain_symbol(int dominio);
QString priority_emoji(int prioridade);
QString priority_label(const QWidget *context, int prioridade);
QString priority_text_without_emoji(const QWidget *context, int prioridade);
QString domain_label(const QWidget *context, int dominio, bool short_form = false);
QVariantMap normalized_concept(const QVariantMap &conceito);
QVariantList normalized_concepts(const QVariantList &concepts);
QVariantMap preview_payload(const QWidget *context, const QVariantMap &conceito);
QVariantMap sort_payload_from_concept(const QVariantMap &conceito);
bool has_host_task_sort();
bool host_task_sort_less(const QVariantMap &left, const QVariantMap &right);
DescriptionParts parse_description_text(const QString &text);
QString composed_description(const QWidget *context,
                             const QString &title,
                             const QString &explanation,
                             const QString &gaps,
                             const QString &review);
QString merge_description_edit(const QString &previous_generated_text,
                               const QString &edited_text,
                               const QString &next_generated_text);
QString manual_description_text(const QString &display_text, const QString &generated_text);
QString merged_description_text(const QString &generated_text, const QString &manual_text);
bool has_external_description_base(const QVariantMap &conceito);
QString description_with_external_base(const QString &base_text, const QString &generated_text);
QString concept_description_text(const QWidget *context, const QVariantMap &conceito);
QString shared_task_description(const QWidget *context, const QVariantMap &conceito);
QStringList integration_metadata_keys();
QString tooltip_html(const QWidget *context, const QVariantMap &conceito);
QString human_date_for_display(const QWidget *context, const QString &date_iso);
QString visible_concept_title(const QWidget *context, const QVariantMap &conceito, const QString &base_text);
QString watermark_concept_text(const QWidget *context, const QVariantMap &conceito);
QString concept_group_key(const QVariantMap &conceito);
bool is_group_header(QListWidgetItem *item);
void apply_window_as_base(QWidget *widget);
void sync_backgrounds(MetodoFeynman &self);
QString action_text_with_path(const QString &label, const QString &path);
QLocale effective_locale(const QWidget *context);
QString effective_date_format(const QWidget *context);

}  // namespace source::plugin::Feynman
