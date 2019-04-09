#ifndef TESPARSE_INPUT_SERIALIZATION_STREAM_H
#define TESPARSE_INPUT_SERIALIZATION_STREAM_H

#include <tesparse/SerializationStream.h>

namespace tesparse {
	class InputSerializationStream final : public SerializationStream {
	public:
		InputSerializationStream(const unsigned char *begin, const unsigned char *end);
		~InputSerializationStream();

		virtual unsigned char *getRegionForWrite(size_t size) override;
		virtual const unsigned char *getRegionForRead(size_t size) override;

		virtual size_t getCurrentPosition() const override;
		virtual void setCurrentPosition(size_t position) override;

		virtual size_t remainingSize() const override;

	private:
		const unsigned char *m_begin;
		const unsigned char *m_end;
		const unsigned char *m_ptr;
	};
}

#endif
