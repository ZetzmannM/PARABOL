#include "Timer.h"

TimeHandler::TimeHandler(bool limitFPS, uint64 fps, double ticksPerFrame) {
	this->fps = fps;
	this->limit = limitFPS;
	this->ticksPerSecond = ticksPerFrame;
}

TimeHandler::TimeHandler() : TimeHandler(false, 60, 20) {}

uint64 TimeHandler::getTickCount() {
	if (!this->ticksCalled) {
		this->ticksCalled = true;
		this->tickLast = std::chrono::high_resolution_clock::now();
		return 0;
	}
	auto elep = std::chrono::high_resolution_clock::now() - this->tickLast;
	uint64 val =  (uint64)(((elep.count()+ ticksOverhead) / 1000000000.0f) * ticksPerSecond);
	ticksOverhead = static_cast<uint64>((elep.count() + ticksOverhead) - val * 1000000000 / ticksPerSecond);
	this->tickLast = std::chrono::high_resolution_clock::now();
	return val;
}

void TimeHandler::vsync() {
	if (limit) {
		if (!running) {
			auto f = std::chrono::high_resolution_clock::now();
			double diff = 0;
			double fixValue = 1000000000.0L / fps - elapsedTime.count();
			while (diff < fixValue) {
				diff = static_cast<double>((std::chrono::high_resolution_clock::now() - f).count());
			}
		}
		else {
			PRINT_ERR("Measurement still running!", CH_SEVERITY_HINT, CHANNEL_DEBUG);
		}
	}
}
void TimeHandler::start() {
	if (!this->running) {
		this->cycVal = _cyclicMeasurement();

		auto it = this->marks.begin();
		while (it != this->marks.end()) {
			it->second.erase();
			it++;
		}

		this->running = true;
		this->tstart = std::chrono::high_resolution_clock::now();
	}
	else {
		PRINT_ERR("Measurement already running!", CH_SEVERITY_HINT, CHANNEL_DEBUG);
	}
}
void TimeHandler::stop() {
	if (this->running) {
		this->elapsedTime = std::chrono::high_resolution_clock::now() - this->tstart;
		this->running = false;

		auto it = this->marks.begin();
		while (it != this->marks.end()) {
			if (it->second.running) {
				(*it).second.stop();
			}
			it++;
		}
	}
	else {
		PRINT_ERR("No measurement running!", CH_SEVERITY_HINT, CHANNEL_DEBUG);
	}
}
void TimeHandler::startMark(const std::string& in) {
	if (this->running) {
		if (!this->marks.count(in)) {
			this->marks[in] = __TimeMark();
		}
		this->marks[in].start();
	}
	else {
		PRINT_ERR("No measurement running!", CH_SEVERITY_HINT, CHANNEL_DEBUG);
	}
}
std::chrono::duration<double, std::nano> TimeHandler::_cyclicMeasurement() {
	if (this->cycMeasure.running) {
		this->cycMeasure.stop();
		std::chrono::duration<double, std::nano> rt = this->cycMeasure.elapsedTime;
		this->cycMeasure.start();
		return rt;
	}
	else {
		this->cycMeasure.start();
		return std::chrono::duration<double, std::nano>(0);
	}
}
void TimeHandler::stopMark(const std::string& in) {
	if (this->running) {
		if (this->marks.count(in)) {
			this->marks[in].stop();
		}
		else {
			PRINT_ERR("No such mark! (_MKEY:" + in + ")", CH_SEVERITY_HINT, CHANNEL_DEBUG);
		}
	}
	else {
		PRINT_ERR("No measurement running!", CH_SEVERITY_HINT, CHANNEL_DEBUG);
	}
}
std::chrono::duration<double, std::nano> TimeHandler::getMarkMeasurement(const std::string& in) {
	if (this->running) {
		if (this->marks.count(in)) {
			return this->marks[in].giveMeasurement();
		}
		else {
			PRINT_ERR("No such mark! (_MKEY:" + in + ")", CH_SEVERITY_HINT, CHANNEL_DEBUG);
		}
	}
	else {
		PRINT_ERR("No measurement running!", CH_SEVERITY_HINT, CHANNEL_DEBUG);
	}
	return std::chrono::duration<double, std::nano>(0);
}
void TimeHandler::__TimeMark::start() {
	if (!this->running) {
		this->running = true;
		erase();
		this->tstart = std::chrono::high_resolution_clock::now();
	}
	else {
		PRINT_ERR("Mark already running!", CH_SEVERITY_HINT, CHANNEL_DEBUG);
	}
}
void TimeHandler::__TimeMark::stop() {
	if (this->running) {
		this->running = false;
		this->elapsedTime = this->elapsedTime + (std::chrono::high_resolution_clock::now() - this->tstart);
	}
	else {
		PRINT_ERR("Mark not running!", CH_SEVERITY_HINT, CHANNEL_DEBUG);
	}
}
void TimeHandler::__TimeMark::erase() {
	this->elapsedTime = std::chrono::duration<double, std::nano>(0);
}
std::chrono::duration<double, std::nano> TimeHandler::__TimeMark::giveMeasurement() {
	return this->elapsedTime;
}
TimeHandler::__TimeMark::__TimeMark() {
	this->elapsedTime = std::chrono::duration<double, std::nano>(0);
	this->running = false;
}
void TimeHandler::setPrintFrequency(uint64 freqRecip) {
	this->printInterludeMaximum = freqRecip;
}
void TimeHandler::printMeasurements(const std::string& key, ChannelPrintStream::channel_indx channel) {
	if (this->printInterludeCount % this->printInterludeMaximum == 0) {
		this->printInterludeCount = 1;
	}
	else {
		this->printInterludeCount++;
		return;
	}
	if (!this->running) {
		auto it = this->marks.begin();
		double total = this->elapsedTime.count() / 1000000;
		double val = 0;

		PRINT("", channel);
		while (it != this->marks.end()) {
			val = it->second.giveMeasurement().count() / 1000000;
			PRINT("Mark \"" + it->first + "\": " + std::to_string(val) + "ms (" + std::to_string(val / total) + "%)", channel);
			it++;
		}
		double cycDiff = this->cycVal.count();

		PRINT("-----------------------------------------------------------------------", channel);
		PRINT("Total: " + std::to_string(total) + "ms (100%)" + "[ActFPS: " + std::to_string(1000000000.0 / cycDiff) + " ; PotFPS: " + std::to_string(1000.0 / (((!key.empty()) ? (total - this->marks[key].giveMeasurement().count() / 1000000) : total))) + "]", channel);
		PRINT("#######################################################################", channel);

	}
	else {
		PRINT_ERR("Measurement still running!", CH_SEVERITY_HINT, CHANNEL_DEBUG);
	}
}
