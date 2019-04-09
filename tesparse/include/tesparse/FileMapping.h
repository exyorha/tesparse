#ifndef TESPARSE_FILE_MAPPING_H
#define TESPARSE_FILE_MAPPING_H

#include <string_view>

#include <tesparse/WindowsHandle.h>

namespace tesparse {
	class FileMapping {
	public:
		FileMapping(const std::string_view &filename);
		~FileMapping();

		FileMapping(const FileMapping &other) = delete;
		FileMapping &operator =(const FileMapping &other) = delete;

		inline const void *base() const { return m_mapping.get(); }
		inline const size_t size() const { return m_size; }

	private:
		struct MappingDeleter {
			void operator()(void *base) const;
		};

		WindowsHandle m_fileHandle;
		WindowsHandle m_sectionHandle;
		std::unique_ptr<void, MappingDeleter> m_mapping;
		size_t m_size;
	};
}

#endif
