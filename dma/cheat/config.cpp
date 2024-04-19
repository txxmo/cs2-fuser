#include "config.hpp"

#include <fstream>
#include <iterator>
#include <system_error>
#include <vector>
#include <Windows.h>
#include <shellapi.h>
#include <ShlObj.h>

#include "nlohmann/json.hpp"
#include "JsonForward.h"

Config::Config( ) noexcept
{
    PWSTR pathToDocuments;
    while ( !SUCCEEDED( SHGetKnownFolderPath( FOLDERID_Documents, 0, nullptr, &pathToDocuments ) ) )
        continue;

    path = pathToDocuments;
    CoTaskMemFree( pathToDocuments );

    path /= "CURBSTOMPCS2";
    createConfigDir( );
    listConfigs( );
    load( "default.json" );
}

void Config::createConfigDir( ) const noexcept
{
    std::error_code ec; std::filesystem::create_directory( path, ec );
}

void Config::listConfigs( ) noexcept
{
    configs.clear( );

    std::error_code ec;
    std::transform( std::filesystem::directory_iterator{ path, ec },
        std::filesystem::directory_iterator{ },
        std::back_inserter( configs ),
        [ ]( const auto& entry ) { return std::string{ ( const char* )entry.path( ).filename( ).u8string( ).c_str( ) }; } );
}

using json = nlohmann::basic_json<std::map, std::vector, std::string, bool, std::int64_t, std::uint64_t, float>;
using value_t = json::value_t;

template <value_t Type, typename T>
static typename std::enable_if_t<!std::is_same_v<T, bool>> read( const json& j, const char* key, T& o ) noexcept
{
    if ( !j.contains( key ) )
        return;

    if ( const auto& val = j[ key ]; val.type( ) == Type )
        val.get_to( o );
}

static void read( const json& j, const char* key, bool& o ) noexcept
{
    if ( !j.contains( key ) )
        return;

    if ( const auto& val = j[ key ]; val.type( ) == value_t::boolean )
        val.get_to( o );
}

static void read( const json& j, const char* key, float& o ) noexcept
{
    if ( !j.contains( key ) )
        return;

    if ( const auto& val = j[ key ]; val.type( ) == value_t::number_float )
        val.get_to( o );
}

static void read( const json& j, const char* key, int& o ) noexcept
{
    if ( !j.contains( key ) )
        return;

    if ( const auto& val = j[ key ]; val.is_number_integer( ) )
        val.get_to( o );
}

static void read( const json& j, const std::string key, int& o ) noexcept
{
    if ( !j.contains( key ) )
        return;

    if ( const auto& val = j[ key ]; val.is_number_integer( ) )
        val.get_to( o );
}

static void read( const json& j, const std::string key, bool& o ) noexcept
{
    if ( !j.contains( key ) )
        return;

    if ( const auto& val = j[ key ]; val.type( ) == value_t::boolean )
        val.get_to( o );
}

template <typename T, size_t Size>
static void read_array_opt( const json& j, const char* key, std::array<T, Size>& o ) noexcept
{
    if ( j.contains( key ) && j[ key ].type( ) == value_t::array ) {
        std::size_t i = 0;
        for ( const auto& e : j[ key ] ) {
            if ( i >= o.size( ) )
                break;

            if ( e.is_null( ) )
                continue;

            e.get_to( o[ i ] );
            ++i;
        }
    }
}

template <typename T, size_t Size>
static void read( const json& j, const char* key, std::array<T, Size>& o ) noexcept
{
    if ( !j.contains( key ) )
        return;

    if ( const auto& val = j[ key ]; val.type( ) == value_t::array && val.size( ) == o.size( ) ) {
        for ( std::size_t i = 0; i < val.size( ); ++i ) {
            if ( !val[ i ].empty( ) )
                val[ i ].get_to( o[ i ] );
        }
    }
}

template <typename T>
static void read( const json& j, const char* key, std::unordered_map<std::string, T>& o ) noexcept
{
    if ( j.contains( key ) && j[ key ].is_object( ) ) {
        for ( auto& element : j[ key ].items( ) )
            element.value( ).get_to( o[ element.key( ) ] );
    }
}

static void from_json( const json& j, Config::Aim& m )
{
    read( j, "Aimbot Enable", m.aimbotEnabled );
    read( j, "Aimbot Key", m.aimbotKey );
    read( j, "Aimbot Bone", m.aimbotBone );
    read( j, "Aimbot Smooth", m.aimbotSmooth );
    read( j, "Aimbot Speed", m.aimbotSpeed );
    read( j, "Aimbot FOV", m.aimbotFov );

    read( j, "Team Check", m.teamCheck );
    read( j, "Visible Check", m.visCheck );
    read( j, "Dynamic Target", m.dynamicTarget );
    read( j, "Dynamic FOV", m.aimbotFovDynamic );
    read( j, "Distance Scale", m.distanceScaleFactor );
}

static void from_json( const json& j, Config::Visuals& v )
{
    read( j, "Draw Aim Point", v.drawAimPoint );
    read( j, "Draw FOV", v.drawFov );
}

static void from_json( const json& j, Config::ESP& e )
{
    read( j, "Enable ESP", e.enable );
    read( j, "Enable Skeleton", e.skeleton );
    read( j, "Enable Name", e.name );
    read( j, "Enable Health Bar", e.headSpot );
    read( j, "Enable Health", e.health );
}

void Config::load( size_t id ) noexcept
{
    load( configs[ id ].c_str( ) );
}

void Config::load( std::string name ) noexcept
{
    reset( );

    json j;
    if ( std::ifstream in{ path / name }; in.good( ) ) {
        j = json::parse( in, nullptr, false, true );
        if ( j.is_discarded( ) )
            return;
    }
    else {
        return;
    }

    read<value_t::object>( j, "Aim", aim );
    read<value_t::object>( j, "Visuals", visuals );
    read<value_t::object>( j, "ESP", esp );
}

// WRITE macro requires:
// - json object named 'j'
// - object holding default values named 'dummy'
// - object to write to json named 'o'
#define WRITE(name, valueName) to_json(j[name], o.valueName, dummy.valueName)

template <typename T>
static void to_json( json& j, const T& o, const T& dummy )
{
    if ( o != dummy )
        j = o;
}

static void to_json( json& j, const Config::Aim& o )
{
    const Config::Aim dummy;

    WRITE( "Aimbot Enable", aimbotEnabled );
    WRITE( "Aimbot Key", aimbotKey );
    WRITE( "Aimbot Bone", aimbotBone );
    WRITE( "Aimbot Smooth", aimbotSmooth );
    WRITE( "Aimbot Speed", aimbotSpeed );
    WRITE( "Aimbot FOV", aimbotFov );
    WRITE( "Team Check", teamCheck );
    WRITE( "Visible Check", visCheck );

    WRITE( "Dynamic Target", dynamicTarget );
    WRITE( "Dynamic FOV", aimbotFovDynamic );
    WRITE( "Distance Scale", distanceScaleFactor );
}

static void to_json( json& j, const Config::Visuals& o )
{
    const Config::Visuals dummy;

    WRITE( "Draw Aim Point", drawAimPoint );
    WRITE( "Draw FOV", drawFov );
}

static void to_json( json& j, const Config::ESP& o )
{
    const Config::ESP dummy;

    WRITE( "Enable ESP", enable );
    WRITE( "Enable Skeleton", skeleton );
    WRITE( "Enable Name", name );
    WRITE( "Enable Health Bar", headSpot );
    WRITE( "Enable Health", health );
}

void removeEmptyObjects( json& j ) noexcept
{
    for ( auto it = j.begin( ); it != j.end( );) {
        auto& val = it.value( );
        if ( val.is_object( ) || val.is_array( ) )
            removeEmptyObjects( val );
        if ( val.empty( ) && !j.is_array( ) )
            it = j.erase( it );
        else
            ++it;
    }
}

void Config::save( size_t id ) const noexcept
{
    createConfigDir( );

    if ( id < 0 || id > configs.size( ) )
        return;

    if ( std::ofstream out{ path / ( const char* )configs[ id ].c_str( ) }; out.good( ) )
    {
        json j;

        j[ "Aim" ] = aim;
        j[ "Visuals" ] = visuals;
        j[ "ESP" ] = esp;

        removeEmptyObjects( j );
        out << std::setw( 2 ) << j;
    }
}

void Config::add( std::string name ) noexcept
{
    if ( std::find( configs.cbegin( ), configs.cend( ), name ) == configs.cend( ) ) {
        configs.emplace_back( name );
        reset( );
        save( configs.size( ) - 1 );
        load( name );
    }
}

void Config::remove( size_t id ) noexcept
{
    std::error_code ec;
    std::filesystem::remove( path / ( const char8_t* )configs[ id ].c_str( ), ec );
    configs.erase( configs.cbegin( ) + id );
}

void Config::rename( size_t item, std::string newName ) noexcept
{
    std::error_code ec;
    std::filesystem::rename( path / ( const char* )configs[ item ].c_str( ), path / ( const char* )newName.c_str( ), ec );
    configs[ item ] = newName;
}

void Config::reset( ) noexcept
{
    aim = { };
    visuals = { };
    esp = { };
}