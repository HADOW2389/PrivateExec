#pragma once
#include "LuaEngine.hpp"
#include <Windows.h>
#include <string>
#include <thread>
#include <atomic>

// Named pipe server — receives Base64-encoded Lua scripts from the C# bridge.
// Pipe name: \\.\pipe\PrivateExec_{pid}

namespace PipeServer {

    inline std::string g_pipeName;
    inline std::atomic<bool> g_running{ false };
    inline std::thread g_thread;

    // Base64 decoder
    static std::string B64Decode(const std::string& enc) {
        static const std::string table =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;
        std::vector<int> T(256, -1);
        for (int i = 0; i < 64; ++i) T[table[i]] = i;

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

    static void ServeOnePipe() {
        HANDLE hPipe = CreateNamedPipeA(
            g_pipeName.c_str(),
            PIPE_ACCESS_INBOUND,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            1,          // max instances
            0,          // out buffer
            1 << 20,    // in buffer 1 MB
            0,
            nullptr);

        if (hPipe == INVALID_HANDLE_VALUE) return;

        if (!ConnectNamedPipe(hPipe, nullptr) &&
            GetLastError() != ERROR_PIPE_CONNECTED) {
            CloseHandle(hPipe);
            return;
        }

        std::string data;
        char buf[4096];
        DWORD read = 0;
        while (ReadFile(hPipe, buf, sizeof(buf), &read, nullptr) && read)
            data.append(buf, read);

        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);

        if (data.empty()) return;

        std::string script = B64Decode(data);

        // Special commands (same protocol as Velocity for UI compatibility)
        if (script.rfind("setworkspacefolder: ", 0) == 0) {
            LuaEngine::g_workspaceFolder = script.substr(20);
            return;
        }

        LuaEngine::PushScript(std::move(script));
    }

    static void ServerLoop() {
        while (g_running) {
            ServeOnePipe(); // blocks until a client connects and disconnects
        }
    }

    inline void Start() {
        DWORD pid = GetCurrentProcessId();
        g_pipeName = "\\\\.\\pipe\\PrivateExec_" + std::to_string(pid);
        g_running  = true;
        g_thread   = std::thread(ServerLoop);
        g_thread.detach();
        OutputDebugStringA(("[PipeServer] Listening on " + g_pipeName).c_str());
    }

    inline void Stop() {
        g_running = false;
        // Unblock the blocking ConnectNamedPipe by connecting ourselves
        HANDLE h = CreateFileA(g_pipeName.c_str(), GENERIC_WRITE, 0,
                               nullptr, OPEN_EXISTING, 0, nullptr);
        if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
    }

} // namespace PipeServer