#pragma once

#include <map>
#include <chrono>
#include <string>

#include "inttypes.h"
#include "Env.h"
#include "ChannelPrintStream.h"

#ifdef ENV_WINDOWS
#define __CLOCK steady_clock
#endif
#ifdef ENV_LINUX
#define __CLOCK system_clock
#endif

/// @brief Interface for measuring Time, with it: FPS, time elapsed in specific parts etc.. It also is able to block for VSync, and includes a minor TickHandler 
struct TimeHandler {
private:
	struct __TimeMark {
		std::chrono::duration<double, std::nano> elapsedTime;
		std::chrono::time_point<std::chrono::__CLOCK> tstart;
		bool running = false;

		__TimeMark();

		void start();
		void stop();
		void erase();

		std::chrono::duration<double, std::nano> giveMeasurement();
	};

	uint64 printInterludeMaximum = 1;
	uint64 printInterludeCount = 1;

	double ticksPerSecond = 20;
	uint64 ticksOverhead = 0;
	std::chrono::time_point<std::chrono::__CLOCK> tickLast;
	bool ticksCalled = false;

	uint64 fps = 60;
	bool limit = false;
	std::map<std::string, __TimeMark> marks;

	std::chrono::duration<double, std::nano> elapsedTime = std::chrono::duration<double, std::nano>(0);
	__TimeMark cycMeasure;
	std::chrono::duration<double, std::nano> cycVal;

	std::chrono::time_point<std::chrono::__CLOCK> tstart;
	bool running = false;

public:

	/// @brief Creates a new TimeHandler interface. The interface keeps track of FPS limits (if desired) and tick handling 
	TimeHandler(bool limitFPS, uint64 fps, double ticksPerFrame);

	/// @brief Default TimeHandler, with a limit of 60 FPS, and 1 tick per supposed Frame
	TimeHandler();

	/// @brief Returns the amount of ticks to be calculated since the last call of this function
	uint64 getTickCount();

	/// @brief blocks, until the elapsed time is at least the required amount of time for the FPS to be 60
	void vsync();

	/// @brief Start taking time measurements. Usually done BEFORE a render pass
	void start();

	/// @brief Stops taking time measurements. Usually done AFTER a render pass
	void stop();

	/// @brief Starts the time mark.
	/// @param id The mark id
	void startMark(const std::string& in);

	/// @brief Stops the time mark.
	/// @param id The mark id
	void stopMark(const std::string& in);

	/// @param id The mark id
	/// @return the measurement for the given mark in NANO seconds
	std::chrono::duration<double, std::nano> getMarkMeasurement(const std::string& in);

	/// @brief Used to set the frequency of prints for 'printMeasurements' If set to n printMeasurements will (only) every n calls, the other calls are ignored
	void setPrintFrequency(uint64 freqRecip);

	/// @brief Prints all Measurements taken (for every mark). Use setPrintFrequency to set the frequency of the outputs
	void printMeasurements(const std::string& ref = "", ChannelPrintStream::channel_indx channel = CHANNEL_DEBUG);

private:
	///<summary>This method returns the time that has elapsed since the last function call, or 0 if there was none yet</summary>
	std::chrono::duration<double, std::nano> _cyclicMeasurement();

};
