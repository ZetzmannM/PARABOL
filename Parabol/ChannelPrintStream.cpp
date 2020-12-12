#include "ChannelPrintStream.h"

#include "StringUtils.h"

#include <sstream>

wrap_ptr<ChannelPrintStream> ChannelPrintStream::stream = nullptr;

ChannelPrintStream& ChannelPrintStream::instance() {
	if(!stream) {
		stream = pass_ptr<ChannelPrintStream>(new ChannelPrintStream());
	}
	return *stream.get();
}
size_t ChannelPrintStream::addPrintStream(const print_channel& y, const std::string& name) {
	size_t ind = this->channels.size();
	channels.push_back({y, true, name});
	return ind;
}
void ChannelPrintStream::printInfo(const std::string& loc, const std::string& ref, channel_indx ind) {
	if (this->channels.size() >= ind) {
		if(this->channels[ind].enabled){
			_prntFormattedSplit(loc, ref, "INFO", 0, ind);
		}
	}else{
		throw std::exception("Invalid PrintStream Index");
	}
}
void ChannelPrintStream::printError(const std::string& loc, const std::string& f, uint64 severity, channel_indx ind) noexcept(false) {
	_prntFormattedSplit(loc, f, "ERROR", severity, ind);
	if (severity >= CH_SEVERITY_WARNING) {
		_haltOrWarn(loc, f, severity >= CH_SEVERITY_HALT);
	}
}
void ChannelPrintStream::stassert(bool in, const std::string& loc, const std::string& mes, channel_indx channel) {
	if (!in) {
		printError(loc, mes, CH_SEVERITY_HALT, channel);
	}
}

void ChannelPrintStream::_prntFormattedSplit(const std::string& locationID, const std::string& message, const std::string& tag, uint64 param, channel_indx channel) {
	std::vector<std::string> out = Util::StringUtils::split(message, '\n');
	for (unsigned int i = 0; i < out.size(); i++) {
		(this->channels[channel]).chn((_TAG_SYNTAX_FORMATTER(this->channels[channel].name, locationID, tag, out.at(i)) + "\n"), param);
	}
}
std::string ChannelPrintStream::pointerToString(const void* vkInst) const {
	std::stringstream sstream;
	sstream << "0x" << std::hex << reinterpret_cast<intptr_t>(vkInst);
	return sstream.str();
}
std::string ChannelPrintStream::devicePointerToString(const void* vkInst) const {
	std::stringstream sstream;
	sstream << "D_" << std::hex << reinterpret_cast<intptr_t>(vkInst);
	return sstream.str();
}
ChannelPrintStream::ChannelPrintStream() {
	auto simplePrint = [](std::string& ref, size_t t) {
		std::string prrr = ref;
		PRINT_FUNC(prrr.c_str());
	};

	this->addPrintStream(simplePrint, "Debug");
	this->addPrintStream(simplePrint, "Vulkan");
	this->addPrintStream(simplePrint, "VulkanDebug");
	this->addPrintStream(simplePrint, "GLFW");
}
void ChannelPrintStream::setStreamFlag(uint64 ind, bool enabled) {
	if (this->channels.size() >= ind) {
		this->channels[ind].enabled = enabled;
	}
	else {
		throw std::exception("Invalid PrintStream Index");
	}
}
void ChannelPrintStream::_haltOrWarn(const std::string& briefDescript, const std::string& message, bool halt) {
#ifdef ENV_WINDOWS
	ShowCursor(1);
	std::string title = "Called from @" + briefDescript;
	std::string mess = "With Message (HaltRequested (Exception):"+std::to_string(halt)+"): \n\n" + message;
	MessageBox(NULL,
		mess.c_str(),
		title.c_str(),
		NULL);
#endif

	if (halt) {
		throw std::exception(briefDescript.c_str());
	}
}