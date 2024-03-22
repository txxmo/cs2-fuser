#pragma once
#define M_PI 3.14159265358979323846264338327950288

#include "../../menu.h"

#ifndef _VECTOR_ESP_
#define _VECTOR_ESP_
#include <numbers>
#include <cmath>

struct vector3
{
	// constructor
	constexpr vector3(
		const float x = 0.f,
		const float y = 0.f,
		const float z = 0.f ) noexcept :
		x( x ), y( y ), z( z ) { }

	// operator overloads
	constexpr const vector3& operator-( const vector3& other ) const noexcept
	{
		return vector3{ x - other.x, y - other.y, z - other.z };
	}

	constexpr const vector3& operator+( const vector3& other ) const noexcept
	{
		return vector3{ x + other.x, y + other.y, z + other.z };
	}

	constexpr const vector3& operator/( const float factor ) const noexcept
	{
		return vector3{ x / factor, y / factor, z / factor };
	}

	constexpr const vector3& operator*( const float factor ) const noexcept
	{
		return vector3{ x * factor, y * factor, z * factor };
	}

	constexpr const bool operator>( const vector3& other ) const noexcept {
		return x > other.x && y > other.y && z > other.z;
	}

	constexpr const bool operator>=( const vector3& other ) const noexcept {
		return x >= other.x && y >= other.y && z >= other.z;
	}

	constexpr const bool operator<( const vector3& other ) const noexcept {
		return x < other.x && y < other.y && z < other.z;
	}

	constexpr const bool operator<=( const vector3& other ) const noexcept {
		return x <= other.x && y <= other.y && z <= other.z;
	}

	// utils
	constexpr const vector3& ToAngle( ) const noexcept
	{
		return vector3{
			std::atan2( -z, std::hypot( x, y ) ) * ( 180.0f / std::numbers::pi_v<float> ),
			std::atan2( y, x ) * ( 180.0f / std::numbers::pi_v<float> ),
			0.0f };
	}

	float length( ) const {
		return std::sqrt( x * x + y * y + z * z );
	}

	float length2d( ) const {
		return std::sqrt( x * x + y * y );
	}

	vector3& operator+=( const vector3& other )
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	constexpr const bool IsZero( ) const noexcept
	{
		return x == 0.f && y == 0.f && z == 0.f;
	}

	float calculateDistance( const vector3& point ) {
		float dx = point.x - x;
		float dy = point.y - y;

		return sqrt( pow( dx, 2 ) + pow( dy, 2 ) );
	}

	// struct data
	float x, y, z;
};
#endif

namespace math
{
	inline vector3 aimbotCalculation( vector3 bonePos, vector3 localPos, vector3 viewAngle, int fFlags )
	{
		float yaw, pitch, distance, fov, deltaX, deltaY, deltaZ;

		deltaX = bonePos.x - localPos.x;
		deltaY = bonePos.y - localPos.y;

		if ( fFlags == 65667 )
			deltaZ = ( bonePos.z - 40.f ) - localPos.z;
		else
			deltaZ = ( bonePos.z - 60.f ) - localPos.z;

		distance = sqrt( pow( deltaX, 2 ) + pow( deltaY, 2 ) );
		yaw = atan2f( deltaY, deltaX ) * 180 / M_PI - viewAngle.y;
		pitch = -atan( deltaZ / distance ) * 180 / M_PI - viewAngle.x;

		vector3 aimPos{ pitch, yaw, 0.0f };

		return aimPos;
	}

	inline float fovCalculation( vector3 aimPos, float distance, float aimbotFov )
	{
		float fov = sqrt( pow( aimPos.x, 2 ) + pow( aimPos.y, 2 ) );

		fov -= ( distance * -1 ) / ( aimbotFov + distance );

		return fov * 6;
	}

	inline vector3 calculateScreenOffset( float angleX, float angleY, float previousX, float previousY, float fov )
	{
		int width = menu::screenSize.x;
		int height = menu::screenSize.y;

		float fovRad = fov * ( M_PI / 180.0f );

		double thetaX = ( angleX - previousX ) * ( M_PI / 180.0f );
		double offsetX = ( width * tan( thetaX ) ) / ( 2 * tan( fovRad / 2 ) );

		double thetaY = ( angleY - previousY ) * ( M_PI / 180.0f );
		double offsetY = ( height * tan( thetaY ) ) / ( 2 * tan( fovRad / 2 ) );

		return vector3( -offsetX, offsetY, 0.0f );
	}

	inline bool isInsideScreen( const vector3& screenPos )
	{
		return screenPos.x >= 0 && screenPos.x <= menu::screenSize.x && screenPos.y >= 0 && screenPos.y <= menu::screenSize.y;
	}

	inline bool isEqual( const vector3& v1, const vector3& v2 )
	{
		const float epsilon = 0.0001f;
		return std::abs( v1.x - v2.x ) < epsilon && std::abs( v1.y - v2.y ) < epsilon && std::abs( v1.z - v2.z ) < epsilon;
	}
}

struct vector2
{
	float x;
	float y;

	vector2( float _x, float _y ) : x( _x ), y( _y ) {}
	vector2() {}
};