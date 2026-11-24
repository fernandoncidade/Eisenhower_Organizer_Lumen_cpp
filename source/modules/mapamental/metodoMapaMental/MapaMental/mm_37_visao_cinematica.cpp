#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_37_visao_cinematica.hpp"

#include <QCoreApplication>
#include <QLabel>
#include <QTimer>

#include "source/modules/mapamental/metodoMapaMental/mp_01_NoConceito.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_07_SmoothGraphicsView.hpp"

namespace source::plugin::MapaMental {

void MapaMental::focar_no_para_qa(NoConceito *no)
{
    if (no == nullptr) {
        return;
    }

    for (NoConceito *item : std::as_const(nos_)) {
        if (item != nullptr) {
            item->setSelected(item == no);
        }
    }

    if (view_ != nullptr) {
        view_->animate_focus_on(no);
    }

    refresh_counter();
    if (status_label_ != nullptr) {
        const QString focused_text =
            QCoreApplication::translate("MapaMentalPlugin", "Contexto focado: %1").arg(no->texto());
        const QString counter_text = status_label_->text().trimmed();
        status_label_->setText(counter_text.isEmpty()
                                   ? focused_text
                                   : QStringLiteral("%1 — %2").arg(counter_text, focused_text));
    }
}

void MapaMental::iniciar_visao_cinematica()
{
    const auto visible_nodes = [this]() {
        QList<NoConceito *> result;
        for (NoConceito *no : std::as_const(nos_)) {
            if (no != nullptr && no->isVisible()) {
                result.append(no);
            }
        }
        return result;
    };

    const QList<NoConceito *> iniciais = visible_nodes();
    if (iniciais.isEmpty()) {
        if (cinematic_timer_ != nullptr && cinematic_timer_->isActive()) {
            cinematic_timer_->stop();
        }
        cinematic_index_ = 0;
        return;
    }

    if (cinematic_timer_ != nullptr && cinematic_timer_->isActive()) {
        cinematic_timer_->stop();
        cinematic_index_ = 0;
        refresh_counter();
        return;
    }

    if (cinematic_timer_ == nullptr) {
        cinematic_timer_ = new QTimer(this);
        cinematic_timer_->setInterval(1300);
        connect(cinematic_timer_, &QTimer::timeout, this, [this, visible_nodes]() {
            const QList<NoConceito *> visiveis = visible_nodes();
            if (visiveis.isEmpty()) {
                cinematic_timer_->stop();
                cinematic_index_ = 0;
                return;
            }

            if (cinematic_index_ >= visiveis.size()) {
                cinematic_timer_->stop();
                cinematic_index_ = 0;
                refresh_counter();
                return;
            }

            focar_no_para_qa(visiveis.at(cinematic_index_));
            ++cinematic_index_;
        });
    }

    cinematic_index_ = 0;
    cinematic_timer_->start();
    focar_no_para_qa(iniciais.first());
    cinematic_index_ = 1;
}

}  // namespace source::plugin::MapaMental
