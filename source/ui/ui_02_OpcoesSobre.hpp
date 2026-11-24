#pragma once

#include <QString>
#include <QVector>

namespace source::GerenciamentoUI {

struct DocumentoSobre {
    QString titulo;
    QString texto;
};

class OpcoesSobre {
public:
    static const QString &SITE_LICENSES();
    static QString LICENSE_TEXT_PT_BR(bool include_pomodoro_plugin = false,
                                      bool include_feynman_plugin = false,
                                      bool include_mapamental_plugin = false,
                                      bool include_audiolivro_plugin = false,
                                      bool include_agenda_plugin = false,
                                      bool include_modelo_plugin = false,
                                      bool include_gantt_plugin = false);
    static QString LICENSE_TEXT_EN_US(bool include_pomodoro_plugin = false,
                                      bool include_feynman_plugin = false,
                                      bool include_mapamental_plugin = false,
                                      bool include_audiolivro_plugin = false,
                                      bool include_agenda_plugin = false,
                                      bool include_modelo_plugin = false,
                                      bool include_gantt_plugin = false);
    static QString NOTICE_TEXT_PT_BR(bool include_pomodoro_plugin = false,
                                     bool include_feynman_plugin = false,
                                     bool include_mapamental_plugin = false,
                                     bool include_audiolivro_plugin = false,
                                     bool include_agenda_plugin = false,
                                     bool include_modelo_plugin = false,
                                     bool include_gantt_plugin = false);
    static QString NOTICE_TEXT_EN_US(bool include_pomodoro_plugin = false,
                                     bool include_feynman_plugin = false,
                                     bool include_mapamental_plugin = false,
                                     bool include_audiolivro_plugin = false,
                                     bool include_agenda_plugin = false,
                                     bool include_modelo_plugin = false,
                                     bool include_gantt_plugin = false);
    static QString ABOUT_TEXT_PT_BR(bool include_pomodoro_plugin = false,
                                    bool include_feynman_plugin = false,
                                    bool include_mapamental_plugin = false,
                                    bool include_audiolivro_plugin = false,
                                    bool include_agenda_plugin = false,
                                    bool include_modelo_plugin = false,
                                    bool include_gantt_plugin = false);
    static QString ABOUT_TEXT_EN_US(bool include_pomodoro_plugin = false,
                                    bool include_feynman_plugin = false,
                                    bool include_mapamental_plugin = false,
                                    bool include_audiolivro_plugin = false,
                                    bool include_agenda_plugin = false,
                                    bool include_modelo_plugin = false,
                                    bool include_gantt_plugin = false);
    static QVector<DocumentoSobre> About_Documents_pt_BR(bool include_pomodoro_plugin = false,
                                                         bool include_feynman_plugin = false,
                                                         bool include_mapamental_plugin = false,
                                                         bool include_audiolivro_plugin = false,
                                                         bool include_agenda_plugin = false,
                                                         bool include_modelo_plugin = false,
                                                         bool include_gantt_plugin = false);
    static QVector<DocumentoSobre> About_Documents_en_US(bool include_pomodoro_plugin = false,
                                                         bool include_feynman_plugin = false,
                                                         bool include_mapamental_plugin = false,
                                                         bool include_audiolivro_plugin = false,
                                                         bool include_agenda_plugin = false,
                                                         bool include_modelo_plugin = false,
                                                         bool include_gantt_plugin = false);
    static QVector<DocumentoSobre> License_Documents_pt_BR(bool include_pomodoro_plugin = false,
                                                           bool include_feynman_plugin = false,
                                                           bool include_mapamental_plugin = false,
                                                           bool include_audiolivro_plugin = false,
                                                           bool include_agenda_plugin = false,
                                                           bool include_modelo_plugin = false,
                                                           bool include_gantt_plugin = false);
    static QVector<DocumentoSobre> License_Documents_en_US(bool include_pomodoro_plugin = false,
                                                           bool include_feynman_plugin = false,
                                                           bool include_mapamental_plugin = false,
                                                           bool include_audiolivro_plugin = false,
                                                           bool include_agenda_plugin = false,
                                                           bool include_modelo_plugin = false,
                                                           bool include_gantt_plugin = false);
    static QVector<DocumentoSobre> Contract_Documents_pt_BR(bool include_pomodoro_plugin = false,
                                                            bool include_feynman_plugin = false,
                                                            bool include_mapamental_plugin = false,
                                                            bool include_audiolivro_plugin = false,
                                                            bool include_agenda_plugin = false,
                                                            bool include_modelo_plugin = false,
                                                            bool include_gantt_plugin = false);
    static QVector<DocumentoSobre> Contract_Documents_en_US(bool include_pomodoro_plugin = false,
                                                            bool include_feynman_plugin = false,
                                                            bool include_mapamental_plugin = false,
                                                            bool include_audiolivro_plugin = false,
                                                            bool include_agenda_plugin = false,
                                                            bool include_modelo_plugin = false,
                                                            bool include_gantt_plugin = false);
    static QVector<DocumentoSobre> Copyright_Documents_pt_BR(bool include_pomodoro_plugin = false,
                                                             bool include_feynman_plugin = false,
                                                             bool include_mapamental_plugin = false,
                                                             bool include_audiolivro_plugin = false,
                                                             bool include_agenda_plugin = false,
                                                             bool include_modelo_plugin = false,
                                                             bool include_gantt_plugin = false);
    static QVector<DocumentoSobre> Copyright_Documents_en_US(bool include_pomodoro_plugin = false,
                                                             bool include_feynman_plugin = false,
                                                             bool include_mapamental_plugin = false,
                                                             bool include_audiolivro_plugin = false,
                                                             bool include_agenda_plugin = false,
                                                             bool include_modelo_plugin = false,
                                                             bool include_gantt_plugin = false);
    static QVector<DocumentoSobre> Notice_Documents_pt_BR(bool include_pomodoro_plugin = false,
                                                          bool include_feynman_plugin = false,
                                                          bool include_mapamental_plugin = false,
                                                          bool include_audiolivro_plugin = false,
                                                          bool include_agenda_plugin = false,
                                                          bool include_modelo_plugin = false,
                                                          bool include_gantt_plugin = false);
    static QVector<DocumentoSobre> Notice_Documents_en_US(bool include_pomodoro_plugin = false,
                                                          bool include_feynman_plugin = false,
                                                          bool include_mapamental_plugin = false,
                                                          bool include_audiolivro_plugin = false,
                                                          bool include_agenda_plugin = false,
                                                          bool include_modelo_plugin = false,
                                                          bool include_gantt_plugin = false);
    static QString Privacy_Policy_pt_BR(bool include_pomodoro_plugin = false,
                                        bool include_feynman_plugin = false,
                                        bool include_mapamental_plugin = false,
                                        bool include_audiolivro_plugin = false,
                                        bool include_agenda_plugin = false,
                                        bool include_modelo_plugin = false,
                                        bool include_gantt_plugin = false);
    static QString Privacy_Policy_en_US(bool include_pomodoro_plugin = false,
                                        bool include_feynman_plugin = false,
                                        bool include_mapamental_plugin = false,
                                        bool include_audiolivro_plugin = false,
                                        bool include_agenda_plugin = false,
                                        bool include_modelo_plugin = false,
                                        bool include_gantt_plugin = false);
    static QVector<DocumentoSobre> Privacy_Policy_Documents_pt_BR(bool include_pomodoro_plugin = false,
                                                                  bool include_feynman_plugin = false,
                                                                  bool include_mapamental_plugin = false,
                                                                  bool include_audiolivro_plugin = false,
                                                                  bool include_agenda_plugin = false,
                                                                  bool include_modelo_plugin = false,
                                                                  bool include_gantt_plugin = false);
    static QVector<DocumentoSobre> Privacy_Policy_Documents_en_US(bool include_pomodoro_plugin = false,
                                                                  bool include_feynman_plugin = false,
                                                                  bool include_mapamental_plugin = false,
                                                                  bool include_audiolivro_plugin = false,
                                                                  bool include_agenda_plugin = false,
                                                                  bool include_modelo_plugin = false,
                                                                  bool include_gantt_plugin = false);
    static QString History_APP_pt_BR(bool include_pomodoro_plugin = false,
                                     bool include_feynman_plugin = false,
                                     bool include_mapamental_plugin = false,
                                     bool include_audiolivro_plugin = false,
                                     bool include_agenda_plugin = false,
                                     bool include_modelo_plugin = false,
                                     bool include_gantt_plugin = false);
    static QString History_APP_en_US(bool include_pomodoro_plugin = false,
                                     bool include_feynman_plugin = false,
                                     bool include_mapamental_plugin = false,
                                     bool include_audiolivro_plugin = false,
                                     bool include_agenda_plugin = false,
                                     bool include_modelo_plugin = false,
                                     bool include_gantt_plugin = false);
    static QString RELEASE_NOTES_pt_BR(bool include_pomodoro_plugin = false,
                                       bool include_feynman_plugin = false,
                                       bool include_mapamental_plugin = false,
                                       bool include_audiolivro_plugin = false,
                                       bool include_agenda_plugin = false,
                                       bool include_modelo_plugin = false,
                                       bool include_gantt_plugin = false);
    static QString RELEASE_NOTES_en_US(bool include_pomodoro_plugin = false,
                                       bool include_feynman_plugin = false,
                                       bool include_mapamental_plugin = false,
                                       bool include_audiolivro_plugin = false,
                                       bool include_agenda_plugin = false,
                                       bool include_modelo_plugin = false,
                                       bool include_gantt_plugin = false);
};

}  // namespace source::GerenciamentoUI
