#include <tesparse/TESFileFormatDescription.h>
#include <tesparse/TESGameData.h>

#include "CLI11.hpp"
#include <nlohmann/json.hpp>

nlohmann::json convertValue(const tesparse::TESStruct &st);
nlohmann::json convertValue(const tesparse::TESArray &st);

nlohmann::json convertValue(tesparse::TESUInt v) {
	return v;
}

nlohmann::json convertValue(tesparse::TESInt v) {
	return v;
}

nlohmann::json convertValue(float v) {
	return v;
}

nlohmann::json convertValue(const std::vector<unsigned char> &v) {
	static const char characterTable[] { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	std::string output;
	output.resize(v.size() * 2);
	for (size_t pos = 0, size = v.size(); pos < size; pos++) {
		auto byte = v[pos];

		output[pos * 2] = characterTable[byte >> 4];
		output[pos * 2 + 1] = characterTable[byte & 15];
	}

	return output;
}

nlohmann::json convertValue(const std::string &v) {
	return v;
}

nlohmann::json convertValue(const tesparse::TESValue &val) {
	return std::visit([](const auto &val) {
		return convertValue(val);
	}, val);
}

nlohmann::json convertValue(const tesparse::TESStruct &st) {
	auto obj = nlohmann::json::object();

	for (const auto &pair : st.fields) {
		obj[pair.first] = convertValue(pair.second);
	}

	return obj;
}

nlohmann::json convertValue(const tesparse::TESArray &st) {
	auto obj = nlohmann::json::array();

	for (const auto &val : st.values) {
		obj.push_back(convertValue(val));
	}

	return obj;
}

nlohmann::json convertValue(const std::unique_ptr<tesparse::TESStruct> &ptrToStruct) {
	return convertValue(*ptrToStruct);
}

nlohmann::json convertValue(const std::vector<std::pair<std::string, std::unique_ptr<tesparse::TESStruct>>> &val) {
	auto out = nlohmann::json::array();

	for (const auto &entry : val) {
		out.push_back({
			{ "type", entry.first },
			{ "data", convertValue(entry.second) }
		});
	}

	return out;
}

int main(int argc, char *argv[]) {
	CLI::App app;

	std::string descriptionFile;
	std::string esmFile;
	std::string jsonFile;
	app.add_option("description", descriptionFile)->mandatory();
	app.add_option("input", esmFile)->mandatory();
	app.add_option("output", jsonFile)->mandatory();
	
	CLI11_PARSE(app, argc, argv);

	tesparse::TESFileFormatDescription desc;
	try {
		desc.loadFromFile(descriptionFile);
	}
	catch (const std::exception &e) {
		fprintf(stderr, "Description file has failed to load: %s\n", e.what());
		return 1;
	}

	tesparse::TESGameData gameData;
	try {
		gameData.load(esmFile, desc);
	}
	catch (const std::exception &e) {
		fprintf(stderr, "Parse error: %s\n", e.what());
		return 1;
	}

	nlohmann::json json{
		{ "header", convertValue(gameData.header()) },
		{ "records", convertValue(gameData.records()) },
	};

//	try {
		std::ofstream stream;
		stream.exceptions(std::ios::badbit | std::ios::eofbit | std::ios::failbit);
		stream.open(jsonFile, std::ios::out | std::ios::trunc | std::ios::binary);
		stream << json.dump(2, ' ', false, nlohmann::json::error_handler_t::replace);
//	}
//	catch (const std::exception &e) {
//		fprintf(stderr, "Unable to write JSON representation: %s\n", e.what());
//		return 1;
//	}
}


