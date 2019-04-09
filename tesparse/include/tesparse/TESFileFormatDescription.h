#ifndef TESPARSE_TES_FILE_FORMAT_DESCRIPTION_H
#define TESPARSE_TES_FILE_FORMAT_DESCRIPTION_H

#include <string>
#include <vector>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <variant>
#include <unordered_set>

#include <Windows.h>
#include <Shlwapi.h>
#include <comdef.h>
#include <xmllite.h>

#include <tesparse/Expression.h>

_COM_SMARTPTR_TYPEDEF(IXmlReader, IID_IXmlReader);

namespace tesparse {
	enum class FieldType {
		FourCC,
		UInt8,
		UInt16,
		Int32,
		UInt32,
		Float,
		ByteArray,
		String,
		Array,
		StructRef
	};

	struct FieldDefinition {
		std::string name;
		FieldType type;

		Expression length; // ByteArray, String, Array only: length expression. If empty, then until EOF

		std::string structName; // StructRef only

		std::unique_ptr<FieldDefinition> dataType; // Array only
	};

	struct StructDefinition {
		std::vector<FieldDefinition> fields;
	};

	struct SubrecordDefinition {
		uint32_t fourcc;
		bool required;
		std::vector<FieldDefinition> fields;
	};

	struct SubrecordArrayDefinition {
		std::string name;
		std::vector<uint32_t> leader;
		std::vector<SubrecordDefinition> subrecords;
	};

	struct RecordDefinition {
		std::string name;
		std::vector<std::variant<SubrecordDefinition, SubrecordArrayDefinition>> entries;
	};

	class TESFileFormatDescription {
	public:
		TESFileFormatDescription();
		~TESFileFormatDescription();

		TESFileFormatDescription(const TESFileFormatDescription &other) = delete;
		TESFileFormatDescription &operator =(const TESFileFormatDescription &other) = delete;

		void loadFromFile(const std::string_view &filename);
		void loadFromMemory(const unsigned char *data, unsigned int dataSize);

		const StructDefinition &getStructByName(const std::string &name) const;
		const RecordDefinition *tryGetRecordByFourCC(uint32_t fourcc) const;

		inline const std::string &headerRecord() const { return m_headerRecord; }

	private:
		void parseStream(IStreamPtr &&stream);
		
		void checkHR(HRESULT hr);
		std::wstring_view getLocalName(const IXmlReaderPtr &reader);
		void expectElement(const IXmlReaderPtr &reader, const std::wstring_view &name);
		void readAndExpectType(const IXmlReaderPtr &reader, XmlNodeType expectedType);
		bool iterateOnChildElements(const IXmlReaderPtr &reader);
		std::wstring_view getNamedAttribute(const IXmlReaderPtr &reader, const std::wstring &attributeName);
		void expectEmptyElement(const IXmlReaderPtr &reader);
		void expectNonEmptyElement(const IXmlReaderPtr &reader);
		void parseStruct(const IXmlReaderPtr &reader);
		void parseFields(const IXmlReaderPtr &reader, std::vector<FieldDefinition> &fields);
		void parseField(const IXmlReaderPtr &reader, FieldDefinition &field);
		void parseRecord(const IXmlReaderPtr &reader);
		void parseSubrecord(const IXmlReaderPtr &reader, SubrecordDefinition &definition);
		void parseSubrecordArray(const IXmlReaderPtr &reader, SubrecordArrayDefinition &definition);

		std::string m_headerRecord;
		std::unordered_map<std::string, StructDefinition> m_structs;
		std::unordered_map<uint32_t, RecordDefinition> m_records;
	};
}

#endif
