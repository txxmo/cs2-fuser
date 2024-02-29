#include "../cheat.hpp"

void cheat::renderESP( )
{
	for ( auto player : cheat::players )
	{
		if ( player.origin.x == global::localPos.x && player.origin.y == global::localPos.y && player.origin.z == global::localPos.z )
			continue;

		vector3 screenPos = cheat::worldToScreen( &player.origin );
		
		if ( screenPos.z >= 0.01f )
		{
			draw->AddText( { screenPos.x, screenPos.y }, ImColor( 255, 255, 255 ), player.name.c_str( ) );
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