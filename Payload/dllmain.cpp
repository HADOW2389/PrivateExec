#include "Offsets.hpp"
#include "LuaEngine.hpp"
#include "PipeServer.hpp"
#include "UNC.hpp"
#include <Windows.h>
#include <thread>

// DLL entry point — called by the manual mapper loader shellcode.
// Everything runs asynchronously to avoid blocking the loader thread.

static void MainThread() {
    // Small delay to let Roblox finish loading its modules
    Sleep(2500);

    OutputDebugStringA("[Exec] DLL loaded, resolving offsets...");

    if (!LuaOffsets::Init()) {
        OutputDebugStringA("[Exec] Offset resolution failed — version mismatch?");
        // Don't bail; pipe server still starts so the UI shows connected
    }

    if (!LuaEngine::Init()) {
        OutputDebugStringA("[Exec] LuaEngine init failed");
    }

    // Start named pipe server — ready to receive scripts from the bridge
    PipeServer::Start();

    OutputDebugStringA("[Exec] Ready.");

    // Keep main thread alive — PipeServer runs on its own thread
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