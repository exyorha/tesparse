#include <tesparse/OutputSerializationStream.h>

namespace tesparse {
	OutputSerializationStream::OutputSerializationStream() : m_targetStream(nullptr), m_position(0), m_offset(0) {

	}

	OutputSerializationStream::OutputSerializationStream(SerializationStream *otherStream) : m_targetStream(otherStream), m_position(0), m_offset(m_targetStream->getCurrentPosition()) {

	}

	OutputSerializationStream::~OutputSerializationStream() = default;

	unsigned char *OutputSerializationStream::getRegionForWrite(size_t size) {
		unsigned char *ptr;

		if (m_targetStream) {
			ptr = m_targetStream->getRegionForWrite(size);
		}
		else {

			if (m_data.size() < m_position + size)
				m_data.resize(m_position + size);

			ptr = m_data.data() + m_position;
		}

		m_position += size;

		return ptr;
	}

	const unsigned char *OutputSerializationStream::getRegionForRead(size_t size) {
		const unsigned char *ptr;

		if (m_targetStream) {
			ptr = m_targetStream->getRegionForRead(size);
		}
		else {
			if (m_data.size() < m_position + size)
				throw std::logic_error("read is out of bounds");

			ptr = m_data.data() + m_position;
		}

		m_position += size;

		return ptr;
	}

	size_t OutputSerializationStream::getCurrentPosition() const {
		return m_position;
	}

	void OutputSerializationStream::setCurrentPosition(size_t position) {
		m_position = position;
		if(m_targetStream)
			m_targetStream->setCurrentPosition(m_position + m_offset);
	}
	
	size_t OutputSerializationStream::remainingSize() const {
		if (m_targetStream) {
			return m_targetStream->remainingSize();
		}
		else {
			return m_data.size() - m_position;
		}
	}

}
