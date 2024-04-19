#pragma once

#include <Windows.h>
#include "../math/vector3.hpp"
#include "../../DMALibrary/Memory/Memory.h"

#include "../cheat.hpp"
#include "../offsets.hpp"

namespace sdk
{
	class localPlayer
	{
		uintptr_t address = offsets::dwLocalPlayerController;

		uintptr_t pawn = offsets::dwLocalPlayerPawn;

	public:

		localPlayer( );
		~localPlayer( );

		std::uintptr_t getAddress( );
		std::uintptr_t getPawn( );
	};
}