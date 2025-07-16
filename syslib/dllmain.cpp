#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "mubot.h"
#include "imgui_hook.h"

static HMODULE g_hModule = nullptr;
static bool g_isDetached = false;

DWORD WINAPI MainThread(LPVOID lpThreadParameter)
{
    g_hModule = (HMODULE)lpThreadParameter;
    MuBot::Initialize();
    return S_OK;
}

DWORD WINAPI ExitThread(LPVOID lpThreadParameter)
{
    if (!g_isDetached)
    {
        g_isDetached = true;
        MuBot::Shutdown();
        ImGuiHook::Unload();
        FreeLibraryAndExitThread(g_hModule, TRUE);
    }
    return S_OK;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hMod);
        CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        CreateThread(nullptr, 0, ExitThread, nullptr, 0, nullptr);
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}