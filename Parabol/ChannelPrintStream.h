#pragma once
#include <map>
#include <string>
#include <vector>
#include <string>
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
#define CHANNEL_VULKAN 1
#define CHANNEL_VULKAN_DEBUG 2
#define CHANNEL_GLFW 3

#define CH_SEVERITY_HALT 32
#define CH_SEVERITY_WARNING 16
#define CH_SEVERITY_HINT 0


#define PRINT(M, C) ChannelPrintStream::instance().printInfo(__func__, M, C)
#define PRINT_DEBUG(M) PRINT(M, CHANNEL_DEBUG)

#define PRINT_ERR(M, S, C) ChannelPrintStream::instance().printError(__func__, M, S, C)

#define PTRSTR(A) ChannelPrintStream::instance().pointerToString(A)
#define DEVPTRSTR(A) ChannelPrintStream::instance().devicePointerToString(A)

#define ASSERT(X,M,C) if(!(X)) { PRINT_ERR(M, CH_SEVERITY_HALT, C); }
#define COND_INFO(X, M, C) if(!(X)) {PRINT(M, C); }

#define _TAG_SYNTAX_FORMATTER(d, a,b,c) (std::string("|")+d+"| "+ "[" + b + "] @" + a + ": " + c)

/// @brief Interface used by the application to print information to some stream.
/// It splits the output into channels, which can be individually disabled/enabled.
/// Custom Channels can be added. The use of this class is mostly if not exclusively through the macros defined above
struct ChannelPrintStream {
private:
	using print_channel = std::function<void(std::string&, uint64 arg)>;
	using channel_indx = size_t;

	struct print_channel_entry {
		print_channel chn;
		bool enabled;
		std::string name;
	};

	std::vector<print_channel_entry> channels;

	ChannelPrintStream();
	ChannelPrintStream(ChannelPrintStream&&) = delete;
	ChannelPrintStream(const ChannelPrintStream&) = delete;
	ChannelPrintStream& operator=(ChannelPrintStream&&) = delete;
	ChannelPrintStream& operator=(const ChannelPrintStream&) = delete;

public:

	/// @brief enables or disables a Channel, ERRORS WILL HOWEVER NOT BE AFFECTED! only printInfo calls
	void setStreamFlag(uint64 ind, bool enabled);
	
	/// @brief Prints a string into a specified Channel
	/// @param loc Caller Location 
	/// @param f string to be printed
	/// @param ind Index that specifies which channel is to be used
	void printInfo(const std::string& loc, const std::string& f, channel_indx ind = CHANNEL_DEBUG);

	/// @brief Prints an error into the specified channel
	/// @param loc Caller Location 
	/// @param f Error message
	/// @param severity Whether to stop the Application see CH_SEVERITY_HALT, CH_SEVERITY_WARNING
	/// @param ind Channel Identifier to print into
	void printError(const std::string& loc,const std::string& f, uint64 severity, channel_indx ind = CHANNEL_DEBUG);

	/// @brief Asserts the input to be true. If it is not, it causes an exception
	/// @param in input
	/// @param mes Optional Message
	/// @param channel Destination Channel
	void assert(bool in, const std::string& loc, const std::string& mes = "Assert Exception Triggered!", channel_indx channel = CHANNEL_DEBUG);

	/// @brief Adds a Channel handle
	/// @param  Function that handles the output for this Channel
	/// @return Index of the PrintChannel, this index is to be used to refer to this newly added Channel in print
	size_t addPrintStream(const print_channel&, const std::string& name);

	///@brief Converts the passed pointer into a string
	///Useful for deconst debugging
	std::string pointerToString(const void* ptr) const;

	///@brief Converts the passed device pointer into a string
	///Useful for deconst debugging
	///Difference to pointerToString are solely about formatting
	std::string devicePointerToString(const void* ptr) const;

	/// @brief Returns the instance (Singleton)
	static ChannelPrintStream& instance();

private:

	/// @brief  Prints a string in the format of: 
	///	"[$locationID$][$tag$]: $message$ \n". The message is split into lines to then print every single resulting line in this format. 
	/// @param locationID Info where this message was sent from (Usually the function with the PRINT call)
	/// @param message	The message
	/// @param tag	Usually the Channel name
	/// @param param Additional Parameter passed into the stream func
	/// @param channel 
	void _prntFormattedSplit(const std::string& locationID, const std::string& message, const std::string& tag, uint64 param, channel_indx channel = CHANNEL_DEBUG);

	/// @brief Halts the application
	void _haltOrWarn(const std::string& briefDescript, const std::string& message, bool halt);

	static wrap_ptr<ChannelPrintStream> stream;
};

