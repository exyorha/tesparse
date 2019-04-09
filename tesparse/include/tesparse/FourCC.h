#ifndef TESPARSE_FOURCC_H
#define TESPARSE_FOURCC_H

#include <stdint.h>
#include <string_view>
#include <unordered_set>

namespace tesparse {
	uint32_t fourCCFromString(const std::string_view &string);
	std::vector<uint32_t> fourCCSetFromString(const std::string_view &string);
	std::string fourCCToString(uint32_t fourcc);
}

#endif
