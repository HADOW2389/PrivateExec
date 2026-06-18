#include "Offsets.hpp"
#include "LuaEngine.hpp"
#include "PipeServer.hpp"
#include "UNC.hpp"
#include <Windows.h>
#include <thread>

// DLL entry point — called by the manual mapper loader shellcode.
// Everything runs asynchronously to avoid blocking the loader thread.

static void MainThread() {
    // Start the pipe server FIRST so the UI can connect immediately.
    // Scripts are queued and executed once LuaEngine is ready.
    PipeServer::Start();
    OutputDebugStringA("[Exec] Pipe server started.");

    // Let Roblox finish loading its modules before touching Lua state
    Sleep(2500);

    // LuaEngine::Init() runs LuaOffsets::Init() internally and writes the log
    OutputDebugStringA("[Exec] Initialising Lua engine...");
    if (!LuaEngine::Init())
        OutputDebugStringA("[Exec] LuaEngine init failed");

    OutputDebugStringA("[Exec] Ready.");

    while (true) Sleep(5000);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        // Spawn initialisation on a fresh thread to avoid loader-lock deadlock
        HANDLE h = CreateThread(nullptr, 0,
            [](LPVOID) -> DWORD { MainThread(); return 0; },
            nullptr, 0, nullptr);
        if (h) CloseHandle(h);
    }
    return TRUE;
}