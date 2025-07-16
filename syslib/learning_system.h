#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>

struct GameEvent
{
    std::chrono::system_clock::time_point timestamp;
    std::string type;
    std::string data;
};

struct HealthManaInfo
{
    int health;
    int maxHealth;
    int mana;
    int maxMana;
    std::chrono::system_clock::time_point timestamp;
};

struct CharacterInfo
{
    std::string name;
    int level;
    int x, y;
    bool isPlayer;
    std::chrono::system_clock::time_point lastSeen;
};

struct ItemInfo
{
    std::string name;
    int x, y;
    std::string rarity;
    std::chrono::system_clock::time_point timestamp;
};

class LearningSystem
{
public:
    void Initialize();
    void Shutdown();
    void Update();
    
    void SetEnabled(bool enabled);
    bool IsEnabled() const;
    
    void LogEvent(const std::string& type, const std::string& data);
    void LogHealthMana(int health, int maxHealth, int mana, int maxMana);
    void LogCharacter(const std::string& name, int level, int x, int y, bool isPlayer);
    void LogItem(const std::string& name, int x, int y, const std::string& rarity);
    
    void ExportLogs(const std::string& format = "txt"); // "txt" or "json"
    void ClearLogs();
    
    std::vector<GameEvent> GetRecentEvents(int count = 50) const;
    std::vector<HealthManaInfo> GetHealthManaHistory() const;

private:
    void ProcessColorAnalysis();
    void ProcessOCR();
    void ProcessPixelScan();
    void SaveToFile();
    
    std::string GetCurrentTimeString();
    void WriteTextLog(const std::string& filename);
    void WriteJsonLog(const std::string& filename);
    
    bool m_enabled = false;
    
    std::vector<GameEvent> m_events;
    std::vector<HealthManaInfo> m_healthManaHistory;
    std::vector<CharacterInfo> m_characters;
    std::vector<ItemInfo> m_items;
    
    std::chrono::steady_clock::time_point m_lastUpdate;
    std::chrono::steady_clock::time_point m_lastSave;
    
    static const size_t MAX_EVENTS = 1000;
    static const size_t MAX_HEALTH_MANA_HISTORY = 500;
    static const size_t MAX_CHARACTERS = 100;
    static const size_t MAX_ITEMS = 200;
};