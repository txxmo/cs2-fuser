#include "cheat.hpp"
#include "offsets.hpp"

#include <thread>
#include <chrono>
#include <iostream>
#include <mutex>

#include "../DMALibrary/Memory/Memory.h"

#define ENDSCATTER mem.ExecuteReadScatter( handle ); \
				   mem.CloseScatterHandle( handle );

std::vector<basePlayer> playersList;

cheatFunction::cheatFunction( int time, std::function<void( )> func )
{
	msSleep = time;
	function = func;
}

void cheatFunction::execute( )
{

	if ( GetTickCount64( ) - lastExecution > msSleep )
	{
		function( );
		lastExecution = GetTickCount64( );
	}
}

std::shared_ptr<cheatFunction> cachePlayers = std::make_shared<cheatFunction>( 1000, [ ] {

	std::lock_guard<std::mutex> lock( global::cacheMutex );

	auto handle = mem.CreateScatterHandle( );

	uintptr_t localAddress = global::baseClient + offsets::dwLocalPlayerController;
	mem.AddScatterReadRequest( handle, localAddress, &global::localPlayer, sizeof( uintptr_t ) );

	ENDSCATTER

	if ( !global::localPlayer )
		return;

	handle = mem.CreateScatterHandle( );

	uintptr_t localPawnAddress = global::baseClient + offsets::dwLocalPlayerPawn;
	mem.AddScatterReadRequest( handle, localPawnAddress, &global::localPawn, sizeof( uintptr_t ) );

	uintptr_t localTeamAddress = global::localPlayer + 0x3CB;
	mem.AddScatterReadRequest( handle, localTeamAddress, &global::localTeam, sizeof( int ) );

	uintptr_t entityListAddress = global::baseClient + offsets::dwEntityList;
	mem.AddScatterReadRequest( handle, entityListAddress, &global::entityList, sizeof( uintptr_t ) );

	ENDSCATTER

	handle = mem.CreateScatterHandle( );

	uintptr_t localPosAddress = global::localPawn + 0x127C;
	mem.AddScatterReadRequest( handle, localPosAddress, &global::localPos, sizeof( vector3 ) );

	ENDSCATTER

	handle = mem.CreateScatterHandle( );

	uintptr_t listEntry, player, networkedPlayer, playerPawn, weaponData, playerNameData;
	basePlayer playerEntity;

	// GOD MY EYES PLEASE
	for ( int i = 1; i < 64; i++ )
	{
		handle = mem.CreateScatterHandle( ); // list entry

		uintptr_t entityListAddress = global::entityList + ( 8 * ( i & 0x7FFF ) >> 9 ) + 16;
		mem.AddScatterReadRequest( handle, entityListAddress, &listEntry, sizeof( uintptr_t ) );

		ENDSCATTER

		if ( !listEntry )
			continue;

		handle = mem.CreateScatterHandle( ); // player

		uintptr_t listEntryAddress = listEntry + 120 * ( i & 0x1FF );
		mem.AddScatterReadRequest( handle, listEntryAddress, &player, sizeof( uintptr_t ) );

		ENDSCATTER

		if ( !player )
			continue;

		playerEntity.address = player;

		handle = mem.CreateScatterHandle( ); // player name data

		uintptr_t nameDataAddress = player + 0x748;
		mem.AddScatterReadRequest( handle, nameDataAddress, &playerNameData, sizeof( uintptr_t ) );

		ENDSCATTER

		char buffer[ 256 ];

		handle = mem.CreateScatterHandle( ); // player name

		mem.AddScatterReadRequest( handle, playerNameData, &buffer, sizeof( buffer ) );

		ENDSCATTER

		playerEntity.name = std::string( buffer );

		handle = mem.CreateScatterHandle( ); // player team

		uintptr_t teamAddress = player + 0x3CB;
		mem.AddScatterReadRequest( handle, teamAddress, &playerEntity.team, sizeof( int ) );

		ENDSCATTER

		if ( playerEntity.team == global::localTeam )
			continue;

		handle = mem.CreateScatterHandle( ); // player pawn

		uintptr_t playerPawnAddress = player + 0x604;
		mem.AddScatterReadRequest( handle, playerPawnAddress, &playerPawn, sizeof( uintptr_t ) );

		ENDSCATTER

		handle = mem.CreateScatterHandle( ); // wonky naming but "listentry2"

		uintptr_t networkedPlayerAddress = global::entityList + 0x8 * ( ( playerPawn & 0x7FFF ) >> 9 ) + 16;
		mem.AddScatterReadRequest( handle, networkedPlayerAddress, &networkedPlayer, sizeof( uintptr_t ) );

		ENDSCATTER

		handle = mem.CreateScatterHandle( ); // player pawn x2

		uintptr_t entityPawnAddress = networkedPlayer + 120 * ( playerPawn & 0x1FF );
		mem.AddScatterReadRequest( handle, entityPawnAddress, &playerEntity.pawn, sizeof( uintptr_t ) );

		ENDSCATTER

		handle = mem.CreateScatterHandle( ); // player health

		uintptr_t healthDataAddress = playerEntity.pawn + 0x334;
		mem.AddScatterReadRequest( handle, healthDataAddress, &playerEntity.health, sizeof( int ) );

		ENDSCATTER

		if ( playerEntity.health <= 0 || playerEntity.health > 100 )
			continue;

		playersList.push_back( playerEntity );
	}

	cheat::players.clear( );
	cheat::players.assign( playersList.begin( ), playersList.end( ) );

	playersList.clear(  );
} );

std::shared_ptr<cheatFunction> updatePlayerPos = std::make_shared<cheatFunction>( 10, [ ] {
	std::lock_guard<std::mutex> lock( global::cacheMutex );

	if ( !global::localPlayer )
		return;

	auto handle = mem.CreateScatterHandle( );

	for ( auto& player : cheat::players ) 
	{
		handle = mem.CreateScatterHandle( );
		
		mem.AddScatterReadRequest( handle, player.pawn + 0x127C, &player.origin, sizeof( vector3 ) );
		
		// for some reason macro doesn't work here :/
		mem.ExecuteReadScatter( handle );
		mem.CloseScatterHandle( handle );
	}

	for ( auto& player : cheat::players ) {
		player.head = { player.origin.x, player.origin.y, player.origin.z + 75.f };
	}
} );

std::shared_ptr<cheatFunction> updateViewMatrix = std::make_shared<cheatFunction>( 5, [ ] {

	if ( !global::localPlayer )
		return;

	auto handle = mem.CreateScatterHandle( );

	uintptr_t viewMatrixAddress = global::baseClient + offsets::dwViewMatrix;

	mem.AddScatterReadRequest( handle, viewMatrixAddress, &global::viewM, sizeof( viewMatrix ) );

	ENDSCATTER
} );

void cheat::init( )
{
	if ( !mem.Init( "cs2.exe", true, false ) )
		return;

	global::baseClient = mem.getBaseAddress( "client.dll" );
	if ( !global::baseClient )
		return;
}