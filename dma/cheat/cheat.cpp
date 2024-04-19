#include "cheat.hpp"
#include "offsets.hpp"
#include "kmbox.hpp"

#include <thread>
#include <chrono>
#include <iostream>
#include <mutex>
#include <unordered_set>

#include "../DMALibrary/Memory/Memory.h"

#define ENDSCATTER mem.ExecuteReadScatter( handle ); \
				   mem.CloseScatterHandle( handle );

std::shared_ptr<sdk::basePlayer> global::localPlayer = nullptr;
std::vector<std::shared_ptr<sdk::basePlayer>> cheat::players = { };
std::vector< std::uintptr_t > cheat::friends = { };

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

bool cheat::isFriend( std::uintptr_t address )
{
	for ( const auto& friendAddress : cheat::friends ) 
	{
		if ( friendAddress == address ) 
			return true;
	}

	return false;
}

std::shared_ptr<cheatFunction> cachePlayers = std::make_shared<cheatFunction>( 20, [ ] {

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
		mem.AddScatterReadRequest( handle, listAddress, reinterpret_cast< void* >( &playerList[ i ] ), sizeof( uintptr_t ) );
	}

	mem.ExecuteReadScatter( handle );

	std::vector<std::uintptr_t> addressList;
	addressList.resize( 64 );
	for ( int i = 0; i < playerList.size( ); i++ )
	{
		if ( playerList[ i ] == NULL )
			continue;

		uintptr_t entryAddress = playerList[ i ] + 120 * ( i & 0x1FF );
		mem.AddScatterReadRequest( handle, entryAddress, reinterpret_cast< void* >( &addressList[ i ] ), sizeof( uintptr_t ) );
	}

	mem.ExecuteReadScatter( handle );

	for ( int i = 0; i < playerList.size( ); i++ )
	{
		if ( addressList[ i ] == NULL )
			continue;

		tempPlayerList.push_back( std::make_shared<sdk::basePlayer>( addressList[ i ], handle ) );
	}

	mem.ExecuteReadScatter( handle );

	for ( int i = 0; i < tempPlayerList.size( ); i++ )
	{
		std::shared_ptr<sdk::basePlayer> player = tempPlayerList[ i ];
		if ( player )
			player->preCache( handle );
	}

	mem.ExecuteReadScatter( handle );

	for ( int i = 0; i < tempPlayerList.size( ); i++ )
	{
		std::shared_ptr<sdk::basePlayer> player = tempPlayerList[ i ];
		player->cachePawn( handle );

		if ( player == global::localPlayer )
			global::localPlayer->cachePawn( handle );
	}

	mem.ExecuteReadScatter( handle );

	for ( int i = 0; i < tempPlayerList.size( ); i++ )
	{
		std::shared_ptr<sdk::basePlayer> player = tempPlayerList[ i ];
		player->updateGameScene( handle );
	}

	mem.ExecuteReadScatter( handle );

	for ( int i = 0; i < tempPlayerList.size( ); i++ )
	{
		std::shared_ptr<sdk::basePlayer> player = tempPlayerList[ i ];
		player->updateBoneArray( handle );
	}

	mem.ExecuteReadScatter( handle );

	for ( int i = 0; i < tempPlayerList.size( ); i++ )
	{
		std::shared_ptr<sdk::basePlayer> player = tempPlayerList[ i ];
		player->getBoneArrayList( handle );
	}

	mem.ExecuteReadScatter( handle );

	for ( int i = 0; i < tempPlayerList.size( ); i++ )
	{
		std::shared_ptr<sdk::basePlayer> player = tempPlayerList[ i ];
		player->updateBoneData( );
	}

	global::localPlayer->updatePosition( handle );

	for ( int i = 0; i < tempPlayerList.size( ); i++ )
	{
		std::shared_ptr<sdk::basePlayer> player = tempPlayerList[ i ];
		player->updatePosition( handle );
		player->updateHealth( handle );
		player->updateSpotted( handle );
	}

	mem.ExecuteReadScatter( handle );
	mem.CloseScatterHandle( handle );

	cheat::players = tempPlayerList;

	if ( !cheat::players.empty( ) )
		cheat::connected = true;
	else
		cheat::connected = false;
} );

std::shared_ptr<cheatFunction> updateViewMatrix = std::make_shared<cheatFunction>( 5, [ ] {

	std::lock_guard<std::mutex> lock( global::cacheMutex );

	if ( !global::localPlayer )
		return;

	uintptr_t viewMatrixAddress = global::baseClient + offsets::dwViewMatrix;

	global::viewM = mem.Read<viewMatrix>( viewMatrixAddress );
} );

void cheat::init( )
{
	if ( !mem.Init( "cs2.exe", true, false ) )
	{
		std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );

		return;
	}

	global::baseClient = mem.getBaseAddress( "client.dll" );
	while ( !global::baseClient )
	{
		std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
	}

	if ( !mem.GetKeyboard( )->InitKeyboard( ) )
	{
		printf( "[DMA] failed to get keyboard\n" );
		return;
	}

	if ( !kmBox::init( ) )
	{
		printf( "[KMBOX] failed to get kmbox B+\n" );
		return;
	}

	global::active = true; // open menu !
	preInit( );

	printf( "[DMA] pre-initialization completed, entering game loop.\n" );

	constexpr std::chrono::milliseconds cooldownDuration( 250 );
	std::chrono::steady_clock::time_point lastActivationTime = std::chrono::steady_clock::now( );

	while ( true ) 
	{
		auto currentTime = std::chrono::steady_clock::now( );

		if ( mem.GetKeyboard( )->IsKeyDown( VK_INSERT ) || GetAsyncKeyState( VK_INSERT ) ) 
		{
			auto timeElapsed = std::chrono::duration_cast< std::chrono::milliseconds >( currentTime - lastActivationTime );

			if ( timeElapsed >= cooldownDuration ) 
			{
				global::active = !global::active;
				lastActivationTime = currentTime;
			}
		}

		updateViewMatrix->execute( );
		cachePlayers->execute( );
		updateAimbot->execute( );
	}
}