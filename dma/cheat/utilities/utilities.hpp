#pragma once

#include "../cheat.hpp"

namespace utilities
{
    template <typename T>
    constexpr T clamp( const T& value, const T& min, const T& max ) {
        return ( value < min ) ? min : ( value > max ) ? max : value;
    }

    inline double distance( int x1, int y1, int x2, int y2 )
    {
        int dx = x2 - x1;
        int dy = y2 - y1;
        return std::sqrt( dx * dx + dy * dy );
    }

    void aimAtPosition( vector2 targetPos, int aimSpeed, int smoothAmount, int curve );

    inline BONEINDEX getBone( )
    {
        switch ( config->aim.aimbotBone )
        {
        case 0: return BONEINDEX::head;
        case 1: return BONEINDEX::neck_0;
        case 2: return BONEINDEX::spine_1;
        case 3: return BONEINDEX::pelvis;
        default: return BONEINDEX::head;
        }
    }
}