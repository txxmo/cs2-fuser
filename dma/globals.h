#pragma once

#include <mutex>

namespace global
{
	inline bool active = false;

	inline std::mutex cacheMutex;
}