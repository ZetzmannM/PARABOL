#pragma once

#if defined(_WIN32) | defined(_WIN64) | defined(__WIN32__) | defined(__WINDOWS__)
	#define ENV_WINDOWS
#endif

#if defined(__linux__) 
	#define ENV_LINUX
#endif
