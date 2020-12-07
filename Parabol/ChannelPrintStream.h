#pragma once
#include <map>
#include <string>
#include <vector>
#include <functional>

#include "Env.h"
#include "inttypes.h"
#include "ptr.h"

#ifdef ENV_WINDOWS
#include <Windows.h>
#define PRINT_FUNC(A) OutputDebugStringA(A);
#endif

#ifdef ENV_LINUX
#include <iostream>
#define PRINT_FUNC(A) std::cout << A std::endl; 
#endif

#define CHANNEL_DEBUG 0

#define PRINT(A, B, C) ChannelPrintStream::instance().print(std::string("@")+std::string( __func__ )+": "+std::string(A), B, C);
#define PRINTA(A, C) PRINT(A,0,C)
#define PRINT_DEBUG(A) PRINT_A(A, CHANNEL_DEBUG)

struct ChannelPrintStream {
private:
	using print_channel = std::function<void(std::string&, uint64 arg)>;
	
	struct print_channel_entry {
		print_channel chn;
		bool enabled;
	};

	std::vector<print_channel_entry> channels;

	ChannelPrintStream();
	ChannelPrintStream(ChannelPrintStream&&) = delete;
	ChannelPrintStream(const ChannelPrintStream&) = delete;
	ChannelPrintStream& operator=(ChannelPrintStream&&) = delete;
	ChannelPrintStream& operator=(const ChannelPrintStream&) = delete;

public:

	/// @brief enables or disables a Chennel
	void setStreamFlag(uint64 ind, bool enabled);
	
	void print(std::string& f, uint64 param, size_t ind=0);
	size_t addPrintStream(const print_channel&);

	static ChannelPrintStream& instance();

private:
	static wrap_ptr<ChannelPrintStream> stream;
};

