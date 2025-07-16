# MuBot - Bot Multifuncional para MU Online

## Visão Geral

O MuBot é um bot completo para automação do jogo MU Online, desenvolvido com base na estrutura do Universal-OpenGL-2-Kiero-Hook. Inclui funcionalidades PvE e PvP, sistema de licenciamento offline, overlay in-game com ImGui, e injetor próprio com segurança reforçada.

## Estrutura do Projeto

### MuInject (C# .NET 8)
- **Arquivo de Solução**: `MuInject.sln`
- **Injetor com design dark** estilo MU Online
- **Sistema de licenciamento offline** com criptografia AES-256
- **Verificação de HWID** para proteção contra pirataria
- **Interface moderna** em WinForms com tema preto/vermelho

### syslib.dll (C++)
- **Arquivo de Solução**: `syslib.sln`
- **DLL injetável** baseada no Universal-OpenGL-2-Kiero-Hook
- **Menu in-game** com ImGui customizado
- **Sistemas PvE e PvP** completos
- **Sistema de aprendizado** para coleta de dados do jogo

## Funcionalidades Principais

### Menu In-Game (F5)
- **Abertura**: Pressione F5 para abrir/minimizar o menu
- **Navegação**: Setas direcionais para navegar
- **Estados**: Menu principal, PvE, PvP, Licença
- **Modo compacto**: F5 novamente para alternar tamanho

### Sistema PvE
- ✅ Auto farm com detecção de monstros
- ✅ Auto reset configurável por nível
- ✅ Coleta automática de itens (ALT)
- ✅ Auto pot para vida e mana
- ✅ Sistema de skills com delay configurável
- ✅ Configuração de teclas personalizadas

### Sistema PvP
- ✅ Alvo automático para jogadores próximos
- ✅ Sistema de combos configurável
- ✅ Auto shield baseado em % de vida
- ✅ Estatísticas de PvP (dano, vitórias, mortes)
- ✅ Overlay do alvo com informações
- ✅ Aprendizado automático de combos

### Sistema de Licenciamento
- 🔒 **Criptografia AES-256** com IV dinâmico
- 🔒 **HMAC SHA256** para verificação de integridade
- 🔒 **HWID único** por dispositivo
- 🔒 **Limite de PCs** configurável
- 🔒 **Validação offline** sem necessidade de internet
- 🔒 **Proteção anti-tamper** contra edição manual

### Sistema de Aprendizado
- 📊 **Coleta de dados** em tempo real do jogo
- 📊 **Análise de cores** para vida/mana
- 📊 **OCR** para nomes e itens
- 📊 **Pixel scanning** para eventos do jogo
- 📊 **Exportação** em TXT e JSON
- 📊 **Logs detalhados** para análise posterior

## Como Usar

### 1. Compilação
```bash
# Compilar o injetor (C#)
cd MuInject
dotnet build -c Release

# Compilar a DLL (C++)
# Abrir syslib.sln no Visual Studio e compilar
```

### 2. Preparação da Licença
1. Criar arquivo `app.licenca` com licença válida
2. Colocar na pasta do MuInject.exe

### 3. Injeção
1. Executar `MuInject.exe`
2. Importar licença (se necessário)
3. Selecionar processo `Main.exe` do MU
4. Clicar em **INJETAR**

### 4. Uso no Jogo
1. Pressionar **F5** para abrir o menu
2. Navegar com setas direcionais
3. Configurar PvE ou PvP conforme necessário
4. Ativar modo aprendizado para coleta de dados

## Segurança

### Proteções Implementadas
- ✅ Apenas MuInject pode injetar syslib.dll
- ✅ Verificação de processo pai na DLL
- ✅ Licença criptografada com AES-256
- ✅ HWID único por máquina
- ✅ Verificação de integridade HMAC
- ✅ Nome stealth para DLL (syslib.dll)

### Anti-Detecção
- ✅ Hook OpenGL2 de baixo nível
- ✅ Injeção via LoadLibrary padrão
- ✅ Sem modificação de arquivos do jogo
- ✅ Overlay transparente e removível

## Estrutura de Arquivos

```
MuBot/
├── MuInject/                 # Injetor C#
│   ├── MuInject.csproj
│   ├── Program.cs
│   ├── MainForm.cs
│   └── LicenseManager.cs
├── syslib/                   # DLL C++
│   ├── syslib.vcxproj
│   ├── dllmain.cpp
│   ├── mubot.h/.cpp
│   ├── pve_system.h/.cpp
│   ├── pvp_system.h/.cpp
│   ├── learning_system.h/.cpp
│   ├── game_reader.h/.cpp
│   ├── license_validator.h/.cpp
│   └── external/             # ImGui + Kiero
├── MuInject.sln             # Solução C#
├── syslib.sln               # Solução C++
└── README.md
```

## Requisitos

### Sistema
- Windows 10/11 (x64)
- .NET 8.0 Runtime
- Visual C++ Redistributable 2022
- OpenGL 2.0+ support

### Desenvolvimento
- Visual Studio 2022
- .NET 8.0 SDK
- Windows SDK 10.0

## Licença

Este projeto é fornecido apenas para fins educacionais. O uso em servidores oficiais pode violar os termos de serviço do jogo.

---

**By CarlosNTC** - MuBot v1.0