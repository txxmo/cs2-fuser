#include "../cheat.hpp"

#include "../kmbox.hpp"

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

	return true;
}

void kmBox::sendMove( int x, int y )
{
	//std::stringstream commandStream;
	//commandStream << "km.move(" << x << "," << y << ")\r\n";
	//SendCommand( hSerial, commandStream.str( ) );

    std::string command = "km.move(" + std::to_string( x ) + "," + std::to_string( y ) + ")\r\n";
    SendCommand( hSerial, command.c_str( ) );
}

std::shared_ptr< sdk::basePlayer > findTarget( )
{
	std::shared_ptr< sdk::basePlayer > temp{ };

	vector3 localView = global::localPlayer->getViewAngles( );
	vector3 localPos = global::localPlayer->getPosition( );
	int localFlags = global::localPlayer->getFlags( );

	vector3 center = { menu::screenSize.x / 2, menu::screenSize.y / 2 };

	vector3 finalAimpoint{ };

	float aimFov = 50.0f * 6.5;

	for ( auto player : cheat::players )
	{
		if ( player->getTeam( ) == global::localPlayer->getTeam( ) )
			continue;

		if ( !player->isValid( ) )
			continue;

		if ( player == global::localPlayer )
			continue;

		if ( player->getPosition( ).calculateDistance( global::localPlayer->getViewAngles( ) ) > 500.0f )
			continue;

		vector3 aimpoint = player->boneList( )[ BONEINDEX::neck_0 ].position;

		vector3 aimpointScreen = cheat::worldToScreen( &aimpoint );

		float distance = aimpointScreen.calculateDistance( center );

		if ( distance < aimFov )
		{
			temp = player;
			aimFov = distance;
			finalAimpoint = aimpoint;
		}
	}

	return temp;
}

void aimAtPos( float x, float y, int aimSpeed, int smoothAmount )
{
    float TargetX = 0;
    float TargetY = 0;

    int ScreenCenterX = menu::screenSize.x / 2;
    int ScreenCenterY = menu::screenSize.y / 2;

    bool Smooth = true;

    if ( aimSpeed == 0 ) aimSpeed = 1;
    if ( smoothAmount == 0 ) smoothAmount = 1;

    if ( x != 0 )
    {
        if ( x > ScreenCenterX )
        {
            TargetX = -( ScreenCenterX - x );
            TargetX /= aimSpeed;
            if ( TargetX + ScreenCenterX > ScreenCenterX * 2 ) TargetX = 0;
        }

        if ( x < ScreenCenterX )
        {
            TargetX = x - ScreenCenterX;
            TargetX /= aimSpeed;
            if ( TargetX + ScreenCenterX < 0 ) TargetX = 0;
        }
    }

    if ( y != 0 )
    {
        if ( y > ScreenCenterY )
        {
            TargetY = -( ScreenCenterY - y );
            TargetY /= aimSpeed;
            if ( TargetY + ScreenCenterY > ScreenCenterY * 2 ) TargetY = 0;
        }

        if ( y < ScreenCenterY )
        {
            TargetY = y - ScreenCenterY;
            TargetY /= aimSpeed;
            if ( TargetY + ScreenCenterY < 0 ) TargetY = 0;
        }
    }

    if ( !Smooth )
    {
        mouse_event( 0x0001, ( UINT )( TargetX ), ( UINT )( TargetY ), NULL, NULL );
        return;
    }
    else
    {
        TargetX /= smoothAmount;
        TargetY /= smoothAmount;
        if ( abs( TargetX ) < 1 )
        {
            if ( TargetX > 0 )
            {
                TargetX = 1;
            }
            if ( TargetX < 0 )
            {
                TargetX = -1;
            }
        }
        if ( abs( TargetY ) < 1 )
        {
            if ( TargetY > 0 )
            {
                TargetY = 1;
            }
            if ( TargetY < 0 )
            {
                TargetY = -1;
            }
        }

        kmBox::sendMove( ( int )TargetX, ( int )TargetY );
        return;
    }
}

double distance( int x1, int y1, int x2, int y2 ) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    return std::sqrt( dx * dx + dy * dy );
}

void cheat::updateAimbot( )
{
    constexpr float MAX_DISTANCE = 99999.f;
    constexpr int FOV_HALF = 100; // Assuming FOV is 200 degrees total

    vector3 localView = global::localPlayer->getViewAngles( );
    vector3 localPos = global::localPlayer->getPosition( );
    int localFlags = global::localPlayer->getFlags( );

    vector3 aimSpot;

    int centerX = menu::screenSize.x / 2;
    int centerY = menu::screenSize.y / 2;

    int fovHalf = FOV_HALF;

    int lowerX = centerX - fovHalf;
    int upperX = centerX + fovHalf;

    int lowerY = centerY - fovHalf;
    int upperY = centerY + fovHalf;

    if ( GetAsyncKeyState( VK_RCONTROL ) )
    {
        float bestDistance = MAX_DISTANCE;
        bool targetFound = false;

        for ( auto player : cheat::players )
        {
            if ( player == global::localPlayer )
                continue;

            if ( !player->isAlive( ) || player->getTeam( ) == global::localPlayer->getTeam( ) || player->boneList( ).empty( ) )
                continue;

            vector3 playerPos = player->boneList( )[ BONEINDEX::head ].position;
            vector3 pos = cheat::worldToScreen( &playerPos );
            int x = pos.x;
            int y = pos.y;

            if ( x > lowerX && x < upperX && y > lowerY && y < upperY )
            {
                double dist = distance( centerX, centerY, x, y );

                if ( dist < bestDistance )
                {
                    bestDistance = dist;
                    aimSpot = vector3( x, y, 0 );
                    targetFound = true;
                }
            }
        }

        if ( bestDistance < MAX_DISTANCE && targetFound )
        {
            aimAtPos( aimSpot.x, aimSpot.y, 5, 1 );
            //kmBox::sendMove( 100, 100 );
            draw->AddRectFilled( { aimSpot.x, aimSpot.y }, { aimSpot.x + 6, aimSpot.y + 6 }, ImColor( 125, 125, 255 ) );
        }
    }
}