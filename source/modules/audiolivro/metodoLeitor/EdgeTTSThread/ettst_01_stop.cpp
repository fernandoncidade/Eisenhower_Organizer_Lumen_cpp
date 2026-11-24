#include "source/modules/audiolivro/metodoLeitor/EdgeTTSThread/ettst_01_stop.hpp"

#include <QMutexLocker>

namespace source::plugin::Audiolivro {

void EdgeTTSThread::stop()
{
    QMutexLocker locker(&mutex_);
    should_stop_ = true;
    if (process_ != nullptr) {
        process_->kill();
    }
}

}  // namespace source::plugin::Audiolivro
