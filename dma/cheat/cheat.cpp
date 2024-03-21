#include "cheat.hpp"
#include "offsets.hpp"

#include <thread>
#include <chrono>
#include <iostream>
#include <mutex>

#include "../DMALibrary/Memory/Memory.h"

#define ENDSCATTER mem.ExecuteReadScatter( handle ); \
				   mem.CloseScatterHandle( handle );

std::shared_ptr<sdk::basePlayer> global::localPlayer = nullptr;
std::vector<std::shared_ptr<sdk::basePlayer>> cheat::players = { };

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

void preInit( )
{
	std::shared_ptr<sdk::localPlayer> local = std::make_shared<sdk::localPlayer>( );

	auto handle = mem.CreateScatterHandle( );

	global::localPlayer = std::make_shared<sdk::basePlayer>( local->getAddress( ), local->getPawn( ), handle );

	uintptr_t entityListAddress = global::baseClient + offsets::dwEntityList;
	mem.AddScatterReadRequest( handle, entityListAddress, &global::entityList, sizeof( uintptr_t ) );

	ENDSCATTER
}

std::shared_ptr<cheatFunction> cachePlayers = std::make_shared<cheatFunction>( 25, [ ] {

	std::lock_guard<std::mutex> lock( global::cacheMutex );

	if ( !global::localPlayer )
		return;

	std::vector< std::shared_ptr<sdk::basePlayer> > tempPlayerList;

	auto handle = mem.CreateScatterHandle( );

	std::vector<std::uintptr_t> playerList;
	playerList.resize( 64 );
	for ( int i = 1; i < 64; i++ )
	{
		uintptr_t listAddress = global::entityList + ( 8 * ( i & 0x7FFF ) >> 9 ) + 16;
		mem.AddScatterReadRequest( handle, listAddress, reinterpret_cast<void*>( &playerList[ i ] ), sizeof( uintptr_t ) );
	}

	ENDSCATTER

	handle = mem.CreateScatterHandle( );

	std::vector<std::uintptr_t> addressList;
	addressList.resize( 64 );
	for ( int i = 0; i < playerList.size(); i++ )
	{
		if ( playerList[ i ] == NULL )
			continue;

		uintptr_t entryAddress = playerList[ i ] + 120 * ( i & 0x1FF );
		mem.AddScatterReadRequest( handle, entryAddress, reinterpret_cast< void* >( &addressList[ i ] ), sizeof( uintptr_t ) );
	}

	ENDSCATTER

	handle = mem.CreateScatterHandle( );

	for ( int i = 0; i < playerList.size( ); i++ )
	{
		if ( addressList[ i ] == NULL )
			continue;

		tempPlayerList.push_back( std::make_shared<sdk::basePlayer>( addressList[ i ], handle ) );
	}

	ENDSCATTER

	handle = mem.CreateScatterHandle( );

	for ( int i = 0; i < tempPlayerList.size( ); i++ )
	{
		std::shared_ptr<sdk::basePlayer> player = tempPlayerList[ i ];
		if ( player )
			player->preCache( handle );
	}

	ENDSCATTER

	handle = mem.CreateScatterHandle( );

	for ( int i = 0; i < tempPlayerList.size( ); i++ )
	{
		std::shared_ptr<sdk::basePlayer> player = tempPlayerList[ i ];
		player->cachePawn( handle );

		if ( player == global::localPlayer )
			global::localPlayer->cachePawn( handle );
	}

	ENDSCATTER

	handle = mem.CreateScatterHandle( );

	for ( int i = 0; i < tempPlayerList.size( ); i++ )
	{
		std::shared_ptr<sdk::basePlayer> player = tempPlayerList[ i ];
		player->updateGameScene( handle );
	}

	ENDSCATTER

	handle = mem.CreateScatterHandle( );

	for ( int i = 0; i < tempPlayerList.size( ); i++ )
	{
		std::shared_ptr<sdk::basePlayer> player = tempPlayerList[ i ];
		player->updateBoneArray( handle );
	}

	ENDSCATTER

	handle = mem.CreateScatterHandle( );

	for ( int i = 0; i < tempPlayerList.size( ); i++ )
	{
		std::shared_ptr<sdk::basePlayer> player = tempPlayerList[ i ];
		player->getBoneArrayList( handle );
	}

	ENDSCATTER

	for ( int i = 0; i < tempPlayerList.size( ); i++ )
	{
		std::shared_ptr<sdk::basePlayer> player = tempPlayerList[ i ];
		player->updateBoneData( );
	}

	cheat::players = tempPlayerList;
} );

std::shared_ptr<cheatFunction> updatePlayers = std::make_shared<cheatFunction>( 25, [ ] {
	
	std::lock_guard<std::mutex> lock( global::cacheMutex );

	auto handle = mem.CreateScatterHandle( );

	global::localPlayer->updatePosition( handle );
	global::localPlayer->updateFlags( handle );

	for ( auto player : cheat::players )
	{
		if ( !player->isValid( ) )
			continue;
		
		player->updatePosition( handle );
		player->updateHealth( handle );
	}

	ENDSCATTER
} );

std::shared_ptr<cheatFunction> updatePlayerBones = std::make_shared<cheatFunction>( 10, [ ] {

	std::lock_guard<std::mutex> lock( global::cacheMutex );

} );

std::shared_ptr<cheatFunction> updateViewMatrix = std::make_shared<cheatFunction>( 5, [ ] {

	std::lock_guard<std::mutex> lock( global::cacheMutex );

	if ( !global::localPlayer )
		return;

	uintptr_t viewMatrixAddress = global::baseClient + offsets::dwViewMatrix;

	global::viewM = mem.read<viewMatrix>( viewMatrixAddress );
} );

void update( )
{
	while ( true )
	{
		updateViewMatrix->execute( );
		cachePlayers->execute( );
		updatePlayers->execute( );
	}
}

void cheat::init( )
{
	if ( !mem.Init( "cs2.exe", true, false ) )
		return;

	global::baseClient = mem.getBaseAddress( "client.dll" );
	if ( !global::baseClient )
		return;

	preInit( );
}