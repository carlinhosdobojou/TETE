using System;
using System.IO;
using System.Security.Cryptography;
using System.Text;
using Newtonsoft.Json;
using System.Management;
using System.Linq;

namespace MuInject
{
    public class LicenseInfo
    {
        public string Usuario { get; set; } = "";
        public string Senha { get; set; } = "";
        public DateTime Validade { get; set; }
        public int QuantidadePCs { get; set; }
        public string[] Fingerprints { get; set; } = Array.Empty<string>();
        public string Assinatura { get; set; } = "";
    }

    public class LicenseManager
    {
        private readonly string secretKey = "MuBot2025SecretKey!@#$%^&*()_+{}|:<>?";
        private readonly string hmacKey = "MuBotHMACKey2025!@#$%^&*()_+";

        public LicenseInfo? ValidateLicense(string filePath)
        {
            try
            {
                if (!File.Exists(filePath))
                    return null;

                string encryptedContent = File.ReadAllText(filePath);
                string decryptedContent = DecryptAES(encryptedContent);
                
                var licenseInfo = JsonConvert.DeserializeObject<LicenseInfo>(decryptedContent);
                if (licenseInfo == null)
                    return null;

                // Verify signature
                if (!VerifySignature(licenseInfo))
                    return null;

                // Check expiration
                if (DateTime.Now > licenseInfo.Validade)
                    return null;

                // Check HWID
                string currentHWID = GetHWID();
                if (!licenseInfo.Fingerprints.Contains(currentHWID))
                    return null;

                return licenseInfo;
            }
            catch
            {
                return null;
            }
        }

        private string DecryptAES(string encryptedText)
        {
            byte[] fullCipher = Convert.FromBase64String(encryptedText);
            byte[] iv = new byte[16];
            byte[] cipher = new byte[fullCipher.Length - 16];

            Array.Copy(fullCipher, 0, iv, 0, 16);
            Array.Copy(fullCipher, 16, cipher, 0, cipher.Length);

            using (var aes = Aes.Create())
            {
                aes.Key = Encoding.UTF8.GetBytes(secretKey.Substring(0, 32));
                aes.IV = iv;
                aes.Mode = CipherMode.CBC;
                aes.Padding = PaddingMode.PKCS7;

                using (var decryptor = aes.CreateDecryptor())
                using (var msDecrypt = new MemoryStream(cipher))
                using (var csDecrypt = new CryptoStream(msDecrypt, decryptor, CryptoStreamMode.Read))
                using (var srDecrypt = new StreamReader(csDecrypt))
                {
                    return srDecrypt.ReadToEnd();
                }
            }
        }

        private bool VerifySignature(LicenseInfo licenseInfo)
        {
            try
            {
                string dataToSign = $"{licenseInfo.Usuario}{licenseInfo.Validade:yyyy-MM-dd}{licenseInfo.QuantidadePCs}";
                
                using (var hmac = new HMACSHA256(Encoding.UTF8.GetBytes(hmacKey)))
                {
                    byte[] computedHash = hmac.ComputeHash(Encoding.UTF8.GetBytes(dataToSign));
                    string computedSignature = Convert.ToBase64String(computedHash);
                    
                    return computedSignature == licenseInfo.Assinatura;
                }
            }
            catch
            {
                return false;
            }
        }

        private string GetHWID()
        {
            try
            {
                string cpu = "";
                string motherboard = "";
                string disk = "";

                // Get CPU ID
                using (var searcher = new ManagementObjectSearcher("SELECT ProcessorId FROM Win32_Processor"))
                {
                    foreach (ManagementObject obj in searcher.Get())
                    {
                        cpu = obj["ProcessorId"]?.ToString() ?? "";
                        break;
                    }
                }

                // Get Motherboard Serial
                using (var searcher = new ManagementObjectSearcher("SELECT SerialNumber FROM Win32_BaseBoard"))
                {
                    foreach (ManagementObject obj in searcher.Get())
                    {
                        motherboard = obj["SerialNumber"]?.ToString() ?? "";
                        break;
                    }
                }

                // Get Disk Serial
                using (var searcher = new ManagementObjectSearcher("SELECT SerialNumber FROM Win32_PhysicalMedia"))
                {
                    foreach (ManagementObject obj in searcher.Get())
                    {
                        disk = obj["SerialNumber"]?.ToString()?.Trim() ?? "";
                        if (!string.IsNullOrEmpty(disk))
                            break;
                    }
                }

                string combined = $"{cpu}{motherboard}{disk}";
                
                using (var sha256 = SHA256.Create())
                {
                    byte[] hash = sha256.ComputeHash(Encoding.UTF8.GetBytes(combined));
                    return Convert.ToBase64String(hash).Substring(0, 16);
                }
            }
            catch
            {
                return "DefaultHWID123456";
            }
        }
    }
}