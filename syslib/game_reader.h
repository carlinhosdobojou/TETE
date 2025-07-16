#pragma once

#include <Windows.h>
#include <string>
#include <vector>

struct PlayerInfo
{
    std::string name;
    int level = 0;
    int health = 100;
    int maxHealth = 100;
    int mana = 100;
    int maxMana = 100;
    int x = 0, y = 0;
    bool isValid = false;
};

struct MonsterInfo
{
    std::string name;
    int level = 0;
    int health = 100;
    int maxHealth = 100;
    int x = 0, y = 0;
    bool isAlive = true;
    float distance = 0.0f;
};

class GameReader
{
public:
    void Initialize();
    void Shutdown();
    void Update();
    
    PlayerInfo GetPlayerInfo() const;
    std::vector<MonsterInfo> GetNearbyMonsters() const;
    std::vector<std::string> GetItemsOnGround() const;
    
    bool IsInGame() const;
    bool IsPlayerAlive() const;
    
    // Memory reading functions
    int ReadInt(DWORD address);
    float ReadFloat(DWORD address);
    std::string ReadString(DWORD address, int maxLength = 32);
    
private:
    void UpdatePlayerInfo();
    void UpdateMonsters();
    void UpdateItems();
    
    HANDLE m_processHandle = nullptr;
    DWORD m_processId = 0;
    
    PlayerInfo m_playerInfo;
    std::vector<MonsterInfo> m_monsters;
    std::vector<std::string> m_items;
    
    // Memory addresses (these would be found through reverse engineering)
    static const DWORD PLAYER_BASE_ADDR = 0x00400000;
    static const DWORD MONSTER_LIST_ADDR = 0x00500000;
    static const DWORD ITEM_LIST_ADDR = 0x00600000;
};