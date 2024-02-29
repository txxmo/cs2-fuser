#pragma once

#include <mutex>

namespace global
{
	inline bool active = true;

	inline std::mutex cacheMutex;
}