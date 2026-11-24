#pragma once

#include <algorithm>
#include <exception>

#include <QAbstractItemView>
#include <QApplication>
#include <QColor>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QEvent>
#include <QFont>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPalette>
#include <QProgressBar>
#include <QPushButton>
#include <QSaveFile>
#include <QSizePolicy>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>

#include "source/modules/pomodoro/Pomodoro/tmp_02_PomodoroTimer.hpp"

namespace source::plugin::Pomodoro::PomodoroTimerInternal {

inline source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

inline QString pomodoro_timer_config_path()
{
    return QDir(compat::plugin_persistent_dir()).filePath(QStringLiteral("pomodoro_timer_config.json"));
}

inline int carregar_ciclos_permitidos_persistidos()
{
    const QString path = pomodoro_timer_config_path();
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        const auto doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject()) {
            const int c = doc.object().value(QStringLiteral("ciclos_permitidos")).toInt();
            if (c > 0) {
                return c;
            }
        }
    }
    return 4;
}

inline void salvar_configuracoes_timer_persistidas(int ciclos, int foco_segundos, int descanso_segundos)
{
    const QString path = pomodoro_timer_config_path();
    QDir().mkpath(QFileInfo(path).absolutePath());
    QJsonObject obj;
    obj.insert(QStringLiteral("ciclos_permitidos"), qMax(1, ciclos));
    obj.insert(QStringLiteral("tempo_foco"), foco_segundos > 0 ? foco_segundos : 25 * 60);
    obj.insert(QStringLiteral("tempo_descanso_curto"), descanso_segundos > 0 ? descanso_segundos : 5 * 60);
    QSaveFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        const QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Indented);
        if (file.write(data) == data.size()) {
            file.commit();
        }
    }
}

inline bool is_light_color(const QColor &color)
{
    const double lum =
        (0.2126 * color.red() + 0.7152 * color.green() + 0.0722 * color.blue()) / 255.0;
    return lum >= 0.85;
}

inline QString normalized_ui_layout_key(const QString &value)
{
    return layout::normalized_ui_layout_key(value);
}

inline bool is_robust_ui_layout(const QString &value)
{
    return layout::is_robust_ui_layout(value);
}

}  // namespace source::plugin::Pomodoro::PomodoroTimerInternal
