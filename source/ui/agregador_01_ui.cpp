#include "source/ui/agregador_01_ui.hpp"

#include <QAbstractButton>
#include <QBrush>
#include <QColor>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDateTime>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFont>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QSignalBlocker>
#include <QTime>
#include <QTimeEdit>
#include <QVariantMap>
#include <QVBoxLayout>
#include <QUuid>
#include <memory>
#include <vector>

#include "source/services/MostrarMenuContexto/mmc_09_base_text_from_item.hpp"
#include "source/services/incore_13_prioridade_display.hpp"
#include "source/eisenhower_app.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"
#include "source/utils/LinkedTitleUnderlineDelegate.hpp"
#include "source/utils/LogManager.hpp"
#include "source/utils/UnifiedTaskStore.hpp"
#include "source/app/app_02_get_text.hpp"
#include "source/modules/gantt/gantt_plugin_interface.hpp"
#include "source/utils/TaskProgressHelper.hpp"

namespace source::GerenciamentoUI {
namespace {

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

bool list_has_selectable_items(QListWidget *list)
{
    if (list == nullptr) {
        return false;
    }

    for (int i = 0; i < list->count(); ++i) {
        QListWidgetItem *item = list->item(i);
        if (item != nullptr && (item->flags() & Qt::ItemIsSelectable)) {
            return true;
        }
    }
    return false;
}

void apply_task_title_appearance(QListWidgetItem *item, const QVariantMap &data)
{
    if (item == nullptr) {
        return;
    }

    const QVariantMap font_config = data.value(QStringLiteral("font_config")).toMap();
    if (!font_config.isEmpty() && item->data(source::utils::kCustomFontSettingsRole).toMap().isEmpty()) {
        item->setData(source::utils::kCustomFontSettingsRole, font_config);
    }

    const bool has_file = !data.value(QStringLiteral("file_path")).toString().trimmed().isEmpty();
    source::utils::apply_task_title_font(item, data.value(QStringLiteral("text")).toString(), has_file);

    const QColor text_color(data.value(QStringLiteral("cor_texto")).toString().trimmed());
    if (text_color.isValid()) {
        item->setForeground(QBrush(text_color));
    } else {
        item->setForeground(QBrush());
    }
}

QString placeholder_text_for_list(source::EisenhowerMatrixApp *app, QListWidget *list)
{
    if (list == app->quadrant1_list) {
        return get_text(QStringLiteral("1º Quadrante"));
    }
    if (list == app->quadrant2_list) {
        return get_text(QStringLiteral("2º Quadrante"));
    }
    if (list == app->quadrant3_list) {
        return get_text(QStringLiteral("3º Quadrante"));
    }
    if (list == app->quadrant4_list) {
        return get_text(QStringLiteral("4º Quadrante"));
    }
    return get_text(QStringLiteral("Nenhuma Tarefa Concluída"));
}

void ensure_placeholder(source::EisenhowerMatrixApp *app, QListWidget *list, const QString &placeholder_text)
{
    if (list == nullptr || list_has_selectable_items(list)) {
        return;
    }

    const QSignalBlocker blocker(list);
    list->clear();
    app->add_placeholder(list, placeholder_text);
}

void ensure_all_placeholders(source::EisenhowerMatrixApp *app)
{
    ensure_placeholder(app, app->quadrant1_list, get_text(QStringLiteral("1º Quadrante")));
    ensure_placeholder(app, app->quadrant2_list, get_text(QStringLiteral("2º Quadrante")));
    ensure_placeholder(app, app->quadrant3_list, get_text(QStringLiteral("3º Quadrante")));
    ensure_placeholder(app, app->quadrant4_list, get_text(QStringLiteral("4º Quadrante")));
    ensure_placeholder(app, app->quadrant1_in_progress_list, get_text(QStringLiteral("Nenhuma Tarefa em Progresso")));
    ensure_placeholder(app, app->quadrant2_in_progress_list, get_text(QStringLiteral("Nenhuma Tarefa em Progresso")));
    ensure_placeholder(app, app->quadrant3_in_progress_list, get_text(QStringLiteral("Nenhuma Tarefa em Progresso")));
    ensure_placeholder(app, app->quadrant4_in_progress_list, get_text(QStringLiteral("Nenhuma Tarefa em Progresso")));
    ensure_placeholder(app, app->quadrant1_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída")));
    ensure_placeholder(app, app->quadrant2_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída")));
    ensure_placeholder(app, app->quadrant3_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída")));
    ensure_placeholder(app, app->quadrant4_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída")));
}

void clear_all_task_lists(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }

    const QList<QListWidget *> lists = {
        app->quadrant1_list,
        app->quadrant2_list,
        app->quadrant3_list,
        app->quadrant4_list,
        app->quadrant1_in_progress_list,
        app->quadrant2_in_progress_list,
        app->quadrant3_in_progress_list,
        app->quadrant4_in_progress_list,
        app->quadrant1_completed_list,
        app->quadrant2_completed_list,
        app->quadrant3_completed_list,
        app->quadrant4_completed_list,
    };

    for (QListWidget *list : lists) {
        if (list == nullptr) {
            continue;
        }

        const QSignalBlocker blocker(list);
        list->clear();
    }
}

QList<QListWidget *> eisenhower_task_lists(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return {};
    }

    return {
        app->quadrant1_list,
        app->quadrant2_list,
        app->quadrant3_list,
        app->quadrant4_list,
        app->quadrant1_in_progress_list,
        app->quadrant2_in_progress_list,
        app->quadrant3_in_progress_list,
        app->quadrant4_in_progress_list,
        app->quadrant1_completed_list,
        app->quadrant2_completed_list,
        app->quadrant3_completed_list,
        app->quadrant4_completed_list,
    };
}

bool is_eisenhower_task_list(source::EisenhowerMatrixApp *app, QListWidget *list)
{
    return list != nullptr && eisenhower_task_lists(app).contains(list);
}

QString human_date_for_display(source::EisenhowerMatrixApp *app, const QString &date_iso)
{
    const QString cleaned = date_iso.trimmed();
    if (cleaned.isEmpty()) {
        return {};
    }

    const QDate date = QDate::fromString(cleaned, Qt::ISODate);
    if (!date.isValid()) {
        return cleaned;
    }

    return (app != nullptr && app->date_input != nullptr) ? date.toString(app->date_input->displayFormat()) : cleaned;
}

QString visible_task_text(source::EisenhowerMatrixApp *app,
                          const QString &decorated_text,
                          const QString &date_iso,
                          const QString &time_str)
{
    QString display_text = decorated_text.trimmed();
    if (app == nullptr || !app->reserved_space_eisenhower_enabled()) {
        return display_text;
    }

    const QString human_date = human_date_for_display(app, date_iso);
    if (human_date.isEmpty()) {
        return display_text;
    }

    const QString cleaned_time = time_str.trimmed();
    return cleaned_time.isEmpty() ? QStringLiteral("%1 — %2").arg(display_text, human_date)
                                  : QStringLiteral("%1 — %2 %3").arg(display_text, human_date, cleaned_time);
}

QString compose_task_display_text(source::EisenhowerMatrixApp *app,
                                  const QString &decorated_text,
                                  const QString &date_iso,
                                  const QString &time_str)
{
    return visible_task_text(app, decorated_text, date_iso, time_str);
}

QPair<QString, QString> build_display_and_tooltip(source::EisenhowerMatrixApp *app,
                                                  const QString &text,
                                                  const QString &date_iso,
                                                  const QString &time_str,
                                                  const QVariant &domain = QVariant())
{
    const QString clean_text = text.trimmed();
    const QString domain_badge =
        domain.isValid() && !domain.toString().trimmed().isEmpty() ? source::InterfaceCore::dominio_insignia(domain)
                                                                   : QString();
    const QString decorated_text =
        domain_badge.isEmpty() ? clean_text : QStringLiteral("%1 %2").arg(domain_badge, clean_text);
    QString display_text = compose_task_display_text(app, decorated_text, date_iso, time_str);
    QStringList tooltip_lines;

    if (!date_iso.isEmpty()) {
        const QString date_human = human_date_for_display(app, date_iso);
        if (!date_human.isEmpty()) {
            tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Data")), date_human);
        }
    }

    if (!time_str.isEmpty()) {
        tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Horário")), time_str);
    }

    return {display_text, tooltip_lines.join(QLatin1Char('\n'))};
}

QVariantMap item_data(QListWidgetItem *item)
{
    return item != nullptr ? item->data(Qt::UserRole).toMap() : QVariantMap();
}

int priority_for_list(source::EisenhowerMatrixApp *app, QListWidget *list)
{
    if (app == nullptr || list == nullptr) {
        return 0;
    }
    if (list == app->quadrant1_list || list == app->quadrant1_in_progress_list || list == app->quadrant1_completed_list) {
        return 1;
    }
    if (list == app->quadrant2_list || list == app->quadrant2_in_progress_list || list == app->quadrant2_completed_list) {
        return 2;
    }
    if (list == app->quadrant3_list || list == app->quadrant3_in_progress_list || list == app->quadrant3_completed_list) {
        return 3;
    }
    if (list == app->quadrant4_list || list == app->quadrant4_in_progress_list || list == app->quadrant4_completed_list) {
        return 4;
    }
    return 0;
}

QString norm_text(const QVariant &value)
{
    return value.toString().trimmed();
}

QVariant norm_prio(const QVariant &value)
{
    bool ok = false;
    const int int_value = value.toInt(&ok);
    return ok ? QVariant(int_value) : QVariant(norm_text(value));
}

QVariant norm_domain(const QVariant &value)
{
    bool ok = false;
    const int int_value = value.toInt(&ok);
    return ok ? QVariant(qBound(0, int_value, 2)) : QVariant(norm_text(value));
}

QString fold_text(QString value)
{
    value = value.trimmed().toCaseFolded();
    value.replace(QStringLiteral("não"), QStringLiteral("nao"));
    value.replace(QStringLiteral("ã"), QStringLiteral("a"));
    value.replace(QStringLiteral("á"), QStringLiteral("a"));
    value.replace(QStringLiteral("â"), QStringLiteral("a"));
    value.replace(QStringLiteral("é"), QStringLiteral("e"));
    value.replace(QStringLiteral("ê"), QStringLiteral("e"));
    value.replace(QStringLiteral("í"), QStringLiteral("i"));
    value.replace(QStringLiteral("ó"), QStringLiteral("o"));
    value.replace(QStringLiteral("ô"), QStringLiteral("o"));
    value.replace(QStringLiteral("ú"), QStringLiteral("u"));
    value.remove(QRegularExpression(QStringLiteral("^[🔴🟠🟡🟢🥇🥈🥉\\s]+")));
    value.replace(QLatin1Char(','), QLatin1Char(' '));
    return value.simplified();
}

QString priority_title_for_value(const QVariant &value)
{
    return source::InterfaceCore::prioridade_para_texto(value).trimmed();
}

QString domain_title_for_value(const QVariant &value)
{
    return source::InterfaceCore::dominio_para_texto(value, false).trimmed();
}

int priority_value_from_entry(const QJsonObject &entry, source::EisenhowerMatrixApp *app, QListWidget *list)
{
    const int numeric_priority = entry.value(QStringLiteral("priority")).toInt();
    if (numeric_priority > 0) {
        return numeric_priority;
    }

    const QString folded = fold_text(entry.value(QStringLiteral("titulo_prioridade")).toString());
    if (folded.startsWith(QStringLiteral("importante e urgente"))
        || folded.startsWith(QStringLiteral("important and urgent"))) {
        return 1;
    }
    if (folded.startsWith(QStringLiteral("importante mas nao urgente"))
        || folded.startsWith(QStringLiteral("important but not urgent"))) {
        return 2;
    }
    if (folded.startsWith(QStringLiteral("nao importante mas urgente"))
        || folded.startsWith(QStringLiteral("not important but urgent"))) {
        return 3;
    }
    if (folded.startsWith(QStringLiteral("nao importante e nao urgente"))
        || folded.startsWith(QStringLiteral("not important and not urgent"))) {
        return 4;
    }

    return priority_for_list(app, list);
}

int domain_value_from_entry(const QJsonObject &entry)
{
    if (entry.contains(QStringLiteral("dominio"))) {
        return entry.value(QStringLiteral("dominio")).toInt();
    }

    const QString folded = fold_text(entry.value(QStringLiteral("titulo_dominio")).toString());
    if (folded.startsWith(QStringLiteral("avancado")) || folded.startsWith(QStringLiteral("advanced"))) {
        return 2;
    }
    if (folded.startsWith(QStringLiteral("intermediario")) || folded.startsWith(QStringLiteral("intermediate"))) {
        return 1;
    }
    if (folded.startsWith(QStringLiteral("iniciante")) || folded.startsWith(QStringLiteral("beginner"))) {
        return 0;
    }

    return -1;
}

bool same_task(const QVariantMap &left, const QVariantMap &right)
{
    const QString left_id = norm_text(left.value(QStringLiteral("id")));
    const QString right_id = norm_text(right.value(QStringLiteral("id")));
    if (!left_id.isEmpty() && !right_id.isEmpty()) {
        return left_id == right_id;
    }

    return norm_text(left.value(QStringLiteral("text"))) == norm_text(right.value(QStringLiteral("text")))
           && norm_text(left.value(QStringLiteral("date"))) == norm_text(right.value(QStringLiteral("date")))
           && norm_text(left.value(QStringLiteral("time"))) == norm_text(right.value(QStringLiteral("time")))
           && norm_text(left.value(QStringLiteral("file_path"))) == norm_text(right.value(QStringLiteral("file_path")))
           && norm_text(left.value(QStringLiteral("description")))
                  == norm_text(right.value(QStringLiteral("description")))
           && norm_prio(left.value(QStringLiteral("priority"))) == norm_prio(right.value(QStringLiteral("priority")))
           && norm_domain(left.value(QStringLiteral("dominio"))) == norm_domain(right.value(QStringLiteral("dominio")));
}

QJsonArray list_to_entries(QListWidget *list)
{
    QJsonArray entries;
    if (list == nullptr) {
        return entries;
    }

    for (int i = 0; i < list->count(); ++i) {
        QListWidgetItem *item = list->item(i);
        if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable)) {
            continue;
        }

        QVariantMap data = item_data(item);
        QString task_id = data.value(QStringLiteral("id")).toString().trimmed();
        if (task_id.isEmpty()) {
            task_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
            data.insert(QStringLiteral("id"), task_id);
            item->setData(Qt::UserRole, data);
        }

        QJsonObject entry;
        entry.insert(QStringLiteral("id"), task_id);
        QString text = data.value(QStringLiteral("text")).toString();
        if (text.isEmpty()) {
            text = source::InterfaceCore::MostrarMenuContextoModules::base_text_from_item(item);
        }
        entry.insert(QStringLiteral("text"), text);
        entry.insert(QStringLiteral("date"), data.value(QStringLiteral("date")).toString());
        entry.insert(QStringLiteral("time"), data.value(QStringLiteral("time")).toString());
        bool order_ok = false;
        const qint64 insertion_order = data.value(QStringLiteral("insertion_order")).toLongLong(&order_ok);
        if (order_ok && insertion_order > 0) {
            entry.insert(QStringLiteral("insertion_order"), insertion_order);
        }

        const QString file_path = data.value(QStringLiteral("file_path")).toString();
        if (!file_path.isEmpty()) {
            entry.insert(QStringLiteral("file_path"), file_path);
        }

        const QString integration_origin =
            data.value(QStringLiteral("integracao_origem"), data.value(QStringLiteral("origin")))
                .toString()
                .trimmed();
        if (!integration_origin.isEmpty()) {
            entry.insert(QStringLiteral("integracao_origem"), integration_origin);
        }

        const QString text_color = data.value(QStringLiteral("cor_texto")).toString().trimmed();
        if (!text_color.isEmpty()) {
            entry.insert(QStringLiteral("cor_texto"), text_color);
        }
        const QString original_text_color =
            data.value(QStringLiteral("cor_texto_original"), data.value(QStringLiteral("agenda_cor_texto")))
                .toString()
                .trimmed();
        if (!original_text_color.isEmpty()) {
            entry.insert(QStringLiteral("cor_texto_original"), original_text_color);
        }
        const QString custom_text_color = data.value(QStringLiteral("cor_texto_customizada")).toString().trimmed();
        if (!custom_text_color.isEmpty()) {
            entry.insert(QStringLiteral("cor_texto_customizada"), custom_text_color);
        }

        const QString description = data.value(QStringLiteral("description")).toString();
        if (!description.isEmpty()) {
            entry.insert(QStringLiteral("description"), description);
        }

        const QVariant priority_value = data.value(QStringLiteral("priority"));
        bool ok = false;
        const int priority = priority_value.toInt(&ok);
        if (ok) {
            entry.insert(QStringLiteral("priority"), priority);
            entry.insert(QStringLiteral("titulo_prioridade"), priority_title_for_value(priority));
        } else if (priority_value.isValid() && !priority_value.toString().trimmed().isEmpty()) {
            entry.insert(QStringLiteral("priority"), priority_value.toString().trimmed());
            entry.insert(QStringLiteral("titulo_prioridade"), priority_title_for_value(priority_value));
        }

        const QVariant domain = data.value(QStringLiteral("dominio"));
        if (domain.isValid() && !domain.toString().trimmed().isEmpty()) {
            const int normalized_domain = qBound(0, domain.toInt(), 2);
            entry.insert(QStringLiteral("dominio"), normalized_domain);
            entry.insert(QStringLiteral("titulo_dominio"), domain_title_for_value(normalized_domain));
        }

        entries.append(entry);
    }

    return entries;
}

QVariantList unified_records_from_eisenhower(source::EisenhowerMatrixApp *app)
{
    QVariantList records;
    if (app == nullptr) {
        return records;
    }

    const QList<QPair<QListWidget *, int>> mapping = {
        {app->quadrant1_list, 1},
        {app->quadrant1_in_progress_list, 1},
        {app->quadrant1_completed_list, 1},
        {app->quadrant2_list, 2},
        {app->quadrant2_in_progress_list, 2},
        {app->quadrant2_completed_list, 2},
        {app->quadrant3_list, 3},
        {app->quadrant3_in_progress_list, 3},
        {app->quadrant3_completed_list, 3},
        {app->quadrant4_list, 4},
        {app->quadrant4_in_progress_list, 4},
        {app->quadrant4_completed_list, 4},
    };

    for (const auto &entry : mapping) {
        QListWidget *list = entry.first;
        const int quadrant = entry.second;
        const bool completed = app->is_completed_list(list);
        if (list == nullptr) {
            continue;
        }

        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem *item = list->item(i);
            if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable)) {
                continue;
            }

            QVariantMap data = item_data(item);
            QString task_id = data.value(QStringLiteral("id")).toString().trimmed();
            if (task_id.isEmpty()) {
                task_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
                data.insert(QStringLiteral("id"), task_id);
                item->setData(Qt::UserRole, data);
            }

            bool order_ok = false;
            const qint64 insertion_order = data.value(QStringLiteral("insertion_order")).toLongLong(&order_ok);
            const int priority = data.value(QStringLiteral("priority"), quadrant).toInt();
            const int domain = data.value(QStringLiteral("dominio"), 0).toInt();
            const QString title = data.value(QStringLiteral("text")).toString().trimmed().isEmpty()
                                      ? source::InterfaceCore::MostrarMenuContextoModules::base_text_from_item(item)
                                            .trimmed()
                                      : data.value(QStringLiteral("text")).toString().trimmed();

            double dur = 1.0;
            if (data.contains(QStringLiteral("duracao")) && data.value(QStringLiteral("duracao")).toDouble() > 0.0) {
                dur = data.value(QStringLiteral("duracao")).toDouble();
            } else if (data.contains(QStringLiteral("duration")) && data.value(QStringLiteral("duration")).toDouble() > 0.0) {
                dur = data.value(QStringLiteral("duration")).toDouble();
            } else if (data.contains(QStringLiteral("duracao_dias")) && data.value(QStringLiteral("duracao_dias")).toDouble() > 0.0) {
                dur = data.value(QStringLiteral("duracao_dias")).toDouble();
            }

            const bool in_progress = !completed && app->is_in_progress_list(list);
            QVariantMap record = {
                {QStringLiteral("id"), task_id},
                {QStringLiteral("modulo"), QStringLiteral("eisenhower")},
                {QStringLiteral("tipo"), QStringLiteral("tarefa")},
                {QStringLiteral("titulo"), title},
                {QStringLiteral("descricao"), data.value(QStringLiteral("description")).toString()},
                {QStringLiteral("prioridade"), priority > 0 ? priority : quadrant},
                {QStringLiteral("quadrante"), quadrant},
                {QStringLiteral("categoria"), QStringLiteral("quadrant%1").arg(quadrant)},
                {QStringLiteral("dominio"), qBound(0, domain, 2)},
                {QStringLiteral("status"), completed ? QStringLiteral("Done") : (in_progress ? QStringLiteral("Doing") : QStringLiteral("Todo"))},
                {QStringLiteral("concluida"), completed},
                {QStringLiteral("in_progress"), in_progress},
                {QStringLiteral("em_progresso"), in_progress},
                {QStringLiteral("em_andamento"), in_progress},
                {QStringLiteral("data"), data.value(QStringLiteral("date")).toString()},
                {QStringLiteral("hora"), data.value(QStringLiteral("time")).toString()},
                {QStringLiteral("arquivo_vinculado"), data.value(QStringLiteral("file_path")).toString()},
                {QStringLiteral("cor_texto"), data.value(QStringLiteral("cor_texto")).toString()},
                {QStringLiteral("cor_texto_original"), data.value(QStringLiteral("cor_texto_original")).toString()},
                {QStringLiteral("cor_texto_customizada"), data.value(QStringLiteral("cor_texto_customizada")).toString()},
                {QStringLiteral("integracao_origem"),
                 data.value(QStringLiteral("integracao_origem"), data.value(QStringLiteral("origin"))).toString()},
                {QStringLiteral("integracao_task_id"), data.value(QStringLiteral("integracao_task_id")).toString()},
                {QStringLiteral("ordem_insercao"), order_ok ? insertion_order : qint64(0)},
                {QStringLiteral("recurrence"), data.value(QStringLiteral("recurrence")).toString()},
                {QStringLiteral("recurrence_until"), data.value(QStringLiteral("recurrence_until")).toString()},
                {QStringLiteral("recurrence_source_id"), data.value(QStringLiteral("recurrence_source_id")).toString()},
                {QStringLiteral("review_due"), data.value(QStringLiteral("review_due")).toString()},
                {QStringLiteral("review_interval_days"), data.value(QStringLiteral("review_interval_days")).toInt()},
                {QStringLiteral("completed_at"), data.value(QStringLiteral("completed_at")).toString()},
                {QStringLiteral("font_config"),
                 !item->data(source::utils::kCustomFontSettingsRole).toMap().isEmpty()
                     ? item->data(source::utils::kCustomFontSettingsRole).toMap()
                     : data.value(QStringLiteral("font_config")).toMap()},
                {QStringLiteral("duracao"), dur},
                {QStringLiteral("duration"), dur},
                {QStringLiteral("duracao_dias"), dur},
                {QStringLiteral("pomodoros"), data.value(QStringLiteral("pomodoros"), data.value(QStringLiteral("pomodoros_completados"), 0)).toInt()},
                {QStringLiteral("pomodoros_completados"), data.value(QStringLiteral("pomodoros_completados"), data.value(QStringLiteral("pomodoros"), 0)).toInt()},
                {QStringLiteral("ciclos_totais"), data.value(QStringLiteral("ciclos_totais"), data.value(QStringLiteral("ciclos_permitidos"), data.value(QStringLiteral("total_ciclos"), app->default_pomodoro_cycle_count()))).toInt() > 0 ? data.value(QStringLiteral("ciclos_totais"), data.value(QStringLiteral("ciclos_permitidos"), data.value(QStringLiteral("total_ciclos"), app->default_pomodoro_cycle_count()))).toInt() : app->default_pomodoro_cycle_count()},
                {QStringLiteral("ciclos_permitidos"), data.value(QStringLiteral("ciclos_totais"), data.value(QStringLiteral("ciclos_permitidos"), data.value(QStringLiteral("total_ciclos"), app->default_pomodoro_cycle_count()))).toInt() > 0 ? data.value(QStringLiteral("ciclos_totais"), data.value(QStringLiteral("ciclos_permitidos"), data.value(QStringLiteral("total_ciclos"), app->default_pomodoro_cycle_count()))).toInt() : app->default_pomodoro_cycle_count()},
                {QStringLiteral("total_ciclos"), data.value(QStringLiteral("ciclos_totais"), data.value(QStringLiteral("ciclos_permitidos"), data.value(QStringLiteral("total_ciclos"), app->default_pomodoro_cycle_count()))).toInt() > 0 ? data.value(QStringLiteral("ciclos_totais"), data.value(QStringLiteral("ciclos_permitidos"), data.value(QStringLiteral("total_ciclos"), app->default_pomodoro_cycle_count()))).toInt() : app->default_pomodoro_cycle_count()},
                {QStringLiteral("ciclos_customizados"), data.value(QStringLiteral("ciclos_customizados"), false).toBool()},
            };

            records.append(source::utils::UnifiedTaskStore::normalize_record(record));
        }
    }

    return records;
}

bool load_eisenhower_from_unified_records(source::EisenhowerMatrixApp *app, const QVariantList &records)
{
    if (app == nullptr || records.isEmpty()) {
        return false;
    }

    const QVariantList eisenhower_records =
        source::utils::UnifiedTaskStore::project_records_for_module(records, QStringLiteral("eisenhower"));
    if (eisenhower_records.isEmpty()) {
        return false;
    }

    const QList<QListWidget *> lists = {
        app->quadrant1_list,
        app->quadrant2_list,
        app->quadrant3_list,
        app->quadrant4_list,
        app->quadrant1_in_progress_list,
        app->quadrant2_in_progress_list,
        app->quadrant3_in_progress_list,
        app->quadrant4_in_progress_list,
        app->quadrant1_completed_list,
        app->quadrant2_completed_list,
        app->quadrant3_completed_list,
        app->quadrant4_completed_list,
    };

    std::vector<std::unique_ptr<QSignalBlocker>> blockers;
    for (QListWidget *list : lists) {
        blockers.push_back(std::make_unique<QSignalBlocker>(list));
        list->clear();
    }

    const QString date_format = (app->date_input != nullptr) ? app->date_input->displayFormat()
                                                             : QStringLiteral("dd/MM/yyyy");

    for (const QVariant &value : eisenhower_records) {
        const QVariantMap record = source::utils::UnifiedTaskStore::normalize_record(value.toMap());
        const QString title = record.value(QStringLiteral("titulo")).toString().trimmed();
        if (title.isEmpty()) {
            continue;
        }

        const int quadrant = qBound(1, record.value(QStringLiteral("quadrante")).toInt(), 4);
        const bool completed = record.value(QStringLiteral("concluida")).toBool();
        const QString status = record.value(QStringLiteral("status")).toString();
        const bool is_done_rec = completed || status.compare(QStringLiteral("Done"), Qt::CaseInsensitive) == 0
                                 || status.compare(QStringLiteral("Completed"), Qt::CaseInsensitive) == 0;
        const bool in_progress_rec = !is_done_rec && (record.value(QStringLiteral("in_progress")).toBool() ||
                                     record.value(QStringLiteral("em_progresso")).toBool() ||
                                     record.value(QStringLiteral("em_andamento")).toBool() ||
                                     status.compare(QStringLiteral("Doing"), Qt::CaseInsensitive) == 0 ||
                                     status.contains(QStringLiteral("progress"), Qt::CaseInsensitive) ||
                                     status.contains(QStringLiteral("andamento"), Qt::CaseInsensitive));
        QListWidget *list = nullptr;
        if (is_done_rec) {
            list = app->quadrant_list(quadrant, true);
        } else if (in_progress_rec) {
            list = app->quadrant_in_progress_list(quadrant);
        } else {
            list = app->quadrant_list(quadrant, false);
        }
        if (list == nullptr) {
            continue;
        }

        const QString date_value = record.value(QStringLiteral("data")).toString().trimmed();
        const QString time_value = record.value(QStringLiteral("hora")).toString().trimmed();
        const QString domain_badge = source::InterfaceCore::dominio_insignia(record.value(QStringLiteral("dominio")));
        const QString decorated_title =
            domain_badge.isEmpty() ? title : QStringLiteral("%1 %2").arg(domain_badge, title);

        QString display_text = compose_task_display_text(app, decorated_title, date_value, time_value);
        QStringList tooltip_lines;
        if (!date_value.isEmpty()) {
            const QString date_human = human_date_for_display(app, date_value);
            if (!date_human.isEmpty()) {
                tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Data")), date_human);
            }
        }
        if (!time_value.isEmpty()) {
            tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Horário")), time_value);
        }

        auto *item = new QListWidgetItem(display_text);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        item->setCheckState(completed ? Qt::Checked : Qt::Unchecked);

        QVariantMap data = {
            {QStringLiteral("id"), record.value(QStringLiteral("id")).toString()},
            {QStringLiteral("text"), title},
            {QStringLiteral("date"), date_value},
            {QStringLiteral("time"), time_value},
            {QStringLiteral("priority"), record.value(QStringLiteral("prioridade")).toInt()},
            {QStringLiteral("dominio"), record.value(QStringLiteral("dominio")).toInt()},
            {QStringLiteral("description"), record.value(QStringLiteral("descricao")).toString()},
            {QStringLiteral("file_path"), record.value(QStringLiteral("arquivo_vinculado")).toString()},
            {QStringLiteral("cor_texto"), record.value(QStringLiteral("cor_texto")).toString()},
            {QStringLiteral("cor_texto_original"), record.value(QStringLiteral("cor_texto_original")).toString()},
            {QStringLiteral("cor_texto_customizada"), record.value(QStringLiteral("cor_texto_customizada")).toString()},
            {QStringLiteral("integracao_origem"), record.value(QStringLiteral("integracao_origem")).toString()},
            {QStringLiteral("integracao_task_id"), record.value(QStringLiteral("integracao_task_id")).toString()},
            {QStringLiteral("recurrence"), record.value(QStringLiteral("recurrence")).toString()},
            {QStringLiteral("recurrence_until"), record.value(QStringLiteral("recurrence_until")).toString()},
            {QStringLiteral("recurrence_source_id"), record.value(QStringLiteral("recurrence_source_id")).toString()},
            {QStringLiteral("review_due"), record.value(QStringLiteral("review_due")).toString()},
            {QStringLiteral("review_interval_days"), record.value(QStringLiteral("review_interval_days")).toInt()},
            {QStringLiteral("completed_at"), record.value(QStringLiteral("completed_at")).toString()},
            {QStringLiteral("font_config"), record.value(QStringLiteral("font_config")).toMap()},
        };
        double dur = 1.0;
        if (record.contains(QStringLiteral("duracao")) && record.value(QStringLiteral("duracao")).toDouble() > 0.0) {
            dur = record.value(QStringLiteral("duracao")).toDouble();
        } else if (record.contains(QStringLiteral("duration")) && record.value(QStringLiteral("duration")).toDouble() > 0.0) {
            dur = record.value(QStringLiteral("duration")).toDouble();
        } else if (record.contains(QStringLiteral("duracao_dias")) && record.value(QStringLiteral("duracao_dias")).toDouble() > 0.0) {
            dur = record.value(QStringLiteral("duracao_dias")).toDouble();
        }
        data.insert(QStringLiteral("duracao"), dur);
        data.insert(QStringLiteral("duration"), dur);
        data.insert(QStringLiteral("duracao_dias"), dur);
        const int pom = record.value(QStringLiteral("pomodoros"), record.value(QStringLiteral("pomodoros_completados"), 0)).toInt();
        data.insert(QStringLiteral("pomodoros"), pom);
        data.insert(QStringLiteral("pomodoros_completados"), pom);
        const int def_c = app != nullptr ? app->default_pomodoro_cycle_count() : 4;
        const int c_tot = record.value(QStringLiteral("ciclos_totais"), record.value(QStringLiteral("ciclos_permitidos"), record.value(QStringLiteral("total_ciclos"), def_c))).toInt();
        const int safe_c_tot = c_tot > 0 ? c_tot : def_c;
        data.insert(QStringLiteral("ciclos_totais"), safe_c_tot);
        data.insert(QStringLiteral("ciclos_permitidos"), safe_c_tot);
        data.insert(QStringLiteral("total_ciclos"), safe_c_tot);
        data.insert(QStringLiteral("ciclos_customizados"), record.value(QStringLiteral("ciclos_customizados"), false).toBool());

        const bool is_doing = (list == app->quadrant_in_progress_list(quadrant));
        data.insert(QStringLiteral("status"), completed ? QStringLiteral("Done") : (is_doing ? QStringLiteral("Doing") : QStringLiteral("Todo")));
        data.insert(QStringLiteral("in_progress"), is_doing);
        data.insert(QStringLiteral("em_progresso"), is_doing);
        data.insert(QStringLiteral("em_andamento"), is_doing);

        const qint64 insertion_order = record.value(QStringLiteral("ordem_insercao")).toLongLong();
        if (insertion_order > 0) {
            data.insert(QStringLiteral("insertion_order"), insertion_order);
        }
        item->setData(Qt::UserRole, data);
        const QVariantMap font_config = record.value(QStringLiteral("font_config")).toMap();
        if (!font_config.isEmpty()) {
            item->setData(source::utils::kCustomFontSettingsRole, font_config);
        }

        const QString file_path = data.value(QStringLiteral("file_path")).toString().trimmed();
        if (!file_path.isEmpty()) {
            tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Arquivo")), file_path);
        }
        apply_task_title_appearance(item, data);

        const QString description = data.value(QStringLiteral("description")).toString().trimmed();
        if (!description.isEmpty()) {
            tooltip_lines << QStringLiteral("%1:\n%2").arg(get_text(QStringLiteral("Descrição")), description);
        }

        tooltip_lines << QStringLiteral("%1: %2")
                             .arg(get_text(QStringLiteral("Nível de Domínio")),
                                  source::InterfaceCore::dominio_para_texto(record.value(QStringLiteral("dominio")), false));
        tooltip_lines << QStringLiteral("%1: %2")
                             .arg(get_text(QStringLiteral("Prioridade")),
                                  source::InterfaceCore::prioridade_para_texto(record.value(QStringLiteral("prioridade")), app));
        tooltip_lines << source::app::format_duration_tooltip_line(dur);
        if (pom > 0 || safe_c_tot > 0) {
            const double pom_pct = source::utils::calculate_task_completion_percentage(pom, safe_c_tot, completed);
            const QString pom_line_fmt = get_text(QStringLiteral("Ciclos Pomodoro: %1/%2 (%3%)"));
            tooltip_lines << pom_line_fmt
                                 .arg(QString::number(pom),
                                      QString::number(safe_c_tot),
                                      QString::number(pom_pct, 'f', 0));
        }

        item->setToolTip(tooltip_lines.join(QLatin1Char('\n')));
        app->insert_task_into_quadrant_list(list, item);
    }

    ensure_all_placeholders(app);
    return true;
}

}  // namespace

namespace task_utils {

QList<QVariantMap> collect_tasks_from_app(source::EisenhowerMatrixApp *app)
{
    QList<QVariantMap> tasks;
    if (app == nullptr) {
        return tasks;
    }

    const QString em_progresso = QCoreApplication::translate("InterfaceGrafica", "Em Progresso");
    const QList<QPair<QListWidget *, QPair<QString, bool>>> mapping = {
        {app->quadrant1_list, {app->quadrant1_label->text(), false}},
        {app->quadrant1_in_progress_list,
         {QStringLiteral("%1 - %2").arg(app->quadrant1_label->text(), em_progresso), false}},
        {app->quadrant1_completed_list,
         {QStringLiteral("%1 - %2").arg(app->quadrant1_label->text(), app->quadrant1_completed_label->text()), true}},
        {app->quadrant2_list, {app->quadrant2_label->text(), false}},
        {app->quadrant2_in_progress_list,
         {QStringLiteral("%1 - %2").arg(app->quadrant2_label->text(), em_progresso), false}},
        {app->quadrant2_completed_list,
         {QStringLiteral("%1 - %2").arg(app->quadrant2_label->text(), app->quadrant2_completed_label->text()), true}},
        {app->quadrant3_list, {app->quadrant3_label->text(), false}},
        {app->quadrant3_in_progress_list,
         {QStringLiteral("%1 - %2").arg(app->quadrant3_label->text(), em_progresso), false}},
        {app->quadrant3_completed_list,
         {QStringLiteral("%1 - %2").arg(app->quadrant3_label->text(), app->quadrant3_completed_label->text()), true}},
        {app->quadrant4_list, {app->quadrant4_label->text(), false}},
        {app->quadrant4_in_progress_list,
         {QStringLiteral("%1 - %2").arg(app->quadrant4_label->text(), em_progresso), false}},
        {app->quadrant4_completed_list,
         {QStringLiteral("%1 - %2").arg(app->quadrant4_label->text(), app->quadrant4_completed_label->text()), true}},
    };

    for (const auto &entry : mapping) {
        QListWidget *list = entry.first;
        const QString category = entry.second.first;
        const bool completed = entry.second.second;
        if (list == nullptr) {
            continue;
        }

        const bool in_progress_list = (app != nullptr && app->is_in_progress_list(list));

        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem *item = list->item(i);
            if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable)) {
                continue;
            }

            QVariantMap data = item->data(Qt::UserRole).toMap();
            const bool in_progress = in_progress_list
                || data.value(QStringLiteral("in_progress")).toBool()
                || data.value(QStringLiteral("em_progresso")).toBool()
                || (data.value(QStringLiteral("status")).toString().compare(QStringLiteral("doing"), Qt::CaseInsensitive) == 0)
                || (data.value(QStringLiteral("status")).toString().compare(QStringLiteral("in_progress"), Qt::CaseInsensitive) == 0);

            QVariantMap task;
            task.insert(QStringLiteral("text"),
                        data.value(QStringLiteral("text")).toString().isEmpty()
                            ? source::InterfaceCore::MostrarMenuContextoModules::base_text_from_item(item)
                            : data.value(QStringLiteral("text")));
            task.insert(QStringLiteral("date"), data.value(QStringLiteral("date")));
            task.insert(QStringLiteral("time"), data.value(QStringLiteral("time")));
            task.insert(QStringLiteral("category"), category);
            task.insert(QStringLiteral("completed"), completed);
            task.insert(QStringLiteral("in_progress"), in_progress);
            task.insert(QStringLiteral("em_progresso"), in_progress);
            task.insert(QStringLiteral("status"),
                        completed ? QStringLiteral("Done")
                                  : (in_progress ? QStringLiteral("Doing") : QStringLiteral("Todo")));
            task.insert(QStringLiteral("id"), data.value(QStringLiteral("id")));
            task.insert(QStringLiteral("file_path"), data.value(QStringLiteral("file_path")));
            task.insert(QStringLiteral("description"), data.value(QStringLiteral("description")));
            const QString integration_origin =
                data.value(QStringLiteral("integracao_origem"), data.value(QStringLiteral("origin")))
                    .toString()
                    .trimmed();
            if (!integration_origin.isEmpty()) {
                task.insert(QStringLiteral("integracao_origem"), integration_origin);
            } else {
                task.insert(QStringLiteral("integracao_origem"), QStringLiteral("eisenhower"));
            }
            const QString text_color =
                data.value(QStringLiteral("cor_texto"), data.value(QStringLiteral("task_color"))).toString().trimmed();
            if (!text_color.isEmpty()) {
                task.insert(QStringLiteral("cor_texto"), text_color);
                task.insert(QStringLiteral("task_color"), text_color);
            }
            const QString original_text_color =
                data.value(QStringLiteral("cor_texto_original"), data.value(QStringLiteral("agenda_cor_texto"))).toString().trimmed();
            if (!original_text_color.isEmpty()) {
                task.insert(QStringLiteral("cor_texto_original"), original_text_color);
            }
            const QString custom_text_color =
                data.value(QStringLiteral("cor_texto_customizada")).toString().trimmed();
            if (!custom_text_color.isEmpty()) {
                task.insert(QStringLiteral("cor_texto_customizada"), custom_text_color);
            }
            QVariant priority = data.value(QStringLiteral("priority"));
            if (!priority.isValid() || priority.toString().trimmed().isEmpty() || priority.toInt() <= 0) {
                const int inferred_priority = priority_for_list(app, list);
                if (inferred_priority > 0) {
                    priority = inferred_priority;
                }
            }
            task.insert(QStringLiteral("priority"), priority);
            task.insert(QStringLiteral("prioridade"), priority);
            task.insert(QStringLiteral("quadrante"), priority);
            if (data.contains(QStringLiteral("dominio"))) {
                task.insert(QStringLiteral("dominio"), data.value(QStringLiteral("dominio")));
            }
            const int pomodoros = qMax(0, data.value(QStringLiteral("pomodoros"),
                                                     data.value(QStringLiteral("pomodoros_completados"), 0)).toInt());
            task.insert(QStringLiteral("pomodoros"), pomodoros);
            task.insert(QStringLiteral("pomodoros_completados"), pomodoros);
            const double dur_val = data.contains(QStringLiteral("duracao"))
                                       ? data.value(QStringLiteral("duracao")).toDouble()
                                       : (data.contains(QStringLiteral("duration"))
                                              ? data.value(QStringLiteral("duration")).toDouble()
                                              : 1.0);
            task.insert(QStringLiteral("duracao"), dur_val > 0.0 ? dur_val : 1.0);
            task.insert(QStringLiteral("duration"), dur_val > 0.0 ? dur_val : 1.0);
            tasks.append(task);
        }
    }

    return tasks;
}

QString priority_emoji(const QVariant &prio, source::EisenhowerMatrixApp *app)
{
    const QString label = source::InterfaceCore::prioridade_para_texto(prio, app);
    if (label.startsWith(QStringLiteral("🔴"))) {
        return QStringLiteral("🔴");
    }
    if (label.startsWith(QStringLiteral("🟠"))) {
        return QStringLiteral("🟠");
    }
    if (label.startsWith(QStringLiteral("🟡"))) {
        return QStringLiteral("🟡");
    }
    if (label.startsWith(QStringLiteral("🟢"))) {
        return QStringLiteral("🟢");
    }
    return QString();
}

bool is_task_in_progress(const QVariantMap &task)
{
    if (task.value(QStringLiteral("completed")).toBool()) {
        return false;
    }
    if (task.value(QStringLiteral("in_progress")).toBool() ||
        task.value(QStringLiteral("em_progresso")).toBool()) {
        return true;
    }
    const QString status = task.value(QStringLiteral("status")).toString().trimmed();
    if (status.compare(QStringLiteral("doing"), Qt::CaseInsensitive) == 0 ||
        status.compare(QStringLiteral("in_progress"), Qt::CaseInsensitive) == 0 ||
        status.compare(QStringLiteral("in progress"), Qt::CaseInsensitive) == 0 ||
        status.compare(QStringLiteral("em progresso"), Qt::CaseInsensitive) == 0) {
        return true;
    }
    const QString category = task.value(QStringLiteral("category")).toString();
    if (category.contains(QStringLiteral("Em Progresso"), Qt::CaseInsensitive) ||
        category.contains(QStringLiteral("In Progress"), Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

QString task_status_text(const QVariantMap &task)
{
    if (task.value(QStringLiteral("completed")).toBool()) {
        return QCoreApplication::translate("InterfaceGrafica", "Concluída");
    }
    if (is_task_in_progress(task)) {
        return QCoreApplication::translate("InterfaceGrafica", "Em Progresso");
    }
    return QCoreApplication::translate("InterfaceGrafica", "Pendente");
}

QString task_label_for_day(const QVariantMap &task,
                           bool show_priority,
                           bool show_domain,
                           source::EisenhowerMatrixApp *app)
{
    const QString text = task.value(QStringLiteral("text")).toString().trimmed();
    const QString time_str = task.value(QStringLiteral("time")).toString();
    const QString status_text = task_status_text(task);
    const QString emoji = show_priority ? priority_emoji(task.value(QStringLiteral("priority")), app) : QString();
    const QString domain_badge =
        show_domain ? source::InterfaceCore::dominio_insignia(task.value(QStringLiteral("dominio"))) : QString();
    const QString title = domain_badge.isEmpty() ? text : QStringLiteral("%1 %2").arg(domain_badge, text);

    if (!time_str.isEmpty()) {
        return QStringLiteral("%1 %2 — %3 [%4]").arg(emoji, time_str, title, status_text).trimmed();
    }
    return QStringLiteral("%1 %2 [%3]").arg(emoji, title, status_text).trimmed();
}

QPair<QListWidget *, QListWidgetItem *> find_source_item_for_calendar(source::EisenhowerMatrixApp *app,
                                                                      const QVariantMap &task_dict)
{
    if (app == nullptr || task_dict.isEmpty()) {
        return {nullptr, nullptr};
    }

    const QString cat = task_dict.value(QStringLiteral("category")).toString();
    const bool completed = task_dict.value(QStringLiteral("completed")).toBool();
    const QStringList labels = {
        app->quadrant1_label != nullptr ? app->quadrant1_label->text() : QString(),
        app->quadrant2_label != nullptr ? app->quadrant2_label->text() : QString(),
        app->quadrant3_label != nullptr ? app->quadrant3_label->text() : QString(),
        app->quadrant4_label != nullptr ? app->quadrant4_label->text() : QString(),
    };
    const QList<QListWidget *> pending = {app->quadrant1_list,
                                          app->quadrant2_list,
                                          app->quadrant3_list,
                                          app->quadrant4_list};
    const QList<QListWidget *> done = {app->quadrant1_completed_list,
                                       app->quadrant2_completed_list,
                                       app->quadrant3_completed_list,
                                       app->quadrant4_completed_list};

    const QList<QListWidget *> in_progress = {app->quadrant1_in_progress_list,
                                              app->quadrant2_in_progress_list,
                                              app->quadrant3_in_progress_list,
                                              app->quadrant4_in_progress_list};

    for (int idx = 0; idx < labels.size(); ++idx) {
        if (labels.at(idx).isEmpty() || !cat.startsWith(labels.at(idx))) {
            continue;
        }

        QList<QListWidget *> candidate_lists;
        if (completed) {
            candidate_lists.append(done.at(idx));
        } else if (is_task_in_progress(task_dict)) {
            candidate_lists.append(in_progress.at(idx));
            candidate_lists.append(pending.at(idx));
        } else {
            candidate_lists.append(pending.at(idx));
            candidate_lists.append(in_progress.at(idx));
        }

        for (QListWidget *src_list : candidate_lists) {
            if (src_list == nullptr) {
                continue;
            }

            for (int i = 0; i < src_list->count(); ++i) {
                QListWidgetItem *it = src_list->item(i);
                if (it == nullptr || !(it->flags() & Qt::ItemIsSelectable)) {
                    continue;
                }

                if (same_task(it->data(Qt::UserRole).toMap(), task_dict)) {
                    return {src_list, it};
                }
            }
        }
    }

    return {nullptr, nullptr};
}

}  // namespace task_utils

void add_task(source::EisenhowerMatrixApp *app)
{
    try {
        const QString task_text = app->task_input->text().trimmed();
        if (task_text.isEmpty()) {
            QMessageBox::warning(app,
                                 get_text(QStringLiteral("Erro")),
                                 get_text(QStringLiteral("A tarefa não pode estar vazia.")));
            return;
        }

        const int selected_quadrant = app->quadrant_selector->currentIndex();
        QListWidget *target_list = app->quadrant_list(selected_quadrant + 1, false);
        if (target_list == nullptr) {
            logger()->error(QStringLiteral("Quadrante inválido selecionado: %1").arg(selected_quadrant));
            return;
        }

        QString date_str;
        QString time_str;
        QString tooltip_text;
        const int domain_value = app->domain_selector != nullptr ? qBound(0, app->domain_selector->currentIndex(), 2) : 0;
        if (app->date_checkbox->isChecked()) {
            date_str = app->date_input->date().toString(Qt::ISODate);
            const QString date_human = app->date_input->date().toString(app->date_input->displayFormat());
            if (app->time_checkbox != nullptr && app->time_checkbox->isChecked()) {
                time_str = app->time_input->time().toString(QStringLiteral("HH:mm"));
            }

            QStringList tooltip_lines;
            tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Data")), date_human);
            if (!time_str.isEmpty()) {
                tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Horário")), time_str);
            }
            tooltip_text = tooltip_lines.join(QLatin1Char('\n'));
        }

        if (target_list->count() == 1 && !(target_list->item(0)->flags() & Qt::ItemIsSelectable)) {
            target_list->clear();
        }

        app->show_task_sync_add_info(QStringLiteral("eisenhower"));

        const QString domain_badge = source::InterfaceCore::dominio_insignia(domain_value);
        const QString decorated_task_text =
            domain_badge.isEmpty() ? task_text : QStringLiteral("%1 %2").arg(domain_badge, task_text);
        QString display_text = compose_task_display_text(app, decorated_task_text, date_str, time_str);

        auto *task_item = new QListWidgetItem(display_text);
        task_item->setFlags(task_item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        task_item->setCheckState(Qt::Unchecked);

        QVariantMap data;
        data.insert(QStringLiteral("text"), task_text);
        data.insert(QStringLiteral("date"), date_str);
        data.insert(QStringLiteral("time"), time_str);
        data.insert(QStringLiteral("priority"), selected_quadrant + 1);
        data.insert(QStringLiteral("dominio"), domain_value);
        const double duration_days = (app->duration_spin != nullptr) ? app->duration_spin->value() : 1.0;
        if (duration_days > 0.0) {
            data.insert(QStringLiteral("duracao_dias"), duration_days);
        }
        task_item->setData(Qt::UserRole, data);
        QStringList tooltip_lines;
        if (!tooltip_text.isEmpty()) {
            tooltip_lines << tooltip_text;
        }
        tooltip_lines << QStringLiteral("%1: %2")
                             .arg(get_text(QStringLiteral("Prioridade")),
                                  source::InterfaceCore::prioridade_para_texto(selected_quadrant + 1, app));
        tooltip_lines << QStringLiteral("%1: %2")
                             .arg(get_text(QStringLiteral("Nível de Domínio")),
                                  source::InterfaceCore::dominio_para_texto(domain_value, false));
        if (duration_days > 0.0) {
            tooltip_lines << source::app::format_duration_tooltip_line(duration_days);
        }
        task_item->setToolTip(tooltip_lines.join(QLatin1Char('\n')));

        const QVariant dragged_path = app->property("_dragged_file_path");
        if (dragged_path.isValid() && !dragged_path.toString().isEmpty()) {
            const auto reply = QMessageBox::question(
                app,
                get_text(QStringLiteral("Vincular Arquivo")),
                get_text(QStringLiteral("Deseja vincular o arquivo a esta tarefa?")),
                QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                data.insert(QStringLiteral("file_path"), dragged_path.toString());
                task_item->setData(Qt::UserRole, data);
                QString tooltip = task_item->toolTip();
                if (!tooltip.isEmpty()) {
                    tooltip += QLatin1Char('\n');
                }
                tooltip += QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Arquivo")), dragged_path.toString());
                task_item->setToolTip(tooltip);
                apply_task_title_appearance(task_item, data);
            }
            app->setProperty("_dragged_file_path", QVariant());
        }

        app->insert_task_into_quadrant_list(target_list, task_item);
        app->task_input->clear();
        app->save_tasks();
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao adicionar tarefa."));
    }
}

void handle_item_checked(source::EisenhowerMatrixApp *app,
                         QListWidgetItem *item,
                         QListWidget *source_list,
                         QListWidget *target_list)
{
    if (item == nullptr || source_list == nullptr || target_list == nullptr) {
        return;
    }

    const Qt::CheckState new_state = app->is_completed_list(target_list) ? Qt::Checked : Qt::Unchecked;
    if (item->checkState() != new_state) {
        return;
    }

    move_item_between_lists(app, item, source_list, target_list, new_state);
    app->save_tasks();
    if (app->calendar_pane != nullptr && app->calendar_pane->calendar_panel != nullptr) {
        app->calendar_pane->calendar_panel->update_task_list();
    }
}

void move_item_between_lists(source::EisenhowerMatrixApp *app,
                             QListWidgetItem *item,
                             QListWidget *source,
                             QListWidget *target,
                             Qt::CheckState new_check_state)
{
    try {
        if (item == nullptr || source == nullptr || target == nullptr) {
            return;
        }

        QVariantMap data = item_data(item);
        QString base_text = data.value(QStringLiteral("text")).toString();
        if (base_text.isEmpty()) {
            base_text = source::InterfaceCore::MostrarMenuContextoModules::base_text_from_item(item);
        }
        const QString date_value = data.value(QStringLiteral("date")).toString();
        const QString time_value = data.value(QStringLiteral("time")).toString();

        const QString domain_badge = source::InterfaceCore::dominio_insignia(data.value(QStringLiteral("dominio")));
        const QString decorated_base_text =
            domain_badge.isEmpty() ? base_text : QStringLiteral("%1 %2").arg(domain_badge, base_text);
        QString display_text = compose_task_display_text(app, decorated_base_text, date_value, time_value);

        const QString tooltip = item->toolTip();
        {
            const QSignalBlocker source_blocker(source);
            source->takeItem(source->row(item));
        }
        app->cleanup_time_groups(source);

        if (!list_has_selectable_items(source)) {
            source->clear();
            app->add_placeholder(source, placeholder_text_for_list(app, source));
        }

        if (target->count() == 1 && !(target->item(0)->flags() & Qt::ItemIsSelectable)) {
            target->clear();
        }

        auto *new_item = new QListWidgetItem(display_text);
        new_item->setFlags(new_item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        const int target_prio = priority_for_list(app, target);
        if (target_prio > 0) {
            data.insert(QStringLiteral("priority"), target_prio);
            data.insert(QStringLiteral("prioridade"), target_prio);
            data.insert(QStringLiteral("quadrante"), target_prio);
        }
        if (app->is_completed_list(target)) {
            if (data.value(QStringLiteral("completed_at")).toString().trimmed().isEmpty()) {
                data.insert(QStringLiteral("completed_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
            }
            data.insert(QStringLiteral("status"), QStringLiteral("Done"));
            data.insert(QStringLiteral("in_progress"), false);
            data.insert(QStringLiteral("em_progresso"), false);
            data.insert(QStringLiteral("em_andamento"), false);
        } else if (app->is_in_progress_list(target)) {
            data.remove(QStringLiteral("completed_at"));
            data.insert(QStringLiteral("status"), QStringLiteral("Doing"));
            data.insert(QStringLiteral("in_progress"), true);
            data.insert(QStringLiteral("em_progresso"), true);
            data.insert(QStringLiteral("em_andamento"), true);
        } else {
            data.remove(QStringLiteral("completed_at"));
            data.insert(QStringLiteral("status"), QStringLiteral("Todo"));
            data.insert(QStringLiteral("in_progress"), false);
            data.insert(QStringLiteral("em_progresso"), false);
            data.insert(QStringLiteral("em_andamento"), false);
        }
        new_item->setData(Qt::UserRole, data);
        new_item->setCheckState(new_check_state);
        new_item->setToolTip(tooltip);

        apply_task_title_appearance(new_item, data);

        {
            const QSignalBlocker target_blocker(target);
            app->insert_task_into_quadrant_list(target, new_item);
        }
        app->cleanup_time_groups(target);
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao mover item entre listas."));
    }
}

bool remove_task(source::EisenhowerMatrixApp *app,
                 QListWidgetItem *item,
                 QListWidget *list_widget,
                 bool confirm)
{
    try {
        if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable)) {
            return false;
        }

        bool removed = false;
        if (confirm) {
            QMessageBox msg(app);
            msg.setWindowTitle(get_text(QStringLiteral("🗑️ Remover Tarefa")));
            msg.setText(get_text(QStringLiteral("Deseja remover a tarefa '{item}'?")).replace(QStringLiteral("{item}"),
                                                                                              source::InterfaceCore::MostrarMenuContextoModules::base_text_from_item(item)));
            QAbstractButton *btn_yes = msg.addButton(get_text(QStringLiteral("Yes")), QMessageBox::YesRole);
            msg.addButton(get_text(QStringLiteral("No")), QMessageBox::NoRole);
            msg.exec();
            if (msg.clickedButton() != btn_yes) {
                return false;
            }
            app->show_task_sync_delete_info(QStringLiteral("eisenhower"));
        }

        const QVariantMap data = item_data(item);
        auto same_task = [](const QVariantMap &left, const QVariantMap &right) {
            const auto norm = [](const QVariant &value) { return value.toString().trimmed(); };
            const QString left_id = norm(left.value(QStringLiteral("id")));
            const QString right_id = norm(right.value(QStringLiteral("id")));
            if (!left_id.isEmpty() && !right_id.isEmpty()) {
                return left_id == right_id;
            }

            return norm(left.value(QStringLiteral("text"))) == norm(right.value(QStringLiteral("text")))
                   && norm(left.value(QStringLiteral("date"))) == norm(right.value(QStringLiteral("date")))
                   && norm(left.value(QStringLiteral("time"))) == norm(right.value(QStringLiteral("time")))
                   && norm(left.value(QStringLiteral("file_path"))) == norm(right.value(QStringLiteral("file_path")))
                   && norm(left.value(QStringLiteral("description"))) == norm(right.value(QStringLiteral("description")))
                   && norm(left.value(QStringLiteral("priority"))) == norm(right.value(QStringLiteral("priority")))
                   && norm(left.value(QStringLiteral("dominio"))) == norm(right.value(QStringLiteral("dominio")));
        };

        if (is_eisenhower_task_list(app, list_widget)) {
            const int row = list_widget->row(item);
            if (row >= 0) {
                delete list_widget->takeItem(row);
                removed = true;
            }
        }

        if (!removed) {
            for (QListWidget *list : eisenhower_task_lists(app)) {
                if (list == nullptr) {
                    continue;
                }

                for (int i = 0; i < list->count(); ++i) {
                    QListWidgetItem *candidate = list->item(i);
                    if (candidate == nullptr || !(candidate->flags() & Qt::ItemIsSelectable)) {
                        continue;
                    }

                    if (same_task(item_data(candidate), data)) {
                        delete list->takeItem(i);
                        removed = true;
                        app->cleanup_time_groups(list);
                        break;
                    }
                }

                if (removed) {
                    break;
                }
            }
        }

        app->cleanup_time_groups(list_widget);
        ensure_all_placeholders(app);
        if (removed) {
            app->remove_task_from_synced_modules(data, QStringLiteral("eisenhower"));
        }
        app->save_tasks();

        if (app->calendar_pane != nullptr && app->calendar_pane->calendar_panel != nullptr) {
            app->calendar_pane->calendar_panel->update_task_list();
        }

        if (confirm) {
            if (removed) {
                QMessageBox::information(app,
                                         get_text(QStringLiteral("🗑️ Remover Tarefa")),
                                         get_text(QStringLiteral("Tarefa removida com sucesso.")));
            } else {
                QMessageBox::warning(app,
                                     get_text(QStringLiteral("🗑️ Remover Tarefa")),
                                     get_text(QStringLiteral("Nao foi possivel remover a tarefa.")));
            }
        }

        return removed;
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao remover tarefa."));
        return false;
    }
}

void save_tasks(source::EisenhowerMatrixApp *app)
{
    try {
        const QString path = app != nullptr ? app->tasks_path : QString();
        source::utils::UnifiedTaskStore::replace_module_records(path,
                                                                QStringLiteral("eisenhower"),
                                                                unified_records_from_eisenhower(app));
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao salvar tarefas."));
    }
}

bool load_tasks_from_path(source::EisenhowerMatrixApp *app, const QString &path)
{
    try {
        if (app == nullptr) {
            return false;
        }

        const QVariantList unified_records = source::utils::UnifiedTaskStore::load_records(path);
        if (load_eisenhower_from_unified_records(app, unified_records)) {
            return true;
        }

        QFile file(path);
        if (!file.exists()) {
            ensure_all_placeholders(app);
            return false;
        }
        if (!file.open(QIODevice::ReadOnly)) {
            ensure_all_placeholders(app);
            return false;
        }

        const QByteArray raw_json = file.readAll();
        QJsonParseError parse_error{};
        const QJsonDocument doc = QJsonDocument::fromJson(raw_json, &parse_error);
        const QJsonObject tasks = doc.object();
        if (parse_error.error == QJsonParseError::NoError && doc.isObject()
            && tasks.value(QStringLiteral("schema")).toString() == QStringLiteral("eisenhower_unified_tasks")
            && tasks.value(QStringLiteral("records")).isArray()) {
            clear_all_task_lists(app);
            ensure_all_placeholders(app);
            return true;
        }

        const QStringList expected_keys = {
            QStringLiteral("quadrant1"),
            QStringLiteral("quadrant1_completed"),
            QStringLiteral("quadrant2"),
            QStringLiteral("quadrant2_completed"),
            QStringLiteral("quadrant3"),
            QStringLiteral("quadrant3_completed"),
            QStringLiteral("quadrant4"),
            QStringLiteral("quadrant4_completed"),
        };

        bool has_known_key = false;
        if (parse_error.error != QJsonParseError::NoError || !doc.isObject()) {
            logger()->error(QStringLiteral("Erro ao carregar tarefas: JSON invalido em %1.").arg(path));
            return false;
        }

        for (const QString &key : expected_keys) {
            if (!tasks.contains(key)) {
                continue;
            }

            has_known_key = true;
            if (!tasks.value(key).isArray()) {
                logger()->error(QStringLiteral("Erro ao carregar tarefas: estrutura invalida em %1 para chave %2.")
                                    .arg(path, key));
                return false;
            }
        }

        if (!has_known_key) {
            logger()->error(QStringLiteral("Erro ao carregar tarefas: estrutura de autosave nao reconhecida em %1.")
                                .arg(path));
            return false;
        }

        const QList<QListWidget *> lists = {
            app->quadrant1_list,
            app->quadrant2_list,
            app->quadrant3_list,
            app->quadrant4_list,
            app->quadrant1_completed_list,
            app->quadrant2_completed_list,
            app->quadrant3_completed_list,
            app->quadrant4_completed_list,
        };

        std::vector<std::unique_ptr<QSignalBlocker>> blockers;
        for (QListWidget *list : lists) {
            blockers.push_back(std::make_unique<QSignalBlocker>(list));
            list->clear();
        }
        const QString date_format = (app->date_input != nullptr) ? app->date_input->displayFormat()
                                                                 : QStringLiteral("dd/MM/yyyy");

        auto create_item = [&](const QString &text,
                               const QString &date_value,
                               const QString &time_value,
                               int domain_value,
                               bool completed) {
            const QString domain_badge = domain_value >= 0 ? source::InterfaceCore::dominio_insignia(domain_value)
                                                           : QString();
            const QString decorated_text =
                domain_badge.isEmpty() ? text : QStringLiteral("%1 %2").arg(domain_badge, text);
            QString display_text = compose_task_display_text(app, decorated_text, date_value, time_value);
            QStringList tooltip_lines;

            if (!date_value.isEmpty()) {
                const QString date_human = human_date_for_display(app, date_value);
                if (!date_human.isEmpty()) {
                    tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Data")), date_human);
                }
            }

            if (!time_value.isEmpty()) {
                tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Horário")), time_value);
            }

            auto *item = new QListWidgetItem(display_text);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            item->setCheckState(completed ? Qt::Checked : Qt::Unchecked);

            QVariantMap data;
            data.insert(QStringLiteral("text"), text);
            data.insert(QStringLiteral("date"), date_value);
            data.insert(QStringLiteral("time"), time_value);
            item->setData(Qt::UserRole, data);
            item->setToolTip(tooltip_lines.join(QLatin1Char('\n')));
            return item;
        };

        auto populate_list = [&](const QString &key, QListWidget *list, bool completed) {
            const QJsonArray entries = tasks.value(key).toArray();
            if (entries.isEmpty()) {
                return;
            }
 
            for (const QJsonValue &value : entries) {
                QString text;
                QString date_value;
                QString time_value;
                QString task_id;
                qint64 insertion_order = 0;
                int priority_value = 0;
                int domain_value = -1;
                QString file_path;
                QString description;
                QString text_color;
                QString original_text_color;
                QString custom_text_color;
                QString integration_origin;

                if (value.isObject()) {
                    const QJsonObject entry = value.toObject();
                    text = entry.value(QStringLiteral("text")).toString();
                    date_value = entry.value(QStringLiteral("date")).toString();
                    time_value = entry.value(QStringLiteral("time")).toString();
                    task_id = entry.value(QStringLiteral("id")).toString();
                    insertion_order = entry.value(QStringLiteral("insertion_order")).toInteger();
                    priority_value = priority_value_from_entry(entry, app, list);
                    domain_value = domain_value_from_entry(entry);
                    file_path = entry.value(QStringLiteral("file_path")).toString();
                    description = entry.value(QStringLiteral("description")).toString();
                    text_color = entry.value(QStringLiteral("cor_texto")).toString().trimmed();
                    if (text_color.isEmpty()) {
                        text_color = entry.value(QStringLiteral("task_color")).toString().trimmed();
                    }
                    original_text_color = entry.value(QStringLiteral("cor_texto_original")).toString().trimmed();
                    if (original_text_color.isEmpty()) {
                        original_text_color = entry.value(QStringLiteral("agenda_cor_texto")).toString().trimmed();
                    }
                    custom_text_color = entry.value(QStringLiteral("cor_texto_customizada")).toString().trimmed();
                    integration_origin = entry.value(QStringLiteral("integracao_origem")).toString().trimmed();
                    if (integration_origin.isEmpty()) {
                        integration_origin = entry.value(QStringLiteral("origin")).toString().trimmed();
                    }
                } else {
                    text = value.toString();
                }

                text = text.trimmed();
                if (text.isEmpty()) {
                    continue;
                }

                QListWidgetItem *item = create_item(text, date_value, time_value, domain_value, completed);
                QVariantMap data = item_data(item);
                if (!task_id.isEmpty()) {
                    data.insert(QStringLiteral("id"), task_id);
                }
                if (insertion_order > 0) {
                    data.insert(QStringLiteral("insertion_order"), insertion_order);
                }
                if (!integration_origin.isEmpty()) {
                    data.insert(QStringLiteral("integracao_origem"), integration_origin);
                }

                const int priority = (priority_value > 0) ? priority_value : priority_for_list(app, list);
                if (priority > 0) {
                    data.insert(QStringLiteral("priority"), priority);
                }
                if (domain_value >= 0) {
                    data.insert(QStringLiteral("dominio"), qBound(0, domain_value, 2));
                }
                if (!text_color.isEmpty()) {
                    data.insert(QStringLiteral("cor_texto"), text_color);
                }
                if (!original_text_color.isEmpty()) {
                    data.insert(QStringLiteral("cor_texto_original"), original_text_color);
                }
                if (!custom_text_color.isEmpty()) {
                    data.insert(QStringLiteral("cor_texto_customizada"), custom_text_color);
                }

                if (!file_path.isEmpty()) {
                    data.insert(QStringLiteral("file_path"), file_path);
                    QString tooltip = item->toolTip();
                    if (!tooltip.isEmpty()) {
                        tooltip += QLatin1Char('\n');
                    }
                    tooltip += QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Arquivo")), file_path);
                    item->setToolTip(tooltip);
                }
                apply_task_title_appearance(item, data);

                if (!description.isEmpty()) {
                    data.insert(QStringLiteral("description"), description);
                    QString tooltip = item->toolTip();
                    if (!tooltip.isEmpty()) {
                        tooltip += QLatin1Char('\n');
                    }
                    tooltip += QStringLiteral("%1:\n%2").arg(get_text(QStringLiteral("Descrição")), description);
                    item->setToolTip(tooltip);
                }

                if (domain_value >= 0) {
                    QString tooltip = item->toolTip();
                    if (!tooltip.isEmpty()) {
                        tooltip += QLatin1Char('\n');
                    }
                    tooltip += QStringLiteral("%1: %2")
                                   .arg(get_text(QStringLiteral("Nível de Domínio")),
                                        source::InterfaceCore::dominio_para_texto(domain_value, false));
                    item->setToolTip(tooltip);
                }

                if (priority > 0) {
                    QString tooltip = item->toolTip();
                    if (!tooltip.isEmpty()) {
                        tooltip += QLatin1Char('\n');
                    }
                    tooltip += QStringLiteral("%1: %2")
                                   .arg(get_text(QStringLiteral("Prioridade")),
                                        source::InterfaceCore::prioridade_para_texto(priority, app));
                    item->setToolTip(tooltip);
                }

                item->setData(Qt::UserRole, data);
                app->insert_task_into_quadrant_list(list, item);
            }
        };

        populate_list(QStringLiteral("quadrant1"), app->quadrant1_list, false);
        populate_list(QStringLiteral("quadrant1_completed"), app->quadrant1_completed_list, true);
        populate_list(QStringLiteral("quadrant2"), app->quadrant2_list, false);
        populate_list(QStringLiteral("quadrant2_completed"), app->quadrant2_completed_list, true);
        populate_list(QStringLiteral("quadrant3"), app->quadrant3_list, false);
        populate_list(QStringLiteral("quadrant3_completed"), app->quadrant3_completed_list, true);
        populate_list(QStringLiteral("quadrant4"), app->quadrant4_list, false);
        populate_list(QStringLiteral("quadrant4_completed"), app->quadrant4_completed_list, true);

        ensure_all_placeholders(app);
        return true;
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao carregar tarefas."));
        return false;
    }
}

void load_tasks(source::EisenhowerMatrixApp *app)
{
    load_tasks_from_path(app, app != nullptr ? app->tasks_path : QString());
}

void edit_task_datetime(source::EisenhowerMatrixApp *app, QListWidgetItem *item, QListWidget *list_widget)
{
    try {
        if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable)) {
            return;
        }

        const QVariantMap old_data = item_data(item);
        QString text = old_data.value(QStringLiteral("text")).toString();
        if (text.isEmpty()) {
            text = source::InterfaceCore::MostrarMenuContextoModules::base_text_from_item(item);
        }
        const QString current_date_iso = old_data.value(QStringLiteral("date")).toString();
        const QString current_time_str = old_data.value(QStringLiteral("time")).toString();

        EditDateTimeDialog dialog(app, current_date_iso, current_time_str, app);
        if (dialog.exec() != QDialog::Accepted) {
            return;
        }

        const auto [new_date_iso, new_time_str] = dialog.values();
        const auto [display_text, tooltip] =
            build_display_and_tooltip(app, text, new_date_iso, new_time_str, old_data.value(QStringLiteral("dominio")));

        auto *new_item = new QListWidgetItem(display_text);
        new_item->setFlags(new_item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        new_item->setCheckState(item->checkState());

        QVariantMap new_data = old_data;
        new_data.insert(QStringLiteral("text"), text);
        new_data.insert(QStringLiteral("date"), new_date_iso);
        new_data.insert(QStringLiteral("time"), new_time_str);
        new_item->setData(Qt::UserRole, new_data);
        new_item->setToolTip(tooltip);

        apply_task_title_appearance(new_item, new_data);

        {
            const QSignalBlocker blocker(list_widget);
            list_widget->takeItem(list_widget->row(item));
            app->cleanup_time_groups(list_widget);

            if (!list_has_selectable_items(list_widget)) {
                list_widget->clear();
                app->add_placeholder(list_widget, placeholder_text_for_list(app, list_widget));
            }

            if (list_widget->count() == 1 && !(list_widget->item(0)->flags() & Qt::ItemIsSelectable)) {
                list_widget->clear();
            }

            app->insert_task_into_quadrant_list(list_widget, new_item);
        }

        app->save_tasks();
        if (app->calendar_pane != nullptr && app->calendar_pane->calendar_panel != nullptr) {
            app->calendar_pane->calendar_panel->update_task_list();
        }
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao editar data/horário da tarefa."));
    }
}

void move_task_to_quadrant(source::EisenhowerMatrixApp *app,
                           QListWidgetItem *item,
                           QListWidget *source_list,
                           QListWidget *target_list)
{
    try {
        if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable) || source_list == target_list) {
            return;
        }

        move_item_between_lists(app, item, source_list, target_list, Qt::Unchecked);
        app->save_tasks();
        if (app->calendar_pane != nullptr && app->calendar_pane->calendar_panel != nullptr) {
            app->calendar_pane->calendar_panel->update_task_list();
        }
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao mover tarefa de quadrante via menu."));
    }
}

}  // namespace source::GerenciamentoUI
