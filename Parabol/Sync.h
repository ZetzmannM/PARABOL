#pragma once

#include <condition_variable>
#include <mutex>

#include "meta_tags.h"
#include "inttypes.h"

namespace Sync {

	struct Barrier {
		enum struct semaphore_type {
			binary = 0,
			threshold = 1
		};

	private:
		std::condition_variable var;
		volatile uint64 state = 0;
		bool autoReset = false;
		const uint64 threshold = 1;
		mutable std::mutex mtx;
		
	public:

		/// @brief Creates a binary Barrier
		Barrier(bool initState = false, bool autoReset = false);

		/// @brief Creates a Barrier with a specific threshold
		/// This Barrier will release all waiting agents only if the state is
		/// greater equal the threashold. The state is increased by calling signal() 
		/// and reset by calling reset
		Barrier(uint64 threshold, uint64 initialValue =0);

		thread_safe void signal();
		thread_safe void reset();
		thread_safe bool signaled();

		thread_safe void wait();
	};

}