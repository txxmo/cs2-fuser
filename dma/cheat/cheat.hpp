#pragma once

#include "../globals.h"
#include "../menu.h"
#include "config.hpp"

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

	inline vector2 aimPoint;
	inline std::shared_ptr<sdk::basePlayer> targetPlayer;

	inline bool connected = false;
	inline bool kmbBoxConnected = false;

	extern std::vector< std::shared_ptr<sdk::basePlayer> > players;
	extern std::vector< std::uintptr_t > friends;

	bool isFriend( std::uintptr_t address );

}

// this is where we will store all of the found stuff.
namespace global
{
	extern std::shared_ptr<sdk::basePlayer> localPlayer;
	inline std::uintptr_t localPawn;
	inline std::uintptr_t baseClient;

	inline int localTeam;

	inline std::uintptr_t entityList;
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

extern std::shared_ptr<cheatFunction> cachePlayers;
extern std::shared_ptr<cheatFunction> updateViewMatrix;

extern std::shared_ptr<cheatFunction> updateAimbot;