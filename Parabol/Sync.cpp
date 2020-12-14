#include "Sync.h"

using namespace Sync;

Barrier::Barrier(uint64 threshold, uint64 initValue) : threshold{threshold}, state{ initValue } {}
Barrier::Barrier(bool initVal, bool autoReset) : threshold{ 1u }, autoReset{ autoReset }{
	state = (initVal ? 1 : 0);
}

void Barrier::signal() {
	mtx.lock();
	this->state++;
	mtx.unlock();
	if (this->state >= threshold) {
		this->var.notify_all();
	}
}
bool Barrier::signaled() {
	return this->state >= this->threshold;
}
void Barrier::reset() {
	mtx.lock();
	this->state = 0;
	mtx.unlock();
}
void Barrier::wait() {
	std::unique_lock<std::mutex> lock = std::unique_lock<std::mutex>(this->mtx);
	while (this->state < threshold) {
		this->var.wait(lock);
	}
	if (this->autoReset) {
		reset();
	}
}
