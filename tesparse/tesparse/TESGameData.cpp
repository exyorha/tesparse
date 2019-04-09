#include <tesparse/TESGameData.h>
#include <tesparse/FileMapping.h>
#include <tesparse/InputSerializationStream.h>
#include <tesparse/TESFileFormatDescription.h>
#include <tesparse/ExpressionEvaluator.h>
#include <tesparse/FourCC.h>

#include <sstream>
#include <unordered_set>

namespace tesparse {
	TESGameData::TESGameData() = default;

	TESGameData::~TESGameData() = default;

	template<typename T>
	bool inSet(T value, const std::vector<T> &set) {
		auto it = std::find(set.begin(), set.end(), value);
		return it != set.end();
	}

	void TESGameData::load(const std::string_view &filename, const tesparse::TESFileFormatDescription &desc) {
		static const std::unordered_set<std::string> builtinRecordFields{ "Name", "Size", "Data" };

		m_description = &desc;

		FileMapping mapping(filename);

		auto begin = static_cast<const unsigned char *>(mapping.base());
		auto end = begin + mapping.size();
		InputSerializationStream stream(begin, end);

		const auto &record = desc.getStructByName("Record");
		const auto &subrecord = desc.getStructByName("Subrecord");

		std::unordered_set<uint32_t> unknownRecords;

		bool headerExpected = true;

		while (!stream.atEnd()) {
			TESStruct recordData;
			parseFields(stream, record.fields, recordData);

			auto recordFourCC = recordData.value<TESUInt>("Name");
			auto recordDesc = desc.tryGetRecordByFourCC(recordFourCC);
			if (!recordDesc) {
				if (unknownRecords.count(recordFourCC) == 0) {
					fprintf(stderr, "unknown record: %s\n", fourCCToString(recordFourCC).c_str());
					unknownRecords.insert(recordFourCC);
				}

				continue;
			}

			if (headerExpected) {
				if (recordDesc->name != desc.headerRecord()) {
					std::stringstream error;
					error << "Header (" << desc.headerRecord() << ") expected, got " << recordDesc->name;
					throw std::runtime_error(error.str());
				}
			}

			auto recordContents = std::make_unique<TESStruct>();
			for (auto &&pair : recordData.fields) {
				if (builtinRecordFields.count(pair.first) == 0) {
					recordContents->fields.emplace(std::move(pair));
				}
			}
			
			auto parsingPos = recordDesc->entries.begin();
			std::vector<SubrecordDefinition>::const_iterator arrayParsingPos;
			TESArray *buildingArray = nullptr;
			TESStruct *buildingArrayMember = nullptr;
			bool inArray = false;

			const auto &recordDataBytes = recordData.value<std::vector<unsigned char>>("Data");
			InputSerializationStream subrecordStream(recordDataBytes.data(), recordDataBytes.data() + recordDataBytes.size());
			std::stringstream chain;
			while (!subrecordStream.atEnd()) {
				TESStruct subrecordData;

				parseFields(subrecordStream, subrecord.fields, subrecordData);

				auto subrecordFourcc = subrecordData.value<uint32_t>("Name");

				chain << fourCCToString(subrecordFourcc) << " ";

				const auto &subrecordDataBytes = subrecordData.value<std::vector<unsigned char>>("Data");
				InputSerializationStream subrecordDataStream(subrecordDataBytes.data(), subrecordDataBytes.data() + subrecordDataBytes.size());

				if (parsingPos == recordDesc->entries.end()) {
					std::stringstream error;
					error << recordDesc->name << ": EOF expected, got " << chain.str();
					throw std::runtime_error(error.str());
				}

				bool retryLookup;
				do {
					retryLookup = false;

					auto currentArray = std::get_if<SubrecordArrayDefinition>(&*parsingPos);
					if (currentArray && !inArray) {
						arrayParsingPos = currentArray->subrecords.begin();
						inArray = true;
					}

					bool found = false;

					if (currentArray) {
						for (auto it = arrayParsingPos; it != currentArray->subrecords.end(); ++it) {
							const auto &subrecordDesc = *it;
							if (subrecordDesc.fourcc == subrecordFourcc) {
								arrayParsingPos = it;
								found = true;
								break;
							}
							else if (subrecordDesc.required) {
								break;
							}

						}

						if (!found) {
							if (inSet(subrecordFourcc, currentArray->leader)) {
								for (auto it = arrayParsingPos; it != currentArray->subrecords.end(); ++it) {
									const auto &subrecordDesc = *it;

									if (subrecordDesc.required) {
										std::stringstream error;
										error << recordDesc->name << ": unexpected subrecord (early array restart): " << chain.str() << ": expected " << fourCCToString(subrecordDesc.fourcc);
										throw std::runtime_error(error.str());
									}
								}

								buildingArrayMember = &std::get<TESStruct>(buildingArray->values.emplace_back(TESStruct()));

								arrayParsingPos = currentArray->subrecords.begin();
							}
						}
					}

					if (!found) {
						for (auto it = parsingPos; it != recordDesc->entries.end(); ++it) {
							const auto &subrecordDesc = std::get_if<SubrecordDefinition>(&*it);
							if (subrecordDesc) {
								if (subrecordDesc->fourcc == subrecordFourcc) {
									parsingPos = it;
									found = true;
									inArray = false;
									currentArray = nullptr;
									buildingArray = nullptr;
									break;
								}
								else if (subrecordDesc->required) {
									break;
								}

							}
							else {
								const auto &arrayDesc = std::get<SubrecordArrayDefinition>(*it);
								
								if (inSet(subrecordFourcc, arrayDesc.leader)) {
									parsingPos = it;
									found = true;
									inArray = false;
									currentArray = nullptr;
									buildingArray = nullptr;
									break;
								}
							}
						}

						if (!found) {
							std::stringstream error;
							error << recordDesc->name << ": unexpected subrecord: " << chain.str() << ": expected";

							bool walkOutsideArray = true;

							if (currentArray) {
								for (auto it = arrayParsingPos; it != currentArray->subrecords.end(); ++it) {
									const auto &subrecordDesc = *it;
									error << " " << fourCCToString(subrecordDesc.fourcc);

									if (subrecordDesc.required) {
										if(it != currentArray->subrecords.begin())
											walkOutsideArray = false;

										break;
									}
								}
							}

							if (walkOutsideArray) {
								for (auto it = parsingPos; it != recordDesc->entries.end(); ++it) {
									const auto &subrecordDesc = std::get_if<SubrecordDefinition>(&*it);
									if (subrecordDesc) {
										error << " " << fourCCToString(subrecordDesc->fourcc);

										if (subrecordDesc->required)
											break;
									}
									else {
										const auto &arrayDesc = std::get<SubrecordArrayDefinition>(*it);

										for (auto entry : arrayDesc.leader) {
											error << " " << fourCCToString(entry);
										}
									}
								}
							}

							throw std::runtime_error(error.str());
						}
					}

					if (currentArray) {
						const auto &subrecordDesc = *arrayParsingPos;

						if (!buildingArray) {
							auto result = recordContents->fields.emplace(currentArray->name, TESArray());
							buildingArray = &std::get<TESArray>(result.first->second);
						}

						if (!buildingArrayMember) {
							buildingArrayMember = &std::get<TESStruct>(buildingArray->values.emplace_back(TESStruct()));
						}

						parseFields(subrecordDataStream, subrecordDesc.fields, *buildingArrayMember);

						++arrayParsingPos;
					}
					else {
						auto subrecordDesc = std::get_if<SubrecordDefinition>(&*parsingPos);
						if (subrecordDesc) {
							parseFields(subrecordDataStream, subrecordDesc->fields, *recordContents);

							++parsingPos;
						}
						else {
							retryLookup = true;
						}
					}
				} while (retryLookup);
			}

			if (headerExpected) {
				m_header = std::move(recordContents);
				headerExpected = false;
			}
			else {
				m_records.emplace_back(std::make_pair(recordDesc->name, std::move(recordContents)));
			}
		}
	}

	void TESGameData::parseFields(SerializationStream &stream, const std::vector<FieldDefinition> &fields, TESStruct &record) {
		for (const auto &field : fields) {
			record.fields.emplace(field.name, parseFieldValue(stream, field, record));
		}
	}

	TESValue TESGameData::parseFieldValue(SerializationStream &stream, const FieldDefinition &field, const TESStruct &context) {
		switch (field.type) {
		case FieldType::FourCC:
		case FieldType::UInt32:
		{
			uint32_t val;
			stream >> val;
			return static_cast<TESUInt>(val);
		}	
		
		case FieldType::UInt8:
		{
			uint8_t val;
			stream >> val;
			return static_cast<TESUInt>(val);
		}

		case FieldType::UInt16:
		{
			uint16_t val;
			stream >> val;
			return static_cast<TESUInt>(val);
		}

		case FieldType::Int32:
		{
			int32_t val;
			stream >> val;
			return static_cast<TESUInt>(val);
		}

		case FieldType::Float:
		{
			float val;
			stream >> val;
			return val;
		}

		case FieldType::ByteArray:
		{
			ExpressionEvaluator evaluator;
			ExpressionInteger length;
			if (field.length.empty()) {
				length = stream.remainingSize();
			}
			else {
				length = evaluator.evaluate(field.length, context);
			}
			std::vector<unsigned char> data(length);
			stream >> data;
			return data;
		}

		case FieldType::String:
		{
			ExpressionEvaluator evaluator;
			ExpressionInteger length;
			if (field.length.empty()) {
				length = stream.remainingSize();
			}
			else {
				length = evaluator.evaluate(field.length, context);
			}

			std::vector<char> data(length);
			stream >> data;

			auto terminator = std::find(data.begin(), data.end(), 0);
			
			return std::string(data.begin(), terminator);
		}

		case FieldType::Array:
		{
			TESArray data;

			ExpressionEvaluator evaluator;
			if (field.length.empty()) {
				while (!stream.atEnd()) {
					auto value = parseFieldValue(stream, *field.dataType, context);
					data.values.emplace_back(std::move(value));
				}
			}
			else {
				auto length = evaluator.evaluate(field.length, context);
				data.values.resize(length);

				for (auto &entry : data.values) {
					entry = parseFieldValue(stream, *field.dataType, context);
				}
			}

			return data;
		}

		case FieldType::StructRef:
		{
			const auto &structDef = m_description->getStructByName(field.structName);
			TESStruct st;

			parseFields(stream, structDef.fields, st);
			
			return st;
		}

		default:
		{
			std::stringstream error;
			error << "Unsupported field type: " << static_cast<unsigned int>(field.type);
			throw std::runtime_error(error.str());
		}
		}
	}
}
