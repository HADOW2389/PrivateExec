#pragma once
#include "LuaEngine.hpp"
#include <Windows.h>
#include <string>
#include <atomic>

// Script watcher — polls a temp file instead of named pipes.
// Hyperion can block CreateNamedPipe; plain file I/O is never filtered.
// Protocol: UI writes Base64(script) to %TEMP%\PrivateExec_<pid>.b64
//           DLL reads, deletes, decodes, pushes to LuaEngine queue.

namespace PipeServer {

    inline std::atomic<bool> g_running{ false };
    inline std::string       g_cmdFile;

    static std::string B64Decode(const std::string& enc) {
        static const char tbl[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        int T[256];
        for (auto& v : T) v = -1;
        for (int i = 0; i < 64; ++i) T[(unsigned char)tbl[i]] = i;

        std::string out;
        int val = 0, bits = -8;
        for (unsigned char c : enc) {
            if (T[c] == -1) continue;
            val = (val << 6) + T[c];
            bits += 6;
            if (bits >= 0) {
                out.push_back(static_cast<char>((val >> bits) & 0xFF));
                bits -= 8;
            }
        }
        return out;
    }

    static DWORD WINAPI WatchThread(LPVOID) {
        while (g_running) {
            Sleep(300);

            // Try to open and read the command file
            HANDLE hFile = CreateFileA(
                g_cmdFile.c_str(),
                GENERIC_READ,
                0,          // exclusive — prevents partial reads
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                nullptr);

            if (hFile == INVALID_HANDLE_VALUE) continue;

            DWORD sz = GetFileSize(hFile, nullptr);
            std::string data;
            if (sz && sz != INVALID_FILE_SIZE) {
                data.resize(sz);
                DWORD rd = 0;
                ReadFile(hFile, data.data(), sz, &rd, nullptr);
                data.resize(rd);
            }
            CloseHandle(hFile);

            // Delete command file so we don't re-execute
            DeleteFileA(g_cmdFile.c_str());

            if (data.empty()) continue;

            std::string script = B64Decode(data);
            if (script.rfind("setworkspacefolder: ", 0) == 0) {
                LuaEngine::g_workspaceFolder = script.substr(20);
                continue;
            }

            LuaEngine::PushScript(std::move(script));
        }
        return 0;
    }

    inline void Start() {
        DWORD pid = GetCurrentProcessId();
        char tmp[MAX_PATH]{};
        GetTempPathA(MAX_PATH, tmp);
        g_cmdFile = std::string(tmp) + "PrivateExec_" + std::to_string(pid) + ".b64";
        g_running = true;

        HANDLE h = CreateThread(nullptr, 0, WatchThread, nullptr, 0, nullptr);
        if (h) CloseHandle(h);

        OutputDebugStringA(("[ScriptWatcher] Watching: " + g_cmdFile).c_str());
    }

    inline void Stop() { g_running = false; }

} // namespace PipeServer