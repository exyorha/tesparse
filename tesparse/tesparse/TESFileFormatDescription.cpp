#include <tesparse/TESFileFormatDescription.h>
#include <tesparse/StringConversions.h>
#include <tesparse/FourCC.h>
#include <tesparse/ExpressionParser.h>

#include <sstream>

namespace tesparse {

	TESFileFormatDescription::TESFileFormatDescription() = default;

	TESFileFormatDescription::~TESFileFormatDescription() = default;

	void TESFileFormatDescription::loadFromFile(const std::string_view &filename) {
		IStreamPtr stream;

		checkHR(SHCreateStreamOnFile(
			utf8ToWide(filename).c_str(),
			STGM_READ | STGM_SHARE_DENY_WRITE,
			&stream));

		parseStream(std::move(stream));
	}

	void TESFileFormatDescription::loadFromMemory(const unsigned char *data, unsigned int dataSize) {
		IStreamPtr stream;

		stream = SHCreateMemStream(data, dataSize);
		if (!stream)
			checkHR(E_OUTOFMEMORY);

		parseStream(std::move(stream));
	}

	void TESFileFormatDescription::checkHR(HRESULT hr) {
		if (FAILED(hr))
			_com_raise_error(hr);
	}

	std::wstring_view TESFileFormatDescription::getLocalName(const IXmlReaderPtr &reader) {
		const wchar_t *name;
		unsigned int nameSize;

		checkHR(reader->GetLocalName(&name, &nameSize));

		return std::wstring_view(name, nameSize);
	}

	void TESFileFormatDescription::expectElement(const IXmlReaderPtr &reader, const std::wstring_view &name) {
		auto actualName = getLocalName(reader);
		if (name != actualName) {
			std::stringstream error;
			error << "Unexpected element: expected " << wideToUtf8(name) << ", got " << wideToUtf8(actualName);
			throw std::runtime_error(error.str());
		}
	}

	void TESFileFormatDescription::readAndExpectType(const IXmlReaderPtr &reader, XmlNodeType expectedType) {
		XmlNodeType type;
		do {
			auto hr = reader->Read(&type);
			if (hr == S_FALSE)
				throw std::runtime_error("unexpected EOF");
			checkHR(hr);
		} while (
			type == XmlNodeType_ProcessingInstruction ||
			type == XmlNodeType_Comment ||
			type == XmlNodeType_DocumentType ||
			type == XmlNodeType_Whitespace ||
			type == XmlNodeType_XmlDeclaration
		);

		if (type != expectedType) {
			std::stringstream error;
			error << "Unexpected node type: expected " << expectedType << ", got " << type;
			throw std::runtime_error(error.str());
		}
	}

	bool TESFileFormatDescription::iterateOnChildElements(const IXmlReaderPtr &reader) {
		XmlNodeType type;
		do {
			auto hr = reader->Read(&type);
			if (hr == S_FALSE)
				throw std::runtime_error("unexpected EOF");
			checkHR(hr);
		} while (
			type == XmlNodeType_ProcessingInstruction ||
			type == XmlNodeType_Comment ||
			type == XmlNodeType_DocumentType ||
			type == XmlNodeType_Whitespace ||
			type == XmlNodeType_XmlDeclaration
		);

		if (type == XmlNodeType_EndElement)
			return false;

		if(type != XmlNodeType_Element) {
			std::stringstream error;
			error << "Unexpected node type: expected Element or EndElement, got " << type;
			throw std::runtime_error(error.str());
		}

		return true;
	}

	std::wstring_view TESFileFormatDescription::getNamedAttribute(const IXmlReaderPtr &reader, const std::wstring &attributeName) {
		auto hr = reader->MoveToAttributeByName(attributeName.c_str(), nullptr);
		checkHR(hr);
		if (hr == S_FALSE) {
			std::stringstream error;
			error << "Unexpected node type: required attribute was not found: " << wideToUtf8(attributeName);
			throw std::runtime_error(error.str());
		}

		const wchar_t *value;
		unsigned int valueSize;

		checkHR(reader->GetValue(&value, &valueSize));

		return std::wstring_view(value, valueSize);
	}

	void TESFileFormatDescription::expectEmptyElement(const IXmlReaderPtr &reader) {
		if (!reader->IsEmptyElement())
			throw std::runtime_error("Empty element expected");
	}

	void TESFileFormatDescription::expectNonEmptyElement(const IXmlReaderPtr &reader) {
		if (reader->IsEmptyElement())
			throw std::runtime_error("Non-empty element expected");
	}

	void TESFileFormatDescription::parseStream(IStreamPtr &&stream) {
		IXmlReaderPtr reader;

		checkHR(CreateXmlReader(IID_PPV_ARGS(&reader), nullptr));
		checkHR(reader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Parse));
		checkHR(reader->SetInput(stream));

		readAndExpectType(reader, XmlNodeType_Element);
		expectElement(reader, L"Layout");
		expectNonEmptyElement(reader);

		readAndExpectType(reader, XmlNodeType_Element);
		expectElement(reader, L"RecordOrder");
		expectNonEmptyElement(reader);

		readAndExpectType(reader, XmlNodeType_Element);
		expectElement(reader, L"Header");
		m_headerRecord = wideToUtf8(getNamedAttribute(reader, L"Name"));
		checkHR(reader->MoveToElement());
		expectEmptyElement(reader);

		readAndExpectType(reader, XmlNodeType_EndElement);

		/* RecordOrder ends here */

		readAndExpectType(reader, XmlNodeType_Element);
		expectElement(reader, L"Types");

		if (!reader->IsEmptyElement()) {
			while (iterateOnChildElements(reader)) {
				auto name = getLocalName(reader);

				if (name == L"Struct") {
					parseStruct(reader);
				}
				else if (name == L"Record") {
					parseRecord(reader);
				}
				else {
					std::stringstream error;
					error << "Unsupported type element: " << wideToUtf8(name);
					throw std::runtime_error(error.str());
				}
			}
		}

		/* Types end here */

		readAndExpectType(reader, XmlNodeType_EndElement);

		/* Layout ends here */
	}

	void TESFileFormatDescription::parseField(const IXmlReaderPtr &reader, FieldDefinition &field) {
		static const std::unordered_map<std::wstring_view, FieldType> fieldTypeMap{
			{ L"FourCC", FieldType::FourCC },
			{ L"UInt8", FieldType::UInt8 },
			{ L"UInt16", FieldType::UInt16 },
			{ L"Int32", FieldType::Int32 },
			{ L"UInt32", FieldType::UInt32 },
			{ L"Float", FieldType::Float },
			{ L"ByteArray", FieldType::ByteArray },
			{ L"String", FieldType::String },
			{ L"Array", FieldType::Array },
			{ L"StructRef", FieldType::StructRef }
		};

		expectNonEmptyElement(reader);

		readAndExpectType(reader, XmlNodeType_Element);
		auto name = getLocalName(reader);
		auto it = fieldTypeMap.find(name);
		if (it == fieldTypeMap.end()) {
			std::stringstream stream;
			stream << "Unsupported field type: " << wideToUtf8(name);
		}

		field.type = it->second;

		if (field.type == FieldType::Array || field.type == FieldType::ByteArray || field.type == FieldType::String) {
			auto hr = reader->MoveToAttributeByName(L"Length", nullptr);
			checkHR(hr);
			if (hr != S_FALSE) {
				const wchar_t *value;
				unsigned int valueSize;

				checkHR(reader->GetValue(&value, &valueSize));

				auto expression = wideToUtf8(std::wstring_view(value, valueSize));

				ExpressionParser parser;
				try {
					parser.parse(expression);
				}
				catch (const std::exception &e) {
					std::stringstream error;
					error << "Failed to parse '" << expression << "': " << e.what();
					throw std::runtime_error(error.str());
				}

				field.length = std::move(parser.expression());

				checkHR(reader->MoveToElement());
			}
		}

		if (field.type == FieldType::StructRef) {
			field.structName = wideToUtf8(getNamedAttribute(reader, L"Name"));
			checkHR(reader->MoveToElement());
		}

		if (field.type == FieldType::Array) {
			expectNonEmptyElement(reader);

			field.dataType = std::make_unique<FieldDefinition>();
			parseField(reader, *field.dataType);
		}
		else {
			expectEmptyElement(reader);
		}

		readAndExpectType(reader, XmlNodeType_EndElement);
	}

	void TESFileFormatDescription::parseFields(const IXmlReaderPtr &reader, std::vector<FieldDefinition> &fields) {


		if (!reader->IsEmptyElement()) {
			while (iterateOnChildElements(reader)) {
				expectElement(reader, L"Field");

				auto &field = fields.emplace_back();
				field.name = wideToUtf8(getNamedAttribute(reader, L"Name"));
				checkHR(reader->MoveToElement());

				parseField(reader, field);

			}
		}
	}

	void TESFileFormatDescription::parseStruct(const IXmlReaderPtr &reader) {
		auto structName = getNamedAttribute(reader, L"Name");
		auto &st = m_structs.emplace(wideToUtf8(structName), StructDefinition{}).first->second;

		checkHR(reader->MoveToElement());
		
		parseFields(reader, st.fields);
	}
	
	void TESFileFormatDescription::parseRecord(const IXmlReaderPtr &reader) {
		auto fourCC = fourCCFromString(wideToUtf8(getNamedAttribute(reader, L"FourCC")));

		auto &record = m_records.emplace(fourCC, RecordDefinition{}).first->second;
		record.name = wideToUtf8(getNamedAttribute(reader, L"Name"));
		checkHR(reader->MoveToElement());

		if (!reader->IsEmptyElement()) {
			while (iterateOnChildElements(reader)) {
				auto name = getLocalName(reader);

				if (name == L"Subrecord") {
					SubrecordDefinition subrecord;

					parseSubrecord(reader, subrecord);

					record.entries.emplace_back(std::move(subrecord));
				}
				else if(name == L"SubrecordArray") {
					SubrecordArrayDefinition subrecord;

					parseSubrecordArray(reader, subrecord);

					record.entries.emplace_back(std::move(subrecord));
				}
				else {
					std::stringstream error;
					error << "Unsupported element in record: " << wideToUtf8(name);
					throw std::runtime_error(error.str());
				}
			}
		}
	}

	void TESFileFormatDescription::parseSubrecord(const IXmlReaderPtr &reader, SubrecordDefinition &definition) {
		definition.fourcc = fourCCFromString(wideToUtf8(getNamedAttribute(reader, L"FourCC")));
		definition.required = getNamedAttribute(reader, L"Presence") == L"Required";
		checkHR(reader->MoveToElement());

		parseFields(reader, definition.fields);
	}

	void TESFileFormatDescription::parseSubrecordArray(const IXmlReaderPtr &reader, SubrecordArrayDefinition &definition) {
		definition.name = wideToUtf8(getNamedAttribute(reader, L"Name"));
		definition.leader = fourCCSetFromString(wideToUtf8(getNamedAttribute(reader, L"Leader")));
		checkHR(reader->MoveToElement());

		if (!reader->IsEmptyElement()) {
			while (iterateOnChildElements(reader)) {
				expectElement(reader, L"Subrecord");

				auto &subrecord = definition.subrecords.emplace_back();
				parseSubrecord(reader, subrecord);
			}
		}
	}

	const StructDefinition &TESFileFormatDescription::getStructByName(const std::string &name) const {
		auto it = m_structs.find(name);
		if (it == m_structs.end()) {
			std::stringstream error;
			error << "Undefined structure: " << name;
			throw std::logic_error(error.str());
		}

		return it->second;
	}
	
	const RecordDefinition *TESFileFormatDescription::tryGetRecordByFourCC(uint32_t fourcc) const {
		auto it = m_records.find(fourcc);
		if (it == m_records.end()) {
			return nullptr;
		}

		return &it->second;
	}
}
