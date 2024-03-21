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

void aimAtPos( float x, float y, int aimSpeed, int smoothAmount, int curve )
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

        kmBox::sendMove( ( int )TargetX, ( int )TargetY, curve );
        return;
    }
}

double distance( int x1, int y1, int x2, int y2 ) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    return std::sqrt( dx * dx + dy * dy );
}

int getKey( )
{
    int temp = 0;
    switch ( config.aimbotKey )
    {
    case 0:
        return 0x01;
        break;
    case 1:
        return 0x02;
        break;
    case 2:
        return 0x04;
        break;
    case 3:
        return 0x05;
        break;
    case 4:
        return 0x06;
        break;
    }

    return 0x01;
}

BONEINDEX getBone( )
{
    int temp = 0;
    switch ( config.aimbotBone )
    {
    case 0:
        return BONEINDEX::head;
        break;
    case 1:
        return BONEINDEX::neck_0;
        break;
    case 2:
        return BONEINDEX::spine_1;
        break;
    case 3:
        return BONEINDEX::pelvis;
        break;
    }

    return BONEINDEX::head;
}

void cheat::updateAimbot( )
{
    constexpr float MAX_DISTANCE = 99999.f;
    int FOV_HALF = config.aimbotFov / 2; // Assuming FOV is 200 degrees total

    vector2 aimSpot( 0, 0 );

    vector2 center = { menu::screenSize.x / 2, menu::screenSize.y / 2 };

    int fovHalf = FOV_HALF;

    vector2 xPos = { center.x - fovHalf, center.x + fovHalf };
    vector2 yPos = { center.y - fovHalf, center.y + fovHalf };

    int aimbotKey = getKey( );

    draw->AddCircle( { ( float )center.x, ( float )center.y }, FOV_HALF, ImColor( 255, 255, 255 ) );

    if ( mem.GetKeyboard()->IsKeyDown( aimbotKey ) )
    {
        float bestDistance = MAX_DISTANCE;
        bool targetFound = false;

        for ( auto player : cheat::players )
        {
            if ( !player->isValid( ) )
                continue;

            if ( player == global::localPlayer )
                continue;

            if ( !player->isAlive( ) || player->boneList( ).empty( ) )
                continue;

            if ( player->getTeam( ) == global::localPlayer->getTeam( ) && config.teamCheck )
                continue;

            vector3 playerPos = player->boneList( )[ getBone( ) ].position;
            vector3 pos = cheat::worldToScreen( &playerPos );
            int x = pos.x;
            int y = pos.y;

            if ( x > xPos.x && x < xPos.y && y > yPos.x && y < yPos.y )
            {
                double dist = distance( center.x, center.y, x, y );

                if ( dist < bestDistance )
                {
                    bestDistance = dist;
                    aimSpot = vector2( x, y );
                    targetFound = true;
                }
            }
        }

        if ( bestDistance < MAX_DISTANCE && targetFound )
        {
            aimAtPos( aimSpot.x, aimSpot.y, config.aimbotSpeed, config.aimbotSmooth, config.aimbotCurve );
            draw->AddRectFilled( { aimSpot.x, aimSpot.y }, { aimSpot.x + 6, aimSpot.y + 6 }, ImColor( 125, 125, 255 ) );
        }
    }
}