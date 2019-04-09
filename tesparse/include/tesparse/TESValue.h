#ifndef TESPARSE_TES_VALUE_H
#define TESPARSE_TES_VALUE_H

#include <variant>
#include <unordered_map>
#include <vector>

namespace tesparse {
	struct TESStruct;
	struct TESArray;

	using TESUInt = uint32_t;
	using TESInt = int32_t;
	using TESValue = std::variant<std::monostate, TESStruct, TESArray, TESUInt, TESInt, float, std::vector<unsigned char>, std::string>;

	struct TESStruct {
		std::unordered_map<std::string, TESValue> fields;

		template<typename T>
		const T &value(const std::string &name) const {
			auto it = fields.find(name);
			if (it == fields.end()) {
				throw std::logic_error("Required field is not present: " + name);
			}

			return std::get<T>(it->second);
		}
	};

	struct TESArray {
		std::vector<TESValue> values;
	};

}

#endif
