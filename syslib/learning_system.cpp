#include "learning_system.h"
#include "mubot.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

void LearningSystem::Initialize()
{
    MuBot::LogMessage("Sistema de Aprendizado inicializado");
}

void LearningSystem::Shutdown()
{
    if (m_enabled)
    {
        SaveToFile();
        MuBot::LogMessage("Logs do sistema de aprendizado salvos");
    }
    MuBot::LogMessage("Sistema de Aprendizado finalizado");
}

void LearningSystem::Update()
{
    if (!m_enabled)
        return;
        
    auto now = std::chrono::steady_clock::now();
    
    // Update every 100ms
    if (now - m_lastUpdate < std::chrono::milliseconds(100))
        return;
        
    m_lastUpdate = now;
    
    ProcessColorAnalysis();
    ProcessOCR();
    ProcessPixelScan();
    
    // Save to file every 30 seconds
    if (now - m_lastSave > std::chrono::seconds(30))
    {
        m_lastSave = now;
        SaveToFile();
    }
}

void LearningSystem::SetEnabled(bool enabled)
{
    m_enabled = enabled;
    if (enabled)
    {
        MuBot::LogMessage("Modo Aprendizado ATIVADO - Coletando dados do jogo");
        LogEvent("SYSTEM", "Learning mode activated");
    }
    else
    {
        MuBot::LogMessage("Modo Aprendizado DESATIVADO");
        LogEvent("SYSTEM", "Learning mode deactivated");
        SaveToFile();
    }
}

bool LearningSystem::IsEnabled() const
{
    return m_enabled;
}

void LearningSystem::LogEvent(const std::string& type, const std::string& data)
{
    if (!m_enabled)
        return;
        
    GameEvent event;
    event.timestamp = std::chrono::system_clock::now();
    event.type = type;
    event.data = data;
    
    m_events.push_back(event);
    
    // Keep only recent events
    if (m_events.size() > MAX_EVENTS)
    {
        m_events.erase(m_events.begin());
    }
}

void LearningSystem::LogHealthMana(int health, int maxHealth, int mana, int maxMana)
{
    if (!m_enabled)
        return;
        
    HealthManaInfo info;
    info.health = health;
    info.maxHealth = maxHealth;
    info.mana = mana;
    info.maxMana = maxMana;
    info.timestamp = std::chrono::system_clock::now();
    
    m_healthManaHistory.push_back(info);
    
    // Keep only recent history
    if (m_healthManaHistory.size() > MAX_HEALTH_MANA_HISTORY)
    {
        m_healthManaHistory.erase(m_healthManaHistory.begin());
    }
    
    // Log significant changes
    if (m_healthManaHistory.size() > 1)
    {
        auto& prev = m_healthManaHistory[m_healthManaHistory.size() - 2];
        
        if (abs(health - prev.health) > 10)
        {
            std::stringstream ss;
            ss << "Health changed from " << prev.health << " to " << health;
            LogEvent("HEALTH", ss.str());
        }
        
        if (abs(mana - prev.mana) > 10)
        {
            std::stringstream ss;
            ss << "Mana changed from " << prev.mana << " to " << mana;
            LogEvent("MANA", ss.str());
        }
    }
}

void LearningSystem::LogCharacter(const std::string& name, int level, int x, int y, bool isPlayer)
{
    if (!m_enabled)
        return;
        
    // Find existing character or create new one
    auto it = std::find_if(m_characters.begin(), m_characters.end(),
        [&name](const CharacterInfo& c) { return c.name == name; });
        
    if (it != m_characters.end())
    {
        // Update existing character
        it->level = level;
        it->x = x;
        it->y = y;
        it->lastSeen = std::chrono::system_clock::now();
    }
    else
    {
        // Add new character
        CharacterInfo info;
        info.name = name;
        info.level = level;
        info.x = x;
        info.y = y;
        info.isPlayer = isPlayer;
        info.lastSeen = std::chrono::system_clock::now();
        
        m_characters.push_back(info);
        
        // Keep only recent characters
        if (m_characters.size() > MAX_CHARACTERS)
        {
            m_characters.erase(m_characters.begin());
        }
        
        std::stringstream ss;
        ss << (isPlayer ? "Player" : "Monster") << " detected: " << name 
           << " (Level " << level << ") at (" << x << ", " << y << ")";
        LogEvent("CHARACTER", ss.str());
    }
}

void LearningSystem::LogItem(const std::string& name, int x, int y, const std::string& rarity)
{
    if (!m_enabled)
        return;
        
    ItemInfo info;
    info.name = name;
    info.x = x;
    info.y = y;
    info.rarity = rarity;
    info.timestamp = std::chrono::system_clock::now();
    
    m_items.push_back(info);
    
    // Keep only recent items
    if (m_items.size() > MAX_ITEMS)
    {
        m_items.erase(m_items.begin());
    }
    
    std::stringstream ss;
    ss << "Item found: " << name << " (" << rarity << ") at (" << x << ", " << y << ")";
    LogEvent("ITEM", ss.str());
}

void LearningSystem::ExportLogs(const std::string& format)
{
    if (format == "json")
    {
        WriteJsonLog("mubot_learning_export.json");
    }
    else
    {
        WriteTextLog("mubot_learning_export.txt");
    }
    
    MuBot::LogMessage("Logs exportados para mubot_learning_export." + format);
}

void LearningSystem::ClearLogs()
{
    m_events.clear();
    m_healthManaHistory.clear();
    m_characters.clear();
    m_items.clear();
    
    MuBot::LogMessage("Logs do sistema de aprendizado limpos");
}

std::vector<GameEvent> LearningSystem::GetRecentEvents(int count) const
{
    std::vector<GameEvent> recent;
    
    int start = std::max(0, (int)m_events.size() - count);
    for (int i = start; i < (int)m_events.size(); ++i)
    {
        recent.push_back(m_events[i]);
    }
    
    return recent;
}

std::vector<HealthManaInfo> LearningSystem::GetHealthManaHistory() const
{
    return m_healthManaHistory;
}

void LearningSystem::ProcessColorAnalysis()
{
    // This would analyze screen colors to detect health/mana bars
    // For demonstration, we'll simulate color analysis
    
    static int frameCounter = 0;
    frameCounter++;
    
    if (frameCounter % 50 == 0) // Every 5 seconds at 10 FPS
    {
        // Simulate health/mana detection
        int simulatedHealth = 70 + (rand() % 30);
        int simulatedMana = 60 + (rand() % 40);
        
        LogHealthMana(simulatedHealth, 100, simulatedMana, 100);
    }
}

void LearningSystem::ProcessOCR()
{
    // This would perform OCR on screen text to detect names, items, etc.
    // For demonstration, we'll simulate OCR detection
    
    static int ocrCounter = 0;
    ocrCounter++;
    
    if (ocrCounter % 100 == 0) // Every 10 seconds at 10 FPS
    {
        // Simulate character detection
        std::vector<std::string> monsterNames = {
            "Orc", "Goblin", "Skeleton", "Dragon", "Wizard"
        };
        
        std::string name = monsterNames[rand() % monsterNames.size()];
        int level = 50 + (rand() % 100);
        int x = rand() % 800;
        int y = rand() % 600;
        
        LogCharacter(name, level, x, y, false);
    }
    
    if (ocrCounter % 150 == 0) // Every 15 seconds at 10 FPS
    {
        // Simulate item detection
        std::vector<std::string> itemNames = {
            "Sword of Power", "Magic Ring", "Health Potion", "Mana Potion", "Armor"
        };
        
        std::vector<std::string> rarities = {
            "Common", "Rare", "Epic", "Legendary"
        };
        
        std::string itemName = itemNames[rand() % itemNames.size()];
        std::string rarity = rarities[rand() % rarities.size()];
        int x = rand() % 800;
        int y = rand() % 600;
        
        LogItem(itemName, x, y, rarity);
    }
}

void LearningSystem::ProcessPixelScan()
{
    // This would scan specific pixels for game state changes
    // For demonstration, we'll simulate pixel scanning
    
    static int pixelCounter = 0;
    pixelCounter++;
    
    if (pixelCounter % 200 == 0) // Every 20 seconds at 10 FPS
    {
        // Simulate buff/debuff detection
        std::vector<std::string> effects = {
            "Strength Buff", "Speed Boost", "Poison", "Shield", "Invisibility"
        };
        
        std::string effect = effects[rand() % effects.size()];
        LogEvent("EFFECT", effect + " detected");
    }
}

void LearningSystem::SaveToFile()
{
    WriteTextLog("mubot_learning_log.txt");
}

std::string LearningSystem::GetCurrentTimeString()
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void LearningSystem::WriteTextLog(const std::string& filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
        return;
        
    file << "MuBot Learning System Log\n";
    file << "Generated: " << GetCurrentTimeString() << "\n";
    file << "========================================\n\n";
    
    // Write events
    file << "EVENTS (" << m_events.size() << " total):\n";
    file << "----------------------------------------\n";
    for (const auto& event : m_events)
    {
        auto time_t = std::chrono::system_clock::to_time_t(event.timestamp);
        auto tm = *std::localtime(&time_t);
        
        file << "[" << std::put_time(&tm, "%H:%M:%S") << "] "
             << event.type << ": " << event.data << "\n";
    }
    
    // Write health/mana history
    file << "\nHEALTH/MANA HISTORY (" << m_healthManaHistory.size() << " entries):\n";
    file << "----------------------------------------\n";
    for (const auto& hm : m_healthManaHistory)
    {
        auto time_t = std::chrono::system_clock::to_time_t(hm.timestamp);
        auto tm = *std::localtime(&time_t);
        
        file << "[" << std::put_time(&tm, "%H:%M:%S") << "] "
             << "HP: " << hm.health << "/" << hm.maxHealth
             << " MP: " << hm.mana << "/" << hm.maxMana << "\n";
    }
    
    // Write characters
    file << "\nCHARACTERS (" << m_characters.size() << " total):\n";
    file << "----------------------------------------\n";
    for (const auto& char_info : m_characters)
    {
        auto time_t = std::chrono::system_clock::to_time_t(char_info.lastSeen);
        auto tm = *std::localtime(&time_t);
        
        file << "[" << std::put_time(&tm, "%H:%M:%S") << "] "
             << (char_info.isPlayer ? "PLAYER" : "MONSTER") << ": "
             << char_info.name << " (Lv." << char_info.level << ") "
             << "at (" << char_info.x << ", " << char_info.y << ")\n";
    }
    
    // Write items
    file << "\nITEMS (" << m_items.size() << " total):\n";
    file << "----------------------------------------\n";
    for (const auto& item : m_items)
    {
        auto time_t = std::chrono::system_clock::to_time_t(item.timestamp);
        auto tm = *std::localtime(&time_t);
        
        file << "[" << std::put_time(&tm, "%H:%M:%S") << "] "
             << item.name << " (" << item.rarity << ") "
             << "at (" << item.x << ", " << item.y << ")\n";
    }
    
    file.close();
}

void LearningSystem::WriteJsonLog(const std::string& filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
        return;
        
    file << "{\n";
    file << "  \"generated\": \"" << GetCurrentTimeString() << "\",\n";
    file << "  \"events\": [\n";
    
    // Write events
    for (size_t i = 0; i < m_events.size(); ++i)
    {
        const auto& event = m_events[i];
        auto time_t = std::chrono::system_clock::to_time_t(event.timestamp);
        auto tm = *std::localtime(&time_t);
        
        file << "    {\n";
        file << "      \"timestamp\": \"" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "\",\n";
        file << "      \"type\": \"" << event.type << "\",\n";
        file << "      \"data\": \"" << event.data << "\"\n";
        file << "    }";
        
        if (i < m_events.size() - 1)
            file << ",";
        file << "\n";
    }
    
    file << "  ],\n";
    file << "  \"healthManaHistory\": [\n";
    
    // Write health/mana history
    for (size_t i = 0; i < m_healthManaHistory.size(); ++i)
    {
        const auto& hm = m_healthManaHistory[i];
        auto time_t = std::chrono::system_clock::to_time_t(hm.timestamp);
        auto tm = *std::localtime(&time_t);
        
        file << "    {\n";
        file << "      \"timestamp\": \"" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "\",\n";
        file << "      \"health\": " << hm.health << ",\n";
        file << "      \"maxHealth\": " << hm.maxHealth << ",\n";
        file << "      \"mana\": " << hm.mana << ",\n";
        file << "      \"maxMana\": " << hm.maxMana << "\n";
        file << "    }";
        
        if (i < m_healthManaHistory.size() - 1)
            file << ",";
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    
    file.close();
}