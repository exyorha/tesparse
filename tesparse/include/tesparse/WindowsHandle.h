#ifndef TESPARSE_WINDOWS_HANDLE_H
#define TESPARSE_WINDOWS_HANDLE_H

#include <memory>

namespace tesparse {
	struct WindowsHandleDeleter {
		void operator()(void *handle) const;
	};

	using WindowsHandle = std::unique_ptr<void, WindowsHandleDeleter>;
}

#endif
