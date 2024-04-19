#pragma once
#include <vector>
#include <string>
#include <filesystem>

class Config {
public:
    Config( ) noexcept;
    void load( size_t ) noexcept;
    void load( std::string name ) noexcept;
    void save( size_t ) const noexcept;
    void add( std::string ) noexcept;
    void remove( size_t ) noexcept;
    void rename( size_t, std::string ) noexcept;
    void reset( ) noexcept;
    void listConfigs( ) noexcept;
    void createConfigDir( ) const noexcept;

    constexpr auto& getConfigs( ) noexcept
    {
        return configs;
    }

    struct Aim {
        int aimbotSpeed{ 1 }; // lower = faster
        int aimbotSmooth{ 1 };
        float aimbotCurve{ 0 };
        int aimbotFov = { 5 };
        int aimbotFovDynamic = { 2 };
        int aimbotKey = { 0 };
        int aimbotBone = { 0 };
        bool aimbotEnabled{ 0 };
        bool teamCheck{ 0 };
        bool visCheck{ 0 };
        bool dynamicTarget{ 0 };
        float distanceScaleFactor{ 0.500f };

    } aim;

    struct Visuals {
        bool drawFov{ false };
        bool drawAimPoint{ false };
    } visuals;

    struct ESP {
        bool enable{ false };
        bool skeleton{ true };
        bool name{ true };
        bool headSpot{ false };
        bool health{ false };
    } esp;

private:
    std::filesystem::path path;
    std::vector<std::string> configs;
};

inline std::unique_ptr<Config> config;