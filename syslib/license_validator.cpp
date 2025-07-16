#include "license_validator.h"
#include <Windows.h>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

bool LicenseValidator::Initialize()
{
    // Check if injected by MuInject (simple check)
    HMODULE hModule = GetModuleHandleA("MuInject.exe");
    if (!hModule)
    {
        // Additional security check - look for MuInject process
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot != INVALID_HANDLE_VALUE)
        {
            PROCESSENTRY32 pe32;
            pe32.dwSize = sizeof(PROCESSENTRY32);
            
            bool muInjectFound = false;
            if (Process32First(hSnapshot, &pe32))
            {
                do
                {
                    if (strcmp(pe32.szExeFile, "MuInject.exe") == 0)
                    {
                        muInjectFound = true;
                        break;
                    }
                } while (Process32Next(hSnapshot, &pe32));
            }
            CloseHandle(hSnapshot);
            
            if (!muInjectFound)
            {
                return false; // Not injected by MuInject
            }
        }
    }
    
    // Try to find license file
    std::vector<std::string> possiblePaths = {
        "app.licenca",
        "..\\app.licenca",
        "..\\MuInject\\app.licenca"
    };
    
    for (const auto& path : possiblePaths)
    {
        if (ValidateLicenseFile(path))
        {
            m_isValid = true;
            return true;
        }
    }
    
    return false;
}

bool LicenseValidator::IsValid() const
{
    return m_isValid;
}

LicenseInfo LicenseValidator::GetLicenseInfo() const
{
    return m_licenseInfo;
}

int LicenseValidator::GetDaysRemaining() const
{
    if (!m_isValid)
        return 0;
        
    // Parse date string (assuming format: YYYY-MM-DD)
    std::tm tm = {};
    std::istringstream ss(m_licenseInfo.validadeStr);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    
    auto expiry = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    auto now = std::chrono::system_clock::now();
    
    auto diff = std::chrono::duration_cast<std::chrono::hours>(expiry - now);
    return static_cast<int>(diff.count() / 24);
}

bool LicenseValidator::ValidateLicenseFile(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
        return false;
        
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    if (content.empty())
        return false;
    
    try
    {
        // For this implementation, we'll do a simple validation
        // In a real scenario, you would implement proper AES decryption
        
        // Mock license data for demonstration
        m_licenseInfo.usuario = "cliente123";
        m_licenseInfo.validadeStr = "2025-08-01";
        m_licenseInfo.quantidadePCs = 2;
        
        // Check HWID (simplified)
        std::string currentHWID = GetHWID();
        m_licenseInfo.fingerprints.push_back(currentHWID);
        
        return true;
    }
    catch (...)
    {
        return false;
    }
}

std::string LicenseValidator::DecryptAES(const std::string& encryptedText)
{
    // Simplified implementation - in real scenario, implement proper AES decryption
    return encryptedText;
}

bool LicenseValidator::VerifySignature(const LicenseInfo& info)
{
    // Simplified implementation - in real scenario, implement HMAC verification
    return true;
}

std::string LicenseValidator::GetHWID()
{
    // Simplified HWID generation
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computerName);
    GetComputerNameA(computerName, &size);
    
    return std::string(computerName) + "_HWID";
}