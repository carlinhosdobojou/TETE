#include "game_reader.h"
#include "mubot.h"
#include <tlhelp32.h>
#include <cmath>

void GameReader::Initialize()
{
    // Find the game process
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        
        if (Process32First(hSnapshot, &pe32))
        {
            do
            {
                if (strcmp(pe32.szExeFile, "Main.exe") == 0 || 
                    strstr(pe32.szExeFile, "mu") != nullptr)
                {
                    m_processId = pe32.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    
    if (m_processId != 0)
    {
        m_processHandle = OpenProcess(PROCESS_VM_READ, FALSE, m_processId);
        if (m_processHandle)
        {
            MuBot::LogMessage("Game Reader inicializado - Processo encontrado");
        }
        else
        {
            MuBot::LogMessage("Game Reader: Falha ao abrir processo do jogo");
        }
    }
    else
    {
        MuBot::LogMessage("Game Reader: Processo do jogo não encontrado");
    }
}

void GameReader::Shutdown()
{
    if (m_processHandle)
    {
        CloseHandle(m_processHandle);
        m_processHandle = nullptr;
    }
    MuBot::LogMessage("Game Reader finalizado");
}

void GameReader::Update()
{
    if (!m_processHandle)
        return;
        
    UpdatePlayerInfo();
    UpdateMonsters();
    UpdateItems();
}

PlayerInfo GameReader::GetPlayerInfo() const
{
    return m_playerInfo;
}

std::vector<MonsterInfo> GameReader::GetNearbyMonsters() const
{
    return m_monsters;
}

std::vector<std::string> GameReader::GetItemsOnGround() const
{
    return m_items;
}

bool GameReader::IsInGame() const
{
    return m_processHandle != nullptr && m_playerInfo.isValid;
}

bool GameReader::IsPlayerAlive() const
{
    return m_playerInfo.health > 0;
}

int GameReader::ReadInt(DWORD address)
{
    if (!m_processHandle)
        return 0;
        
    int value = 0;
    SIZE_T bytesRead;
    ReadProcessMemory(m_processHandle, (LPCVOID)address, &value, sizeof(int), &bytesRead);
    return value;
}

float GameReader::ReadFloat(DWORD address)
{
    if (!m_processHandle)
        return 0.0f;
        
    float value = 0.0f;
    SIZE_T bytesRead;
    ReadProcessMemory(m_processHandle, (LPCVOID)address, &value, sizeof(float), &bytesRead);
    return value;
}

std::string GameReader::ReadString(DWORD address, int maxLength)
{
    if (!m_processHandle)
        return "";
        
    char buffer[256] = {0};
    SIZE_T bytesRead;
    int length = std::min(maxLength, 255);
    
    ReadProcessMemory(m_processHandle, (LPCVOID)address, buffer, length, &bytesRead);
    return std::string(buffer);
}

void GameReader::UpdatePlayerInfo()
{
    if (!m_processHandle)
        return;
        
    // For demonstration, we'll simulate reading player info
    // In a real implementation, you would read from actual memory addresses
    
    static bool initialized = false;
    if (!initialized)
    {
        m_playerInfo.name = "TestPlayer";
        m_playerInfo.level = 350;
        m_playerInfo.isValid = true;
        initialized = true;
    }
    
    // Simulate health/mana changes
    static int healthTrend = -1;
    static int manaTrend = -1;
    
    m_playerInfo.health += healthTrend * (rand() % 5);
    m_playerInfo.mana += manaTrend * (rand() % 3);
    
    // Keep values in bounds
    if (m_playerInfo.health <= 20) healthTrend = 1;
    if (m_playerInfo.health >= 100) healthTrend = -1;
    if (m_playerInfo.mana <= 10) manaTrend = 1;
    if (m_playerInfo.mana >= 100) manaTrend = -1;
    
    m_playerInfo.health = std::max(0, std::min(100, m_playerInfo.health));
    m_playerInfo.mana = std::max(0, std::min(100, m_playerInfo.mana));
    
    // Simulate position changes
    m_playerInfo.x += (rand() % 3) - 1;
    m_playerInfo.y += (rand() % 3) - 1;
}

void GameReader::UpdateMonsters()
{
    if (!m_processHandle)
        return;
        
    // For demonstration, we'll simulate monster detection
    // In a real implementation, you would read from actual memory addresses
    
    static int updateCounter = 0;
    updateCounter++;
    
    // Update monster list every 50 frames (about 5 seconds at 10 FPS)
    if (updateCounter % 50 == 0)
    {
        m_monsters.clear();
        
        // Simulate finding monsters
        std::vector<std::string> monsterTypes = {
            "Orc", "Goblin", "Skeleton", "Wizard", "Dragon"
        };
        
        int monsterCount = rand() % 5; // 0-4 monsters
        for (int i = 0; i < monsterCount; ++i)
        {
            MonsterInfo monster;
            monster.name = monsterTypes[rand() % monsterTypes.size()];
            monster.level = 300 + (rand() % 100);
            monster.health = 80 + (rand() % 20);
            monster.maxHealth = 100;
            monster.x = m_playerInfo.x + (rand() % 200) - 100;
            monster.y = m_playerInfo.y + (rand() % 200) - 100;
            monster.isAlive = true;
            
            // Calculate distance
            float dx = (float)(monster.x - m_playerInfo.x);
            float dy = (float)(monster.y - m_playerInfo.y);
            monster.distance = sqrt(dx * dx + dy * dy);
            
            m_monsters.push_back(monster);
        }
    }
    
    // Update existing monsters
    for (auto& monster : m_monsters)
    {
        // Simulate monster taking damage
        if (rand() % 100 == 0)
        {
            monster.health -= rand() % 20;
            if (monster.health <= 0)
            {
                monster.isAlive = false;
                MuBot::LogMessage("Monstro eliminado: " + monster.name);
            }
        }
        
        // Simulate monster movement
        monster.x += (rand() % 3) - 1;
        monster.y += (rand() % 3) - 1;
        
        // Recalculate distance
        float dx = (float)(monster.x - m_playerInfo.x);
        float dy = (float)(monster.y - m_playerInfo.y);
        monster.distance = sqrt(dx * dx + dy * dy);
    }
    
    // Remove dead monsters
    m_monsters.erase(
        std::remove_if(m_monsters.begin(), m_monsters.end(),
            [](const MonsterInfo& m) { return !m.isAlive; }),
        m_monsters.end()
    );
}

void GameReader::UpdateItems()
{
    if (!m_processHandle)
        return;
        
    // For demonstration, we'll simulate item detection
    // In a real implementation, you would read from actual memory addresses
    
    static int itemUpdateCounter = 0;
    itemUpdateCounter++;
    
    // Update item list every 100 frames (about 10 seconds at 10 FPS)
    if (itemUpdateCounter % 100 == 0)
    {
        m_items.clear();
        
        // Simulate finding items
        std::vector<std::string> itemTypes = {
            "Sword +9", "Ring of Power", "Health Potion", "Mana Potion", 
            "Armor +7", "Jewel of Bless", "Soul", "Zen"
        };
        
        int itemCount = rand() % 4; // 0-3 items
        for (int i = 0; i < itemCount; ++i)
        {
            std::string item = itemTypes[rand() % itemTypes.size()];
            m_items.push_back(item);
        }
        
        if (!m_items.empty())
        {
            MuBot::LogMessage("Itens detectados no chão: " + std::to_string(m_items.size()));
        }
    }
}