#include "../cheat.hpp"

void renderBones( std::shared_ptr<sdk::basePlayer> player )
{
	for ( int i = 0; i < sizeof( boneConnections ) / sizeof( boneConnections[ 0 ] ); ++i )
	{
		int bone1 = boneConnections[ i ].bone1;
		int bone2 = boneConnections[ i ].bone2;

		vector3 VectorBone1 = player->boneList( )[ bone1 ].position;
		vector3 VectorBone2 = player->boneList( )[ bone2 ].position;

		vector3 b1 = cheat::worldToScreen( &VectorBone1 );
		vector3 b2 = cheat::worldToScreen( &VectorBone2 );

		draw->AddLine( { b1.x, b1.y }, { b2.x, b2.y }, ImColor( 171, 216, 237 ) );
	}
}

bool isInsideScreen( const vector3& screenPos )
{
	return screenPos.x >= 0 && screenPos.x <= menu::screenSize.x && screenPos.y >= 0 && screenPos.y <= menu::screenSize.y;
}

bool isEqual( const vector3& v1, const vector3& v2 )
{
	const float epsilon = 0.0001f;
	return std::abs( v1.x - v2.x ) < epsilon && std::abs( v1.y - v2.y ) < epsilon && std::abs( v1.z - v2.z ) < epsilon;
}

void cheat::renderESP( )
{
	for ( auto player : cheat::players )
	{
		if ( player == global::localPlayer )
			continue;

		if ( !player->isAlive( ) || player->boneList( ).empty( ) )
			continue;

		if ( player->getTeam( ) == global::localPlayer->getTeam( ) && config.teamCheck )
			continue;

		vector3 playerPos = player->getPosition( );
		if ( isEqual( playerPos, global::localPos ) )
			continue;

		vector3 screenPos = cheat::worldToScreen( &playerPos );
		if ( !isInsideScreen( screenPos ) )
			continue;

		vector3 headPos = cheat::worldToScreen( &player->boneList( )[ BONEINDEX::head ].position );
		
		if ( screenPos.z >= 0.01f )
		{
			if ( config.nameESP )
				draw->AddText( { screenPos.x, screenPos.y }, ImColor( 255, 255, 255 ), player->getName( ).c_str( ) );

			if ( config.headESP )
				draw->AddCircle( { headPos.x, headPos.y }, 4, ImColor( 171, 216, 237 ), 30 );

			if ( config.skeletonESP )
				renderBones( player );
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