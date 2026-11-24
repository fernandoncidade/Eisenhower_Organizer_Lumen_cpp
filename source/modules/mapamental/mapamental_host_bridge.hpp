#pragma once

#include <functional>
#include <utility>

#include <QLocale>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <Qt>

class QObject;

namespace source::plugin {

class MapaMentalHostBridge {
public:
    using StandardButtonsHandler = std::function<void(QObject *)>;
    using ModelAvailabilityChecker = std::function<bool()>;
    using ModelDiagnosticHandler = std::function<QVariantMap()>;
    using ModelAnalysisHandler = std::function<QVariantMap(const QString &,
                                                           const QString &,
                                                           const QVariantList &,
                                                           const QVariantList &)>;

    void set_language_code(const QString &language_code)
    {
        language_code_ = language_code.trimmed().isEmpty() ? QLocale::system().name() : language_code.trimmed();
    }

    QString language_code() const
    {
        return language_code_;
    }

    bool is_english() const
    {
        return language_code_.startsWith(QStringLiteral("en"), Qt::CaseInsensitive);
    }

    void set_standard_buttons_handler(StandardButtonsHandler handler)
    {
        standard_buttons_handler_ = std::move(handler);
    }

    void translate_standard_buttons(QObject *dialog) const
    {
        if (standard_buttons_handler_) {
            standard_buttons_handler_(dialog);
        }
    }

    void set_model_availability_checker(ModelAvailabilityChecker checker)
    {
        model_availability_checker_ = std::move(checker);
    }

    bool model_available() const
    {
        return model_analysis_handler_
               && (!model_availability_checker_ || model_availability_checker_());
    }

    void set_model_diagnostic_handler(ModelDiagnosticHandler handler)
    {
        model_diagnostic_handler_ = std::move(handler);
    }

    QVariantMap model_diagnostic() const
    {
        return model_diagnostic_handler_ ? model_diagnostic_handler_() : QVariantMap();
    }

    void set_model_analysis_handler(ModelAnalysisHandler handler)
    {
        model_analysis_handler_ = std::move(handler);
    }

    QVariantMap analyze_with_model(const QString &text,
                                   const QString &language,
                                   const QVariantList &concepts,
                                   const QVariantList &relations) const
    {
        if (!model_available()) {
            return {};
        }
        return model_analysis_handler_(text, language, concepts, relations);
    }

private:
    QString language_code_ = QLocale::system().name();
    StandardButtonsHandler standard_buttons_handler_;
    ModelAvailabilityChecker model_availability_checker_;
    ModelDiagnosticHandler model_diagnostic_handler_;
    ModelAnalysisHandler model_analysis_handler_;
};

}  // namespace source::plugin
