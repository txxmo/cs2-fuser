#include "ImGui/ImGui.h"
#include "ImGui/imgui_impl_dx9.h"
#include "ImGui/imgui_impl_win32.h"

#include "menu.h"
#include "globals.h"
#include "cheat/cheat.hpp"

#include <iostream>
#include <windows.h>

class initWindow {
public:
    const char* window_title = "Tomo DMA External";
    ImVec2 window_size{ 450, 260 };
    
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

void menu::render()
{
    std::lock_guard<std::mutex> lock( global::cacheMutex );

    if (global::active)
    {
        ImGui::SetNextWindowSize(iw.window_size);
        ImGui::SetNextWindowBgAlpha(1.0f);
        ImGui::Begin(iw.window_title, &global::active, iw.window_flags);
        {
            if ( ImGui::BeginTabBar( "cheattabs" ) ) 
            {
                if ( ImGui::BeginTabItem( "aimbot" ) ) 
                {
                    static const char* aimbotKeys[ ] = { "mouse 1", "mouse 2", "mouse 3", "mouse 4", "mouse 5" };
                    ImGui::Text( "keybind" );
                    if ( ImGui::BeginCombo( "##aimbotKeyCombo", aimbotKeys[ config.aimbotKey ] ) ) // ##aimbotKeyCombo is a unique identifier for the combo box
                    {
                        for ( int i = 0; i < IM_ARRAYSIZE( aimbotKeys ); i++ )
                        {
                            const bool isSelected = ( config.aimbotKey == i );
                            if ( ImGui::Selectable( aimbotKeys[ i ], isSelected ) )
                                config.aimbotKey = i;

                            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                            if ( isSelected )
                                ImGui::SetItemDefaultFocus( );
                        }

                        ImGui::EndCombo( );
                    }

                    static const char* aimbotBones[ ] = { "head", "neck", "chest", "pelvis" };
                    ImGui::Text( "target bone" );
                    if ( ImGui::BeginCombo( "##aimbotBoneCombo", aimbotBones[ config.aimbotBone ] ) ) // ##aimbotKeyCombo is a unique identifier for the combo box
                    {
                        for ( int i = 0; i < IM_ARRAYSIZE( aimbotBones ); i++ )
                        {
                            const bool isSelected = ( config.aimbotBone == i );
                            if ( ImGui::Selectable( aimbotBones[ i ], isSelected ) )
                                config.aimbotBone = i;

                            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                            if ( isSelected )
                                ImGui::SetItemDefaultFocus( );
                        }

                        ImGui::EndCombo( );
                    }

                    ImGui::Text( "speed" );
                    ImGui::SliderInt( "##aimbotspeed", &config.aimbotSpeed, 1, 30 );

                    ImGui::Text( "smoothing factor" );
                    ImGui::SliderInt( "##aimbotsmooth", &config.aimbotSmooth, 1, 30 );

                    ImGui::Text( "fov" );
                    ImGui::SliderInt( "##aimbotfov", &config.aimbotFov, 0, 1000 );

                    ImGui::Checkbox( "draw fov", &config.drawFov ); ImGui::SameLine( );
                    ImGui::Checkbox( "draw aim point", &config.drawAimPoint );
                    ImGui::EndTabItem( );
                }

                
                if ( ImGui::BeginTabItem( "esp" ) ) 
                {
                    ImGui::Checkbox( "show esp window", &visualsWindowSettings::isVisible );
                    ImGui::Checkbox( "name esp", &config.nameESP );
                    ImGui::Checkbox( "skeleton esp", &config.skeletonESP );
                    ImGui::Checkbox( "head circle", &config.headESP );
                    ImGui::EndTabItem( );
                }

                
                if ( ImGui::BeginTabItem( "misc" ) ) 
                {
                    ImGui::Checkbox( "team check", &config.teamCheck );
                    if ( ImGui::IsItemHovered( ) )
                        ImGui::SetTooltip( "option is applied globally." );

                    ImGui::Checkbox( "show debug window", &visualsWindowSettings::debugWindow );

                    ImGui::EndTabItem( );
                }

                if ( ImGui::BeginTabItem( "players" ) )
                {
                    ImGui::Text( "Name" );
                    ImGui::SameLine( 150 );
                    ImGui::Text( "Team" );
                    ImGui::SameLine( 240 );
                    ImGui::Text( "Health" );
                    ImGui::SameLine( 330 );
                    ImGui::Text( "Force Friendly" );

                    ImGui::Separator( );

                    for ( const auto& player : cheat::players ) {

                        ImGui::Text( player->getName( ).c_str( ) );
                        ImGui::SameLine( 150 );
                        ImGui::Text( "%s", player->getTeamString( ).c_str( ) );
                        ImGui::SameLine( 240 );
                        ImGui::Text( "%d", player->getHealth( ) );
                        ImGui::SameLine( 330 );
                        
                        bool isFriend = std::find( cheat::friends.begin( ), cheat::friends.end( ), player->getAddress( ) ) != cheat::friends.end( );
                        if ( ImGui::Checkbox( ( "##Checkbox" + std::to_string( player->getAddress( ) ) ).c_str( ), &isFriend ) ) {
                            if ( isFriend )
                                cheat::friends.push_back( player->getAddress( ) );
                            else 
                                cheat::friends.erase( std::remove( cheat::friends.begin( ), cheat::friends.end( ), player->getAddress( ) ), cheat::friends.end( ) );
                        }
                    }

                    ImGui::EndTabItem( );
                }

                ImGui::EndTabBar( );
            }
        }
        ImGui::End();
    }

    if ( visualsWindowSettings::isVisible )
    {
        ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( 0.0f, 0.0f, 0.0f, 1.0f ) );
        ImGui::Begin( "ESP Window", &global::active, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove );
        {
            if ( !visualsWindowSettings::initilized )
            {
                draw = ImGui::GetWindowDrawList( );
                visualsWindowSettings::initilized = true;
            }

            if ( visualsWindowSettings::initilized )
            {
                ImGui::SetWindowSize( ImVec2( static_cast< float >( menu::screenSize.x ), static_cast< float >( menu::screenSize.y ) ) );

                ImGui::SetWindowPos( { 0, 0 } );
            }
        }
        ImGui::End( );
        ImGui::PopStyleColor( );
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