#include <tesparse/FourCC.h>

#include <stdexcept>

namespace tesparse {
	uint32_t fourCCFromString(const std::string_view &string) {
		if (string.size() != 4)
			throw std::logic_error("Bad FourCC length");

		return (string[3] << 24) | (string[2] << 16) | (string[1] << 8) | string[0];
	}

	std::vector<uint32_t> fourCCSetFromString(const std::string_view &string) {
		std::vector<uint32_t> result;

		auto partBegin = string.begin();
		std::string_view::const_iterator partEnd;
		do {
			partEnd = std::find(partBegin, string.end(), '|');
			result.emplace_back(fourCCFromString(std::string_view(&*partBegin, partEnd - partBegin)));

			if (partEnd != string.end())
				partBegin = partEnd + 1;
		} while (partEnd != string.end());

		return result;
	}

	std::string fourCCToString(uint32_t fourcc) {
		char characters[4];

		characters[3] = static_cast<char>(fourcc >> 24);
		characters[2] = static_cast<char>(fourcc >> 16);
		characters[1] = static_cast<char>(fourcc >> 8);
		characters[0] = static_cast<char>(fourcc);

		return std::string(characters, characters + sizeof(characters));
	}
}
