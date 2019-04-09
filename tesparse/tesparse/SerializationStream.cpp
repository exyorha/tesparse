#include <tesparse/SerializationStream.h>

#include <algorithm>

namespace tesparse {
	SerializationStream::SerializationStream() : m_swapEndian(false) {

	}

	SerializationStream::~SerializationStream() = default;

	void SerializationStream::writeArithmetic(const unsigned char *data, size_t dataSize) {
		if (m_swapEndian) {
			auto interim = getRegionForWrite(dataSize);
			std::reverse_copy(data, data + dataSize, interim);
		}
		else {
			writeData(data, dataSize);
		}
	}

	void SerializationStream::readArithmetic(unsigned char *data, size_t dataSize) {
		if (m_swapEndian) {
			auto interim = getRegionForRead(dataSize);
			std::reverse_copy(interim, interim + dataSize, data);
		}
		else {
			readData(data, dataSize);
		}
	}

	void SerializationStream::writeData(const unsigned char *data, size_t dataSize) {
		auto region = getRegionForWrite(dataSize);
		memcpy(region, data, dataSize);
	}

	void SerializationStream::readData(unsigned char *data, size_t dataSize) {
		auto region = getRegionForRead(dataSize);
		memcpy(data, region, dataSize);
	}

	template<>
	SerializationStream &operator <<(SerializationStream &stream, const std::vector<uint8_t> &value) {
		stream.writeData(value.data(), value.size());
		return stream;
	}

	template<>
	SerializationStream &operator >>(SerializationStream &stream, std::vector<uint8_t> &value) {
		stream.readData(value.data(), value.size());
		return stream;
	}

	template<>
	SerializationStream &operator <<(SerializationStream &stream, const std::vector<char> &value) {
		stream.writeData(reinterpret_cast<const unsigned char *>(value.data()), value.size());
		return stream;
	}

	template<>
	SerializationStream &operator >>(SerializationStream &stream, std::vector<char> &value) {
		stream.readData(reinterpret_cast<unsigned char *>(value.data()), value.size());
		return stream;
	}

	SerializationStream &operator <<(SerializationStream &stream, const std::string &value) {
		stream << static_cast<uint16_t>(value.size());

		auto region = stream.getRegionForWrite(value.size() + 1);
		memcpy(region, value.c_str(), value.size() + 1);

		return stream;
	}

	SerializationStream &operator >>(SerializationStream &stream, std::string &value) {
		uint16_t length;
		stream >> length;

		auto region = stream.getRegionForRead(length + 1);

		value.assign(reinterpret_cast<const char *>(region),
			reinterpret_cast<const char *>(region) + length);

		return stream;
	}
}
