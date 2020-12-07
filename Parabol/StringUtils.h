#pragma once

#include <string>
#include <vector>

#include "Env.h"

namespace Util {
	struct StringUtils {
		
		///	@brief Splits the given string at each occurance of the split argument
		/// @param input The input string</param>
		/// @param split The split argument</param>
		static std::vector<std::string> split(const std::string& input, char split);
#ifdef ENV_WINDOWS
		static std::wstring stringToWideString(const std::string& str);
#endif
	};

}
