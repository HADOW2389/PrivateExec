#include "Syscalls.hpp"
#include "ManualMapper.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <TlHelp32.h>

// ── Entrypoint ───────────────────────────────────────────────────────────────
// Usage: Injector.exe <pid> <path_to_dll>
// Writes a log to %TEMP%\PrivateExec_inject.log so the UI can surface errors.

static std::ofstream g_log;

static void Log(const std::string& msg) {
    std::cout << msg << "\n";
    if (g_log.is_open()) g_log << msg << "\n" << std::flush;
}

static void OpenLog() {
    char tmp[MAX_PATH]{};
    GetTempPathA(MAX_PATH, tmp);
    std::string path = std::string(tmp) + "PrivateExec_inject.log";
    g_log.open(path, std::ios::trunc);
}

static DWORD FindRobloxPid(DWORD hintPid) {
    if (hintPid) return hintPid;
    // Fallback: find RobloxPlayerBeta.exe
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32W pe{ sizeof(pe) };
    if (Process32FirstW(snap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, L"RobloxPlayerBeta.exe") == 0) {
                CloseHandle(snap);
                return pe.th32ProcessID;
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return 0;
}

int main(int argc, char* argv[]) {
    OpenLog();
    Log("[Injector] Start");

    if (argc < 3) {
        Log("[Injector] ERROR: wrong arg count");
        return 1;
    }

    DWORD pid     = static_cast<DWORD>(std::stoul(argv[1]));
    std::string dllPath = argv[2];
    Log(std::string("[Injector] pid=") + std::to_string(pid) + " dll=" + dllPath);

    pid = FindRobloxPid(pid);
    if (!pid) { Log("[Injector] ERROR: process not found"); return 2; }
    Log(std::string("[Injector] Using pid=") + std::to_string(pid));

    // 1. Resolve SSNs from the in-memory ntdll.
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll) { Log("[Injector] ERROR: ntdll not loaded"); return 3; }
    Log("[Injector] Resolving syscalls...");
    if (!Syscall::Resolve(hNtdll)) {
        Log("[Injector] ERROR: SSN resolution failed");
        return 3;
    }
    Log("[Injector] Syscalls OK");

    // 2. Read DLL from disk
    std::ifstream file(dllPath, std::ios::binary | std::ios::ate);
    if (!file) { Log("[Injector] ERROR: DLL not found: " + dllPath); return 4; }
    auto size = file.tellg(); file.seekg(0);
    std::vector<BYTE> dllBuf(static_cast<size_t>(size));
    file.read(reinterpret_cast<char*>(dllBuf.data()), size);
    file.close();
    Log(std::string("[Injector] DLL read: ") + std::to_string(size) + " bytes");

    // 3. Open target process via indirect syscall
    HANDLE hProcess = nullptr;
    OBJECT_ATTRIBUTES oa{sizeof(oa)};
    CLIENT_ID cid{ reinterpret_cast<HANDLE>(static_cast<uintptr_t>(pid)), nullptr };
    constexpr DWORD NEEDED =
        PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ |
        PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD;
    NTSTATUS st = IndirectNtOpenProcess(&hProcess, NEEDED, &oa, &cid);
    if (!NT_SUCCESS(st) || !hProcess) {
        std::ostringstream oss;
        oss << "[Injector] ERROR: NtOpenProcess NTSTATUS=0x" << std::hex << st;
        Log(oss.str());
        return 5;
    }
    Log("[Injector] Process opened OK");

    // 4. Manual map
    Log("[Injector] Mapping DLL...");
    bool ok = ManualMap::Map(hProcess, dllBuf);
    CloseHandle(hProcess);
    Log(ok ? "[Injector] Map OK" : "[Injector] ERROR: Map FAILED");
    return ok ? 0 : 6;
}