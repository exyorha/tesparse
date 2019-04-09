#ifndef TESPARSE_TES_GAME_DATA_H
#define TESPARSE_TES_GAME_DATA_H

#include <string_view>
#include <memory>

#include <tesparse/TESValue.h>

namespace tesparse {
	class TESFileFormatDescription;
	class SerializationStream;
	struct FieldDefinition;

	class TESGameData {
	public:
		TESGameData();
		~TESGameData();

		TESGameData(const TESGameData &other) = delete;
		TESGameData &operator =(const TESGameData &other) = delete;

		void load(const std::string_view &filename, const tesparse::TESFileFormatDescription &desc);

		inline const std::unique_ptr<TESStruct> &header() const { return m_header; }
		inline const std::vector<std::pair<std::string, std::unique_ptr<TESStruct>>> &records() const { return m_records; }

	private:
		void parseFields(SerializationStream &stream, const std::vector<FieldDefinition> &fields, TESStruct &record);
		TESValue parseFieldValue(SerializationStream &stream, const FieldDefinition &field, const TESStruct &context);

		std::unique_ptr<TESStruct> m_header;
		std::vector<std::pair<std::string, std::unique_ptr<TESStruct>>> m_records;
		const tesparse::TESFileFormatDescription *m_description;
	};
}

#endif
