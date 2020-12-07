#include "StringUtils.h"
#include <codecvt>
#include <locale>

using namespace Util;

std::vector<std::string> StringUtils::split(const std::string& input, char split) {
	std::vector<std::string> out;
	std::string parse = input;
	size_t index = parse.find_first_of('\n');
	while (index != std::string::npos) {
		out.push_back(std::string(parse.begin(), parse.begin() + index));
		parse = std::string(parse.begin() + index + 1, parse.end());
		index = parse.find_first_of('\n');
	}
	out.push_back(parse);
	return out;
}
#ifdef ENV_WINDOWS
#include "Windows.h"
std::wstring StringUtils::stringToWideString(const std::string& str) {
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}
#endif