#include "GanttHolidaysDialog.hpp"
#include "BiDirectionalResizeHeaderView.hpp"
#include "GanttCellHoverDelegate.hpp"
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>
#include <QTextCharFormat>
#include <QVBoxLayout>
#include <QTreeWidgetItem>
#include <algorithm>

namespace source::gantt {

static QDate calculate_easter_sunday(int year)
{
    const int a = year % 19;
    const int b = year / 100;
    const int c = year % 100;
    const int d = b / 4;
    const int e = b % 4;
    const int f = (b + 8) / 25;
    const int g = (b - f + 1) / 3;
    const int h = (19 * a + b - d - g + 15) % 30;
    const int i = c / 4;
    const int k = c % 4;
    const int l = (32 + 2 * e + 2 * i - h - k) % 7;
    const int m = (a + 11 * h + 22 * l) / 451;
    const int month = (h + l - 7 * m + 114) / 31;
    const int day = ((h + l - 7 * m + 114) % 31) + 1;
    return QDate(year, month, day);
}

static QDate nth_weekday_of_month(int year, int month, int target_dow, int occurrence)
{
    QDate d(year, month, 1);
    int count = 0;
    while (d.month() == month) {
        if (d.dayOfWeek() == target_dow) {
            count++;
            if (count == occurrence) {
                return d;
            }
        }
        d = d.addDays(1);
    }
    return QDate();
}

static QDate last_weekday_of_month(int year, int month, int target_dow)
{
    QDate d(year, month, 1);
    d = d.addMonths(1).addDays(-1);
    while (d.month() == month) {
        if (d.dayOfWeek() == target_dow) {
            return d;
        }
        d = d.addDays(-1);
    }
    return QDate();
}

static QList<QPair<QDate, QString>> generate_country_holidays(const QString &code, int year)
{
    QList<QPair<QDate, QString>> list;
    const QDate easter = calculate_easter_sunday(year);

    auto add = [&](int m, int d, const QString &name) {
        QDate dt(year, m, d);
        if (dt.isValid()) list.append({dt, name});
    };
    auto add_rel = [&](int offset, const QString &name) {
        list.append({easter.addDays(offset), name});
    };
    auto add_nth = [&](int m, int dow, int occ, const QString &name) {
        QDate dt = nth_weekday_of_month(year, m, dow, occ);
        if (dt.isValid()) list.append({dt, name});
    };
    auto add_last = [&](int m, int dow, const QString &name) {
        QDate dt = last_weekday_of_month(year, m, dow);
        if (dt.isValid()) list.append({dt, name});
    };

    if (code == QStringLiteral("BR")) {
        add(1, 1, GanttHolidaysDialog::tr("Confraternização Universal (Ano Novo)"));
        add_rel(-47, GanttHolidaysDialog::tr("Carnaval"));
        add_rel(-2, GanttHolidaysDialog::tr("Sexta-feira Santa"));
        add_rel(0, GanttHolidaysDialog::tr("Páscoa"));
        add(4, 21, GanttHolidaysDialog::tr("Tiradentes"));
        add(5, 1, GanttHolidaysDialog::tr("Dia Mundial do Trabalho"));
        add_rel(60, GanttHolidaysDialog::tr("Corpus Christi"));
        add(9, 7, GanttHolidaysDialog::tr("Independência do Brasil"));
        add(10, 12, GanttHolidaysDialog::tr("Nossa Senhora Aparecida"));
        add(11, 2, GanttHolidaysDialog::tr("Finados"));
        add(11, 15, GanttHolidaysDialog::tr("Proclamação da República"));
        add(11, 20, GanttHolidaysDialog::tr("Dia Nacional de Zumbi e da Consciência Negra"));
        add(12, 25, GanttHolidaysDialog::tr("Natal"));
    } else if (code == QStringLiteral("US")) {
        add(1, 1, GanttHolidaysDialog::tr("New Year's Day"));
        add_nth(1, 1, 3, GanttHolidaysDialog::tr("Martin Luther King Jr. Day"));
        add_nth(2, 1, 3, GanttHolidaysDialog::tr("Presidents' Day"));
        add_last(5, 1, GanttHolidaysDialog::tr("Memorial Day"));
        add(6, 19, GanttHolidaysDialog::tr("Juneteenth National Independence Day"));
        add(7, 4, GanttHolidaysDialog::tr("Independence Day"));
        add_nth(9, 1, 1, GanttHolidaysDialog::tr("Labor Day"));
        add_nth(10, 1, 2, GanttHolidaysDialog::tr("Columbus Day"));
        add(11, 11, GanttHolidaysDialog::tr("Veterans Day"));
        add_nth(11, 4, 4, GanttHolidaysDialog::tr("Thanksgiving Day"));
        add(12, 25, GanttHolidaysDialog::tr("Christmas Day"));
    } else if (code == QStringLiteral("PT")) {
        add(1, 1, GanttHolidaysDialog::tr("Ano Novo"));
        add_rel(-2, GanttHolidaysDialog::tr("Sexta-feira Santa"));
        add_rel(0, GanttHolidaysDialog::tr("Domingo de Páscoa"));
        add(4, 25, GanttHolidaysDialog::tr("Dia da Liberdade"));
        add(5, 1, GanttHolidaysDialog::tr("Dia do Trabalhador"));
        add_rel(60, GanttHolidaysDialog::tr("Corpo de Deus"));
        add(6, 10, GanttHolidaysDialog::tr("Dia de Portugal"));
        add(8, 15, GanttHolidaysDialog::tr("Assunção de Nossa Senhora"));
        add(10, 5, GanttHolidaysDialog::tr("Implantação da República"));
        add(11, 1, GanttHolidaysDialog::tr("Dia de Todos os Santos"));
        add(12, 1, GanttHolidaysDialog::tr("Restauração da Independência"));
        add(12, 8, GanttHolidaysDialog::tr("Imaculada Conceição"));
        add(12, 25, GanttHolidaysDialog::tr("Natal"));
    } else if (code == QStringLiteral("AR")) {
        add(1, 1, GanttHolidaysDialog::tr("Año Nuevo"));
        add_rel(-48, GanttHolidaysDialog::tr("Carnaval"));
        add_rel(-47, GanttHolidaysDialog::tr("Carnaval"));
        add(3, 24, GanttHolidaysDialog::tr("Día de la Memoria por la Verdad y la Justicia"));
        add(4, 2, GanttHolidaysDialog::tr("Día del Veterano y de los Caídos en Malvinas"));
        add_rel(-2, GanttHolidaysDialog::tr("Viernes Santo"));
        add(5, 1, GanttHolidaysDialog::tr("Día del Trabajador"));
        add(5, 25, GanttHolidaysDialog::tr("Día de la Revolución de Mayo"));
        add(6, 20, GanttHolidaysDialog::tr("Paso a la Inmortalidad del Gral. Manuel Belgrano"));
        add(7, 9, GanttHolidaysDialog::tr("Día de la Independencia"));
        add(12, 8, GanttHolidaysDialog::tr("Inmaculada Concepción"));
        add(12, 25, GanttHolidaysDialog::tr("Navidad"));
    } else if (code == QStringLiteral("CA")) {
        add(1, 1, GanttHolidaysDialog::tr("New Year's Day"));
        add_rel(-2, GanttHolidaysDialog::tr("Good Friday"));
        QDate may24(year, 5, 24);
        while (may24.dayOfWeek() != 1) may24 = may24.addDays(-1);
        list.append({may24, GanttHolidaysDialog::tr("Victoria Day")});
        add(7, 1, GanttHolidaysDialog::tr("Canada Day"));
        add_nth(9, 1, 1, GanttHolidaysDialog::tr("Labour Day"));
        add(9, 30, GanttHolidaysDialog::tr("National Day for Truth and Reconciliation"));
        add_nth(10, 1, 2, GanttHolidaysDialog::tr("Thanksgiving"));
        add(11, 11, GanttHolidaysDialog::tr("Remembrance Day"));
        add(12, 25, GanttHolidaysDialog::tr("Christmas Day"));
        add(12, 26, GanttHolidaysDialog::tr("Boxing Day"));
    } else if (code == QStringLiteral("GB")) {
        add(1, 1, GanttHolidaysDialog::tr("New Year's Day"));
        add_rel(-2, GanttHolidaysDialog::tr("Good Friday"));
        add_rel(1, GanttHolidaysDialog::tr("Easter Monday"));
        add_nth(5, 1, 1, GanttHolidaysDialog::tr("Early May Bank Holiday"));
        add_last(5, 1, GanttHolidaysDialog::tr("Spring Bank Holiday"));
        add_last(8, 1, GanttHolidaysDialog::tr("Summer Bank Holiday"));
        add(12, 25, GanttHolidaysDialog::tr("Christmas Day"));
        add(12, 26, GanttHolidaysDialog::tr("Boxing Day"));
    } else if (code == QStringLiteral("FR")) {
        add(1, 1, GanttHolidaysDialog::tr("Jour de l'An"));
        add_rel(1, GanttHolidaysDialog::tr("Lundi de Pâques"));
        add(5, 1, GanttHolidaysDialog::tr("Fête du Travail"));
        add(5, 8, GanttHolidaysDialog::tr("Victoire 1945"));
        add_rel(39, GanttHolidaysDialog::tr("Ascension"));
        add_rel(50, GanttHolidaysDialog::tr("Lundi de Pentecôte"));
        add(7, 14, GanttHolidaysDialog::tr("Fête Nationale"));
        add(8, 15, GanttHolidaysDialog::tr("Assomption"));
        add(11, 1, GanttHolidaysDialog::tr("Toussaint"));
        add(11, 11, GanttHolidaysDialog::tr("Armistice 1918"));
        add(12, 25, GanttHolidaysDialog::tr("Noël"));
    } else if (code == QStringLiteral("DE")) {
        add(1, 1, GanttHolidaysDialog::tr("Neujahr"));
        add_rel(-2, GanttHolidaysDialog::tr("Karfreitag"));
        add_rel(1, GanttHolidaysDialog::tr("Ostermontag"));
        add(5, 1, GanttHolidaysDialog::tr("Tag der Arbeit"));
        add_rel(39, GanttHolidaysDialog::tr("Christi Himmelfahrt"));
        add_rel(50, GanttHolidaysDialog::tr("Pfingstmontag"));
        add(10, 3, GanttHolidaysDialog::tr("Tag der Deutschen Einheit"));
        add(12, 25, GanttHolidaysDialog::tr("1. Weihnachtstag"));
        add(12, 26, GanttHolidaysDialog::tr("2. Weihnachtstag"));
    } else if (code == QStringLiteral("IT")) {
        add(1, 1, GanttHolidaysDialog::tr("Capodanno"));
        add(1, 6, GanttHolidaysDialog::tr("Epifania"));
        add_rel(1, GanttHolidaysDialog::tr("Lunedì dell'Angelo"));
        add(4, 25, GanttHolidaysDialog::tr("Festa della Liberazione"));
        add(5, 1, GanttHolidaysDialog::tr("Festa del Lavoro"));
        add(6, 2, GanttHolidaysDialog::tr("Festa della Repubblica"));
        add(8, 15, GanttHolidaysDialog::tr("Ferragosto (Assunzione)"));
        add(11, 1, GanttHolidaysDialog::tr("Tutti i Santi"));
        add(12, 8, GanttHolidaysDialog::tr("Immacolata Concezione"));
        add(12, 25, GanttHolidaysDialog::tr("Natale"));
        add(12, 26, GanttHolidaysDialog::tr("Santo Stefano"));
    } else if (code == QStringLiteral("ES")) {
        add(1, 1, GanttHolidaysDialog::tr("Año Nuevo"));
        add(1, 6, GanttHolidaysDialog::tr("Epifanía del Señor"));
        add_rel(-2, GanttHolidaysDialog::tr("Viernes Santo"));
        add(5, 1, GanttHolidaysDialog::tr("Fiesta del Trabajo"));
        add(8, 15, GanttHolidaysDialog::tr("Asunción de la Virgen"));
        add(10, 12, GanttHolidaysDialog::tr("Fiesta Nacional de España"));
        add(11, 1, GanttHolidaysDialog::tr("Todos los Santos"));
        add(12, 6, GanttHolidaysDialog::tr("Día de la Constitución"));
        add(12, 8, GanttHolidaysDialog::tr("Inmaculada Concepción"));
        add(12, 25, GanttHolidaysDialog::tr("Natividad del Señor"));
    } else if (code == QStringLiteral("JP")) {
        add(1, 1, GanttHolidaysDialog::tr("Ganjitsu (Ano Novo)"));
        add_nth(1, 1, 2, GanttHolidaysDialog::tr("Seijin no Hi (Dia da Maioridade)"));
        add(2, 11, GanttHolidaysDialog::tr("Kenkoku Kinen no Hi (Fundação Nacional)"));
        add(2, 23, GanttHolidaysDialog::tr("Tennō Tanjōbi (Aniversário do Imperador)"));
        add(3, 20, GanttHolidaysDialog::tr("Shunbun no Hi (Equinócio de Primavera)"));
        add(4, 29, GanttHolidaysDialog::tr("Shōwa no Hi"));
        add(5, 3, GanttHolidaysDialog::tr("Kenpō Kinenbi (Dia da Constituição)"));
        add(5, 4, GanttHolidaysDialog::tr("Midori no Hi (Dia do Verde)"));
        add(5, 5, GanttHolidaysDialog::tr("Kodomo no Hi (Dia das Crianças)"));
        add_nth(7, 1, 3, GanttHolidaysDialog::tr("Umi no Hi (Dia do Mar)"));
        add(8, 11, GanttHolidaysDialog::tr("Yama no Hi (Dia da Montanha)"));
        add_nth(9, 1, 3, GanttHolidaysDialog::tr("Keirō no Hi (Respeito aos Idosos)"));
        add(9, 23, GanttHolidaysDialog::tr("Shūbun no Hi (Equinócio de Outono)"));
        add_nth(10, 1, 2, GanttHolidaysDialog::tr("Sports no Hi (Dia dos Esportes)"));
        add(11, 3, GanttHolidaysDialog::tr("Bunka no Hi (Dia da Cultura)"));
        add(11, 23, GanttHolidaysDialog::tr("Kinrō Kansha no Hi (Ação de Graças ao Trabalho)"));
    } else if (code == QStringLiteral("CN")) {
        add(1, 1, GanttHolidaysDialog::tr("Ano Novo Solar"));
        add(5, 1, GanttHolidaysDialog::tr("Dia Internacional do Trabalho"));
        add(10, 1, GanttHolidaysDialog::tr("Dia Nacional da China"));
        add(10, 2, GanttHolidaysDialog::tr("Feriado do Dia Nacional"));
        add(10, 3, GanttHolidaysDialog::tr("Feriado do Dia Nacional"));
    } else if (code == QStringLiteral("MX")) {
        add(1, 1, GanttHolidaysDialog::tr("Año Nuevo"));
        add_nth(2, 1, 1, GanttHolidaysDialog::tr("Día de la Constitución"));
        add_nth(3, 1, 3, GanttHolidaysDialog::tr("Natalicio de Benito Juárez"));
        add(5, 1, GanttHolidaysDialog::tr("Día del Trabajo"));
        add(9, 16, GanttHolidaysDialog::tr("Día de la Independencia"));
        add_nth(11, 1, 3, GanttHolidaysDialog::tr("Revolución Mexicana"));
        add(12, 25, GanttHolidaysDialog::tr("Navidad"));
    } else if (code == QStringLiteral("CL")) {
        add(1, 1, GanttHolidaysDialog::tr("Año Nuevo"));
        add_rel(-2, GanttHolidaysDialog::tr("Viernes Santo"));
        add_rel(-1, GanttHolidaysDialog::tr("Sábado Santo"));
        add(5, 1, GanttHolidaysDialog::tr("Día del Trabajo"));
        add(5, 21, GanttHolidaysDialog::tr("Día de las Glorias Navales"));
        add(7, 16, GanttHolidaysDialog::tr("Virgen del Carmen"));
        add(8, 15, GanttHolidaysDialog::tr("Asunción de la Virgen"));
        add(9, 18, GanttHolidaysDialog::tr("Fiestas Patrias"));
        add(9, 19, GanttHolidaysDialog::tr("Glorias del Ejército"));
        add(10, 12, GanttHolidaysDialog::tr("Encuentro de Dos Mundos"));
        add(10, 31, GanttHolidaysDialog::tr("Día de las Iglesias Evangélicas"));
        add(11, 1, GanttHolidaysDialog::tr("Día de Todos los Santos"));
        add(12, 8, GanttHolidaysDialog::tr("Inmaculada Concepción"));
        add(12, 25, GanttHolidaysDialog::tr("Navidad"));
    } else if (code == QStringLiteral("CO")) {
        add(1, 1, GanttHolidaysDialog::tr("Año Nuevo"));
        add(1, 6, GanttHolidaysDialog::tr("Reyes Magos"));
        add(3, 19, GanttHolidaysDialog::tr("Día de San José"));
        add_rel(-3, GanttHolidaysDialog::tr("Jueves Santo"));
        add_rel(-2, GanttHolidaysDialog::tr("Viernes Santo"));
        add(5, 1, GanttHolidaysDialog::tr("Día del Trabajo"));
        add_rel(43, GanttHolidaysDialog::tr("Ascensión del Señor"));
        add_rel(64, GanttHolidaysDialog::tr("Corpus Christi"));
        add_rel(71, GanttHolidaysDialog::tr("Sagrado Corazón de Jesús"));
        add(7, 20, GanttHolidaysDialog::tr("Día de la Independencia"));
        add(8, 7, GanttHolidaysDialog::tr("Batalla de Boyacá"));
        add(8, 15, GanttHolidaysDialog::tr("La Asunción"));
        add(10, 12, GanttHolidaysDialog::tr("Día de la Raza"));
        add(11, 1, GanttHolidaysDialog::tr("Todos los Santos"));
        add(11, 11, GanttHolidaysDialog::tr("Independencia de Cartagena"));
        add(12, 8, GanttHolidaysDialog::tr("Inmaculada Concepción"));
        add(12, 25, GanttHolidaysDialog::tr("Navidad"));
    } else if (code == QStringLiteral("UY")) {
        add(1, 1, GanttHolidaysDialog::tr("Año Nuevo"));
        add(1, 6, GanttHolidaysDialog::tr("Día de Reyes"));
        add_rel(-48, GanttHolidaysDialog::tr("Carnaval"));
        add_rel(-47, GanttHolidaysDialog::tr("Carnaval"));
        add_rel(-3, GanttHolidaysDialog::tr("Semana de Turismo"));
        add_rel(-2, GanttHolidaysDialog::tr("Semana de Turismo"));
        add(4, 19, GanttHolidaysDialog::tr("Desembarco de los 33 Orientales"));
        add(5, 1, GanttHolidaysDialog::tr("Día de los Trabajadores"));
        add(5, 18, GanttHolidaysDialog::tr("Batalla de las Piedras"));
        add(6, 19, GanttHolidaysDialog::tr("Natalicio de Artigas"));
        add(7, 18, GanttHolidaysDialog::tr("Jura de la Constitución"));
        add(8, 25, GanttHolidaysDialog::tr("Declaratoria de la Independencia"));
        add(10, 12, GanttHolidaysDialog::tr("Día de la Raza"));
        add(11, 2, GanttHolidaysDialog::tr("Día de los Difuntos"));
        add(12, 25, GanttHolidaysDialog::tr("Día de la Familia (Navidad)"));
    } else if (code == QStringLiteral("PY")) {
        add(1, 1, GanttHolidaysDialog::tr("Año Nuevo"));
        add(3, 1, GanttHolidaysDialog::tr("Día de los Héroes"));
        add_rel(-3, GanttHolidaysDialog::tr("Jueves Santo"));
        add_rel(-2, GanttHolidaysDialog::tr("Viernes Santo"));
        add(5, 1, GanttHolidaysDialog::tr("Día del Trabajador"));
        add(5, 14, GanttHolidaysDialog::tr("Día de la Independencia"));
        add(5, 15, GanttHolidaysDialog::tr("Día de la Independencia"));
        add(6, 12, GanttHolidaysDialog::tr("Paz del Chaco"));
        add(8, 15, GanttHolidaysDialog::tr("Fundación de Asunción"));
        add(9, 29, GanttHolidaysDialog::tr("Victoria de Boquerón"));
        add(12, 8, GanttHolidaysDialog::tr("Virgen de Caacupé"));
        add(12, 25, GanttHolidaysDialog::tr("Navidad"));
    } else if (code == QStringLiteral("AU")) {
        add(1, 1, GanttHolidaysDialog::tr("New Year's Day"));
        add(1, 26, GanttHolidaysDialog::tr("Australia Day"));
        add_rel(-2, GanttHolidaysDialog::tr("Good Friday"));
        add_rel(1, GanttHolidaysDialog::tr("Easter Monday"));
        add(4, 25, GanttHolidaysDialog::tr("Anzac Day"));
        add_nth(6, 1, 2, GanttHolidaysDialog::tr("King's Birthday"));
        add(12, 25, GanttHolidaysDialog::tr("Christmas Day"));
        add(12, 26, GanttHolidaysDialog::tr("Boxing Day"));
    }

    return list;
}

GanttHolidaysDialog::GanttHolidaysDialog(const WorkCalendar &current_cal, QWidget *parent)
    : QDialog(parent),
      calendar_copy_(current_cal)
{
    setWindowTitle(tr("Gerenciar Feriados e Dias Não Úteis"));
    resize(880, 580);
    setup_ui();
    refresh_calendar_formats();
    refresh_tree();
    update_inputs_for_date(calendar_widget_->selectedDate());
}

void GanttHolidaysDialog::setup_ui()
{
    auto *main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(10);

    auto *content_layout = new QHBoxLayout();
    content_layout->setSpacing(12);

    // -------------------------------------------------------------
    // Coluna Esquerda: Calendário e Gerenciamento da Data
    // -------------------------------------------------------------
    auto *group_calendar = new QGroupBox(tr("Seleção no Calendário"), this);
    auto *left_layout = new QVBoxLayout(group_calendar);
    left_layout->setSpacing(8);

    calendar_widget_ = new QCalendarWidget(group_calendar);
    calendar_widget_->setGridVisible(true);
    calendar_widget_->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    connect(calendar_widget_, &QCalendarWidget::selectionChanged, this, &GanttHolidaysDialog::on_calendar_selection_changed);
    left_layout->addWidget(calendar_widget_);

    lbl_selected_date_ = new QLabel(group_calendar);
    lbl_selected_date_->setStyleSheet(QStringLiteral("font-weight: bold; color: #1976D2; font-size: 13px;"));
    left_layout->addWidget(lbl_selected_date_);

    auto *form_holiday = new QHBoxLayout();
    form_holiday->setSpacing(6);
    auto *lbl_name = new QLabel(tr("Descrição:"), group_calendar);
    edit_name_ = new QLineEdit(group_calendar);
    edit_name_->setPlaceholderText(tr("Ex: Natal, Carnaval, Feriado Municipal..."));
    form_holiday->addWidget(lbl_name);
    form_holiday->addWidget(edit_name_);
    left_layout->addLayout(form_holiday);

    auto *btn_layout = new QHBoxLayout();
    btn_layout->setSpacing(8);
    btn_add_update_ = new QPushButton(tr("➕ Adicionar como Feriado"), group_calendar);
    btn_add_update_->setStyleSheet(QStringLiteral("font-weight: bold; background-color: #2E7D32; color: white; padding: 5px 12px; border-radius: 4px;"));
    connect(btn_add_update_, &QPushButton::clicked, this, &GanttHolidaysDialog::on_add_or_update_holiday);
    btn_layout->addWidget(btn_add_update_);

    btn_remove_ = new QPushButton(tr("❌ Remover Feriado"), group_calendar);
    btn_remove_->setStyleSheet(QStringLiteral("background-color: #D32F2F; color: white; padding: 5px 12px; border-radius: 4px;"));
    connect(btn_remove_, &QPushButton::clicked, this, &GanttHolidaysDialog::on_remove_selected_holiday);
    btn_layout->addWidget(btn_remove_);
    left_layout->addLayout(btn_layout);

    content_layout->addWidget(group_calendar, 1);

    // -------------------------------------------------------------
    // Coluna Direita: Tabela WBS de Feriados Cadastrados
    // -------------------------------------------------------------
    auto *group_table = new QGroupBox(tr("Feriados Cadastrados no Cronograma"), this);
    auto *right_layout = new QVBoxLayout(group_table);
    right_layout->setSpacing(8);

    // Seleção de Países / Nações com botão Aplicar
    auto *country_layout = new QHBoxLayout();
    country_layout->setSpacing(6);
    auto *lbl_country = new QLabel(tr("País / Nação:"), group_table);
    country_layout->addWidget(lbl_country);

    combo_countries_ = new QComboBox(group_table);
    combo_countries_->addItem(tr("🇧🇷 Brasil"), QStringLiteral("BR"));
    combo_countries_->addItem(tr("🇺🇸 Estados Unidos"), QStringLiteral("US"));
    combo_countries_->addItem(tr("🇵🇹 Portugal"), QStringLiteral("PT"));
    combo_countries_->addItem(tr("🇦🇷 Argentina"), QStringLiteral("AR"));
    combo_countries_->addItem(tr("🇨🇦 Canadá"), QStringLiteral("CA"));
    combo_countries_->addItem(tr("🇬🇧 Reino Unido"), QStringLiteral("GB"));
    combo_countries_->addItem(tr("🇫🇷 França"), QStringLiteral("FR"));
    combo_countries_->addItem(tr("🇩🇪 Alemanha"), QStringLiteral("DE"));
    combo_countries_->addItem(tr("🇮🇹 Itália"), QStringLiteral("IT"));
    combo_countries_->addItem(tr("🇪🇸 Espanha"), QStringLiteral("ES"));
    combo_countries_->addItem(tr("🇯🇵 Japão"), QStringLiteral("JP"));
    combo_countries_->addItem(tr("🇨🇳 China"), QStringLiteral("CN"));
    combo_countries_->addItem(tr("🇲🇽 México"), QStringLiteral("MX"));
    combo_countries_->addItem(tr("🇨🇱 Chile"), QStringLiteral("CL"));
    combo_countries_->addItem(tr("🇨🇴 Colômbia"), QStringLiteral("CO"));
    combo_countries_->addItem(tr("🇺🇾 Uruguai"), QStringLiteral("UY"));
    combo_countries_->addItem(tr("🇵🇾 Paraguai"), QStringLiteral("PY"));
    combo_countries_->addItem(tr("🇦🇺 Austrália"), QStringLiteral("AU"));
    country_layout->addWidget(combo_countries_, 1);

    btn_apply_country_ = new QPushButton(tr("Aplicar"), group_table);
    btn_apply_country_->setToolTip(tr("Carregar feriados oficiais da nação selecionada"));
    btn_apply_country_->setStyleSheet(QStringLiteral("font-weight: bold; background-color: #0288D1; color: white; padding: 4px 14px; border-radius: 4px;"));
    connect(btn_apply_country_, &QPushButton::clicked, this, &GanttHolidaysDialog::on_apply_country_holidays);
    country_layout->addWidget(btn_apply_country_);

    right_layout->addLayout(country_layout);

    // Tabela WBS com cabeçalho bidirecional e hover
    tree_holidays_ = new QTreeWidget(group_table);
    auto *custom_header = new BiDirectionalResizeHeaderView(Qt::Horizontal, tree_holidays_);
    tree_holidays_->setHeader(custom_header);

    tree_holidays_->setColumnCount(3);
    tree_holidays_->setHeaderLabels({tr("Data"), tr("Dia da Semana"), tr("Descrição / Motivo")});
    tree_holidays_->header()->setSectionResizeMode(QHeaderView::Interactive);
    tree_holidays_->header()->setStretchLastSection(true);
    tree_holidays_->header()->resizeSection(0, 110);
    tree_holidays_->header()->resizeSection(1, 130);
    tree_holidays_->header()->resizeSection(2, 220);

    tree_holidays_->setAlternatingRowColors(true);
    tree_holidays_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tree_holidays_->setSelectionMode(QAbstractItemView::SingleSelection);
    tree_holidays_->setUniformRowHeights(true);
    tree_holidays_->setRootIsDecorated(false);
    tree_holidays_->setItemsExpandable(false);

    tree_holidays_->setItemDelegate(new GanttCellHoverDelegate(tree_holidays_));
    tree_holidays_->setMouseTracking(true);
    if (tree_holidays_->viewport()) {
        tree_holidays_->viewport()->setMouseTracking(true);
        tree_holidays_->viewport()->setAttribute(Qt::WA_Hover, true);
    }
    connect(tree_holidays_, &QTreeWidget::itemSelectionChanged, this, &GanttHolidaysDialog::on_tree_selection_changed);
    right_layout->addWidget(tree_holidays_);

    auto *table_btns_layout = new QHBoxLayout();
    table_btns_layout->setSpacing(6);

    btn_delete_tree_ = new QPushButton(tr("🗑️ Excluir Selecionado"), group_table);
    connect(btn_delete_tree_, &QPushButton::clicked, this, &GanttHolidaysDialog::on_delete_tree_row);
    table_btns_layout->addWidget(btn_delete_tree_);

    btn_clear_ = new QPushButton(tr("🧹 Limpar Todos"), group_table);
    connect(btn_clear_, &QPushButton::clicked, this, &GanttHolidaysDialog::on_clear_all_holidays);
    table_btns_layout->addWidget(btn_clear_);

    right_layout->addLayout(table_btns_layout);
    content_layout->addWidget(group_table, 1);

    main_layout->addLayout(content_layout);

    // Rodapé com Salvar e Cancelar
    auto *button_box = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    main_layout->addWidget(button_box);
}

void GanttHolidaysDialog::refresh_calendar_formats()
{
    calendar_widget_->setDateTextFormat(QDate(), QTextCharFormat());

    QTextCharFormat holiday_format;
    holiday_format.setFontWeight(QFont::Bold);
    holiday_format.setForeground(QColor(198, 40, 40));
    holiday_format.setBackground(QColor(255, 235, 238));

    for (const auto &ex : calendar_copy_.exceptions) {
        if (!ex.is_working_day) {
            holiday_format.setToolTip(ex.name.isEmpty() ? tr("Feriado") : ex.name);
            calendar_widget_->setDateTextFormat(ex.date, holiday_format);
        }
    }
}

void GanttHolidaysDialog::refresh_tree()
{
    tree_holidays_->clear();

    QList<CalendarException> holidays;
    for (const auto &ex : calendar_copy_.exceptions) {
        if (!ex.is_working_day) {
            holidays.append(ex);
        }
    }

    std::sort(holidays.begin(), holidays.end(), [](const CalendarException &a, const CalendarException &b) {
        return a.date < b.date;
    });

    const QLocale loc;
    for (const auto &h : holidays) {
        auto *item = new QTreeWidgetItem(tree_holidays_);
        item->setText(0, h.date.toString(QStringLiteral("dd/MM/yyyy")));
        item->setData(0, Qt::UserRole, h.date.toString(Qt::ISODate));
        item->setText(1, loc.dayName(h.date.dayOfWeek(), QLocale::LongFormat));
        item->setText(2, h.name);
    }
}

void GanttHolidaysDialog::update_inputs_for_date(const QDate &date)
{
    const QLocale loc;
    lbl_selected_date_->setText(tr("Data Selecionada: %1 (%2)")
                                .arg(date.toString(QStringLiteral("dd/MM/yyyy")),
                                     loc.dayName(date.dayOfWeek(), QLocale::LongFormat)));

    const bool is_hol = calendar_copy_.is_holiday(date);
    if (is_hol) {
        edit_name_->setText(calendar_copy_.holiday_name(date));
        btn_add_update_->setText(tr("✏️ Atualizar Feriado"));
        btn_remove_->setEnabled(true);
    } else {
        edit_name_->clear();
        btn_add_update_->setText(tr("➕ Adicionar como Feriado"));
        btn_remove_->setEnabled(false);
    }
}

void GanttHolidaysDialog::on_calendar_selection_changed()
{
    const QDate sel_date = calendar_widget_->selectedDate();
    update_inputs_for_date(sel_date);

    for (int i = 0; i < tree_holidays_->topLevelItemCount(); ++i) {
        auto *item = tree_holidays_->topLevelItem(i);
        if (item && QDate::fromString(item->data(0, Qt::UserRole).toString(), Qt::ISODate) == sel_date) {
            tree_holidays_->setCurrentItem(item);
            return;
        }
    }
    tree_holidays_->clearSelection();
}

void GanttHolidaysDialog::on_tree_selection_changed()
{
    const auto sel_items = tree_holidays_->selectedItems();
    if (sel_items.isEmpty()) return;

    auto *item = sel_items.first();
    if (!item) return;

    const QDate d = QDate::fromString(item->data(0, Qt::UserRole).toString(), Qt::ISODate);
    if (d.isValid()) {
        calendar_widget_->setSelectedDate(d);
        update_inputs_for_date(d);
    }
}

void GanttHolidaysDialog::on_add_or_update_holiday()
{
    const QDate date = calendar_widget_->selectedDate();
    if (!date.isValid()) return;

    QString name = edit_name_->text().trimmed();
    if (name.isEmpty()) {
        name = tr("Feriado");
    }

    calendar_copy_.add_holiday(date, name);
    refresh_calendar_formats();
    refresh_tree();
    update_inputs_for_date(date);
}

void GanttHolidaysDialog::on_remove_selected_holiday()
{
    const QDate date = calendar_widget_->selectedDate();
    if (!date.isValid()) return;

    calendar_copy_.remove_holiday(date);
    refresh_calendar_formats();
    refresh_tree();
    update_inputs_for_date(date);
}

void GanttHolidaysDialog::on_delete_tree_row()
{
    const auto sel_items = tree_holidays_->selectedItems();
    if (sel_items.isEmpty()) {
        QMessageBox::information(this, tr("Informação"), tr("Selecione um feriado na tabela para remover."));
        return;
    }

    auto *item = sel_items.first();
    if (!item) return;

    const QDate d = QDate::fromString(item->data(0, Qt::UserRole).toString(), Qt::ISODate);
    if (d.isValid()) {
        calendar_copy_.remove_holiday(d);
        refresh_calendar_formats();
        refresh_tree();
        update_inputs_for_date(calendar_widget_->selectedDate());
    }
}

void GanttHolidaysDialog::on_clear_all_holidays()
{
    if (calendar_copy_.exceptions.isEmpty()) return;

    if (QMessageBox::question(this, tr("Confirmar"),
                              tr("Deseja realmente remover todos os feriados cadastrados?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    calendar_copy_.exceptions.removeIf([](const CalendarException &ex) {
        return !ex.is_working_day;
    });

    refresh_calendar_formats();
    refresh_tree();
    update_inputs_for_date(calendar_widget_->selectedDate());
}

void GanttHolidaysDialog::on_apply_country_holidays()
{
    if (!combo_countries_) return;

    const QString code = combo_countries_->currentData().toString();
    const QString country_name = combo_countries_->currentText();
    const int year = calendar_widget_->yearShown();

    const auto holidays = generate_country_holidays(code, year);
    int added_count = 0;

    for (const auto &h : holidays) {
        calendar_copy_.add_holiday(h.first, h.second);
        added_count++;
    }

    refresh_calendar_formats();
    refresh_tree();
    update_inputs_for_date(calendar_widget_->selectedDate());

    QMessageBox::information(this, tr("Feriados Nacionais"),
                             tr("%1 feriados de %2 foram aplicados para o ano de %3.")
                             .arg(added_count).arg(country_name).arg(year));
}

QList<CalendarException> GanttHolidaysDialog::updated_exceptions() const
{
    return calendar_copy_.exceptions;
}

} // namespace source::gantt
