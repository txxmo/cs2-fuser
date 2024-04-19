#include "../cheat.hpp"

#include "../kmbox.hpp"

#include "../utilities/utilities.hpp"

#include <algorithm>

std::shared_ptr < sdk::basePlayer > targetPlayer;

int getKey( ) 
{
    switch ( config->aim.aimbotKey ) 
    {
        case 0: return 0x01;
        case 1: return 0x02;
        case 2: return 0x04;
        case 3: return 0x05;
        case 4: return 0x06;
        default: return 0x01; 
    }
}

bool triggerBot( int i )
{
    for ( auto players : cheat::players )
    {

    }   

    return false;
}

vector2 getAimbotTarget( ) {
    const float MAX_DISTANCE = 99999.f;
    const vector2 center = { menu::screenSize.x / 2, menu::screenSize.y / 2 };

    float bestDistance = MAX_DISTANCE;
    vector2 aimSpot( 0, 0 );
    std::shared_ptr<sdk::basePlayer> targetPlayer = nullptr;

    for ( auto player : cheat::players ) {
        if ( player->getAddress( ) == global::localPlayer->getAddress( ) || player->getHealth( ) <= 0 || player->getHealth( ) > 100 || player->boneList( ).empty( ) )
            continue;

        if ( config->aim.teamCheck && player->getTeam( ) == global::localPlayer->getTeam( ) )
            continue;

        if ( cheat::isFriend( player->getAddress( ) ) )
            continue;

        vector3 playerPos = player->boneList( )[ utilities::getBone( ) ].position;
        vector3 pos = cheat::worldToScreen( &playerPos );
        if ( !math::isInsideScreen( pos ) || pos.z < 0.01f )
            continue;

        double distanceToPlayer = utilities::distance( global::localPlayer->getPosition( ).x, global::localPlayer->getPosition( ).y, player->getPosition( ).x, player->getPosition( ).y );

        float scaledDynamicFOV = config->aim.aimbotFovDynamic * pow( distanceToPlayer, config->aim.distanceScaleFactor );

        double distToCenter = utilities::distance( center.x, center.y, pos.x, pos.y );

        // Check if the distance is within the scaled dynamic FOV radius
        if ( distToCenter <= scaledDynamicFOV ) {
            bool isObstructed = config->aim.visCheck && !player->isSpotted( );

            if ( !isObstructed && distToCenter < bestDistance ) {
                bestDistance = distToCenter;
                aimSpot = { pos.x, pos.y };
                targetPlayer = player;
            }
        }
    }

    cheat::aimPoint = aimSpot;
    cheat::targetPlayer = targetPlayer;

    if ( bestDistance < MAX_DISTANCE ) {
        return aimSpot;
    }
    else {
        return { menu::screenSize.x / 2, menu::screenSize.y / 2 }; // Return the center of the screen if no target is found
    }
}

std::shared_ptr<cheatFunction> updateAimbot = std::make_shared<cheatFunction>( 5, [ ] 
{
    std::lock_guard<std::mutex> lock( global::cacheMutex );

    if ( !global::localPlayer || !global::localPlayer->isAlive( ) )
        return;

    const float MAX_DISTANCE = 99999.f;
    const vector2 center = { menu::screenSize.x / 2, menu::screenSize.y / 2 };

    const int aimbotKey = getKey( );
    if ( !mem.GetKeyboard( )->IsKeyDown( aimbotKey ) ) 
    {
        cheat::targetPlayer = nullptr;
        cheat::aimPoint = { 0, 0 };
        return;
    }

    vector2 targetSpot = getAimbotTarget( );

    if ( targetSpot.x == -1 && targetSpot.y == -1 ) 
        return;

    // Aim at the target spot
    utilities::aimAtPosition( targetSpot, config->aim.aimbotSpeed, config->aim.aimbotSmooth, config->aim.aimbotCurve );
} );