#include "Syscalls.hpp"
#include "ManualMapper.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <TlHelp32.h>

// ── Entrypoint ───────────────────────────────────────────────────────────────
// Usage: Injector.exe <pid> <path_to_dll>
// Called by the C# bridge with the Roblox PID.

static HMODULE LoadCleanNtdll() {
    // Load a clean copy of ntdll from disk — bypasses any AV hooks in the
    // in-memory ntdll by reading from KnownDlls or the file directly.
    wchar_t sysdir[MAX_PATH]{};
    GetSystemDirectoryW(sysdir, MAX_PATH);
    std::wstring path = std::wstring(sysdir) + L"\\ntdll.dll";

    // Map as image via file I/O — intentionally not using LoadLibrary so
    // the second ntdll mapping is invisible to most integrity scanners.
    HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                               nullptr, OPEN_EXISTING, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return nullptr;

    DWORD sz = GetFileSize(hFile, nullptr);
    std::vector<BYTE> buf(sz);
    DWORD read = 0;
    ReadFile(hFile, buf.data(), sz, &read, nullptr);
    CloseHandle(hFile);

    // Map as data — we only need exports to resolve SSNs
    return reinterpret_cast<HMODULE>(buf.data()); // lifetime == buf scope
    // NOTE: caller must keep buf alive while Resolve() runs
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
    if (argc < 3) {
        std::cerr << "Usage: Injector.exe <pid> <dll_path>\n";
        return 1;
    }

    DWORD pid     = static_cast<DWORD>(std::stoul(argv[1]));
    std::string dllPath = argv[2];

    pid = FindRobloxPid(pid);
    if (!pid) { std::cerr << "Process not found\n"; return 2; }

    // 1. Load clean ntdll and resolve SSNs
    wchar_t sysdir[MAX_PATH]{};
    GetSystemDirectoryW(sysdir, MAX_PATH);
    std::wstring ntdllPath = std::wstring(sysdir) + L"\\ntdll.dll";

    std::vector<BYTE> ntdllBuf;
    {
        HANDLE hFile = CreateFileW(ntdllPath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                   nullptr, OPEN_EXISTING, 0, nullptr);
        DWORD sz = GetFileSize(hFile, nullptr);
        ntdllBuf.resize(sz);
        DWORD r = 0; ReadFile(hFile, ntdllBuf.data(), sz, &r, nullptr);
        CloseHandle(hFile);
    }
    HMODULE hNtdll = reinterpret_cast<HMODULE>(ntdllBuf.data());
    if (!Syscall::Resolve(hNtdll)) {
        std::cerr << "SSN resolution failed\n"; return 3;
    }

    // 2. Read DLL from disk
    std::ifstream file(dllPath, std::ios::binary | std::ios::ate);
    if (!file) { std::cerr << "DLL not found: " << dllPath << "\n"; return 4; }
    auto size = file.tellg(); file.seekg(0);
    std::vector<BYTE> dllBuf(static_cast<size_t>(size));
    file.read(reinterpret_cast<char*>(dllBuf.data()), size);
    file.close();

    // 3. Open target process via indirect syscall
    HANDLE hProcess = nullptr;
    OBJECT_ATTRIBUTES oa{sizeof(oa)};
    CLIENT_ID cid{ reinterpret_cast<HANDLE>(static_cast<uintptr_t>(pid)), nullptr };
    NTSTATUS st = IndirectNtOpenProcess(&hProcess, PROCESS_ALL_ACCESS, &oa, &cid);
    if (!NT_SUCCESS(st) || !hProcess) {
        std::cerr << "NtOpenProcess failed: " << std::hex << st << "\n"; return 5;
    }

    // 4. Manual map
    bool ok = ManualMap::Map(hProcess, dllBuf);
    CloseHandle(hProcess);

    std::cout << (ok ? "OK" : "FAIL") << "\n";
    return ok ? 0 : 6;
}