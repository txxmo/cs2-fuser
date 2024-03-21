#pragma once

#include <Windows.h>
#include "../math/vector3.hpp"
#include "../../DMALibrary/Memory/Memory.h"

#include "../cheat.hpp"

namespace sdk
{
	class localPlayer
	{
		uintptr_t address = 0x1912578;

		uintptr_t pawn = 0x17371A8;

	public:

		localPlayer( );
		~localPlayer( );

		std::uintptr_t getAddress( );
		std::uintptr_t getPawn( );
	};
}