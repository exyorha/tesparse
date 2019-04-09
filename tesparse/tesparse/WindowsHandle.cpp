#include <tesparse/WindowsHandle.h>

#include <Windows.h>

namespace tesparse {
	void WindowsHandleDeleter::operator()(void *handle) const {
		CloseHandle(handle);
	}
}