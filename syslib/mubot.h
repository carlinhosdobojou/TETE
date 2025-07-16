#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <memory>

namespace MuBot
{
    enum class MenuState
    {
        Main,
        PvE,
        PvP,
        License
    };

    enum class MenuSize
    {
        Full,
        Compact
    };

    struct BotConfig
    {
        bool pveEnabled = false;
        bool pvpEnabled = false;
        bool autoFarm = false;
        bool autoReset = false;
        bool autoCollectItems = false;
        bool autoPot = false;
        
        int resetLevel = 400;
        int healthPotPercent = 30;
        int manaPotPercent = 20;
        
        std::vector<std::string> skillList;
        std::vector<int> skillDelays;
        std::vector<int> skillKeys;
        
        bool pvpAutoTarget = false;
        bool autoShield = false;
        int shieldHealthPercent = 50;
        
        std::vector<std::string> combos;
        
        bool learningMode = false;
    };

    // Core functions
    void Initialize();
    void Shutdown();
    void Update();
    
    // Menu functions
    void RenderMenu();
    void HandleInput();
    
    // State management
    MenuState GetCurrentState();
    void SetMenuState(MenuState state);
    MenuSize GetMenuSize();
    void ToggleMenuSize();
    
    // Config management
    BotConfig& GetConfig();
    void SaveConfig();
    void LoadConfig();
    
    // License management
    bool IsLicenseValid();
    std::string GetLicenseInfo();
    int GetDaysRemaining();
    
    // Utility functions
    void LogMessage(const std::string& message);
    void ShowConfirmDialog(const std::string& message, std::function<void()> onConfirm);
}