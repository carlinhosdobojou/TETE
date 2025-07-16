#pragma once

#include <Windows.h>
#include <vector>
#include <chrono>

class PvESystem
{
public:
    void Initialize();
    void Shutdown();
    void Update();
    
    void SetAutoFarm(bool enabled);
    void SetAutoReset(bool enabled, int level);
    void SetAutoCollectItems(bool enabled);
    void SetAutoPot(bool enabled, int healthPercent, int manaPercent);
    
    void AddSkill(const std::string& name, int delay, int key);
    void RemoveSkill(size_t index);
    void ClearSkills();

private:
    void ProcessAutoFarm();
    void ProcessAutoReset();
    void ProcessAutoCollectItems();
    void ProcessAutoPot();
    void ProcessSkills();
    
    void SendKey(int vkCode);
    void SendKeyWithDelay(int vkCode, int delay);
    
    bool m_autoFarm = false;
    bool m_autoReset = false;
    bool m_autoCollectItems = false;
    bool m_autoPot = false;
    
    int m_resetLevel = 400;
    int m_healthPotPercent = 30;
    int m_manaPotPercent = 20;
    
    struct Skill
    {
        std::string name;
        int delay;
        int key;
        std::chrono::steady_clock::time_point lastUsed;
    };
    
    std::vector<Skill> m_skills;
    size_t m_currentSkillIndex = 0;
    
    std::chrono::steady_clock::time_point m_lastFarmAction;
    std::chrono::steady_clock::time_point m_lastPotCheck;
    std::chrono::steady_clock::time_point m_lastItemCollection;
};