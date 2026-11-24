#include "source/services/RecursosProdutividade/recpro_07_filtros.hpp"
#include "source/services/RecursosProdutividade/recpro_01_utilitarios_compartilhados.hpp"
#include "source/app/app_02_get_text.hpp"
#include "source/app/app_01_eisenhower_matrix_app.hpp"
#include "source/services/incore_13_prioridade_display.hpp"

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSet>
#include <QStringList>
#include <QVBoxLayout>
#include <algorithm>

namespace source::InterfaceCore::ProductivityFeatures {
namespace {

// ---------------------------------------------------------------------------
// Persistent filter state: keys are "category::value" (all lowercase)
// ---------------------------------------------------------------------------
static QSet<QString> s_unchecked_filters;

QString filter_key(const QString &category, const QString &value)
{
    return QStringLiteral("%1::%2").arg(category.trimmed().toCaseFolded(),
                                        value.trimmed().toCaseFolded());
}

// ---------------------------------------------------------------------------
// Helper to check if an item is a watermark or group header across all modules
// (Eisenhower/Pomodoro use Qt::UserRole+1, Feynman uses Qt::UserRole+2)
// ---------------------------------------------------------------------------
bool is_group_header_item(const QListWidgetItem *item)
{
    if (item == nullptr) {
        return false;
    }
    for (int role : {static_cast<int>(Qt::UserRole), static_cast<int>(Qt::UserRole) + 1, static_cast<int>(Qt::UserRole) + 2, static_cast<int>(Qt::UserRole) + 3}) {
        if (item->data(role).toString() == QStringLiteral("group_header")) {
            return true;
        }
    }
    return false;
}

bool is_valid_task_item(QListWidgetItem *item)
{
    if (item == nullptr) {
        return false;
    }
    if (is_group_header_item(item)) {
        return false;
    }
    if (!(item->flags() & Qt::ItemIsSelectable)) {
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Structured representation of a parsed task property for filtering
// ---------------------------------------------------------------------------
struct ParsedCategoryValue {
    QString category_key;   // e.g. "prioridade", "curso", "disciplina"
    QString internal_val;   // language-independent key (e.g. "1" for priority 1)
    QString display_val;    // dynamically translated text to display in UI
};

struct ParsedTaskInfo {
    bool is_agenda = false;
    QString title;
    QString description;
    QList<ParsedCategoryValue> category_values;  // agenda fields
    QStringList title_words;                      // non-agenda title words
};

// ---------------------------------------------------------------------------
// Known category labels -> canonical internal category keys
// ---------------------------------------------------------------------------
struct KnownCategory {
    QString label_pattern;   // substring to match (case-insensitive)
    QString category_key;
};

static const QList<KnownCategory> &known_categories()
{
    static const QList<KnownCategory> cats = {
        {QStringLiteral("curso"),                       QStringLiteral("curso")},
        {QStringLiteral("ementa"),                      QStringLiteral("ementa")},
        {QStringLiteral("semestre"),                    QStringLiteral("semestre")},
        {QStringLiteral("disciplina"),                  QStringLiteral("disciplina")},
        {QStringLiteral("turma"),                       QStringLiteral("turma")},
        {QStringLiteral("tipo de atividade"),           QStringLiteral("tipo")},
        {QStringLiteral("sequência"),                   QStringLiteral("sequencia")},
        {QStringLiteral("sequence"),                    QStringLiteral("sequencia")},
        {QStringLiteral("agenda"),                      QStringLiteral("agenda")},
        {QStringLiteral("course"),                      QStringLiteral("curso")},
        {QStringLiteral("subject"),                     QStringLiteral("disciplina")},
        {QStringLiteral("semester"),                    QStringLiteral("semestre")},
        {QStringLiteral("syllabus"),                    QStringLiteral("ementa")},
        {QStringLiteral("class"),                       QStringLiteral("turma")},
        {QStringLiteral("activity type"),               QStringLiteral("tipo")},
    };
    return cats;
}

QString classify_category_key(const QString &label)
{
    const QString lower = label.trimmed().toCaseFolded();
    for (const KnownCategory &cat : known_categories()) {
        if (lower.contains(cat.label_pattern)) {
            return cat.category_key;
        }
    }
    return QString(); // Restrict filters strictly to known agenda categories
}

// ---------------------------------------------------------------------------
// Text helpers
// ---------------------------------------------------------------------------
QString strip_emojis_and_formatting(QString text)
{
    text.remove(QStringLiteral("🔴"));
    text.remove(QStringLiteral("🟠"));
    text.remove(QStringLiteral("🟡"));
    text.remove(QStringLiteral("🟢"));
    text.remove(QStringLiteral("🥇"));
    text.remove(QStringLiteral("🥈"));
    text.remove(QStringLiteral("🥉"));
    text.remove(QStringLiteral("🍅"));
    return text.trimmed();
}

// ---------------------------------------------------------------------------
// Helpers to extract Priority and Domain across all plugins and methods
// ---------------------------------------------------------------------------
static int extract_priority(QListWidgetItem *item, const QVariantMap &data)
{
    // 1. Try QVariantMap data
    QVariant prio_var = data.value(QStringLiteral("prioridade"));
    if (!prio_var.isValid()) {
        prio_var = data.value(QStringLiteral("priority"));
    }
    
    if (prio_var.isValid()) {
        bool ok = false;
        const int p_int = prio_var.toInt(&ok);
        if (ok && p_int >= 1 && p_int <= 4) {
            return p_int;
        }
        
        const QString p_str = prio_var.toString().trimmed().toLower();
        if (p_str.contains(QStringLiteral("importante")) && p_str.contains(QStringLiteral("urgente"))) {
            if (p_str.contains(QStringLiteral("não urgente")) || p_str.contains(QStringLiteral("nao urgente"))) return 2;
            if (p_str.contains(QStringLiteral("não importante")) || p_str.contains(QStringLiteral("nao importante"))) return 3;
            if (!p_str.contains(QStringLiteral("não")) && !p_str.contains(QStringLiteral("nao"))) return 1;
            return 4;
        }
    }
    
    const QString titulo_prio = data.value(QStringLiteral("titulo_prioridade")).toString().trimmed().toLower();
    if (titulo_prio.contains(QStringLiteral("importante")) && titulo_prio.contains(QStringLiteral("urgente"))) {
        if (titulo_prio.contains(QStringLiteral("não urgente")) || titulo_prio.contains(QStringLiteral("nao urgente"))) return 2;
        if (titulo_prio.contains(QStringLiteral("não importante")) || titulo_prio.contains(QStringLiteral("nao importante"))) return 3;
        if (!titulo_prio.contains(QStringLiteral("não")) && !titulo_prio.contains(QStringLiteral("nao"))) return 1;
        return 4;
    }
    
    // 2. Try Tooltip
    if (item != nullptr) {
        const QStringList lines = item->toolTip().split(QLatin1Char('\n'));
        for (const QString &line : lines) {
            const int colon = line.indexOf(QLatin1Char(':'));
            if (colon > 0) {
                const QString label = line.left(colon).trimmed().toLower();
                if (label == QStringLiteral("prioridade") || label == QStringLiteral("priority")) {
                    const QString val = line.mid(colon + 1).trimmed().toLower();
                    if ((val.contains(QStringLiteral("importante")) || val.contains(QStringLiteral("important"))) &&
                        (val.contains(QStringLiteral("urgente")) || val.contains(QStringLiteral("urgent")))) {
                        
                        if (val.contains(QStringLiteral("não urgente")) || val.contains(QStringLiteral("nao urgente")) || val.contains(QStringLiteral("not urgent"))) return 2;
                        if (val.contains(QStringLiteral("não importante")) || val.contains(QStringLiteral("nao importante")) || val.contains(QStringLiteral("not important"))) return 3;
                        if (!val.contains(QStringLiteral("não")) && !val.contains(QStringLiteral("nao")) && !val.contains(QStringLiteral("not"))) return 1;
                        return 4;
                    }
                }
            }
        }
    }
    
    // 3. Try Eisenhower List Widget Pointers
    if (item != nullptr && item->listWidget() != nullptr) {
        if (QWidget *win = item->listWidget()->window()) {
            if (auto *app = qobject_cast<source::EisenhowerMatrixApp *>(win)) {
                if (item->listWidget() == app->quadrant1_list || item->listWidget() == app->quadrant1_in_progress_list || item->listWidget() == app->quadrant1_completed_list) return 1;
                if (item->listWidget() == app->quadrant2_list || item->listWidget() == app->quadrant2_in_progress_list || item->listWidget() == app->quadrant2_completed_list) return 2;
                if (item->listWidget() == app->quadrant3_list || item->listWidget() == app->quadrant3_in_progress_list || item->listWidget() == app->quadrant3_completed_list) return 3;
                if (item->listWidget() == app->quadrant4_list || item->listWidget() == app->quadrant4_in_progress_list || item->listWidget() == app->quadrant4_completed_list) return 4;
            }
        }
    }
    
    return -1;
}

static int extract_domain(QListWidgetItem *item, const QVariantMap &data)
{
    // 1. Try QVariantMap data
    QVariant dom_var = data.value(QStringLiteral("dominio"));
    if (!dom_var.isValid()) {
        dom_var = data.value(QStringLiteral("domain"));
    }
    if (dom_var.isValid()) {
        bool ok = false;
        const int d_int = dom_var.toInt(&ok);
        if (ok && d_int >= 0 && d_int <= 2) {
            return d_int;
        }
        
        const QString d_str = dom_var.toString().trimmed().toLower();
        if (d_str.contains(QStringLiteral("iniciante"))) return 0;
        if (d_str.contains(QStringLiteral("intermediário")) || d_str.contains(QStringLiteral("intermediario"))) return 1;
        if (d_str.contains(QStringLiteral("avançado")) || d_str.contains(QStringLiteral("avancado"))) return 2;
    }
    
    const QString titulo_dom = data.value(QStringLiteral("titulo_dominio")).toString().trimmed().toLower();
    if (titulo_dom.contains(QStringLiteral("iniciante"))) return 0;
    if (titulo_dom.contains(QStringLiteral("intermediário")) || titulo_dom.contains(QStringLiteral("intermediario"))) return 1;
    if (titulo_dom.contains(QStringLiteral("avançado")) || titulo_dom.contains(QStringLiteral("avancado"))) return 2;
    
    // 2. Try Tooltip
    if (item != nullptr) {
        const QStringList lines = item->toolTip().split(QLatin1Char('\n'));
        for (const QString &line : lines) {
            const int colon = line.indexOf(QLatin1Char(':'));
            if (colon > 0) {
                const QString label = line.left(colon).trimmed().toLower();
                if (label == QStringLiteral("nível de domínio") || label == QStringLiteral("nivel de dominio") || label == QStringLiteral("mastery level") || label == QStringLiteral("domain level") || label == QStringLiteral("domain")) {
                    const QString val = line.mid(colon + 1).trimmed().toLower();
                    if (val.contains(QStringLiteral("iniciante")) || val.contains(QStringLiteral("beginner"))) return 0;
                    if (val.contains(QStringLiteral("intermediário")) || val.contains(QStringLiteral("intermediario")) || val.contains(QStringLiteral("intermediate"))) return 1;
                    if (val.contains(QStringLiteral("avançado")) || val.contains(QStringLiteral("avancado")) || val.contains(QStringLiteral("advanced"))) return 2;
                }
            }
        }
    }
    
    return -1;
}

static QString normalize_extracted_date(const QString &date_str)
{
    if (date_str.isEmpty()) return date_str;
    QDate d = QDate::fromString(date_str, Qt::ISODate);
    if (d.isValid()) return date_str;

    d = QDate::fromString(date_str, QStringLiteral("dd/MM/yyyy"));
    if (d.isValid()) return d.toString(Qt::ISODate);

    d = QDate::fromString(date_str, QStringLiteral("MM/dd/yyyy"));
    if (d.isValid()) return d.toString(Qt::ISODate);

    d = QLocale::system().toDate(date_str, QLocale::ShortFormat);
    if (d.isValid()) return d.toString(Qt::ISODate);

    return date_str;
}

static QString normalize_extracted_time(const QString &time_str)
{
    if (time_str.isEmpty()) return time_str;
    QTime t = QTime::fromString(time_str, QStringLiteral("HH:mm"));
    if (t.isValid()) return time_str;

    t = QTime::fromString(time_str, QStringLiteral("HH:mm:ss"));
    if (t.isValid()) return t.toString(QStringLiteral("HH:mm"));

    t = QLocale::system().toTime(time_str, QLocale::ShortFormat);
    if (t.isValid()) return t.toString(QStringLiteral("HH:mm"));

    return time_str;
}

static QString extract_date(QListWidgetItem *item, const QVariantMap &data)
{
    const QString date_str = data.value(QStringLiteral("date")).toString().trimmed();
    if (!date_str.isEmpty()) return normalize_extracted_date(date_str);
    
    const QString data_str = data.value(QStringLiteral("data")).toString().trimmed();
    if (!data_str.isEmpty()) return normalize_extracted_date(data_str);
    
    if (item != nullptr) {
        const QStringList lines = item->toolTip().split(QLatin1Char('\n'));
        for (const QString &line : lines) {
            const int colon = line.indexOf(QLatin1Char(':'));
            if (colon > 0) {
                const QString label = line.left(colon).trimmed().toLower();
                if (label == QStringLiteral("data") || label == QStringLiteral("date")) {
                    return normalize_extracted_date(line.mid(colon + 1).trimmed());
                }
            }
        }
    }
    return QString();
}

static QString extract_time(QListWidgetItem *item, const QVariantMap &data)
{
    const QString time_str = data.value(QStringLiteral("time")).toString().trimmed();
    if (!time_str.isEmpty()) return normalize_extracted_time(time_str);
    
    const QString horario_str = data.value(QStringLiteral("horario")).toString().trimmed();
    if (!horario_str.isEmpty()) return normalize_extracted_time(horario_str);
    
    if (item != nullptr) {
        const QStringList lines = item->toolTip().split(QLatin1Char('\n'));
        for (const QString &line : lines) {
            const int colon = line.indexOf(QLatin1Char(':'));
            if (colon > 0) {
                const QString label = line.left(colon).trimmed().toLower();
                if (label == QStringLiteral("horário") || label == QStringLiteral("horario") || label == QStringLiteral("time")) {
                    return normalize_extracted_time(line.mid(colon + 1).trimmed());
                }
            }
        }
    }
    return QString();
}

// ---------------------------------------------------------------------------
// Parse a single task item into structured info
// ---------------------------------------------------------------------------
ParsedTaskInfo parse_task_item_info(QListWidgetItem *item)
{
    ParsedTaskInfo info;
    if (item == nullptr) {
        return info;
    }

    const QVariantMap data = item->data(Qt::UserRole).toMap();
    const QString origin = data.value(QStringLiteral("origin")).toString().trimmed().toLower();
    const QString integracao_origem = data.value(QStringLiteral("integracao_origem")).toString().trimmed().toLower();

    info.title = data.value(QStringLiteral("text")).toString().trimmed();
    if (info.title.isEmpty()) {
        info.title = data.value(QStringLiteral("titulo")).toString().trimmed();
    }
    if (info.title.isEmpty()) {
        info.title = item->text().split(QLatin1Char('\n')).value(0).trimmed();
    }
    info.title = strip_emojis_and_formatting(info.title);

    info.description = data.value(QStringLiteral("description")).toString().trimmed();
    if (info.description.isEmpty()) {
        info.description = data.value(QStringLiteral("descricao")).toString().trimmed();
    }
    if (info.description.isEmpty()) {
        info.description = item->toolTip().trimmed();
    }

    const bool has_agenda_origin = (origin == QStringLiteral("agenda") || integracao_origem == QStringLiteral("agenda"));
    const bool has_agenda_labels = info.description.contains(QStringLiteral("Curso:"))
        || info.description.contains(QStringLiteral("Course:"))
        || info.description.contains(QStringLiteral("Disciplina:"))
        || info.description.contains(QStringLiteral("Subject:"))
        || info.description.contains(QStringLiteral("Tipo de Atividade"))
        || info.description.contains(QStringLiteral("Activity Type"))
        || info.description.contains(QStringLiteral("Turma"));

    info.is_agenda = (has_agenda_origin || has_agenda_labels);

    if (info.is_agenda) {
        // Extract field values from description lines (Key: Value)
        const QStringList lines = info.description.split(QLatin1Char('\n'));
        for (const QString &line : lines) {
            const int colon = line.indexOf(QLatin1Char(':'));
            if (colon > 0 && colon + 1 < line.length()) {
                const QString label = line.left(colon).trimmed();
                const QString val = line.mid(colon + 1).trimmed();

                if (!val.isEmpty() && !label.isEmpty()) {
                    const QString cat_key = classify_category_key(label).toLower();
                    if (cat_key.isEmpty()) {
                        continue; // Strictly ignore any free text or unknown categories
                    }
                    if (cat_key == QStringLiteral("prioridade") ||
                        cat_key == QStringLiteral("priority") ||
                        cat_key == QStringLiteral("dominio") ||
                        cat_key == QStringLiteral("domínio") ||
                        cat_key == QStringLiteral("domain") ||
                        cat_key == QStringLiteral("data") ||
                        cat_key == QStringLiteral("date") ||
                        cat_key == QStringLiteral("horário") ||
                        cat_key == QStringLiteral("horario") ||
                        cat_key == QStringLiteral("time")) {
                        continue;
                    }
                    info.category_values.append({cat_key, val, val});
                }
            }
        }
    } else {
        // Non-agenda task: use the full task title instead of individual words
        if (!info.title.isEmpty()) {
            info.category_values.append({QStringLiteral("palavra"), info.title, info.title});
        }
    }

    // Extract Prioridade universally
    const int priority = extract_priority(item, data);
    if (priority >= 1 && priority <= 4) {
        info.category_values.append({QStringLiteral("prioridade"),
                                     QString::number(priority),
                                     source::InterfaceCore::prioridade_para_texto(priority)});
    }

    // Extract Nível de Domínio universally
    const int domain = extract_domain(item, data);
    if (domain >= 0 && domain <= 2) {
        info.category_values.append({QStringLiteral("dominio"),
                                     QString::number(domain),
                                     source::InterfaceCore::dominio_para_texto(domain, false)});
    }
    
    // Extract Data universally
    const QString date_val = extract_date(item, data);
    if (!date_val.isEmpty()) {
        info.category_values.append({QStringLiteral("data"), date_val, date_val});
    }

    // Extract Horário universally
    const QString time_val = extract_time(item, data);
    if (!time_val.isEmpty()) {
        info.category_values.append({QStringLiteral("horário"), time_val, time_val});
    }

    return info;
}

// ---------------------------------------------------------------------------
// Collect all list widgets from Eisenhower + Pomodoro + Feynman
// ---------------------------------------------------------------------------
QList<QListWidget *> collect_all_app_list_widgets(source::EisenhowerMatrixApp *app)
{
    QList<QListWidget *> lists;
    if (app == nullptr) {
        return lists;
    }

    for (QListWidget *w : source::app::all_task_lists(app)) {
        if (w != nullptr && !lists.contains(w)) {
            lists.append(w);
        }
    }

    if (app->pomodoro_module_page != nullptr) {
        for (QListWidget *w : app->pomodoro_module_page->findChildren<QListWidget *>()) {
            if (w != nullptr && !lists.contains(w)) {
                lists.append(w);
            }
        }
    }

    if (app->feynman_module_page != nullptr) {
        for (QListWidget *w : app->feynman_module_page->findChildren<QListWidget *>()) {
            if (w != nullptr && !lists.contains(w)) {
                lists.append(w);
            }
        }
    }

    return lists;
}

// ---------------------------------------------------------------------------
// Filter collection structures
// ---------------------------------------------------------------------------
struct FilterItem {
    QString category_key;
    QString internal_value;
    QString display_value;
};

struct FilterCollection {
    QList<FilterItem> agenda_filters;
    QStringList title_filters;
};

FilterCollection collect_available_filters(source::EisenhowerMatrixApp *app)
{
    FilterCollection result;
    QHash<QString, FilterItem> agenda_map; // key: filter_key(category_key, internal_value)
    QSet<QString> word_set;
    QStringList agenda_order;

    const QList<QListWidget *> lists = collect_all_app_list_widgets(app);
    for (QListWidget *list : lists) {
        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem *item = list->item(i);
            if (!is_valid_task_item(item)) {
                continue;
            }

            ParsedTaskInfo info = parse_task_item_info(item);
            for (const ParsedCategoryValue &cv : info.category_values) {
                if (cv.internal_val.isEmpty()) {
                    continue;
                }
                if (cv.category_key == QStringLiteral("palavra")) {
                    word_set.insert(cv.internal_val);
                } else {
                    const QString k = filter_key(cv.category_key, cv.internal_val);
                    if (!agenda_map.contains(k)) {
                        agenda_order.append(k);
                        agenda_map.insert(k, {cv.category_key, cv.internal_val, cv.display_val});
                    }
                }
            }
        }
    }

    for (const QString &k : agenda_order) {
        result.agenda_filters.append(agenda_map.value(k));
    }

    // Sort agenda filters cleanly by display value
    std::sort(result.agenda_filters.begin(), result.agenda_filters.end(), [](const FilterItem &a, const FilterItem &b) {
        return QString::localeAwareCompare(a.display_value, b.display_value) < 0;
    });

    result.title_filters = word_set.values();
    std::sort(result.title_filters.begin(), result.title_filters.end(), [](const QString &a, const QString &b) {
        return QString::localeAwareCompare(a, b) < 0;
    });

    return result;
}

// ---------------------------------------------------------------------------
// Matching logic: does a task pass the active filters?
// ---------------------------------------------------------------------------
bool task_matches_active_filters(QListWidgetItem *item, const QSet<QString> &unchecked)
{
    if (unchecked.isEmpty()) {
        return true;
    }

    ParsedTaskInfo info = parse_task_item_info(item);

    if (info.category_values.isEmpty()) {
        return true;
    }
    for (const ParsedCategoryValue &cv : info.category_values) {
        const QString key = filter_key(cv.category_key, cv.internal_val);
        if (!unchecked.contains(key)) {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Update group header / watermark visibility across lists
// ---------------------------------------------------------------------------
void update_group_headers_visibility(QListWidget *list)
{
    if (list == nullptr) {
        return;
    }

    for (int i = 0; i < list->count(); ++i) {
        QListWidgetItem *item = list->item(i);
        if (item != nullptr && is_group_header_item(item)) {
            bool has_visible_child = false;
            for (int j = i + 1; j < list->count(); ++j) {
                QListWidgetItem *next = list->item(j);
                if (next == nullptr || is_group_header_item(next)) {
                    break;
                }
                if (is_valid_task_item(next) && !next->isHidden()) {
                    has_visible_child = true;
                    break;
                }
            }
            item->setHidden(!has_visible_child);
        }
    }
}

} // namespace

// ===========================================================================
// Public API
// ===========================================================================

void apply_active_filters(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }

    const QList<QListWidget *> lists = collect_all_app_list_widgets(app);
    for (QListWidget *list : lists) {
        const QSignalBlocker blocker(list);
        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem *item = list->item(i);
            if (!is_valid_task_item(item)) {
                continue;
            }

            const bool visible = task_matches_active_filters(item, s_unchecked_filters);
            item->setHidden(!visible);
        }
        update_group_headers_visibility(list);
    }
}

void show_filters_dialog(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }

    FilterCollection filters = collect_available_filters(app);

    QDialog dialog(app);
    dialog.setWindowTitle(get_text(QStringLiteral("Filtros")));
    dialog.resize(500, 580);

    auto *main_layout = new QVBoxLayout(&dialog);
    main_layout->setSpacing(10);
    main_layout->setContentsMargins(16, 16, 16, 16);

    auto *header_label = new QLabel(get_text(QStringLiteral("Selecione os filtros que deseja exibir na interface:")), &dialog);
    header_label->setWordWrap(true);
    QFont font = header_label->font();
    font.setBold(true);
    header_label->setFont(font);
    main_layout->addWidget(header_label);

    auto *search_edit = new QLineEdit(&dialog);
    search_edit->setPlaceholderText(get_text(QStringLiteral("Pesquisar filtros...")));
    search_edit->setClearButtonEnabled(true);
    main_layout->addWidget(search_edit);

    auto *select_all_cb = new QCheckBox(get_text(QStringLiteral("Selecionar Tudo")), &dialog);
    select_all_cb->setTristate(false);
    select_all_cb->setChecked(s_unchecked_filters.isEmpty());
    main_layout->addWidget(select_all_cb);

    auto *filter_list_widget = new QListWidget(&dialog);
    filter_list_widget->setSelectionMode(QAbstractItemView::NoSelection);
    main_layout->addWidget(filter_list_widget);

    // Populate function - displays only filter items directly without grouping headers or labels
    const auto populate_list = [&](const QString &search_filter) {
        filter_list_widget->clear();
        const QString query = search_filter.trimmed().toLower();
        QSet<QString> added_keys;

        const auto add_filter_option = [&](const QString &cat_key, const QString &internal_val, const QString &display_val) {
            if (internal_val.isEmpty() || display_val.isEmpty()) {
                return;
            }
            if (!query.isEmpty() && !display_val.toLower().contains(query)) {
                return;
            }
            const QString key = filter_key(cat_key, internal_val);
            if (added_keys.contains(key)) {
                return;
            }
            added_keys.insert(key);

            auto *item = new QListWidgetItem(display_val, filter_list_widget);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            item->setData(Qt::UserRole, cat_key);
            item->setData(Qt::UserRole + 2, internal_val);
            const bool checked = !s_unchecked_filters.contains(key);
            item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
        };

        for (const FilterItem &fi : filters.agenda_filters) {
            add_filter_option(fi.category_key, fi.internal_value, fi.display_value);
        }

        for (const QString &word : filters.title_filters) {
            add_filter_option(QStringLiteral("palavra"), word, word);
        }

        if (filter_list_widget->count() == 0) {
            auto *empty_item = new QListWidgetItem(get_text(QStringLiteral("Nenhum filtro disponível.")), filter_list_widget);
            empty_item->setFlags(Qt::NoItemFlags);
        }
    };

    populate_list(QString());

    auto *status_label = new QLabel(&dialog);
    status_label->setStyleSheet(QStringLiteral("color: #777777; font-size: 11px;"));
    main_layout->addWidget(status_label);

    const auto update_status = [&]() {
        int total_tasks = 0;
        int visible_tasks = 0;
        for (QListWidget *list : collect_all_app_list_widgets(app)) {
            for (int i = 0; i < list->count(); ++i) {
                QListWidgetItem *item = list->item(i);
                if (is_valid_task_item(item)) {
                    ++total_tasks;
                    if (!item->isHidden()) {
                        ++visible_tasks;
                    }
                }
            }
        }
        status_label->setText(get_text(QStringLiteral("%1 de %2 tarefas visíveis"))
                                  .arg(QString::number(visible_tasks), QString::number(total_tasks)));
    };

    update_status();

    QObject::connect(search_edit, &QLineEdit::textChanged, &dialog, [populate_list](const QString &text) {
        populate_list(text);
    });

    QObject::connect(filter_list_widget, &QListWidget::itemChanged, &dialog, [app, update_status](QListWidgetItem *item) {
        if (item == nullptr || !(item->flags() & Qt::ItemIsUserCheckable)) {
            return;
        }

        const QString cat_key = item->data(Qt::UserRole).toString();
        const QString value = item->data(Qt::UserRole + 2).toString();
        const QString key = filter_key(cat_key, value);

        if (item->checkState() == Qt::Unchecked) {
            s_unchecked_filters.insert(key);
        } else {
            s_unchecked_filters.remove(key);
        }

        apply_active_filters(app);
        update_status();
    });

    QObject::connect(select_all_cb, &QCheckBox::toggled, &dialog, [app, filter_list_widget, update_status](bool checked) {
        const QSignalBlocker blocker(filter_list_widget);
        for (int i = 0; i < filter_list_widget->count(); ++i) {
            QListWidgetItem *item = filter_list_widget->item(i);
            if (item != nullptr && (item->flags() & Qt::ItemIsUserCheckable)) {
                item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
                const QString cat_key = item->data(Qt::UserRole).toString();
                const QString value = item->data(Qt::UserRole + 2).toString();
                const QString key = filter_key(cat_key, value);
                if (checked) {
                    s_unchecked_filters.remove(key);
                } else {
                    s_unchecked_filters.insert(key);
                }
            }
        }
        apply_active_filters(app);
        update_status();
    });

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
    translate_standard_buttons(app, buttons);
    main_layout->addWidget(buttons);

    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);

    dialog.exec();
}

} // namespace source::InterfaceCore::ProductivityFeatures
