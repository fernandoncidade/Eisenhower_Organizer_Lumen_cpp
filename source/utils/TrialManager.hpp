#pragma once

#include <QJsonObject>
#include <QString>
#include <optional>

class QWidget;

namespace source::utils {

class TrialManager {
public:
    static QString get_config_path();
    static std::optional<quint64> get_first_run_timestamp();
    static void set_first_run_timestamp(quint64 timestamp);
    static void delete_first_run_timestamp();
    static QJsonObject get_trial_info();
    static bool is_trial_expired();
    static void enforce_trial(QWidget *parent = nullptr);

private:
    static constexpr const char *CONFIG_FILE = "trial_info.json";
    static constexpr const char *DEFAULT_TRIAL = "days";
    static constexpr int DEFAULT_TRIAL_VALUE = 7;
    static constexpr const char *PAID_VERSION_URL = "ms-windows-store://pdp/?productid=9P289X0185C3";
    static constexpr bool LIBERAR_USO_DEFINITIVO = false;
};

}  // namespace source::utils
