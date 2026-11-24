#pragma once

#include <QDate>
#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTabWidget;

namespace mocks::version_editor {

// Visual editor and checker for version strings across the host application and all integrated modules (former plugins).
class VersionEditorWidget : public QWidget {
public:
    explicit VersionEditorWidget(QWidget *parent = nullptr);

    static bool updateAllRegisteredFiles(const QString &base_dir,
                                         const QString &pt_version,
                                         const QDate &pt_date,
                                         const QString &en_version,
                                         const QDate &en_date);

private:
    void selectBaseDirectory();
    void checkFiles();
    void saveChanges();
    void setStatusLines(const QStringList &lines);
    void setStatusGroups(const QList<QPair<QString, QStringList>> &groups);

    QString selected_base_dir_;

    QLineEdit *folder_path_ = nullptr;

    QLineEdit *pt_version_ = nullptr;
    QSpinBox *pt_day_ = nullptr;
    QComboBox *pt_month_ = nullptr;
    QSpinBox *pt_year_ = nullptr;

    QLineEdit *en_version_ = nullptr;
    QSpinBox *en_day_ = nullptr;
    QComboBox *en_month_ = nullptr;
    QSpinBox *en_year_ = nullptr;

    QPushButton *select_folder_btn_ = nullptr;
    QPushButton *check_btn_ = nullptr;
    QPushButton *save_btn_ = nullptr;
    QPushButton *refresh_btn_ = nullptr;

    QTabWidget *status_tabs_ = nullptr;
};

}  // namespace mocks::version_editor
