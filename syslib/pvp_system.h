#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <chrono>

struct PvPStatistics
{
    float totalDamage = 0.0f;
    int wins = 0;
    int deaths = 0;
    int combosExecuted = 0;
};

struct Target
{
    std::string name;
    int health = 100;
    int x = 0, y = 0;
    bool isValid = false;
};

class PvPSystem
{
public:
    void Initialize();
    void Shutdown();
    void Update();
    
    void SetAutoTarget(bool enabled);
    void SetAutoShield(bool enabled, int healthPercent);
    
    void AddCombo(const std::string& combo);
    void RemoveCombo(size_t index);
    void ClearCombos();
    
    PvPStatistics GetStatistics() const;
    Target GetCurrentTarget() const;

private:
    void ProcessAutoTarget();
    void ProcessAutoShield();
    void ProcessCombos();
    void ProcessTargetOverlay();
    
    void ExecuteCombo(const std::string& combo);
    void SendKey(int vkCode);
    void SendKeySequence(const std::vector<int>& keys, int delay);
    
    Target FindNearestPlayer();
    void UpdateStatistics();
    
    bool m_autoTarget = false;
    bool m_autoShield = false;
    int m_shieldHealthPercent = 50;
    
    std::vector<std::string> m_combos;
    size_t m_currentComboIndex = 0;
    
    Target m_currentTarget;
    PvPStatistics m_statistics;
    
    std::chrono::steady_clock::time_point m_lastTargetUpdate;
    std::chrono::steady_clock::time_point m_lastShieldCheck;
    std::chrono::steady_clock::time_point m_lastComboExecution;
};