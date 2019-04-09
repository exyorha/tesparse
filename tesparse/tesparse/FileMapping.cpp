#include <tesparse/FileMapping.h>
#include <tesparse/StringConversions.h>

#include <Windows.h>
#include <comdef.h>

namespace tesparse {
	FileMapping::FileMapping(const std::string_view &filename) {
		auto rawFileHandle = CreateFile(
			utf8ToWide(filename).c_str(),
			GENERIC_READ,
			FILE_SHARE_READ,
			nullptr,
			OPEN_EXISTING,
			0,
			nullptr
		);
		if (rawFileHandle == INVALID_HANDLE_VALUE)
			_com_raise_error(HRESULT_FROM_WIN32(GetLastError()));
		m_fileHandle.reset(rawFileHandle);

		auto rawSectionHandle = CreateFileMapping(
			m_fileHandle.get(),
			nullptr,
			PAGE_READONLY,
			0, 0,
			nullptr
		);
		if(!rawSectionHandle)
			_com_raise_error(HRESULT_FROM_WIN32(GetLastError()));

		m_sectionHandle.reset(rawSectionHandle);

		auto rawMapping = MapViewOfFile(m_sectionHandle.get(), FILE_MAP_READ, 0, 0, 0);
		if(!rawMapping)
			_com_raise_error(HRESULT_FROM_WIN32(GetLastError()));

		m_mapping.reset(rawMapping);

		LARGE_INTEGER size;
		if(!GetFileSizeEx(m_fileHandle.get(), &size))
			_com_raise_error(HRESULT_FROM_WIN32(GetLastError()));

		m_size = static_cast<size_t>(size.QuadPart);
	}

	FileMapping::~FileMapping() = default;

	void FileMapping::MappingDeleter::operator()(void *base) const {
		UnmapViewOfFile(base);
	}
}