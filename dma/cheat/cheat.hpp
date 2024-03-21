#pragma once

#include "../globals.h"
#include "../menu.h"

#include "math/vector3.hpp"
#include "classes/baseplayer.hpp"
#include "classes/localplayer.hpp"

#include <functional>
#include <memory>

struct viewMatrix {
	float* operator[ ]( int index ) {
		return matrix[ index ];
	}

	float matrix[ 4 ][ 4 ];
};

namespace cheat
{
	void init( );
	vector3 worldToScreen( vector3* v );
	void renderESP( );

	void updateAimbot( );

	extern std::vector< std::shared_ptr<sdk::basePlayer> > players;
}

// this is where we will store all of the found stuff.
namespace global
{
	extern std::shared_ptr<sdk::basePlayer> localPlayer;
	inline uintptr_t localPawn;
	inline uintptr_t baseClient;

	inline int localTeam;

	inline uintptr_t entityList;
	inline viewMatrix viewM;

	inline vector3 localPos;
}

class cheatFunction
{
	int msSleep = 0;
	
	std::function<void( )> function;
	
	int lastExecution;

public:
	
	cheatFunction( int time, std::function<void( )>func );
	
	void execute( );
};

extern std::shared_ptr<cheatFunction> updatePlayers;
extern std::shared_ptr<cheatFunction> updatePlayerBones;
extern std::shared_ptr<cheatFunction> cachePlayers;
extern std::shared_ptr<cheatFunction> updateViewMatrix;