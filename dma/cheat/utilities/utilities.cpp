#include "utilities.hpp"

#include "../kmbox.hpp"

#include <random>

bool kmBox::init( )
{
	std::string port = kmBox::FindPort( "USB-SERIAL CH340" );
	if ( port.empty( ) )
	{
		printf( "[KMBOX] failed to find port!\n" );
		return false;
	}

	if ( !kmBox::OpenPort( hSerial, port.c_str( ), CBR_115200 ) )
	{
		printf( "[KMBOX] failed to open port!\n" );
		return false;
	}

	printf( "[KMBOX] created successfully!\n" );
	cheat::kmbBoxConnected = true;

	return true;
}

void kmBox::sendMove( int x, int y )
{
	std::stringstream commandStream;
	commandStream << "km.move(" << x << "," << y << ")\r\n";
	SendCommand( hSerial, commandStream.str( ) );
}

void kmBox::sendMove( int x, int y, int curve )
{
	std::stringstream commandStream;
	commandStream << "km.move(" << x << "," << y << "," << curve << ")\r\n";
	SendCommand( hSerial, commandStream.str( ) );
}

void kmBox::sendClick( )
{
	std::string cmdDown = "km.left(" + std::to_string( 1 ) + ")\r\n";
	std::string cmdRelease = "km.left(" + std::to_string( 0 ) + ")\r\n"; 

	SendCommand( hSerial, cmdDown.c_str( ) );
	std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
	SendCommand( hSerial, cmdRelease.c_str( ) );
}

float inGameFOV = 90.f; float inGameSensitivity = 1.25f;
void utilities::aimAtPosition( vector2 targetPos, int aimSpeed, int smoothAmount, int curve ) {
	// Calculate screen center coordinates
	const float ScreenCenterX = menu::screenSize.x / 2.0f;
	const float ScreenCenterY = menu::screenSize.y / 2.0f;

	// Calculate target offsets
	float TargetX = ( targetPos.x - ScreenCenterX ) / aimSpeed;
	float TargetY = ( targetPos.y - ScreenCenterY ) / aimSpeed;

	// Apply FOV and sensitivity adjustments
	float FOVScale = inGameFOV / 90.0f; // Assume 90 degrees is the default FOV
	float SensitivityScale = inGameSensitivity / 1.0f; // Assume 1.0 is the default sensitivity

	TargetX *= FOVScale * SensitivityScale;
	TargetY *= FOVScale * SensitivityScale;

	// Apply limits to target offsets
	TargetX = utilities::clamp<float>( TargetX, -ScreenCenterX, ScreenCenterX );
	TargetY = utilities::clamp<float>( TargetY, -ScreenCenterY, ScreenCenterY );

	// Apply smoothing if necessary
	if ( smoothAmount > 1 ) {
		TargetX /= smoothAmount;
		TargetY /= smoothAmount;

		// Ensure minimum movement threshold
		TargetX = ( std::abs( TargetX ) < 1 ) ? ( TargetX > 0 ? 1 : -1 ) : TargetX;
		TargetY = ( std::abs( TargetY ) < 1 ) ? ( TargetY > 0 ? 1 : -1 ) : TargetY;
	}

	// Send mouse movement
	kmBox::sendMove( static_cast< int >( TargetX ), static_cast< int >( TargetY ) );
}