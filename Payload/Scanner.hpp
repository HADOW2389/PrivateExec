#pragma once
#include <Windows.h>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

// AoB (Array of Bytes) scanner for Roblox version-8884371d30284041.
// Patterns are searched within the RobloxPlayerBeta.exe module image.
// Wildcards represented as "??" in the pattern string.

namespace Scanner {

    // Compile a pattern string "48 8B 05 ?? ?? ?? ??" into bytes + mask
    struct Pattern {
        std::vector<BYTE> bytes;
        std::vector<bool> mask; // true = match, false = wildcard

        static Pattern Parse(std::string_view pat) {
            Pattern p;
            size_t i = 0;
            while (i < pat.size()) {
                while (i < pat.size() && pat[i] == ' ') ++i;
                if (i + 1 >= pat.size()) break;
                if (pat[i] == '?' && pat[i+1] == '?') {
                    p.bytes.push_back(0);
                    p.mask.push_back(false);
                    i += 2;
                } else {
                    auto nibble = [](char c) -> BYTE {
                        if (c >= '0' && c <= '9') return c - '0';
                        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                        return 0;
                    };
                    p.bytes.push_back((nibble(pat[i]) << 4) | nibble(pat[i+1]));
                    p.mask.push_back(true);
                    i += 2;
                }
            }
            return p;
        }
    };

    // Scan [start, start+size) for the pattern; return VA of first match or 0
    inline uintptr_t Scan(uintptr_t start, size_t size, const Pattern& pat) {
        if (pat.bytes.empty() || size < pat.bytes.size()) return 0;
        size_t len = pat.bytes.size();
        auto mem = reinterpret_cast<const BYTE*>(start);
        for (size_t i = 0; i <= size - len; ++i) {
            bool found = true;
            for (size_t j = 0; j < len; ++j) {
                if (pat.mask[j] && mem[i + j] != pat.bytes[j]) { found = false; break; }
            }
            if (found) return start + i;
        }
        return 0;
    }

    // Scan the .text section of the specified module
    inline uintptr_t ScanModule(const char* modName, std::string_view patStr) {
        HMODULE hMod = GetModuleHandleA(modName);
        if (!hMod) return 0;

        auto base = reinterpret_cast<uintptr_t>(hMod);
        auto dos  = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
        auto nt   = reinterpret_cast<IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);

        uintptr_t textStart = 0, textSize = 0;
        auto sec = IMAGE_FIRST_SECTION(nt);
        for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++sec) {
            // Scan all executable sections
            if (sec->Characteristics & IMAGE_SCN_MEM_EXECUTE) {
                uintptr_t addr = Scan(base + sec->VirtualAddress, sec->Misc.VirtualSize,
                                      Pattern::Parse(patStr));
                if (addr) return addr;
            }
        }
        return 0;
    }

    // Read a relative 32-bit offset and compute the target VA.
    // Used for resolving call/jmp rel32 instructions.
    // offset_from_pattern: byte offset within the match where rel32 begins
    // instruction_len: length of the instruction (to compute next IP)
    inline uintptr_t ResolveRip(uintptr_t matchAddr, int offsetFromPattern, int instrLen) {
        INT32 rel = *reinterpret_cast<INT32*>(matchAddr + offsetFromPattern);
        return matchAddr + offsetFromPattern + 4 + rel - (instrLen - offsetFromPattern - 4);
        // simpler: addr_of_next_instruction + rel32
    }

    inline uintptr_t RipRelative(uintptr_t instrStart, int rel32Offset) {
        INT32 rel = *reinterpret_cast<INT32*>(instrStart + rel32Offset);
        return instrStart + rel32Offset + 4 + rel;
    }

} // namespace Scanner