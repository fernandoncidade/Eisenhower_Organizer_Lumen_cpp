#include "source/services/incore_12_Manual.hpp"

#include "source/services/incore_16_manual_pt-BR.hpp"
#include "source/services/incore_17_manual_en-US.hpp"
#include "source/utils/AppConfig.hpp"
#include "source/utils/BarraTitulosUtils.hpp"

namespace source::InterfaceCore::incore_12_Manual {
namespace {

QString platform_name()
{
    return source::utils::AppConfig::app_manual_name();
}

QString manual_intro_text(const QString &normalized_lang)
{
    return normalized_lang == QStringLiteral("en_US")
               ? QStringLiteral("This manual describes how to use the %1 platform.").arg(platform_name())
               : QStringLiteral("Este manual descreve como utilizar a plataforma %1.").arg(platform_name());
}

void append_plugin_sections(QList<ManualSection> &document,
                            const QString &normalized_lang,
                            bool include_pomodoro_plugin,
                            bool include_feynman_plugin,
                            bool include_mapamental_plugin,
                            bool include_audiolivro_plugin,
                            bool include_agenda_plugin,
                            bool include_modelo_plugin,
                            bool include_gantt_plugin)
{
    if (include_feynman_plugin) {
        const QList<ManualSection> feynman_sections =
            normalized_lang == QStringLiteral("en_US")
                ? incore_17_manual_en_US::feynman_en_document()
                : incore_16_manual_pt_BR::feynman_pt_document();
        document.append(feynman_sections);
    }

    if (include_pomodoro_plugin) {
        const QList<ManualSection> pomodoro_sections =
            normalized_lang == QStringLiteral("en_US")
                ? incore_17_manual_en_US::pomodoro_en_document()
                : incore_16_manual_pt_BR::pomodoro_pt_document();
        document.append(pomodoro_sections);
    }

    if (include_mapamental_plugin) {
        const QList<ManualSection> mapamental_sections =
            normalized_lang == QStringLiteral("en_US")
                ? incore_17_manual_en_US::mapamental_en_document()
                : incore_16_manual_pt_BR::mapamental_pt_document();
        document.append(mapamental_sections);
    }

    if (include_audiolivro_plugin) {
        const QList<ManualSection> audiolivro_sections =
            normalized_lang == QStringLiteral("en_US")
                ? incore_17_manual_en_US::audiolivro_en_document()
                : incore_16_manual_pt_BR::audiolivro_pt_document();
        document.append(audiolivro_sections);
    }

    if (include_agenda_plugin) {
        const QList<ManualSection> agenda_sections =
            normalized_lang == QStringLiteral("en_US")
                ? incore_17_manual_en_US::agenda_en_document()
                : incore_16_manual_pt_BR::agenda_pt_document();
        document.append(agenda_sections);
    }

    if (include_modelo_plugin) {
        const QList<ManualSection> modelo_sections =
            normalized_lang == QStringLiteral("en_US")
                ? incore_17_manual_en_US::modelo_en_document()
                : incore_16_manual_pt_BR::modelo_pt_document();
        document.append(modelo_sections);
    }

    if (include_gantt_plugin) {
        const QList<ManualSection> gantt_sections =
            normalized_lang == QStringLiteral("en_US")
                ? incore_17_manual_en_US::gantt_en_document()
                : incore_16_manual_pt_BR::gantt_pt_document();
        document.append(gantt_sections);
    }
}

}  // namespace

QString normalize_language(const QString &lang)
{
    if (lang.trimmed().isEmpty()) {
        return QStringLiteral("pt_BR");
    }

    const QString value = lang.trimmed().replace(QLatin1Char('-'), QLatin1Char('_')).toLower();
    if (value == QStringLiteral("pt") || value == QStringLiteral("pt_br")) {
        return QStringLiteral("pt_BR");
    }
    if (value == QStringLiteral("en") || value == QStringLiteral("en_us")) {
        return QStringLiteral("en_US");
    }
    return QStringLiteral("pt_BR");
}

QString get_manual_title(const QString &lang)
{
    Q_UNUSED(lang);
    return source::utils::titulo_janela_manual();
}

QString to_unicode_bold(const QString &text)
{
    QString output;
    output.reserve(text.size());
    for (QChar ch : text) {
        const char32_t u = ch.unicode();
        if (u >= 'A' && u <= 'Z') {
            const char32_t codepoint = 0x1D400 + (u - 'A');
            output.append(QString::fromUcs4(&codepoint, 1));
        } else if (u >= 'a' && u <= 'z') {
            const char32_t codepoint = 0x1D41A + (u - 'a');
            output.append(QString::fromUcs4(&codepoint, 1));
        } else if (u >= '0' && u <= '9') {
            const char32_t codepoint = 0x1D7CE + (u - '0');
            output.append(QString::fromUcs4(&codepoint, 1));
        } else {
            output.append(ch);
        }
    }
    return output;
}

QList<ManualSection> get_manual_document(const QString &lang,
                                         bool include_pomodoro_plugin,
                                         bool include_feynman_plugin,
                                         bool include_mapamental_plugin,
                                         bool include_audiolivro_plugin,
                                         bool include_agenda_plugin,
                                         bool include_modelo_plugin,
                                         bool include_gantt_plugin)
{
    const QString normalized = normalize_language(lang);
    QList<ManualSection> document = normalized == QStringLiteral("en_US")
                                        ? incore_17_manual_en_US::en_document()
                                        : incore_16_manual_pt_BR::pt_document();
    append_plugin_sections(document,
                           normalized,
                           include_pomodoro_plugin,
                           include_feynman_plugin,
                           include_mapamental_plugin,
                           include_audiolivro_plugin,
                           include_agenda_plugin,
                           include_modelo_plugin,
                           include_gantt_plugin);
    const QList<ManualSection> final_sections = normalized == QStringLiteral("en_US")
                                                    ? incore_17_manual_en_US::final_sections_en_document()
                                                    : incore_16_manual_pt_BR::final_sections_pt_document();
    document.append(final_sections);
    return document;
}

QPair<QList<ManualBlock>, QStringList> get_manual_blocks(const QString &lang,
                                                         bool include_pomodoro_plugin,
                                                         bool include_feynman_plugin,
                                                         bool include_mapamental_plugin,
                                                         bool include_audiolivro_plugin,
                                                         bool include_agenda_plugin,
                                                         bool include_modelo_plugin,
                                                         bool include_gantt_plugin)
{
    const QString normalized = normalize_language(lang);
    const QList<ManualSection> sections =
        get_manual_document(normalized,
                            include_pomodoro_plugin,
                            include_feynman_plugin,
                            include_mapamental_plugin,
                            include_audiolivro_plugin,
                            include_agenda_plugin,
                            include_modelo_plugin,
                            include_gantt_plugin);
    QList<ManualBlock> blocks;
    QStringList order;

    const QString title = get_manual_title(normalized);
    blocks.append({QStringLiteral("line"), title, QString()});
    blocks.append({QStringLiteral("line"), QString(title.size(), QLatin1Char('=')), QString()});
    blocks.append({QStringLiteral("blank"), QString(), QString()});

    blocks.append({QStringLiteral("paragraph"),
                   manual_intro_text(normalized),
                   QString()});
    blocks.append({QStringLiteral("blank"), QString(), QString()});
    blocks.append({QStringLiteral("toc_title"),
                   normalized == QStringLiteral("en_US") ? QStringLiteral("Table of Contents") : QStringLiteral("Índice"),
                   QString()});

    for (int i = 0; i < sections.size(); ++i) {
        blocks.append({QStringLiteral("toc_item"), QStringLiteral("%1. %2").arg(i + 1).arg(sections.at(i).title), sections.at(i).id});
    }

    blocks.append({QStringLiteral("blank"), QString(), QString()});
    blocks.append({QStringLiteral("divider"), QString(60, QLatin1Char('-')), QString()});
    blocks.append({QStringLiteral("blank"), QString(), QString()});

    for (const ManualSection &section : sections) {
        order << section.id;
        blocks.append({QStringLiteral("section_title"), section.title, section.id});
        blocks.append({QStringLiteral("blank"), QString(), QString()});
        for (const QString &paragraph : section.paragraphs) {
            blocks.append({QStringLiteral("paragraph"), paragraph, QString()});
            blocks.append({QStringLiteral("blank"), QString(), QString()});
        }
        for (const QString &bullet : section.bullets) {
            blocks.append({QStringLiteral("bullet"), bullet, QString()});
        }
        if (!section.bullets.isEmpty()) {
            blocks.append({QStringLiteral("blank"), QString(), QString()});
        }
        for (const ManualDetails &detail : section.details) {
            blocks.append({QStringLiteral("detail_title"), detail.summary, QString()});
            blocks.append({QStringLiteral("blank"), QString(), QString()});
            for (const QString &paragraph : detail.paragraphs) {
                blocks.append({QStringLiteral("paragraph"), paragraph, QString()});
                blocks.append({QStringLiteral("blank"), QString(), QString()});
            }
            for (const QString &bullet : detail.bullets) {
                blocks.append({QStringLiteral("bullet"), bullet, QString()});
            }
            if (!detail.bullets.isEmpty()) {
                blocks.append({QStringLiteral("blank"), QString(), QString()});
            }
        }
        blocks.append({QStringLiteral("divider"), QString(60, QLatin1Char('-')), QString()});
        blocks.append({QStringLiteral("blank"), QString(), QString()});
    }

    return {blocks, order};
}

QString get_manual_text(const QString &lang,
                        bool include_pomodoro_plugin,
                        bool include_feynman_plugin,
                        bool include_mapamental_plugin,
                        bool include_audiolivro_plugin,
                        bool include_agenda_plugin,
                        bool include_modelo_plugin,
                        bool include_gantt_plugin)
{
    return get_manual_text_with_positions(lang,
                                          include_pomodoro_plugin,
                                          include_feynman_plugin,
                                          include_mapamental_plugin,
                                          include_audiolivro_plugin,
                                          include_agenda_plugin,
                                          include_modelo_plugin,
                                          include_gantt_plugin)
        .text;
}

ManualTextWithPositions get_manual_text_with_positions(const QString &lang,
                                                       bool include_pomodoro_plugin,
                                                       bool include_feynman_plugin,
                                                       bool include_mapamental_plugin,
                                                       bool include_audiolivro_plugin,
                                                       bool include_agenda_plugin,
                                                       bool include_modelo_plugin,
                                                       bool include_gantt_plugin)
{
    const QString normalized = normalize_language(lang);
    const QList<ManualSection> sections =
        get_manual_document(normalized,
                            include_pomodoro_plugin,
                            include_feynman_plugin,
                            include_mapamental_plugin,
                            include_audiolivro_plugin,
                            include_agenda_plugin,
                            include_modelo_plugin,
                            include_gantt_plugin);

    QStringList lines;
    QMap<QString, int> positions;
    QStringList order;

    auto add_line = [&lines](const QString &line = QString()) { lines << line; };
    auto current_offset = [&lines]() {
        int total = 0;
        for (const QString &line : lines) {
            total += line.size() + 1;
        }
        return total;
    };

    const QString title = get_manual_title(normalized);
    add_line(title);
    add_line(QString(title.size(), QLatin1Char('=')));
    add_line();
    add_line(manual_intro_text(normalized));
    add_line();
    add_line(normalized == QStringLiteral("en_US") ? QStringLiteral("Table of Contents") : QStringLiteral("Índice"));
    add_line(QStringLiteral("----------"));
    for (int i = 0; i < sections.size(); ++i) {
        add_line(QStringLiteral("%1. %2").arg(i + 1).arg(sections.at(i).title));
    }
    add_line();
    add_line(QString(60, QLatin1Char('-')));
    add_line();

    for (const ManualSection &section : sections) {
        positions.insert(section.id, current_offset());
        order << section.id;
        add_line(section.title);
        add_line(QString(section.title.size(), QLatin1Char('-')));
        add_line();
        for (const QString &paragraph : section.paragraphs) {
            add_line(paragraph);
            add_line();
        }
        for (const QString &bullet : section.bullets) {
            add_line(QStringLiteral("- %1").arg(bullet));
        }
        if (!section.bullets.isEmpty()) {
            add_line();
        }
        for (const ManualDetails &detail : section.details) {
            add_line(detail.summary);
            add_line(QString(detail.summary.size(), QLatin1Char('.')));
            add_line();
            for (const QString &paragraph : detail.paragraphs) {
                add_line(paragraph);
                add_line();
            }
            for (const QString &bullet : detail.bullets) {
                add_line(QStringLiteral("- %1").arg(bullet));
            }
            if (!detail.bullets.isEmpty()) {
                add_line();
            }
        }
        add_line(QString(60, QLatin1Char('-')));
        add_line();
    }

    return {lines.join(QLatin1Char('\n')), positions, order};
}

}  // namespace source::InterfaceCore::incore_12_Manual
