#include "source/modules/feynman/Feynman/fey_15_stop_feynman_thread.hpp"

#include <QThread>

#include "source/modules/feynman/Feynman/fey_01_FeynmanWorker.hpp"

namespace source::plugin::Feynman {

void MetodoFeynman::_stop_feynman_thread()
{
	if (feynman_thread_ != nullptr) {
		if (feynman_thread_->isRunning()) {
			feynman_thread_->requestInterruption();
			feynman_thread_->quit();
			if (!feynman_thread_->wait(3000)) {
				if (feynman_worker_ != nullptr) {
					feynman_worker_->deleteLater();
					feynman_worker_ = nullptr;
				}
				QObject::connect(feynman_thread_, &QThread::finished, feynman_thread_, &QObject::deleteLater);
				feynman_thread_ = nullptr;
				return;
			}
		}

		delete feynman_worker_;
		feynman_worker_ = nullptr;

		delete feynman_thread_;
		feynman_thread_ = nullptr;
	}
}

}  // namespace source::plugin::Feynman

namespace source::plugin::Feynman::MetodoFeynmanFns {
void _stop_feynman_thread(MetodoFeynman &self) { self._stop_feynman_thread(); }
}
