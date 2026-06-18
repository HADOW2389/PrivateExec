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


// ── Lua VM function pointers (resolved via AoB at runtime) ──────────────────
// These are code addresses inside RobloxPlayerBeta.exe, NOT field offsets.
// Patterns are for version-8884371d30284041 — update if the binary changes.

namespace LuaOffsets {

    inline uintptr_t rLuaL_loadbuffer  = 0;
    inline uintptr_t rLuaD_call        = 0;
    inline uintptr_t rLuaD_pcall       = 0;
    inline uintptr_t rLuaE_newthread   = 0;
    inline uintptr_t rLua_newthread    = 0;
    inline uintptr_t rLuaO_nilobject   = 0;
    inline uintptr_t rLua_getfield     = 0;
    inline uintptr_t rLua_setfield     = 0;
    inline uintptr_t rLua_pushstring   = 0;
    inline uintptr_t rLua_pushnumber   = 0;
    inline uintptr_t rLua_settop       = 0;
    inline uintptr_t rLua_gettop       = 0;
    inline uintptr_t rLua_type         = 0;
    inline uintptr_t rLua_tostring     = 0;
    inline uintptr_t rLua_tonumber     = 0;

    // AoB patterns for version-8884371d30284041
    // Verified against the binary — update after re-scan if needed.

    constexpr auto PAT_LOADBUFFER =
        "55 8B EC 83 E4 F8 83 EC 1C 53 56 57 8B 7D 08 8B 75 10";

    constexpr auto PAT_LUAD_CALL =
        "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 49 8B F0 8B DA 48 8B F9";

    constexpr auto PAT_LUAD_PCALL =
        "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 20 41 8B E9";

    constexpr auto PAT_LUAE_NEWTHREAD =
        "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B F9 33 F6 E8 ?? ?? ?? ??";

    constexpr auto PAT_LUA_GETFIELD =
        "48 89 5C 24 08 57 48 83 EC 20 48 8B FA 48 8B D9 E8 ?? ?? ?? ?? 48 8B CB";

    constexpr auto PAT_LUA_SETFIELD =
        "48 89 5C 24 10 48 89 74 24 18 57 48 83 EC 20 48 8B D9 41 8B F0 8B FA";

    inline bool Init() {
        const char* mod = "RobloxPlayerBeta.exe";

        rLuaL_loadbuffer = Scanner::ScanModule(mod, PAT_LOADBUFFER);
        rLuaD_call       = Scanner::ScanModule(mod, PAT_LUAD_CALL);
        rLuaD_pcall      = Scanner::ScanModule(mod, PAT_LUAD_PCALL);
        rLuaE_newthread  = Scanner::ScanModule(mod, PAT_LUAE_NEWTHREAD);
        rLua_getfield    = Scanner::ScanModule(mod, PAT_LUA_GETFIELD);
        rLua_setfield    = Scanner::ScanModule(mod, PAT_LUA_SETFIELD);

        // Log what we found / missed
        char buf[256];
        sprintf_s(buf, "[Offsets] loadbuffer=%llX dcall=%llX pcall=%llX newthread=%llX",
            rLuaL_loadbuffer, rLuaD_call, rLuaD_pcall, rLuaE_newthread);
        OutputDebugStringA(buf);

        // At minimum we need loadbuffer + dcall to execute scripts
        return rLuaL_loadbuffer && rLuaD_call;
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