#include "mubot.h"
#include "imgui_hook.h"
#include "license_validator.h"
#include "pve_system.h"
#include "pvp_system.h"
#include "learning_system.h"
#include "game_reader.h"

#include "external/imgui/imgui.h"
#include <fstream>
#include <sstream>
#include <chrono>

namespace MuBot
{
    static MenuState g_currentState = MenuState::Main;
    static MenuSize g_menuSize = MenuSize::Full;
    static BotConfig g_config;
    static LicenseValidator g_licenseValidator;
    static PvESystem g_pveSystem;
    static PvPSystem g_pvpSystem;
    static LearningSystem g_learningSystem;
    static GameReader g_gameReader;
    
    static bool g_showConfirmDialog = false;
    static std::string g_confirmMessage;
    static std::function<void()> g_confirmCallback;
    
    static std::vector<std::string> g_logMessages;
    static const size_t MAX_LOG_MESSAGES = 100;

    void Initialize()
    {
        LogMessage("MuBot inicializando...");
        
        // Initialize license validator
        if (!g_licenseValidator.Initialize())
        {
            LogMessage("ERRO: Falha ao validar licença!");
            return;
        }
        
        // Initialize systems
        g_pveSystem.Initialize();
        g_pvpSystem.Initialize();
        g_learningSystem.Initialize();
        g_gameReader.Initialize();
        
        // Load configuration
        LoadConfig();
        
        // Initialize ImGui hook
        if (!ImGuiHook::Load(RenderMenu, []() {
            ImGui::StyleColorsDark();
            
            // Customize colors for MU theme
            ImGuiStyle& style = ImGui::GetStyle();
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
            style.Colors[ImGuiCol_Header] = ImVec4(0.6f, 0.2f, 0.2f, 0.8f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.7f, 0.3f, 0.3f, 0.9f);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.8f, 0.4f, 0.4f, 1.0f);
            style.Colors[ImGuiCol_Button] = ImVec4(0.5f, 0.2f, 0.2f, 0.8f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.6f, 0.3f, 0.3f, 0.9f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.7f, 0.4f, 0.4f, 1.0f);
        }))
        {
            LogMessage("ERRO: Falha ao inicializar ImGui hook!");
            return;
        }
        
        LogMessage("MuBot inicializado com sucesso!");
    }

    void Shutdown()
    {
        LogMessage("MuBot finalizando...");
        
        // Stop all systems
        g_config.pveEnabled = false;
        g_config.pvpEnabled = false;
        
        SaveConfig();
        
        g_pveSystem.Shutdown();
        g_pvpSystem.Shutdown();
        g_learningSystem.Shutdown();
        g_gameReader.Shutdown();
        
        LogMessage("MuBot finalizado.");
    }

    void Update()
    {
        if (!g_licenseValidator.IsValid())
            return;
            
        // Update systems
        if (g_config.pveEnabled)
            g_pveSystem.Update();
            
        if (g_config.pvpEnabled)
            g_pvpSystem.Update();
            
        if (g_config.learningMode)
            g_learningSystem.Update();
            
        g_gameReader.Update();
    }

    void RenderMenu()
    {
        Update();
        
        // Handle F5 key for menu toggle
        static bool f5Pressed = false;
        if (GetAsyncKeyState(VK_F5) & 0x8000)
        {
            if (!f5Pressed)
            {
                f5Pressed = true;
                if (g_menuSize == MenuSize::Full)
                    g_menuSize = MenuSize::Compact;
                else
                    g_menuSize = MenuSize::Full;
            }
        }
        else
        {
            f5Pressed = false;
        }
        
        // Handle keyboard navigation
        HandleInput();
        
        // Set window size based on menu size
        ImVec2 windowSize = (g_menuSize == MenuSize::Full) ? ImVec2(500, 400) : ImVec2(300, 200);
        
        ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
        
        if (ImGui::Begin("MuBot v1.0", nullptr, ImGuiWindowFlags_NoResize))
        {
            // License status at top
            if (!g_licenseValidator.IsValid())
            {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "LICENÇA INVÁLIDA!");
                ImGui::End();
                return;
            }
            
            int daysRemaining = GetDaysRemaining();
            ImVec4 licenseColor = (daysRemaining < 5) ? ImVec4(1.0f, 0.0f, 0.0f, 1.0f) : ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
            ImGui::TextColored(licenseColor, "Validade: %d dias restantes", daysRemaining);
            
            ImGui::Separator();
            
            // Render current menu state
            switch (g_currentState)
            {
                case MenuState::Main:
                    RenderMainMenu();
                    break;
                case MenuState::PvE:
                    RenderPvEMenu();
                    break;
                case MenuState::PvP:
                    RenderPvPMenu();
                    break;
                case MenuState::License:
                    RenderLicenseMenu();
                    break;
            }
            
            // Show confirm dialog if needed
            if (g_showConfirmDialog)
            {
                ImGui::OpenPopup("Confirmação");
                if (ImGui::BeginPopupModal("Confirmação", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("%s", g_confirmMessage.c_str());
                    ImGui::Separator();
                    
                    if (ImGui::Button("Sim"))
                    {
                        if (g_confirmCallback)
                            g_confirmCallback();
                        g_showConfirmDialog = false;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancelar"))
                    {
                        g_showConfirmDialog = false;
                        ImGui::CloseCurrentPopup();
                    }
                    
                    ImGui::EndPopup();
                }
            }
        }
        ImGui::End();
    }

    void RenderMainMenu()
    {
        if (g_menuSize == MenuSize::Compact)
        {
            ImGui::Text("MuBot - Menu Compacto");
            if (ImGui::Button("PvE")) SetMenuState(MenuState::PvE);
            ImGui::SameLine();
            if (ImGui::Button("PvP")) SetMenuState(MenuState::PvP);
            return;
        }
        
        ImGui::Text("Menu Principal");
        ImGui::Separator();
        
        // PvE Section
        if (ImGui::Button("PvE", ImVec2(150, 40)))
        {
            SetMenuState(MenuState::PvE);
        }
        ImGui::SameLine();
        ImGui::Text("Status: %s", g_config.pveEnabled ? "ATIVO" : "Inativo");
        
        // PvP Section
        if (ImGui::Button("PvP", ImVec2(150, 40)))
        {
            SetMenuState(MenuState::PvP);
        }
        ImGui::SameLine();
        ImGui::Text("Status: %s", g_config.pvpEnabled ? "ATIVO" : "Inativo");
        
        // License info
        if (ImGui::Button("Informações da Licença", ImVec2(200, 30)))
        {
            SetMenuState(MenuState::License);
        }
        
        ImGui::Separator();
        
        // Learning mode toggle
        if (ImGui::Checkbox("Modo Aprendizado", &g_config.learningMode))
        {
            if (g_config.learningMode)
                LogMessage("Modo aprendizado ativado - coletando dados do jogo");
            else
                LogMessage("Modo aprendizado desativado");
        }
        
        // Log messages
        if (ImGui::CollapsingHeader("Log"))
        {
            ImGui::BeginChild("LogScroll", ImVec2(0, 100), true);
            for (const auto& msg : g_logMessages)
            {
                ImGui::Text("%s", msg.c_str());
            }
            ImGui::SetScrollHereY(1.0f);
            ImGui::EndChild();
        }
    }

    void RenderPvEMenu()
    {
        if (ImGui::Button("← Voltar"))
        {
            if (g_config.pveEnabled)
            {
                ShowConfirmDialog("Deseja retornar ao menu principal? Isso desativará o PvE.", []() {
                    g_config.pveEnabled = false;
                    SetMenuState(MenuState::Main);
                });
            }
            else
            {
                SetMenuState(MenuState::Main);
            }
        }
        
        ImGui::Separator();
        ImGui::Text("Sistema PvE");
        
        if (ImGui::Checkbox("Ativar PvE", &g_config.pveEnabled))
        {
            if (g_config.pveEnabled)
                LogMessage("PvE ativado");
            else
                LogMessage("PvE desativado");
        }
        
        if (g_config.pveEnabled)
        {
            ImGui::Separator();
            
            ImGui::Checkbox("Auto Farm", &g_config.autoFarm);
            ImGui::Checkbox("Auto Reset", &g_config.autoReset);
            if (g_config.autoReset)
            {
                ImGui::SliderInt("Nível para Reset", &g_config.resetLevel, 300, 500);
            }
            
            ImGui::Checkbox("Coleta de Itens (ALT)", &g_config.autoCollectItems);
            ImGui::Checkbox("Auto Pot", &g_config.autoPot);
            
            if (g_config.autoPot)
            {
                ImGui::SliderInt("Vida %", &g_config.healthPotPercent, 10, 90);
                ImGui::SliderInt("Mana %", &g_config.manaPotPercent, 10, 90);
            }
            
            // Skills configuration
            if (ImGui::CollapsingHeader("Configuração de Skills"))
            {
                static char skillName[64] = "";
                static int skillDelay = 1000;
                static int skillKey = VK_F1;
                
                ImGui::InputText("Nome da Skill", skillName, sizeof(skillName));
                ImGui::SliderInt("Delay (ms)", &skillDelay, 100, 5000);
                ImGui::SliderInt("Tecla", &skillKey, VK_F1, VK_F12);
                
                if (ImGui::Button("Adicionar Skill"))
                {
                    if (strlen(skillName) > 0)
                    {
                        g_config.skillList.push_back(std::string(skillName));
                        g_config.skillDelays.push_back(skillDelay);
                        g_config.skillKeys.push_back(skillKey);
                        memset(skillName, 0, sizeof(skillName));
                    }
                }
                
                // Display current skills
                for (size_t i = 0; i < g_config.skillList.size(); ++i)
                {
                    ImGui::Text("%s - Delay: %dms - Tecla: F%d", 
                               g_config.skillList[i].c_str(), 
                               g_config.skillDelays[i],
                               g_config.skillKeys[i] - VK_F1 + 1);
                    ImGui::SameLine();
                    if (ImGui::Button(("Remover##" + std::to_string(i)).c_str()))
                    {
                        g_config.skillList.erase(g_config.skillList.begin() + i);
                        g_config.skillDelays.erase(g_config.skillDelays.begin() + i);
                        g_config.skillKeys.erase(g_config.skillKeys.begin() + i);
                        break;
                    }
                }
            }
        }
    }

    void RenderPvPMenu()
    {
        if (ImGui::Button("← Voltar"))
        {
            if (g_config.pvpEnabled)
            {
                ShowConfirmDialog("Deseja retornar ao menu principal? Isso desativará o PvP.", []() {
                    g_config.pvpEnabled = false;
                    SetMenuState(MenuState::Main);
                });
            }
            else
            {
                SetMenuState(MenuState::Main);
            }
        }
        
        ImGui::Separator();
        ImGui::Text("Sistema PvP");
        
        if (ImGui::Checkbox("Ativar PvP", &g_config.pvpEnabled))
        {
            if (g_config.pvpEnabled)
                LogMessage("PvP ativado");
            else
                LogMessage("PvP desativado");
        }
        
        if (g_config.pvpEnabled)
        {
            ImGui::Separator();
            
            ImGui::Checkbox("Alvo Automático", &g_config.pvpAutoTarget);
            ImGui::Checkbox("Auto Shield", &g_config.autoShield);
            
            if (g_config.autoShield)
            {
                ImGui::SliderInt("Shield quando vida < %", &g_config.shieldHealthPercent, 10, 90);
            }
            
            // Combos configuration
            if (ImGui::CollapsingHeader("Combos"))
            {
                static char comboName[128] = "";
                ImGui::InputText("Combo", comboName, sizeof(comboName));
                
                if (ImGui::Button("Adicionar Combo"))
                {
                    if (strlen(comboName) > 0)
                    {
                        g_config.combos.push_back(std::string(comboName));
                        memset(comboName, 0, sizeof(comboName));
                    }
                }
                
                // Display current combos
                for (size_t i = 0; i < g_config.combos.size(); ++i)
                {
                    ImGui::Text("%zu. %s", i + 1, g_config.combos[i].c_str());
                    ImGui::SameLine();
                    if (ImGui::Button(("Remover##combo" + std::to_string(i)).c_str()))
                    {
                        g_config.combos.erase(g_config.combos.begin() + i);
                        break;
                    }
                }
            }
            
            // PvP Statistics
            if (ImGui::CollapsingHeader("Estatísticas"))
            {
                auto stats = g_pvpSystem.GetStatistics();
                ImGui::Text("Dano Total: %.0f", stats.totalDamage);
                ImGui::Text("Vitórias: %d", stats.wins);
                ImGui::Text("Mortes: %d", stats.deaths);
                ImGui::Text("K/D Ratio: %.2f", stats.deaths > 0 ? (float)stats.wins / stats.deaths : (float)stats.wins);
            }
        }
    }

    void RenderLicenseMenu()
    {
        if (ImGui::Button("← Voltar"))
        {
            SetMenuState(MenuState::Main);
        }
        
        ImGui::Separator();
        ImGui::Text("Informações da Licença");
        ImGui::Separator();
        
        auto licenseInfo = g_licenseValidator.GetLicenseInfo();
        
        ImGui::Text("Usuário: %s", licenseInfo.usuario.c_str());
        ImGui::Text("Validade: %s", licenseInfo.validadeStr.c_str());
        
        int daysRemaining = GetDaysRemaining();
        ImVec4 color = (daysRemaining < 5) ? ImVec4(1.0f, 0.0f, 0.0f, 1.0f) : ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        ImGui::TextColored(color, "Dias restantes: %d", daysRemaining);
        
        ImGui::Text("PCs permitidos: %d", licenseInfo.quantidadePCs);
        ImGui::Text("Status: %s", g_licenseValidator.IsValid() ? "VÁLIDA" : "INVÁLIDA");
    }

    void HandleInput()
    {
        // Handle arrow key navigation
        static bool rightPressed = false;
        static bool leftPressed = false;
        
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
        {
            if (!rightPressed)
            {
                rightPressed = true;
                // Enter submenu logic here if needed
            }
        }
        else
        {
            rightPressed = false;
        }
        
        if (GetAsyncKeyState(VK_LEFT) & 0x8000)
        {
            if (!leftPressed)
            {
                leftPressed = true;
                // Go back logic
                if (g_currentState != MenuState::Main)
                {
                    if (g_config.pveEnabled || g_config.pvpEnabled)
                    {
                        ShowConfirmDialog("Deseja retornar ao menu principal? Isso desativará a função atual.", []() {
                            g_config.pveEnabled = false;
                            g_config.pvpEnabled = false;
                            SetMenuState(MenuState::Main);
                        });
                    }
                    else
                    {
                        SetMenuState(MenuState::Main);
                    }
                }
            }
        }
        else
        {
            leftPressed = false;
        }
    }

    MenuState GetCurrentState()
    {
        return g_currentState;
    }

    void SetMenuState(MenuState state)
    {
        g_currentState = state;
    }

    MenuSize GetMenuSize()
    {
        return g_menuSize;
    }

    void ToggleMenuSize()
    {
        g_menuSize = (g_menuSize == MenuSize::Full) ? MenuSize::Compact : MenuSize::Full;
    }

    BotConfig& GetConfig()
    {
        return g_config;
    }

    void SaveConfig()
    {
        // Save configuration to file
        std::ofstream file("mubot_config.txt");
        if (file.is_open())
        {
            file << "pveEnabled=" << g_config.pveEnabled << std::endl;
            file << "pvpEnabled=" << g_config.pvpEnabled << std::endl;
            file << "autoFarm=" << g_config.autoFarm << std::endl;
            file << "autoReset=" << g_config.autoReset << std::endl;
            file << "resetLevel=" << g_config.resetLevel << std::endl;
            file << "healthPotPercent=" << g_config.healthPotPercent << std::endl;
            file << "manaPotPercent=" << g_config.manaPotPercent << std::endl;
            file << "learningMode=" << g_config.learningMode << std::endl;
            file.close();
        }
    }

    void LoadConfig()
    {
        std::ifstream file("mubot_config.txt");
        if (file.is_open())
        {
            std::string line;
            while (std::getline(file, line))
            {
                size_t pos = line.find('=');
                if (pos != std::string::npos)
                {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);
                    
                    if (key == "pveEnabled") g_config.pveEnabled = (value == "1");
                    else if (key == "pvpEnabled") g_config.pvpEnabled = (value == "1");
                    else if (key == "autoFarm") g_config.autoFarm = (value == "1");
                    else if (key == "autoReset") g_config.autoReset = (value == "1");
                    else if (key == "resetLevel") g_config.resetLevel = std::stoi(value);
                    else if (key == "healthPotPercent") g_config.healthPotPercent = std::stoi(value);
                    else if (key == "manaPotPercent") g_config.manaPotPercent = std::stoi(value);
                    else if (key == "learningMode") g_config.learningMode = (value == "1");
                }
            }
            file.close();
        }
    }

    bool IsLicenseValid()
    {
        return g_licenseValidator.IsValid();
    }

    std::string GetLicenseInfo()
    {
        auto info = g_licenseValidator.GetLicenseInfo();
        return "Usuário: " + info.usuario + " | Validade: " + info.validadeStr;
    }

    int GetDaysRemaining()
    {
        return g_licenseValidator.GetDaysRemaining();
    }

    void LogMessage(const std::string& message)
    {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        
        std::stringstream ss;
        ss << "[" << std::put_time(&tm, "%H:%M:%S") << "] " << message;
        
        g_logMessages.push_back(ss.str());
        
        if (g_logMessages.size() > MAX_LOG_MESSAGES)
        {
            g_logMessages.erase(g_logMessages.begin());
        }
    }

    void ShowConfirmDialog(const std::string& message, std::function<void()> onConfirm)
    {
        g_confirmMessage = message;
        g_confirmCallback = onConfirm;
        g_showConfirmDialog = true;
    }
}