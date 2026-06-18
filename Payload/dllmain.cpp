#include "Offsets.hpp"
#include "LuaEngine.hpp"
#include "PipeServer.hpp"
#include "UNC.hpp"
#include <Windows.h>
#include <thread>

// Write a one-line checkpoint to %TEMP%\PrivateExec_trace.txt (append mode).
static void Trace(const char* msg) {
    char tmp[MAX_PATH]{};
    GetTempPathA(MAX_PATH, tmp);
    char path[MAX_PATH]{};
    sprintf_s(path, "%sPrivateExec_trace.txt", tmp);
    HANDLE hf = CreateFileA(path, GENERIC_WRITE | FILE_APPEND_DATA, FILE_SHARE_READ,
                            nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hf != INVALID_HANDLE_VALUE) {
        DWORD w; WriteFile(hf, msg, (DWORD)strlen(msg), &w, nullptr);
        CloseHandle(hf);
    }
    OutputDebugStringA(msg);
}

static void MainThread() {
    Trace("[1] MainThread start\n");

    PipeServer::Start();
    Trace("[2] PipeServer started\n");

    Sleep(2500);
    Trace("[3] Sleep done\n");

    if (!LuaEngine::Init())
        Trace("[4] LuaEngine init FAILED\n");
    else
        Trace("[4] LuaEngine init OK\n");

    Trace("[5] Ready\n");
    while (true) Sleep(5000);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        // Checkpoint: DllMain reached — write before anything else
        char tmp[MAX_PATH]{};
        GetTempPathA(MAX_PATH, tmp);
        char path[MAX_PATH]{};
        sprintf_s(path, "%sPrivateExec_trace.txt", tmp);
        // Truncate/create on first attach
        HANDLE hf = CreateFileA(path, GENERIC_WRITE, 0, nullptr,
                                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hf != INVALID_HANDLE_VALUE) {
            const char* m = "[0] DllMain reached\n";
            DWORD w; WriteFile(hf, m, (DWORD)strlen(m), &w, nullptr);
            CloseHandle(hf);
        }

        DisableThreadLibraryCalls(hinstDLL);
        HANDLE h = CreateThread(nullptr, 0,
            [](LPVOID) -> DWORD { MainThread(); return 0; },
            nullptr, 0, nullptr);
        if (h) {
            CloseHandle(h);
        } else {
            // CreateThread failed — log error code
            char err[64]{};
            sprintf_s(err, "[0] CreateThread FAILED err=%lu\n", GetLastError());
            HANDLE hf2 = CreateFileA(path, GENERIC_WRITE | FILE_APPEND_DATA,
                                     FILE_SHARE_READ, nullptr, OPEN_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL, nullptr);
            if (hf2 != INVALID_HANDLE_VALUE) {
                DWORD w; WriteFile(hf2, err, (DWORD)strlen(err), &w, nullptr);
                CloseHandle(hf2);
            }
        }
    }
    return TRUE;
}