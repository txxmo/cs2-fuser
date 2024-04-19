#include "baseplayer.hpp"

#include "../cheat.hpp"

using namespace sdk;

basePlayer::basePlayer( std::uintptr_t address, VMMDLL_SCATTER_HANDLE handle )
{
	this->address = address;

	if ( address == 0 )
		return;

	mem.AddScatterReadRequest( handle, this->address + 0x3CB, reinterpret_cast< void* >( &team ), sizeof( int ) );
	mem.AddScatterReadRequest( handle, this->address + 0x604, reinterpret_cast< void* >( &prePawn ), sizeof( std::uintptr_t ) );
	mem.AddScatterReadRequest( handle, this->address + 0x748, reinterpret_cast< void* >( &nameData ), sizeof( std::uintptr_t ) );
}

basePlayer::basePlayer( std::uintptr_t address, std::uintptr_t pawn, VMMDLL_SCATTER_HANDLE handle )
{
	this->address = address;

	if ( address == 0 )
		return;

	mem.AddScatterReadRequest( handle, this->address + 0x3CB, reinterpret_cast< void* >( &team ), sizeof( int ) );
	mem.AddScatterReadRequest( handle, this->address + 0x748, reinterpret_cast< void* >( &nameData ), sizeof( std::uintptr_t ) );

	this->pawn = pawn;
}

basePlayer::~basePlayer( ) { }

void basePlayer::preCache( VMMDLL_SCATTER_HANDLE handle )
{
	mem.AddScatterReadRequest( handle, this->nameData, reinterpret_cast< void* >( &this->name ), sizeof( this->name ) );
	mem.AddScatterReadRequest( handle, global::entityList + 0x8 * ( ( this->prePawn & 0x7FFF ) >> 9 ) + 16, reinterpret_cast< void* >( &networkedData ), sizeof( std::uintptr_t ) );
}

void basePlayer::cachePawn( VMMDLL_SCATTER_HANDLE handle )
{
	std::uintptr_t address = this->networkedData + 120 * ( this->prePawn & 0x1FF );
	mem.AddScatterReadRequest( handle, address, reinterpret_cast< void* >( &this->pawn ), sizeof( std::uintptr_t ) );
}

vector3 basePlayer::getPosition( )
{
	return this->origin;
}

void basePlayer::updatePosition( VMMDLL_SCATTER_HANDLE handle )
{
	mem.AddScatterReadRequest( handle, this->pawn + 0x127C, reinterpret_cast< void* >( &this->origin ), sizeof( vector3 ) );
}

void basePlayer::updateHealth( VMMDLL_SCATTER_HANDLE handle )
{
	mem.AddScatterReadRequest( handle, this->pawn + 0x334, reinterpret_cast< void* >( &this->health ), sizeof( int ) );
}

void basePlayer::updateFlags( VMMDLL_SCATTER_HANDLE handle )
{
	mem.AddScatterReadRequest( handle, this->pawn + 0x3D4, reinterpret_cast< void* >( &this->flags ), sizeof( int32_t ) );
}

int basePlayer::getFlags( )
{
	return this->flags;
}

void basePlayer::updateViewAngles( VMMDLL_SCATTER_HANDLE handle )
{
	mem.AddScatterReadRequest( handle, this->pawn + 0x11A4, reinterpret_cast< void* >( &this->viewAngles ), sizeof( vector3 ) );
}

vector3 basePlayer::getViewAngles( )
{
	return this->viewAngles;
}

int basePlayer::getHealth( )
{
	return this->health;
}

bool basePlayer::isSpotted( )
{
	return this->spotted;
}

void basePlayer::updateSpotted( VMMDLL_SCATTER_HANDLE handle )
{
	this->spotted = mem.Read<bool>( this->pawn + 0x1698 + 0xC );
}

bool basePlayer::isAlive( )
{
	if ( this->health > 0 || this->health < 100 )
		return true;

	return false;
}

std::string basePlayer::getName( )
{
	return std::string( this->name );
}

bool basePlayer::isValid( )
{
	return this->address != 0;
}

std::uintptr_t basePlayer::getPawn( )
{
	return this->pawn;
}

int basePlayer::getTeam( )
{
	return this->team;
}

std::string basePlayer::getTeamString( )
{
	switch ( this->team )
	{
	case 0:
		return "N/A";
		break;
	case 1:
		return "N/A";
		break;
	case 2:
		return "T";
		break;
	case 3:
		return "CT";
		break;
	}

	return "N/A";
}

void basePlayer::updatePawn( VMMDLL_SCATTER_HANDLE handle )
{
	std::uintptr_t address = this->networkedData + 120 * ( this->prePawn & 0x1FF );
	mem.AddScatterReadRequest( handle, address, reinterpret_cast< void* >( &this->pawn ), sizeof( std::uintptr_t ) );
}

std::uintptr_t basePlayer::getAddress( )
{
	return this->address;
}

void basePlayer::updateGameScene( VMMDLL_SCATTER_HANDLE handle )
{
	mem.AddScatterReadRequest( handle, this->pawn + 0x318, reinterpret_cast< void* >( &this->gameSceneNode ), sizeof( uintptr_t ) );
}

void basePlayer::updateBoneArray( VMMDLL_SCATTER_HANDLE handle )
{
	mem.AddScatterReadRequest( handle, this->gameSceneNode + 0x1E0, reinterpret_cast< void* >( &this->boneArray ), sizeof( uintptr_t ) );
}

void basePlayer::getBoneArrayList( VMMDLL_SCATTER_HANDLE handle )
{
	mem.AddScatterReadRequest( handle, this->boneArray, reinterpret_cast< void* >( &this->bonesArray ), 30 * sizeof( boneJointData ) );
}

void basePlayer::updateGameScene( VMMDLL_SCATTER_HANDLE handle, std::uintptr_t override )
{
	mem.AddScatterReadRequest( handle, override + 0x318, reinterpret_cast< void* >( &this->gameSceneNode ), sizeof( uintptr_t ) );
}

void basePlayer::updateBoneArray( VMMDLL_SCATTER_HANDLE handle, std::uintptr_t override )
{
	mem.AddScatterReadRequest( handle, this->gameSceneNode + 0x1E0, reinterpret_cast< void* >( &this->boneArray ), sizeof( uintptr_t ) );
}

void basePlayer::getBoneArrayList( VMMDLL_SCATTER_HANDLE handle, std::uintptr_t override )
{
	mem.AddScatterReadRequest( handle, this->boneArray, reinterpret_cast< void* >( &this->bonesArray ), 30 * sizeof( boneJointData ) );
}

bool basePlayer::updateBoneData( )
{
	if ( !this->bonesArray )
		return false;

	for ( int i = 0; i < 30; i++ )
		this->bonePosList.push_back( { bonesArray[ i ].position } );

	return this->bonePosList.size( ) > 0;
}