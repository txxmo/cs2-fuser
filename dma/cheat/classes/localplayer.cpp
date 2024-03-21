#include "localplayer.hpp"

using namespace sdk;

localPlayer::localPlayer( )
{
	this->address = mem.read<uintptr_t>( global::baseClient + address );
	printf( "[LocalPlayer] LocalPlayer: 0x%llX\n", address );

	this->pawn = mem.read<uintptr_t>( global::baseClient + pawn );
	printf( "[LocalPlayer] LocalPlayerPawn: 0x%llX\n", pawn );
}

localPlayer::~localPlayer( ) { }

std::uintptr_t localPlayer::getAddress( )
{
	return this->address;
}

std::uintptr_t localPlayer::getPawn( )
{
	return this->pawn;
}