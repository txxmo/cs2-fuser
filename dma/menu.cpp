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
    ImVec2 window_size{ 400, 400 };
    
    DWORD window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize;
} iw;

namespace visualsWindowSettings
{
    bool isFullscreen = false;
    bool isVisible = false;
    bool resize = true;
    bool titleBar = true;
    bool forceTopMost = false;
    bool enableDragging = false;
    
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
            ImGui::Checkbox( "show esp window", &visualsWindowSettings::isVisible );
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
                    updatePlayerPos->execute( );
                    cheat::renderESP( );

                    // putting this in brackets for scope
                    std::string localPlayerString = "localPlayer: " + std::to_string( global::localPlayer );
                    std::string localPawnString = "localPawn: " + std::to_string( global::localPawn );
                    std::string localTeamString = "localTeam: " + std::to_string( global::localTeam );

                    draw->AddText( { 10, 50 }, IM_COL32( 0, 0, 255, 255 ), localPlayerString.c_str( ) );
                    draw->AddText( { 10, 65 }, IM_COL32( 0, 0, 255, 255 ), localPawnString.c_str( ) );
                    draw->AddText( { 10, 80 }, IM_COL32( 0, 0, 255, 255 ), localTeamString.c_str( ) );
                }
            }
            ImGui::End( );
            ImGui::PopStyleColor( );
        }
    }
    else
    {
        exit(0);
    }
}