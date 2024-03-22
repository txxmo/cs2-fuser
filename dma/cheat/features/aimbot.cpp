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

void aimAtPos( vector2 targetPos, int aimSpeed, int smoothAmount, int curve )
{
    // Initialize target coordinates
    float TargetX = 0;
    float TargetY = 0;

    // Calculate screen center coordinates
    int ScreenCenterX = menu::screenSize.x / 2;
    int ScreenCenterY = menu::screenSize.y / 2;

    // Ensure aimSpeed and smoothAmount are not zero
    if ( aimSpeed == 0 ) aimSpeed = 1;
    if ( smoothAmount == 0 ) smoothAmount = 1;

    // Calculate target offset along X-axis
    if ( targetPos.x != 0 )
    {
        if ( targetPos.x > ScreenCenterX )
        {
            TargetX = -( ScreenCenterX - targetPos.x );
            TargetX /= aimSpeed;
            if ( TargetX + ScreenCenterX > ScreenCenterX * 2 ) TargetX = 0;
        }
        else if ( targetPos.x < ScreenCenterX )
        {
            TargetX = targetPos.x - ScreenCenterX;
            TargetX /= aimSpeed;
            if ( TargetX + ScreenCenterX < 0 ) TargetX = 0;
        }
    }

    // Calculate target offset along Y-axis
    if ( targetPos.y != 0 )
    {
        if ( targetPos.y > ScreenCenterY )
        {
            TargetY = -( ScreenCenterY - targetPos.y );
            TargetY /= aimSpeed;
            if ( TargetY + ScreenCenterY > ScreenCenterY * 2 ) TargetY = 0;
        }
        else if ( targetPos.y < ScreenCenterY )
        {
            TargetY = targetPos.y - ScreenCenterY;
            TargetY /= aimSpeed;
            if ( TargetY + ScreenCenterY < 0 ) TargetY = 0;
        }
    }

    // Check if aiming should be smooth
    if ( smoothAmount > 1 )
    {
        // Calculate smooth movement
        TargetX /= smoothAmount;
        TargetY /= smoothAmount;

        // Ensure minimum movement threshold
        if ( abs( TargetX ) < 1 )
        {
            TargetX = TargetX > 0 ? 1 : -1;
        }
        if ( abs( TargetY ) < 1 )
        {
            TargetY = TargetY > 0 ? 1 : -1;
        }

        // Send mouse movement using a smooth method
        kmBox::sendMove( ( int )TargetX, ( int )TargetY );
    }
    else
    {
        // Send direct mouse movement without smoothing
        kmBox::sendMove( ( int )TargetX, ( int )TargetY );
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

std::shared_ptr<cheatFunction> updateAimbot = std::make_shared<cheatFunction>( 5, [ ] {

    std::lock_guard<std::mutex> lock( global::cacheMutex );

    std::shared_ptr < sdk::basePlayer > targetPlayer;
        
    float MAX_DISTANCE = 99999.f;
    int FOV_HALF = config.aimbotFov / 2;
    int fovHalf = FOV_HALF;

    vector2 aimSpot( 0, 0 );

    vector2 center = { menu::screenSize.x / 2, menu::screenSize.y / 2 };

    vector2 xPos = { center.x - FOV_HALF, center.x + FOV_HALF };
    
    vector2 yPos = { center.y - FOV_HALF, center.y + FOV_HALF };

    if ( !global::localPlayer )
        return;

    if ( !global::localPlayer->isAlive( ) )
        return;

    int aimbotKey = getKey( );
    if ( mem.GetKeyboard( )->IsKeyDown( aimbotKey ) ) 
    {
        float bestDistance = MAX_DISTANCE;
        bool targetFound = false;

        for ( auto player : cheat::players ) 
        {
            if ( !player->isValid( ) )
                continue;

            if ( player == global::localPlayer || player->getHealth( ) <= 0 || player->getHealth( ) > 100 || player->boneList( ).empty( ) )
                continue;

            if ( config.teamCheck && player->getTeam( ) == global::localPlayer->getTeam( ) )
                continue;

            if ( targetFound )
                continue;

            bool isFriend = cheat::isFriend( player->getAddress( ) );
            if ( isFriend )
                continue;

            vector3 playerPos = player->boneList( )[ getBone( ) ].position;
            vector3 pos = cheat::worldToScreen( &playerPos );
            if ( !math::isInsideScreen( pos ) )
                continue;

            if ( pos.z >= 0.01f )
            {
                int x = pos.x;
                int y = pos.y;

                if ( x >= xPos.x && x <= xPos.y && y >= yPos.x && y <= yPos.y )
                {
                    double dist = distance( center.x, center.y, x, y );
                    bool isObstructed = false; // Implement line-of-sight check

                    if ( !isObstructed && dist < bestDistance ) {
                        bestDistance = dist;
                        aimSpot = vector2( x, y );
                        targetPlayer = player;
                        targetFound = true;
                    }
                }
            }
        }

        // aim at the target if found
        if ( bestDistance < MAX_DISTANCE && targetFound ) 
        {
            aimAtPos( aimSpot, config.aimbotSpeed, config.aimbotSmooth, config.aimbotCurve );
            cheat::aimPoint = aimSpot;
            cheat::targetPlayer = targetPlayer;
        }
    }
    else
    {
        cheat::targetPlayer = NULL;
        cheat::aimPoint = { 0, 0 };
    }
} );