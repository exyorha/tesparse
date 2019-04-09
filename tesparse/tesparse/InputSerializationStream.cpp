#include <tesparse/InputSerializationStream.h>

#include <stdexcept>

namespace tesparse {
	InputSerializationStream::InputSerializationStream(const unsigned char *begin, const unsigned char *end) :
		m_begin(begin), m_end(end), m_ptr(m_begin) {

	}

	InputSerializationStream::~InputSerializationStream() = default;
	
	unsigned char *InputSerializationStream::getRegionForWrite(size_t size) {
		(void)size;

		throw std::logic_error("InputSerializationStream cannot be written");
	}

	const unsigned char *InputSerializationStream::getRegionForRead(size_t size) {
		if (static_cast<intptr_t>(size) > m_end - m_ptr) {
			throw std::logic_error("read is out of bounds");
		}

		auto ptr = m_ptr;

		m_ptr += size;

		return ptr;
	}

	size_t InputSerializationStream::getCurrentPosition() const {
		return static_cast<size_t>(m_ptr - m_begin);
	}

	void InputSerializationStream::setCurrentPosition(size_t position) {
		if(static_cast<intptr_t>(position) > m_end - m_begin)
			throw std::logic_error("seek is out of bounds");

		m_ptr = m_begin + position;
	}

	size_t InputSerializationStream::remainingSize() const {
		return static_cast<size_t>(m_end - m_ptr);
	}

}

