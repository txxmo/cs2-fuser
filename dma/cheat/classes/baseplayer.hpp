#pragma once

#include <Windows.h>
#include "../math/vector3.hpp"
#include "../../DMALibrary/Memory/Memory.h"

class basePlayer
{
public:
	uintptr_t address;
	
	int team;
	
	std::string name;
	
	uintptr_t pawn;
	
	vector3 origin;

	vector3 head;

	uintptr_t gameSceneNode;

	int health;
};