#include "source/utils/LinkedTitleUnderlineDelegate.hpp"

#include <QAbstractItemView>
#include <QApplication>
#include <QBrush>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFontComboBox>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QListWidgetItem>
#include <QMutex>
#include <QMutexLocker>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSpinBox>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>
#include <QTextLayout>
#include <QTextLine>
#include <QTextOption>
#include <QVariant>
#include <QVBoxLayout>

#include "source/utils/CaminhoPersistenteUtils.hpp"

namespace source::utils {
namespace {

QString general_task_title_font_config_path()
{
    return QDir(source::utils::obter_caminho_persistente())
        .filePath(QStringLiteral("tarefas_gerais_font_config.json"));
}

QString linked_file_title_font_config_path()
{
    return QDir(source::utils::obter_caminho_persistente())
        .filePath(QStringLiteral("arquivo_vinculado_font_config.json"));
}

QString plugin_general_task_title_font_config_path(PluginFontContext context)
{
    switch (context) {
        case PluginFontContext::Feynman:
            return QDir(source::utils::obter_caminho_persistente()).filePath(QStringLiteral("feynman_tarefas_gerais_font_config.json"));
        case PluginFontContext::Pomodoro:
            return QDir(source::utils::obter_caminho_persistente()).filePath(QStringLiteral("pomodoro_tarefas_gerais_font_config.json"));
        case PluginFontContext::MapaMental:
            return QDir(source::utils::obter_caminho_persistente()).filePath(QStringLiteral("mapamental_tarefas_gerais_font_config.json"));
        default:
            return general_task_title_font_config_path();
    }
}

QString plugin_linked_file_title_font_config_path(PluginFontContext context)
{
    switch (context) {
        case PluginFontContext::Feynman:
            return QDir(source::utils::obter_caminho_persistente()).filePath(QStringLiteral("feynman_arquivo_vinculado_font_config.json"));
        case PluginFontContext::Pomodoro:
            return QDir(source::utils::obter_caminho_persistente()).filePath(QStringLiteral("pomodoro_arquivo_vinculado_font_config.json"));
        case PluginFontContext::MapaMental:
            return QDir(source::utils::obter_caminho_persistente()).filePath(QStringLiteral("mapamental_arquivo_vinculado_font_config.json"));
        default:
            return linked_file_title_font_config_path();
    }
}


struct FontSettingsCacheEntry {
    TaskTitleFontSettings settings;
    qint64 last_modified_msecs = 0;
    bool exists = false;
};
static QHash<QString, FontSettingsCacheEntry> s_font_settings_cache;
static QMutex s_font_settings_cache_mutex;

QFont base_font_for_item(QListWidgetItem *item)
{
    if (item != nullptr && item->listWidget() != nullptr) {
        return item->listWidget()->font();
    }
    return QApplication::font();
}

TaskTitleFontSettings load_task_title_font_settings(const QString &path, const TaskTitleFontSettings &defaults)
{
    if (path.trimmed().isEmpty()) {
        return defaults;
    }

    const QFileInfo info(path);
    const bool file_exists = info.exists();
    const qint64 mtime = file_exists ? info.lastModified().toMSecsSinceEpoch() : 0;

    {
        QMutexLocker locker(&s_font_settings_cache_mutex);
        auto it = s_font_settings_cache.find(path);
        if (it != s_font_settings_cache.end()) {
            if (it->exists == file_exists && it->last_modified_msecs == mtime) {
                return it->settings;
            }
        }
    }

    TaskTitleFontSettings settings = defaults;

    if (!file_exists) {
        QMutexLocker locker(&s_font_settings_cache_mutex);
        s_font_settings_cache[path] = FontSettingsCacheEntry{settings, 0, false};
        return settings;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return settings;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (document.isObject()) {
        const QJsonObject object = document.object();
        settings.family = object.value(QStringLiteral("family")).toString(settings.family).trimmed();
        settings.style = object.value(QStringLiteral("style")).toString(settings.style).trimmed();
        const int point_size = object.value(QStringLiteral("point_size")).toInt(settings.point_size);
        settings.point_size = point_size >= 6 && point_size <= 72 ? point_size : 0;
        settings.bold = object.value(QStringLiteral("bold")).toBool(settings.bold);
        settings.italic = object.value(QStringLiteral("italic")).toBool(settings.italic);
        settings.underline = object.value(QStringLiteral("underline")).toBool(settings.underline);
        settings.strike_out = object.value(QStringLiteral("strike_out")).toBool(settings.strike_out);
    }

    {
        QMutexLocker locker(&s_font_settings_cache_mutex);
        s_font_settings_cache[path] = FontSettingsCacheEntry{settings, mtime, true};
    }

    return settings;
}

bool save_task_title_font_settings(const QString &path, const TaskTitleFontSettings &settings)
{
    if (path.isEmpty()) {
        return false;
    }

    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonObject object;
    object.insert(QStringLiteral("family"), settings.family.trimmed());
    object.insert(QStringLiteral("style"), settings.style.trimmed());
    object.insert(QStringLiteral("point_size"), settings.point_size >= 6 && settings.point_size <= 72 ? settings.point_size : 0);
    object.insert(QStringLiteral("bold"), settings.bold);
    object.insert(QStringLiteral("italic"), settings.italic);
    object.insert(QStringLiteral("underline"), settings.underline);
    object.insert(QStringLiteral("strike_out"), settings.strike_out);

    const QByteArray payload = QJsonDocument(object).toJson(QJsonDocument::Indented);
    const bool ok = file.write(payload) == payload.size() && file.commit();
    if (ok) {
        QMutexLocker locker(&s_font_settings_cache_mutex);
        s_font_settings_cache.remove(path);
    }
    return ok;
}

class LinkedTitleUnderlineDelegate final : public QStyledItemDelegate {
public:
    explicit LinkedTitleUnderlineDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem clean_option(option);
        initStyleOption(&clean_option, index);
        clean_option.font.setUnderline(false);
        clean_option.font.setStrikeOut(false);

        const QVariant foreground = index.data(Qt::ForegroundRole);
        if ((clean_option.state & QStyle::State_Selected) && foreground.canConvert<QBrush>()) {
            const QColor custom_color = qvariant_cast<QBrush>(foreground).color();
            if (custom_color.isValid()) {
                clean_option.palette.setColor(QPalette::HighlightedText, custom_color);
            }
        }

        QStyledItemDelegate::paint(painter, clean_option, index);

        const bool enabled = index.data(kLinkedTitleUnderlineRole).toBool();
        const QString title = index.data(kLinkedTitleUnderlineTextRole).toString().trimmed();
        const bool draw_underline = index.data(kLinkedTitleUnderlineVisibleRole).isValid()
                                        ? index.data(kLinkedTitleUnderlineVisibleRole).toBool()
                                        : true;
        const bool draw_strike_out = index.data(kLinkedTitleStrikeOutVisibleRole).toBool();
        if (!enabled || title.isEmpty() || painter == nullptr) {
            return;
        }

        draw_title_decorations(painter, clean_option, index, title, draw_underline, draw_strike_out);
    }

private:
    static void draw_title_decorations(QPainter *painter,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index,
                                       const QString &title,
                                       bool draw_underline,
                                       bool draw_strike_out)
    {
        if (!draw_underline && !draw_strike_out) {
            return;
        }

        if (option.text.isEmpty() || title.isEmpty() || painter == nullptr) {
            return;
        }

        QStyle *style = option.widget != nullptr ? option.widget->style() : QApplication::style();
        if (style == nullptr) {
            return;
        }

        QRect text_rect = style->subElementRect(QStyle::SE_ItemViewItemText, &option, option.widget);
        if (!text_rect.isValid()) {
            text_rect = option.rect;
        }

        const int title_pos = option.text.indexOf(title);
        if (title_pos < 0) {
            return;
        }

        const int title_end = title_pos + title.length();

        QTextLayout text_layout(option.text, option.font);
        QTextOption text_option(option.displayAlignment);
        text_option.setTextDirection(option.direction);
        text_layout.setTextOption(text_option);

        text_layout.beginLayout();
        qreal height = 0;
        while (true) {
            QTextLine line = text_layout.createLine();
            if (!line.isValid()) {
                break;
            }
            line.setLineWidth(text_rect.width());
            line.setPosition(QPointF(0, height));
            height += line.height();
        }
        text_layout.endLayout();

        const QRect layout_rect = QStyle::alignedRect(
            option.direction,
            option.displayAlignment,
            text_layout.boundingRect().size().toSize(),
            text_rect
        );

        QFontMetrics metrics(option.font);
        const qreal underline_offset = qBound<qreal>(
            1.0,
            metrics.underlinePos() > 0 ? qreal(metrics.underlinePos()) : 1.5,
            qMax<qreal>(1.0, metrics.descent() - 1.0)
        );
        const qreal strike_out_offset = qMax<qreal>(
            1.0,
            metrics.strikeOutPos() > 0 ? qreal(metrics.strikeOutPos()) : (metrics.ascent() / 3.0)
        );

        QColor pen_color;
        if (option.state & QStyle::State_Selected) {
            pen_color = option.palette.color(QPalette::HighlightedText);
        } else {
            const QVariant foreground = index.data(Qt::ForegroundRole);
            if (foreground.canConvert<QBrush>()) {
                pen_color = qvariant_cast<QBrush>(foreground).color();
            }
            if (!pen_color.isValid()) {
                pen_color = option.palette.color(QPalette::Text);
            }
        }

        painter->save();
        QPen pen(pen_color);
        pen.setWidthF(1.0);
        pen.setCapStyle(Qt::FlatCap);
        painter->setPen(pen);

        for (int i = 0; i < text_layout.lineCount(); ++i) {
            const QTextLine line = text_layout.lineAt(i);
            const int line_start = line.textStart();
            const int line_end = line_start + line.textLength();

            const int seg_start = qMax(title_pos, line_start);
            const int seg_end = qMin(title_end, line_end);

            if (seg_start < seg_end) {
                const QPointF line_origin = layout_rect.topLeft() + line.position();
                qreal start_x = line_origin.x() + line.cursorToX(seg_start);
                qreal end_x = line_origin.x() + line.cursorToX(seg_end);
                if (start_x > end_x) {
                    std::swap(start_x, end_x);
                }

                start_x = qMax<qreal>(start_x, text_rect.left());
                end_x = qMin<qreal>(end_x, text_rect.right());

                if (end_x > start_x) {
                    const qreal baseline = line_origin.y() + line.ascent();
                    if (draw_underline) {
                        const qreal underline_y = baseline + underline_offset;
                        painter->drawLine(QPointF(start_x, underline_y), QPointF(end_x, underline_y));
                    }
                    if (draw_strike_out) {
                        const qreal strike_y = baseline - strike_out_offset;
                        painter->drawLine(QPointF(start_x, strike_y), QPointF(end_x, strike_y));
                    }
                }
            }
        }

        painter->restore();
    }
};

}  // namespace

TaskTitleFontSettings default_general_task_title_font_settings()
{
    return {};
}

TaskTitleFontSettings load_general_task_title_font_settings()
{
    return load_task_title_font_settings(general_task_title_font_config_path(),
                                         default_general_task_title_font_settings());
}

bool save_general_task_title_font_settings(const TaskTitleFontSettings &settings)
{
    return save_task_title_font_settings(general_task_title_font_config_path(), settings);
}

TaskTitleFontSettings default_linked_file_title_font_settings()
{
    TaskTitleFontSettings settings;
    settings.bold = true;
    settings.underline = true;
    return settings;
}

TaskTitleFontSettings load_linked_file_title_font_settings()
{
    return load_task_title_font_settings(linked_file_title_font_config_path(),
                                         default_linked_file_title_font_settings());
}

bool save_linked_file_title_font_settings(const TaskTitleFontSettings &settings)
{
    return save_task_title_font_settings(linked_file_title_font_config_path(), settings);
}

TaskTitleFontSettings load_plugin_general_task_title_font_settings(PluginFontContext context)
{
    return load_task_title_font_settings(plugin_general_task_title_font_config_path(context),
                                         default_general_task_title_font_settings());
}

bool save_plugin_general_task_title_font_settings(PluginFontContext context, const TaskTitleFontSettings &settings)
{
    return save_task_title_font_settings(plugin_general_task_title_font_config_path(context), settings);
}

TaskTitleFontSettings load_plugin_linked_file_title_font_settings(PluginFontContext context)
{
    return load_task_title_font_settings(plugin_linked_file_title_font_config_path(context),
                                         default_linked_file_title_font_settings());
}

bool save_plugin_linked_file_title_font_settings(PluginFontContext context, const TaskTitleFontSettings &settings)
{
    return save_task_title_font_settings(plugin_linked_file_title_font_config_path(context), settings);
}

QVariantMap task_title_font_settings_to_map(const TaskTitleFontSettings &settings)
{
    QVariantMap map;
    map.insert(QStringLiteral("family"), settings.family.trimmed());
    map.insert(QStringLiteral("style"), settings.style.trimmed());
    map.insert(QStringLiteral("point_size"), settings.point_size);
    map.insert(QStringLiteral("bold"), settings.bold);
    map.insert(QStringLiteral("italic"), settings.italic);
    map.insert(QStringLiteral("underline"), settings.underline);
    map.insert(QStringLiteral("strike_out"), settings.strike_out);
    map.insert(QStringLiteral("is_custom"), true);
    return map;
}

TaskTitleFontSettings task_title_font_settings_from_map(const QVariantMap &map, const TaskTitleFontSettings &defaults)
{
    TaskTitleFontSettings settings = defaults;
    if (map.isEmpty()) {
        return settings;
    }
    settings.family = map.value(QStringLiteral("family"), settings.family).toString().trimmed();
    settings.style = map.value(QStringLiteral("style"), settings.style).toString().trimmed();
    const int point_size = map.value(QStringLiteral("point_size"), settings.point_size).toInt();
    settings.point_size = (point_size >= 6 && point_size <= 72) ? point_size : 0;
    settings.bold = map.value(QStringLiteral("bold"), settings.bold).toBool();
    settings.italic = map.value(QStringLiteral("italic"), settings.italic).toBool();
    settings.underline = map.value(QStringLiteral("underline"), settings.underline).toBool();
    settings.strike_out = map.value(QStringLiteral("strike_out"), settings.strike_out).toBool();
    return settings;
}

bool item_has_custom_font_settings(const QListWidgetItem *item)
{
    if (item == nullptr) {
        return false;
    }
    QVariantMap map = item->data(kCustomFontSettingsRole).toMap();
    if (map.isEmpty()) {
        const QVariant user_data = item->data(Qt::UserRole);
        if (user_data.canConvert<QVariantMap>()) {
            map = user_data.toMap().value(QStringLiteral("font_config")).toMap();
        }
    }
    return !map.isEmpty() && (map.contains(QStringLiteral("family")) || map.value(QStringLiteral("is_custom")).toBool());
}

TaskTitleFontSettings get_item_current_font_settings(const QListWidgetItem *item, bool has_linked_file, PluginFontContext context)
{
    const TaskTitleFontSettings global_settings =
        has_linked_file ? load_plugin_linked_file_title_font_settings(context) : load_plugin_general_task_title_font_settings(context);

    if (item == nullptr) {
        return global_settings;
    }

    QVariantMap map = item->data(kCustomFontSettingsRole).toMap();
    if (map.isEmpty()) {
        const QVariant user_data = item->data(Qt::UserRole);
        if (user_data.canConvert<QVariantMap>()) {
            map = user_data.toMap().value(QStringLiteral("font_config")).toMap();
        }
    }

    if (!map.isEmpty() && (map.contains(QStringLiteral("family")) || map.value(QStringLiteral("is_custom")).toBool())) {
        return task_title_font_settings_from_map(map, global_settings);
    }
    return global_settings;
}

void set_item_custom_font_settings(QListWidgetItem *item, const TaskTitleFontSettings &settings)
{
    if (item == nullptr) {
        return;
    }
    const QVariantMap map = task_title_font_settings_to_map(settings);
    item->setData(kCustomFontSettingsRole, map);

    const QVariant user_data = item->data(Qt::UserRole);
    QVariantMap user_map = user_data.canConvert<QVariantMap>() ? user_data.toMap() : QVariantMap();
    user_map.insert(QStringLiteral("font_config"), map);
    item->setData(Qt::UserRole, user_map);
}

void clear_item_custom_font_settings(QListWidgetItem *item)
{
    if (item == nullptr) {
        return;
    }
    item->setData(kCustomFontSettingsRole, QVariantMap());

    const QVariant user_data = item->data(Qt::UserRole);
    if (user_data.canConvert<QVariantMap>()) {
        QVariantMap user_map = user_data.toMap();
        user_map.remove(QStringLiteral("font_config"));
        item->setData(Qt::UserRole, user_map);
    }
}

QFont task_title_font(const QFont &base_font, const TaskTitleFontSettings &settings)
{
    QFont font = base_font;
    if (!settings.family.trimmed().isEmpty()) {
        font.setFamily(settings.family.trimmed());
    }
    if (settings.point_size >= 6 && settings.point_size <= 72) {
        font.setPointSize(settings.point_size);
    }
    if (!settings.style.trimmed().isEmpty()) {
        font.setStyleName(settings.style.trimmed());
    }
    font.setBold(settings.bold);
    font.setItalic(settings.italic);
    font.setUnderline(false);
    font.setStrikeOut(false);
    return font;
}

QFont linked_file_title_font(const QFont &base_font, const TaskTitleFontSettings &settings)
{
    return task_title_font(base_font, settings);
}

void apply_task_title_font(QListWidgetItem *item, const QString &title, bool has_linked_file, PluginFontContext context)
{
    if (item == nullptr) {
        return;
    }

    const TaskTitleFontSettings settings = get_item_current_font_settings(item, has_linked_file, context);
    item->setFont(task_title_font(base_font_for_item(item), settings));

    QString clean_title = title.trimmed();
    if (clean_title.isEmpty()) {
        clean_title = item->data(kLinkedTitleUnderlineTextRole).toString().trimmed();
        if (clean_title.isEmpty() && item->text().contains(QLatin1Char('\n'))) {
            clean_title = item->text().section(QLatin1Char('\n'), 0, 0).trimmed();
        } else if (clean_title.isEmpty()) {
            clean_title = item->text().trimmed();
        }
    }

    const bool active = !clean_title.isEmpty() && (settings.underline || settings.strike_out);
    item->setData(kLinkedTitleUnderlineRole, active);
    item->setData(kLinkedTitleUnderlineTextRole, !clean_title.isEmpty() ? clean_title : QString());
    item->setData(kLinkedTitleUnderlineVisibleRole, settings.underline);
    item->setData(kLinkedTitleStrikeOutVisibleRole, settings.strike_out);
    item->setData(kHasLinkedFileRole, has_linked_file);
}

void apply_linked_file_title_font(QListWidgetItem *item, bool enabled, PluginFontContext context)
{
    if (item == nullptr) {
        return;
    }

    apply_task_title_font(item, item->data(kLinkedTitleUnderlineTextRole).toString(), enabled, context);
}

void install_linked_title_underline_delegate(QAbstractItemView *view)
{
    if (view == nullptr || dynamic_cast<LinkedTitleUnderlineDelegate *>(view->itemDelegate()) != nullptr) {
        return;
    }

    view->setItemDelegate(new LinkedTitleUnderlineDelegate(view));
}

void set_linked_title_underline(QListWidgetItem *item, const QString &title, bool enabled)
{
    if (item == nullptr) {
        return;
    }

    const QString clean_title = title.trimmed();
    const bool active = enabled && !clean_title.isEmpty();
    item->setData(kLinkedTitleUnderlineRole, active);
    item->setData(kLinkedTitleUnderlineTextRole, active ? clean_title : QString());
    item->setData(kLinkedTitleUnderlineVisibleRole, active);
    item->setData(kLinkedTitleStrikeOutVisibleRole, false);
}

namespace {
static QString tr_font_dialog(PluginFontContext context, const QString &text)
{
    QString translated;
    switch (context) {
    case PluginFontContext::Pomodoro:
        translated = QCoreApplication::translate("PomodoroPlugin", text.toUtf8().constData());
        break;
    case PluginFontContext::Feynman:
        translated = QCoreApplication::translate("FeynmanPlugin", text.toUtf8().constData());
        break;
    case PluginFontContext::MapaMental:
        translated = QCoreApplication::translate("MapaMentalPlugin", text.toUtf8().constData());
        break;
    case PluginFontContext::Global:
    default:
        translated = QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
        break;
    }
    if (translated.isEmpty() || translated == text) {
        const QString fallback = QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
        if (!fallback.isEmpty()) {
            return fallback;
        }
    }
    return translated;
}

static QString translate_font_style(const QString &style_name, PluginFontContext context)
{
    const QString trimmed = style_name.trimmed();
    if (trimmed.isEmpty()) {
        return trimmed;
    }

    const QString qt_tr = tr_font_dialog(context, trimmed);
    if (!qt_tr.isEmpty() && qt_tr != trimmed) {
        return qt_tr;
    }

    const QString font_settings_tr = tr_font_dialog(context, QStringLiteral("Configurações de Fonte"));
    const bool is_english = (font_settings_tr == QStringLiteral("Font Settings"));

    if (is_english) {
        return trimmed;
    }

    static const QMap<QString, QString> exact_pt = {
        {QStringLiteral("Normal"), QStringLiteral("Normal")},
        {QStringLiteral("Regular"), QStringLiteral("Normal")},
        {QStringLiteral("Bold"), QStringLiteral("Negrito")},
        {QStringLiteral("Italic"), QStringLiteral("Itálico")},
        {QStringLiteral("Bold Italic"), QStringLiteral("Negrito Itálico")},
        {QStringLiteral("Oblique"), QStringLiteral("Oblíquo")},
        {QStringLiteral("Bold Oblique"), QStringLiteral("Negrito Oblíquo")},
        {QStringLiteral("BoldOblique"), QStringLiteral("Negrito Oblíquo")},
        {QStringLiteral("Slanted"), QStringLiteral("Inclinado")},
        {QStringLiteral("Upright"), QStringLiteral("Vertical")},
        {QStringLiteral("Roman"), QStringLiteral("Romano")},
        {QStringLiteral("Black"), QStringLiteral("Preto")},
        {QStringLiteral("Black Italic"), QStringLiteral("Preto Itálico")},
        {QStringLiteral("Light"), QStringLiteral("Fina")},
        {QStringLiteral("Light Italic"), QStringLiteral("Fina Itálico")},
        {QStringLiteral("Semilight"), QStringLiteral("Semileve")},
        {QStringLiteral("SemiLight"), QStringLiteral("Semileve")},
        {QStringLiteral("Semi Light"), QStringLiteral("Semileve")},
        {QStringLiteral("Semi-Light"), QStringLiteral("Semileve")},
        {QStringLiteral("Semilight Italic"), QStringLiteral("Semileve Itálico")},
        {QStringLiteral("SemiLight Italic"), QStringLiteral("Semileve Itálico")},
        {QStringLiteral("Semi Light Italic"), QStringLiteral("Semileve Itálico")},
        {QStringLiteral("Semi-Light Italic"), QStringLiteral("Semileve Itálico")},
        {QStringLiteral("Medium"), QStringLiteral("Média")},
        {QStringLiteral("Medium Italic"), QStringLiteral("Média Itálico")},
        {QStringLiteral("Thin"), QStringLiteral("Muito Fina")},
        {QStringLiteral("Thin Italic"), QStringLiteral("Muito Fina Itálico")},
        {QStringLiteral("Hairline"), QStringLiteral("Ultrafina")},
        {QStringLiteral("Hair"), QStringLiteral("Ultrafina")},
        {QStringLiteral("Hair Italic"), QStringLiteral("Ultrafina Itálico")},
        {QStringLiteral("Extra Light"), QStringLiteral("Extra Fina")},
        {QStringLiteral("ExtraLight"), QStringLiteral("Extra Fina")},
        {QStringLiteral("Extralight"), QStringLiteral("Extra Fina")},
        {QStringLiteral("Extra-Light"), QStringLiteral("Extra Fina")},
        {QStringLiteral("Extra Light Italic"), QStringLiteral("Extra Fina Itálico")},
        {QStringLiteral("ExtraLight Italic"), QStringLiteral("Extra Fina Itálico")},
        {QStringLiteral("Extralight Italic"), QStringLiteral("Extra Fina Itálico")},
        {QStringLiteral("Extra-Light Italic"), QStringLiteral("Extra Fina Itálico")},
        {QStringLiteral("Semibold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("SemiBold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Semi Bold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Semi-Bold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Semibold Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("SemiBold Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("Semi Bold Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("Semi-Bold Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("Demi"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Demibold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Demi Bold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Demi-Bold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Demi Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("Demibold Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("Demi Bold Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("Demi-Bold Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("Demibold Roman"), QStringLiteral("Seminegrito Romano")},
        {QStringLiteral("Extra Bold"), QStringLiteral("Extra Negrito")},
        {QStringLiteral("ExtraBold"), QStringLiteral("Extra Negrito")},
        {QStringLiteral("Extrabold"), QStringLiteral("Extra Negrito")},
        {QStringLiteral("Extra-Bold"), QStringLiteral("Extra Negrito")},
        {QStringLiteral("Extra Bold Italic"), QStringLiteral("Extra Negrito Itálico")},
        {QStringLiteral("ExtraBold Italic"), QStringLiteral("Extra Negrito Itálico")},
        {QStringLiteral("Extrabold Italic"), QStringLiteral("Extra Negrito Itálico")},
        {QStringLiteral("Extra-Bold Italic"), QStringLiteral("Extra Negrito Itálico")},
        {QStringLiteral("Ultra Light"), QStringLiteral("Ultrafina")},
        {QStringLiteral("UltraLight"), QStringLiteral("Ultrafina")},
        {QStringLiteral("Ultralight"), QStringLiteral("Ultrafina")},
        {QStringLiteral("Ultra-Light"), QStringLiteral("Ultrafina")},
        {QStringLiteral("Ultra Light Italic"), QStringLiteral("Ultrafina Itálico")},
        {QStringLiteral("UltraLight Italic"), QStringLiteral("Ultrafina Itálico")},
        {QStringLiteral("Ultralight Italic"), QStringLiteral("Ultrafina Itálico")},
        {QStringLiteral("Ultra-Light Italic"), QStringLiteral("Ultrafina Itálico")},
        {QStringLiteral("Ultra Bold"), QStringLiteral("Ultra Negrito")},
        {QStringLiteral("UltraBold"), QStringLiteral("Ultra Negrito")},
        {QStringLiteral("Ultrabold"), QStringLiteral("Ultra Negrito")},
        {QStringLiteral("Ultra-Bold"), QStringLiteral("Ultra Negrito")},
        {QStringLiteral("Ultra Bold Italic"), QStringLiteral("Ultra Negrito Itálico")},
        {QStringLiteral("UltraBold Italic"), QStringLiteral("Ultra Negrito Itálico")},
        {QStringLiteral("Ultrabold Italic"), QStringLiteral("Ultra Negrito Itálico")},
        {QStringLiteral("Ultra-Bold Italic"), QStringLiteral("Ultra Negrito Itálico")},
        {QStringLiteral("Heavy"), QStringLiteral("Pesada")},
        {QStringLiteral("Heavy Italic"), QStringLiteral("Pesada Itálico")},
        {QStringLiteral("Book"), QStringLiteral("Livro")},
        {QStringLiteral("Book Italic"), QStringLiteral("Livro Itálico")},
        {QStringLiteral("Bold Outline"), QStringLiteral("Negrito Contorno")},
        {QStringLiteral("Condensed"), QStringLiteral("Condensada")},
        {QStringLiteral("Condensed Italic"), QStringLiteral("Condensada Itálico")},
        {QStringLiteral("Condensed Bold"), QStringLiteral("Negrito Condensado")},
        {QStringLiteral("Bold Condensed"), QStringLiteral("Negrito Condensado")},
        {QStringLiteral("Condensed Bold Italic"), QStringLiteral("Negrito Condensado Itálico")},
        {QStringLiteral("Bold Condensed Italic"), QStringLiteral("Negrito Condensado Itálico")},
        {QStringLiteral("Condensed Extra Bold"), QStringLiteral("Condensada Extra Negrito")},
        {QStringLiteral("Condensed Ultra Bold"), QStringLiteral("Condensada Ultra Negrito")},
        {QStringLiteral("Light Condensed"), QStringLiteral("Fina Condensada")},
        {QStringLiteral("SemiCondensed"), QStringLiteral("Semicondensada")},
        {QStringLiteral("Semi Condensed"), QStringLiteral("Semicondensada")},
        {QStringLiteral("Semi-Condensed"), QStringLiteral("Semicondensada")},
        {QStringLiteral("Bold SemiCondensed"), QStringLiteral("Negrito Semicondensado")},
        {QStringLiteral("Bold Semi Condensed"), QStringLiteral("Negrito Semicondensado")},
        {QStringLiteral("Light SemiCondensed"), QStringLiteral("Fina Semicondensada")},
        {QStringLiteral("Light Semi Condensed"), QStringLiteral("Fina Semicondensada")},
        {QStringLiteral("SemiBold Condensed"), QStringLiteral("Seminegrito Condensado")},
        {QStringLiteral("SemiBold SemiCondensed"), QStringLiteral("Seminegrito Semicondensado")},
        {QStringLiteral("SemiLight Condensed"), QStringLiteral("Semileve Condensada")},
        {QStringLiteral("SemiLight SemiCondensed"), QStringLiteral("Semileve Semicondensada")},
        {QStringLiteral("Ext Condensed Bold"), QStringLiteral("Extra Condensada Negrito")},
        {QStringLiteral("Extra Condensed"), QStringLiteral("Extra Condensada")},
        {QStringLiteral("Extra-Condensed"), QStringLiteral("Extra Condensada")},
        {QStringLiteral("Ultra Condensed"), QStringLiteral("Ultra Condensada")},
        {QStringLiteral("Ultra-Condensed"), QStringLiteral("Ultra Condensada")},
        {QStringLiteral("Cond Demi"), QStringLiteral("Condensada Seminegrito")},
        {QStringLiteral("Cond Medium"), QStringLiteral("Condensada Média")},
        {QStringLiteral("Compressed"), QStringLiteral("Comprimida")},
        {QStringLiteral("Compressed Italic"), QStringLiteral("Comprimida Itálico")},
        {QStringLiteral("Poster Compressed"), QStringLiteral("Poster Comprimida")},
        {QStringLiteral("Narrow"), QStringLiteral("Estreita")},
        {QStringLiteral("Narrow Italic"), QStringLiteral("Estreita Itálico")},
        {QStringLiteral("Narrow Bold"), QStringLiteral("Negrito Estreito")},
        {QStringLiteral("Narrow Bold Italic"), QStringLiteral("Negrito Estreito Itálico")},
        {QStringLiteral("Expanded"), QStringLiteral("Expandida")},
        {QStringLiteral("Expanded Italic"), QStringLiteral("Expandida Itálico")},
        {QStringLiteral("Extended"), QStringLiteral("Estendida")},
        {QStringLiteral("Extended Italic"), QStringLiteral("Estendida Itálico")},
        {QStringLiteral("SemiExpanded Italic"), QStringLiteral("Semiexpandida Itálico")},
        {QStringLiteral("Semi Expanded"), QStringLiteral("Semiexpandida")},
        {QStringLiteral("Semi-Expanded"), QStringLiteral("Semiexpandida")},
        {QStringLiteral("Extra Expanded"), QStringLiteral("Extra Expandida")},
        {QStringLiteral("Extra-Expanded"), QStringLiteral("Extra Expandida")},
        {QStringLiteral("Ultra Expanded"), QStringLiteral("Ultra Expandida")},
        {QStringLiteral("Ultra-Expanded"), QStringLiteral("Ultra Expandida")},
        {QStringLiteral("Small"), QStringLiteral("Pequena")},
        {QStringLiteral("Small Italic"), QStringLiteral("Pequena Itálico")},
        {QStringLiteral("Small Bold"), QStringLiteral("Pequena Negrito")},
        {QStringLiteral("Small Bold Italic"), QStringLiteral("Pequena Negrito Itálico")},
        {QStringLiteral("Small Semibold"), QStringLiteral("Pequena Seminegrito")},
        {QStringLiteral("Small Semibold Italic"), QStringLiteral("Pequena Seminegrito Itálico")},
        {QStringLiteral("Small Light"), QStringLiteral("Pequena Fina")},
        {QStringLiteral("Small Semilight"), QStringLiteral("Pequena Semileve")},
        {QStringLiteral("Text"), QStringLiteral("Texto")},
        {QStringLiteral("Text Italic"), QStringLiteral("Texto Itálico")},
        {QStringLiteral("Text Bold"), QStringLiteral("Texto Negrito")},
        {QStringLiteral("Text Bold Italic"), QStringLiteral("Texto Negrito Itálico")},
        {QStringLiteral("Text Semibold"), QStringLiteral("Texto Seminegrito")},
        {QStringLiteral("Text Semibold Italic"), QStringLiteral("Texto Seminegrito Itálico")},
        {QStringLiteral("Text Light"), QStringLiteral("Texto Fina")},
        {QStringLiteral("Text Semilight"), QStringLiteral("Texto Semileve")},
        {QStringLiteral("Heading"), QStringLiteral("Título")},
        {QStringLiteral("Heading Italic"), QStringLiteral("Título Itálico")},
        {QStringLiteral("Heading Bold"), QStringLiteral("Título Negrito")},
        {QStringLiteral("Heading Bold Italic"), QStringLiteral("Título Negrito Itálico")},
        {QStringLiteral("Heading Semibold"), QStringLiteral("Título Seminegrito")},
        {QStringLiteral("Heading Semibold Italic"), QStringLiteral("Título Seminegrito Itálico")},
        {QStringLiteral("Subheading"), QStringLiteral("Subtítulo")},
        {QStringLiteral("Subheading Italic"), QStringLiteral("Subtítulo Itálico")},
        {QStringLiteral("Subheading Bold"), QStringLiteral("Subtítulo Negrito")},
        {QStringLiteral("Subheading Bold Italic"), QStringLiteral("Subtítulo Negrito Itálico")},
        {QStringLiteral("Subheading Semibold"), QStringLiteral("Subtítulo Seminegrito")},
        {QStringLiteral("Subheading Semibold Italic"), QStringLiteral("Subtítulo Seminegrito Itálico")},
        {QStringLiteral("Display"), QStringLiteral("Exibição")},
        {QStringLiteral("Display Italic"), QStringLiteral("Exibição Itálico")},
        {QStringLiteral("Display Bold"), QStringLiteral("Exibição Negrito")},
        {QStringLiteral("Display Bold Italic"), QStringLiteral("Exibição Negrito Itálico")},
        {QStringLiteral("Display Semibold"), QStringLiteral("Exibição Seminegrito")},
        {QStringLiteral("Display Semibold Italic"), QStringLiteral("Exibição Seminegrito Itálico")},
        {QStringLiteral("Display Light"), QStringLiteral("Exibição Fina")},
        {QStringLiteral("Display Semilight"), QStringLiteral("Exibição Semileve")},
        {QStringLiteral("Banner"), QStringLiteral("Faixa")},
        {QStringLiteral("Banner Italic"), QStringLiteral("Faixa Itálico")},
        {QStringLiteral("Banner Bold"), QStringLiteral("Faixa Negrito")},
        {QStringLiteral("Banner Bold Italic"), QStringLiteral("Faixa Negrito Itálico")},
        {QStringLiteral("Banner Semibold"), QStringLiteral("Faixa Seminegrito")},
        {QStringLiteral("Banner Semibold Italic"), QStringLiteral("Faixa Seminegrito Itálico")},
        {QStringLiteral("Caption"), QStringLiteral("Legenda")},
        {QStringLiteral("Poster"), QStringLiteral("Poster")},
        {QStringLiteral("Headline"), QStringLiteral("Manchete")},
        {QStringLiteral("Subhead"), QStringLiteral("Subtítulo")},
        {QStringLiteral("18pt"), QStringLiteral("18pt")},
        {QStringLiteral("18pt Bold"), QStringLiteral("18pt Negrito")},
        {QStringLiteral("18pt Light"), QStringLiteral("18pt Fina")},
        {QStringLiteral("18pt Medium"), QStringLiteral("18pt Média")},
        {QStringLiteral("18pt Thin"), QStringLiteral("18pt Muito Fina")}
    };

    auto it = exact_pt.constFind(trimmed);
    if (it != exact_pt.constEnd()) {
        return it.value();
    }
    for (auto map_it = exact_pt.constBegin(); map_it != exact_pt.constEnd(); ++map_it) {
        if (map_it.key().compare(trimmed, Qt::CaseInsensitive) == 0) {
            return map_it.value();
        }
    }

    struct ReplacementRule {
        const char *en;
        const char *pt;
    };
    static const ReplacementRule rules[] = {
        {"Ext Condensed", "Extra Condensada"},
        {"Ultra Light", "Ultrafina"},
        {"Ultra-Light", "Ultrafina"},
        {"Ultralight", "Ultrafina"},
        {"Extra Light", "Extra Fina"},
        {"Extra-Light", "Extra Fina"},
        {"ExtraLight", "Extra Fina"},
        {"Extralight", "Extra Fina"},
        {"Ultra Bold", "Ultra Negrito"},
        {"Ultra-Bold", "Ultra Negrito"},
        {"UltraBold", "Ultra Negrito"},
        {"Ultrabold", "Ultra Negrito"},
        {"Extra Bold", "Extra Negrito"},
        {"Extra-Bold", "Extra Negrito"},
        {"ExtraBold", "Extra Negrito"},
        {"Extrabold", "Extra Negrito"},
        {"Semi Bold", "Seminegrito"},
        {"Semi-Bold", "Seminegrito"},
        {"SemiBold", "Seminegrito"},
        {"Semibold", "Seminegrito"},
        {"Semi Light", "Semileve"},
        {"Semi-Light", "Semileve"},
        {"SemiLight", "Semileve"},
        {"Semilight", "Semileve"},
        {"Demi Bold", "Seminegrito"},
        {"Demi-Bold", "Seminegrito"},
        {"Demibold", "Seminegrito"},
        {"Semi Condensed", "Semicondensada"},
        {"Semi-Condensed", "Semicondensada"},
        {"SemiCondensed", "Semicondensada"},
        {"Semicondensed", "Semicondensada"},
        {"Extra Condensed", "Extra Condensada"},
        {"Extra-Condensed", "Extra Condensada"},
        {"Extracondensed", "Extra Condensada"},
        {"Ultra Condensed", "Ultra Condensada"},
        {"Ultra-Condensed", "Ultra Condensada"},
        {"Ultracondensed", "Ultra Condensada"},
        {"Semi Expanded", "Semiexpandida"},
        {"Semi-Expanded", "Semiexpandida"},
        {"SemiExpanded", "Semiexpandida"},
        {"Extra Expanded", "Extra Expandida"},
        {"Extra-Expanded", "Extra Expandida"},
        {"Extraexpanded", "Extra Expandida"},
        {"Ultra Expanded", "Ultra Expandida"},
        {"Ultra-Expanded", "Ultra Expandida"},
        {"Ultraexpanded", "Ultra Expandida"},
        {"Bold Outline", "Negrito Contorno"},
        {"Cond", "Condensada"},
        {"Ext", "Extra"},
        {"Subheading", "Subtítulo"},
        {"Thin", "Muito Fina"},
        {"Hairline", "Ultrafina"},
        {"Hair", "Ultrafina"},
        {"Light", "Fina"},
        {"Medium", "Média"},
        {"Regular", "Normal"},
        {"Normal", "Normal"},
        {"Demi", "Seminegrito"},
        {"Bold", "Negrito"},
        {"Black", "Preto"},
        {"Heavy", "Pesada"},
        {"Book", "Livro"},
        {"Roman", "Romano"},
        {"Italic", "Itálico"},
        {"Oblique", "Oblíquo"},
        {"Slanted", "Inclinado"},
        {"Upright", "Vertical"},
        {"Outline", "Contorno"},
        {"Condensed", "Condensada"},
        {"Compressed", "Comprimida"},
        {"Narrow", "Estreita"},
        {"Expanded", "Expandida"},
        {"Extended", "Estendida"},
        {"Small", "Pequena"},
        {"Text", "Texto"},
        {"Display", "Exibição"},
        {"Banner", "Faixa"},
        {"Heading", "Título"},
        {"Subhead", "Subtítulo"},
        {"Caption", "Legenda"},
        {"Poster", "Poster"},
        {"Headline", "Manchete"}
    };

    QString result = trimmed;
    for (const auto &r : rules) {
        const QString pattern = QString::fromUtf8(r.en);
        const QString replacement = QString::fromUtf8(r.pt);
        result.replace(QRegularExpression(QStringLiteral("\\b") + QRegularExpression::escape(pattern) + QStringLiteral("\\b"),
                                           QRegularExpression::CaseInsensitiveOption),
                       replacement);
    }
    return result;
}

class CustomItemFontDialog final : public QDialog {
public:
    CustomItemFontDialog(QWidget *parent,
                         const QString &title_text,
                         const QString &preview_text,
                         const TaskTitleFontSettings &initial_settings,
                         const TaskTitleFontSettings &default_settings,
                         PluginFontContext context = PluginFontContext::Global)
        : QDialog(parent)
        , title_text_(title_text)
        , preview_text_(preview_text.trimmed().isEmpty() ? tr_font_dialog(context, QStringLiteral("Exemplo de Tarefa Selecionada")) : preview_text.trimmed())
        , defaults_(default_settings)
        , context_(context)
    {
        setModal(true);
        setMinimumWidth(480);

        auto *main_layout = new QVBoxLayout(this);
        auto *form_layout = new QGridLayout();
        form_layout->setColumnStretch(1, 1);

        family_label_ = new QLabel(this);
        family_combo_ = new QFontComboBox(this);

        style_label_ = new QLabel(this);
        style_combo_ = new QComboBox(this);

        size_label_ = new QLabel(this);
        size_spin_ = new QSpinBox(this);
        size_spin_->setRange(6, 72);
        size_spin_->setSuffix(QStringLiteral(" pt"));

        format_label_ = new QLabel(this);
        bold_check_ = new QCheckBox(this);
        italic_check_ = new QCheckBox(this);
        underline_check_ = new QCheckBox(this);
        strike_out_check_ = new QCheckBox(this);

        auto *format_layout = new QHBoxLayout();
        format_layout->addWidget(bold_check_);
        format_layout->addWidget(italic_check_);
        format_layout->addWidget(underline_check_);
        format_layout->addWidget(strike_out_check_);
        format_layout->addStretch();

        form_layout->addWidget(family_label_, 0, 0);
        form_layout->addWidget(family_combo_, 0, 1);
        form_layout->addWidget(style_label_, 1, 0);
        form_layout->addWidget(style_combo_, 1, 1);
        form_layout->addWidget(size_label_, 2, 0);
        form_layout->addWidget(size_spin_, 2, 1);
        form_layout->addWidget(format_label_, 3, 0);
        form_layout->addLayout(format_layout, 3, 1);
        main_layout->addLayout(form_layout);

        preview_title_label_ = new QLabel(this);
        main_layout->addWidget(preview_title_label_);

        auto *preview_frame = new QFrame(this);
        preview_frame->setFrameShape(QFrame::StyledPanel);
        preview_frame->setFrameShadow(QFrame::Sunken);
        preview_frame->setMinimumHeight(80);
        auto *preview_layout = new QVBoxLayout(preview_frame);
        preview_label_ = new QLabel(this);
        preview_label_->setAlignment(Qt::AlignCenter);
        preview_layout->addWidget(preview_label_);
        main_layout->addWidget(preview_frame);

        auto *button_layout = new QHBoxLayout();
        restore_button_ = new QPushButton(this);
        ok_button_ = new QPushButton(this);
        cancel_button_ = new QPushButton(this);
        button_layout->addWidget(restore_button_);
        button_layout->addStretch();
        button_layout->addWidget(ok_button_);
        button_layout->addWidget(cancel_button_);
        main_layout->addLayout(button_layout);

        connect(family_combo_, &QFontComboBox::currentFontChanged, this, [this](const QFont &) {
            update_style_options(QString(), true);
            update_preview();
        });
        connect(style_combo_, &QComboBox::currentTextChanged, this, [this](const QString &) { update_preview(); });
        connect(size_spin_, &QSpinBox::valueChanged, this, [this](int) { update_preview(); });
        connect(bold_check_, &QCheckBox::toggled, this, [this](bool) { update_preview(); });
        connect(italic_check_, &QCheckBox::toggled, this, [this](bool) { update_preview(); });
        connect(underline_check_, &QCheckBox::toggled, this, [this](bool) { update_preview(); });
        connect(strike_out_check_, &QCheckBox::toggled, this, [this](bool) { update_preview(); });
        connect(restore_button_, &QPushButton::clicked, this, [this]() {
            set_settings(defaults_);
            update_preview();
        });
        connect(ok_button_, &QPushButton::clicked, this, &QDialog::accept);
        connect(cancel_button_, &QPushButton::clicked, this, &QDialog::reject);

        set_settings(initial_settings);
        retranslate_ui();
        update_preview();
    }

    TaskTitleFontSettings selected_settings() const
    {
        TaskTitleFontSettings settings;
        settings.family = family_combo_ != nullptr ? family_combo_->currentFont().family().trimmed() : QString();
        if (style_combo_ != nullptr) {
            const QString raw_style = style_combo_->currentData(Qt::UserRole).toString().trimmed();
            settings.style = !raw_style.isEmpty() ? raw_style : style_combo_->currentText().trimmed();
        }
        settings.point_size = size_spin_ != nullptr ? size_spin_->value() : 0;
        settings.bold = bold_check_ != nullptr && bold_check_->isChecked();
        settings.italic = italic_check_ != nullptr && italic_check_->isChecked();
        settings.underline = underline_check_ != nullptr && underline_check_->isChecked();
        settings.strike_out = strike_out_check_ != nullptr && strike_out_check_->isChecked();
        return settings;
    }

protected:
    void changeEvent(QEvent *event) override
    {
        if (event != nullptr && event->type() == QEvent::LanguageChange) {
            retranslate_ui();
        }
        QDialog::changeEvent(event);
    }

private:
    void retranslate_ui()
    {
        const QString raw_title = title_text_.trimmed().isEmpty() ? QStringLiteral("Configurações de Fonte") : title_text_;
        setWindowTitle(tr_font_dialog(context_, raw_title));
        if (family_label_ != nullptr) family_label_->setText(tr_font_dialog(context_, QStringLiteral("Tipo da fonte:")));
        if (style_label_ != nullptr) style_label_->setText(tr_font_dialog(context_, QStringLiteral("Estilo da fonte:")));
        if (size_label_ != nullptr) size_label_->setText(tr_font_dialog(context_, QStringLiteral("Tamanho:")));
        if (format_label_ != nullptr) format_label_->setText(tr_font_dialog(context_, QStringLiteral("Formatação:")));
        if (bold_check_ != nullptr) bold_check_->setText(tr_font_dialog(context_, QStringLiteral("Negrito")));
        if (italic_check_ != nullptr) italic_check_->setText(tr_font_dialog(context_, QStringLiteral("Itálico")));
        if (underline_check_ != nullptr) underline_check_->setText(tr_font_dialog(context_, QStringLiteral("Sublinhado")));
        if (strike_out_check_ != nullptr) strike_out_check_->setText(tr_font_dialog(context_, QStringLiteral("Tachado")));
        if (preview_title_label_ != nullptr) preview_title_label_->setText(tr_font_dialog(context_, QStringLiteral("Prévia")));
        if (restore_button_ != nullptr) restore_button_->setText(tr_font_dialog(context_, QStringLiteral("Restaurar")));
        if (ok_button_ != nullptr) ok_button_->setText(tr_font_dialog(context_, QStringLiteral("OK")));
        if (cancel_button_ != nullptr) cancel_button_->setText(tr_font_dialog(context_, QStringLiteral("Cancelar")));

        if (style_combo_ != nullptr) {
            const QSignalBlocker blocker(style_combo_);
            for (int i = 0; i < style_combo_->count(); ++i) {
                const QString raw_style = style_combo_->itemData(i, Qt::UserRole).toString();
                if (!raw_style.isEmpty()) {
                    style_combo_->setItemText(i, translate_font_style(raw_style, context_));
                }
            }
        }
        update_preview();
    }

    void update_style_options(const QString &preferred_style, bool keep_current_style_when_empty)
    {
        if (family_combo_ == nullptr || style_combo_ == nullptr) {
            return;
        }

        const QString trimmed_preferred_style = preferred_style.trimmed();
        const QString current_raw_style = style_combo_->currentData(Qt::UserRole).toString().trimmed();
        const QString target_style = trimmed_preferred_style.isEmpty() && keep_current_style_when_empty
                                         ? (current_raw_style.isEmpty() ? style_combo_->currentText().trimmed() : current_raw_style)
                                         : trimmed_preferred_style;
        const QString family = family_combo_->currentFont().family();
        QFontDatabase database;
        QStringList styles = database.styles(family);
        if (styles.isEmpty()) {
            styles << QStringLiteral("Normal");
        }

        const QSignalBlocker blocker(style_combo_);
        style_combo_->clear();
        for (const QString &raw_style : styles) {
            style_combo_->addItem(translate_font_style(raw_style, context_), raw_style);
        }

        int target_index = -1;
        if (!target_style.isEmpty()) {
            for (int i = 0; i < style_combo_->count(); ++i) {
                const QString item_raw = style_combo_->itemData(i, Qt::UserRole).toString();
                if (item_raw == target_style || style_combo_->itemText(i) == target_style) {
                    target_index = i;
                    break;
                }
            }
        }

        if (target_index >= 0) {
            style_combo_->setCurrentIndex(target_index);
        } else {
            int regular_index = -1;
            for (int i = 0; i < style_combo_->count(); ++i) {
                const QString item_raw = style_combo_->itemData(i, Qt::UserRole).toString();
                if (item_raw == QStringLiteral("Regular") || item_raw == QStringLiteral("Normal")) {
                    regular_index = i;
                    break;
                }
            }
            style_combo_->setCurrentIndex(regular_index >= 0 ? regular_index : 0);
        }
    }

    void set_settings(const TaskTitleFontSettings &settings)
    {
        const QFont base_font = QApplication::font();
        QFont selected_font = base_font;
        if (!settings.family.trimmed().isEmpty()) {
            selected_font.setFamily(settings.family.trimmed());
        }

        if (family_combo_ != nullptr) {
            const QSignalBlocker blocker(family_combo_);
            family_combo_->setCurrentFont(selected_font);
        }
        update_style_options(settings.style, false);

        if (size_spin_ != nullptr) {
            const QSignalBlocker blocker(size_spin_);
            int pt_size = settings.point_size;
            if (pt_size < 6 || pt_size > 72) {
                pt_size = selected_font.pointSize();
                if (pt_size <= 0) {
                    pt_size = 10;
                }
            }
            size_spin_->setValue(pt_size);
        }
        if (bold_check_ != nullptr) {
            const QSignalBlocker blocker(bold_check_);
            bold_check_->setChecked(settings.bold);
        }
        if (italic_check_ != nullptr) {
            const QSignalBlocker blocker(italic_check_);
            italic_check_->setChecked(settings.italic);
        }
        if (underline_check_ != nullptr) {
            const QSignalBlocker blocker(underline_check_);
            underline_check_->setChecked(settings.underline);
        }
        if (strike_out_check_ != nullptr) {
            const QSignalBlocker blocker(strike_out_check_);
            strike_out_check_->setChecked(settings.strike_out);
        }
    }

    void update_preview()
    {
        if (preview_label_ == nullptr) {
            return;
        }

        const TaskTitleFontSettings settings = selected_settings();
        QFont font = task_title_font(QApplication::font(), settings);
        font.setUnderline(settings.underline);
        font.setStrikeOut(settings.strike_out);
        preview_label_->setFont(font);
        preview_label_->setText(preview_text_);
    }

    QString title_text_;
    QString preview_text_;
    TaskTitleFontSettings defaults_;
    PluginFontContext context_ = PluginFontContext::Global;
    QLabel *family_label_ = nullptr;
    QLabel *style_label_ = nullptr;
    QLabel *size_label_ = nullptr;
    QLabel *format_label_ = nullptr;
    QLabel *preview_title_label_ = nullptr;
    QPushButton *restore_button_ = nullptr;
    QPushButton *ok_button_ = nullptr;
    QPushButton *cancel_button_ = nullptr;
    QFontComboBox *family_combo_ = nullptr;
    QComboBox *style_combo_ = nullptr;
    QSpinBox *size_spin_ = nullptr;
    QCheckBox *bold_check_ = nullptr;
    QCheckBox *italic_check_ = nullptr;
    QCheckBox *underline_check_ = nullptr;
    QCheckBox *strike_out_check_ = nullptr;
    QLabel *preview_label_ = nullptr;
};
}  // namespace

bool open_task_font_settings_dialog(QWidget *parent, const QString &dialog_title, const QString &preview_text, const TaskTitleFontSettings &initial_settings, const TaskTitleFontSettings &default_settings, TaskTitleFontSettings *out_settings, PluginFontContext context)
{
    CustomItemFontDialog dialog(parent, dialog_title, preview_text, initial_settings, default_settings, context);
    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }
    if (out_settings != nullptr) {
        *out_settings = dialog.selected_settings();
    }
    return true;
}

namespace {

bool check_item_has_linked_file(const QListWidgetItem *item)
{
    if (item == nullptr) {
        return false;
    }
    if (item->data(kHasLinkedFileRole).isValid()) {
        return item->data(kHasLinkedFileRole).toBool();
    }
    if (item->data(Qt::UserRole).canConvert<QVariantMap>()) {
        const QVariantMap m = item->data(Qt::UserRole).toMap();
        if (!m.value(QStringLiteral("arquivo_vinculado")).toString().trimmed().isEmpty() ||
            !m.value(QStringLiteral("file_path")).toString().trimmed().isEmpty()) {
            return true;
        }
    }
    return false;
}

}  // namespace

bool open_task_font_settings_dialog(QWidget *parent, const QList<QListWidgetItem *> &items, PluginFontContext context)
{
    if (items.isEmpty()) {
        return false;
    }
    QListWidgetItem *first_item = nullptr;
    for (QListWidgetItem *item : items) {
        if (item != nullptr) {
            first_item = item;
            break;
        }
    }
    if (first_item == nullptr) {
        return false;
    }

    QString preview_text = first_item->text().trimmed();
    if (preview_text.isEmpty()) {
        preview_text = first_item->data(kLinkedTitleUnderlineTextRole).toString().trimmed();
    }
    if (preview_text.isEmpty()) {
        preview_text = QStringLiteral("Exemplo de Tarefa Selecionada");
    } else if (preview_text.length() > 50) {
        preview_text = preview_text.left(47) + QStringLiteral("...");
    }

    const bool has_linked = check_item_has_linked_file(first_item);
    const TaskTitleFontSettings old_global = has_linked ? load_plugin_linked_file_title_font_settings(context)
                                                        : load_plugin_general_task_title_font_settings(context);

    const TaskTitleFontSettings initial_settings = get_item_current_font_settings(first_item, has_linked, context);
    const TaskTitleFontSettings default_settings = has_linked ? default_linked_file_title_font_settings() : default_general_task_title_font_settings();
    const QString dialog_title = has_linked ? QStringLiteral("Configurações de Fonte (Arquivo Vinculado)") : QStringLiteral("Configurações de Fonte (Tarefas em Geral)");

    TaskTitleFontSettings new_settings;
    if (!open_task_font_settings_dialog(parent, dialog_title, preview_text, initial_settings, default_settings, &new_settings, context)) {
        return false;
    }

    QWidget *top = parent ? parent->window() : nullptr;
    if (top == nullptr && first_item->listWidget() != nullptr) {
        top = first_item->listWidget()->window();
    }
    if (top != nullptr) {
        const QList<QListWidget *> list_widgets = top->findChildren<QListWidget *>();
        for (QListWidget *list : list_widgets) {
            if (list == nullptr) continue;
            for (int i = 0; i < list->count(); ++i) {
                QListWidgetItem *list_item = list->item(i);
                if (list_item != nullptr && !items.contains(list_item) && (list_item->flags() & Qt::ItemIsSelectable)) {
                    if (check_item_has_linked_file(list_item) == has_linked && !item_has_custom_font_settings(list_item)) {
                        set_item_custom_font_settings(list_item, old_global);
                        apply_task_title_font(list_item, QString(), has_linked, context);
                    }
                }
            }
        }
    }

    for (QListWidgetItem *item : items) {
        if (item != nullptr) {
            set_item_custom_font_settings(item, new_settings);
            const bool item_has_linked = check_item_has_linked_file(item);
            apply_task_title_font(item, QString(), item_has_linked, context);
        }
    }

    return true;
}

QColor choose_task_color(QWidget *parent, const QColor &initial_color, PluginFontContext context, const QString &dialog_title)
{
    QColorDialog dialog(initial_color.isValid() ? initial_color : QColor(QStringLiteral("#1f6feb")), parent);
    const QString window_title = dialog_title.trimmed().isEmpty()
                                     ? tr_font_dialog(context, QStringLiteral("Colorir tarefas"))
                                     : dialog_title;
    dialog.setWindowTitle(window_title);
    dialog.setOption(QColorDialog::DontUseNativeDialog, true);
    dialog.setOption(QColorDialog::ShowAlphaChannel, false);

    const int result = dialog.exec();
    return result == QDialog::Accepted ? dialog.selectedColor() : QColor();
}

}  // namespace source::utils
