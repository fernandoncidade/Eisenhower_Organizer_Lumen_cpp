#include "source/modules/audiolivro/metodoLeitor/lt_02_EdgeTTSThread.hpp"

#include "source/modules/audiolivro/audiolivro_settings.hpp"

#include <QDir>
#include <QMutexLocker>

namespace source::plugin::Audiolivro {

EdgeTTSThread::EdgeTTSThread(QString texto,
                             QString voz,
                             int rate_pct,
                             int volume_pct,
                             QString outdir,
                             QObject *parent)
    : QThread(parent),
      texto_(std::move(texto)),
      voz_(std::move(voz)),
      rate_pct_(rate_pct),
      volume_pct_(volume_pct),
      outdir_(std::move(outdir))
{
    if (outdir_.trimmed().isEmpty()) {
        outdir_ = audiolivro_persistent_dir();
    }
    QDir().mkpath(outdir_);
}

EdgeTTSThread::~EdgeTTSThread()
{
    stop();
    wait(1500);
}

}  // namespace source::plugin::Audiolivro
