#pragma once
#include "Scanner.hpp"
#include <cstdint>
#include <string>

// Offsets for RobloxPlayerBeta.exe version-8884371d30284041
// Static addresses are relative to module base (RobloxPlayerBeta.exe).
// Field offsets are from object pointer.

namespace offsets {

    // ── Static pointers (module base + offset) ───────────────────────────────
    inline constexpr uintptr_t TaskSchedulerPointer     = 0x815c668;
    inline constexpr uintptr_t JobsPointer              = 0x815c668; // same as TS
    inline constexpr uintptr_t FakeDataModelPointer     = 0x7bcf6a8;
    inline constexpr uintptr_t VisualEnginePointer      = 0x82ea3f8;
    inline constexpr uintptr_t PhysicsSenderMaxBandwidth= 0x77844c0;

    // ── TaskScheduler fields ─────────────────────────────────────────────────
    inline constexpr uintptr_t TaskSchedulerMaxFPS      = 0xb0;
    inline constexpr uintptr_t HeartbeatTask            = 0xf8;
    inline constexpr uintptr_t HeartbeatFPS             = 0xb8;
    inline constexpr uintptr_t JobStart                 = 0xc8;
    inline constexpr uintptr_t JobEnd                   = 0xd0;
    inline constexpr uintptr_t Job_Name                 = 0x18;

    // ── DataModel / FakeDataModel ─────────────────────────────────────────────
    inline constexpr uintptr_t FakeDataModel            = 0xa90;
    inline constexpr uintptr_t FakeDataModelToDataModel = 0x1d8;
    inline constexpr uintptr_t ScriptContext            = 0x440; // DataModel → ScriptContext
    inline constexpr uintptr_t Camera                  = 0x4a8;
    inline constexpr uintptr_t Workspace               = 0x178;
    inline constexpr uintptr_t LocalPlayer             = 0x148;
    inline constexpr uintptr_t GameLoaded              = 0x678;
    inline constexpr uintptr_t PlaceId                 = 0x1a8;
    inline constexpr uintptr_t GameId                  = 0x1a0;
    inline constexpr uintptr_t JobId                   = 0x138;

    // ── Instance base fields ──────────────────────────────────────────────────
    inline constexpr uintptr_t ClassDescriptor         = 0x18;
    inline constexpr uintptr_t ClassDescriptorToClassName = 0x8;
    inline constexpr uintptr_t Name                    = 0xb0;
    inline constexpr uintptr_t NameSize                = 0x10;
    inline constexpr uintptr_t Parent                  = 0x70;
    inline constexpr uintptr_t Children                = 0x78;
    inline constexpr uintptr_t ChildrenEnd             = 0x8;   // offset from Children ptr
    inline constexpr uintptr_t AttributeContainer      = 0x48;

    // ── Humanoid ─────────────────────────────────────────────────────────────
    inline constexpr uintptr_t Health                  = 0x194;
    inline constexpr uintptr_t MaxHealth               = 0x1b4;
    inline constexpr uintptr_t WalkSpeed               = 0x1dc;
    inline constexpr uintptr_t JumpPower               = 0x1b0;
    inline constexpr uintptr_t JumpHeight              = 0x1ac;
    inline constexpr uintptr_t HipHeight               = 0x1a0;
    inline constexpr uintptr_t HumanoidRootPartRef     = 0x480;
    inline constexpr uintptr_t HumanoidState           = 0x8a0;
    inline constexpr uintptr_t HumanoidStateId         = 0x20;
    inline constexpr uintptr_t IsWalking               = 0x91f;
    inline constexpr uintptr_t MoveDirection           = 0x158;
    inline constexpr uintptr_t Sit                     = 0x1e9;
    inline constexpr uintptr_t Jump                    = 0x1e6;
    inline constexpr uintptr_t UseJumpPower            = 0x1ec;
    inline constexpr uintptr_t RigType                 = 0x1cc;
    inline constexpr uintptr_t WalkSpeedCheck          = 0x3c4;

    // ── BasePart ─────────────────────────────────────────────────────────────
    inline constexpr uintptr_t CFrame                  = 0xc8;
    inline constexpr uintptr_t Position                = 0xec;
    inline constexpr uintptr_t Velocity                = 0xf8;
    inline constexpr uintptr_t PartSize                = 0x1b8;
    inline constexpr uintptr_t Transparency            = 0xf0;
    inline constexpr uintptr_t Anchored                = 0x2;
    inline constexpr uintptr_t CanCollide              = 0x8;
    inline constexpr uintptr_t CanQuery                = 0x20;
    inline constexpr uintptr_t CanTouch                = 0x10;
    inline constexpr uintptr_t BasePartCastShadow      = 0xf5;
    inline constexpr uintptr_t BasePartLocked          = 0xf6;
    inline constexpr uintptr_t BasePartMassless        = 0xf7;
    inline constexpr uintptr_t BasePartReflectance     = 0xec;
    inline constexpr uintptr_t Primitive               = 0x148;
    inline constexpr uintptr_t PrimitiveAssemblyLinearVelocity  = 0xf8;
    inline constexpr uintptr_t PrimitiveAssemblyAngularVelocity = 0x104;
    inline constexpr uintptr_t PrimitiveFlags          = 0x1b6;

    // ── Camera ───────────────────────────────────────────────────────────────
    inline constexpr uintptr_t CameraPos               = 0x11c;
    inline constexpr uintptr_t CameraRotation          = 0xf8;
    inline constexpr uintptr_t CameraSubject           = 0xe8;
    inline constexpr uintptr_t CameraType              = 0x158;
    inline constexpr uintptr_t FOV                     = 0x160;
    inline constexpr uintptr_t CameraViewport          = 0x2ac;
    inline constexpr uintptr_t ViewportSize            = 0x2e8;
    inline constexpr uintptr_t viewmatrix              = 0x150;

    // ── Player ───────────────────────────────────────────────────────────────
    inline constexpr uintptr_t UserId                  = 0x2f8;
    inline constexpr uintptr_t AccountAge              = 0x34c;
    inline constexpr uintptr_t DisplayName             = 0x150;
    inline constexpr uintptr_t Team                    = 0x2d0;
    inline constexpr uintptr_t TeamColor               = 0x398;
    inline constexpr uintptr_t PlayerMouse             = 0x1188;
    inline constexpr uintptr_t MousePosition           = 0xec;

    // ── Scripts ───────────────────────────────────────────────────────────────
    inline constexpr uintptr_t LocalScriptByteCode     = 0x1a8;
    inline constexpr uintptr_t LocalScriptBytecodePointer = 0x10;
    inline constexpr uintptr_t LocalScriptGUID         = 0xe8;
    inline constexpr uintptr_t LocalScriptHash         = 0x1b8;
    inline constexpr uintptr_t ModuleScriptByteCode    = 0x150;
    inline constexpr uintptr_t ModuleScriptBytecodePointer = 0x10;
    inline constexpr uintptr_t ModuleScriptGUID        = 0xe8;
    inline constexpr uintptr_t ModuleScriptHash        = 0x160;
    inline constexpr uintptr_t ScriptByteCode          = 0x1a8;
    inline constexpr uintptr_t ScriptGUID              = 0xe8;
    inline constexpr uintptr_t ScriptHash              = 0x1b8;
    inline constexpr uintptr_t ByteCode_Pointer        = 0x10;
    inline constexpr uintptr_t ByteCode_Size           = 0x20;
    inline constexpr uintptr_t Source                  = 0x17c;
    inline constexpr uintptr_t IsCoreScript            = 0x0;

    // ── GUI ───────────────────────────────────────────────────────────────────
    inline constexpr uintptr_t AbsolutePosition        = 0x110;
    inline constexpr uintptr_t AbsoluteSize            = 0x118;
    inline constexpr uintptr_t AbsoluteRotation        = 0x188;
    inline constexpr uintptr_t GuiPosition             = 0x510;
    inline constexpr uintptr_t GuiSize                 = 0x530;
    inline constexpr uintptr_t GuiRotation             = 0x188;
    inline constexpr uintptr_t BackgroundColor3        = 0x540;
    inline constexpr uintptr_t BackgroundTransparency  = 0x54c;
    inline constexpr uintptr_t TextLabelText           = 0xda0;
    inline constexpr uintptr_t TextColor3              = 0xe50;
    inline constexpr uintptr_t TextLabelVisible        = 0x5ad;
    inline constexpr uintptr_t ScreenGui_Enabled       = 0x4c4;
    inline constexpr uintptr_t ZIndex                  = 0x19b;
    inline constexpr uintptr_t LayoutOrder             = 0x580;
    inline constexpr uintptr_t RichText                = 0xb50;
    inline constexpr uintptr_t Image                   = 0x988;

    // ── Lighting / Atmosphere ─────────────────────────────────────────────────
    inline constexpr uintptr_t LightingAmbient         = 0xe0;
    inline constexpr uintptr_t LightingBrightness      = 0x128;
    inline constexpr uintptr_t OutdoorAmbient          = 0x110;
    inline constexpr uintptr_t FogColor                = 0x104;
    inline constexpr uintptr_t FogEnd                  = 0x13c;
    inline constexpr uintptr_t FogStart                = 0x140;
    inline constexpr uintptr_t GlobalShadows           = 0x150;
    inline constexpr uintptr_t ClockTime               = 0x1c0;
    inline constexpr uintptr_t GeographicLatitude      = 0x198;
    inline constexpr uintptr_t AtmosphereDensity       = 0xe8;
    inline constexpr uintptr_t AtmosphereHaze          = 0xf0;
    inline constexpr uintptr_t AtmosphereDecay         = 0xdc;
    inline constexpr uintptr_t AtmosphereGlare         = 0xec;
    inline constexpr uintptr_t AtmosphereOffset        = 0xf4;
    inline constexpr uintptr_t AtmosphereColor         = 0xd0;

    // ── Physics / World ───────────────────────────────────────────────────────
    inline constexpr uintptr_t Gravity                 = 0x210;
    inline constexpr uintptr_t ReadOnlyGravity         = 0x9e8;
    inline constexpr uintptr_t FallenPartsDestroyHeight= 0x208;
    inline constexpr uintptr_t worldStepsPerSec        = 0x680;
    inline constexpr uintptr_t GlobalWind              = 0x3c;

    // ── ProximityPrompt ───────────────────────────────────────────────────────
    inline constexpr uintptr_t ProximityPromptEnabled           = 0x14e;
    inline constexpr uintptr_t ProximityPromptMaxActivationDistance = 0x140;
    inline constexpr uintptr_t ProximityPromptHoldDuraction     = 0x138;
    inline constexpr uintptr_t ProximityPromptActionText        = 0xc8;
    inline constexpr uintptr_t ProximityPromptGamepadKeyCode    = 0x134;
    inline constexpr uintptr_t RequiresLineOfSight              = 0x14f;

    // ── ClickDetector ─────────────────────────────────────────────────────────
    inline constexpr uintptr_t ClickDetectorMaxActivationDistance = 0x100;

    // ── Server / Network ─────────────────────────────────────────────────────
    inline constexpr uintptr_t ServerIP                = 0x660;
    inline constexpr uintptr_t DistributedGameTime     = 0x4c8;
    inline constexpr uintptr_t PlaceVersion            = 0x1c4;
    inline constexpr uintptr_t CreatorId               = 0x198;

    // ── Render ───────────────────────────────────────────────────────────────
    inline constexpr uintptr_t RenderView              = 0xbb8;
    inline constexpr uintptr_t DataModelToRenderView1  = 0x1e0;
    inline constexpr uintptr_t DataModelToRenderView2  = 0x8;
    inline constexpr uintptr_t DataModelToRenderView3  = 0x28;
    inline constexpr uintptr_t RenderJobRealDataModel  = 0x1c8;
    inline constexpr uintptr_t RenderJobRenderView     = 0x1d0;
    inline constexpr uintptr_t RenderJobFakeDataModel  = 0x38;
    inline constexpr uintptr_t VisualEngine            = 0x10;

} // namespace offsets


// ── Lua VM function pointers (resolved at runtime from decrypted process memory) ─

namespace LuaOffsets {

    inline uintptr_t rLuaL_loadbuffer = 0;
    inline uintptr_t rLuaD_call       = 0;
    inline uintptr_t rLuaD_pcall      = 0;
    inline uintptr_t rLuaE_newthread  = 0;

    // ── String-reference based finder ────────────────────────────────────────
    // Locates a function by finding a LEA instruction that loads a known string
    // literal, then walks backward through INT3 padding to the function start.
    // Works on the decrypted in-memory binary (disk scan fails because Hyperion
    // encrypts the .text section on disk but decrypts it before execution).

    static uintptr_t FindViaStringRef(uintptr_t base, const char* needle) {
        auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
        auto nt  = reinterpret_cast<IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
        size_t nlen = strlen(needle);

        // Step 1 — locate the string in any non-executable section
        uintptr_t strAddr = 0;
        {
            auto sec = IMAGE_FIRST_SECTION(nt);
            for (WORD i = 0; i < nt->FileHeader.NumberOfSections && !strAddr; ++i, ++sec) {
                if (!(sec->Characteristics & IMAGE_SCN_MEM_READ))   continue;
                if (  sec->Characteristics & IMAGE_SCN_MEM_EXECUTE) continue;
                auto start = base + sec->VirtualAddress;
                auto end   = start + sec->Misc.VirtualSize;
                for (auto p = start; p + nlen <= end; ++p)
                    if (memcmp(reinterpret_cast<void*>(p), needle, nlen) == 0)
                        { strAddr = p; break; }
            }
        }
        if (!strAddr) return 0;

        // Step 2 — scan executable sections for a RIP-relative LEA that targets strAddr
        //   Encoding: [REX] 8D /r (mod=00, rm=101) disp32
        //   REX byte is 0x48/0x49/0x4C/0x4D for 64-bit forms
        {
            auto sec = IMAGE_FIRST_SECTION(nt);
            for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++sec) {
                if (!(sec->Characteristics & IMAGE_SCN_MEM_EXECUTE)) continue;
                auto start = base + sec->VirtualAddress;
                auto end   = start + sec->Misc.VirtualSize;

                for (auto p = start; p + 7 <= end; ++p) {
                    auto b = reinterpret_cast<const BYTE*>(p);
                    // REX prefix (48-4F) + LEA opcode (8D) + ModRM mod=00 rm=101
                    if (b[0] < 0x48 || b[0] > 0x4F) continue;
                    if (b[1] != 0x8D)                continue;
                    if ((b[2] & 0xC7) != 0x05)       continue;   // mod=00, rm=101

                    INT32 disp = *reinterpret_cast<const INT32*>(p + 3);
                    if (reinterpret_cast<uintptr_t>(b + 7) + disp != strAddr) continue;

                    // Found the LEA — walk backward to function start.
                    // MSVC pads functions with 0xCC (INT3) bytes.
                    auto scan = reinterpret_cast<const BYTE*>(p);
                    uintptr_t limit = (p - start > 4096) ? p - 4096 : start;
                    while (reinterpret_cast<uintptr_t>(scan) > limit) {
                        --scan;
                        if (*scan == 0xCC) {
                            uintptr_t candidate = reinterpret_cast<uintptr_t>(scan + 1);
                            // Sanity: must be >= 4 bytes before the LEA and < 4096 away
                            if (candidate < p && p - candidate < 4096)
                                return candidate;
                        }
                    }
                    return p; // fallback: return address of the LEA itself
                }
            }
        }
        return 0;
    }

    // ── Follow a CALL rel32 at callSite+offset to its target ─────────────────
    static uintptr_t FollowCall(uintptr_t callSite, int opcodeOff = 0) {
        // call rel32 = E8 xx xx xx xx
        if (*reinterpret_cast<BYTE*>(callSite + opcodeOff) != 0xE8) return 0;
        INT32 rel = *reinterpret_cast<INT32*>(callSite + opcodeOff + 1);
        return callSite + opcodeOff + 5 + rel;
    }

    // ── Find luaL_loadbuffer via "=(load)" string referenced from loadstring ──
    // luaL_loadstring calls luaL_loadbuffer(L, s, strlen(s), "=(load)").
    // We find the LEA that loads "=(load)" and then the CALL right after it.
    static uintptr_t FindLoadBuffer(uintptr_t base) {
        auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
        auto nt  = reinterpret_cast<IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);

        // Find "=(load)" string
        const char* needle = "=(load)";
        size_t nlen = strlen(needle);
        uintptr_t strAddr = 0;
        {
            auto sec = IMAGE_FIRST_SECTION(nt);
            for (WORD i = 0; i < nt->FileHeader.NumberOfSections && !strAddr; ++i, ++sec) {
                if (!(sec->Characteristics & IMAGE_SCN_MEM_READ))   continue;
                if (  sec->Characteristics & IMAGE_SCN_MEM_EXECUTE) continue;
                auto start = base + sec->VirtualAddress;
                auto end   = start + sec->Misc.VirtualSize;
                for (auto p = start; p + nlen <= end; ++p)
                    if (memcmp(reinterpret_cast<void*>(p), needle, nlen) == 0)
                        { strAddr = p; break; }
            }
        }
        if (!strAddr) return 0;

        // Find the LEA then look for CALL within 32 bytes after it
        auto sec = IMAGE_FIRST_SECTION(nt);
        for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++sec) {
            if (!(sec->Characteristics & IMAGE_SCN_MEM_EXECUTE)) continue;
            auto start = base + sec->VirtualAddress;
            auto end   = start + sec->Misc.VirtualSize;

            for (auto p = start; p + 7 <= end; ++p) {
                auto b = reinterpret_cast<const BYTE*>(p);
                if (b[0] < 0x48 || b[0] > 0x4F) continue;
                if (b[1] != 0x8D)                continue;
                if ((b[2] & 0xC7) != 0x05)       continue;
                INT32 disp = *reinterpret_cast<const INT32*>(p + 3);
                if (reinterpret_cast<uintptr_t>(b + 7) + disp != strAddr) continue;

                // Scan up to 32 bytes after LEA for a CALL rel32 (E8)
                for (int off = 7; off < 40; ++off) {
                    uintptr_t addr = p + off;
                    if (addr + 5 > end) break;
                    if (*reinterpret_cast<const BYTE*>(addr) == 0xE8) {
                        uintptr_t target = FollowCall(addr);
                        if (target > base && target < base + 0xC000000)
                            return target;
                    }
                }
            }
        }
        return 0;
    }

    inline bool Init() {
        static bool s_done = false;
        if (s_done) return rLuaD_call != 0;
        s_done = true;

        uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandleA("RobloxPlayerBeta.exe"));

        rLuaD_call       = FindViaStringRef(base, "C stack overflow");
        rLuaD_pcall      = FindViaStringRef(base, "error in error handling");
        rLuaL_loadbuffer = FindLoadBuffer(base);
        rLuaE_newthread  = 0; // no reliable string ref; left for manual pattern update

        // Write diagnostic log → %TEMP%\PrivateExec_addrs.txt
        // Safe RVA helper: avoids underflow when addr == 0
        auto rva = [base](uintptr_t addr) -> uintptr_t {
            return addr ? addr - base : 0;
        };

        char tmp[MAX_PATH]{};
        GetTempPathA(MAX_PATH, tmp);
        char logPath[MAX_PATH]{};
        sprintf_s(logPath, "%sPrivateExec_addrs.txt", tmp);

        char buf[512]{};
        sprintf_s(buf,
            "base           = 0x%llX\n"
            "luaD_call      = 0x%llX  (RVA 0x%llX)\n"
            "luaD_pcall     = 0x%llX  (RVA 0x%llX)\n"
            "luaL_loadbuffer= 0x%llX  (RVA 0x%llX)\n"
            "luaE_newthread = 0x%llX  (RVA 0x%llX)\n",
            base,
            rLuaD_call,       rva(rLuaD_call),
            rLuaD_pcall,      rva(rLuaD_pcall),
            rLuaL_loadbuffer, rva(rLuaL_loadbuffer),
            rLuaE_newthread,  rva(rLuaE_newthread));

        HANDLE hf = CreateFileA(logPath, GENERIC_WRITE, 0, nullptr,
                                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hf != INVALID_HANDLE_VALUE) {
            DWORD w; WriteFile(hf, buf, (DWORD)strlen(buf), &w, nullptr);
            CloseHandle(hf);
        }
        OutputDebugStringA(buf);

        return rLuaD_call != 0; // minimum: need dcall; loadbuffer will also log 0 if missing
    }

} // namespace LuaOffsets


// ── Runtime helpers using the offsets ───────────────────────────────────────
namespace Runtime {

    // Get module base of RobloxPlayerBeta.exe
    inline uintptr_t ModuleBase() {
        return reinterpret_cast<uintptr_t>(GetModuleHandleA("RobloxPlayerBeta.exe"));
    }

    // Follow: module_base + FakeDataModelPointer → FakeDataModel + FakeDataModelToDataModel
    inline uintptr_t GetDataModel() {
        uintptr_t base  = ModuleBase();
        uintptr_t fdmp  = *reinterpret_cast<uintptr_t*>(base + offsets::FakeDataModelPointer);
        uintptr_t fdm   = *reinterpret_cast<uintptr_t*>(fdmp + offsets::FakeDataModel);
        return *reinterpret_cast<uintptr_t*>(fdm + offsets::FakeDataModelToDataModel);
    }

    // DataModel + ScriptContext offset → ScriptContext pointer
    inline uintptr_t GetScriptContext() {
        uintptr_t dm = GetDataModel();
        if (!dm) return 0;
        return *reinterpret_cast<uintptr_t*>(dm + offsets::ScriptContext);
    }

    // TaskScheduler singleton
    inline uintptr_t GetTaskScheduler() {
        uintptr_t base = ModuleBase();
        return *reinterpret_cast<uintptr_t*>(base + offsets::TaskSchedulerPointer);
    }

    // Unlock FPS — set TaskScheduler::maxFPS to 0 (uncapped)
    inline void UnlockFPS() {
        uintptr_t ts = GetTaskScheduler();
        if (!ts) return;
        *reinterpret_cast<double*>(ts + offsets::TaskSchedulerMaxFPS) = 0.0;
        OutputDebugStringA("[Runtime] FPS unlocked");
    }

    // Read instance name
    inline std::string GetName(uintptr_t inst) {
        if (!inst) return "";
        auto strPtr = *reinterpret_cast<uintptr_t*>(inst + offsets::Name);
        auto len    = *reinterpret_cast<size_t*>(inst + offsets::Name + offsets::NameSize);
        if (!strPtr || len == 0 || len > 512) return "";
        return std::string(reinterpret_cast<const char*>(strPtr), len);
    }

    // Read class name (renamed to avoid conflict with Win32 GetClassName macro)
    inline std::string GetInstanceClass(uintptr_t inst) {
        if (!inst) return "";
        auto desc      = *reinterpret_cast<uintptr_t*>(inst + offsets::ClassDescriptor);
        auto namePtr   = *reinterpret_cast<uintptr_t*>(desc + offsets::ClassDescriptorToClassName);
        if (!namePtr) return "";
        return std::string(reinterpret_cast<const char*>(namePtr));
    }

} // namespace Runtime