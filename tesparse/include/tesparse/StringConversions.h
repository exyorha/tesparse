#ifndef TESPARSE__STRING_CONVERSIONS__H
#define TESPARSE__STRING_CONVERSIONS__H

#include <string>
#include <string_view>

namespace tesparse {

	std::wstring utf8ToWide(const std::string_view &string);

	std::string wideToUtf8(const std::wstring_view &string);

}

#endif
