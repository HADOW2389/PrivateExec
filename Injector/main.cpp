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

    // 1. Resolve SSNs from the in-memory ntdll.
    // SSNs are intact even in hooked stubs — hooks jump before the SSN is read.
    // The in-memory mapping has correct virtual addresses, unlike a raw file buffer.
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll) { std::cerr << "ntdll not found\n"; return 3; }
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
    // PROCESS_ALL_ACCESS is often blocked; request only what mapping needs
    constexpr DWORD NEEDED =
        PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ |
        PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD;
    NTSTATUS st = IndirectNtOpenProcess(&hProcess, NEEDED, &oa, &cid);
    if (!NT_SUCCESS(st) || !hProcess) {
        std::cerr << "NtOpenProcess failed: " << std::hex << st << "\n"; return 5;
    }

    // 4. Manual map
    bool ok = ManualMap::Map(hProcess, dllBuf);
    CloseHandle(hProcess);

    std::cout << (ok ? "OK" : "FAIL") << "\n";
    return ok ? 0 : 6;
}