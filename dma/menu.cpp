#include "ImGui/ImGui.h"
#include "ImGui/imgui_impl_dx9.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui//imgui_stdlib.h"

#include "menu.h"
#include "globals.h"
#include "cheat/cheat.hpp"

#include <iostream>
#include <windows.h>

class initWindow {
public:
    const char* window_title = "Tomo DMA External";
    ImVec2 window_size{ 410, 460 };
    
    DWORD window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
} iw;

namespace visualsWindowSettings
{
    bool isFullscreen = true;
    bool isVisible = false;
    bool resize = false;
    bool titleBar = false;
    bool forceTopMost = false;
    bool enableDragging = false;
    bool debugWindow = false;

    bool initilized = false;

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None;
}

ImVec2 originalWindowSize;
ImVec2 originalWindowPos;

void GetPrimaryMonitorSize( ) {
    menu::screenSize.x = GetSystemMetrics( SM_CXSCREEN );
    menu::screenSize.y = GetSystemMetrics( SM_CYSCREEN );
}

ImDrawList* draw = NULL;

ImFont* menu::iconFont = nullptr;
ImVec2 menu::screenSize = { 0, 0 };

void renderConfigs( )
{
    static int currentConfig = -1;
    static std::string buffer;

    auto& configItems = config->getConfigs( );

    if ( static_cast< std::size_t >( currentConfig ) >= configItems.size( ) )
        currentConfig = -1;

    ImGui::PushItemWidth( -1 );
    if ( ImGui::InputTextWithHint( "##configname", "config name", &buffer, ImGuiInputTextFlags_EnterReturnsTrue ) ) {
        if ( currentConfig != -1 )
            config->rename( currentConfig, buffer.c_str( ) );
    }

    if ( ImGui::ListBox( "##listbox", &currentConfig, [ ]( void* data, int idx, const char** out_text ) {
        auto& vector = *static_cast< std::vector<std::string>* >( data );
        *out_text = vector[ idx ].c_str( );
        return true;
        }, &configItems, configItems.size( ), 5 ) && currentConfig != -1 )
    {
        buffer = configItems[ currentConfig ];
    }

    if ( ImGui::Button( "Create config", { -1, 25.0f } ) && !buffer.empty( ) )
    {
        config->add( buffer.c_str( ) );
        buffer.clear( );
    }

    if ( ImGui::Button( "Load", { -1, 25.0f } ) && currentConfig != -1 )
        config->load( currentConfig );

    if ( ImGui::Button( "Save", { -1, 25.0f } ) && currentConfig != -1 )
        config->save( currentConfig );

    if ( ImGui::Button( "Delete", { -1, 25.0f } ) && currentConfig != -1 )
    {
        config->remove( currentConfig );
        if ( static_cast< std::size_t >( currentConfig ) < configItems.size( ) )
            buffer = configItems[ currentConfig ];
        else
            buffer.clear( );
    }
    ImGui::PopItemWidth( );
}

void menu::render()
{
    std::lock_guard<std::mutex> lock( global::cacheMutex );

    if (global::active)
    {
        ImGui::SetNextWindowSize(iw.window_size);
        ImGui::SetNextWindowBgAlpha(1.0f);
        ImGui::Begin(iw.window_title, &global::active, iw.window_flags);
        {
            draw = ImGui::GetWindowDrawList( );

            if ( ImGui::BeginTabBar( "cheattabs" ) ) 
            {
                if ( ImGui::BeginTabItem( "main" ) ) 
                {
                    ImGui::Checkbox( "Visible check (slow)", &config->aim.visCheck );
                    if ( ImGui::IsItemHovered( ) )
                        ImGui::SetTooltip( "Uses spotted check" );

                    // Aimbot settings
                    ImGui::Text( "Aimbot" );
                    static const char* aimbotKeys[ ] = { "mouse 1", "mouse 2", "mouse 3", "mouse 4", "mouse 5" };
                    static int aimbotTempKey = 0;
                    if ( ImGui::BeginCombo( "##aimbotKeyCombo", aimbotKeys[ aimbotTempKey ] ) ) {
                        for ( int i = 0; i < IM_ARRAYSIZE( aimbotKeys ); i++ ) {
                            const bool isSelected = ( aimbotTempKey == i );
                            if ( ImGui::Selectable( aimbotKeys[ i ], isSelected ) )
                                aimbotTempKey = i;

                            if ( isSelected )
                                ImGui::SetItemDefaultFocus( );
                        }

                        ImGui::EndCombo( );
                    }
                    config->aim.aimbotKey = aimbotTempKey;

                    static const char* aimbotBones[ ] = { "head", "neck", "chest", "pelvis" };
                    ImGui::Text( "target bone" );
                    if ( ImGui::BeginCombo( "##aimbotBoneCombo", aimbotBones[ config->aim.aimbotBone ] ) ) // ##aimbotKeyCombo is a unique identifier for the combo box
                    {
                        for ( int i = 0; i < IM_ARRAYSIZE( aimbotBones ); i++ )
                        {
                            const bool isSelected = ( config->aim.aimbotBone == i );
                            if ( ImGui::Selectable( aimbotBones[ i ], isSelected ) )
                                config->aim.aimbotBone = i;

                            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                            if ( isSelected )
                                ImGui::SetItemDefaultFocus( );
                        }

                        ImGui::EndCombo( );
                    }

                    ImGui::Text( "speed" );
                    ImGui::SliderInt( "##aimbotspeed", &config->aim.aimbotSpeed, 1, 30 );

                    ImGui::Text( "smoothing factor" );
                    ImGui::SliderInt( "##aimbotsmooth", &config->aim.aimbotSmooth, 1, 30 );

                    ImGui::Text( "fov" );
                    ImGui::SliderInt( "##aimbotfov2", &config->aim.aimbotFovDynamic, 0, 20 );
                    if ( ImGui::IsItemHovered( ) )
                        ImGui::SetTooltip( "fov is dynamic and is around the player, not the crosshair. \nenable show fov for visualization of it." );

                    ImGui::Text( "distance scale" );
                    ImGui::SliderFloat( "##distanescale", &config->aim.distanceScaleFactor, 0.001f, 0.600f );

                    // ESP settings
                    ImGui::Separator( );
                    ImGui::Text( "ESP" );
                    ImGui::Checkbox( "Name ESP", &config->esp.name );
                    ImGui::Checkbox( "Skeleton ESP", &config->esp.skeleton );
                    ImGui::Checkbox( "Head Circle", &config->esp.headSpot );

                    // Miscellaneous settings
                    ImGui::Separator( );
                    ImGui::Text( "Miscellaneous" );
                    ImGui::Checkbox( "Team Check", &config->aim.teamCheck );
                    if ( ImGui::IsItemHovered( ) )
                        ImGui::SetTooltip( "Option is applied globally." );

                    ImGui::Checkbox( "Show Debug Window", &visualsWindowSettings::debugWindow );

                    ImGui::Checkbox( "Draw FOV", &config->visuals.drawFov );
                    ImGui::Checkbox( "Draw Aim Point", &config->visuals.drawAimPoint );


                    ImGui::EndTabItem( );
                }

                if ( ImGui::BeginTabItem( "configs" ) )
                {
                    renderConfigs( );

                    ImGui::EndTabItem( );
                }

                if ( ImGui::BeginTabItem( "players" ) ) 
                {
                    ImGui::Columns( 4, nullptr, false );

                    ImGui::Text( "Name" );
                    ImGui::NextColumn( );

                    ImGui::Text( "Team" );
                    ImGui::NextColumn( );

                    ImGui::Text( "Health" );
                    ImGui::NextColumn( );

                    ImGui::Text( "Friend (?)" );
                    ImGui::NextColumn( );

                    ImGui::Separator( );

                    for ( const auto& player : cheat::players ) {

                        if ( player->getAddress( ) == global::localPlayer->getAddress( ) )
                            continue;

                        ImGui::Text( player->getName( ).c_str( ) );
                        ImGui::NextColumn( );

                        ImGui::Text( "%s", player->getTeamString( ).c_str( ) );
                        ImGui::NextColumn( );

                        ImGui::Text( "%d", player->getHealth( ) );
                        ImGui::NextColumn( );

                        bool isFriend = std::find( cheat::friends.begin( ), cheat::friends.end( ), player->getAddress( ) ) != cheat::friends.end( );
                        if ( ImGui::Checkbox( ( "##Checkbox" + std::to_string( player->getAddress( ) ) ).c_str( ), &isFriend ) ) {
                            if ( isFriend )
                                cheat::friends.push_back( player->getAddress( ) );
                            else
                                cheat::friends.erase( std::remove( cheat::friends.begin( ), cheat::friends.end( ), player->getAddress( ) ), cheat::friends.end( ) );
                        }
                        ImGui::NextColumn( );
                    }

                    ImGui::EndTabItem( );
                }

                ImGui::EndTabBar( );
            }
        }
        ImGui::End();
    }

    ImGui::SetNextWindowSize( { 200, 150 } );
    if ( visualsWindowSettings::debugWindow )
    {
        ImGui::Begin( "debug window", &global::active, iw.window_flags );
        {
            ImGui::Text( cheat::kmbBoxConnected ? "kmbox B+ connected" : "kmbox B+ not found" );
            ImGui::Text( cheat::connected ? "client is in-game" : "game server not found" );


            ImGui::End( );
        }
    }
}