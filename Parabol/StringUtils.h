#pragma once

#include <string>
#include <vector>

#include "inttypes.h"
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

	struct NonCpyStringContainer {
	private:
		std::vector<char*> strs;

	public:
		NonCpyStringContainer() {}
		~NonCpyStringContainer();

		NonCpyStringContainer(NonCpyStringContainer&& ref) noexcept;
		NonCpyStringContainer& operator=(NonCpyStringContainer&& d) noexcept;

		NonCpyStringContainer(const NonCpyStringContainer& ref) = delete;
		NonCpyStringContainer& operator=(const NonCpyStringContainer&) = delete;

		/// @return C-Str container
		char** data();

		/// @brief Adds a name to the catalog 
		void add(const std::string&);

		/// @brief Adds all strings to the catalog 
		void add(const std::vector<std::string>&);

		size_t size();

	};



}
