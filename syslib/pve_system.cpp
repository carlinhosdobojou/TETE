#include "pve_system.h"
#include "mubot.h"
#include <thread>

void PvESystem::Initialize()
{
    MuBot::LogMessage("Sistema PvE inicializado");
}

void PvESystem::Shutdown()
{
    m_autoFarm = false;
    m_autoReset = false;
    m_autoCollectItems = false;
    m_autoPot = false;
    MuBot::LogMessage("Sistema PvE finalizado");
}

void PvESystem::Update()
{
    if (m_autoFarm)
        ProcessAutoFarm();
        
    if (m_autoReset)
        ProcessAutoReset();
        
    if (m_autoCollectItems)
        ProcessAutoCollectItems();
        
    if (m_autoPot)
        ProcessAutoPot();
        
    ProcessSkills();
}

void PvESystem::SetAutoFarm(bool enabled)
{
    m_autoFarm = enabled;
    if (enabled)
        MuBot::LogMessage("Auto Farm ativado");
    else
        MuBot::LogMessage("Auto Farm desativado");
}

void PvESystem::SetAutoReset(bool enabled, int level)
{
    m_autoReset = enabled;
    m_resetLevel = level;
    if (enabled)
        MuBot::LogMessage("Auto Reset ativado para nível " + std::to_string(level));
    else
        MuBot::LogMessage("Auto Reset desativado");
}

void PvESystem::SetAutoCollectItems(bool enabled)
{
    m_autoCollectItems = enabled;
    if (enabled)
        MuBot::LogMessage("Auto Coleta de Itens ativado");
    else
        MuBot::LogMessage("Auto Coleta de Itens desativado");
}

void PvESystem::SetAutoPot(bool enabled, int healthPercent, int manaPercent)
{
    m_autoPot = enabled;
    m_healthPotPercent = healthPercent;
    m_manaPotPercent = manaPercent;
    if (enabled)
        MuBot::LogMessage("Auto Pot ativado (Vida: " + std::to_string(healthPercent) + "%, Mana: " + std::to_string(manaPercent) + "%)");
    else
        MuBot::LogMessage("Auto Pot desativado");
}

void PvESystem::AddSkill(const std::string& name, int delay, int key)
{
    Skill skill;
    skill.name = name;
    skill.delay = delay;
    skill.key = key;
    skill.lastUsed = std::chrono::steady_clock::now() - std::chrono::milliseconds(delay);
    
    m_skills.push_back(skill);
    MuBot::LogMessage("Skill adicionada: " + name);
}

void PvESystem::RemoveSkill(size_t index)
{
    if (index < m_skills.size())
    {
        std::string name = m_skills[index].name;
        m_skills.erase(m_skills.begin() + index);
        MuBot::LogMessage("Skill removida: " + name);
    }
}

void PvESystem::ClearSkills()
{
    m_skills.clear();
    MuBot::LogMessage("Todas as skills foram removidas");
}

void PvESystem::ProcessAutoFarm()
{
    auto now = std::chrono::steady_clock::now();
    
    // Check if enough time has passed since last farm action
    if (now - m_lastFarmAction < std::chrono::milliseconds(500))
        return;
        
    m_lastFarmAction = now;
    
    // Simple farming logic - attack nearby monsters
    // This would need to be implemented with proper game memory reading
    
    // For demonstration, we'll simulate attacking
    static int attackCounter = 0;
    attackCounter++;
    
    if (attackCounter % 10 == 0) // Every 5 seconds
    {
        SendKey(VK_SPACE); // Attack key
        MuBot::LogMessage("Auto Farm: Atacando monstro");
    }
}

void PvESystem::ProcessAutoReset()
{
    // This would need to read the current character level from game memory
    // For demonstration, we'll simulate level checking
    
    static int simulatedLevel = 350;
    simulatedLevel++; // Simulate leveling up
    
    if (simulatedLevel >= m_resetLevel)
    {
        MuBot::LogMessage("Nível " + std::to_string(m_resetLevel) + " atingido - Executando reset");
        
        // Simulate reset process
        SendKey(VK_F10); // Open reset window (example)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        SendKey(VK_RETURN); // Confirm reset
        
        simulatedLevel = 1; // Reset level
        MuBot::LogMessage("Reset executado com sucesso");
    }
}

void PvESystem::ProcessAutoCollectItems()
{
    auto now = std::chrono::steady_clock::now();
    
    // Check if enough time has passed since last collection
    if (now - m_lastItemCollection < std::chrono::milliseconds(2000))
        return;
        
    m_lastItemCollection = now;
    
    // Simulate item collection with ALT key
    SendKey(VK_MENU); // ALT key
    MuBot::LogMessage("Auto Coleta: Coletando itens");
}

void PvESystem::ProcessAutoPot()
{
    auto now = std::chrono::steady_clock::now();
    
    // Check if enough time has passed since last pot check
    if (now - m_lastPotCheck < std::chrono::milliseconds(1000))
        return;
        
    m_lastPotCheck = now;
    
    // This would need to read health/mana from game memory
    // For demonstration, we'll simulate health/mana checking
    
    static int simulatedHealth = 100;
    static int simulatedMana = 100;
    
    // Simulate damage taken
    if (rand() % 10 == 0)
    {
        simulatedHealth -= rand() % 30;
        simulatedMana -= rand() % 20;
    }
    
    // Use health pot if needed
    if (simulatedHealth < m_healthPotPercent)
    {
        SendKey(VK_F1); // Health pot key (example)
        simulatedHealth = 100;
        MuBot::LogMessage("Auto Pot: Usando poção de vida");
    }
    
    // Use mana pot if needed
    if (simulatedMana < m_manaPotPercent)
    {
        SendKey(VK_F2); // Mana pot key (example)
        simulatedMana = 100;
        MuBot::LogMessage("Auto Pot: Usando poção de mana");
    }
}

void PvESystem::ProcessSkills()
{
    if (m_skills.empty())
        return;
        
    auto now = std::chrono::steady_clock::now();
    
    // Check if current skill is ready to use
    if (m_currentSkillIndex < m_skills.size())
    {
        auto& skill = m_skills[m_currentSkillIndex];
        
        if (now - skill.lastUsed >= std::chrono::milliseconds(skill.delay))
        {
            SendKey(skill.key);
            skill.lastUsed = now;
            MuBot::LogMessage("Usando skill: " + skill.name);
            
            // Move to next skill
            m_currentSkillIndex = (m_currentSkillIndex + 1) % m_skills.size();
        }
    }
}

void PvESystem::SendKey(int vkCode)
{
    // Send key press
    keybd_event(vkCode, 0, 0, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    keybd_event(vkCode, 0, KEYEVENTF_KEYUP, 0);
}

void PvESystem::SendKeyWithDelay(int vkCode, int delay)
{
    SendKey(vkCode);
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
}