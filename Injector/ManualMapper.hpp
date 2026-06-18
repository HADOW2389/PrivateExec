#pragma once
#include "Syscalls.hpp"
#include <string>
#include <vector>

// Manual-map loader: maps a DLL into a remote process without LdrLoadDll.
// No PEB module list entry — invisible to Byfron module enumeration.

namespace ManualMap {

    struct LoaderData {
        PVOID  imageBase;
        DWORD  entryPointRva;
        BOOL   hasTls;
        DWORD  tlsCallbacksRva;
        BYTE   _pad[4];
    };

    // ── Helper: RVA → raw file offset ────────────────────────────────────────
    // MUST be defined before Map() which calls it.
    inline DWORD RvaToOffset(const IMAGE_NT_HEADERS64* nt, DWORD rva) {
        auto sec = IMAGE_FIRST_SECTION(nt);
        for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++sec) {
            if (rva >= sec->VirtualAddress &&
                rva <  sec->VirtualAddress + sec->Misc.VirtualSize)
                return sec->PointerToRawData + (rva - sec->VirtualAddress);
        }
        return rva; // falls back for header-range RVAs
    }

    // ── Shellcode executed inside the target process ──────────────────────────
    // Calls TLS callbacks, then DllMain(DLL_PROCESS_ATTACH).
    // Must be position-independent (no external calls except through data).
    __declspec(noinline) static void LoaderShellcode(LoaderData* d) {
        if (d->hasTls) {
            using TlsCb = void(NTAPI*)(PVOID, DWORD, PVOID);
            auto cbs = reinterpret_cast<TlsCb*>(
                reinterpret_cast<uintptr_t>(d->imageBase) + d->tlsCallbacksRva);
            while (*cbs) {
                (*cbs)(d->imageBase, DLL_PROCESS_ATTACH, nullptr);
                ++cbs;
            }
        }
        using DllEntry = BOOL(WINAPI*)(HINSTANCE, DWORD, LPVOID);
        auto entry = reinterpret_cast<DllEntry>(
            reinterpret_cast<uintptr_t>(d->imageBase) + d->entryPointRva);
        entry(reinterpret_cast<HINSTANCE>(d->imageBase), DLL_PROCESS_ATTACH, nullptr);
    }
    static void LoaderShellcodeEnd() {} // size marker

    // ── Core mapping ──────────────────────────────────────────────────────────
    inline bool Map(HANDLE hProcess, const std::vector<BYTE>& raw) {
        if (raw.size() < sizeof(IMAGE_DOS_HEADER)) return false;

        auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(raw.data());
        if (dos->e_magic != IMAGE_DOS_SIGNATURE) return false;

        auto nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(raw.data() + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE) return false;
        if (nt->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) return false;

        SIZE_T imgSize = nt->OptionalHeader.SizeOfImage;
        PVOID  imgBase = nullptr;
        SIZE_T written = 0;

        // 1. Allocate image memory in target process
        NTSTATUS st = IndirectNtAllocateVirtualMemory(
            hProcess, &imgBase, 0, &imgSize,
            MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!NT_SUCCESS(st)) return false;

        // 2. Write PE headers
        IndirectNtWriteVirtualMemory(
            hProcess, imgBase,
            const_cast<BYTE*>(raw.data()),
            nt->OptionalHeader.SizeOfHeaders, &written);

        // 3. Map sections
        {
            auto sec = IMAGE_FIRST_SECTION(nt);
            for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++sec) {
                if (!sec->SizeOfRawData) continue;
                PVOID  dst  = reinterpret_cast<PVOID>(
                    reinterpret_cast<uintptr_t>(imgBase) + sec->VirtualAddress);
                PBYTE  src  = const_cast<BYTE*>(raw.data()) + sec->PointerToRawData;
                SIZE_T size = sec->SizeOfRawData;
                IndirectNtWriteVirtualMemory(hProcess, dst, src, size, &written);
            }
        }

        // 4. Apply base relocations
        uintptr_t delta = reinterpret_cast<uintptr_t>(imgBase)
                        - nt->OptionalHeader.ImageBase;

        auto& relocDD = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
        if (delta && relocDD.Size) {
            auto block = reinterpret_cast<const IMAGE_BASE_RELOCATION*>(
                raw.data() + RvaToOffset(nt, relocDD.VirtualAddress));
            SIZE_T remaining = relocDD.Size;

            while (remaining > sizeof(IMAGE_BASE_RELOCATION) && block->SizeOfBlock > 0) {
                DWORD entryCount = (block->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
                auto  entries    = reinterpret_cast<const WORD*>(block + 1);

                for (DWORD j = 0; j < entryCount; ++j) {
                    if ((entries[j] >> 12) != IMAGE_REL_BASED_DIR64) continue;
                    DWORD     rva = block->VirtualAddress + (entries[j] & 0xFFF);
                    PVOID     ptr = reinterpret_cast<PVOID>(
                        reinterpret_cast<uintptr_t>(imgBase) + rva);
                    uintptr_t val = 0;
                    ReadProcessMemory(hProcess, ptr, &val, sizeof(val), nullptr);
                    val += delta;
                    IndirectNtWriteVirtualMemory(hProcess, ptr, &val, sizeof(val), &written);
                }
                remaining -= block->SizeOfBlock;
                block = reinterpret_cast<const IMAGE_BASE_RELOCATION*>(
                    reinterpret_cast<const BYTE*>(block) + block->SizeOfBlock);
            }
        }

        // 5. Resolve IAT
        auto& importDD = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
        if (importDD.Size) {
            auto desc = reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR*>(
                raw.data() + RvaToOffset(nt, importDD.VirtualAddress));

            while (desc->Name) {
                const char* dllName = reinterpret_cast<const char*>(
                    raw.data() + RvaToOffset(nt, desc->Name));
                HMODULE hMod = LoadLibraryA(dllName);

                auto origThunk = reinterpret_cast<const IMAGE_THUNK_DATA64*>(
                    raw.data() + RvaToOffset(nt, desc->OriginalFirstThunk
                        ? desc->OriginalFirstThunk : desc->FirstThunk));

                for (SIZE_T k = 0; origThunk->u1.AddressOfData; ++origThunk, ++k) {
                    uintptr_t funcAddr = 0;
                    if (IMAGE_SNAP_BY_ORDINAL64(origThunk->u1.Ordinal)) {
                        funcAddr = reinterpret_cast<uintptr_t>(
                            GetProcAddress(hMod,
                                MAKEINTRESOURCEA(IMAGE_ORDINAL64(origThunk->u1.Ordinal))));
                    } else {
                        auto byName = reinterpret_cast<const IMAGE_IMPORT_BY_NAME*>(
                            raw.data() + RvaToOffset(nt,
                                static_cast<DWORD>(origThunk->u1.AddressOfData)));
                        funcAddr = reinterpret_cast<uintptr_t>(
                            GetProcAddress(hMod, byName->Name));
                    }
                    if (funcAddr) {
                        PVOID iatSlot = reinterpret_cast<PVOID>(
                            reinterpret_cast<uintptr_t>(imgBase)
                            + desc->FirstThunk
                            + k * sizeof(uintptr_t));
                        IndirectNtWriteVirtualMemory(hProcess, iatSlot,
                            &funcAddr, sizeof(funcAddr), &written);
                    }
                }
                ++desc;
            }
        }

        // 6. Write and launch shellcode (TLS + DllMain)
        SIZE_T shellSize = reinterpret_cast<uintptr_t>(LoaderShellcodeEnd)
                         - reinterpret_cast<uintptr_t>(LoaderShellcode);
        shellSize += 0x100; // safety padding

        SIZE_T allocSize = shellSize + sizeof(LoaderData);
        PVOID  shellMem  = nullptr;
        IndirectNtAllocateVirtualMemory(
            hProcess, &shellMem, 0, &allocSize,
            MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!shellMem) return false;

        LoaderData ld{};
        ld.imageBase     = imgBase;
        ld.entryPointRva = nt->OptionalHeader.AddressOfEntryPoint;

        auto& tlsDD = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
        if (tlsDD.Size) {
            auto tlsDir = reinterpret_cast<const IMAGE_TLS_DIRECTORY64*>(
                raw.data() + RvaToOffset(nt, tlsDD.VirtualAddress));
            ld.hasTls = TRUE;
            ld.tlsCallbacksRva = static_cast<DWORD>(
                tlsDir->AddressOfCallBacks - nt->OptionalHeader.ImageBase);
        }

        PVOID dataPtr = reinterpret_cast<PVOID>(
            reinterpret_cast<uintptr_t>(shellMem) + shellSize);

        IndirectNtWriteVirtualMemory(hProcess, shellMem,
            reinterpret_cast<PVOID>(LoaderShellcode), shellSize, &written);
        IndirectNtWriteVirtualMemory(hProcess, dataPtr,
            &ld, sizeof(ld), &written);

        // 7. Execute loader in remote thread
        HANDLE hThread = nullptr;
        IndirectNtCreateThreadEx(
            &hThread, THREAD_ALL_ACCESS, nullptr,
            hProcess,
            shellMem,   // start address
            dataPtr,    // parameter (LoaderData*)
            0, 0, 0, 0, nullptr);

        if (hThread) {
            WaitForSingleObject(hThread, 10000);
            IndirectNtClose(hThread);
        }

        return hThread != nullptr;
    }

} // namespace ManualMap