#include "source/modules/feynman/Feynman/feynman_metodo_helpers.hpp"

#include <algorithm>

#include <QAbstractScrollArea>
#include <QApplication>
#include <QBrush>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QFont>
#include <QFontMetrics>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHash>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMargins>
#include <QPaintEvent>
#include <QPainter>
#include <QPalette>
#include <QRect>
#include <QScreen>
#include <QSizePolicy>
#include <QStringList>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QtGlobal>

#include "source/modules/feynman/Feynman/fey_19_RotatedTabButton.hpp"
#include "source/modules/feynman/Feynman/feynman_support.hpp"
#include "source/modules/feynman/mod_05_metodo_feynman.hpp"
#include "source/utils/AccentTabStyle.hpp"

namespace source::plugin::Feynman {
namespace {

QString normalized_priority_key(const QString &text)
{
    QString result = text.trimmed();
    const QStringList prefixes = {
        QStringLiteral("🔴 "),
        QStringLiteral("🟠 "),
        QStringLiteral("🟡 "),
        QStringLiteral("🟢 "),
        QStringLiteral("🔴"),
        QStringLiteral("🟠"),
        QStringLiteral("🟡"),
        QStringLiteral("🟢"),
    };

    bool removed_prefix = true;
    while (removed_prefix) {
        removed_prefix = false;
        for (const QString &prefix : prefixes) {
            if (result.startsWith(prefix)) {
                result.remove(0, prefix.size());
                result = result.trimmed();
                removed_prefix = true;
                break;
            }
        }
    }
    result = result.simplified().trimmed().toCaseFolded();
    result.remove(QLatin1Char(','));
    return result.simplified();
}

QString stored_description(const QVariantMap &conceito)
{
    return conceito.value(QStringLiteral("descricao")).toString().trimmed();
}

bool has_custom_description(const QVariantMap &conceito)
{
    return conceito.value(QStringLiteral("descricao_customizada")).toBool() || conceito.contains(QStringLiteral("descricao"));
}

bool is_external_integrated_concept(const QVariantMap &conceito)
{
    const QString origin = conceito.value(QStringLiteral("integracao_origem")).toString().trimmed().toCaseFolded();
    return !origin.isEmpty() && origin != QStringLiteral("feynman");
}

struct DescriptionToken {
    QString text;
    QString core;
};

QVector<DescriptionToken> description_tokens(const QString &text)
{
    QVector<DescriptionToken> tokens;
    QString pending_space;

    for (int i = 0; i < text.size();) {
        const QChar ch = text.at(i);
        if (ch.isSpace()) {
            pending_space.append(ch);
            ++i;
            continue;
        }

        QString core;
        while (i < text.size() && !text.at(i).isSpace()) {
            core.append(text.at(i));
            ++i;
        }
        tokens.append({pending_space + core, core});
        pending_space.clear();
    }

    if (!pending_space.isEmpty()) {
        if (tokens.isEmpty()) {
            tokens.append({pending_space, QString()});
        } else {
            tokens.last().text.append(pending_space);
        }
    }

    return tokens;
}

QString tokens_to_text(const QVector<DescriptionToken> &tokens)
{
    QString text;
    for (const DescriptionToken &token : tokens) {
        text.append(token.text);
    }
    return text.trimmed();
}

bool description_tokens_equal(const DescriptionToken &left, const DescriptionToken &right)
{
    return !left.core.isEmpty() && left.core == right.core;
}

QVector<QPair<int, int>> lcs_token_pairs(const QVector<DescriptionToken> &left,
                                         const QVector<DescriptionToken> &right)
{
    const int left_size = left.size();
    const int right_size = right.size();
    if (left_size == 0 || right_size == 0) {
        return {};
    }
    if (static_cast<qint64>(left_size) * static_cast<qint64>(right_size) > 4000000) {
        return {};
    }

    const int row_width = right_size + 1;
    QVector<int> dp((left_size + 1) * row_width, 0);
    const auto at = [row_width](int row, int column) { return row * row_width + column; };

    for (int i = left_size - 1; i >= 0; --i) {
        for (int j = right_size - 1; j >= 0; --j) {
            if (description_tokens_equal(left.at(i), right.at(j))) {
                dp[at(i, j)] = 1 + dp[at(i + 1, j + 1)];
            } else {
                dp[at(i, j)] = std::max(dp[at(i + 1, j)], dp[at(i, j + 1)]);
            }
        }
    }

    QVector<QPair<int, int>> pairs;
    int i = 0;
    int j = 0;
    while (i < left_size && j < right_size) {
        if (description_tokens_equal(left.at(i), right.at(j))) {
            pairs.append({i, j});
            ++i;
            ++j;
        } else if (dp[at(i + 1, j)] >= dp[at(i, j + 1)]) {
            ++i;
        } else {
            ++j;
        }
    }
    return pairs;
}

struct DescriptionEdit {
    int base_start = 0;
    int base_end = 0;
    QVector<DescriptionToken> replacement;
};

QVector<DescriptionEdit> description_edits(const QVector<DescriptionToken> &base,
                                           const QVector<DescriptionToken> &edited)
{
    QVector<DescriptionEdit> edits;
    QVector<QPair<int, int>> pairs = lcs_token_pairs(base, edited);
    const int base_size = static_cast<int>(base.size());
    const int edited_size = static_cast<int>(edited.size());
    if (base_size > 0 && edited_size > 0 && pairs.isEmpty()) {
        edits.append({0, base_size, edited});
        return edits;
    }

    pairs.append({base_size, edited_size});
    int previous_base = 0;
    int previous_edited = 0;

    for (const QPair<int, int> &pair : pairs) {
        if (pair.first > previous_base || pair.second > previous_edited) {
            QVector<DescriptionToken> replacement;
            for (int i = previous_edited; i < pair.second; ++i) {
                replacement.append(edited.at(i));
            }
            edits.append({previous_base, pair.first, replacement});
        }
        previous_base = pair.first + 1;
        previous_edited = pair.second + 1;
    }

    return edits;
}

int mapped_description_boundary(int boundary, const QVector<int> &base_to_target, int target_size)
{
    if (boundary <= 0) {
        return 0;
    }
    if (boundary >= base_to_target.size()) {
        return target_size;
    }

    for (int i = boundary; i < base_to_target.size(); ++i) {
        if (base_to_target.at(i) >= 0) {
            return base_to_target.at(i);
        }
    }
    for (int i = boundary - 1; i >= 0; --i) {
        if (base_to_target.at(i) >= 0) {
            return std::min(target_size, base_to_target.at(i) + 1);
        }
    }

    return qBound(0, boundary, target_size);
}

QString normalized_section_heading(const QString &text)
{
    QString heading = text.trimmed();
    while (heading.endsWith(QLatin1Char(':'))) {
        heading.chop(1);
        heading = heading.trimmed();
    }
    return heading.simplified().trimmed().toCaseFolded();
}

QString description_section_title(const QWidget *context, const QString &key)
{
    if (key == QStringLiteral("concept")) {
        return compat::tr(context, QStringLiteral("1️⃣ Conceito"), QStringLiteral("1️⃣ Concept"));
    }
    if (key == QStringLiteral("explanation")) {
        return compat::tr(context, QStringLiteral("2️⃣ Explique"), QStringLiteral("2️⃣ Explain"));
    }
    if (key == QStringLiteral("gaps")) {
        return compat::tr(context, QStringLiteral("3️⃣ Lacunas"), QStringLiteral("3️⃣ Gaps"));
    }
    if (key == QStringLiteral("review")) {
        return compat::tr(context, QStringLiteral("4️⃣ Revisão"), QStringLiteral("4️⃣ Review"));
    }
    return {};
}

QString description_section_key(const QString &line)
{
    const QString normalized = normalized_section_heading(line);
    const QList<QPair<QString, QStringList>> sections = {
        {QStringLiteral("concept"),
         {QStringLiteral("1️⃣ Conceito"), QStringLiteral("1️⃣ Concept"), QStringLiteral("Conceito"), QStringLiteral("Concept")}},
        {QStringLiteral("explanation"),
         {QStringLiteral("2️⃣ Explique"), QStringLiteral("2️⃣ Explain"), QStringLiteral("Explique"), QStringLiteral("Explain")}},
        {QStringLiteral("gaps"),
         {QStringLiteral("3️⃣ Lacunas"), QStringLiteral("3️⃣ Gaps"), QStringLiteral("Lacunas"), QStringLiteral("Gaps")}},
        {QStringLiteral("review"),
         {QStringLiteral("4️⃣ Revisão"), QStringLiteral("4️⃣ Review"), QStringLiteral("Revisão"), QStringLiteral("Review")}},
    };

    for (const auto &section : sections) {
        for (const QString &variant : section.second) {
            if (normalized == normalized_section_heading(variant)) {
                return section.first;
            }
        }
    }

    return {};
}

bool description_has_generated_sections(const QString &text)
{
    for (const QString &line : text.split(QLatin1Char('\n'))) {
        if (!description_section_key(line).isEmpty()) {
            return true;
        }
    }
    return false;
}

QString external_description_with_generated_sections(const QString &stored_text, const QString &generated_text)
{
    const QString stored = stored_text.trimmed();
    const QString generated = generated_text.trimmed();
    if (stored.isEmpty()) {
        return generated;
    }
    if (generated.isEmpty() || stored == generated || stored.contains(generated)
        || description_has_generated_sections(stored)) {
        return stored;
    }
    return QStringLiteral("%1\n\n%2").arg(stored, generated);
}

QString joined_section_text(const QStringList &lines)
{
    QString text = lines.join(QLatin1Char('\n')).trimmed();
    while (text.endsWith(QLatin1Char('\n'))) {
        text.chop(1);
    }
    return text.trimmed();
}

void apply_task_panel_base(QAbstractScrollArea *widget, const QColor &base_color)
{
    if (widget == nullptr || !base_color.isValid()) {
        return;
    }

    QPalette palette = widget->palette();
    palette.setColor(QPalette::Base, base_color);
    palette.setColor(QPalette::AlternateBase, base_color);
    palette.setColor(QPalette::Window, base_color);
    widget->setPalette(palette);
    widget->setAutoFillBackground(true);
    if (widget->viewport() != nullptr) {
        widget->viewport()->setAutoFillBackground(true);
        widget->viewport()->setPalette(palette);
    } else {
        widget->setAutoFillBackground(true);
    }
}

void restore_native_report_frame(QGroupBox *group)
{
    if (group == nullptr) {
        return;
    }

    if (!group->styleSheet().isEmpty()) {
        group->setStyleSheet(QString());
    }
    group->setFlat(false);
    group->update();
}

void sync_report_frame_title_text_colors(MetodoFeynman &self)
{
    restore_native_report_frame(self.frame_conceito);
    restore_native_report_frame(self.frame_meus_conceitos);
    restore_native_report_frame(self.frame_explique);
    restore_native_report_frame(self.frame_lacunas);
    restore_native_report_frame(self.frame_revisao);
    restore_native_report_frame(self.frame_descricao);
}

class FeynmanPlaceholderTextEdit final : public QTextEdit {
public:
    explicit FeynmanPlaceholderTextEdit(QWidget *parent = nullptr) : QTextEdit(parent)
    {
        QTextEdit::setPlaceholderText(QString());
    }

    void set_placeholder_base_text(const QString &text)
    {
        if (placeholder_base_text_ == text) {
            return;
        }
        placeholder_base_text_ = text;
        if (viewport() != nullptr) {
            viewport()->update();
        } else {
            update();
        }
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QTextEdit::paintEvent(event);

        if (placeholder_base_text_.isEmpty() || !toPlainText().isEmpty()) {
            return;
        }

        const QRect rect = placeholder_rect();
        if (!rect.isValid() || rect.width() <= 0 || rect.height() <= 0) {
            return;
        }

        QPainter painter(viewport());
        painter.setPen(placeholder_text_color());
        painter.setFont(font());
        painter.drawText(rect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, placeholder_base_text_);
    }

private:
    QString placeholder_base_text_;

    QRect placeholder_rect() const
    {
        QRect rect = viewport() != nullptr ? viewport()->rect() : QRect();
        const QMargins margins = contentsMargins();
        const int doc_margin = document() != nullptr ? static_cast<int>(document()->documentMargin()) : 0;
        rect.adjust(margins.left() + doc_margin,
                    margins.top() + doc_margin,
                    -(margins.right() + doc_margin),
                    -(margins.bottom() + doc_margin));
        return rect;
    }

    QColor placeholder_text_color() const
    {
        QPalette palette = this->palette();
        QColor color = palette.color(QPalette::PlaceholderText);
        if (!color.isValid()) {
            color = palette.color(QPalette::Disabled, QPalette::Text);
        }
        if (!color.isValid()) {
            color = palette.color(QPalette::Text);
        }
        return color;
    }
};

}  // namespace

QString first_n_lines(const QString &text, int max_lines)
{
    if (text.trimmed().isEmpty()) {
        return {};
    }

    QStringList lines;
    for (const QString &line : text.split(QLatin1Char('\n'))) {
        if (!line.trimmed().isEmpty()) {
            lines.append(line.trimmed());
        }
        if (lines.size() >= max_lines) {
            break;
        }
    }

    return lines.join(QLatin1Char('\n'));
}

QString expand_task_panel_text(const QWidget *context)
{
    return compat::tr(context, QStringLiteral("Expandir Painel de Tarefas"), QStringLiteral("Expand Task Panel"));
}

QString collapse_task_panel_text(const QWidget *context)
{
    return compat::tr(context, QStringLiteral("Recolher Painel de Tarefas"), QStringLiteral("Collapse Task Panel"));
}

int rotated_button_width(const source::GerenciamentoUI::RotatedTabButton *button)
{
    return button != nullptr ? std::max(button->minimumWidth(), button->sizeHint().width()) : 0;
}

QString normalized_ui_layout_key(const QString &value)
{
    const QString key = value.trimmed().toLower();
    if (key == QStringLiteral("horizontal") || key == QStringLiteral("vertical")) {
        return key;
    }
    return QStringLiteral("tabs");
}

bool is_horizontal_ui_layout(const QString &value)
{
    return normalized_ui_layout_key(value) == QStringLiteral("horizontal");
}

bool is_vertical_ui_layout(const QString &value)
{
    return normalized_ui_layout_key(value) == QStringLiteral("vertical");
}

void delete_layout_item(QLayoutItem *item)
{
    if (item == nullptr) {
        return;
    }

    if (QWidget *widget = item->widget()) {
        delete widget;
    } else if (QLayout *layout = item->layout()) {
        while (QLayoutItem *child = layout->takeAt(0)) {
            delete_layout_item(child);
        }
        delete layout;
        return;
    }

    delete item;
}

void clear_layout(QLayout *layout)
{
    if (layout == nullptr) {
        return;
    }

    while (QLayoutItem *item = layout->takeAt(0)) {
        delete_layout_item(item);
    }
}

void configure_compact_label(QLabel *label)
{
    if (label == nullptr) {
        return;
    }

    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void configure_compact_scroll_area(QAbstractScrollArea *widget)
{
    if (widget == nullptr) {
        return;
    }

    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    widget->setMinimumHeight(0);
    widget->setFrameShape(QFrame::NoFrame);
}

PanelMinimums effective_panel_minimums(int total_width,
                                       int left_base,
                                       int right_base,
                                       bool left_visible)
{
    PanelMinimums result;
    int left_min = left_visible ? std::max(0, left_base) : 0;
    int right_min = std::max(0, right_base);

    if (total_width <= 0) {
        result.left = left_min;
        result.right = right_min;
        return result;
    }

    const int available = std::max(0, total_width);
    int overflow = left_min + right_min - available;
    if (overflow > 0) {
        const int left_floor = left_visible ? kFeynmanLeftPanelCompactWidth : 0;
        int right_floor = std::max(0, right_base);
        if (total_width < right_floor) {
            right_floor = total_width;
        }

        const int reduce_left = std::min(overflow, std::max(0, left_min - left_floor));
        left_min -= reduce_left;
        overflow -= reduce_left;

        const int reduce_right = std::min(overflow, std::max(0, right_min - right_floor));
        right_min -= reduce_right;
        overflow -= reduce_right;

        if (overflow > 0) {
            const int extra_left = std::min(overflow, left_min);
            left_min -= extra_left;
            overflow -= extra_left;

            const int extra_right = std::min(overflow, right_min);
            right_min -= extra_right;
            overflow -= extra_right;
        }
    }

    result.left = std::max(0, left_min);
    result.right = std::max(0, right_min);
    return result;
}

QGroupBox *create_report_frame(QWidget *parent, QLabel *label, QTextEdit *editor)
{
    auto *frame = new QGroupBox(parent);
    frame->setFlat(false);
    frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    frame->setMinimumHeight(0);

    auto *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);
    if (label != nullptr) {
        layout->addWidget(label);
    }
    if (editor != nullptr) {
        layout->addWidget(editor, 1);
    }

    return frame;
}

QTextEdit *create_placeholder_text_edit(QWidget *parent)
{
    return new FeynmanPlaceholderTextEdit(parent);
}

void set_placeholder_base_text(QTextEdit *editor, const QString &text)
{
    if (editor == nullptr) {
        return;
    }

    if (auto *custom = dynamic_cast<FeynmanPlaceholderTextEdit *>(editor)) {
        custom->set_placeholder_base_text(text);
        return;
    }

    editor->setPlaceholderText(text);
}

QSize available_window_size(const QWidget *widget)
{
    const QWidget *top_level = widget != nullptr && widget->window() != nullptr ? widget->window() : widget;
    QScreen *screen = top_level != nullptr ? top_level->screen() : nullptr;
    if (screen == nullptr && widget != nullptr) {
        screen = widget->screen();
    }
    if (screen == nullptr) {
        screen = QApplication::primaryScreen();
    }

    return screen != nullptr ? screen->availableGeometry().size() : QSize(1380, 840);
}

int normalized_domain_value(const QVariant &value)
{
    return qBound(kDefaultDomain, value.toInt(), 2);
}

int normalized_priority_value(const QVariant &value)
{
    bool ok = false;
    const int as_int = value.toInt(&ok);
    if (ok) {
        if (as_int >= 1 && as_int <= 4) {
            return as_int;
        }
        if (as_int >= 0 && as_int <= 3) {
            return as_int + 1;
        }
    }

    const QString normalized = normalized_priority_key(value.toString());
    if (normalized == normalized_priority_key(QStringLiteral("Importante e Urgente"))
        || normalized == normalized_priority_key(QStringLiteral("Important and Urgent"))) {
        return 1;
    }
    if (normalized == normalized_priority_key(QStringLiteral("Importante, mas Não Urgente"))
        || normalized == normalized_priority_key(QStringLiteral("Important, but Not Urgent"))
        || normalized == normalized_priority_key(QStringLiteral("Important but Not Urgent"))) {
        return 2;
    }
    if (normalized == normalized_priority_key(QStringLiteral("Não Importante, mas Urgente"))
        || normalized == normalized_priority_key(QStringLiteral("Not Important, but Urgent"))
        || normalized == normalized_priority_key(QStringLiteral("Not Important but Urgent"))) {
        return 3;
    }
    if (normalized == normalized_priority_key(QStringLiteral("Não Importante e Não Urgente"))
        || normalized == normalized_priority_key(QStringLiteral("Not Important and Not Urgent"))) {
        return 4;
    }

    return kDefaultPriority;
}

QString domain_symbol(int dominio)
{
    switch (qBound(kDefaultDomain, dominio, 2)) {
    case 0:
        return QStringLiteral("🥉");
    case 1:
        return QStringLiteral("🥈");
    default:
        return QStringLiteral("🥇");
    }
}

QString priority_emoji(int prioridade)
{
    switch (normalized_priority_value(prioridade)) {
    case 1:
        return QStringLiteral("🔴");
    case 2:
        return QStringLiteral("🟠");
    case 3:
        return QStringLiteral("🟡");
    default:
        return QStringLiteral("🟢");
    }
}

QString priority_label(const QWidget *context, int prioridade)
{
    switch (normalized_priority_value(prioridade)) {
    case 1:
        return compat::tr(context,
                          QStringLiteral("🔴 Importante e Urgente"),
                          QStringLiteral("🔴 Important and Urgent"));
    case 2:
        return compat::tr(context,
                          QStringLiteral("🟠 Importante, mas Não Urgente"),
                          QStringLiteral("🟠 Important, but Not Urgent"));
    case 3:
        return compat::tr(context,
                          QStringLiteral("🟡 Não Importante, mas Urgente"),
                          QStringLiteral("🟡 Not Important, but Urgent"));
    default:
        return compat::tr(context,
                          QStringLiteral("🟢 Não Importante e Não Urgente"),
                          QStringLiteral("🟢 Not Important and Not Urgent"));
    }
}

QString priority_text_without_emoji(const QWidget *context, int prioridade)
{
    QString text = priority_label(context, prioridade).trimmed();
    const QStringList prefixes = {
        QStringLiteral("🔴 "),
        QStringLiteral("🟠 "),
        QStringLiteral("🟡 "),
        QStringLiteral("🟢 "),
    };

    for (const QString &prefix : prefixes) {
        if (text.startsWith(prefix)) {
            text.remove(0, prefix.size());
            break;
        }
    }

    return text.trimmed();
}

QString domain_label(const QWidget *context, int dominio, bool short_form)
{
    switch (qBound(kDefaultDomain, dominio, 2)) {
    case 0:
        return short_form
                   ? compat::tr(context, QStringLiteral("🥉 Iniciante"), QStringLiteral("🥉 Beginner"))
                   : compat::tr(context,
                                QStringLiteral("🥉 Iniciante (não consigo explicar)"),
                                QStringLiteral("🥉 Beginner (I can't explain it)"));
    case 1:
        return short_form
                   ? compat::tr(context, QStringLiteral("🥈 Intermediário"), QStringLiteral("🥈 Intermediate"))
                   : compat::tr(context,
                                QStringLiteral("🥈 Intermediário (explico com dificuldade)"),
                                QStringLiteral("🥈 Intermediate (I explain it with difficulty)"));
    default:
        return short_form
                   ? compat::tr(context, QStringLiteral("🥇 Avançado"), QStringLiteral("🥇 Advanced"))
                   : compat::tr(context,
                                QStringLiteral("🥇 Avançado (explico facilmente)"),
                                QStringLiteral("🥇 Advanced (I explain it easily)"));
    }
}

QVariantMap normalized_concept(const QVariantMap &conceito)
{
    QVariantMap normalized = conceito;
    normalized.insert(QStringLiteral("dominio"), normalized_domain_value(conceito.value(QStringLiteral("dominio"))));
    normalized.insert(QStringLiteral("prioridade"), normalized_priority_value(conceito.value(QStringLiteral("prioridade"))));

    double dur = 1.0;
    if (conceito.contains(QStringLiteral("duracao")) && conceito.value(QStringLiteral("duracao")).toDouble() > 0.0) {
        dur = conceito.value(QStringLiteral("duracao")).toDouble();
    } else if (conceito.contains(QStringLiteral("duration")) && conceito.value(QStringLiteral("duration")).toDouble() > 0.0) {
        dur = conceito.value(QStringLiteral("duration")).toDouble();
    }
    normalized.insert(QStringLiteral("duracao"), dur);
    normalized.insert(QStringLiteral("duration"), dur);

    int pom = 0;
    if (conceito.contains(QStringLiteral("pomodoros"))) {
        pom = conceito.value(QStringLiteral("pomodoros")).toInt();
    } else if (conceito.contains(QStringLiteral("pomodoros_completados"))) {
        pom = conceito.value(QStringLiteral("pomodoros_completados")).toInt();
    }
    normalized.insert(QStringLiteral("pomodoros"), pom);
    normalized.insert(QStringLiteral("pomodoros_completados"), pom);

    int c_tot = 0;
    if (conceito.contains(QStringLiteral("ciclos_totais")) && conceito.value(QStringLiteral("ciclos_totais")).toInt() > 0) {
        c_tot = conceito.value(QStringLiteral("ciclos_totais")).toInt();
    } else if (conceito.contains(QStringLiteral("ciclos_permitidos")) && conceito.value(QStringLiteral("ciclos_permitidos")).toInt() > 0) {
        c_tot = conceito.value(QStringLiteral("ciclos_permitidos")).toInt();
    } else if (conceito.contains(QStringLiteral("total_ciclos")) && conceito.value(QStringLiteral("total_ciclos")).toInt() > 0) {
        c_tot = conceito.value(QStringLiteral("total_ciclos")).toInt();
    }
    if (c_tot > 0) {
        normalized.insert(QStringLiteral("ciclos_totais"), c_tot);
        normalized.insert(QStringLiteral("ciclos_permitidos"), c_tot);
        normalized.insert(QStringLiteral("total_ciclos"), c_tot);
    }
    const bool custom_cycles = conceito.value(QStringLiteral("ciclos_customizados"), c_tot > 0).toBool();
    normalized.insert(QStringLiteral("ciclos_customizados"), custom_cycles);

    if (conceito.contains(QStringLiteral("status"))) {
        normalized.insert(QStringLiteral("status"), conceito.value(QStringLiteral("status")).toString().trimmed());
    }
    if (conceito.contains(QStringLiteral("concluida"))) {
        normalized.insert(QStringLiteral("concluida"), conceito.value(QStringLiteral("concluida")).toBool());
    }
    if (conceito.contains(QStringLiteral("completed"))) {
        normalized.insert(QStringLiteral("completed"), conceito.value(QStringLiteral("completed")).toBool());
    }
    if (conceito.contains(QStringLiteral("em_progresso"))) {
        normalized.insert(QStringLiteral("em_progresso"), conceito.value(QStringLiteral("em_progresso")).toBool());
    }
    if (conceito.contains(QStringLiteral("in_progress"))) {
        normalized.insert(QStringLiteral("in_progress"), conceito.value(QStringLiteral("in_progress")).toBool());
    }

    const QString arquivo = conceito.value(QStringLiteral("arquivo_vinculado")).toString().trimmed();
    normalized.insert(QStringLiteral("arquivo_vinculado"), arquivo.isEmpty() ? QVariant() : QVariant(arquivo));
    const bool descricao_customizada =
        conceito.value(QStringLiteral("descricao_customizada")).toBool() || conceito.contains(QStringLiteral("descricao"));
    const QString descricao = conceito.value(QStringLiteral("descricao")).toString().trimmed();
    normalized.insert(QStringLiteral("descricao_customizada"), descricao_customizada);
    if (descricao_customizada) {
        normalized.insert(QStringLiteral("descricao"), descricao);
    } else {
        normalized.remove(QStringLiteral("descricao"));
    }
    return normalized;
}

QVariantList normalized_concepts(const QVariantList &concepts)
{
    QVariantList normalized;
    normalized.reserve(concepts.size());
    for (const QVariant &value : concepts) {
        normalized.append(normalized_concept(value.toMap()));
    }
    return normalized;
}

QVariantMap preview_payload(const QWidget *context, const QVariantMap &conceito)
{
    QVariantMap payload;
    payload.insert(QStringLiteral("conceito"), first_n_lines(conceito.value(QStringLiteral("titulo")).toString()));
    payload.insert(QStringLiteral("prioridade"), normalized_priority_value(conceito.value(QStringLiteral("prioridade"))));
    payload.insert(QStringLiteral("dominio"), normalized_domain_value(conceito.value(QStringLiteral("dominio"))));
    payload.insert(QStringLiteral("explique"), first_n_lines(conceito.value(QStringLiteral("explicacao")).toString()));
    payload.insert(QStringLiteral("lacunas"), first_n_lines(conceito.value(QStringLiteral("lacunas")).toString()));
    payload.insert(QStringLiteral("revisao"), first_n_lines(conceito.value(QStringLiteral("revisao")).toString()));
    payload.insert(QStringLiteral("descricao"), first_n_lines(concept_description_text(context, conceito)));
    payload.insert(QStringLiteral("arquivo_vinculado"), conceito.value(QStringLiteral("arquivo_vinculado")).toString().trimmed());
    return payload;
}

QVariantMap sort_payload_from_concept(const QVariantMap &conceito)
{
    const QVariantMap normalized = normalized_concept(conceito);
    return {
        {QStringLiteral("id"), normalized.value(QStringLiteral("integracao_task_id")).toString()},
        {QStringLiteral("text"), normalized.value(QStringLiteral("titulo")).toString()},
        {QStringLiteral("date"), normalized.value(QStringLiteral("integracao_date")).toString()},
        {QStringLiteral("time"), normalized.value(QStringLiteral("integracao_time")).toString()},
    };
}

bool has_host_task_sort()
{
    return compat::host_bridge() != nullptr && compat::host_bridge()->has_active_task_sort();
}

bool host_task_sort_less(const QVariantMap &left, const QVariantMap &right)
{
    if (compat::host_bridge() == nullptr) {
        return false;
    }

    return compat::host_bridge()->task_sort_less(sort_payload_from_concept(left), sort_payload_from_concept(right));
}

DescriptionParts parse_description_text(const QString &text)
{
    DescriptionParts parts;
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        return parts;
    }

    QString current_section;
    QStringList preamble;
    QHash<QString, QStringList> section_lines;
    const QStringList lines = trimmed.split(QLatin1Char('\n'));

    for (const QString &raw_line : lines) {
        const QString section_key = description_section_key(raw_line);
        if (!section_key.isEmpty()) {
            current_section = section_key;
            parts.structured = true;
            continue;
        }

        if (parts.structured && !current_section.isEmpty()) {
            section_lines[current_section].append(raw_line);
        } else {
            preamble.append(raw_line);
        }
    }

    if (!parts.structured) {
        parts.explanation = trimmed;
        return parts;
    }

    parts.concept_text = joined_section_text(section_lines.value(QStringLiteral("concept")));
    parts.explanation = joined_section_text(section_lines.value(QStringLiteral("explanation")));
    parts.gaps = joined_section_text(section_lines.value(QStringLiteral("gaps")));
    parts.review = joined_section_text(section_lines.value(QStringLiteral("review")));

    const QString prefix = joined_section_text(preamble);
    if (!prefix.isEmpty()) {
        if (parts.explanation.isEmpty()) {
            parts.explanation = prefix;
        } else {
            parts.explanation = QStringLiteral("%1\n\n%2").arg(prefix, parts.explanation);
        }
    }

    return parts;
}

QString composed_description(const QWidget *context,
                             const QString &title,
                             const QString &explanation,
                             const QString &gaps,
                             const QString &review)
{
    struct Section {
        QString key;
        QString content;
    };

    const QList<Section> sections = {
        {QStringLiteral("concept"), title.trimmed()},
        {QStringLiteral("explanation"), explanation.trimmed()},
        {QStringLiteral("gaps"), gaps.trimmed()},
        {QStringLiteral("review"), review.trimmed()},
    };

    QStringList lines;
    for (const Section &section : sections) {
        if (!section.content.isEmpty()) {
            lines << QStringLiteral("%1:\n%2")
                         .arg(description_section_title(context, section.key), section.content);
        }
    }

    return lines.join(QStringLiteral("\n\n"));
}

QString trim_description_separator(QString text)
{
    text = text.trimmed();
    while (text.startsWith(QLatin1Char('\n'))) {
        text.remove(0, 1);
        text = text.trimmed();
    }
    return text.trimmed();
}

QString merge_description_edit(const QString &previous_generated_text,
                               const QString &edited_text,
                               const QString &next_generated_text)
{
    const QString previous_generated = previous_generated_text.trimmed();
    const QString edited = edited_text.trimmed();
    const QString next_generated = next_generated_text.trimmed();

    if (edited.isEmpty() || edited == previous_generated) {
        return next_generated;
    }
    if (previous_generated.isEmpty()) {
        return edited == next_generated
                   ? next_generated
                   : merged_description_text(next_generated, manual_description_text(edited, next_generated));
    }

    const QVector<DescriptionToken> base_tokens = description_tokens(previous_generated);
    const QVector<DescriptionToken> edited_tokens = description_tokens(edited);
    const QVector<DescriptionToken> next_tokens = description_tokens(next_generated);
    const QVector<DescriptionEdit> edits = description_edits(base_tokens, edited_tokens);
    if (edits.isEmpty()) {
        return next_generated;
    }

    const QVector<QPair<int, int>> mapping_pairs = lcs_token_pairs(base_tokens, next_tokens);
    if (mapping_pairs.isEmpty() && !base_tokens.isEmpty() && !next_tokens.isEmpty()) {
        return merged_description_text(next_generated, edited);
    }

    QVector<int> base_to_next(base_tokens.size(), -1);
    for (const QPair<int, int> &pair : mapping_pairs) {
        if (pair.first >= 0 && pair.first < base_to_next.size()) {
            base_to_next[pair.first] = pair.second;
        }
    }

    QVector<DescriptionToken> merged_tokens = next_tokens;
    int delta = 0;
    for (const DescriptionEdit &edit : edits) {
        int start = mapped_description_boundary(edit.base_start, base_to_next, next_tokens.size()) + delta;
        int end = mapped_description_boundary(edit.base_end, base_to_next, next_tokens.size()) + delta;
        if (edit.base_start == edit.base_end) {
            end = start;
        }

        start = qBound(0, start, merged_tokens.size());
        end = qBound(start, end, merged_tokens.size());
        merged_tokens.erase(merged_tokens.begin() + start, merged_tokens.begin() + end);
        for (int i = 0; i < edit.replacement.size(); ++i) {
            merged_tokens.insert(start + i, edit.replacement.at(i));
        }
        delta += edit.replacement.size() - (end - start);
    }

    const QString merged = tokens_to_text(merged_tokens);
    return merged.isEmpty() ? next_generated : merged;
}

QString manual_description_text(const QString &display_text, const QString &generated_text)
{
    const QString display = display_text.trimmed();
    const QString generated = generated_text.trimmed();

    if (display.isEmpty()) {
        return {};
    }
    if (generated.isEmpty()) {
        return display;
    }
    if (display == generated || generated.contains(display)) {
        return {};
    }

    if (display.startsWith(generated)) {
        return trim_description_separator(display.mid(generated.size()));
    }

    const int generated_index = display.indexOf(generated);
    if (generated_index >= 0) {
        const QString before = trim_description_separator(display.left(generated_index));
        const QString after = trim_description_separator(display.mid(generated_index + generated.size()));
        if (before.isEmpty()) {
            return after;
        }
        if (after.isEmpty()) {
            return before;
        }
        return QStringLiteral("%1\n\n%2").arg(before, after);
    }

    return display;
}

QString merged_description_text(const QString &generated_text, const QString &manual_text)
{
    const QString generated = generated_text.trimmed();
    const QString manual = manual_text.trimmed();
    if (generated.isEmpty()) {
        return manual;
    }
    if (manual.isEmpty()) {
        return generated;
    }
    return QStringLiteral("%1\n\n%2").arg(generated, manual);
}

bool has_external_description_base(const QVariantMap &conceito)
{
    return is_external_integrated_concept(conceito);
}

QString description_with_external_base(const QString &base_text, const QString &generated_text)
{
    return external_description_with_generated_sections(base_text, generated_text);
}

QString concept_description_text(const QWidget *context, const QVariantMap &conceito)
{
    const QString generated = composed_description(context,
                                                   conceito.value(QStringLiteral("titulo")).toString(),
                                                   conceito.value(QStringLiteral("explicacao")).toString(),
                                                   conceito.value(QStringLiteral("lacunas")).toString(),
                                                   conceito.value(QStringLiteral("revisao")).toString());
    if (!has_custom_description(conceito)) {
        return generated;
    }

    const QString stored = stored_description(conceito);
    if (has_external_description_base(conceito)) {
        return description_with_external_base(stored, generated);
    }
    return stored.isEmpty() ? generated : stored;
}

QString shared_task_description(const QWidget *context, const QVariantMap &conceito)
{
    return concept_description_text(context, conceito);
}

QStringList integration_metadata_keys()
{
    return {
        QStringLiteral("integracao_origem"),
        QStringLiteral("integracao_task_id"),
        QStringLiteral("integracao_status"),
        QStringLiteral("integracao_date"),
        QStringLiteral("integracao_time"),
        QStringLiteral("integracao_completed"),
    };
}

QString tooltip_html(const QWidget *context, const QVariantMap &conceito)
{
    const QVariantMap normalized = normalized_concept(conceito);
    QStringList tooltip;
    const QString arquivo = normalized.value(QStringLiteral("arquivo_vinculado")).toString().trimmed();
    if (!arquivo.isEmpty()) {
        tooltip << QStringLiteral("%1: %2")
                       .arg(compat::tr(context, QStringLiteral("Arquivo"), QStringLiteral("File")),
                            QDir::toNativeSeparators(arquivo));
    }

    const QString data = normalized.value(QStringLiteral("integracao_date")).toString().trimmed();
    if (!data.isEmpty()) {
        QString human_date = data;
        const QDate parsed = QDate::fromString(data, Qt::ISODate);
        if (parsed.isValid()) {
            human_date = parsed.toString(compat::is_english(context) ? QStringLiteral("MM/dd/yyyy")
                                                                     : QStringLiteral("dd/MM/yyyy"));
        }
        tooltip << QStringLiteral("%1: %2")
                       .arg(compat::tr(context, QStringLiteral("Data"), QStringLiteral("Date")), human_date);
    }

    const QString horario = normalized.value(QStringLiteral("integracao_time")).toString().trimmed();
    if (!horario.isEmpty()) {
        tooltip << QStringLiteral("%1: %2")
                       .arg(compat::tr(context, QStringLiteral("Horário"), QStringLiteral("Time")), horario);
    }

    const QString descricao = concept_description_text(context, normalized).trimmed();
    if (!descricao.isEmpty()) {
        tooltip << QStringLiteral("%1: %2")
                       .arg(compat::tr(context, QStringLiteral("Descrição"), QStringLiteral("Description")),
                            descricao);
    }

    tooltip << QStringLiteral("%1: %2")
                   .arg(compat::tr(context, QStringLiteral("Prioridade"), QStringLiteral("Priority")),
                        priority_label(context, normalized.value(QStringLiteral("prioridade")).toInt()));
    tooltip << QStringLiteral("%1: %2")
                   .arg(compat::tr(context, QStringLiteral("Nível de Domínio"), QStringLiteral("Mastery Level")),
                        domain_label(context, normalized.value(QStringLiteral("dominio")).toInt(), false));

    double dur_days = 1.0;
    if (normalized.contains(QStringLiteral("duracao")) && normalized.value(QStringLiteral("duracao")).toDouble() > 0.0) {
        dur_days = normalized.value(QStringLiteral("duracao")).toDouble();
    } else if (normalized.contains(QStringLiteral("duration")) && normalized.value(QStringLiteral("duration")).toDouble() > 0.0) {
        dur_days = normalized.value(QStringLiteral("duration")).toDouble();
    }

    const bool is_en = compat::is_english(context);
    QString dur_text;
    if (dur_days == 1.0) {
        dur_text = is_en ? QStringLiteral("1 day") : QStringLiteral("1 dia");
    } else if (dur_days == static_cast<int>(dur_days)) {
        dur_text = QStringLiteral("%1 %2").arg(QString::number(static_cast<int>(dur_days)), is_en ? QStringLiteral("days") : QStringLiteral("dias"));
    } else {
        dur_text = QStringLiteral("%1 %2").arg(QString::number(dur_days, 'f', 1), is_en ? QStringLiteral("days") : QStringLiteral("dias"));
    }
    const QString dur_label = compat::tr(context, QStringLiteral("Duração:"), QStringLiteral("Duration:"));
    tooltip << (dur_label.endsWith(QLatin1Char(':'))
                   ? QStringLiteral("%1 %2").arg(dur_label, dur_text)
                   : QStringLiteral("%1: %2").arg(dur_label, dur_text));

    const int default_ciclos = compat::host_bridge() != nullptr ? compat::host_bridge()->default_pomodoro_cycle_count() : 4;
    const int pom = normalized.value(QStringLiteral("pomodoros"), normalized.value(QStringLiteral("pomodoros_completados"), 0)).toInt();
    const int c_tot = normalized.value(QStringLiteral("ciclos_totais"),
                      normalized.value(QStringLiteral("ciclos_permitidos"),
                      normalized.value(QStringLiteral("total_ciclos"), default_ciclos))).toInt();
    const bool is_done = is_concept_completed(normalized);
    const bool in_prog = is_concept_in_progress(normalized);
    const bool is_custom = normalized.value(QStringLiteral("ciclos_customizados"), false).toBool()
                           || (c_tot > 0 && c_tot != default_ciclos);
    int safe_c_tot = default_ciclos;
    if (in_prog) {
        safe_c_tot = (is_custom && c_tot > 0) ? c_tot : default_ciclos;
    } else {
        safe_c_tot = c_tot > 0 ? c_tot : default_ciclos;
    }
    if (pom > 0 || safe_c_tot > 0) {
        double pom_pct = 0.0;
        if (is_done) {
            pom_pct = 100.0;
        } else {
            pom_pct = (static_cast<double>(pom) / static_cast<double>(safe_c_tot)) * 100.0;
            if (pom_pct >= 100.0) {
                pom_pct = 99.0;
            } else if (pom_pct < 0.0) {
                pom_pct = 0.0;
            }
        }
        const QString pom_fmt = compat::tr(context, QStringLiteral("Ciclos Pomodoro: %1/%2 (%3%)"), QStringLiteral("Pomodoro Cycles: %1/%2 (%3%)"));
        tooltip << pom_fmt
                       .arg(QString::number(pom),
                            QString::number(safe_c_tot),
                            QString::number(pom_pct, 'f', 0));
    }

    return tooltip.join(QLatin1Char('\n'));
}

QString human_date_for_display(const QWidget *context, const QString &date_iso)
{
    const QString cleaned = date_iso.trimmed();
    if (cleaned.isEmpty()) {
        return {};
    }

    const QDate date = QDate::fromString(cleaned, Qt::ISODate);
    if (!date.isValid()) {
        return cleaned;
    }

    return date.toString(compat::is_english(context) ? QStringLiteral("MM/dd/yyyy") : QStringLiteral("dd/MM/yyyy"));
}

QString visible_concept_title(const QWidget *context, const QVariantMap &conceito, const QString &base_text)
{
    QString display_text = base_text.trimmed();
    if (compat::host_bridge() == nullptr || !compat::host_bridge()->reserved_space_enabled()) {
        return display_text;
    }

    const QString human_date = human_date_for_display(context, conceito.value(QStringLiteral("integracao_date")).toString());
    if (human_date.isEmpty()) {
        return display_text;
    }

    const QString time_text = conceito.value(QStringLiteral("integracao_time")).toString().trimmed();
    return time_text.isEmpty() ? QStringLiteral("%1 — %2").arg(display_text, human_date)
                               : QStringLiteral("%1 — %2 %3").arg(display_text, human_date, time_text);
}

QString watermark_concept_text(const QWidget *context, const QVariantMap &conceito)
{
    if (compat::host_bridge() == nullptr || !compat::host_bridge()->watermark_enabled()) {
        return {};
    }

    QStringList parts;
    const QString human_date =
        human_date_for_display(context, conceito.value(QStringLiteral("integracao_date")).toString());
    if (!human_date.trimmed().isEmpty()) {
        parts << QStringLiteral("%1: %2")
                     .arg(compat::tr(context, QStringLiteral("Data"), QStringLiteral("Date")), human_date.trimmed());
    }

    const QString cleaned_time = conceito.value(QStringLiteral("integracao_time")).toString().trimmed();
    if (!cleaned_time.isEmpty()) {
        parts << QStringLiteral("%1: %2")
                     .arg(compat::tr(context, QStringLiteral("Horário"), QStringLiteral("Time")), cleaned_time);
    }

    return parts.join(QStringLiteral("  |  "));
}

QString concept_group_key(const QVariantMap &conceito)
{
    const QString time_text = conceito.value(QStringLiteral("integracao_time")).toString().trimmed();
    if (time_text.isEmpty()) {
        return {};
    }

    const QString date_text = conceito.value(QStringLiteral("integracao_date")).toString().trimmed().isEmpty()
                                  ? QStringLiteral("9999-99-99")
                                  : conceito.value(QStringLiteral("integracao_date")).toString().trimmed();
    return QStringLiteral("%1|%2").arg(date_text, time_text);
}

bool is_group_header(QListWidgetItem *item)
{
    return item != nullptr && item->data(kGroupHeaderRole).toString() == QStringLiteral("group_header");
}

void apply_window_as_base(QWidget *widget)
{
    if (widget == nullptr) {
        return;
    }

    auto *app = qobject_cast<QApplication *>(QCoreApplication::instance());
    if (app == nullptr) {
        return;
    }

    const QPalette app_palette = app->palette();
    const QColor window_color = app_palette.color(QPalette::Window);
    const auto is_light_color = [](const QColor &color) {
        const double luminance = (0.2126 * color.red() + 0.7152 * color.green() + 0.0722 * color.blue()) / 255.0;
        return luminance >= 0.85;
    };

    QPalette palette = widget->palette();

    if (auto *combo = qobject_cast<QComboBox *>(widget)) {
        palette.setColor(QPalette::Button, window_color);
        combo->setPalette(palette);
        if (combo->view() != nullptr) {
            QPalette popup_palette = combo->view()->palette();
            popup_palette.setColor(QPalette::Base, is_light_color(window_color) ? window_color : window_color.darker(115));
            popup_palette.setColor(QPalette::AlternateBase,
                                   is_light_color(window_color) ? window_color : window_color.darker(115));
            combo->view()->setPalette(popup_palette);
            if (combo->view()->viewport() != nullptr) {
                combo->view()->viewport()->setAutoFillBackground(true);
                combo->view()->viewport()->setPalette(popup_palette);
            }
        }
        return;
    }

    palette.setColor(QPalette::Base, window_color);
    palette.setColor(QPalette::AlternateBase, window_color);
    widget->setPalette(palette);

    if (auto *list = qobject_cast<QListWidget *>(widget)) {
        list->viewport()->setAutoFillBackground(true);
        list->viewport()->setPalette(palette);
        return;
    }

    widget->setAutoFillBackground(true);
}

void sync_backgrounds(MetodoFeynman &self)
{
    auto *app = qobject_cast<QApplication *>(QCoreApplication::instance());
    const QPalette palette = app != nullptr ? app->palette() : self.palette();
    QColor gui_base = self.palette().color(QPalette::Window);
    if (!gui_base.isValid() || gui_base.alpha() == 0) {
        gui_base = palette.color(QPalette::Window);
    }
    if (!gui_base.isValid() || gui_base.alpha() == 0) {
        gui_base = palette.color(QPalette::Base);
    }

    const auto is_light_color = [](const QColor &color) {
        const double luminance = (0.2126 * color.red() + 0.7152 * color.green() + 0.0722 * color.blue()) / 255.0;
        return luminance >= 0.85;
    };
    const QColor task_panel_base = is_light_color(gui_base) ? gui_base.lighter(95) : gui_base.darker(115);

    apply_task_panel_base(self.lista_conceitos, task_panel_base);
    apply_task_panel_base(self.input_titulo, task_panel_base);
    apply_task_panel_base(self.texto_explicacao, task_panel_base);
    apply_task_panel_base(self.texto_lacunas, task_panel_base);
    apply_task_panel_base(self.texto_revisao, task_panel_base);
    apply_task_panel_base(self.texto_descricao, task_panel_base);
    sync_report_frame_title_text_colors(self);
    apply_window_as_base(self.combo_prioridade);
    apply_window_as_base(self.combo_dominio);
    source::utils::AccentTabStyle::refresh(self.tab_widget);
}

QString action_text_with_path(const QString &label, const QString &path)
{
    const QString cleaned = path.trimmed();
    return cleaned.isEmpty() ? label : QStringLiteral("%1\t%2").arg(label, QDir::toNativeSeparators(cleaned));
}

QLocale effective_locale(const QWidget *context)
{
    return compat::is_english(context) ? QLocale(QLocale::English, QLocale::UnitedStates)
                                       : QLocale(QLocale::Portuguese, QLocale::Brazil);
}

QString effective_date_format(const QWidget *context)
{
    return compat::is_english(context) ? QStringLiteral("MM/dd/yyyy") : QStringLiteral("dd/MM/yyyy");
}

bool is_concept_completed(const QVariantMap &conceito)
{
    const QString status = conceito.value(QStringLiteral("integracao_status"),
                                          conceito.value(QStringLiteral("status"))).toString().trimmed();
    const bool completed = conceito.value(QStringLiteral("integracao_completed"),
                                          conceito.value(QStringLiteral("completed"),
                                                        conceito.value(QStringLiteral("concluida"), false))).toBool();
    return completed
           || status.compare(QStringLiteral("Done"), Qt::CaseInsensitive) == 0
           || status.compare(QStringLiteral("Completed"), Qt::CaseInsensitive) == 0
           || status.compare(QStringLiteral("Concluída"), Qt::CaseInsensitive) == 0
           || status.compare(QStringLiteral("Concluida"), Qt::CaseInsensitive) == 0;
}

bool is_concept_in_progress(const QVariantMap &conceito)
{
    if (is_concept_completed(conceito)) {
        return false;
    }
    const QString status = conceito.value(QStringLiteral("integracao_status"),
                                          conceito.value(QStringLiteral("status"))).toString().trimmed();
    return conceito.value(QStringLiteral("in_progress")).toBool()
           || conceito.value(QStringLiteral("em_progresso")).toBool()
           || conceito.value(QStringLiteral("em_andamento")).toBool()
           || status.compare(QStringLiteral("Doing"), Qt::CaseInsensitive) == 0
           || status.compare(QStringLiteral("In Progress"), Qt::CaseInsensitive) == 0
           || status.compare(QStringLiteral("Em Progresso"), Qt::CaseInsensitive) == 0
           || status.compare(QStringLiteral("Em Andamento"), Qt::CaseInsensitive) == 0
           || status.contains(QStringLiteral("progress"), Qt::CaseInsensitive)
           || status.contains(QStringLiteral("andamento"), Qt::CaseInsensitive);
}

bool is_concept_pending(const QVariantMap &conceito)
{
    return !is_concept_completed(conceito) && !is_concept_in_progress(conceito);
}

}  // namespace source::plugin::Feynman
