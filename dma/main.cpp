#include "menu.h"

#include <d3d9.h>
#include <tchar.h>
#include <iostream>
#include <thread>

// cheat includes
#include "cheat/cheat.hpp"
#include "cheat/kmbox.hpp"
#include "cheat/window/window.hpp"
#include "cheat/config.hpp"

void aimbot( )
{
    while ( true )
    {
        updateAimbot->execute( );
    }
}

int main( )
{
    SetConsoleTitleA( "TOMO DMA PROJECT" );

    config = std::make_unique<Config>( );

    std::thread( window::create ).detach( );

    std::thread( aimbot ).detach( );
    
    cheat::init( );
}