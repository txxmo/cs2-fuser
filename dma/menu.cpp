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
    ImVec2 window_size{ 310, 300 };
    
    DWORD window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize;
} iw;

namespace visualsWindowSettings
{
    bool isFullscreen = false;
    bool isVisible = false;
    bool resize = false;
    bool titleBar = false;
    bool forceTopMost = false;
    bool enableDragging = false;
    bool debugWindow = true;

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
    if (global::active)
    {
        ImGui::SetNextWindowSize(iw.window_size);
        ImGui::SetNextWindowBgAlpha(1.0f);
        ImGui::Begin(iw.window_title, &global::active, iw.window_flags);
        {
            static const char* aimbotKeys[ ] = { "mouse 1", "mouse 2", "mouse 3", "mouse 4", "mouse 5" };
            ImGui::Text( "aimbot key" );
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
            ImGui::Text( "aimbot bone" );
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

            ImGui::SliderInt( "aimbot speed", &config.aimbotSpeed, 1, 30 );
            ImGui::SliderInt( "aimbot smooth", &config.aimbotSmooth, 1, 30 );
            ImGui::SliderInt( "aimbot fov", &config.aimbotFov, 0, 600 );

            ImGui::Checkbox( "team check", &config.teamCheck );

            ImGui::Checkbox( "show esp window", &visualsWindowSettings::isVisible );
            ImGui::Checkbox( "name esp", &config.nameESP );
            ImGui::Checkbox( "skeleton esp", &config.skeletonESP );
            ImGui::Checkbox( "head circle", &config.headESP );
        }
        ImGui::End();

        if ( visualsWindowSettings::isVisible )
        {
            ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( 0.0f, 0.0f, 0.0f, 1.0f ) );
            ImGui::Begin( "ESP Window", &global::active, visualsWindowSettings::windowFlags );
            {
                if ( !visualsWindowSettings::initilized )
                {
                    draw = ImGui::GetWindowDrawList( );
                    visualsWindowSettings::initilized = true;
                }

                if ( visualsWindowSettings::initilized )
                {
                    if ( !visualsWindowSettings::isFullscreen ) {
                        originalWindowSize = ImGui::GetWindowSize( );
                        originalWindowPos = ImGui::GetWindowPos( );
                    }

                    if ( ImGui::Button( "SETTINGS" ) ) {
                        ImGui::OpenPopup( "SettingsPopup" );
                    }

                    // Settings popup
                    if ( ImGui::BeginPopup( "SettingsPopup" ) ) {
                        // Content of the settings popup goes here
                        ImGui::Text( "esp window settings" );
                        ImGui::Checkbox( "fullscreen mode", &visualsWindowSettings::isFullscreen );

                        ImGui::Checkbox( "show title bar", &visualsWindowSettings::titleBar );
                        if ( !visualsWindowSettings::titleBar ) {
                            visualsWindowSettings::windowFlags |= ImGuiWindowFlags_NoTitleBar;
                        }
                        else {
                            visualsWindowSettings::windowFlags &= ~ImGuiWindowFlags_NoTitleBar;
                        }

                        ImGui::Checkbox( "enable manual resize", &visualsWindowSettings::resize );
                        if ( visualsWindowSettings::resize ) {
                            visualsWindowSettings::windowFlags &= ~ImGuiWindowFlags_NoResize;
                        }
                        else {
                            visualsWindowSettings::windowFlags |= ImGuiWindowFlags_NoResize;
                        }

                        ImGui::Checkbox( "enable dragging", &visualsWindowSettings::enableDragging );
                        if ( visualsWindowSettings::enableDragging ) {
                            visualsWindowSettings::windowFlags &= ~ImGuiWindowFlags_NoMove;
                        }
                        else {
                            visualsWindowSettings::windowFlags |= ImGuiWindowFlags_NoMove;
                        }

                        ImGui::EndPopup( );
                    }

                    if ( visualsWindowSettings::isFullscreen ) {

                        originalWindowSize = ImGui::GetWindowSize( );
                        originalWindowPos = ImGui::GetWindowPos( );

                        GetPrimaryMonitorSize( );
                        ImGui::SetWindowSize( ImVec2( static_cast< float >( menu::screenSize.x ), static_cast< float >( menu::screenSize.y ) ) );

                        ImGui::SetWindowPos( { 0, 0 } );
                    }
                    else {
                        // Restore original size and position after exiting fullscreen
                        ImGui::SetWindowSize( originalWindowSize );
                        ImGui::SetWindowPos( originalWindowPos );
                    }

                    updateViewMatrix->execute( );
                    cachePlayers->execute( );
                    updatePlayers->execute( );
                    cheat::renderESP( );

                    cheat::updateAimbot( );
                }
            }
            ImGui::End( );
            ImGui::PopStyleColor( );
        }

        ImGui::SetNextWindowSize( { 700, 300 } );
        if ( visualsWindowSettings::debugWindow )
        {
            ImGui::Begin( "crappy debug window", &global::active, iw.window_flags );
            {
                ImGui::Text( "Name" );
                ImGui::SameLine( 150 );
                ImGui::Text( "Team" );
                ImGui::SameLine( 300 );
                ImGui::Text( "Health" );
                ImGui::SameLine( 450 );
                ImGui::Text( "Pawn Address" );
                ImGui::SameLine( 600 );
                ImGui::Text( "Address" );

                ImGui::Separator( );

                for ( const auto& player : cheat::players ) {
                    ImGui::Text( player->getName( ).c_str( ) );
                    ImGui::SameLine( 150 );
                    ImGui::Text( "%d", player->getTeam( ) );
                    ImGui::SameLine( 300 );
                    ImGui::Text( "%d", player->getHealth( ) );
                    ImGui::SameLine( 450 );
                    ImGui::Text( "%#llx", static_cast< uint64_t >( player->getPawn( ) ) ); // Assuming uintptr_t is 64-bit
                    ImGui::SameLine( 600 );
                    ImGui::Text( "%#llx", static_cast< uint64_t >( player->getAddress( ) ) ); // Assuming uintptr_t is 64-bit
                }

                ImGui::End( );
            }
        }
    }
    else
    {
        exit(0);
    }
}