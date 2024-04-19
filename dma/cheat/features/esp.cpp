#include "../cheat.hpp"

#include "../utilities/utilities.hpp"

ImColor enemyColor = ImColor( 227, 83, 70 );

ImColor teamColor = ImColor( 171, 216, 237 );

ImColor friendColor = ImColor( 70, 227, 117 );

void renderBones( std::shared_ptr<sdk::basePlayer> player, ImColor color )
{
	for ( int i = 0; i < sizeof( boneConnections ) / sizeof( boneConnections[ 0 ] ); ++i )
	{
		int bone1 = boneConnections[ i ].bone1;
		int bone2 = boneConnections[ i ].bone2;

		vector3 VectorBone1 = player->boneList( )[ bone1 ].position;
		vector3 VectorBone2 = player->boneList( )[ bone2 ].position;

		vector3 b1 = cheat::worldToScreen( &VectorBone1 );
		vector3 b2 = cheat::worldToScreen( &VectorBone2 );

		draw->AddLine( { b1.x, b1.y }, { b2.x, b2.y }, color );
	}
}

void renderBox( std::shared_ptr<sdk::basePlayer> player, ImColor color )
{
    vector3 headPos = player->boneList( )[ head ].position;
    vector3 originPos = player->getPosition( );

    vector3 screenHead = cheat::worldToScreen( &headPos );
    vector3 screenOrigin = cheat::worldToScreen( &originPos );

    float height = screenOrigin.y - screenHead.y;
    float width = height / 2.0f;

    draw->AddRect( { screenHead.x - 25.f, screenHead.y }, { screenHead.x + width, screenHead.y + height }, color );
}

void cheat::renderESP( ) 
{
    std::lock_guard<std::mutex> lock( global::cacheMutex );

    if ( !draw || !config )
        return;

    // Initialize colors
    ImColor enemyColor( 255, 0, 0 ); // Example enemy color
    ImColor friendColor( 0, 255, 0 ); // Example friend color
    ImColor teamColor( 0, 0, 255 ); // Example team color

    vector2 center = { menu::screenSize.x / 2, menu::screenSize.y / 2 };

    const bool drawAimPoint = config->visuals.drawAimPoint;

    for ( auto& player : cheat::players ) {
        if ( player->getAddress( ) == global::localPlayer->getAddress( ) || player->getHealth( ) <= 0 || player->getHealth( ) > 100 )
            continue;

        if ( player->getTeam( ) == global::localPlayer->getTeam( ) && config->aim.teamCheck )
            continue;

        vector3 playerPos = player->getPosition( );
        if ( math::isEqual( playerPos, global::localPos ) )
            continue;

        vector3 screenPos = cheat::worldToScreen( &playerPos );

        vector3 headPos = cheat::worldToScreen( &player->boneList( )[ BONEINDEX::head ].position );

        if ( !math::isInsideScreen( headPos ) || screenPos.z < 0.01f )
            continue;

        const ImColor& textColor = ( cheat::isFriend( player->getAddress( ) ) ? friendColor :
            ( player->getTeam( ) == global::localPlayer->getTeam( ) && !config->aim.teamCheck ? teamColor : enemyColor ) );

        if ( drawAimPoint && cheat::aimPoint.x != 0 && cheat::aimPoint.y != 0 )
            draw->AddRectFilled( { cheat::aimPoint.x, cheat::aimPoint.y }, { cheat::aimPoint.x + 6, cheat::aimPoint.y + 6 }, ImColor( 125, 125, 255 ) );

        if ( config->esp.name )
            draw->AddText( { screenPos.x, screenPos.y }, textColor, player->getName( ).c_str( ) );

        if ( config->esp.headSpot )
            draw->AddCircle( { headPos.x, headPos.y }, 4, textColor, 30 );

        if ( config->esp.skeleton )
            renderBones( player, textColor );

        if ( config->visuals.drawFov && !cheat::isFriend( player->getAddress( ) ) )
        {
            vector3 playerBone = cheat::worldToScreen( &player->boneList( )[ utilities::getBone( ) ].position );
            double distToCenter = utilities::distance( playerBone.x, playerBone.y, menu::screenSize.x / 2, menu::screenSize.y / 2 );
            double distanceToPlayer = utilities::distance( global::localPlayer->getPosition( ).x, global::localPlayer->getPosition( ).y, player->getPosition( ).x, player->getPosition( ).y );

            // Scale the dynamic FOV based on the distance
            float scaledDynamicFOV = config->aim.aimbotFovDynamic * pow( distanceToPlayer, config->aim.distanceScaleFactor );

            if ( distToCenter <= scaledDynamicFOV )
            {
                draw->AddCircle( { playerBone.x, playerBone.y }, scaledDynamicFOV, ImColor( 255, 255, 255 ) );
                draw->AddText( { playerBone.x, playerBone.y }, ImColor( 255, 255, 255 ), std::to_string( scaledDynamicFOV ).c_str( ) );
            }
        }
    }
}

vector3 cheat::worldToScreen( vector3* v )
{
	using namespace global;

	float _x = viewM[ 0 ][ 0 ] * v->x + viewM[ 0 ][ 1 ] * v->y + viewM[ 0 ][ 2 ] * v->z + viewM[ 0 ][ 3 ];
	float _y = viewM[ 1 ][ 0 ] * v->x + viewM[ 1 ][ 1 ] * v->y + viewM[ 1 ][ 2 ] * v->z + viewM[ 1 ][ 3 ];

	float w = viewM[ 3 ][ 0 ] * v->x + viewM[ 3 ][ 1 ] * v->y + viewM[ 3 ][ 2 ] * v->z + viewM[ 3 ][ 3 ];

	float inv_w = 1.f / w;
	_x *= inv_w;
	_y *= inv_w;

	float x = menu::screenSize.x * .5f;
	float y = menu::screenSize.y * .5f;

	x += 0.5f * _x * menu::screenSize.x + 0.5f;
	y -= 0.5f * _y * menu::screenSize.y + 0.5f;

	return { x, y, w };
}