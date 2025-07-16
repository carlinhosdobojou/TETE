#pragma once

#include <string>
#include <vector>

struct LicenseInfo
{
    std::string usuario;
    std::string senha;
    std::string validadeStr;
    int quantidadePCs;
    std::vector<std::string> fingerprints;
    std::string assinatura;
};

class LicenseValidator
{
public:
    bool Initialize();
    bool IsValid() const;
    LicenseInfo GetLicenseInfo() const;
    int GetDaysRemaining() const;

private:
    bool ValidateLicenseFile(const std::string& filePath);
    std::string DecryptAES(const std::string& encryptedText);
    bool VerifySignature(const LicenseInfo& info);
    std::string GetHWID();
    
    bool m_isValid = false;
    LicenseInfo m_licenseInfo;
};