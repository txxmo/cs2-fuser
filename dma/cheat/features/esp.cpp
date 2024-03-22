#include "../cheat.hpp"

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

void cheat::renderESP( )
{
	std::lock_guard<std::mutex> lock( global::cacheMutex );

	if ( !draw )
		return;

	vector2 center = { menu::screenSize.x / 2, menu::screenSize.y / 2 };

	if ( config.drawFov )
		draw->AddCircle( { ( float )center.x, ( float )center.y }, config.aimbotFov / 2, ImColor( 255, 255, 255 ) );

	if ( config.drawAimPoint && cheat::aimPoint.x != 0 && cheat::aimPoint.y != 0 )
		draw->AddRectFilled( { cheat::aimPoint.x,  cheat::aimPoint.y }, { cheat::aimPoint.x + 6,  cheat::aimPoint.y + 6 }, ImColor( 125, 125, 255 ) );

	if ( config.drawAimPoint && cheat::targetPlayer && cheat::targetPlayer->isValid( ) && cheat::targetPlayer != NULL )
		draw->AddText( { 1080, 720 }, ImColor( 255, 255, 255 ), cheat::targetPlayer->getName( ).c_str( ) );

	for ( auto player : cheat::players )
	{
		if ( player == global::localPlayer )
			continue;

		if ( player->boneList( ).empty( ) )
			continue;

		if ( player->getHealth( ) <= 0 || player->getHealth( ) > 100 )
			continue;

		if ( player->getTeam( ) == global::localPlayer->getTeam( ) && config.teamCheck )
			continue;

		vector3 playerPos = player->getPosition( );
		if ( math::isEqual( playerPos, global::localPos ) )
			continue;

		vector3 screenPos = cheat::worldToScreen( &playerPos );

		vector3 headPos = cheat::worldToScreen( &player->boneList( )[ BONEINDEX::head ].position );

		if ( !math::isInsideScreen( headPos ) )
			continue;

		bool isFriend = cheat::isFriend( player->getAddress( ) );
		
		if ( screenPos.z >= 0.01f )
		{
			if ( isFriend )
			{
				if ( config.nameESP )
					draw->AddText( { screenPos.x, screenPos.y }, friendColor, player->getName( ).c_str( ) );

				if ( config.headESP )
					draw->AddCircle( { headPos.x, headPos.y }, 4, friendColor, 30 );

				if ( config.skeletonESP )
					renderBones( player, friendColor );
			}
			else if ( player->getTeam( ) == global::localPlayer->getTeam( ) && !config.teamCheck )
			{
				if ( config.nameESP )
					draw->AddText( { screenPos.x, screenPos.y }, teamColor, player->getName( ).c_str( ) );

				if ( config.headESP )
					draw->AddCircle( { headPos.x, headPos.y }, 4, teamColor, 30 );

				if ( config.skeletonESP )
					renderBones( player, teamColor );
			}
			else
			{
				if ( config.nameESP )
					draw->AddText( { screenPos.x, screenPos.y }, enemyColor, player->getName( ).c_str( ) );

				if ( config.headESP )
					draw->AddCircle( { headPos.x, headPos.y }, 4, enemyColor, 30 );

				if ( config.skeletonESP )
					renderBones( player, enemyColor );
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