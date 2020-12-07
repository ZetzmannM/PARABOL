#include "ChannelPrintStream.h"

wrap_ptr<ChannelPrintStream> ChannelPrintStream::stream = nullptr;

ChannelPrintStream& ChannelPrintStream::instance() {
	if(!stream) {
		stream = pass_ptr<ChannelPrintStream>(new ChannelPrintStream());
	}
	return *stream.get();
}
size_t ChannelPrintStream::addPrintStream(const print_channel& y) {
	size_t ind = this->channels.size();
	channels.push_back({y, true});
	return ind;
}
void ChannelPrintStream::print(std::string& ref, uint64 param, size_t ind) {
	if (this->channels.size() >= ind) {
		this->channels[ind].chn(ref, param);
	}else{
		throw std::exception("Invalid PrintStream Index");
	}
}
ChannelPrintStream::ChannelPrintStream() {
	this->addPrintStream([](std::string& ref, size_t t) { 
		std::string prrr = "[DEBUG] " + ref;
		PRINT_FUNC(prrr.c_str()); 
	});
}
void ChannelPrintStream::setStreamFlag(uint64 ind, bool enabled) {
	if (this->channels.size() >= ind) {
		this->channels[ind].enabled = enabled;
	}
	else {
		throw std::exception("Invalid PrintStream Index");
	}
}
